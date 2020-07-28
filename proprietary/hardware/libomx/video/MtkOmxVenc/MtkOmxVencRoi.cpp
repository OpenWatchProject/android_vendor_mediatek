
#include <cutils/log.h>
#include "MtkOmxVencRoi.h"
#include "properties.h"

#define LOG_TAG "MtkOmxVencRoi"

#define ROI_LOGD(fmt, arg...) \
    do { \
        if (mEnableMoreLog) { \
            ALOGD("[ROI][0x%08x] " fmt, this, ##arg); \
        } \
    } while(0)

MtkOmxVencRoi::MtkOmxVencRoi()
{
    this->mRoiWidth = 0;
    this->mRoiHeight = 0;
    this->mEnabledROI = 0;
    this->mRoiMVAMgr = NULL;

    this->mRoiLicenseSize = 0;
    this->mRoiLicense = NULL;

    INIT_MUTEX(this->mRoiBufferTableLock);
    INIT_MUTEX(this->mFreeRoiBufferListLock);

    GET_SYSTEM_PROP(int, this->mEnabledROI, "vendor.mtk.venc.h264.roi", "0");
    mEnableMoreLog = (bool) MtkVenc::EnableMoreLog("0");

    this->mDrvHandle = 0;
    this->mRoiSize = 0;
    this->mConfig = NULL;
    memset(&this->mRoiInfo, 0, sizeof(OMX_VIDEO_CONFIG_ROI_INFO));
}

MtkOmxVencRoi::~MtkOmxVencRoi()
{
    DESTROY_MUTEX(this->mRoiBufferTableLock);
    DESTROY_MUTEX(this->mFreeRoiBufferListLock);

    if(this->mRoiLicense != NULL) free(this->mRoiLicense);
}

inline uint32_t VENC_ROUND_N(uint32_t x, uint32_t n)
{
    //n must be exponential of 2
    return ((x + (n - 1)) & (~(n - 1)));
}

bool MtkOmxVencRoi::shouldEnable()
{
    if(this->mEnabledROI == 0)
    {
        GET_SYSTEM_PROP(bool, this->mEnabledROI, "vendor.mtk.venc.h264.roi", "0");
    }
    return (this->mEnabledROI != 0);
}

// InitHW before after encode param (first buffer came in)
void MtkOmxVencRoi::onInit(VENC_DRV_PARAM_ENC_T* param, VAL_HANDLE_T pDrvHandle)
{
    this->mRoiWidth = param->u4BufWidth;
    this->mRoiHeight = param->u4BufHeight;
    this->mRoiSize = (VENC_ROUND_N(this->mRoiWidth, 16)>>4) * (VENC_ROUND_N(this->mRoiHeight, 16)>>4);

    ROI_LOGD("[ROI] w: %d, h: %d, mapsize: %d", mRoiWidth, mRoiHeight, mRoiSize);

    if(this->mEnabledROI == 1) // platform
    {
        ROI_LOGD("[ROI] Use platform solution");

        bool readDataPart = false;
        GET_SYSTEM_PROP(bool, readDataPart, "vendor.mtk.venc.roi.read.config.data", "0");

        char roinet_config[] = "/vendor/etc/roinet_config.txt";
        char jnd_config[] = "/vendor/etc/jnd_config.txt";

        char roinet_config_data[] = "/data/local/tmp/roinet_config.txt";
        char jnd_config_data[] = "/data/local/tmp/jnd_config.txt";

        if(this->mRoiMVAMgr == NULL) this->mRoiMVAMgr = new OmxMVAManager("ion");

        #ifdef MTK_ROINET_SUPPORT
        if(readDataPart)
            this->mNet = new ROINet(this->mRoiWidth, this->mRoiHeight, roinet_config_data, jnd_config_data);
        else
            this->mNet = new ROINet(this->mRoiWidth, this->mRoiHeight, roinet_config, jnd_config);
        #endif
    }
    else if(this->mEnabledROI >= 2)
    {
        ROI_LOGD("[ROI] Use 3rd party solution");
        if(mConfig)
        {
            ROI_LOGD("[ROI] 0. Adjust Size");
            mConfig->adjustSize(VENC_ROUND_N(this->mRoiWidth, 16)>>4, VENC_ROUND_N(this->mRoiHeight, 16)>>4);
        }
    }

    this->mDrvHandle = pDrvHandle;

    uint32_t roi_on = (this->mEnabledROI != 0)?1:0;

    VENC_DRV_PARAM_ENC_EXTRA_T* pExtraEncDrvSetting = (VENC_DRV_PARAM_ENC_EXTRA_T*)param->pvExtraEnc;
    pExtraEncDrvSetting->u4RoiOn = roi_on;

    if( VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(this->mDrvHandle, VENC_DRV_SET_TYPE_ROI_ON, &roi_on, 0) )
    {
        ROI_LOGD("[ROI] set Roi On failed.");
        this->mEnabledROI = 0;
    }
}

// DeInitHW after driver deinit
void MtkOmxVencRoi::onDeInit()
{
    if(this->mEnabledROI == 0) return;

    #ifdef MTK_ROINET_SUPPORT
    if(this->mNet != NULL) {
        delete this->mNet;
        this->mNet = NULL;
    }
    #endif

    if(this->mConfig != NULL) {
        delete this->mConfig;
        this->mConfig = NULL;
    }

    if(this->mRoiMVAMgr != NULL) {
        delete this->mRoiMVAMgr;
        this->mRoiMVAMgr = NULL;
    }
}

void MtkOmxVencRoi::GetFreeRoiBuffer(OMX_U8** ppRoiBuffer, unsigned* pRoiBufferSize)
{
    OMX_U8* freeBufferPtr = 0;

    LOCK(this->mFreeRoiBufferListLock);
    if(this->mFreeRoiBufferList.size() > 0)
    {
        freeBufferPtr = this->mFreeRoiBufferList[0];
        this->mFreeRoiBufferList.removeAt(0);
    }
    UNLOCK(this->mFreeRoiBufferListLock);

    if(freeBufferPtr != 0)
    {
        *ppRoiBuffer = freeBufferPtr;
        *pRoiBufferSize = this->mRoiSize;

        ROI_LOGD("Handover free roi buf %p [%d]", *ppRoiBuffer, *pRoiBufferSize);
    }
    else
    {
        this->mRoiMVAMgr->newOmxMVAandVA(128, this->mRoiSize, NULL, (void**)ppRoiBuffer);
        *pRoiBufferSize = this->mRoiSize;

        ROI_LOGD("Allocate new roi buf %p [%d]", *ppRoiBuffer, *pRoiBufferSize);
    }
}

void MtkOmxVencRoi::SetFreeRoiBuffer(OMX_U8* pRoiBuffer)
{
    if(pRoiBuffer == NULL) return;

    ROI_LOGD("SetFreeRoiBuffer: %p", pRoiBuffer);

    LOCK(this->mFreeRoiBufferListLock);
    this->mFreeRoiBufferList.add(pRoiBuffer);
    UNLOCK(this->mFreeRoiBufferListLock);
}

// before eVEncDrvEncode
void MtkOmxVencRoi::UpdateInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf)
{
    if(this->mEnabledROI == 0) return;

    unsigned char* pRoiBuf = NULL;
    unsigned int size = 0;
    uint32_t pa = 0;

    OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 aInputSize  = pInputBuf->nFilledLen;

    OMX_U8 *aCnvtBuf = (OMX_U8 *)pInputBuf->pPlatformPrivate;

    // get latest RC QP
    int qp = 0;
    eVEncDrvGetParam(this->mDrvHandle, VENC_DRV_GET_TYPE_QUERY_ROI_RC_QP, 0, &qp);

    // get available roi buffer for ROI MAP
    GetFreeRoiBuffer(&pRoiBuf, &size);

    if(this->mEnabledROI == 1) // platform
    {
        ROI_LOGD("[ROI] get platform roi info");

        #ifdef MTK_ROINET_SUPPORT
        // get roi info and flush
        this->mNet->get_ROIInfo((unsigned char*)pFrameBuf->rFrmBufAddr.u4VA, pRoiBuf, qp); //uchar* frame va, uchar* roi va
        #endif
    }
    else if(this->mEnabledROI >= 2)
    {
        ROI_LOGD("[ROI] 4. get Roi Map %p from buffer %p baseQP %d", pRoiBuf, pInputBuf, qp);

        this->mConfig->getBufferRoiMap(pInputBuf, pRoiBuf, qp);
    }

    if(pRoiBuf != NULL)
    {
        // pass roi buffer PA using reserved
        this->mRoiMVAMgr->getOmxMVAFromVA(pRoiBuf, &pa);
        pFrameBuf->rRoiBufAddr.u4PA = pa;

        LOCK(this->mRoiBufferTableLock);
        this->mRoiBufferTable.add(pInputBuf, pRoiBuf);
        UNLOCK(this->mRoiBufferTableLock);
    }

    ROI_LOGD("Frame %p, VA address %p, PA address %p, qp %d", pInputBuf, pRoiBuf, pa, qp);

}

