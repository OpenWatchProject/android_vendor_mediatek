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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxCore.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Core
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "OMX_Component.h"
#include "OMX_Core.h"
#include <dlfcn.h>
#include <signal.h>
#include "osal_utils.h"
#include <cutils/properties.h>
#undef LOG_TAG
#define LOG_TAG "MtkOmxCore"

#define ENABLE_OMX_CPU_POWER_CONTROL 1
#define ENABLE_OMX_CPU_DVFS_CONTROL 0

//#ifdef MT6577
#include "MtkOmxCore.h"
#include <sys/ioctl.h>
#include <fcntl.h>
//#include "videocodec_kernel_driver.h"
#include "val_types_public.h"
#include "hal_types_public.h"
#if ENABLE_OMX_CPU_POWER_CONTROL
extern "C" {
    //VAL_RESULT_T eVideoVcodecSetCpuOppLimit (VAL_VCODEC_CPU_OPP_LIMIT_T *a_prCpuOppLimit);
}
#endif
#if ENABLE_OMX_CPU_DVFS_CONTROL
extern "C" {
    VAL_RESULT_T eHalDVFSDCMCtrl(HAL_POWER_T *a_pvParam);
}
#endif
//#endif

#include <sys/time.h>

int64_t getTickCountMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}


#define MAX_COMPONENT_INSTANCE 512 //32
#define MAX_COMPONENT_TYPE 64
#define MTK_OMXCORE_CFG_FILE "/vendor/etc/mtk_omx_core.cfg"
#define MAX_LINE_LEN 256
#define DELIMITERS " \t\r\n"
#define MTKOMXTAG "OMX.MTK."

#define OMX_COMP_STR_LEN 80
#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif
#define VDEC_UNUSED(x) (void)x

typedef struct _mtk_omx_comp_type
{
    OMX_S8 name[OMX_COMP_STR_LEN];
    OMX_S8 role[OMX_COMP_STR_LEN];
    OMX_S8 lib_path[OMX_COMP_STR_LEN];
    OMX_S8 max_inst[OMX_COMP_STR_LEN]; // max-concurrent-instance
} mtk_omx_comp_type;



typedef struct _mtk_omx_comp_instance_type
{
    OMX_PTR comp_module;
    OMX_PTR comp_handle;
    OMX_STRING comp_name;
} mtk_omx_comp_instance_type;

typedef struct _mtk_omx_comp_count_type
{
    OMX_STRING comp_name;
    OMX_S32 comp_count;
} mtk_omx_comp_count_type;

/*
typedef struct _mtk_omx_comp_instance_history
{
    OMX_U32 numOfGetHandle;
    OMX_U32 numOfFreeHandle;
    OMX_BOOL alreadyProfiled;
} mtk_omx_comp_instance_history;
*/

/**
// Note: gCoreComponents will look like this:
mtk_omx_comp_type gCoreComponents[] = {
    // { "Component name", "Component role", "lib path" }
    { "OMX.MTK.VIDEO.DECODER.H263"   , "video_decoder.h263",   "./libMtkOmxVdec.so"},
    { "OMX.MTK.VIDEO.DECODER.MPEG4", "video_decoder.mpeg4", "./libMtkOmxVdec.so"},
    { "OMX.MTK.VIDEO.DECODER.AVC"    , "video_decoder.avc",     "./libMtkOmxVdec.so"},
    { "OMX.MTK.VIDEO.DECODER.RV"    , "video_decoder.rv",       "./libMtkOmxVdec.so"},
    { "OMX.MTK.VIDEO.DECODER.VC1"    , "video_decoder.vc1",     "./libMtkOmxVdec.so"}
};
*/

//#ifdef MT6577
#if ENABLE_OMX_CPU_POWER_CONTROL
mtk_omx_core_global gCoreGlobal;
#endif
//#endif

pthread_mutex_t gInstancePoolLock;
bool gInitialized = false;
unsigned int gCoreComponentCounts = 0;
unsigned int gCoreReferenceCounts = 0;
mtk_omx_comp_type *gCoreComponents = NULL;
static unsigned int gOMXCoreLogProperty = 0;

mtk_omx_comp_instance_type gCoreCompInstance[MAX_COMPONENT_INSTANCE] =
{
    {NULL, NULL, NULL},
};

mtk_omx_comp_count_type *gCompInstanceCount = NULL;//[MAX_COMPONENT_TYPE];


//mtk_omx_comp_instance_history gCoreCompInstanceHistory[MAX_COMPONENT_INSTANCE] =
//{
//    {0, 0, OMX_FALSE},
//};


