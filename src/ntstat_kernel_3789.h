// definitions from bsd/net/ntstat.h kernel header
// https://github.com/apple/darwin-xnu
// git checkout xnu-3789.70.16
// Sierra 10.12


// bsd/netinet/tcp.h
struct tcp_conn_status {
  unsigned int    probe_activated : 1;
  unsigned int    write_probe_failed : 1;
  unsigned int    read_probe_failed : 1;
  unsigned int    conn_probe_failed : 1;
};
#define    IFNAMSIZ    16

#pragma pack(push, 4)
#define __NSTAT_REVISION__      8

typedef u_int32_t       nstat_provider_id_t;
typedef u_int64_t       nstat_src_ref_t;
typedef u_int64_t       nstat_event_flags_t;

/*
typedef struct nstat_msg_hdr
{
  u_int64_t       context;
  u_int32_t       type;
  u_int16_t       length;
  u_int16_t       flags;
} nstat_msg_hdr;
*/

typedef struct nstat_counts
{
  /* Counters */
  u_int64_t       nstat_rxpackets __attribute__((aligned(8)));
  u_int64_t       nstat_rxbytes   __attribute__((aligned(8)));
  u_int64_t       nstat_txpackets __attribute__((aligned(8)));
  u_int64_t       nstat_txbytes   __attribute__((aligned(8)));

  u_int32_t       nstat_rxduplicatebytes;
  u_int32_t       nstat_rxoutoforderbytes;
  u_int32_t       nstat_txretransmit;

  u_int32_t       nstat_connectattempts;
  u_int32_t       nstat_connectsuccesses;

  u_int32_t       nstat_min_rtt;
  u_int32_t       nstat_avg_rtt;
  u_int32_t       nstat_var_rtt;

  u_int64_t       nstat_cell_rxbytes      __attribute__((aligned(8)));
  u_int64_t       nstat_cell_txbytes      __attribute__((aligned(8)));
  u_int64_t       nstat_wifi_rxbytes      __attribute__((aligned(8)));
  u_int64_t       nstat_wifi_txbytes      __attribute__((aligned(8)));
  u_int64_t       nstat_wired_rxbytes     __attribute__((aligned(8)));
  u_int64_t       nstat_wired_txbytes     __attribute__((aligned(8)));
} nstat_counts;

typedef struct nstat_tcp_descriptor
{
  union
  {
    struct sockaddr_in      v4;
    struct sockaddr_in6     v6;
  } local;

  union
  {
    struct sockaddr_in      v4;
    struct sockaddr_in6     v6;
  } remote;

  u_int32_t       ifindex;

  u_int32_t       state;

  u_int32_t       sndbufsize;
  u_int32_t       sndbufused;
  u_int32_t       rcvbufsize;
  u_int32_t       rcvbufused;
  u_int32_t       txunacked;
  u_int32_t       txwindow;
  u_int32_t       txcwindow;
  u_int32_t       traffic_class;
  u_int32_t       traffic_mgt_flags;
  char            cc_algo[16];

  u_int64_t       upid;
  u_int32_t       pid;
  char            pname[64];
  u_int64_t       eupid;
  u_int32_t       epid;

  uuid_t          uuid;
  uuid_t          euuid;
  uuid_t          vuuid;
  struct tcp_conn_status connstatus;
  uint16_t        ifnet_properties        __attribute__((aligned(4)));
} nstat_tcp_descriptor;

typedef struct nstat_udp_descriptor
{
  union
  {
    struct sockaddr_in      v4;
    struct sockaddr_in6     v6;
  } local;

  union
  {
    struct sockaddr_in      v4;
    struct sockaddr_in6     v6;
  } remote;

  u_int32_t       ifindex;

  u_int32_t       rcvbufsize;
  u_int32_t       rcvbufused;
  u_int32_t       traffic_class;

  u_int64_t       upid;
  u_int32_t       pid;
  char            pname[64];
  u_int64_t       eupid;
  u_int32_t       epid;

  uuid_t          uuid;
  uuid_t          euuid;
  uuid_t          vuuid;
  uint16_t        ifnet_properties;
} nstat_udp_descriptor;

