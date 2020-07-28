
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
#if 0
#include "../setkey/log_setky.h"
#endif
#define LOG_TAG "setkey"
#include <log/log.h>


#if 0
extern void plog_android(int level, char *format, ...);
extern int setkey_main(int argc, char ** argv);

#define POLICY_LEN 640
#define POLICY_MODE 320
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define BUFF_SIZE 128
#define RM_FILE_LEN 64

char setkey_conf[]="/data/misc/setkey/setkey.conf";
char setkey_conf_bak[]="/data/misc/setkey/setkey_bak.conf";
char setkey_conf_latest[]="/data/misc/setkey/setkey_latest.conf";


static int RemoveString(char * src,char * dst,char * ipsec_type,char * spi_src)
{
	FILE *fpSrc = NULL;
	FILE *fpDst = NULL;

	char achBuf[POLICY_LEN] = {0};
	fpSrc = fopen(setkey_conf, "rt");
	if (NULL == fpSrc)
	{
		plog_android(LLV_WARNING, "RemoveString:can't open %s,errno:%d",setkey_conf,errno);
		return -1;
	}
	chown(setkey_conf,0,1000);
	fpDst = fopen(setkey_conf_bak, "wt");
	if (NULL== fpDst)
	{
		plog_android(LLV_WARNING,"RemoveString:Create source file: %s failed,errno:%d\n", setkey_conf_bak,errno);
		fclose(fpSrc);
		return -1;
	}
	chown(setkey_conf_bak,0,1000);
	while (!feof(fpSrc))
	{
		memset(achBuf, 0, sizeof(achBuf));
		fgets(achBuf, sizeof(achBuf), fpSrc);
	/* include below parameter is right*/
		if ((strstr(achBuf, "add")!=NULL)&&(strstr(achBuf, "spdadd")==NULL)&&(strstr(achBuf, dst)!=NULL)&&(strstr(achBuf,ipsec_type)!=NULL)&&(strstr(achBuf, spi_src)!=NULL))
		{
			/*to make sure sequence,first src,then dst*/
			if(strstr(achBuf, src)<strstr(achBuf, dst))
				plog_android(LLV_WARNING,"Has found SA,%s,remove it\n",achBuf);
			else
				fputs(achBuf, fpDst);
		} else {
			fputs(achBuf, fpDst);
		}         
    
	}     
	fclose(fpSrc);
	fclose(fpDst);

	return 0; 
}
 

static int RemoveString_SP(char * src,char * dst,int protocol,char * src_port,char * dst_port,char * direction) 
{     
	FILE *fpSrc = NULL;     
	FILE *fpDst = NULL;    
  
        char protocol_str[16]={0};
	char achBuf[POLICY_LEN] = {0}; 
        sprintf(protocol_str,"%d",protocol);

	fpSrc = fopen(setkey_conf, "rt");     
	if (NULL == fpSrc)     
	{         
		plog_android(LLV_WARNING, "RemoveString_SP:can't open %s,errno:%d",setkey_conf,errno);       
		return -1;     
	} 
	chown(setkey_conf,0,1000);      
	fpDst = fopen(setkey_conf_bak, "wt");     
	if (NULL== fpDst)     
	{         
		plog_android(LLV_WARNING,"RemoveString_SP:Create source file: %s failed,errno:%d\n", setkey_conf_bak,errno);         
		fclose(fpSrc);         
		return -1;     
	}       
	chown(setkey_conf_bak,0,1000);        
	while (!feof(fpSrc))     
	{         
		memset(achBuf, 0, sizeof(achBuf));         
		fgets(achBuf, sizeof(achBuf), fpSrc);         
		if ((strstr(achBuf, "spdadd")!=NULL)&&(strstr(achBuf, dst)!=NULL)&&(strstr(achBuf, src)!=NULL)&&(strstr(achBuf, src_port)!=NULL)
                   &&(strstr(achBuf, dst_port)!=NULL)&&(strstr(achBuf, protocol_str)!=NULL)&&(strstr(achBuf, direction)!=NULL))         
		{                     
            if((strstr(achBuf, src)<strstr(achBuf, dst))&&(strstr(achBuf, src_port)<strstr(achBuf, dst_port)))          
		        plog_android(LLV_WARNING,"Has found SP policy,%s,remove it\n",achBuf);  
            else
                fputs(achBuf, fpDst); 
		} 
		else
		{
			fputs(achBuf, fpDst); 			
		}            
	}     
	fclose(fpSrc);     
	fclose(fpDst);     
    
	return 0; 
} 


