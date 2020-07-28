#include "mtk_geofence_main.h"
#include "mtk_flp_main.h"
#include "data_coder.h"
#include <log/log.h>     /*logging in logcat*/
#include <string.h>

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

#define ONE_SEC_NS 1000000000
#define FLP_TECH_MASK_GNSS      (1U<<0)
#define MAX_GFC_BATCHING_MSG   100

/**********************************************************
*  Global vars                                           *
**********************************************************/
static unsigned int fgMnldGfcStatus = MNL_STATUS_NONE;
static unsigned int gCurrentSession = 0;
static unsigned int gCurrentSupport = 0;
static unsigned int geofence_enabled = 0;
static unsigned int flp_geofence_source = 0;
static int gfc_mode = MTK_GEOFENCE_IDLE_MODE;
static int32_t geofence_in_use = 0;
static uint32_t geofence_source_to_use = 0;
static float current_gnss_acc = 0;
char pGFCVersion[] = {GEOFENCE_VER_INFO,0x00};
static MTK_GFC_MNL_MSG_T gGfc2mnl_msg[MAX_GFC_BATCHING_MSG];
static int gGfc2mnl_cnt = 0;

/**********************************************************
*  Function Declaration                                  *
**********************************************************/
extern void mtk_geofence_offload_main(MTK_FLP_MSG_T *prmsg);
extern void mtk_flp_geofence_expire();

/************************************************************************/
//  Process msgs from GPS/GFC HAL and send to offload kernel
/************************************************************************/
void mtk_gfc_ofl2mnl_process(MTK_FLP_MSG_T *prmsg) {
    MTK_GFC_MNL_MSG_HEADER_T* gfc2mnl_msg = NULL;
    if (prmsg->type == MTK_FLP_MSG_CONN_SCREEN_STATUS) {
        if(fgMnldGfcStatus == MNL_STATUS_OFL_LINK_OPEN_DONE) {
            gfc2mnl_msg = (MTK_GFC_MNL_MSG_HEADER_T *)malloc( sizeof(MTK_GFC_MNL_MSG_HEADER_T)+ sizeof(MTK_FLP_MSG_T)+prmsg->length);
            if(gfc2mnl_msg != NULL) {
                gfc2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_SEND;
                gfc2mnl_msg->session = gCurrentSession;
                gfc2mnl_msg->length = (prmsg->length) + sizeof(MTK_FLP_MSG_T);
                memcpy(((char*)gfc2mnl_msg)+sizeof(MTK_GFC_MNL_MSG_HEADER_T), ((char*)prmsg), (sizeof(MTK_FLP_MSG_T)+ (prmsg->length)));
                gfc2mnl_hdlr((MTK_GFC_MNL_MSG_T *)gfc2mnl_msg);
                free(gfc2mnl_msg);
            }
        } else {
            LOGD("free screen dbg");
        }
        return;
    }
    if (fgMnldGfcStatus != MNL_STATUS_OFL_LINK_OPEN_DONE) {
        if (gGfc2mnl_cnt == 0) {
            //send init message to mnld first
            gfc2mnl_msg = (MTK_GFC_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_GFC_MNL_MSG_HEADER_T));
            //send header only, no message body
            if(gfc2mnl_msg != NULL) {
                gfc2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_OPEN;
                gfc2mnl_msg->session = gCurrentSession;
                gfc2mnl_msg->length = 0;
                LOGD("MNLD_FLP_TYPE_OFL_LINK_OPEN");
                fgMnldGfcStatus = MNL_STATUS_OFL_LINK_OPEN;
                gfc2mnl_hdlr((MTK_GFC_MNL_MSG_T *)gfc2mnl_msg);
                free(gfc2mnl_msg);
            }
        }
        if (gGfc2mnl_cnt < MAX_GFC_BATCHING_MSG) {
            gGfc2mnl_msg[gGfc2mnl_cnt].type = MNLD_FLP_TYPE_OFL_LINK_SEND;
            gGfc2mnl_msg[gGfc2mnl_cnt].length = prmsg->length + sizeof(MTK_FLP_MSG_T);
            gGfc2mnl_msg[gGfc2mnl_cnt].session = gCurrentSession;
            memcpy(&gGfc2mnl_msg[gGfc2mnl_cnt].data[0], ((char*)prmsg), (sizeof(MTK_FLP_MSG_T)+ prmsg->length));
            gGfc2mnl_cnt++;
            LOGD("Keep gfc data cnt %d", gGfc2mnl_cnt);
        } else {
            LOGD("Batching size overflow");
        }
    } else {
        if ((sizeof(MTK_GFC_MNL_MSG_HEADER_T)+sizeof(MTK_FLP_MSG_T)+prmsg->length) <= 0){
            LOGW("Input Zero or Negative length");
            return;
        }
        //send flp message to ofl gfc
        gfc2mnl_msg = (MTK_GFC_MNL_MSG_HEADER_T *)malloc( sizeof(MTK_GFC_MNL_MSG_HEADER_T)+sizeof(MTK_FLP_MSG_T)+prmsg->length);
        if(gfc2mnl_msg != NULL) {
            gfc2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_SEND;
            gfc2mnl_msg->session = gCurrentSession;
            gfc2mnl_msg->length = prmsg->length + sizeof(MTK_FLP_MSG_T);
            memcpy( ((char*)gfc2mnl_msg)+sizeof(MTK_GFC_MNL_MSG_HEADER_T), ((char*)prmsg), (sizeof(MTK_FLP_MSG_T)+ prmsg->length));
            LOGD("Send to OFL GFC,message type:0x%02x, len:%u", prmsg->type, (gfc2mnl_msg->length + sizeof(MTK_GFC_MNL_MSG_HEADER_T)));
            gfc2mnl_hdlr((MTK_GFC_MNL_MSG_T *)gfc2mnl_msg);
            free(gfc2mnl_msg);
        }
    }
    return;
}

