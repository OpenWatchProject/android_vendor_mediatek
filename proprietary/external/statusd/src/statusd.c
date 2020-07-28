/*
 * Statusd Implementation with User Space
 *

 *
 * Copyright (C) 2012 jinx <jinx@via-telecom.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


/* If compiled with the STATUSD_ANDROID flag this mux will be enabled to run under Android */

/**************************/
/* INCLUDES                          */
/**************************/
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <statusd.h>
#include <stdlib.h>
#include <time.h>
#include <c2kutils.h>
#include <termios.h>
#include <string.h>
#include <cutils/android_filesystem_config.h>
#include <cutils/sockets.h>
#include <poll.h>
#include <errno.h>

#ifndef STATUS_ANDROID
#include <syslog.h>
//  #define LOG(lvl, f, ...) do{if(lvl<=statusd_log_level)syslog(lvl,"%s:%d:%s(): " f "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);}while(0)
#define LOGSTATUS(lvl,f,...) do{if(lvl<=statusd_log_level){\
                                  printf("STATUSD %d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
                                }\
                            }while(0)
#else //will enable logging using android logging framework (not to file)
#define LOG_TAG "STATUSD"
#include <utils/Log.h> //all Android LOG macros are defined here.
#include <cutils/properties.h>

#define MD1_STATUS_SYNC_PROP "vendor.mtk.md1.status"
#define MD3_STATUS_SYNC_PROP "vendor.mtk.md3.status"

enum {
    MD_STATUS_SYNC_STOP,
    MD_STATUS_SYNC_INIT,
    MD_STATUS_SYNC_HS,
    MD_STATUS_SYNC_RDY,
    MD_STATUS_SYNC_EXP,
    MD_STATUS_SYNC_RST,
    MD_STATUS_SYNC_RST_START,
    MD_STATUS_SYNC_FLIGHT_MODE,
};

static char* md_status_sync_str[] = {
    [MD_STATUS_SYNC_STOP] = "stop",
    [MD_STATUS_SYNC_INIT] = "init",
    [MD_STATUS_SYNC_HS] = "handshake",
    [MD_STATUS_SYNC_RDY] = "ready",
    [MD_STATUS_SYNC_EXP] = "exception",
    [MD_STATUS_SYNC_RST] = "reset",
    [MD_STATUS_SYNC_RST_START] = "reset_start",
    [MD_STATUS_SYNC_FLIGHT_MODE] = "flightmode",
};

static unsigned char need_md1_md3_reset_sync = 0;
static unsigned char need_silent_reboot = 0;

time_t rawtime;
struct tm *timeinfo;
char timE [80]={0};

#define LOGSTATUS(lvl,f,...) do{\
    if(logtofile){ \
        if(lvl <= statusd_log_level){ \
            time ( &rawtime ); \
            timeinfo = localtime ( &rawtime ); \
            strftime ( timE,80,"%Y-%m-%d %I:%M:%S",timeinfo); \
            if(logfile){ \
                fprintf(logfile, "%s %d:%s(): " f"\n", timE, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
                fflush(logfile); \
            }else{ \
                fprintf(stderr,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            }  \
        }  \
    }else{ \
        if(lvl<=statusd_log_level){\
                LOG_PRI(android_log_lvl_convert[lvl],LOG_TAG,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);}\
     }  \
}while(0)

//just dummy defines since were not including syslog.h.
#define LOG_EMERG    0
#define LOG_ALERT    1
#define LOG_CRIT    2
#define LOG_ERR        3
#define LOG_WARNING    4
#define LOG_NOTICE    5
#define LOG_INFO    6
#define LOG_DEBUG    7
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
#define SYSCHECK(c) do{if((c)<0){LOGSTATUS(LOG_ERR,"system-error: '%s' (code: %d)", strerror(errno), errno);\
                        return NULL;}\
                    }while(0)


#ifndef min
#define min(a,b) ((a < b) ? a :b)
#endif
#define VMODEM_DEVICE_PATH        "/dev/vmodem"
#define LOG_FILE_PATH    "/data/vendor/status.log"


/**************************/
/* CONSTANTS & GLOBALS     */
/**************************/
static char *revision = "0.3.2";
struct statusd_info_t statusd_info;
struct Statusd_Buffer statusd_in_buffer;

pthread_mutex_t statusd_datacount_lock;
pthread_mutex_t statusd_newdataready_lock;
pthread_mutex_t statusd_child_lock;
pthread_mutex_t statusd_ramdump_lock;
pthread_mutex_t statusd_err_lock;

pthread_cond_t statusd_newdata_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t child_ready_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t child_exit_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t statusd_rstind_signal = PTHREAD_COND_INITIALIZER;

static int statusd_log_level = LOG_NOTICE;
static int statusd_have_flashless = 0;
static int statusd_have_mux = 0;
static int statusd_have_ril = 0;
static int statusd_have_ctclient = 0;
static int statusd_child_err = 0;
static int statusd_ril_exit = 0;
static int statusd_ril_not_ready = 0;
static int statusd_ramdump_cbp = 0;
static int statud_mdm_reset = 1;/*mdm need reset*/
static int statud_mdm_reset_single = 0;
static int statusd_mdm_error = 0;
static int statusd_mdm_reset_ongoing = 0;
static int logtofile = 0;
static FILE * logfile = NULL;
static int ipoflag = -1;

char *argv_fls[STATUSD_PARSER_MAXARGS]={NULL};
char *argv_mux[STATUSD_PARSER_MAXARGS]={NULL};
char *argv_ril[STATUSD_PARSER_MAXARGS]={NULL};
char *argv_ctclient[STATUSD_PARSER_MAXARGS]={NULL};

typedef enum{
    MD_BOOTING = 0,
    MD_READY,
    MD_EXCEPTION,
    MD_EXCEPTION_ONGOING,
    MD_RESET_ON_GOING,
    MD_OFF,
}MD_STATUS;

/**************************/
/* function declare                        */
/**************************/
static int statusd_free_child_para(void);




/**************************/
/* MAIN CODE                        */
/**************************/

/*
* Purpose:  Creates a detached thread. also checks for errors on exit.
* Input:      thread_id - pointer to pthread_t id
*                thread_function - void pointer to thread function
*                thread_function_arg - void pointer to thread function args
* Return:    0 if success, 1 if fail
*/
int create_thread(
        pthread_t * thread_id,
        void * thread_function,
        void * thread_function_arg)
{
    pthread_attr_t statusd_thread_attr;
    LOGSTATUS(LOG_DEBUG,"Enter");
    pthread_attr_init(&statusd_thread_attr);
    pthread_attr_setdetachstate(&statusd_thread_attr, PTHREAD_CREATE_DETACHED);

    if(pthread_create(thread_id, &statusd_thread_attr, thread_function, thread_function_arg)!=0){
        perror ("create_thread");
        return 1;
    }
    pthread_attr_destroy(&statusd_thread_attr); /* Not strictly necessary */
    LOGSTATUS(LOG_DEBUG,"LEAVE");
    return 0; //thread created successfully
}

/*
* Purpose:  Writes data to the buffer
* Input:      buf - pointer to the buffer
*                input - input data (in user memory)
*                length - how many characters should be written
* Return:    number of characters written
*/
static int statusd_data_push(
            struct Statusd_Buffer* buf,
            const unsigned char *input,
    int length)
{
    int c = buf->endp - buf->writep;

    LOGSTATUS(LOG_DEBUG, "Enter");
    LOGSTATUS(LOG_DEBUG,"buffer :free %d, stored %d", statusd_buffer_free(buf),statusd_buffer_count(buf));
    length = min((unsigned int)length, statusd_buffer_free(buf));
    if (length > c){
        memcpy(buf->writep, input, c);
        memcpy(buf->data, input + c, length - c);
        buf->writep = buf->data + (length - c);
    }else{
        memcpy(buf->writep, input, length);
        buf->writep += length;
        if (buf->writep == buf->endp)
            buf->writep = buf->data;
    }

    pthread_mutex_lock(&statusd_datacount_lock);
    buf->datacount += length; /*updating the data-not-yet-read counter*/
    LOGSTATUS(LOG_DEBUG,"buffer: written %d, free %d, stored %d", length,statusd_buffer_free(buf),statusd_buffer_count(buf));
    pthread_mutex_unlock(&statusd_datacount_lock);

    pthread_mutex_lock(&statusd_newdataready_lock);
    buf->newdataready = 1; /*signal assembly thread that new buffer data is ready*/
    pthread_mutex_unlock(&statusd_newdataready_lock);
    pthread_cond_signal(&statusd_newdata_signal);
    LOGSTATUS(LOG_DEBUG,"Leave");

    return length;
}

/*
* Purpose:  destroys a frame
* Input:      frame - pointer to the frame
* Return:    -
*/
static void statusd_destroy_frame(
        struct Via_Ipc_Data * frame)
{
    if (frame->data_len > 0)
        free(frame->data);

    free(frame);
}

/*
* Purpose:  Gets a complete basic mode frame from buffer. You have to remember to free this frame
*                when it's not needed anymore
* Input:      buf - the buffer, where the frame is extracted
* Return:    frame or null, if there isn't ready frame with given index
*/
static struct Via_Ipc_Data* statusd_get_frame(
        struct Statusd_Buffer * buf)
{
    int end;
    unsigned int length_needed = 7;//start_flag+src_module_id + dst_module_id+data_type+data_len+ 1 byte data+end_flag
    struct Via_Ipc_Data *frame = NULL;
    unsigned char *local_readp;
    unsigned int local_datacount, local_datacount_backup;

    LOGSTATUS(LOG_DEBUG, "Enter");

    if(buf->datacount < length_needed)/*we have not enough data or all data had been taken away*/
        return NULL;
    /*Find start flag*/
    while (!buf->flag_found && statusd_buffer_count(buf) > 0){
        if (*buf->readp == STATUSD_FRAME_FLAG)
            buf->flag_found = 1;
        pthread_mutex_lock(&statusd_datacount_lock); /* need to lock to operate on buf->datacount*/
        statusd_buffer_inc(buf->readp,buf->datacount);
        pthread_mutex_unlock(&statusd_datacount_lock);
    }
    if (!buf->flag_found){// no frame started
        LOGSTATUS(LOG_ERR, "Leave. No start frame 0x27 found in bytes stored in frame");
        return NULL;
    }
    /*skip empty frames  - skipping frame all spare start flags*/
    while (statusd_buffer_count(buf) > 0 && (*buf->readp == STATUSD_FRAME_FLAG)){
        LOGSTATUS(LOG_DEBUG, "%d",__LINE__);
        pthread_mutex_lock(&statusd_datacount_lock); /* need to lock to operate on buf->datacount*/
        statusd_buffer_inc(buf->readp,buf->datacount);
        pthread_mutex_unlock(&statusd_datacount_lock);
    }
    length_needed --;
    /* Okay, we're ready to analyze a proper frame header */

    /*Make local copy of buffer pointer and data counter.
     *They are shared between 2 threads, so we want to
     *update them only after a frame extraction
     */
    /*From now on, only operate on these local copies */
    local_readp = buf->readp;
    local_datacount = local_datacount_backup = buf->datacount; /* current no. of stored bytes in buffer */
    if (local_datacount >= length_needed){ /* enough data stored for 0710 frame header+footer? */
        if ((frame = (struct Via_Ipc_Data*)malloc(sizeof(struct Via_Ipc_Data))) != NULL){
            frame->src_module_id = *local_readp ; /*frame src module id read*/
            if (frame->src_module_id >= MODULE_TYPE_COUNT ){ /* Field Sanity check if channel ID actually exists */
                LOGSTATUS(LOG_WARNING, "Dropping frame: Corrupt! src module id field indicated %d, which does not exist",frame->src_module_id);
                free(frame);
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }
            length_needed --;
            statusd_buffer_inc(local_readp,local_datacount);
            frame->dst_module_id = *local_readp ; /*frame dst module id read*/
            if (frame->dst_module_id > MODULE_TYPE_COUNT ){ /* Field Sanity check if channel ID actually exists */
                LOGSTATUS(LOG_WARNING, "Dropping frame: Corrupt! dst module id field indicated %d, which does not exist",frame->dst_module_id);
                free(frame);
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }
            length_needed --;
            statusd_buffer_inc(local_readp,local_datacount);
            frame->data_type = *local_readp ; /*frame dst module id read*/
            if (frame->data_type > STATUS_DATATYPE_DATA){ /* Field Sanity check if data type ID actually exists */
                LOGSTATUS(LOG_WARNING, "Dropping frame: Corrupt! data type  field indicated %d, which does not exist",frame->data_type);
                free(frame);
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }
            length_needed --;
            statusd_buffer_inc(local_readp,local_datacount);
            frame->data_len = *local_readp ; /*frame dst module id read*/
            if (frame->data_len > STATUSD_MAX_DATA_LEN ){ /* Field Sanity check if data length is okey*/
                LOGSTATUS(LOG_WARNING, "Dropping frame: Corrupt! data length field indicated %d, which does not exist",frame->data_type);
                free(frame);
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }
            length_needed --;
        }else{
            LOGSTATUS(LOG_ERR, "Out of memory, when allocating space for frame");
        }

        statusd_buffer_inc(local_readp,local_datacount);
/*Okay, done with the frame header. Start extracting the payload data */
        if (frame && frame->data_len > 0){
            if ((frame->data = malloc(sizeof(char) * frame->data_len)) != NULL){
                end = buf->endp - local_readp;
                if (frame->data_len > end){ /*wrap-around necessary*/
                    memcpy(frame->data, local_readp, end);
                    memcpy(frame->data + end, buf->data, frame->data_len - end);
                    local_readp = buf->data + (frame->data_len - end);
                    local_datacount -= frame->data_len;
                }else{
                    memcpy(frame->data, local_readp, frame->data_len);
                    local_readp += frame->data_len;
                    local_datacount -= frame->data_len;
                    if (local_readp == buf->endp)
                        local_readp = buf->data;
                }
            }else{
                LOGSTATUS(LOG_ERR, "Out of memory, when allocating space for frame data");
                frame->data_len = 0;
            }
        }
/*Okay, check end flag */
        //statusd_buffer_inc(local_readp,local_datacount);
        if (*local_readp != STATUSD_FRAME_FLAG){
            LOGSTATUS(LOG_WARNING, "Dropping frame: End flag not present. Instead: %d", *local_readp);
            if (frame)
                statusd_destroy_frame(frame);
            buf->flag_found = 0;
            buf->dropped_count++;
            goto update_buffer_dropping_frame;
        }else
            buf->received_count++;
        statusd_buffer_inc(local_readp,local_datacount); /* prepare readp for next frame extraction */
    }
    else {
        LOGSTATUS(LOG_NOTICE, "Leave, not enough bytes stored in buffer for header information yet");
        return NULL;
    }
/* Everything went fine, update STATUSD buffer pointer and counter */
    pthread_mutex_lock(&statusd_datacount_lock);
    buf->readp = local_readp;
    buf->datacount -= (local_datacount_backup - local_datacount); /* subtract whatever we analyzed */
    pthread_mutex_unlock(&statusd_datacount_lock);
    buf->flag_found = 0; /* prepare for any future frame processing*/
    LOGSTATUS(LOG_DEBUG, "Leave, frame found");

