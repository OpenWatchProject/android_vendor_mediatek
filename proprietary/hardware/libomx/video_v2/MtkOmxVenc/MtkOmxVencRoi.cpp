#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"
#include "MtkOmx2V4L2.h"

#include <cutils/properties.h>
#include <sched.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVencRoi"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

void MtkOmxVenc::MtkOmxVencRoi()
{
    mRoiWidth = 0;
    mRoiHeight = 0;
    mEnabledROI = 0;
    mRoiMVAMgr = NULL;

    mRoiLicenseSize = 0;
    mRoiLicense = NULL;

    INIT_MUTEX(mRoiBufferTableLock);
    INIT_MUTEX(mFreeRoiBufferListLock);
}
void MtkOmxVenc::deMtkOmxVencRoi()
{
    DESTROY_MUTEX(mRoiBufferTableLock);
    DESTROY_MUTEX(mFreeRoiBufferListLock);

    if(mRoiLicense != NULL) free(mRoiLicense);
}
void MtkOmxVenc::ComponentInitRoi()
{

}
void MtkOmxVenc::ComponentDeInitRoi()
{

}

// InitHW before stream on (first buffer came in)
void MtkOmxVenc::ROI_OnInit(video_encode_param* param)
{
    if(mEnabledROI == 0)
    {
        GET_SYSTEM_PROP(int, mEnabledROI, "vendor.mtk.venc.h264.roi", "0");
        if(mEnabledROI == 0) {
            MTK_OMX_LOGD_ENG("ROI NOT ENABLED!!");
            return;
        }
    }

    GET_SYSTEM_PROP(bool, mEnableDumpROI, "vendor.mtk.venc.roi.dump.map", "0");

    mRoiWidth = param->buf_w;
    mRoiHeight = param->buf_h;
    mRoiSize = (VENC_ROUND_N(mRoiWidth, 16)>>4) * (VENC_ROUND_N(mRoiHeight, 16)>>4);

    MTK_OMX_LOGD_ENG("[ROI] w: %d, h: %d, mapsize: %d", mRoiWidth, mRoiHeight, mRoiSize);

    if(mEnabledROI == 1) // platform
    {
        MTK_OMX_LOGD_ENG("[ROI] Use platform solution");

        bool readDataPart = false;
        GET_SYSTEM_PROP(bool, readDataPart, "vendor.mtk.venc.roi.read.config.data", "0");

        if(mRoiMVAMgr == NULL) mRoiMVAMgr = new OmxMVAManager("ion");

        #ifdef MTK_ROINET_SUPPORT
        mNet = new Saliency(mRoiWidth, mRoiHeight, 0);
        #endif
    }
    else if(mEnabledROI >= 2)
    {
        MTK_OMX_LOGD_ENG("[ROI] Use 3rd party solution");
        if(mConfig)
        {
            MTK_OMX_LOGD_ENG("[ROI] 0. Adjust Size");
            mConfig->adjustSize(VENC_ROUND_N(mRoiWidth, 16)>>4, VENC_ROUND_N(mRoiHeight, 16)>>4);
        }
    }

    ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_ROI_ON, 1);
}

// DeInitHW after stream off
void MtkOmxVenc::ROI_OnDeInit()
{
    if(mEnabledROI == 0) return;

    #ifdef MTK_ROINET_SUPPORT
    if(mNet != NULL) {
        delete mNet;
        mNet = NULL;
    }
    #endif

    if(mConfig != NULL) {
        delete mConfig;
        mConfig = NULL;
    }

    if(mRoiMVAMgr != NULL) {
        delete mRoiMVAMgr;
        mRoiMVAMgr = NULL;
    }
}

