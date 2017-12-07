//
//  main.cpp
//  demo
//
//  Created by Alex Malone on 12/7/17.
//  Copyright © 2017 AlexMalone. All rights reserved.
//

#include "../include/NetworkStatisticsClient.hpp"
#include <stdio.h>

#include <string>
using namespace std;

string addr2text ( const in_addr& Addr );
string addr2text ( const in6_addr& Addr );
string stream2text (const NTStatStreamKey& s);

class MyNetstatListener : public NetworkStatisticsListener
{
  virtual void onStreamAdded(const NTStatStream *stream)
  {
    printf("A   pid:%d (%s) stream %s\n", stream->process.pid, stream->process.name, stream2text(stream->key).c_str());
  }
  virtual void onStreamRemoved(const NTStatStream *stream)
  {
    printf(" D  pid:%d (%s) stream %s\n", stream->process.pid, stream->process.name, stream2text(stream->key).c_str());

  }
  virtual void onStreamStatsUpdate(const NTStatStream *stream)
  {
    printf("  S pid:%d (%s) stream %s\n", stream->process.pid, stream->process.name, stream2text(stream->key).c_str());

  }

};

int main(int argc, const char * argv[])
{
  MyNetstatListener listener = MyNetstatListener();
  NetworkStatisticsClient* netstatClient =  NetworkStatisticsClientNew(&listener);

  if (false == netstatClient->connectToKernel()) {
    printf("Failed to establish network.statistics system control socket\n");
    return 2;
  }
  
  netstatClient->run();
  
  return 0;
}

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
  char tmp[64];
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
