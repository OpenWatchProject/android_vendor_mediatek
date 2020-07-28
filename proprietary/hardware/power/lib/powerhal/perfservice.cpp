/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "libPowerHal"
#define ATRACE_TAG ATRACE_TAG_PERF

#define LOG_NDEBUG 0 // support ALOGV

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include <dlfcn.h>
#include <string.h>
#include <utils/Trace.h>
#include "perfservice.h"
#include "perfservice_xmlparse.h"
#include "common.h"

#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <expat.h>

#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>

using namespace vendor::mediatek::hardware::power::V2_0;

#include <power_cmd_types.h>
#include "mtkpower_hint.h"
#include "mtkperf_resource.h"
#include "mtkpower_types.h"
#include <utils/Timers.h>
#include "perfservice_rsccfgtbl.h"
#include "perfservice_prop.h"
#include "utility_ux.h"
#include "utility_ril.h"
#include "utility_netd.h"

#include "tinyxml2.h"
using namespace tinyxml2;

/* Definition */
#define VERSION "10.0"

#define STATE_ON 1
#define STATE_OFF 0
#define STATE_WAIT_RESTORE 2

#define PACK_NAME_MAX   128
#define CLASS_NAME_MAX  128
#define CUS_SCN_TABLE           "/vendor/etc/powerscntbl.xml"
#define CUS_CONFIG_TABLE        "/vendor/etc/powercontable.xml"
#define CUS_CONFIG_TABLE_T      "/vendor/etc/powercontable_t.xml"
#define COMM_NAME_SIZE  64
#define PPM_MODE_LEN    32

#define REG_SCN_MAX     256   // user scenario max number
#define CLUSTER_MAX     8

#define CPU_CORE_MIN_RESET  (-1)
#define CPU_CORE_MAX_RESET  (0xff)
#define CPU_FREQ_MIN_RESET  (-1)
#define CPU_FREQ_MAX_RESET  (0xffffff)
#define GPU_FREQ_MIN_RESET  (-1)
#define GPU_FREQ_MAX_RESET  (-1)

#define CORE_MAX        0xff
#define FREQ_MAX        0xffffff
#define PPM_IGNORE      (-1)

#define FBC_LIB_FULL_NAME  "libperfctl_vendor.so"

const string LESS("less");

#define HANDLE_RAND_MAX 2147483645

#define USER_DURATION_MAX 30000 /*30s for non permission user*/

#define RSC_TBL_INVALID_VALUE (-123456)

//#define PROC_NAME_BROWS "com.android.browser" //TEST

#ifdef max
#undef max
#endif
#define max(a,b) (((a) > (b)) ? (a) : (b))

#ifdef min
#undef min
#endif
#define min(a,b) (((a) < (b)) ? (a) : (b))

using namespace std;

typedef struct tClusterInfo {
    int  cpuNum;
    int  cpuFirstIndex;
    int  cpuMinNow;
    int  cpuMaxNow;
    int *pFreqTbl;
    int  freqCount;
    int  freqMax;
    int  freqMinNow;
    int  freqMaxNow;
    int  freqHardMinNow;
    int  freqHardMaxNow;
} tClusterInfo;

typedef struct tDrvInfo {
    int cpuNum;
    int perfmgrLegacy; // /proc/perfmgr/legacy/perfserv_freq
    int perfmgrCpu;    // /proc/perfmgr/boost_ctrl/cpu_ctrl/perfserv_freq
    int ppmSupport;
    int ppmAll;        // userlimit_cpu_freq
    int acao;
    int hmp;
    int hps;
    int dvfsHevc;
    int fliper;
    int sysfs;
    int dvfs;
    int turbo;
    int fstb;
    int hard_user_limit; // /proc/ppm/policy/hard_userlimit_cpu_freq
} tDrvInfo;

enum {
    SPORTS_BENCHMARK    = 0,
    SPORTS_USER_NOTIFY  = 1,
};

enum {
    SCN_POWER_HINT      = 0,
    SCN_CUS_POWER_HINT  = 1,
    SCN_USER_HINT       = 2,
    SCN_PERF_LOCK_HINT  = 3,
    SCN_PACK_HINT       = 4,
};

/* Function prototype */
void setClusterCores(int scenario, int clusterNum, int totalCore, int *pCoreTbl, int *pMaxCoreTbl);
void setClusterFreq(int scenario, int clusterNum, int *pFreqTbl, int *pMaxFreqTbl);
void setClusterHardFreq(int scenario, int clusterNum, int *pFreqTbl, int *pMaxFreqTbl);
int perfScnEnable(int scenario);
int perfScnDisable(int scenario);
int perfScnUpdate(int scenario, int force_update);
void setGpuFreq(int scenario, int level);
void setGpuFreqMax(int scenario, int level);
void resetScenario(int handle, int reset_all);
void checkDrvSupport(tDrvInfo *ptDrvInfo);
void getCputopoInfo(int, int *, int *, tClusterInfo **);
int cmdSetting(int, char *, tScnNode *, int);
int LegacyCmdSetting(int, tScnNode *, int, int, int, int);

//int initPowerMode(void);
int switchPowerMode(int mode);
int switchSportsMode(int reason, int bEnable);
static int load_fbc_api(void);

int loadRscTable(int power_on_init);
int whilelist_reload(void);

typedef int (*fbc_touch)(int);

/* function pointer to perfserv client */
static int  (*fbcNotifyTouch)(int) = NULL;

/* Global variable */
static int nIsReady = 0;
/*static char* sProjName = (char*)PROJ_ALL;*/
static Mutex sMutex;
static int scn_cores_now = 0;

static tClusterInfo *ptClusterTbl = NULL;
static int           nClusterNum = 0;

static tScnNode  *ptScnList = NULL;

static tDrvInfo   gtDrvInfo;
static int        nPackNum = 0;
static int        nCpuNum = 0;
static int        nUserScnBase = 0;
static int        SCN_APP_RUN_BASE = MTKPOWER_HINT_NUM + REG_SCN_MAX;
static int user_handle_now = 0;

static int nGpuFreqCount = 0;
static int nGpuHighestFreqLevel = 0;
static int scn_gpu_freq_now = 0;
static int scn_gpu_freq_max_now = 0;

static int foreground_pid = -1;
static int foreground_uid = -1;
static int last_from_uid  = 1;
static int fg_launch_time_cold = 0;
static int fg_launch_time_warm = 0;
static char foreground_pack[PACK_NAME_MAX];
static char foreground_act[PACK_NAME_MAX];

static int nDisplayType = DISPLAY_TYPE_OTHERS;

static int nCurrPowerMode = PERF_MODE_NORMAL;
static int nCurrBenchmarkMode = 0;
static int nUserNotifyBenchmark = 0;

static nsecs_t last_touch_time = 0;

char pPpmDefaultMode[PPM_MODE_LEN] = "";
#if 0
char tPpmMode[PPM_MODE_NUM][PPM_MODE_LEN] =
{
    "Low_Power",
    "Just_Make",
    "Performance",
};
#endif
static int nPpmCurrMode = PPM_IGNORE;

static int  nFbcSupport = 1;

extern tScnConTable tConTable[FIELD_SIZE];
extern tRscConfig RscCfgTbl[];

int    gRscCtlTblLen = 0;
static tRscCtlEntry *gRscCtlTbl = NULL;


int init()
{
    int i;
    char str[PPM_MODE_LEN];
    char prop_content[PROPERTY_VALUE_MAX] = "\0";
    int  prop_value = 0, power_on_init = 0;
    struct stat stat_buf;

    if (!nIsReady) {
        ALOGI("perfservice ver:%s", VERSION);

        /* check HMP support */
        checkDrvSupport(&gtDrvInfo);
        if(gtDrvInfo.sysfs == 0 && gtDrvInfo.dvfs == 0) // /sys/devices/system/cpu/possible is not existed
            return 0;
        getCputopoInfo(gtDrvInfo.hmp, &nCpuNum, &nClusterNum, &ptClusterTbl);

        if(gtDrvInfo.ppmSupport && !gtDrvInfo.acao) {
            get_str_value(PATH_PPM_MODE, str, sizeof(str)-1);
            sscanf(str, "%31s", pPpmDefaultMode);
            ALOGI("pPpmDefaultMode:%s, %d", pPpmDefaultMode, nPpmCurrMode);
        }

        /* temp for D3 */
        if (nClusterNum == 1) gtDrvInfo.hmp = 0;

        /* We won't reset CPU freq because powerhal always set it */

        /* GPU info */
        get_gpu_freq_level_count(&nGpuFreqCount);
        /* Since Gpu Opp table range was from 0 to nGpuFreqCount-1, */
        /* we use nGpuFreqCount to represent free run.              */
        nGpuHighestFreqLevel = scn_gpu_freq_now = nGpuFreqCount - 1; // opp(n-1) is the lowest freq
        scn_gpu_freq_max_now = 0; // opp 0 is the highest freq
        ALOGI("nGpuFreqCount:%d", nGpuFreqCount);
        /* GPU init value */
        //setGpuFreq(0, 0);
        //setGpuFreqMax(0, nGpuHighestFreqLevel);


        nPackNum = perfservice_xmlparse_init();

        if (nPackNum == 0) {
            ALOGE("perfservice_xmlparse_init fail");
            //return 0;
        }

        if (nPackNum >= 0) {
            if((ptScnList = (tScnNode*)malloc(sizeof(tScnNode) * (SCN_APP_RUN_BASE + nPackNum))) == NULL) {
                ALOGE("Can't allocate memory");
                return 0;
            }
        }

        //SCN_APP_RUN_BASE = (int)(MtkPowerHint::MTK_POWER_HINT_NUM) + REG_SCN_MAX;
        SCN_APP_RUN_BASE = MTKPOWER_HINT_NUM + REG_SCN_MAX;
        memset(ptScnList, 0, sizeof(tScnNode)*(SCN_APP_RUN_BASE + nPackNum));
        nIsReady = 1;

        if(gtDrvInfo.turbo && (0 == stat(CUS_CONFIG_TABLE_T, &stat_buf)))
            loadConTable(CUS_CONFIG_TABLE_T);
        else
            loadConTable(CUS_CONFIG_TABLE);

        /* Is it the first init during power on */
        property_get(POWER_PROP_INIT, prop_content, "0"); // init before ?
        prop_value = atoi(prop_content);           // prop_value:1 means powerhal init before
        power_on_init = (prop_value == 1) ? 0 : 1; // power_on_init:1 means init during power on
        if(prop_value == 0) {
            property_set(POWER_PROP_INIT, "1");
        }

        /* resouce config table */
        gRscCtlTblLen = sizeof(RscCfgTbl) / sizeof(*RscCfgTbl);
        ALOGI("[init] loadRscTable:%d", gRscCtlTblLen);
        if ((gRscCtlTbl = (tRscCtlEntry*)malloc(sizeof(tRscCtlEntry) * gRscCtlTblLen)) == NULL) {
            ALOGE("Can't allocate memory");
            return 0;
        }
        ALOGI("[init] loadRscTable:%d memset", gRscCtlTblLen);
        memset(gRscCtlTbl, 0, sizeof(tRscCtlEntry)*gRscCtlTblLen);
        loadRscTable(power_on_init);

        smart_init(); // init before white list

        // configure the sProjName if needed.
        for (i = 0; i < MTKPOWER_HINT_NUM; i++) {
            resetScenario(i, 1); // reset all scenarios
            ptScnList[i].scn_type = SCN_POWER_HINT;
        }
        for (i = SCN_APP_RUN_BASE; i < SCN_APP_RUN_BASE + nPackNum; i++) {
            resetScenario(i, 1); // reset all scenarios
            ptScnList[i].scn_type = SCN_PACK_HINT;
        }

        updateScnListfromXML(ptScnList+SCN_APP_RUN_BASE);

        perfservice_xmlparse_freeList();
        ALOGE("[init] perfservice_xmlparse_freeList");

        // empty list for user registration
        nUserScnBase = MTKPOWER_HINT_NUM;
        ALOGE("[init] nUserScnBase:%d", nUserScnBase);
        for (i = nUserScnBase; i < nUserScnBase + REG_SCN_MAX; i++) {
            resetScenario(i, 1);
        }

        ALOGE("[init] updateCusScnTable:%s", CUS_SCN_TABLE);

        updateCusScnTable(CUS_SCN_TABLE);

        /* perfd */
        if(load_fbc_api() < 0) {
            nFbcSupport = 0; // fbc is supported
        }
        ALOGI("[init] nFbcSupport:%d", nFbcSupport);

        /* MTK: alps/full_k50sv1_bsptc1_c2k5m_ss/
           k50sv1:8.0.0/O00623/1506069785:userdebug/dev-keys*/
        property_get("ro.build.fingerprint", prop_content, "0");
        ALOGI("ro.build.fingerpring:%s", prop_content);
        if(strstr(prop_content, "Android/aosp") != NULL)
            perfScnEnable(MTKPOWER_HINT_TEST_MODE);
    }
    return 1;
}