void MtkOmxVenc::GetFreeRoiBuffer(OMX_U8** ppRoiBuffer, unsigned* pRoiBufferSize)
{
    IN_FUNC();

    OMX_U8* freeBufferPtr = 0;

    LOCK(mFreeRoiBufferListLock);
    if(mFreeRoiBufferList.size() > 0)
    {
        freeBufferPtr = mFreeRoiBufferList[0];
        mFreeRoiBufferList.removeAt(0);
    }
    UNLOCK(mFreeRoiBufferListLock);

    if(freeBufferPtr != 0)
    {
        PROP();

        *ppRoiBuffer = freeBufferPtr;
        *pRoiBufferSize = mRoiSize;

        MTK_OMX_LOGV_ENG("Handover free roi buf %p [%d]", *ppRoiBuffer, *pRoiBufferSize);
    }
    else
    {
        PROP();

        mRoiMVAMgr->newOmxMVAandVA(128, mRoiSize, NULL, (void**)ppRoiBuffer);
        *pRoiBufferSize = mRoiSize;

        MTK_OMX_LOGV_ENG("Allocate new roi buf %p [%d]", *ppRoiBuffer, *pRoiBufferSize);
    }

    OUT_FUNC();
}

void MtkOmxVenc::SetFreeRoiBuffer(OMX_U8* pRoiBuffer)
{
    if(pRoiBuffer == NULL) return;

    MTK_OMX_LOGV_ENG("SetFreeRoiBuffer: %p", pRoiBuffer);

    LOCK(mFreeRoiBufferListLock);
    mFreeRoiBufferList.add(pRoiBuffer);
    UNLOCK(mFreeRoiBufferListLock);
}

// before enque
void MtkOmxVenc::ROI_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, struct v4l2_buffer* qbuf)
{
    if(mEnabledROI == 0) return;
    IN_FUNC();

    unsigned char* pRoiBuf = NULL;
    unsigned int size = 0;
    uint32_t pa = 0;

    OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 aInputSize  = pInputBuf->nFilledLen;
    VENC_DRV_PARAM_FRM_BUF_T aFrameBuf;

    OMX_U8 *aCnvtBuf = (OMX_U8 *)pInputBuf->pPlatformPrivate;

    GetVEncDrvFrmBuffer(aInputBuf, aInputSize, aCnvtBuf, 0, &aFrameBuf);

    // get latest RC QP
    int qp = 0;
    ioctl_runtime_query(V4L2_CID_MPEG_MTK_ENCODE_ROI_RC_QP, &qp);

    // get available roi buffer for ROI MAP
    GetFreeRoiBuffer(&pRoiBuf, &size);

    if(mEnabledROI == 1) // platform
    {
        MTK_OMX_LOGD_ENG("[ROI] get platform roi info");

        #ifdef MTK_ROINET_SUPPORT
        // get roi info and flush
        mNet->get_Saliency((unsigned char*)aFrameBuf.rFrmBufAddr.u4VA, mRoiWidth, mRoiHeight, pRoiBuf); //uchar* frame va, uchar* roi va
        #endif
    }
    else if(mEnabledROI >= 2)
    {
        MTK_OMX_LOGD_ENG("[ROI] 4. get Roi Map %p from buffer %p baseQP %d", pRoiBuf, pInputBuf, qp);

        mConfig->getBufferRoiMap(pInputBuf, pRoiBuf, qp);
    }

    if(mEnableDumpROI)
    {
        char name[128];
        snprintf(name, sizeof(name), "/sdcard/roimap_dump_%d_%d_%d.bin", mRoiWidth, mRoiHeight, gettid());
        dumpBuffer(name, pRoiBuf, mRoiSize);
    }

    if(pRoiBuf != NULL)
    {
        mRoiMVAMgr->syncBufferCacheFrm(pRoiBuf, false);

        // pass roi buffer PA using reserved
        mRoiMVAMgr->getOmxMVAFromVA(pRoiBuf, &pa);
        qbuf->flags |= V4L2_BUF_FLAG_ROI;
        qbuf->reserved2 = pa;
        qbuf->m.planes[0].reserved[0] = pa;
        qbuf->sequence = pa;

        LOCK(mRoiBufferTableLock);
        mRoiBufferTable.add(qbuf->index, pRoiBuf);
        UNLOCK(mRoiBufferTableLock);
    }

    MTK_OMX_LOGD_ENG("Frame %p, VA address %p, PA address %p, qp %d", pInputBuf, pRoiBuf, pa, qp);

    OUT_FUNC();
}
void MtkOmxVenc::ROI_UpdateOutputV4L2Buffer(OMX_BUFFERHEADERTYPE *pOutputBuf, struct v4l2_buffer* qbuf)
{
    if(mEnabledROI == 0) return;
}