/* 1: some error; 0: process successfully */
int shell(char * cmd)
{
	FILE * fp = NULL,* fp_exit_code =NULL;
	int    bufflen;
	char * buffer = (char *)malloc((BUFF_SIZE));
	if(buffer == NULL)
    {
       plog_android(LLV_WARNING, "run shell command buffer is null");
       return -1;
    }
    char * buffer_retcode =  NULL;
    char * cmd_exit_code = (char *)malloc((BUFF_SIZE));
    int    ret_code = 0;
	if(cmd_exit_code == NULL)
    {
       plog_android(LLV_WARNING, "alloc cmd_exit_code failed ");
       goto ret_sec;
    }
	if(cmd == NULL)
    {
       plog_android(LLV_WARNING, "run shell command is null");
       goto ret_fir;
    }
	buffer[0] = 0;
    strcpy(cmd_exit_code,cmd);
    strcat(cmd_exit_code,";echo ret_code:$?");
	fp = popen(cmd_exit_code,"r");
	if(fp == NULL)
    {
        plog_android(LLV_WARNING, "can't run shell command");
        goto ret_fir;
    }
    //plog_android(LLV_WARNING, "run shell command:%s ...",cmd);
    while(fgets(buffer,BUFF_SIZE,fp)!=NULL)
    {
        plog_android(LLV_WARNING, "%s",buffer);
    }
    buffer_retcode = strstr(buffer,"ret_code:");
    if(buffer_retcode)
    {
        ret_code = atoi(buffer_retcode+strlen("ret_code:"));
        plog_android(LLV_WARNING, "processing cmd:%s return code:%d",cmd,ret_code);
    }
    pclose(fp);


ret_fir:
    if(cmd_exit_code)
        free(cmd_exit_code);
ret_sec:
    if(buffer)
        free(buffer);
    return ret_code;
}

/*rm setkey.conf file*/
int shell_rm(void)
{
    char rm_file[RM_FILE_LEN];
    int ret = 1;
    memset(rm_file,0,RM_FILE_LEN);
    snprintf(rm_file,sizeof(rm_file),"rm %s",setkey_conf);
    if(access(setkey_conf,0) == 0)
    {
        ret = shell(rm_file);
        if(ret != 0)
        {
            plog_android(LLV_WARNING,"shell %s failed,errno:%d",rm_file,errno);
	    return -1;
        } 
    }
    return 0;

}

/*setkey -f setkey.conf */
int function_setkey(char * file_conf)
{
    char * argv[4];

    argv[0] = "setkey";
    argv[1] = "-f";
    argv[2] = file_conf;
    argv[3] = NULL;
    int ret = setkey_main(3,(char **)argv);
    if(ret != 0)
    {
        plog_android(LLV_WARNING," setkey -f  %s failed,errno:%d",file_conf,errno);
        return ret;
    }
    return 0;

}
#endif
/*flush all SA*/
int setkey_flushSAD(void)
{
#if 0
    char * argv[3];

    argv[0] = "setkey";
    argv[1] = "-F";
    argv[2] = NULL;
    if(shell_rm() == -1)
    {
	return -1;
    }  
    
    int ret = setkey_main(2,(char **)argv);
    if(ret != 0)
    {
        plog_android(LLV_WARNING,"setkey -F failed,errno:%d",errno);
	return ret;
    }  
    return 0;
#endif
    int ret = setkey_flushSAD_xfrm("esp");
    return ret;
}

/*flush all SP*/
int setkey_flushSPD(void)
{
#if 0
    char * argv[4];

    argv[0] = "setkey";
    argv[1] = "-F";
    argv[2] = "-P";
    argv[3] = NULL;
    if(shell_rm() == -1)
    {
	return -1;
    }  
    int ret = setkey_main(3,(char **)argv);
    if(ret != 0)
    {
        plog_android(LLV_WARNING,"setkey -FP failed,errno:%d",errno);
	return ret;
    }  
    return 0;
#endif
    int ret = setkey_flushSPD_xfrm();
    return ret;
}

/*delete one SA entry*/
int setkey_deleteSA(char * src,char * dst,char * ipsec_type,char * spi_src)
{
#if 0
    char delSA[POLICY_LEN];
    FILE * fd_config = NULL;

   
	
    memset(delSA,0,sizeof(delSA));

    snprintf(delSA,sizeof(delSA),"delete %s %s %s %s;\n",src,dst,ipsec_type,spi_src);
    fd_config = fopen(setkey_conf_latest, "w+");
    if(fd_config == NULL)
    {
	    plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	    return -1;
		
    }
    chown(setkey_conf_latest,0,1000);
    fwrite(  delSA, strlen(delSA),1,fd_config);
    plog_android(LLV_WARNING,"setkey_deleteSA:%s",delSA);
    fclose(fd_config);
    if(function_setkey(setkey_conf_latest)== -1)
        return -1;
    else
    {
    	/*update setkey.conf to record how many pairs of SA have been established*/

        if(RemoveString(src,dst,ipsec_type,spi_src)  == 0)
        {
    	    if(shell_rm() == -1)
            {
	            return -1;
            } 
            if(rename(setkey_conf_bak,setkey_conf)<0)
            {
                plog_android(LLV_WARNING,"rename setkey.conf failed,errno:%d",errno);        	
        	    return -1;
            }	
        }
        else
        {
            plog_android(LLV_WARNING,"RemoveString failed");        	
            return -1;
        }
    }
    return 0;	
#endif
    int ret = setkey_deleteSA_xfrm(src,dst,ipsec_type,spi_src);
    return ret;
}

