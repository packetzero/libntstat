//  NetworkStatisticsClient.hpp
//  Copyright © 2017 Alex Malone. All rights reserved.
//

#ifndef NetworkStatisticsClient_hpp
#define NetworkStatisticsClient_hpp

#include <stdint.h>
#include <netinet/in.h>

struct NTStatStream;

/*
 * NetworkStatisticsListener
 *
 * Applications need to implement this interface to receive callbacks
 * from the NetworkStatisticsClient.
 */
class NetworkStatisticsListener
{
public:
  virtual void onStreamAdded(const NTStatStream *stream)=0;

  virtual void onStreamRemoved(const NTStatStream *stream)=0;
  
  virtual void onStreamStatsUpdate(const NTStatStream *stream)=0;
};

/*
 * NetworkStatisticsClient
 *
 * This is the interface to interact with the com.apple.network.statistics data.
 * Behind the scenes, the implementation creates a system socket, subscribes
 * to TCP and UDP sources, and passes along the data to the listener.
 */
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
   * @params:
   *   withCounts    If false, listener is only notified when streams added or removed (no count updates).
   */
  virtual void run(bool withCounts) = 0;

  /*
   * Will set the stop flag, so run() will exit.
   */
  virtual void stop() = 0;

};

// Instantiate (singleton) the NetworkStatisticsClient

NetworkStatisticsClient* NetworkStatisticsClientNew(NetworkStatisticsListener* l);

// Data types for reporting

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
  uint32_t state;      // TCPS_LISTEN, TCPS_SYN_SENT, etc.
  uint32_t txcwindow;  // TCP only
  uint32_t txwindow;   // TCP only
};

struct NTStatStreamKey
{
  uint8_t     isV6;
  uint8_t     ipproto; // IPPROTO_TCP or IPPROTO_UDP
  uint16_t    pad;
  uint32_t    ifindex;
  uint16_t    lport;   // local port (network-endian)
  uint16_t    rport;   // remove port (network-endian)
  addr_t      local;
  addr_t      remote;
  
  bool operator<(const NTStatStreamKey& b) const; // needed to be a key type for std::map
};

struct NTStatProcess
{
  uint32_t     pid;
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

#endif /* NetworkStatisticsClient_hpp */