// after deque
void MtkOmxVencRoi::DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf)
{
    if(mEnabledROI == 0) return;
    if(pInputBuf == NULL) return;

    LOCK(mRoiBufferTableLock);
    unsigned char* pRoiBuf = mRoiBufferTable.valueFor(pInputBuf);
    mRoiBufferTable.removeItem(pInputBuf);
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

        ROI_LOGD("[ROI] 5. ungive Buffer %p", pInputBuf);// return 0 to freeRoiBuffer
    }
}

void MtkOmxVencRoi::EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pBuffHdr)
{
    if(this->mEnabledROI != 2 && this->mEnabledROI != 3) return;

    int refcount = this->mConfig->giveBuffer(pBuffHdr);

    ROI_LOGD("[ROI] 3. give Buffer key %p to Roi Map, ref count %d", pBuffHdr, refcount);
}

OMX_ERRORTYPE MtkOmxVencRoi::SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
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

OMX_ERRORTYPE MtkOmxVencRoi::SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    if(this->mEnabledROI == 0) return OMX_ErrorNotReady;

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

OMX_ERRORTYPE MtkOmxVencRoi::GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    if(this->mEnabledROI == 0) return OMX_ErrorNotReady;

    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nConfigIndex)
    {
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVencRoi::GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType)
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