/*delete one SP entry*/
int setkey_deleteSP(char * src,char * dst,enum PROTOCOL_TYPE protocol,char * src_port,char * dst_port,char * direction)
{
#if 0
    char delSP[POLICY_LEN];

    FILE * fd_config = NULL;

    
  
    memset(delSP,0,sizeof(delSP));

    snprintf(delSP,sizeof(delSP),"spddelete %s[%s] %s[%s] %d -P %s;\n",src,src_port,dst,dst_port,protocol,direction);
    fd_config = fopen(setkey_conf_latest, "w+");
    if(fd_config == NULL)
    {
	    plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	    return -1;
		
    }
    chown(setkey_conf_latest,0,1000);
    fwrite(  delSP, strlen(delSP),1,fd_config);
    plog_android(LLV_WARNING,"setkey_deleteSP:%s",delSP);
    fclose(fd_config);
    if(function_setkey(setkey_conf_latest)==-1)
    	return -1;
    else
    {
    	/*update setkey.conf to record how many pairs of SP policy have been established*/

        if(RemoveString_SP( src, dst, protocol, src_port, dst_port, direction) == 0)
        {
    	    if(shell_rm() == -1)
            {
	            return -1;
            } 
            if(rename(setkey_conf_bak,setkey_conf)<0)
            {
                plog_android(LLV_WARNING,"rename setkey.conf failed,errno:%d",errno);        	
        	    return -1;
            }	
        }  
        else
        {
            plog_android(LLV_WARNING,"RemoveString---%s failed",delSP);        	
            return -1;
        }  	
    }
    return 0;
#endif
	int ret = setkey_deleteSP_xfrm(src,dst,protocol,src_port,dst_port,direction);  
	return ret;		
}

/*dump SA */
int dump_setkeySA(void)
{
/*To do*/
#if 0
    char * argv[3];

    argv[0] = "setkey";
    argv[1] = "-D";
    argv[2] = NULL;
    int ret = setkey_main(2,(char **)argv);
    if(ret != 0)
    {
        plog_android(LLV_WARNING," setkey -D failed,errno:%d",errno);
        return -1;
    } 
#endif 
    return 0;

}

/*dump SP */
int dump_setkeySP(void)
{
/*To do*/
#if 0 
   char * argv[4];

    argv[0] = "setkey";
    argv[1] = "-D";
    argv[2] = "-P";
    argv[3] = NULL;
    int ret = setkey_main(3,(char **)argv);
    if(ret != 0)
    {
        plog_android(LLV_WARNING,"setkey -DP failed,errno:%d",errno);
        return ret;
    } 
#endif 
    return 0;

}
#if 0
void setkey_get_aid_and_cap() {

        plog_android(LLV_WARNING,"Warning: gid:%d,uid:%d,pid:%d !\n",getgid(),getuid(),getpid());
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap;
	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = getpid();
	capget(&header, &cap);
        plog_android(LLV_WARNING, "Warning: permitted:%x,cap.effective:%x !\n",cap.permitted,cap.effective);
}
#endif
/*set one SA*/
/*ipsec_type:ah esp
  mode:transport tunnel
  encrp_algo_src:encryption algorithm,des-cbc,3des-cbc...
  encrp_algo_src:key of encryption algorithm
  intergrity_algo_src:authentication algorithm ,hmac-md5,hmac-sha1       
  intergrity_key_src:key of authentication algorithm
*/
int setkey_setSA(char * ip_src,char * ip_dst,char * ipsec_type,char * spi_src,char * mode, 
                 char * encrp_algo_src,char * encrp_key_src,char * intergrity_algo_src,char * intergrity_key_src,int u_id)
{
#if 0
    char sad_policy[POLICY_LEN];

    FILE * fd_config = NULL;
    FILE * fd_config_tmp = NULL;

    memset(sad_policy,0,sizeof(sad_policy));

    setkey_get_aid_and_cap();

    fd_config_tmp = fopen(setkey_conf_latest, "w+" );
    if(fd_config_tmp == NULL)
    {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf_latest,errno);
	      return -1;	
    }
    chown(setkey_conf_latest,0,1000);
   if(strcmp(encrp_algo_src,"null")==0)
    {
        if(u_id ==0 )
            snprintf(sad_policy,sizeof(sad_policy),"add %s %s %s %s  -m %s -E null  -A %s %s;\n",ip_src,ip_dst,ipsec_type,spi_src,mode,
                       intergrity_algo_src,intergrity_key_src);
        else
            snprintf(sad_policy,sizeof(sad_policy),"add %s %s %s %s  -u %d -m %s -E null  -A %s %s;\n",ip_src,ip_dst,ipsec_type,spi_src,u_id,mode,
                       intergrity_algo_src,intergrity_key_src);
    }
    else
    {
        if(u_id ==0 )        
            snprintf(sad_policy,sizeof(sad_policy),"add %s %s %s %s  -m %s -E %s %s  -A %s %s;\n",ip_src,ip_dst,ipsec_type,spi_src,mode,
                       encrp_algo_src,encrp_key_src,intergrity_algo_src,intergrity_key_src);
        else
            snprintf(sad_policy,sizeof(sad_policy),"add %s %s %s %s -u %d -m %s -E %s %s  -A %s %s;\n",ip_src,ip_dst,ipsec_type,spi_src,u_id,mode,
                       encrp_algo_src,encrp_key_src,intergrity_algo_src,intergrity_key_src);
    }
    fwrite( sad_policy, strlen(sad_policy),1,fd_config_tmp );
    plog_android(LLV_WARNING,"setkey_SA:%s",sad_policy);
    fclose(fd_config_tmp);

    if(function_setkey(setkey_conf_latest)==0)
    {
        fd_config = fopen(setkey_conf, "a+" );
        if(fd_config == NULL)
        {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	      return -1;	
        }
	chown(setkey_conf,0,1000);
        fwrite( sad_policy, strlen(sad_policy),1,fd_config );
        fclose(fd_config);   
    }
    else
        return -1;

    return 0;
