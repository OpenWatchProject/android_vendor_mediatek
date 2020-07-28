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
#include "utils_xfrm.h"


/*flush all SA*/
extern int setkey_flushSAD(void);
extern int setkey_flushSAD_xfrm(char * ipsec_type);
/*flush all SP*/
extern int setkey_flushSPD(void);
extern int setkey_flushSPD_xfrm(void);
/*delete one SA entry*/
extern int setkey_deleteSA(char * src,char * dst,char * ipsec_type,char * spi_src);
extern int setkey_deleteSA_xfrm(char * src,char * dst,char * ipsec_type,char * spi_src);

/*delete one SP entry*/
int setkey_deleteSP(char * src,char * dst,enum PROTOCOL_TYPE protocol,char * src_port,char * dst_port,char * direction);
int setkey_deleteSP_xfrm(char * src,char * dst,enum PROTOCOL_TYPE protocol,char * src_port,char * dst_port,char * direction);

/*dump all SA */
extern int dump_setkeySA(void);

/*dump all SP */
extern int dump_setkeySP(void);

/*set one SA*/
/*ipsec_type:ah esp
  mode:transport tunnel
  encrp_algo_src:encryption algorithm,des-cbc,3des-cbc...
  encrp_algo_src:key of encryption algorithm
  intergrity_algo_src:authentication algorithm ,hmac-md5,hmac-sha1       
  intergrity_key_src:key of authentication algorithm
*/
extern int setkey_setSA(char * ip_src,char * ip_dst,char * ipsec_type,char * spi_src,char * mode, char * encrp_algo_src,char * encrp_key_src,char * intergrity_algo_src,char * intergrity_key_src,int u_id);
extern int setkey_setSA_xfrm(int cmd,char * ip_src,char * ip_dst,char * ipsec_type,char * spi_src,char * mode, char * encrp_algo_src,char * encrp_key_src,char * intergrity_algo_src,char * intergrity_key_src,int u_id);

extern int setkey_setSA_update(char * ip_src,char * ip_dst,char * ipsec_type,char * spi_src,char * mode, 
                 char * encrp_algo_src,char * encrp_key_src,char * intergrity_algo_src,char * intergrity_key_src,int u_id);

/*set one SP of one direction just for transport mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst */
extern int setkey_SP(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * ipsec_type,char * mode, char * direction,int u_id);
extern int setkey_SP_xfrm(int cmd,char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type,char * mode, char * direction,int u_id);

/*set one SP of one direction, just for tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
extern int setkey_SP_tunnel(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type,char * mode, char * direction,int u_id);


/*set one SP of one direction, for 2 layers' ipsec--tunnel mode+transport mode or transport mode+tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
extern int setkey_SP_tunnel_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2);

/*update one SP of one direction, just for transport mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst*/
int setkey_SP_update_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * ipsec_type, char * direction,int u_id);


/*update one SP of one direction, for 2 layers' ipsec--tunnel mode+transport mode or transport mode+tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
int setkey_SP_update_tunnel_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2);
int setkey_SP_2layer_xfrm(int cmd,char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2);

/*flush SA\SP from setkey.conf*/
extern int flush_SA_SP_exist();
extern int flush_SA_SP_exist_xfrm();

