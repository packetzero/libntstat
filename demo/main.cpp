//
//  libntstat demo
//
//  Copyright © 2017 Alex Malone. All rights reserved.

#include "../include/NetworkStatisticsClient.hpp"
#include <stdio.h>

#include <string>
using namespace std;

string stream2text (const NTStatStreamKey& s);

/*
 * Implementing this listener allows receipt of events.
 */
class MyNetstatListener : public NetworkStatisticsListener
{
  virtual void onStreamAdded(const NTStatStream *stream)
  {
    printf(" + %s pid:%d (%s)\n", stream2text(stream->key).c_str(), stream->process.pid, stream->process.name);
  }
  virtual void onStreamRemoved(const NTStatStream *stream)
  {
    printf(" - %s pid:%d (%s)\n", stream2text(stream->key).c_str(), stream->process.pid, stream->process.name);
    printf("   bytes (tx/rx):%llu/%llu  packets:%llu/%llu\n",stream->stats.txbytes, stream->stats.rxbytes, stream->stats.txpackets, stream->stats.rxpackets);
  }
  virtual void onStreamStatsUpdate(const NTStatStream *stream)
  {
    printf("   %s pid:%d (%s)\n", stream2text(stream->key).c_str(), stream->process.pid, stream->process.name);
    printf("   bytes (tx/rx):%llu/%llu  packets:%llu/%llu\n",stream->stats.txbytes, stream->stats.rxbytes, stream->stats.txpackets, stream->stats.rxpackets);

  }

};

int main(int argc, const char * argv[])
{
  // create

  MyNetstatListener listener = MyNetstatListener();
  NetworkStatisticsClient* netstatClient =  NetworkStatisticsClientNew(&listener);

  // connect to ntstat via kernel control module socket
  
  if (false == netstatClient->connectToKernel()) {
    printf("Failed to establish network.statistics system control socket\n");
    return 2;
  }

  // in a real app, we would want to run this in a dedicated thread

  netstatClient->run();
  
  return 0;
}

// ==================== helper functions ======================

#include <arpa/inet.h>

string addr2text ( const in_addr& Addr )
{
  char IPv4AddressAsString[INET_ADDRSTRLEN];      //buffer needs 16 characters min
  if ( NULL != inet_ntop ( AF_INET, &Addr, IPv4AddressAsString, sizeof(IPv4AddressAsString) ) )
    return string(IPv4AddressAsString);
  return "?";
}

string addr2text ( const in6_addr& Addr )
{
  char IPv6AddressAsString[INET6_ADDRSTRLEN];     //buffer needs 46 characters min
  if ( NULL != inet_ntop ( AF_INET6, &Addr, IPv6AddressAsString, sizeof(IPv6AddressAsString) ) )
    return string(IPv6AddressAsString);
  return "?";
}

string stream2text (const NTStatStreamKey& s)
{
  char tmp[256];
  string val = s.ipproto == IPPROTO_TCP ? "TCP " : "UDP ";
  if (s.isV6)
  {
    val += addr2text(s.local.addr6);
    sprintf(tmp," %d - ", ntohs(s.lport));
    val += tmp;
    val += addr2text(s.remote.addr6);
    sprintf(tmp," %d", ntohs(s.rport));
    val += tmp;
  } else {
    val += addr2text(s.local.addr4);
    sprintf(tmp," %d - ", ntohs(s.lport));
    val += tmp;
    val += addr2text(s.remote.addr4);
    sprintf(tmp," %d", ntohs(s.rport));
    val += tmp;
}
  return val;
}
