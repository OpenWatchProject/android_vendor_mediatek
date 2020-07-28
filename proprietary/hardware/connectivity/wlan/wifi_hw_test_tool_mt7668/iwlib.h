#ifndef IWLIB_H
#define IWLIB_H

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
//#include <net/if_ether.h>
//#include <netinet/if_ether.h>
#include <sys/time.h>
#include <unistd.h>

#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>


#ifndef __user
#define __user
#endif

#include <linux/types.h>

#include <sys/socket.h>
//#include <net/if.h>

#include <linux/wireless.h>


#undef IW_GCC_HAS_BROKEN_INLINE
#if __GNUC__ == 3
#if __GNUC_MINOR__ >= 1 && __GNUC_MINOR__ < 4
#define IW_GCC_HAS_BROKEN_INLINE	1
#endif
#endif

#if __GNUC__ == 4
#define IW_GCC_HAS_BROKEN_INLINE	1
#endif

#ifdef IW_GCC_HAS_BROKEN_INLINE
#ifdef inline
#undef inline
#endif
#define inline		inline		__attribute__((always_inline))
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define WE_VERSION	21
#define WE_MAX_VERSION	22
#define WT_VERSION	29

#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_NET_DEV		"/proc/net/dev"

#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9
#define LOG10_MAGIC	1.25892541179

#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801
#endif

#ifndef IW_EV_LCP_PK_LEN
#define IW_EV_LCP_PK_LEN	(4)
#define IW_EV_CHAR_PK_LEN	(IW_EV_LCP_PK_LEN + IFNAMSIZ)
#define IW_EV_UINT_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(__u32))
#define IW_EV_FREQ_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_freq))
#define IW_EV_PARAM_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_param))
#define IW_EV_ADDR_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct sockaddr))
#define IW_EV_QUAL_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_quality))
#define IW_EV_POINT_PK_LEN	(IW_EV_LCP_PK_LEN + 4)
#endif

struct iw_pk_event
{
	__u16		len;
	__u16		cmd;
	union iwreq_data	u;
} __attribute__ ((packed));
struct	iw_pk_point
{
  void __user	*pointer;
  __u16		length;
  __u16		flags;
} __attribute__ ((packed));

#define IW_EV_LCP_PK2_LEN	(sizeof(struct iw_pk_event) - sizeof(union iwreq_data))
#define IW_EV_POINT_PK2_LEN	(IW_EV_LCP_PK2_LEN + sizeof(struct iw_pk_point) - IW_EV_POINT_OFF)


typedef struct iw_statistics	iwstats;
typedef struct iw_range		iwrange;
typedef struct iw_param		iwparam;
typedef struct iw_freq		iwfreq;
typedef struct iw_quality	iwqual;
typedef struct iw_priv_args	iwprivargs;
typedef struct sockaddr		sockaddr;


typedef struct wireless_config
{
  char		name[IFNAMSIZ + 1];
  int		has_nwid;
  iwparam	nwid;
  int		has_freq;
  double	freq;
  int		freq_flags;
  int		has_key;
  unsigned char	key[IW_ENCODING_TOKEN_MAX];
  int		key_size;
  int		key_flags;
  int		has_essid;
  int		essid_on;
  char		essid[IW_ESSID_MAX_SIZE + 1];
  int		has_mode;
  int		mode;
} wireless_config;


typedef struct wireless_info
{
  struct wireless_config	b;

  int		has_sens;
  iwparam	sens;
  int		has_nickname;
  char		nickname[IW_ESSID_MAX_SIZE + 1];
  int		has_ap_addr;
  sockaddr	ap_addr;
  int		has_bitrate;
  iwparam	bitrate;
  int		has_rts;
  iwparam	rts;
  int		has_frag;
  iwparam	frag;
  int		has_power;
  iwparam	power;
  int		has_txpower;
  iwparam	txpower;
  int		has_retry;
  iwparam	retry;

  iwstats	stats;
  int		has_stats;
  iwrange	range;
  int		has_range;

  int		auth_key_mgmt;
  int		has_auth_key_mgmt;
  int		auth_cipher_pairwise;
  int		has_auth_cipher_pairwise;
  int		auth_cipher_group;
  int		has_auth_cipher_group;
} wireless_info;


typedef struct wireless_scan
{
  struct wireless_scan *	next;

  int		has_ap_addr;
  sockaddr	ap_addr;

  struct wireless_config	b;
  iwstats	stats;
  int		has_stats;
  iwparam	maxbitrate;
  int		has_maxbitrate;
} wireless_scan;

typedef struct wireless_scan_head
{
  wireless_scan *	result;
  int			retry;
} wireless_scan_head;

typedef struct stream_descr
{
  char *	end;
  char *	current;
  char *	value;
} stream_descr;


typedef int (*iw_enum_handler)(int	skfd,
			       char *	ifname,
			       char *	args[],
			       int	count);

typedef struct iw_modul_descr
{
  unsigned int		mask;
  char			cmd[8];
  char *		verbose;
} iw_modul_descr;