typedef struct nstat_ifnet_desc_cellular_status
{
  u_int32_t valid_bitmask; /* indicates which fields are valid */
#define NSTAT_IFNET_DESC_CELL_LINK_QUALITY_METRIC_VALID         0x1
#define NSTAT_IFNET_DESC_CELL_UL_EFFECTIVE_BANDWIDTH_VALID      0x2
#define NSTAT_IFNET_DESC_CELL_UL_MAX_BANDWIDTH_VALID            0x4
#define NSTAT_IFNET_DESC_CELL_UL_MIN_LATENCY_VALID              0x8
#define NSTAT_IFNET_DESC_CELL_UL_EFFECTIVE_LATENCY_VALID        0x10
#define NSTAT_IFNET_DESC_CELL_UL_MAX_LATENCY_VALID              0x20
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_VALID              0x40
#define NSTAT_IFNET_DESC_CELL_UL_BYTES_LOST_VALID               0x80
#define NSTAT_IFNET_DESC_CELL_UL_MIN_QUEUE_SIZE_VALID           0x100
#define NSTAT_IFNET_DESC_CELL_UL_AVG_QUEUE_SIZE_VALID           0x200
#define NSTAT_IFNET_DESC_CELL_UL_MAX_QUEUE_SIZE_VALID           0x400
#define NSTAT_IFNET_DESC_CELL_DL_EFFECTIVE_BANDWIDTH_VALID      0x800
#define NSTAT_IFNET_DESC_CELL_DL_MAX_BANDWIDTH_VALID            0x1000
#define NSTAT_IFNET_DESC_CELL_CONFIG_INACTIVITY_TIME_VALID      0x2000
#define NSTAT_IFNET_DESC_CELL_CONFIG_BACKOFF_TIME_VALID         0x4000
#define NSTAT_IFNET_DESC_CELL_MSS_RECOMMENDED_VALID             0x8000
  u_int32_t link_quality_metric;
  u_int32_t ul_effective_bandwidth; /* Measured uplink bandwidth based on
                                     current activity (bps) */
  u_int32_t ul_max_bandwidth; /* Maximum supported uplink bandwidth
                               (bps) */
  u_int32_t ul_min_latency; /* min expected uplink latency for first hop
                             (ms) */
  u_int32_t ul_effective_latency; /* current expected uplink latency for
                                   first hop (ms) */
  u_int32_t ul_max_latency; /* max expected uplink latency first hop
                             (ms) */
  u_int32_t ul_retxt_level; /* Retransmission metric */
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_NONE       1
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_LOW        2
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_MEDIUM     3
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_HIGH       4

  u_int32_t ul_bytes_lost; /* % of total bytes lost on uplink in Q10
                            format */
  u_int32_t ul_min_queue_size; /* minimum bytes in queue */
  u_int32_t ul_avg_queue_size; /* average bytes in queue */
  u_int32_t ul_max_queue_size; /* maximum bytes in queue */
  u_int32_t dl_effective_bandwidth; /* Measured downlink bandwidth based
                                     on current activity (bps) */
  u_int32_t dl_max_bandwidth; /* Maximum supported downlink bandwidth
                               (bps) */
  u_int32_t config_inactivity_time; /* ms */
  u_int32_t config_backoff_time; /* new connections backoff time in ms */
#define NSTAT_IFNET_DESC_MSS_RECOMMENDED_NONE   0x0
#define NSTAT_IFNET_DESC_MSS_RECOMMENDED_MEDIUM 0x1
#define NSTAT_IFNET_DESC_MSS_RECOMMENDED_LOW    0x2
  u_int16_t mss_recommended; /* recommended MSS */
} nstat_ifnet_desc_cellular_status;

typedef struct nstat_ifnet_desc_wifi_status {
  u_int32_t valid_bitmask;
#define NSTAT_IFNET_DESC_WIFI_LINK_QUALITY_METRIC_VALID         0x1
#define NSTAT_IFNET_DESC_WIFI_UL_EFFECTIVE_BANDWIDTH_VALID      0x2
#define NSTAT_IFNET_DESC_WIFI_UL_MAX_BANDWIDTH_VALID            0x4
#define NSTAT_IFNET_DESC_WIFI_UL_MIN_LATENCY_VALID              0x8
#define NSTAT_IFNET_DESC_WIFI_UL_EFFECTIVE_LATENCY_VALID        0x10
#define NSTAT_IFNET_DESC_WIFI_UL_MAX_LATENCY_VALID              0x20
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_VALID              0x40
#define NSTAT_IFNET_DESC_WIFI_UL_ERROR_RATE_VALID               0x80
#define NSTAT_IFNET_DESC_WIFI_UL_BYTES_LOST_VALID               0x100
#define NSTAT_IFNET_DESC_WIFI_DL_EFFECTIVE_BANDWIDTH_VALID      0x200
#define NSTAT_IFNET_DESC_WIFI_DL_MAX_BANDWIDTH_VALID            0x400
#define NSTAT_IFNET_DESC_WIFI_DL_MIN_LATENCY_VALID              0x800
#define NSTAT_IFNET_DESC_WIFI_DL_EFFECTIVE_LATENCY_VALID        0x1000
#define NSTAT_IFNET_DESC_WIFI_DL_MAX_LATENCY_VALID              0x2000
#define NSTAT_IFNET_DESC_WIFI_DL_ERROR_RATE_VALID               0x4000
#define NSTAT_IFNET_DESC_WIFI_CONFIG_FREQUENCY_VALID            0x8000
#define NSTAT_IFNET_DESC_WIFI_CONFIG_MULTICAST_RATE_VALID       0x10000
#define NSTAT_IFNET_DESC_WIFI_CONFIG_SCAN_COUNT_VALID           0x20000
#define NSTAT_IFNET_DESC_WIFI_CONFIG_SCAN_DURATION_VALID        0x40000
  u_int32_t link_quality_metric; /* link quality metric */
  u_int32_t ul_effective_bandwidth; /* Measured uplink bandwidth based on
                                     current activity (bps) */
  u_int32_t ul_max_bandwidth; /* Maximum supported uplink bandwidth
                               (bps) */
  u_int32_t ul_min_latency; /* min expected uplink latency for first hop
                             (ms) */
  u_int32_t ul_effective_latency; /* current expected uplink latency for
                                   first hop (ms) */
  u_int32_t ul_max_latency; /* max expected uplink latency for first hop
                             (ms) */
  u_int32_t ul_retxt_level; /* Retransmission metric */
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_NONE       1
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_LOW        2
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_MEDIUM     3
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_HIGH       4

  u_int32_t ul_bytes_lost; /* % of total bytes lost on uplink in Q10
                            format */
  u_int32_t ul_error_rate; /* % of bytes dropped on uplink after many
                            retransmissions in Q10 format */
  u_int32_t dl_effective_bandwidth; /* Measured downlink bandwidth based
                                     on current activity (bps) */
  u_int32_t dl_max_bandwidth; /* Maximum supported downlink bandwidth
                               (bps) */
  /*
   * The download latency values indicate the time AP may have to wait
   * for the  driver to receive the packet. These values give the range
   * of expected latency mainly due to co-existence events and channel
   * hopping where the interface becomes unavailable.
   */
  u_int32_t dl_min_latency; /* min expected latency for first hop in ms */
  u_int32_t dl_effective_latency; /* current expected latency for first
                                   hop in ms */
  u_int32_t dl_max_latency; /* max expected latency for first hop in ms */
  u_int32_t dl_error_rate; /* % of CRC or other errors in Q10 format */
  u_int32_t config_frequency; /* 2.4 or 5 GHz */
#define NSTAT_IFNET_DESC_WIFI_CONFIG_FREQUENCY_2_4_GHZ  1
#define NSTAT_IFNET_DESC_WIFI_CONFIG_FREQUENCY_5_0_GHZ  2
  u_int32_t config_multicast_rate; /* bps */
  u_int32_t scan_count; /* scan count during the previous period */
  u_int32_t scan_duration; /* scan duration in ms */
} nstat_ifnet_desc_wifi_status;

