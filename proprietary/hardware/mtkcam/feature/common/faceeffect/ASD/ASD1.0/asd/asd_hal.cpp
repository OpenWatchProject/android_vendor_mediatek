/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "mHalAsd"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "../inc/MediaLog.h"
//#include "../inc/MediaAssert.h"
#include <faces.h>
#include <mtkcam/utils/std/Log.h>

#include <utils/Mutex.h>
#include <cutils/atomic.h>

#include "AppAsd.h"
#include "asd_hal.h"
#include "camera_custom_asd.h"
#include <mtkcam/feature/ASD/asd_hal_base.h>
#include <cutils/properties.h>

using namespace android;


//#define   ASDdebug
#define   ASD_IMAGE_WIDTH  160
#define   ASD_IMAGE_HEIGHT 120
#define   ASD_ORT          4
/*******************************************************************************
*
********************************************************************************/
#ifdef ASDdebug
MUINT32 gfilecun=0;
#endif
volatile MINT32     gASDCount = 0;
static Mutex       gsLock;

static halASDBase *pHalASD = NULL;
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/*******************************************************************************
*
********************************************************************************/
halASDBase*
halASD::
getInstance()
{
    Mutex::Autolock _l(gsLock);
    MY_LOGD("[Create] &gASDCount:%p &gsLock:%p", &gASDCount, &gsLock);
    int const oldCount = ::android_atomic_inc(&gASDCount);

    int g_oldCount = oldCount;
    if(g_oldCount < 0)
    {
        MY_LOGW("Warning! [Create] count:%d->%d pHalASD:%p", oldCount, gASDCount, pHalASD);
        g_oldCount = 0;
        gASDCount = 1;
        pHalASD = NULL;
    }

    if  ( 0 == oldCount )
    {
        if  ( ! pHalASD )
        {
            MY_LOGW("Get Instance Warning!");
        }
        pHalASD = new halASD();
    }
    MY_LOGD("[Create] g_oldCount:%d->%d pHalASD:%p", g_oldCount, gASDCount, pHalASD);
    return  pHalASD;
}

/*******************************************************************************
*
********************************************************************************/
void
halASD::
destroyInstance()
{
    Mutex::Autolock _l(gsLock);
    MY_LOGD("[Delete] &gASDCount:%p &gsLock:%p", &gASDCount, &gsLock);
    int const oldCount = ::android_atomic_dec(&gASDCount);
    MY_LOGD("[Delete] count:%d->%d pHalASD:%p", oldCount, gASDCount, pHalASD);
    if  ( 0 == oldCount || ! pHalASD )
    {
        MY_LOGW("Delete Instance Warning!");
    }

    if  ( 1 == oldCount )
    {
        delete pHalASD;
        pHalASD = NULL;
    }
}

/*******************************************************************************
*
********************************************************************************/
halASD::halASD()
{
    mpMTKAsdObj = NULL;
    MY_LOGD("[halASD] Reset mCount:%d \n", mCount);
    mCount = 0;
    mScaleFactor = 1;
    mHalASDDumpOPT = 0;
}

/*******************************************************************************
*
********************************************************************************/
halASD::~halASD()
{
}

