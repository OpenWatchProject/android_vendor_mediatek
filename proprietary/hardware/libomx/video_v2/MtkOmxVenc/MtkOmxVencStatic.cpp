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

#define MTK_OMX_H263_ENCODER    "OMX.MTK.VIDEO.ENCODER.H263"
#define MTK_OMX_MPEG4_ENCODER   "OMX.MTK.VIDEO.ENCODER.MPEG4"
#define MTK_OMX_AVC_ENCODER     "OMX.MTK.VIDEO.ENCODER.AVC"
#define MTK_OMX_HEVC_ENCODER    "OMX.MTK.VIDEO.ENCODER.HEVC"
#define MTK_OMX_AVC_SEC_ENCODER "OMX.MTK.VIDEO.ENCODER.AVC.secure"
#define MTK_OMX_VP8_ENCODER     "OMX.MTK.VIDEO.ENCODER.VPX"

#define H264_MAX_BS_SIZE    1024*1024
#define HEVC_MAX_BS_SIZE    1024*1024
#define MP4_MAX_BS_SIZE     1024*1024
#define VP8_MAX_BS_SIZE     1024*1024

// Morris Yang 20120214 add for live effect recording [
#ifdef ANDROID_ICS
#include <ui/Rect.h>
//#include <ui/android_native_buffer.h> // for ICS
#include <android/native_window.h> // for JB
// #include <media/stagefright/HardwareAPI.h> // for ICS
#include <HardwareAPI.h> // for JB
//#include <media/stagefright/MetadataBufferType.h> // for ICS
#include <MetadataBufferType.h> // for JB

#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
#endif
// ]
#define OMX_CHECK_DUMMY
#include "../../../omx/core/src/MtkOmxCore.h"

#include <poll.h>

#include <utils/Trace.h>
//#include <utils/AndroidThreads.h>

////////for fence in M0/////////////
#include <ui/Fence.h>
#include <media/IOMX.h>
///////////////////end///////////

/////////////////////////// -------------------   globalc functions -----------------------------------------///////////
OMX_ERRORTYPE MtkVenc_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_ComponentInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentInit(hComponent, componentName);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                   OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SetCallbacks");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetCallbacks(hComponent, pCallBacks, pAppData);
    }

    return err;
}


OMX_ERRORTYPE MtkVenc_ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_ComponentDeInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentDeInit(hComponent);
        delete(MtkOmxBase *)pHandle->pComponentPrivate;
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_STRING componentName,
                                          OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                          OMX_OUT OMX_VERSIONTYPE *specVersion,
                                          OMX_OUT OMX_UUIDTYPE *componentUUID)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetComponentVersion");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetComponentVersion(hComponent, componentName,
                                                                              componentVersion, specVersion,
                                                                              componentUUID);
    }
    return err;
}

OMX_ERRORTYPE MtkVenc_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_COMMANDTYPE Cmd,
                                  OMX_IN OMX_U32 nParam1,
                                  OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SendCommand");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SendCommand(hComponent, Cmd, nParam1, pCmdData);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_INDEXTYPE nParamIndex,
                                   OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetParameter(hComponent, nParamIndex, pCompParam);
    }
    return err;
}

OMX_ERRORTYPE MtkVenc_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN  OMX_INDEXTYPE nParamIndex,
                                   OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetParameter(hComponent, nParamIndex,
                                                                       ComponentParameterStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING parameterName,
                                        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetExtensionIndex");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetExtensionIndex(hComponent, parameterName, pIndexType);
    }
    return err;
}

OMX_ERRORTYPE MtkVenc_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                               OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetState");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetState(hComponent, pState);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_AllocateBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->AllocateBuffer(hComponent, pBuffHead, nPortIndex,
                                                                         pAppPrivate, nSizeBytes);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                OMX_IN OMX_U32 nPortIndex,
                                OMX_IN OMX_PTR pAppPrivate,
                                OMX_IN OMX_U32 nSizeBytes,
                                OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    // MTK_OMX_LOGD ("MtkVenc_UseBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate,
                                                                    nSizeBytes, pBuffer);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_IN OMX_U32 nPortIndex,
                                 OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_FreeBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FreeBuffer(hComponent, nPortIndex, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_EmptyThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->EmptyThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_FillThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FillThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U8 *cRole,
                                        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_ComponentRoleEnum");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentRoleEnum(hComponent, cRole, nIndex);
    }
    return err;
}


// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{

    MtkOmxBase *pVenc  = new MtkOmxVenc;

    if (NULL == pVenc)
    {
        ALOGE("[0x%08x] ""MtkOmxComponentCreate out of memory!!!", pVenc);
        return NULL;
    }

    OMX_COMPONENTTYPE *pHandle = pVenc->GetComponentHandle();
    ALOGD("[0x%08x] ""MtkOmxComponentCreate mCompHandle(0x%08X)", pVenc, (unsigned int)pHandle);

    pHandle->SetCallbacks                  = MtkVenc_SetCallbacks;
    pHandle->ComponentDeInit               = MtkVenc_ComponentDeInit;
    pHandle->SendCommand                   = MtkVenc_SendCommand;
    pHandle->SetParameter                  = MtkVenc_SetParameter;
    pHandle->GetParameter                  = MtkVenc_GetParameter;
    pHandle->GetExtensionIndex        = MtkVenc_GetExtensionIndex;
    pHandle->GetState                      = MtkVenc_GetState;
    pHandle->SetConfig                     = MtkVenc_SetConfig;
    pHandle->GetConfig                     = MtkVenc_GetConfig;
    pHandle->AllocateBuffer                = MtkVenc_AllocateBuffer;
    pHandle->UseBuffer                     = MtkVenc_UseBuffer;
    pHandle->FreeBuffer                    = MtkVenc_FreeBuffer;
    pHandle->GetComponentVersion           = MtkVenc_GetComponentVersion;
    pHandle->EmptyThisBuffer            = MtkVenc_EmptyThisBuffer;
    pHandle->FillThisBuffer                 = MtkVenc_FillThisBuffer;

    MtkVenc_ComponentInit((OMX_HANDLETYPE)pHandle, componentName);

    return pHandle;
}

extern "C" void MtkOmxSetCoreGlobal(OMX_COMPONENTTYPE *pHandle, void *data)
{
    ((mtk_omx_core_global *)data)->video_instance_count++;
    ((MtkOmxBase *)(pHandle->pComponentPrivate))->SetCoreGlobal(data);
}
