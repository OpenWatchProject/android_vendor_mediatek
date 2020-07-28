#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "mtk_flp_main.h"
#include "data_coder.h"
#include "gps_controller.h"
#include "mtk_gps.h"
#include "mnld.h"
//#include "mtk_flp_wake_monitor.h"
#include "mtk_flp_controller.h"

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
#define LOGD(...) tag_log(1, "[mnl2flp]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnl2flp] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnl2flp] ERR: ", __VA_ARGS__);
#else
//#define LOG_TAG "flpmain"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif

#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)

/**********************************************************
 *  Global vars                                           *
 **********************************************************/
flp2mnl_interface* flp2mnl_hdlr_cb;
static unsigned int gModeFlag = FLP_AP_MODE;
static char g_mnl2flp_path[128] = MNL_TO_FLP;
int ofl_lpbk_tst_start = 0;
int ofl_lpbk_tst_size = 300;

MTK_FLP_Session_T mnld_flp_session = {
    .type = MNLD_FLP_CAPABILITY_AP_MODE,
    .pre_type = MNLD_FLP_CAPABILITY_AP_MODE,
    .id = 1,
};

/**********************************************************
 *  Function Declaration                                  *
 **********************************************************/
extern int mnl2gfc_data_send(unsigned char *buf, unsigned int len);

/**********************************************************
 *  Socket Function                                       *
 **********************************************************/