static bool ParseMtkCoreConfig(const char *cfgFileName)
{
#define MAX_FILE_SIZE MAX_LINE_LEN*MAX_COMPONENT_INSTANCE
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("+ParseMtkCoreConfig");
    }
    bool ret = false;
    FILE *fp = NULL;
    int i;
    int moffset = 0;
    char *str = NULL;
    char str_line[MAX_LINE_LEN]={0};//store one line
    unsigned int cur_idx = 0;
    //char *pchEOL = NULL;
    char *pchOMX = NULL;
    int mfileSize = 0;
    int mfileRealSize = 0;

    if ((fp = fopen(cfgFileName, "r")) == NULL)
    {
        MTK_OMX_LOGE("ParseMtkCoreConfig failed. Can't open %s", cfgFileName);
        ret = false;
        goto EXIT;
    }

    fseek(fp, 0, SEEK_END);
    mfileRealSize = ftell(fp);
    rewind(fp);

    if( (mfileRealSize <= 1) || (mfileRealSize >= (MAX_FILE_SIZE-1)) )
    {
        MTK_OMX_LOGE("[ERROR] abnormal file size: %d", mfileRealSize);
        ret = false;
        goto EXIT;
    }

    str = (char *)malloc(mfileRealSize);
    if(NULL != str)
        {
        memset((char *)str, 0x0, mfileRealSize);
        }
    else
    {
        MTK_OMX_LOGE("[ERROR] allocate file size %d fail", mfileRealSize);
        ret = false;
        goto EXIT;
    }

    gCoreComponentCounts = 0;

    pchOMX = fgets(str_line, MAX_LINE_LEN, fp);

    while (pchOMX != NULL) {

        //MTK_OMX_LOGD("pchOMX = %s", pchOMX);
        if(strncmp (pchOMX, MTKOMXTAG, sizeof(MTKOMXTAG)-1) == 0)
        {
            gCoreComponentCounts++;
        }
        pchOMX = fgets(str_line, MAX_LINE_LEN, fp);
        //MTK_OMX_LOGD("pchEOL = %x, diff %d", pchEOL, (pchEOL==NULL)?0:pchEOL-str);
    }

    if (gCoreComponentCounts == 0)
    {
        MTK_OMX_LOGE("[ERROR] No core component available");
        ret = false;
        goto EXIT;
    }

    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("ParseMtkCoreConfig: gCoreComponentCounts = %d", gCoreComponentCounts);
    }

    // allocate memory for gCoreComponents
    gCoreComponents = (mtk_omx_comp_type *)malloc(sizeof(mtk_omx_comp_type) * gCoreComponentCounts);
    memset(gCoreComponents, 0, sizeof(mtk_omx_comp_type)*gCoreComponentCounts);

    if (NULL == gCoreComponents)
    {
        MTK_OMX_LOGE("[ERROR] out of memory to allocate gCoreComponents");
        ret = false;
        goto EXIT;
    }
    rewind(fp);
    // parse OMX component name, role, and path

    for(unsigned int i=0; i<gCoreComponentCounts; i++)
    {
        //MTK_OMX_LOGD("i = %d, cur_idx %d", i, cur_idx);
        pchOMX = fgets(str_line, MAX_LINE_LEN, fp);

        //moffset += (((pchEOL==NULL)?(strlen(str)-moffset):(pchEOL-(str+moffset)))+1);
        //MTK_OMX_LOGD("3 pchEOL = %x, moffset = %d", pchEOL, moffset);
        //MTK_OMX_LOGD("gets [%d]: %s", cur_idx, str_line);

        char *pch, *store_ptr = NULL;
        int len=0;
        pch = strtok_r(str_line, DELIMITERS, &store_ptr);

        // for coverity id:127411 Out-of-bounds read
        if (cur_idx >= gCoreComponentCounts)
        {
            ret = false;
            MTK_OMX_LOGE("[ERROR] parse config error");
            goto EXIT;
        }

        if (pch != NULL)
        {
            assert((pch != NULL));
            len = strlen(pch);
            assert((len > 0) && (len < OMX_COMP_STR_LEN));
            strncpy((char *)gCoreComponents[cur_idx].name, pch, len);

            pch = strtok_r(NULL, DELIMITERS, &store_ptr);
            assert((pch != NULL));
            len = strlen(pch);
            assert((len > 0) && (len < OMX_COMP_STR_LEN));
            strncpy((char *)gCoreComponents[cur_idx].role, pch, len);

            pch = strtok_r(NULL, DELIMITERS, &store_ptr);
            assert((pch != NULL));
            len = strlen(pch);
            assert((len > 0) && (len < OMX_COMP_STR_LEN));
            strncpy((char *)gCoreComponents[cur_idx].lib_path, pch, len);
            //pch = strtok(NULL, DELIMITERS);

            pch = strtok_r(NULL, DELIMITERS, &store_ptr);
            assert((pch != NULL));
            len = strlen(pch);
            assert((len > 0) && (len < OMX_COMP_STR_LEN));
            strncpy((char *)gCoreComponents[cur_idx].max_inst, pch, len);
        }

        cur_idx++;
        if (cur_idx > gCoreComponentCounts)
        {
            ret = false;
            MTK_OMX_LOGE("[ERROR] parse config error");
            goto EXIT;
        }
        memset(str_line, 0, MAX_LINE_LEN);
    }

    ret = true;

EXIT:
    if (fp)
    {
        fclose(fp);
    }

    if(str)
    {
        free(str);
    }

    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("-ParseMtkCoreConfig");
    }
    return ret;
}

// find the count and do the math
static int add_inst_count(char *name, int add)
{
    //MTK_OMX_LOGD("add_inst_count() %s +(%d)", name, add);
    int i = 0;
    while (gCompInstanceCount[i].comp_name)
    {
        if (!strncmp(name, gCompInstanceCount[i].comp_name, strlen(name))) {
            gCompInstanceCount[i].comp_count += add;
            // return current count
            //MTK_OMX_LOGD("add_inst_count() %s returning %d", name, gCompInstanceCount[i].comp_count);
            return gCompInstanceCount[i].comp_count;
        }
        if (MAX_COMPONENT_TYPE == ++i)
        {
            return -1;
        }
    }
    // not found
    //MTK_OMX_LOGD("add_inst_count() %s NOT FOUND! add=%d", name, add);
    // create new count
    if (add > 0)
    {
        // free these buffers in MTK_OMX_Deinit()
        gCompInstanceCount[i].comp_name = (char *)malloc(strlen(name)+sizeof(OMX_STRING));
        memset(gCompInstanceCount[i].comp_name, 0x0, strlen(name)+sizeof(OMX_STRING));
        strncpy(gCompInstanceCount[i].comp_name, name, strlen(name)+sizeof(OMX_STRING));
        gCompInstanceCount[i].comp_count = 1;

        return 1;
    }
    return -1;
}

static int get_inst_count(char *name)
{
    int i = 0;
    while (gCompInstanceCount[i].comp_name)
    {
        if (!strncmp(name, gCompInstanceCount[i].comp_name, strlen(name)))
        {
            //MTK_OMX_LOGD("get_inst_count() %s returning %d", name, gCompInstanceCount[i].comp_count);
            return gCompInstanceCount[i].comp_count;
        }
        if (++i > MAX_COMPONENT_TYPE)
        {
            return -1;
        }
    }
    // not found
    return 0;
}