int mtk_gfc_controller_query_mode () {
    return gfc_mode;
}

float mtk_flp_controller_get_current_acc() {
    return current_gnss_acc;
}

void mtk_smart_geofence_entry(int mode, unsigned int duration_sec) {
    LOGD("Smart geofence entry mode:%d timeout:%u", mode, duration_sec);
    gfc_mode = mode;
}

void mtk_smart_geofence_exit() {
    gfc_mode = MTK_BATCH_GEOFENCE_ONLY;
}

static int mtk_loc_check(GpsLocation* gpsloc, MTK_FLP_LOCATION_T *flploc) {
    if (gpsloc == NULL || flploc == NULL) {
        return MTK_GFC_ERROR;
    }
    memcpy(flploc, gpsloc, sizeof(GpsLocation));
    flploc->size = sizeof(MTK_FLP_LOCATION_T);
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
    return MTK_GFC_SUCCESS;
}

/************************************************************************/
//  Process msgs from GFC HAL and send to offload kernel
/************************************************************************/
int mtk_gfc2mnl_process(MTK_FLP_MSG_T *prmsg) {
    MTK_GFC_MNL_MSG_HEADER_T* gfc2mnl_msg = NULL;
    if(prmsg == NULL) {
        LOGE("mtk_gfc2mnl_process, recv prmsg is null pointer\n");
        return MTK_GFC_ERROR;
    }
    //LOGD("mtk_gfc2mnl_process, recv prmsg, type: 0x%02x, len:%u status:%u\r\n", prmsg->type, prmsg->length, fgMnldGfcStatus);
    switch(prmsg->type) {
        case MTK_FLP_MSG_DC_START_CMD: // host-based start gnss
            //send init message to mnld first
            gfc2mnl_msg = (MTK_GFC_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_GFC_MNL_MSG_HEADER_T));
            //send header only, no message body
            if(gfc2mnl_msg != NULL) {
                gfc2mnl_msg->type = MNLD_FLP_TYPE_HBD_GPS_OPEN;
                gfc2mnl_msg->length = 0;
                gfc2mnl_msg->session = gCurrentSession;
                LOGD("MNLD_FLP_TYPE_HBD_GPS_OPEN");
                fgMnldGfcStatus = MNL_STATUS_HBD_GPS_OPEN;
                gfc2mnl_hdlr((MTK_GFC_MNL_MSG_T *)gfc2mnl_msg);
                free(gfc2mnl_msg);
            }
            break;
        case MTK_FLP_MSG_DC_STOP_CMD:
            //send stop message to mnld
            gfc2mnl_msg = (MTK_GFC_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_GFC_MNL_MSG_HEADER_T));
            //send header only, no message body
            if(gfc2mnl_msg != NULL) {
                gfc2mnl_msg->type = MNLD_FLP_TYPE_HBD_GPS_CLOSE;
                gfc2mnl_msg->length = 0;
                gfc2mnl_msg->session = gCurrentSession;
                //LOGD("Send to mnld for deinit,message type:0x%02x, len:%d", gfc2mnl_msg->type, gfc2mnl_msg->length);
                fgMnldGfcStatus = MNL_STATUS_HBD_GPS_CLOSE;
                gfc2mnl_hdlr((MTK_GFC_MNL_MSG_T *)gfc2mnl_msg);
                free(gfc2mnl_msg);
            }
            break;
    }
    return MTK_GFC_SUCCESS;
}

