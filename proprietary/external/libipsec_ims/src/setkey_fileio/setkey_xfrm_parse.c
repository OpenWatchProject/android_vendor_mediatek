
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <linux/capability.h>
#include <sys/capability.h>
#include <cutils/properties.h>
#include "setkey_fileio.h"
#include "utils_xfrm.h"
#include "setkey_xfrm_parse.h"
#define LOG_TAG "setkey"
#include <log/log.h>



const struct typeent xfrmproto_types[]= {
	{ "esp", IPPROTO_ESP }, { "ah", IPPROTO_AH }, { "comp", IPPROTO_COMP },
	{ "route2", IPPROTO_ROUTING }, { "hao", IPPROTO_DSTOPTS },
	{ "ipsec-any", IPSEC_PROTO_ANY },
	{ NULL, 0 }
};


__u8 xfrm_xfrmproto_getbyname(char *name)
{
	int i;

	for (i = 0; ; i++) {
		const struct typeent *t = &xfrmproto_types[i];
		if (!t->t_name || t->t_type == -1)
			break;

		if (strcmp(t->t_name, name) == 0)
			return t->t_type;
	}

	return 0;
}


int xfrm_id_parse(xfrm_address_t *saddr_xfrm, struct xfrm_id *id, __u16 *family,
		 char * src,char * dst,char * ipsec_type)
{

	inet_prefix dst_prefix;
	inet_prefix src_prefix;

	memset(&dst_prefix, 0, sizeof(dst_prefix));
	memset(&src_prefix, 0, sizeof(src_prefix));

	get_prefix(&src_prefix, src, AF_UNSPEC);
	if (family)
		*family = src_prefix.family;
	memcpy(saddr_xfrm, &src_prefix.data, sizeof(*saddr_xfrm));
	get_prefix(&dst_prefix, dst, AF_UNSPEC);
	memcpy(&id->daddr, &dst_prefix.data, sizeof(id->daddr));

	int ret = xfrm_xfrmproto_getbyname(ipsec_type);
	if(ret<0)
	{
		ALOGD("xfrm_id_parse %s is wrong\n",ipsec_type);
		return -1;
	}
	id->proto = (__u8)ret;


	return 0;
}

void xfrm_encry_algo_parse(char * encry_src, char *name)
{
	if(encry_src == NULL)
		memcpy(name,"ecb(cipher_null)",strlen("ecb(cipher_null)"));
	else 	if(strcmp(encry_src,"des-cbc")==0)
		memcpy(name,"cbc(des)",strlen("cbc(des)"));
	else 	if(strcmp(encry_src,"des-ede3-cbc")==0)
		memcpy(name,"cbc(des3_ede)",strlen("cbc(des3_ede)"));
	else 	if(strcmp(encry_src,"3des-cbc")==0)
		memcpy(name,"cbc(des3_ede)",strlen("cbc(des3_ede)"));
	else 	if(strcmp(encry_src,"cast5-cbc")==0)
		memcpy(name,"cbc(cast5)",strlen("cbc(cast5)"));
	else 	if(strcmp(encry_src,"blowfish-cbc")==0)
		memcpy(name,"cbc(blowfish)",strlen("cbc(blowfish)"));
	else 	if(strcmp(encry_src,"aes-cbc")==0)
		memcpy(name,"cbc(aes)",strlen("cbc(aes)"));
	else 	if(strcmp(encry_src,"serpent-cbc")==0)
		memcpy(name,"cbc(serpent)",strlen("cbc(serpent)"));
	else 	if(strcmp(encry_src,"camellia-cbc")==0)
		memcpy(name,"cbc(camellia)",strlen("cbc(camellia)"));
	else 	if(strcmp(encry_src,"twofish-cbc")==0)
		memcpy(name,"cbc(twofish)",strlen("cbc(twofish)"));
	else 	if(strcmp(encry_src,"aes-ctr-rfc3686")==0)
		memcpy(name,"rfc3686(ctr(aes))",strlen("rfc3686(ctr(aes))"));
	else    if(strcmp(encry_src,"null")==0)
		memcpy(name,"ecb(cipher_null)",strlen("ecb(cipher_null)"));
	else
	{
		memcpy(name,"not-supported",strlen("not-supported"));	
		ALOGD("xfrm_encry_algo_parse not supported algorithm--%s\n",encry_src);
	}
}

void xfrm_interg_algo_parse(char * interg_src, char *name)
{
	if(interg_src == NULL)
		memcpy(name,"digest_null",strlen("digest_null"));
	else 	if(strcmp(interg_src,"hmac-md5")==0)
		memcpy(name,"hmac(md5)",strlen("hmac(md5)"));
	else 	if(strcmp(interg_src,"hmac-sha1")==0)
		memcpy(name,"hmac(sha1)",strlen("hmac(sha1)"));
	else 	if(strcmp(interg_src,"hmac-sha256")==0)
		memcpy(name,"hmac(sha256)",strlen("hmac(sha256)"));
	else 	if(strcmp(interg_src,"hmac-sha384)")==0)
		memcpy(name,"hmac(sha384)",strlen("hmac(sha384)"));	
	else 	if(strcmp(interg_src,"hmac-sha512")==0)
		memcpy(name,"hmac(sha512)",strlen("hmac(sha512)"));
	else 	if(strcmp(interg_src,"hmac-rmd160")==0)
		memcpy(name,"hmac(rmd160)",strlen("hmac(rmd160)"));
	else 	if(strcmp(interg_src,"aes-xcbc")==0)
		memcpy(name,"xcbc(aes)",strlen("xcbc(aes)"));
	else 	if(strcmp(interg_src,"cmac(aes)")==0)
		memcpy(name,"aes-cmac",strlen("aes-cmac"));
	else 	if(strcmp(interg_src,"null")==0)
		memcpy(name,"digest_null",strlen("digest_null"));
	else
	{	
		memcpy(name,"not-supported",strlen("not-supported"));
		ALOGD("xfrm_interg_algo_parse not supported algorithm--%s\n",interg_src);
	}
}