static int find_omx_comp_index(char *name)
{
    int index = -1;
    unsigned int i = 0;

    for (i = 0; i < gCoreComponentCounts; i++)
    {
        //MTK_OMX_LOGD ("@@ gCoreComponents[%d].name = [%s]", i, gCoreComponents[i].name);
        if (!strcmp(name, (const char *)gCoreComponents[i].name))
        {
            index = i;
            break;
        }
    }

    return index;
}

void free_inst_handle(int index)
{
    if (index < 0 || index >= MAX_COMPONENT_INSTANCE)
    {
        MTK_OMX_LOGE("free_inst_handle invalid index: %d", index);
        return;
    }

    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("free_inst_handle dlclose(idx %d),(0x%p), free(0x%p)", index, gCoreCompInstance[index].comp_module, gCoreCompInstance[index].comp_handle);
    }

    if (0 != dlclose(gCoreCompInstance[index].comp_module))
    {
        MTK_OMX_LOGE("free_inst_handle dlclose failed (%s)", dlerror());
    }

    gCoreCompInstance[index].comp_module = NULL;

    // don't need to free comp_handle since it's a member variable in the vdec class
    //free (gCoreCompInstance[index].comp_handle);
    gCoreCompInstance[index].comp_handle = NULL;

    // count-1
    add_inst_count(gCoreCompInstance[index].comp_name, -1);
    if (gCoreCompInstance[index].comp_name)
    {
        free(gCoreCompInstance[index].comp_name);
        gCoreCompInstance[index].comp_name = NULL;
    }
}


static int find_inst_handle_index(void *pHandle)
{
    int index = -1;
    int i = 0;

    for (i = 0; i < MAX_COMPONENT_INSTANCE; i++)
    {
        if (pHandle == gCoreCompInstance[i].comp_handle)
        {
            index = i;
            break;
        }
    }

    return index;
}

static int get_free_inst_handle_index()
{
    int index = -1;
    int i = 0;

    for (i = 0; i < MAX_COMPONENT_INSTANCE; i++)
    {
        if (NULL == gCoreCompInstance[i].comp_module)
        {
            index = i;
            break;
        }
    }

    return index;
}




static void dump_core_comp_table()
{
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("+++++ dump_core_comp_table +++++");
        for (unsigned int i = 0 ; i < gCoreComponentCounts ; i++)
        {
            MTK_OMX_LOGD("name(%s), role(%s), path(%s)\n", gCoreComponents[i].name, gCoreComponents[i].role, gCoreComponents[i].lib_path);
        }
        MTK_OMX_LOGD("----- dump_core_comp_table -----");
    }
}

static void dump_inst_handle_pool()
{
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("+++++ dump_inst_handle_pool +++++");
        for (int i = 0; i < MAX_COMPONENT_INSTANCE; i++)
        {
            if (NULL != gCoreCompInstance[i].comp_module)
            {
                MTK_OMX_LOGD("gCoreCompInstance[%d].comp_module=0x%p, comp_handle=0x%p, comp_name=%s",
                    i, gCoreCompInstance[i].comp_module, gCoreCompInstance[i].comp_handle, gCoreCompInstance[i].comp_name);
            }
        }
        MTK_OMX_LOGD("----- dump_inst_handle_pool -----");
    }
}

//static int gHandleCount = 0;

#if 0 // 20150918 mtk09689: This memory monitoring method may be used in the future...
static int gCompIndex = 0;
static void add_getHandle_count(int index)
{
    if (index < 0 || index >= MAX_COMPONENT_INSTANCE)
    {
        MTK_OMX_LOGE("update_omx_comp_instance_history invalid index: %d", index);
        return;
    }
    gCompIndex = index;
    gCoreCompInstanceHistory[index].numOfGetHandle = gHandleCount;
    gCoreCompInstanceHistory[index].numOfFreeHandle = 0;
}


static void add_freeHandle_count()
{
    gCoreCompInstanceHistory[gCompIndex].numOfFreeHandle++;
    if (gCoreCompInstanceHistory[gCompIndex].numOfFreeHandle > gCoreCompInstanceHistory[gCompIndex].numOfGetHandle) {
        MTK_OMX_LOGE("For component index %d, it's weird that #freeHandle < #gethandle.",gCompIndex);
        return;
    }

    if (!gCoreCompInstanceHistory[gCompIndex].alreadyProfiled && (gCoreCompInstanceHistory[gCompIndex].numOfFreeHandle == gCoreCompInstanceHistory[gCompIndex].numOfFreeHandle)) {
        gCoreCompInstanceHistory[gCompIndex].alreadyProfiled = OMX_TRUE;
    }

}

static OMX_BOOL isProflied(int index) {
    if (index < 0 || index >= MAX_COMPONENT_INSTANCE)
    {
        MTK_OMX_LOGE("update_omx_comp_instance_history invalid index: %d", index);
        return OMX_FALSE;
    }
     return gCoreCompInstanceHistory[index].alreadyProfiled;
}
#endif