#define IW_HEADER_TYPE_NULL	0
#define IW_HEADER_TYPE_CHAR	2
#define IW_HEADER_TYPE_UINT	4
#define IW_HEADER_TYPE_FREQ	5
#define IW_HEADER_TYPE_ADDR	6
#define IW_HEADER_TYPE_POINT	8
#define IW_HEADER_TYPE_PARAM	9
#define IW_HEADER_TYPE_QUAL	10

#define IW_DESCR_FLAG_NONE	0x0000
#define IW_DESCR_FLAG_DUMP	0x0001
#define IW_DESCR_FLAG_EVENT	0x0002
#define IW_DESCR_FLAG_RESTRICT	0x0004
#define IW_DESCR_FLAG_NOMAX	0x0008
#define IW_DESCR_FLAG_WAIT	0x0100

struct iw_ioctl_description
{
	__u8	header_type;
	__u8	token_type;
	__u16	token_size;
	__u16	min_tokens;
	__u16	max_tokens;
	__u32	flags;
};

#define SIOCSIWMODUL	0x8B2E
#define SIOCGIWMODUL	0x8B2F
static const struct iw_ioctl_description standard_ioctl_descr[] = {
	[SIOCSIWCOMMIT	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWNAME	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_CHAR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWNWID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWNWID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWFREQ	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWFREQ	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWMODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWMODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWSENS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWSENS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRANGE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWRANGE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_range),
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWPRIV	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWPRIV	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCSIWSTATS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWSTATS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr),
		.max_tokens	= IW_MAX_SPY,
	},
	[SIOCGIWSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_SPY,
	},
	[SIOCSIWTHRSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[SIOCGIWTHRSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[SIOCSIWAP	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[SIOCGIWAP	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWMLME	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_mlme),
		.max_tokens	= sizeof(struct iw_mlme),
	},
	[SIOCGIWAPLIST	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_AP,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWSCAN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= 0,
		.max_tokens	= sizeof(struct iw_scan_req),
	},
	[SIOCGIWSCAN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_SCAN_MAX_DATA,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWESSID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWESSID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWNICKN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
	},
	[SIOCGIWNICKN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
	},
	[SIOCSIWRATE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRATE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRTS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRTS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWFRAG	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWFRAG	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWTXPOW	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWTXPOW	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRETRY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRETRY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWENCODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_EVENT | IW_DESCR_FLAG_RESTRICT,
	},
	[SIOCGIWENCODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_DUMP | IW_DESCR_FLAG_RESTRICT,
	},
	[SIOCSIWPOWER	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWPOWER	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWMODUL	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWMODUL	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWGENIE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[SIOCGIWGENIE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[SIOCSIWAUTH	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWAUTH	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWENCODEEXT - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[SIOCGIWENCODEEXT - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[SIOCSIWPMKSA - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_pmksa),
		.max_tokens	= sizeof(struct iw_pmksa),
	},
};
static const unsigned int standard_ioctl_num = (sizeof(standard_ioctl_descr) /
						sizeof(struct iw_ioctl_description));


static const struct iw_ioctl_description standard_event_descr[] = {
	[IWEVTXDROP	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVQUAL	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_QUAL,
	},
	[IWEVCUSTOM	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_CUSTOM_MAX,
	},
	[IWEVREGISTERED	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVEXPIRED	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR, 
	},
	[IWEVGENIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVMICHAELMICFAILURE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT, 
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_michaelmicfailure),
	},
	[IWEVASSOCREQIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVASSOCRESPIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVPMKIDCAND	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_pmkid_cand),
	},
};
static const unsigned int standard_event_num = (sizeof(standard_event_descr) /
						sizeof(struct iw_ioctl_description));

static const int event_type_size[] = {
	IW_EV_LCP_PK_LEN,
	0,
	IW_EV_CHAR_PK_LEN,
	0,
	IW_EV_UINT_PK_LEN,
	IW_EV_FREQ_PK_LEN,
	IW_EV_ADDR_PK_LEN,
	0,
	IW_EV_POINT_PK_LEN,
	IW_EV_PARAM_PK_LEN,
	IW_EV_QUAL_PK_LEN,
};

#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

struct	iw15_range
{
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

union	iw_range_raw
{
	struct iw15_range	range15;
	struct iw_range		range;
};

#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)


int
	iw_sockets_open(void);
void
	iw_enum_devices(int		skfd,
			iw_enum_handler fn,
			char *		args[],
			int		count);
int
	iw_get_kernel_we_version(void);
int
	iw_print_version_info(const char *	toolname);
int
	iw_get_range_info(int		skfd,
			  const char *	ifname,
			  iwrange *	range);
int
	iw_get_priv_info(int		skfd,
			 const char *	ifname,
			 iwprivargs **	ppriv);
int
	iw_get_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
int
	iw_set_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
int
	iw_protocol_compare(const char *	protocol1,
			    const char *	protocol2);
void
	iw_float2freq(double	in,
		      iwfreq *	out);
double
	iw_freq2float(const iwfreq *	in);
void
	iw_print_freq_value(char *	buffer,
			    int		buflen,
			    double	freq);
void
	iw_print_freq(char *	buffer,
		      int	buflen,
		      double	freq,
		      int	channel,
		      int	freq_flags);
int
	iw_freq_to_channel(double			freq,
			   const struct iw_range *	range);
int
	iw_channel_to_freq(int				channel,
			   double *			pfreq,
			   const struct iw_range *	range);
void
	iw_print_bitrate(char *	buffer,
			 int	buflen,
			 int	bitrate);
int
	iw_dbm2mwatt(int	in);
int
	iw_mwatt2dbm(int	in);
void
	iw_print_txpower(char *			buffer,
			 int			buflen,
			 struct iw_param *	txpower);
int
	iw_get_stats(int		skfd,
		     const char *	ifname,
		     iwstats *		stats,
		     const iwrange *	range,
		     int		has_range);
void
	iw_print_stats(char *		buffer,
		       int		buflen,
		       const iwqual *	qual,
		       const iwrange *	range,
		       int		has_range);
void
	iw_print_key(char *			buffer,
		     int			buflen,
		     const unsigned char *	key,
		     int			key_size,
		     int			key_flags);
int
	iw_in_key(const char *		input,
		  unsigned char *	key);
int
	iw_in_key_full(int		skfd,
		       const char *	ifname,
		       const char *	input,
		       unsigned char *	key,
		       __u16 *		flags);
void
	iw_print_pm_value(char *	buffer,
			  int		buflen,
			  int		value,
			  int		flags,
			  int		we_version);
void
	iw_print_pm_mode(char *		buffer,
			 int		buflen,
			 int		flags);
void
	iw_print_retry_value(char *	buffer,
			     int	buflen,
			     int	value,
			     int	flags,
			     int	we_version);
void
	iw_print_timeval(char *				buffer,
			 int				buflen,
			 const struct timeval *		time,
			 const struct timezone *	tz);
int
	iw_check_mac_addr_type(int		skfd,
			       const char *	ifname);
int
	iw_check_if_addr_type(int		skfd,
			      const char *	ifname);
#if 0
int
	iw_check_addr_type(int		skfd,
			   const char *	ifname);
#endif
#if 0
int
	iw_get_mac_addr(int			skfd,
			const char *		name,
			struct ether_addr *	eth,
			unsigned short *	ptype);
#endif
char *
	iw_mac_ntop(const unsigned char *	mac,
		    int				maclen,
		    char *			buf,
		    int				buflen);
void
	iw_ether_ntop(const struct ether_addr *	eth,
		      char *			buf);
char *
	iw_sawap_ntop(const struct sockaddr *	sap,
		      char *			buf);
int
	iw_mac_aton(const char *	orig,
		    unsigned char *	mac,
		    int			macmax);
int
	iw_ether_aton(const char* bufp, struct ether_addr* eth);
int
	iw_in_inet(char *bufp, struct sockaddr *sap);
int
	iw_in_addr(int			skfd,
		   const char *		ifname,
		   char *		bufp,
		   struct sockaddr *	sap);
int
	iw_get_priv_size(int		args);

void
	iw_init_event_stream(struct stream_descr *	stream,
			     char *			data,
			     int			len);
int
	iw_extract_event_stream(struct stream_descr *	stream,
				struct iw_event *	iwe,
				int			we_version);
int
	iw_process_scan(int			skfd,
			char *			ifname,
			int			we_version,
			wireless_scan_head *	context);
int
	iw_scan(int			skfd,
		char *			ifname,
		int			we_version,
		wireless_scan_head *	context);

extern const char * const	iw_operation_mode[];
#define IW_NUM_OPER_MODE	7
#define IW_NUM_OPER_MODE_EXT	8

extern const struct iw_modul_descr	iw_modul_list[];
#define IW_SIZE_MODUL_LIST	16


static inline int
iw_set_ext(int			skfd,
	   const char *		ifname,
	   int			request,
	   struct iwreq *	pwrq)
{
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  return(ioctl(skfd, request, pwrq));
}

static inline int
my_iw_get_ext(int			skfd,
	   const char *		ifname,
	   int			request,
	   struct iwreq *	pwrq)
{
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  return(ioctl(skfd, request, pwrq));
}

static inline void
iw_sockets_close(int	skfd)
{
  close(skfd);
}

static inline char *
iw_saether_ntop(const struct sockaddr *sap, char* bufp)
{
  iw_ether_ntop((const struct ether_addr *) sap->sa_data, bufp);
  return bufp;
}

static inline int
iw_saether_aton(const char *bufp, struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  return iw_ether_aton(bufp, (struct ether_addr *) sap->sa_data);
}

static inline void
iw_broad_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0xFF, ETH_ALEN);
}

static inline void
iw_null_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0x00, ETH_ALEN);
}

static inline int
iw_ether_cmp(const struct ether_addr* eth1, const struct ether_addr* eth2)
{
  return memcmp(eth1, eth2, sizeof(struct ether_addr));
}

#ifdef __cplusplus
}
#endif

#endif
