
#include  <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <statusd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <com_intf.h>

#ifndef COM_ANDROID
#include <syslog.h>
//  #define LOG(lvl, f, ...) do{if(lvl<=syslog_level)syslog(lvl,"%s:%d:%s(): " f "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);}while(0)
#define LOGINTF(lvl,f,...) do{if(lvl<=syslog_level){\
								  printf("COM INTF %d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
								}\
							}while(0)
#else //will enable logging using android logging framework (not to file)
#define LOG_TAG "STATUSD"
#include <utils/Log.h> //all Android LOG macros are defined here.
#define LOGINTF(lvl,f,...) do{if(lvl<=syslog_level){\
								LOG_PRI(android_log_lvl_convert[lvl],LOG_TAG,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);}\
						  }while(0)

//just dummy defines since were not including syslog.h.
#define LOG_EMERG	0
#define LOG_ALERT	1
#define LOG_CRIT	2
#define LOG_ERR		3
#define LOG_WARNING	4
#define LOG_NOTICE	5
#define LOG_INFO	6
#define LOG_DEBUG	7
/* Android's log level are in opposite order of syslog.h */
int android_log_lvl_convert[8]={ANDROID_LOG_SILENT, /*8*/
								ANDROID_LOG_SILENT, /*7*/
								ANDROID_LOG_FATAL, /*6*/
								ANDROID_LOG_ERROR,/*5*/
								ANDROID_LOG_WARN,/*4*/
								ANDROID_LOG_INFO,/*3*/
								ANDROID_LOG_DEBUG,/*2*/
								ANDROID_LOG_VERBOSE};/*1*/
#endif /*MUX_ANDROID*/

#define SYSCHECK(c) do{if((c)<0){LOGINTF(LOG_ERR,"system-error: '%s' (code: %d)", strerror(errno), errno);\
						return -1;}\
					}while(0)


pthread_mutex_t syslogdump_lock;

static int syslog_level = LOG_NOTICE;
struct cli_info_t cli_info[CLIENT_MAX];
statusd_data_callback callback[CLIENT_MAX];
pthread_t thread_id[CLIENT_MAX];
pthread_attr_t thread_attr;
char intial_flag = 0;/*if init_com_inf had been initialed*/
pthread_mutex_t statusd_send_lock;
pthread_mutex_t statusd_fd_lock;


const char* name_inquery(char modeule_id)
{

	switch(modeule_id){
		case MODULE_TYPE_FLS:
			return "flashless";
		case MODULE_TYPE_MUX:
			return "mux";
		case MODULE_TYPE_RIL:
			return "ril";
		case MODULE_TYPE_CTC:
			return "ctclient";
		case MODULE_TYPE_SR:
				return "server";
		case MODULE_TYPE_DG:
				return "debugger";
		default:
			return "unknow";
	}
}

const char* type_inquery(char type_id)
{

	switch(type_id){
		case CMD_CLIENT_START:
				return "start";
		case CMD_CLIENT_INIT:
			return "init";
		case CMD_CLIENT_READY:
			return "ready";
		case CMD_CLIENT_EXITING:
			return "exiting";
		case CMD_CLIENT_ERROR:
			return "err";
		case CMD_DATA_ACK:
			return "ack";
		case CMD_DATA_NOACK:
			return "noack";
		case CMD_RESET_CLIENT:
			return "reset";
		case CMD_KILL_CLIENT:
			return "kill";
		default:
			return "unknow";
	}
}


/*
* Purpose:  ascii/hexdump a byte buffer
* Input:	    prefix - string to printed before hex data on every line
*                ptr - the string to be dumped
*                length - the length of the string to be dumped
* Return:    0
*/
int com_syslogdump(
	const char *prefix,
	const unsigned char *ptr,
	unsigned int length)
{
	if(syslog_level <= LOG_ERR)
		return 0;


	unsigned int len = length;
	unsigned int off = 0;
	char buffer[100*3+20]={0};
	unsigned int i;
	char *p;

	pthread_mutex_lock(&syslogdump_lock); 	//new lock

	if ((syslog_level<=LOG_DEBUG) && (syslog_level >=LOG_ERR)) /*No need for all frame logging if it's not to be seen */
	{
			strcpy(buffer, prefix);
			off = strlen(buffer);
			SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "[%d]", len));
			off = strlen(buffer);
			p = (char *)name_inquery(*(ptr+1));
			SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "(%c",*p));
			off = strlen(buffer);
			p = (char *)name_inquery(*(ptr+2));
			SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " %c", *p));
			if(ptr[3] == STATUS_DATATYPE_CMD){
				off = strlen(buffer);
				p = (char *)type_inquery(*(ptr+5));
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " %s)", p));
				off = strlen(buffer);
			}
			if((len <7) && (*(ptr+3) == STATUS_DATATYPE_CMD)){
				LOGINTF(LOG_EMERG,"invalid cmd");
				pthread_mutex_unlock(&syslogdump_lock);
				return -1;
			}
			SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%s", "....." ));
			off = strlen(buffer);
			while (len>0){
				for (i = off; i < (len>100?100:len)+off; i+=4){
					SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%02x ", ptr[i-off]));
					off = strlen(buffer);
				}
			LOGINTF(LOG_EMERG,"%s", buffer);
			len = len-(len>100?100:len);
			memset(buffer,0,sizeof(buffer));
			off = 0;
		}
	}

	pthread_mutex_unlock(&syslogdump_lock);/*new lock*/
	return 0;
}

