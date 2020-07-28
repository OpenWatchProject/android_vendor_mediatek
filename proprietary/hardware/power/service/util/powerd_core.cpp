//#ifdef __cplusplus
//extern "C" {
//#endif

/*** STANDARD INCLUDES *******************************************************/
#define LOG_NDEBUG 0 // support ALOGV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>

/*** PROJECT INCLUDES ********************************************************/
#define PERFD

#include "ports.h"
#include "mi_types.h"
#include "mi_util.h"
#include "ptimer.h"

#include "power_util.h"
#include "powerd_int.h"
#include "powerd_core.h"
#include "mtkpower_hint.h"
#include "mtkpower_types.h"
#include "perfservice_types.h"

#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPower.h>

/* customization config */
#include "power_cus.h"

//#define PERFD_DEBUG_LOG

//using namespace vendor::mediatek::hardware::power::V2_0;
using namespace vendor::mediatek::hardware::mtkpower::V1_0;

#define LIB_FULL_NAME "libpowerhal.so"

typedef int (*perf_boost_enable)(int);
typedef int (*perf_boost_disable)(int);
typedef int (*perf_notify_state)(const char*, const char*, int, int, int);
typedef int (*perf_user_scn_disable_all)(void);
typedef int (*perf_user_scn_restore_all)(void);
typedef int (*perf_user_scn_check_all)(void);
typedef int (*perf_set_sysinfo)(int, const char*);
typedef int (*perf_user_get_capability)(int, int);
typedef int (*perf_dump_all)(void);
typedef int (*perf_lock_acq)(int*, int, int, int, int, int);
typedef int (*perf_lock_rel)(int);
typedef int (*perf_libpower_init)(void);
typedef int (*perf_user_scn_reg)(int, int);
typedef int (*perf_user_scn_config)(int, int, int, int, int, int);
typedef int (*perf_user_scn_unreg)(int);
typedef int (*perf_user_enable)(int);
typedef int (*perf_user_disable)(int);

/* function pointer to perfserv client */
static int (*perfBoostEnable)(int) = NULL;
static int (*perfBoostDisable)(int) = NULL;
static int (*perfNotifyAppState)(const char*, const char*, int, int, int) = NULL;
static int (*perfUserScnDisableAll)(void) = NULL;
static int (*perfUserScnCheckAll)(void) = NULL;
static int (*perfUserScnRestoreAll)(void) = NULL;
static int (*perfSetSysInfo)(int, const char*) = NULL;
static int (*perfUserGetCapability)(int, int) = NULL;
static int (*perfDumpAll)(void) = NULL;
static int (*perfLockAcq)(int*, int, int, int, int, int) = NULL;
static int (*perfLockRel)(int) = NULL;
static int (*perfLibpowerInit)(void) = NULL;
static int (*perfUserRegScn)(int, int) = NULL;
static int (*perfUserRegScnConfig)(int, int, int, int, int, int) = NULL;
static int (*perfUserUnregScn)(int) = NULL;
static int (*perfUserScnEnable)(int) = NULL;
static int (*perfUserScnDisable)(int) = NULL;

/* Global variable */
static void * _gpTimerMng;
static int pboost_timeout = 0;

static int gMyPid = 0;
static int gMyTid = 0;

static int powerdOnOff = 1;
static int bDuringProcessCreate = 0;
static char currPackname[512];

#define MAX_EXT_LAUNCH_COUNT          3
#define MAX_TIMER_COUNT             256
#define MAX_CUS_HINT_COUNT          128
#define GAME_LAUNCH_DURATION      10000
#define CHECK_USER_SCN_DURATION  300000

enum {
    TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT = 0,
    TIMER_MSG_PERF_LOCK_TIMEOUT         = 1,
    TIMER_MSG_USER_SCN_ENABLE_TIMEOUT   = 2,
    TIMER_MSG_CHECK_USER_SCN_TIMEOUT    = 3,
};

struct tTimer {
    int used;
    int idx;
    int msg;
    int handle;
    void *p_pTimer;
};

struct extLaunchScn {
    int scn;
    int dur;
    int ud;
};

