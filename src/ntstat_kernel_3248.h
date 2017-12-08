// definitions from bsd/net/ntstat.h kernel header
// https://github.com/apple/darwin-xnu
// git checkout xnu-3248.60.10
// El Capitan 10.11

// bsd/netinet/tcp.h
struct tcp_conn_status {
  unsigned int    probe_activated : 1;
  unsigned int    write_probe_failed : 1;
  unsigned int    read_probe_failed : 1;
  unsigned int    conn_probe_failed : 1;
};
#define    IFNAMSIZ    16

#pragma pack(push, 4)
#pragma mark -- Common Data Structures --

#define __NSTAT_REVISION__	7

typedef	u_int32_t	nstat_provider_id_t;
typedef	u_int32_t	nstat_src_ref_t;

typedef struct nstat_counts
{
	/* Counters */
	u_int64_t	nstat_rxpackets	__attribute__((aligned(8)));
	u_int64_t	nstat_rxbytes	__attribute__((aligned(8)));
	u_int64_t	nstat_txpackets	__attribute__((aligned(8)));
	u_int64_t	nstat_txbytes	__attribute__((aligned(8)));

	u_int32_t	nstat_rxduplicatebytes;
	u_int32_t	nstat_rxoutoforderbytes;
	u_int32_t	nstat_txretransmit;

	u_int32_t	nstat_connectattempts;
	u_int32_t	nstat_connectsuccesses;

	u_int32_t	nstat_min_rtt;
	u_int32_t	nstat_avg_rtt;
	u_int32_t	nstat_var_rtt;

	u_int64_t	nstat_cell_rxbytes	__attribute__((aligned(8)));
	u_int64_t	nstat_cell_txbytes	__attribute__((aligned(8)));
	u_int64_t	nstat_wifi_rxbytes	__attribute__((aligned(8)));
	u_int64_t	nstat_wifi_txbytes	__attribute__((aligned(8)));
	u_int64_t	nstat_wired_rxbytes	__attribute__((aligned(8)));
	u_int64_t	nstat_wired_txbytes	__attribute__((aligned(8)));
} nstat_counts;

typedef struct nstat_sysinfo_keyval
{
	u_int32_t	nstat_sysinfo_key;
	u_int32_t	nstat_sysinfo_flags;
	union {
			int64_t	nstat_sysinfo_scalar;
			double	nstat_sysinfo_distribution;
	} u;
} __attribute__((packed)) nstat_sysinfo_keyval;

#define	NSTAT_SYSINFO_FLAG_SCALAR	0x0001
#define	NSTAT_SYSINFO_FLAG_DISTRIBUTION	0x0002

#define NSTAT_MAX_MSG_SIZE	4096

typedef struct nstat_sysinfo_counts
{
	/* Counters */
	u_int32_t	nstat_sysinfo_len;
	u_int32_t	pad;
	u_int8_t	nstat_sysinfo_keyvals[];
} __attribute__((packed)) nstat_sysinfo_counts;

