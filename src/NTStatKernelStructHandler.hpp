#ifndef _NETWORK_STATISTICS_CLIENT_BASE_HPP_
#define _NETWORK_STATISTICS_CLIENT_BASE_HPP_

#include <stdint.h>
#include "../include/NetworkStatisticsClient.hpp"

typedef struct nstat_msg_hdr
{
        uint64_t       context    __attribute__((aligned(sizeof(u_int64_t))));
        uint32_t       type;
        uint16_t       length;
        uint16_t       flags;
} nstat_msg_hdr;

enum
{
  // generic response messages
  NSTAT_MSG_TYPE_SUCCESS                  = 0
  ,NSTAT_MSG_TYPE_ERROR                   = 1
  
  // Requests
  ,NSTAT_MSG_TYPE_ADD_SRC                 = 1001
  ,NSTAT_MSG_TYPE_ADD_ALL_SRCS    = 1002
  ,NSTAT_MSG_TYPE_REM_SRC                 = 1003
  ,NSTAT_MSG_TYPE_QUERY_SRC               = 1004
  ,NSTAT_MSG_TYPE_GET_SRC_DESC    = 1005
  
  // Responses/Notfications
  ,NSTAT_MSG_TYPE_SRC_ADDED               = 10001
  ,NSTAT_MSG_TYPE_SRC_REMOVED             = 10002
  ,NSTAT_MSG_TYPE_SRC_DESC                = 10003
  ,NSTAT_MSG_TYPE_SRC_COUNTS              = 10004
};

#define CONTEXT_QUERY_SRC    9995
#define CONTEXT_ADD_ALL_SRCS 9996
#define CONTEXT_GET_SRC_DESC 9997

#include <vector>

class NTStatKernelStructHandler
{
public:

  virtual void writeSrcDesc(std::vector<uint8_t> &dest, uint64_t providerId, uint64_t srcRef ) = 0;

  virtual void writeAddAllSrc(std::vector<uint8_t> &dest, uint32_t providerId) = 0;

//  virtual void _write_query_src() = 0;

  virtual void getSrcRef(nstat_msg_hdr* msg, int structlen, uint64_t &srcRef, uint32_t &providerId) = 0;

  virtual void readTcpSrcDesc(nstat_msg_hdr*msg, int structlen, NTStatStream* dest ) = 0;
  virtual void readUdpSrcDesc(nstat_msg_hdr*msg, int structlen, NTStatStream* dest ) = 0;

  virtual void readCounts(nstat_msg_hdr*msg, int structlen, NTStatCounters& dest ) = 0;

};


#endif // _NETWORK_STATISTICS_CLIENT_BASE_HPP_