    return frame;

update_buffer_dropping_frame:
    /*Update  buffer pointer and counter */
    pthread_mutex_lock(&statusd_datacount_lock);
    buf->readp = local_readp;
    buf->datacount -= (local_datacount_backup - local_datacount); /* subtract whatever we analyzed */
    pthread_mutex_unlock(&statusd_datacount_lock);
    return statusd_get_frame(buf);/*continue extracting more frames if any*/
}


/*
* Purpose:  accept socket connection and Poll devices (file descriptor) using select()
*                if select returns data to be read. accept connection from client if it is a client connection request
*            or call a reading function for the particular device
* Input:      vargp - a pointer to a struct statusd_info_t* struct.
* Return:    NULL if error
*/
void* statusd_com_thread(void *vargp)
{
    LOGSTATUS(LOG_DEBUG,"Enter");
    struct statusd_info_t *statusd_info = (struct statusd_info_t*)vargp;
    struct socket_info_t *socket_info = &statusd_info->socket_info;;
    struct timeval timeout;
    struct sockaddr_un    cli_sockaddr ;
    int socklen = sizeof( cli_sockaddr );
    int server_sockfd = socket_info->server_sockfd;
    char buffer[STATUSD_MAX_DATA_LEN];
    fd_set    watchset;
    int dst_fd = -1;
    int    new_cli_fd = 0;
    int nread = 0;
    int ret = 0;
    int i = 0;
    int len;

    if(server_sockfd == -1 ){
        LOGSTATUS(LOG_ERR, "server not initial");
        goto terminate;
    }

    while (1){
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        LOGSTATUS(LOG_DEBUG,"SR1 fd=%d ",server_sockfd);
        FD_ZERO( &watchset ) ;
        FD_SET( server_sockfd, &watchset ) ;
        for ( i=0;i<PROCESS_MAX;i++){
            if(socket_info->cli_info[i].cli_sock_fd != -1){
                FD_SET( socket_info->cli_info[i].cli_sock_fd, &watchset ) ;
                LOGSTATUS(LOG_DEBUG,"client%d fd=0x%x add to select",
                    socket_info->cli_info[i].module_id,socket_info->cli_info[i].cli_sock_fd);
            }
            if ( socket_info->maxfd < socket_info->cli_info[i].cli_sock_fd ) {
                pthread_mutex_lock(&statusd_datacount_lock);
                socket_info->maxfd = socket_info->cli_info[i].cli_sock_fd ;
                pthread_mutex_unlock(&statusd_datacount_lock);
            }
        }
        LOGSTATUS(LOG_INFO,"SR2 select begin");
        ret = select( socket_info->maxfd+1, &watchset, NULL, NULL, &timeout ) ;
        if ( ret < 0 ) {
            LOGSTATUS(LOG_ERR,"SERVER::Server error");
            continue;
        }
        LOGSTATUS(LOG_INFO,"SR3 select end ret=%d errno=0x%x",ret,errno);
        if ( FD_ISSET( server_sockfd, &watchset )) {
            new_cli_fd = accept( server_sockfd, ( struct sockaddr * )&( cli_sockaddr ), &socklen ) ;
            LOGSTATUS(LOG_INFO,"SR4 add communication Client %s on socket %d", cli_sockaddr.sun_path,new_cli_fd);
            if(new_cli_fd !=-1){
                for ( i=0;i < PROCESS_MAX;i++){
                    if(socket_info->cli_info[i].cli_sock_fd == new_cli_fd)/*client info already marked*/
                        break;
                }
                if(i >= PROCESS_MAX)/*mark client info*/
                {
                    for ( i=0;i < PROCESS_MAX;i++){
                        if(socket_info->cli_info[i].cli_sock_fd == -1) {
                            //cli_info_t[ci].module_id = cli_sockaddr.sun_path[0];
                            pthread_mutex_lock(&statusd_datacount_lock);
                            socket_info->cli_info[i].cli_sock_fd = new_cli_fd;
                            pthread_mutex_unlock(&statusd_datacount_lock);
                            LOGSTATUS(LOG_DEBUG,"mark cli_info_t[%d].cli_sock_fd=%d",
                                i,socket_info->cli_info[i].cli_sock_fd);
                            break;
                        }
                    }
                }
            }else{/*accept failed ,what can I do?*/
                LOGSTATUS(LOG_ERR,"accept failed,ret=%d server_sockfd=%d [%s]",new_cli_fd,server_sockfd,strerror(errno));
                usleep(500*1000);
            }
            //continue;/*accept cliet's connection,go to select the client*/
        }
        for ( i = 0; i < PROCESS_MAX; i++ ) {
            if (socket_info->cli_info[i].cli_sock_fd == -1)
                continue;
            if ( !FD_ISSET( socket_info->cli_info[i].cli_sock_fd, &watchset ) )
                continue ;
            LOGSTATUS(LOG_DEBUG,"SR5 cli[%d] fd=%d ",i,socket_info->cli_info[i].cli_sock_fd);
            ioctl(socket_info->cli_info[i].cli_sock_fd,FIONREAD,&nread);
            if (nread==0){/*socket disconnected*/
                close(socket_info->cli_info[i].cli_sock_fd);
                LOGSTATUS(LOG_DEBUG,"clear cli_info_t[%d].cli_sock_fd=%d",
                    i,socket_info->cli_info[i].cli_sock_fd);
                socket_info->cli_info[i].cli_sock_fd = -1;
                continue;
            }
            memset(buffer,0,sizeof(buffer));
            len = read( socket_info->cli_info[i].cli_sock_fd, buffer, sizeof(buffer) ) ;
            if(len < 0){
                LOGSTATUS(LOG_ERR,"read data from cliet %d failed",socket_info->cli_info[i].cli_sock_fd);
                continue;
            }
            dst_fd =socket_info->cli_info[i].cli_sock_fd;

            if(socket_info->cli_info[i].module_id == (char)-1){
                pthread_mutex_lock(&statusd_datacount_lock);
                socket_info->cli_info[i].module_id = buffer[1];
                LOGSTATUS(LOG_DEBUG,"socket_info.cli_info[%d].module_id=%d",
                        i,socket_info->cli_info[i].module_id);
                pthread_mutex_unlock(&statusd_datacount_lock);
            }
            else if(socket_info->cli_info[i].module_id != buffer[1]){
                LOGSTATUS(LOG_ERR,"not correct src module id %d /%d",
                        socket_info->cli_info[i].module_id,buffer[1]);
                continue;
            }
            LOGSTATUS(LOG_DEBUG,"SR6::data from Client %d",socket_info->cli_info[i].module_id);

            if(statusd_data_push(&statusd_in_buffer,(const unsigned char *)buffer,len) != len){
                LOGSTATUS(LOG_ERR,"push event failed");
            }else{
                LOGSTATUS(LOG_DEBUG,"SR7 push frame to buffer");
            }
        }
        //socket_info->maxfd = server_sockfd;
    }
terminate:
    LOGSTATUS(LOG_DEBUG, "Device polling thread terminated");
    LOGSTATUS(LOG_DEBUG,"Leave");
    //pthread_cond_signal(&statusd_newdata_signal);//release assemble_frame_thread

    return NULL;
}

/*
* Purpose:  clear child process informations and relate socket informations
* socket_info    :     socket info struct
* module_id        :    module num to be cleared
* Return:    1 if fail, 0 if success
*/
static int clear_client_info(
        struct socket_info_t *socket_info,
        char module_id)
{
    int maxfd = socket_info->server_sockfd;
    int i = 0;

    pthread_mutex_lock(&statusd_datacount_lock);
    for (i=0;i < PROCESS_MAX;i++){
        if(socket_info->cli_info[i].module_id == module_id) {
            LOGSTATUS(LOG_DEBUG,"cli_info_t[%d].cli_sock_fd=%d \n",i,socket_info->cli_info[i].cli_sock_fd);
            socket_info->cli_info[i].module_id = -1;
            socket_info->cli_info[i].cli_sock_fd = -1;
            break;
        }
    }
    if(i >= PROCESS_MAX){
        pthread_mutex_unlock(&statusd_datacount_lock);
        LOGSTATUS(LOG_ERR,"why come here,please check\n");
        return -1;
    }

    if(socket_info->cli_info[i].cli_sock_fd !=-1){
        close(socket_info->cli_info[i].cli_sock_fd);
        socket_info->cli_info[i].cli_sock_fd = -1;
    }

    for (i=0;i < PROCESS_MAX;i++){
        if(maxfd < socket_info->cli_info[i].cli_sock_fd)
            maxfd = socket_info->cli_info[i].cli_sock_fd;
    }
    socket_info->maxfd = maxfd;
    pthread_mutex_unlock(&statusd_datacount_lock);
    return 0;
}

static const char* id2str(char modeule_id)
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

static const char* cmd2str(char cmd)
{

    switch(cmd){
        case CMD_CLIENT_START:
                return "CMD_CLIENT_START";
        case CMD_CLIENT_INIT:
            return "CMD_CLIENT_INIT";
        case CMD_CLIENT_READY:
            return "CMD_CLIENT_READY";
        case CMD_CLIENT_EXITING:
            return "CMD_CLIENT_EXITING";
        case CMD_CLIENT_ERROR:
            return "CMD_CLIENT_ERROR";
        default:
            return "unknow";
    }
}

static void statusd_set_errflag(int flag)
{
    pthread_mutex_lock(&statusd_err_lock);
    if(statusd_child_err!=flag)
        statusd_child_err = flag;
    pthread_mutex_unlock(&statusd_err_lock);
}

static void statusd_get_errflag(int *flag)
{
    pthread_mutex_lock(&statusd_err_lock);
    *flag=statusd_child_err;
    pthread_mutex_unlock(&statusd_err_lock);

}

/*reuse the same lock with err flag*/
static void statusd_set_reset_flag(int flag)
{
    pthread_mutex_lock(&statusd_err_lock);
    if(statusd_mdm_reset_ongoing !=flag)
        statusd_mdm_reset_ongoing = flag;
    pthread_mutex_unlock(&statusd_err_lock);
}

static void statusd_get_reset_flag(int *flag)
{
    pthread_mutex_lock(&statusd_err_lock);
    *flag=statusd_mdm_reset_ongoing;
    pthread_mutex_unlock(&statusd_err_lock);

}