int xfrm_algo_parse(struct xfrm_algo *alg, char *name, char *key, char *buf, int max)
{
	int len;
	int slen = strlen(key);

	strncpy(alg->alg_name, name, sizeof(alg->alg_name));

	if (slen > 2 && strncmp(key, "0x", 2) == 0) {
		/* split two chars "0x" from the top */
		char *p = key + 2;
		int plen = slen - 2;
		int i;
		int j;

		/* Converting hexadecimal numbered string into real key;
		 * Convert each two chars into one char(value). If number
		 * of the length is odd, add zero on the top for rounding.
		 */

		/* calculate length of the converted values(real key) */
		len = (plen + 1) / 2;
		if (len > max)
		{
			ALOGD("xfrm_algo_parse key(len:%d) makes buffer overflow\n",len);	
			return -1;
		}

		for (i = - (plen % 2), j = 0; j < len; i += 2, j++) {
			char vbuf[3];
			__u8 val;

			vbuf[0] = i >= 0 ? p[i] : '0';
			vbuf[1] = p[i + 1];
			vbuf[2] = '\0';

			if (get_u8(&val, vbuf, 16))
			{
				ALOGD("xfrm_algo_parse key(len:%s) is invalid\n",key);	
				return -1;
			}
			buf[j] = val;
		}
	} else {
		len = slen;
		if (len > 0) {
			if (len > max)
			{
				ALOGD("xfrm_algo_parse key(len:%d) makes buffer overflow\n",len);	
				return -1;
			}

			strncpy(buf, key, len);
		}
	}

	alg->alg_key_len = len * 8;

	return 0;
}

__u8 xfrm_dir_parse(char * dir_str)
{
	__u8 dir;
	if(strcmp(dir_str,"out")==0)
		dir = XFRM_POLICY_OUT;
	else if(strcmp(dir_str,"in")==0)
		dir = XFRM_POLICY_IN;
	else if(strcmp(dir_str,"fwd")==0)
		dir = XFRM_POLICY_FWD;
	else
		dir = XFRM_POLICY_ERROR;
	return dir;
}

int xfrm_mode_parse(__u8 *mode, char * mode_str)
{


	if (strcmp(mode_str, "transport") == 0)
		*mode = XFRM_MODE_TRANSPORT;
	else if (strcmp(mode_str, "tunnel") == 0)
		*mode = XFRM_MODE_TUNNEL;
	else if (strcmp(mode_str, "ro") == 0)
		*mode = XFRM_MODE_ROUTEOPTIMIZATION;
	else if (strcmp(mode_str, "in_trigger") == 0)
		*mode = XFRM_MODE_IN_TRIGGER;
	else if (strcmp(mode_str, "beet") == 0)
		*mode = XFRM_MODE_BEET;
	else
		return -1;


	return 0;
}

void xfrm_selector_parse(struct xfrm_selector *sel, char * src,char * dst,enum PROTOCOL_TYPE protocol,char * src_port,char * dst_port)
{

	inet_prefix dst_prefix;
	inet_prefix src_prefix;

	memset(&dst_prefix, 0, sizeof(dst_prefix));
	memset(&src_prefix, 0, sizeof(src_prefix));


	get_prefix(&src_prefix, src, AF_UNSPEC);
	memcpy(&sel->saddr, &src_prefix.data, sizeof(sel->saddr));
	sel->prefixlen_s = src_prefix.bitlen;

	get_prefix(&dst_prefix, dst, AF_UNSPEC);
	memcpy(&sel->daddr, &dst_prefix.data, sizeof(sel->daddr));
	sel->prefixlen_d = dst_prefix.bitlen;

	sel->family = dst_prefix.family;

	sel->sport = htons(atoi(src_port));
	sel->dport = htons(atoi(dst_port));
	sel->dport_mask = ~((__u16)0);
	sel->sport_mask = ~((__u16)0);

	sel->user = getpid();

	sel->proto = protocol;
#ifdef INIT_ENG_BUILD
	if(sel->family == AF_INET)
	ALOGD("xfrm_selector_parse family:%u,prefix_d:%u,prefix_s:%u,daddr:0x%x,saddr:0x%x,sel->sport:%d,sel->dport:%d,proto:%u,user:%u\n",sel->family,sel->prefixlen_d,sel->prefixlen_s,sel->daddr.a4,sel->daddr.a4,sel->sport,sel->dport,sel->proto,sel->user);
	else
	ALOGD("xfrm_selector_parse family:%u,prefix_d:%u,prefix_s:%u,daddr:0x%x %x %x %x,saddr:0x%x %x %x %x ,sel->sport:%d,sel->dport:%d,proto:%u,user:%u\n",sel->family,sel->prefixlen_d,sel->prefixlen_s,sel->daddr.a6[0],sel->daddr.a6[1],sel->daddr.a6[2],sel->daddr.a6[3],sel->saddr.a6[0],sel->saddr.a6[1],sel->saddr.a6[2],sel->saddr.a6[3],sel->sport,sel->dport,sel->proto,sel->user);
#endif
}