enum
{
	NSTAT_SYSINFO_KEY_MBUF_256B_TOTAL	= 1
	,NSTAT_SYSINFO_KEY_MBUF_2KB_TOTAL	= 2
	,NSTAT_SYSINFO_KEY_MBUF_4KB_TOTAL	= 3
	,NSTAT_SYSINFO_KEY_SOCK_MBCNT		= 4
	,NSTAT_SYSINFO_KEY_SOCK_ATMBLIMIT	= 5
	,NSTAT_SYSINFO_KEY_IPV4_AVGRTT		= 6
	,NSTAT_SYSINFO_KEY_IPV6_AVGRTT		= 7
	,NSTAT_SYSINFO_KEY_SEND_PLR		= 8
	,NSTAT_SYSINFO_KEY_RECV_PLR		= 9
	,NSTAT_SYSINFO_KEY_SEND_TLRTO		= 10
	,NSTAT_SYSINFO_KEY_SEND_REORDERRATE	= 11
	,NSTAT_SYSINFO_CONNECTION_ATTEMPTS	= 12
	,NSTAT_SYSINFO_CONNECTION_ACCEPTS	= 13
	,NSTAT_SYSINFO_ECN_CLIENT_SETUP		= 14
	,NSTAT_SYSINFO_ECN_SERVER_SETUP		= 15
	,NSTAT_SYSINFO_ECN_CLIENT_SUCCESS	= 16
	,NSTAT_SYSINFO_ECN_SERVER_SUCCESS	= 17
	,NSTAT_SYSINFO_ECN_NOT_SUPPORTED	= 18
	,NSTAT_SYSINFO_ECN_LOST_SYN		= 19
	,NSTAT_SYSINFO_ECN_LOST_SYNACK		= 20
	,NSTAT_SYSINFO_ECN_RECV_CE		= 21
	,NSTAT_SYSINFO_ECN_RECV_ECE		= 22
	,NSTAT_SYSINFO_ECN_SENT_ECE		= 23
	,NSTAT_SYSINFO_ECN_CONN_RECV_CE		= 24
	,NSTAT_SYSINFO_ECN_CONN_PLNOCE		= 25
	,NSTAT_SYSINFO_ECN_CONN_PL_CE		= 26
	,NSTAT_SYSINFO_ECN_CONN_NOPL_CE		= 27
	,NSTAT_SYSINFO_MBUF_16KB_TOTAL		= 28
	,NSTAT_SYSINFO_ECN_CLIENT_ENABLED	= 29
	,NSTAT_SYSINFO_ECN_SERVER_ENABLED	= 30
	,NSTAT_SYSINFO_ECN_CONN_RECV_ECE	= 31
	,NSTAT_SYSINFO_MBUF_MEM_RELEASED	= 32
	,NSTAT_SYSINFO_MBUF_DRAIN_CNT		= 33
	,NSTAT_SYSINFO_TFO_SYN_DATA_RCV		= 34
	,NSTAT_SYSINFO_TFO_COOKIE_REQ_RCV	= 35
	,NSTAT_SYSINFO_TFO_COOKIE_SENT		= 36
	,NSTAT_SYSINFO_TFO_COOKIE_INVALID	= 37
	,NSTAT_SYSINFO_TFO_COOKIE_REQ		= 38
	,NSTAT_SYSINFO_TFO_COOKIE_RCV		= 39
	,NSTAT_SYSINFO_TFO_SYN_DATA_SENT	= 40
	,NSTAT_SYSINFO_TFO_SYN_DATA_ACKED	= 41
	,NSTAT_SYSINFO_TFO_SYN_LOSS		= 42
	,NSTAT_SYSINFO_TFO_BLACKHOLE		= 43
	,NSTAT_SYSINFO_ECN_FALLBACK_SYNLOSS	= 44
	,NSTAT_SYSINFO_ECN_FALLBACK_REORDER	= 45
	,NSTAT_SYSINFO_ECN_FALLBACK_CE		= 46
	,NSTAT_SYSINFO_ECN_IFNET_TYPE		= 47
	,NSTAT_SYSINFO_ECN_IFNET_PROTO		= 48
	,NSTAT_SYSINFO_ECN_IFNET_CLIENT_SETUP	= 49
	,NSTAT_SYSINFO_ECN_IFNET_SERVER_SETUP	= 50
	,NSTAT_SYSINFO_ECN_IFNET_CLIENT_SUCCESS	= 51
	,NSTAT_SYSINFO_ECN_IFNET_SERVER_SUCCESS	= 52
	,NSTAT_SYSINFO_ECN_IFNET_PEER_NOSUPPORT	= 53
	,NSTAT_SYSINFO_ECN_IFNET_SYN_LOST	= 54
	,NSTAT_SYSINFO_ECN_IFNET_SYNACK_LOST	= 55
	,NSTAT_SYSINFO_ECN_IFNET_RECV_CE	= 56
	,NSTAT_SYSINFO_ECN_IFNET_RECV_ECE	= 57
	,NSTAT_SYSINFO_ECN_IFNET_SENT_ECE	= 58
	,NSTAT_SYSINFO_ECN_IFNET_CONN_RECV_CE	= 59
	,NSTAT_SYSINFO_ECN_IFNET_CONN_RECV_ECE	= 60
	,NSTAT_SYSINFO_ECN_IFNET_CONN_PLNOCE	= 61
	,NSTAT_SYSINFO_ECN_IFNET_CONN_PLCE	= 62
	,NSTAT_SYSINFO_ECN_IFNET_CONN_NOPLCE	= 63
	,NSTAT_SYSINFO_ECN_IFNET_FALLBACK_SYNLOSS = 64
	,NSTAT_SYSINFO_ECN_IFNET_FALLBACK_REORDER = 65
	,NSTAT_SYSINFO_ECN_IFNET_FALLBACK_CE	= 66
	,NSTAT_SYSINFO_ECN_IFNET_ON_RTT_AVG	= 67
	,NSTAT_SYSINFO_ECN_IFNET_ON_RTT_VAR	= 68
	,NSTAT_SYSINFO_ECN_IFNET_ON_OOPERCENT	= 69
	,NSTAT_SYSINFO_ECN_IFNET_ON_SACK_EPISODE = 70
	,NSTAT_SYSINFO_ECN_IFNET_ON_REORDER_PERCENT = 71
	,NSTAT_SYSINFO_ECN_IFNET_ON_RXMIT_PERCENT = 72
	,NSTAT_SYSINFO_ECN_IFNET_ON_RXMIT_DROP	= 73
	,NSTAT_SYSINFO_ECN_IFNET_OFF_RTT_AVG	= 74
	,NSTAT_SYSINFO_ECN_IFNET_OFF_RTT_VAR	= 75
	,NSTAT_SYSINFO_ECN_IFNET_OFF_OOPERCENT	= 76
	,NSTAT_SYSINFO_ECN_IFNET_OFF_SACK_EPISODE = 77
	,NSTAT_SYSINFO_ECN_IFNET_OFF_REORDER_PERCENT = 78
	,NSTAT_SYSINFO_ECN_IFNET_OFF_RXMIT_PERCENT = 79
	,NSTAT_SYSINFO_ECN_IFNET_OFF_RXMIT_DROP = 80
	,NSTAT_SYSINFO_ECN_IFNET_ON_TOTAL_TXPKTS = 81
	,NSTAT_SYSINFO_ECN_IFNET_ON_TOTAL_RXMTPKTS = 82
	,NSTAT_SYSINFO_ECN_IFNET_ON_TOTAL_RXPKTS = 83
	,NSTAT_SYSINFO_ECN_IFNET_ON_TOTAL_OOPKTS = 84
	,NSTAT_SYSINFO_ECN_IFNET_ON_DROP_RST = 85
	,NSTAT_SYSINFO_ECN_IFNET_OFF_TOTAL_TXPKTS = 86
	,NSTAT_SYSINFO_ECN_IFNET_OFF_TOTAL_RXMTPKTS = 87
	,NSTAT_SYSINFO_ECN_IFNET_OFF_TOTAL_RXPKTS = 88
	,NSTAT_SYSINFO_ECN_IFNET_OFF_TOTAL_OOPKTS = 89
	,NSTAT_SYSINFO_ECN_IFNET_OFF_DROP_RST = 90
	,NSTAT_SYSINFO_ECN_IFNET_TOTAL_CONN = 91
// NSTAT_SYSINFO_ENUM_VERSION must be updated any time a value is added
#define	NSTAT_SYSINFO_ENUM_VERSION	20151208
};

