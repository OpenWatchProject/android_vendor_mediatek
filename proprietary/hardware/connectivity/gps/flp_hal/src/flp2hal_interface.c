#include <log/log.h>     /*logging in logcat*/
//#include <cutils/android_filesystem_config.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pthread.h>
#include "flphal_interface.h"
#include "data_coder.h"

#if 1
#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[gps_dbg_log]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gps_dbg_log] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gps_dbg_log] ERR: ", __VA_ARGS__);
#else
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
#endif

/**********************************************************
 *  Define                                                *
 **********************************************************/
typedef void*(*threadptr)(void*);
#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)

/**********************************************************
 *  Function Declaration                                  *
 **********************************************************/
void mnl2flphal_jni_thread(void);
extern void mtk_mnl2flp_hal_response (MTK_FLP_MSG_T *prmsg);

/**********************************************************
 *  Global vars                                           *
 **********************************************************/
static pthread_t    hal_jni_thread_id;
static int  g_ThreadExitJniSocket = 0, isflp_exist = 0;
static char g_flp2mnl_path[128] = FLP_TO_MNL;
int  g_server_socket_fd;

/**********************************************************
 *  Socket Function                                       *
 **********************************************************/
static int safe_recvfrom(int sockfd, char* buf, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = recvfrom(sockfd, buf, len, 0, NULL, NULL)) == -1) {
        //LOGW("ret=%d len=%d\n", ret, len);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto reason=[%s]\n", strerror(errno));
        break;
    }
    return ret;
}

// -1 means failure
static int safe_sendto(int sockfd, const char* dest, const char* buf, int size) {
    int len = 0;
    struct sockaddr_un soc_addr;
    int retry = 10;

    memset(&soc_addr, 0, sizeof(soc_addr));
    soc_addr.sun_path[0] = 0;
    MNLD_STRNCPY(soc_addr.sun_path + 1, dest,sizeof(soc_addr.sun_path) - 1);
    soc_addr.sun_family = AF_UNIX;

    while ((len = sendto(sockfd, buf, size, 0,  (const struct sockaddr *)&soc_addr, sizeof(soc_addr))) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto dest=[%s] len=%d reason=[%s]\n",
        dest, size, strerror(errno));
        break;
    }
    return len;
}

int flp_send2mnl(const char* buff, int len) {
    int ret = 0;
    int sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        LOGE("flp_send2mnl() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    if (safe_sendto(sockfd, g_flp2mnl_path, buff, len) < 0) {
        LOGE("flp_send2mnl safe_sendto failed\n");
        ret = -1;
    }
    close(sockfd);
    return ret;
}

