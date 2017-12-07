//
//  NetworkStatisticsClient.cpp
//  netstat-socket
//
//  Created by Alex Malone on 12/6/17.
//  Copyright © 2017 Ziften. All rights reserved.
//

#include "NTStatKernelStructHandler.hpp"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/utsname.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>

NTStatKernelStructHandler* NewNTStatKernel2422();
NTStatKernelStructHandler* NewNTStatKernel2782();
NTStatKernelStructHandler* NewNTStatKernel3789();
NTStatKernelStructHandler* NewNTStatKernel3248();
NTStatKernelStructHandler* NewNTStatKernel4570();

#define      NET_STAT_CONTROL_NAME   "com.apple.network.statistics"

enum
{
  NSTAT_PROVIDER_ROUTE    = 1
  ,NSTAT_PROVIDER_TCP_KERNEL  = 2
  ,NSTAT_PROVIDER_TCP_USERLAND = 3
  ,NSTAT_PROVIDER_UDP_KERNEL  = 4
  ,NSTAT_PROVIDER_UDP_USERLAND = 5
  ,NSTAT_PROVIDER_IFNET  = 6
  ,NSTAT_PROVIDER_SYSINFO = 7
};

typedef struct nstat_msg_error
{
  nstat_msg_hdr   hdr;
  u_int32_t               error;  // errno error
  u_int8_t        reserved[4];
} nstat_msg_error;

const int BUFSIZE = 2048;

static bool _logDbg = false;
static bool _logTrace = true;
static bool _logErrors = true;

#include <string>
#include <map>
using namespace std;

string msg_name(uint32_t msg_type);
unsigned int getXnuVersion();

struct NetstatSource
{
  NetstatSource(uint64_t srcRef, uint32_t providerId) : _srcRef(srcRef), _providerId(providerId) {}
  
  uint64_t _srcRef;
  uint32_t _providerId;
  NTStatStream obj;
};

class NetworkStatisticsClientImpl : public NetworkStatisticsClient
{
public:
  NetworkStatisticsClientImpl(NetworkStatisticsListener* listener): _listener(listener), _map(), _keepRunning(false), _fd(0) {
    
  }
  
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
      if (_logDbg) printf("socket id:%d unit:%d\n", ctlInfo.ctl_id, sc.sc_unit);
      return true;
    }
    
    // no dice
    
    close(_fd);
    return false;
  }
  
  bool isConnected()
  {
    return (_fd > 0);
  }

  void run()
  {
    if (!isConnected()) {
      printf("E run() not connected.\n"); return;
    }
    
    _keepRunning = true;
    unsigned int xnuVersion = getXnuVersion();
    
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


    vector<uint8_t> vec;
    _structHandler->writeAddAllSrc(vec, NSTAT_PROVIDER_TCP_KERNEL);
    SEND(vec.data(), vec.size());
  
    while (_keepRunning)
    {
      _readNextMessage();
    }
    
    close(_fd);
    _fd = 0;
  }

  
  
private:
  
  //---------------------------------------------------------------
  // write struct to socket fd
  //---------------------------------------------------------------
  ssize_t SEND(void *pstruct, size_t structlen)
  {
    nstat_msg_hdr* hdr = (nstat_msg_hdr*)pstruct;
    
    if (_logTrace) printf("T SEND type:%s(%d) context:%d\n", msg_name(hdr->type).c_str(), hdr->type, hdr->context );
    
    ssize_t rc = write (_fd, pstruct, structlen);
    
    if (_logErrors && rc < structlen) printf("E ERROR on SEND write returned %d expecting %d\n", rc, structlen);
    
    return rc;
  }

void _removeSource(uint64_t srcRef)
{
  auto fit = _map.find(srcRef);
  if (fit != _map.end()) {
    _map.erase(fit);
  }
}

void _resetSource(uint64_t srcRef, uint32_t providerId)
{
  _removeSource(srcRef);
  
  NetstatSource* src = new NetstatSource(srcRef, providerId);
  _map[srcRef] = src;
}

NetstatSource* _lookupSource(uint64_t srcRef)
{
  auto fit = _map.find(srcRef);
  if (fit != _map.end()) {
    return fit->second;
  }
  
  return 0L;
}

void onSrcAdded(uint32_t providerId, uint64_t srcRef)
{
  _resetSource(srcRef, providerId);
}