static int flp_safe_recvfrom(int sockfd, char* buf, int len) {
    int ret = 0;
    int retry = 10;

    while ((ret = recvfrom(sockfd, buf, len, 0,NULL, NULL)) == -1) {
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
static int flp_set_socket_blocking(int fd, int blocking) {
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
static int flp_safe_sendto(int sockfd, const char* dest, const char* buf, int size) {
    int len = 0;
    struct sockaddr_un soc_addr;
    int retry = 10;

    memset(&soc_addr, 0, sizeof(soc_addr));
    soc_addr.sun_path[0] = 0;
    MNLD_STRNCPY(soc_addr.sun_path + 1, dest,sizeof(soc_addr.sun_path) - 1);
    soc_addr.sun_family = AF_UNIX;
    while ((len = sendto(sockfd, buf, size, 0,(const struct sockaddr *)&soc_addr, sizeof(soc_addr))) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        LOGE("sendto dest=[%s] len=%d reason=[%s]\n", dest, size, strerror(errno));
        break;
    }
    return len;
}

int mnl_send2flp(const char* buff, int len) {
    int ret = 0;
    int sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        LOGE("mnl_send2flp() socket() failed reason=[%s]%d",
            strerror(errno), errno);
        return -1;
    }

    if (flp_safe_sendto(sockfd, g_mnl2flp_path, buff, len) < 0) {
        LOGE("mnl_send2gfc safe_sendto failed\n");
        ret = -1;
    }
    close(sockfd);
    return ret;
}

static int flp_bind_udp_socket(char* path) {
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

    if (chmod(path, 0660) < 0) {
        LOGE("chmod err = [%s]\n", strerror(errno));
    }
    return sockfd;
}


/**********************************************************
 *  Response from Offload kernel                          *
 **********************************************************/
INT32 mtk_gps_ofl_sys_rst_stpgps_req() {
    int ret = 0;

    LOGD("");
    gps_mnld_restart_mnl_process();
    return ret;
}

INT32 mtk_gps_ofl_sys_submit_flp_data(UINT8 *buf, UINT32 len) {
    static UINT8 tmp_buf[1024];
    static int  tmp_buf_w = 0;
    UINT8 buff[1024] = {0};
    MTK_FLP_MNL_MSG_T *prMsg = NULL;

    LOGD("[OFL]flp submit to host, buf=%p, len=%d", buf, len);
    if (ofl_lpbk_tst_start == 1) {
        memcpy(&tmp_buf[tmp_buf_w], buf, len);
        tmp_buf_w+=len;
        if (tmp_buf_w >= ofl_lpbk_tst_size) {
            LOGD("[OFL] lpbk recv enougth data, actual=%d, except=%d, lpbk restart...",
            tmp_buf_w, ofl_lpbk_tst_size);
            mtk_gps_ofl_send_flp_data(&tmp_buf[0], ofl_lpbk_tst_size);
            tmp_buf_w = 0;
        }
    } else if ((buf != NULL) && (len != 0) && (len < 1023)) {
        //MTK_FLP_OFFLOAD_MSG_T *prMsg = (MTK_FLP_OFFLOAD_MSG_T *)&buff[0];
        prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];
        prMsg->type = MNLD_FLP_TYPE_OFL_LINK_RECV;
        prMsg->length = len;
        prMsg->session = mnld_flp_session.id;
        if (len < sizeof(prMsg->data)) {
            memcpy(&prMsg->data[0], buf, len);
        } else {
            LOGE("memcpy buf to prMsg->data out of bounds");
        }

        if (-1 == mnl2flp_data_send(buff, sizeof(buff))) {
            LOGE("[FLP2MNLD]Send to FLP failed, %s\n", strerror(errno));
        } else {
            //LOGD("[FLP2MNLD]Send to FLP successfully\n");
        }
        ///TODO: single CB func from ofl GPS, add another delicate CB func for geofence
        if (-1 == mnl2gfc_data_send(buff, sizeof(buff))) {
            LOGE("[GFC2MNLD]Send to GFC failed, %s\n", strerror(errno));
        }
        else {
            //LOGD("[GFC2MNLD]Send to GFC successfully\n");
        }
    }
    return MTK_GPS_SUCCESS;
}


INT32 mtk_gps_ofl_sys_mnl_offload_callback(MTK_GPS_OFL_CB_TYPE type, UINT16 length, UINT8 *data) {
    int offset;
    UINT32 first_heartbeat, heartbeat_idx, ms_to_next;
    //LOGD("type:%d, length:%d\n", type, length);

    switch (type) {
        case MTK_GPS_OFL_CB_HEART_BEAT:
        {
            if (length >= 12) {
                offset = 0;
                first_heartbeat = get_int((char*)data, &offset, length);
                heartbeat_idx   = get_int((char*)data, &offset, length);
                ms_to_next      = get_int((char*)data, &offset, length);
                LOGD("type:%d, heartbeat, is_first:%u, idx:%u, next:%u",
                type, first_heartbeat, heartbeat_idx, ms_to_next);
                if (mnl_offload_is_enabled() && (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
                    if (first_heartbeat) {
                        LOGD("mnl offload notify start done (mnl offload thread running)");
                        // TODO: flp-dedicated timer
                        mnld_gps_start_done(0);  // notify GPS is start
                    } else {
                        // TODO: set flp timer by ms_to_next_heartbeat
                    }
                }
            }
            else {
                LOGE("type:0x%02x, error length:%d", type, length);
            }
        }
            break;
        default:
            break;
    }
    return MTK_GPS_SUCCESS;
}

/*********************************************************/
/* Response functions from mnl                           */
/*********************************************************/
int mnl2flp_mnld_reboot() {
    LOGD("mnl2flp_mnld_reboot");
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    //int offset = 0;
    int * payload = NULL;
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    if (mnld_flp_session_update() == -1) {
        LOGE("mnld_main_thread() mnld_flp_session_update failed");
    }
    prMsg->type = MNLD_FLP_TYPE_MNL_BOOTUP;
    prMsg->session = mnld_flp_session.id;
    prMsg->length = sizeof(int);
    payload = (int *)&prMsg->data[0];
    *payload = FLP_MNL_INTERFACE_VERSION;
    return mnl2flp_hdlr(prMsg);
}

int mnl2flp_data_send(UINT8 *buf, UINT32 len) {
    int ret = 0;
    if(len >0) {
        ret = mnl2flp_hdlr((MTK_FLP_MNL_MSG_T *)buf);
    } else {
        LOGE("len err %d",len);
    }
    return ret;
}

int mnld_flp_attach_done() {
    LOGD("mnld_flp_attach_done");
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_FLP_ATTACH_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_flp_session.id;
    return mnl2flp_hdlr(prMsg);
}

int mnld_flp_hbd_gps_open_done() {
    LOGD("mnld_flp_hbd_gps_open_done");
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_flp_session.id;
    return mnl2flp_hdlr(prMsg);
}

int mnld_flp_hbd_gps_close_done() {
    LOGD("mnld_flp_hbd_gps_close_done");
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_HBD_GPS_CLOSE_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_flp_session.id;
    return mnl2flp_hdlr(prMsg);
}

int mnld_flp_ofl_gps_open_done() {
    LOGD("mnld_flp_ofl_gps_open_done");
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_flp_session.id;
    return mnl2flp_hdlr(prMsg);
}

int mnld_flp_ofl_gps_close_done() {
    LOGD("mnld_flp_ofl_gps_close_done");
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE;
    prMsg->length = 0;
    prMsg->session = mnld_flp_session.id;
    return mnl2flp_hdlr(prMsg);
}

int mnld_flp_session_update() {
    LOGD("mnld_flp_session_update,id=%d,type=%d",mnld_flp_session.id,mnld_flp_session.type);
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    int * payload = NULL;
    MTK_FLP_MNL_MSG_T *prMsg = (MTK_FLP_MNL_MSG_T *)&buff[0];

    prMsg->type = MNLD_FLP_TYPE_SESSION_UPDATE;
    prMsg->length = sizeof(int);
    prMsg->session = mnld_flp_session.id;
    payload = (int *)&prMsg->data[0];
    *payload = mnld_flp_session.type;
    return mnl2flp_hdlr(prMsg);
}

int mnld_flp_gen_new_session() {
    mnld_flp_session.id++;
    if (mnld_flp_session.id >= 0xFFFFFFFF) {
        mnld_flp_session.id = 0;
        LOGW("Session id overflow!!");
    }
    return mnld_flp_session.id;
}

// Add for dbg usage
#define MAX_DUMP_CHAR 128
void mnl_flp_dump_buf(char *p, int len) {
    int i, r, n;
    char str[MAX_DUMP_CHAR] = {0};
    for (i = 0, n = 0; i < len ; i++) {
        r = snprintf(&str[n], MAX_DUMP_CHAR - n, "%3d,", p[i]);
        if (r + n >= MAX_DUMP_CHAR || r <= 0) {
            LOGE("[FLP2MNLD]data from flp: data=%s, error return", str);
            return;
        }

        n += r;
        if ((i % 8 == 7) || i + 1 == len) {
            LOGD("[FLP2MNLD]data from flp: data=%s", str);
            n = 0;
            str[0] = '0';
        }
    }
}

void mtk_flp_set_sys_mode (unsigned int sysMode) {
    //char localbuf[20];
    //char buff[HAL_FLP_BUFF_SIZE] = {0};
    //int ret, offset = 0;

    if(sysMode == FLP_AP_MODE || sysMode == FLP_OFFLOAD_MODE) {
        gModeFlag = sysMode;
        LOGD("mtk_flp_set_sys_mode success = %d", gModeFlag);
    } else {
        LOGD("mtk_flp_set_sys_mode error = %d", sysMode);
    }
}


int mtk_flp_get_sys_mode() {
    return gModeFlag;
}

//*****************************************************************************
//
// mtk_hal2flp_main_hdlr(): FLP main function to process CMD from FLP HAL
//
// CMD from FWK: FLP JNI --> FLP HAL --> Mnld main --> mtk_hal2flp_main_hdlr
// RSP to FWK  : mtk_flp2hal_main_hdlr --> mnl2flphal_hdlr(HAL) --> FLP JNI
//*****************************************************************************
int mtk_hal2flp_main_hdlr(int fd, flp2mnl_interface* hdlr) {
    char buff[FLP_MNL_BUFF_SIZE] = {0};
    //char localbuf[512] = {0};
    char data_recv[1024] ={0};
    //char *data = NULL;
    //char *loc_ntf = NULL;
    int ret = MTK_FLP_SUCCESS, read_len, offset = 0, len;
    //unsigned int msg_size=0;
    MTK_FLP_MSG_T *prmsg = NULL;
    //MTK_FLP_LOCATION_T outloc;

    read_len = flp_safe_recvfrom(fd, buff, sizeof(buff));
    if ((read_len <= 0) || (read_len>FLP_MNL_BUFF_SIZE)) {
        LOGE("flp2mnl_hdlr_cb() safe_recvfrom() failed read_len=%d", read_len);
        return MTK_FLP_ERROR;
    }
    if(hdlr != NULL) {
        flp2mnl_hdlr_cb = hdlr;
    } else {
        LOGD("flp2mnl_hdlr_cb is null");
    }

    len = get_binary(buff, &offset, data_recv, sizeof(buff), sizeof(data_recv));
    if((len > 0) && (len<=FLP_MNL_BUFF_SIZE)) {
        prmsg = (MTK_FLP_MSG_T *)&data_recv[0];
    } else {
        LOGE("len err:%d",len);
        return ret;
    }

    switch (prmsg->type) {
        case MTK_FLP_MSG_HAL_INIT_CMD:                  //process init request from HAL
            break;
        case MTK_FLP_MSG_HAL_START_CMD:               //process init request from HAL
        case MTK_FLP_MSG_HAL_STOP_CMD:                //process init request from HAL
        case MTK_FLP_MSG_HAL_SET_OPTION_CMD:            //process init request from HAL
            if(mtk_flp_get_sys_mode() == FLP_OFFLOAD_MODE) {
                mtk_flp2mnl_process(prmsg);
            } else {
                mtk_flp_controller_process(prmsg);
            }
            break;
        case MTK_FLP_MSG_HAL_DIAG_INJECT_DATA_NTF:      //process init request from HAL
        case MTK_FLP_MSG_CONN_SCREEN_STATUS:            //process synch screen status from HAL
            if(mtk_flp_get_sys_mode() == FLP_OFFLOAD_MODE) {
                mtk_flp2mnl_process(prmsg);             //send HAL(AP)->Kernel(connsys) msg here
            }
            break;
        case MTK_FLP_MSG_HAL_REQUEST_LOC_NTF:           //message from JNI, need forwarding only.
        case MTK_FLP_MSG_HAL_FLUSH_LOC_NTF:             //message from JNI, need forwarding only.
            if(mtk_flp_get_sys_mode() == FLP_OFFLOAD_MODE) {
                mtk_flp2mnl_process(prmsg);
            } else {
                put_binary(buff, &offset, (const char*)prmsg, sizeof(MTK_FLP_MSG_T)+ sizeof(prmsg->length));
                ret = mnl_send2flp(buff, offset);
                if(ret < 0) {
                    LOGE("MTK_FLP2HAL send error return error");
                }
            }
            break;
        default:
            LOGE("FLP HAL: Uknown msessage, type:0x%02x", prmsg->type);
            break;
    }
    return ret;
}

/************************************************************************/
//  Handle FLP Request                                                  */
/************************************************************************/
int flp2mnl_hdlr(MTK_FLP_MNL_MSG_T  *prMsg) {
    int ret = MTK_FLP_ERROR, read_len;

    if(prMsg != NULL) {
        read_len = prMsg->length + sizeof(MTK_FLP_MNL_MSG_HEADER_T);
        LOGD("[FLP2MNLD]data from flp: type=0x%02x, session_id=%u, len=%d, read_len = %d\n", prMsg->type, prMsg->session, prMsg->length, read_len);
        //mnl_flp_dump_buf((char*)prMsg, read_len);
    } else {
        LOGE("flp2mnl_hdlr recv null msg");
        return ret;
    }
    if(flp2mnl_hdlr_cb == NULL) {
        LOGE("flp2mnl_hdlr_cb is NULL");
        return ret;
    }

    switch (prMsg->type) {
        case MNLD_FLP_TYPE_FLP_ATTACH: {
            if (flp2mnl_hdlr_cb->mnld_flp_attach) {
                flp2mnl_hdlr_cb->mnld_flp_attach();
                if (mnld_flp_session_update() == -1) {
                    LOGE("mnld_main_thread() mnld_flp_session_update failed");
                }
                ret = MTK_FLP_SUCCESS;
            } else {
                LOGE("flp2mnl_hdlr_cb() mnld_flp_attach is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_HBD_GPS_OPEN: {
            if (prMsg->session != mnld_flp_session.id) {
                LOGE("flp2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (flp2mnl_hdlr_cb->mnld_flp_hbd_gps_open) {
                flp2mnl_hdlr_cb->mnld_flp_hbd_gps_open();
                ret = MTK_FLP_SUCCESS;
            } else {
                LOGE("flp2mnl_hdlr() mnld_flp_hbd_gps_open is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_HBD_GPS_CLOSE: {
            if (prMsg->session != mnld_flp_session.id) {
                LOGE("flp2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (flp2mnl_hdlr_cb->mnld_flp_hbd_gps_close) {
                flp2mnl_hdlr_cb->mnld_flp_hbd_gps_close();
                ret = MTK_FLP_SUCCESS;
            } else {
                LOGE("flp2mnl_hdlr() mnld_flp_hbd_gps_close is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_OFL_LINK_OPEN: {
            if (prMsg->session != mnld_flp_session.id) {
                LOGE("flp2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (flp2mnl_hdlr_cb->mnld_flp_ofl_link_open) {
                flp2mnl_hdlr_cb->mnld_flp_ofl_link_open();
                ret = MTK_FLP_SUCCESS;
            } else {
                LOGE("flp2mnl_hdlr() mnld_flp_ofl_link_open is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_OFL_LINK_CLOSE: {
            if (prMsg->session != mnld_flp_session.id) {
                LOGE("flp2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (flp2mnl_hdlr_cb->mnld_flp_ofl_link_close) {
                flp2mnl_hdlr_cb->mnld_flp_ofl_link_close();
                ret = MTK_FLP_SUCCESS;
            } else {
                LOGE("flp2mnl_hdlr() mnld_flp_ofl_link_close is NULL");
            }
            break;
        }
        case MNLD_FLP_TYPE_OFL_LINK_SEND: {
            if (prMsg->session != mnld_flp_session.id) {
                LOGE("flp2mnl_hdlr() session_id doesn't match, ignore");
                break;
            }
            if (flp2mnl_hdlr_cb->mnld_flp_ofl_link_send) {
                flp2mnl_hdlr_cb->mnld_flp_ofl_link_send(prMsg);
                ret = MTK_FLP_SUCCESS;
            } else {
                LOGE("flp2mnl_hdlr() mnld_flp_ofl_link_send is NULL");
            }
            break;
        }
        default: {
            LOGE("flp2mnl_hdlr() unknown cmd=0x%02x", prMsg->type);
            break;
        }
    }
    return ret;
}

int create_flphal2mnl_fd() {
    int fd = flp_bind_udp_socket(FLP_TO_MNL);
    flp_set_socket_blocking(fd, 0);
    return fd;
}


