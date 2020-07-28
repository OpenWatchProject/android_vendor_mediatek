#include <cutils/sockets.h>
#include <sys/time.h>
#include <time.h>
#include <log/log.h>     /*logging in logcat*/
#include <hardware/fused_location.h>
#include "flphal_interface.h"

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

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif


/**********************************************************
 *  Define                                                *
 **********************************************************/
#define MTK_FLP_BATCH_SIZE  30
#define UNICODE_BUF_LEN 2048
#define FLP_BATCHED_LOCATION_FLUSH_HANDLER_TIMEOUT       (200)


/**********************************************************
 *  Global vars                                           *
 **********************************************************/
static FlpCallbacks *mtkFlpCallbacks;
static FlpDiagnosticCallbacks *mtkFlpDiagCallbacks;
static FlpBatchOptions  gFusedOptions;
static int FlpCapability = 0;
static int FlpLastStatus = 2;
static FlpLocation  FlpLocBuffer[MTK_FLP_BATCH_SIZE];
static int          FlpLocNum=0;
static int          FlpLocBegin=0;
static int          FlpLocEnd=0;
static char         fgtimerStarted = 0;
static char         fgtimerInit = 0;
static timer_t batch_loc_timer;


extern struct hw_module_t HAL_MODULE_INFO_SYM;
static void mtk_flp_flush_loc();
static void flp_timeout_handler();

/*********************************************************/
/* Timer Functions                                       */
/*********************************************************/
// -1 means failure
timer_t init_timer_id(timer_callback cb, int id) {
    struct sigevent sevp;
    timer_t timerid;

    memset(&sevp, 0, sizeof(sevp));
    sevp.sigev_value.sival_int = id;
    sevp.sigev_notify = SIGEV_THREAD;
    sevp.sigev_notify_function = cb;

    if (timer_create(CLOCK_MONOTONIC, &sevp, &timerid) == -1) {
        LOGE("timer_create  failed reason=[%s]", strerror(errno));
        return (timer_t)-1;
    }
    LOGD("timer create ok\n");
    return timerid;
}

// -1 means failure
timer_t init_timer(timer_callback cb) {
    return init_timer_id(cb, 0);
}

// -1 means failure
int start_timer(timer_t timerid, int milliseconds) {
    struct itimerspec expire;
    expire.it_interval.tv_sec = 0;
    expire.it_interval.tv_nsec = 0;
    expire.it_value.tv_sec = milliseconds/1000;
    expire.it_value.tv_nsec = (milliseconds%1000)*1000000;
    return timer_settime(timerid, 0, &expire, NULL);
}

// -1 means failure
int stop_timer(timer_t timerid) {
    return start_timer(timerid, 0);
}

/*********************************************************/
/* FLP Location Diagnostic Interface implementation      */
/*********************************************************/
void mtk_flp_diag_init(FlpDiagnosticCallbacks* callbacks) {
    TRC();
    if(callbacks == NULL) {
        return;
    }
    mtkFlpDiagCallbacks = callbacks;
}

int mtk_flp_diag_inject_data(char* data, int length) {
    TRC();
    return flphal2mnl_diag_inject_data(data,length);
}

/*********************************************************/
/* FLP Device Context Interface implementation           */
/*********************************************************/
int  mtk_flp_dev_inject_device_context(uint32_t enabledMask) {
    UNUSED(enabledMask);
    return MTK_FLP_SUCCESS;
}

/*********************************************************/
/* FLP Location Interface implementation                 */
/*********************************************************/
int mtk_flp_init(FlpCallbacks* callbacks) {
    int ret = MTK_FLP_ERROR;
    TRC();
    if(callbacks == NULL) {
        return ret;
    }
    mtkFlpCallbacks = callbacks;
    if(mtkFlpCallbacks == NULL) {
        LOGE("mtkFlpCallbacks == NULL");
        return ret;
    }

    if(mtkFlpCallbacks->set_thread_event_cb) {
        LOGE("mtkFlpCallbacks.set_thread_event_cb");
        mtkFlpCallbacks->set_thread_event_cb(ASSOCIATE_JVM);
    } else {
        LOGE("set_thread_event_cb not set!!");
    }

    if(mtkFlpCallbacks->flp_capabilities_cb) {
        LOGE("mtkFlpCallbacks.flp_capabilities_cb");
        FlpCapability = (int)(CAPABILITY_GNSS);
        mtkFlpCallbacks->flp_capabilities_cb(FlpCapability);
    } else {
        LOGE("mtkFlpCallbacks.flp_capabilities_cb not set!!");
    }
    if(!fgtimerInit) {
    batch_loc_timer = init_timer(flp_timeout_handler);
        fgtimerInit = 1;
    }

    flphal2mnl_flp_init();
    ret = mnl2flphal_flp_init();
    return ret;
}