void mtk_gfc_controller_update_option() {
    unsigned int local_source = 0;
    unsigned int new_source = 0;
    //unsigned int temp_start = 0;
    MTK_FLP_MSG_T *flp_cmd;
    MTK_FLP_BATCH_OPTION_T batch_option;
    //assign stop command;
    memset(&batch_option,0,sizeof(MTK_FLP_BATCH_OPTION_T));
    local_source = flp_geofence_source;
    new_source = geofence_source_to_use;
    if(local_source != new_source) {
        if((local_source == 0) && (new_source == 1)) {
            //new req is open dc_xxx
            current_gnss_acc = 0;
            flp_cmd = malloc(sizeof(MTK_FLP_BATCH_OPTION_T)+ sizeof(MTK_FLP_MSG_T));
            if(flp_cmd != NULL) {
                flp_cmd->type = MTK_FLP_MSG_DC_START_CMD;
                flp_cmd->length = sizeof(MTK_FLP_BATCH_OPTION_T);
                batch_option.flags = gfc_mode;
                batch_option.max_power_allocation_mW = 0;
                batch_option.period_ns = ONE_SEC_NS;
                batch_option.sources_to_use = FLP_TECH_MASK_GNSS;
                memcpy((INT8 *)flp_cmd + sizeof(MTK_FLP_MSG_T), &batch_option, sizeof(MTK_FLP_BATCH_OPTION_T));
                mtk_gfc2mnl_process(flp_cmd);
                free(flp_cmd);
            } else {
                LOGE("open DC: malloc failed");
            }
        } else if((local_source == 1) && (new_source == 0)) {
            //new req is close dc_xxx
            flp_cmd = malloc(sizeof(MTK_FLP_MSG_T));
            if(flp_cmd != NULL) {
                flp_cmd->type = MTK_FLP_MSG_DC_STOP_CMD;
                flp_cmd->length = 0;
                mtk_gfc2mnl_process(flp_cmd);
                free(flp_cmd);
            } else {
                LOGE("close DC malloc failed");
            }
        }
    }
    flp_geofence_source = geofence_source_to_use;
}

int mtk_gfc_controller_reset_status(void) {
    geofence_enabled = 0;
    gfc_mode = MTK_GEOFENCE_IDLE_MODE;
    geofence_in_use = 0;
    geofence_source_to_use = 0;
    current_gnss_acc = 0;
    flp_geofence_source = 0;
    mtk_flp_geofence_clear_geofences();
    return MTK_GFC_SUCCESS;
}

