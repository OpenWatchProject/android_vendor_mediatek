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

#define LOG_TAG "cctsvr_entry"
//
#include <dlfcn.h>
#include <utils/Mutex.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/main/acdk/CctIF.h>


/******************************************************************************
 *
 ******************************************************************************/
#ifdef USING_MTK_LDVT

#define MY_LOGV(fmt, arg...)        //ALOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        //ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        //ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        //ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        //ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        //ALOGF("[%s] " fmt, __FUNCTION__, ##arg)

#else

#define MY_LOGV(fmt, arg...)        ALOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        ALOGF("[%s] " fmt, __FUNCTION__, ##arg)

#endif


//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace
{
static android::Mutex&   gMutex = *new Mutex();
static void*            gModuleLib = NULL;
#define MODULE_PATH "libcam.hal3a.cctsvr.so"
}


/******************************************************************************
 *
 ******************************************************************************/
static
void*
getModuleLib()
{
    android::Mutex::Autolock _l(gMutex);
    if  ( gModuleLib )
    {
        return  gModuleLib;
    }
    //
    gModuleLib = ::dlopen(MODULE_PATH, RTLD_NOW);
    if  ( ! gModuleLib )
    {
        char const *err_str = ::dlerror();
        MY_LOGE("dlopen: %s error=%s", MODULE_PATH, (err_str ? err_str : "unknown"));
    }
    //
    return  gModuleLib;
}


/******************************************************************************
 *
 ******************************************************************************/
static
void*
getModuleEntry(char const szEntrySymbol[])
{
    void*const lib = getModuleLib();
    void*const pfnEntry = ::dlsym(lib, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        MY_LOGE("dlsym: %s error=%s getModuleLib:%p", szEntrySymbol, (err_str ? err_str : "unknown"), lib);
        return  NULL;
    }
    //
    return  pfnEntry;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CctSvrEnt_Init(MINT32 sendorID)
{
    //  entry type / entry name
    typedef MBOOL (*pfnEntry_T)(MINT32);
    char const szEntrySymbol[] = "CctSvrIF_Init";
    //
    pfnEntry_T pfnEntry = (pfnEntry_T)::getModuleEntry(szEntrySymbol);
    if  ( ! pfnEntry )
    {
        return  MFALSE;
    }
    //
    MY_LOGD("sensor(%d), %p", sendorID, pfnEntry);
    MBOOL ret = pfnEntry(sendorID);
    if  ( ! ret )
    {
        MY_LOGE("%s() return false", szEntrySymbol);
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CctSvrEnt_DeInit()
{
    //  entry type / entry name
    typedef MBOOL (*pfnEntry_T)();
    char const szEntrySymbol[] = "CctSvrIF_DeInit";
    //
    pfnEntry_T pfnEntry = (pfnEntry_T)::getModuleEntry(szEntrySymbol);
    if  ( ! pfnEntry )
    {
        return  MFALSE;
    }
    //
    MBOOL ret = pfnEntry();
    if  ( ! ret )
    {
        MY_LOGE("%s() return false", szEntrySymbol);
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CctSvrEnt_Ctrl(MUINT32 cmd)
{
    //  entry type / entry name
    typedef MBOOL (*pfnEntry_T)(MUINT32);
    char const szEntrySymbol[] = "CctSvrIF_Ctrl";
    //
    pfnEntry_T pfnEntry = (pfnEntry_T)::getModuleEntry(szEntrySymbol);
    if  ( ! pfnEntry )
    {
        return  MFALSE;
    }
    //
    MBOOL ret = pfnEntry(cmd);
    if  ( ! ret )
    {
        MY_LOGE("%s() return false", szEntrySymbol);
        return  MFALSE;
    }
    //
    return  MTRUE;
}



