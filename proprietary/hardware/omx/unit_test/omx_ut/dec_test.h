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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   dec_test.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video unit test code
 *
 * Author:
 * -------
 *   Bruce Hsu (mtk04278)
 *
 ****************************************************************************/

#ifndef __DEC_TEST_H__
#define __DEC_TEST_H__

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
#include <gui/SurfaceComposerClient.h>
using namespace android;

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <HardwareAPI.h>

#define MAX_DEC_INPUT_BUF_NUM  24
#define MAX_DEC_OUTPUT_BUF_NUM 24
#define DEC_INPUT_BUF_SIZE     102400

#define MTK_OMX_AVC_DEC_COMP_NAME   "OMX.MTK.VIDEO.DECODER.AVC"
#define MTK_OMX_MP4_DEC_COMP_NAME   "OMX.MTK.VIDEO.DECODER.MPEG4"
#define MTK_OMX_VPX_DEC_COMP_NAME   "OMX.MTK.VIDEO.DECODER.VPX"
//#ifdef MTK_SUPPORT_MJPEG
#define MTK_OMX_MJPG_DEC_COMP_NAME  "OMX.MTK.VIDEO.DECODER.MJPEG"
//#endif//MTK_SUPPORT_MJPEG

#include "omx_func.h"

typedef enum
{
    DEC_CODEC_ZERO=0,
    DEC_CODEC_MP4,
    DEC_CODEC_AVC,
    DEC_CODEC_VP8,
//#ifdef MTK_SUPPORT_MJPEG
    DEC_CODEC_MJPG,
//#endif//MTK_SUPPORT_MJPEG
} DecTestCodec;

struct DecTestConf {
    DecTestCodec    eCodec;
    const char *szInFile1;
    const char *szInFile2;
    char    *szGolden;
    int     iPortReconfigType;//0: don't, 1: passive, 2: active
    int     iWidth1;//for iPortReconfigType 0 and 2
    int     iHeight1;//for iPortReconfigType 0 and 2
    int     iWidth2;//for iPortReconfigType 2
    int     iHeight2;//for iPortReconfigType 2
    int     iFrameNum;
    bool    bCompareGolden;
    bool    bFrame1NotI;
    bool    bPartialFrame;
    bool    bBufFlagLeak;
    bool    bNoEOS;
    bool    bPortReconfigFail;
    bool    bOutBufFull;
    bool    bNoSequenceHead;
    bool    bCorruptData;
    bool    bLoopPlayback;
    bool    bTrickPlay;
    bool    bMultiInstance;
    pthread_mutex_t tInitLock;
};

class DecTest {
public:
    int init(DecTestConf tConf);
    int prepare();
    int decode();
    int finalize();
    int deInit();
    int errorhandle();
    friend  void *DecTestEmptyBufThread(void *pData);
    friend  void *DecTestFillBufThread(void *pData);
    friend  OMX_ERRORTYPE DecTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
    friend  OMX_ERRORTYPE DecTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);
    friend  OMX_ERRORTYPE DecTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);

    int checkLastFrame();

private:
    static void             *mOmxLibHandle;
    static TOmxCoreFuncs    mOmxCoreFuncs;
    static pthread_mutex_t  mOmxCoreFuncLock;
    static int              mOmxInitCount;
    static int              mInitCount;
    int                 mIsPrepared;
    int                 mError;
    int                 mEos;
    int                 mFlushOk;
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
    DecTestConf         mConf;

    OMX_HANDLETYPE          mCompHndl;
    OMX_CALLBACKTYPE        mOmxCallback;
    OMX_BUFFERHEADERTYPE    *mInBufHdrs[MAX_DEC_INPUT_BUF_NUM];
    OMX_BUFFERHEADERTYPE    *mOutBufHdrs[MAX_DEC_OUTPUT_BUF_NUM];
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
    int     mOutStride;
    int     mOutSliceHeight;
    int     mOutPortBufSize;
    //for 1st frame not I
    int     mGet1stI;
    //for partial frame
    unsigned char   *mTmpBuf;
    int     mTmpBufLen;
    int     mTmpBufSize;
    int     mTmpTs;
    unsigned int    mTmpFlag;
    int     mInPortBufSize;
    //for out buf full
    int     mDoBufBlock;
    int     mFillBufCount;
    //for loop playback
    int     mMeetLoopEnd;
    int     mSpecialTestEOS;
    //for trick play
    int     mFirstILocation;
    int     mDoSeek;

    char            *mInFileName;
    FILE            *mFin;
    int             mFileBufSize;
    unsigned char   *mFileBuf;
    unsigned char   *mFileHead;
    unsigned char   *mFileTail;
    int             mGetConf;
    unsigned char   *mConfBuf;
    int             mConfLen;
    int             mTs;
    int             mGetIDR;
    int             mFileReset;
    int             mCount;
    struct timeval  mStartTime;

    //for last frame test
    int             mFillBufDoneWithDataCount;
    int             mAfterLastFrame;
    //for dump output
    FILE            *mFOut;
    //for buf flag leak
    int             mETBCount;

    //for multiple instance
    bool            mMultiple;

    int allocBuffers(unsigned int uiPortIndex);
    int deallocBuffers(unsigned int uiPortIndex);
    int useBuffers(unsigned int uiPortIndex);
    int freeBuffers(unsigned int uiPortIndex);
    int fillOneBuffer(int iOutBufHdrInx);
    int emptyOneBuffer(int iInBufHdrInx);
    int checkPortDef(unsigned int uiPortIndex, int iPortReconfigType, int iWidth, int iHeight);
    int portReconfig(int iActive);
    int simple_decode(int iFrameNum);
    int findMatchingBufferHdr(unsigned int uiPortIndex, OMX_BUFFERHEADERTYPE *pBuffer);
    int flush(unsigned int uiPortIndex);
    int readInputFrame(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag);

    int dumpOutputToFile(unsigned char *pData, int iDataLen, int iClose);
    //for corrupt data
    int putErrorPattern(unsigned char *pData, int iDataLen, float fErrorRate);

    //H264 reader
    int readH264Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag);
    int getH264NAL(unsigned char *pbyData, unsigned char *pbyEndAddr, int *piNalType, int *piNalLen);
    int findH264StartCode(unsigned char **ppbyData, unsigned char *pbyEndAddr);
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
    //end H264 reader

    //MPEG4 reader
    int readMPEG4Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag);
    unsigned char *findVOP(unsigned char *pBegin, int iMaxLen, int *piIsShortHeader);
    //end MPEG4 reader

    //VP8 reader
    int readVP8Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag);
    unsigned int findNextFrame(unsigned char *pbyData, int iMaxLen);
    //end VP8 reader

//#ifdef MTK_SUPPORT_MJPEG
    //MJPG reader
    int readMJPGInput(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag);
//#endif//MTK_SUPPORT_MJPEG
};

#endif//__DEC_TEST_H__

