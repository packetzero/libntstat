//  NetworkStatisticsClient.cpp
//
//  Copyright © 2017 Alex Malone. All rights reserved.

// typical order of message received for a stream:
//
// T RECV <    0 type:SRC_ADDED(10001) len:28 srcRef:20
// ..
// T RECV <    0 type:SRC_COUNTS(10004) len:144 srcRef:20
// T RECV <    0 type:SRC_DESC(10003) len:296 srcRef:20
// T RECV <    0 type:SRC_REMOVED(10002) len:24 srcRef:20


#include "NTStatKernelStructHandler.hpp"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/utsname.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>

#include <string.h> // memcmp
#include <string>
#include <map>
#include <vector>
using namespace std;

// references to the factory functions to allocate struct handlers for kernel versions

NTStatKernelStructHandler* NewNTStatKernel2422();
NTStatKernelStructHandler* NewNTStatKernel2782();
NTStatKernelStructHandler* NewNTStatKernel3789();
NTStatKernelStructHandler* NewNTStatKernel3248();
NTStatKernelStructHandler* NewNTStatKernel4570();

// minimum ntstat.h definitions needed here

#define      NET_STAT_CONTROL_NAME   "com.apple.network.statistics"

enum
{
  // generic response messages
  NSTAT_MSG_TYPE_SUCCESS                  = 0
  ,NSTAT_MSG_TYPE_ERROR                   = 1

  // Requests
  ,NSTAT_MSG_TYPE_ADD_SRC                 = 1001
  ,NSTAT_MSG_TYPE_ADD_ALL_SRCS            = 1002
  ,NSTAT_MSG_TYPE_REM_SRC                 = 1003
  ,NSTAT_MSG_TYPE_QUERY_SRC               = 1004
  ,NSTAT_MSG_TYPE_GET_SRC_DESC            = 1005

  // Responses/Notfications
  ,NSTAT_MSG_TYPE_SRC_ADDED               = 10001
  ,NSTAT_MSG_TYPE_SRC_REMOVED             = 10002
  ,NSTAT_MSG_TYPE_SRC_DESC                = 10003
  ,NSTAT_MSG_TYPE_SRC_COUNTS              = 10004
};


typedef struct nstat_msg_error
{
  nstat_msg_hdr   hdr;
  u_int32_t               error;  // errno error
} nstat_msg_error;

// local defs

#define REMOVED_SOURCE_KEEP_SECONDS 30
#define CLEANUP_SOURCE_LIST_SECONDS 60
#define UPDATE_STATS_INTERVAL_SECONDS 30

const int BUFSIZE = 2048;

string msg_name(uint32_t msg_type);
char msg_dir(uint32_t msg_type);
unsigned int getXnuVersion();

#define LOG_ERROR(a)     if (_logFlags & NTSTAT_LOGF_ERROR) printf a
#define LOG_SENDRECV(a)  if (_logFlags & NTSTAT_LOGF_SENDRECV) printf a
#define LOG_DEBUG(a)     if (_logFlags & NTSTAT_LOGF_DEBUG) printf a
#define LOG_TRACE(a)     if (_logFlags & NTSTAT_LOGF_TRACE) printf a
#define LOG_DROPS(a)     if (_logFlags & NTSTAT_LOGF_DROPS) printf a

/*
 * Wrapper around NTStatStream so we can track srcRef
 */
struct NetstatSource
{
  NetstatSource(uint64_t srcRef, uint32_t providerId) : _srcRef(srcRef), _providerId(providerId), obj(),
   _haveDesc(false), _haveNotifiedAdded(false), _requestedCount(false), _tsAdded(0L), _tsRemoved(0L), _tsLastUpdate(0L) {}

  uint64_t _srcRef;
  uint32_t _providerId;
  NTStatStream obj;

  bool     _haveDesc;
  bool     _haveNotifiedAdded;
  bool     _requestedCount;

  time_t   _tsAdded;
  time_t   _tsRemoved;
  time_t   _tsLastUpdate;
};

// tracking of messages

struct QMsg
{
  uint64_t         seqnum;
  vector<uint8_t>  msgbytes;
  NetstatSource*   ntsrc;
};

typedef enum {
  STATE_START
  ,STATE_REQUEST_IFNET_SRC
  ,STATE_REQUEST_TCP_SRC
  ,STATE_REQUEST_UDP_SRC
  ,STATE_RUNNING
} state_t;

#define NOT_FATAL(errno) (ENOBUFS == (errno) || 0 == (errno))

/*
 * Implementation of NetworkStatisticsClient
 */