/*
* Purpose:  init communication relate struct
* Input:	    -
* Return:    0 if succes
*/
static int init_com_inf(void)
{
	int  i = 0;

	for(i=0;i<CLIENT_MAX;i++){
		cli_info[i].cli_sock_fd = -1;
		cli_info[i].module_id = -1;
	}
	return 0;
}

/*
* Purpose: send data to server
* Input:		src_modeule_id : data source		:enum VIA_IPC_MODULE
*			dst_module_id	:data destation	:enum VIA_IPC_MODULE
*			type			:data type 		:enum DATA_TYPE
*			buffer			:data			:char
*			length			:data length except protocol content
* Return:    data length which had been send to server success except protocol content
*/
int statusd_c2s_comdata(char src_modeule_id,
								char dst_module_id,
								unsigned char type,
								char *buffer,
								unsigned char length)
{
	unsigned char frame[STATUSD_MAX_DATA_LEN+PROTOCOL_DATA_LEN];
	int client_sockfd = 0;
	struct timeval timeout;

	int retry = 0;
	int error_flag = 0;
	fd_set fdw;
	int ret=0;
	int i = 0;

	LOGINTF(LOG_DEBUG, "Enter");

	if((src_modeule_id>=MODULE_TYPE_COUNT)
		|| (dst_module_id>=MODULE_TYPE_COUNT)
		|| (type >= STATUS_DATATYPE_COUNT)
		|| (length > STATUSD_MAX_DATA_LEN)){
		LOGINTF(LOG_DEBUG,"data struct error::src_modeule_id=%d.dst_module_id=%d type=%d length=%d",
			src_modeule_id,dst_module_id,type,length);
		return -1;
	}

	for(i=0;i<CLIENT_MAX;i++){
		if(cli_info[i].module_id == src_modeule_id){
			client_sockfd = cli_info[i].cli_sock_fd;
			break;
		}
	}
	LOGINTF(LOG_DEBUG,"got client fd=%d",client_sockfd);
	if(i >= CLIENT_MAX){
		LOGINTF(LOG_ERR, "data struct error::data from unknown module %d",src_modeule_id);
		goto terminate;
	}
	memset(frame,0,STATUSD_MAX_DATA_LEN);
	frame[0] = STATUSD_FRAME_FLAG;
	frame[1] = src_modeule_id;
	frame[2] = dst_module_id;
	frame[3] = type;
	frame[4] = length;
	memcpy(frame+PROTOCOL_DATA_LEN-1,buffer,length);
	frame[length+PROTOCOL_DATA_LEN-1] = STATUSD_FRAME_FLAG;
	com_syslogdump(">C2S",frame,length+PROTOCOL_DATA_LEN);
	pthread_mutex_lock(&statusd_send_lock);
	do{
		LOGINTF(LOG_DEBUG, "retry=%d",retry);
		if(retry >= RETRY_TIMES)
			break;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO( &fdw ) ;
		FD_SET( client_sockfd, &fdw ) ;
		ret = select( client_sockfd+1, NULL, &fdw, NULL, &timeout ) ;
		if(ret == 0){
			LOGINTF(LOG_DEBUG, "select interface timeout,try again");
			retry++;
			continue;
		}else if (ret >0) {
			if (FD_ISSET(client_sockfd,&fdw)){
				LOGINTF(LOG_DEBUG, "client socket is writeable");
				ret = write(client_sockfd, frame, length+PROTOCOL_DATA_LEN);
				if(ret == length+PROTOCOL_DATA_LEN){
					LOGINTF(LOG_DEBUG, "client socket write finsh ret=%d/%d ",ret,length);
					break;
				}
				else if (ret != length+6 && errno == EAGAIN){/*if it still not ready ,try again*/
					LOGINTF(LOG_ERR, "client socket write failed,try again ret=%d errno=0x%x",ret,errno);
					retry ++;
					continue;
				}else {
					LOGINTF(LOG_ERR, "unknow error,ret=%d",ret);
					perror("write");
					error_flag = 1;
					break;
				}
			}
			else{
				LOGINTF(LOG_ERR, "why come here");
				perror("select");
			}
		}
	}while (1);
	pthread_mutex_unlock(&statusd_send_lock);
terminate:
	LOGINTF(LOG_DEBUG, "Leave");
	return ret-PROTOCOL_DATA_LEN;
}

