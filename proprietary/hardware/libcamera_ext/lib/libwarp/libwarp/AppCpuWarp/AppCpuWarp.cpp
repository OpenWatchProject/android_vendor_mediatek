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

#define LOG_TAG "AppCpuWarp"

#define MTK_LOG_ENABLE 1
#include <string.h>
#include "AppCpuWarp.h"
#include "coreCpuWarp.h"


#ifdef SIM_MAIN
#include <stdio.h>
#ifdef DEBUG
#define LOGD printf
#else
#define LOGD(...)
#endif
#else
#include <sys/time.h>
#include <android/log.h>
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,##__VA_ARGS__) 
#endif

/* profling */
#define PROF_NUM (4)
#if defined(TIME_PROF) && !defined(SIM_MAIN)
#include <sys/time.h>
static timeval start_time, end_time;
static MINT32 elapse_time[PROF_NUM] = {0};
#define MY_GET_TIME gettimeofday
#define MY_DIFF_TIME time_diff
#define MY_DISPLAY_TIME time_display
static void time_diff(MINT32 *elapse_t, timeval start_t, timeval end_t)
{
    *elapse_t = (end_t.tv_sec - start_t.tv_sec)*1000000 + (end_t.tv_usec - start_t.tv_usec);
}
static void time_display(MINT32 elapse_t, const char *string)
{
    printf("%s: %8d(us) = %5.4f(ms)\n", string, elapse_t, elapse_t/1000.0f);
}
#else
//static MINT32 start_time, end_time;
//static MINT32 elapse_time[PROF_NUM] = {0};
#define MY_GET_TIME(...)
#define MY_DIFF_TIME(...)
#define MY_DISPLAY_TIME(...)
#endif



/*******************************************************************************
* Global Define
********************************************************************************/
/*
static WARP_STATE_ENUM      WarpState;
//static MUINT32              gWarpCurrImageNum;
//static MUINT32              gWarpCurrProcNum;
static WarpImageExtInfo     WarpImageInfo;
static WarpResultInfo       WarpResultInfo;
static MUINT32              WarpSetBufferSize = WARP_BUFFER_UNSET;
static MUINT32              WarpReqBufferSize;
*/

/*******************************************************************************
*  Anti-Clone
********************************************************************************/
/*
#ifndef SIM_MAIN
extern "C"
{
extern int drvb_f0(void);
}
#endif
*/
/*******************************************************************************
*
********************************************************************************/
MTKWarpExt*
AppCpuWarp::
getInstance()
{
    MTKWarpExt *pAppCpuWarp = NULL;
    LOGD("[%s] getInstance \n", LOG_TAG);
    if (pAppCpuWarp == NULL) {
        pAppCpuWarp = new AppCpuWarp();
    }
/*
#ifndef SIM_MAIN
    int r_anti_clone = drvb_f0();
#endif
*/
    return pAppCpuWarp;
}

/*******************************************************************************
*
********************************************************************************/
void
AppCpuWarp::
destroyInstance(MTKWarpExt* pAppCpuWarp)
{
    if (pAppCpuWarp) {
        delete pAppCpuWarp;
    }
    //pAppCpuWarp = NULL;
}

/*******************************************************************************
*
********************************************************************************/
AppCpuWarp::AppCpuWarp()
{
    LOGD("[%s] constructor\n", LOG_TAG);
    WarpSetBufferSize = WARP_BUFFER_UNSET;
    WarpState = WARP_STATE_STANDBY;
}

/*******************************************************************************
*
********************************************************************************/
AppCpuWarp::~AppCpuWarp()
{
    LOGD("[%s] destructor\n", LOG_TAG);
}

MRESULT AppCpuWarp::WarpInit(MUINT32 *InitInData, MUINT32 *InitOutData)
{
    LOGD("[%s] WarpInit \n", LOG_TAG);

    CPU_WARP_IMG_EXT_INFO* core = &warp_obj.core_info;
    MRESULT RetCode = S_WARP_OK;
    //gWarpCurrImageNum = 0;
    //gWarpImageInfo.WorkingBuffAddr = (MUINT32)InitInData;
    //LOGD("[%s] WorkingBuffAddr 0x%x \n", LOG_TAG, gWarpImageInfo.WorkingBuffAddr);
    WarpState = WARP_STATE_INIT;
    WarpImageExtInfo* input = (WarpImageExtInfo*) InitInData;

    core->Features = input->Features;
    core->ImgFmt = (CPU_WARP_IMAGE_ENUM)input->ImgFmt;
    core->OutImgFmt =  (CPU_WARP_IMAGE_ENUM)input->OutImgFmt;
    //core->SrcGraphicBuffer = (void*) input->SrcGraphicBuffer;
    //core->DstGraphicBuffer = (void*) input->DstGraphicBuffer;
    core->Width = input->Width;
    core->Height = input->Height;
    core->WarpMapNum = input->WarpMapNum;
    core->WarpMatrixNum = input->WarpMatrixNum;
    //core->InputGBNum = input->InputGBNum; //input GB queue
    //core->OutputGBNum = input->OutputGBNum; //output GB queue
    core->MaxWarpMapSize[0] = input->MaxWarpMapSize[0];
    core->MaxWarpMapSize[1] = input->MaxWarpMapSize[1];
    //core->demo = input->Demo; //enable EIS demo mode

    //core->TuningPara.demo = input->pTuningPara->Demo;
    //core->TuningPara.GLESVersion = input->pTuningPara->GLESVersion;

    //create GPU GLES conrext
    //context_obj.GPUContextCreate((MTK_GPU_CONTEXT_VERSION_ENUM)core->TuningPara.GLESVersion);
    //get gpu display handler
    //context_obj.GPUContextGetDisplayHandle(warp_obj.gles_display);
    //initial GPU warping
    warp_obj.CpuWarpingInit();


    WarpResult.RetCode = CORE_OK;
    InitOutData = NULL;
    //memset(WarpResultInfo.ElapseTime, 0, 3*sizeof(MINT32));
    return RetCode;
}