#pragma mark -- Network Statistics Providers --


// Interface properties

#define NSTAT_IFNET_IS_UNKNOWN_TYPE      0x01
#define NSTAT_IFNET_IS_LOOPBACK          0x02
#define NSTAT_IFNET_IS_CELLULAR          0x04
#define NSTAT_IFNET_IS_WIFI              0x08
#define NSTAT_IFNET_IS_WIRED             0x10
#define NSTAT_IFNET_IS_AWDL              0x20
#define NSTAT_IFNET_IS_EXPENSIVE         0x40
#define NSTAT_IFNET_IS_VPN               0x80
#define NSTAT_IFNET_VIA_CELLFALLBACK     0x100

enum
{
  // generic response messages
  NSTAT_MSG_TYPE_SUCCESS      = 0
  ,NSTAT_MSG_TYPE_ERROR      = 1
  
  // Requests
  ,NSTAT_MSG_TYPE_ADD_SRC      = 1001
  ,NSTAT_MSG_TYPE_ADD_ALL_SRCS    = 1002
  ,NSTAT_MSG_TYPE_REM_SRC      = 1003
  ,NSTAT_MSG_TYPE_QUERY_SRC    = 1004
  ,NSTAT_MSG_TYPE_GET_SRC_DESC    = 1005
  ,NSTAT_MSG_TYPE_SET_FILTER     = 1006
  