// after enque
void MtkOmxVenc::ROI_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if(mEnabledROI == 0) return;

}
void MtkOmxVenc::ROI_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** pOutputBuf)
{
    if(mEnabledROI == 0) return;
}

// after deque
void MtkOmxVenc::ROI_DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, void* dqbuf)
{
    if(mEnabledROI == 0) return;
    if(pInputBuf == NULL) return;

    IN_FUNC();

    v4l2_buffer* vb = (v4l2_buffer*)dqbuf;

    LOCK(mRoiBufferTableLock);
    unsigned char* pRoiBuf = mRoiBufferTable.valueFor(vb->index);
    mRoiBufferTable.add(vb->index, NULL);
    UNLOCK(mRoiBufferTableLock);

    if(mEnabledROI == 1)
    {
        SetFreeRoiBuffer(pRoiBuf);
    }
    else if(mEnabledROI >= 2)
    {
        mConfig->ungiveBuffer(pInputBuf);

        //if(refCount == 0) SetFreeRoiBuffer(pRoiBuf);

        SetFreeRoiBuffer(pRoiBuf);

        MTK_OMX_LOGD_ENG("[ROI] 5. ungive Buffer %p", pInputBuf);// return 0 to freeRoiBuffer
    }

    OUT_FUNC();
}
void MtkOmxVenc::ROI_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, void* dqbuf)
{
    if(mEnabledROI == 0) return;
}

void MtkOmxVenc::ROI_EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pBuffHdr)
{
    if(mEnabledROI != 2 && mEnabledROI != 3) return;

    int refcount = mConfig->giveBuffer(pBuffHdr);

    MTK_OMX_LOGD_ENG("[ROI] 3. give Buffer key %p to Roi Map, ref count %d", pBuffHdr, refcount);
}