#endif
	int ret = setkey_setSA_xfrm(XFRM_MSG_NEWSA,ip_src,ip_dst,ipsec_type,spi_src,mode, 
                 encrp_algo_src,encrp_key_src,intergrity_algo_src,intergrity_key_src,u_id);
	return ret;
}
int setkey_setSA_update(char * ip_src,char * ip_dst,char * ipsec_type,char * spi_src,char * mode, 
                 char * encrp_algo_src,char * encrp_key_src,char * intergrity_algo_src,char * intergrity_key_src,int u_id)
{
	int ret = setkey_setSA_xfrm(XFRM_MSG_UPDSA,ip_src,ip_dst,ipsec_type,spi_src,mode, 
                 encrp_algo_src,encrp_key_src,intergrity_algo_src,intergrity_key_src,u_id);
	return ret;
}
/*set one SP of one direction, just for transport mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst */
int setkey_SP(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * ipsec_type,char * mode, char * direction,int u_id)
{
#if 0
    char spd_policy[POLICY_LEN];
    FILE * fd_config = NULL;
    FILE * fd_config_tmp = NULL;
    memset(spd_policy,0,sizeof(spd_policy));


    fd_config_tmp = fopen(setkey_conf_latest, "w+" );
    if(fd_config_tmp == NULL)
    {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf_latest,errno);
	      return -1;	
    }
    chown(setkey_conf_latest,0,1000);
    if(u_id ==0)
        snprintf(spd_policy,sizeof(spd_policy),"spdadd %s[%s] %s[%s] %d -P %s ipsec %s/%s//require;\n",src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type,mode);
    else
        snprintf(spd_policy,sizeof(spd_policy),"spdadd %s[%s] %s[%s] %d -P %s ipsec %s/%s//unique:%d;\n",src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type,mode,u_id);
    fwrite( spd_policy, strlen(spd_policy),1,fd_config_tmp );
    plog_android(LLV_WARNING,"setkey_SP:%s",spd_policy);
    fclose(fd_config_tmp);

    if(function_setkey(setkey_conf_latest) == 0)
    {
        fd_config = fopen(setkey_conf, "a+" );
        if(fd_config == NULL)
        {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	      return -1;	
        }
	chown(setkey_conf,0,1000);
        fwrite( spd_policy, strlen(spd_policy),1,fd_config );
        fclose(fd_config);   
    }
    else
        return -1;
    return 0;
#endif
	mode = "transport";
	int ret = setkey_SP_xfrm(XFRM_MSG_NEWPOLICY,src_range,dst_range,protocol,port_src,port_dst,NULL,NULL,ipsec_type,mode,direction,u_id);
	return ret;
}