class NetworkStatisticsClientImpl : public NetworkStatisticsClient, public MsgDest
{
public:
  NetworkStatisticsClientImpl(NetworkStatisticsListener* listener): _listener(listener), _map(), _keepRunning(false),
   _fd(0), _seqnum(1), _qmsgMap(), _state(STATE_START),
   _wantTcp(true), _wantUdp(false), _wantKernel(false), _updateIntervalSeconds(30),
   _recordEnabled(false), _recordFd(0), _numDrops(0), _numErrors(0),_logFlags(0),
   _mapWaitingForDesc(), _mapWaitingForCount()
  {
    INC_QMSG();
  }

  QMsg _workingMsg;
  void INC_QMSG(NetstatSource* src = 0L)
  {
    _workingMsg.seqnum = _seqnum;
    _workingMsg.msgbytes.clear();
    _workingMsg.ntsrc = src;
  }

  virtual void configure(bool wantTcp, bool wantUdp, uint32_t updateIntervalSeconds) {
    _wantTcp = wantTcp; _wantUdp = wantUdp; _updateIntervalSeconds = updateIntervalSeconds;
    if (_updateIntervalSeconds < 30) {
      printf("E Invalid updateIntervalSeconds (%d).  Using 30\n", updateIntervalSeconds);
      _updateIntervalSeconds = 30;
    }
  }

  // MsgDest::seqnum
  virtual uint64_t seqnum() { return _seqnum; }

  // MsgDest::send
  // The message gets enqueued, and later sent in sendNextMessage()
  virtual void send(nstat_msg_hdr* hdr, size_t len)
  {
    if (_inReplayMode()) return;

    LOG_TRACE(("T ENQ %s\n", _sprintMsg(hdr).c_str() ));

    // make copy of message bytes

    _workingMsg.msgbytes.resize(len);
    memcpy(_workingMsg.msgbytes.data(), hdr, len);

    // enqueue

    _workingMsg.seqnum = hdr->context;
    _outq.push_back(_workingMsg);

    // advance sequence number for each message

    _seqnum++;

    INC_QMSG();
  }

  bool _inReplayMode() { return (_recordFd > 0 && _recordEnabled == false); }

  //------------------------------------------------------------------------
  // returns true on success, false otherwise
  //------------------------------------------------------------------------
  bool connectToKernel()
  {
    // create socket

    if ((_fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL)) == -1) {
      fprintf(stderr,"socket(SYSPROTO_CONTROL): %s", strerror(errno));
      return false;

    }

    // init ctl_info

    struct ctl_info ctlInfo;
    memset(&ctlInfo, 0, sizeof(ctlInfo));

    // copy name and make sure the name isn't too long

    if (strlcpy(ctlInfo.ctl_name, NET_STAT_CONTROL_NAME, sizeof(ctlInfo.ctl_name)) >=
        sizeof(ctlInfo.ctl_name)) {
      fprintf(stderr,"CONTROL NAME too long");
      return false;
    }

    // iotcl ctl info
    if (ioctl(_fd, CTLIOCGINFO, &ctlInfo)) { //} == -1) {
      fprintf(stderr,"ioctl(CTLIOCGINFO): %s", strerror(errno));
      close(_fd);
      return false;
    }

    // connect socket

    struct sockaddr_ctl sc;
    memset(&sc, 0, sizeof(sc));
    sc.sc_id = ctlInfo.ctl_id;
    sc.sc_len = sizeof(sc);
    sc.sc_family = AF_SYSTEM;
    sc.ss_sysaddr = AF_SYS_CONTROL;

    sc.sc_unit = 0 ;           /* zero means unspecified */

    if (connect(_fd, (struct sockaddr *)&sc, sizeof(sc)) != 0)
    {
      fprintf(stderr,"connect(AF_SYS_CONTROL): %s\n", strerror(errno));
    } else {
      LOG_DEBUG(("socket id:%d unit:%d\n", ctlInfo.ctl_id, sc.sc_unit));
      return true;
    }

    // no dice

