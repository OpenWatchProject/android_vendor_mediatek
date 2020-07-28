
#include <cutils/log.h>

#include "properties.h"
#include "MtkOmxVencRoi.h"
#include "MtkOmxVencExtension.h"

#define LOG_TAG "MtkOmxVencExt"

#define EXT_LOGD(fmt, arg...) \
    do { \
        if (mEnableMoreLog) { \
            ALOGD("[EXTENSION][0x%08x] " fmt, this, ##arg); \
        } \
    } while(0)
#define IN_FUNC() EXT_LOGD("+ %s:%d", __func__, __LINE__)
#define OUT_FUNC() EXT_LOGD("- %s:%d", __func__, __LINE__)


#define EXECUTE_UNTIL_FIRST_ERRORNONE(_VECTOR_, FUNC, ...) \
    for(int i=0; i<_VECTOR_.size(); i++) { \
        MtkOmxVencStrategy* observer = _VECTOR_[i]; \
        if(OMX_ErrorNone == observer->FUNC (__VA_ARGS__)) return OMX_ErrorNone; \
    } \
    return OMX_ErrorNotReady\

#define EXECUTE_ALL(_VECTOR_, FUNC, ...) \
    do { for(int i=0; i<_VECTOR_.size(); i++) { \
        MtkOmxVencStrategy* o = _VECTOR_[i]; \
        o->FUNC (__VA_ARGS__); \
    } } while(0)

template <class S_TYPE>
void MtkOmxVencExtension::mountStrategy()
{
    MtkOmxVencStrategy* strategy = new S_TYPE();

    this->mAllStrategies.push(strategy);
}

void MtkOmxVencExtension::unmountAllStrategies()
{
    this->mEnabledStrategies.clear();

    while(this->mAllStrategies.size() != 0)
    {
        MtkOmxVencStrategy* strategy = this->mAllStrategies[0];
        this->mAllStrategies.removeAt(0);
        delete strategy;
    }
}

MtkOmxVencExtension::MtkOmxVencExtension()
{
    mEnableMoreLog = (bool) MtkVenc::EnableMoreLog("0");

    // Add new extension here:
    //this->mountStrategy<CLASSNAME>();
    this->mountStrategy<MtkOmxVencRoi>();
}

MtkOmxVencExtension::~MtkOmxVencExtension()
{
    this->unmountAllStrategies();
}

void MtkOmxVencExtension::ComponentInit()
{
    EXECUTE_ALL(this->mEnabledStrategies, ComponentInit);
}

void MtkOmxVencExtension::ComponentDeInit()
{
    EXECUTE_ALL(this->mEnabledStrategies, ComponentDeInit);
}

void MtkOmxVencExtension::onInit(VENC_DRV_PARAM_ENC_T* param, VAL_HANDLE_T pDrvHandle)
{
    for(int i=0; i<this->mAllStrategies.size(); i++)
    {
        MtkOmxVencStrategy* strategy = this->mAllStrategies[i];
        if(strategy->shouldEnable())
        {
            this->mEnabledStrategies.push(strategy);
        }
    }

    EXT_LOGD("Extension enabled count %d", this->mEnabledStrategies.size());
    EXECUTE_ALL(this->mEnabledStrategies, onInit, param, pDrvHandle);
}

void MtkOmxVencExtension::onDeInit()
{
    EXECUTE_ALL(this->mEnabledStrategies, onDeInit);

    this->mEnabledStrategies.clear();
}

OMX_ERRORTYPE MtkOmxVencExtension::SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    EXECUTE_UNTIL_FIRST_ERRORNONE(this->mAllStrategies, SetParameter, nParamIndex, pCompParam);
}
OMX_ERRORTYPE MtkOmxVencExtension::GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    EXECUTE_UNTIL_FIRST_ERRORNONE(this->mAllStrategies, GetParameter, nParamIndex, pCompParam);
}
OMX_ERRORTYPE MtkOmxVencExtension::GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType)
{
    EXECUTE_UNTIL_FIRST_ERRORNONE(this->mAllStrategies, GetExtensionIndex, parameterName, pIndexType);
}
OMX_ERRORTYPE MtkOmxVencExtension::SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    EXECUTE_UNTIL_FIRST_ERRORNONE(this->mEnabledStrategies, SetConfig, nConfigIndex, pCompConfig);
}
OMX_ERRORTYPE MtkOmxVencExtension::GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    EXECUTE_UNTIL_FIRST_ERRORNONE(this->mEnabledStrategies, GetConfig, nConfigIndex, pCompConfig);
}
void MtkOmxVencExtension::UpdateInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, UpdateInputBuffer, pInputBuf, pFrameBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::UpdateOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf, VENC_DRV_PARAM_BS_BUF_T* pBsBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, UpdateOutputBuffer, pOutputBuf, pBsBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, OnEnqueBuffer, ppInputBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, OnEnqueOutputBuffer, ppOutputBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, DequeOnGetInputBuffer, pInputBuf, pFrameBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, VENC_DRV_PARAM_BS_BUF_T* pBsBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, DequeOnGetOutputBuffer, pOutputBuf, pBsBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pInputBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, EmptyThisBuffer, pInputBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::FillThisBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, FillThisBuffer, pOutputBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::EmptyBufferDone(OMX_BUFFERHEADERTYPE* pInputBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, EmptyBufferDone, pInputBuf);
    OUT_FUNC();
}
void MtkOmxVencExtension::FillBufferDone(OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    IN_FUNC();
    EXECUTE_ALL(this->mEnabledStrategies, FillBufferDone, pOutputBuf);
    OUT_FUNC();
}
