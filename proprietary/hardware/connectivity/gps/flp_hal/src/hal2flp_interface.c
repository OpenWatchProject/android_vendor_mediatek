#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#include <inttypes.h>
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
#define FLP_OPTION_MAX  10
#define GEOFENCE_INJECT_LOC_ENUM 600
#define MTK_GFC2HAL "mtk_gfc2hal"

/**********************************************************
 *  Global vars                                           *
 **********************************************************/
static FlpBatchOptions  gFlpOptions[FLP_OPTION_MAX];
static FlpBatchOptions  gFusedOptions;
static int gFlpFirstBatch = 1;

/**********************************************************
 *  Function Declaration                                  *
 **********************************************************/
static void mtk_flp_dump_options(FlpBatchOptions* options);
static void mtk_flp_set_options(int id, FlpBatchOptions* options);
static void mtk_flp_update_options();

/*********************************************************/
/* FLP Location Interface implementation                 */
/*********************************************************/
void flphal2mnl_flp_init() {
    memset(gFlpOptions, 0, FLP_OPTION_MAX*sizeof(FlpBatchOptions));
    return;
}

int flphal2mnl_flp_start(int id, FlpBatchOptions* options) {
    int  ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    if((options == NULL) || (!isflp_thread_exist())) {
        LOGE("isflp_thread_exist:%d",isflp_thread_exist());
        return MTK_FLP_ERROR;
    }

    mtk_flp_set_options(id, options);
    mtk_flp_update_options();
    if(options->sources_to_use == 0) {
        LOGD("mtk flp start zero sources");
        return MTK_FLP_SUCCESS;
    }
    if(gFlpFirstBatch == 1) {
        LOGD("first instance, add id = %d", id);
        flp_header = malloc(sizeof(MTK_FLP_MSG_T) + sizeof(FlpBatchOptions));
        if(flp_header== NULL) {
            LOGE("flp start malloc failed");
            return MTK_FLP_ERROR;
        }
        flp_header->type = MTK_FLP_MSG_HAL_START_CMD;
        flp_header->length = sizeof(FlpBatchOptions);
        memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&gFusedOptions, sizeof(FlpBatchOptions));
        put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
        ret = flp_send2mnl(buff, offset);
        free(flp_header);
        if(ret < 0) {
            LOGE("MTK_HAL2FLP send error return error");
            return MTK_FLP_ERROR;
        }
        gFlpFirstBatch = 0;
    } else {
        LOGD("update instance, add id = %d", id);
        flp_header = malloc(sizeof(MTK_FLP_MSG_T) + sizeof(FlpBatchOptions));
        if(flp_header== NULL) {
            LOGE("flp start malloc failed");
            return MTK_FLP_ERROR;
        }
        flp_header->type = MTK_FLP_MSG_HAL_SET_OPTION_CMD;
        flp_header->length = sizeof(FlpBatchOptions);
        memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&gFusedOptions, sizeof(FlpBatchOptions));
        put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
        ret = flp_send2mnl(buff, offset);
        free(flp_header);
        if(ret < 0) {
            LOGE("MTK_HAL2FLP send error return error");
            return MTK_FLP_ERROR;
        }
    }
    return MTK_FLP_SUCCESS;
}

int flphal2mnl_flp_reboot_done_ntf() {
    int  ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    if(!gFlpFirstBatch) {
        LOGD("FLP HAL RECOVER: send start cmd");
        flp_header = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(FlpBatchOptions));
        if(flp_header== NULL) {
            LOGE("flp reboot malloc failed");
            return MTK_FLP_ERROR;
        }
        flp_header->type = MTK_FLP_MSG_HAL_START_CMD;
        flp_header->length = sizeof(FlpBatchOptions);
        memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&gFusedOptions, sizeof(FlpBatchOptions));
        put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
        ret = flp_send2mnl(buff, offset);
        free(flp_header);
        if(ret < 0) {
            LOGE("MTK_HAL2FLP send error return error");
            return MTK_FLP_ERROR;
        }
    } else {
        LOGD("FLP HAL RECOVER: No need send start flp cmd");
    }
    return MTK_FLP_SUCCESS;
}

int flphal2mnl_update_batching_options(int id, FlpBatchOptions* options) {
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    if((options == NULL)||(!isflp_thread_exist())) {
        LOGE("isflp_thread_exist:%d",isflp_thread_exist());
        return MTK_FLP_ERROR;
    }
    mtk_flp_set_options(id, options);
    mtk_flp_update_options();

    //LOGD("update instance, update id = %d", id);
    flp_header = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(FlpBatchOptions));
    if(flp_header== NULL) {
        LOGE("flp update malloc failed");
        return MTK_FLP_ERROR;
    }
    flp_header->type = MTK_FLP_MSG_HAL_SET_OPTION_CMD;
    flp_header->length = sizeof(FlpBatchOptions);
    memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&gFusedOptions, sizeof(FlpBatchOptions));
    put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
    ret = flp_send2mnl(buff, offset);
    free(flp_header);
    if(ret < 0) {
        LOGE("MTK_HAL2FLP send error return error");
        return MTK_FLP_ERROR;
    }
    return MTK_FLP_SUCCESS;
}