static int statusd_start_child(int module,char ** arg)
{
    int ret = -1;
    int wati_time = 0;
    int timeval=0;
    struct timeval tv_b,tv_e;
    struct timespec timeout;
    int flag;
    int md_status;

    LOGSTATUS(LOG_DEBUG,"Enter");
    if((module < MODULE_TYPE_FLS) || (module > MODULE_TYPE_DG))
        return -1;

    if (need_silent_reboot) {
        LOGSTATUS(LOG_NOTICE, "set vendor.ril.cdma.report 1\n");
        property_set("vendor.ril.cdma.report", "1"); /* set rild flag here, should before rild */
    }

    gettimeofday(&statusd_info.process_info[module].start_time, NULL);
    if(module == MODULE_TYPE_FLS){
        wati_time = STATUSD_WAIT_FLASHLESS_TIME_MAX;
    }else if(module == MODULE_TYPE_MUX){
        wati_time = STATUSD_WAIT_MUX_TIME_MAX;
    }else if(module == MODULE_TYPE_RIL){
        wati_time = STATUSD_WAIT_RIL_TIME_MAX;
    }else if(module == MODULE_TYPE_CTC){
        wati_time = STATUSD_WAIT_CTCLIENT_TIME_MAX;
    }
    errno=0;
    statusd_info.process_info[module].process_pid = fork();
    if(statusd_info.process_info[module].process_pid < 0) {
        LOGSTATUS(LOG_ERR,"fork %s failed [%s]",id2str(module),strerror(errno));
        return -1;
    }
    if (statusd_info.process_info[module].process_pid == 0) {
        // child side of the fork
        LOGSTATUS(LOG_NOTICE,"This is son. and his pid is: %d",getpid());
        if(module == MODULE_TYPE_FLS){
            ret = execv("/vendor/bin/flashlessd", arg);
        }else if(module == MODULE_TYPE_MUX){
            ret = execv("/vendor/bin/viagsm0710muxd", arg);
        }else if(module == MODULE_TYPE_RIL){
            ret = execv("/vendor/bin/viarild", arg);
        }else if(module == MODULE_TYPE_CTC){
            LOGSTATUS(LOG_ERR,"not support yet");
            //ret = execv("/system/bin/ctclient", arg);
        }
        /* if exec return ,that means child process excev failed*/
        LOGSTATUS(LOG_ERR,"execl child failed ret=%d errno=%d",ret,errno);
        perror ("child");
        exit(0);
    } else    if(statusd_info.process_info[module].process_pid != -1){
        pthread_mutex_lock(&statusd_child_lock);
        gettimeofday(&tv_b, NULL);
        LOGSTATUS(LOG_NOTICE,"This is father. his pid is: %d", getpid());
        statusd_info.process_info[module].module_id = module;
        strcpy(statusd_info.process_info[module].process_name,id2str(module));
        while(statusd_info.process_info[module].process_state != PROCESS_STATE_READY){
            statusd_get_errflag(&flag);
            if(flag !=0){
                LOGSTATUS(LOG_ERR, "err found when waitting %s ready ",id2str(module));
                pthread_mutex_unlock(&statusd_child_lock);
                return -1;
            }
            LOGSTATUS(LOG_NOTICE,"%s not ready,wait state=%d ",
                id2str(module),statusd_info.process_info[module].process_state);

            timeout.tv_sec = tv_b.tv_sec + wati_time+1;
            timeout.tv_nsec = tv_b.tv_usec * 1000;
            ret = pthread_cond_timedwait(&child_ready_signal, &statusd_child_lock, &timeout);
            gettimeofday(&tv_e, NULL);
            timeval = (float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000;
            LOGSTATUS(LOG_NOTICE,"timeval= %d  %f", timeval,(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
            if (statusd_info.process_info[module].process_state != PROCESS_STATE_READY &&
                timeval >= wati_time){
                LOGSTATUS(LOG_NOTICE, "wait %s ready timeout, kill it %d",id2str(module),statusd_info.process_info[module].process_pid);
                viatelGetSdioStatus();
                md_status = viatelGetModemStatus();
                if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
                    LOGSTATUS(LOG_ERR,"md no exception, set error flag");
                    statusd_set_errflag(1);
                    if (module == MODULE_TYPE_RIL){
                        LOGSTATUS(LOG_NOTICE,"c2krild not ready, so reset cbp");
                        statusd_ril_not_ready = 1;
                    }
                }
                pthread_mutex_unlock(&statusd_child_lock);
                return -1;
            }
        }

        statusd_get_errflag(&flag);
        if(flag !=0){
            LOGSTATUS(LOG_ERR, "err found when waitting %s ready ",id2str(module));
            pthread_mutex_unlock(&statusd_child_lock);
            return -1;
        }

        pthread_mutex_unlock(&statusd_child_lock);
        LOGSTATUS(LOG_NOTICE,"okey,%s is ready,next[%f]",id2str(module),(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    }else{
        LOGSTATUS(LOG_NOTICE,"This is father. fork %s failed", id2str(module));
        return -1;
    }
    LOGSTATUS(LOG_DEBUG,"Leave");
    return 0;
}



/*
* Purpose:  handle msg from client
* frame    :   data form client
* Return:    1 if fail, 0 if success
*/

static int handle_msg_flashless(struct Via_Ipc_Data *frame)
{
    unsigned char type = frame->data_type;
    unsigned char event;

    if(type == STATUS_DATATYPE_CMD){
        event = frame->data[0];
        switch(event)
        {
            case CMD_CLIENT_START:
                    statusd_info.process_info[MODULE_TYPE_FLS].process_state = PROCESS_STATE_STARTED;
                    break;
            case CMD_CLIENT_INIT:
                    statusd_info.process_info[MODULE_TYPE_FLS].process_state = PROCESS_STATE_INTIALLING;
                    break;
            case CMD_CLIENT_READY:
                    statusd_info.process_info[MODULE_TYPE_FLS].process_state = PROCESS_STATE_READY;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_ERROR:
                    statusd_info.process_info[MODULE_TYPE_FLS].process_state = PROCESS_STATE_ERROR;
                    LOGSTATUS(LOG_ERR,"CMD_CLIENT_ERROR found on %s",id2str(MODULE_TYPE_FLS));
                    if(ipoflag != 1) {
                        statusd_set_errflag(1);
                    }
                    else{
                        LOGSTATUS(LOG_NOTICE,"IPO OFF ignore CMD_CLIENT_ERROR from flashless");
                    }
                    //kill(statusd_info.process_info[MODULE_TYPE_FLS].process_pid,SIGKILL);
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_EXITING:
                    statusd_info.process_info[MODULE_TYPE_FLS].process_state = PROCESS_STATE_EXITING;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            default :

                    break;
        }
        //statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_FLS,CMD_DATA_ACK);
    }
    return 0;
}

/*
* Purpose:  handle msg from client of muxd
* frame    :   data form client
* Return:    1 if fail, 0 if success
*/
static int handle_msg_mux(struct Via_Ipc_Data *frame)
{
    unsigned char type = frame->data_type;
    unsigned char event;

    if(type == STATUS_DATATYPE_CMD){
        event = frame->data[0];
        switch(event)
        {
            case CMD_CLIENT_START:
                    statusd_info.process_info[MODULE_TYPE_MUX].process_state = PROCESS_STATE_STARTED;
                    break;
            case CMD_CLIENT_INIT:
                    statusd_info.process_info[MODULE_TYPE_MUX].process_state = PROCESS_STATE_INTIALLING;
                    break;
            case CMD_CLIENT_READY:
                    statusd_info.process_info[MODULE_TYPE_MUX].process_state = PROCESS_STATE_READY;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_ERROR:
                    statusd_info.process_info[MODULE_TYPE_MUX].process_state = PROCESS_STATE_ERROR;
                    LOGSTATUS(LOG_ERR,"CMD_CLIENT_ERROR found on %s",id2str(MODULE_TYPE_MUX));
                    statusd_set_errflag(1);
                    //kill(statusd_info.process_info[MODULE_TYPE_MUX].process_pid,SIGKILL);
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_EXITING:
                    statusd_info.process_info[MODULE_TYPE_MUX].process_state = PROCESS_STATE_EXITING;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            default :

                    break;
        }
        //statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_MUX,CMD_DATA_ACK);
    }
    return 0;
}

/*
* Purpose:  handle msg from client of rild
* frame    :   data form client
* Return:    1 if fail, 0 if success
*/
//#define CHECK_STRICT 0
static int handle_msg_ril(struct Via_Ipc_Data *frame)
{
    unsigned char type = frame->data_type;
    unsigned char event;
    int state = -1;
    int md_status;

    if(type == STATUS_DATATYPE_CMD){
        event = frame->data[0];
        switch(event)
        {
            case CMD_CLIENT_START:
                    state = PROCESS_STATE_STARTED;
                    break;
            case CMD_CLIENT_INIT:
#ifdef CHECK_STRICT
        if(statusd_info.process_info[MODULE_TYPE_RIL].process_state != PROCESS_STATE_STARTED){
            state = PROCESS_STATE_ERROR;
            goto terminate;
        }
#else
        state = PROCESS_STATE_INTIALLING;
#endif
                    break;
            case CMD_CLIENT_READY:
#ifdef CHECK_STRICT
                    if(statusd_info.process_info[MODULE_TYPE_RIL].process_state != PROCESS_STATE_INTIALLING){
                        state = PROCESS_STATE_ERROR;
                        goto terminate;
                    }
#else
        state = PROCESS_STATE_READY;
#endif
                    LOGSTATUS(LOG_ERR,"ready received");
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_ERROR:
                    state = PROCESS_STATE_ERROR;
                    LOGSTATUS(LOG_ERR,"CMD_CLIENT_ERROR found on %s",id2str(MODULE_TYPE_RIL));
                    md_status = viatelGetModemStatus();
                    if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
                        LOGSTATUS(LOG_ERR,"md no exception, set error flag");
                        statusd_set_errflag(1);
                        pthread_cond_signal(&child_ready_signal);
                    }
                    break;
            case CMD_CLIENT_EXITING:
                    state = PROCESS_STATE_EXITING;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            default :

                    break;
        }
    }
    if (state == -1)
        LOGSTATUS(LOG_ERR, "ril invalid process state!!");
    statusd_info.process_info[MODULE_TYPE_RIL].process_state = state;

    return 0;

#ifdef CHECK_STRICT
terminate:
    LOGSTATUS(LOG_ERR,"state of ril process erorr from %d to %d",
                    statusd_info.process_info[MODULE_TYPE_RIL].process_state,event);
    statusd_info.process_info[MODULE_TYPE_RIL].process_state = state;

    statusd_set_errflag(1);
    //kill(statusd_info.process_info[MODULE_TYPE_RIL].process_pid,SIGKILL);
#endif
    return 1;
}

/*
* Purpose:  handle msg from client of ctclient
* frame    :   data form client
* Return:    1 if fail, 0 if success
*ctclient data analysis
*Bit0          \A3\BA data bypass
                            0\A3\BA disable bypass
                            1\A3\BA enable bypass
*Bit1          \A3\BAets  bypass
                            0\A3\BA disable bypass
                            1\A3\BA enable bypass
*Bit2          \A3\BAat  bypass
                            0\A3\BA disable bypass
                            1\A3\BA enable bypass
*Bit3          \A3\BAgps  bypass
                            0\A3\BA disable bypass
                            1\A3\BA enable bypass
*Bit4          \A3\BApc voice  bypass
                            0\A3\BA disable bypass
                            1\A3\BA enable bypass
*Bit5~bit15:       reserved
*/
static int handle_msg_ctclient(struct Via_Ipc_Data *frame)
{
    unsigned char type = frame->data_type;
    char  event = 0;
    int state = -1;
    int ret = 0;

    if(type == STATUS_DATATYPE_CMD){
        event = frame->data[0];
        switch(event)
        {
            case CMD_CLIENT_START:
                    state = PROCESS_STATE_STARTED;
                    break;
            case CMD_CLIENT_INIT:
#ifdef CHECK_STRICT
                    if(statusd_info.process_info[MODULE_TYPE_CTC].process_state != PROCESS_STATE_STARTED){
                        state = PROCESS_STATE_ERROR;
                        goto terminate;
                    }
#else
                    state = PROCESS_STATE_INTIALLING;
#endif
                    break;
            case CMD_CLIENT_READY:
#ifdef CHECK_STRICT
                    if(statusd_info.process_info[MODULE_TYPE_CTC].process_state != PROCESS_STATE_INTIALLING){
                        state = PROCESS_STATE_ERROR;
                        goto terminate;
                    }
#else
                    state = PROCESS_STATE_READY;
#endif
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_ERROR:
                    state = PROCESS_STATE_ERROR;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_EXITING:
                    state = PROCESS_STATE_EXITING;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            default :

                    break;
        }
        if (state == -1)
            LOGSTATUS(LOG_ERR, "ctclient invalid process state!!");
        statusd_info.process_info[MODULE_TYPE_RIL].process_state = state;

    }else if(type == STATUS_DATATYPE_DATA){
        event = frame->data[0];
        if(event & 1<<2){/*stop ril and release port which is usd by ril for bypass*/
            ret = statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_RIL,CMD_KILL_CLIENT);
            if(ret !=1){
                LOGSTATUS(LOG_EMERG,"send CMD_KILL_CLIENT to ril failed ret=%d",ret);
            }
        }else{
            if(statusd_info.process_info[MODULE_TYPE_RIL].process_state == PROCESS_STATE_EXITED){
                if(statusd_have_ril){
                    ret = statusd_start_child(MODULE_TYPE_RIL,argv_ril);
                    if(ret !=0){
                        LOGSTATUS(LOG_ERR,"start ril failed after turn off bypass");
                    }
                }
            }
        }
    }
    return 0;

#ifdef CHECK_STRICT
terminate:
        LOGSTATUS(LOG_ERR,"state of ctclient process erorr from %d to %d",
                        statusd_info.process_info[MODULE_TYPE_CTC].process_state,event);
        statusd_info.process_info[MODULE_TYPE_RIL].process_state = state;

        LOGSTATUS(LOG_ERR,"CMD_CLIENT_ERROR found on %s",id2str(MODULE_TYPE_CTC));
        statusd_set_errflag(1);
        //kill(statusd_info.process_info[MODULE_TYPE_CTC].process_pid,SIGKILL);
#endif

    return 1;
}

#ifdef DEBUGGER
static int handle_msg_debugger(struct Via_Ipc_Data *frame)
{
    unsigned char type = frame->data_type;
    char  event = 0;
    int state = -1;
    int ret = 0;
    LOGSTATUS(LOG_EMERG,"Enter");
    if(type == STATUS_DATATYPE_CMD){
        event = frame->data[0];
        switch(event)
        {
            case CMD_CLIENT_START:
                    state = PROCESS_STATE_STARTED;
                    break;
            case CMD_CLIENT_INIT:
                    state = PROCESS_STATE_INTIALLING;
                    break;
            case CMD_CLIENT_READY:
                    state = PROCESS_STATE_READY;
                    pthread_cond_signal(&child_ready_signal);
                    break;
            case CMD_CLIENT_ERROR:
                pthread_cond_signal(&child_ready_signal);
                    state = PROCESS_STATE_ERROR;
                    break;
            case CMD_CLIENT_EXITING:
                pthread_cond_signal(&child_ready_signal);
                    state = PROCESS_STATE_EXITING;
                    break;
            default :
                    break;
        }
        if (state == -1)
            LOGSTATUS(LOG_ERR, "debugger invalid process state!!");
        statusd_info.process_info[MODULE_TYPE_RIL].process_state = state;

    }else if(type == STATUS_DATATYPE_DATA){
        LOGSTATUS(LOG_EMERG,"data frame");
        LOGSTATUS(LOG_EMERG,"%s",frame->data);
        if((strstr(frame->data,"?")!=NULL) || (strstr(frame->data,"-h")!=NULL)){
            LOGSTATUS(LOG_EMERG,"\t-l : debug level [%d]",statusd_log_level);
            LOGSTATUS(LOG_EMERG,"\t-l : log to file %s %s",LOG_FILE_PATH,logtofile?"yes":"no");
            LOGSTATUS(LOG_EMERG,"\t-s fls [%d]",statusd_info.process_info[MODULE_TYPE_FLS].process_state);
            LOGSTATUS(LOG_EMERG,"\t-s mux [%d]",statusd_info.process_info[MODULE_TYPE_MUX].process_state);
            LOGSTATUS(LOG_EMERG,"\t-s ril [%d]",statusd_info.process_info[MODULE_TYPE_RIL].process_state);
            LOGSTATUS(LOG_EMERG,"\t-s ctc [%d]",statusd_info.process_info[MODULE_TYPE_CTC].process_state);
            LOGSTATUS(LOG_EMERG,"\t-k kill");
        }else if(strstr(frame->data,KEY_DEBUG_LEVEL)!=NULL){
            char *ptr=strstr(frame->data,KEY_DEBUG_LEVEL);
            if(ptr !=NULL){
                ptr+=strlen(KEY_DEBUG_LEVEL);
                if((atoi(ptr)>=LOG_EMERG) && (atoi(ptr)<=LOG_DEBUG))
                        statusd_log_level = atoi(ptr);
            }
        }else if((strstr(frame->data,KEY_LOG2FILE)!=NULL)){
            char *ptr=strstr(frame->data,KEY_LOG2FILE);
            int flag = 0;
            time_t tStart;
            if(ptr !=NULL){
                ptr+=strlen(KEY_LOG2FILE);
                if((atoi(ptr)>=0) && (atoi(ptr)<=1))
                        flag = atoi(ptr);
                if(flag !=logtofile){
                    if(logtofile){
                        if (logfile){
                            fclose(logfile);
                        }
                    }else {
                        if ((logfile = fopen(LOG_FILE_PATH, "a+")) == NULL){
                            fprintf(stderr, "Error: %s.\n", strerror(errno));
                            exit(0);
                        }else{
                            fprintf(logfile, "\n################### status log to file %s \n", ctime(&tStart));
                        }
                    }
                    logtofile = flag;
                }
            }
        }else if(strstr(frame->data,KEY_START_FLS)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_START_FLS);
            if(statusd_info.process_info[MODULE_TYPE_FLS].process_state == PROCESS_STATE_EXITED){
                if(statusd_have_flashless){
                    ret = statusd_start_child(MODULE_TYPE_FLS,argv_fls);
                    if(ret !=0){
                        LOGSTATUS(LOG_EMERG,"start fls failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_STOP_FLS)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_STOP_FLS);
            if(statusd_info.process_info[MODULE_TYPE_FLS].process_state != PROCESS_STATE_EXITED){
                if(statusd_have_flashless){
                    ret = statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_FLS,CMD_KILL_CLIENT);
                    if(ret !=1){
                        LOGSTATUS(LOG_EMERG,"send CMD_KILL_CLIENT to fls failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_START_MUX)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_START_MUX);
            if(statusd_info.process_info[MODULE_TYPE_MUX].process_state == PROCESS_STATE_EXITED){
                if(statusd_have_mux){
                    ret = statusd_start_child(MODULE_TYPE_MUX,argv_mux);
                    if(ret !=0){
                        LOGSTATUS(LOG_EMERG,"start mux failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_STOP_MUX)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_STOP_MUX);
            if(statusd_info.process_info[MODULE_TYPE_MUX].process_state != PROCESS_STATE_EXITED){
                if(statusd_have_mux){
                    ret = statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_MUX,CMD_KILL_CLIENT);
                    if(ret !=1){
                        LOGSTATUS(LOG_EMERG,"send CMD_KILL_CLIENT to mux failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_START_RIL)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_START_RIL);
            if(statusd_info.process_info[MODULE_TYPE_RIL].process_state == PROCESS_STATE_EXITED){
                if(statusd_have_ril){
                    ret = statusd_start_child(MODULE_TYPE_RIL,argv_ril);
                    if(ret !=0){
                        LOGSTATUS(LOG_EMERG,"start mux failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_STOP_RIL)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_STOP_RIL);
            if(statusd_info.process_info[MODULE_TYPE_RIL].process_state != PROCESS_STATE_EXITED){
                if(statusd_have_ril){
                    ret = statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_RIL,CMD_KILL_CLIENT);
                    if(ret !=1){
                        LOGSTATUS(LOG_EMERG,"send CMD_KILL_CLIENT to ril failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_START_CTC)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_START_CTC);
            if(statusd_info.process_info[MODULE_TYPE_CTC].process_state == PROCESS_STATE_EXITED){
                if(statusd_have_ctclient){
                    ret = statusd_start_child(MODULE_TYPE_CTC,argv_ctclient);
                    if(ret !=0){
                        LOGSTATUS(LOG_EMERG,"start mux failed ret=%d",ret);
                    }
                }
            }
        }else if(strstr(frame->data,KEY_STOP_CTC)!=NULL){
            LOGSTATUS(LOG_EMERG,"%s",KEY_STOP_CTC);
            if(statusd_info.process_info[MODULE_TYPE_CTC].process_state != PROCESS_STATE_EXITED){
                if(statusd_have_ctclient){
                    ret = statusd_s2csend_cmd(MODULE_TYPE_SR,MODULE_TYPE_CTC,CMD_KILL_CLIENT);
                    if(ret !=1){
                        LOGSTATUS(LOG_EMERG,"send CMD_KILL_CLIENT to ctclient failed ret=%d",ret);
                    }
                }
            }
        }

    }
    return 0;

#ifdef CHECK_STRICT
terminate:
        LOGSTATUS(LOG_ERR,"state of ctclient process erorr from %d to %d",
                        statusd_info.process_info[MODULE_TYPE_DG].process_state,event);
        statusd_info.process_info[MODULE_TYPE_RIL].process_state = state;

        statusd_set_errflag(1);
#endif

    return 1;
}
#endif

/*
* Purpose:  analysis msg from client so that we can understand chich fram client send to server
*            and what dose client need server to do
* Input:      vargp - a pointer to a struct struct socket_info_t*
* Return:    1 if fail, 0 if success
*/
void* statusd_analysis_thread(void *vargp)
{
    struct Via_Ipc_Data *iRev = NULL;
    struct Statusd_Buffer *in_buffer = &statusd_in_buffer;

    LOGSTATUS(LOG_DEBUG,"Enter");
    while(1){
        //statusd_data_pop(&statusd_in_buffer,iRev);
        pthread_mutex_lock(&statusd_newdataready_lock);
        pthread_cond_wait(&statusd_newdata_signal,&statusd_newdataready_lock); /* sleep until signalled by thread_serial_device_read() */
        LOGSTATUS(LOG_DEBUG,"SA1 %d data in buffer",statusd_buffer_count(in_buffer));
        while(1){
            iRev = statusd_get_frame(in_buffer);
            if (iRev == NULL)
                break;
            LOGSTATUS(LOG_NOTICE,"SA2 msg from [%s]/[%s]",
                        id2str(iRev->src_module_id),cmd2str(iRev->data[0]));
            switch(iRev->src_module_id)
            {
                case MODULE_TYPE_FLS:
                    handle_msg_flashless(iRev);
                    break;
                case MODULE_TYPE_MUX:
                    handle_msg_mux(iRev);
                    break;
                case MODULE_TYPE_RIL:
                    handle_msg_ril(iRev);
                    break;
                case MODULE_TYPE_CTC:
                    handle_msg_ctclient(iRev);
                    break;
                case MODULE_TYPE_SR:/*loopback*/
                    break;
            #ifdef DEBUGGER
                case MODULE_TYPE_DG:/*for dynamic debugging*/
                    handle_msg_debugger(iRev);
                    break;
            #endif
                default:
                    LOGSTATUS(LOG_ERR,"msg from unknown client");
                    break;
            }
            statusd_destroy_frame(iRev);
        }
        pthread_mutex_unlock(&statusd_newdataready_lock);


    }

    LOGSTATUS(LOG_DEBUG,"Leave");
}


/*
* Purpose: send data to child process
* Input:        src_modeule_id : data source        :enum VIA_IPC_MODULE
*            dst_module_id    :data destation    :enum VIA_IPC_MODULE
*            type            :data type         :enum DATA_TYPE
*            buffer            :data            :char
*            length            :data length except protocol content
* Return:    data length which had been send to server success except protocol content
*/
int statusd_s2c_comdata(char src_modeule_id,
                                char dst_module_id,
                                unsigned char type,
                                char *buffer,
                                unsigned char length)
{
    unsigned char frame[STATUSD_MAX_DATA_LEN + PROTOCOL_DATA_LEN];
    struct socket_info_t socket_info = statusd_info.socket_info;;
    int client_sockfd = 0;
    struct timeval timeout;
    int md_status;

    int retry = 0;
    fd_set fdw;
    int ret=0;
    int i = 0;

    LOGSTATUS(LOG_DEBUG, "Enter");
    LOGSTATUS(LOG_DEBUG,"src_modeule_id=%d.dst_module_id=%d type=%d length=%d",
        src_modeule_id,dst_module_id,type,length);
    if((src_modeule_id>=MODULE_TYPE_COUNT)
        || (dst_module_id>=MODULE_TYPE_COUNT)
        || (type >= STATUS_DATATYPE_COUNT)
        || (length > STATUSD_MAX_DATA_LEN)){
        LOGSTATUS(LOG_DEBUG,"data struct error::src_modeule_id=%d.dst_module_id=%d type=%d length=%d",
            src_modeule_id,dst_module_id,type,length);
        return -1;
    }

    for(i=0;i<PROCESS_MAX;i++){
        LOGSTATUS(LOG_DEBUG,"dst_module_id=%d [%d] module_id=%d cli_sock_fd=%d",
            dst_module_id,i,socket_info.cli_info[i].module_id,socket_info.cli_info[i].cli_sock_fd);
        if(socket_info.cli_info[i].module_id == dst_module_id){
            client_sockfd = socket_info.cli_info[i].cli_sock_fd;
            break;
        }
    }
    LOGSTATUS(LOG_DEBUG,"okey we get client fd=%d",client_sockfd);
       if((i >= PROCESS_MAX) || (client_sockfd == -1)){
           LOGSTATUS(LOG_ERR, "data to %s fd=%d",name_inquery(dst_module_id),client_sockfd);
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
    com_syslogdump(">ST",frame,length+PROTOCOL_DATA_LEN);

    do{
        LOGSTATUS(LOG_DEBUG, "retry=%d",retry);
        if(retry >= RETRY_TIMES)
            break;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO( &fdw ) ;
        FD_SET( client_sockfd, &fdw ) ;
        ret = select( client_sockfd+1, NULL, &fdw, NULL, &timeout ) ;
        if(ret == 0){
            LOGSTATUS(LOG_DEBUG, "select interface timeout,try again");
            retry++;
        }else if (ret >0) {
            if (FD_ISSET(client_sockfd,&fdw)){
                LOGSTATUS(LOG_DEBUG, "client socket is writeable");
                ret = write(client_sockfd, frame, length+PROTOCOL_DATA_LEN);
                if(ret == length+PROTOCOL_DATA_LEN){
                    LOGSTATUS(LOG_DEBUG, "client socket write finsh ret=%d/%d ",ret,length);
                    break;
                }
                else if (ret != length+6 && errno == EAGAIN){/*if it still not ready ,try again*/
                    LOGSTATUS(LOG_ERR, "client socket write failed,try again ret=%d errno=0x%x",ret,errno);
                    retry ++;
                }else {
                    LOGSTATUS(LOG_ERR, "unknow error,ret=%d [%s]",ret,strerror(errno));
                    md_status = viatelGetModemStatus();
                    if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
                        LOGSTATUS(LOG_ERR,"md no exception, set error flag");
                        statusd_set_errflag(1);
                    }

                    break;
                }
            }
            else{
                LOGSTATUS(LOG_ERR, "why come here");
                perror("select");
                retry ++;
            }
        }else{
            LOGSTATUS(LOG_ERR, "socket error ret=%d [%s]",ret,strerror(errno));
            retry ++;
        }
        usleep(10*1000);
    }while (1);
terminate:
    LOGSTATUS(LOG_DEBUG, "Leave");
    return ret-PROTOCOL_DATA_LEN;
}


/*
* Purpose: send one cmd to server
* Input:        src_modeule_id : data source        :enum VIA_IPC_MODULE
*            dst_module_id    :data destation    :enum VIA_IPC_MODULE
*            cmd                :data type         :enum VIA_IPC_CMD_TYPE
* Return:    data length which had been send to server success except protocol content
*/
int statusd_s2csend_cmd(char src_modeule_id,
                            char dst_module_id,
                            char cmd)
{
    unsigned char buf[2] ={0, 0};

    buf[0] = cmd;
    return statusd_s2c_comdata(src_modeule_id,dst_module_id,STATUS_DATATYPE_CMD,(char*)buf,1);
}

static void statusd_init_struct(void)
{
    int i;

    statusd_info.vpup_state=0;
    for (i=0;i < PROCESS_MAX;i++){
        statusd_info.socket_info.cli_info[i].module_id=-1;
        statusd_info.socket_info.cli_info[i].cli_sock_fd = -1;
        statusd_info.process_info[i].module_id = -1;
        statusd_info.process_info[i].process_pid = -1;
        statusd_info.process_info[i].process_state = PROCESS_STATE_EXITED;
        memset(&statusd_info.process_info[i].start_time ,0,
            sizeof(struct timeval));
        memset(statusd_info.process_info[i].process_name,0,
            sizeof(statusd_info.process_info[i].process_name));
    }
}

/*
* Purpose:  init communicatin interface between client and server
* Input:      void
* Return:    1 if fail, 0 if success
*/
static int statusd_init_com(void)
{
    int server_sockfd;
    int    ret ;
    pthread_t statusd_com_thread_id;
    pthread_t statusd_analysis_thread_id;

    statusd_init_struct();
    ret = unlink(SOCKETNAME);
    if(ret!=0){
        LOGSTATUS(LOG_WARNING,"unlink failed ret=%d [%s]",ret,strerror(errno));
    }
#if 0
    server_sockfd = socket( AF_UNIX, SOCK_STREAM, 0 ) ;
    if(server_sockfd==-1){
        LOGSTATUS(LOG_ERR,"creat socket failed ret=%d [%s]\n",server_sockfd,strerror(errno));
        return -1;
    }
    server_sockaddr.sun_family = AF_UNIX ;
//    server_sockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
//    server_sockaddr.sin_port=htons(9734);
    strcpy( server_sockaddr.sun_path, SOCKETNAME ) ;
    server_len=sizeof(server_sockaddr);
    ret = bind( server_sockfd, ( struct sockaddr * )&server_sockaddr, server_len ) ;
    if(ret !=0){
        LOGSTATUS(LOG_ERR,"bind socket failed ret=%d [%s]\n",ret,strerror(errno));
    }
#else
    server_sockfd = android_get_control_socket("statusd");
    if(server_sockfd==-1){
        LOGSTATUS(LOG_ERR,"get socket failed ret=%d [%s]\n",server_sockfd,strerror(errno));
        return -1;
    }
#endif
    ret = listen( server_sockfd, PROCESS_MAX ) ;
    if(ret !=0){
        LOGSTATUS(LOG_ERR,"listen socket failed ret=%d [%s]\n",ret,strerror(errno));
    }
    statusd_info.socket_info.server_sockfd = server_sockfd;
    statusd_info.socket_info.maxfd = server_sockfd;
    chown(SOCKETNAME, AID_RADIO, AID_RADIO);
    if (chmod(SOCKETNAME, 0660))
        LOGSTATUS(LOG_ERR,"fail to chmod");
    if(create_thread(&statusd_com_thread_id, statusd_com_thread,(void*) &statusd_info)!=0){ //create thread for reading input from virtual port
      LOGSTATUS(LOG_ERR,"Could not create thread for listening on socket");
      return -1;
    }

    if(create_thread(&statusd_analysis_thread_id, statusd_analysis_thread,(void*) &statusd_info)!=0){ //create thread for reading input from virtual port
      LOGSTATUS(LOG_ERR,"Could not create thread for listening on socket");
      return -1;
    }
    return 0;
}


static int find_arg(char *src_arg,
                    char **dst_arg,
                    char *key,
                    int src_len)
{
    char argv[255]={0};
    char *ptr = strstr(src_arg,key);
    char *start_pos,*end_pos,*now_pos;
    int len =0;
    char *right_mark;
    char *tmp_src;
    int i;

    LOGSTATUS(LOG_DEBUG,"ENTER");
    if(ptr){
        memcpy(argv,ptr,src_len-(ptr-src_arg));/*find first key*/
        len = src_len-(ptr-src_arg);
    }else{
        LOGSTATUS(LOG_INFO,"statusd cann't find %s argument",key);
        return 0;
    }
    start_pos = strchr(argv,'[');/*find key char '[',it is the start of the param*/
    if(start_pos){
        now_pos = ++start_pos ;
        len --;
        while((*now_pos == ' ') && (--len>0)){/*skip empty char*/
            now_pos ++;
        }
        start_pos = now_pos;
        end_pos = strchr(argv,']');/*find key char ']',it is the end of the param*/
        right_mark = end_pos;
        len = end_pos - start_pos + 1;/*add an end flag \0 in the end of the param*/
        if(end_pos && len>0){
            now_pos = --end_pos;/*delete ']'*/
            len --;
            while((*now_pos == ' ') && (--len>0)){/*skip empty char*/
                now_pos --;
            }
            end_pos = now_pos;
            len = end_pos - start_pos + 1;
            *dst_arg=malloc(len+1);
            if(*dst_arg ==NULL){
                return 0;
            }
            memset(*dst_arg,0,len+1);
            memcpy(*dst_arg,start_pos,len);
            len = right_mark-argv+1;
            tmp_src = ptr+len;
            for (i=0; i<(src_len-(ptr-src_arg)-len+1); i++)
            	ptr[i] = tmp_src[i];
        }else{
            LOGSTATUS(LOG_ERR,"invalid %s argument,']' ",key);
            return 0;
        }
    }else{
        LOGSTATUS(LOG_ERR,"invalid %s argument,'[' no found",key);
        return 0;
    }
    LOGSTATUS(LOG_DEBUG,"LEAVE ");
    return 1;
}

/*
* Purpose:  clear child process information if one child process had been exited
* Input:     void
* Return:    -
*/
void handle_child_exit(void)
{
    pid_t pid;
    int stat;
    int i = 0;
    char id = 0;
    int ret = 0;
    int err = 0;

    if((pid = waitpid(-1,&stat,0)) > 0)
    {
        LOGSTATUS(LOG_NOTICE,"this is pid=%d",getpid());
        if(WIFEXITED(stat)){
            LOGSTATUS(LOG_NOTICE,"child %d exit(%d)WIFEXITED",pid,WEXITSTATUS(stat));
        }else if(WIFSIGNALED(stat)){
            LOGSTATUS(LOG_NOTICE,"child %d exit signal=%d(WTERMSIG)",pid,WTERMSIG(stat));
        }else if(WIFSTOPPED(stat)){
            LOGSTATUS(LOG_NOTICE,"child %d exit signal=%d(WSTOPSIG)",pid,WSTOPSIG(stat));
        }else{
            LOGSTATUS(LOG_NOTICE,"child unknow exit");
        }

        for (i=0;i < PROCESS_MAX;i++){
            LOGSTATUS(LOG_DEBUG,"pid=%d statusd_info.process_info[%d].process_pid=%d",pid,i,statusd_info.process_info[i].process_pid);
            if(pid == statusd_info.process_info[i].process_pid){
                LOGSTATUS(LOG_DEBUG,"ok,equal");
                LOGSTATUS(LOG_DEBUG,"statusd_info.process_info[%d].module_id=%d",i,statusd_info.process_info[i].module_id);
                statusd_child_err =1;
                if (i == MODULE_TYPE_RIL){
                    LOGSTATUS(LOG_NOTICE,"c2krild exit, so reset cbp");
                    statusd_ril_exit = 1;
                }
                id = statusd_info.process_info[i].module_id;
                LOGSTATUS(LOG_DEBUG,"id=%d statusd_info.process_info[%d].module_id=%d",id,i,statusd_info.process_info[i].module_id);
                statusd_info.process_info[i].module_id = -1;
                //statusd_info.process_info[i].process_pid = -1;
                statusd_info.process_info[i].process_state = PROCESS_STATE_EXITED;
                memset(&statusd_info.process_info[i].start_time ,0,
                    sizeof(struct timeval));
                memset(statusd_info.process_info[i].process_name,0,
                    sizeof(statusd_info.process_info[i].process_name));
                break;
            }
        }
        if(i>=PROCESS_MAX){
            LOGSTATUS(LOG_ERR,"unknown child process exit");
            goto terminate;
        }else{
            LOGSTATUS(LOG_NOTICE,"child(pid=%d) %s exit",pid,id2str(id));
            pthread_cond_signal(&child_exit_signal);
        }
        for (i=0;i<PROCESS_MAX;i++){
            if(statusd_info.socket_info.cli_info[i].module_id == id){
                break;
            }
        }
        clear_client_info((struct socket_info_t *)&statusd_info.socket_info,id);
    }else{
        err = errno;
        LOGSTATUS(LOG_NOTICE,"wait child exit info error pid=%d [%s][%d]",pid,strerror(err), err);
        if(err != ECHILD) {
            LOGSTATUS(LOG_NOTICE,"die myself");
            close (statusd_info.socket_info.server_sockfd);
            statusd_info.socket_info.maxfd =-1;
            statusd_info.socket_info.server_sockfd =-1;
            statusd_free_child_para();
            ret=unlink(SOCKETNAME);
            if(ret!=0){
                LOGSTATUS(LOG_ERR,"unlink failed ret=%d [%s]",ret,strerror(errno));
            }else{
                LOGSTATUS(LOG_NOTICE,"unlink SUCCESS ret=%d [%s]",ret,strerror(errno));
            }
            LOGSTATUS(LOG_NOTICE,"reset cbp");
            viatelModemReset();/*statusd exit or killed,power off modem*/
            exit(-1);
        }
    }

terminate:

    return;
}


/*
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:      param - signal ID
* Return:    -
*/
void status_signal_treatment(
    int param)
{
    struct socket_info_t *socket_info = &statusd_info.socket_info;;
    int i=0;
    int ret=-1;
    signal(SIGCHLD, status_signal_treatment);
    LOGSTATUS(LOG_DEBUG,"Enter");
    switch (param){
    case SIGPIPE:
        LOGSTATUS(LOG_EMERG,"SIGPIPE");
        break;
    case SIGCHLD:
        if(ipoflag != 1) {
            LOGSTATUS(LOG_EMERG,"SIGCHLD");
            handle_child_exit();
        }
        else{
            LOGSTATUS(LOG_NOTICE,"IPO OFF ignore SIGCHLD");
        }
        break;
    case SIGTERM:
        LOGSTATUS(LOG_EMERG,"SIGTERM");
        /*fall through*/
    case SIGKILL:
        LOGSTATUS(LOG_EMERG,"SIGKILL");
        /*fall through*/
    case SIGSEGV:
        LOGSTATUS(LOG_EMERG,"SIGSEGV %d",getpid());
        /*fall through*/
    default:
        LOGSTATUS(LOG_EMERG,"default");
        /*This property may be used by other module to check whether modem is ready*/
        property_set("vendor.net.cdma.mdmstat", "not ready");
        for ( i = 0; i < PROCESS_MAX; i++ ) {
            if(socket_info->cli_info[i].cli_sock_fd !=-1){
                close(socket_info->cli_info[i].cli_sock_fd);
                socket_info->cli_info[i].cli_sock_fd = -1;
            }
        }
        close (statusd_info.socket_info.server_sockfd);
        statusd_info.socket_info.maxfd =-1;
        statusd_info.socket_info.server_sockfd =-1;
        statusd_free_child_para();
        ret=unlink(SOCKETNAME);
        if(ret!=0){
            LOGSTATUS(LOG_ERR,"unlink failed ret=%d [%s]",ret,strerror(errno));
        }else{
            LOGSTATUS(LOG_NOTICE,"unlink SUCCESS ret=%d [%s]",ret,strerror(errno));
        }
        viatelModemReset();
        exit(-1);
        break;
    }
    LOGSTATUS(LOG_DEBUG,"exit");

}

void status_signal_treatment_wrapper(int para,
					__attribute__((unused))struct siginfo * dummy0,
					__attribute__((unused))void* dummy1) /* Fix for build waring */
{
	status_signal_treatment(para);
}

void sync_with_md1_for_reset()
{
    char property_val[PROPERTY_VALUE_MAX] = {0};
    int cnt = 0;

    if(!need_md1_md3_reset_sync)
        return;

    property_get(MD3_STATUS_SYNC_PROP, property_val, NULL);
    LOGSTATUS(LOG_NOTICE,"step 0: %s is %s", MD3_STATUS_SYNC_PROP, property_val);

    property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_RST]);
    do {
        property_get(MD1_STATUS_SYNC_PROP, property_val, NULL);
        if(strcmp(property_val, md_status_sync_str[MD_STATUS_SYNC_RST])==0) {
            LOGSTATUS(LOG_NOTICE,"step 1: %s is %s", MD1_STATUS_SYNC_PROP, property_val);
            break;
        }
        cnt++;
        if((cnt%50)==0)
            LOGSTATUS(LOG_NOTICE,"step 1: %s is %s", MD1_STATUS_SYNC_PROP, property_val);
        usleep(30*1000);
    } while(1);

    viatelModemResetPCCIF();

    property_set(MD1_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_RST_START]);
    do {
        property_get(MD3_STATUS_SYNC_PROP, property_val, NULL);
        if(strcmp(property_val, md_status_sync_str[MD_STATUS_SYNC_RST_START])==0) {
            LOGSTATUS(LOG_NOTICE,"step 2: %s is %s", MD3_STATUS_SYNC_PROP, property_val);
            break;
        }
        cnt++;
        if((cnt%50)==0)
            LOGSTATUS(LOG_NOTICE,"step 2: %s is %s", MD3_STATUS_SYNC_PROP, property_val);
        usleep(30*1000);
    } while(1);
}

void status_sig_handle(int signum, siginfo_t *info, __attribute__((unused))void *data)
{
    char property_val[PROPERTY_VALUE_MAX] = {0};
    int cnt = 0;
    int flag = 0;
    LOGSTATUS(LOG_NOTICE,"PID:%d receive SIG=%d, No=%d, code=%d, val=0x%lx",
                    getpid(), signum, info->si_signo, info->si_code, info->si_band);

    switch(info->si_code)
    {
        case ASC_USER_USB_WAKE:
            break;
        case ASC_USER_USB_SLEEP:
            break;
        case ASC_USER_UART_WAKE:
            break;
        case ASC_USER_UART_SLEEP:
            break;
        case ASC_USER_SDIO_WAKE:
            break;
        case ASC_USER_SDIO_SLEEP:
            break;
        case ASC_USER_MDM_POWER_ON:
            break;
        case ASC_USER_MDM_POWER_OFF:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_POWER_OFF");
            property_set("vendor.net.cdma.mdmstat", "not ready");
            if(need_md1_md3_reset_sync) {
                property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_STOP]);
                do {
                    property_get(MD1_STATUS_SYNC_PROP, property_val, NULL);
                    if((strcmp(property_val, md_status_sync_str[MD_STATUS_SYNC_FLIGHT_MODE])==0) ||
                        (strcmp(property_val, md_status_sync_str[MD_STATUS_SYNC_RST])==0)) {
                        LOGSTATUS(LOG_NOTICE,"OK: %s is %s", MD1_STATUS_SYNC_PROP, property_val);
                        break;
                    }
                    cnt++;
                    if((cnt%50)==0)
                        LOGSTATUS(LOG_NOTICE,"Need retry: %s is %s", MD1_STATUS_SYNC_PROP, property_val);
                    usleep(30*1000);
                } while(1);

                viatelModemResetPCCIF();
            }
            break;
        case ASC_USER_MDM_RESET_ON:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_RESET_ON");
            LOGSTATUS(LOG_NOTICE,"cbp reset occured");
            /*This property may be used by other module to check whether modem is ready*/
            property_set("vendor.net.cdma.mdmstat", "not ready");
            /*signal send by kernel must be quicker than error found by
             * child process(ex:flasslessd or viarild) *
             * if not,there may some problems,       *
             * ex:modem will be reset once more    */
            if(statusd_child_err==0){
                statusd_child_err =1;
            }
            else{
                LOGSTATUS(LOG_NOTICE,"child process had already found errs,and modem's recovery is on the way or had been done.");
            }
            statud_mdm_reset = 0;
            pthread_cond_signal(&statusd_rstind_signal);
            break;
        case ASC_USER_MDM_RESET_ON_SINGLE:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_RESET_ON_SINGLE");
            LOGSTATUS(LOG_NOTICE,"cbp reset occured, ignore MD1");
            statusd_get_reset_flag(&flag);
            if (flag) {
            	LOGSTATUS(LOG_NOTICE,"cbp reset is ongoing, ignore reset_on_single sig");
            } else {
                property_set("vendor.net.cdma.mdmstat", "not ready");
                if(statusd_child_err==0){
                    statusd_child_err =1;
                } else{
                    LOGSTATUS(LOG_NOTICE,"child process had already found errs,and modem's recovery is on the way or had been done.");
                }
                statud_mdm_reset = 0;
                statud_mdm_reset_single = 1;
                pthread_cond_signal(&statusd_rstind_signal);
            }
            break;
        case ASC_USER_MDM_RESET_OFF:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_RESET_OFF");
            statud_mdm_reset ++;
            break;
        case ASC_USER_MDM_ERR:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_ERR");
            if(statusd_child_err==0)
                statusd_child_err =1;
            if(statusd_mdm_error==0)
                statusd_mdm_error =1;
            else{
                LOGSTATUS(LOG_NOTICE,"err had already been found,and modem's recovery is on the way or had been done.");
            }
            break;
        case ASC_USER_MDM_ERR_ENHANCE:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_ERR_ENHANCE");
            statusd_ramdump_cbp =1;
            if(statusd_child_err==0){
                statusd_child_err =1;
                LOGSTATUS(LOG_NOTICE,"let cbp die and catch ramdup");
                viatelModemDie();
            }
            else{
                LOGSTATUS(LOG_NOTICE,"err had already been found,and modem's recovery is on the way or had been done.");
            }
            break;
        case ASC_USER_MDM_IPOH:
            property_set("vendor.net.cdma.mdmstat", "not ready");
            if(need_md1_md3_reset_sync)
                property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_STOP]);
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_IPOH");
            /*
            LOGSTATUS(LOG_NOTICE,"power off modem");
            viatelModemPower(0);
            sleep(1);
            LOGSTATUS(LOG_NOTICE,"die myself");
            exit(-1);
            */
            break;
        case ASC_USER_MDM_WDT:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_WDT");
            LOGSTATUS(LOG_NOTICE,"cbp wdt occured");
            statusd_get_reset_flag(&flag);
            if (flag) {
            	LOGSTATUS(LOG_NOTICE,"cbp reset is ongoing, ignore wdt reset sig");
            } else {
                /*This property may be used by other module to check whether modem is ready*/
                property_set("vendor.net.cdma.mdmstat", "not ready");
                /*signal send by kernel must be quicker than error found by
                 * child process(ex:flasslessd or viarild) *
                 * if not,there may some problems,       *
                 * ex:modem will be reset once more    */
                if(statusd_child_err==0){
                    statusd_child_err =1;
                }
                else{
                    LOGSTATUS(LOG_NOTICE,"child process had already found errs,and modem's recovery is on the way or had been done.");
                }
                statud_mdm_reset++;    //when wdt happend, c2k is waiting for ap to reset it, so we need to set this flag.
                pthread_cond_signal(&statusd_rstind_signal);
            }
            break;
        case ASC_USER_MDM_EXCEPTION:
            LOGSTATUS(LOG_NOTICE,"ASC_USER_MDM_EXCEPTION");
            property_set("vendor.net.cdma.mdmstat", "exception");
            if(need_md1_md3_reset_sync)
                property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_EXP]);
            need_silent_reboot = 1;
            break;
        default:
            LOGSTATUS(LOG_NOTICE, "sig_handle unknow command %x", info->si_code);
    }
}



/*
* Purpose:  Allocates memory for a new buffer and initializes it.
* Input:      -
* Return:    pointer to a new buffer
*/
static int statusd_inbuffer_init(struct Statusd_Buffer *buf)
{
    if (buf){
        pthread_mutex_lock(&statusd_datacount_lock);
        memset(buf, 0, sizeof(struct Statusd_Buffer));
        buf->readp = buf->data;
        buf->writep = buf->data;
        buf->endp = buf->data + STATUSD_BUFFER_SIZE;
        pthread_mutex_unlock(&statusd_datacount_lock);
    }
    return 0;
}

#if 0
/*
* Purpose:  Compares two strings.
*                strstr might not work because WebBox sends garbage before the first OK
*                when it's not needed anymore
* Input:      haystack - string to check
*                length - length of string to check
*                needle - reference string to compare to. must be null-terminated.
* Return:    1 if comparison was success, else 0
*/
static int memstr(
    const char *haystack,
    int length,
    const char *needle)
{
    int i;
    int j = 0;
    if (needle[0] == '\0')
        return 1;
    for (i = 0; i < length; i++){
        if (needle[j] == haystack[i]){
            j++;
            if (needle[j] == '\0') // Entire needle was found
                return 1;
        }
        else
            j = 0;
    }
    return 0;
}
#endif

static int statusd_wait_cbp_up(int wait_time )
{
    time_t begin_time,current_time;
    struct timeval tv_b,tv_e;
    unsigned char buf[1024];
    struct timeval timeout;
    struct termios ios;
    char* atDevPath=NULL;
    fd_set rfds;
    int sel=-1;
    int ret = 0;
    int len=0,n=0;
    int fd=-1;
    char c;
    int set_attr_fail_cnt = 0;

    LOGSTATUS(LOG_DEBUG, "ENTER");
    gettimeofday(&tv_b, NULL);
    time(&begin_time); //get the current time

retry:
    while(fd < 0 && atDevPath == NULL){
        atDevPath = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_AT);
        if(atDevPath != NULL){
            LOGSTATUS(LOG_DEBUG,"atDevPath=%s",atDevPath);
            fd = open(atDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
            if(fd < 0){
                LOGSTATUS(LOG_ERR, "open at device %s error fd=%d",atDevPath,fd);
            }else{/*okey*/
                break;
            }
        }
        time(&current_time); //get the current time
        if(current_time - begin_time > wait_time){
            LOGSTATUS(LOG_ERR, "open at device %s error fd=%d",atDevPath,fd);
            goto terminate;
        }
        usleep(1000*50);
    }
    /* LOGSTATUS(LOG_NOTICE, "Configured serial device %s fd=%d",atDevPath,fd); */

    tcgetattr(fd, &ios);
    ios.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
    ios.c_cflag |= CREAD | CLOCAL | CS8 ;
    ios.c_cflag &= ~(CRTSCTS);
    ios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    ios.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
    ios.c_iflag &= ~(IXON | IXOFF);
    ios.c_oflag &= ~(OPOST | OCRNL);
    ios.c_cc[VMIN] = 1;
    ios.c_cc[VTIME] = 0;
    cfsetispeed( &ios, B115200);
    cfsetospeed( &ios, B115200);
    //tcflush( fd, TCIFLUSH );

    if(tcsetattr(fd, TCSANOW, &ios) < 0) {// affect at now
        if ((set_attr_fail_cnt % 1000) == 0)
            LOGSTATUS(LOG_ERR,"setPort: ERROR SET ATTR fd=%d [%s]",fd,strerror(errno));
        set_attr_fail_cnt++;
        goto terminate;
    }

    memset(buf, 0, sizeof(buf));
    do{
        FD_ZERO( &rfds );
        FD_SET( fd, &rfds );
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        sel = select(fd + 1, &rfds, NULL, NULL, &timeout);
        LOGSTATUS(LOG_DEBUG, "Selected %d", sel);
        if(sel > 0){//read data
            if (FD_ISSET(fd, &rfds)){
                len = read(fd, &c, 1);
                if((len <=0) && (errno != EINTR)){
                    LOGSTATUS(LOG_ERR, "read error ret=%d [%s]",len,strerror(errno));
                    goto terminate;
                }
                buf[n] = c;
                n +=len;
                errno = 0;

                if (strstr((char *) buf,"+VPUP") != NULL){
                    gettimeofday(&tv_e, NULL);
                    LOGSTATUS(LOG_NOTICE, "okey got vpup[%f]",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
                    statusd_info.vpup_state=1;
                    /*This property may be used by other module to check whether modem is ready*/
                    property_set("vendor.net.cdma.mdmstat", "ready");
                    if(need_md1_md3_reset_sync)
                        property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_RDY]);
                    need_silent_reboot = 0;
                    viatelModemReady();
                    ret = 1;
                    break;
                }
            }
        }else if(sel == 0){
            LOGSTATUS(LOG_DEBUG, "read select timeout\n");
        }else{
            LOGSTATUS(LOG_ERR, "read select error '%s' (code: %d)\n", strerror(errno), errno);
            break;
        }
        time(&current_time); //get the current time
    } while (current_time - begin_time < wait_time);
terminate:
    /* LOGSTATUS(LOG_NOTICE, "Read %d bytes from cbp:[%s]", n,buf+2); */
    time(&current_time); //get the current time
    if((ret == 0) && (current_time - begin_time < wait_time)){
        usleep(1000*50);
        goto retry;
    }

    if (fd >= 0)
        close(fd);
    fd = -1;

    LOGSTATUS(LOG_DEBUG, "LEAVE");

    return ret ;
}

static int usage(char *_name)
{
    int i=0;

    LOGSTATUS(LOG_EMERG,"Called [%s]with following options:",_name);
    LOGSTATUS(LOG_EMERG,"\t-v: verbose logging level(0(Silent)-7(Debug)):\t[%d]",statusd_log_level);
    LOGSTATUS(LOG_EMERG, "\t-l: Log to file %s: \t\t[%s]", LOG_FILE_PATH, logtofile?"yes":"no");
    LOGSTATUS(LOG_EMERG, "\t-f: Start Flashless \t\t[%s]", statusd_have_flashless?"yes":"no");
    if(statusd_have_flashless){
        for(i=0;i<statusd_have_flashless;i++)
        LOGSTATUS(LOG_EMERG, "\t\t: Flashless arg[%d][%s]", i,argv_fls[i]);
    }
    LOGSTATUS(LOG_EMERG, "\t-m: Start viagsm0710muxd \t[%s]", statusd_have_mux?"yes":"no");
    if(statusd_have_mux){
        for(i=0;i<statusd_have_mux;i++)
            LOGSTATUS(LOG_EMERG, "\t\t: mux arg[%d][%s]",i,argv_mux[i]);
    }
    LOGSTATUS(LOG_EMERG, "\t-r: Start viaril \t\t\t[%s]", statusd_have_ril?"yes":"no");
    if(statusd_have_ril){
        for(i=0;i<statusd_have_ril;i++)
            LOGSTATUS(LOG_EMERG, "\t\t: viaril arg[%d][%s]",i, argv_ril[i]);
    }
    LOGSTATUS(LOG_EMERG, "\t-c: Start ctclient \t\t\t[%s]", statusd_have_ctclient?"yes":"no");
    if(statusd_have_ctclient){
        for(i=0;i<statusd_have_ctclient;i++)
            LOGSTATUS(LOG_EMERG, "\t\t: ctclient arg[%d][%s]", i,argv_ctclient[i]);
    }
    //help
    LOGSTATUS(LOG_EMERG, "\t-h: Show this help message and show current settings.");
    return -1;
}


/*
* Purpose:  get child process parameter
* Input:      argc - number of input arguments from main
*                argv - array of strings (input arguments) from main
* Return:     0  : successd
*            -1: failed
*test code
*        for(i=0;i<arg_idx;i++)
*            LOGSTATUS(LOG_DEBUG,"argv_fls[%d]=%s",
*                    i,argv_fls[i]);
*/
static int statusd_analysis_para(int argc, char **argv)
{
    char *argv_st[STATUSD_PARSER_MAXARGS]={NULL};
    char *next = NULL;
    char *bword= NULL;
    char *argv_als;
    int arg_idx = 0;
    int arg_len = 0;
    int ret = 0;
    int opt;
    int i=0;
    int left;
    int curr_arg_len;

    /*step1.1. save all argument in argv_als */
    argv_als = malloc(STATUSD_PARSER_MAXARGS*64);
    if (argv_als == NULL) {
        LOGSTATUS(LOG_ERR,"allock args parser memory fail!");
        return -1;
    }
    left = STATUSD_PARSER_MAXARGS*64;
    memset(argv_als,0,STATUSD_PARSER_MAXARGS*64);
    for (i=0; i < argc ;i++) {
        curr_arg_len = strlen(argv[i]);
    	if ((curr_arg_len + 2) > left) {
    	    LOGSTATUS(LOG_ERR,"memory not enough, para list abnormal");
    	    free(argv_als);
    	    return -1;
        }
        strcat(argv_als+arg_len," ");
        strcat(argv_als+arg_len,argv[i]);
        arg_len += curr_arg_len + 1;
        left = left - curr_arg_len - 1;
    }

    /*step1.2get flashless argument if we have ,it begins as "-f"*/
    ret = find_arg(argv_als,&next,"-f",arg_len);
    LOGSTATUS(LOG_INFO,"flashless arg=%s",next);
    if(ret != 0){
        /*step1.3  setup flashless argument struct*/
        while((bword = strsep(&next, " "))) {
            if(*bword == 0)
                continue;
            argv_fls[arg_idx++] = bword;
            if (arg_idx == STATUSD_PARSER_MAXARGS - 1)
                break;
        }
        argv_fls[arg_idx++] = next;/*set last element null*/
        statusd_have_flashless = arg_idx;
        arg_idx = 0;
    }else{
        LOGSTATUS(LOG_INFO,"invalid argument for flashless");
    }

    /*step1.4get mux argument if we have ,it begins as "-m"*/
    ret = find_arg(argv_als,&next,"-m",arg_len);
    LOGSTATUS(LOG_INFO,"mux arg=%s",next);
    if(ret != 0){
        /*step1.5  setup flashless argument struct*/
        while((bword = strsep(&next, " "))) {
            if(*bword == 0)
                continue;
            argv_mux[arg_idx++] = bword;
            if (arg_idx == STATUSD_PARSER_MAXARGS - 1)
                break;
        }
        argv_mux[arg_idx++] = next;/*set last element null*/
        statusd_have_mux = arg_idx;
        arg_idx = 0;
    }else{
        LOGSTATUS(LOG_INFO,"invalid argument for mux");
    }

    /*step1.6 get ril argument if we have ,it begins as "-r"*/
    ret = find_arg(argv_als,&next,"-r",arg_len);
    LOGSTATUS(LOG_INFO,"ril arg=%s",next);
    if(ret != 0){
        /*step1.7  setup flashless argument struct*/
        while((bword = strsep(&next, " "))) {
            if(*bword == 0)
                continue;
            argv_ril[arg_idx++] = bword;
            if (arg_idx == STATUSD_PARSER_MAXARGS - 1)
                break;
        }
        argv_ril[arg_idx++] = next;/*set last element null*/
        statusd_have_ril = arg_idx;
        arg_idx = 0;
    }else{
        LOGSTATUS(LOG_ERR,"invalid argument for ril");
    }
    /*step1.7 get ctclient argument if we have ,it begins as "-c"*/
    ret = find_arg(argv_als,&next,"-c",arg_len);
    LOGSTATUS(LOG_INFO,"ctclient arg=%s",next);
    if(ret != 0){
        LOGSTATUS(LOG_DEBUG,"ctclient arg=%s",next);
        /*step1.7  setup flashless argument struct*/
        while((bword = strsep(&next, " "))) {
            if(*bword == 0)
                continue;
            argv_ctclient[arg_idx++] = bword;
            if (arg_idx == STATUSD_PARSER_MAXARGS - 1)
                break;
        }
        argv_ctclient[arg_idx++] = next;/*set last element null*/
        statusd_have_ctclient = arg_idx;
        arg_idx = 0;
    }else{
        LOGSTATUS(LOG_INFO,"invalid argument for ctclient");
    }

    while((bword = strsep(&argv_als, " "))) {
        if(*bword == 0)
            continue;
        argv_st[arg_idx++] = bword;
        if (arg_idx == STATUSD_PARSER_MAXARGS - 1)
            break;
    }
    LOGSTATUS(LOG_ERR,"arg_idx=%d",arg_idx);
    for(i=0;i<arg_idx;i++){
        LOGSTATUS(LOG_ERR,"argv_st[%d]=%s arg_idx=%d",i,argv_st[i],arg_idx);
    }
    argv_st[arg_idx] = NULL;
    while ((opt = getopt(arg_idx, argv_st, "lv:h?")) > 0)
    {
        switch (opt)
        {
            case 'l':
                if(logtofile==0){
                    logtofile = 1;
                    if ((logfile = fopen(LOG_FILE_PATH, "a+")) == NULL){
                        LOGSTATUS(LOG_ERR,"%s open error",LOG_FILE_PATH);
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(0);
                    }else{
                        time ( &rawtime );
                        timeinfo = localtime ( &rawtime );
                        strftime ( timE,80,"%Y-%m-%d %I:%M:%S",timeinfo);
                        if (chmod(LOG_FILE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
                            LOGSTATUS(LOG_ERR,"fail to chmod");
                        fprintf(logfile, "\n\n########### statusd log %s ###########\n", timE);
                    }
                }
                break;
            case 'v':
                statusd_log_level = atoi(optarg);
                if ((statusd_log_level>LOG_DEBUG) || (statusd_log_level < 0)){
                    usage(argv[0]);
                     exit(0);
                }
                break;
            default:
            case '?':
            case 'h':
                usage(argv[0]);
                exit(0);
                break;
        }
    }
    free(argv_als);
    usage("print arg");
    return 0;
}

static int statusd_free_child_para()
{

    LOGSTATUS(LOG_DEBUG,"Enter\n");

    if(argv_fls[0] != NULL){
        free(argv_fls[0]);
    }
    if(argv_mux[0] !=NULL){
        free(argv_mux[0]);
    }
    if(argv_ril[0] !=NULL){
        free(argv_ril[0]);
    }
    if(argv_ctclient[0] !=NULL){
        free(argv_ctclient[0]);
    }

    LOGSTATUS(LOG_DEBUG,"exit\n");

    return 0;
}

static int statusd_vmodem_init(int *vm_fd)
{
    int fd=0;
    int flags = 0;

    fd = open(VMODEM_DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        LOGSTATUS(LOG_ERR,"Make sure the vmodem is existed or not opened by others\n");
        return -ENODEV;
    }

    flags = fcntl(fd, F_SETOWN, getpid());
    if (flags == -1)
    {
        LOGSTATUS(LOG_ERR,"fcntl failed");
    }
    flags = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);
    if (flags == -1)
    {
        LOGSTATUS(LOG_ERR,"fcntl failed");
    }
    flags = fcntl(fd, F_SETSIG, SIGRTMIN);
    if (flags == -1)
    {
        LOGSTATUS(LOG_ERR,"fcntl failed");
    }
    *vm_fd= fd;
    return 0;
}


static int statusd_enable_ctlch(int sw)
{
    int fd = -1;
    ssize_t ret;

    fd = open(VMODEM_DEVICE_PATH, O_RDWR);
    if(fd < 0){
        LOGSTATUS(LOG_ERR,"Fail to open device %s.\n", VMODEM_DEVICE_PATH);
        return -1;
    }
    sw = !!sw;
    ret = ioctl(fd, CMDM_IOCTL_CRL, &sw);
    if(ret < 0){
        LOGSTATUS(LOG_ERR,"Fail to power %s vmodem.\n", sw?"on":"off");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

/*
 * switchUser - Switches UID to radio, preserving CAP_NET_ADMIN capabilities.
 * Our group, cache, was set by init.
 */
static void switch_user()
{
    uid_t uid = getuid();

    if(uid==0){/*if it is root*/
        LOGSTATUS(LOG_ERR,"current uid=%d change to %d",uid,AID_RADIO);
        setuid(AID_RADIO);
    }
}

void* statusd_test_thread(__attribute__((unused))void *vargp)
{
    time_t begin_time,current_time;
    unsigned char buf[1024];
    struct timeval timeout;
    struct termios ios;
    char* atDevPath="/dev/ttyUSB1";
    fd_set rfds;
    int sel;
    int ret = 0;
    int len;
    int fd;

    LOGSTATUS(LOG_DEBUG, "ENTER");
    LOGSTATUS(LOG_ERR,"atDevPath=%s",atDevPath);
    time(&begin_time); //get the current time
    do{
        fd = open(atDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if(fd <0){
            LOGSTATUS(LOG_ERR, "open ets device %s error fd=%d[%s]",atDevPath,fd,strerror(errno));
            sleep(1);
            continue;
        }
        LOGSTATUS(LOG_DEBUG, "Configured ets device %s fd=%d",atDevPath,fd);
        tcgetattr(fd, &ios);
        {
            ios.c_lflag &= ~( ECHO | ECHOE | ECHOK | ECHONL ); // None ECHO mode
            ios.c_lflag &= ~( ICANON );    // raw data
            ios.c_iflag = IGNBRK | IGNPAR;
            ios.c_oflag &= ~( OPOST );
            ios.c_cflag |= CRTSCTS;
            ios.c_cflag |= (CLOCAL | CREAD );
            ios.c_cflag &= ~( CSIZE );          // disable bit mask for data bits
            ios.c_cflag |= CS8;               // set 8-bit characters
            ios.c_cflag &= ~( PARENB );       // disable parity bit
            ios.c_cflag |= ( HUPCL );          // Hangup(drop DTR) on last close
            ios.c_cc[VTIME] = 0;
            ios.c_cc[VMIN] = 1;
            cfsetispeed( &ios, B115200);
            cfsetospeed( &ios, B115200);
            tcflush( fd, TCIFLUSH );
        }
        if(tcsetattr(fd, TCSANOW, &ios) < 0) {
            // affect at now
            LOGSTATUS(LOG_DEBUG,"setMuxPorts: ERROR SET ATTR (fd = %d)", fd);
        }

        time(&current_time); //get the current time
        FD_ZERO( &rfds );
        FD_SET( fd, &rfds );
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        SYSCHECK(sel = select(fd + 1, &rfds, NULL, NULL, &timeout));
        LOGSTATUS(LOG_DEBUG, "Selected %d", sel);
        if (FD_ISSET(fd, &rfds)){
            memset(buf, 0, sizeof(buf));
            len = read(fd, buf, 10);
            SYSCHECK(len);
            LOGSTATUS(LOG_DEBUG, "Read %d bytes from ets device", len);
            LOGSTATUS(LOG_DEBUG, "read:%s", buf);
            errno = 0;
        }
        ret = close(fd);
        if(ret !=0){
            LOGSTATUS(LOG_ERR, "close device %s error ret=%d[%s]",atDevPath,fd,strerror(errno));
        }
    } while (1);
    close(fd);
    LOGSTATUS(LOG_DEBUG, "LEAVE");

        return NULL;
}


static int statusd_wait_rstind(int wait_time)
{
    struct timeval tv_b;
    struct timespec timeout;
    int ret;

    gettimeofday(&tv_b, NULL);
    timeout.tv_sec = tv_b.tv_sec + wait_time;
    timeout.tv_nsec = tv_b.tv_usec * 1000;
    pthread_mutex_lock(&statusd_ramdump_lock);
    /*if we pull cp's gpio9 down to gnd, cp will die,we wait until cp die and reset itself*/
    LOGSTATUS(LOG_EMERG,"wait rstind before");
    ret = pthread_cond_timedwait(&statusd_rstind_signal, &statusd_ramdump_lock, &timeout);
    LOGSTATUS(LOG_EMERG,"wait rstind after");
    pthread_mutex_unlock(&statusd_ramdump_lock);

    return ret;
}

static int statusd_wait_child_exit(int wait_time,int *timeval)
{
    struct timeval tv_b,tv_e;
    struct timespec timeout;
    int ret;

    gettimeofday(&tv_b, NULL);
    timeout.tv_sec = tv_b.tv_sec + wait_time;
    timeout.tv_nsec = tv_b.tv_usec * 1000;
    pthread_mutex_lock(&statusd_child_lock);
    ret = pthread_cond_timedwait(&child_exit_signal, &statusd_child_lock, &timeout);
    pthread_mutex_unlock(&statusd_child_lock);
    gettimeofday(&tv_e, NULL);
    *timeval = (int)((float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    LOGSTATUS(LOG_NOTICE,"timeval=%f",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    return ret;
}

/*
* Purpose:  make sure the /data/vendor/flashless and /mnt/vendor/nvdata/md_via folder exist
* Input:    none
*
*Return:    none
*/
void c2k_fs_env_prepare(void)
{
    int ret;
    struct stat buf;

    umask(0007);
    ret = stat("/data/vendor/flashless",&buf);
    if (ret < 0) {
        LOGSTATUS(LOG_NOTICE, "%s:stat /data/vendor/flashless failed, ret=%d, errno=%d.\n", __func__, ret, errno);
        ret = mkdir("/data/vendor/flashless", 0770);
        if (ret < 0)
	    LOGSTATUS(LOG_NOTICE, "%s:mkdir /data/vendor/flashless failed,ret=%d, errno=%d.\n", __func__, ret, errno);
    }

    ret = stat("/mnt/vendor/nvdata/md_via",&buf);
    if (ret < 0) {
        LOGSTATUS(LOG_NOTICE, "%s:stat /mnt/vendor/nvdata/md_via failed, ret=%d, errno=%d.\n", __func__, ret, errno);
        ret = mkdir("/mnt/vendor/nvdata/md_via", 0770);
        if (ret < 0)
	    LOGSTATUS(LOG_NOTICE, "%s:mkdir /mnt/vendor/nvdata/md_via failed,ret=%d, errno=%d.\n", __func__, ret, errno);
    }
}

/*
* Purpose:  The main program loop
* Input:      argc - number of input arguments
*                argv - array of strings (input arguments)
*Return:    0
*/

int main(int argc, char **argv)
{
    struct sigaction act;
    int vmodem_fd =0;
    int ret = 0;
    int i = 0;
    int err_flag=0;
    struct timeval tv_b,tv_e;
    int timeval=0;
    pid_t pid;
    char module;
    int md_status;
    char property_val[PROPERTY_VALUE_MAX] = {0};

    ret = property_get("ro.vendor.mtk_eccci_c2k", property_val, NULL);
    if(ret == 0)
        LOGSTATUS(LOG_NOTICE, "ro.vendor.mtk_eccci_c2k NULL");
    else if (atoi(property_val) == 0)
        LOGSTATUS(LOG_NOTICE, "ro.vendor.mtk_eccci_c2k is 0");
    else {
    	LOGSTATUS(LOG_NOTICE, "ignore");
    	/* We need always sleep to avoid restart again */
    	while(1)
    		sleep(-1);
    }

    LOGSTATUS(LOG_NOTICE, "statusd: wait rfs_access_ok");
    rfs_access_ok();

    c2k_fs_env_prepare();

    ret = property_get(MD3_STATUS_SYNC_PROP, property_val, NULL);
    if(ret == 0) {
        LOGSTATUS(LOG_NOTICE, "MD3 status property not exist");
        need_md1_md3_reset_sync = 1;
    } else {
        LOGSTATUS(LOG_NOTICE, "MD3 status property: %s", property_val);
        need_md1_md3_reset_sync = 1;
    }

    gettimeofday(&tv_b, NULL);
    if ((logfile = fopen(LOG_FILE_PATH, "r+")) != NULL){
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        strftime ( timE,80,"%Y-%m-%d %I:%M:%S",timeinfo);
        if (chmod(LOG_FILE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
            LOGSTATUS(LOG_ERR,"fail to chmod");
        fprintf(logfile, "\n\n########### statusd log %s ###########\n", timE);
        logtofile =1;
    }

    LOGSTATUS(LOG_NOTICE, "statusd version: %s", revision);
    /*step1. get and analysis child process argument*/
    ret = statusd_analysis_para(argc,argv);
    if(ret != 0){
        LOGSTATUS(LOG_ERR,"get child para failed");
        return -1;
    }
    /*This property may be used by other module to check whether modem is ready*/
    property_set("vendor.net.cdma.mdmstat", "not ready");
    if(need_md1_md3_reset_sync)
        property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_INIT]);
    //logtofile =0;
    /*stetp2.init statusd communication interface */
    statusd_inbuffer_init((struct Statusd_Buffer*) &statusd_in_buffer);
    if(statusd_init_com() !=0 ){
        LOGSTATUS(LOG_ERR,"communication thread create failed");
        return -1;
    }
    umask(077);
    //signals treatment

    signal(SIGKILL, status_signal_treatment);
    signal(SIGTERM, status_signal_treatment);
    signal(SIGSEGV, status_signal_treatment);
    signal(SIGPIPE, status_signal_treatment);
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = status_signal_treatment_wrapper;
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigaction(SIGCHLD, &act, NULL);

    act.sa_sigaction = status_sig_handle;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &act, NULL);
    statusd_vmodem_init(&vmodem_fd);

    /*step3. power up cbp*/
    LOGSTATUS(LOG_EMERG,"power on modem");
    viatelModemPower(1);
    if(need_md1_md3_reset_sync)
        property_set(MD3_STATUS_SYNC_PROP, md_status_sync_str[MD_STATUS_SYNC_HS]);

start_child:
    /*step 4. fork flashless and wait it ready if flashless is needed for our platform*/
    if(statusd_have_flashless){
        ret = statusd_start_child(MODULE_TYPE_FLS,argv_fls);
        if(ret !=0){
            statusd_set_errflag(1);
            goto wait_terminate;
        }
    }
#if 0
    if(create_thread(&statusd_test_thread_id, statusd_test_thread,(void*) &statusd_info)!=0){ //create thread for reading input from virtual port
      LOGSTATUS(LOG_ERR,"Could not create thread for listening on socket");
      return -1;
    }
#endif
     /*step5. wait for cbp start up flag ,an at command*/
    if(!statusd_wait_cbp_up(15)){
        LOGSTATUS(LOG_ERR,"wait cbp vpup failed");
        //if md exception now, we should wait for mem dump end and md logger will reset md.
        md_status = viatelGetModemStatus();
        if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
            LOGSTATUS(LOG_ERR,"md no exception, set error flag");
            statusd_set_errflag(1);
        }
        goto wait_terminate;
    }

    /*if it is cbp8.2c ,we must enable sdio's ctrl channel*/
    if(statusd_enable_ctlch(1) < 0){
        LOGSTATUS(LOG_ERR,"enable ctrl channel failed");
        //goto terminate;
    }

    /*step6. fork mux and wait it ready if mux is needed for our platform*/
    if(statusd_have_mux){
        ret = statusd_start_child(MODULE_TYPE_MUX,argv_mux);
        if(ret !=0){
            md_status = viatelGetModemStatus();
            if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
                LOGSTATUS(LOG_ERR,"md no exception, set error flag");
                statusd_set_errflag(1);
            }
            goto wait_terminate;
        }
    }

    /*step7. fork ril and wait it ready if ril is needed for our platform*/
    if(statusd_have_ril){
        ret = statusd_start_child(MODULE_TYPE_RIL,argv_ril);
        if(ret !=0){
            md_status = viatelGetModemStatus();
            if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
                LOGSTATUS(LOG_ERR,"md no exception, set error flag");
                statusd_set_errflag(1);
            }
            goto wait_terminate;
        }
    }
     /*step8. fork ctclient and wait it ready if ril is needed for our platform*/
    if(statusd_have_ctclient){
        ret = statusd_start_child(MODULE_TYPE_CTC,argv_ctclient);
        if(ret !=0){
            md_status = viatelGetModemStatus();
            if(md_status != MD_EXCEPTION && md_status != MD_EXCEPTION_ONGOING){
                LOGSTATUS(LOG_ERR,"md no exception, set error flag");
                statusd_set_errflag(1);
            }
            goto wait_terminate;
        }
    }
    switch_user();/*need to be doubble checked */
    gettimeofday(&tv_e, NULL);
    LOGSTATUS(LOG_NOTICE,"startup end,takes [%f]",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    viatelModemWakeLock(0);
wait_terminate:
    while(1){
        statusd_get_errflag(&err_flag);
        if(err_flag){
            viatelModemWakeLock(1);
            if (statusd_ril_exit == 1){
                statusd_ril_exit++;
            }
            for(i=MODULE_TYPE_FLS;i<MODULE_TYPE_SR;i++){
                if(statusd_info.process_info[i].process_pid != -1){
                    LOGSTATUS(LOG_ERR,"ask %s\t[pid=%d] to exit",id2str(i),statusd_info.process_info[i].process_pid);
                    ret=statusd_s2csend_cmd(MODULE_TYPE_SR,i,CMD_KILL_CLIENT);
                    if(ret==1){
                        if(statusd_info.process_info[i].module_id == MODULE_TYPE_RIL) {
                            int j;
                            int status = 0;
                            int ril_running = 1;
                            int ril_wait_cnt = 0;
                            struct pollfd pollfds[1];
                            for (j=0; j<PROCESS_MAX; j++){
                                if(statusd_info.socket_info.cli_info[j].module_id == MODULE_TYPE_RIL) {
                                    LOGSTATUS(LOG_ERR, "check RIL socket %d/%d\n", j, statusd_info.socket_info.cli_info[j].cli_sock_fd);
                                    break;
                                }
                            }
                            if(j==PROCESS_MAX)
                                ril_running = 0;
                            else
                                pollfds[0].fd = statusd_info.socket_info.cli_info[j].cli_sock_fd;
                            pollfds[0].events = POLLRDHUP  | POLLNVAL;
                            pollfds[0].revents = 0;
                            while(ril_running) {
                                status = TEMP_FAILURE_RETRY(poll(pollfds, 1, 0));
                                if ((status < 0 && errno != EINTR) || (pollfds[0].revents & POLLRDHUP) || (pollfds[0].revents & POLLNVAL)) {
                                    ril_running = 0;
                                }
                                if((ril_wait_cnt%30)==0 || !ril_running)
                                    LOGSTATUS(LOG_ERR, "check RIL status %d/%d/%d/%d/%d\n", ril_running, status, errno, pollfds[0].revents, pollfds[0].fd);
                                if(ril_running) {
                                    usleep(30*1000);
                                    ril_wait_cnt++;
                                } else {
                                    break;
                                }
                                if(ril_wait_cnt>100)
                                    break;
                            }
                            timeval = 3;
                        } else {
                            statusd_wait_child_exit(3,&timeval);
                            module = statusd_info.process_info[i].module_id;
                        }
                        if((timeval < 3) && (module != (char)-1)){
                            LOGSTATUS(LOG_NOTICE,"timeval=%d module=%d ",timeval,module);
                            statusd_wait_child_exit(3,&timeval);
                        }
                    }else{
                        usleep(1000*50);
                    }
                }
            }
            for(i=MODULE_TYPE_FLS;i<MODULE_TYPE_SR;i++){
                pid = statusd_info.process_info[i].process_pid;
                module = statusd_info.process_info[i].module_id;
                LOGSTATUS(LOG_NOTICE,"%s pid=%d module=%d ",id2str(i),pid,module);
                if((pid != -1) && (module != (char)-1)){
                    ret = kill(pid,SIGKILL);
                    if(ret != 0)
                        LOGSTATUS(LOG_ERR,"kill %s[pid=%d] failed[%s]",
                                        statusd_info.process_info[i].process_name,
                                        statusd_info.process_info[i].process_pid,strerror(errno));
                    else
                        LOGSTATUS(LOG_ERR,"kill %s[pid=%d] success",
                                        statusd_info.process_info[i].process_name,
                                        statusd_info.process_info[i].process_pid);
                }
            }
            goto terminate;
        }
        sleep(1);
    }
terminate:
    wait(NULL);   //wait child process to exit
    usleep(100*1000);
    LOGSTATUS(LOG_EMERG,"ramdump=%d reset=%d",statusd_ramdump_cbp,statud_mdm_reset);
    if(statusd_ramdump_cbp){
        /*if we pull gpio for ramdump,it'll take a few seconds for cp to be ready to reset,we'll wait until cp reset occur,
         * if cp reset occur,rest_ind on signal handler will signal statusd_rstind_signal ,then we can go through;
         * if not,wait will timeout after 15 seconds
         */
        ret = statusd_wait_rstind(15);
        if(ret ==0){
            statusd_ramdump_cbp=0;
        }else{
            LOGSTATUS(LOG_EMERG,"wait rst ind timeout statusd_ramdump_cbp=%d",statusd_ramdump_cbp);
        }
    }
    LOGSTATUS(LOG_ERR,"vpup=%d reset=%d",statusd_info.vpup_state,statud_mdm_reset);
    statusd_set_reset_flag(1);
    /*vpup cmmand had not been received yet,try power off/on cp */
    if(statusd_info.vpup_state == 0){
        statusd_init_struct();
        LOGSTATUS(LOG_EMERG,"power off modem %d", statud_mdm_reset_single);
        if (statud_mdm_reset_single) {
            viatelModemPower(0); // ignore reset MD1
        } else {
            viatelModemPower(2); // also reset MD1
        }
        usleep(1000*200);/*delay 200ms*/
        LOGSTATUS(LOG_EMERG,"power on modem");
        statud_mdm_reset++;
        viatelModemPower(1);
    }else{
        statusd_init_struct();
        /*if cp had not been reseted or gpio had been pulled ,but cp not reset itselt,reset it*/
        if (statusd_ril_exit == 2 || statusd_ril_not_ready){
            LOGSTATUS(LOG_EMERG,"reset modem for ril exit");
            statud_mdm_reset++;
            C2KReset();
        }else if(statusd_mdm_error == 1){
            LOGSTATUS(LOG_EMERG,"reset modem for mdm error");
            LOGSTATUS(LOG_EMERG,"power off modem");
            viatelModemPower(2); // also reset MD1
            usleep(1000*200);/*delay 200ms*/
            LOGSTATUS(LOG_EMERG,"power on modem");
            statud_mdm_reset++;
            viatelModemPower(1);
        }else if(statud_mdm_reset || statusd_ramdump_cbp){
            LOGSTATUS(LOG_EMERG,"reset modem");
            statud_mdm_reset++;
            viatelModemReset();
        }
    }
    LOGSTATUS(LOG_ERR,"reset=%d",statud_mdm_reset);
    if(statud_mdm_reset!=0){/*reset happen,wait reset indication signal form drv*/
        ret = statusd_wait_rstind(5);
        if(ret !=0){
            LOGSTATUS(LOG_EMERG,"wait rst ind timeout statud_mdm_reset=%d",statud_mdm_reset);
        }
    }else{
        LOGSTATUS(LOG_EMERG,"receive rst ind immediately statud_mdm_reset=%d",statud_mdm_reset);
    }
    statusd_ramdump_cbp=0;
    statud_mdm_reset=1;
    statud_mdm_reset_single = 0;
    statusd_set_errflag(0);
    statusd_ril_exit = 0;
    statusd_ril_not_ready = 0;
    statusd_mdm_error = 0;
    gettimeofday(&tv_b, NULL);
    sync_with_md1_for_reset();
    statusd_set_reset_flag(0);
    goto start_child;
}