    close(_fd);
    return false;
  }

  //----------------------------------------------------------
  // return true if we have a socket connection active
  //----------------------------------------------------------
  bool isConnected()
  {
    return (_fd > 0);
  }

  void _loadStructHandler(unsigned int xnuVersion)
  {
    printf("XNU version:%d\n", xnuVersion);

    if (xnuVersion > 3800)
      _structHandler = NewNTStatKernel4570();
    else if (xnuVersion > 3300)
      _structHandler = NewNTStatKernel3789();
    else if (xnuVersion > 3200)
      _structHandler = NewNTStatKernel3248();
    else if (xnuVersion > 2700)
      _structHandler = NewNTStatKernel2782();
    else
      _structHandler = NewNTStatKernel2422();
  }
  
  //----------------------------------------------------------
  // Go through list of sources and request counts (stats)
  // for those who it's been at least 15 seconds since added
  // or updated.  NOTE: this gets called from run() after
  // _updateIntervalSeconds.
  //----------------------------------------------------------
  void _updateWaitForCountsQueue(time_t now)
  {
    for (auto it = _map.begin();it != _map.end(); it++)
    {
      NetstatSource* source = it->second;
      
      if (source->_tsRemoved != 0) continue;
      
      if (IS_LISTEN_PORT(&source->obj)) continue;
      
      if (source->obj.key.lport == 0 && source->obj.key.rport == 0) continue; // TODO: what are these?

      time_t delta = 0;
      if (0L == source->_tsLastUpdate)
        delta = now - source->_tsAdded;
      else
        delta = now - source->_tsLastUpdate;

      if (delta >= 15) {
        source->_requestedCount = true;
        _mapWaitingForCount[source->_srcRef] = source;
      }
    }
  }

  //----------------------------------------------------------
  // run
  //----------------------------------------------------------
  void run()
  {
    if (!isConnected()) {
      printf("E run() not connected.\n"); return;
    }

    _keepRunning = true;
    unsigned int xnuVersion = getXnuVersion();

    _loadStructHandler(xnuVersion);

    // need to start by subscribing to either UDP or TCP

    if (_wantTcp) _enterStateRequestTcpSrc();
    else _enterStateRequestUdpSrc();

    time_t tLastCleanup = time(NULL);
    time_t tLastUpdate = time(NULL);

    while (_keepRunning)
    {
      time_t now = time(NULL);

      if ((now - tLastCleanup) > CLEANUP_SOURCE_LIST_SECONDS) {
        tLastCleanup = now;
        _removeOldSources();
      }

      if (_updateIntervalSeconds > 0 && ((now - tLastUpdate) >= _updateIntervalSeconds)) {
        tLastUpdate = now;
        
        _updateWaitForCountsQueue(now);
      }

      sendNextMsg();

      while (haveIncomingMessage())
        _readNextMessage();
    }

    close(_fd);
    _fd = 0;
  }