int flphal2mnl_stop_batching(int id) {
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    if( id >= FLP_OPTION_MAX || id <0 || (!isflp_thread_exist())) {
        LOGD("Error batching id:%d, %d", id,isflp_thread_exist());
        return MTK_FLP_SUCCESS;
    }
    memset(&gFlpOptions[id], 0, sizeof(FlpBatchOptions));
    mtk_flp_update_options();

    if(gFusedOptions.sources_to_use == 0) {
        LOGD("stop all instance, last id = %d", id);
        flp_header = malloc(sizeof(MTK_FLP_MSG_T));
        if(flp_header== NULL) {
            LOGE("flp stop malloc failed");
            return MTK_FLP_ERROR;
        }
        flp_header->type = MTK_FLP_MSG_HAL_STOP_CMD;
        flp_header->length = 0;
        put_binary(buff, &offset, (const char*)flp_header, sizeof(MTK_FLP_MSG_T));
        ret = flp_send2mnl(buff, offset);
        free(flp_header);
        if(ret < 0) {
            LOGE("MTK_HAL2FLP send error return error");
            return MTK_FLP_ERROR;
        }
        gFlpFirstBatch = 1;
    } else {
        LOGD("update instance, stop id = %d", id);
        flp_header = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(FlpBatchOptions));
        if(flp_header== NULL) {
            LOGE("flp stop malloc failed");
            return MTK_FLP_ERROR;
        }
        flp_header->type = MTK_FLP_MSG_HAL_SET_OPTION_CMD;
        flp_header->length = sizeof(FlpBatchOptions);
        memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&gFusedOptions, sizeof(FlpBatchOptions));
        put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
        ret = flp_send2mnl(buff, offset);
        free(flp_header);
        if(ret < 0) {
            LOGE("MTK_HAL2FLP send error return error");
            return MTK_FLP_ERROR;
        }
    }
    return MTK_FLP_SUCCESS;
}

void flphal2mnl_get_batched_location(int last_n_locations) {
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    flp_header = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(int));
    if(flp_header== NULL) {
        LOGE("flp get batch malloc failed");
        return;
    }

    flp_header->type = MTK_FLP_MSG_HAL_REQUEST_LOC_NTF;
    flp_header->length = sizeof(int);
    memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),&last_n_locations, sizeof(int));
    put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
    ret = flp_send2mnl(buff, offset);
    free(flp_header);
    if(ret < 0) {
        LOGE("MTK_HAL2FLP send error return error");
    }
}

int flphal2mnl_inject_location(FlpLocation* location) {
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T *flp_header = NULL;

    if(location == NULL) {
        return MTK_FLP_ERROR;
    }

    flp_header = malloc(sizeof(MTK_FLP_MSG_T)+sizeof(FlpLocation));
    if(flp_header== NULL) {
        LOGE("flp inject loc malloc failed");
        return MTK_FLP_ERROR;
    }

    flp_header->type = MTK_FLP_MSG_HAL_INJECT_LOC_CMD;
    flp_header->length = sizeof(FlpLocation);
    memcpy((char *)flp_header +sizeof(MTK_FLP_MSG_T),location, sizeof(FlpLocation));
    put_binary(buff, &offset, (const char*)flp_header, (flp_header->length+sizeof(MTK_FLP_MSG_T)));
    ret = flp_send2mnl(buff, offset);
    free(flp_header);
    if(ret < 0) {
        LOGE("MTK_HAL2FLP send error return error");
        return MTK_FLP_ERROR;
    }
    return MTK_FLP_SUCCESS;
}

/*******************************************
  unicode to ASCII conversion
********************************************/
int Unicode2Ascii(char* unicode, char * ascii, int len) {
    int i;
    for(i = 0; i < len; i++) {
        ascii[i] = unicode[i*2];
    }
    return len;
}


