#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "mtk_flp_controller.h"
#include "data_coder.h"

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
#define LOG_TAG "flpmain"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif

#define ONE_SEC_NS 1000000000

/**********************************************************
 *  Global vars                                           *
 **********************************************************/
char pFLPVersion[] = {FLP_VER_INFO,0x00};
static unsigned int flp_enabled = 0, flp_source_to_use = 0;
static UINT32 flp_running_source = 0;
static int flp_batch_mode = MTK_FLP_IDLE_MODE;
static int g_loc_available = -1;
static int64_t g_flp_period_ns = 0;
static int64_t g_flp_update_last_time = 0;
static float current_gnss_acc = 0;


int64_t mtk_flp_get_timestamp(struct timeval *ptv) {
    struct timeval tv;

    if(ptv) {
        return ((unsigned long long)ptv->tv_sec*1000ULL+(unsigned long long)ptv->tv_usec/1000ULL);
    } else {
        gettimeofday(&tv, NULL);
        return ((unsigned long long)tv.tv_sec*1000ULL + (unsigned long long)tv.tv_usec/1000ULL);
    }
}

unsigned int mtk_flp_sys_get_time_tick() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec;
}

int mtk_flp_controller_query_mode () {
    return flp_batch_mode;
}

void mtk_flp_controller_check_loc_available_expired() {
    int current_available;
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    int64_t current_time = (int64_t)mtk_flp_sys_get_time_tick();

    if (mtk_flp_controller_query_mode() == MTK_FLP_IDLE_MODE) {
        return;
    }

    if ((current_time - g_flp_update_last_time) > (g_flp_period_ns/ONE_SEC_NS + 1)) {
        current_available = 1;
    } else {
        current_available = 0;
    }
    if (g_loc_available != current_available) {
        g_loc_available = current_available;

        flp_header = malloc(sizeof(MTK_FLP_MSG_T)+ sizeof(int));
        if(flp_header==NULL) {
            LOGE("report status malloc failed");
            return;
        }
        flp_header->type = MTK_FLP_MSG_HAL_REPORT_STATUS_NTF;
        flp_header->length = sizeof(int);
        memcpy((char *)flp_header+sizeof(MTK_FLP_MSG_T), &g_loc_available, sizeof(int));
        put_binary(buff, &offset, (const char*)flp_header, (sizeof(MTK_FLP_MSG_T) + flp_header->length));
        ret = mnl_send2flp(buff, offset);
        free(flp_header);
        if(ret < 0) {
            LOGE("MTK_FLP2HAL send error return error");
        }
    }
}

int mtk_flp_controller_report_loc(MTK_FLP_LOCATION_T outloc) {
    unsigned char update_flploc = 0;
    int64_t current_time = 0;

    // one second timer
    if (flp_enabled) {
        mtk_flp_controller_check_loc_available_expired();
    } else {
        LOGE("flp bypass, no source open");
        return MTK_FLP_ERROR;
    }

    current_gnss_acc = outloc.accuracy;
    if (outloc.accuracy < 0.001) {
        LOGE("bypass loc - loc no fix");
        return MTK_FLP_ERROR;
    }

    // Location fix
    if (g_flp_update_last_time == 0) {
        g_flp_update_last_time = (int64_t)(mtk_flp_sys_get_time_tick());
        update_flploc = 1;
    } else {
        current_time = (int64_t)(mtk_flp_sys_get_time_tick());
        if(g_flp_update_last_time > current_time) {
            g_flp_update_last_time = 0;
        }

        if((current_time - g_flp_update_last_time) >= g_flp_period_ns/ONE_SEC_NS) {
            update_flploc = 1;
            g_flp_update_last_time = current_time;
        }
    }
    if (!update_flploc) {
        LOGE("bypass loc - on period");
        return MTK_FLP_ERROR;
    }
#if 0
    LOGD("Location,LNG:%f LAT:%f ALT:%f ACC:%f SPD:%f BEARING:%f, FLAGS:%04X SOURCES:%08X Timestamp:%lld",
    outloc.longitude, outloc.latitude, outloc.altitude, outloc.accuracy,
    outloc.speed, outloc.bearing, outloc.flags, outloc.sources_used, outloc.timestamp);
#endif
    return MTK_FLP_SUCCESS;
}