private:

  //---------------------------------------------------------------
  // write message to socket fd
  // returns true if successful
  //---------------------------------------------------------------
  bool SEND(QMsg &qm)
  {
    nstat_msg_hdr* hdr = (nstat_msg_hdr*)qm.msgbytes.data();

    if (_logFlags & NTSTAT_LOGF_SENDRECV) {
      uint32_t providerId=0;
      uint64_t srcRef=0L;
      _structHandler->getSrcRef(hdr, (int)qm.msgbytes.size(), srcRef, providerId);
      LOG_SENDRECV(("T SEND %s\n", _sprintMsg(hdr, srcRef).c_str()));
    }

    if (_recordEnabled) RECORD(qm.msgbytes.data(), (unsigned int)qm.msgbytes.size());

    ssize_t rc = write (_fd, qm.msgbytes.data(), qm.msgbytes.size());

    // add to map so we can map responses to request

    _qmsgMap[qm.seqnum] = qm;

    return (rc == qm.msgbytes.size());
  }

  //----------------------------------------------------------
  // Take first QMsg from outq and send on socket
  // Adds QMsg to qmsgMap so we can look it up when the corresponding
  // ERROR/SUCCESS response arrives
  //----------------------------------------------------------
  void sendNextMsg()
  {
    // no message waiting, do we have any sources that need descriptions?
    
    if (_outq.empty() && !_mapWaitingForDesc.empty())
    {
      auto it = _mapWaitingForDesc.begin();
      NetstatSource* source = it->second;
      _structHandler->writeSrcDesc(*this, source->_providerId, source->_srcRef);
      _mapWaitingForDesc.erase(it++);
    }

    // no message waiting, do we have any sources that need count updates?
    
    if (_outq.empty() && !_mapWaitingForCount.empty())
    {
      auto it = _mapWaitingForCount.begin();
      NetstatSource* source = it->second;

      // make sure we still want this data

      if (source->_tsRemoved == 0 && source->_requestedCount) {
        _structHandler->writeQuerySrc(*this, source->_srcRef);
      }
      _mapWaitingForCount.erase(it++);
    }
    
    // send if we have
    
    while (!_outq.empty())
    {
      // get ref to first message in outq

      QMsg &qm = *_outq.begin();

      // try to write to KCQ socket

      if (!SEND(qm))
      {
        // error ... drop on floor
        LOG_ERROR(("E Failed to send\n"));
      }

      // pop off message sent
      _outq.erase(_outq.begin());

      return;
    }

  }

  //----------------------------------------------------------
  // markSourceForRemove
  //----------------------------------------------------------
  void _markSourceForRemove(uint64_t srcRef)
  {
    auto fit = _map.find(srcRef);
    if (fit != _map.end()) {
      fit->second->_tsRemoved = time(NULL);
    }
  }

  void _markSourceForRemove(NetstatSource *source)
  {
    source->_tsRemoved = time(NULL);
  }

  //----------------------------------------------------------
  // removeOldSources
  //----------------------------------------------------------
  void _removeOldSources()
  {
    time_t now = time(NULL);

    auto it = _map.begin();
    while (it != _map.end())
    {
      if (it->second->_tsRemoved > 0)
      {
        time_t delta = now - it->second->_tsRemoved;
        if (delta > REMOVED_SOURCE_KEEP_SECONDS) {
          _map.erase(it++);
          continue;
        }
      }
      it++;
    }
  }

  //----------------------------------------------------------
  // check KCQ socket to see if bytes ready for reading.
  //----------------------------------------------------------
  bool haveIncomingMessage()
  {
    fd_set  fds;
    struct timeval to;
    to.tv_sec = 0;
    to.tv_usec = 50000;
    FD_ZERO (&fds);
    FD_SET (_fd, &fds);

    // select on socket, rather than read..
    return (select(_fd +1, &fds, NULL, NULL, &to) > 0);
  }

  //----------------------------------------------------------
  // resetSource : allocate and assign to map
  //----------------------------------------------------------
  NetstatSource* _resetSource(uint64_t srcRef, uint32_t providerId)
  {
    NetstatSource* src = 0L;
    auto fit = _map.find(srcRef);
    if (fit != _map.end()) {
      // already have it
      if (fit->second->_tsRemoved > 0) {
        _map.erase(fit++);
      } else {
        LOG_ERROR(("add for existing src\n"));
        src = fit->second;
      }
    }

    if (0L == src) {
      src = new NetstatSource(srcRef, providerId);
      src->obj.id = srcRef;
      src->_tsAdded = time(NULL);
      _map[srcRef] = src;
    }

    return src;
  }

  //----------------------------------------------------------
  // lookupSource
  //----------------------------------------------------------
  NetstatSource* _lookupSource(uint64_t srcRef)
  {
    NetstatSource* retval = 0L;

    auto fit = _map.find(srcRef);
    if (fit != _map.end()) {
      retval = fit->second;
    }

    return retval;
  }

  //----------------------------------------------------------
  // return message detail string for consistent logging
  //----------------------------------------------------------
  std::string _sprintMsg(nstat_msg_hdr* hdr, uint64_t srcRef = 0L)
  {
    char tmp[256];

    if (hdr == 0L) return "NULL";

    sprintf(tmp, "%c %4llu type:%s(%d) hdr->len:%d srcRef:%llu", msg_dir(hdr->type), hdr->context, msg_name(hdr->type).c_str(), hdr->type, hdr->length, srcRef);

    return string(tmp);
  }

  //----------------------------------------------------------
  // Isolates read() on socket fd
  // If record-mode enabled, also writes to file
  //----------------------------------------------------------
  int _socketRead(char* dest, int destsize)
  {
    int num_bytes = (int)read (_fd, dest, destsize);

    LOG_DEBUG(("D READ %d bytes\n", num_bytes));

    if (num_bytes > 0 && _recordEnabled) RECORD(dest, num_bytes);

    return num_bytes;
  }

  //----------------------------------------------------------
  // RECORD()
  // persist message to file.
  //
  //   uint32_t timestamp
  //   uint32_t length
  //   char     data[length]
  //----------------------------------------------------------
  void RECORD(const void *src, uint32_t num_bytes)
  {
    uint32_t now = (uint32_t)time(NULL);      // hardcode to 32-bit for platform consistency
    write(_recordFd, &now, sizeof(now));
    write(_recordFd, &num_bytes, sizeof(num_bytes));

    write(_recordFd, src, num_bytes);
  }

  //----------------------------------------------------------
  // _readNextMessage()
  // The KCQ socket is really a queue.  This reads the next
  // message off the queue
  //----------------------------------------------------------
  int _readNextMessage()
  {
    char c[BUFSIZE];

    int num_bytes = _socketRead(c, BUFSIZE);
    if (num_bytes <= 0) return -1;

    return _handleResponseMessage((nstat_msg_hdr *) c, num_bytes);
  }
  
  //----------------------------------------------------------
  // Given the request message to ADD_ALL_SRC, extract
  // the providerId and request GET_SRC_DESC for ALL.
  //----------------------------------------------------------
  void _queryAllSrcDesc(nstat_msg_hdr* addAllSrcReqHdr, uint32_t reqMsgLen)
  {
    uint32_t providerId=0;
    uint64_t reqSrcRef=0L;
    _structHandler->getSrcRef(addAllSrcReqHdr, reqMsgLen, reqSrcRef, providerId);
    _structHandler->writeSrcDesc(*this, providerId, (uint64_t)-1);
  }

  //----------------------------------------------------------
  // returns errno code and logs if enabled
  //----------------------------------------------------------
  int _getAndLogError(nstat_msg_hdr* msgHdr, int num_bytes, QMsg &reqMsg)
  {
    nstat_msg_error* perr = (nstat_msg_error*)msgHdr;
    
    // consider special case errors

    _numErrors++;

    if (num_bytes < sizeof(nstat_msg_error))
    {
      LOG_ERROR(("E error struct size mismatch\n"));
      return EFAULT;
    }

    if (perr->error == ENOBUFS) {
      _numDrops++;
    }

    // log if desired

    if (_logFlags & NTSTAT_LOGF_ERROR) {
      if (perr->error == ENOBUFS)
      {
        LOG_DROPS(("  ENOBUFS drop\n"));
        LOG_ERROR(("T error ENOBUFS - app not keeping up\n"));
      } else {
        LOG_ERROR(("T error code:%d (0x%x) \n", perr->error, perr->error));
        if (reqMsg.msgbytes.size() > 0) {
          uint64_t requestSrcRef = (reqMsg.ntsrc != 0L) ?  reqMsg.ntsrc->_srcRef : 0L;
          LOG_ERROR(("  for REQUEST (%s) srcRef:%llu\n", _sprintMsg((nstat_msg_hdr*)reqMsg.msgbytes.data()).c_str(), requestSrcRef));
        }
      }
    }

    return perr->error;
  }

  //----------------------------------------------------------
  // _enterStateRequestUdpSrc
  // @returns 0
  //----------------------------------------------------------
  int _enterStateRequestUdpSrc()
  {
    _state = STATE_REQUEST_UDP_SRC;

    _structHandler->writeAddAllUdpSrc(*this);

    return 0;
  }

  //----------------------------------------------------------
  // _enterStateRequestTcpSrc
  // @returns 0
  //----------------------------------------------------------
  int _enterStateRequestTcpSrc()
  {
    _state = STATE_REQUEST_TCP_SRC;
    
    _structHandler->writeAddAllTcpSrc(*this);
    
    return 0;
  }

  
  //----------------------------------------------------------
  // _enterRunningState
  // The "running" state is mostly passive... listening to events.
  // We can get here even if there was an error on initialization.
  // @returns 0
  //----------------------------------------------------------
  int _enterRunningState(nstat_msg_hdr* reqHdr, int num_bytes, QMsg &reqMsg)
  {
    _state = STATE_RUNNING;
    return 0;
  }
  
  //----------------------------------------------------------
  // handleState
  //----------------------------------------------------------
  int _handleState(nstat_msg_hdr* msgHdr, int num_bytes, QMsg &reqMsg)
  {
    bool isSuccessResponse = (msgHdr->type == NSTAT_MSG_TYPE_SUCCESS && reqMsg.msgbytes.size() > 0);

    int err = (msgHdr->type == NSTAT_MSG_TYPE_ERROR ? _getAndLogError(msgHdr, num_bytes, reqMsg) : 0);

    switch(_state)
    {
      case STATE_REQUEST_IFNET_SRC:
      {
        if (_wantTcp) _enterStateRequestTcpSrc();
        else _enterStateRequestUdpSrc();
        break;
      }
      case STATE_REQUEST_TCP_SRC:
      {
        if (isSuccessResponse || NOT_FATAL(err))
        {
          if (_wantUdp)
            return _enterStateRequestUdpSrc();
          else
            return _enterRunningState(msgHdr, num_bytes, reqMsg);
        }
        else
        {
          return _enterRunningState(msgHdr, num_bytes, reqMsg);
        }
        break;
      }
      case STATE_REQUEST_UDP_SRC:
      {
        if (isSuccessResponse || NOT_FATAL(err))
        {
            return _enterRunningState(msgHdr, num_bytes, reqMsg);
        }
        else
        {
          return _enterRunningState(msgHdr, num_bytes, reqMsg);
        }
        break;
      }
      default:
        // unexpected.  Enter passive listening state ... _getAndLogError() above logs error
        return _enterRunningState(msgHdr, num_bytes, reqMsg);
        break;
    }
    
    return 0;
  }

  //----------------------------------------------------------
  //
  //----------------------------------------------------------
  void addToWaitingForDescQueue(NetstatSource* src)
  {
    _mapWaitingForDesc[src->_srcRef] = src;
  }
  
  void removeFromWaitingForDescQueue(NetstatSource* src)
  {
    _mapWaitingForDesc.erase(src->_srcRef); // TODO: exception if not found?
  }
  
  //----------------------------------------------------------
  // _handleResponseMessage()
  // Separated from _socketRead for testing purposes.
  //----------------------------------------------------------
  int _handleResponseMessage(nstat_msg_hdr* ns, int num_bytes)
  {
    uint64_t srcRef = 0L;
    uint32_t providerId = 0;
    _structHandler->getSrcRef(ns, num_bytes, srcRef, providerId);

    LOG_SENDRECV(("T RECV %s\n", _sprintMsg(ns, srcRef).c_str()));

    // get corresponding request message (if possible)
    // SRC_ADDED, SRC_REMOVED, SRC_DESC, SRC_COUNTS, etc. all have context == 0

    auto fit = _qmsgMap.find(ns->context);
    QMsg reqMsg;
    if (fit != _qmsgMap.end()) {
      reqMsg = fit->second;
      _qmsgMap.erase(fit);
    }

    // until we are in RUNNING state, handle changes

    if (_state != STATE_RUNNING && (ns->type == NSTAT_MSG_TYPE_ERROR || ns->type == NSTAT_MSG_TYPE_SUCCESS))
    {
      return _handleState(ns, num_bytes, reqMsg);
    }
    
    switch (ns->type)
    {

      case NSTAT_MSG_TYPE_SRC_ADDED:
      {
        // it's possible to get SRC_ADDED for one that you already have.
        // SRC_ADDED are typically sent (resent) right before the SRC_REMOVED

        NetstatSource* src = _resetSource(srcRef, providerId);
        if (!src->_haveDesc)
          addToWaitingForDescQueue(src);

      }
      break;
      case NSTAT_MSG_TYPE_SRC_REMOVED:
      {
        NetstatSource* source = _lookupSource(srcRef);

        if (source != 0L) {
          _markSourceForRemove(source);
          removeFromWaitingForDescQueue(source);

          if (!(source->obj.key.lport == 0 && source->obj.key.rport == 0)) {
            _listener->onStreamRemoved(&source->obj);
          }
        }
      }
      break;
      case NSTAT_MSG_TYPE_SRC_DESC:
      {
        NetstatSource* source = _lookupSource(srcRef);

        if (source != 0L)
        {
          removeFromWaitingForDescQueue(source);

          {
            if (_structHandler->readSrcDesc(ns, num_bytes, &source->obj))
            {
              source->_haveDesc = true;

              // misc cleanups

              if (source->obj.process.pid == 0) strcpy(source->obj.process.name, "kernel_task");

              // notify application (it not already done)

              if (!source->_haveNotifiedAdded) {
                if (source->obj.key.lport == 0 && source->obj.key.rport == 0) {
                  // ignore... TODO: not sure what these are.
                } else {
                  _listener->onStreamAdded(&source->obj);
                }
              }

              source->_haveNotifiedAdded = true;
            } else {
              LOG_DEBUG(("E not TCP or UDP provider:%u\n", providerId));
            }
          }
        } else {
          LOG_ERROR(("desc before src defined\n"));
        }
      }
      break;
      case NSTAT_MSG_TYPE_SRC_COUNTS:
      {
        NetstatSource* source = _lookupSource(srcRef);
        if (source != 0L) {

          _structHandler->readCounts(ns, num_bytes, source->obj.stats);

          if (source->_haveDesc) {

            if (source->_requestedCount && (source->obj.stats.rxpackets > 0 || source->obj.stats.txpackets > 0))
                _listener->onStreamStatsUpdate(&source->obj);

          } else {
            // typically we receive ADDED,COUNTS,DESC,REMOVED
            // So if we don't have DESC, we can't report anything yet.
          }

          // update count state

          source->_tsLastUpdate = time(NULL);
          source->_requestedCount = false;
        }
        else {
          LOG_ERROR(("counts before src defined\n"));
        }
      }
      break;
      case NSTAT_MSG_TYPE_SUCCESS:
      {
        // the success message doesn't tell us anything.. have to lookup request to get context (done above)
        
        if (reqMsg.msgbytes.size() > 0)
        {
          //nstat_msg_hdr* reqHdr = (nstat_msg_hdr*)reqMsg.msgbytes.data();

        } else {
          LOG_DEBUG(("E unhandled success response\n"));
        }
      }
      break;

      case NSTAT_MSG_TYPE_ERROR:
      {
        int err = _getAndLogError(ns, num_bytes, reqMsg);
        return (NOT_FATAL(err) ? 0 : -1);
      }
      default:
        LOG_ERROR(("E unknown message type:%d\n", ns->type));
        return -1;
    }

    return 0;
  }

  void enqueueRequestForSrcDesc(NetstatSource* source)
  {
    // first check to make sure we don't already have a request in flight

    for (auto it = _outq.begin(); it != _outq.end(); it++) {
      NetstatSource *tmp = it->ntsrc;
      nstat_msg_hdr *msghdr = (nstat_msg_hdr*)it->msgbytes.data();
      if (0L != tmp && msghdr->type == NSTAT_MSG_TYPE_GET_SRC_DESC && tmp->_srcRef == source->_srcRef) return;
    }

    // don't have any matching outstanding requests, so send it

    _workingMsg.ntsrc = source;
    _structHandler->writeSrcDesc(*this, source->_providerId, source->_srcRef);
  }

  virtual void stop() { _keepRunning = false; }

  //----------------------------------------------------------
  // enableRecording()
  //----------------------------------------------------------
  virtual void enableRecording()
  {
    char filename[64];
    sprintf(filename, "ntstat-xnu-%d.bin", getXnuVersion());

    _recordFd = open(filename, O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, 0664);
    if (_recordFd <= 0) {
      printf("ERROR: unable to open %s for writing\n", filename);
      return;
    }
    _recordEnabled = true;
  }

  //----------------------------------------------------------
  // emulate run() without an actual kernel connection by
  // reading and processing ntstat messages from file
  //----------------------------------------------------------
  virtual void runRecording(char *filename, unsigned int xnuVersion)
  {
    uint32_t lastMsgTimestamp = 0;
    _recordFd = open(filename, O_RDONLY);
    if (_recordFd <= 0) {
      printf("ERROR: unable to open %s for reading\n", filename);
      return;
    }

    _loadStructHandler(xnuVersion);

    // By default, use running state.  We try to detect based on first message below

    _state = STATE_RUNNING;

    int replayMsgCount = 0;
    while (true)
    {
      uint32_t msgTimestamp;
      uint32_t msgLength;

      // read timestamp
      int num_bytes = (int)read(_recordFd, &msgTimestamp, sizeof(msgTimestamp));
      if (num_bytes < sizeof(msgTimestamp)) {
        // end of file?
        break;
      }

      num_bytes = (int)read(_recordFd, &msgLength, sizeof(msgLength));
      if (num_bytes < sizeof(msgLength)) {
        printf("ERROR: invalid replay header\n");
        break;
      }

      // sanity check
      if (msgLength < sizeof(nstat_msg_hdr) ||  msgLength > 2048) {
        printf("ERROR: invalid length in recorded message: %d\n", msgLength);
        break;
      }

      // read message
      vector<uint8_t> vec;
      vec.resize(msgLength);
      nstat_msg_hdr *hdr = (nstat_msg_hdr*)vec.data();
      num_bytes = (int)read(_recordFd, (unsigned char*)vec.data(), msgLength);

      if (num_bytes != msgLength) {
        printf("WARN: partial msg in recording\n");
        break;
      }

      // if first message in file is an ADD_ALL, then assume it contains the start of a session

      if (replayMsgCount == 0 && hdr->type == NSTAT_MSG_TYPE_ADD_ALL_SRCS) _state = STATE_START;

      replayMsgCount++;

      switch(hdr->type) {
        case NSTAT_MSG_TYPE_ADD_SRC:
        case NSTAT_MSG_TYPE_QUERY_SRC:
        case NSTAT_MSG_TYPE_GET_SRC_DESC:
        case NSTAT_MSG_TYPE_ADD_ALL_SRCS:
        case NSTAT_MSG_TYPE_REM_SRC:
        {
          // this is a request
          QMsg qmsg = QMsg();
          qmsg.seqnum = hdr->context;
          qmsg.msgbytes = vec;
          qmsg.ntsrc = 0L;      // TODO: lookup
          uint32_t providerId=0;
          uint64_t srcRef=0L;
          _structHandler->getSrcRef(hdr, msgLength, srcRef, providerId);
          LOG_SENDRECV(("T SEND %s\n", _sprintMsg(hdr, srcRef).c_str()));

          _qmsgMap[hdr->context] = qmsg;
        }
        break;
        default:
          // response
          int secDelay = msgTimestamp - lastMsgTimestamp;
          if (secDelay > 0 && lastMsgTimestamp != 0) sleep(secDelay);  // try to somewhat emulate natural rate
          _handleResponseMessage((nstat_msg_hdr*)vec.data(), msgLength);
          break;
      }

      lastMsgTimestamp = msgTimestamp;
    }

  }

  //-------------------------------------------------------
  // configure logging. Default flags == 0, no logging.
  //-------------------------------------------------------
  virtual void setLogging(uint8_t flags) { _logFlags = flags; }
  
  //-------------------------------------------------------
  // returns the number of ENOBUFS errors received from kernel, indicating
  // the inability to send some requested information due to full buffer.
  // For example, attempting to send stream counts or descriptions.  The
  // default send buffer size for kernel is 2048 bytes.
  //-------------------------------------------------------
  virtual uint32_t getNumDrops() { return _numDrops; }
  
  // private data members

  NetworkStatisticsListener*    _listener;

  map<uint64_t, NetstatSource*> _map;

  bool                          _keepRunning;

  int                           _fd;

  NTStatKernelStructHandler*    _structHandler;

  state_t                       _state;

  vector<QMsg>                  _outq;  // messages that need to be sent

  uint16_t                      _seqnum;

  map<uint64_t, QMsg>           _qmsgMap; // messages waiting for response

  bool                          _wantTcp;
  bool                          _wantUdp;
  bool                          _wantKernel;
  
  uint32_t                      _updateIntervalSeconds;

  bool                          _recordEnabled;
  int                           _recordFd;
  uint32_t                      _numDrops;
  uint32_t                      _numErrors;
  
  int                           _logFd;
  uint8_t                       _logFlags;
  
  map<uint64_t, NetstatSource*> _mapWaitingForDesc;
  map<uint64_t, NetstatSource*> _mapWaitingForCount;

};