/*
* Purpose: send one cmd to server
* Input:		src_modeule_id : data source		:enum VIA_IPC_MODULE
*			dst_module_id	:data destation	:enum VIA_IPC_MODULE
*			cmd				:data type 		:enum VIA_IPC_CMD_TYPE
* Return:    data length which had been send to server success except protocol content
*/
int statusd_c2ssend_cmd(unsigned char src_modeule_id,
									unsigned char dst_module_id,
									char cmd)
{
	unsigned char buf[2] ={0};

	buf[0] = cmd;
	return statusd_c2s_comdata(src_modeule_id,dst_module_id,STATUS_DATATYPE_CMD,(char *)buf,1);
}

/*
* Purpose: send one data to server
* Input:		src_modeule_id : data source		:enum VIA_IPC_MODULE
*			dst_module_id	:data destation	:enum VIA_IPC_MODULE
*			*buf			:data type 		:enum VIA_IPC_CMD_TYPE
*			len				:data length
* Return:    data length which had been send to server success except protocol content
*/
int statusd_c2ssend_data(unsigned char src_modeule_id,
									unsigned char dst_module_id,
									char *buf,unsigned int len)
{
	if(NULL == buf)
		return -1;
	return statusd_c2s_comdata(src_modeule_id,dst_module_id,STATUS_DATATYPE_DATA,buf,len);
}



/*
* Purpose:  Poll a device (file descriptor) using select()
*                if select returns data to be read. call a reading function for the particular device
* Input:      vargp - a pointer to a struct cli_info_t * struct.
* Return:    NULL if error
*/

void* child_poll_data(void *vargp)
{
	struct cli_info_t *cli_info =(struct cli_info_t *)vargp;
	char dst_module_id,src_module_id,data_type;
	statusd_data_callback icallback = NULL;
	struct timeval timeout;
	unsigned char buffer[1024] = {0};
	int data_len;
	fd_set fdr;
	int ret=0;

	LOGINTF(LOG_DEBUG, "Enter");
	while(1){
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		FD_ZERO( &fdr ) ;
		pthread_mutex_lock(&statusd_fd_lock);
		if (cli_info->cli_sock_fd > 0)
			FD_SET( cli_info->cli_sock_fd, &fdr);
		pthread_mutex_unlock(&statusd_fd_lock);

		ret = select( cli_info->cli_sock_fd+1, &fdr, NULL, NULL, &timeout ) ;
		if (ret >0) {
			pthread_mutex_lock(&statusd_fd_lock);
			if ((cli_info->cli_sock_fd > 0) && FD_ISSET(cli_info->cli_sock_fd,&fdr)){
				LOGINTF(LOG_NOTICE, "fd is ok");
				ret = read(cli_info->cli_sock_fd,buffer,sizeof(buffer));
				pthread_mutex_unlock(&statusd_fd_lock);
				src_module_id = buffer[1];
				dst_module_id = buffer[2];
				data_type = buffer[3];
				data_len = buffer[4];
				if(dst_module_id >= 0 && dst_module_id < CLIENT_MAX)
					icallback = callback[(int)dst_module_id];
				if(NULL != icallback)
					icallback(&src_module_id,&dst_module_id,&data_type,&data_len,(unsigned char *)(buffer+5));
			}
			else
				pthread_mutex_unlock(&statusd_fd_lock);
		}else if(ret == 0){//timeout
			 LOGINTF(LOG_DEBUG, "Device read function wake up ");
		}
		else{
			LOGINTF(LOG_ERR, "client communication interface error,server is shutdown or unknown error");
			perror("child_poll_data");
		}
	}
	LOGINTF(LOG_DEBUG, "Leave");
}

/*
* Purpose:  Creates a detached thread. also checks for errors on exit.
* Input:      thread_id - pointer to pthread_t id
*                thread_function - void pointer to thread function
*                thread_function_arg - void pointer to thread function args
* Return:    0 if success, 1 if fail
*/
int com_create_thread(pthread_t * thread_id, void * thread_function, void * thread_function_arg ){
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	LOGINTF(LOG_DEBUG,"Enter");
	if(pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg)!=0){
		perror("com_create_thread");
		LOGINTF(LOG_ERR,"Could not create thread");
		return 1;
	}
	pthread_attr_destroy(&thread_attr); /* Not strictly necessary */

	return 0; //thread created successfully
}