OMX_ERRORTYPE MtkOmxVencRoi::HandleSetParamVencorSetRoiOn(OMX_PARAM_U32TYPE* pCompParam)
{
    this->mEnabledROI = (int)pCompParam->nU32;
    ROI_LOGD("[ROI] enabled ROI: %d", this->mEnabledROI);

    if(this->mEnabledROI == 0) return OMX_ErrorNone;

    if(this->mEnabledROI >= 2) // third party
    {
        ROI_LOGD("[ROI] new 3rd party solution object");
        if(this->mRoiMVAMgr == NULL) this->mRoiMVAMgr = new OmxMVAManager("ion");

        if(this->mEnabledROI == 2) // third party value = quality
        {
            this->mConfig =
                new MtkVenc::MtkRoiConfig<OMX_BUFFERHEADERTYPE*, MtkVenc::MtkRoiConfigEntry>(&MtkVenc::doGenMapFromQualJump);
        }
        else if(this->mEnabledROI == 3) // third party value = favor qp
        {
            this->mConfig =
                new MtkVenc::MtkRoiConfig<OMX_BUFFERHEADERTYPE*, MtkVenc::MtkRoiConfigEntry>(&MtkVenc::doGenMapFromQpJump);
        }
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVencRoi::HandleSetParamVencorSetRoiLicenseSize(OMX_PARAM_U32TYPE* pCompParam)
{
    if(this->mEnabledROI == 0) return OMX_ErrorNone;

    if(this->mEnabledROI == 1)
    {
        this->mRoiLicenseSize = (OMX_U32) pCompParam->nU32;
        ROI_LOGD("[ROI][LEASE] Size %d", this->mRoiLicenseSize);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVencRoi::HandleSetParamVencorSetRoiLicense(OMX_U8* pCompParam)
{
    if(this->mEnabledROI == 0) return OMX_ErrorNone;

    if(this->mEnabledROI == 1)
    {
        if(this->mRoiLicenseSize <= 0) return OMX_ErrorNone;

        if(this->mRoiLicense != NULL) free(mRoiLicense);
        this->mRoiLicense = (OMX_U8*) malloc(sizeof(OMX_U8)*this->mRoiLicenseSize);

        memcpy(this->mRoiLicense, pCompParam, this->mRoiLicenseSize);
        ROI_LOGD("[ROI][LEASE] %d %c..", this->mRoiLicenseSize, this->mRoiLicense[0]);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVencRoi::HandleSetConfigVendorSetRoiSize(OMX_VIDEO_CONFIG_ROI_INFO* pConfig)
{
    if(this->mEnabledROI != 2 && this->mEnabledROI != 3) return OMX_ErrorUnsupportedIndex;
    if(pConfig == NULL) return OMX_ErrorBadParameter;

    ROI_LOGD("[ROI][SetSize] info count: %u base64 str length: %u", pConfig->nRoiInfoSize, pConfig->nRoiStringSize);

    memcpy(&this->mRoiInfo, pConfig, sizeof(OMX_VIDEO_CONFIG_ROI_INFO));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVencRoi::HandleSetConfigVendorSetRoiInfo(char* pConfig)
{
    if(this->mEnabledROI != 2 && this->mEnabledROI != 3) return OMX_ErrorUnsupportedIndex;
    if(pConfig == NULL) return OMX_ErrorBadParameter;

    if(0 != strncmp(pConfig, (char*)this->mRoiInfo.pRoiInfoCheck, sizeof(this->mRoiInfo.pRoiInfoCheck)))
        return OMX_ErrorBadParameter;

    ROI_LOGD("[ROI][SetInfo] 1+2. length: %u %s", this->mRoiInfo.nRoiInfoSize, pConfig);

    this->mConfig->resetInfoSession();
    this->mConfig->giveRoiInfo(this->mRoiInfo.nRoiInfoSize, pConfig);

    return OMX_ErrorNone;
}
