/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <math.h>
#include <hardware/gps.h>
#include "mtk_flp_controller.h"
#include "data_coder.h"

//#include "mtk_flp_wake_monitor.h"

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
#define LOG_TAG "flpint"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif


#define MAX_FLP_BATCHING_MSG   10
/**********************************************************
 *  Global vars                                           *
 **********************************************************/
static unsigned int fgMnldStatus = MNL_STATUS_NONE;
static unsigned int gCurrentSession = 0;
static unsigned int gCurrentSupport = 0;

static int gFlp2mnl_cnt = 0;
static MTK_FLP_MNL_MSG_T gFlp2mnl_msg[MAX_FLP_BATCHING_MSG];

enum{
    CMD_FLP_REPORT_GNSS_LOC = 58,
};


static int mtk_flp_gpsloc2flploc(GpsLocation* gpsloc, FlpLocation *flploc) {
    if (gpsloc == NULL || flploc == NULL) {
        return MTK_FLP_ERROR;
    }

    memset(flploc,0,sizeof(FlpLocation));
    flploc->size = sizeof(FlpLocation);
    flploc->sources_used = FLP_TECH_MASK_GNSS;
    flploc->accuracy = 1.7*(gpsloc->accuracy); //convert accuracy from 67% to 95%
    flploc->altitude = gpsloc->altitude;
    flploc->bearing = gpsloc->bearing;
    flploc->flags =gpsloc->flags;
    flploc->latitude = gpsloc->latitude;
    flploc->longitude = gpsloc->longitude;
    flploc->speed = gpsloc->speed;
    flploc->timestamp = gpsloc->timestamp;//mtk_flp_get_timestamp(NULL);
    #if 0
    LOGD("[DC_GNSS]:dc2gnss: acc:%.4f,alt:%.4f,bearing:%.4f,flag:%d,lat:%.4f,lon:%.4f,spd:%.4f",
    flploc->accuracy,flploc->altitude,flploc->bearing,flploc->flags,flploc->latitude,
    flploc->longitude,flploc->speed);
    #endif

    return MTK_FLP_SUCCESS;
}

static void mtk_flp_dc_loc_rearrange(MTK_FLP_LOCATION_T *loc_in, MTK_FLP_LOCATION_T *loc_out) {
    loc_out->size = loc_in->size;
    loc_out->sources_used = loc_in->sources_used;
    loc_out->flags = loc_in->flags;
    loc_out->latitude = loc_in->latitude;
    loc_out->longitude = loc_in->longitude;
    loc_out->accuracy = loc_in->accuracy;
    loc_out->altitude = loc_in->altitude;
    loc_out->speed = loc_in->speed;
    loc_out->bearing = loc_in->bearing;
    loc_out->timestamp = loc_in->timestamp;
    #if 0
    LOGD("Loc from CNN:LNG:%f LAT:%f ALT:%f ACC:%f SPD:%f BEARING:%f, FLAGS:%04X SOURCES:%08X Timestamp:%lld",
    loc_out->longitude, loc_out->latitude, loc_out->altitude, loc_out->accuracy,
    loc_out->speed, loc_out->bearing, loc_out->flags, loc_out->sources_used, loc_out->timestamp);
    LOGD("Loc processed:LNG:%f LAT:%f ALT:%f ACC:%f SPD:%f BEARING:%f, FLAGS:%04X SOURCES:%08X Timestamp:%lld",
    loc_in->longitude, loc_in->latitude, loc_in->altitude, loc_in->accuracy,
    loc_in->speed, loc_in->bearing, loc_in->flags, loc_in->sources_used, loc_in->timestamp);
    #endif
}

#if 0
void mtk_flp2mnl_send_screen_state() {
    MTK_FLP_MNL_MSG_HEADER_T *flp2mnl_msg;
    MTK_FLP_MSG_T flp2cnn_msg;
    unsigned char screen_state = 0;
    mtk_flp_get_wake_monitor_state(&screen_state);
    flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_FLP_MNL_MSG_HEADER_T) + sizeof(MTK_FLP_MSG_T) + sizeof(unsigned char));
  if(flp2mnl_msg != NULL) {
    flp2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_SEND;
    flp2mnl_msg->session= gCurrentSession;
    flp2mnl_msg->length = sizeof(MTK_FLP_MSG_T) + sizeof(UINT8);
    flp2cnn_msg.type = MTK_FLP_MSG_CONN_SCREEN_STATUS;
    flp2cnn_msg.length = sizeof(UINT8);
    memcpy(((char*)flp2mnl_msg) + sizeof(MTK_FLP_MNL_MSG_HEADER_T), ((char*)&flp2cnn_msg), sizeof(MTK_FLP_MSG_T));
    memcpy(((char*)flp2mnl_msg) + sizeof(MTK_FLP_MNL_MSG_HEADER_T) + sizeof(MTK_FLP_MSG_T), &screen_state, sizeof(unsigned char));
    LOGD("Send to OFL FLP, screen_state = %u", screen_state);
    flp2mnl_hdlr(flp2mnl_msg);
    free(flp2mnl_msg);
  }
}
#endif