/*set one SP of one direction, just for tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
int setkey_SP_tunnel(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type,char * mode, char * direction,int u_id)
{
#if 0
    char spd_policy[POLICY_LEN];
    FILE * fd_config = NULL;
    FILE * fd_config_tmp = NULL;
    memset(spd_policy,0,sizeof(spd_policy));


    fd_config_tmp = fopen(setkey_conf_latest, "w+" );
    if(fd_config_tmp == NULL)
    {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf_latest,errno);
	      return -1;	
    }
    chown(setkey_conf_latest,0,1000);
    if(u_id ==0)
        snprintf(spd_policy,sizeof(spd_policy),"spdadd %s[%s] %s[%s] %d -P %s ipsec %s/%s/%s-%s/require;\n",src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type,mode,src_tunnel, dst_tunnel);
    else
        snprintf(spd_policy,sizeof(spd_policy),"spdadd %s[%s] %s[%s] %d -P %s ipsec %s/%s/%s-%s/unique:%d;\n",src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type,mode,src_tunnel, dst_tunnel,u_id);
    fwrite( spd_policy, strlen(spd_policy),1,fd_config_tmp );
    plog_android(LLV_WARNING,"setkey_SP_tunnel:%s",spd_policy);
    fclose(fd_config_tmp);

    if(function_setkey(setkey_conf_latest) == 0)
    {
        fd_config = fopen(setkey_conf, "a+" );
        if(fd_config == NULL)
        {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	      return -1;	
        }
	chown(setkey_conf,0,1000);
        fwrite( spd_policy, strlen(spd_policy),1,fd_config );
        fclose(fd_config);   
    }
    else
        return -1;
    return 0;
#endif
	mode = "tunnel";
	int ret = setkey_SP_xfrm(XFRM_MSG_NEWPOLICY,src_range,dst_range,protocol,port_src,port_dst,src_tunnel,dst_tunnel,ipsec_type,mode,direction,u_id);
	return ret;
}

/*set one SP of one direction, for 2 layers' ipsec--tunnel mode+transport mode or transport mode+tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
int setkey_SP_tunnel_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2)
{

    char version[128] = {0};
    int ret_mapping = 0;
    property_get("vendor.net.ims.ipsec.version",version,"");
    //plog_android(LLV_WARNING,"getproperty-- vendor.net.ims.ipsec.version :%s\n",version); 
    if(strcmp(version,"2.0")==0)
    {
	ret_mapping = setkey_SP(src_range,dst_range,protocol,port_src,port_dst,ipsec_type1,mode1, direction,u_id1);
	return ret_mapping; 
    }

#if 0
    char spd_policy[POLICY_LEN]={0};
    char * spd_policy_mode1= (char *)malloc(POLICY_MODE);

    FILE * fd_config = NULL;
    FILE * fd_config_tmp = NULL;

    
    if(spd_policy_mode1==NULL)
    {
	      plog_android(LLV_WARNING,"malloc spd_policy_mode1 failed,errno:%d",errno);
	      return -1;	
    }
    memset(spd_policy_mode1,0,POLICY_MODE);

    char * spd_policy_mode2= (char *)malloc(POLICY_MODE);
    if(spd_policy_mode2==NULL)
    {
	      plog_android(LLV_WARNING,"malloc spd_policy_mode2 failed,errno:%d",errno);
	      if(spd_policy_mode1)
			free(spd_policy_mode1);
	      return -1;	
    }
    memset(spd_policy_mode2,0,POLICY_MODE);

    fd_config_tmp = fopen(setkey_conf_latest, "w+" );
    if(fd_config_tmp == NULL)
    {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf_latest,errno);
	      if(spd_policy_mode1)
			free(spd_policy_mode1);
	      if(spd_policy_mode2)
			free(spd_policy_mode2);
	      return -1;	
    }
    chown(setkey_conf_latest,0,1000);
    if(u_id1 ==0)
        if(strcmp(mode1,"transport")==0)
        	snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s//require",ipsec_type1,mode1);
	else
		snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s/%s-%s/require",ipsec_type1,mode1,src_tunnel,dst_tunnel);
    else
        if(strcmp(mode1,"transport")==0)
        	snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s//unique:%d",ipsec_type1,mode1,u_id1);
	else
		snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s/%s-%s/unique:%d",ipsec_type1,mode1,src_tunnel,dst_tunnel,u_id1);
    if(u_id2 ==0)
        if(strcmp(mode2,"transport")==0)
        	snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s//require;\n",ipsec_type2,mode2);
	else
		snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s/%s-%s/require;\n",ipsec_type2,mode2,src_tunnel,dst_tunnel);
    else
        if(strcmp(mode2,"transport")==0)
        	snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s//unique:%d;\n",ipsec_type2,mode2,u_id2);
	else
		snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s/%s-%s/unique:%d;\n",ipsec_type2,mode2,src_tunnel,dst_tunnel,u_id2);

    snprintf(spd_policy,sizeof(spd_policy),"spdadd %s[%s] %s[%s] %d -P %s prio 2147482648 ipsec %s %s",src_range,port_src,dst_range,port_dst,protocol,direction,spd_policy_mode1,spd_policy_mode2);
    fwrite( spd_policy, strlen(spd_policy),1,fd_config_tmp );
    plog_android(LLV_WARNING,"setkey_SP_tunnel_transport:%s",spd_policy);
    fclose(fd_config_tmp);
    if(spd_policy_mode1)
	free(spd_policy_mode1);
    if(spd_policy_mode2)
	free(spd_policy_mode2);
    if(function_setkey(setkey_conf_latest) == 0)
    {
        fd_config = fopen(setkey_conf, "a+" );
        if(fd_config == NULL)
        {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	      return -1;	
        }
	chown(setkey_conf,0,1000);
        fwrite( spd_policy, strlen(spd_policy),1,fd_config );
        fclose(fd_config);   
    }
    else
        return -1;
    return 0;
#endif
	int ret = setkey_SP_2layer_xfrm(XFRM_MSG_UPDPOLICY,src_range,dst_range,protocol,port_src,port_dst,src_tunnel,dst_tunnel,ipsec_type1,mode1,ipsec_type2,mode2,direction,u_id1,u_id2);
	return ret;
}


/*update one SP of one direction, just for transport mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst*/
int setkey_SP_update_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * ipsec_type, char * direction,int u_id)
{
#if 0
    char spd_policy[POLICY_LEN];
    FILE * fd_config = NULL;
    FILE * fd_config_tmp = NULL;
    memset(spd_policy,0,sizeof(spd_policy));


    fd_config_tmp = fopen(setkey_conf_latest, "w+" );
    if(fd_config_tmp == NULL)
    {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf_latest,errno);
	      return -1;	
    }
    chown(setkey_conf_latest,0,1000);
    if(u_id ==0)
        snprintf(spd_policy,sizeof(spd_policy),"spdupdate %s[%s] %s[%s] %d -P %s ipsec %s/transport//require;\n",src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type);
    else
        snprintf(spd_policy,sizeof(spd_policy),"spdupdate %s[%s] %s[%s] %d -P %s ipsec %s/transport//unique:%d;\n",src_range,port_src,dst_range,port_dst,protocol,direction,ipsec_type,u_id);
    fwrite( spd_policy, strlen(spd_policy),1,fd_config_tmp );
    plog_android(LLV_WARNING,"setkey_SP_update_transport:%s",spd_policy);
    fclose(fd_config_tmp);

    if(function_setkey(setkey_conf_latest) == 0)
    {
        fd_config = fopen(setkey_conf, "a+" );
        if(fd_config == NULL)
        {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	      return -1;	
        }
	chown(setkey_conf,0,1000);
        fwrite( spd_policy, strlen(spd_policy),1,fd_config );
        fclose(fd_config);   
    }
    else
        return -1;
    return 0;
