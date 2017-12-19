#ifndef _NT_STAT_KERNEL_STRUCT_HANDLER_H_
#define _NT_STAT_KERNEL_STRUCT_HANDLER_H_

#include <stdint.h>
#include "../include/NetworkStatisticsClient.hpp"

// the message header is consistent across versions, so defining here

typedef struct nstat_msg_hdr
{
        uint64_t       context    __attribute__((aligned(sizeof(u_int64_t))));
        uint32_t       type;
        uint16_t       length;
        uint16_t       flags;
} nstat_msg_hdr;

class MsgDest
{
public:
  /*
   * Returns current message sequence number.
   * StructHandlers need to use this value in the hdr.context of each message.
   */
  virtual uint64_t seqnum() = 0;

  /*
   * send message
   */
  virtual void send(nstat_msg_hdr* msg, size_t msglen) = 0;
};


/*
 * Abstraction for reading and writing versioned kernel structs.
 * In XNU-3789 (NSTAT_REVISION 8), srcRef became uint64_t. Prior versions
 * need to convert to uint32_t.
 */
class NTStatKernelStructHandler
{
public:

  /*
   * write NSTAT_MSG_TYPE_GET_SRC_DESC to dest
   */
  virtual void writeSrcDesc(MsgDest &dest, uint64_t providerId, uint64_t srcRef ) = 0;

  /*
   * write NSTAT_MSG_TYPE_ADD_ALL_SRCS for TCP or UDP
   */
  virtual void writeAddAllTcpSrc(MsgDest &dest) = 0;
  virtual void writeAddAllUdpSrc(MsgDest &dest) = 0;

  /*
   * Provider IDs are abstracted.  Some versions have multiple TCP and UDP.
   * In v3789, UDP changes from 3 to 4.  Early versions don't have interface provider.
   * Thus, we resort to querying.
   */
  virtual bool isProviderTcp(uint64_t providerId)=0;
  virtual bool isProviderUdp(uint64_t providerId)=0;

  /*
   * write NSTAT_MSG_TYPE_QUERY_SRC
   */
  virtual void writeQuerySrc(MsgDest &dest, uint64_t srcRef) = 0;

  
  /*
   * Extract from msg and populate srcRef and providerId (if in message).
   */
  virtual void getSrcRef(nstat_msg_hdr* msg, int structlen, uint64_t &srcRef, uint32_t &providerId) = 0;

  /*
   * Read src desc and populate relevant fields in dest.
   */
  virtual bool readSrcDesc(nstat_msg_hdr*msg, int structlen, NTStatStream* dest ) = 0;

  /*
   * Update dest counts using msg.
   */
  virtual void readCounts(nstat_msg_hdr*msg, int structlen, NTStatCounters& dest ) = 0;

};

// macro for consistency in setting hdr fields.  context in particular

#define NTSTAT_MSG_HDR(msg_struct, MsgDestRef, MSG_TYPE)  { \
  (msg_struct).hdr.type = MSG_TYPE;                         \
  (msg_struct).hdr.length = sizeof(msg_struct);             \
  (msg_struct).hdr.context = (MsgDestRef).seqnum();         \
}

#endif // _NT_STAT_KERNEL_STRUCT_HANDLER_H_