typedef struct nstat_ifnet_desc_link_status
{
  u_int32_t       link_status_type;
  union {
    nstat_ifnet_desc_cellular_status        cellular;
    nstat_ifnet_desc_wifi_status            wifi;
  } u;
} nstat_ifnet_desc_link_status;

#ifndef IF_DESCSIZE
#define IF_DESCSIZE 128
#endif
typedef struct nstat_ifnet_descriptor
{
  char                            name[IFNAMSIZ+1];
  u_int32_t                       ifindex;
  u_int64_t                       threshold;
  unsigned int                    type;
  char                            description[IF_DESCSIZE];
  nstat_ifnet_desc_link_status    link_status;
} nstat_ifnet_descriptor;

typedef struct nstat_sysinfo_descriptor
{
  u_int32_t       flags;
} nstat_sysinfo_descriptor;


typedef struct nstat_msg_add_src
{
  nstat_msg_hdr           hdr;
  nstat_provider_id_t     provider;
  u_int8_t                        param[];
} nstat_msg_add_src_req;

typedef struct nstat_msg_add_all_srcs
{
  nstat_msg_hdr           hdr;
  nstat_provider_id_t     provider;
  u_int64_t                       filter;
  nstat_event_flags_t     events;
  pid_t                           target_pid;
  uuid_t                          target_uuid;
} nstat_msg_add_all_srcs;

typedef struct nstat_msg_src_added
{
  nstat_msg_hdr           hdr;
  nstat_provider_id_t     provider;
  nstat_src_ref_t         srcref;
} nstat_msg_src_added;

typedef struct nstat_msg_rem_src
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
} nstat_msg_rem_src_req;

typedef struct nstat_msg_get_src_description
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
} nstat_msg_get_src_description;

typedef struct nstat_msg_set_filter
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
  u_int32_t               filter;
} nstat_msg_set_filter;

typedef struct nstat_msg_src_description
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
  nstat_event_flags_t     event_flags;
  nstat_provider_id_t     provider;
  u_int8_t                        data[];
} nstat_msg_src_description;

typedef struct nstat_msg_query_src
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
} nstat_msg_query_src_req;

typedef struct nstat_msg_src_counts
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
  nstat_event_flags_t     event_flags;
  nstat_counts            counts;
} nstat_msg_src_counts;

typedef struct nstat_msg_src_update
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
  nstat_event_flags_t     event_flags;
  nstat_counts            counts;
  nstat_provider_id_t     provider;
  u_int8_t                        data[];
} nstat_msg_src_update;

typedef struct nstat_msg_src_removed
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
} nstat_msg_src_removed;

typedef struct nstat_sysinfo_counts
{
  /* Counters */
  u_int32_t       nstat_sysinfo_len;
  u_int32_t       pad;
  u_int8_t        nstat_sysinfo_keyvals[];
} __attribute__((packed)) nstat_sysinfo_counts;

typedef struct nstat_msg_sysinfo_counts
{
  nstat_msg_hdr           hdr;
  nstat_src_ref_t         srcref;
  nstat_sysinfo_counts    counts;
} __attribute__((packed)) nstat_msg_sysinfo_counts;

#pragma pack(pop)
