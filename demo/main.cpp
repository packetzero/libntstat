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
std::string stateToText(const NTStatStream *stream);

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
    log(stream, '#');
  }


  void log(const NTStatStream* stream, char displayChar)
  {
    if (IS_LISTEN_PORT(stream))
    {
       printf(" @%c %s pid:%u (%s) LISTEN %s port:%u\n", displayChar, timestr().c_str(), stream->process.pid,
              stream->process.name,(stream->key.ipproto == IPPROTO_TCP ? "TCP" : "UDP"), ntohs(stream->key.lport));
    }
    else
    {
      //string stateStr = (displayChar == '-' ? stateToText(stream) : "");
      string notes = "";
      if (displayChar == '-' && stream->key.ipproto == IPPROTO_TCP &&
          stream->stats.rxpackets == 0) notes = "FAILED";
      
      printf(" %c %s %s pid:%u (%s) %s id:%llu\n", displayChar, timestr().c_str(),
           stream2text(stream->key).c_str(), stream->process.pid, stream->process.name, notes.c_str(), stream->id);

      // write bytes if present on update/remove

      string medium = "wired";
      if (stream->stats.cell_txbytes > 0) medium = "cell";
      if (stream->stats.wifi_txbytes > 0) medium = "wifi";

      if (displayChar != '+' && (stream->stats.rxpackets > 0 || stream->stats.txpackets > 0))
        printf("   bytes (tx/rx):%llu/%llu  packets:%llu/%llu %s\n",stream->stats.txbytes, stream->stats.rxbytes, stream->stats.txpackets, stream->stats.rxpackets, medium.c_str());
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

  // configure
  bool wantTcp=true, wantUdp=false;
  uint32_t updateIntervalSeconds = 60;
  netstatClient->configure(wantTcp, wantUdp, updateIntervalSeconds);
  
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

std::string stateToText(const NTStatStream *stream)
{
  switch(stream->states.state)
  {
    case TCPS_CLOSED: return ("CLOSED");
    case TCPS_LISTEN: return ("LISTENING");
    case TCPS_ESTABLISHED: return ("ESTABLISHED");
    case TCPS_CLOSING: return ("CLOSING");
    case TCPS_SYN_SENT: return ("SYN_SENT");
    case TCPS_LAST_ACK: return ("LAST_ACK");
    case TCPS_CLOSE_WAIT: return ("CLOSE_WAIT");
    case TCPS_TIME_WAIT: return ("TIME_WAIT");
    case TCPS_FIN_WAIT_1 : return ("FIN_WAIT_1");
    case TCPS_FIN_WAIT_2 : return ("FIN_WAIT_2");
      
    default:
      return("?");
  }
  
} // stateToText

#include <time.h>
string timestr()
{
  char timestamp[64]="";
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  strftime(timestamp, 63, "%H:%M:%S", tm);
  return string(timestamp);
}