//----------------------------------------------------------
// Return new instance of impl
//----------------------------------------------------------
NetworkStatisticsClient* NetworkStatisticsClientNew(NetworkStatisticsListener* l)
{
  return new NetworkStatisticsClientImpl(l);
}

//----------------------------------------------------------
// getXnuVersion
//
// uname() will yield a version string like "xnu-3789.71.6~1/RELEASE_X86_64"
// This function extracts the integer after 'xnu-'.  3789 in this
// example.
//----------------------------------------------------------
unsigned int getXnuVersion()
{
   struct utsname name;

   uname (&name);

  char *p = strstr(name.version, "xnu-");
  if (0L == p) {
    // unexpected
    return 2000;
  }

  unsigned int val = (unsigned int)atol(p + 4);
  return val;
}

//----------------------------------------------------------
// string name for message type
//----------------------------------------------------------
string msg_name(uint32_t msg_type)
{
  switch(msg_type) {
    case NSTAT_MSG_TYPE_ERROR: return "ERROR";
    case NSTAT_MSG_TYPE_SUCCESS: return "SUCCESS";

    case NSTAT_MSG_TYPE_ADD_SRC: return "ADD_SRC";
    case NSTAT_MSG_TYPE_ADD_ALL_SRCS: return "ADD_ALL_SRC";
    case NSTAT_MSG_TYPE_REM_SRC: return "REM_SRC";
    case NSTAT_MSG_TYPE_QUERY_SRC: return "QUERY_SRC";
    case NSTAT_MSG_TYPE_GET_SRC_DESC: return "GET_SRC_DESC";

    case NSTAT_MSG_TYPE_SRC_ADDED: return "SRC_ADDED";
    case NSTAT_MSG_TYPE_SRC_REMOVED: return "SRC_REMOVED";
    case NSTAT_MSG_TYPE_SRC_DESC: return "SRC_DESC";
    case NSTAT_MSG_TYPE_SRC_COUNTS: return "SRC_COUNTS";
    default:
      break;
  }
  return "?";
}

