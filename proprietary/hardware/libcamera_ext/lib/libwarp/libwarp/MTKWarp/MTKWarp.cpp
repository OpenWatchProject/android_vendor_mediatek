/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MTKWarpExt"
#define MTK_LOG_ENABLE 1
#include "MTKWarp.h"

#include "AppGlesWarp.h"
#include "AppCpuWarp.h"


#ifdef SIM_MAIN
#include <stdio.h>
#define LOGD printf
#else
#include <android/log.h>
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,##__VA_ARGS__) 
#endif

MTKWarpExt*
MTKWarpExt::createInstance(DrvWarpObject_e eobject)
{

    if (eobject == DRV_WARP_OBJ_GLES)
    {
        return AppGlesWarpExt::getInstance();
    }
    else if (eobject == DRV_WARP_OBJ_CPU)
    {
        return AppCpuWarp::getInstance();
    }
    else
    {
        return AppWarpTmp::getInstance();
    }

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MTKWarpExt*
AppWarpTmp::
getInstance()
{
    LOGD("[halFDTmp] getInstance \n");
    static AppWarpTmp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
AppWarpTmp::
destroyInstance(MTKWarpExt* obj)
{
    if(NULL == obj)
        delete obj;
}


/*******************************************************************************
*
********************************************************************************/
MTKWarpExt::~MTKWarpExt()
{
    //LOGD("MTKWarp destructor \n");
}

MRESULT MTKWarpExt::WarpInit(MUINT32 *InitInData, MUINT32 *InitOutData)
{
    LOGD("MTKWarp WarpInit ");
    if((InitInData!=NULL) ||(InitOutData!=NULL))
    	LOGD("...\n");
    return E_WARP_NEED_OVER_WRITE;
}

MRESULT MTKWarpExt::WarpMain(void)
{
    LOGD("MTKWarp WarpMain \n");
    return E_WARP_NEED_OVER_WRITE;
}

MRESULT MTKWarpExt::WarpReset(void)
{
    LOGD("MTKWarp WarpReset \n");
    return E_WARP_NEED_OVER_WRITE;
}

MRESULT MTKWarpExt::WarpFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut)
{
    LOGD("MTKWarp WarpFeatureCtrl \n");
    if((pParaIn!=NULL) ||(pParaOut!=NULL)||(FeatureID > 0))
    	LOGD("...\n");    
    return E_WARP_NEED_OVER_WRITE;
}