MINT32 halASD::mHalAsdInit(void* AAAData,void* working_buffer,MUINT8 SensorType, MINT32 Asd_Buf_Width, MINT32 Asd_Buf_Height)
{
#if (MTKCAM_BASIC_PACKAGE != 1)
    ASD_Customize_PARA1 ASDThres1;
    ASD_Customize_PARA2 ASDThres2;
    mCount++;
    MY_LOGD("[halASD]  mCount++:%d \n", mCount);
    AAA_ASD_PARAM* rASDInfo=(AAA_ASD_PARAM*)AAAData;
    MINT32 Retcode = S_ASD_OK;
    MUINT32* AFtable=(MUINT32*)malloc((rASDInfo->i4AFTableIdxNum + 1)*sizeof(MUINT32));
    //********Binchang 20110810 Add ASD Debug Opition****************//
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.ASD.debug.dump", value, "0");
    mHalASDDumpOPT = atoi(value);

    mMyAsdInitInfo.pInfo = &mMyAsdEnvInfo;
    mMyAsdInitInfo.pDeciderInfo = &mMyDeciderEnvInfo;
    mMyAsdInitInfo.pDeciderTuningInfo = &mMyDeciderTuningInfo;

    mMyAsdInitInfo.pInfo->ext_mem_start_addr=working_buffer;
    if (Asd_Buf_Width > ASD_IMAGE_WIDTH || Asd_Buf_Height > ASD_IMAGE_HEIGHT) {
        // down scale input image
        mMyAsdInitInfo.pInfo->image_width =Asd_Buf_Width/2;
        mMyAsdInitInfo.pInfo->image_height=Asd_Buf_Height/2;
        mScaleFactor = 2;
    } else {
        mMyAsdInitInfo.pInfo->image_width =Asd_Buf_Width;
        mMyAsdInitInfo.pInfo->image_height=Asd_Buf_Height;
        mScaleFactor = 1;
    }

    if (mMyAsdInitInfo.pInfo->image_width * 3 < mMyAsdInitInfo.pInfo->image_height * 4) {
        // ASD algo only support 4:3
        mMyAsdInitInfo.pInfo->image_height = mMyAsdInitInfo.pInfo->image_width * 3 / 4;
    }
    mMyAsdInitInfo.pInfo->asd_tuning_data.num_of_ort=ASD_ORT;                 // num_of_ort, drfault = 4

    MY_LOGD("[mHalAsdInit] ASD w = %d, h = %d",mMyAsdInitInfo.pInfo->image_width, mMyAsdInitInfo.pInfo->image_height);

    if(SensorType)
    {
        MY_LOGD("[YUV Sensor] \n");
        mMyAsdInitInfo.pDeciderInfo->DeciderInfoVer = ASD_DECIDER_INFO_SRC_YUV;
    }
    else
    {
        MY_LOGD("[SP_RAW_Sensor] \n");
        mMyAsdInitInfo.pDeciderInfo->DeciderInfoVer = ASD_DECIDER_INFO_SRC_SP_RAW;
    }

    mMyAsdInitInfo.pDeciderInfo->RefAwbD65Rgain = rASDInfo->i4AWBRgain_D65_X128;                    // reference 3A info, get from sensor or AWB data
    mMyAsdInitInfo.pDeciderInfo->RefAwbD65Bgain = rASDInfo->i4AWBBgain_D65_X128;                    // reference 3A info, get from sensor or AWB data
    mMyAsdInitInfo.pDeciderInfo->RefAwbCwfRgain = rASDInfo->i4AWBRgain_CWF_X128;                    // reference 3A info, get from sensor or AWB data
    mMyAsdInitInfo.pDeciderInfo->RefAwbCwfBgain = rASDInfo->i4AWBBgain_CWF_X128;                    // reference 3A info, get from sensor or AWB data
    *AFtable=rASDInfo->i4AFTableMacroIdx;
    memcpy((AFtable+1),rASDInfo->pAFTable,(rASDInfo->i4AFTableIdxNum*sizeof(MUINT32)));

    MY_LOGD("[mHalAsdInit] i4AFTableMacroIdx %d i4AFTableIdxNum %d i4AFTableOffset %d\n",rASDInfo->i4AFTableMacroIdx,rASDInfo->i4AFTableIdxNum,rASDInfo->i4AFTableOffset);

    for(int i=1;i<rASDInfo->i4AFTableIdxNum+1;i++)
    {
        *(AFtable+i) =  *(AFtable+i) + rASDInfo->i4AFTableOffset;
        MY_LOGD("[mHalAsdInit] After (Offset) AF table %d \n",*(AFtable+i));
    }

    if(rASDInfo->i4AFTableMacroIdx != 0)
        mMyAsdInitInfo.pDeciderInfo->RefAfTbl = (void *)AFtable;
    else
        mMyAsdInitInfo.pDeciderInfo->RefAfTbl = NULL;

    mMyAsdInitInfo.pDeciderTuningInfo = 0;                      // use default value

    if (mpMTKAsdObj) {
        Retcode = E_ASD_ERR;
        MY_LOGD("[v] Err, Init has been called \n");
    }

    /*  Create MTKPano Interface  */
    if(mpMTKAsdObj == NULL)
    {
        mpMTKAsdObj = MTKAsd::createInstance(DRV_ASD_OBJ_SW);
        MY_LOGW_IF(mpMTKAsdObj == NULL, "Err");
    }

//**********************************************************************************************************************************//
#if (0)
    get_asd_CustomizeData1(&ASDThres1);
    get_asd_CustomizeData2(&ASDThres2);

    mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightBlAe = ASDThres2.s2IdxWeightBlAe;
    mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightBlScd= ASDThres2.s2IdxWeightBlScd;
    mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightLsAe = ASDThres2.s2IdxWeightLsAe;
    mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightLsAwb= ASDThres2.s2IdxWeightLsAwb;
    mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightLsAf = ASDThres2.s2IdxWeightLsAf;
    mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightLsScd= ASDThres2.s2IdxWeightLsScd;
    mMyAsdInitInfo.pDeciderTuningInfo->TimeWeightType = (ASD_DECIDER_TIME_WEIGHT_TYPE_ENUM) ASDThres1.u1TimeWeightType;
    mMyAsdInitInfo.pDeciderTuningInfo->TimeWeightRange = (ASD_DECIDER_TIME_WEIGHT_RANGE_ENUM) ASDThres1.u1TimeWeightRange;
    mMyAsdInitInfo.pDeciderTuningInfo->EvLoThrNight = ASDThres2.s2EvLoThrNight;
    mMyAsdInitInfo.pDeciderTuningInfo->EvHiThrNight = ASDThres2.s2EvHiThrNight;
    mMyAsdInitInfo.pDeciderTuningInfo->EvLoThrOutdoor= ASDThres2.s2EvLoThrOutdoor;
    mMyAsdInitInfo.pDeciderTuningInfo->EvHiThrOutdoor= ASDThres2.s2EvHiThrOutdoor;
    mMyAsdInitInfo.pDeciderTuningInfo->ScoreThrNight = ASDThres1.u1ScoreThrNight;
    mMyAsdInitInfo.pDeciderTuningInfo->ScoreThrBacklit = ASDThres1.u1ScoreThrBacklit;
    mMyAsdInitInfo.pDeciderTuningInfo->ScoreThrPortrait = ASDThres1.u1ScoreThrPortrait;
    mMyAsdInitInfo.pDeciderTuningInfo->ScoreThrLandscape = ASDThres1.u1ScoreThrLandscape;
    mMyAsdInitInfo.pDeciderTuningInfo->BacklitLockEnable = ASDThres2.boolBacklitLockEnable;
    mMyAsdInitInfo.pDeciderTuningInfo->BacklitLockEvDiff= ASDThres2.s2BacklitLockEvDiff;

    gMyAsdInitInfo.pDeciderTuningInfo->bReserved= 0;
    gMyAsdInitInfo.pDeciderTuningInfo->pReserved= NULL;

    //HDR init para
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrTimeWeightType = (ASD_DECIDER_TIME_WEIGHT_TYPE_ENUM)1;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrTimeWeightRange = (ASD_DECIDER_TIME_WEIGHT_RANGE_ENUM)10;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrIdxWeightBlAe = 1;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrIdxWeightBlScd = 8;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrScoreThrBacklit = 50;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrBacklitLockEnable = true;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.HdrBacklitLockEvDiff = -10;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.bReserved = 0;
    gMyAsdInitInfo.pDeciderTuningInfo->HdrIdxTuning.pReserved = NULL;

    MY_LOGD("ASD_Custom:  IdxWeightBlAe(1):%d, IdxWeightBlScd(8):%d, EvLoThrNight(50):%d \n", mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightBlAe, mMyAsdInitInfo.pDeciderTuningInfo->IdxWeightBlScd,
                   mMyAsdInitInfo.pDeciderTuningInfo->EvLoThrNight);
#endif
    mpMTKAsdObj->AsdInit(&mMyAsdInitInfo, 0);

    if (AFtable) {
        free(AFtable);
    }
    return Retcode;
#else
    return S_ASD_OK;
#endif
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halASD::mHalAsdUnInit( )
{
#if (MTKCAM_BASIC_PACKAGE != 1)
    MY_LOGD("[mHalAsdUnInit] \n");

    if(mHalASDDumpOPT)
    {
        MY_LOGD("[mHalAsdUnInit] Save log \n");
        mpMTKAsdObj->AsdFeatureCtrl(ASD_FEATURE_SAVE_DECIDER_LOG_INFO, &mCount, 0);
    }
    if (mpMTKAsdObj) {
        mpMTKAsdObj->destroyInstance();
    }
    mpMTKAsdObj = NULL;

    MY_LOGD("[mHalAsdUnInit] OK\n");
#endif
    return S_ASD_OK;
}

MINT32 halASD::mHalAsdDoSceneDet(void* src, MUINT16 imgw, MUINT16 imgh)
{
#if (MTKCAM_BASIC_PACKAGE != 1)
    MINT32 Retcode = S_ASD_OK;
    MUINT16* img = (MUINT16*)src;
    MUINT32 count=0;
    ASD_SCD_INFO_STRUCT gMyAsdInfo;

    if (mScaleFactor != 1) {
        for(int i=0;i<imgh;i+=mScaleFactor)
        {
            for(int j=0;j<imgw;j+=mScaleFactor)
            {
                *(img+count)=*(img+(i*imgw)+j);
                count++;
            }
        }
    }
    #ifdef ASDdebug
    dumpASDInputImage(src, imgw/mScaleFactor, imgh/mScaleFactor, gfilecun);
    gfilecun++;
    #endif
    gMyAsdInfo.src_buffer_addr = src;
    if(mpMTKAsdObj == NULL)
    {
        mpMTKAsdObj = MTKAsd::createInstance(DRV_ASD_OBJ_SW);
        MY_LOGW_IF(mpMTKAsdObj == NULL, "Err");
    }
    mpMTKAsdObj->AsdMain(ASD_PROC_MAIN, &gMyAsdInfo);

    return Retcode;
#else
    return S_ASD_OK;
#endif
}

MINT32 halASD::mHalAsdDecider(void* AAAData,MINT32 Face_Num,mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM &Scene)
{
#if (MTKCAM_BASIC_PACKAGE != 1)
    MINT32 Retcode = S_ASD_OK;
    AAA_ASD_PARAM* rASDInfo=(AAA_ASD_PARAM*)AAAData;
    ASD_DECIDER_RESULT_STRUCT MyDeciderResult;
    ASD_SCD_RESULT_STRUCT MyAsdResultInfo;
    ASD_DECIDER_INFO_STRUCT MyDeciderInfo;

    // set Face Info
    MyDeciderInfo.InfoFd.FdFaceNum=Face_Num;
    MyDeciderInfo.InfoFd.FdMainFaceX0=0;  // for positive > 0
    MyDeciderInfo.InfoFd.FdMainFaceY0=0;  // for positive > 0
    MyDeciderInfo.InfoFd.FdMainFaceX1=0;  // for positive > 0
    MyDeciderInfo.InfoFd.FdMainFaceY1=0;  // for positive > 0
    MyDeciderInfo.InfoFd.FdMainFacePose=0;
    MyDeciderInfo.InfoFd.FdMainFaceLuma = 0;

    // Set 3A info
    MyDeciderInfo.InfoAaa.AeEv = rASDInfo->i4AELv_x10;                     // reference 3A info, get from sensor or AWB data
    MyDeciderInfo.InfoAaa.AeFaceEnhanceEv = rASDInfo->i2AEFaceDiffIndex;   // reference 3A info, get Face AE enhance Ev value */
    MyDeciderInfo.InfoAaa.AeIsBacklit = rASDInfo->bAEBacklit;              // reference 3A info, get from sensor or AWB data
    MyDeciderInfo.InfoAaa.AeIsStable = rASDInfo->bAEStable;                // reference 3A info, get from sensor or AWB data
    MyDeciderInfo.InfoAaa.AwbCurRgain = rASDInfo->i4AWBRgain_X128;         // reference 3A info, get from sensor or AWB data
    MyDeciderInfo.InfoAaa.AwbCurBgain = rASDInfo->i4AWBBgain_X128;         // reference 3A info, get from lens or AF data
    MyDeciderInfo.InfoAaa.AwbIsStable = rASDInfo->bAWBStable;
    MyDeciderInfo.InfoAaa.AfPosition = rASDInfo->i4AFPos;
    MyDeciderInfo.InfoAaa.AfIsStable = rASDInfo->bAFStable;

    mpMTKAsdObj->AsdFeatureCtrl(ASD_FEATURE_GET_RESULT, 0, &MyAsdResultInfo);
    memcpy(&(MyDeciderInfo.InfoScd),&MyAsdResultInfo, sizeof(ASD_SCD_RESULT_STRUCT));

    mpMTKAsdObj->AsdMain(ASD_PROC_DECIDER, &MyDeciderInfo);
    mpMTKAsdObj->AsdFeatureCtrl(DECIDER_FEATURE_GET_RESULT, 0, &MyDeciderResult);
    MY_LOGD("[mHalAsdDecider] detect Scene is %d, Face Num:%d \n",MyDeciderResult.DeciderUiScene, MyDeciderInfo.InfoFd.FdFaceNum);

    Scene=(mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM) MyDeciderResult.DeciderUiScene;

    return Retcode;
#else
    return S_ASD_OK;
#endif
}

void halASD::dumpASDInputImage(void *src, MUINT16 width, MUINT16 height, MUINT32 count)
{
    char szFileName[100];
    sprintf(szFileName, "/sdcard/ASDResize%04d_%04d_%04d.raw", width, height,count);
    FILE * pRawFp = fopen(szFileName, "wb");

    if (NULL == pRawFp )
    {
     MY_LOGE("Can't open file to save RAW Image\n");
     while(1);
    }

    int i4WriteCnt = fwrite((void *)src, 2, ( width * height * 1), pRawFp);
    fflush(pRawFp);
    fclose(pRawFp);
}