int mtk_gfc_controller_process(MTK_FLP_MSG_T *prmsg) {
    MTK_FLP_MSG_T *geofence_msg = NULL;
    //MTK_GFC_MNL_MSG_HEADER_T gfc2mnl_msg;
    MTK_GEOFENCE_PROPERTY_T geofence_input;
    int32_t fence_num;
    if(prmsg == NULL) {
        LOGE("mtk_gfc_controller_process, recv prmsg is null pointer\r\n");
        return MTK_GFC_ERROR;
    }
    LOGD("mtk_gfc_controller_process, recv prmsg, type:%d, len:%d\r\n", prmsg->type, prmsg->length);
    switch( prmsg->type ) {
        case MTK_FLP_MSG_OFL_GEOFENCE_CMD:
            if (((prmsg->length) - sizeof(MTK_FLP_MSG_T)) <= 0){
                LOGW("Input Zero or Negative length");
                return MTK_GFC_ERROR;
            }
            geofence_msg = malloc((prmsg->length)-sizeof(MTK_FLP_MSG_T));
            if(geofence_msg != NULL ) {
                memcpy( geofence_msg, ((char*)prmsg)+sizeof(MTK_FLP_MSG_T), ((prmsg->length)-sizeof(MTK_FLP_MSG_T)));
                if(geofence_enabled) {
                    if (mtk_gfc_controller_query_mode() > MTK_BATCH_GEOFENCE_ONLY) {
                        LOGD("DEV_GEO: Smart geofence exit from geofence");
                        mtk_smart_geofence_exit();
                    }
                }
                mtk_geofence_offload_main(geofence_msg);
                if( (geofence_msg->type == ADD_GEOFENCE_AREA) || (geofence_msg->type == RECOVER_GEOFENCE) ) {
                    LOGD("%s", pGFCVersion);
                    memcpy(&fence_num, ((char *)geofence_msg + sizeof(MTK_FLP_MSG_T)), sizeof(int32_t));
                    memcpy(&geofence_input, ((char *)geofence_msg + sizeof(MTK_FLP_MSG_T)+sizeof(int32_t)),sizeof(MTK_GEOFENCE_PROPERTY_T));
                    if (geofence_in_use > 0) {
                        geofence_source_to_use = update_fence_source();
                    } else {
                        // first fence
                        geofence_source_to_use = 1;
                    }
                    if (geofence_in_use == 0) {
                        gfc_mode = MTK_BATCH_GEOFENCE_ONLY;
                    }
                    mtk_gfc_controller_update_option();
                    geofence_enabled = 1;
                    geofence_in_use++;
                } else if(geofence_msg->type == REMOVE_GEOFENCE ) {
                    memcpy( &fence_num, ((char *)geofence_msg + sizeof(MTK_FLP_MSG_T)), sizeof(int32_t));
                    if(geofence_in_use > 0) {
                        geofence_in_use -= 1;
                    }
                    geofence_source_to_use = update_fence_source();
                    mtk_gfc_controller_update_option();
                    if( geofence_in_use == 0 ) {
                        geofence_enabled = 0;
                        gfc_mode = MTK_GEOFENCE_IDLE_MODE;
                    }
                } else if(geofence_msg->type == CLEAR_GEOFENCE) {
                    geofence_in_use = 0;
                    geofence_enabled = 0;
                    geofence_source_to_use = 0;
                    mtk_gfc_controller_update_option();
                    gfc_mode = MTK_GEOFENCE_IDLE_MODE;
                }
                free(geofence_msg);
            }
            break;
        default:
            LOGE("unknown cmd:0x%02x",prmsg->type);
            break;
    }
    return MTK_GFC_SUCCESS;
}

