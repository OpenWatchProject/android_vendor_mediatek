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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
 /*
** $Log: hr_hal.cpp $
 *
*/
#define LOG_TAG "mHalHR"

#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/atomic.h>
#include "hr_hal.h"
#include <mtkcam/Log.h>

#include "MTKHrd.h"
#include "MTKHrdErrCode.h"
#include "MTKHrdType.h"
#include "camera_custom_hrd.h"

#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
using namespace android;

#define DUMP_IMAGE (0)
#define HRD_ENABLE (1)

//****************************//

//****************************//

#define MHAL_NO_ERROR 0
#define MHAL_INPUT_SIZE_ERROR 1
#define MHAL_INIT_ERROR 2

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

//-------------------------------------------//
//  Global heart rate related parameter  //
//-------------------------------------------//
//
static halHRBase *pHalHR = NULL;
volatile MINT32     mHRCount = 0;
static Mutex       gLock;

//int g_count = 0;

/*******************************************************************************
*
********************************************************************************/
halHRBase*
halHR::
getInstance()
{
    Mutex::Autolock _l(gLock);
    MY_LOGD("[Create] &mHRCount:%p &gLock:%p", &mHRCount, &gLock);
    int const oldCount = ::android_atomic_inc(&mHRCount);

    MY_LOGD("[Create] oldCount:%d->%d pHalHR:%p", oldCount, mHRCount, pHalHR);
    if  ( 0 == oldCount )
    {
        if  ( ! pHalHR )
        {
            MY_LOGW("Get Instance Warning!");
        }
        pHalHR = new halHR();
    }
    return  pHalHR;

}

/*******************************************************************************
*
********************************************************************************/
void
halHR::
destroyInstance()
{
    Mutex::Autolock _l(gLock);
    MY_LOGD("[Delete] &mHRCount:%p &gLock:%p", &mHRCount, &gLock);
    int const oldCount = ::android_atomic_dec(&mHRCount);
    MY_LOGD("[Delete] count:%d->%d pHalHR:%p", oldCount, mHRCount, pHalHR);

    if  ( 1 == oldCount )
    {
        delete pHalHR;
        pHalHR = NULL;
    }

}

/*******************************************************************************
*
********************************************************************************/
halHR::halHR()
{
    m_pMTKHRObj = NULL;

    m_HRW = 0;
    m_HRH = 0;

    //MY_LOGD("MTK_HR_Lib version is %s", m_pMTKHRObj->getVersion());
}


halHR::~halHR()
{
    // TODO: Destroy heart rate object
#if (HRD_ENABLE)
    if (m_pMTKHRObj) {
        m_pMTKHRObj->destroyInstance(m_pMTKHRObj);
    }
#endif
    m_pMTKHRObj = NULL;
}