//#ifdef MT6577
#if ENABLE_OMX_CPU_POWER_CONTROL
void *MtkOmxCoreCpuControlThread(void *pData)
{
    VDEC_UNUSED(pData);
    sem_init(&(gCoreGlobal.omx_core_ctrl_thread_sem), 0, 0);
    while (1)
    {
        //MTK_OMX_LOGD ("## video_instance_count(%d), gCoreGlobal.video_operation_count(%d)", gCoreGlobal.video_instance_count, gCoreGlobal.video_operation_count);
        if (gCoreGlobal.video_instance_count > 0)
        {
            // check video operation count
            if (gCoreGlobal.video_operation_count != gCoreGlobal.video_prev_operation_count)
            {
                // video is online
                if (gCoreGlobal.video_power_saving_enabled == OMX_FALSE)
                {
                    // switch to one core
                    MTK_OMX_LOGD("## CPU OPP MASK ON (Video)");
                    VAL_VCODEC_CPU_OPP_LIMIT_T cpu_opp_limit;
                    //cpu_opp_limit.limited_freq = 1001000;
                    //cpu_opp_limit.limited_cpu = 1;
                    cpu_opp_limit.enable = true;
                    //eVideoVcodecSetCpuOppLimit(&cpu_opp_limit);
                    gCoreGlobal.video_power_saving_enabled = OMX_TRUE;

#if ENABLE_OMX_CPU_DVFS_CONTROL
                    // disable DVFS
                    MTK_OMX_LOGD("## disable DVFS");
                    HAL_POWER_T emi;
                    emi.fgEnable = VAL_FALSE;
                    eHalDVFSDCMCtrl(&emi);
#endif
                }
            }
            else
            {
                // video is offline
                if (gCoreGlobal.video_power_saving_enabled == OMX_TRUE)
                {
                    // switch to dual core
                    MTK_OMX_LOGD("## CPU OPP MASK OFF (Video)");
                    VAL_VCODEC_CPU_OPP_LIMIT_T cpu_opp_limit;
                    //cpu_opp_limit.limited_freq = 1001000;
                    //cpu_opp_limit.limited_cpu = 1;
                    cpu_opp_limit.enable = false;
                    //eVideoVcodecSetCpuOppLimit(&cpu_opp_limit);
                    gCoreGlobal.video_power_saving_enabled = OMX_FALSE;

#if ENABLE_OMX_CPU_DVFS_CONTROL
                    // enable DVFS
                    MTK_OMX_LOGD("## enable DVFS");
                    HAL_POWER_T emi;
                    emi.fgEnable = VAL_TRUE;
                    eHalDVFSDCMCtrl(&emi);
#endif
                }
            }
            gCoreGlobal.video_prev_operation_count = gCoreGlobal.video_operation_count;
#if 0
            for (int i = 0 ; i < gCoreGlobal.gInstanceList.size() ; i++)
            {
                mtk_omx_instance_data *pInstanceData = gCoreGlobal.gInstanceList.itemAt(i);
                MTK_OMX_LOGD("@@ [%d] processing(%d), op_thread(%d), last_processing_tick(%lld)", gCoreGlobal.gInstanceList.size(), pInstanceData->processing, pInstanceData->op_thread, pInstanceData->last_processing_tick);
                if (OMX_TRUE == pInstanceData->processing)
                {
                    int64_t _interval = getTickCountMs() - pInstanceData->last_processing_tick;
                    MTK_OMX_LOGD("@@ last interval = %lld", _interval);
                    if (_interval > 5 * 1000)
                    {
                        MTK_OMX_LOGE("@@ OOPS decode thread is possibly hang !!");
                        pInstanceData->processing = OMX_FALSE;
                        pthread_kill(pInstanceData->op_thread, SIGUSR1);
                    }
                }
            }
#endif
        }
        else
        {
            // no video instance
            // video is offline
            if (gCoreGlobal.video_power_saving_enabled == OMX_TRUE)
            {
                // switch to dual core
                MTK_OMX_LOGD("## CPU OPP MASK OFF (Video)");
                VAL_VCODEC_CPU_OPP_LIMIT_T cpu_opp_limit;
                //cpu_opp_limit.limited_freq = 1001000;
                //cpu_opp_limit.limited_cpu = 1;
                cpu_opp_limit.enable = false;
                //eVideoVcodecSetCpuOppLimit(&cpu_opp_limit);
                gCoreGlobal.video_power_saving_enabled = OMX_FALSE;

#if ENABLE_OMX_CPU_DVFS_CONTROL
                // enable DVFS
                MTK_OMX_LOGD("## enable DVFS");
                HAL_POWER_T emi;
                emi.fgEnable = VAL_TRUE;
                eHalDVFSDCMCtrl(&emi);
#endif
            }

            MTK_OMX_LOGD("## MtkOmxCoreCpuControlThread terminated");
            return NULL;
        }

        timespec ts;
        timeval tv;
        gettimeofday(&tv, NULL);
        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = (tv.tv_usec + 500 * 1000) * 1000; // wait 500 ms
        ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec %= (1000 * 1000 * 1000);
        int ret = sem_timedwait(&(gCoreGlobal.omx_core_ctrl_thread_sem), &ts);
        // when thread is going to terminate (signaled, ret should be 0, otherwise the ret should be -1)
        //MTK_OMX_LOGD ("## sem_timedwait ret (%d), errno=%d", ret, errno);
    }
    return NULL;
}
#endif
//#endif

