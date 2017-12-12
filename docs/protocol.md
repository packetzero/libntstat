# Darwin "network.statistics" Binary API

This document gives an overview of the API that can be used by applications to receive network stream data from the Darwin kernel.  The NetworkStatistics private framework library used by *Activity Monitor* application uses this API.  The API is accessible via a [kernel control socket](https://developer.apple.com/library/content/documentation/Darwin/Conceptual/NKEConceptual/control/control.html), and currently does not require elevated privilege.

**NOTE: This API is undocumented.  Applications leveraging this API will have to anticipate and adapt to breaking changes in each major kernel release.**

**NOTE: Domain names are not part of the API, only IP Addresses**.

### Terminology

**Source**  A source is a bi-directional TCP connection, or uni-directional UDP flow.  The kernel assigns a unique srcRef to each source.  The srcRef datatype changed from uin32_t to uint64_t in XNU version 3789 (Sierra).

### Struct header
Each request and response message is a struct that starts with the following header.  The *context* field is used by the application to match response messages to the request message.  The *type* field needs to be one of the *NSTAT_MSG_TYPE_* enum values.  The *length* field should match the length of the request or response message struct.  The kernel (and application) can use this as a versioning check, as some message structs have different size across kernel versions.

```
typedef struct nstat_msg_hdr
{
  u_int64_t       context;
  u_int32_t       type;
  u_int16_t       length;
  u_int16_t       flags;
} nstat_msg_hdr;
```

## Request Types
- **NSTAT_MSG_TYPE_ADD_SRC** Add a specific source.
- **NSTAT_MSG_TYPE_ADD_ALL_SRCS** Subscribe to all sources for a specific provider (TCP, UDP, etc).
- **NSTAT_MSG_TYPE_REM_SRC** Unsubscribe from a source.
- **NSTAT_MSG_TYPE_QUERY_SRC** Request SRC_COUNTS for a specific source or all active sources.
- **NSTAT_MSG_TYPE_GET_SRC_DESC** Request details (addresses and ports) for all a specific source.

## Response Types
- **NSTAT_MSG_TYPE_SUCCESS** Last response message for a request, indicating success.
- **NSTAT_MSG_TYPE_SRC_ADDED** After successfully subscribing to sources (via ADD_SRC or ADD_ALL_SRC), the application will receive *SRC_ADDED* messages for new sources. The message is not very informative.  It consists simply of the header, providerId, and srcRef.  To get the stream details, the application must send a GET_SRC_DESC message for it.
- **NSTAT_MSG_TYPE_SRC_REMOVED** When the kernel has determined that a source is finished (e.g. TCP connection terminated), the application will receive a *SRC_REMOVED* message for it.
- **NSTAT_MSG_TYPE_SRC_DESC** Provides details about a source, such as local and remote address and layer-4 ports, interface (en0, etc.), process ID, and abbreviated process name.
- **NSTAT_MSG_TYPE_SRC_COUNTS** Prior to a *SRC_REMOVED* message, a SRC_COUNTS will be received to summarize the packets and bytes transmitted and received.  If the application wants to receive an update on persistent connections, it must send a *QUERY_SRC* request message.  In addition to the total counters, there are counters broken down by medium: wifi, wired, and cellular.  Note that prior to XNU version 3789, the *wired* counters were not specified.
- **NSTAT_MSG_TYPE_ERROR** Direct response to a request message, indicating error.  The response message struct has an *errno.h* error code.  To determine the request that was the cause of the failure, the application needs to keep track of request messages and lookup request message with matching *context* field.  Examples of common errors:  Trying to ADD_ALL_SRC with invalid providerId will yield ENOENT error.  Requesting GET_SRC_DESC or QUERY_SRC for a source that has already been reported with a SRC_REMOVED message.  If your application is not keeping up with reading the response messages, the buffer is full, and the kernel cannot write any more messages.  This cause an ERROR message with ENOMEM error code.

## Kernel Control Socket

If you run the command `netstat -a` from a command-prompt, you will see something similar to the following in the output.  The **com.apple.network.statistics** name is the one we are interested in.  In this example, there are 6 active socket connections established by applications.  We can see that the receive buffer size is 8192 bytes, while the output buffer size is 2048 bytes.  My guess is that this is from the viewpoint of the application, and the kernel can write 8KB of response messages.
```
Registered kernel control modules
id       flags    pcbcount rcvbuf   sndbuf   name
       3        9        0   524288   524288 com.apple.content-filter
       6        1       12    65536    65536 com.apple.net.netagent
       9        0       32     8192     2048 com.apple.netsrc
       a       18        6     8192     2048 com.apple.network.statistics
..
Active kernel control sockets
Proto Recv-Q Send-Q   unit     id name
..
kctl       0      0      1     10 com.apple.network.statistics
kctl       0      0      2     10 com.apple.network.statistics
kctl       0      0      3     10 com.apple.network.statistics
kctl       0      0      4     10 com.apple.network.statistics
kctl       0      0      5     10 com.apple.network.statistics
kctl       0      0      8     10 com.apple.network.statistics
```