int mtk_flp_get_batch_size() {
    TRC();
    return MTK_FLP_BATCH_SIZE;
}

int mtk_flp_start_batching(int id, FlpBatchOptions* options) {
    TRC();
    memcpy(&gFusedOptions,options,sizeof(FlpBatchOptions));
    return flphal2mnl_flp_start(id, options);
}

int mtk_flp_update_batching_options(int id, FlpBatchOptions* options) {
    TRC();
    return flphal2mnl_update_batching_options(id, options);
}

int mtk_flp_stop_batching(int id) {
    TRC();
    return flphal2mnl_stop_batching(id);
}

void mtk_flp_cleanup() {
    TRC();
    mtkFlpDiagCallbacks = NULL;
    mtkFlpCallbacks = NULL;
}

void mtk_flp_get_batched_location(int last_n_locations) {
    TRC();
    flphal2mnl_get_batched_location(last_n_locations);
}

int  mtk_flp_inject_location(FlpLocation* location) {
    TRC();
    return flphal2mnl_inject_location(location);
}

static const FlpDiagnosticInterface mtkFlpDiagnosticInterface = {
    sizeof(FlpDiagnosticInterface),
    mtk_flp_diag_init,
    mtk_flp_diag_inject_data,
};

static const FlpDeviceContextInterface mtkFlpDeviceContextInterface = {
    sizeof(FlpDeviceContextInterface),
    mtk_flp_dev_inject_device_context,
};

const void* mtk_flp_get_extension(const char* name) {
    TRC();
    if(!strcmp(name, FLP_DIAGNOSTIC_INTERFACE)) {
        return &mtkFlpDiagnosticInterface;
    } else if(!strcmp(name, FLP_DEVICE_CONTEXT_INTERFACE)) {
        return &mtkFlpDeviceContextInterface;
    }
    return MTK_FLP_SUCCESS;
}

void mtk_flp_flush_batched_locations() {
    TRC();
    flphal2mnl_flush_batched_locations();
}

static void flp_timeout_handler() {
    //flush location
    mtk_flp_flush_loc();
    stop_timer(batch_loc_timer);
    fgtimerStarted  =0;
    LOGD("ofl_batch_timeout\n");
}

/*********************************************************/
/* FLP Location check and debug print                    */
/*********************************************************/

static char mtk_flp_sys_dbg_check_loc(FlpLocation *loc) {
    if(loc->size < sizeof(FlpLocation)) {
        LOGE("Wrong location size:%zu, %zu", loc->size, sizeof(FlpLocation));
        return 0;
    }
    if(loc->longitude>180 || loc->longitude < -180 || loc->latitude >90 || loc->latitude<-90) {
        LOGE("Wrong location value, longitude:%f, latitude:%f", loc->longitude, loc->latitude);
        return 0;
    }
    return 1;
}

static void mtk_flp_sys_dbg_dump_loc(FlpLocation *loc) {
    #if 0
    LOGD("Location(%x):LNG:%f LAT:%f ALT:%f ACC:%f SPD:%f BEARING:%f, FLAGS:%04X SOURCES:%08X Timestamp:%lld",
    loc, loc->longitude, loc->latitude, loc->altitude, loc->accuracy,
    loc->speed, loc->bearing, loc->flags, loc->sources_used, loc->timestamp);
    #endif

    if(mtk_flp_sys_dbg_check_loc(loc) != 1) {
        LOGE("ERROR dumping location!!!!");
        return;
    }
}

static void mtk_flp_dump_locations(int n, FlpLocation** locs) {
    int i;
    TRC();
    for(i = 0; i < n; i++){
        mtk_flp_sys_dbg_dump_loc(locs[i]);
    }
}

/*********************************************************/
/* FLP Location ring buffer preparation                  */
/*********************************************************/
static int FlpLocRingIsFull(void) {
    return (FlpLocNum==MTK_FLP_BATCH_SIZE);
}

static int FlpLocRingIsEmpty(void) {
    return (FlpLocNum==0);
}

//Add one location to the Ring buffer.  Remove oldest when full.
static void FlpLocRingAdd(FlpLocation *p) {
    if(p == NULL) {
        LOGE("Adding NULL");
        return;
    }
    mtk_flp_sys_dbg_dump_loc(p);
    memcpy(&FlpLocBuffer[FlpLocBegin], p, sizeof(FlpLocation));

    //LOGD("LocRing Add to %d(%x),%d", FlpLocBegin, &FlpLocBuffer[FlpLocBegin],FlpLocNum);

    FlpLocBegin++;
    if(FlpLocBegin == MTK_FLP_BATCH_SIZE) {
        FlpLocBegin = 0;
    }

    if( FlpLocNum < MTK_FLP_BATCH_SIZE) {       //if already not full yet.
        FlpLocNum++;
    } else {                                       //if full. drop oldest
        FlpLocEnd++;
        if(FlpLocEnd == MTK_FLP_BATCH_SIZE) {
            FlpLocEnd = 0;
        }
    }
}

