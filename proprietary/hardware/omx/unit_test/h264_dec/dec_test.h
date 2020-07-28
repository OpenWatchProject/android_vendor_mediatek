/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

//#define USE_NATIVE_BUFFER

#define CPP_STL_SUPPORT 1
#if CPP_STL_SUPPORT
#include <vector>
using namespace std;
#endif

#include <ui/GraphicBuffer.h>
#include <utils/String8.h>
//#include <surfaceflinger/SurfaceComposerClient.h>
#include <SurfaceComposerClient.h>
using namespace android;

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <HardwareAPI.h>

#define INPUT_PORT_INDEX    0
#define OUTPUT_PORT_INDEX   1
#define ALL_PORT_INDEX      0xFFFFFFFF

#define MAX_H264_INPUT_BUF_NUM  24
#define MAX_H264_OUTPUT_BUF_NUM 24
#define H264_INPUT_BUF_SIZE     102400

#define OMX_CORE_PATHFILENAME   "/vendor/lib/libMtkOmxCore.so"
#define MTK_OMX_DEC_COMP_NAME   "OMX.MTK.VIDEO.DECODER.AVC"

#define LOGE(x) x
#define LOGD(x) x
#define LOGI(x)

typedef OMX_ERRORTYPE (*PfOMX_Init)(void);
typedef OMX_ERRORTYPE (*PfOMX_Deinit)(void);
typedef OMX_ERRORTYPE (*PfOMX_ComponentNameEnum)    (OMX_OUT OMX_STRING cComponentName, OMX_IN      OMX_U32 nNameLength,        OMX_IN      OMX_U32 nIndex);
typedef OMX_ERRORTYPE (*PfOMX_GetHandle)            (OMX_OUT OMX_HANDLETYPE* pHandle,   OMX_IN      OMX_STRING cComponentName,  OMX_IN      OMX_PTR pAppData,       OMX_IN  OMX_CALLBACKTYPE* pCallBacks);
typedef OMX_ERRORTYPE (*PfOMX_FreeHandle)           (OMX_IN  OMX_HANDLETYPE hComponent);
typedef OMX_ERRORTYPE (*PfOMX_SetupTunnel)          (OMX_IN  OMX_HANDLETYPE hOutput,    OMX_IN      OMX_U32 nPortOutput,        OMX_IN      OMX_HANDLETYPE hInput,  OMX_IN  OMX_U32 nPortInput);
typedef OMX_ERRORTYPE (*PfOMX_GetContentPipe)       (OMX_OUT OMX_HANDLETYPE *hPipe,     OMX_IN      OMX_STRING szURI);
typedef OMX_ERRORTYPE (*PfOMX_GetComponentsOfRole)  (OMX_IN  OMX_STRING role,           OMX_INOUT   OMX_U32 *pNumComps,         OMX_INOUT   OMX_U8  **compNames);
typedef OMX_ERRORTYPE (*PfOMX_GetRolesOfComponent)  (OMX_IN  OMX_STRING compName,       OMX_INOUT   OMX_U32 *pNumRoles,         OMX_OUT     OMX_U8 **roles);

typedef struct omx_core_function {
    PfOMX_Init                 pf_OMX_Init;
    PfOMX_Deinit               pf_OMX_Deinit;
    PfOMX_ComponentNameEnum    pf_OMX_ComponentNameEnum;
    PfOMX_GetHandle            pf_OMX_GetHandle;
    PfOMX_FreeHandle           pf_OMX_FreeHandle;
    PfOMX_SetupTunnel          pf_OMX_SetupTunnel;          //SetupTunnel is NULL!
    PfOMX_GetContentPipe       pf_OMX_GetContentPipe;
    PfOMX_GetComponentsOfRole  pf_OMX_GetComponentsOfRole;
    PfOMX_GetRolesOfComponent  pf_OMX_GetRolesOfComponent;
} TOmxCoreFuncs;

class DecTest {
public:
    int init(const char *szFileName);
    int prepare();
    int decode();
    int finalize();
    int deInit();
    friend  void *DecTestEmptyBufThread(void *pData);
    friend  void *DecTestFillBufThread(void *pData);
    friend  OMX_ERRORTYPE DecTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
    friend  OMX_ERRORTYPE DecTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);
    friend  OMX_ERRORTYPE DecTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);