//----------------------------------------------------------
// '>' for request '<' for response
//----------------------------------------------------------
char msg_dir(uint32_t msg_type)
{
  switch(msg_type) {
    case NSTAT_MSG_TYPE_ADD_SRC:
    case NSTAT_MSG_TYPE_ADD_ALL_SRCS:
    case NSTAT_MSG_TYPE_REM_SRC:
    case NSTAT_MSG_TYPE_QUERY_SRC:
    case NSTAT_MSG_TYPE_GET_SRC_DESC:
      return '>';

    case NSTAT_MSG_TYPE_ERROR:
    case NSTAT_MSG_TYPE_SUCCESS:
    case NSTAT_MSG_TYPE_SRC_ADDED:
    case NSTAT_MSG_TYPE_SRC_REMOVED:
    case NSTAT_MSG_TYPE_SRC_DESC:
    case NSTAT_MSG_TYPE_SRC_COUNTS:
    default:
      break;
  }
  return '<';
}

//----------------------------------------------------------
// less-than operator for NTStatStreamKey
// so applications can use it in std::map
//----------------------------------------------------------
bool NTStatStreamKey::operator<(const NTStatStreamKey& b) const
{
  if (isV6 < b.isV6) return true;
  if (isV6 > b.isV6) return false;

  if (ipproto < b.ipproto) return true;
  if (ipproto > b.ipproto) return false;

  if (lport < b.lport) return true;
  if (lport > b.lport) return false;

  if (rport < b.rport) return true;
  if (rport > b.rport) return false;

  if (ifindex < b.ifindex) return true;
  if (ifindex > b.ifindex) return false;

  if (isV6)
  {
    int d = memcmp(&local.addr6, &b.local.addr6,sizeof(in6_addr));
    if (d < 0) return true;
    if (d > 0) return false;
    d = memcmp(&remote.addr6, &b.remote.addr6,sizeof(in6_addr));
    if (d < 0) return true;
    if (d > 0) return false;
  } else {
    if (local.addr4.s_addr < b.local.addr4.s_addr) return true;
    if (local.addr4.s_addr > b.local.addr4.s_addr) return false;

    if (remote.addr4.s_addr < b.remote.addr4.s_addr) return true;
    if (remote.addr4.s_addr > b.remote.addr4.s_addr) return false;
  }
  return false;
}