/*************************************************************************
* OMX_Init()
*
* Description:
*
*Parameters:
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_Init()
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    //#ifdef MT6577
#if ENABLE_OMX_CPU_POWER_CONTROL
    //memset(&gCoreGlobal, 0, sizeof(gCoreGlobal));
    // TODO: initialize member one by one
    gCoreGlobal.omx_core_control_thread = (pthread_t)NULL;
    gCoreGlobal.video_instance_count = 0;
    gCoreGlobal.video_operation_count = 0;
    gCoreGlobal.video_power_saving_enabled = OMX_FALSE;
    gCoreGlobal.video_prev_operation_count = 0;
    gCoreGlobal.gInstanceList.clear();
#endif
    //#endif

    char value[PROPERTY_VALUE_MAX];

    property_get("mtk.vendor.omx.core.log", value, "0");
    gOMXCoreLogProperty = atoi(value);

    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("+MTK_OMX_Init tick=%lld", (long long)getTickCountMs());
    }


    //pthread_mutex_init(&gInstancePoolLock, NULL);
    pthread_mutex_lock(&gInstancePoolLock);

    gCoreReferenceCounts ++;


    if (gInitialized)
    {
        if(gOMXCoreLogProperty)
        {
            MTK_OMX_LOGD("MTK_OMX_Init MTK OMX Core has already initialized !!!");
        }
        pthread_mutex_unlock(&gInstancePoolLock);
        return err;
    }

    if (false == ParseMtkCoreConfig(MTK_OMXCORE_CFG_FILE))
    {
        MTK_OMX_LOGE("MTK_OMX_Init failed!!! ");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    gCompInstanceCount = (mtk_omx_comp_count_type *)malloc(sizeof(mtk_omx_comp_count_type) * MAX_COMPONENT_TYPE);
    memset(gCompInstanceCount, 0, sizeof(mtk_omx_comp_count_type) * MAX_COMPONENT_TYPE);

    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("Mtk_OMX_Init gCoreComponents 0x%lx", (unsigned long)gCoreComponents);
    }

    gInitialized = true;

    dump_core_comp_table();

EXIT:
    pthread_mutex_unlock(&gInstancePoolLock);
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("-MTK_OMX_Init tick=%lld", (long long)getTickCountMs());
    }
    return err;
}

#if 0 // 20150918 mtk09689: This memory monitoring method may be used in the future...
#define SAFE_BOUND_BOOT 30*1024*1024LL // margin for buffer usage by other process
#define SAFE_BOUND 10*1024*1024LL // margin for buffer usage by other process

/*************************************************************************
* Description:
*   Get the least memory for system
* Parameters: N/A
* Returns:
*   LMK threshold on successful
*   -1 on error
*
* Note
*   Add this function for ResourceManagerTest on M
*   Query LMK threshold to decide the number of multiple instance of codec
*
**************************************************************************/
static long getSysRetainMem() {
    int fd = open("/proc/zoneinfo", O_RDONLY);
    if (fd < 0) {
        ALOGW("Unable to open /proc/zoneinfo: %s\n", strerror(errno));
        return -1;
    }

    char buffer[1024];
    int len = ::read(fd, buffer, sizeof(buffer)-1);
    if (len < 0) {
        ALOGW("Empty /proc/zoneinfo");
        close(fd);
        return -1;
    }
    buffer[len] = 0;
    close(fd);

    char* ptr = buffer;
    char str[] = "high";
    long sysretainmem = 0;

    ptr = strstr(ptr, str);
    if (ptr == NULL){
        ALOGE("not find the sysretainmem");
        return -1;
    }
    while(*ptr && (*ptr<'1' || *ptr>'9')) ptr++;
    if(!(*ptr)){
        ALOGE("Not find the sysretainmem");
        return -1;
    }
    sysretainmem = atoll(ptr);

    if (sysretainmem <= 0){
        ALOGE("sysretainmem(%ld)<=0", sysretainmem);
        return -1;
    }

    ALOGD("PAGESIZE(%d), sysretainmem(%ld), SysRetainMem(%ld)", PAGESIZE, sysretainmem, sysretainmem*PAGESIZE);
    return sysretainmem*PAGESIZE;
}


/*************************************************************************
* Description:
*   Get the threshold of low-memory-killer
* Parameters: N/A
* Returns:
*   LMK threshold on successful
*   -1 on error
*
* Note
*   Add this function for ResourceManagerTest on M
*   Query LMK threshold to decide the number of multiple instance of codec
*
**************************************************************************/
static long getMinFreeMem() {
    int fd = open("/sys/module/lowmemorykiller/parameters/minfree", O_RDONLY);
    if (fd < 0) {
        ALOGW("Failed to open minfree under LMK: %s\n", strerror(errno));
        return -1;
    }
    char buffer[1024] = {0};
    int len = ::read(fd, buffer, (sizeof(buffer) -  1));
    close(fd);
    if (len < 0) {
        ALOGW("Empty  minfree under LMK");
        return -1;
    }
    buffer[len] = 0;

    char* ptr = buffer;
    long minFreeMax = 0L;
#define MINFREE_LEN 4
    long minFree[MINFREE_LEN];
    //TODO: The first number is enough
    for (int i = 0; i < MINFREE_LEN; ++i) {
        while (*ptr && (*ptr < '1' || *ptr > '9'))
            ++ptr;
        if (!(*ptr))
            break;
        minFree[i] = atoll(ptr);
        ALOGD("got minFree\td\t%ld", i, minFree[i]);
        if (minFree[i] > minFreeMax)
            minFreeMax = minFree[i];
        while(*ptr && (*ptr >= '0' && *ptr <= '9'))
            ++ptr;
    }

    if (minFreeMax <= 0)
        return -1;

    ALOGD("PAGESIZE(%ld), minFreeMax(%ld), minFreeMem(%ld)",
            PAGESIZE, minFreeMax, (minFreeMax * PAGESIZE));
    return (minFreeMax * PAGESIZE);
}

static void getMemInfo(long *memFree, long *cached)
{
    char buffer[1024];
    size_t numFound = 0;

    int fd = open("/proc/meminfo", O_RDONLY);

    if (fd < 0) {
        ALOGW("Unable to open /proc/meminfo: %s\n", strerror(errno));
        return;
    }

    int len = read(fd, buffer, sizeof(buffer)-1);
    close(fd);

    if (len < 0) {
        ALOGW("Empty /proc/meminfo");
        return;
    }
    buffer[len] = 0;

    static const char* const tags[] = {
            "MemTotal:",
            "MemFree:",
            "Buffers:",
            "Cached:",
            NULL
    };
    static const int tagsLen[] = {
            9,
            8,
            8,
            7,
            0
    };
    long mem[] = { 0, 0, 0, 0, 0};

    char* p = buffer;
    while (*p && numFound < (sizeof(tagsLen) / sizeof(tagsLen[0]))) {
        int i = 0;
        while (tags[i]) {
            if (strncmp(p, tags[i], tagsLen[i]) == 0) {
                p += tagsLen[i];
                while (*p == ' ') p++;
                char* num = p;
                while (*p >= '0' && *p <= '9') p++;
                if (*p != 0) {
                    *p = 0;
                    p++;
                }
                mem[i] = atoll(num);
                numFound++;
                break;
            }
            i++;
        }
        while (*p && *p != '\n') {
            p++;
        }
        if (*p) p++;
    }
    *memFree = mem[1]<<10;
    *cached = mem[3]<<10;
}

