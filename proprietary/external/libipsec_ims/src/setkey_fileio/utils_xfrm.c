
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <asm/types.h>
#include <linux/pkt_sched.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "utils_xfrm.h"
#define LOG_TAG "setkey"
#include <log/log.h>




int mask2bits(__u32 netmask_xfrm)
{
	unsigned bits_xfrm = 0;
	__u32 mask_xfrm = ntohl(netmask_xfrm);
	__u32 host_xfrm = ~mask_xfrm;

	/* a valid netmask must be 2^n - 1 */
	if ((host_xfrm & (host_xfrm + 1)) != 0)
		return -1;

	for (; mask_xfrm; mask_xfrm <<= 1)
		++bits_xfrm;
	return bits_xfrm;
}

int get_netmask(unsigned *val_xfrm, const char *arg_xfrm, int base_xfrm)
{
	inet_prefix addr_xfrm;

	if (!get_unsigned(val_xfrm, arg_xfrm, base_xfrm))
		return 0;

	/* try coverting dotted quad to CIDR */
	if (!get_addr_1(&addr_xfrm, arg_xfrm, AF_INET) && addr_xfrm.family == AF_INET) {
		int b_xfrm = mask2bits(addr_xfrm.data[0]);
		
		if (b_xfrm >= 0) {
			*val_xfrm = b_xfrm;
			return 0;
		}
	}

	return -1;
}

int get_unsigned(unsigned *val_xfrm, const char *arg_xfrm, int base_xfrm)
{
	unsigned long res_xfrm = 0;
	char *ptr_xfrm = NULL;

	if (!arg_xfrm || !*arg_xfrm)
		return -1;
	res_xfrm = strtoul(arg_xfrm, &ptr_xfrm, base_xfrm);
	if (!ptr_xfrm || ptr_xfrm == arg_xfrm || *ptr_xfrm || res_xfrm > UINT_MAX)
		return -1;
	*val_xfrm = res_xfrm;
	return 0;
}


int get_u32(__u32 *val_xfrm, const char *arg_xfrm, int base_xfrm)
{
	unsigned long res_xfrm = 0;
	char *ptr_xfrm = NULL;

	if (!arg_xfrm || !*arg_xfrm)
		return -1;
	res_xfrm = strtoul(arg_xfrm, &ptr_xfrm, base_xfrm);
	if (!ptr_xfrm || ptr_xfrm == arg_xfrm || *ptr_xfrm || res_xfrm > 0xFFFFFFFFUL)
		return -1;
	*val_xfrm = res_xfrm;
	return 0;
}


int get_u8(__u8 *val_xfrm, const char *arg_xfrm, int base_xfrm)
{
	unsigned long res_xfrm = 0;
	char *ptr_xfrm = NULL;

	if (!arg_xfrm || !*arg_xfrm)
		return -1;
	res_xfrm = strtoul(arg_xfrm, &ptr_xfrm, base_xfrm);
	if (!ptr_xfrm || ptr_xfrm == arg_xfrm || *ptr_xfrm || res_xfrm > 0xFF)
		return -1;
	*val_xfrm = res_xfrm;
	return 0;
}



/* This uses a non-standard parsing (ie not inet_aton, or inet_pton)
 * because of legacy choice to parse 10.8 as 10.8.0.0 not 10.0.0.8
 */
int get_addr_ipv4(__u8 *ap_xfrm, const char *cp_xfrm)
{
	int i_xfrm = 0;

	for (i_xfrm = 0; i_xfrm < 4; i_xfrm++) {
		unsigned long n_xfrm = 0;
		char *endp_xfrm = NULL;
		
		n_xfrm = strtoul(cp_xfrm, &endp_xfrm, 0);
		if (n_xfrm > 255)
			return -1;	/* bogus network value */

		if (endp_xfrm == cp_xfrm) /* no digits */
			return -1;

		ap_xfrm[i_xfrm] = n_xfrm;

		if (*endp_xfrm == '\0')
			break;

		if (i_xfrm == 3 || *endp_xfrm != '.')
			return -1; 	/* extra characters */
		cp_xfrm = endp_xfrm + 1;
	}

	return 1;
}

int get_addr_1(inet_prefix *addr_xfrm, const char *name_xfrm, int family)
{
	memset(addr_xfrm, 0, sizeof(*addr_xfrm));

	if (strcmp(name_xfrm, "default") == 0 ||
	    strcmp(name_xfrm, "all") == 0 ||
	    strcmp(name_xfrm, "any") == 0) {
		if (family == AF_DECnet)
			return -1;
		addr_xfrm->family = family;
		addr_xfrm->bytelen = (family == AF_INET6 ? 16 : 4);
		addr_xfrm->bitlen = -1;
		return 0;
	}

	if (strchr(name_xfrm, ':')) {
		addr_xfrm->family = AF_INET6;
		if (family != AF_UNSPEC && family != AF_INET6)
			return -1;
		if (inet_pton(AF_INET6, name_xfrm, addr_xfrm->data) <= 0)
			return -1;
		addr_xfrm->bytelen = 16;
		addr_xfrm->bitlen = -1;
		return 0;
	}


	addr_xfrm->family = AF_INET;
	if (family != AF_UNSPEC && family != AF_INET)
		return -1;

	if (get_addr_ipv4((__u8 *)addr_xfrm->data, name_xfrm) <= 0)
		return -1;

	addr_xfrm->bytelen = 4;
	addr_xfrm->bitlen = -1;
	return 0;
}

int get_prefix(inet_prefix *dst_xfrm, char *arg_xfrm, int family)
{
	int err = 0 ;
	unsigned plen  = 0;
	char *slash = NULL;

	memset(dst_xfrm, 0, sizeof(*dst_xfrm));

	if (strcmp(arg_xfrm, "default") == 0 ||
	    strcmp(arg_xfrm, "any") == 0 ||
	    strcmp(arg_xfrm, "all") == 0) {
		if (family == AF_DECnet)
			return -1;
		dst_xfrm->family = family;
		dst_xfrm->bytelen = 0;
		dst_xfrm->bitlen = 0;
		return 0;
	}

	slash = strchr(arg_xfrm, '/');
	if (slash)
		*slash = 0;

	err = get_addr_1(dst_xfrm, arg_xfrm, family);
	if (err == 0) {
		switch(dst_xfrm->family) {
			case AF_INET6:
				dst_xfrm->bitlen = 128;
				break;
			case AF_DECnet:
				dst_xfrm->bitlen = 16;
				break;
			default:
			case AF_INET:
				dst_xfrm->bitlen = 32;
		}
		if (slash) {
			if (get_netmask(&plen, slash+1, 0)
					|| plen > dst_xfrm->bitlen) {
				err = -1;
				goto done;
			}
			dst_xfrm->flags |= PREFIXLEN_SPECIFIED;
			dst_xfrm->bitlen = plen;
		}
	}
done:
	if (slash)
		*slash = '/';
	return err;
}