  // Responses/Notfications
  ,NSTAT_MSG_TYPE_SRC_ADDED    = 10001
  ,NSTAT_MSG_TYPE_SRC_REMOVED    = 10002
  ,NSTAT_MSG_TYPE_SRC_DESC    = 10003
  ,NSTAT_MSG_TYPE_SRC_COUNTS    = 10004
};


enum
{
	NSTAT_PROVIDER_NONE	= 0
	,NSTAT_PROVIDER_ROUTE	= 1
	,NSTAT_PROVIDER_TCP	= 2
	,NSTAT_PROVIDER_UDP	= 3
	,NSTAT_PROVIDER_IFNET	= 4
	,NSTAT_PROVIDER_SYSINFO = 5
};
#define NSTAT_PROVIDER_LAST NSTAT_PROVIDER_SYSINFO
#define NSTAT_PROVIDER_COUNT (NSTAT_PROVIDER_LAST+1)

typedef struct nstat_route_add_param
{
	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} dst;
	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} mask;
	u_int32_t	ifindex;
} nstat_route_add_param;

typedef struct nstat_tcp_add_param
{
	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} local;
	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} remote;
} nstat_tcp_add_param;

typedef struct nstat_tcp_descriptor
{
	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} local;

	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} remote;

	u_int32_t	ifindex;

	u_int32_t	state;

	u_int32_t	sndbufsize;
	u_int32_t	sndbufused;
	u_int32_t	rcvbufsize;
	u_int32_t	rcvbufused;
	u_int32_t	txunacked;
	u_int32_t	txwindow;
	u_int32_t	txcwindow;
	u_int32_t	traffic_class;
	u_int32_t	traffic_mgt_flags;
	char		cc_algo[16];

	u_int64_t	upid;
	u_int32_t	pid;
	char		pname[64];
	u_int64_t	eupid;
	u_int32_t	epid;

	uint8_t		uuid[16];
	uint8_t		euuid[16];
	uint8_t		vuuid[16];
	struct tcp_conn_status connstatus;
	uint16_t	ifnet_properties	__attribute__((aligned(4)));
} nstat_tcp_descriptor;

typedef struct nstat_tcp_add_param	nstat_udp_add_param;

typedef struct nstat_udp_descriptor
{
	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} local;

	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	} remote;

	u_int32_t	ifindex;

	u_int32_t	rcvbufsize;
	u_int32_t	rcvbufused;
	u_int32_t	traffic_class;

	u_int64_t	upid;
	u_int32_t	pid;
	char		pname[64];
	u_int64_t	eupid;
	u_int32_t	epid;

	uint8_t		uuid[16];
	uint8_t		euuid[16];
	uint8_t		vuuid[16];
	uint16_t	ifnet_properties;
} nstat_udp_descriptor;