#endif
	int ret = setkey_SP_xfrm(XFRM_MSG_UPDPOLICY,src_range,dst_range,protocol,port_src,port_dst,NULL,NULL,ipsec_type,"transport",direction,u_id);
	return ret;
}

/*update one SP of one direction, for 2 layers' ipsec--tunnel mode+transport mode or transport mode+tunnel mode*/
/*protocol:tcp icmp udp icmp6 ip4 gre
  direction:src->dst
src_tunnel,dst_tunnel: tunnel src ip tunnel dst ip */
int setkey_SP_update_tunnel_transport(char * src_range,char * dst_range,enum PROTOCOL_TYPE protocol,char * port_src,char * port_dst,char * src_tunnel,char * dst_tunnel,char * ipsec_type1,char * mode1, char * ipsec_type2,char * mode2,char * direction,int u_id1,int u_id2)
{
#if 0
    char spd_policy[POLICY_LEN]={0};
    char * spd_policy_mode1= (char *)malloc(POLICY_MODE);
    FILE * fd_config = NULL;
    FILE * fd_config_tmp = NULL;

    
    if(spd_policy_mode1==NULL)
    {
	      plog_android(LLV_WARNING,"malloc spd_policy_mode1 failed,errno:%d",errno);
	      return -1;	
    }
    memset(spd_policy_mode1,0,POLICY_MODE);
    char * spd_policy_mode2= (char *)malloc(POLICY_MODE);
    if(spd_policy_mode2==NULL)
    {
	      plog_android(LLV_WARNING,"malloc spd_policy_mode2 failed,errno:%d",errno);
	      if(spd_policy_mode1)
					free(spd_policy_mode1);
	      return -1;	
    }
    memset(spd_policy_mode2,0,POLICY_MODE);

    fd_config_tmp = fopen(setkey_conf_latest, "w+" );
    if(fd_config_tmp == NULL)
    {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf_latest,errno);
	      if(spd_policy_mode1)
			free(spd_policy_mode1);
	      if(spd_policy_mode2)
			free(spd_policy_mode2);
	      return -1;	
    }
    chown(setkey_conf_latest,0,1000);
    if(u_id1 ==0)
        if(strcmp(mode1,"transport")==0)
        	snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s//require",ipsec_type1,mode1);
	else
		snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s/%s-%s/require",ipsec_type1,mode1,src_tunnel,dst_tunnel);
    else
        if(strcmp(mode1,"transport")==0)
        	snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s//unique:%d",ipsec_type1,mode1,u_id1);
	else
		snprintf(spd_policy_mode1,POLICY_MODE,"%s/%s/%s-%s/unique:%d",ipsec_type1,mode1,src_tunnel,dst_tunnel,u_id1);
    if(u_id2 ==0)
        if(strcmp(mode2,"transport")==0)
        	snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s//require;\n",ipsec_type2,mode2);
	else
		snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s/%s-%s/require;\n",ipsec_type2,mode2,src_tunnel,dst_tunnel);
    else
        if(strcmp(mode2,"transport")==0)
        	snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s//unique:%d;\n",ipsec_type2,mode2,u_id2);
	else
		snprintf(spd_policy_mode2,POLICY_MODE,"%s/%s/%s-%s/unique:%d;\n",ipsec_type2,mode2,src_tunnel,dst_tunnel,u_id2);

    snprintf(spd_policy,sizeof(spd_policy),"spdupdate %s[%s] %s[%s] %d -P %s prio 2147482648 ipsec %s %s",src_range,port_src,dst_range,port_dst,protocol,direction,spd_policy_mode1,spd_policy_mode2);
    fwrite( spd_policy, strlen(spd_policy),1,fd_config_tmp );
    plog_android(LLV_WARNING,"setkey_SP_update_tunnel_transport:%s",spd_policy);
    fclose(fd_config_tmp);
    if(spd_policy_mode1)
	free(spd_policy_mode1);
    if(spd_policy_mode2)
	free(spd_policy_mode2);
    if(function_setkey(setkey_conf_latest) == 0)
    {
        fd_config = fopen(setkey_conf, "a+" );
        if(fd_config == NULL)
        {
	      plog_android(LLV_WARNING,"open %s failed,errno:%d",setkey_conf,errno);
	      return -1;	
        }
	chown(setkey_conf,0,1000);
        fwrite( spd_policy, strlen(spd_policy),1,fd_config );
        fclose(fd_config);   
    }
    else
        return -1;
    return 0;