static void getIonInfo(long *IonMMHeapInPool)
{
    char buffer[1024];
    size_t numFound = 0;

    int fd = open("/d/ion/heaps/ion_mm_heap_total_in_pool", O_RDONLY);

    if (fd < 0) {
        ALOGW("Unable to open /d/ion/heaps/ion_mm_heap_total_in_pool: %s\n", strerror(errno));
        return;
    }

    int len = read(fd, buffer, sizeof(buffer)-1);
    close(fd);

    if (len < 0) {
        ALOGW("Empty /d/ion/heaps/ion_mm_heap_total_in_pool");
        return;
    }
    buffer[len] = 0;

    char* ptr = buffer;
    char str[] = "total_in_pool";
    long IonTotalInPool = 0;

    ptr = strstr(ptr, str);
    if (ptr == NULL){
        ALOGE("not find the ion_mm_heap_total_in_pool");
        return;
    }
    while(*ptr && (*ptr<'1' || *ptr>'9')) ptr++;
    if(!(*ptr)){
        ALOGE("Not find the ion_mm_heap_total_in_pool");
        return;
    }
    IonTotalInPool = atoll(ptr);

    if (IonTotalInPool <= 0){
        ALOGE("IonTotalInPool(%ld)<=0", IonTotalInPool);
        return;
    }
    *IonMMHeapInPool = IonTotalInPool<<10;
}
#endif

