#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"

#include <cutils/properties.h>
#include <sched.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVenc"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

#define CHECK_OMX_PARAM(x) \
    if (!checkOMXParams(x)) { \
        MTK_OMX_LOGE("invalid OMX header 0x%08x", x);\
        /*return OMX_ErrorBadParameter*/;\
    }

#define RETURN_BADPORT_IF_NOT_PORT(x,p) \
    if (x->nPortIndex != p->nPortIndex) return OMX_ErrorBadPortIndex

#define RETURN_BADPORT_IF_NOT_INPUT(x) \
    if (x->nPortIndex != mInputPortFormat.nPortIndex) return OMX_ErrorBadPortIndex

#define RETURN_BADPORT_IF_NOT_OUTPUT(x) \
    if (x->nPortIndex != mOutputPortFormat.nPortIndex) return OMX_ErrorBadPortIndex

template<typename T>
inline OMX_ERRORTYPE MtkOmxVenc::HandleConfigCopy(T* source, T* dest, OMX_VIDEO_PARAM_PORTFORMATTYPE* port)
{
    CHECK_OMX_PARAM(source);
    if(port) RETURN_BADPORT_IF_NOT_PORT(source, port);

    memcpy(dest, source, sizeof(T));

    return OMX_ErrorNone;
}

/* Set Config Handle */
OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVideoFramerate(OMX_CONFIG_FRAMERATETYPE* pFrameRateType)
{
    HandleConfigCopy(pFrameRateType, &mFrameRateType, &mOutputPortFormat);
    /*CHECK_OMX_PARAM(pFrameRateType);

    RETURN_BADPORT_IF_NOT_OUTPUT(pFrameRateType);

    memcpy(&mFrameRateType, pFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));*/
    mFrameRateUpdated = OMX_TRUE;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVideoBitrate(OMX_VIDEO_CONFIG_BITRATETYPE* pConfigBitrate)
{
    HandleConfigCopy(pConfigBitrate, &mConfigBitrate, &mOutputPortFormat);
    /*CHECK_OMX_PARAM(pConfigBitrate);

    RETURN_BADPORT_IF_NOT_OUTPUT(pConfigBitrate);

    memcpy(&mConfigBitrate, pConfigBitrate, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));*/
    mBitRateUpdated = OMX_TRUE;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVideoIntraVOPRefresh(OMX_CONFIG_INTRAREFRESHVOPTYPE* pConfigIntraRefreshVopType)
{
    HandleConfigCopy(pConfigIntraRefreshVopType, &mConfigIntraRefreshVopType, &mOutputPortFormat);

    /*CHECK_OMX_PARAM(pConfigIntraRefreshVopType);

    RETURN_BADPORT_IF_NOT_OUTPUT(pConfigIntraRefreshVopType);

    memcpy(&mConfigIntraRefreshVopType, pConfigIntraRefreshVopType, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));*/
    mForceIFrame = mConfigIntraRefreshVopType.IntraRefreshVOP;

    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig -> Refresh Vop to %d", mConfigIntraRefreshVopType.IntraRefreshVOP);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVideoAVCIntraPeriod(OMX_VIDEO_CONFIG_AVCINTRAPERIOD* pConfigAVCIntraPeriod)
{
    HandleConfigCopy(pConfigAVCIntraPeriod, &mConfigAVCIntraPeriod, &mOutputPortFormat);
    /*CHECK_OMX_PARAM(pConfigAVCIntraPeriod);

    RETURN_BADPORT_IF_NOT_OUTPUT(pConfigAVCIntraPeriod);

    memcpy(&mConfigAVCIntraPeriod, pConfigAVCIntraPeriod, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));*/
    mIInterval = mConfigAVCIntraPeriod.nPFrames;
    mSetIInterval = OMX_TRUE;

    MTK_OMX_LOGD_ENG("OMX_IndexConfigVideoAVCIntraPeriod MtkOmxVenc::SetConfig -> I interval set to %d", mIInterval);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSetForceIframe(OMX_PARAM_U32TYPE* pSetForceIframeInfo)
{
    mForceIFrame = (OMX_BOOL)pSetForceIframeInfo->nU32;

    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig -> Force I frame set to %d", mForceIFrame);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSetIInterval(OMX_PARAM_U32TYPE* pIntervalInfo)
{
    mIInterval = pIntervalInfo->nU32;
    mSetIInterval = OMX_TRUE;

    MTK_OMX_LOGD_ENG("OMX_IndexVendorMtkOmxVencSetIInterval MtkOmxVenc::SetConfig -> I interval set to %d", mIInterval);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSetForceFullIframe(OMX_PARAM_U32TYPE* pForceFullIInfo)
{
    mForceFullIFrame = (OMX_BOOL)pForceFullIInfo->nU32;
    mForceFullIFramePrependHeader = (OMX_BOOL)pForceFullIInfo->nU32;
    mPrependSPSPPSToIDRFramesNotify = OMX_TRUE;

    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig -> Force Full I frame set to %d, mForceFullIFramePrependHeader: %d, mPrependSPSPPSToIDRFrames: %d",
        mForceFullIFrame, mForceFullIFramePrependHeader, mPrependSPSPPSToIDRFrames);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSkipFrame(OMX_PARAM_U32TYPE* pSkipFrameInfo)
{
    mSkipFrame = pSkipFrameInfo->nU32;

    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig -> Skip frame");

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorDrawBlack(OMX_PARAM_U32TYPE* pDrawBlackInfo)
{
    int enable = pDrawBlackInfo->nU32;
    mDrawBlack = (enable == 0) ? OMX_FALSE : OMX_TRUE;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorInputScaling(OMX_CONFIG_BOOLEANTYPE* pInputScalingMode)
{
    mInputScalingMode = (OMX_BOOL)pInputScalingMode->bEnabled;

    // input scaling only for overspec
    if ( mInputPortDef.format.video.nFrameWidth <  mInputPortDef.format.video.nFrameHeight)
    {
        if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledNarrow && mInputPortDef.format.video.nFrameHeight <= mMaxScaledWide)
        {
            mInputScalingMode = OMX_FALSE;
        }
    } else {
        if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledWide && mInputPortDef.format.video.nFrameHeight <= mMaxScaledNarrow)
        {
            mInputScalingMode = OMX_FALSE;
        }
    }

    MTK_OMX_LOGD_ENG("MtkOmxVenc set mInputScalingMode %d; Scaling target resolution: wide: %d narrow %d", mInputScalingMode, mMaxScaledWide, mMaxScaledNarrow);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorConfigQP(OMX_VIDEO_CONFIG_QP* pConfig)
{
    mSetQP = OMX_TRUE;
    mQP = pConfig->nQP;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigOperatingRate(OMX_PARAM_U32TYPE* pOperationRate)
{
    return SMVR_SetConfigOperatingRate(pOperationRate);
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigGoogleIntraRefresh(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE* pConfigAndroidIntraPeriod)
{
    HandleConfigCopy(pConfigAndroidIntraPeriod, &mConfigAndroidIntraPeriod, &mOutputPortFormat);

    /*RETURN_BADPORT_IF_NOT_OUTPUT(pConfigAndroidIntraPeriod);

    memcpy(&mConfigAndroidIntraPeriod, pConfigAndroidIntraPeriod, sizeof(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE));*/

    mIDRInterval = mConfigAndroidIntraPeriod.nRefreshPeriod; // in frames
    mSetIDRInterval = OMX_TRUE;

    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig -> IDR interval set to %d", mIDRInterval);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigGoogleTemporalLayering(OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE* pLayerConfig)
{
    HandleConfigCopy(pLayerConfig, &mLayerConfig, &mOutputPortFormat);

    /*RETURN_BADPORT_IF_NOT_OUTPUT(pLayerConfig);

    //todo: copy to venc member
    memcpy(&mLayerConfig, pLayerConfig, sizeof(OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE));*/

    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig -> VideoTemporalLayering ePattern %d, nPLayerCountActual %d, "
        "nBLayerCountActual %d, bBitrateRatiosSpecified %d, nBitrateRatios[0] %d",
        pLayerConfig->ePattern, pLayerConfig->nPLayerCountActual,
        pLayerConfig->nBLayerCountActual, pLayerConfig->bBitrateRatiosSpecified, pLayerConfig->nBitrateRatios[0]);

    //return error temporary till implement this
    return OMX_ErrorBadPortIndex;
}

/* Get Config Handle */
OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigVideoFramerate(OMX_CONFIG_FRAMERATETYPE* pFrameRateType)
{
    HandleConfigCopy(&mFrameRateType, pFrameRateType, &mOutputPortFormat);
    /*CHECK_OMX_PARAM(pFrameRateType);

    RETURN_BADPORT_IF_NOT_OUTPUT(pFrameRateType);

    memcpy(pFrameRateType, &mFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));*/

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigVideoBitrate(OMX_VIDEO_CONFIG_BITRATETYPE* pConfigBitrate)
{
    HandleConfigCopy(&mConfigBitrate, pConfigBitrate, &mOutputPortFormat);
    /*CHECK_OMX_PARAM(pConfigBitrate);

    RETURN_BADPORT_IF_NOT_OUTPUT(pConfigBitrate);

    memcpy(pConfigBitrate, &mConfigBitrate, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));*/

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigVideoIntraVOPRefresh(OMX_CONFIG_INTRAREFRESHVOPTYPE* pConfigIntraRefreshVopType)
{
    HandleConfigCopy(&mConfigIntraRefreshVopType, pConfigIntraRefreshVopType, &mOutputPortFormat);
    /*CHECK_OMX_PARAM(pConfigIntraRefreshVopType);

    RETURN_BADPORT_IF_NOT_OUTPUT(pConfigIntraRefreshVopType);

    memcpy(pConfigIntraRefreshVopType, &mConfigIntraRefreshVopType, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));*/

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigVideoAVCIntraPeriod(OMX_VIDEO_CONFIG_AVCINTRAPERIOD* pConfigAVCIntraPeriod)
{
    HandleConfigCopy(&mConfigAVCIntraPeriod, pConfigAVCIntraPeriod, &mOutputPortFormat);
    /*RETURN_BADPORT_IF_NOT_OUTPUT(pConfigAVCIntraPeriod);

    memcpy(pConfigAVCIntraPeriod, &mConfigAVCIntraPeriod, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));*/

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigGoogleIntraRefresh(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE* pConfigAndroidIntraPeriod)
{
    HandleConfigCopy(&mConfigAndroidIntraPeriod, pConfigAndroidIntraPeriod, &mOutputPortFormat);
    /*RETURN_BADPORT_IF_NOT_OUTPUT(pConfigAndroidIntraPeriod);

    memcpy(pConfigAndroidIntraPeriod, &mConfigAndroidIntraPeriod, sizeof(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE));*/

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigGoogleTemporalLayering(OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE* pLayerConfig)
{
    HandleConfigCopy(&mLayerConfig, pLayerConfig, &mOutputPortFormat);
    /*RETURN_BADPORT_IF_NOT_OUTPUT(pLayerConfig);

    memcpy(pLayerConfig, &mLayerConfig, sizeof(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE));*/

    MTK_OMX_LOGD_ENG("there shouldn't have caller from framework");

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetConfigCommonRotate(OMX_CONFIG_ROTATIONTYPE* pConfigRotate)
{
    mRotationType.nPortIndex = mOutputPortFormat.nPortIndex;
    HandleConfigCopy(&mRotationType, pConfigRotate, &mOutputPortFormat);
    MTK_OMX_LOGD_ENG("MtkOmxVenc:: HandleGetConfigCommonRotate OMX_IndexConfigCommonRotate size %d nVersion %d portindex %d nRotation %d", mRotationType.nSize, mRotationType.nVersion.nVersion, mRotationType.nPortIndex, mRotationType.nRotation);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigCommonRotate(OMX_CONFIG_ROTATIONTYPE* pConfigRotate)
{
    OMX_CONFIG_ROTATIONTYPE *pRotate = (OMX_CONFIG_ROTATIONTYPE *)pConfigRotate;

    /*
    typedef struct OMX_CONFIG_ROTATIONTYPE {
        OMX_U32 nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32 nPortIndex;
        OMX_S32 nRotation;
    } OMX_CONFIG_ROTATIONTYPE;
    */

    // input scaling only for overspec
    if (pRotate->nPortIndex == mOutputPortFormat.nPortIndex)
    {
        MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig OMX_IndexConfigCommonRotate size %d nVersion %d portindex %d nRotation %d", pRotate->nSize, pRotate->nVersion.nVersion, pRotate->nPortIndex, pRotate->nRotation);
        mRotationType.nRotation = pRotate->nRotation;
    }
    MTK_OMX_LOGD_ENG("MtkOmxVenc:: HandleSetConfigCommonRotate OMX_IndexConfigCommonRotate size %d nVersion %d portindex %d nRotation %d", mRotationType.nSize, mRotationType.nVersion.nVersion, mRotationType.nPortIndex, mRotationType.nRotation);
    return OMX_ErrorNone;
}

