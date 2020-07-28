#ifndef __MTKOMXVENC_ROI_H__
#define __MTKOMXVENC_ROI_H__

#include "osal_utils.h"
#include "OMX_Core.h"
#include "OMX_IndexExt.h"
#include "OMX_Video.h"
#include "OMX_Video_Extensions.h"

#include "MtkOmxMVAMgr.h"
#include "MtkOmxVencExtension.h"
#include "RoiInfoConfig.h"

struct ExtensionIndex {
    const OMX_STRING name;
    const OMX_INDEXTYPE index;
};

const struct ExtensionIndex roiExt[] = {
    {"OMX.MTK.index.param.video.roi.switch", OMX_IndexVendorMtkOmxVencRoiSwitch},
    {"OMX.MTK.index.param.video.roi.size", OMX_IndexVendorMtkOmxVencRoiSize},
    {"OMX.MTK.index.param.video.roi.info", OMX_IndexVendorMtkOmxVencRoiInfo},
    {"OMX.MTK.index.param.video.roi.license", OMX_IndexVendorMtkOmxVencRoiLicense},
    {"OMX.MTK.index.param.video.roi.license.size", OMX_IndexVendorMtkOmxVencRoiLicenseSize}
};

class MtkOmxVencRoi : public MtkOmxVencStrategy
{
public:
    MtkOmxVencRoi();
    ~MtkOmxVencRoi();

    bool shouldEnable() override;

    OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam) override;
    OMX_ERRORTYPE GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType) override;

    void onInit(VENC_DRV_PARAM_ENC_T* param, VAL_HANDLE_T pDrvHandle) override;
    void onDeInit() override;

    void UpdateInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf) override;
    void DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf) override;
    void EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pInputBuf) override;

    OMX_ERRORTYPE SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig) override;
    OMX_ERRORTYPE GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig) override;

private:

    #ifdef MTK_ROINET_SUPPORT
    ROINet *mNet;
    #endif

    VAL_HANDLE_T mDrvHandle;
    int mRoiWidth;
    int mRoiHeight;
    int mRoiSize;
    int mEnabledROI; /*0:disable, 1:platform solution 2:apk solution*/

    MtkVenc::MtkRoiConfig<OMX_BUFFERHEADERTYPE*, MtkVenc::MtkRoiConfigEntry>* mConfig;
    OmxMVAManager *mRoiMVAMgr;

    pthread_mutex_t mRoiBufferTableLock;
    pthread_mutex_t mFreeRoiBufferListLock;
    DefaultKeyedVector<OMX_BUFFERHEADERTYPE*, OMX_U8*> mRoiBufferTable = DefaultKeyedVector<OMX_BUFFERHEADERTYPE*, OMX_U8*>(0);
    Vector<OMX_U8*> mFreeRoiBufferList;

    OMX_VIDEO_CONFIG_ROI_INFO mRoiInfo;
    OMX_U8* mRoiLicense;
    OMX_U32 mRoiLicenseSize;

    void GetFreeRoiBuffer(OMX_U8** ppRoiBuffer, unsigned* pRoiBufferSize);
    void SetFreeRoiBuffer(OMX_U8* pRoiBuffer);
    OMX_ERRORTYPE HandleSetParamVencorSetRoiOn(OMX_PARAM_U32TYPE* pCompParam);
    OMX_ERRORTYPE HandleSetParamVencorSetRoiLicenseSize(OMX_PARAM_U32TYPE* pCompParam);
    OMX_ERRORTYPE HandleSetParamVencorSetRoiLicense(OMX_U8* pCompParam);
    OMX_ERRORTYPE HandleSetConfigVendorSetRoiSize(OMX_VIDEO_CONFIG_ROI_INFO* pConfig);
    OMX_ERRORTYPE HandleSetConfigVendorSetRoiInfo(char* pConfig);
};

#endif