OMX_ERRORTYPE MtkOmxVenc::ROI_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nConfigIndex)
    {
        case OMX_IndexVendorMtkOmxVencRoiSize:
            err = HandleSetConfigVendorSetRoiSize((OMX_VIDEO_CONFIG_ROI_INFO*)pCompConfig);
            break;
        case OMX_IndexVendorMtkOmxVencRoiInfo:
            err = HandleSetConfigVendorSetRoiInfo((char*)pCompConfig);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::ROI_GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nConfigIndex)
    {
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::ROI_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nParamIndex)
    {
        case OMX_IndexVendorMtkOmxVencRoiSwitch:
            err = HandleSetParamVencorSetRoiOn((OMX_PARAM_U32TYPE*) pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencRoiLicenseSize:
            err = HandleSetParamVencorSetRoiLicenseSize((OMX_PARAM_U32TYPE*) pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencRoiLicense:
            err = HandleSetParamVencorSetRoiLicense((OMX_U8*) pCompParam);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

struct ExtensionIndex {
    const OMX_STRING name;
    const OMX_INDEXTYPE index;
};

struct ExtensionIndex roiExt[] = {
    {"OMX.MTK.index.param.video.roi.switch", OMX_IndexVendorMtkOmxVencRoiSwitch},
    {"OMX.MTK.index.param.video.roi.size", OMX_IndexVendorMtkOmxVencRoiSize},
    {"OMX.MTK.index.param.video.roi.info", OMX_IndexVendorMtkOmxVencRoiInfo},
    {"OMX.MTK.index.param.video.roi.license", OMX_IndexVendorMtkOmxVencRoiLicense},
    {"OMX.MTK.index.param.video.roi.license.size", OMX_IndexVendorMtkOmxVencRoiLicenseSize}
};

OMX_ERRORTYPE MtkOmxVenc::ROI_GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType)
{
    int extCount = sizeof(roiExt)/sizeof(struct ExtensionIndex);
    for(int i=0; i<extCount; i++)
    {
        if(!strncmp(parameterName,roiExt[i].name,strlen(roiExt[i].name)))
        {
            *pIndexType = (OMX_INDEXTYPE) roiExt[i].index;
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorUnsupportedIndex;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetParamVencorSetRoiOn(OMX_PARAM_U32TYPE* pCompParam)
{
    mEnabledROI = (int)pCompParam->nU32;
    MTK_OMX_LOGD_ENG("[ROI] enabled ROI: %d", mEnabledROI);

    if(mEnabledROI == 0) return OMX_ErrorNone;

    if(mEnabledROI >= 2) // third party
    {
        MTK_OMX_LOGD_ENG("[ROI] new 3rd party solution object");
        if(mRoiMVAMgr == NULL) mRoiMVAMgr = new OmxMVAManager("ion");

        if(mEnabledROI == 2) // third party value = quality
        {
            mConfig =
                new MtkVenc::MtkRoiConfig<OMX_BUFFERHEADERTYPE*, MtkVenc::MtkRoiConfigEntry>(&MtkVenc::doGenMapFromQualJump);
        }
        else if(mEnabledROI == 3) // third party value = favor qp
        {
            mConfig =
                new MtkVenc::MtkRoiConfig<OMX_BUFFERHEADERTYPE*, MtkVenc::MtkRoiConfigEntry>(&MtkVenc::doGenMapFromQpJump);
        }
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetParamVencorSetRoiLicenseSize(OMX_PARAM_U32TYPE* pCompParam)
{
    if(mEnabledROI == 0) return OMX_ErrorNone;

    if(mEnabledROI == 1)
    {
        mRoiLicenseSize = (OMX_U32) pCompParam->nU32;
        MTK_OMX_LOGD_ENG("[ROI][LEASE] Size %d", mRoiLicenseSize);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetParamVencorSetRoiLicense(OMX_U8* pCompParam)
{
    if(mEnabledROI == 0) return OMX_ErrorNone;

    if(mEnabledROI == 1)
    {
        if(mRoiLicenseSize <= 0) return OMX_ErrorNone;

        if(mRoiLicense != NULL) free(mRoiLicense);
        mRoiLicense = (OMX_U8*) malloc(sizeof(OMX_U8)*mRoiLicenseSize);

        if(NULL != mRoiLicense)
        {
           memcpy(mRoiLicense, pCompParam, mRoiLicenseSize);
           MTK_OMX_LOGD_ENG("[ROI][LEASE] %d %c..", mRoiLicenseSize, mRoiLicense[0]);
        }
        else
        {
           MTK_OMX_LOGE("alloc roi license buffer fail:%u",mRoiLicenseSize);
        }
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSetRoiSize(OMX_VIDEO_CONFIG_ROI_INFO* pConfig)
{
    if(mEnabledROI != 2 && mEnabledROI != 3) return OMX_ErrorNone;
    if(pConfig == NULL) return OMX_ErrorBadParameter;

    MTK_OMX_LOGD_ENG("[ROI][SetSize] info count: %u base64 str length: %u", pConfig->nRoiInfoSize, pConfig->nRoiStringSize);

    memcpy(&mRoiInfo, pConfig, sizeof(OMX_VIDEO_CONFIG_ROI_INFO));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSetRoiInfo(char* pConfig)
{
    if(mEnabledROI != 2 && mEnabledROI != 3) return OMX_ErrorNone;
    if(pConfig == NULL) return OMX_ErrorBadParameter;

    if(0 != strncmp(pConfig, (char*)mRoiInfo.pRoiInfoCheck, sizeof(mRoiInfo.pRoiInfoCheck)))
        return OMX_ErrorBadParameter;

    MTK_OMX_LOGD_ENG("[ROI][SetInfo] 1+2. length: %u %s", mRoiInfo.nRoiInfoSize, pConfig);

    mConfig->resetInfoSession();
    mConfig->giveRoiInfo(mRoiInfo.nRoiInfoSize, pConfig);

    return OMX_ErrorNone;
}