static int load_fbc_api(void)
{
    void *handle, *func;

    handle = dlopen(FBC_LIB_FULL_NAME, RTLD_NOW);
    if (handle == NULL) {
        ALOGI("dlopen error: %s", dlerror());
        return -1;
    }

    func = dlsym(handle, "fbcNotifyTouch");
    fbcNotifyTouch = reinterpret_cast<fbc_touch>(func);

    if (fbcNotifyTouch == NULL) {
        ALOGI("fbcNotifyTouch error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    return 0;
}

void setClusterCores(int scenario, int clusterNum, int totalCore, int *pCoreTbl, int *pMaxCoreTbl)
{
    int coreToSet = 0, maxToSet = 0, coreToSetBig = 0, maxToSetBig = 0;
    int i;
    char str[128], buf[32];

    if(gtDrvInfo.acao)
        return;

    ALOGV("[setClusterCores] scn:%d, total:%d, cores:%d, %d", scenario, totalCore, pCoreTbl[0], pCoreTbl[1]);

    if (gtDrvInfo.perfmgrCpu) {
        str[0] = '\0';

        for (i=0; i<clusterNum; i++) {
            coreToSet = (pCoreTbl[i] <= 0 || pCoreTbl[i] > ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pCoreTbl[i];
            maxToSet = (pMaxCoreTbl[i] < 0 || pMaxCoreTbl[i] >= ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pMaxCoreTbl[i];
            sprintf(buf, "%d %d ", coreToSet, maxToSet);
            /*strcat(str, buf);*/
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_BOOST_CORE_CTRL, str);
        ALOGI("%d: cpu_ctrl set: %s", scenario, str);
    }
    else if (gtDrvInfo.perfmgrLegacy) {
        str[0] = '\0';

        for (i=0; i<clusterNum; i++) {
            coreToSet = (pCoreTbl[i] <= 0 || pCoreTbl[i] > ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pCoreTbl[i];
            maxToSet = (pMaxCoreTbl[i] < 0 || pMaxCoreTbl[i] >= ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pMaxCoreTbl[i];
            sprintf(buf, "%d %d ", coreToSet, maxToSet);
            /*strcat(str, buf);*/
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_PERFMGR_CORE_CTRL, str);
        ALOGI("%d: legacy set: %s", scenario, str);
    }
    else if (gtDrvInfo.ppmAll) {
        str[0] = '\0';

        for (i=0; i<clusterNum; i++) {
            coreToSet = (pCoreTbl[i] <= 0 || pCoreTbl[i] > ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pCoreTbl[i];
            maxToSet = (pMaxCoreTbl[i] < 0 || pMaxCoreTbl[i] >= ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pMaxCoreTbl[i];
            sprintf(buf, "%d %d ", coreToSet, maxToSet);
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_PPM_CORE_CTRL, str);
        ALOGI("%d: ppmall set: %s", scenario, str);
    }
    else if (gtDrvInfo.ppmSupport) {
        str[0] = '\0';

        for (i=0; i<clusterNum; i++) {
            coreToSet = (pCoreTbl[i] <= 0 || pCoreTbl[i] > ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pCoreTbl[i];
            maxToSet = (pMaxCoreTbl[i] < 0 || pMaxCoreTbl[i] >= ptClusterTbl[i].cpuNum) ? PPM_IGNORE : pMaxCoreTbl[i];
            sprintf(buf, "%d %d ", coreToSet, maxToSet);
            strncat(str, buf, strlen(buf));
            set_value(PATH_PPM_CORE_BASE, i, coreToSet);
            set_value(PATH_PPM_CORE_LIMIT, i, maxToSet);
        }
        str[strlen(str)-1] = '\0'; // remove last space
        ALOGI("%d: ppmsupport set: %s", scenario, str);
    }
    else if (gtDrvInfo.hps) {
        // HPS: cpu_num_base must be >= 1
        coreToSet = (pCoreTbl[0] <= 0 || pCoreTbl[0] > ptClusterTbl[0].cpuNum) ? 1 : pCoreTbl[0];
        maxToSet = (pMaxCoreTbl[0] < 0 || pMaxCoreTbl[0] >= ptClusterTbl[0].cpuNum) ? ptClusterTbl[0].cpuNum : pMaxCoreTbl[0];
        if (gtDrvInfo.hmp) {
            coreToSetBig = (pCoreTbl[1] <= 0 || pCoreTbl[1] > ptClusterTbl[1].cpuNum) ? 0 : pCoreTbl[1];
            maxToSetBig = (pMaxCoreTbl[1] < 0 || pMaxCoreTbl[1] >= ptClusterTbl[1].cpuNum) ? ptClusterTbl[1].cpuNum : pMaxCoreTbl[1];
            set_value(PATH_CPUHOTPLUG_HPS_MIN, coreToSet, coreToSetBig);
            set_value(PATH_CPUHOTPLUG_HPS_MAX, maxToSet, maxToSetBig);
            ALOGI("%d: set: %d, %d", scenario, coreToSet, coreToSetBig);
            ALOGI("%d: set max: %d, %d", scenario, maxToSet, maxToSetBig);
        }
        else if (gtDrvInfo.hps) {
            set_value(PATH_CPUHOTPLUG_HPS_MIN, coreToSet);
            set_value(PATH_CPUHOTPLUG_HPS_MAX, maxToSet);
            ALOGI("%d: set: %d, set max:%d", scenario, coreToSet, maxToSet);
        }
        else {
            set_value(PATH_CPUHOTPLUG_CFG, coreToSet);
            ALOGI("%d: set: %d", scenario, coreToSet);
        }
    }
}

void setClusterHardFreq(int scenario, int clusterNum, int *pFreqTbl, int *pMaxFreqTbl)
{
    int freqToSet = 0, maxToSet = 0;
    int i;
    char str[128], buf[32];

    ALOGV("[setClusterHardFreq] scn:%d, freq:%d, %d , %d, %d", scenario, pFreqTbl[0], pMaxFreqTbl[0],pFreqTbl[1], pMaxFreqTbl[1]);
    if (gtDrvInfo.hard_user_limit) {
        str[0] = '\0';
        for (i=0; i<clusterNum; i++) {
            freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : pFreqTbl[i];
            maxToSet = (pMaxFreqTbl[i] <= 0 || pMaxFreqTbl[i] >= ptClusterTbl[i].freqMax) ? PPM_IGNORE : pMaxFreqTbl[i];
            sprintf(buf, "%d %d ", freqToSet, maxToSet);
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_HARD_USER_LIMIT, str);
        ALOGI("%d: hard user limit set freq: %s", scenario, str);
    }
}
void setClusterFreq(int scenario, int clusterNum, int *pFreqTbl, int *pMaxFreqTbl)
{
    int freqToSet = 0, maxToSet = 0, freqToSetBig = 0, maxToSetBig = 0;
    int i;
    char str[128], buf[32];

    ALOGV("[setClusterFreq] scn:%d, freq:%d, %d", scenario, pFreqTbl[0], pFreqTbl[1]);
    if (gtDrvInfo.perfmgrCpu) {
        str[0] = '\0';
        for (i=0; i<clusterNum; i++) {
            //freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : (pFreqTbl[i] > ptClusterTbl[i].freqMax) ? ptClusterTbl[i].freqMax : pFreqTbl[i];
            freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : pFreqTbl[i];
            maxToSet = (pMaxFreqTbl[i] <= 0 || pMaxFreqTbl[i] >= ptClusterTbl[i].freqMax) ? PPM_IGNORE : pMaxFreqTbl[i];
            sprintf(buf, "%d %d ", freqToSet, maxToSet);
            /*strcat(str, buf);*/
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_BOOST_FREQ_CTRL, str);
        ALOGI("%d: cpu_ctrl set freq: %s", scenario, str);
    }
    else if (gtDrvInfo.perfmgrLegacy) {
        str[0] = '\0';
        for (i=0; i<clusterNum; i++) {
            //freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : (pFreqTbl[i] > ptClusterTbl[i].freqMax) ? ptClusterTbl[i].freqMax : pFreqTbl[i];
            freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : pFreqTbl[i];
            maxToSet = (pMaxFreqTbl[i] <= 0 || pMaxFreqTbl[i] >= ptClusterTbl[i].freqMax) ? PPM_IGNORE : pMaxFreqTbl[i];
            sprintf(buf, "%d %d ", freqToSet, maxToSet);
            /*strcat(str, buf);*/
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_PERFMGR_FREQ_CTRL, str);
        ALOGI("%d: legacy set freq: %s", scenario, str);
    }
    else if (gtDrvInfo.ppmAll) {
        str[0] = '\0';
        for (i=0; i<clusterNum; i++) {
            //freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : (pFreqTbl[i] > ptClusterTbl[i].freqMax) ? ptClusterTbl[i].freqMax : pFreqTbl[i];
            freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : pFreqTbl[i];
            maxToSet = (pMaxFreqTbl[i] <= 0 || pMaxFreqTbl[i] >= ptClusterTbl[i].freqMax) ? PPM_IGNORE : pMaxFreqTbl[i];
            sprintf(buf, "%d %d ", freqToSet, maxToSet);
            /*strcat(str, buf);*/
            strncat(str, buf, strlen(buf));
        }
        str[strlen(str)-1] = '\0'; // remove last space
        set_value(PATH_PPM_FREQ_CTRL, str);
        ALOGI("%d: ppmall set freq: %s", scenario, str);
    }
    else if (gtDrvInfo.ppmSupport) {
        str[0] = '\0';
        for (i=0; i<clusterNum; i++) {
            //freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : (pFreqTbl[i] > ptClusterTbl[i].freqMax) ? ptClusterTbl[i].freqMax : pFreqTbl[i];
            freqToSet = (pFreqTbl[i] <= 0) ? PPM_IGNORE : pFreqTbl[i];
            maxToSet = (pMaxFreqTbl[i] <= 0 || pMaxFreqTbl[i] >= ptClusterTbl[i].freqMax) ? PPM_IGNORE : pMaxFreqTbl[i];
            sprintf(buf, "%d %d ", freqToSet, maxToSet);
            /*strcat(str, buf);*/
            strncat(str, buf, strlen(buf));
            set_value(PATH_PPM_FREQ_BASE, i, freqToSet);
            set_value(PATH_PPM_FREQ_LIMIT, i, maxToSet);
        }
        str[strlen(str)-1] = '\0'; // remove last space
        ALOGI("%d: ppm set freq: %s", scenario, str);
    }
    else {
        freqToSet = (pFreqTbl[0] <= 0) ? 0 : ((pFreqTbl[0] > ptClusterTbl[0].freqMax) ? ptClusterTbl[0].freqMax : pFreqTbl[0]);
        maxToSet = (pMaxFreqTbl[0] <= 0 || pMaxFreqTbl[0] >= ptClusterTbl[0].freqMax) ? 0 : pMaxFreqTbl[0];
        if (gtDrvInfo.dvfsHevc) {
            if (gtDrvInfo.hmp) {
                freqToSetBig = (pFreqTbl[1] <= 0) ? 0 : ((pFreqTbl[1] > ptClusterTbl[1].freqMax) ? ptClusterTbl[1].freqMax : pFreqTbl[1]);
                maxToSetBig = (pMaxFreqTbl[1] <= 0 || pMaxFreqTbl[1] >= ptClusterTbl[1].freqMax) ? 0 : pMaxFreqTbl[1];
                set_value(PATH_CPUFREQ_LIMIT, freqToSet);
                set_value(PATH_CPUFREQ_BIG_LIMIT, freqToSetBig);
                set_value(PATH_CPUFREQ_MAX_FREQ, maxToSet);
                set_value(PATH_CPUFREQ_MAX_FREQ_BIG, maxToSetBig);
                ALOGI("%d: set cpufreq freq: %d, %d", scenario, freqToSet, freqToSetBig);
                ALOGI("%d: set cpufreq freq max: %d, %d", scenario, maxToSet, maxToSetBig);
            } else {
                set_value(PATH_CPUFREQ_LIMIT, freqToSet);
                set_value(PATH_CPUFREQ_MAX_FREQ, maxToSet);
                ALOGI("%d: set cpufreq freq: %d, freq max:%d", scenario, freqToSet, maxToSet);
            }
        } else {
            maxToSet = (pMaxFreqTbl[0] <= 0 || pMaxFreqTbl[0] >= ptClusterTbl[0].freqMax) ? ptClusterTbl[0].freqMax : pMaxFreqTbl[0];
            set_value(PATH_CPUFREQ_MIN_FREQ_CPU0, freqToSet);
            set_value(PATH_CPUFREQ_MAX_FREQ_CPU0, maxToSet);
            ALOGI("%d: set freq: %d, freq max:%d", scenario, freqToSet, maxToSet);
        }
    }
}

void setGpuFreq(int scenario, int level)
{
    int levelToSet = 0;
    static int nSetFreqInit = 0;
    static int nIsGpuFreqSupport = 0;
    struct stat stat_buf;

    if(!nSetFreqInit) {
        nIsGpuFreqSupport = (0 == stat(PATH_GPUFREQ_COUNT, &stat_buf)) ? 1 : 0;
        nSetFreqInit = 1;
    }

    if(!nIsGpuFreqSupport)
        return;

    /*-- nGpuFreqCount means free run      --*/
    /*-- lowest index opp give to free run --*/
    if(level >= nGpuFreqCount)
        levelToSet = (nGpuFreqCount - 1);
    else
        levelToSet = level;
    ALOGI("%d: set gpu level: %d", scenario, levelToSet);
    set_gpu_freq_level(levelToSet); // 0 means maximum
}

void setGpuFreqMax(int scenario, int level)
{
    /*-- Since we use 0 to compare with other setting,--*/
    /*-- we don't need to use the condition check.   --*/
    /*-- nGpuFreqCount means free run       --*/
    /*-- highest index opp give to free run --*/
    /*-- if(level == nGpuFreqCount) levelToSet = 0;   --*/
    ALOGI("%d: set gpu level max: %d", scenario, level);
    set_gpu_freq_level_max(level); // 0 means maximun
}

void checkDrvSupport(tDrvInfo *ptDrvInfo)
{
    struct stat stat_buf;
    int ppmCore;

    ptDrvInfo->perfmgrCpu = (0 == stat(PATH_BOOST_FREQ_CTRL, &stat_buf)) ? 1 : 0;
    ptDrvInfo->perfmgrLegacy = (0 == stat(PATH_PERFMGR_FREQ_CTRL, &stat_buf)) ? 1 : 0;
    ptDrvInfo->ppmSupport = (0 == stat(PATH_PPM_FREQ_LIMIT, &stat_buf)) ? 1 : 0;
    ptDrvInfo->ppmAll = (0 == stat(PATH_PPM_FREQ_CTRL, &stat_buf)) ? 1 : 0;
    if(0 == stat(PATH_PERFMGR_TOPO_CHECK_HMP, &stat_buf))
        ptDrvInfo->hmp = (get_int_value(PATH_PERFMGR_TOPO_CHECK_HMP)==1) ? 1 : 0;
    else
        ptDrvInfo->hmp = (get_int_value(PATH_CPUTOPO_CHECK_HMP)==1) ? 1 : 0;
    ptDrvInfo->hps = (0 == stat(PATH_CPUHOTPLUG_HPS_MIN, &stat_buf)) ? 1 : 0;
    if(ptDrvInfo->hps == 0) {
        ALOGI("checkDrvSupport hps failed: %s\n", strerror(errno));
    }
    ptDrvInfo->dvfsHevc = (0 == stat(PATH_CPUFREQ_LIMIT, &stat_buf)) ? 1 : 0;
    if(ptDrvInfo->dvfsHevc == 0) {
        ALOGI("checkDrvSupport cpufreq failed: %s\n", strerror(errno));
    }
    ptDrvInfo->fliper = (0 == stat(PATH_VCORE, &stat_buf)) ? 1 : 0;
    ptDrvInfo->sysfs = (0 == stat(PATH_CPU_CPUFREQ, &stat_buf)) ? 1 : 0;
    ptDrvInfo->dvfs = (0 == stat(PATH_CPUFREQ_ROOT, &stat_buf)) ? 1 : 0;
    ptDrvInfo->fstb = (0 == stat(PATH_FSTB_SOFT_FPS, &stat_buf)) ? 1 : 0;

    ptDrvInfo->hard_user_limit = (0 == stat(PATH_HARD_USER_LIMIT, &stat_buf)) ? 1 : 0;
    if(ptDrvInfo->hard_user_limit == 0) {
        ALOGV("checkDrvSupport hard user limit failed: %s\n", strerror(errno));
    }

    ppmCore = (0 == stat(PATH_PPM_CORE_LIMIT, &stat_buf)) ? 1 : 0;
    if(ptDrvInfo->ppmSupport)
        ptDrvInfo->acao = (ppmCore) ? 0 : 1; // PPM not support core => ACAO
    else
        ptDrvInfo->acao = 0; // no PPM => no ACAO

    ptDrvInfo->turbo = (get_int_value(PATH_TURBO_SUPPORT)==1) ? 1 : 0;

    ALOGI("checkDrvSupport - perfmgr:%d, ppm:%d, ppmAll:%d, acao:%d, hmp:%d, hps:%d, hevc:%d, fliper:%d, sysfs:%d, dvfs:%d, turbo:%d, fstb:%d",
        ptDrvInfo->perfmgrLegacy, ptDrvInfo->ppmSupport, ptDrvInfo->ppmAll,ptDrvInfo->acao, ptDrvInfo->hmp,
        ptDrvInfo->hps, ptDrvInfo->dvfsHevc, ptDrvInfo->fliper, ptDrvInfo->sysfs, ptDrvInfo->dvfs, ptDrvInfo->turbo, ptDrvInfo->fstb);
}

void getCputopoInfo(int isHmpSupport, int *pnCpuNum, int *pnClusterNum, tClusterInfo **pptClusterTbl)
{
    int i, j;
    int cpu_num[CLUSTER_MAX], cpu_index[CLUSTER_MAX];
    int cputopoClusterNum;
    struct stat stat_buf;

    *pnCpuNum = get_cpu_num();

    if(0 == stat(PATH_PERFMGR_TOPO_NR_CLUSTER, &stat_buf))
        cputopoClusterNum = get_int_value(PATH_PERFMGR_TOPO_NR_CLUSTER);
    else
        cputopoClusterNum = get_int_value(PATH_CPUTOPO_NR_CLUSTER);
    *pnClusterNum = (cputopoClusterNum > 0 && isHmpSupport == 0) ? 1 : cputopoClusterNum; // temp solution for D3
    ALOGI("getCputopoInfo - cpuNum:%d, cluster:%d, cputopoCluster:%d", *pnCpuNum, *pnClusterNum, cputopoClusterNum);

    if((*pnClusterNum) < 0 || (*pnClusterNum) > CLUSTER_MAX) {
        ALOGE("wrong cluster number:%d", *pnClusterNum);
        return;
    }

    *pptClusterTbl = (tClusterInfo*)malloc(sizeof(tClusterInfo) * (*pnClusterNum));
    if (*pptClusterTbl  == NULL) {
        ALOGE("Can't allocate memory for pptClusterTbl");
        return;
    }

    get_cputopo_cpu_info(*pnClusterNum, cpu_num, cpu_index);

    for (i=0; i<*pnClusterNum; i++) {
        (*pptClusterTbl)[i].cpuNum = cpu_num[i];
        (*pptClusterTbl)[i].cpuFirstIndex = cpu_index[i];
        (*pptClusterTbl)[i].cpuMinNow = 0;
        (*pptClusterTbl)[i].cpuMaxNow = cpu_num[i];
        if (gtDrvInfo.ppmSupport)
            get_ppm_cpu_freq_info(i, &((*pptClusterTbl)[i].freqMax),&((*pptClusterTbl)[i].freqCount), &((*pptClusterTbl)[i].pFreqTbl));
        else
            get_cpu_freq_info(cpu_index[i], &((*pptClusterTbl)[i].freqMax), &((*pptClusterTbl)[i].freqCount), &((*pptClusterTbl)[i].pFreqTbl));
        (*pptClusterTbl)[i].freqMinNow = (*pptClusterTbl)[i].freqHardMinNow = 0;
        (*pptClusterTbl)[i].freqMaxNow = (*pptClusterTbl)[i].freqHardMaxNow = (*pptClusterTbl)[i].freqMax;
        ALOGI("[cluster %d]: cpu:%d, first:%d, freq count:%d, max_freq:%d", i, (*pptClusterTbl)[i].cpuNum, (*pptClusterTbl)[i].cpuFirstIndex, (*pptClusterTbl)[i].freqCount, (*pptClusterTbl)[i].freqMax);
        for (j=0; j<(*pptClusterTbl)[i].freqCount; j++) {
            if (j == 0 || j == (*pptClusterTbl)[i].freqCount - 1)
                ALOGI("  %d: %d", j, (*pptClusterTbl)[i].pFreqTbl[j]);
            else
                ALOGD("  %d: %d", j, (*pptClusterTbl)[i].pFreqTbl[j]);
        }
    }

    /* special case for Denali-3: 2 clusters but SMP */
    if(gtDrvInfo.ppmSupport == 0 && gtDrvInfo.hmp == 0 && cputopoClusterNum > 1) {
        (*pptClusterTbl)[0].cpuNum = *pnCpuNum;
        ALOGI("[cluster 0]: cpu:%d",(*pptClusterTbl)[0].cpuNum);
        (*pptClusterTbl)[0].cpuMaxNow = *pnCpuNum;
        ALOGI("[cluster 0]: cpu_max_now:%d",(*pptClusterTbl)[0].cpuMaxNow);
    }
}

inline int checkSuccess(int scenario)
{
    return (((scenario < MTKPOWER_HINT_NUM || scenario >= MTKPOWER_HINT_NUM + REG_SCN_MAX) && (scenario > 0)) && ptScnList[scenario].scn_valid) || (scenario >= nUserScnBase && scenario < nUserScnBase + REG_SCN_MAX);
}

int perfScnEnable(int scenario)
{
    int needUpdateCores = 0, needUpdateCoresMax = 0, needUpdateFreq = 0, needUpdateFreqMax = 0, i = 0;
    int scn_core_min[CLUSTER_MAX], actual_core_min[CLUSTER_MAX], totalCore, coreToSet;
    int scn_core_max[CLUSTER_MAX];
    int scn_freq_min[CLUSTER_MAX];
    int scn_freq_max[CLUSTER_MAX];
    int scn_freq_hard_min[CLUSTER_MAX];
    int scn_freq_hard_max[CLUSTER_MAX];
    int needUpdateHardFreq = 0;
    int result;

    if (checkSuccess(scenario)) {
        if (STATE_ON == ptScnList[scenario].scn_state)
            return 0;
        ALOGD("[perfScnEnable] scn:%d", scenario);

        ptScnList[scenario].scn_state = STATE_ON;

        ALOGV("[perfScnEnable] scn:%d, scn_cores_now:%d, scn_core_total:%d",  scenario, scn_cores_now, ptScnList[scenario].scn_core_total);
        if (scn_cores_now < ptScnList[scenario].scn_core_total) {
            scn_cores_now = ptScnList[scenario].scn_core_total;
            needUpdateCores = 1;
        }

        for (i=0; i<nClusterNum; i++) {

            if (ptClusterTbl[i].cpuMinNow < ptScnList[scenario].scn_core_min[i]) {
                ptClusterTbl[i].cpuMinNow = ptScnList[scenario].scn_core_min[i];
                needUpdateCores = 1;
            }
            scn_core_min[i] = ptClusterTbl[i].cpuMinNow;

            //ALOGV("[perfScnEnable] scn:%d, i:%d, cpuMaxNow:%d, scn_core_max:%d", scenario, i, ptClusterTbl[i].cpuMaxNow, ptScnList[scenario].scn_core_max[i]);
            if ((ptClusterTbl[i].cpuMaxNow > ptScnList[scenario].scn_core_max[i] || ptClusterTbl[i].cpuMaxNow == PPM_IGNORE)) {
                ptClusterTbl[i].cpuMaxNow = ptScnList[scenario].scn_core_max[i];
                needUpdateCoresMax = 1;
            }

            ALOGV("[perfScnEnable] scn:%d, i:%d, freqMinNow:%d, scn_freq_min:%d",  scenario, i, ptClusterTbl[i].freqMinNow, ptScnList[scenario].scn_freq_min[i]);
            if (ptClusterTbl[i].freqMinNow < ptScnList[scenario].scn_freq_min[i]) {
                ptClusterTbl[i].freqMinNow = ptScnList[scenario].scn_freq_min[i];
                needUpdateFreq = 1;
            }
            scn_freq_min[i] = ptClusterTbl[i].freqMinNow;

            ALOGV("[perfScnEnable] scn:%d, i:%d, freqMaxNow:%d, scn_freq_max:%d",  scenario, i, ptClusterTbl[i].freqMaxNow, ptScnList[scenario].scn_freq_max[i]);
            if (ptClusterTbl[i].freqMaxNow > ptScnList[scenario].scn_freq_max[i]) {
                ptClusterTbl[i].freqMaxNow = ptScnList[scenario].scn_freq_max[i];
                needUpdateFreqMax = 1;
            }
            if (ptClusterTbl[i].freqMaxNow < scn_freq_min[i]) { // if max < min => align max with min
                ptClusterTbl[i].freqMaxNow = scn_freq_min[i];
                needUpdateFreqMax = 1;
            }
            scn_freq_max[i] = ptClusterTbl[i].freqMaxNow;

            ALOGV("[perfScnEnable] scn:%d, i:%d, freqHardMinNow:%d, scn_freq_hard_min:%d",  scenario, i, ptClusterTbl[i].freqHardMinNow, ptScnList[scenario].scn_freq_hard_min[i]);
            if (ptClusterTbl[i].freqHardMinNow < ptScnList[scenario].scn_freq_hard_min[i]) {
                ptClusterTbl[i].freqHardMinNow = ptScnList[scenario].scn_freq_hard_min[i];
                needUpdateHardFreq = 1;
            }
            scn_freq_hard_min[i] = ptClusterTbl[i].freqHardMinNow;

            ALOGV("[perfScnEnable] scn:%d, i:%d, freqHardMaxNow:%d, scn_freq_hard_max:%d",  scenario, i, ptClusterTbl[i].freqHardMaxNow, ptScnList[scenario].scn_freq_hard_max[i]);
            if (ptClusterTbl[i].freqHardMaxNow > ptScnList[scenario].scn_freq_hard_max[i]) {
                ptClusterTbl[i].freqHardMaxNow = ptScnList[scenario].scn_freq_hard_max[i];
                needUpdateHardFreq = 1;
            }
            if (ptClusterTbl[i].freqHardMaxNow < scn_freq_hard_min[i]) { // if max < min => align ceiling with floor
                ptClusterTbl[i].freqHardMaxNow = scn_freq_hard_min[i];
                needUpdateHardFreq = 1;
            }
            scn_freq_hard_max[i] = ptClusterTbl[i].freqHardMaxNow;
        }

        /*--gpu opp used start--*/
        if ((ptScnList[scenario].scn_gpu_freq != -1) || (ptScnList[scenario].scn_gpu_freq_max != -1)) {
            /*-- If scn_gpu-freq was equal to -1 which mean don't care, --*/
            /*-- we don't do anything and keep the scn_gpu_freq_now     --*/
            if ((ptScnList[scenario].scn_gpu_freq != -1) &&
                (scn_gpu_freq_now > ptScnList[scenario].scn_gpu_freq)) {
                 scn_gpu_freq_now = ptScnList[scenario].scn_gpu_freq;
                 setGpuFreq(scenario, scn_gpu_freq_now);
            }

            /*-- check freq max --*/
            if ((ptScnList[scenario].scn_gpu_freq_max != -1) &&
                (scn_gpu_freq_max_now < ptScnList[scenario].scn_gpu_freq_max))
                scn_gpu_freq_max_now = ptScnList[scenario].scn_gpu_freq_max;

            if (scn_gpu_freq_max_now > scn_gpu_freq_now)
                scn_gpu_freq_max_now = scn_gpu_freq_now;

            setGpuFreqMax(scenario, scn_gpu_freq_max_now);
        } /*--gpu opp used end--*/

        // fine tune max
        totalCore = scn_cores_now;
        for (i=nClusterNum-1; i>=0; i--) {
            coreToSet = (scn_core_min[i] <= 0 || scn_core_min[i] > ptClusterTbl[i].cpuNum || totalCore <= 0) ? PPM_IGNORE : ((scn_core_min[i] > totalCore) ? totalCore : scn_core_min[i]);
            if(coreToSet >= 0)
                totalCore -= coreToSet;
            actual_core_min[i] = coreToSet;

            if (ptClusterTbl[i].cpuMaxNow < actual_core_min[i]) { // min priority is higher than max
                ptClusterTbl[i].cpuMaxNow = actual_core_min[i];
                needUpdateCoresMax = 1;
            }
            scn_core_max[i] = ptClusterTbl[i].cpuMaxNow;
        }

        // L and LL: only one cluster can set max cpu = 0
        if (nClusterNum > 1 && ptClusterTbl[1].cpuMaxNow == 0 && ptClusterTbl[0].cpuMaxNow == 0) {
            ptClusterTbl[0].cpuMaxNow = scn_core_max[0] = PPM_IGNORE;
            needUpdateCoresMax = 1;
        }

        if (needUpdateFreq || needUpdateFreqMax) {
            setClusterFreq(scenario, nClusterNum, scn_freq_min, scn_freq_max);
        }

        if (needUpdateHardFreq) {
            setClusterHardFreq(scenario, nClusterNum, scn_freq_hard_min, scn_freq_hard_max);
        }

        if (needUpdateCoresMax || needUpdateCores) {
            setClusterCores(scenario, nClusterNum, scn_cores_now, actual_core_min, scn_core_max);
        }

        /*
            scan control table(perfcontable.txt) and judge which setting of scene is beeter
            and then replace it.
            less is meaning system setting less than current scene value is better
            more is meaning system setting more than current scene value is better
        */
        for(int idx = 0; idx < FIELD_SIZE; idx++) {
            if(tConTable[idx].entry.length() == 0)
                break;

            ALOGV("[perfScnEnable] scn:%d, cmd:%x, isValid:%d, cur:%d, param:%d", scenario, tConTable[idx].cmdID, tConTable[idx].isValid,
                tConTable[idx].curVal, ptScnList[scenario].scn_param[idx]);

            if(tConTable[idx].isValid == -1)
                continue;

            if(tConTable[idx].comp.compare(LESS) == 0) {
                if(ptScnList[scenario].scn_param[idx] < tConTable[idx].curVal )
                {
                    tConTable[idx].curVal = ptScnList[scenario].scn_param[idx];
                    if(tConTable[idx].prefix.length() == 0)
                        set_value(tConTable[idx].entry.c_str(),
                                ptScnList[scenario].scn_param[idx]);
                    else {
                        char inBuf[64];
                        //ALOGD("[perfScnEnable] cmd:%x, (less) prefix:%s; %d", tConTable[idx].cmdID, tConTable[idx].prefix.c_str(), ptScnList[scenario].scn_param[idx]);
                        snprintf(inBuf, 64, "%s%d", tConTable[idx].prefix.c_str(), ptScnList[scenario].scn_param[idx]);
                        set_value(tConTable[idx].entry.c_str(), inBuf);
                        ALOGI("[perfScnEnable] (less) %s set +prefix:%s;", tConTable[idx].entry.c_str(), inBuf);
                    }
                }
            }
            else {
                if(ptScnList[scenario].scn_param[idx] > tConTable[idx].curVal)
                {
                    tConTable[idx].curVal = ptScnList[scenario].scn_param[idx];
                    if(tConTable[idx].prefix.length() == 0)
                        set_value(tConTable[idx].entry.c_str(),
                              ptScnList[scenario].scn_param[idx]);
                    else {
                        char inBuf[64];
                        //ALOGD("[perfScnEnable] cmd:%x, (more) prefix:%s; %d", tConTable[idx].cmdID, tConTable[idx].prefix.c_str(), ptScnList[scenario].scn_param[idx]);
                        snprintf(inBuf, 64, "%s%d", tConTable[idx].prefix.c_str(), ptScnList[scenario].scn_param[idx]);
                        set_value(tConTable[idx].entry.c_str(), inBuf);
                        ALOGI("[perfScnEnable] (more) %s set +prefix:%s;", tConTable[idx].entry.c_str(), inBuf);
                    }
                }
            }
        }

        /*Rescontable*/
        for(int idx = 0; idx < gRscCtlTblLen; idx++) {

            if(RscCfgTbl[idx].set_func == NULL || RscCfgTbl[idx].unset_func == NULL)
                break;

            ALOGV("[perfScnEnable] RscCfgTbl[%d] scn:%d, cmd:%x, isValid:%d, cur:%d, param:%d resetVal:%d", idx,
                scenario, RscCfgTbl[idx].cmdID, gRscCtlTbl[idx].isValid,
                gRscCtlTbl[idx].curVal, ptScnList[scenario].scn_rsc[idx], gRscCtlTbl[idx].resetVal);

            if(gRscCtlTbl[idx].isValid != 1)
                continue;

            if(RscCfgTbl[idx].comp == SMALLEST) {
                if(ptScnList[scenario].scn_rsc[idx] < gRscCtlTbl[idx].curVal)
                {
                    gRscCtlTbl[idx].curVal = ptScnList[scenario].scn_rsc[idx];
                    result = RscCfgTbl[idx].set_func(ptScnList[scenario].scn_rsc[idx], (void*)&ptScnList[scenario]);
                    ALOGV("[perfScnEnable] RscCfgTbl SMALLEST cur:%d, ret:%d", gRscCtlTbl[idx].curVal, result);
                }
            } else if(RscCfgTbl[idx].comp == BIGGEST) {
                if(ptScnList[scenario].scn_rsc[idx] > gRscCtlTbl[idx].curVal)
                {
                    gRscCtlTbl[idx].curVal = ptScnList[scenario].scn_rsc[idx];
                    result = RscCfgTbl[idx].set_func(ptScnList[scenario].scn_rsc[idx], (void*)&ptScnList[scenario]);
                    ALOGV("[perfScnEnable] RscCfgTbl BIGGEST cur:%d, ret:%d", gRscCtlTbl[idx].curVal, result);
                }
            } else {
                if(ptScnList[scenario].scn_rsc[idx] != gRscCtlTbl[idx].resetVal)
                {
                    gRscCtlTbl[idx].curVal = ptScnList[scenario].scn_rsc[idx];
                    result = RscCfgTbl[idx].set_func(ptScnList[scenario].scn_rsc[idx], (void*)&ptScnList[scenario]);
                    ALOGV("[perfScnEnable] RscCfgTbl ONESHOT cur:%d, ret:%d", gRscCtlTbl[idx].curVal, result);
                }
            }
        }
    }

    return 0;
}

int perfScnDisable(int scenario)
{
    return perfScnUpdate(scenario, 0);
}

/*
    force_update == 0 => disable scenario
    force_update == 1 => scan all scenario and update setting
 */
int perfScnUpdate(int scenario, int force_update)
{
    int needUpdateCores = 0;
    int totalCoresToSet = 0;
    /*-- nGpuFreqCount means free run for base --*/
    int gpuFreqToSet = nGpuHighestFreqLevel; // opp(n-1) is the lowest freq
    /*-- 0 means free run for upbound --*/
    int gpuFreqMaxToSet = 0; // opp 0
    int needUpdate = 0;
    int coresToSet[CLUSTER_MAX], actual_core_min[CLUSTER_MAX], maxCoresToSet[CLUSTER_MAX], lastCore[CLUSTER_MAX];
    int freqToSet[CLUSTER_MAX], lastFreq[CLUSTER_MAX], maxFreqToSet[CLUSTER_MAX], lastGpuFreq, lastGpuMaxFreq;
    int totalCore, coreToSet, numToSet, numofCurr;
    int i, j;
    int hardFreqToSet[CLUSTER_MAX], lastHardFreq[CLUSTER_MAX], maxHardFreqToSet[CLUSTER_MAX];
    int needUpdateHardFreq = 0;
    int result;

    if (checkSuccess(scenario)) {
        if (STATE_OFF == ptScnList[scenario].scn_state)
            return 0;

        if (force_update) {
            ALOGI("[perfScnUpdate] scn:%d, update", scenario);
        }
        else {
            ALOGD("[perfScnDisable] scn:%d", scenario);
            ptScnList[scenario].scn_state = STATE_OFF;
        }

        // check core
        //ALOGV("[perfScnUpdate] scenario:%d, scn_cores_now:%d, scn_core_total:%d", scenario, scn_cores_now, ptScnList[scenario].scn_core_total);

        needUpdateCores = 0;
        if (scn_cores_now <= ptScnList[scenario].scn_core_total || force_update) {
            for (i = 0; i < SCN_APP_RUN_BASE + nPackNum; i++) {
                if (ptScnList[i].scn_state == STATE_ON)
                    totalCoresToSet = max(totalCoresToSet, ptScnList[i].scn_core_total);
            }

            if (scn_cores_now != totalCoresToSet) {
                ALOGV("[perfScnUpdate] scn_cores_now:%d, totalCoresToSet:%d", scn_cores_now, totalCoresToSet);
                scn_cores_now = totalCoresToSet;
                needUpdateCores = 1;
            }
        }
        else {
            totalCoresToSet = scn_cores_now;
        }

        for (i=0; i<nClusterNum; i++) {
            lastCore[i] = ptClusterTbl[i].cpuMinNow;
            if (ptClusterTbl[i].cpuMinNow <= ptScnList[scenario].scn_core_min[i] || force_update) {
                coresToSet[i] = 0;
                for (j = 0; j < SCN_APP_RUN_BASE + nPackNum; j++) {
                    if (ptScnList[j].scn_state == STATE_ON) {
                        coresToSet[i] = max(coresToSet[i], ptScnList[j].scn_core_min[i]);
                    }
                }
                if(coresToSet[i] != ptClusterTbl[i].cpuMinNow) {
                    ALOGV("[perfScnUpdate] i:%d, cpuMinNow:%d, coresToSet:%d", i, ptClusterTbl[i].cpuMinNow, coresToSet[i]);
                    ptClusterTbl[i].cpuMinNow = coresToSet[i];
                    needUpdateCores = 1;
                }
            }
            else {
                coresToSet[i] = ptClusterTbl[i].cpuMinNow;
            }

            ALOGV("[perfScnUpdate] scn:%d, i:%d, cpuMaxNow:%d, scn_core_max:%d",  scenario, i, ptClusterTbl[i].cpuMaxNow, ptScnList[scenario].scn_core_max[i]);
            if (ptClusterTbl[i].cpuMaxNow >= ptScnList[scenario].scn_core_max[i] || ptClusterTbl[i].cpuMaxNow == lastCore[i] || force_update) {
                maxCoresToSet[i] = CORE_MAX;
                for (j = 0; j < SCN_APP_RUN_BASE + nPackNum; j++) {
                    if (ptScnList[j].scn_state == STATE_ON)
                        maxCoresToSet[i] = min(maxCoresToSet[i], ptScnList[j].scn_core_max[i]);
                }
                if(maxCoresToSet[i] != ptClusterTbl[i].cpuMaxNow) {
                    ptClusterTbl[i].cpuMaxNow = maxCoresToSet[i];
                    needUpdateCores = 1;
                }
            }
        }

        // fine tune max
        totalCore = scn_cores_now;
        for (i=nClusterNum-1; i>=0; i--) {
            coreToSet = (coresToSet[i] <= 0 || coresToSet[i] > ptClusterTbl[i].cpuNum || totalCore <= 0) ? PPM_IGNORE : ((coresToSet[i] > totalCore) ? totalCore : coresToSet[i]);
            if(coreToSet >= 0)
                totalCore -= coreToSet;
            actual_core_min[i] = coreToSet;

            if (ptClusterTbl[i].cpuMaxNow < actual_core_min[i]) { // min priority is higher than max
                ptClusterTbl[i].cpuMaxNow = actual_core_min[i];
                needUpdateCores = 1;
            }
            maxCoresToSet[i] = ptClusterTbl[i].cpuMaxNow;
        }

        /* update core */
        if(needUpdateCores) {
            setClusterCores(scenario, nClusterNum, scn_cores_now, actual_core_min, maxCoresToSet);
        }

        needUpdate = needUpdateHardFreq = 0;
        for (i=0; i<nClusterNum; i++) {
            /* CPU freq floor */
            lastFreq[i] = ptClusterTbl[i].freqMinNow;
            if (ptClusterTbl[i].freqMinNow <= ptScnList[scenario].scn_freq_min[i] || force_update) {
                freqToSet[i] = 0;
                for (j = 0; j < SCN_APP_RUN_BASE + nPackNum; j++) {
                    if (ptScnList[j].scn_state == STATE_ON)
                        freqToSet[i] = max(freqToSet[i], ptScnList[j].scn_freq_min[i]);
                }
                if(freqToSet[i] != ptClusterTbl[i].freqMinNow) {
                    ptClusterTbl[i].freqMinNow = freqToSet[i];
                    needUpdate = 1;
                }
            }
            else {
                freqToSet[i] = ptClusterTbl[i].freqMinNow;
            }

            lastHardFreq[i] = ptClusterTbl[i].freqHardMinNow;
            if (ptClusterTbl[i].freqHardMinNow <= ptScnList[scenario].scn_freq_hard_min[i] || force_update) {
                hardFreqToSet[i] = 0;
                for (j = 0; j < SCN_APP_RUN_BASE + nPackNum; j++) {
                    if (ptScnList[j].scn_state == STATE_ON)
                        hardFreqToSet[i] = max(hardFreqToSet[i], ptScnList[j].scn_freq_hard_min[i]);
                }
                if(hardFreqToSet[i] != ptClusterTbl[i].freqHardMinNow) {
                    ptClusterTbl[i].freqHardMinNow = hardFreqToSet[i];
                    needUpdateHardFreq = 1;
                }
            }
            else {
                hardFreqToSet[i] = ptClusterTbl[i].freqHardMinNow;
            }

            /* CPU freq ceiling */
            ALOGV("[perfScnUpdate] scn:%d, i:%d, last_min:%d, global_max:%d, max:%d",  scenario, i, lastFreq[i], ptClusterTbl[i].freqMaxNow, ptScnList[scenario].scn_freq_max[i]);
            if (ptClusterTbl[i].freqMaxNow >= ptScnList[scenario].scn_freq_max[i] || \
                ptClusterTbl[i].freqMaxNow == lastFreq[i] || force_update) { // perfservice might ignore someone's setting before
                maxFreqToSet[i] = FREQ_MAX;
                for (j = 0; j < SCN_APP_RUN_BASE + nPackNum; j++) {
                    if (ptScnList[j].scn_state == STATE_ON)
                        maxFreqToSet[i] = min(maxFreqToSet[i], ptScnList[j].scn_freq_max[i]);
                }
                if(maxFreqToSet[i] < freqToSet[i]) { // if max < min => align max with min
                    maxFreqToSet[i] = freqToSet[i];
                }
                if(maxFreqToSet[i] != ptClusterTbl[i].freqMaxNow) {
                    ptClusterTbl[i].freqMaxNow = maxFreqToSet[i];
                    needUpdate = 1;
                }
            }
            else {
                maxFreqToSet[i] = ptClusterTbl[i].freqMaxNow;
            }

            ALOGV("[perfScnUpdate] scn:%d, i:%d, last_hard_min:%d, global_hard_max:%d, max:%d",  scenario, i, lastHardFreq[i], ptClusterTbl[i].freqHardMaxNow, ptScnList[scenario].scn_freq_max[i]);
            if (ptClusterTbl[i].freqHardMaxNow >= ptScnList[scenario].scn_freq_hard_max[i] || \
                ptClusterTbl[i].freqHardMaxNow == lastHardFreq[i] || force_update) { // perfservice might ignore someone's setting before
                maxHardFreqToSet[i] = FREQ_MAX;
                for (j = 0; j < SCN_APP_RUN_BASE + nPackNum; j++) {
                    if (ptScnList[j].scn_state == STATE_ON)
                        maxHardFreqToSet[i] = min(maxHardFreqToSet[i], ptScnList[j].scn_freq_hard_max[i]);
                }
                if(maxHardFreqToSet[i] < hardFreqToSet[i]) { // if max < min => align max with min
                    maxHardFreqToSet[i] = hardFreqToSet[i];
                }
                if(maxHardFreqToSet[i] != ptClusterTbl[i].freqHardMaxNow) {
                    ptClusterTbl[i].freqHardMaxNow = maxHardFreqToSet[i];
                    needUpdateHardFreq = 1;
                }
            }
            else {
                maxHardFreqToSet[i] = ptClusterTbl[i].freqHardMaxNow;
            }
        }

        if(needUpdate) {
            setClusterFreq(scenario, nClusterNum, freqToSet, maxFreqToSet);
        }

        if(needUpdateHardFreq) {
            setClusterHardFreq(scenario, nClusterNum, hardFreqToSet, maxHardFreqToSet);
        }

        /*--gpu opp used start--*/
        if ((ptScnList[scenario].scn_gpu_freq != -1) || (ptScnList[scenario].scn_gpu_freq_max != -1) || force_update) {
            int baseNeedTraverSal = 0;
            int upBaseNeedTraverSal = 0;
            ALOGD("[perfScnUpdate] gpu_freq:%d, gpu_max:%d, scn_gpu_freq:%d, scn_gpu_max:%d, gpuFreqToSet:%d, gpuFreqMaxToSet:%d",
                scn_gpu_freq_now, scn_gpu_freq_max_now, ptScnList[scenario].scn_gpu_freq, ptScnList[scenario].scn_gpu_freq_max, gpuFreqToSet, gpuFreqMaxToSet);

            lastGpuFreq = scn_gpu_freq_now;
            lastGpuMaxFreq = scn_gpu_freq_max_now;
            /*--GPU opp base was equal to the setting of scenario or--*/
            /*--the setting of scenario don't care the base.        --*/
            /*--We need to choose the min request for base.         --*/
            if((scn_gpu_freq_now == ptScnList[scenario].scn_gpu_freq) ||
                (ptScnList[scenario].scn_gpu_freq != -1) || force_update) {
                    baseNeedTraverSal = 1;
            }
            /*--GPU opp upbound base was equal to the base              --*/
            /*--which might had adjusted by the method (base < upbound )--*/
            /*--the setting of scenario don't care the base.            --*/
            /*--We need to choose the min request for base.             --*/
            if((scn_gpu_freq_max_now == lastGpuFreq) ||
                (scn_gpu_freq_max_now == ptScnList[scenario].scn_gpu_freq_max) ||
                (ptScnList[scenario].scn_gpu_freq_max != -1) || force_update) {
                upBaseNeedTraverSal = 1;
            }

            for (i = 0; i < SCN_APP_RUN_BASE + nPackNum; i++) {
                if (ptScnList[i].scn_state == STATE_ON) {

                    if (baseNeedTraverSal && (ptScnList[i].scn_gpu_freq != -1))
                        gpuFreqToSet = min(gpuFreqToSet, ptScnList[i].scn_gpu_freq);

                    if (upBaseNeedTraverSal && (ptScnList[i].scn_gpu_freq_max != -1))
                        gpuFreqMaxToSet = max(gpuFreqMaxToSet, ptScnList[i].scn_gpu_freq_max);

                 }
             }

             if (baseNeedTraverSal)
                 scn_gpu_freq_now = gpuFreqToSet;

             if (scn_gpu_freq_now != lastGpuFreq || scn_gpu_freq_max_now != lastGpuMaxFreq)
                setGpuFreq(scenario, scn_gpu_freq_now);

             if (upBaseNeedTraverSal)
                 scn_gpu_freq_max_now = ((gpuFreqMaxToSet > scn_gpu_freq_now) ? (scn_gpu_freq_now) : (gpuFreqMaxToSet));
             else
                 scn_gpu_freq_max_now = ((scn_gpu_freq_max_now > scn_gpu_freq_now) ? (scn_gpu_freq_now) : (scn_gpu_freq_max_now));

             if (scn_gpu_freq_now != lastGpuFreq || scn_gpu_freq_max_now != lastGpuMaxFreq)
                setGpuFreqMax(scenario, scn_gpu_freq_max_now);
        } /*--gpu opp used end--*/

        for(int idx = 0; idx < FIELD_SIZE; idx++) {
            if( tConTable[idx].entry.length() == 0 )
                break;

            if( tConTable[idx].isValid == -1)
                continue;

            ALOGV("[perfScnUpdate] scn:%d, cmd:%x, reset:%d, default:%d, cur:%d, param:%d", scenario, tConTable[idx].cmdID,
                tConTable[idx].resetVal, tConTable[idx].defaultVal, tConTable[idx].curVal, ptScnList[scenario].scn_param[idx]);
            if(tConTable[idx].comp.compare(LESS) == 0) {
                if(force_update || (ptScnList[scenario].scn_param[idx] < tConTable[idx].resetVal
                        && ptScnList[scenario].scn_param[idx] <= tConTable[idx].curVal)) {
                    numToSet = tConTable[idx].resetVal;

                    for (i = 0; i < SCN_APP_RUN_BASE + nPackNum; i++) {
                        if (ptScnList[i].scn_state == STATE_ON) {
                            numToSet = min(numToSet, ptScnList[i].scn_param[idx]);
                        }
                    }

                    numofCurr = tConTable[idx].curVal;
                    tConTable[idx].curVal = numToSet;

                    if(tConTable[idx].curVal != numofCurr) {
                        if(tConTable[idx].curVal == tConTable[idx].resetVal)
                            numToSet = tConTable[idx].defaultVal;
                        else
                            numToSet = tConTable[idx].curVal;

                        if(tConTable[idx].prefix.length() == 0)
                            set_value(tConTable[idx].entry.c_str(), numToSet);
                        else {
                            char inBuf[64];
                            snprintf(inBuf, 64, "%s%d", tConTable[idx].prefix.c_str(), numToSet);
                            set_value(tConTable[idx].entry.c_str(), inBuf);
                            ALOGI("[perfScnUpdate] (less) %s set +prefix:%s;", tConTable[idx].entry.c_str(), inBuf);
                        }
                    }
                }
            }
            else {
                if(force_update || (ptScnList[scenario].scn_param[idx] > tConTable[idx].resetVal
                        && ptScnList[scenario].scn_param[idx] >= tConTable[idx].curVal)) {
                    numToSet = tConTable[idx].resetVal;

                    for (i = 0; i < SCN_APP_RUN_BASE + nPackNum; i++) {
                        if (ptScnList[i].scn_state == STATE_ON) {
                            numToSet = max(numToSet, ptScnList[i].scn_param[idx]);
                        }
                    }

                    numofCurr = tConTable[idx].curVal;
                    tConTable[idx].curVal = numToSet;

                    if(tConTable[idx].curVal != numofCurr) {
                        if(tConTable[idx].curVal == tConTable[idx].resetVal)
                            numToSet = tConTable[idx].defaultVal;
                        else
                            numToSet = tConTable[idx].curVal;

                        if(tConTable[idx].prefix.length() == 0)
                            set_value(tConTable[idx].entry.c_str(), numToSet);
                        else {
                            char inBuf[64];
                            snprintf(inBuf, 64, "%s%d", tConTable[idx].prefix.c_str(), numToSet);
                            set_value(tConTable[idx].entry.c_str(), inBuf);
                            ALOGI("[perfScnUpdate] (more) %s set +prefix:%s;", tConTable[idx].entry.c_str(), inBuf);
                        }
                    }
                }
            }
        }

        /*Rescontable*/
        for(int idx = 0; idx < gRscCtlTblLen; idx++) {
            ALOGV("[perfScnUpdate] RscCfgTbl[%d] scn:%d, cmd:%x, param:%d",
                idx, scenario, RscCfgTbl[idx].cmdID, ptScnList[scenario].scn_rsc[idx]);

            if(RscCfgTbl[idx].set_func == NULL || RscCfgTbl[idx].unset_func == NULL)
                break;

            if(gRscCtlTbl[idx].isValid != 1)
                continue;

            ALOGV("[perfScnUpdate] RscCtlTbl[%d] scn:%d, cmd:%x, reset:%d, default:%d, cur:%d, param:%d", idx, scenario, RscCfgTbl[idx].cmdID,
                gRscCtlTbl[idx].resetVal, RscCfgTbl[idx].defaultVal, gRscCtlTbl[idx].curVal, ptScnList[scenario].scn_rsc[idx]);

            if (RscCfgTbl[idx].comp == SMALLEST) {
                if(force_update || (ptScnList[scenario].scn_rsc[idx] < gRscCtlTbl[idx].resetVal
                        && ptScnList[scenario].scn_rsc[idx] <= gRscCtlTbl[idx].curVal)) {
                    numToSet = gRscCtlTbl[idx].resetVal;

                    for (i = 0; i < SCN_APP_RUN_BASE + nPackNum; i++) {
                        if (ptScnList[i].scn_state == STATE_ON) {
                            numToSet = min(numToSet, ptScnList[i].scn_rsc[idx]);
                        }
                    }

                    numofCurr = gRscCtlTbl[idx].curVal;
                    gRscCtlTbl[idx].curVal = numToSet;

                    if(gRscCtlTbl[idx].curVal != numofCurr || RscCfgTbl[idx].force_update == 1) {
                        if(gRscCtlTbl[idx].curVal == gRscCtlTbl[idx].resetVal) {
                            numToSet = RscCfgTbl[idx].defaultVal;
                            result = RscCfgTbl[idx].unset_func(numToSet, (void*)&ptScnList[scenario]);
                        } else {
                            numToSet = gRscCtlTbl[idx].curVal;
                            result = RscCfgTbl[idx].set_func(numToSet, (void*)&ptScnList[scenario]);
                        }
                    }
                    ALOGV("[perfScnUpdate] RscCfgTbl SMALLEST numToSet:%d, ret:%d", numToSet, result);
                }
            } else if (RscCfgTbl[idx].comp == BIGGEST) {
                if(force_update || (ptScnList[scenario].scn_rsc[idx] > gRscCtlTbl[idx].resetVal
                        && ptScnList[scenario].scn_rsc[idx] >= gRscCtlTbl[idx].curVal)) {
                    numToSet = gRscCtlTbl[idx].resetVal;

                    for (i = 0; i < SCN_APP_RUN_BASE + nPackNum; i++) {
                        if (ptScnList[i].scn_state == STATE_ON) {
                            numToSet = max(numToSet, ptScnList[i].scn_rsc[idx]);
                        }
                    }

                    numofCurr = gRscCtlTbl[idx].curVal;
                    gRscCtlTbl[idx].curVal = numToSet;

                    if(gRscCtlTbl[idx].curVal != numofCurr || RscCfgTbl[idx].force_update == 1) {
                        if(gRscCtlTbl[idx].curVal == gRscCtlTbl[idx].resetVal) {
                            numToSet = RscCfgTbl[idx].defaultVal;
                            result = RscCfgTbl[idx].unset_func(numToSet, (void*)&ptScnList[scenario]);
                        } else {
                            numToSet = gRscCtlTbl[idx].curVal;
                            result = RscCfgTbl[idx].set_func(numToSet, (void*)&ptScnList[scenario]);
                        }
                    }
                    ALOGV("[perfScnUpdate] RscCfgTbl BIGGEST numToSet:%d, ret:%d", numToSet, result);
                }
            } else {
                if(force_update) {
                    // unset original value
                    ALOGD("[perfScnUpdate] idx:%d, reset:%d, prev:%d, rsc:%d", idx, gRscCtlTbl[idx].resetVal, ptScnList[scenario].scn_prev_rsc[idx], ptScnList[scenario].scn_rsc[idx]);
                    if(ptScnList[scenario].scn_prev_rsc[idx] != gRscCtlTbl[idx].resetVal && \
                       ptScnList[scenario].scn_prev_rsc[idx] != ptScnList[scenario].scn_rsc[idx] ) {
                        result = RscCfgTbl[idx].unset_func(ptScnList[scenario].scn_prev_rsc[idx], (void*)&ptScnList[scenario]);
                        ALOGV("[perfScnUpdate] RscCfgTbl ONESHOT unset_func:%d, ret:%d", ptScnList[scenario].scn_prev_rsc[idx], result);
                    }

                    // set new value
                    if((ptScnList[scenario].scn_rsc[idx] != gRscCtlTbl[idx].resetVal && \
                        ptScnList[scenario].scn_rsc[idx] != ptScnList[scenario].scn_prev_rsc[idx]) \
                        || RscCfgTbl[idx].force_update == 1) {
                        result = RscCfgTbl[idx].set_func(ptScnList[scenario].scn_rsc[idx], (void*)&ptScnList[scenario]);
                        ALOGV("[perfScnUpdate] RscCfgTbl ONESHOT cur:%d, ret:%d", gRscCtlTbl[idx].curVal, result);
                    }
                } else if(ptScnList[scenario].scn_rsc[idx] != gRscCtlTbl[idx].resetVal) {
                    result = RscCfgTbl[idx].unset_func(ptScnList[scenario].scn_rsc[idx], (void*)&ptScnList[scenario]);
                    ALOGV("[perfScnUpdate] RscCfgTbl ONESHOT unset_func, ret:%d", result);
                }
            }
        }
    }

    return 0;
}

int perfScnDumpAll(void)
{
    int i, j;

    ALOGI("perfScnDumpAll");

    // check predefined scenario
    for (i = 0; i < nUserScnBase; i++) {
        if (ptScnList[i].scn_state == STATE_ON) {
            ALOGI("perfDumpAll (predefined) - type:%d", ptScnList[i].scn_type);
            for (j = 0; j < nClusterNum; j++) {
                ALOGD("            - cpu:%d, freq:%d", ptScnList[i].scn_core_min[j], ptScnList[i].scn_freq_min[j]);
            }
        }
    }
    // check user defined scenario
    for (i = nUserScnBase; i < nUserScnBase+REG_SCN_MAX; i++) {
        if (ptScnList[i].scn_state == STATE_ON) {
            ALOGI("perfDumpAll (user register)- handle:%d, pid:%d, tid:%d, comm:%s", ptScnList[i].handle_idx, ptScnList[i].pid, ptScnList[i].tid, ptScnList[i].comm);
            for (j = 0; j < nClusterNum; j++) {
                ALOGD("            - cpu:%d, freq:%d", ptScnList[i].scn_core_min[j], ptScnList[i].scn_freq_min[j]);
            }
        }
    }
    // check white list scenario
    for (i = SCN_APP_RUN_BASE; i < SCN_APP_RUN_BASE+nPackNum; i++) {
        if (ptScnList[i].scn_state == STATE_ON) {
            ALOGI("perfDumpAll (app list)- type:%d, pack_name:%s, act_name:%s", ptScnList[i].scn_type, ptScnList[i].pack_name, ptScnList[i].act_name);
            for (j = 0; j < nClusterNum; j++) {
                ALOGD("            - cpu:%d, freq:%d", ptScnList[i].scn_core_min[j], ptScnList[i].scn_freq_min[j]);
            }
        }
    }
    return 0;
}


/*
    reset_all == 1 => reset all
    reset_all == 0 => reset resource only
 */
void resetScenario(int handle, int reset_all)
{
    int i;
    if (reset_all) {
        ptScnList[handle].pack_name[0]      = '\0';
        ptScnList[handle].handle_idx        = -1;
        ptScnList[handle].scn_type          = -1;
        ptScnList[handle].scn_state         = STATE_OFF;
        ptScnList[handle].scn_valid         = 1; // default valid
        ptScnList[handle].pid               = ptScnList[handle].tid = -1;
        strncpy(ptScnList[handle].comm, "   ", COMM_NAME_SIZE);
        ptScnList[handle].lock_rsc_size = 0;
        ptScnList[handle].lock_rsc_list = NULL;
        ptScnList[handle].lock_duration = 0;
    }
    ptScnList[handle].scn_core_total    = 0;
    ptScnList[handle].scn_gpu_freq      = GPU_FREQ_MIN_RESET;
    ptScnList[handle].scn_gpu_freq_max  = GPU_FREQ_MAX_RESET;
    ptScnList[handle].screen_off_action = MTKPOWER_SCREEN_OFF_DISABLE;

    for (i=0; i<nClusterNum; i++) {
        ptScnList[handle].scn_core_min[i] = CPU_CORE_MIN_RESET;
        ptScnList[handle].scn_core_max[i] = CPU_CORE_MAX_RESET;
        ptScnList[handle].scn_freq_min[i] = CPU_FREQ_MIN_RESET;
        ptScnList[handle].scn_freq_max[i] = CPU_FREQ_MAX_RESET;
        ptScnList[handle].scn_freq_hard_min[i] = CPU_FREQ_MIN_RESET;
        ptScnList[handle].scn_freq_hard_max[i] = CPU_FREQ_MAX_RESET;
    }

    for (i = 0; i < FIELD_SIZE; i++) {
        ptScnList[handle].scn_param[i] = tConTable[i].resetVal;
    }

    for (i = 0; i < gRscCtlTblLen; i++) {
        if (reset_all)
             ptScnList[handle].scn_prev_rsc[i] = gRscCtlTbl[i].resetVal;
        else // backup setting
             ptScnList[handle].scn_prev_rsc[i] = ptScnList[handle].scn_rsc[i];

        ptScnList[handle].scn_rsc[i] = gRscCtlTbl[i].resetVal;
    }
}

int cmdSetting(int icmd, char *scmd, tScnNode *scenario, int param_1)
{
    int i = 0, ret = 0;

    if ((icmd < 0) && !scmd) {
        ALOGE("cmdSetting - scmd is NULL");
        return -1;
    }

    if (((icmd == -1) && !strcmp(scmd, "PERF_RES_GPU_FREQ_MIN")) ||
            icmd == PERF_RES_GPU_FREQ_MIN) {
        scenario->scn_gpu_freq =
            (param_1 >= 0 && param_1 <= nGpuHighestFreqLevel) ? param_1 : nGpuHighestFreqLevel;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_GPU_FREQ_MAX")) ||
            icmd == PERF_RES_GPU_FREQ_MAX) {
        scenario->scn_gpu_freq_max =
            (param_1 >= 0 && param_1 <= nGpuHighestFreqLevel) ? param_1 : nGpuHighestFreqLevel;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUCORE_MIN_CLUSTER_0")) ||
            icmd == PERF_RES_CPUCORE_MIN_CLUSTER_0) {
        scenario->scn_core_min[0] =
            (param_1 <= ptClusterTbl[0].cpuNum) ?
            ((param_1 < 0) ? 0 : param_1) : ptClusterTbl[0].cpuNum;

        scenario->scn_core_total = 0;
        for (i=0; i<nClusterNum; i++) {
            if (scenario->scn_core_min[i] >= 0 && scenario->scn_core_min[i] <= ptClusterTbl[i].cpuNum)
                scenario->scn_core_total += scenario->scn_core_min[i];
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUCORE_MIN_CLUSTER_1")) ||
            icmd == PERF_RES_CPUCORE_MIN_CLUSTER_1) {
        if(nClusterNum > 1) {
            scenario->scn_core_min[1] =
                (param_1 <= ptClusterTbl[1].cpuNum) ?
                ((param_1 < 0) ? 0 : param_1) : ptClusterTbl[1].cpuNum;

            scenario->scn_core_total = 0;
            for (i=0; i<nClusterNum; i++) {
                if (scenario->scn_core_min[i] >= 0 && scenario->scn_core_min[i] <= ptClusterTbl[i].cpuNum)
                    scenario->scn_core_total += scenario->scn_core_min[i];
            }
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUCORE_MAX_CLUSTER_0")) ||
            icmd == PERF_RES_CPUCORE_MAX_CLUSTER_0) {
        scenario->scn_core_max[0] = (param_1 >= 0) ?
            param_1 : ptClusterTbl[0].cpuNum;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUCORE_MAX_CLUSTER_1")) ||
            icmd == PERF_RES_CPUCORE_MAX_CLUSTER_1) {
        if(nClusterNum > 1) {
            scenario->scn_core_max[1] = (param_1 >= 0) ?
                param_1 : ptClusterTbl[1].cpuNum;
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MIN_CLUSTER_0")) ||
            icmd == PERF_RES_CPUFREQ_MIN_CLUSTER_0) {
        scenario->scn_freq_min[0] =
            (param_1 >= ptClusterTbl[0].freqMax) ? ptClusterTbl[0].freqMax : param_1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MIN_CLUSTER_1")) ||
            icmd == PERF_RES_CPUFREQ_MIN_CLUSTER_1) {
        if(nClusterNum > 1) {
            scenario->scn_freq_min[1] =
                (param_1 >= ptClusterTbl[1].freqMax) ? ptClusterTbl[1].freqMax : param_1;
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MAX_CLUSTER_0")) ||
            icmd == PERF_RES_CPUFREQ_MAX_CLUSTER_0) {
        scenario->scn_freq_max[0] =
            (param_1 >= ptClusterTbl[0].freqMax) ? ptClusterTbl[0].freqMax : param_1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MAX_CLUSTER_1")) ||
            icmd == PERF_RES_CPUFREQ_MAX_CLUSTER_1) {
        if(nClusterNum > 1) {
            scenario->scn_freq_max[1] =
                (param_1 >= ptClusterTbl[1].freqMax) ? ptClusterTbl[1].freqMax : param_1;
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_PERF_MODE")) ||
            icmd == PERF_RES_CPUFREQ_PERF_MODE) {
        if(param_1 != 1) // param_1 must be 1
            return 0;

        for (i = 0; i < nClusterNum; i++) {
            //ALOGI("cmdSetting - cmd:%x, i:%d, cpu:%d, freq:%d", icmd, i, ptClusterTbl[i].cpuNum, ptClusterTbl[i].freqMax);
            scenario->scn_core_min[i] = ptClusterTbl[i].cpuNum;
            scenario->scn_core_total += scenario->scn_core_min[i];
            scenario->scn_freq_min[i] = ptClusterTbl[i].freqMax;
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWERHAL_SCREEN_OFF_STATE")) ||
            icmd == PERF_RES_POWERHAL_SCREEN_OFF_STATE) {
            scenario->screen_off_action = param_1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWERHAL_SPORTS_MODE")) ||
            icmd == PERF_RES_POWERHAL_SPORTS_MODE) {
        if(param_1 == 1) {
            smart_reset(scenario->pack_name, 0);
            if(smart_check_pack_existed(APK_BENCHMARK, scenario->pack_name, foreground_uid, last_from_uid) == -1) {
                smart_add_benchmark();
            }
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWERHAL_WHITELIST_APP_LAUNCH_TIME_COLD")) ||
            icmd == PERF_RES_POWERHAL_WHITELIST_APP_LAUNCH_TIME_COLD) {
        scenario->launch_time_cold = (param_1 >= 0) ? param_1 : -1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWERHAL_WHITELIST_APP_LAUNCH_TIME_WARM")) ||
            icmd == PERF_RES_POWERHAL_WHITELIST_APP_LAUNCH_TIME_WARM) {
        scenario->launch_time_warm = (param_1 >= 0) ? param_1 : -1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWER_HINT_EXT_HINT")) ||
            icmd == PERF_RES_POWER_HINT_EXT_HINT) {
            scenario->ext_hint = (param_1 > 0) ? param_1 : 0;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWER_HINT_EXT_HINT_HOLD_TIME")) ||
            icmd == PERF_RES_POWER_HINT_EXT_HINT) {
            scenario->ext_hint_hold_time = (param_1 > 0) ? param_1 : 0;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_POWER_HINT_HOLD_TIME")) ||
            icmd == PERF_RES_POWER_HINT_HOLD_TIME) {
            scenario->hint_hold_time = (param_1 > 0) ? param_1 : 0;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MIN_HL_CLUSTER_0")) ||
            icmd == PERF_RES_CPUFREQ_MIN_HL_CLUSTER_0) {
        scenario->scn_freq_hard_min[0] =
            (param_1 >= ptClusterTbl[0].freqMax) ? ptClusterTbl[0].freqMax : param_1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MIN_HL_CLUSTER_1")) ||
            icmd == PERF_RES_CPUFREQ_MIN_HL_CLUSTER_1) {
        if(nClusterNum > 1) {
            scenario->scn_freq_hard_min[1] =
                (param_1 >= ptClusterTbl[1].freqMax) ? ptClusterTbl[1].freqMax : param_1;
        }
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MAX_HL_CLUSTER_0")) ||
            icmd == PERF_RES_CPUFREQ_MAX_HL_CLUSTER_0) {
        scenario->scn_freq_hard_max[0] =
            (param_1 >= ptClusterTbl[0].freqMax) ? ptClusterTbl[0].freqMax : param_1;
    }
    else if (((icmd == -1) && !strcmp(scmd, "PERF_RES_CPUFREQ_MAX_HL_CLUSTER_1")) ||
            icmd == PERF_RES_CPUFREQ_MAX_HL_CLUSTER_1) {
        if(nClusterNum > 1) {
            scenario->scn_freq_hard_max[1] =
                (param_1 >= ptClusterTbl[1].freqMax) ? ptClusterTbl[1].freqMax : param_1;
        }
    }
    else {
        ret = -1;
    }

    if (ret == 0)
        return ret;

    for(int idx = 0; idx < FIELD_SIZE; idx++) {
        if(((icmd == -1) && !strcmp(scmd, tConTable[idx].cmdName.c_str())) ||
                icmd == tConTable[idx].cmdID) {
            if(param_1 >= tConTable[idx].minVal && param_1 <= tConTable[idx].maxVal)
                scenario->scn_param[idx] = param_1;
            else
                ALOGE("input parameter exceed reasonable range %d %d %d %d", tConTable[idx].cmdID, param_1, tConTable[idx].maxVal,
                        tConTable[idx].minVal);

            ret = 0;
        }
    }

    for(int idx = 0; idx < gRscCtlTblLen; idx++) {
        if(((icmd == -1) && !strcmp(scmd, RscCfgTbl[idx].cmdName.c_str())) ||
                icmd == RscCfgTbl[idx].cmdID) {
            ALOGD("cmdSetting RscCfgTbl cmdID:%x param_1:%d ,maxVal:%d ,minVal:%d, force:%d", RscCfgTbl[idx].cmdID, param_1, RscCfgTbl[idx].maxVal,
                  RscCfgTbl[idx].minVal, RscCfgTbl[idx].force_update);
            if(param_1 >= RscCfgTbl[idx].minVal && param_1 <= RscCfgTbl[idx].maxVal) {
                scenario->scn_rsc[idx] = param_1;
            }
            else
                ALOGE("input parameter exceed reasonable range %d %d %d %d", RscCfgTbl[idx].cmdID, param_1, RscCfgTbl[idx].maxVal,
                        RscCfgTbl[idx].minVal);

            if(RscCfgTbl[idx].force_update == 1)
                ret = 1;
            else
                ret = 0;
        }
    }

    if (ret == -1)
        ALOGI("cmdSetting - unknown cmd:%x, scmd:%s ,param_1:%d", icmd, scmd, param_1);

    return ret;
}

void Scn_cmdSetting(char *cmd, int scn, int param_1)
{
    cmdSetting(-1, cmd, &ptScnList[scn], param_1);
}

int LegacyCmdSetting(int icmd, tScnNode *scenario, int param_1, int param_2, int param_3, int param_4)
{
    int i = 0, ret = 0;

    if (icmd < 0) {
        ALOGE("LegacyCmdSetting - icmd is not valid");
        return -1;
    }

    if (icmd == (int)MtkPowerCmd::CMD_SET_GPU_FREQ_MIN) {
        scenario->scn_gpu_freq =
            (param_1 >= 0 && param_1 <= nGpuHighestFreqLevel) ? param_1 : nGpuHighestFreqLevel;
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_GPU_FREQ_MAX) {
        scenario->scn_gpu_freq_max =
            (param_1 >= 0 && param_1 <= nGpuHighestFreqLevel) ? param_1 : nGpuHighestFreqLevel;
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN) {
        if (param_1 >= nClusterNum || param_1 < 0)
            return -1;
        scenario->scn_core_min[param_1] =
            (param_2 <= ptClusterTbl[param_1].cpuNum) ?
            ((param_2 < 0) ? 0 : param_2) : ptClusterTbl[param_1].cpuNum;

        scenario->scn_core_total = 0;
        for (i=0; i<nClusterNum; i++) {
            if (scenario->scn_core_min[i] >= 0 && scenario->scn_core_min[i] <= ptClusterTbl[i].cpuNum)
                scenario->scn_core_total += scenario->scn_core_min[i];
        }
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX) {
        if (param_1 >= nClusterNum || param_1 < 0)
            return -1;
        scenario->scn_core_max[param_1] = (param_2 >= 0) ?
            param_2 : ptClusterTbl[param_1].cpuNum;
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN) {
        if (param_1 >= nClusterNum || param_1 < 0)
            return -1;
        scenario->scn_freq_min[param_1] =
            (param_2 >= ptClusterTbl[param_1].freqMax) ? ptClusterTbl[param_1].freqMax : param_2;
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX) {
        if (param_1 >= nClusterNum || param_1 < 0)
            return -1;
        scenario->scn_freq_max[param_1] =
            (param_2 >= ptClusterTbl[param_1].freqMax) ? ptClusterTbl[param_1].freqMax : param_2;
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_CPU_PERF_MODE) {
        if(param_1 != 1) // param_1 must be 1
            return 0;

        for (i=0; i<nClusterNum; i++) {
            //ALOGI("cmdSetting - cmd:%d, i:%d, cpu:%d, freq:%d", icmd, i, ptClusterTbl[i].cpuNum, ptClusterTbl[i].freqMax);
            scenario->scn_core_min[i] = ptClusterTbl[i].cpuNum;
            scenario->scn_core_total += scenario->scn_core_min[i];
            scenario->scn_freq_min[i] = ptClusterTbl[i].freqMax;
        }
    }
    else if (icmd == (int)MtkPowerCmd::CMD_SET_SCREEN_OFF_STATE) {
            scenario->screen_off_action = param_1;
    }
    else if (icmd == (int)MtkPowerCmdInternal::CMD_SET_APP_CRASH) {
        scenario->scn_app_crash = (param_1 > 0) ? param_1 : -1;
        ALOGI("CMD_SET_APP_CRASH: %d", scenario->scn_app_crash);
    } else {
        ret = -1;
    }

    for(int idx = 0; idx < FIELD_SIZE; idx++) {
        if(icmd == tConTable[idx].legacyCmdID) {
            if(param_1 >= tConTable[idx].minVal && param_1 <= tConTable[idx].maxVal)
                scenario->scn_param[idx] = param_1;
            else
                ALOGI("input parameter exceed reasonable range %d %d %d %d", tConTable[idx].legacyCmdID, param_1, tConTable[idx].maxVal,
                        tConTable[idx].minVal);
            ret = 0;
        }
    }

    if (ret == -1)
        ALOGI("LegacyCmdSetting - unknown cmd:%d, p1:%d, p2:%d, p3:%d, p4:%d", icmd, param_1, param_2, param_3, param_4);

    return ret;

}

#if 0
int initPowerMode(void)
{
    return 0;
}
#endif

/*
check which mode need to change (sport and normal)

*/
void switchNormalAndSportMode(int mode) {

    for(int idx = 0; idx < FIELD_SIZE; idx++) {
        if(tConTable[idx].cmdName.length() == 0)
            continue;

        if(tConTable[idx].isValid == -1)
            continue;

        if(tConTable[idx].sportVal == CFG_TBL_INVALID_VALUE)
            continue;

        if(tConTable[idx].normalVal == tConTable[idx].sportVal)
            continue;

        ALOGV("[switchNormalAndSportMode] mode:%d, idx:%d, id:%x, default:%d, normal:%d, sports:%d, reset:%d, cur:%d",
            mode, idx, tConTable[idx].cmdID, tConTable[idx].defaultVal, tConTable[idx].normalVal,
            tConTable[idx].sportVal, tConTable[idx].resetVal, tConTable[idx].curVal);

        if(mode == PERF_MODE_SPORTS) {
            if(tConTable[idx].sportVal != CFG_TBL_INVALID_VALUE)
                tConTable[idx].defaultVal = tConTable[idx].sportVal;
        }
        else {
            if(tConTable[idx].normalVal != CFG_TBL_INVALID_VALUE)
                tConTable[idx].defaultVal = tConTable[idx].normalVal;
        }

        if(tConTable[idx].curVal != tConTable[idx].resetVal) // already set
            continue;

        ALOGV("[switchNormalAndSportMode] default switch to:%d", tConTable[idx].defaultVal);
        if(tConTable[idx].prefix.length() == 0)
            set_value(tConTable[idx].entry.c_str(), tConTable[idx].defaultVal);
        else {
            char inBuf[64];
            snprintf(inBuf, 64, "%s%d", tConTable[idx].prefix.c_str(), tConTable[idx].defaultVal);
            set_value(tConTable[idx].entry.c_str(), inBuf);
            ALOGD("[switchNormalAndSportMode] (less) %s set +prefix:%s;", tConTable[idx].entry.c_str(), inBuf);
        }
    }
}

int switchPowerMode(int mode) // call by perfNotifyUserStatus
{
//    int i;
    ALOGI("[switchPowerMode] mode:%d", mode);

    if(mode < 0 || mode > PERF_MODE_SPORTS)
        return 0;

    if(mode == PERF_MODE_SPORTS) {
        switchSportsMode(SPORTS_USER_NOTIFY, 1);
    }
    else {
        switchSportsMode(SPORTS_USER_NOTIFY, 0);
    }

    return 0;
}

int switchSportsMode(int reason, int bEnable) // for smart detection
{
    int newPowerMode;

    ALOGD("[switchSportsMode] reason:%d, enable:%d, turbo:%d", reason, bEnable, gtDrvInfo.turbo);

    if(reason == SPORTS_BENCHMARK) // smart detect
        nCurrBenchmarkMode = bEnable;
    else if(reason == SPORTS_USER_NOTIFY) // user notify
        nUserNotifyBenchmark = bEnable;
    else
        return 0;

    if(nCurrBenchmarkMode || nUserNotifyBenchmark)
        newPowerMode = PERF_MODE_SPORTS;
    else
        newPowerMode = PERF_MODE_NORMAL;

    if(nCurrPowerMode == newPowerMode) // does not change
        return 0; // do nothing

    nCurrPowerMode = newPowerMode;

    if(nCurrPowerMode == PERF_MODE_SPORTS) {
        switchNormalAndSportMode(PERF_MODE_SPORTS);
        perfScnEnable(MTKPOWER_HINT_SPORTS);
    }
    else {
        switchNormalAndSportMode(PERF_MODE_NORMAL);
        perfScnDisable(MTKPOWER_HINT_SPORTS); // only enable if app is running and it's benchmark mdoe
    }

    return 0;
}

int gameNotifyDisplayType(int type)
{
    ALOGI("gameNotifyDisplayType - type:%d", type);

    if(type == DISPLAY_TYPE_GAME ) {
        perfScnDisable(MTKPOWER_HINT_APP_TOUCH);
        if (nFbcSupport)
            fbcNotifyTouch(0);

        perfScnEnable(MTKPOWER_HINT_GAMING);

        ALOGE("foreground_uid %d last_from_uid %d",foreground_uid, last_from_uid);
        if(smart_check_pack_existed(APK_GAME, foreground_pack, foreground_uid, last_from_uid) == -1) {
            smart_add_game();
        }
    }
    else {
        if(type == DISPLAY_TYPE_NO_TOUCH_BOOST)
            perfScnDisable(MTKPOWER_HINT_APP_TOUCH);

        perfScnDisable(MTKPOWER_HINT_GAMING);
    }
    return 0;
}

int getForegroundInfo(char **pPackName, int *pPid, int *pUid)
{
    if(pPackName != NULL)
        *pPackName = foreground_pack;

    if(pPid != NULL)
        *pPid = foreground_pid;

    if(pUid != NULL)
        *pUid = foreground_uid;

    return 0;
}

static int allocNewHandleNum(void)
{
    user_handle_now = (user_handle_now + 1) % HANDLE_RAND_MAX;
    if (user_handle_now == 0)
        user_handle_now++;

    return user_handle_now;
}

static int findHandleToIndex(int handle)
{
    int i, idx = -1;

    for (i = nUserScnBase; i < nUserScnBase + REG_SCN_MAX; i++) {
        if((ptScnList[i].scn_type == SCN_USER_HINT || ptScnList[i].scn_type == SCN_CUS_POWER_HINT || \
            ptScnList[i].scn_type == SCN_PERF_LOCK_HINT) && (handle == ptScnList[i].handle_idx)) {
            idx = i;
            ALOGD("findHandleIndex find match handle- handle:%d idx:%d", handle, idx);
            break;
        }
    }
    return idx;
}

extern "C"
int perfBoostEnable(int scenario)
{
    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    if(scenario == MTKPOWER_HINT_APP_TOUCH && nDisplayType == DISPLAY_TYPE_GAME)
        return 0;  // disable touch boost

    if (scenario < MTKPOWER_HINT_NUM)
        perfScnEnable(scenario);

    switch(scenario) {
    case MTKPOWER_HINT_GAMING:
        perfScnDisable(MTKPOWER_HINT_APP_TOUCH);
        nDisplayType = DISPLAY_TYPE_GAME;
        gameNotifyDisplayType(nDisplayType);
        break;

    case MTKPOWER_HINT_APP_TOUCH:
        if (nFbcSupport)
            fbcNotifyTouch(1);
        last_touch_time = systemTime();
        break;

    case MTKPOWER_HINT_SPORTS:
        switchSportsMode(SPORTS_USER_NOTIFY, 1);
        break;

    case MTKPOWER_HINT_WFD:
        /*ALOGI("[perfBoostEnable] FPSGO");*/
        break;

    default:
        break;
    }

    return 0;
}

extern "C"
int perfBoostDisable(int scenario)
{
    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    if (scenario < MTKPOWER_HINT_NUM)
        perfScnDisable(scenario);

    switch(scenario) {
    case MTKPOWER_HINT_GAMING:
        nDisplayType = DISPLAY_TYPE_OTHERS;
        gameNotifyDisplayType(nDisplayType);
        break;

    case MTKPOWER_HINT_APP_TOUCH:
        if (nFbcSupport)
           fbcNotifyTouch(0);
        break;

    case MTKPOWER_HINT_SPORTS:
        switchSportsMode(SPORTS_USER_NOTIFY, 0);
        break;

    case MTKPOWER_HINT_WFD:
        /*ALOGI("[perfBoostDisable] FPSGO");*/
        break;

    default:
        break;
    }

    return 0;
}

extern "C"
int perfNotifyAppState(const char *packName, const char *actName, int state, int pid, int uid)
{
    int i,ret = 0;
    int Act_Match = 0,Common_index=-1;
    tScnNode *pPackList = NULL;
    static int notifyThermalPid = 1; /* 0: not existed */
    static int notifyGxPid = 1; /*0: not existed */

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    //if (!smart_table_init_flag())
    //    if(!smart_table_init()) return 0;

    ALOGV("[perfNotifyAppState] pack:%s, com:%s, state:%d, pid:%d, uid:%d", packName, actName, state, pid, uid);

    if(state == STATE_DEAD) {
        for(i=nUserScnBase; i<nUserScnBase+REG_SCN_MAX; i++) {
            if(pid == ptScnList[i].pid) {
                ALOGI("[perfNotifyAppState] nPackNum:%d, pack:%s, com:%s, state:%d, pid:%d",
                        nPackNum, packName, actName, state, pid);
                perfScnDisable(i);
                resetScenario(i, 1);
            }
        }
    }
    if(state == STATE_DEAD && pid != foreground_pid)
        return 0;

    pPackList = ptScnList + SCN_APP_RUN_BASE;

    if(!packName) {
        return 0;
    }

    /* foreground change: update pack name */
    if(state == STATE_RESUMED && strcmp(foreground_pack, packName)) {
        strncpy(foreground_pack, packName, PACK_NAME_MAX-1); // update pack name
        foreground_pid = pid;
        foreground_uid = uid;
        //foregroundName.setTo(packName);
        //mFpsInfo->setForegroundInfo(foreground_pid, foregroundName); // notify fpsInfo

        ALOGI("[perfNotifyAppState] foreground:%s, pid:%d, uid:%d", foreground_pack, foreground_pid, uid);
        if(smart_check_pack_existed(APK_BENCHMARK, foreground_pack, foreground_uid, last_from_uid) != -1) {
            ALOGI("[perfNotifyAppState] match !!!");
            switchSportsMode(SPORTS_BENCHMARK, 1);
        }
        else {
            smart_reset(foreground_pack, foreground_pid); // start detection after get request
            switchSportsMode(SPORTS_BENCHMARK, 0);
        }

        // notify other modules
        if(notifyThermalPid && 0 == set_value(PATH_THERMAL_PID, pid)) // notify thermal
            notifyThermalPid = 0;
        if(notifyGxPid && 0 == set_value(PATH_GX_PID, pid)) // notify thermal
            notifyGxPid = 0;
        notifyForegroundApp(packName, uid); // notify UX module
    }

    /* foreground change: handle white list */
    if(state == STATE_RESUMED && strcmp(foreground_act, actName)) {
        strncpy(foreground_act, actName, CLASS_NAME_MAX-1); // update pack name

        fg_launch_time_cold = fg_launch_time_warm = 0;

        // check white list
        Act_Match = 0;
        Common_index = -1;
        for(i=0; i<nPackNum; i++) {
            /* activity rule */
            if(strncmp(pPackList[i].act_name, "Common", 6)!=0){
                if(!strcmp(pPackList[i].pack_name, packName) && !strcmp(pPackList[i].act_name, actName)){
                    Act_Match = 1;

                    ALOGI("[perfNotifyAppState] launch cold:%d, warm:%d",
                        pPackList[i].launch_time_cold, pPackList[i].launch_time_warm);
                    fg_launch_time_warm = pPackList[i].launch_time_warm;

                    //if(pPackList[i].boost_timeout >= 0) {
                    if(fg_launch_time_cold > 0 || fg_launch_time_warm > 0) {
                       ret = (SCN_APP_RUN_BASE + i);
                       ALOGI("[perfNotifyAppState] Activity policy match launch time=%d, %d!!!", fg_launch_time_cold, fg_launch_time_warm);
                    } else {
                       ALOGI("[perfNotifyAppState] Activity policy match !!!");
                    }

                    perfScnEnable(SCN_APP_RUN_BASE + i);
                }
                else{
                    perfScnDisable(SCN_APP_RUN_BASE + i);
                }
            }
            else{
                if(!strcmp(pPackList[i].pack_name, packName)){
                    Common_index = i;
                }
                else{
                    perfScnDisable(SCN_APP_RUN_BASE + i);
                }
            }
        }

        /* common rule */
        if(Act_Match !=1 && Common_index != -1){

            ALOGI("[perfNotifyAppState] launch cold:%d, warm:%d",
                pPackList[Common_index].launch_time_cold, pPackList[Common_index].launch_time_warm);
            fg_launch_time_cold = pPackList[Common_index].launch_time_cold;
            fg_launch_time_warm = pPackList[Common_index].launch_time_warm;

            if(fg_launch_time_cold > 0 || fg_launch_time_warm > 0) {
                  ret = (SCN_APP_RUN_BASE + Common_index);
                  ALOGI("[perfNotifyAppState] Package common policy match launch time=%d, %d!!!",
                                                    fg_launch_time_cold, fg_launch_time_warm);
            } else {
                  ALOGI("[perfNotifyAppState] Package common policy match !!!");
            }

            perfScnEnable(SCN_APP_RUN_BASE + Common_index);
        }
        else
        if(Act_Match ==1 && Common_index != -1){
            perfScnDisable(SCN_APP_RUN_BASE + Common_index);
        }
    }

    return ret;
}

extern "C"
int perfUserScnEnable(int handle)
{
    int idx = -1;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    //ALOGI("perfUserScnEnable - handle:%d", handle);

    idx = findHandleToIndex(handle);
    if(idx < nUserScnBase || idx >= nUserScnBase + REG_SCN_MAX || idx < 0)
        return -1;

    if(ptScnList[idx].scn_type == SCN_USER_HINT)
        ALOGI("perfUserScnEnable - handle:%d", handle);
    else if (ptScnList[idx].scn_type == SCN_CUS_POWER_HINT)
        ALOGI("perfCusUserScnEnable - handle:%d", handle);
    else if (ptScnList[idx].scn_type == SCN_PERF_LOCK_HINT)
        ALOGE("perfUserScnEnable - wrong hint type:%d", handle);

    perfScnEnable(idx);
    return 0;
}

extern "C"
int perfUserScnDisable(int handle)
{
    int idx = -1;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    ALOGI("perfUserScnDisable - handle:%d", handle);

    idx = findHandleToIndex(handle);
    if(idx < nUserScnBase || idx >= nUserScnBase + REG_SCN_MAX)
        return -1;

    perfScnDisable(idx);
    return 0;
}

extern "C"
int perfUserScnResetAll(void)
{
    int i;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    //ALOGI("perfUserScnResetAll");

    for(i=nUserScnBase; i<nUserScnBase+REG_SCN_MAX; i++) {
        if(ptScnList[i].scn_type != -1 && ptScnList[i].scn_state == STATE_ON)
            perfScnDisable(i);
        resetScenario(i, 1);
    }
    return 0;
}

extern "C"
int perfUserScnDisableAll(void)
{
    int i;
    struct stat stat_buf;
    int exist;
    char proc_path[128];

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    //ALOGI("perfUserScnDisableAll");

    for(i=0; i<SCN_APP_RUN_BASE + nPackNum; i++) {
        if(ptScnList[i].scn_type != -1) {
            if(ptScnList[i].scn_state == STATE_ON && ptScnList[i].screen_off_action != MTKPOWER_SCREEN_OFF_ENABLE) {
                ALOGI("perfUserScnDisableAll, h:%d, s:%d, a:%d", i, ptScnList[i].scn_state, ptScnList[i].screen_off_action);
                perfScnDisable(i);
                if(ptScnList[i].screen_off_action == MTKPOWER_SCREEN_OFF_WAIT_RESTORE)
                    ptScnList[i].scn_state = STATE_WAIT_RESTORE;
            }
            // kill handle if process is dead
            if(i >= nUserScnBase && i < SCN_APP_RUN_BASE) {
              if(ptScnList[i].scn_type == SCN_USER_HINT || ptScnList[i].scn_type == SCN_PERF_LOCK_HINT) {
                sprintf(proc_path, "/proc/%d", ptScnList[i].pid);
                exist = (0 == stat(proc_path, &stat_buf)) ? 1 : 0;
                if(!exist) {
                    ALOGI("perfUserScnDisableAll, hdl:%d, pid:%d, comm:%s, is not existed", i, ptScnList[i].pid, ptScnList[i].comm);
                    perfScnDisable(i);
                    resetScenario(i, 1);
                }
              }
            }
        }
    }

    return 0;
}

extern "C"
int perfUserScnRestoreAll(void)
{
    int i;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    //ALOGI("perfUserScnRestoreAll");

    for(i=0; i<SCN_APP_RUN_BASE + nPackNum; i++) {
        if(ptScnList[i].scn_type != -1 && ptScnList[i].scn_state == STATE_WAIT_RESTORE) {
            ALOGI("perfUserScnRestoreAll, h:%d, s:%d, a:%d", i, ptScnList[i].scn_state, ptScnList[i].screen_off_action);
            perfScnEnable(i);
        }
    }
    return 0;
}

extern "C"
int perfUserScnCheckAll(void)
{
    int i;
    struct stat stat_buf;
    int exist;
    char proc_path[128];

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    ALOGI("perfUserScnCheckAll");

    for(i=0; i<SCN_APP_RUN_BASE + nPackNum; i++) {
        if(ptScnList[i].scn_type != -1) {
            // kill handle if process is dead
            if(i >= nUserScnBase && i < SCN_APP_RUN_BASE) {
              if(ptScnList[i].scn_type == SCN_USER_HINT || ptScnList[i].scn_type == SCN_PERF_LOCK_HINT) {
                sprintf(proc_path, "/proc/%d", ptScnList[i].pid);
                exist = (0 == stat(proc_path, &stat_buf)) ? 1 : 0;
                if(!exist) {
                    ALOGI("perfUserScnDisableAll, hdl:%d, pid:%d, comm:%s, is not existed", i, ptScnList[i].pid, ptScnList[i].comm);
                    perfScnDisable(i);
                    resetScenario(i, 1);
                }
              }
            }
        }
    }
    return 0;
}

extern "C"
int perfSetSysInfo(int type, const char *data)
{
    int ret = 0;
    char *pack_name = NULL, *str = NULL, *saveptr = NULL, buf[128];
    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    if (!data) {
        return -1;
    }

    ALOGI("perfSetSysInfo type:%d, %s", type, data);

    switch(type) {

    case SETSYS_MANAGEMENT_PREDICT:
        update_packet(data);
        notifyCmdMode(CMD_PREDICT_MODE);
        break;

    case SETSYS_SPORTS_APK:
        if(smart_check_pack_existed(APK_BENCHMARK, data, -1, -1) == -1) { // fake "uid" and "from uid"
            smart_add_specific_benchmark(data);
        }
        break;

    case SETSYS_FOREGROUND_SPORTS:
        if(smart_check_pack_existed(APK_BENCHMARK, foreground_pack, -1, -1) == -1) { // fake "uid" and "from uid"
            smart_add_specific_benchmark(foreground_pack);
            switchSportsMode(SPORTS_BENCHMARK, 1);
        }
        break;

    case SETSYS_MANAGEMENT_PERIODIC:
        update_packet(data);
        notifyCmdMode(CMD_AGGRESSIVE_MODE);
        break;

    case SETSYS_INTERNET_STATUS:
        break;

    case SETSYS_NETD_STATUS:
        ret = property_set("persist.vendor.powerhal.PERF_RES_NET_NETD_BOOST_UID", data);
        ALOGI("SETSYS_NETD_STATUS:%s ret:%d", data, ret);
        break;

    case SETSYS_PREDICT_INFO:
        strncpy(buf, data, sizeof(buf)-1);
        str = strtok_r(buf, " ", &saveptr);
        if(str != NULL) {
            int uid = -1;
            pack_name = str;
            if((str = strtok_r(NULL, " ", &saveptr)) != NULL)
                uid = atoi(str);
            ALOGD("SETSYS_PREDICT_INFO, pack:%s, uid:%d", pack_name, uid);
            notify_APPState(pack_name, uid);
        }
        break;

    case SETSYS_NETD_DUPLICATE_PACKET_LINK:
        if(strcmp(data, "DELETE_ALL") == 0) {
            if(deleteAllDupPackerLink() < 0) {
                ret = -1;
            }
        } else if(strncmp(data, "MULTI", 5) == 0) {
            if(SetDupPacketMultiLink(data) < 0) {
                ret = -1;
            }
        } else {
            if(SetOnePacketLink(data) < 0) {
                ret = -1;
            }
        }
        break;

    default:
        ALOGI("perfSetSysInfo unknown type");
        break;
    }

    return ret;
}

extern "C"
int perfDumpAll(void)
{
    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    return perfScnDumpAll();
}

extern "C"
int perfUserGetCapability(int cmd, int id)
{
    int value = -1, idx = -1;
    nsecs_t now = 0;
    int interval = -1;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    ALOGV("perfUserGetCapability - cmd:%d, id:%d", cmd, id);

    //if (cmd != MTKPOWER_CMD_GET_DEBUG_SET_LVL && (id >= nClusterNum || id < 0))
    //    return value;

    switch(cmd) {
    case MTKPOWER_CMD_GET_CLUSTER_NUM:
        value = nClusterNum;
        break;

    case MTKPOWER_CMD_GET_CLUSTER_CPU_NUM:
        if(id >= 0 && id <= nClusterNum)
            value = ptClusterTbl[id].cpuNum;
        break;

    case MTKPOWER_CMD_GET_CLUSTER_CPU_FREQ_MIN:
        if(id >= 0 && id <= nClusterNum)
            value = ptClusterTbl[id].pFreqTbl[0];
        break;

    case MTKPOWER_CMD_GET_CLUSTER_CPU_FREQ_MAX:
        if(id >= 0 && id <= nClusterNum)
           value = ptClusterTbl[id].freqMax;
        break;

    case MTKPOWER_CMD_GET_FOREGROUND_PID:
        value = foreground_pid;
        break;

    case MTKPOWER_CMD_GET_FOREGROUND_TYPE:
        if(smart_check_pack_existed(APK_GAME, foreground_pack, foreground_uid, last_from_uid) != -1)
            value = APK_GAME;
        break;

    /*case (int)MtkQueryCmd::CMD_GET_BOOST_TIMEOUT:
            value = (pboost_timeout * 1000);
        break;*/

    case MTKPOWER_CMD_GET_POWER_HINT_HOLD_TIME:
        if(checkSuccess(id))
            value = (ptScnList[id].hint_hold_time * 1000);
        break;

    case MTKPOWER_CMD_GET_POWER_HINT_EXT_HINT:
        if(checkSuccess(id))
            value = ptScnList[id].ext_hint;
        break;

    case MTKPOWER_CMD_GET_GPU_FREQ_COUNT:
        value = nGpuFreqCount;
        break;

    case MTKPOWER_CMD_GET_RILD_CAP:
        value = query_capability(id, "GLEN");
        break;

    case MTKPOWER_CMD_GET_TIME_TO_LAST_TOUCH:
        now = systemTime();
        interval = ns2ms(now - last_touch_time);
        if (interval > 100000 || interval < 0)
            value = 100000;
        else
            value = interval;
        ALOGD("perfUserGetCapability - now:%ld, value:%ld interval:%d",
            now, last_touch_time, interval);
        break;

    case MTKPOWER_CMD_GET_POWER_HINT_EXT_HINT_HOLD_TIME:
        if(checkSuccess(id))
            value = (ptScnList[id].ext_hint_hold_time * 1000);
        break;

    case MTKPOWER_CMD_GET_LAUNCH_TIME_COLD:
        value = fg_launch_time_cold * 1000;
        ALOGV("perfUserGetCapability - cmd:%d, id:%d, value:%d", cmd, id, value);
        break;

    case MTKPOWER_CMD_GET_LAUNCH_TIME_WARM:
        value = fg_launch_time_warm * 1000;
        ALOGV("perfUserGetCapability - cmd:%d, id:%d, value:%d", cmd, id, value);
        break;

    case MTKPOWER_CMD_GET_POWER_SCN_TYPE:
        idx = findHandleToIndex(id);
        if (idx != -1)
            value = ptScnList[idx].scn_type;
        else
            value = -1;
        break;

    case MTKPOWER_CMD_GET_CPU_TOPOLOGY:
        //value = ptClusterTbl[id].cpuNum;
        break;

    default:
        break;
    }

    ALOGD("perfUserGetCapability - cmd:%d, id:%d, value:%d", cmd, id, value);
    return value;
}

extern "C"
int perfCusUserRegScn(void)
{
    int i , handle = -1, add_for_cus_power_hint = 0;
    /*char filepath[64];*/

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    ALOGD("perfCusUserRegScn");

    for (i = nUserScnBase; i < nUserScnBase + REG_SCN_MAX; i++) {
        if(ptScnList[i].scn_type == -1) {
            handle = allocNewHandleNum();
            ALOGD("perfUserRegScn - handle:%d", handle);
            resetScenario(i, 1);
            ptScnList[i].handle_idx = handle;
            ptScnList[i].scn_type = SCN_CUS_POWER_HINT;
            ptScnList[i].scn_state = STATE_OFF;
            add_for_cus_power_hint = 1;
            break;
        }
    }
    return handle;
}

extern "C"
int perfUserRegScn(int pid, int tid)
{
    int i , handle = -1;
    char filepath[64] = "\0";

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    ALOGD("perfUserRegScn - pid:%d, tid:%d", pid, tid);

    for (i = nUserScnBase; i < nUserScnBase + REG_SCN_MAX; i++) {
        if(ptScnList[i].scn_type == -1) {
            handle = allocNewHandleNum();
            ALOGD("perfUserRegScn - handle:%d", handle);
            resetScenario(i, 1);
            ptScnList[i].scn_type = SCN_USER_HINT;
            ptScnList[i].handle_idx = handle;
            ptScnList[i].pid = pid;
            ptScnList[i].tid = tid;
            snprintf(filepath, sizeof(filepath), "/proc/%d/comm", pid);
            get_task_comm(filepath, ptScnList[i].comm);
            ptScnList[i].scn_state = STATE_OFF;
            break;
        }
    }

    return handle;
}

extern "C"
int perfUserRegScnConfig(int handle, int cmd, int param_1, int param_2, int param_3, int param_4)
{
    int idx = -1;
    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    ALOGD("perfUserRegScnConfig - handle:%d, cmd:%d, p1:%d, p2:%d, p3:%d, p4:%d", handle, cmd, param_1, param_2, param_3, param_4);

    idx = findHandleToIndex(handle);
    if((idx < nUserScnBase || idx >= nUserScnBase + REG_SCN_MAX) && param_4 != 1) // param_4 == 1 => modify pre-defined scn
        return -1;

    LegacyCmdSetting(cmd, &ptScnList[idx], param_1, param_2, param_3, param_4);

    return 0;
}

extern "C"
int perfUserUnregScn(int handle)
{
    int idx = -1;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;
    ALOGD("perfUserUnregScn - handle:%d", handle);

    idx = findHandleToIndex(handle);
    if(idx < nUserScnBase || idx >= nUserScnBase + REG_SCN_MAX || idx < 0)
        return -1;

    if(ptScnList[idx].scn_state == STATE_ON) {
        return -1;
    }

    resetScenario(idx, 1);
    return 0;
}

extern "C"
int perfSetUidInfo(int uid, int fromUid)
{
    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    if (foreground_uid == uid && last_from_uid == fromUid)
        return -1;

    ALOGI("perfSetUidInfo - uid:%d, fromUid:%d", uid, fromUid);
    foreground_uid = uid;
    last_from_uid = fromUid;

    return 0;
}

extern "C"
int perfLockAcq(int *list, int handle, int size, int pid, int tid, int duration)
{
    int i, idx = -1;
    int exist;
    int cmd, value, new_handle = 0;
    char filepath[64] = "\0";
    char proc_path[128];
    struct stat stat_buf;
    int hdl_enabled = 0, rsc_modified = 0, force_update = 0;

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    ALOGD("perfLockAcq handle- handle:%d size:%d duration:%d ", handle, size, duration);

    if (size % 2 == 0) {
        for (i=0; i<size; i+=2)
            ALOGD("data:0x%08x, %d", list[i], list[i+1]);
    }

    /*check user have permission otherwise duration must < 30s */
    sprintf(proc_path, "/proc/%d", pid);
    exist = (0 == stat(proc_path, &stat_buf)) ? 1 : 0;

    ALOGD("perfLockAcq handle:%d pid:%d duration:%d exist:%d", handle, pid, duration, exist);

    /* duration == 0 means infinite duration */
    if(!exist && (duration == 0 || duration > USER_DURATION_MAX)) {
        ALOGE("perf_lock_acq pid:%d don't have permission !! duration:%d ms > 30s", pid, duration);
        return -1;
    }

    if (handle <= 0)
        goto regiter_handle;

#if 0
    for (i = nUserScnBase; i < nUserScnBase + REG_SCN_MAX; i++) {
        if((ptScnList[i].scn_type == SCN_PERF_LOCK_HINT || ptScnList[i].scn_type == SCN_CUS_POWER_HINT)
            && (handle == ptScnList[i].handle_idx)) {
            resetScenario(i, 0); // reset resource only
            idx = i;
            hdl_enabled = 1;
            ptScnList[i].lock_duration = duration;
            ALOGI("perf_lock_acq find match handle- handle:%d idx:%d", handle, idx);
            break;
        }
    }
#else
    idx = findHandleToIndex(handle);
    if(idx != -1) {
        resetScenario(idx, 0); // reset resource only
        hdl_enabled = 1;
        ptScnList[i].lock_duration = duration;
        ALOGI("perf_lock_acq find match handle - handle:%d idx:%d", handle, idx);
    }
#endif

regiter_handle:
    ALOGV("perfLockAcq regiter_handle - handle:%d", handle);
    if (idx == -1) {
        for (i = nUserScnBase; i < nUserScnBase + REG_SCN_MAX; i++) {
            if(ptScnList[i].scn_type == -1) {
                new_handle = allocNewHandleNum();
                resetScenario(i, 1);
                ptScnList[i].scn_type = SCN_PERF_LOCK_HINT;
                ptScnList[i].handle_idx = new_handle;
                ptScnList[i].lock_duration = duration;
                ptScnList[i].pid = pid;
                ptScnList[i].tid = tid;
                snprintf(filepath, sizeof(filepath), "/proc/%d/comm", pid);
                get_task_comm(filepath, ptScnList[i].comm);
                ptScnList[i].scn_state = STATE_OFF;
                idx = i;
                ALOGD("perf_lock_acq register handle - handle:%d idx:%d", new_handle, idx);
                break;
            }
        }
    }
    else {
        new_handle = handle;
    }

    /* no more empty slot */
    if(idx == -1) {
        ALOGE("perf_lock_acq cannot alloc handle");
        perfScnDumpAll();
        return -1;
    }

    /* backup resource */
    if(hdl_enabled == 0) {
        if((ptScnList[idx].lock_rsc_list = (int*)malloc(sizeof(int) * size)) == NULL) {
            ALOGE("perf_lock_acq cannot alloc memory(%d)", __LINE__);
        } else {
            ptScnList[idx].lock_rsc_size = size;
            memcpy(ptScnList[idx].lock_rsc_list, list, sizeof(int) * size);
        }
    } else {
        if (ptScnList[idx].lock_rsc_size != size || \
            memcmp(ptScnList[idx].lock_rsc_list, list, sizeof(int) * size) != 0) {
            rsc_modified = 1;

            if(ptScnList[idx].lock_rsc_size != size) {
                ptScnList[idx].lock_rsc_size = size;
                if((ptScnList[idx].lock_rsc_list = (int*)realloc(ptScnList[idx].lock_rsc_list, sizeof(int) * size)) == NULL)
                    ALOGE("perf_lock_acq cannot alloc memory(%d)", __LINE__);
            }

            memcpy(ptScnList[idx].lock_rsc_list, list, sizeof(int) * size);
        }
    }

    for(i = 0; (2*i) < size; i++) {
        cmd = list[2*i];
        value = list[2*i+1];
        if(cmdSetting(cmd, NULL, &ptScnList[idx], value) == 1) {
            ALOGV("perfLockAcq force_update");
            force_update = 1;
        }
    }

    ALOGI("perfLockAcq hdl:%d pid:%d duration:%d pid_exist:%d => new_hdl:%d", handle, pid, duration, exist, new_handle);
    if(hdl_enabled == 1) {
        if(rsc_modified == 1 || force_update == 1)
            perfScnUpdate(idx, 1); // update setting
    }
    else {
        perfScnEnable(idx);
    }

    return new_handle;
}

extern "C"
int perfLockRel(int handle)
{
    int idx = -1;

    ALOGE("perfLockRel handle:%d", handle);

    Mutex::Autolock lock(sMutex);
    if (!nIsReady)
        if(!init()) return 0;

    idx = findHandleToIndex(handle);
    if(idx < nUserScnBase || idx >= nUserScnBase + REG_SCN_MAX)
        return -1;

    perfScnDisable(idx);

    if(ptScnList[idx].lock_rsc_list != NULL) {
        free(ptScnList[idx].lock_rsc_list);
        ptScnList[idx].lock_rsc_list = NULL;
        ptScnList[idx].lock_rsc_size = 0;
    }

    if(ptScnList[idx].scn_state == STATE_ON)
        return -1;

    resetScenario(idx, 1);

    return 0;
}

extern "C"
int perfLibpowerInit(void)
{
    Mutex::Autolock lock(sMutex);

    if (!nIsReady)
        if(!init()) return -1;

    return 0;
}

int loadRscTable(int power_on_init)
{
    int i;
    int ret;

    for(i = 0; i < gRscCtlTblLen; i++) {
        ALOGI("[loadRscTable] RscCfgTbl[%d] cmdName:%s cmdID:%x, param:%d, defaultVal:%d comp:%d maxVal:%d",
        i, RscCfgTbl[i].cmdName.c_str(), RscCfgTbl[i].cmdID, RscCfgTbl[i].defaultVal, RscCfgTbl[i].comp,
            RscCfgTbl[i].maxVal, RscCfgTbl[i].minVal);

        /*reset all Rsctable*/
        if (RscCfgTbl[i].init_func != NULL)
            ret = RscCfgTbl[i].init_func(power_on_init);

        // initial setting should be an invalid value
        if (RscCfgTbl[i].comp == SMALLEST)
            gRscCtlTbl[i].resetVal = RscCfgTbl[i].maxVal + 1;
        else if (RscCfgTbl[i].comp == BIGGEST)
            gRscCtlTbl[i].resetVal = RscCfgTbl[i].minVal - 1;
        else
            gRscCtlTbl[i].resetVal = RSC_TBL_INVALID_VALUE;

        gRscCtlTbl[i].curVal = gRscCtlTbl[i].resetVal;
        gRscCtlTbl[i].isValid = 1;
        gRscCtlTbl[i].log = 1;

        ALOGD("[loadRscTable] gRscCtlTbl[%d] resetVal:%d curVal:%d isValid:%d log:%d",
        i, gRscCtlTbl[i].resetVal, gRscCtlTbl[i].curVal, gRscCtlTbl[i].isValid, gRscCtlTbl[i].log);
    }

    return 1;
}

int whilelist_reload(void)
{
    int i;

    ALOGI("whilelist_reload");


    nPackNum = perfservice_xmlparse_reload_whitelist();

    if((ptScnList = (tScnNode*)realloc(ptScnList, sizeof(tScnNode) * (SCN_APP_RUN_BASE + nPackNum))) == NULL) {
        ALOGE("Can't allocate memory");
        return -1;
    }

    for (i = SCN_APP_RUN_BASE; i < SCN_APP_RUN_BASE + nPackNum; i++) {
        resetScenario(i, 1); // reset all scenarios
        ptScnList[i].scn_type = SCN_PACK_HINT;
    }

    updateScnListfromXML(ptScnList+SCN_APP_RUN_BASE);
    perfservice_xmlparse_freeList();

    return 0;
}

