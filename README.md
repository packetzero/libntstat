## C++ library to receive network stats on MacOS (XNU Darwin)

### Introduction

Darwin kernel provides an unpublished API to receive pseudo realtime notifications of network connections and stats.

The API is accessible via a [kernel control socket](https://developer.apple.com/library/content/documentation/Darwin/Conceptual/NKEConceptual/control/control.html), and the application reads and writes C-structs to request and receive data.  Over time, there have been kernel changes to the structs, such as some fields changing to 64-bit and additional fields. It's safe to assume that changes will continue to be made with each major XNU release, and this library will have to be changed to adapt.

### Usage

The application implements the NetworkStatisticsListener interface (C++ pure virtual class), then creates a NetworkStatisticsClient instance, connects it to the kernel, and calls run() from a dedicated thread.  The interface is simple and has the following methods.

- onStreamAdded()
- onStreamRemoved()
- onStreamStatsUpdate() - TODO

### Credits
This is based on lsock by Jonathan Levin (http://newosxbook.com/index.php?page=code).  There were several significant changes to the socket protocol in El Capitan that breaks lsock.