typedef struct nstat_route_descriptor
{
	u_int64_t	id;
	u_int64_t	parent_id;
	u_int64_t	gateway_id;

	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
		struct sockaddr		sa;
	} dst;

	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
		struct sockaddr		sa;
	} mask;

	union
	{
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
		struct sockaddr		sa;
	} gateway;

	u_int32_t	ifindex;
	u_int32_t	flags;

} nstat_route_descriptor;

typedef struct nstat_ifnet_add_param
{
	u_int32_t	ifindex;
	u_int64_t	threshold;
} nstat_ifnet_add_param;

typedef struct nstat_ifnet_desc_cellular_status
{
	u_int32_t valid_bitmask; /* indicates which fields are valid */
#define NSTAT_IFNET_DESC_CELL_LINK_QUALITY_METRIC_VALID		0x1
#define NSTAT_IFNET_DESC_CELL_UL_EFFECTIVE_BANDWIDTH_VALID	0x2
#define NSTAT_IFNET_DESC_CELL_UL_MAX_BANDWIDTH_VALID		0x4
#define NSTAT_IFNET_DESC_CELL_UL_MIN_LATENCY_VALID		0x8
#define NSTAT_IFNET_DESC_CELL_UL_EFFECTIVE_LATENCY_VALID	0x10
#define NSTAT_IFNET_DESC_CELL_UL_MAX_LATENCY_VALID		0x20
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_VALID		0x40
#define NSTAT_IFNET_DESC_CELL_UL_BYTES_LOST_VALID		0x80
#define NSTAT_IFNET_DESC_CELL_UL_MIN_QUEUE_SIZE_VALID		0x100
#define NSTAT_IFNET_DESC_CELL_UL_AVG_QUEUE_SIZE_VALID		0x200
#define NSTAT_IFNET_DESC_CELL_UL_MAX_QUEUE_SIZE_VALID		0x400
#define NSTAT_IFNET_DESC_CELL_DL_EFFECTIVE_BANDWIDTH_VALID	0x800
#define NSTAT_IFNET_DESC_CELL_DL_MAX_BANDWIDTH_VALID		0x1000
#define NSTAT_IFNET_DESC_CELL_CONFIG_INACTIVITY_TIME_VALID	0x2000
#define NSTAT_IFNET_DESC_CELL_CONFIG_BACKOFF_TIME_VALID		0x4000
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
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_NONE	1
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_LOW	2
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_MEDIUM	3
#define NSTAT_IFNET_DESC_CELL_UL_RETXT_LEVEL_HIGH	4

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
} nstat_ifnet_desc_cellular_status;