/*********************************************************/
/* FLP Location Diagnostic Interface implementation      */
/*********************************************************/
int flphal2mnl_diag_inject_data(char* data, int length) {
    int i, ret, offset = 0;
    char asciibuf[1024];
    char buff[HAL_FLP_BUFF_SIZE] = {0};

    //geofence info injection
    float injectGeoInfo[4] = {0};
    char* injectGeoField;
    int injectGeoIndex = 0;
    MTK_FLP_MSG_T *flp_msg = NULL;

    if(data == NULL || length <= 0) {
        return MTK_FLP_ERROR;
    }
    Unicode2Ascii(data, asciibuf, length);
    asciibuf[length] = '\0';
    //LOGD("DIAG_IN:%s\n", asciibuf);

    if (strncmp(asciibuf, "RESET_FLP_DATA", 14) == 0) {
        // stop FLP
        if (!gFlpFirstBatch) {
            //LOGD("Reset FLP\n");
            for (i = 0; i < FLP_OPTION_MAX; i++) {
                memset(&gFlpOptions[i], 0, sizeof(FlpBatchOptions));
            }
            memset(&gFusedOptions, 0, sizeof(FlpBatchOptions));

            flp_msg = malloc(sizeof(MTK_FLP_MSG_T));
            if(flp_msg == NULL) {
                LOGE("diag inject alloc failed");
                return MTK_FLP_ERROR;
            }
            flp_msg->type = MTK_FLP_MSG_HAL_STOP_CMD;
            flp_msg->length = 0;
            put_binary(buff, &offset, (const char*)flp_msg, sizeof(MTK_FLP_MSG_T));
            ret = flp_send2mnl(buff, offset);
            free(flp_msg);
            if(ret < 0) {
                LOGE("MTK_HAL2FLP send error return error");
                return MTK_FLP_ERROR;
            }
            gFlpFirstBatch = 1;
        }
    } else if (strncmp(asciibuf, "GEO_COR", 7) == 0) {
        injectGeoField = strtok(asciibuf, ",");
        if (injectGeoField) {
            injectGeoField = strtok(NULL, ",");
        }
        while (injectGeoField && injectGeoIndex < 4) {
            injectGeoInfo[injectGeoIndex] = atof(injectGeoField);
            injectGeoIndex++;
            injectGeoField = strtok(NULL, ",");
        }
        flp_msg = malloc(sizeof(MTK_FLP_MSG_T)+3*sizeof(float));
        if(flp_msg == NULL) {
            LOGE("GEO_COR alloc failed");
            return MTK_FLP_ERROR;
        }
        flp_msg->type = GEOFENCE_INJECT_LOC_ENUM;
        flp_msg->length = 3*sizeof(float);
        memcpy((char *)flp_msg+sizeof(MTK_FLP_MSG_T),&injectGeoInfo[0],3*sizeof(float));
        put_binary(buff, &offset, (const char*)flp_msg, (flp_msg->length+sizeof(MTK_FLP_MSG_T)));
        ret = flp_send2mnl(buff, offset);
        free(flp_msg);
        if(ret < 0) {
            LOGE("GEOFENCE_INJECT_LOC_ENUM MTK_GFC2HAL send error return error");
            return MTK_FLP_ERROR;
        }
    }
    return MTK_FLP_SUCCESS;
}

void flphal2mnl_flush_batched_locations() {
    int ret, offset = 0;
    char buff[HAL_FLP_BUFF_SIZE] = {0};
    MTK_FLP_MSG_T flp_header;

    flp_header.type = MTK_FLP_MSG_HAL_FLUSH_LOC_NTF;
    flp_header.length = 0;
    put_binary(buff, &offset, (const char*)&flp_header, sizeof(MTK_FLP_MSG_T));
    ret = flp_send2mnl(buff, offset);
    if(ret < 0) {
        LOGE("MTK_HAL2FLP send error return error");
    }
}

static void mtk_flp_dump_options(FlpBatchOptions* options) {
    LOGD("BatchOptions:flg %d,max_pow: %lf, period %" PRId64 ", source %d\n",
        options->flags, options->max_power_allocation_mW, options->period_ns,
        options->sources_to_use);
}

static void mtk_flp_set_options(int id, FlpBatchOptions* options) {
    if( options == NULL) {
        LOGE("WRONG USAGE of mtk_flp_set_options");
        return;
    }
    //LOGD("mtk_flp_set_options id = %d",id);
    memcpy( &gFlpOptions[id], options, sizeof(FlpBatchOptions));
    mtk_flp_dump_options((FlpBatchOptions*)&gFlpOptions[id]);
}

static void mtk_flp_update_options() {
    int i;

    memset(&gFusedOptions, 0, sizeof(FlpBatchOptions));
    gFusedOptions.period_ns = 1000000000000;

    // update gFusedOptions
    for(i = 0; i < FLP_OPTION_MAX; i++) {
        if(gFlpOptions[i].sources_to_use == 0) {
            continue;
        }
        gFusedOptions.sources_to_use |= gFlpOptions[i].sources_to_use;
        if(gFusedOptions.period_ns > gFlpOptions[i].period_ns) {
            gFusedOptions.period_ns = gFlpOptions[i].period_ns;
        }
        if(gFusedOptions.flags < gFlpOptions[i].flags) {
            gFusedOptions.flags = gFlpOptions[i].flags;
        }
        gFusedOptions.max_power_allocation_mW = gFlpOptions[i].max_power_allocation_mW;
    }
    if(gFusedOptions.period_ns == 1000000000000) {
        gFusedOptions.period_ns = 0;
    }
    //LOGD("mtk_flp_update_options");
    mtk_flp_dump_options(&gFusedOptions);
}