MRESULT AppCpuWarp::WarpMain(void)
{
    LOGD("[%s] WarpMain \n", LOG_TAG);
    WarpState = WARP_STATE_PROC;
    MRESULT RetCode = S_WARP_OK;

    if(WarpState == WARP_STATE_PROC)
    {
        LOGD("[%s] WarpMain Go\n", LOG_TAG);

        // buffer check
        WarpReqBufferSize = warp_obj.core_info.Width * warp_obj.core_info.Height * 2 + 2048;
        if ((WarpSetBufferSize != WARP_BUFFER_UNSET) && (WarpSetBufferSize < WarpReqBufferSize))
        {
            RetCode = E_WARP_INSUFF_WORK_BUF;
            return RetCode;
        }

        warp_obj.CpuWarpingMain();


    }
    else
    {
        return E_WARP_WRONG_STATE;
    };

    //gWarpState = WARP_STATE_PROC_READY;
    return S_WARP_OK;
}

MRESULT AppCpuWarp::WarpReset(void)
{
    LOGD("[%s] WarpReset \n", LOG_TAG);
    WarpState = WARP_STATE_STANDBY;
    //gWarpCurrImageNum = 0;
    WarpSetBufferSize = WARP_BUFFER_UNSET;

    //reset GPU warping
    warp_obj.CpuWarpingReset();
    //destroy GPU GLES context
    //context_obj.GPUContextDestroy();


    return S_WARP_OK;
}

MRESULT AppCpuWarp::WarpFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut)
{
    MRESULT RetCode = S_WARP_OK;
    LOGD("[%s] WarpFeatureCtrl \n", LOG_TAG);

    WarpImageExtInfo *ImgInfo = (WarpImageExtInfo *)pParaIn;
    MUINT32 *pGetBufferSize = (MUINT32 *)pParaOut;
    MUINT32 *pSetBufferAddr = (MUINT32 *)pParaIn;

    switch ( FeatureID )
    {
    case WARP_FEATURE_ADD_IMAGE:
        LOGD("[%s] WarpFeatureCtrl(WARP_FEATURE_ADD_IMAGE)\n", LOG_TAG);
        if(  // Illegal image number
            ImgInfo->Width==0  ||
            ImgInfo->Height==0
          )
        {
            RetCode = E_WARP_WRONG_CMD_PARAM;
        }
        else
        {
            CPU_WARP_IMG_EXT_INFO* core = &warp_obj.core_info;
            //core->WorkingBuffAddr = WarpImageInfo.WorkingBuffAddr;

            core->ClipWidth = ImgInfo->ClipWidth;
            core->ClipHeight = ImgInfo->ClipHeight;
            core->Width = ImgInfo->Width;
            core->Height = ImgInfo->Height;


            for(unsigned int i=0; i<core->WarpMatrixNum; i++)
            {
                memcpy(core->Hmtx[i], ImgInfo->Hmtx[i], sizeof(float)*9);
                core->ClipX[i] = ImgInfo->ClipX[i];
                core->ClipY[i] = ImgInfo->ClipY[i];
            }
            for(unsigned int i=0; i<core->WarpMapNum; i++)
            {
                core->WarpMapSize[i][0]= ImgInfo->WarpMapSize[i][0];
                core->WarpMapSize[i][1]= ImgInfo->WarpMapSize[i][1];
                core->WarpMapAddr[i][0] = ImgInfo->WarpMapAddr[i][0];
                core->WarpMapAddr[i][1] = ImgInfo->WarpMapAddr[i][1];
            }

            core->WarpLevel = ImgInfo->WarpLevel;//EnlargeEyeLevel

            core->SrcBuffer = ImgInfo->SrcGraphicBuffer;
            core->DstBuffer = ImgInfo->DstGraphicBuffer;

        }
        break;
    case WARP_FEATURE_GET_RESULT:
        LOGD("[%s] WarpFeatureCtrl(WARP_FEATURE_GET_RESULT)\n", LOG_TAG);
        memcpy(pParaOut, &WarpResult, sizeof(WarpResult));
        break;
    case WARP_FEATURE_GET_WORKBUF_SIZE:
        WarpSetBufferSize = warp_obj.core_info.Width * warp_obj.core_info.Height * 2 + 2048;
        *pGetBufferSize = WarpSetBufferSize;
        LOGD("[%s] WarpFeatureCtrl(WARP_FEATURE_GET_WORKBUF_SIZE): BufferSize = %d\n", LOG_TAG, WarpSetBufferSize);
        break;
    case WARP_FEATURE_SET_WORKBUF_ADDR:
        warp_obj.core_info.WorkingBuffAddr = *pSetBufferAddr;
        LOGD("[%s] WarpFeatureCtrl(WARP_FEATURE_SET_WORKBUF_ADDR): BufferAddr = 0x%x\n", LOG_TAG, MUINT32(*pSetBufferAddr));
        break;
    case WARP_FEATURE_GET_LOG:
        // TBD
        break;
    default:
        RetCode = E_WARP_WRONG_CMD_ID;
    }

    return RetCode;
}

