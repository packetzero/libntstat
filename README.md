## C++ library to receive network stats on MacOS (XNU Darwin)

### Introduction

Darwin kernel provides an unpublished API to receive pseudo realtime notifications of network connections and stats. See [protocol.md](./docs/protocol.md) for details on the underlying mechanism and protocol.

### Usage

The application implements the NetworkStatisticsListener interface (C++ pure virtual class), then creates a NetworkStatisticsClient instance, connects it to the kernel, optionally call configure(), and calls run() from a dedicated thread.  The interface is simple and has the following methods.  See [NetworkStatisticsClient.hpp](./include/NetworkStatisticsClient.hpp) for details.

- onStreamAdded()
- onStreamRemoved()
- onStreamStatsUpdate()

### Demo Application
There is a command-line application called 'demo' that prints simple network stream information to stdout.  Example output:
```
XNU version:3789
 + 15:23:27 TCP 10.0.20.179 58760 - 52.86.2.3 443 pid:66 (epagd)
 + 15:23:27 TCP 10.0.20.179 58718 - 17.249.60.9 5223 pid:83 (apsd)
 @+ 15:23:28 pid:399 (mongod) LISTEN TCP port:27017
 @+ 15:23:28 pid:1 (launchd) LISTEN TCP port:22
+ 15:23:54 TCP 10.0.20.179 61608 - 172.217.9.170 443 pid:668 (com.apple.Safari)
+ 15:23:54 TCP 10.0.20.179 61609 - 17.249.219.246 443 pid:353 (nsurlsessiond)
+ 15:23:54 TCP 10.0.20.179 61610 - 72.247.204.12 443 pid:18175 (com.apple.WebKit)
+ 15:23:54 TCP 10.0.20.179 61611 - 13.33.115.210 443 pid:18175 (com.apple.WebKit)
+ 15:23:55 TCP 10.0.20.179 61612 - 13.33.115.210 443 pid:18175 (com.apple.WebKit)
+ 15:23:55 TCP fdd4:69cc:c669:be20::1d4c 61615 - 2607:f8b0:4000:814::200e 443 pid:18175 (com.apple.WebKit)
- 15:23:55 TCP fdd4:69cc:c669:be20::1d4c 61615 - 2607:f8b0:4000:814::200e 443 pid:18175 (com.apple.WebKit) FAILED
- 15:23:56 TCP 10.0.20.179 61609 - 17.249.219.246 443 pid:353 (nsurlsessiond)
  bytes (tx/rx):2181/3269  packets:6/5 wifi
# 15:24:22 TCP 10.0.20.179 58718 - 17.249.60.9 5223 pid:83 (apsd)
  bytes (tx/rx):3956/4924  packets:27/28 wifi
# 15:24:22 TCP 10.0.20.179 58760 - 52.86.2.3 443 pid:66 (epagd)
  bytes (tx/rx):926/17047  packets:5/314 wifi
```

### Credits
This is based on lsock by Jonathan Levin (http://newosxbook.com/index.php?page=code).  There were several significant changes to the socket protocol in 10.12 Sierra (XNU v3789) that breaks lsock.