int _readNextMessage()
{
  uint64_t srcRef = 0L;
  uint32_t providerId = 0;
  char c[BUFSIZE];

  int num_bytes = (int)read (_fd, c, BUFSIZE);

  if (_logDbg) printf("D READ %d bytes\n", num_bytes);

  if (num_bytes <= 0) return -1;

    nstat_msg_hdr *ns = (nstat_msg_hdr *) c;

    if (_logTrace) printf("T RECV type:%s(%d) context:%llu len:%d\n", msg_name(ns->type).c_str(), ns->type, ns->context, num_bytes);

    switch (ns->type)
    {

      case NSTAT_MSG_TYPE_SRC_ADDED:
      {
        _structHandler->getSrcRef(ns, num_bytes, srcRef, providerId);
        onSrcAdded(providerId, srcRef);
        break;
      }
      case NSTAT_MSG_TYPE_SRC_REMOVED:
      {
        _structHandler->getSrcRef(ns, num_bytes, srcRef, providerId);
        NetstatSource* source = _lookupSource(srcRef);
        if (source != 0L) {
          _listener->onStreamRemoved(&source->obj);
        }
        break;
      }
      case NSTAT_MSG_TYPE_SRC_DESC:
      {
        _structHandler->getSrcRef(ns, num_bytes, srcRef, providerId);
        NetstatSource* source = _lookupSource(srcRef);
        if (source != 0L) {
          if (providerId == NSTAT_PROVIDER_TCP_KERNEL || providerId == NSTAT_PROVIDER_TCP_USERLAND)
            _structHandler->readTcpSrcDesc(ns, num_bytes, &source->obj);
          else
            _structHandler->readUdpSrcDesc(ns, num_bytes, &source->obj);
          _listener->onStreamAdded(&source->obj);
        }
        break;
      }
      case NSTAT_MSG_TYPE_SRC_COUNTS:
      {
        _structHandler->getSrcRef(ns, num_bytes, srcRef, providerId);

        NetstatSource* source = _lookupSource(srcRef);
        if (source != 0L) {
          _structHandler->readCounts(ns, num_bytes, source->obj.stats);
          _listener->onStreamStatsUpdate(&source->obj);
        }
        break;
      }
      case NSTAT_MSG_TYPE_SUCCESS:
        if (ns->context == CONTEXT_QUERY_SRC) {
          // no sources (count == 0) OR nstat_control_reporting_allowed is FALSE
          //process_response_query_src(c, num_bytes);
        } else
          printf("E unhandled success response\n");
        /*
        // Got all sources, or all counts
        // if we were getting sources, ask now for all descriptions

        if (!tcpAdded)
        { tcpAdded++;

          addAll (fd, NSTAT_PROVIDER_UDP_USERLAND);
        }

        else { if (!udpAdded) udpAdded++; }

        if (tcpAdded && udpAdded )
        {
          if (!gettingCounts)
          {
            memset(&qsreq, 0, sizeof(qsreq));

            qsreq.hdr.type= NSTAT_MSG_TYPE_QUERY_SRC   ; // 1004
            qsreq.srcref= NSTAT_SRC_REF_ALL;
            qsreq.hdr.context = 1005; // This way I can tell if errors get returned for dead sources


            rc = write (fd, &qsreq, sizeof(qsreq));
            gettingCounts++;
          }
          else  gotCounts++;

        }

        */
        break;

      case NSTAT_MSG_TYPE_ERROR:
      {
        // Error message
        nstat_msg_error* perr = (nstat_msg_error*)c;
        if (_logTrace) printf("T error code:%d (0x%x) \n", perr->error, perr->error);
//        remove_ntstat_source(ns->context);
        //Error message - these are usually for dead sources
      }
        return 0;//-1;
        break;

      default:
        printf("E unknown message type:%d\n", ns->type);
        return -1;

    }

  return 0;
}
  
  virtual void stop() { _keepRunning = false; }

  NetworkStatisticsListener* _listener;

  map<uint64_t, NetstatSource*> _map;
  
  bool _keepRunning;

  int _fd;
  
  NTStatKernelStructHandler* _structHandler;
  
};







NetworkStatisticsClient* NetworkStatisticsClientNew(NetworkStatisticsListener* l)
{
  // TODO: add implementations for each kernel change
  return new NetworkStatisticsClientImpl(l);
}


unsigned int getXnuVersion()
{
   struct utsname name;
   
   uname (&name);
  
  char *p = strstr(name.version, "xnu-");
  if (0L == p) {
    // unexpected
    return 2000;
  }
  
  unsigned int val = atol(p + 4);
  return val;
}

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
      //case NSTAT_MSG_TYPE_SET_FILTER: return "SET_FILTER";
      //case NSTAT_MSG_TYPE_GET_UPDATE: return "GET_UPDATE";
      //case NSTAT_MSG_TYPE_SUBSCRIBE_SYSINFO: return "SUBSCRIBE_SYSINFO";
      
    case NSTAT_MSG_TYPE_SRC_ADDED: return "SRC_ADDED";
    case NSTAT_MSG_TYPE_SRC_REMOVED: return "SRC_REMOVED";
    case NSTAT_MSG_TYPE_SRC_DESC: return "SRC_DESC";
    case NSTAT_MSG_TYPE_SRC_COUNTS: return "SRC_COUNTS";
    default:
      break;
  }
  return "?";
}

#include <string.h> // memcmp

/*
  uint8_t     isV6;
  uint8_t     ipproto;
  uint16_t    port;
  uint32_t    ifindex;
  addr_t      local;
  addr_t      remote;
*/
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
  }
  return false;
}