//Remove a location from tail.
static int FlpLocRingRemove(FlpLocation **p) {
    if(FlpLocRingIsEmpty()) {
        LOGE("Ring empty!!!");
        return MTK_FLP_ERROR;
    }
    *p = &FlpLocBuffer[FlpLocEnd];

    FlpLocNum--;
    FlpLocEnd++;
    if(FlpLocEnd == MTK_FLP_BATCH_SIZE) {
        FlpLocEnd = 0;
    }
    return MTK_FLP_SUCCESS;
}

//return the number of locations.
static int FlpLocRingRemoveN(FlpLocation **p, int N) {
    int idx=0;
    if(N<=0) {
        LOGE("Wrong N");
        return 0;
    }

    while(!FlpLocRingIsEmpty()) {
        FlpLocRingRemove(&p[idx]);
        idx++;
        if(idx == N) {
            break;
        }
    }
    return idx;
}


//get the value of last N location without removing it.
static int FlpLocRingPeekLastN(FlpLocation **p, int N) {
    int b,e,num,cur_num=0;

    b = FlpLocBegin;
    e = FlpLocEnd;
    num = FlpLocNum;

    while(cur_num < N && num > 0) {
        b--;
        if(b < 0) {
            b = MTK_FLP_BATCH_SIZE-1;
        }
        mtk_flp_sys_dbg_dump_loc(&FlpLocBuffer[b]);
        p[cur_num] = &FlpLocBuffer[b];  //copy pointer only. not content.
        //LOGD("LocRing Peek to %d(%x)", b, &FlpLocBuffer[b]);
        num--;
        cur_num++;
    }
    return cur_num;
}

/*********************************************************/
/* FLP Location, status & NTF Callback                   */
/*********************************************************/
static void mtk_flp_report_loc(MTK_FLP_MSG_T *prmsg) {
    int loc_num = 0;
    int i;
    FlpLocation* ptr = NULL;
    FlpLocation *locs[MTK_FLP_BATCH_SIZE];

    //TRC();
    //add locations to ring buffer every time location is reported
    loc_num = prmsg->length/sizeof(FlpLocation);
    if(prmsg->type == MTK_FLP_MSG_OFL_REPORT_LOC_NTF) {
        for(i = 0; i < loc_num; i++) {
            ptr = (FlpLocation* )((unsigned char*)prmsg + sizeof(MTK_FLP_MSG_T) + i*sizeof(FlpLocation));
            LOGD("report loc %d/%d", i,loc_num);
            FlpLocRingAdd(ptr);
        }
        if(!fgtimerStarted) {
            start_timer(batch_loc_timer,FLP_BATCHED_LOCATION_FLUSH_HANDLER_TIMEOUT);
            fgtimerStarted = 1;
            LOGD("start timer\n");
        }
    } else if (prmsg->type == MTK_FLP_MSG_HSB_REPORT_LOC_NTF) {
        for(i = 0; i < loc_num; i++) {
            ptr = (FlpLocation* )((unsigned char*)prmsg + sizeof(MTK_FLP_MSG_T) + i*sizeof(FlpLocation));
            LOGD("report loc %d/%d", i,loc_num);
            FlpLocRingAdd(ptr);
        }

        //LOGD("report loc, flag:%x", gFusedOptions.flags);
        if(!FlpLocRingIsFull()) {
            //LOGD("Location Ring not FULL");
            return;
        }
        if(gFusedOptions.flags == MTK_FLP_BATCH_WAKE_ON_FIFO_FULL) { //flush data when fifo full
            loc_num = FlpLocRingPeekLastN(locs, MTK_FLP_BATCH_SIZE);
            LOGD("wakeOnFifoFull_TRUE: %d", loc_num);
            if(mtkFlpCallbacks!=NULL) {
                mtkFlpCallbacks->location_cb(loc_num, locs);
            }
            loc_num = FlpLocRingRemoveN( locs, MTK_FLP_BATCH_SIZE);
            FlpLocBegin = 0;
            FlpLocEnd = 0;
            FlpLocNum = 0;
        }
    }
}

static void mtk_flp_request_loc(int req_num) {
    int loc_num = 0, i = 0;
    FlpLocation *locs[MTK_FLP_BATCH_SIZE];

    TRC();
    if( req_num <0 ) {
        LOGE("request_loc size error, %d",req_num);
        return;
    } else if(req_num>MTK_FLP_BATCH_SIZE) {
        req_num = MTK_FLP_BATCH_SIZE;
    }
    loc_num = FlpLocRingPeekLastN(locs, req_num);
    mtk_flp_dump_locations(loc_num, locs);
    for (i = 0; i < loc_num; i++) {
        if(mtkFlpCallbacks!=NULL) {
            mtkFlpCallbacks->location_cb(1, &locs[i]);
        }
    }
}