static int nPerfSupport = 0;
struct tTimer powerdTimer[MAX_TIMER_COUNT]; // temp
//struct extLaunchScn tExtScn[MAX_EXT_LAUNCH_COUNT]; // temp

static int gtCusHintTbl[MAX_CUS_HINT_COUNT];

//static int cusHintTblSize = 0;

/*****************
   Function
 *****************/
static int load_api(void)
{
    void *handle, *func;

    handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
    if (handle == NULL) {
        ALOGE("dlopen error: %s", dlerror());
        return -1;
    }

    func = dlsym(handle, "perfLibpowerInit");
    perfLibpowerInit = (perf_libpower_init)(func);

    if (perfLibpowerInit == NULL) {
        ALOGE("perfLibpowerInit error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfLockAcq");
    perfLockAcq = (perf_lock_acq)(func);

    if (perfLockAcq == NULL) {
        ALOGE("perfLockAcq error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfLockRel");
    perfLockRel = (perf_lock_rel)(func);

    if (perfLockRel == NULL) {
        ALOGE("perfLockRel error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfBoostEnable");
    perfBoostEnable = (perf_boost_enable)(func);

    if (perfBoostEnable == NULL) {
        printf("perfBoostEnable error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfBoostDisable");
    perfBoostDisable = (perf_boost_disable)(func);

    if (perfBoostDisable == NULL) {
        printf("perfBoostDisable error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfNotifyAppState");
    perfNotifyAppState = (perf_notify_state)(func);

    if (perfNotifyAppState == NULL) {
        printf("perfNotifyAppState error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserGetCapability");
    perfUserGetCapability = (perf_user_get_capability)(func);

    if (perfUserGetCapability == NULL) {
        printf("perfUserGetCapability error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserScnDisableAll");
    perfUserScnDisableAll = (perf_user_scn_disable_all)(func);

    if (perfUserScnDisableAll == NULL) {
        printf("perfUserScnDisableAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserScnRestoreAll");
    perfUserScnRestoreAll = (perf_user_scn_restore_all)(func);

    if (perfUserScnRestoreAll == NULL) {
        printf("perfUserScnRestoreAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserScnCheckAll");
    perfUserScnCheckAll = (perf_user_scn_check_all)(func);

    if (perfUserScnCheckAll == NULL) {
        printf("perfUserScnCheckAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfSetSysInfo");
    perfSetSysInfo = (perf_set_sysinfo)(func);

    if (perfSetSysInfo == NULL) {
        ALOGE("perfSetSysInfo error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfDumpAll");
    perfDumpAll = (perf_dump_all)(func);

    if (perfDumpAll == NULL) {
        printf("perfDumpAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserRegScn");
    perfUserRegScn = (perf_user_scn_reg)(func);

    if (perfUserRegScn == NULL) {
        printf("perfUserRegScn error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserRegScnConfig");
    perfUserRegScnConfig = (perf_user_scn_config)(func);

    if (perfUserRegScnConfig == NULL) {
        printf("perfUserRegScnConfig error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserUnregScn");
    perfUserUnregScn = (perf_user_scn_unreg)(func);

    if (perfUserUnregScn == NULL) {
        printf("perfUserUnregScn error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserScnEnable");
    perfUserScnEnable = (perf_user_enable)(func);

    if (perfUserScnEnable == NULL) {
        printf("perfUserScnEnable error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "perfUserScnDisable");
    perfUserScnDisable = (perf_user_disable)(func);

    if (perfUserScnDisable == NULL) {
        printf("perfUserScnDisable error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    return 0;
}

int reset_timer(int i)
{
    powerdTimer[i].used = 0;
    powerdTimer[i].msg = -1;
    powerdTimer[i].handle = -1;
    powerdTimer[i].p_pTimer = NULL;
    return 0;
}

int allocate_timer(void)
{
    int i;
    for(i=0; i<MAX_TIMER_COUNT; i++) {
        if(powerdTimer[i].used == 0) {
            powerdTimer[i].used = 1;
            return i;
        }
    }
    return -1;
}

int find_timer(int msg, int handle)
{
    int i;
    for(i=0; i<MAX_TIMER_COUNT; i++) {
        if(powerdTimer[i].used && powerdTimer[i].handle == handle && powerdTimer[i].msg == msg)
            return i;
    }
    return -1;
}

int remove_scn_timer(int msg, int handle)
{
    int idx;

    if((idx = find_timer(msg, handle)) >= 0) {
        ptimer_stop(powerdTimer[idx].p_pTimer);
        ptimer_delete(powerdTimer[idx].p_pTimer);
        reset_timer(idx);
    }
    return 0;
}

int start_scn_timer(int msg, int handle, int timeout)
{
    int idx;
    if((idx = allocate_timer()) >= 0) {
        //ALOGI("[start_scn_timer] idx:%d, handle:%d, timeout:%d", idx, handle, timeout);
        powerdTimer[idx].msg = msg;
        powerdTimer[idx].handle = handle;
        ptimer_create(&(powerdTimer[idx].p_pTimer));
        ptimer_start(_gpTimerMng, powerdTimer[idx].p_pTimer, timeout, &(powerdTimer[idx]));
    }
    return 0;
}

int start_periodic_check_timer(int firstLaunch)
{
    static int idx = 0;

    if(firstLaunch) {
        if((idx = allocate_timer()) >= 0) {
            //ALOGI("[start_scn_timer] idx:%d, handle:%d, timeout:%d", idx, handle, timeout);
            powerdTimer[idx].msg = TIMER_MSG_CHECK_USER_SCN_TIMEOUT;
            powerdTimer[idx].handle = 0;
            ptimer_create(&(powerdTimer[idx].p_pTimer));
        }
        else
            return 0;
    }

    ptimer_start(_gpTimerMng, powerdTimer[idx].p_pTimer, CHECK_USER_SCN_DURATION, &(powerdTimer[idx]));
    return 0;
}

int powerd_core_pre_init(void)
{
    if(load_api() == 0)
        nPerfSupport = 1;
    else
        return 0; // libperfservice is not supported

    gMyPid = (int)getpid();
    gMyTid = (int)gettid();

    if (perfLibpowerInit() < 0) {
        ALOGI("[powerd_core_pre_init] perfLibpowerinit fail");
    }

    ALOGI("[powerd_core_pre_init] pboost_timeout: %d", pboost_timeout);

    return 0;
}

int powerd_core_init(void * pTimerMng)
{
    int i = 0;

    _gpTimerMng = pTimerMng;

    for(i=0; i<MAX_TIMER_COUNT; i++) {
        powerdTimer[i].used = 0;
        powerdTimer[i].idx = i;
        powerdTimer[i].msg = -1;
        powerdTimer[i].handle = -1;
        powerdTimer[i].p_pTimer = NULL;
    }

    /* start periodic timer to check invalid user scenario */
    if (nPerfSupport)
        start_periodic_check_timer(1);

    /* cus hint */
    for(i=0; i<MAX_CUS_HINT_COUNT; i++)
        gtCusHintTbl[i] = 0;

    return 0;
}

int powerd_core_timer_handle(void * pTimer, void * pData)
{
    //int i = 0;
    struct tTimer *ptTimer = (struct tTimer *)pData;

    if(ptTimer->p_pTimer != pTimer)
        return -1;

    switch(ptTimer->msg) {
    case TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT:
        if (nPerfSupport)
            perfBoostDisable(ptTimer->handle);
        ptimer_delete(ptTimer->p_pTimer);
        reset_timer(ptTimer->idx);
        break;

    case TIMER_MSG_PERF_LOCK_TIMEOUT:
#ifdef PERFD_DEBUG_LOG
        ALOGI("[powerd_req] TIMER_MSG_PERF_LOCK_TIMEOUT hdl:%d", ptTimer->handle);
#endif
        if (nPerfSupport)
            perfLockRel(ptTimer->handle);
        ptimer_delete(ptTimer->p_pTimer);
        reset_timer(ptTimer->idx);
        break;

    case TIMER_MSG_USER_SCN_ENABLE_TIMEOUT:
        if (nPerfSupport)
            perfUserScnDisable(ptTimer->handle);
        ptimer_delete(ptTimer->p_pTimer);
        reset_timer(ptTimer->idx);
        break;

    case TIMER_MSG_CHECK_USER_SCN_TIMEOUT:
        if (nPerfSupport) {
            perfUserScnCheckAll();
            perfDumpAll();
        }
        start_periodic_check_timer(0);
        break;
    }

    return 0;
}

//extern "C"
int powerd_req(void * pMsg, void ** pRspMsg)
{
    struct tScnData    *vpScnData = NULL, *vpRspScn = NULL;
    struct tHintData   *vpHintData = NULL;
    struct tPerfLockData *vpPerfLockData = NULL, *vpRspPerfLock = NULL;
    struct tAppStateData *vpAppState = NULL;
    struct tQueryInfoData  *vpQueryData = NULL, *vpRspQuery = NULL;
    struct tPowerData * vpData = (struct tPowerData *) pMsg;
    struct tPowerData * vpRsp = NULL;
    struct tSysInfoData *vSysInfoData = NULL, *vpRspSysInfo = NULL;
    int hdl = 0, new_hdl = 0, i, cus_index;
    int hint_hold = 0, ext_hint = 0, ext_hint_hold = 0, launch_boost_time = 0;
    //struct tHintData        *vpRspHint;
    //struct tAppStateDate    *vpRspAppState;

    if(!nPerfSupport) {
        if(vpData->msg != POWER_MSG_MTK_HINT) // log reduction
            ALOGI("libpowerhal not supported\n");
        return -1;
    }

    if((vpRsp = (struct tPowerData *) malloc(sizeof(struct tPowerData))) == NULL) {
        ALOGI("%s malloc failed\n", __func__);
        return -1;
    }

    if(vpRsp) {
        vpRsp->msg = vpData->msg;
        vpRsp->pBuf = NULL;
    }

    if(vpData) {
        switch(vpData->msg) {
        case POWER_MSG_AOSP_HINT:
            if(vpData->pBuf && powerdOnOff) {
                vpHintData = (struct tHintData*)(vpData->pBuf);

                ALOGD("[powerd_req] POWER_MSG_AOSP_HINT: hint:%d, data:%d", vpHintData->hint, vpHintData->data);
                if(vpHintData->data) {
                    perfBoostEnable(vpHintData->hint);

                    /* All power hint support timeout */
                    remove_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, vpHintData->hint);
                    if(vpHintData->data != MTKPOWER_HINT_ALWAYS_ENABLE) { // not MTK_HINT_ALWAYS_ENABLE
                        start_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, vpHintData->hint, vpHintData->data);
                    }
                } else {
                    perfBoostDisable(vpHintData->hint);
                }
            }
            break;

        case POWER_MSG_SCN_DISABLE_ALL:
            if(vpData->pBuf && powerdOnOff) {
                ALOGI("[powerd_req] POWER_MSG_SCN_DISABLE_ALL");
                vpScnData = (struct tScnData*)(vpData->pBuf);
                perfUserScnDisableAll();
            }
            break;

        case POWER_MSG_SCN_RESTORE_ALL:
            if(vpData->pBuf && powerdOnOff) {
                ALOGI("[powerd_req] POWER_MSG_SCN_RESTORE_ALL");
                vpScnData = (struct tScnData*)(vpData->pBuf);
                perfUserScnRestoreAll();
            }
            break;

        case POWER_MSG_MTK_HINT:
            if(vpData->pBuf && powerdOnOff) {
                vpHintData = (struct tHintData*)(vpData->pBuf);

                ALOGD("[powerd_req] POWER_MSG_MTK_HINT: hint:%d, data:%d", vpHintData->hint, vpHintData->data);
                if(vpHintData->data) {
                    remove_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, vpHintData->hint);
                    if(vpHintData->data != MTKPOWER_HINT_ALWAYS_ENABLE) { // not MTKPOWER_HINT_ALWAYS_ENABLE
                        start_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, vpHintData->hint, vpHintData->data);
                    }

                    perfBoostEnable(vpHintData->hint);

                    if (vpHintData->hint == MTKPOWER_HINT_PROCESS_CREATE && vpHintData->data > 1)
                        bDuringProcessCreate = 1; // for white list boost

                } else {
                    remove_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, vpHintData->hint);

                    /* hint hold time */
                    hint_hold = perfUserGetCapability(MTKPOWER_CMD_GET_POWER_HINT_HOLD_TIME, vpHintData->hint);

                    if(hint_hold > 0)
                        start_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, vpHintData->hint, hint_hold);
                    else
                        perfBoostDisable(vpHintData->hint);

                    /* ext hint */
                    ext_hint = perfUserGetCapability(MTKPOWER_CMD_GET_POWER_HINT_EXT_HINT, vpHintData->hint);
                    if(ext_hint > 0) {
                        ext_hint_hold = perfUserGetCapability(MTKPOWER_CMD_GET_POWER_HINT_EXT_HINT_HOLD_TIME, vpHintData->hint);
                        if(ext_hint_hold > 0) {
                            perfBoostEnable(ext_hint);
                            remove_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, ext_hint);
                            start_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, ext_hint, ext_hint_hold);
                        }
                    }

                    ALOGV("[powerd_req] POWER_MSG_MTK_HINT: hint:%d, hold:%d, ext:%d, ext_hold:%d", vpHintData->hint, hint_hold, ext_hint, ext_hint_hold);

                    if (vpHintData->hint == MTKPOWER_HINT_PROCESS_CREATE)
                        bDuringProcessCreate = 0; // for white list boost
                }
            }
            break;

        case POWER_MSG_MTK_CUS_HINT:
            if(vpData->pBuf && powerdOnOff) {
                vpHintData = (struct tHintData*)(vpData->pBuf);

                if(vpHintData->hint >= (int)MtkCusPowerHintInternal::MTK_CUS_HINT_NUM) {
                    ALOGI("[powerd_req] unsupport cus hint:%d", vpHintData->hint);
                    break;
                }

                hdl = gtCusHintTbl[vpHintData->hint];
                ALOGI("[powerd_req] POWER_MSG_MTK_CUS_HINT: hint:%d, hdl:%d, data:%d", vpHintData->hint, hdl, vpHintData->data);
                if(vpHintData->data) {
                    remove_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, hdl);
                    //gtCusHintTbl[vpHintData->hint] = perfLockAcq()
                    cus_index = -1;
                    for (i=0; cusHintConfigImpl[i].hint!=-1; i++) {
                        if(cusHintConfigImpl[i].hint == vpHintData->hint) {
                            cus_index = i;
                            break;
                        }
                    }

                    if(cus_index == -1) {
                        ALOGI("[powerd_req] unsupport cus hint:%d", vpHintData->hint);
                        break;
                    }

                #ifdef PERFD_DEBUG_LOG
                    /* data log */
                    for(i=0; i<cusHintConfigImpl[cus_index].size; i+=2) {
                        ALOGI("[powerd_req] POWER_MSG_MTK_CUS_HINT: data:%x,%d", cusHintConfigImpl[cus_index].rscList[i], cusHintConfigImpl[cus_index].rscList[i+1]);
                    }
                #endif

                    new_hdl = perfLockAcq(cusHintConfigImpl[cus_index].rscList, hdl,
                        cusHintConfigImpl[cus_index].size, gMyPid, gMyTid, vpHintData->data);

                    if(vpHintData->data != MTKPOWER_HINT_ALWAYS_ENABLE) { // not MTK_HINT_ALWAYS_ENABLE
                        if(hdl == new_hdl)
                            remove_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, new_hdl);
                        start_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, new_hdl, vpHintData->data);
                    }
                    gtCusHintTbl[vpHintData->hint] = new_hdl;
                }
                else {
                    hdl = gtCusHintTbl[vpHintData->hint];
                    remove_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, hdl);
                    perfLockRel(hdl);
                    gtCusHintTbl[vpHintData->hint] = 0;
                }
            }
            break;

        case POWER_MSG_NOTIFY_STATE:
            ALOGV("[powerd_req] POWER_MSG_NOTIFY_STATE");
            if(vpData->pBuf && powerdOnOff) {
                vpAppState = (struct tAppStateData*)(vpData->pBuf);
                ALOGV("POWER_MSG_NOTIFY_STATE: %s, %s, %d, %d, %d", vpAppState->pack, vpAppState->activity, vpAppState->state, vpAppState->pid, vpAppState->uid);
                perfNotifyAppState(vpAppState->pack, vpAppState->activity, vpAppState->state, vpAppState->pid, vpAppState->uid);

                /* package switch => check white list boost */
                if (vpAppState->state == STATE_RESUMED && strcmp(vpAppState->pack, currPackname) != 0) {
                    /* disable white list boost first */
                    ALOGI("[powerd_req] POWER_MSG_NOTIFY_STATE, pc:%d, %s => %s", bDuringProcessCreate, currPackname, vpAppState->pack);
                    strncpy(currPackname, vpAppState->pack, sizeof(currPackname)-1);

                    remove_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, MTKPOWER_HINT_WHITELIST_LAUNCH);
                    perfBoostDisable(MTKPOWER_HINT_WHITELIST_LAUNCH);

                    if (bDuringProcessCreate)
                        launch_boost_time = perfUserGetCapability(MTKPOWER_CMD_GET_LAUNCH_TIME_COLD, 0);
                    else
                        launch_boost_time = perfUserGetCapability(MTKPOWER_CMD_GET_LAUNCH_TIME_WARM, 0);

                    if( launch_boost_time > 0) {
                        start_scn_timer(TIMER_MSG_POWER_HINT_ENABLE_TIMEOUT, MTKPOWER_HINT_WHITELIST_LAUNCH, launch_boost_time);
                        perfBoostEnable(MTKPOWER_HINT_WHITELIST_LAUNCH);
                    }
                }
            }
            break;

        case POWER_MSG_QUERY_INFO:
            ALOGV("[powerd_req] POWER_MSG_QUERY_INFO");

            if(vpData->pBuf) {
                if((vpRspQuery = (struct tQueryInfoData*)malloc(sizeof(struct tQueryInfoData))) == NULL)
                    break;

                vpQueryData = (struct tQueryInfoData*)(vpData->pBuf);
                ALOGD("[powerd_req] POWER_MSG_QUERY_INFO: cmd:%d, param:%d", vpQueryData->cmd, vpQueryData->param);

                if(vpQueryData->cmd == MTKPOWER_CMD_GET_DEBUG_DUMP_ALL) {
                    perfDumpAll();
                } else if(vpQueryData->cmd == MTKPOWER_CMD_GET_POWERHAL_ONOFF) {
                    powerdOnOff = vpQueryData->param;
                } else {
                   vpRspQuery->value = perfUserGetCapability(vpQueryData->cmd, vpQueryData->param);
                }
                vpRsp->pBuf = (void*)vpRspQuery;
            }
            break;

        case POWER_MSG_SET_SYSINFO:
            if (vpData->pBuf && powerdOnOff) {
                if((vpRspSysInfo = (struct tSysInfoData*)malloc(sizeof(struct tSysInfoData))) == NULL)
                    break;

                vSysInfoData = (struct tSysInfoData*)(vpData->pBuf);
                ALOGD("[powerd_req] POWER_MSG_SET_SYSINFO, type:%d, str:%s", vSysInfoData->type, vSysInfoData->data);
                vpRspSysInfo->ret = perfSetSysInfo(vSysInfoData->type, vSysInfoData->data);
                vpRsp->pBuf = (void*)vpRspSysInfo;
            }
            break;

        case POWER_MSG_PERF_LOCK_ACQ:
            if(vpData->pBuf && powerdOnOff) {
                if((vpRspPerfLock = (struct tPerfLockData*)malloc(sizeof(struct tPerfLockData))) == NULL)
                    break;

                vpPerfLockData = (struct tPerfLockData*)(vpData->pBuf);

                ALOGI("[powerd_req] POWER_MSG_PERF_LOCK_ACQ: hdl:%d, duration:%d", vpPerfLockData->hdl, vpPerfLockData->duration);

                remove_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, vpPerfLockData->hdl);

            #ifdef PERFD_DEBUG_LOG
                ALOGI("lock data size:%d", vpPerfLockData->size);
                for (i=0; i<vpPerfLockData->size; i++)
                    ALOGI("data:%d", vpPerfLockData->rscList[i]);
            #endif

                hdl = perfLockAcq(vpPerfLockData->rscList, vpPerfLockData->hdl,
                    vpPerfLockData->size, vpPerfLockData->pid, vpPerfLockData->uid, vpPerfLockData->duration);
                ALOGI("[powerd_req] POWER_MSG_PERF_LOCK_ACQ: return hdl:%d", hdl);
                vpRspPerfLock->hdl = hdl;
                vpRsp->pBuf = (void*)vpRspPerfLock;

                if(vpPerfLockData->duration != 0 && hdl > 0) { // not MTK_HINT_ALWAYS_ENABLE
                    start_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, hdl, vpPerfLockData->duration);
                }
            }
            break;

        case POWER_MSG_PERF_LOCK_REL:
            if(vpData->pBuf && powerdOnOff) {
                vpPerfLockData = (struct tPerfLockData*)(vpData->pBuf);

                ALOGI("[powerd_req] POWER_MSG_PERF_LOCK_REL: hdl:%d", vpPerfLockData->hdl);

                remove_scn_timer(TIMER_MSG_PERF_LOCK_TIMEOUT, vpPerfLockData->hdl);

                if(vpPerfLockData->hdl) {
                    perfLockRel(vpPerfLockData->hdl);
                }
            }
            break;

        case POWER_MSG_SCN_REG:
            ALOGV("[powerd_req] POWER_MSG_SCN_REG");

            if(vpData->pBuf) {
                if((vpRspScn = (struct tScnData*)malloc(sizeof(struct tScnData))) == NULL)
                    break;

                vpScnData = (struct tScnData*)(vpData->pBuf);
                ALOGD("[powerd_req] POWER_MSG_SCN_REG: pid:%d, uid:%d", vpScnData->param1, vpScnData->param2);

                vpRspScn->handle = perfUserRegScn(vpScnData->param1,vpScnData->param2); // allocate handle
                vpRsp->pBuf = (void*)vpRspScn;
            }
            break;

        case POWER_MSG_SCN_CONFIG:
            if(vpData->pBuf) {
                vpScnData = (struct tScnData*)(vpData->pBuf);
                ALOGD("[powerd_req] POWER_MSG_SCN_CONFIG: hdl:%d, cmd:%d, param1:%d, param2:%d",
                    vpScnData->handle, vpScnData->command, vpScnData->param1, vpScnData->param2);
                perfUserRegScnConfig(vpScnData->handle, vpScnData->command, vpScnData->param1, vpScnData->param2, vpScnData->param3, vpScnData->param4);
            }
            break;

        case POWER_MSG_SCN_UNREG:
            if(vpData->pBuf) {
                vpScnData = (struct tScnData*)(vpData->pBuf);
                ALOGD("[powerd_req] POWER_MSG_SCN_UNREG: hdl:%d", vpScnData->handle);
                perfUserUnregScn(vpScnData->handle);
            }
            break;

        case POWER_MSG_SCN_ENABLE:
            if(vpData->pBuf) {
                vpScnData = (struct tScnData*)(vpData->pBuf);
                ALOGD("[powerd_req] POWER_MSG_SCN_ENABLE: hdl:%d, timeout:%d", vpScnData->handle, vpScnData->timeout);

                remove_scn_timer(TIMER_MSG_USER_SCN_ENABLE_TIMEOUT, vpScnData->handle);
                if(vpScnData->timeout > 0) {
                    start_scn_timer(TIMER_MSG_USER_SCN_ENABLE_TIMEOUT, vpScnData->handle, vpScnData->timeout);
                }
                perfUserScnEnable(vpScnData->handle);
            }
            break;

        case POWER_MSG_SCN_DISABLE:
            if(vpData->pBuf) {
                vpScnData = (struct tScnData*)(vpData->pBuf);

                remove_scn_timer(TIMER_MSG_USER_SCN_ENABLE_TIMEOUT, vpScnData->handle);
                perfUserScnDisable(vpScnData->handle);
            }
            break;


        default:
            ALOGI("unknown message");
            break;
        }

        *pRspMsg = (void *) vpRsp;
    }

    return 0;
}

//#ifdef __cplusplus
//}
//#endif


