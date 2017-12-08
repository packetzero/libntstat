#ifndef _NT_STAT_KERNEL_STRUCT_HANDLER_H_
#define _NT_STAT_KERNEL_STRUCT_HANDLER_H_

#include <stdint.h>
#include <vector>
#include "../include/NetworkStatisticsClient.hpp"

// the message header is consistent across versions, so defining here

typedef struct nstat_msg_hdr
{
        uint64_t       context    __attribute__((aligned(sizeof(u_int64_t))));
        uint32_t       type;
        uint16_t       length;
        uint16_t       flags;
} nstat_msg_hdr;

// contexts shared between NetworkStatisticsClient implementation and struct handlers

#define CONTEXT_QUERY_SRC    9995
#define CONTEXT_ADD_ALL_SRCS 9996
#define CONTEXT_GET_SRC_DESC 9997

class MsgDest
{
public:
  virtual void send(nstat_msg_hdr* msg, size_t len, uint64_t context, int num = 1) = 0;
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
   * write NSTAT_MSG_TYPE_GET_SRC_DESC to vector
   */
  virtual void writeSrcDesc(std::vector<uint8_t> &dest, uint64_t providerId, uint64_t srcRef ) = 0;

  /*
   * write NSTAT_MSG_TYPE_ADD_ALL_SRCS for TCP or UDP
   */
  virtual void writeAddAllTcpSrc(std::vector<uint8_t> &dest) = 0;
  virtual void writeAddAllUdpSrc(std::vector<uint8_t> &dest) = 0;

  /*
   * write NSTAT_MSG_TYPE_QUERY_SRC for all
   */
  virtual void writeQueryAllSrc(std::vector<uint8_t> &dest) = 0;

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

#endif // _NT_STAT_KERNEL_STRUCT_HANDLER_H_