/************************************************************************/
//  Process msgs from FLP HAL and send to offload kernel
/************************************************************************/
int mtk_flp2mnl_process(MTK_FLP_MSG_T *prmsg) {
    MTK_FLP_MNL_MSG_HEADER_T* flp2mnl_msg = NULL;
    MTK_FLP_MSG_T flp2cnn_msg;

    if(prmsg == NULL) {
        LOGD("mtk_flp2mnl_process, recv prmsg is null pointer\n");
        return MTK_FLP_ERROR;
    }

    LOGD("mtk_flp2mnl_process, recv prmsg, type:%d, len:%d status:%u\r\n", prmsg->type, prmsg->length, fgMnldStatus);

    switch(prmsg->type) {
        case MTK_FLP_MSG_HAL_START_CMD:
        case MTK_FLP_MSG_HAL_DIAG_INJECT_DATA_NTF:
            if (fgMnldStatus != MNL_STATUS_OFL_LINK_OPEN_DONE) {
                if (gFlp2mnl_cnt == 0) {
                    //send init message to mnld first
                    flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_FLP_MNL_MSG_HEADER_T)); //send header only, no message body
                    if(flp2mnl_msg != NULL) {
                        flp2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_OPEN;
                        flp2mnl_msg->session = gCurrentSession;
                        flp2mnl_msg->length = 0;
                        LOGD("MNLD_FLP_TYPE_OFL_LINK_OPEN");
                        fgMnldStatus = MNL_STATUS_OFL_LINK_OPEN;
                        flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                        free(flp2mnl_msg);
                    }
                }
                if (gFlp2mnl_cnt < MAX_FLP_BATCHING_MSG) {
                    gFlp2mnl_msg[gFlp2mnl_cnt].type = MNLD_FLP_TYPE_OFL_LINK_SEND;
                    gFlp2mnl_msg[gFlp2mnl_cnt].length = prmsg->length + sizeof(MTK_FLP_MSG_T);
                    gFlp2mnl_msg[gFlp2mnl_cnt].session = gCurrentSession;
                    memcpy(&gFlp2mnl_msg[gFlp2mnl_cnt].data[0], ((char*)prmsg), (sizeof(MTK_FLP_MSG_T)+ prmsg->length));
                    gFlp2mnl_cnt++;
                    LOGD("Keep flp data cnt %d", gFlp2mnl_cnt);
                } else {
                    LOGD("Batching size overflow");
                }
            }
            break;
        case MTK_FLP_MSG_HAL_STOP_CMD:
            //send msg stop mnl first
            flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc( sizeof(MTK_FLP_MNL_MSG_HEADER_T)+sizeof(MTK_FLP_MSG_T));
            if(flp2mnl_msg != NULL) {
                flp2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_SEND;
                flp2mnl_msg->session = gCurrentSession;
                flp2mnl_msg->length = sizeof(MTK_FLP_MSG_T); // data sent structure: [type, length for AP_conn recognition] + [type,length]
                flp2cnn_msg.type = MTK_FLP_MSG_HAL_STOP_CMD;
                flp2cnn_msg.length = 0;
                memcpy(((char*)flp2mnl_msg)+sizeof(MTK_FLP_MNL_MSG_HEADER_T), ((char*)&flp2cnn_msg), sizeof(MTK_FLP_MSG_T));
                LOGD("Send to OFL FLP,message type:0x%02x, len:%u", flp2cnn_msg.type, (flp2mnl_msg->length + sizeof(MTK_FLP_MNL_MSG_HEADER_T)));
                flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                free(flp2mnl_msg);
            }
            break;
        case MTK_FLP_MSG_DC_START_CMD: // host-based start gnss
            //send init message to mnld first
            flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_FLP_MNL_MSG_HEADER_T)); //send header only, no message body
            if(flp2mnl_msg != NULL) {
                flp2mnl_msg->type = MNLD_FLP_TYPE_HBD_GPS_OPEN;
                flp2mnl_msg->length = 0;
                flp2mnl_msg->session = gCurrentSession;
                LOGD("MNLD_FLP_TYPE_HBD_GPS_OPEN");
                fgMnldStatus = MNL_STATUS_HBD_GPS_OPEN;
                flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                free(flp2mnl_msg);
            }
            break;
        case MTK_FLP_MSG_DC_STOP_CMD:
            //send stop message to mnld
            flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_FLP_MNL_MSG_HEADER_T)); //send header only, no message body
            if(flp2mnl_msg != NULL) {
                flp2mnl_msg->type = MNLD_FLP_TYPE_HBD_GPS_CLOSE;
                flp2mnl_msg->length = 0;
                flp2mnl_msg->session = gCurrentSession;
                LOGD("Send to mnld for deinit,message type:0x%02x, len:%d", flp2mnl_msg->type, flp2mnl_msg->length);
                fgMnldStatus = MNL_STATUS_HBD_GPS_CLOSE;
                flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                free(flp2mnl_msg);
            }
            break;
        case MTK_FLP_MSG_CONN_SCREEN_STATUS:
            if(fgMnldStatus == MNL_STATUS_OFL_LINK_OPEN_DONE) {
                flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc( sizeof(MTK_FLP_MNL_MSG_HEADER_T)+ sizeof(MTK_FLP_MSG_T)+prmsg->length);
                if(flp2mnl_msg != NULL) {
                    flp2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_SEND;
                    flp2mnl_msg->session = gCurrentSession;
                    flp2mnl_msg->length = (prmsg->length) + sizeof(MTK_FLP_MSG_T);
                    memcpy(((char*)flp2mnl_msg)+sizeof(MTK_FLP_MNL_MSG_HEADER_T), ((char*)prmsg), (sizeof(MTK_FLP_MSG_T)+ (prmsg->length)));
                    LOGD("Send to OFL FLP,message type:0x%02x, len:%d", prmsg->type, (flp2mnl_msg->length + sizeof(MTK_FLP_MSG_T)));
                    flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                    free(flp2mnl_msg);
                }
            } else {
                LOGD("free screen dbg");
            }
            break;
        default:
            if(mtk_flp_get_sys_mode() == FLP_OFFLOAD_MODE) {
                if(fgMnldStatus == MNL_STATUS_OFL_LINK_OPEN_DONE) {
                    if ((sizeof(MTK_FLP_MNL_MSG_HEADER_T)+ sizeof(MTK_FLP_MSG_T)+prmsg->length) <= 0){
                        LOGW("Input Zero or Negative length");
                        return MTK_FLP_ERROR;
                    }
                    flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc( sizeof(MTK_FLP_MNL_MSG_HEADER_T)+ sizeof(MTK_FLP_MSG_T)+prmsg->length);
                    if(flp2mnl_msg != NULL) {
                        flp2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_SEND;  //for loopback test between mnld & FLP AP  use FLP_MNL_CMD_LOOPBACK
                        flp2mnl_msg->session = gCurrentSession;
                        flp2mnl_msg->length = (prmsg->length) + sizeof(MTK_FLP_MSG_T); // data sent structure: [type, length for AP_conn recognition] + [type,length+buff for flp recognition]
                        memcpy(((INT8*)flp2mnl_msg)+sizeof(MTK_FLP_MNL_MSG_HEADER_T), ((INT8*)prmsg), (sizeof(MTK_FLP_MSG_T)+ (prmsg->length)));
                        flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                        free(flp2mnl_msg);
                    }
                } else {
                    LOGD("mnld_ret=%u", fgMnldStatus);
                }
            }
            break;
    }
    return MTK_FLP_SUCCESS;
}