/*
* Purpose: register one communcation interface so that you can communicate with statusd
* Input:		src_modeule_id : data source		:enum VIA_IPC_MODULE
*			icallback		    :	client copy data in the callback
* Return:    0:success if success
*		    -1:failed
*/
int statusd_register_cominf(char module_id,statusd_data_callback icallback)
{
	int client_sockfd = 0;
	struct sockaddr_un server_sockaddr,cli_sockaddr;
	fd_set	wtfds;
    int flags;
	int ret;
	struct timeval timeout;
 	int i = module_id;

	LOGINTF(LOG_DEBUG, "Enter module_id=%d",module_id);
	LOGINTF(LOG_DEBUG, "intial_flag=%d",intial_flag);
	if(!intial_flag){
		init_com_inf();
		intial_flag =1;
	}
	if(cli_info[i].module_id != (char)-1){
		LOGINTF(LOG_ERR,"%s communication interface had already been registered cli_info[%d].module_id=%d\n",
			name_inquery(module_id),i,cli_info[i].module_id);
		return -1;
	}
	else{
		cli_info[i].module_id = module_id;
		client_sockfd = socket(AF_UNIX,SOCK_STREAM,0);
		if (client_sockfd < 0)
		{
			LOGINTF(LOG_ERR,"socket failed:%d", client_sockfd);
			return -1;
		}
	//	memset( &cli_sockaddr, 0, sizeof( cli_sockaddr) ) ;
		cli_sockaddr.sun_family = AF_UNIX ;
		strcpy( cli_sockaddr.sun_path, name_inquery(module_id)) ;
		ret = bind( client_sockfd, ( struct sockaddr * )&cli_sockaddr, sizeof( cli_sockaddr ) ) ;
		if (ret)
		{
			LOGINTF(LOG_ERR,"bind failed:%d", ret);
		}
		server_sockaddr.sun_family=AF_UNIX;
	//	server_sockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	//	server_sockaddr.sin_port=htons(9734);
		strcpy( server_sockaddr.sun_path, SOCKETNAME ) ;

		flags=fcntl(client_sockfd,F_GETFL,0);
		if (flags == -1)
		{
			LOGINTF(LOG_ERR,"fcntl failed");
			perror("fcntl");
			close(client_sockfd);
			return -1;
		}

		flags = fcntl(client_sockfd,F_SETFL,flags|O_NONBLOCK);
		if (flags == -1)
		{
			LOGINTF(LOG_ERR,"fcntl failed");
		}

		ret = connect(client_sockfd,( struct sockaddr * )&server_sockaddr,sizeof(server_sockaddr));
		LOGINTF(LOG_NOTICE,"ret=%d",ret);
		if (!ret)
		{
			LOGINTF(LOG_NOTICE,"connect immediately");
			LOGINTF(LOG_DEBUG,"client_sockfd=%d",client_sockfd);
		}
		else if ((ret == -1) && (errno == EINPROGRESS))
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			int iRet1 = select(0, NULL, &wtfds, NULL, &timeout);
			if (iRet1 < 0)
			{
				perror("connect error");
				return -1;
			}
			else if (!iRet1)
			{
				perror("timeout error");
				return -1;
			}
			else
			{
				LOGINTF(LOG_DEBUG,"connect success");
			}
		}else{
			LOGINTF(LOG_ERR,"%s::error on connecting ret=%d errno=%s",name_inquery(module_id),ret,strerror(errno));
			close(client_sockfd);
			return -1;
		}
		cli_info[i].cli_sock_fd = client_sockfd;
		cli_info[i].module_id = module_id;
		callback[i] = icallback;
		ret = com_create_thread(&thread_id[i],child_poll_data,(void*) &cli_info[i]);
		if (0!=ret)
		{
			LOGINTF(LOG_DEBUG,"com_create_thread failed ret=%d",ret);
			return -1;
		}
	}
	ret = statusd_c2ssend_cmd(module_id,MODULE_TYPE_SR,CMD_CLIENT_START);
	if(ret != 1){
		LOGINTF(LOG_ERR,"CLIENT %s::send CMD_CLIENT_START cmd failed ret=%d\n",name_inquery(module_id),ret);
	}

	LOGINTF(LOG_DEBUG, "Leave");
	return 0;
}

/*
* Purpose: deregister one communcation interface if you don't need it again
* Input:		src_modeule_id : data source		:enum VIA_IPC_MODULE
* Return:    0:success if success
*/
int statusd_deregister_cominf(char module_id)
{
	int i = module_id;

	LOGINTF(LOG_DEBUG, "Enter");
	pthread_mutex_lock(&statusd_fd_lock);
	LOGINTF(LOG_NOTICE, "fd is bad");
	shutdown(cli_info[i].cli_sock_fd, SHUT_RDWR);
	close(cli_info[i].cli_sock_fd);
	cli_info[i].cli_sock_fd = -1;
	cli_info[i].module_id = -1;
	callback[i] = NULL;
	pthread_mutex_unlock(&statusd_fd_lock);
	LOGINTF(LOG_DEBUG, "Leave");
	return 0;
}