typedef struct nstat_ifnet_desc_wifi_status {
	u_int32_t valid_bitmask;
#define	NSTAT_IFNET_DESC_WIFI_LINK_QUALITY_METRIC_VALID		0x1
#define	NSTAT_IFNET_DESC_WIFI_UL_EFFECTIVE_BANDWIDTH_VALID	0x2
#define	NSTAT_IFNET_DESC_WIFI_UL_MAX_BANDWIDTH_VALID		0x4
#define	NSTAT_IFNET_DESC_WIFI_UL_MIN_LATENCY_VALID		0x8
#define	NSTAT_IFNET_DESC_WIFI_UL_EFFECTIVE_LATENCY_VALID	0x10
#define	NSTAT_IFNET_DESC_WIFI_UL_MAX_LATENCY_VALID		0x20
#define	NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_VALID		0x40
#define	NSTAT_IFNET_DESC_WIFI_UL_ERROR_RATE_VALID		0x80
#define	NSTAT_IFNET_DESC_WIFI_UL_BYTES_LOST_VALID		0x100
#define	NSTAT_IFNET_DESC_WIFI_DL_EFFECTIVE_BANDWIDTH_VALID	0x200
#define	NSTAT_IFNET_DESC_WIFI_DL_MAX_BANDWIDTH_VALID		0x400
#define	NSTAT_IFNET_DESC_WIFI_DL_MIN_LATENCY_VALID		0x800
#define	NSTAT_IFNET_DESC_WIFI_DL_EFFECTIVE_LATENCY_VALID	0x1000
#define	NSTAT_IFNET_DESC_WIFI_DL_MAX_LATENCY_VALID		0x2000
#define	NSTAT_IFNET_DESC_WIFI_DL_ERROR_RATE_VALID		0x4000
#define	NSTAT_IFNET_DESC_WIFI_CONFIG_FREQUENCY_VALID		0x8000
#define	NSTAT_IFNET_DESC_WIFI_CONFIG_MULTICAST_RATE_VALID	0x10000
#define	NSTAT_IFNET_DESC_WIFI_CONFIG_SCAN_COUNT_VALID		0x20000
#define	NSTAT_IFNET_DESC_WIFI_CONFIG_SCAN_DURATION_VALID	0x40000
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
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_NONE	1
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_LOW	2
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_MEDIUM	3
#define NSTAT_IFNET_DESC_WIFI_UL_RETXT_LEVEL_HIGH	4

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
#define	NSTAT_IFNET_DESC_WIFI_CONFIG_FREQUENCY_2_4_GHZ	1
#define	NSTAT_IFNET_DESC_WIFI_CONFIG_FREQUENCY_5_0_GHZ	2
	u_int32_t config_multicast_rate; /* bps */
	u_int32_t scan_count; /* scan count during the previous period */
	u_int32_t scan_duration; /* scan duration in ms */
} nstat_ifnet_desc_wifi_status;

enum
{
	NSTAT_IFNET_DESC_LINK_STATUS_TYPE_NONE = 0
	,NSTAT_IFNET_DESC_LINK_STATUS_TYPE_CELLULAR = 1
	,NSTAT_IFNET_DESC_LINK_STATUS_TYPE_WIFI	= 2
};

typedef struct nstat_ifnet_desc_link_status
{
	u_int32_t	link_status_type;
	union {
		nstat_ifnet_desc_cellular_status	cellular;
		nstat_ifnet_desc_wifi_status		wifi;
	} u;
} nstat_ifnet_desc_link_status;

#ifndef	IF_DESCSIZE
#define	IF_DESCSIZE 128
#endif
typedef struct nstat_ifnet_descriptor
{
	char				name[IFNAMSIZ+1];
	u_int32_t			ifindex;
	u_int64_t			threshold;
	unsigned int			type;
	char				description[IF_DESCSIZE];
	nstat_ifnet_desc_link_status	link_status;
} nstat_ifnet_descriptor;

typedef struct nstat_sysinfo_descriptor
{
	u_int32_t	flags;
} nstat_sysinfo_descriptor;

typedef struct nstat_sysinfo_add_param
{
	/* To indicate which system level information should be collected */
	u_int32_t	flags;
} nstat_sysinfo_add_param;

#define	NSTAT_SYSINFO_MBUF_STATS	0x0001
#define	NSTAT_SYSINFO_TCP_STATS		0x0002
#define NSTAT_SYSINFO_IFNET_ECN_STATS	0x0003

#pragma mark -- Network Statistics User Client --

#define	NET_STAT_CONTROL_NAME	"com.apple.network.statistics"


enum
{
	NSTAT_SRC_REF_ALL	= 0xffffffff
	,NSTAT_SRC_REF_INVALID	= 0
};

/* Source-level filters */
enum
{
	NSTAT_FILTER_NOZEROBYTES             = 0x00000001
};

