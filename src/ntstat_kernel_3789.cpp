
#include "NTStatKernelStructHandler.hpp"

// definitions from darwin-xnu/bsd/net/ntstat.h kernel header

#include <uuid/uuid.h>

#include "ntstat_kernel_3789.h"

#include <string.h>
#include <vector>
using namespace std;

class NTStatKernel3789 : public NTStatKernelStructHandler
{
public:
  //--------------------------------------------------------------------
  // write GET_SRC_DESC message to dest
  //--------------------------------------------------------------------
  virtual void writeSrcDesc(MsgDest &dest, uint64_t providerId, uint64_t srcRef )
  {
    nstat_msg_get_src_description msg = nstat_msg_get_src_description();

    NTSTAT_MSG_HDR(msg, dest, NSTAT_MSG_TYPE_GET_SRC_DESC);

    msg.srcref = srcRef;
    
    dest.send(&msg.hdr, sizeof(msg));
  }

  //--------------------------------------------------------------------
  // write QUERY_SRC message to dest
  //--------------------------------------------------------------------
  virtual void writeQueryAllSrc(MsgDest &dest) {
    nstat_msg_query_src_req msg = nstat_msg_query_src_req();
    
    NTSTAT_MSG_HDR(msg, dest, NSTAT_MSG_TYPE_QUERY_SRC);

    msg.srcref= NSTAT_SRC_REF_ALL;

    dest.send(&msg.hdr, sizeof(msg));
  }

  //--------------------------------------------------------------------
  // write ADD_ADD_SRCS message to dest
  //--------------------------------------------------------------------
  virtual void writeAddAllSrc(MsgDest &dest, uint32_t providerId)
  {
    nstat_msg_add_all_srcs msg = nstat_msg_add_all_srcs();

    NTSTAT_MSG_HDR(msg, dest, NSTAT_MSG_TYPE_ADD_ALL_SRCS);

    msg.provider = providerId ;
    
    dest.send(&msg.hdr, sizeof(msg));
  }

  // xnu-3789 is first time we see split _KERNEL and _USERLAND

  virtual void writeAddAllTcpSrc(MsgDest &dest) {
    writeAddAllSrc(dest, NSTAT_PROVIDER_TCP_KERNEL);
    writeAddAllSrc(dest, NSTAT_PROVIDER_TCP_USERLAND);
  }

  virtual void writeAddAllUdpSrc(MsgDest &dest) {
    writeAddAllSrc(dest, NSTAT_PROVIDER_UDP_KERNEL);
    writeAddAllSrc(dest, NSTAT_PROVIDER_UDP_USERLAND);
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
    if (msg->provider == NSTAT_PROVIDER_TCP_KERNEL || msg->provider == NSTAT_PROVIDER_TCP_USERLAND) {
      readTcpSrcDesc(hdr, structlen, dest);
    } else if (msg->provider == NSTAT_PROVIDER_UDP_KERNEL || msg->provider == NSTAT_PROVIDER_UDP_USERLAND) {
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
      dest->key.rport = udp->remote.v6.sin6_port;
      dest->key.local.addr6 = udp->local.v6.sin6_addr;
      dest->key.remote.addr6 = udp->remote.v6.sin6_addr;
    } else {
      dest->key.lport = udp->local.v4.sin_port;
      dest->key.rport = udp->remote.v4.sin_port;
      dest->key.local.addr4 = udp->local.v4.sin_addr;
      dest->key.remote.addr4 = udp->remote.v4.sin_addr;
    }

    dest->process.pid = udp->pid;
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


NTStatKernelStructHandler* NewNTStatKernel3789() {
  return new NTStatKernel3789();
}