void mtk_flp_controller_update_option(FlpBatchOptions *option) {
    UINT32 local_source = 0;
    UINT32 new_source = 0;
    //UINT32 temp_start = 0;
    MTK_FLP_MSG_T *flp_cmd;
    MTK_FLP_BATCH_OPTION_T batch_option;

    //assign stop command;
    memset(&batch_option,0,sizeof(MTK_FLP_BATCH_OPTION_T));
    if(option != NULL) {
        local_source = flp_running_source;
        new_source = flp_source_to_use;
        LOGD("local:%d,new:%d",local_source,new_source);
        if(local_source != new_source) {
            if((local_source == 0) && (new_source >= 1)) { //new req is open dc_xxx
                current_gnss_acc = 0;
                g_flp_update_last_time = 0;
                flp_cmd = malloc(sizeof(MTK_FLP_BATCH_OPTION_T)+ sizeof(MTK_FLP_MSG_T));
                if(flp_cmd == NULL) {
                    LOGE("update opt failed");
                    return;
                }
                flp_cmd->type = MTK_FLP_MSG_DC_START_CMD;
                flp_cmd->length = sizeof(MTK_FLP_BATCH_OPTION_T);
                batch_option.flags = flp_batch_mode;
                batch_option.max_power_allocation_mW = option->max_power_allocation_mW;
                batch_option.period_ns = g_flp_period_ns;
                batch_option.sources_to_use = FLP_TECH_MASK_GNSS;
                memcpy((char *)flp_cmd + sizeof(MTK_FLP_MSG_T), &batch_option, sizeof(MTK_FLP_BATCH_OPTION_T));
                mtk_flp2mnl_process(flp_cmd);
                free(flp_cmd);
            } else if((local_source >= 1) && (new_source == 0)) {//new req is close dc_xxx
                flp_cmd = malloc(sizeof(MTK_FLP_MSG_T));
                if(flp_cmd == NULL) {
                    LOGE("update opt failed");
                    return;
                }
                flp_cmd->type = MTK_FLP_MSG_DC_STOP_CMD;
                flp_cmd->length = 0;
                mtk_flp2mnl_process(flp_cmd);
                free(flp_cmd);
            }
        }
        flp_running_source = flp_source_to_use;
    }
}

int mtk_flp_controller_reset_status(void) {
    LOGD("clear flp ap status");
    flp_enabled = 0;
    flp_source_to_use = 0;
    flp_running_source =0;
    flp_batch_mode = MTK_FLP_IDLE_MODE;
    g_flp_period_ns = 0;
    current_gnss_acc = 0;
    return MTK_FLP_SUCCESS;
}

//*****************************************************************************
//  Low Power Main Function:
//  1. Obtain Source_to_use(FLP & geofence) from FLP mtk_flp_input
//  2. Relay command to FLP kernel
//
//  AP --> FLP HAL Offload (mtk_flp_input) --> Low power --> FLP kernel --> DC
//*****************************************************************************
int mtk_flp_controller_process(MTK_FLP_MSG_T *prmsg) {
    FlpBatchOptions option;
    //MTK_FLP_MSG_T *flp_msg = NULL;
    //int result =MTK_FLP_ERROR;
    //int get_loc_num = MTK_FLP_ERROR;

    memset(&option, 0, sizeof(option));

    if(prmsg == NULL) {
        LOGE("mtk_flp_controller_process, recv prmsg is null pointer\r\n");
        return MTK_FLP_ERROR;
    }

    LOGD("mtk_flp_controller_process, recv prmsg, type: 0x%02x, len:%d\r\n", prmsg->type, prmsg->length);

    switch( prmsg->type ) {
        case MTK_FLP_MSG_HAL_START_CMD:      //process init request from HAL
            LOGD("%s", pFLPVersion);
            if(prmsg->length >= (int)(sizeof(FlpBatchOptions))) {
                memcpy(&option, (FlpBatchOptions *)( (unsigned char*)prmsg+sizeof(MTK_FLP_MSG_T) ), sizeof(FlpBatchOptions));
                flp_enabled = 1;
                flp_batch_mode = option.flags;
                g_flp_period_ns = option.period_ns;
                flp_source_to_use = option.sources_to_use; //update flp source
                mtk_flp_controller_update_option(&option);
                LOGD("low power enabled %d %d %lld", flp_source_to_use, flp_enabled, g_flp_period_ns);
            } else {
                LOGE("incrorrect size %d",prmsg->length);
            }
            break;
        case MTK_FLP_MSG_HAL_STOP_CMD:
            LOGD("stop flp");
            memset(&option, 0, sizeof(FlpBatchOptions));
            flp_enabled = 0;
            flp_source_to_use = 0;
            option.sources_to_use = flp_source_to_use;
            mtk_flp_controller_update_option(&option);
            break;
        case MTK_FLP_MSG_HAL_SET_OPTION_CMD:
            if(!flp_enabled) {
                LOGD("hal start");
            }
            flp_enabled = 1;
            if(prmsg->length >= (int)(sizeof(FlpBatchOptions))) {
                memcpy(&option, (FlpBatchOptions *)( (UINT8*)prmsg+sizeof(MTK_FLP_MSG_T) ), sizeof(FlpBatchOptions));

                if(g_flp_period_ns != option.period_ns) {
                    if(option.period_ns > 0) {
                        //LOGD("update period from %lld to %lld",g_flp_period_ns,option.period_ns);
                        g_flp_period_ns = option.period_ns;
                    }
                }

                if(flp_batch_mode != (int)option.flags) {
                    LOGD("update batch mode from %d to %d",flp_batch_mode,option.flags);
                    flp_batch_mode = option.flags;
                }

                if(flp_source_to_use != option.sources_to_use) {
                    LOGD("update source mode from %d to %d",flp_source_to_use,option.sources_to_use);
                    flp_source_to_use = option.sources_to_use;
                }
                mtk_flp_controller_update_option(&option);
            } else {
                LOGE("incrorrect size %d",prmsg->length);
            }
            break;
        default:
            LOGD("lowpower pass through");
            break;
    }
    return MTK_FLP_SUCCESS;
}

