#include <log/log.h>     /*logging in logcat*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "mtk_geofence_main.h"
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
#define  TRC(f)       ((void)0)
#else
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#define  TRC(f)       ALOGD("%s", __func__)
#endif
#endif


/**********************************************************
 *  Define                                                *
 **********************************************************/
#define GEOFENCE_INJECT_LOC_ENUM 600
#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)


/**********************************************************
 *  Global vars                                           *
 **********************************************************/
gfc2mnl_interface* gfc2mnl_hdlr_cb;
static unsigned int gModeFlag = GFC_AP_MODE;
static char g_mnl2gfc_path[128] = MNL_TO_GEOFENCE;
//static char g_gfc2mnl_path[128] = GEOFENCE_TO_MNL;


MTK_GFC_Session_T mnld_gfc_session = {
    .type = MNLD_GFC_CAPABILITY_AP_MODE,
    .pre_type = MNLD_GFC_CAPABILITY_AP_MODE,
    .id = 1,
};

/**********************************************************
 *  Socket Function                                       *
 **********************************************************/
static int safe_recvfrom(int sockfd, char* buf, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = recvfrom(sockfd, buf, len, 0,NULL, NULL)) == -1) {
        LOGW("ret=%d len=%d\n", ret, len);
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
static int set_socket_blocking(int fd, int blocking) {
    if (fd < 0) {
        LOGE("set_socket_blocking  invalid fd=%d\n", fd);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        LOGE("set_socket_blocking  invalid flags=%d\n", flags);
        return -1;
    }
    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? 0 : -1;
}

// -1 means failure
static int safe_sendto(int sockfd, const char* dest, const char* buf, int size)
{
    int len = 0;
    struct sockaddr_un soc_addr;
    int retry = 10;

    memset(&soc_addr, 0, sizeof(soc_addr));
    soc_addr.sun_path[0] = 0;
    MNLD_STRNCPY(soc_addr.sun_path + 1, dest,sizeof(soc_addr.sun_path) - 1);
    soc_addr.sun_family = AF_UNIX;

    while ((len = sendto(sockfd, buf, size, 0,(const struct sockaddr *)&soc_addr,sizeof(soc_addr))) == -1) {
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

int mnl_send2gfc(const char* buff, int len) {
    int ret = 0;
    int sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        LOGE("mnl_send2gfc() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    if (safe_sendto(sockfd, g_mnl2gfc_path, buff, len) < 0) {
        LOGE("mnl_send2gfc safe_sendto failed\n");
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


/*********************************************************/
/* GPS HAL to mnl_geofence socket                        */
/*********************************************************/
int create_gfchal2mnl_fd() {
    int fd = bind_udp_socket(GEOFENCE_TO_MNL);
    set_socket_blocking(fd, 0);
    return fd;
}

/*********************************************************/
/* MNLD Response                                         */
/*********************************************************/
int mnld_gfc_ofl_gps_open_done() {
    LOGD("mnld_gfc_ofl_gps_open_done");
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_gfc_session.id;
    return mnl2gfc_hdlr(prMsg);
}

int mnld_gfc_hbd_gps_open_done() {
    LOGD("mnld_gfc_hbd_gps_open_done");
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_gfc_session.id;
    return mnl2gfc_hdlr(prMsg);
}

int mnld_gfc_ofl_gps_close_done() {
    LOGD("mnld_gfc_ofl_gps_close_done");
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_gfc_session.id;
    return mnl2gfc_hdlr(prMsg);
}

int mnld_gfc_hbd_gps_close_done() {
    LOGD("mnld_gfc_hbd_gps_close_done");
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_HBD_GPS_CLOSE_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_gfc_session.id;
    return mnl2gfc_hdlr(prMsg);
}

int mnld_gfc_attach_done() {
    LOGD("mnld_gfc_attach_done");
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_FLP_ATTACH_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_gfc_session.id;
    return mnl2gfc_hdlr(prMsg);
}


int mnld_gfc_session_update() {
    LOGD("mnld_gfc_session_update,id=%d,type=%d",mnld_gfc_session.id,mnld_gfc_session.type);
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    int * payload = NULL;
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_SESSION_UPDATE;
    prMsg->length = sizeof(int);
    prMsg->session = mnld_gfc_session.id;
    payload = (int *)&prMsg->data[0];
    *payload = mnld_gfc_session.type;
    return mnl2gfc_hdlr(prMsg);
}

int mnld_gfc_gen_new_session() {
    mnld_gfc_session.id++;
    if (mnld_gfc_session.id >= 0xFFFFFFFF) {
        mnld_gfc_session.id = 0;
        LOGW("Session id overflow!!");
    }
    return mnld_gfc_session.id;
}

int mnl2gfc_mnld_reboot() {
    LOGD("mnl2gfc_mnld_reboot");
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    //int offset = 0;
    int * payload = NULL;
    MTK_GFC_MNL_MSG_T *prMsg = (MTK_GFC_MNL_MSG_T *)&buff[0];

    if (mnld_gfc_session_update() == -1) {
        LOGE("mnld_main_thread() mnld_gfc_session_update failed");
    }
    prMsg->type = MNLD_FLP_TYPE_MNL_BOOTUP;
    prMsg->session = mnld_gfc_session.id;
    prMsg->length = sizeof(int);
    payload = (int *)&prMsg->data[0];
    *payload = FLP_MNL_INTERFACE_VERSION;
    return mnl2gfc_hdlr(prMsg);
}

int mnl2gfc_data_send(unsigned char *buf, unsigned int len) {
    int ret = MTK_GFC_ERROR;
    if(len >0) {
        ret = mnl2gfc_hdlr((MTK_GFC_MNL_MSG_T *)buf);
    } else {
        LOGE("len err %d",len);
    }
    return ret;
}

void mtk_gfc_set_sys_mode (unsigned int sysMode) {
    if(sysMode == GFC_AP_MODE || sysMode == GFC_OFFLOAD_MODE)
    {
        gModeFlag = sysMode;
        LOGD("mtk_gfc_set_sys_mode success = %d", gModeFlag);
    } else {
        LOGE("mtk_gfc_set_sys_mode error = %d", sysMode);
    }
}


int mtk_gfc_get_sys_mode () {
    return gModeFlag;
}

#define MAX_DUMP_CHAR 128
/*********************************************************/
/* Geofence Debug Function                               */
/*********************************************************/
void mnl_gfc_dump_buf(char *p, int len) {
    int i, r, n;
    char str[MAX_DUMP_CHAR] = {0};
    for (i = 0, n = 0; i < len ; i++) {
        r = snprintf(&str[n], MAX_DUMP_CHAR - n, "%3d,", p[i]);
        if (r + n >= MAX_DUMP_CHAR || r <= 0) {
            LOGE("[GFC2MNLD]data from gfc: data=%s, error return", str);
            return;
        }

        n += r;
        if ((i % 8 == 7) || i + 1 == len) {
            LOGD("[GFC2MNLD]data from gfc: data=%s", str);
            n = 0;
            str[0] = '0';
        }
    }
}

//********************************************************************************
//
// mtk_hal2gfc_main_hdlr(): Geofence main function to process CMD from GPS/GFC HAL
//
// CMD from FWK: GPS JNI --> GFC HAL --> Mnld main --> mtk_hal2gfc_main_hdlr
//********************************************************************************
int mtk_hal2gfc_main_hdlr(int fd, gfc2mnl_interface* hdlr) {
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    char buff_send[GFC_MNL_BUFF_SIZE] = {0};
    char data[GFC_MNL_BUFF_SIZE] = {0};
    int ret = MTK_GFC_ERROR, read_len, offset = 0, len;
    //unsigned int msg_size=0;
    MTK_FLP_MSG_T *prmsg = NULL;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if ((read_len <= 0) || (read_len>GFC_MNL_BUFF_SIZE)) {
        LOGE("hal2gfc_main_hdlr() safe_recvfrom() failed read_len=%d",read_len);
        return ret;
    }
    if(hdlr != NULL) {
        gfc2mnl_hdlr_cb = hdlr;
    } else {
        LOGE("gfc2mnl_hdlr_cb = NULL");
        return ret;
    }

    len = get_binary(buff, &offset, data, sizeof(buff), sizeof(data));
    if((len > 0) && (len<=GFC_MNL_BUFF_SIZE)) {
        prmsg = (MTK_FLP_MSG_T *)&data[0];
    } else {
        LOGE("len err:%d",len);
        return ret;
    }
#if 0
    LOGD("mtk_hal2gfc_main_hdlr:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
    data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],
    data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
#endif
    if(prmsg->type == GEOFENCE_INJECT_LOC_ENUM) {
        //relay from flpHAL to geofenceHAL
        offset = 0;
        put_binary(buff_send, &offset, (const char*)data,read_len);
        ret = mnl_send2gfc(buff_send, offset);
        if(ret < 0) {
            LOGD("GFC2FLP send error return error");
        }
        return ret;
    }
    if (prmsg->type == MTK_FLP_MSG_HAL_INIT_CMD) {
    } else if (prmsg->type == MTK_FLP_MSG_CONN_SCREEN_STATUS) {
        if (mtk_gfc_get_sys_mode() == GFC_OFFLOAD_MODE) {
            mtk_gfc_ofl2mnl_process(prmsg);
        }
    } else {
        if (mtk_gfc_get_sys_mode() == GFC_OFFLOAD_MODE) {
            mtk_gfc_ofl2mnl_process(prmsg);
        } else {
            mtk_gfc_controller_process(prmsg);
        }
    }
    ret = MTK_GFC_SUCCESS;

    return ret;
}

/************************************************************************/
//  Handle Geofence Kernel Response                                     */
/************************************************************************/
int gfc_kernel_2gfc_hdlr(MTK_FLP_MSG_T* prmsg) {
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    int ret, offset = 0;

    if(prmsg == NULL) {
        LOGE("gfc_kernel_2gfc_hdlr = NULL");
        return MTK_GFC_ERROR;
    }
    put_binary(buff, &offset, (const char*)prmsg,sizeof(MTK_FLP_MSG_T)+prmsg->length);
    ret = mnl_send2gfc(buff, offset);
    if(ret < 0) {
        LOGE("MTK_GFC2HAL send error return error");
    } else {
        LOGD("gfc_kernel_2gfc_hdlr success");
    }
    return MTK_GFC_SUCCESS;
}


/************************************************************************/
//  Handle GFC Request                                                  */
/************************************************************************/
int gfc2mnl_hdlr(MTK_GFC_MNL_MSG_T  *prMsg) {
    int ret = MTK_GFC_ERROR, read_len;

    if(prMsg != NULL) {
        read_len = prMsg->length + sizeof(MTK_GFC_MNL_MSG_HEADER_T);
        LOGD("[GFC2MNLD]data from gfc: type=0x%02x, session_id=%u, len=%d,read_len = %d\n", prMsg->type, prMsg->session, prMsg->length, read_len);
        //mnl_gfc_dump_buf((char*)prMsg, read_len);
    } else {
        LOGE("gfc2mnl_hdlr recv null msg");
        return ret;
    }
    if(gfc2mnl_hdlr_cb == NULL) {
        LOGE("gfc2mnl_hdlr_cb is NULL");
        return ret;
    }

    switch (prMsg->type) {
        case MNLD_FLP_TYPE_FLP_ATTACH: {
            if (gfc2mnl_hdlr_cb->mnld_gfc_attach) {
                gfc2mnl_hdlr_cb->mnld_gfc_attach();
                if (mnld_gfc_session_update() == -1) {
                    LOGE("mnld_main_thread() mnld_gfc_session_update failed");
                } else {
                    ret = MTK_GFC_SUCCESS;
                }
            } else {
                LOGE("gfc2mnl_hdlr() mnld_flp_attach is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_HBD_GPS_OPEN: {
            if (prMsg->session != mnld_gfc_session.id) {
                LOGE("gfc2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (gfc2mnl_hdlr_cb->mnld_gfc_hbd_gps_open) {
                gfc2mnl_hdlr_cb->mnld_gfc_hbd_gps_open();
                ret = MTK_GFC_SUCCESS;
            } else {
                LOGE("gfc2mnl_hdlr() mnld_gfc_hbd_gps_open is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_HBD_GPS_CLOSE: {
            if (prMsg->session != mnld_gfc_session.id) {
                LOGE("gfc2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (gfc2mnl_hdlr_cb->mnld_gfc_hbd_gps_close) {
                gfc2mnl_hdlr_cb->mnld_gfc_hbd_gps_close();
                ret = MTK_GFC_SUCCESS;
            } else {
                LOGE("gfc2mnl_hdlr() mnld_gfc_hbd_gps_close is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_OFL_LINK_OPEN: {
            if (prMsg->session != mnld_gfc_session.id) {
                LOGE("gfc2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (gfc2mnl_hdlr_cb->mnld_gfc_ofl_link_open) {
                gfc2mnl_hdlr_cb->mnld_gfc_ofl_link_open();
                ret = MTK_GFC_SUCCESS;
            } else {
                LOGE("gfc2mnl_hdlr() mnld_gfc_ofl_link_open is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_OFL_LINK_CLOSE: {
            if (prMsg->session != mnld_gfc_session.id) {
                LOGE("gfc2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (gfc2mnl_hdlr_cb->mnld_gfc_ofl_link_close) {
                gfc2mnl_hdlr_cb->mnld_gfc_ofl_link_close();
                ret = MTK_GFC_SUCCESS;
            } else {
                LOGE("gfc2mnl_hdlr() mnld_gfc_ofl_link_close is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_OFL_LINK_SEND: {
            if (prMsg->session != mnld_gfc_session.id) {
                LOGE("gfc2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (gfc2mnl_hdlr_cb->mnld_gfc_ofl_link_send) {
                gfc2mnl_hdlr_cb->mnld_gfc_ofl_link_send(prMsg);
                ret = MTK_GFC_SUCCESS;
            } else {
                LOGE("gfc2mnl_hdlr() mnld_flp_ofl_link_send is NULL");
            }
            break;
        }
        default: {
            LOGE("gfc2mnl_hdlr() unknown cmd=0x%02x", prMsg->type);
            break;
        }
    }
    return ret;
}