/* Provider-level filters */
enum
{
	NSTAT_FILTER_ACCEPT_UNKNOWN          = 0x00000001
	,NSTAT_FILTER_ACCEPT_LOOPBACK        = 0x00000002
	,NSTAT_FILTER_ACCEPT_CELLULAR        = 0x00000004
	,NSTAT_FILTER_ACCEPT_WIFI            = 0x00000008
	,NSTAT_FILTER_ACCEPT_WIRED           = 0x00000010
	,NSTAT_FILTER_ACCEPT_ALL             = 0x0000001F
	,NSTAT_FILTER_IFNET_FLAGS            = 0x000000FF

	,NSTAT_FILTER_PROVIDER_NOZEROBYTES   = 0x00000100

	,NSTAT_FILTER_TCP_NO_LISTENER        = 0x00001000
	,NSTAT_FILTER_TCP_ONLY_LISTENER      = 0x00002000
	,NSTAT_FILTER_TCP_INTERFACE_ATTACH   = 0x00004000
	,NSTAT_FILTER_TCP_FLAGS              = 0x0000F000

	,NSTAT_FILTER_UDP_INTERFACE_ATTACH   = 0x00010000
	,NSTAT_FILTER_UDP_FLAGS              = 0x000F0000

	,NSTAT_FILTER_SUPPRESS_SRC_ADDED     = 0x00100000
	,NSTAT_FILTER_REQUIRE_SRC_ADDED      = 0x00200000
};

enum
{
	NSTAT_MSG_HDR_FLAG_SUPPORTS_AGGREGATE	= 1 << 0,
	NSTAT_MSG_HDR_FLAG_CONTINUATION		= 1 << 1,
	NSTAT_MSG_HDR_FLAG_CLOSING		= 1 << 2,
};
/*
typedef struct nstat_msg_hdr
{
	u_int64_t	context;
	u_int32_t	type;
	u_int16_t	length;
	u_int16_t	flags;
} nstat_msg_hdr;
*/

typedef struct nstat_msg_error
{
	nstat_msg_hdr	hdr;
	u_int32_t		error;	// errno error
} nstat_msg_error;

typedef struct nstat_msg_add_src
{
	nstat_msg_hdr		hdr;
	nstat_provider_id_t	provider;
	u_int8_t			param[];
} nstat_msg_add_src_req;

typedef struct nstat_msg_add_all_srcs
{
	nstat_msg_hdr		hdr;
	nstat_provider_id_t	provider;
	u_int64_t		filter;
} nstat_msg_add_all_srcs;

typedef struct nstat_msg_src_added
{
	nstat_msg_hdr		hdr;
	nstat_provider_id_t	provider;
	nstat_src_ref_t		srcref;
} nstat_msg_src_added;

typedef struct nstat_msg_rem_src
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
} nstat_msg_rem_src_req;

typedef struct nstat_msg_get_src_description
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
} nstat_msg_get_src_description;

typedef struct nstat_msg_set_filter
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
	u_int32_t		filter;
} nstat_msg_set_filter;

typedef struct nstat_msg_src_description
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
	nstat_provider_id_t	provider;
	u_int8_t			data[];
} nstat_msg_src_description;

typedef struct nstat_msg_query_src
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
} nstat_msg_query_src_req;

typedef struct nstat_msg_src_counts
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
	nstat_counts		counts;
} nstat_msg_src_counts;

typedef struct nstat_msg_src_update
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
	nstat_counts		counts;
	nstat_provider_id_t	provider;
	u_int8_t			data[];
} nstat_msg_src_update;

typedef struct nstat_msg_src_removed
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
} nstat_msg_src_removed;

typedef struct nstat_msg_sysinfo_counts
{
	nstat_msg_hdr		hdr;
	nstat_src_ref_t		srcref;
	nstat_sysinfo_counts	counts;
} __attribute__((packed)) nstat_msg_sysinfo_counts;

#pragma pack(pop)