MINT32
halHR::halHRInit(
    MUINT32 HRW,
    MUINT32 HRH,
    MUINT8* WorkingBuffer,
    MUINT32 debug
)
{
//**********************************************************************//
    MY_LOGD("[mHalHRInit] Start");
    HRD_Customize_PARA CustomData;

    get_hrd_CustomizeData(&CustomData);
#if (HRD_ENABLE)
    m_pMTKHRObj = MTKHrd::createInstance();

    m_HRW = HRW;
    m_HRH = HRH;
    m_mode = HRD_PROC_MODE_UNKNOWN;

    m_HRInitData.ImageWidth = m_HRW;
    m_HRInitData.ImageHeight = m_HRH;
    m_HRInitData.ImgFmt = HRD_YUV422_Packed;
    m_HRInitData.debug = debug;

    #if 1
    m_HRInitData.HrdTuningData.OneShotSeconds = CustomData.OneShotSeconds;
    m_HRInitData.HrdTuningData.GoodQualityTh = CustomData.GoodQualityTh;
    m_HRInitData.HrdTuningData.LargeSadResetTh = CustomData.LargeSadResetTh;
    m_HRInitData.HrdTuningData.LargeMoveResetTh = CustomData.LargeMoveResetTh;
    m_HRInitData.HrdTuningData.MinFaceSize = CustomData.MinFaceSize;
    #else
    m_HRInitData.HrdTuningData.OneShotSeconds = 20;
    m_HRInitData.HrdTuningData.GoodQualityTh = 350;
    m_HRInitData.HrdTuningData.LargeSadResetTh = 20;
    m_HRInitData.HrdTuningData.LargeMoveResetTh = 4;
    m_HRInitData.HrdTuningData.MinFaceSize = 64;
    #endif

    if(m_pMTKHRObj->HrdInit(&m_HRInitData) != S_HRD_OK) {
        MY_LOGE("[mHalHRInit] startProcess failed");
        return MHAL_INIT_ERROR;
    }
    //  set algorithm parameters (optional)
    m_pMTKHRObj->HrdFeatureCtrl(HRD_FEATURE_GET_PROC_INFO, NULL, &m_HRProcInfo);
    MY_LOGD("Heart rate algo working buffer size : %d", m_HRProcInfo.ExtMemSize);
    m_HRWorkbufInfo.extMemSize = m_HRProcInfo.ExtMemSize;
    m_pWorkingBuf = new unsigned char[m_HRWorkbufInfo.extMemSize];
    m_HRWorkbufInfo.extMemStartAddr = (void *)m_pWorkingBuf;

    m_pMTKHRObj->HrdFeatureCtrl(HRD_FEATURE_SET_WORK_BUF_INFO, &m_HRWorkbufInfo, NULL);
#endif
    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
halHR::halHRDo(
MUINT8 *ImageBuffer, // YUV422 buffer
MINT32 rRotation_Info,
MUINT64 timestamp,
MINT32 face_num,
MtkCameraFace face,
MUINT32 mode,
MUINT32 DoReset
)
{
#if (HRD_ENABLE)
    if(DoReset || (mode != m_mode && m_mode != HRD_PROC_MODE_UNKNOWN)) {
        MY_LOGD("HRDTest reset mode: %d", mode);
        m_pMTKHRObj->HrdReset();
    }
    m_mode = mode;
    if(face_num) {
        m_HRDataInfo.FaceX0 = face.rect[0];
        m_HRDataInfo.FaceY0 = face.rect[1];
        m_HRDataInfo.FaceX1 = face.rect[2];
        m_HRDataInfo.FaceY1 = face.rect[3];
        m_HRDataInfo.FaceX0 = ((m_HRDataInfo.FaceX0 + 1000) * m_HRW) / 2000;
        m_HRDataInfo.FaceX1 = ((m_HRDataInfo.FaceX1 + 1000) * m_HRW) / 2000;
        m_HRDataInfo.FaceY0 = ((m_HRDataInfo.FaceY0 + 1000) * m_HRH) / 2000;
        m_HRDataInfo.FaceY1 = ((m_HRDataInfo.FaceY1 + 1000) * m_HRH) / 2000;
    } else {
        m_HRDataInfo.FaceX0 = 0;
        m_HRDataInfo.FaceY0 = 0;
        m_HRDataInfo.FaceX1 = 0;
        m_HRDataInfo.FaceY1 = 0;
    }
    MY_LOGD("HRDTest input face number: %d", face_num);
    //MY_LOGD("HRDTest input timestamp: %d", (MUINT32)(timestamp/1000000));
    m_HRDataInfo.DetectMode = HRD_DETECT_MODE_CONTINUOUS;
    m_HRDataInfo.ProcMode = (HRD_PROC_MODE_ENUM)m_mode;

    m_pMTKHRObj->HrdFeatureCtrl(HRD_FEATURE_SET_PROC_INFO, &m_HRDataInfo, NULL);

    m_pMTKHRObj->HrdMain(ImageBuffer, (MUINT32)(timestamp/1000000));

    MY_LOGD("HRDTest input halHRDo Done");
#endif
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halHR::halHRUninit(
)
{
#if (HRD_ENABLE)
    if (m_pMTKHRObj) {
        m_pMTKHRObj->HrdReset();
        if(m_HRInitData.debug)
            m_pMTKHRObj->HrdFeatureCtrl(HRD_FEATURE_SAVE_LOG, NULL, NULL);
        m_pMTKHRObj->destroyInstance(m_pMTKHRObj);
        m_pMTKHRObj = NULL;
    }

    delete [] m_pWorkingBuf;
    m_pWorkingBuf = NULL;
    m_HRWorkbufInfo.extMemStartAddr = NULL;
#endif
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/

MINT32
halHR::halHRGetResult(
    HR_RESULT *result
)
{
#if (HRD_ENABLE)
    m_pMTKHRObj->HrdFeatureCtrl(HRD_FEATURE_GET_RESULT_INFO, NULL, &m_HRresult);
    result->x1 = m_HRresult.heartRateROI.x1;
    result->y1 = m_HRresult.heartRateROI.y1;
    result->x2 = m_HRresult.heartRateROI.x2;
    result->y2 = m_HRresult.heartRateROI.y2;
    result->value = m_HRresult.heartRateValue_PostProcessed;//m_HRresult.heartRateValue;
    result->quality = m_HRresult.heartRateQuality;
    result->isvalid = m_HRresult.heartRateIsValid;
    result->hasPP = m_HRresult.heartRateValue_PostProcessed;
    result->percentage = m_HRresult.heartRateDetectProgressPercentage;
    result->stoptype = m_HRresult.stopType;
    result->aiWaveform = m_HRresult.aiWaveform;
    MY_LOGD("HRDTest output value: %d", m_HRresult.heartRateValue);
    MY_LOGD("HRDTest output quality: %d", m_HRresult.heartRateQuality);
    MY_LOGD("HRDTest output percentage: %d", m_HRresult.heartRateDetectProgressPercentage);
    MY_LOGD("HRDTest output stopType: %d", m_HRresult.stopType);
#endif
    return MHAL_NO_ERROR;
}



