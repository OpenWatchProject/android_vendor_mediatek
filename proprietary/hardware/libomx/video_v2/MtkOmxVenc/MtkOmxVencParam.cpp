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
        MTK_OMX_LOGE("invalid param %s size %d, %d", #x, x->nSize, x->nVersion);\
        /*return OMX_ErrorBadParameter*/;\
    }

#define RETURN_BADPORT_IF_NOT_INPUT(x) \
    if (x->nPortIndex != mInputPortFormat.nPortIndex) return OMX_ErrorBadPortIndex

#define RETURN_BADPORT_IF_NOT_OUTPUT(x) \
    if (x->nPortIndex != mOutputPortFormat.nPortIndex) return OMX_ErrorBadPortIndex

// Profile & Level tables [
MTK_VENC_PROFILE_LEVEL_ENTRY AvcProfileLevelTable[] =
{
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},    // => MFV spec
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},

    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3},

    //Hw codec
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},   // => MT6589 spec
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel32},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel4},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},   // => MT6589 spec
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51},
};

MTK_VENC_PROFILE_LEVEL_ENTRY HevcProfileLevelTable[] =
{
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel1},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel1},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel2},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel2},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel21},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel21},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel3},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel3},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel31},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel31},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel4},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel4},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel41},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel41},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel5},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel5},
};

MTK_VENC_PROFILE_LEVEL_ENTRY H263ProfileLevelTable[] =
{
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},  // => MFV spec
#if 1
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},
#endif
};


MTK_VENC_PROFILE_LEVEL_ENTRY MPEG4ProfileLevelTable[] =
{
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0b},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level1},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level2},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level3},  // => MFV spec
#if 1
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4a},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level5},
#endif
};

#define MAX_AVC_PROFILE_LEVEL_TABLE_SZIE    sizeof(AvcProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
#define MAX_HEVC_PROFILE_LEVEL_TABLE_SZIE   sizeof(HevcProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
#define MAX_H263_PROFILE_LEVEL_TABLE_SZIE   sizeof(H263ProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
#define MAX_MPEG4_PROFILE_LEVEL_TABLE_SZIE  sizeof(MPEG4ProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
// ]

void MtkOmxVenc::MtkOmxVencParam()
{
    mSetStreamingMode = OMX_FALSE;
    mSetConstantBitrateMode = OMX_FALSE;

    mABitrateMode = OMX_Video_ControlRateDisable;
    mABitrateModeValue = 0;
}

void MtkOmxVenc::ComponentInitParam()
{
    InitOMXParams(&mBitrateType);
}

/* Set Param */
OMX_ERRORTYPE MtkOmxVenc::HandleSetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE *pPortDef)
{
    CHECK_OMX_PARAM(pPortDef);

    OMX_ERRORTYPE err;

    if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
    {
        return SetInputPortDefinition(pPortDef);
    }
    else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
    {
        return SetOutputPortDefinition(pPortDef);
    }
    else
    {
        return OMX_ErrorBadParameter;
    }
}

