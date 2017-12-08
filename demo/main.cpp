//
//  libntstat demo
//
//  Copyright © 2017 Alex Malone. All rights reserved.

#include "../include/NetworkStatisticsClient.hpp"
#include <stdio.h>
#include <netinet/tcp_fsm.h>
#include <string>
using namespace std;

string stream2text (const NTStatStreamKey& s);
string timestr();

/*
 * Implementing this listener allows receipt of events.
 */
class MyNetstatListener : public NetworkStatisticsListener
{
  virtual void onStreamAdded(const NTStatStream *stream)
  {
    log(stream, '+');
  }
  virtual void onStreamRemoved(const NTStatStream *stream)
  {
    log(stream, '-');
  }
  virtual void onStreamStatsUpdate(const NTStatStream *stream)
  {
    log(stream, ' ');
  }

  void log(const NTStatStream* stream, char displayChar)
  {
    if (stream->key.ipproto == IPPROTO_TCP && stream->states.state == TCPS_LISTEN)
    {
      printf(" @ %s pid:%u (%s) LISTEN TCP port:%u\n", timestr().c_str(), stream->process.pid, stream->process.name, ntohs(stream->key.lport));
    }
    else
    {
      printf(" %c %s %s pid:%u (%s)\n", displayChar, timestr().c_str(),
           stream2text(stream->key).c_str(), stream->process.pid, stream->process.name);
      if (stream->stats.rxpackets > 0 || stream->stats.txpackets > 0)
        printf("   bytes (tx/rx):%llu/%llu  packets:%llu/%llu\n",stream->stats.txbytes, stream->stats.rxbytes, stream->stats.txpackets, stream->stats.rxpackets);
    }
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
  bool wantStats = true;
  netstatClient->run(wantStats);
  
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

#include <time.h>
string timestr()
{
  char timestamp[64]="";
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  strftime(timestamp, 63, "%H:%M:%S", tm);
  return string(timestamp);
}