private:
    void                *mOmxLibHandle;
    TOmxCoreFuncs       mOmxCoreFuncs;
    int                 mIsPrepared;
    int                 mError;
    int                 mEos;
    int                 mPortChange;
    int                 mWaitPortCmd;
    sem_t               mEmptyBufSem;
    sem_t               mFillBufSem;
    sem_t               mEmptyBufStartSem;
    sem_t               mFillBufStartSem;
    pthread_mutex_t     mAvailInQLock;
    pthread_mutex_t     mAvailOutQLock;
    pthread_cond_t      mCommonCond;
    pthread_mutex_t     mCommonLock;
    pthread_t           mEmptyBufThread;
    pthread_t           mFillBufThread;
    int                 mEmptyThreadStop;
    int                 mFillThreadStop;

    OMX_HANDLETYPE          mCompHndl;
    OMX_CALLBACKTYPE        mOmxCallback;
    OMX_BUFFERHEADERTYPE    *mInBufHdrs[MAX_H264_INPUT_BUF_NUM];
    OMX_BUFFERHEADERTYPE    *mOutBufHdrs[MAX_H264_OUTPUT_BUF_NUM];
    OMX_STATETYPE           mCompState;

    vector<int>     mAvailInBufInx;
    int             mInBufNum;
    int             mInBufSize;
    unsigned char   **mInBufs;//buffer array, every buffer must be 32 bytes alignment!! very important!!
    vector<int>     mAvailOutBufInx;
    int             mOutBufNum;
    int             mOutBufSize;
    unsigned char   **mOutBufs;//buffer array, every buffer must be 32 bytes alignment!! very important!!

    int     mMaxWidth;
    int     mMaxHeight;
    int     mOutWidth;
    int     mOutHeight;

    char            *mInFileName;
    FILE            *mFin;
    int             mFileBufSize;
    unsigned char   *mFileBuf;
    unsigned char   *mFileHead;
    unsigned char   *mFileTail;
    unsigned char   *mConfBuf;
    int             mConfLen;
    int             mTs;
    int             mGetIDR;
    int             mCount;
#ifdef USE_NATIVE_BUFFER
    struct timeval  mStartTime;
#endif

    //for dump output
    FILE            *mFOut;
#ifdef USE_NATIVE_BUFFER
    sp<SurfaceComposerClient>   mComposerClient;
    sp<SurfaceControl>          mSurfaceControl;
    sp<ANativeWindow>           mNativeWindow;
    vector< sp<GraphicBuffer> > mGraphicBuffers;
    //vector<ANativeWindowBuffer> mANBs;
    int                         mANBOwned[MAX_H264_OUTPUT_BUF_NUM];
    OMX_INDEXTYPE               mOmxUseANBInx;
#endif//USE_NATIVE_BUFFER
    int allocBuffers(unsigned int uiPortIndex);
    int deallocBuffers(unsigned int uiPortIndex);
    int useBuffers(unsigned int uiPortIndex);
    int freeBuffers(unsigned int uiPortIndex);
    int fillOneBuffer(int iOutBufHdrInx);
    int emptyOneBuffer(int iInBufHdrInx);
    int checkPortDef(unsigned int uiPortIndex);
    int portReconfig();
    int findMatchingBufferHdr(unsigned int uiPortIndex, OMX_BUFFERHEADERTYPE *pBuffer);
    int readInputFrame(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag);
    int getH264NAL(unsigned char *pbyData, unsigned char *pbyEndAddr, int *piNalType, int *piNalLen);
    int findH264StartCode(unsigned char **ppbyData, unsigned char *pbyEndAddr);

    int dumpOutputToFile(unsigned char *pData, int iDataLen);

    typedef enum
    {
        Delimiter_No,
        Delimiter_3Trailing0,
        Delimiter_StartCode,
    } EDelimiter;

    typedef enum
    {
        FindStartCodeErr_NotFind   = -2,
        FindStartCodeErr_Error     = -3
    } EFSCodeErr;

    typedef enum
    {
        GetNalErr_NullData  = -1,
        GetNalErr_NotFind   = -2,
        GetNalErr_PrsErr    = -3,
        GetNalErr_Excpt     = -4
    } EGetNalErr;
};

