//  NetworkStatisticsClient.hpp
//  Copyright © 2017 Alex Malone. All rights reserved.
//

#ifndef NetworkStatisticsClient_hpp
#define NetworkStatisticsClient_hpp

#include <stdint.h>
#include <netinet/in.h>

typedef union {
  struct in_addr addr4;
  struct in6_addr addr6;
} addr_t;

struct NTStatCounters
{
  uint64_t rxpackets;
  uint64_t txpackets;
  uint64_t rxbytes;
  uint64_t txbytes;
};

struct NTStatStreamState
{
  uint32_t state;
  uint32_t txcwindow;
  uint32_t txwindow;

  uint32_t tfirst;
  uint32_t tlast;
};

struct NTStatStreamKey
{
  uint8_t     isV6;
  uint8_t     ipproto;
  uint16_t    pad;
  uint32_t    ifindex;
  uint16_t    lport;
  uint16_t    rport;
  addr_t      local;
  addr_t      remote;

  bool operator<(const NTStatStreamKey& b) const; // needed to be a key type for std::map
};

struct NTStatProcess
{
  uint32_t     pid;
  uint32_t     upid;
  char         name[64];
};

struct NTStatStream
{
  // these are constant once we see the stream
  NTStatStreamKey    key;
  NTStatProcess      process;

  // these get updated
  NTStatCounters     stats;
  NTStatStreamState  states;
};

class NetworkStatisticsListener
{
public:
  virtual void onStreamAdded(const NTStatStream *stream)=0;
  virtual void onStreamRemoved(const NTStatStream *stream)=0;
  virtual void onStreamStatsUpdate(const NTStatStream *stream)=0;
};

class NetworkStatisticsClient /* interface */
{
public:
  /*
   * This should be done before call to run().
   */
  virtual bool connectToKernel() = 0;

  /*
   * returns true if connectToKernel() was called and was successful.
   */
  virtual bool isConnected() = 0;

  /*
   * Blocking: run from dedicated thread.  Subscribes to TCP/UDP and continuously reads messages.
   * The connection to the kernel will be disconnected when run() exits.
   */
  virtual void run() = 0;

  /*
   * Will set the stop flag, so run() will exit.
   */
  virtual void stop() = 0;

};


NetworkStatisticsClient* NetworkStatisticsClientNew(NetworkStatisticsListener* l);

#endif /* NetworkStatisticsClient_hpp */
