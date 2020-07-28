#ifndef __MTKOMXVENC_EXT_H__
#define __MTKOMXVENC_EXT_H__

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "OMX_Core.h"
#include "OMX_Video.h"
#include "OMX_IndexExt.h"
#include "OMX_Video_Extensions.h"

#include "osal_utils.h"
#include "val_api_public.h"
#include "venc_drv_if_public.h"

#define call_ext(HANDLE, FUNC, ...) \
    if(HANDLE!=NULL)(HANDLE)->FUNC(__VA_ARGS__)

class MtkOmxVencStrategy
{
protected:
    bool mEnableMoreLog;
public:
    virtual ~MtkOmxVencStrategy() {};
    // Statically observed
    virtual void ComponentInit() {};
    virtual void ComponentDeInit() {};

    virtual bool shouldEnable() {return false;}

    // execute until first OMX_ErrorNone
    virtual OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam) {return OMX_ErrorNotReady;}
    virtual OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam) {return OMX_ErrorNotReady;}
    virtual OMX_ERRORTYPE GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType) {return OMX_ErrorNotReady;}

    // Dynamically observed (shouldEnabled false will not invoke)
    virtual void onInit(VENC_DRV_PARAM_ENC_T* param, VAL_HANDLE_T pDrvHandle) {}
    virtual void onDeInit() {}

    virtual void UpdateInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf) {}
    virtual void UpdateOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf, VENC_DRV_PARAM_BS_BUF_T* qbuf) {}
    virtual void DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf) {}
    virtual void DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, VENC_DRV_PARAM_BS_BUF_T* pFrameBuf) {}
    virtual void EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pInputBuf) {}
    virtual void FillThisBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf) {}
    virtual void EmptyBufferDone(OMX_BUFFERHEADERTYPE* pInputBuf) {}
    virtual void FillBufferDone(OMX_BUFFERHEADERTYPE* pOutputBuf) {}

    // used in v4l2 omx
    virtual void OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf) {}
    virtual void OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf) {}

    // execute until first OMX_ErrorNone
    virtual OMX_ERRORTYPE SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig) {return OMX_ErrorNotReady;}
    virtual OMX_ERRORTYPE GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig) {return OMX_ErrorNotReady;}
};

class MtkOmxVencExtension : public MtkOmxVencStrategy
{
public:
    MtkOmxVencExtension();
    ~MtkOmxVencExtension();

    void ComponentInit() override;
    void ComponentDeInit() override;

    bool shouldEnable() override {return false;}

    OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam) override;
    OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam) override;
    OMX_ERRORTYPE GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType) override;

    void onInit(VENC_DRV_PARAM_ENC_T* param, VAL_HANDLE_T pDrvHandle) override;
    void onDeInit() override;

    void UpdateInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf) override;
    void UpdateOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf, VENC_DRV_PARAM_BS_BUF_T* qbuf) override;
    void DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf) override;
    void DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, VENC_DRV_PARAM_BS_BUF_T* pFrameBuf) override;
    void EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pInputBuf) override;
    void FillThisBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf) override;
    void EmptyBufferDone(OMX_BUFFERHEADERTYPE* pInputBuf) override;
    void FillBufferDone(OMX_BUFFERHEADERTYPE* pOutputBuf) override;

    void OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf) override;
    void OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf) override;

    OMX_ERRORTYPE SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig) override;
    OMX_ERRORTYPE GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig) override;

private:
    template <class S_TYPE> void mountStrategy();
    void unmountAllStrategies();

    Vector<MtkOmxVencStrategy*> mAllStrategies;
    Vector<MtkOmxVencStrategy*> mEnabledStrategies;
};

#endif