static int bind_udp_socket(char* path) {
    int sockfd;
    struct sockaddr_un soc_addr;

    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOGE("socket failed reason=[%s]\n", strerror(errno));
        return -1;
    }

    memset(&soc_addr, 0, sizeof(soc_addr));
    soc_addr.sun_path[0] = 0;
    MNLD_STRNCPY(soc_addr.sun_path + 1, path,sizeof(soc_addr.sun_path) - 1);
    soc_addr.sun_family = AF_UNIX;
    unlink(soc_addr.sun_path);
    if (bind(sockfd, (struct sockaddr *)&soc_addr, sizeof(soc_addr)) < 0) {
        LOGE("bind failed path=[%s] reason=[%s]\n", path, strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int isflp_thread_exist() {
    return isflp_exist;
}


/*************************************************************/
/* create thread to collect response from mnl to FLP HAL     */
/*************************************************************/
int mnl2flphal_flp_init() {
    int ret;

    //Init client socket thread
    if(isflp_exist) {
        LOGE("flp thread exist");
        return MTK_FLP_SUCCESS;
    }
    ret = pthread_create(&hal_jni_thread_id, NULL, (threadptr)mnl2flphal_jni_thread, NULL);
    if(ret < 0) {
        LOGE("Create client thread error\n");
        return MTK_FLP_ERROR;
    }
    isflp_exist = 1;
    return MTK_FLP_SUCCESS;
}

/*************************************************************/
/*  mnl to FLP HAL socket, -1 means failure                  */
/*************************************************************/
int create_mnl2flphal_fd() {
    int fd = bind_udp_socket(MNL_TO_FLP);
    return fd;
}

/*************************************************************/
/*  rearrange location structure in 64-bit platform          */
/*************************************************************/
#ifdef MTK_64_PLATFORM
void mnl2flphal_loc_rearrange(char *loc_in, FlpLocation *loc_out) {
    char ratio = 2; //32 to 64-bits
    char padding_diff = 4;
    int sizeof_loc_in = sizeof(FlpLocation) - sizeof(size_t)/ratio - padding_diff;

    if((loc_out == NULL)||(loc_in == NULL)) {
        LOGE("mnl2flphal_loc_rearrange is NULL");
        return;
    }

    memset(loc_out, 0, sizeof(FlpLocation));
    loc_out->size = sizeof(FlpLocation);
    memcpy(&loc_out->flags, loc_in + sizeof(size_t)/ratio, sizeof(uint16_t));
    memcpy(&loc_out->latitude, loc_in + sizeof(size_t), sizeof_loc_in - sizeof(size_t));
}
#endif

/*************************************************************/
/*  Handle message from mnl to FLP HAL                       */
/*************************************************************/
int mnl2flphal_hdlr (char *buff) {
    char data[1024] ={0};
    MTK_FLP_MSG_T *prmsg = NULL;
    int offset = 0;
    int ret = MTK_FLP_ERROR, len;
    unsigned int cmd, msg_len;
#ifdef MTK_64_PLATFORM
    char loc_in[128] = {0};
    FlpLocation loc_out;
    MTK_FLP_MSG_T *prmsg_loc = NULL;
#else
    FlpLocation loc_in = {0};
#endif


    if(buff == NULL) {
        LOGE("mnl2flphal_hdlr, recv prmsg is null pointer\r\n");
        return MTK_FLP_ERROR;
    }

    len = get_binary(buff, &offset, data, HAL_FLP_BUFF_SIZE, sizeof(data));
    if((len > 0) && (len<=1024)) {
        prmsg = (MTK_FLP_MSG_T *)&data[0];
    } else {
        LOGE("len err:%d",len);
        return ret;
    }
    cmd = prmsg->type;
    msg_len = prmsg->length;

    LOGD("msg_len, recv prmsg, type %u, len %d\r\n", cmd, msg_len);
    switch (cmd) {
        case MTK_FLP_MSG_SYS_FLPD_RESET_NTF:
        case MTK_FLP_MSG_HAL_REQUEST_LOC_NTF:
        case MTK_FLP_MSG_HAL_FLUSH_LOC_NTF:
        case MTK_FLP_MSG_HAL_REPORT_STATUS_NTF:
            //Send sys init to trigger flpd attach mnld
            mtk_mnl2flp_hal_response((MTK_FLP_MSG_T *)prmsg);
            break;
        case MTK_FLP_MSG_HSB_REPORT_LOC_NTF:
        case MTK_FLP_MSG_OFL_REPORT_LOC_NTF:
#ifndef MTK_64_PLATFORM
            if(msg_len % sizeof(FlpLocation) != 0) {
                LOGE("REPORT_LOC_NTF: Data length ERROR, %d, %d",msg_len,sizeof(FlpLocation));
                return MTK_FLP_ERROR;
            }
            memcpy( &loc_in, ((char *)prmsg + sizeof(MTK_FLP_MSG_T)), sizeof(FlpLocation));
            if(loc_in.accuracy < 0.000001) {
                LOGE("invalid_LOC_NTF: loc accuracy = %f",loc_in.accuracy);
                return MTK_FLP_ERROR;
            }
            mtk_mnl2flp_hal_response((MTK_FLP_MSG_T *)prmsg);
#else
            memcpy( loc_in, ((char *)prmsg + sizeof(MTK_FLP_MSG_T)), 128*sizeof(char));
            mnl2flphal_loc_rearrange(loc_in, &loc_out);
            if(loc_out.accuracy < 0.000001) {
                LOGE("invalid_LOC_NTF: loc accuracy = %f",loc_out.accuracy);
                return MTK_FLP_ERROR;
            }
            //reform location ntf msg
            prmsg_loc = malloc(sizeof(MTK_FLP_MSG_T)+ sizeof(FlpLocation));
            if(prmsg_loc == NULL) {
                LOGE("mnl2flphal_hdlr malloc failed");
                return MTK_FLP_ERROR;
            }
            prmsg_loc->type = cmd;
            prmsg_loc->length = sizeof(FlpLocation);
            memcpy(((char*)prmsg_loc) + sizeof(MTK_FLP_MSG_T), &loc_out, sizeof(FlpLocation));
            mtk_mnl2flp_hal_response(prmsg_loc);
            free(prmsg_loc);
#endif
            break;
        default:
            LOGE("Uknown  received type:0x%02x", cmd);
            break;
    }
    return MTK_FLP_SUCCESS;
}

/*************************************************************/
/*  mnl to FLP HAL main thread                               */
/*************************************************************/
void mnl2flphal_jni_thread(void) {
    int ret = MTK_FLP_SUCCESS;
    int offset = 0;
    int read_len;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T flp_header;

    LOGD("mtk_flp_hal_jni_thread, Create\n");

    g_server_socket_fd = create_mnl2flphal_fd();

    ///TODO: add system timer function here
    //mtk_flp_sys_timer_create(FLP_TIMER_ID_CHECKCNN);
    //Send SYS_INIT to HAL
    flp_header.type = MTK_FLP_MSG_HAL_INIT_CMD;
    flp_header.length = 0;
    put_binary(buff, &offset, (const char*)&flp_header, sizeof(MTK_FLP_MSG_T));
    ret = flp_send2mnl(buff, offset);
    if(ret < 0) {
        LOGE("MTK_HAL2FLP send error return error");
    }

    if (g_server_socket_fd >= 0) {
        while(!g_ThreadExitJniSocket) {
            // - recv msg from socket interface
            read_len = safe_recvfrom(g_server_socket_fd, buff, sizeof(buff));
            if ((read_len <= 0) || (read_len > HAL_FLP_BUFF_SIZE)) {
                LOGE("mnl2flp safe_recvfrom failed read_len=%d", read_len);
                continue;
            }

            if (!g_ThreadExitJniSocket) {
                // Process received message
                mnl2flphal_hdlr(buff);
            } else {
                LOGE("mtk_flp_hal_jni_thread, read msg fail,exit socket thread\n");
                //read msg fail...
                g_ThreadExitJniSocket = 1;
            }
        }
    }

    //close socket
    LOGD("Closing server_fd,%d\r\n",g_server_socket_fd);
    if(g_server_socket_fd >= 0) {
        close(g_server_socket_fd);
    }
    LOGD("mtk_flp_hal_jni_thread, exit\n");
    g_ThreadExitJniSocket = 1;
    pthread_exit(NULL);

    return;
}