OMX_ERRORTYPE MtkOmxVenc::SetInputPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE *pPortDef)
{
    CHECK_OMX_PARAM(pPortDef);

    if (pPortDef->nBufferCountActual > mInputPortDef.nBufferCountActual) //user need more buffer
    {
        if (pPortDef->nBufferCountActual > 30)
        {
            MTK_OMX_LOGE("Can't support so many input buffers (%d)", pPortDef->nBufferCountActual);
            return OMX_ErrorBadParameter;
        }
        OMX_BUFFERHEADERTYPE **tmp;
        tmp = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(
                  sizeof(OMX_BUFFERHEADERTYPE *)*pPortDef->nBufferCountActual);
        if (tmp == NULL)//realloc fail
        {
            MTK_OMX_LOGE("realloc input fail");
            return OMX_ErrorBadParameter;
        }
        else
        {
            memcpy(tmp, mInputBufferHdrs, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
            MTK_OMX_FREE(mInputBufferHdrs);
            mInputBufferHdrs = tmp;
        }
    }
    memcpy(&mInputPortDef, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    mInputPortDef.nBufferSize = getInputBufferSizeByFormat(mInputPortDef.format.video);
    MTK_OMX_LOGD("@@ Set input port: nFrameWidth(%d), nFrameHeight(%d), nStride(%d), nSliceHeight(%d), "
                 "nBitrate(%d), xFramerate(0x%X), eColorFormat(0x%08X), nBufferSize(%d), "
                 "nBufferCountActual(%d)",
                 (int)mInputPortDef.format.video.nFrameWidth, (int)mInputPortDef.format.video.nFrameHeight,
                 (int)mInputPortDef.format.video.nStride, (int)mInputPortDef.format.video.nSliceHeight,
                 (int)mInputPortDef.format.video.nBitrate,
                 (unsigned int)mInputPortDef.format.video.xFramerate,
                 (unsigned int)mInputPortDef.format.video.eColorFormat, (int)mInputPortDef.nBufferSize,
                 (int)mInputPortDef.nBufferCountActual);

    // input scaling only for overspec
    if ( mInputPortDef.format.video.nFrameWidth <  mInputPortDef.format.video.nFrameHeight)
    {
        if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledNarrow && mInputPortDef.format.video.nFrameHeight <= mMaxScaledWide)
        {
            mInputScalingMode = OMX_FALSE;
            return OMX_ErrorNone;
        }
    } else {
        if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledWide && mInputPortDef.format.video.nFrameHeight <= mMaxScaledNarrow)
        {
            mInputScalingMode = OMX_FALSE;
            return OMX_ErrorNone;
        }
    }

    // keep aspect ratio scaling
    if (mInputPortDef.format.video.nFrameWidth > mInputPortDef.format.video.nFrameHeight && mInputPortDef.format.video.nFrameHeight > 0)
    {
        mScaledHeight = mMaxScaledNarrow;
        mScaledWidth = mInputPortDef.format.video.nFrameWidth * ((float)mMaxScaledNarrow / mInputPortDef.format.video.nFrameHeight);
        if (mScaledWidth > mMaxScaledWide)
        {
            mScaledHeight *= ((float)mMaxScaledWide / mScaledWidth);
            mScaledWidth = mMaxScaledWide;
        }
    }
    else if (mInputPortDef.format.video.nFrameWidth > 0)
    {
        mScaledWidth = mMaxScaledNarrow;
        mScaledHeight = mInputPortDef.format.video.nFrameHeight * ((float)mMaxScaledNarrow / mInputPortDef.format.video.nFrameWidth);
        if (mScaledHeight > mMaxScaledWide)
        {
            mScaledWidth *= ((float)mMaxScaledWide / mScaledHeight);
            mScaledHeight = mMaxScaledWide;
        }
    }
    // avoid MDP odd size green line
    mScaledWidth =  (mScaledWidth % 2)? (mScaledWidth+1):mScaledWidth;
    mScaledHeight = (mScaledHeight % 2)? (mScaledHeight+1):mScaledHeight;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::SetOutputPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE *pPortDef)
{
    CHECK_OMX_PARAM(pPortDef);

    if (pPortDef->nBufferCountActual > mOutputPortDef.nBufferCountActual) //user need more buffer
    {
        if (pPortDef->nBufferCountActual > 30)
        {
            MTK_OMX_LOGD_ENG("Can't support so many output buffers (%d)", pPortDef->nBufferCountActual);
            return OMX_ErrorBadParameter;
        }
        OMX_BUFFERHEADERTYPE **tmp;
        tmp = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) *
                                                     pPortDef->nBufferCountActual);
        if (tmp == NULL)//realloc fail
        {
            MTK_OMX_LOGE("realloc output fail");
            return OMX_ErrorBadParameter;
        }
        else
        {
            memcpy(tmp, mOutputBufferHdrs, sizeof(OMX_BUFFERHEADERTYPE *) *
                   mOutputPortDef.nBufferCountActual);
            MTK_OMX_FREE(mOutputBufferHdrs);
            mOutputBufferHdrs = tmp;
        }
    }
    memcpy(&mOutputPortDef, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    MTK_OMX_LOGD_ENG("mOutputPortDef.format.video.nFrameWidth=%d, mOutputPortDef.format.video.nFrameHeight=%d",
                 (int)mOutputPortDef.format.video.nFrameWidth,
                 (int)mOutputPortDef.format.video.nFrameHeight);
    #ifdef SUPPORT_NATIVE_HANDLE
    if(mIsAllocateOutputNativeBuffers){
        mOutputPortDef.nBufferSize = sizeof(native_handle_t) + sizeof(int) * 12;
    }
    else
    #endif
    {
        if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingHEVC) &&
            ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
             > FHD_AREA))
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC_4K;
        }

        if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingImageHEIC) &&
            ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
             > FHD_AREA))
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEIC;
        }

        if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingAVC) &&
            ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
             > HD_AREA))
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC_1080P;
        }

        if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingAVC) &&
            ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
             > FHD_AREA))
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC_4K;
        }

        if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingVP8) &&
            ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
             > HD_AREA))
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_VP8_1080P;
        }
    }


    MTK_OMX_LOGD("@@ Set output port: nFrameWidth(%d), nFrameHeight(%d), nStride(%d), nSliceHeight(%d),"
                 "nBitrate(%d), xFramerate(0x%X), compressionFormat(0x%08X), nBufferSize(%d),"
                 "nBufferCountActual(%d)",
                 (int)mOutputPortDef.format.video.nFrameWidth,
                 (int)mOutputPortDef.format.video.nFrameHeight, (int)mOutputPortDef.format.video.nStride,
                 (int)mOutputPortDef.format.video.nSliceHeight, (int)mOutputPortDef.format.video.nBitrate,
                 (unsigned int)mOutputPortDef.format.video.xFramerate,
                 (unsigned int)mOutputPortDef.format.video.eCompressionFormat,
                 (int)mOutputPortDef.nBufferSize, (int)mOutputPortDef.nBufferCountActual);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortFormat)
{
    CHECK_OMX_PARAM(pPortFormat);

    if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
    {
        // TODO: should we allow setting the input port param?
        mInputPortFormat.eColorFormat = pPortFormat->eColorFormat;
        mInputPortDef.format.video.eColorFormat = pPortFormat->eColorFormat;
    }
    else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
    {
        mOutputPortFormat.eColorFormat = pPortFormat->eColorFormat;
    }
    MTK_OMX_LOGD_ENG("OMX_IndexParamVideoPortFormat port %d, eColorFormat 0x%x ",
                 pPortFormat->nPortIndex, pPortFormat->eColorFormat);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetStandardComponentRole(OMX_PARAM_COMPONENTROLETYPE* pRoleParams)
{
    CHECK_OMX_PARAM(pRoleParams);

    *((char *)mempcpy((char *)mCompRole, (char *)pRoleParams->cRole, sizeof(mCompRole) - 1)) = '\0';

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoAvc(OMX_VIDEO_PARAM_AVCTYPE* pAvcType)
{
    CHECK_OMX_PARAM(pAvcType);

    if (pAvcType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mAvcType, pAvcType, sizeof(OMX_VIDEO_PARAM_AVCTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoHevc(OMX_VIDEO_PARAM_HEVCTYPE* pHevcType)
{
    CHECK_OMX_PARAM(pHevcType);

    if (pHevcType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mHevcType, pHevcType, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoH263(OMX_VIDEO_PARAM_H263TYPE* pH263Type)
{
    CHECK_OMX_PARAM(pH263Type);

    if (pH263Type->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mH263Type, pH263Type, sizeof(OMX_VIDEO_PARAM_H263TYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoMpeg4(OMX_VIDEO_PARAM_MPEG4TYPE* pMpeg4Type)
{
    CHECK_OMX_PARAM(pMpeg4Type);

    if (pMpeg4Type->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mMpeg4Type, pMpeg4Type, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoVp8(OMX_VIDEO_PARAM_VP8TYPE* pVP8Type)
{
    CHECK_OMX_PARAM(pVP8Type);

    if (pVP8Type->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mVp8Type, pVP8Type, sizeof(OMX_VIDEO_PARAM_VP8TYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE* pBitrateType)
{
    CHECK_OMX_PARAM(pBitrateType);

    if (pBitrateType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mBitrateType, pBitrateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));

    MTK_OMX_LOGD_ENG("nTargetBitrate=%d nQualityFactor=%d, mode=%x",
        (int)pBitrateType->nTargetBitrate, (int)pBitrateType->nTargetBitrate,
                 pBitrateType->eControlRate);

    if (pBitrateType->eControlRate == OMX_Video_ControlRateMtkStreaming)
    {
        mSetStreamingMode = OMX_TRUE;
    }
    else if (pBitrateType->eControlRate == OMX_Video_ControlRateConstant)
    {
        mSetConstantBitrateMode = OMX_TRUE;
    }

    if (pBitrateType->eControlRate == OMX_Video_ControlRateConstant ||
        pBitrateType->eControlRate == OMX_Video_ControlRateVariable)
    {
        mABitrateMode = pBitrateType->eControlRate;
        mABitrateModeValue = pBitrateType->nTargetBitrate;
    }
    else if(pBitrateType->eControlRate == OMX_Video_ControlRateConstantQuality)
    {
        mABitrateMode = pBitrateType->eControlRate;
        mABitrateModeValue = pBitrateType->nQualityFactor;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE* pQuantizationType)
{
    CHECK_OMX_PARAM(pQuantizationType);

    if (pQuantizationType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mQuantizationType, pQuantizationType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoVBSMC(OMX_VIDEO_PARAM_VBSMCTYPE* pVbsmcType)
{
    CHECK_OMX_PARAM(pVbsmcType);

    if (pVbsmcType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mVbsmcType, pVbsmcType, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoMotionVector(OMX_VIDEO_PARAM_MOTIONVECTORTYPE* pMvType)
{
    CHECK_OMX_PARAM(pMvType);

    if (pMvType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mMvType, pMvType, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoIntraRefresh(OMX_VIDEO_PARAM_INTRAREFRESHTYPE* pIntraRefreshType)
{
    CHECK_OMX_PARAM(pIntraRefreshType);

    if (pIntraRefreshType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mIntraRefreshType, pIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoSliceFMO(OMX_VIDEO_PARAM_AVCSLICEFMO* pAvcSliceFMO)
{
    CHECK_OMX_PARAM(pAvcSliceFMO);

    if (pAvcSliceFMO->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mAvcSliceFMO, pAvcSliceFMO, sizeof(OMX_VIDEO_PARAM_AVCSLICEFMO));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoErrorCorrection(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE* pErrorCorrectionType)
{
    CHECK_OMX_PARAM(pErrorCorrectionType);

    if (pErrorCorrectionType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(&mErrorCorrectionType, pErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendor3DVideoRecode(OMX_VIDEO_H264FPATYPE* h264FpaType)
{
    m3DVideoRecordMode = *h264FpaType;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorTimelapseMode(OMX_BOOL* mode)
{
    mIsTimeLapseMode = *mode;
    if (OMX_TRUE == mIsTimeLapseMode)
    {
        MTK_OMX_LOGD_ENG("Enable Timelapse mode");
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorWhiteboardEffectMode(OMX_BOOL* mode)
{
    mIsWhiteboardEffectMode = *mode;
    if (OMX_TRUE == mIsWhiteboardEffectMode)
    {
        MTK_OMX_LOGD_ENG("Enable Whiteboard Effect mode");
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorSetMCIMode(OMX_BOOL* mode)
{
    mIsMCIMode = *mode;
    if (OMX_TRUE == mIsMCIMode)
    {
        MTK_OMX_LOGD_ENG("Enable MCI mode");
        mInputMVAMgr->setBoolProperty("MCI", mIsMCIMode);
        mOutputMVAMgr->setBoolProperty("MCI", mIsMCIMode);
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorSetScenario(OMX_PARAM_U32TYPE* pSetScenarioInfo)
{
    OMX_U32 tmpVal = pSetScenarioInfo->nU32;

    if (tmpVal == OMX_VIDEO_MTKSpecificScenario_LivePhoto) { mIsLivePhoto = true; }
    if (tmpVal == OMX_VIDEO_MTKSpecificScenario_CrossMount) { mIsCrossMount = true; }
    if (tmpVal == OMX_VIDEO_MTKSpecificScenario_WeChatCameraRecord) { mIsWeChatRecording = true; }
    MTK_OMX_LOGD_ENG("@@ set venc scenario %lu", tmpVal);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorPrependSPSPPS(PrependSPSPPSToIDRFramesParams* prependSpsPPs)
{
    //caution that some platform's AVC driver don't support this feature!
    if (1)//always turn on
    {
        mPrependSPSPPSToIDRFrames = prependSpsPPs->bEnable;
        mPrependSPSPPSToIDRFramesNotify = OMX_TRUE;
        //VTS will set prepend header, don't turn WFD settings on by this settings to avoid enable dummy NAL.
        MTK_OMX_LOGD_ENG("@@ set prepend header %d, mPrependSPSPPSToIDRFrames: %d", prependSpsPPs->bEnable, mPrependSPSPPSToIDRFrames);

        {// temp solution to fix unsupport prepend issue for mt8167
            bool unsupport_prepend = (bool) MtkVenc::UnsupportPrepend();
            if (unsupport_prepend)
            {
                MTK_OMX_LOGD_ENG("temp solution to fix unsupport prepend issue for mt8167");
                return OMX_ErrorUnsupportedIndex;
            }
        }
    }
    else
    {
        MTK_OMX_LOGD_ENG("MtkOmxVenc::SetParameter unsupported prepend header");
        return OMX_ErrorUnsupportedIndex;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorNonRefPOp(OMX_VIDEO_NONREFP* nonRefP)
{
    mEnableNonRefP = (OMX_BOOL)nonRefP->bEnable;
    MTK_OMX_LOGD_ENG("@@ enable non-ref P");
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVendorSetClientLocally(OMX_CONFIG_BOOLEANTYPE* pClientLocallyInfo)
{
    mIsClientLocally = pClientLocallyInfo->bEnabled;
    MTK_OMX_LOGD_ENG("@@ mIsClientLocally(%d)", mIsClientLocally);
    return OMX_ErrorNone;
}


OMX_ERRORTYPE MtkOmxVenc::HandleSetGoogleStoreMetaDataInBuffers(StoreMetaDataInBuffersParams* pStoreMetaDataInBuffersParams)
{
    if (pStoreMetaDataInBuffersParams->nPortIndex == mInputPortFormat.nPortIndex)
    {
        mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;

        MTK_OMX_LOGD_ENG("@@ mStoreMetaDataInBuffers(%d)", mStoreMetaDataInBuffers);
        if (mStoreMetaDataInBuffers) {
            mInputPortDef.nBufferSize = sizeof(VideoNativeMetadata);
        }
    }
    else if (pStoreMetaDataInBuffersParams->nPortIndex == mOutputPortFormat.nPortIndex)
    {
        mStoreMetaDataInOutBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
        MTK_OMX_LOGD_ENG("@@ mStoreMetaDataInOutBuffers(%d)", mStoreMetaDataInOutBuffers);
    }
    else
    {
        return OMX_ErrorBadPortIndex;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetGoogleStoreANWBufferInMetadata(StoreMetaDataInBuffersParams* pStoreMetaDataInBuffersParams)
{
    if (pStoreMetaDataInBuffersParams->nPortIndex != mInputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    OMX_BOOL mDisableANWInMetadata = (OMX_BOOL) MtkVenc::DisableANWInMetadata();
    if( 1 == mDisableANWInMetadata )
    {
        MTK_OMX_LOGD_ENG("@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata return un-support by setting property");
        return OMX_ErrorUnsupportedIndex;

    }
    mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
    MTK_OMX_LOGD_ENG("@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata");
    MTK_OMX_LOGD_ENG("@@ mStoreMetaDataInBuffers(%d) for port(%d)", mStoreMetaDataInBuffers, pStoreMetaDataInBuffersParams->nPortIndex);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetGoogleEnableAndroidNativeHandle(AllocateNativeHandleParams* enableNativeHandle)
{
    if (NULL == enableNativeHandle)
    {
        MTK_OMX_LOGE("OMX_GoogleAndroidIndexAllocateAndroidNativeHandle: NULL input");
        return OMX_ErrorBadParameter;
    }
    if (enableNativeHandle->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        MTK_OMX_LOGE("OMX_GoogleAndroidIndexAllocateAndroidNativeHandle: does not support input port");
        return OMX_ErrorBadParameter;
    }

    mIsAllocateOutputNativeBuffers = enableNativeHandle->enable;
    if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
    {
        mStoreMetaDataInOutBuffers = OMX_TRUE;
    }
    MTK_OMX_LOGD_ENG("OMX_GoogleAndroidIndexAllocateAndroidNativeHandle enable(%d), mStoreMetaDataInOutBuffers(%d)",
        enableNativeHandle->enable, mStoreMetaDataInOutBuffers);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetGoogleTemporalLayering(OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE* pLayerParams)
{
    CHECK_OMX_PARAM(pLayerParams);

    if (pLayerParams->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    MTK_OMX_LOGD("SetParameter VideoTemporalLayering eSupportedPatterns %d, ePattern %d, "
        "nLayerCountMax %d, nBLayerCountMax %d, nPLayerCountActual %d, nBLayerCountActual %d, "
        "bBitrateRatiosSpecified %d, nBitrateRatios[0] %d",
        pLayerParams->eSupportedPatterns, pLayerParams->ePattern, pLayerParams->nLayerCountMax,
        pLayerParams->nBLayerCountMax, pLayerParams->nPLayerCountActual, pLayerParams->nBLayerCountActual,
        pLayerParams->bBitrateRatiosSpecified, pLayerParams->nBitrateRatios[0]);

    memcpy(&mLayerParams, pLayerParams, sizeof(mLayerParams));

    return OMX_ErrorNone;
}

/* Get Param */

OMX_ERRORTYPE MtkOmxVenc::HandleGetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE* pPortDef)
{
    CHECK_OMX_PARAM(pPortDef);

    if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
    {
        memcpy(pPortDef, &mInputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    }
    else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
    {
        memcpy(pPortDef, &mOutputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoInit(OMX_PORT_PARAM_TYPE* pPortParam)
{
    CHECK_OMX_PARAM(pPortParam);

    pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
    pPortParam->nStartPortNumber = MTK_OMX_INPUT_PORT;
    pPortParam->nPorts = 2;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetAudioInit(OMX_PORT_PARAM_TYPE* pPortParam)
{
    return HandleGetOtherInit(pPortParam);
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetImageInit(OMX_PORT_PARAM_TYPE* pPortParam)
{
    return HandleGetOtherInit(pPortParam);
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetOtherInit(OMX_PORT_PARAM_TYPE* pPortParam)
{
    CHECK_OMX_PARAM(pPortParam);

    pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
    pPortParam->nStartPortNumber = 0;
    pPortParam->nPorts = 0;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE* pPortFormat)
{
    CHECK_OMX_PARAM(pPortFormat);

    if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
    {
        return GetInputPortFormat(pPortFormat);
    }
    else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
    {
        return GetOutputPortFormat(pPortFormat);
    }
    else
    {
        return OMX_ErrorBadPortIndex;
    }
}

OMX_ERRORTYPE MtkOmxVenc::GetInputPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE* pPortFormat)
{
    if (pPortFormat->nIndex == 0)
    {
        pPortFormat->eColorFormat =  mInputPortFormat.eColorFormat;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    // Morris Yang 20120214 add for live effect recording [
    else if (pPortFormat->nIndex == 1)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_FormatAndroidOpaque;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    // ]
    else if (pPortFormat->nIndex == 2)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_FormatYUV420Flexible;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    else if (pPortFormat->nIndex == 3)
    {
        pPortFormat->eColorFormat =  OMX_MTK_COLOR_FormatYV12;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    else if (pPortFormat->nIndex == 4)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_FormatYUV420Planar;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    else if (pPortFormat->nIndex == 5)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_Format16bitRGB565;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    else if (pPortFormat->nIndex == 6)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_Format24bitRGB888;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    else if (pPortFormat->nIndex == 7)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_Format32bitARGB8888;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    /* for DirectLink Meta Mode + */
    else if (pPortFormat->nIndex == 8)
    {
        pPortFormat->eColorFormat =  OMX_MTK_COLOR_FormatBitStream;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    /* for DirectLink Meta Mode - */
    else if (pPortFormat->nIndex == 9)
    {
        pPortFormat->eColorFormat =  OMX_COLOR_Format32bitBGRA8888;
        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
    }
    else if (pPortFormat->nIndex == 10)
    {
        if (supportNV12())
        {
            pPortFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
        }
        else
        {
            return OMX_ErrorNoMore;
        }
    }
    else
    {
        return OMX_ErrorNoMore;
    }

    MTK_OMX_LOGD_ENG("[GetParameter] OMX_IndexParamVideoPortFormat index = %lu, color format = %d\n",
        pPortFormat->nIndex, pPortFormat->eColorFormat);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::GetOutputPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE* pPortFormat)
{
    if (pPortFormat->nIndex == 0)
    {
        pPortFormat->eColorFormat = OMX_COLOR_FormatUnused;
        pPortFormat->eCompressionFormat =  mOutputPortFormat.eCompressionFormat;
    }
    else
    {
        return OMX_ErrorNoMore;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetStandardComponentRole(OMX_PARAM_COMPONENTROLETYPE* pRoleParams)
{
    CHECK_OMX_PARAM(pRoleParams);

    *((char *)mempcpy((char *)pRoleParams->cRole, (char *)mCompRole, sizeof(pRoleParams->cRole) - 1)) = '\0';

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoAvc(OMX_VIDEO_PARAM_AVCTYPE* pAvcType)
{
    CHECK_OMX_PARAM(pAvcType);

    if (pAvcType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pAvcType, &mAvcType, sizeof(OMX_VIDEO_PARAM_AVCTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoHevc(OMX_VIDEO_PARAM_HEVCTYPE* pHevcType)
{
    CHECK_OMX_PARAM(pHevcType);

    if (pHevcType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pHevcType, &mHevcType, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoH263(OMX_VIDEO_PARAM_H263TYPE* pH263Type)
{
    CHECK_OMX_PARAM(pH263Type);

    if (pH263Type->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pH263Type, &mH263Type, sizeof(OMX_VIDEO_PARAM_H263TYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoMpeg4(OMX_VIDEO_PARAM_MPEG4TYPE* pMpeg4Type)
{
    CHECK_OMX_PARAM(pMpeg4Type);

    if (pMpeg4Type->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pMpeg4Type, &mMpeg4Type, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoVp8(OMX_VIDEO_PARAM_VP8TYPE* pVP8Type)
{
    CHECK_OMX_PARAM(pVP8Type);

    if (pVP8Type->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pVP8Type, &mVp8Type, sizeof(OMX_VIDEO_PARAM_VP8TYPE));

    return OMX_ErrorNone;
}

// Warning: Modify this function should discuss with VT owner
OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoProfileLevelQuerySupported(OMX_VIDEO_PARAM_PROFILELEVELTYPE* pProfileLevel)
{
    CHECK_OMX_PARAM(pProfileLevel);

    if (pProfileLevel->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    switch (mOutputPortDef.format.video.eCompressionFormat)
    {
        case OMX_VIDEO_CodingAVC:
            if (pProfileLevel->nProfileIndex >= MAX_AVC_PROFILE_LEVEL_TABLE_SZIE)
            {
                return OMX_ErrorNoMore;
            }
            else
            {
                pProfileLevel->eProfile = AvcProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                pProfileLevel->eLevel = AvcProfileLevelTable[pProfileLevel->nProfileIndex].level;
            }
            break;

        case OMX_VIDEO_CodingHEVC:
        case OMX_VIDEO_CodingImageHEIC:
            if (pProfileLevel->nProfileIndex >= MAX_HEVC_PROFILE_LEVEL_TABLE_SZIE)
            {
                return OMX_ErrorNoMore;
            }
            else
            {
                pProfileLevel->eProfile = HevcProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                pProfileLevel->eLevel = HevcProfileLevelTable[pProfileLevel->nProfileIndex].level;
            }
            break;

        case OMX_VIDEO_CodingMPEG4:
            if (pProfileLevel->nProfileIndex >= MAX_MPEG4_PROFILE_LEVEL_TABLE_SZIE)
            {
                return OMX_ErrorNoMore;
            }
            else
            {
                pProfileLevel->eProfile = MPEG4ProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                pProfileLevel->eLevel = MPEG4ProfileLevelTable[pProfileLevel->nProfileIndex].level;
            }
            break;

        case OMX_VIDEO_CodingH263:
            if (pProfileLevel->nProfileIndex >= MAX_H263_PROFILE_LEVEL_TABLE_SZIE)
            {
                return OMX_ErrorNoMore;
            }
            else
            {
                pProfileLevel->eProfile = H263ProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                pProfileLevel->eLevel = H263ProfileLevelTable[pProfileLevel->nProfileIndex].level;
            }
            break;

        case OMX_VIDEO_CodingVP8:
        {
            switch (pProfileLevel->nProfileIndex)
            {
                case 0:
                    pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version0;
                    break;
                case 1:
                    pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version1;
                    break;
                case 2:
                    pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version2;
                    break;
                case 3:
                    pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version3;
                    break;
                default:
                    return OMX_ErrorNoMore;
            }

            pProfileLevel->eProfile = OMX_VIDEO_VP8ProfileMain;
            break;
        }

        default:
            return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE* pBitrateType)
{
    CHECK_OMX_PARAM(pBitrateType);

    if (pBitrateType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pBitrateType, &mBitrateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE* pQuantizationType)
{
    CHECK_OMX_PARAM(pQuantizationType);

    if (pQuantizationType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pQuantizationType, &mQuantizationType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoVBSMC(OMX_VIDEO_PARAM_VBSMCTYPE* pVbsmcType)
{
    CHECK_OMX_PARAM(pVbsmcType);

    if (pVbsmcType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pVbsmcType, &mVbsmcType, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoMotionVector(OMX_VIDEO_PARAM_MOTIONVECTORTYPE* pMvType)
{
    CHECK_OMX_PARAM(pMvType);

    if (pMvType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pMvType, &mMvType, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoIntraRefresh(OMX_VIDEO_PARAM_INTRAREFRESHTYPE* pIntraRefreshType)
{
    CHECK_OMX_PARAM(pIntraRefreshType);

    if (pIntraRefreshType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pIntraRefreshType, &mIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoSliceFMO(OMX_VIDEO_PARAM_AVCSLICEFMO* pAvcSliceFMO)
{
    CHECK_OMX_PARAM(pAvcSliceFMO);

    if (pAvcSliceFMO->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pAvcSliceFMO, &mAvcSliceFMO, sizeof(OMX_VIDEO_PARAM_AVCSLICEFMO));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoErrorCorrection(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE* pErrorCorrectionType)
{
    CHECK_OMX_PARAM(pErrorCorrectionType);

    if (pErrorCorrectionType->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    memcpy(pErrorCorrectionType, &mErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));

    return OMX_ErrorNone;
}

// to-do: VENC_DRV_GET_TYPE_NON_REF_P_FREQUENCY
OMX_ERRORTYPE MtkOmxVenc::HandleGetVendorNonRefPOp(OMX_VIDEO_NONREFP* nonRefP)
{
    if (mEnableNonRefP == OMX_TRUE)
    {
        //if( uAVCNonRefPFreq && (mOutputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) )
        if( mOutputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC )
        {
            nonRefP->nFreq = ((3 << 16) | 4); //give non ref p frequency = (3/4) on ROME
        }
    }
    else
    {
        nonRefP->nFreq = 0;
    }

    MTK_OMX_LOGD_ENG("@@ get non-ref P freq %d", nonRefP->nFreq);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVendorOmxHandle(OMX_U32* pHandle)
{
    *pHandle = (OMX_U32)this;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVendorQueryDriverVersion(OMX_VIDEO_PARAM_DRIVERVER* pDriver)
{
    pDriver->nDriverVersion = (DRIVERVER_RESERVED << 32) + (DRIVERVER_MAJOR << 16) + (DRIVERVER_MINOR);
    MTK_OMX_LOGD("Query Driver version: %d, %d, %llu",
                 pDriver->nSize, sizeof(OMX_VIDEO_PARAM_DRIVERVER), pDriver->nDriverVersion);

    return OMX_ErrorNone;
}

// to-do: QueryDriverFormat
OMX_ERRORTYPE MtkOmxVenc::HandleGetVendorQueryCodecsSizes(OMX_VIDEO_PARAM_SPEC_QUERY* pSpecQuery)
{
    VENC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;
    MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(qinfoOut));
    if (OMX_FALSE == QueryDriverFormat(&qinfoOut))
    {
        MTK_OMX_LOGE("Error!! Cannot get decoder property.");
        return OMX_ErrorBadParameter;
    }
    pSpecQuery->profile = qinfoOut.u4Profile;
    pSpecQuery->level = qinfoOut.eLevel;
    pSpecQuery->nFrameWidth = qinfoOut.u4Width;
    pSpecQuery->nFrameHeight = qinfoOut.u4Height;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetGoogleDescribeColorFormat(DescribeColorFormatParams* describeParams)
{
    MTK_OMX_LOGD_ENG("DescribeColorFormat %lx, bUsingNativeBuffers %d, mbYUV420FlexibleMode %d", describeParams->eColorFormat,
        describeParams->bUsingNativeBuffers, mbYUV420FlexibleMode);

    if ((OMX_FALSE == describeParams->bUsingNativeBuffers) &&
         ((HAL_PIXEL_FORMAT_I420 == describeParams->eColorFormat)||
          (OMX_COLOR_FormatYUV420Planar == describeParams->eColorFormat) ||
          (OMX_COLOR_FormatYUV420Flexible == describeParams->eColorFormat)) )
    {
        bool err_return = 0;
        mbYUV420FlexibleMode = OMX_TRUE;
        err_return = DescribeFlexibleColorFormat((DescribeColorFormatParams *)describeParams);

        MTK_OMX_LOGD("client query OMX_COLOR_FormatYUV420Flexible mbYUV420FlexibleMode %d, ret: %d", mbYUV420FlexibleMode, err_return);
    }
    else
    {
        //treat the framework to push YUVFlexible format in codeccodec::queryCodecs()
        return OMX_ErrorUnsupportedIndex;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetGoogleDescribeColorFormat2(DescribeColorFormat2Params* describeParams)
{
    MTK_OMX_LOGD_ENG("DescribeColorFormat2 %lx, bUsingNativeBuffers %d, mbYUV420FlexibleMode %d",
        describeParams->eColorFormat, describeParams->bUsingNativeBuffers, mbYUV420FlexibleMode);

    return OMX_ErrorUnsupportedIndex;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetGoogleTemporalLayering(OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE* pLayerParams)
{
    CHECK_OMX_PARAM(pLayerParams);

    if (pLayerParams->nPortIndex != mOutputPortFormat.nPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    //video encoder not support B frame(HW&SW), so return necessary variables
    //see OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE class in OMX_VideoExt.h
    int max_b_frame = 0;
    ioctl_runtime_query(V4L2_CID_MPEG_VIDEO_B_FRAMES, &max_b_frame);

    mLayerParams.nLayerCountMax = 1;
    mLayerParams.nBLayerCountMax = max_b_frame;
    mLayerParams.nPLayerCountActual = 1;
    mLayerParams.nBLayerCountActual = 0;

    MTK_OMX_LOGD_ENG("GetParameter VideoTemporalLayering eSupportedPatterns %d, ePattern %d, "
        "nLayerCountMax %d, nBLayerCountMax %d, nPLayerCountActual %d, nBLayerCountActual %d, "
        "bBitrateRatiosSpecified %d, nBitrateRatios[0] %d",
        mLayerParams.eSupportedPatterns, mLayerParams.ePattern, mLayerParams.nLayerCountMax,
        mLayerParams.nBLayerCountMax, mLayerParams.nPLayerCountActual, mLayerParams.nBLayerCountActual,
        mLayerParams.bBitrateRatiosSpecified, mLayerParams.nBitrateRatios[0]);

    memcpy(pLayerParams, &mLayerParams, sizeof(mLayerParams));

    return OMX_ErrorNone;
}