#endif
	int ret = setkey_SP_2layer_xfrm(XFRM_MSG_UPDPOLICY,src_range,dst_range,protocol,port_src,port_dst,src_tunnel,dst_tunnel,ipsec_type1,mode1,ipsec_type2,mode2,direction,u_id1,u_id2);
	return ret;
}


/*flush SA\SP from setkey.conf*/
int flush_SA_SP_exist()
{
#if 0
	FILE *fpSrc = NULL;     
	FILE *fpDst = NULL; 
    char * p_add = NULL;  
    char * p_spdadd = NULL;  
    char * p = NULL;
    char * sp = NULL;
    char * sp_ipsec = NULL;   
    char * sp_prio = NULL;  
#if 0
    char * sp_tmp = NULL;
    char * sp_src_tmp = NULL;
    char * sp_dst_tmp = NULL;
    char * sp_dst = NULL;     
#endif  
    if(access(setkey_conf,0) != 0)
    {
	    plog_android(LLV_WARNING,"There is no SA before\n");            
	    return 0; 
    }


	char  * achBuf = (char *)malloc(POLICY_LEN); 
    if(achBuf == NULL)
	{         
		plog_android(LLV_WARNING,"malloc achBuf failed\n");           
		return -1;     
	} 
 	char * achBuf_deletmp = (char *)malloc(POLICY_LEN); 
    if(achBuf_deletmp == NULL)
	{         
		plog_android(LLV_WARNING,"malloc achBuf_deletmp failed\n");      
		if(achBuf) 
			free(achBuf);

		return -1;     
	}  
 	char * achBuf_delet = (char *)malloc(POLICY_LEN); 
    if(achBuf_delet == NULL)
	{         
		plog_android(LLV_WARNING,"malloc achBuf_delet failed\n");   
		if(achBuf) 
			free(achBuf);
		if(achBuf_deletmp) 
			free(achBuf_deletmp);   
		return -1;     
	}   	   
 	

	fpSrc = fopen(setkey_conf, "rt");     
	if (NULL == fpSrc)     
	{         
		plog_android(LLV_WARNING, "can't open %s,errno:%d",setkey_conf,errno);  
		if(achBuf) 
			free(achBuf);
		if(achBuf_deletmp) 
			free(achBuf_deletmp);    
		if(achBuf_delet) 
			free(achBuf_delet);   
		return -1;     
	}     
	chown(setkey_conf,0,1000);  
	fpDst = fopen(setkey_conf_latest, "w+");     
	if (NULL== fpDst)     
	{     
		if(achBuf) 
			free(achBuf);
		if(achBuf_deletmp) 
			free(achBuf_deletmp);    
		if(achBuf_delet) 
			free(achBuf_delet);       
		plog_android(LLV_WARNING,"Create source file: %s failed,errno:%d\n", setkey_conf_bak,errno); 
		if(fpSrc)        
			fclose(fpSrc);         
		return -1;     
	} 
	chown(setkey_conf_latest,0,1000);      
    fseek(fpSrc,0L,SEEK_SET);
	while (!feof(fpSrc))     
	{         
		memset(achBuf, 0, POLICY_LEN);   
		memset(achBuf_deletmp, 0, POLICY_LEN); 
		memset(achBuf_delet, 0, POLICY_LEN); 				      
		fgets(achBuf, POLICY_LEN, fpSrc);  

 		if (((p_add = strstr(achBuf, "add")) != NULL)&&(strstr(achBuf, "spdadd") == NULL) )        
		{ 
                      
			p = strstr(p_add,"-m");  
			if(p!= NULL)
			{
				memcpy(achBuf_deletmp,p_add + strlen("add"),p-p_add-strlen("add"));
				snprintf(achBuf_delet,POLICY_LEN-1,"delete %s;\n",achBuf_deletmp) ; 
               			plog_android(LLV_WARNING,"delete SA:%s\n", achBuf_delet);       
			    	fputs(achBuf_delet, fpDst);                              
			}  
			else
			{
				plog_android(LLV_WARNING,"There are some cmd error in %s,then flush all SAs and SPs\n", setkey_conf); 
                		setkey_flushSAD();
                		setkey_flushSPD();
				if(achBuf) 
					free(achBuf);
				if(achBuf_deletmp) 
					free(achBuf_deletmp);    
				if(achBuf_delet) 
					free(achBuf_delet);   
				if(fpSrc)        
					fclose(fpSrc); 
				if(fpDst)        
					fclose(fpDst); 
				return -1;
			}	     
		}

		if((p_spdadd = strstr(achBuf, "spdadd")) != NULL)
		{
			sp = p_spdadd+strlen("spdadd");
            		if((sp_prio =strstr(sp,"prio"))!=NULL)
            		{
		        memset(achBuf_deletmp, 0, POLICY_LEN); 
		        memset(achBuf_delet, 0, POLICY_LEN); 
		        strncpy(achBuf_deletmp,sp,sp_prio-sp);
			snprintf(achBuf_delet,POLICY_LEN-1,"spddelete %s",achBuf_deletmp);
                	strcat(achBuf_delet,";\n");
		        fputs(achBuf_delet, fpDst); 
               		plog_android(LLV_WARNING,"delete policy: %s\n", achBuf_delet);
           	 	}
			else if((sp_ipsec =strstr(sp,"ipsec"))!=NULL)
            		{
		        memset(achBuf_deletmp, 0, POLICY_LEN); 
		        memset(achBuf_delet, 0, POLICY_LEN); 
		        strncpy(achBuf_deletmp,sp,sp_ipsec-sp);
			    snprintf(achBuf_delet,POLICY_LEN-1,"spddelete %s",achBuf_deletmp);
                strcat(achBuf_delet,";\n");
		        fputs(achBuf_delet, fpDst); 
                plog_android(LLV_WARNING,"delete policy: %s\n", achBuf_delet);
            }
			else
			{
		        plog_android(LLV_WARNING,"There are some cmd error in %s,no ,then flush all SAs and SPs\n", setkey_conf); 
                	setkey_flushSAD();
                	setkey_flushSPD();
			if(achBuf) 
				free(achBuf);
			if(achBuf_deletmp) 
				free(achBuf_deletmp);    
			if(achBuf_delet) 
				free(achBuf_delet);   
			if(fpSrc)        
				fclose(fpSrc); 
			if(fpDst)        
				fclose(fpDst); 
		        return -1;				
			}	
#if 0
			if((sp_tmp =strchr(sp,'['))!=NULL)
			{
		        memset(achBuf_deletmp, 0, POLICY_LEN); 
		        memset(achBuf_delet, 0, POLICY_LEN); 
				strncpy(achBuf_deletmp,sp,sp_tmp-sp);
				snprintf(achBuf_delet,POLICY_LEN-1,"spddelete %s",achBuf_deletmp);
                plog_android(LLV_WARNING,"src[ achBuf_delet: %s,sp_tmp:%s\n", achBuf_deletmp,sp_tmp); 
			    if((sp_src_tmp =strchr(sp_tmp,']'))!=NULL)
			    {

			    	if((sp_dst_tmp =strchr(sp_src_tmp,'['))!=NULL)
			    	{
			    		memset(achBuf_deletmp, 0, POLICY_LEN);
			    		strncpy(achBuf_deletmp,sp_src_tmp+strlen("]"),sp_dst_tmp-sp_src_tmp-strlen("]"));
				        strcat(achBuf_delet,achBuf_deletmp);
                        plog_android(LLV_WARNING,"src]_achBuf_delet: %s,achBuf:%s,sp_dst_tmp:%s\n", achBuf_deletmp,achBuf,sp_dst_tmp); 
				        if((sp_dst =strchr(sp_dst_tmp,']'))!=NULL)
				        {
				        	if((sp_ipsec =strstr(sp_dst,"ipsec"))!=NULL)
				        	{
			    		        memset(achBuf_deletmp, 0, POLICY_LEN);
			    		        strncpy(achBuf_deletmp,sp_dst+strlen("]"),sp_ipsec-sp_dst-strlen("]"));
				                strcat(achBuf_delet,achBuf_deletmp);
                                strcat(achBuf_delet,";\n");
				                fputs(achBuf_delet, fpDst);  
                                plog_android(LLV_WARNING,"spdadd :%s\n",achBuf_delet); 				        		
				            }
				            else
				            {
				            	plog_android(LLV_WARNING,"There are some cmd error in %s,before ipsec,then flush all SAs and SPs\n", setkey_conf);
                                setkey_flushSAD();
                                setkey_flushSPD();
				            	return -1;				            	
				            }
				        }
				        else
			            {
				            plog_android(LLV_WARNING,"There are some cmd error in %s,no dst],then flush all SAs and SPs\n", setkey_conf); 
                            setkey_flushSAD();
                            setkey_flushSPD();
				            return -1;				
			            }				        	
			    	}
			        else
			        {
				        plog_android(LLV_WARNING,"There are some cmd error in %s,no dst[,then flush all SAs and SPs\n", setkey_conf); 
                                        setkey_flushSAD();
                                        setkey_flushSPD();
				        return -1;				
			        }			    	
			    }	
			    else
			    {
				    plog_android(LLV_WARNING,"There are some cmd error in %s,no ],then flush all SAs and SPs\n", setkey_conf); 
                    setkey_flushSAD();
                    setkey_flushSPD();
				    return -1;				
			    }	
		    						
			}
			else
			{
				plog_android(LLV_WARNING,"There are some cmd error in %s,no [,then flush all SAs and SPs\n", setkey_conf); 
                setkey_flushSAD();
                setkey_flushSPD();
				return -1;				
			}
#endif		
		}	
		         
    
	}     
	fclose(fpSrc);     
	fclose(fpDst);     
	if(achBuf) 
		free(achBuf);
	if(achBuf_deletmp) 
		free(achBuf_deletmp);    
	if(achBuf_delet) 
		free(achBuf_delet);      
    if(function_setkey(setkey_conf_latest) == -1)
    {
        return -1;
    }
    else
    {
    	if(shell_rm() == -1)
        {
	        return -1;
        }
    }
	return 0; 	
#endif
	int ret = flush_SA_SP_exist_xfrm();
	return ret;
}