/************************************************************************/
//  Handle MNL Response                                                 */
/************************************************************************/
int mnl2flp_hdlr(MTK_FLP_MNL_MSG_T* prmsg) {
    //char localbuf[512];
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    //char *loc_ntf = NULL;
    //unsigned char* msg_dbg = NULL;
    //unsigned int msg_size=0;
    int ret, i;//, retlen, network_status;
    int offset = 0;
    MTK_FLP_MNL_MSG_HEADER_T *flp2mnl_msg = NULL;
    MTK_FLP_MSG_T *flp2hal_msg = NULL;
    //MTK_FLP_MSG_T *flp2hal_diag = NULL;
    MTK_FLP_MSG_T *cnn2flp_msg = NULL;
    MTK_FLP_LOCATION_T  loc_in, loc_out;
    GpsLocation  *gps_loc;
    MTK_FLP_LOCATION_T  flp_loc;

    if(prmsg == NULL) {
        LOGD("mnl2flp_hdlr recv null msg");
        return MTK_FLP_ERROR;
        }

    LOGD("mtk_mnl2flp_recv_msg type = 0x%02x, session = %u, len = %u, state = %u\n",  prmsg->type, prmsg->session, prmsg->length, fgMnldStatus);

    switch(prmsg->type) {
        case MNLD_FLP_TYPE_MNL_BOOTUP:
            fgMnldStatus = MNL_STATUS_ATTACH_DONE;
            flp2hal_msg = malloc(sizeof(MTK_FLP_MSG_T));
            if(flp2hal_msg == NULL) {
                LOGE("reset ntf malloc failed");
                return MTK_FLP_ERROR;
            }
            flp2hal_msg->type = MTK_FLP_MSG_SYS_FLPD_RESET_NTF;
            flp2hal_msg->length = 0;
            put_binary(buff, &offset, (const char*)flp2hal_msg, sizeof(MTK_FLP_MSG_T));
            ret = mnl_send2flp(buff, offset);
            free(flp2hal_msg);
            if(ret < 0) {
                LOGD("MTK_FLP2HAL send error return error");
            }
            break;
        case MNLD_FLP_TYPE_SESSION_UPDATE:
            gCurrentSession = prmsg->session;
            memcpy(&gCurrentSupport, ((char*)prmsg)+sizeof(MTK_FLP_MNL_MSG_HEADER_T), sizeof(gCurrentSupport));
            LOGD("SESSION_UPDATE %u support %u mode %u", gCurrentSession, gCurrentSupport, fgMnldStatus);
            if (fgMnldStatus == MNL_STATUS_HBD_GPS_OPEN_DONE && gCurrentSupport == FLP_AP_MODE) {
                LOGD("SESSION_UPDATE: already run on host");
            } else if (fgMnldStatus == MNL_STATUS_OFL_LINK_OPEN_DONE && (gCurrentSupport & FLP_OFFLOAD_MODE)) {
                LOGD("SESSION_UPDATE: already run on offlaod");
            } else if (fgMnldStatus == MNL_STATUS_HBD_GPS_OPEN_DONE && (gCurrentSupport & FLP_OFFLOAD_MODE)) {
                LOGD("SESSION_UPDATE: switch to ofl mode");
                mtk_flp_controller_reset_status();
                mtk_flp_set_sys_mode(FLP_OFFLOAD_MODE);

                flp2hal_msg = malloc(sizeof(MTK_FLP_MSG_T));
                if(flp2hal_msg == NULL) {
                    LOGE("reset ntf malloc failed");
                    return MTK_FLP_ERROR;
                }
                flp2hal_msg->type = MTK_FLP_MSG_SYS_FLPD_RESET_NTF;
                flp2hal_msg->length = 0;
                put_binary(buff, &offset, (const char*)flp2hal_msg, sizeof(MTK_FLP_MSG_T));
                ret = mnl_send2flp(buff, offset);
                free(flp2hal_msg);
                if(ret < 0) {
                    LOGD("mnl2flp_hdlr send error return error");
                }
            } else if (fgMnldStatus == MNL_STATUS_OFL_LINK_OPEN_DONE && (gCurrentSupport == FLP_AP_MODE)) {
                LOGD("SESSION_UPDATE: switch to ap mode");
                mtk_flp_set_sys_mode(FLP_AP_MODE);
                flp2hal_msg = malloc(sizeof(MTK_FLP_MSG_T));
                if(flp2hal_msg == NULL) {
                    LOGE("reset ntf malloc failed");
                    return MTK_FLP_ERROR;
                }
                flp2hal_msg->type = MTK_FLP_MSG_SYS_FLPD_RESET_NTF;
                flp2hal_msg->length = 0;
                put_binary(buff, &offset, (const char*)flp2hal_msg, sizeof(MTK_FLP_MSG_T));
                ret = mnl_send2flp(buff, offset);
                free(flp2hal_msg);
                if(ret < 0) {
                    LOGD("mnl2flp_hdlr send error return error");
                }
            } else {
                if (gCurrentSupport & FLP_OFFLOAD_MODE) {
                    mtk_flp_set_sys_mode(FLP_OFFLOAD_MODE);
                    LOGD("SESSION_UPDATE: set offload mode");
                } else if (gCurrentSupport == FLP_AP_MODE) {
                    mtk_flp_set_sys_mode(FLP_AP_MODE);
                    LOGD("SESSION_UPDATE: set ap mode");
                }
            }
            break;
        case MNLD_FLP_TYPE_HBD_GPS_LOCATION:
            gps_loc = (GpsLocation *)(((char*)prmsg)+sizeof(MTK_FLP_MNL_MSG_HEADER_T));
            //LOGD("GNSS LOC DBG: %f, %lf, %lf, %d", gps_loc->accuracy, gps_loc->latitude, gps_loc->longitude, gps_loc->size);

            if(mtk_flp_gpsloc2flploc(gps_loc, (FlpLocation *)&flp_loc) != MTK_FLP_SUCCESS) {
                LOGD("Incorrect data received");
            } else {
                if (mtk_flp_controller_report_loc(flp_loc) == -1) {
                    break;
                }
                //send loc to FLP HAL
                if(mtk_flp_controller_query_mode() > MTK_FLP_IDLE_MODE) {
                    flp2hal_msg = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(MTK_FLP_LOCATION_T));
                    if(flp2hal_msg == NULL) {
                        LOGE("report loc malloc failed");
                        return MTK_FLP_ERROR;
                    }
                    flp2hal_msg->type = MTK_FLP_MSG_HSB_REPORT_LOC_NTF;
                    flp2hal_msg->length = sizeof(MTK_FLP_LOCATION_T);
                    memcpy((char *)flp2hal_msg + sizeof(MTK_FLP_MSG_T),&flp_loc, sizeof(MTK_FLP_LOCATION_T) );
                    put_binary(buff, &offset, (const char*)flp2hal_msg, (sizeof(MTK_FLP_MSG_T)+flp2hal_msg->length));
                    ret = mnl_send2flp(buff, offset);
                    free(flp2hal_msg);
                    if(ret < 0) {
                        LOGD("MTK_FLP2HAL send error return error");
                    }
                }
#if 0 // android o no diag CB
                //send location diag ntf
                if(mtk_flp_controller_query_mode() > MTK_FLP_IDLE_MODE) {
                sprintf(localbuf,"FLP_NTF=LNG:%f LAT:%f ALT:%f", flp_loc.longitude, flp_loc.latitude, flp_loc.altitude);
                LOGD("%s",localbuf);
                flp2hal_diag = malloc(sizeof(MTK_FLP_MSG_T)+strlen(localbuf));
                if(flp2hal_diag == NULL) {
                LOGE("diag report malloc failed");
                return MTK_FLP_ERROR;
                }
                memset(buff,0,HAL_FLP_BUFF_SIZE*sizeof(char));
                offset = 0;
                flp2hal_diag->type = MTK_FLP_MSG_HAL_DIAG_REPORT_DATA_NTF;
                flp2hal_diag->length = strlen(localbuf);
                memcpy((char *)flp2hal_diag + sizeof(MTK_FLP_MSG_T),&localbuf[0], strlen(localbuf) );
                put_binary(buff, &offset, (const char*)flp2hal_diag, (sizeof(MTK_FLP_MSG_T)+flp2hal_diag->length));
                ret = mnl_send2flp(buff, offset);
                free(flp2hal_diag);
                if(ret < 0) {
                LOGD("MTK_FLP2HAL send error return error");
                }
                }
#endif
            }
            break;
        case MNLD_FLP_TYPE_OFL_LINK_RECV:
            if (prmsg->length <= 0){
                LOGW("Input Zero or Negative length");
                return MTK_FLP_ERROR;
            }
            cnn2flp_msg = malloc(prmsg->length);
            if(cnn2flp_msg == NULL) {
                return MTK_FLP_ERROR;
            }
            memcpy((char*)cnn2flp_msg, ((char*)prmsg)+ sizeof(MTK_FLP_MNL_MSG_HEADER_T), prmsg->length);
            LOGD("Received OFFLOAD message type: 0x%02x, len = %u", cnn2flp_msg->type, cnn2flp_msg->length);
            switch(cnn2flp_msg->type) {
                case MTK_FLP_MSG_HAL_STOP_RSP:
                    //send stop message to mnld
                    flp2mnl_msg = (MTK_FLP_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_FLP_MNL_MSG_HEADER_T));
                    if(flp2mnl_msg!= NULL) {
                        flp2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_CLOSE;
                        flp2mnl_msg->session= gCurrentSession;
                        flp2mnl_msg->length = 0;
                        //LOGD("Send to mnld for deinit,message type:%u, len:%u", flp2mnl_msg->type, flp2mnl_msg->length);
                        flp2mnl_hdlr((MTK_FLP_MNL_MSG_T *)flp2mnl_msg);
                        fgMnldStatus = MNL_STATUS_OFL_LINK_CLOSE;
                        free(flp2mnl_msg);
                        LOGD("mnld deinit success");
                    }
                    break;
                case MTK_FLP_MSG_OFL_REPORT_LOC_NTF:
                    //check location size
                    if(cnn2flp_msg->length >= (int)(sizeof(MTK_FLP_LOCATION_T))) {
                        //LOGD("size check: %d %d \n", (cnn2flp_msg->length), sizeof(MTK_FLP_LOCATION_T));
                        memcpy(&loc_in, ((char*)cnn2flp_msg)+sizeof(MTK_FLP_MSG_T), sizeof(MTK_FLP_LOCATION_T));
                        mtk_flp_dc_loc_rearrange(&loc_in, &loc_out);

                        //send loc to FLP HAL
                        flp2hal_msg = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(MTK_FLP_LOCATION_T));
                        if(flp2hal_msg == NULL) {
                            LOGE("report loc malloc failed");
                            free(cnn2flp_msg);
                            return MTK_FLP_ERROR;
                        }
                        flp2hal_msg->type = cnn2flp_msg->type;
                        flp2hal_msg->length = sizeof(MTK_FLP_LOCATION_T);
                        memcpy((char *)flp2hal_msg + sizeof(MTK_FLP_MSG_T),&loc_out, sizeof(MTK_FLP_LOCATION_T) );
                        put_binary(buff, &offset, (const char*)flp2hal_msg, (sizeof(MTK_FLP_MSG_T)+flp2hal_msg->length));
                        ret = mnl_send2flp(buff, offset);
                        free(flp2hal_msg);
                        if(ret < 0) {
                            LOGD("MTK_FLP2HAL send error return error");
                        }
                    }
                    break;
                default:
                    if(cnn2flp_msg->type < MTK_FLP_MSG_END) {
                        put_binary(buff, &offset, (const char*)cnn2flp_msg, sizeof(MTK_FLP_MSG_T)+cnn2flp_msg->length);
                        ret = mnl_send2flp(buff, offset);
                        if(ret < 0) {
                            LOGD("MTK_FLP2HAL send error return error");
                        }
                    }
                    break;
            }
            free(cnn2flp_msg);
            break;
        case MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE:
            fgMnldStatus = MNL_STATUS_HBD_GPS_OPEN_DONE;
            LOGD("MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE");
            break;
        case MNLD_FLP_TYPE_HBD_GPS_CLOSE_DONE:
            fgMnldStatus = MNL_STATUS_HBD_GPS_CLOSE_DONE;
            LOGD("MNL_STATUS_HBD_GPS_CLOSE_DONE");
            break;
        case MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE:
            if (fgMnldStatus == MNL_STATUS_OFL_LINK_OPEN) {
                fgMnldStatus = MNL_STATUS_OFL_LINK_OPEN_DONE;
                LOGD("MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE flush %d", gFlp2mnl_cnt);
                for (i = 0; i < gFlp2mnl_cnt; i++) {
                    flp2mnl_hdlr(&gFlp2mnl_msg[i]);
                }
                gFlp2mnl_cnt = 0;
            } else {
                LOGD("FLP_DBG: mnld state expect 7, get %u", fgMnldStatus);
            }
            LOGD("MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE");
            break;
        case MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE:
            fgMnldStatus = MNL_STATUS_OFL_LINK_CLOSE_DONE;
            LOGD("MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE");
            break;
        default:
            LOGE("undefined cmd:0x%02x",prmsg->type);
            break;
    }
    return MTK_FLP_SUCCESS;
}