/*************************************************************************
* OMX_GetHandle()
*
* Description:
*
*Parameters:
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_GetHandle(OMX_HANDLETYPE *pHandle, OMX_STRING cComponentName,
                                OMX_PTR pAppData, OMX_CALLBACKTYPE *pCallBacks)
{
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("Mtk_OMX_GetHandle (%s)", (char *)cComponentName);
    }

    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_ERRORTYPE(*pComponentInit)(OMX_HANDLETYPE *);
    OMX_COMPONENTTYPE* (*pMtkOmxComponentCreate)(OMX_STRING);

    //#ifdef MT6577
#if ENABLE_OMX_CPU_POWER_CONTROL
    void (*pMtkOmxSetCoreGlobal)(OMX_COMPONENTTYPE *, void *);
#endif
    //#endif

    OMX_COMPONENTTYPE *pComponentType;

    pthread_mutex_lock(&gInstancePoolLock);

    void *pOmxModule = NULL;
    int comp_index, max_instance;
    int free_inst_handle_index;

    int so_nodelete = 0;
#if 0
    uint64_t safeBond = 0;
    uint64_t minMemFree = (uint64_t)getMinFreeMem() + (uint64_t)getSysRetainMem();
    long memFree=0, cached=0, IonMMHeapInPool=0;
#endif
    if ((NULL == pHandle) || (NULL == cComponentName) || (NULL == pCallBacks))
    {
        err = OMX_ErrorBadParameter;
        MTK_OMX_LOGE("Mtk_OMX_GetHandle failed OMX_ErrorBadParameter");
        goto EXIT;
    }

    *pHandle = NULL;

    comp_index = find_omx_comp_index(cComponentName);
    max_instance = atoi((const char*)gCoreComponents[comp_index].max_inst);
    if (comp_index >= 0)
    {
        if(gOMXCoreLogProperty)
        {
            MTK_OMX_LOGD("comp_index(%d), path(%s) max_instance(%d)", comp_index, gCoreComponents[comp_index].lib_path, max_instance);
        }
#if 0 // 20150918 mtk09689: This memory monitoring method may be used in the future...
        // Check whether it's in profling stage
        if (isProflied(comp_index)) {
            safeBond = SAFE_BOUND;
        } else {
            safeBond = SAFE_BOUND_BOOT; // for profling stage, it has free memory so the larger margin is necessary
        }

        // Check the retained buffer:
        // Here we compare the minimum of system free buffe from low-memory thresold & retained memory.
        getMemInfo(&memFree, &cached);
        getIonInfo(&IonMMHeapInPool);
       ALOGD("[Marcus] gHandleCount:%d lowBond:%lld Free:%ld",gHandleCount,(getMinFreeMem()+getSysRetainMem()+safeBond),(MAX(memFree, cached)+IonMMHeapInPool));

        if ((int64_t)(MAX(memFree, cached)+IonMMHeapInPool) < (minMemFree + safeBond)) {
#endif

        if (!strncmp(cComponentName, "OMX.MTK.VIDEO.", 14) && (get_inst_count(cComponentName)>= max_instance)) {
            err = OMX_ErrorInsufficientResources;
            MTK_OMX_LOGE("Returns insufficientResource");
            goto ERROR;
        }

        if (!strncmp(cComponentName, "OMX.MTK.AUDIO.DECODER.RAW", 25) ||
                !strncmp(cComponentName, "OMX.MTK.AUDIO.DECODER.MP3", 25)) {
            so_nodelete = 1;
        }

        free_inst_handle_index = get_free_inst_handle_index();
        if (free_inst_handle_index < 0) {
            err = OMX_ErrorInsufficientResources;
            MTK_OMX_LOGE("%d:: get_free_inst_handle_index failed", __LINE__);
            goto ERROR;
        }

        pOmxModule = dlopen((const char *)gCoreComponents[comp_index].lib_path,
                so_nodelete ? (RTLD_NOW|RTLD_NODELETE) : RTLD_LAZY);

        if (NULL == pOmxModule) {
            MTK_OMX_LOGE("dlopen failed, %s", dlerror());
            err = OMX_ErrorComponentNotFound;
            goto ERROR;
        }

        pMtkOmxComponentCreate = (OMX_COMPONENTTYPE * (*)(OMX_STRING))dlsym(pOmxModule, "MtkOmxComponentCreate");

        if (NULL == pMtkOmxComponentCreate) {
            MTK_OMX_LOGE("%d:: dlsym failed for module %p, dlerror=%s", __LINE__, pOmxModule, dlerror());
            err = OMX_ErrorInvalidComponent;
            goto ERROR;
        }

        *pHandle = (*pMtkOmxComponentCreate)(cComponentName);
        if (*pHandle == NULL) {
            err = OMX_ErrorInsufficientResources;
            MTK_OMX_LOGE("%d:: malloc failed", __LINE__);
            goto ERROR;
        }

        //#ifdef MT6577
#if ENABLE_OMX_CPU_POWER_CONTROL
        // check if core specific data interface exists
        pMtkOmxSetCoreGlobal = (void (*)(OMX_COMPONENTTYPE *, void *))dlsym(pOmxModule, "MtkOmxSetCoreGlobal");
        if (NULL == pMtkOmxSetCoreGlobal) {
            if(gOMXCoreLogProperty) {
                MTK_OMX_LOGD("MtkOmxSetCoreGlobal is NOT supported on this component");
            }
            // leave it, it's fine
        } else {
            (*pMtkOmxSetCoreGlobal)((OMX_COMPONENTTYPE *) *pHandle, (void *)&gCoreGlobal);

            if (gCoreGlobal.video_instance_count == 1) { // first video instance
                //pthread_mutex_init(&(gCoreGlobal.omx_global_Lock), NULL);
                int ret = pthread_create(&(gCoreGlobal.omx_core_control_thread), NULL, &MtkOmxCoreCpuControlThread, NULL);
                if (ret) {
                    MTK_OMX_LOGE("MtkOmxCoreCpuControlThread creation failure");
                    err = OMX_ErrorInsufficientResources;
                    goto ERROR;
                } else {
                    if(gOMXCoreLogProperty) {
                        MTK_OMX_LOGD("MtkOmxCoreCpuControlThread created (0x%08lX)", (unsigned long)gCoreGlobal.omx_core_control_thread);
                    }
                }
            }
        }
#endif
        //#endif
        pComponentType = (OMX_COMPONENTTYPE *) *pHandle;
        err = (pComponentType->SetCallbacks)(*pHandle, pCallBacks, pAppData);
        if (err != OMX_ErrorNone) {
            MTK_OMX_LOGE("%d :: SetCallBack failed %d", __LINE__, err);
            goto ERROR;
        }

        gCoreCompInstance[free_inst_handle_index].comp_module = pOmxModule; // save the omx component module
        gCoreCompInstance[free_inst_handle_index].comp_handle = *pHandle;   // save the omx component handle
        gCoreCompInstance[free_inst_handle_index].comp_name = (char *)malloc(strlen(cComponentName)+sizeof(OMX_STRING));
        if (gCoreCompInstance[free_inst_handle_index].comp_name != NULL)
        {
            memset(gCoreCompInstance[free_inst_handle_index].comp_name, 0x0, strlen(cComponentName)+sizeof(OMX_STRING));
            strncpy(gCoreCompInstance[free_inst_handle_index].comp_name, cComponentName, strlen(cComponentName)+sizeof(OMX_STRING));
        }
        else
        {
            MTK_OMX_LOGE("%d :: allocate comp name failed %d", __LINE__, err);
            goto ERROR;
        }
        dump_inst_handle_pool();

        add_inst_count(cComponentName, 1);

    //++gHandleCount;
//   add_getHandle_count(comp_index);
        goto EXIT;
    } else {
        err = OMX_ErrorInvalidComponent;
        goto EXIT;
    }

ERROR:
    if (*pHandle) {
        free(*pHandle);
        *pHandle = NULL;
    }
    if (pOmxModule) {
        dlclose(pOmxModule);
    }

EXIT:
    pthread_mutex_unlock(&gInstancePoolLock);
    return err;
}


/*************************************************************************
* OMX_FreeHandle()
*
* Description:
*
*Parameters:
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_FreeHandle(OMX_HANDLETYPE hComponent)
{
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("Mtk_OMX_FreeHandle");
    }
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)hComponent;

    pthread_mutex_lock(&gInstancePoolLock);

    int inst_handle_index = find_inst_handle_index(hComponent);

    if (inst_handle_index < 0)
    {
        MTK_OMX_LOGE("%d :: Core: component %p not found\n", __LINE__, hComponent);
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    err = pHandle->ComponentDeInit(hComponent);
    if (err != OMX_ErrorNone)
    {
        MTK_OMX_LOGE("%d :: ComponentDeInit failed %d\n", __LINE__, err);
        goto EXIT;
    }

    //#ifdef MT6577
#if ENABLE_OMX_CPU_POWER_CONTROL
    if ((gCoreGlobal.video_instance_count == 0) && (gCoreGlobal.omx_core_control_thread != (pthread_t)NULL))
    {
        if (!pthread_equal(pthread_self(), gCoreGlobal.omx_core_control_thread))
        {
            // signal gCoreGlobal.omx_core_control_thread to terminate
            sem_post(&(gCoreGlobal.omx_core_ctrl_thread_sem));
            // wait for gCoreGlobal.omx_core_control_thread terminate
            pthread_join(gCoreGlobal.omx_core_control_thread, NULL);
            gCoreGlobal.omx_core_control_thread = (pthread_t) NULL;
            sem_destroy(&(gCoreGlobal.omx_core_ctrl_thread_sem));
        }
    }
#endif
    //#endif

    free_inst_handle(inst_handle_index);
    //--gHandleCount;
    //add_freeHandle_count();
EXIT:
    pthread_mutex_unlock(&gInstancePoolLock);
    return err;
}



/*************************************************************************
* OMX_Deinit()
*
* Description:
*
*Parameters:
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_Deinit()
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("Mtk_OMX_Deinit");
    }
    pthread_mutex_lock(&gInstancePoolLock);
    // check if there are any non-freed instance handles
    gCoreReferenceCounts--;

    if (gCoreReferenceCounts == 0)
    {
        for (int i = 0; i < MAX_COMPONENT_INSTANCE; i++)
        {
            if (NULL != gCoreCompInstance[i].comp_handle)
            {
                err = Mtk_OMX_FreeHandle(gCoreCompInstance[i].comp_handle);
                if (err != OMX_ErrorNone)
                {
                    MTK_OMX_LOGE("%d :: Mtk_OMX_Deinit failed %d\n", __LINE__, err);
                    goto EXIT;
                }
            }
        }
        gInitialized = false;
        if (gCoreComponents)
        {
            if(gOMXCoreLogProperty)
            {
                MTK_OMX_LOGD("Mtk_OMX_Deinit  gCoreComponents  0x%lx", (unsigned long)gCoreComponents);
            }
            free(gCoreComponents);
            gCoreComponents = NULL;
        }
        if (gCompInstanceCount)
        {
            for (int i = 0; i < MAX_COMPONENT_TYPE; ++i)
            {
                if (gCompInstanceCount[i].comp_name)
                {
                    free(gCompInstanceCount[i].comp_name);
                    gCompInstanceCount[i].comp_name = NULL;
                } else {
                    break;
                }

            }
            free(gCompInstanceCount);
            gCompInstanceCount = NULL;
        }
    }


EXIT:
    pthread_mutex_unlock(&gInstancePoolLock);
    return err;
}


/*************************************************************************
* OMX_ComponentNameEnum()
*
* Description: This method will provide the name of the component at the given nIndex
*
*Parameters:
* @param[out] cComponentName       The name of the component at nIndex
* @param[in] nNameLength                The length of the component name
* @param[in] nIndex                         The index number of the component
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("Mtk_OMX_ComponentNameEnum gCoreComponentCounts=%d", gCoreComponentCounts);

    if (nIndex >= gCoreComponentCounts)
    {
        err = OMX_ErrorNoMore;
    }
    else
    {
        strncpy(cComponentName, (const char *)gCoreComponents[nIndex].name, nNameLength);
    }

    return err;
}



/*************************************************************************
* OMX_GetRolesOfComponent()
*
* Description: This method will query the component for its supported roles
*
*Parameters:
* @param[in] cComponentName     The name of the component to query
* @param[in/out] pNumRoles     The number of roles supported by the component
* @param[out] roles     The roles of the component
*
* Returns:    OMX_NOERROR          Successful
*                 OMX_ErrorBadParameter     Faliure due to a bad input parameter
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_GetRolesOfComponent(
    OMX_IN      OMX_STRING compName,
    OMX_INOUT   OMX_U32 *pNumRoles,
    OMX_OUT     OMX_U8 **roles)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("Mtk_OMX_GetRolesOfComponent");
    unsigned int i = 0;

    if (compName == NULL)
    {
        // invalid parameter
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (roles == NULL)
    {
        if (pNumRoles == NULL)
        {
            // invalid parameter
            err = OMX_ErrorBadParameter;
        }
        else
        {
            // get the role counts of this component
            for (i = 0; i < gCoreComponentCounts; i++)
            {
                if (!strcmp(compName, (const char *)gCoreComponents[i].name))
                {
                    *pNumRoles = 1;  // each component always act one role
                    break;
                }
            }
        }
        goto EXIT;
    }

    // both pNumRoles and pNumRoles are not NULL, retrieve the component roles
    if (pNumRoles)
    {
        *pNumRoles = 0;
        for (i = 0; i < gCoreComponentCounts; i++)
        {
            if (!strcmp(compName, (const char *)gCoreComponents[i].name))
            {
                strncpy((char *)roles[(*pNumRoles)], (const char *)gCoreComponents[i].role, OMX_COMP_STR_LEN);
                *pNumRoles = 1;  // each component always act one role
                break;
            }
        }
    }
    else
    {
        err = OMX_ErrorBadParameter;
    }

EXIT:
    return err;
}

/*************************************************************************
* OMX_GetComponentsOfRole()
*
* Description: This method will query the component for its supported roles
*
*Parameters:
* @param[in] role                  The name of the role to query
* @param[in/out] pNumComps     The number of components support this role
* @param[out] compNames   The name of the component
*
* Returns:    OMX_NOERROR          Successful
*                 OMX_ErrorBadParameter     Faliure due to a bad input parameter
*
* Note
*
**************************************************************************/
extern "C"
OMX_ERRORTYPE Mtk_OMX_GetComponentsOfRole(
    OMX_IN        OMX_STRING role,
    OMX_INOUT  OMX_U32 *pNumComps,
    OMX_OUT     OMX_U8 **compNames)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("Mtk_OMX_GetComponentsOfRole");
    }
    unsigned int i = 0;

    if (role == NULL)
    {
        // invalid parameter
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (compNames == NULL)
    {
        if (pNumComps == NULL)
        {
            // invalid parameter
            err = OMX_ErrorBadParameter;
        }
        else
        {
            *pNumComps = 0;
            // get the component counts that play this role
            for (i = 0; i < gCoreComponentCounts; i++)
            {
                if (!strcmp(role, (const char *)gCoreComponents[i].role))
                {
                    (*pNumComps)++;
                }
            }
        }
        goto EXIT;
    }

    // both pNumComps and compNames are not NULL, retrieve the component names
    if (pNumComps)
    {
        *pNumComps = 0;
        for (i = 0; i < gCoreComponentCounts; i++)
        {
            if (!strcmp(role, (const char *)gCoreComponents[i].role))
            {
                strncpy((char *)compNames[(*pNumComps)], (const char *)gCoreComponents[i].name, OMX_COMP_STR_LEN);
                (*pNumComps)++;
            }
        }
    }
    else
    {
        err = OMX_ErrorBadParameter;
    }

EXIT:
    return err;
}

extern "C" {
    void __attribute__((constructor))  Core_Init(void);
    void __attribute__((destructor))  Core_Deinit(void);
}

extern "C"
void Core_Init(void)
{
    if(gOMXCoreLogProperty)
    {
    MTK_OMX_LOGD("Core_Init called");
    }
    if (gInitialized == false)
    {
        pthread_mutex_init(&gInstancePoolLock, NULL);
        if(gOMXCoreLogProperty)
        {
            MTK_OMX_LOGD("Core_Init gInstancePoolLock init");
        }
    }
}

extern "C"
void Core_Deinit(void)
{
    if(gOMXCoreLogProperty)
    {
        MTK_OMX_LOGD("Core_Deinit called");
    }
    if (gInitialized == false)
    {
        pthread_mutex_destroy(&gInstancePoolLock);
        if(gOMXCoreLogProperty)
        {
            MTK_OMX_LOGD("Core_Deinit gInstancePoolLock destroy");
        }
    }
}