static void mtk_flp_flush_loc() {
    int loc_num = 0;
    FlpLocation *locs[MTK_FLP_BATCH_SIZE];

    loc_num = FlpLocRingPeekLastN(locs, MTK_FLP_BATCH_SIZE);
    if(loc_num < 1) {
        LOGD("no loc out\n");
        return;
    } else {
        LOGD("loc out %d\n",loc_num);
    }

    if(mtkFlpCallbacks!=NULL) {
        mtkFlpCallbacks->location_cb(loc_num, locs);
    }
    loc_num = FlpLocRingRemoveN(locs, MTK_FLP_BATCH_SIZE);
    FlpLocBegin = 0;
    FlpLocEnd = 0;
    FlpLocNum = 0;
}

static void mtk_flp_report_status(int status) {
    //LOGD("report status: %d", status);
    if(FlpLastStatus != status) {
        LOGD("status change from: %d to %d", FlpLastStatus, status);
        if(mtkFlpCallbacks!=NULL) {
            mtkFlpCallbacks->flp_status_cb(status);
        }
        FlpLastStatus = status;
    }
}


/*********************************************************/
/* FLP to HAL msg handler                                */
/*********************************************************/
void mtk_mnl2flp_hal_response (MTK_FLP_MSG_T *prmsg) {
    int *param=NULL;

    if(prmsg == NULL) {
        LOGE("mtk_mnl2flp_hal_response, recv prmsg is null pointer\r\n");
        return;
    }
    switch (prmsg->type) {
        case MTK_FLP_MSG_SYS_FLPD_RESET_NTF:
            flphal2mnl_flp_reboot_done_ntf();
            // AP/ofl switch, drop batched data
            FlpLocBegin = 0;
            FlpLocEnd = 0;
            FlpLocNum = 0;
            break;
        case MTK_FLP_MSG_HAL_REQUEST_LOC_NTF:
            if(prmsg->length == 0 ) {
                LOGE("mtk_mnl2flp_hal_response msg error\r\n");
                return;
            }
            param = (int*)((char*)prmsg+sizeof(MTK_FLP_MSG_T));
            mtk_flp_request_loc( *param);
            break;
        case MTK_FLP_MSG_HAL_FLUSH_LOC_NTF:
            mtk_flp_flush_loc();
            break;
        case MTK_FLP_MSG_HAL_REPORT_STATUS_NTF:
            param = (int*)((char*)prmsg+sizeof(MTK_FLP_MSG_T));
            mtk_flp_report_status(*param);
            break;
        case MTK_FLP_MSG_HSB_REPORT_LOC_NTF:
        case MTK_FLP_MSG_OFL_REPORT_LOC_NTF:
            mtk_flp_report_loc(prmsg);
            break;
        default:
            break;
    }
}


const FlpLocationInterface  mtkFlpInterface = {
    sizeof(FlpLocationInterface),
    mtk_flp_init,
    mtk_flp_get_batch_size,
    mtk_flp_start_batching,
    mtk_flp_update_batching_options,
    mtk_flp_stop_batching,
    mtk_flp_cleanup,
    mtk_flp_get_batched_location,
    mtk_flp_inject_location,
    mtk_flp_get_extension,
    mtk_flp_flush_batched_locations,
};


//=========================================================
// Between
//     FLP Interface
//     Hardware Module Interface
#if 0
static const FlpLocationInterface* mtk_flp_get_flp_interface (
    __unused struct flp_device_t* device) {
    TRC();
    UNUSED(device);
    return &mtkFlpInterface;
}

static const struct flp_device_t flp_device = {
    .common = {                           // hw_device_t
        .tag     = HARDWARE_DEVICE_TAG,
        .version = 0,
        .module  = &HAL_MODULE_INFO_SYM,
        .reserved = {0},
        .close   = NULL
    },
    .get_flp_interface = mtk_flp_get_flp_interface
};

//=========================================================
// Implementation of
//     Hardware Module Interface

static int open_flp (
    __unused const struct hw_module_t* module,
    __unused char const* id,
    struct hw_device_t** device) {
    *device = (struct hw_device_t*)&flp_device;
    return 0;
}

static struct hw_module_methods_t flp_module_methods = {
    .open = open_flp
};
#endif
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = FUSED_LOCATION_HARDWARE_MODULE_ID,
    .name = "Hardware FLP Module",
    .author = "The MTK FLP Source Project",
    .methods = NULL,
    .dso     = NULL,
    .reserved = {0}
};



