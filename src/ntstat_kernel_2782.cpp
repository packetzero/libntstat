
#include "NTStatKernelStructHandler.hpp"

// definitions from darwin-xnu/bsd/net/ntstat.h kernel header

#include <uuid/uuid.h>

#include "ntstat_kernel_2782.h"

#include <string.h>
#include <vector>
using namespace std;

class NTStatKernel2782 : public NTStatKernelStructHandler
{
public:
  //--------------------------------------------------------------------
  // write GET_SRC_DESC message to dest
  //--------------------------------------------------------------------
  virtual void writeSrcDesc(vector<uint8_t> &dest, uint64_t providerId, uint64_t srcRef )
  {
    dest.resize(sizeof(nstat_msg_get_src_description));
    nstat_msg_get_src_description &msg = (nstat_msg_get_src_description&)*dest.data();

    msg.hdr.type = NSTAT_MSG_TYPE_GET_SRC_DESC;
    msg.srcref = srcRef;
    msg.hdr.context = CONTEXT_GET_SRC_DESC;
  }

  //--------------------------------------------------------------------
  // write ADD_ADD_SRCS message to dest
  //--------------------------------------------------------------------
  virtual void writeAddAllSrc(vector<uint8_t> &dest, uint32_t providerId) {
    dest.resize(sizeof(nstat_msg_add_all_srcs));
    nstat_msg_add_all_srcs &msg = (nstat_msg_add_all_srcs&)*dest.data();

    msg.provider = providerId ;
    msg.hdr.type = NSTAT_MSG_TYPE_ADD_ALL_SRCS;
    msg.hdr.context = CONTEXT_ADD_ALL_SRCS;

  }

  virtual void writeAddAllTcpSrc(vector<uint8_t> &dest) {
    writeAddAllSrc(dest, NSTAT_PROVIDER_TCP);
  }

  virtual void writeAddAllUdpSrc(vector<uint8_t> &dest) {
    writeAddAllSrc(dest, NSTAT_PROVIDER_UDP);
  }

  //--------------------------------------------------------------------
  // extract srcRef, providerId (if possible) from message
  //--------------------------------------------------------------------
  virtual void getSrcRef(nstat_msg_hdr* msg, int structlen, uint64_t &srcRef, uint32_t &providerId) {
    switch(msg->type)
    {
      case NSTAT_MSG_TYPE_SRC_COUNTS:
        srcRef = ((nstat_msg_src_counts*)msg)->srcref;
        break;
      case NSTAT_MSG_TYPE_SRC_DESC:
        srcRef = ((nstat_msg_src_description*)msg)->srcref;
        providerId = ((nstat_msg_src_description*)msg)->provider;
        break;
      case NSTAT_MSG_TYPE_SRC_ADDED:
        srcRef = ((nstat_msg_src_added*)msg)->srcref;
        providerId = ((nstat_msg_src_added*)msg)->provider;
        break;
      case NSTAT_MSG_TYPE_SRC_REMOVED:
        srcRef = ((nstat_msg_src_removed*)msg)->srcref;
        break;
      default:
        printf("E getSrcRef not implemented for type %d\n", msg->type);
        break;
    }
  }



    //--------------------------------------------------------------------
    // populate dest with message data
    //--------------------------------------------------------------------
    virtual bool readSrcDesc(nstat_msg_hdr*hdr, int structlen, NTStatStream* dest )
    {
      nstat_msg_src_description *msg = (nstat_msg_src_description*)hdr;
      if (msg->provider == NSTAT_PROVIDER_TCP) {
        readTcpSrcDesc(hdr, structlen, dest);
      } else if (msg->provider == NSTAT_PROVIDER_UDP) {
        readUdpSrcDesc(hdr, structlen, dest);
      } else {
        // ??
        return false;
      }
      return true;
    }

  //--------------------------------------------------------------------
  // TCP: populate dest with message data
  //--------------------------------------------------------------------
  virtual void readTcpSrcDesc(nstat_msg_hdr*hdr, int structlen, NTStatStream* dest )
  {
    nstat_msg_src_description *msg = (nstat_msg_src_description*)hdr;
    nstat_tcp_descriptor*tcp = (nstat_tcp_descriptor*)msg->data;

    dest->key.ifindex = tcp->ifindex;
    dest->key.ipproto = IPPROTO_TCP;
    dest->key.isV6 = (tcp->local.v4.sin_family == AF_INET6);

    if (tcp->local.v4.sin_family == AF_INET6)
    {
      dest->key.lport = tcp->local.v6.sin6_port;
      dest->key.local.addr6 = tcp->local.v6.sin6_addr;
      dest->key.rport = tcp->remote.v6.sin6_port;
      dest->key.remote.addr6 = tcp->remote.v6.sin6_addr;
    } else {
      dest->key.lport = tcp->local.v4.sin_port;
      dest->key.rport = tcp->remote.v4.sin_port;
      dest->key.local.addr4 = tcp->local.v4.sin_addr;
      dest->key.remote.addr4 = tcp->remote.v4.sin_addr;
    }
    dest->states.txwindow = tcp->txwindow;
    dest->states.txcwindow = tcp->txcwindow;
    dest->states.state = tcp->state;

    dest->process.pid = tcp->pid;
    dest->process.upid = tcp->upid;

    strcpy(dest->process.name, ((tcp->pid > 0 && tcp->pname[0]) ? tcp->pname : ""));
  }

  //--------------------------------------------------------------------
  // UDP: populate dest with message data
  //--------------------------------------------------------------------
  virtual void readUdpSrcDesc(nstat_msg_hdr*hdr, int structlen, NTStatStream* dest )
  {
    nstat_msg_src_description *msg = (nstat_msg_src_description*)hdr;
    nstat_udp_descriptor*udp = (nstat_udp_descriptor*)msg->data;

    dest->key.ifindex = udp->ifindex;
    dest->key.ipproto = IPPROTO_UDP;
    dest->key.isV6 = (udp->local.v4.sin_family == AF_INET6);

    if (udp->local.v4.sin_family == AF_INET6)
    {
      dest->key.lport = udp->local.v6.sin6_port;
      dest->key.local.addr6 = udp->local.v6.sin6_addr;
      dest->key.rport = udp->remote.v6.sin6_port;
      dest->key.remote.addr6 = udp->remote.v6.sin6_addr;
    } else {
      dest->key.lport = udp->local.v4.sin_port;
      dest->key.rport = udp->remote.v4.sin_port;
      dest->key.local.addr4 = udp->local.v4.sin_addr;
      dest->key.remote.addr4 = udp->remote.v4.sin_addr;
    }

    dest->process.pid = udp->pid;
    dest->process.upid = udp->upid;
    strcpy(dest->process.name, ((udp->pid > 0 && udp->pname[0]) ? udp->pname : ""));
  }

  //--------------------------------------------------------------------
  // populate dest with message counts data
  //--------------------------------------------------------------------
  virtual void readCounts(nstat_msg_hdr*hdr, int structlen, NTStatCounters& dest )
  {
    nstat_msg_src_counts *msg = (nstat_msg_src_counts*)hdr;
    dest.rxbytes = msg->counts.nstat_rxbytes;
    dest.txbytes = msg->counts.nstat_txbytes;
    dest.rxpackets = msg->counts.nstat_rxpackets;
    dest.txpackets = msg->counts.nstat_txpackets;
  }

};


NTStatKernelStructHandler* NewNTStatKernel2782() {
  return new NTStatKernel2782();
}