/************************************************************************/
//  Handle MNL Response                                                 */
/************************************************************************/
int mnl2gfc_hdlr(MTK_GFC_MNL_MSG_T* prmsg) {
    //char localbuf[512];
    char buff[GFC_MNL_BUFF_SIZE] = {0};
    //char *loc_ntf = NULL;
    //unsigned char* msg_dbg = NULL;
    //unsigned int msg_size=0;
    int ret, i;//, retlen, network_status;
    int offset = 0;
    MTK_GFC_MNL_MSG_HEADER_T *gfc2mnl_msg;
    MTK_FLP_MSG_T *gfc2hal_msg;
    MTK_FLP_MSG_T *cnn2gfc_msg;
    //MTK_FLP_LOCATION_T  loc_in, loc_out;
    GpsLocation  *gps_loc;
    FlpLocation  flp_loc;

    if(prmsg == NULL) {
        LOGE("mnl2gfc_hdlr recv null msg");
        return MTK_GFC_ERROR;
    }
    LOGD("mtk_mnl2gfc_recv_msg type = %u, session = %u, len = %u, state = %u\n",  prmsg->type, prmsg->session, prmsg->length, fgMnldGfcStatus);
    switch(prmsg->type) {
        case MNLD_FLP_TYPE_MNL_BOOTUP:
            fgMnldGfcStatus = MNL_STATUS_ATTACH_DONE;
            gfc2hal_msg = malloc(sizeof(MTK_FLP_MSG_T));
            if(gfc2hal_msg == NULL) {
                LOGE("reset ntf malloc failed");
                return MTK_FLP_ERROR;
            }
            gfc2hal_msg->type = MTK_FLP_MSG_SYS_FLPD_RESET_NTF;
            gfc2hal_msg->length = 0;
            put_binary(buff, &offset, (const char*)gfc2hal_msg, sizeof(MTK_FLP_MSG_T));
            ret = mnl_send2gfc(buff, offset);
            free(gfc2hal_msg);
            if(ret < 0) {
                LOGD("mnl2gfc_hdlr send error return error");
            }
            break;
        case MNLD_FLP_TYPE_SESSION_UPDATE:
            gCurrentSession = prmsg->session;
            memcpy(&gCurrentSupport, ((char*)prmsg)+sizeof(MTK_GFC_MNL_MSG_HEADER_T), sizeof(gCurrentSupport));
            LOGD("SESSION_UPDATE %u support %u mode %u", gCurrentSession, gCurrentSupport, fgMnldGfcStatus);
            if (fgMnldGfcStatus == MNL_STATUS_HBD_GPS_OPEN_DONE && gCurrentSupport == GFC_AP_MODE) {
                LOGD("SESSION_UPDATE: already run on host");
            } else if (fgMnldGfcStatus == MNL_STATUS_OFL_LINK_OPEN_DONE && (gCurrentSupport & GFC_OFFLOAD_MODE)) {
                LOGD("SESSION_UPDATE: already run on offlaod");
            } else if (fgMnldGfcStatus == MNL_STATUS_HBD_GPS_OPEN_DONE && (gCurrentSupport & GFC_OFFLOAD_MODE)) {
                LOGD("SESSION_UPDATE: switch to ofl mode");
                mtk_gfc_controller_reset_status();
                mtk_gfc_set_sys_mode(GFC_OFFLOAD_MODE);
                gfc2hal_msg = malloc(sizeof(MTK_FLP_MSG_T));
                if(gfc2hal_msg == NULL) {
                    LOGE("reset ntf malloc failed");
                    return MTK_FLP_ERROR;
                }
                gfc2hal_msg->type = MTK_FLP_MSG_SYS_FLPD_RESET_NTF;
                gfc2hal_msg->length = 0;
                put_binary(buff, &offset, (const char*)gfc2hal_msg, sizeof(MTK_FLP_MSG_T));
                ret = mnl_send2gfc(buff, offset);
                free(gfc2hal_msg);
                if(ret < 0) {
                    LOGE("mnl2gfc_hdlr send error return error");
                }
            } else if (fgMnldGfcStatus == MNL_STATUS_OFL_LINK_OPEN_DONE && (gCurrentSupport == GFC_AP_MODE)) {
                LOGD("SESSION_UPDATE: switch to ap mode");
                mtk_gfc_set_sys_mode(GFC_AP_MODE);
                gfc2hal_msg = malloc(sizeof(MTK_FLP_MSG_T));
                if(gfc2hal_msg == NULL) {
                    LOGE("reset ntf malloc failed");
                    return MTK_FLP_ERROR;
                }
                gfc2hal_msg->type = MTK_FLP_MSG_SYS_FLPD_RESET_NTF;
                gfc2hal_msg->length = 0;
                put_binary(buff, &offset, (const char*)gfc2hal_msg, sizeof(MTK_FLP_MSG_T));
                ret = mnl_send2gfc(buff, offset);
                free(gfc2hal_msg);
                if(ret < 0) {
                    LOGE("mnl2gfc_hdlr send error return error");
                }
            } else {
                if (gCurrentSupport & GFC_OFFLOAD_MODE) {
                    mtk_gfc_set_sys_mode(GFC_OFFLOAD_MODE);
                    LOGD("SESSION_UPDATE: set offload mode");
                } else if (gCurrentSupport == GFC_AP_MODE) {
                    mtk_gfc_set_sys_mode(GFC_AP_MODE);
                    LOGD("SESSION_UPDATE: set ap mode");
                }
            }
            break;
        case MNLD_FLP_TYPE_HBD_GPS_LOCATION:
            if (!flp_geofence_source) {
                LOGE("bypass loc - no source open");
                return MTK_GFC_ERROR;
            }
            gps_loc = (GpsLocation *)(((char*)prmsg)+sizeof(MTK_GFC_MNL_MSG_HEADER_T));
            //LOGD("GNSS LOC DBG: %f, %lf, %lf, %d", gps_loc->accuracy, gps_loc->latitude, gps_loc->longitude, gps_loc->size);
            if(mtk_loc_check(gps_loc, (MTK_FLP_LOCATION_T *)&flp_loc) != MTK_GFC_SUCCESS) {
                LOGE("Incorrect data received");
            } else {
                if (geofence_enabled) {
                    mtk_flp_geofence_expire();
                    current_gnss_acc = flp_loc.accuracy;
                    if (flp_loc.accuracy < 0.001) {
                        LOGE("bypass loc - loc no fix");
                        return MTK_GFC_ERROR;
                    }
                    mtk_set_geofence_location(&flp_loc);
                    //LOGD("Location,LNG:%f LAT:%f ALT:%f ACC:%f SPD:%f BEARING:%f, FLAGS:%04X SOURCES:%08X Timestamp:%lld",
                    //    flp_loc.longitude, flp_loc.latitude, flp_loc.altitude, flp_loc.accuracy,
                    //    flp_loc.speed, flp_loc.bearing, flp_loc.flags, flp_loc.sources_used, flp_loc.timestamp);
                }
            }
            break;
        case MNLD_FLP_TYPE_OFL_LINK_RECV:
            if (prmsg->length <= 0){
                LOGW("Input Zero or Negative length");
                return MTK_GFC_ERROR;
            }
            cnn2gfc_msg = malloc(prmsg->length);
            if(cnn2gfc_msg == NULL) {
                LOGE("cnn2gfc_msg malloc failed");
                return MTK_GFC_ERROR;
            }
            memcpy((char*)cnn2gfc_msg, ((char*)prmsg)+ sizeof(MTK_GFC_MNL_MSG_HEADER_T), prmsg->length);
            //LOGD("Received OFFLOAD message type: 0x%02x, len = %u", cnn2gfc_msg->type, cnn2gfc_msg->length);
            switch(cnn2gfc_msg->type) {
                case MTK_FLP_MSG_HAL_STOP_RSP:
                    //send stop message to mnld
                    gfc2mnl_msg = (MTK_GFC_MNL_MSG_HEADER_T *)malloc(sizeof(MTK_GFC_MNL_MSG_HEADER_T));
                    if(gfc2mnl_msg!= NULL) {
                        gfc2mnl_msg->type = MNLD_FLP_TYPE_OFL_LINK_CLOSE;
                        gfc2mnl_msg->session= gCurrentSession;
                        gfc2mnl_msg->length = 0;
                        LOGD("Send to mnld for deinit,message type:%u, len:%u", gfc2mnl_msg->type, gfc2mnl_msg->length);
                        gfc2mnl_hdlr((MTK_GFC_MNL_MSG_T *)gfc2mnl_msg);
                        fgMnldGfcStatus = MNL_STATUS_OFL_LINK_CLOSE;
                        LOGD("mnld deinit success");
                        free(gfc2mnl_msg);
                    }
                    break;
                case MTK_FLP_MSG_HAL_GEOFENCE_CALLBACK_NTF:
                case MTK_FLP_MSG_OFL_GEOFENCE_CALLBACK_NTF:
                    put_binary(buff, &offset, (const char*)cnn2gfc_msg, sizeof(MTK_FLP_MSG_T)+cnn2gfc_msg->length);
                    ret = mnl_send2gfc(buff, offset);
                    if(ret < 0) {
                        LOGE("MTK_GFC2HAL send error return error");
                    }
                    break;
                default:
                    LOGE("mnl2gfc_hdlr unrecognize msg 0x%02x",cnn2gfc_msg->type);
                    break;
            }
            free(cnn2gfc_msg);
            break;
        case MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE:
            fgMnldGfcStatus = MNL_STATUS_HBD_GPS_OPEN_DONE;
            LOGD("MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE");
            break;
        case MNLD_FLP_TYPE_HBD_GPS_CLOSE_DONE:
            fgMnldGfcStatus = MNL_STATUS_HBD_GPS_CLOSE_DONE;
            LOGD("MNL_STATUS_HBD_GPS_CLOSE_DONE");
            break;
        case MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE:
            if (fgMnldGfcStatus == MNL_STATUS_OFL_LINK_OPEN) {
                fgMnldGfcStatus = MNL_STATUS_OFL_LINK_OPEN_DONE;
                LOGD("MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE flush %d", gGfc2mnl_cnt);
                for (i = 0; i < gGfc2mnl_cnt; i++) {
                    gfc2mnl_hdlr(&gGfc2mnl_msg[i]);
                }
                gGfc2mnl_cnt = 0;
            } else {
                LOGD("GFC_DBG: mnld state expect 7, get %u", fgMnldGfcStatus);
            }
            LOGD("MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE");
            break;
        case MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE:
            fgMnldGfcStatus = MNL_STATUS_OFL_LINK_CLOSE_DONE;
            LOGD("MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE");
            break;
        case MNLD_FLP_TYPE_FLP_ATTACH_DONE:
            fgMnldGfcStatus = MNL_STATUS_ATTACH_DONE;
            LOGD("MNL_STATUS_ATTACH_DONE");
            break;
        default:
            LOGE("undefined cmd:0x%02x",prmsg->type);
            break;
    }
    return MTK_GFC_SUCCESS;
}
