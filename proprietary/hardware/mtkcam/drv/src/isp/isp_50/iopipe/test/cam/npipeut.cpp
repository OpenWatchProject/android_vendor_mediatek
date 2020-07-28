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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "npipeut"

#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <queue>
#include <semaphore.h>
#include <pthread.h>

#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/threads.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>

#include <mtkcam/def/PriorityDefs.h>

using namespace NSCam;
using namespace NSCam::Utils;
using namespace android;
using namespace std;


#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>


/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <imageio/Cam_Notify_datatype.h>
#include <tuning_mgr.h>
#include <mtkcam/drv/IHwSyncDrv.h>
#include "pic/bpci_tbl_s5k2x8.h"    //for PDO test
#include "cam_crop.h"              //for crop test

#include "npipeut.h"

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

static char *mThisName = (char*)"main_camio";

static PortID sttPortIDMap[__MAXDMAO_STT] = {PORT_AAO, PORT_AFO, PORT_FLKO, PORT_PDO, PORT_PSO};
inline MINT InverseFmt(MINT Fmt)
{
    switch(Fmt) {
    case eImgFmt_UFO_BAYER8: return eImgFmt_BAYER8;
    case eImgFmt_UFO_BAYER10: return eImgFmt_BAYER10;
    case eImgFmt_UFO_BAYER12: return eImgFmt_BAYER12;
    case eImgFmt_UFO_BAYER14: return eImgFmt_BAYER14;
    case eImgFmt_UFO_FG_BAYER8: return eImgFmt_FG_BAYER8;
    case eImgFmt_UFO_FG_BAYER10: return eImgFmt_FG_BAYER10;
    case eImgFmt_UFO_FG_BAYER12: return eImgFmt_FG_BAYER12;
    case eImgFmt_UFO_FG_BAYER14: return eImgFmt_FG_BAYER14;
    case eImgFmt_BAYER8: return eImgFmt_UFO_BAYER8;
    case eImgFmt_BAYER10: return eImgFmt_UFO_BAYER10;
    case eImgFmt_BAYER12: return eImgFmt_UFO_BAYER12;
    case eImgFmt_BAYER14: return eImgFmt_UFO_BAYER14;
    case eImgFmt_FG_BAYER8: return eImgFmt_UFO_FG_BAYER8;
    case eImgFmt_FG_BAYER10: return eImgFmt_UFO_FG_BAYER10;
    case eImgFmt_FG_BAYER12: return eImgFmt_UFO_FG_BAYER12;
    case eImgFmt_FG_BAYER14: return eImgFmt_UFO_FG_BAYER14;
    default: MY_LOGD("unsupported uf fmt:0x%x", Fmt);break;
    }
    return eImgFmt_UNKNOWN;
}

void sttProcLoop(MUINT32 dmao, NPipeUT* _this)
{
    MUINT32 portIdx = dmao;

    ::pthread_detach(::pthread_self());

    MY_LOGD("S_%d: sttThread_%d: %d", _this->mSensorIdx, dmao, gettid());

    for ( ; MFALSE == _this->m_bStop; ) {
        QBufInfo    rDQBuf, rEQBuf;
        MUINT32     i;

        /**
         * Blocking wait deque done
         */

        _this->mSttCntMutex.lock();
        if (MFALSE == _this->mpSttPipe->deque(sttPortIDMap[portIdx], rDQBuf)) {
             // deque fail, un-necessary to increase deque cnt
             _this->mSttCntMutex.unlock();

             if (_this->mSuspendState) {
                MUINT32 _cnt = 0;

                MY_LOGD("stt[%d] under suspending ....... ",dmao);

                while (_this->mSuspendState == 1) {
                    if (_this->m_bStop == MTRUE) {
                        break;
                    }
                    if ((_cnt & 0x7F) == 0) {
                        MY_LOGD("stt[%d] under suspending: %d",dmao,_cnt);
                    }
                    usleep(10000);
                    _cnt++;
                }

                continue;
            }
            else {
                MY_LOGE("S_%d: dma0x%x deq failed...", _this->mSensorIdx, sttPortIDMap[portIdx].index);
                usleep(1000);
                continue;
            }
        }
        else {
            // deque success, increase deque cnt
            _this->mSttDeqCnt++;
            _this->mSttCntMutex.unlock();

            MY_LOGI("stt[%d] deque success ....... ", dmao);
        }

        /**
         * Do something after deque buf
         */
        if(dmao != 3){//pdo support no memcpy
            memcpy(_this->mpSttBuf[portIdx], (void*)rDQBuf.mvOut.at(0).mVa, _this->mpSttBufSize[portIdx]);
            _this->mpSttBufSize[portIdx] = rDQBuf.mvOut.at(0).mSize;
            MY_LOGD("Copy %p <- %p, %dbytes", _this->mpSttBuf[portIdx], (void*)rDQBuf.mvOut.at(0).mVa, _this->mpSttBufSize[portIdx]);
        }

        /**
         * Enque back buffers
         * Only fill necessary fields
         */
        rEQBuf.mvOut.resize(rDQBuf.mvOut.size());
        for (i = 0; i < rDQBuf.mvOut.size(); i++) {
            MY_LOGD("S_%d: [deque_stt]dma:0x%x PA:%lx VA:%p Size(%d) mag:0x%x", _this->mSensorIdx,\
                    sttPortIDMap[portIdx].index,\
                    rDQBuf.mvOut.at(i).mPa, (void*)rDQBuf.mvOut.at(i).mVa, rDQBuf.mvOut.at(i).mSize,\
                    rDQBuf.mvOut.at(i).mMetaData.mMagicNum_tuning);
            rEQBuf.mvOut.at(i).mPortID  = sttPortIDMap[portIdx];
            rEQBuf.mvOut.at(i).mPa      = rDQBuf.mvOut.at(i).mPa;
            rEQBuf.mvOut.at(i).mVa      = rDQBuf.mvOut.at(i).mVa;
            rEQBuf.mShutterTimeNs = 10 * 1000 * 1000 * 1000;
        }
        MY_LOGD("S_%d: [enque_stt] dma:0x%x", _this->mSensorIdx, sttPortIDMap[portIdx].index);
        while (1) {
            _this->mSttCntMutex.lock();
            if(_this->mpSttPipe->enque(rEQBuf)){
                // enque success, increase enque cnt
                _this->mSttEnqCnt++;
                _this->mSttCntMutex.unlock();
                break;
            }
            else{
                // enque fail, un-necessary to increase enque cnt
                _this->mSttCntMutex.unlock();
            }

            if (_this->mSuspendState) {
                MUINT32 _cnt = 0;

                MY_LOGD("stt[%d] under suspending ....... ",dmao);

                while (_this->mSuspendState == 1) {
                    if (_this->m_bStop == MTRUE) {
                        goto _STT_STOP;
                    }
                    if ((_cnt & 0x7F) == 0) {
                        MY_LOGD("stt[%d] under suspending: %d",dmao,_cnt);
                    }
                    usleep(10000);
                    _cnt++;
                }
            }
            else {
                MY_LOGE("S_%d: dma0x%x enq failed...", _this->mSensorIdx, sttPortIDMap[portIdx].index);
                break;
            }
        }
    }

_STT_STOP:
    ::sem_post(&_this->m_semSttProc[portIdx]);

    return;
}

void *_sttProcLoop_AAO_ (void *arg)
{
    sttProcLoop(__AAO, (NPipeUT*)arg);
    return 0;
}
void *_sttProcLoop_AFO_ (void *arg)
{
    sttProcLoop(__AFO, (NPipeUT*)arg);
    return 0;
}
void *_sttProcLoop_FLKO_ (void *arg)
{
    sttProcLoop(__FLKO, (NPipeUT*)arg);
    return 0;
}

void *_sttProcLoop_PDO_ (void *arg)
{
    sttProcLoop(__PDO, (NPipeUT*)arg);
    return 0;
}

void *_sttProcLoop_PSO_ (void *arg)
{
    sttProcLoop(__PSO, (NPipeUT*)arg);
    return 0;
}

const ThreadR_t sttProcRoutine[] = {
        _sttProcLoop_AAO_,
        _sttProcLoop_AFO_,
        _sttProcLoop_FLKO_,
        _sttProcLoop_PDO_,
        _sttProcLoop_PSO_
};

MVOID* deque_loop(void* arg)
{
    NPipeUT*        _this = (NPipeUT*)arg;
    QPortID         deqPorts;
    QBufInfo        dequeBufInfo;
    MUINT32         i = 0;

    MY_LOGD("start deque_loop");

    ::pthread_detach(::pthread_self());

    if (_this->m_enablePort & __IMGO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_IMGO);
    }
    if (_this->m_enablePort & __RRZO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_RRZO);
    }
    if (_this->m_enablePort & __EISO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_EISO);
    }
    if (_this->m_enablePort & __RSSO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_RSSO);
    }
    if (_this->m_enablePort & __LCSO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_LCSO);
    }
    while(1){
        if(_this->mSuspendFlow == (MUINT32)NONSTOP_SUSPEND){
            if(_this->m_bStop)
                break;
            else if(_this->mLoopCount > _this->mFrameNum )
                continue;
        }
        else{
            if(_this->mLoopCount > _this->mFrameNum)
                break;
        }

        if (_this->mpNPipe->deque(deqPorts, dequeBufInfo, 19527) == MTRUE) {
            for (i = 0; i < dequeBufInfo.mvOut.size(); i++) {
                if (0 == dequeBufInfo.mvOut.at(i).mBuffer) {
                    MY_LOGE("deque wrong");
                    break;
                }
                if(_this->mSecEnable)
                MY_LOGD("S_%d[deque]: dma:0x%x, sec PA:%lx, crop:%d_%d_%d_%d_%d_%d_%d_%d, size:%dx%d, mag:0x%x, bufidx:0x%x",\
                    _this->mSensorIdx,
                    dequeBufInfo.mvOut.at(i).mPortID.index, dequeBufInfo.mvOut.at(i).mPa,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.y,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.y,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.w,dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning,\
                    dequeBufInfo.mvOut.at(i).mBufIdx);
                else
                MY_LOGD("S_%d[deque]: dma:0x%x, PA:%lx, crop:%d_%d_%d_%d_%d_%d_%d_%d, size:%dx%d, mag:0x%x, bufidx:0x%x",\
                    _this->mSensorIdx,
                    dequeBufInfo.mvOut.at(i).mPortID.index, dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0),\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.y,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.y,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.w,dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning,\
                    dequeBufInfo.mvOut.at(i).mBufIdx);

                #if 0
                onDumpBuffer("camio", _this->mSensorIdx, dequeBufInfo.mvOut.at(i).mPortID.index,
                        (MUINTPTR)dequeBufInfo.mvOut.at(i).mBuffer, _this->mLoopCount);
                #endif
            }

            _this->mDeqCount += _this->mBurstQNum;//dequeBufInfo.mvOut.size();
            _this->mLoopCount++;
        }
        else {
            _this->mFailCount++;
            MY_LOGE("S_%d deque fail", _this->mSensorIdx);

            if (_this->mFailCount > 1) {
                MY_LOGE("S_%d deque break", _this->mSensorIdx);
                while (1) {
                    MY_LOGE("USE CVD");
                    sleep(1);
                }
                break;
            }
        }

        if(_this->m_bEfuse == MTRUE) {
            MY_LOGD("S_%d deque break", _this->mSensorIdx);
            break;
        }

        if (_this->mSuspendStart) {
            MY_LOGD("Entern suspend flow at frame#%d", _this->mLoopCount);
            _this->mSuspendState = 1;
            _this->mpNPipe->suspend();
            _this->mSuspendStart = MFALSE;

            switch (_this->mSuspendFlow) {
            case SUSPEND_STOP:
                MY_LOGD("Stop when suspend");
                _this->m_bStop = MTRUE;
                _this->mSuspendState = 0;

                goto EXIT_DEQ_LOOP;
            case ONE_TIME_SUSPEND:
            case NONSTOP_SUSPEND:
            default:
                {
                    MUINT32     sof_idx = 0, *pMag, enqBufIdx;
                    QBufInfo    enqQ;

                    MY_LOGD("Resume shutter time 100 ...");

                    #if 1
                    if ((_this->mEnqCount + _this->mBurstQNum) > (_this->mDeqCount + _this->mPortBufDepth)) {
                        MY_LOGE("No empty buffer for enque resume fail");
                        goto EXIT_DEQ_LOOP;
                    }

                    _this->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sof_idx, 0, 0);
                    MY_LOGD("@SOF_%d", sof_idx);

                    enqBufIdx = _this->mEnqCount % _this->mPortBufDepth;

                    enqQ.mvOut.clear();
                    enqQ.mShutterTimeNs = 10 * 1000 * 1000 * 1000;
                    pMag = (MUINT32*)malloc(sizeof(MUINT32)*_this->mBurstQNum);
                    for (i = enqBufIdx; i < (enqBufIdx + _this->mBurstQNum); i++, _this->mMagicNum++) {
                        MBOOL bin_en = MFALSE;
                        MUINT32 bin_w,bin_h;

                        _this->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO, (MINTPTR)&bin_w, (MINTPTR)&bin_h, 0);
                        bin_en = ((MINT32)bin_w != _this->mTgSize.w)? (1):(0);

                        if (_this->m_enablePort & __IMGO_ENABLE) {
                            IImageBuffer* pIBuf;
                            MRect _cropsize;
                            MSize imgo_size[2];

                            if(_this->m_ufeo_en == MTRUE && _this->mMagicNum % 3 == 0){ // for per frame control test
                                pIBuf = _this->mpImgBuffer[__UFEO].at(i);
                            }
                            else{
                                pIBuf = _this->mpImgBuffer[__IMGO].at(i);
                            }

                            _this->mpNPipe->sendCommand(ENPipeCmd_GET_IMGO_INFO,(MINTPTR)imgo_size,0,0);
                            if(imgo_size[0].w == 0)
                                _this->m_crop.GetCropInfo(PORT_IMGO,(EImageFormat)pIBuf->getImgFormat(),_cropsize,MTRUE);
                            else
                                _this->m_crop.GetCropInfo(PORT_IMGO,(EImageFormat)pIBuf->getImgFormat(),_cropsize,MFALSE);

                            enqQ.mvOut.push_back(BufInfo(PORT_IMGO, pIBuf, _cropsize.s,
                                                    _cropsize,_this->mMagicNum, sof_idx, ((_this->m_enablePort & __IMGO_PURE_RAW) && (_this->mMagicNum % 3 == 0))?1:0, 0xFFFF));
                        }
                        if (_this->m_enablePort & __RRZO_ENABLE) {
                            IImageBuffer* pIBuf;
                            MRect _cropsize;
                            MSize _rrz;

                            if(_this->m_ufgo_en == MTRUE && _this->mMagicNum % 3 == 0){ // for per frame control test
                                pIBuf = _this->mpImgBuffer[__UFGO].at(i);
                            }
                            else{
                                pIBuf = _this->mpImgBuffer[__RRZO].at(i);
                            }

                            _this->m_crop.GetCropInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_cropsize);
                            _this->m_crop.GetScaleInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_rrz,_this->mScaleSel,_this->mScaleFactor);

                            enqQ.mvOut.push_back(BufInfo(PORT_RRZO, pIBuf, _rrz,\
                                                    _cropsize,_this->mMagicNum, sof_idx, 0xFFFF));
                        }
                        if (_this->m_enablePort & __EISO_ENABLE) {
                            IImageBuffer*   pIBuf = _this->mpImgBuffer[__EISO].at(i);
                            MSize _ssize = _this->mEisNotify.queryEisOutSize();

                            enqQ.mvOut.push_back(BufInfo(PORT_EISO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), _this->mMagicNum, sof_idx, 0xFFFF));
                        }
                        if (_this->m_enablePort & __LCSO_ENABLE) {
                            IImageBuffer*   pIBuf = _this->mpImgBuffer[__LCSO].at(i);
                            MSize _ssize = _this->mLcsNotify.queryLcsOutSize(_this->mTgSize);

                            enqQ.mvOut.push_back(BufInfo(PORT_LCSO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), _this->mMagicNum, sof_idx, 0xFFFF));
                        }
                        pMag[i-enqBufIdx] = _this->mMagicNum;

                    }
                    if (_this->mEnableSttPort) {
                        _this->setTuning(pMag, TYPE_SET_TUNING, NULL, NULL);
                    }

                    #if (TEST_ZHDR_HW == 1)
                    _this->setZHdrTuning(pMag);
                    #endif

                    free(pMag);

                    for (i = 0; i < enqQ.mvOut.size(); i++) {
                        MY_LOGD("S_%d[enque] dma:0x%x sof:%d PA:%lx crop:%d_%d_%d_%d size:%dx%d mag:0x%x",\
                                _this->mSensorIdx, enqQ.mvOut.at(i).mPortID.index, sof_idx, enqQ.mvOut.at(i).mBuffer->getBufPA(0),\
                                enqQ.mvOut.at(i).FrameBased.mCropRect.p.x,enqQ.mvOut.at(i).FrameBased.mCropRect.p.y,\
                                enqQ.mvOut.at(i).FrameBased.mCropRect.s.w,enqQ.mvOut.at(i).FrameBased.mCropRect.s.h,\
                                enqQ.mvOut.at(i).FrameBased.mDstSize.w,enqQ.mvOut.at(i).FrameBased.mDstSize.h,\
                                enqQ.mvOut.at(i).FrameBased.mMagicNum_tuning);
                    }
                    #endif ///////////////////////////////////////////

                    while(1){
                        if(_this->mResumeStart)
                            break;
                        if(_this->m_bStop)
                            goto EXIT_DEQ_LOOP;
                        usleep(3000);
                    }
                    _this->mpNPipe->resume(&enqQ);
                    _this->mEnqCount += _this->mBurstQNum;
                    _this->mSuspendState = 0;
                    _this->mResumeStart = MFALSE;
                    break;
                }
            }
        }
    }

EXIT_DEQ_LOOP:

    _this->m_bStop = MTRUE;

    MY_LOGD("S_%d exit deque_loop, stop", _this->mSensorIdx);

    ::sem_post(&_this->m_semDeqThd);

    return NULL;
}

#define _CHECK_SUSPENDING(_nUtPtr)  \
    do { \
        MUINT32 _cnt = 0;\
        while (_nUtPtr->mSuspendState == 1) {\
            if (_nUtPtr->m_bStop == MTRUE) {\
                break;\
            }\
            if ((_cnt & 0x7F) == 0) {\
                MY_LOGD("under suspending: %d",_cnt);\
            }\
            usleep(10000);\
            _cnt++;\
        }\
    } while(0)

MVOID* enque_loop(void* arg)
{
    NPipeUT*        _this = (NPipeUT*)arg;
    QBufInfo        enqQ;
    MUINT32         i = 0, sof_idx = 0, enqBufIdx = 0;
    MUINT32         *pMag = NULL;

    MY_LOGD("start enque_loop");

    ::pthread_detach(::pthread_self());

    _this->mSofUserKey = _this->mpNPipe->attach("UTEnqLoop");

    if (_this->mSofUserKey < 0) {
        MY_LOGE("Wrong userkey : %d", _this->mSofUserKey);
        return NULL;
    }

    _this->mMagicNum = _this->mEnqCount;
    while (!_this->m_bStop) {
        if ((_this->mEnqCount + _this->mBurstQNum) > (_this->mDeqCount + _this->mPortBufDepth)) {
            MY_LOGD("[%d] No empty buffer for enque, wait (%d_%d/%d_%d)",
                _this->mSensorIdx,_this->mEnqCount,_this->mBurstQNum, _this->mDeqCount,_this->mPortBufDepth);
            do {
                MY_LOGD("[%d]    enq/deq : %d/%d", _this->mSensorIdx, _this->mEnqCount, _this->mDeqCount);//tmp
                usleep(100000); //sleep 100ms to wait deque
            } while (((_this->mEnqCount + _this->mBurstQNum) > (_this->mDeqCount + _this->mPortBufDepth)) && (MFALSE == _this->m_bStop));
            if (_this->m_bStop) {
                break;
            }
        }
        _CHECK_SUSPENDING(_this);

        _this->mpNPipe->wait(EPipeSignal_SOF, EPipeSignal_ClearWait, _this->mSofUserKey, 7788);

        if (_this->m_bStop) {
            MY_LOGD("Stop after wait SOF");
            break;
        }
        _CHECK_SUSPENDING(_this);

        enqBufIdx = _this->mEnqCount % _this->mPortBufDepth;

        _this->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sof_idx, 0, 0);
        MY_LOGD("@SOF_%d", sof_idx);

        enqQ.mvOut.clear();
        enqQ.mShutterTimeNs = 10 * 1000 * 1000 * 1000;
        pMag = (MUINT32*)malloc(sizeof(MUINT32)*_this->mBurstQNum);
        for (i = enqBufIdx; i < (enqBufIdx + _this->mBurstQNum); i++, _this->mMagicNum++) {
            MBOOL bin_en = MFALSE;
            MUINT32 bin_w, bin_h;
            MSize _size;

            _this->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO, (MINTPTR)&bin_w, (MINTPTR)&bin_h, 0);
            bin_en = (bin_w != (MUINT32)_this->mTgSize.w)? (1):(0);
            _size.w = bin_w;
            _size.h = bin_h;
            _this->m_crop.PatchPipeLineIn(_size);

            if (_this->m_enablePort & __IMGO_ENABLE) {
                IImageBuffer* pIBuf;
                MRect _cropsize;

                pIBuf = _this->mpImgBuffer[__IMGO].at(i);

                {
                    MSize imgo_size[2];
                    EImageFormat fmt;
                    if (_this->mbRanUF) {
                        pIBuf->unlockBuf(mThisName);
                        fmt = (EImageFormat)((_this->mMagicNum % 3) ? InverseFmt(pIBuf->getImgFormat()) : pIBuf->getImgFormat());
                        ImgBufCreator creator(fmt);
                        pIBuf = _this->mpHeap[__IMGO].at(i)->createImageBuffer(&creator);
                        pIBuf->incStrong(pIBuf);
                        pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE |
                                                  eBUFFER_USAGE_SW_READ_OFTEN |
                                                  eBUFFER_USAGE_SW_WRITE_OFTEN);
                        _this->mpImgBuffer[__IMGO].at(i) = pIBuf;
                    } else if (_this->mbRanImgoFmt) {
                        pIBuf->unlockBuf(mThisName);
                        fmt = (EImageFormat)((_this->mMagicNum % 3) ? eImgFmt_BAYER10 : eImgFmt_BAYER12);
                        ImgBufCreator creator(fmt);
                        pIBuf = _this->mpHeap[__IMGO].at(i)->createImageBuffer(&creator);
                        pIBuf->incStrong(pIBuf);
                        pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE |
                                                  eBUFFER_USAGE_SW_READ_OFTEN |
                                                  eBUFFER_USAGE_SW_WRITE_OFTEN);
                        _this->mpImgBuffer[__IMGO].at(i) = pIBuf;
                        MY_LOGD("[RanImgoFmt] IMGO(%d) mMagicNum:0x%x fmt:0x%x", i, _this->mMagicNum, fmt);
                    }

                    _this->mpNPipe->sendCommand(ENPipeCmd_GET_IMGO_INFO,(MINTPTR)imgo_size,0,0);
                    if(imgo_size[0].w == 0)
                        _this->m_crop.GetCropInfo(PORT_IMGO,(EImageFormat)pIBuf->getImgFormat(),_cropsize,MTRUE);
                    else
                        _this->m_crop.GetCropInfo(PORT_IMGO,(EImageFormat)pIBuf->getImgFormat(),_cropsize,MFALSE);
                }
                if(_this->mSecEnable){
                    BufInfo _buf(PORT_IMGO, pIBuf, _cropsize.s,
                                        _cropsize,_this->mMagicNum, sof_idx, ((_this->m_enablePort & __IMGO_PURE_RAW))?1:0, 0xFFFF);
                    _buf.mSize  = _this->mImemSecBuf[__IMGO].at(i).size;
                    _buf.mVa    = _this->mImemSecBuf[__IMGO].at(i).virtAddr;
                    _buf.mPa    = _this->mImemSecBuf[__IMGO].at(i).phyAddr;
                    _buf.mMemID = _this->mImemSecBuf[__IMGO].at(i).memID;
                    _buf.mSecon = 1;
                    MY_LOGD("IMGO size:0x%x va:0x%x pa:0x%x memID:0x%x",_buf.mSize,_buf.mVa,_buf.mPa,_buf.mMemID);
                    enqQ.mvOut.push_back(_buf);
                }else{
                enqQ.mvOut.push_back(BufInfo(PORT_IMGO, pIBuf, _cropsize.s,
                                        _cropsize,_this->mMagicNum, sof_idx, ((_this->m_enablePort & __IMGO_PURE_RAW) && (_this->mMagicNum % 3 == 0))?1:0, 0xFFFF));
                }
            }
            if (_this->m_enablePort & __RRZO_ENABLE) {
                IImageBuffer* pIBuf;
                MRect _cropsize;
                MSize _rrz;

                pIBuf = _this->mpImgBuffer[__RRZO].at(i);

                if (_this->mbRanUF) {
                    EImageFormat fmt;
                    pIBuf->unlockBuf(mThisName);
                    fmt = (EImageFormat)((_this->mMagicNum % 3) ? InverseFmt(pIBuf->getImgFormat()) : pIBuf->getImgFormat());
                    ImgBufCreator creator(fmt);
                    pIBuf = _this->mpHeap[__RRZO].at(i)->createImageBuffer(&creator);
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE |
                                              eBUFFER_USAGE_SW_READ_OFTEN |
                                              eBUFFER_USAGE_SW_WRITE_OFTEN);
                    _this->mpImgBuffer[__RRZO].at(i) = pIBuf;
                }

                _this->m_crop.GetCropInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_cropsize);
                _this->m_crop.GetScaleInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_rrz,_this->mScaleSel,_this->mScaleFactor);
                if(_this->mSecEnable){
                    BufInfo _buf(PORT_RRZO, pIBuf, _rrz, _cropsize, _this->mMagicNum, sof_idx, 0xFFFF);
                    _buf.mSize  = _this->mImemSecBuf[__RRZO].at(i).size;
                    _buf.mVa    = _this->mImemSecBuf[__RRZO].at(i).virtAddr;
                    _buf.mPa    = _this->mImemSecBuf[__RRZO].at(i).phyAddr;
                    _buf.mMemID = _this->mImemSecBuf[__RRZO].at(i).memID;
                    _buf.mSecon = 1;
                    MY_LOGD("RRZO size:0x%x va:0x%x pa:0x%x memID:0x%x",_buf.mSize,_buf.mVa,_buf.mPa,_buf.mMemID);
                    enqQ.mvOut.push_back(_buf);
                }else{
                enqQ.mvOut.push_back(BufInfo(PORT_RRZO, pIBuf, _rrz,\
                                        _cropsize,_this->mMagicNum, sof_idx, 0xFFFF));
                }
            }
            if (_this->m_enablePort & __EISO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__EISO].at(i);
                MSize _ssize = _this->mEisNotify.queryEisOutSize();
                enqQ.mvOut.push_back(BufInfo(PORT_EISO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), _this->mMagicNum, sof_idx, 0xFFFF));
            }
            if (_this->m_enablePort & __RSSO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__RSSO].at(i);
                MSize _ssize = _this->mRssNotify.queryRssOutSize();

                enqQ.mvOut.push_back(BufInfo(PORT_RSSO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), _this->mMagicNum, sof_idx, 0xFFFF));
            }
            if (_this->m_enablePort & __LCSO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__LCSO].at(i);
                MSize _ssize = _this->mLcsNotify.queryLcsOutSize(_this->mTgSize);

                enqQ.mvOut.push_back(BufInfo(PORT_LCSO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), _this->mMagicNum, sof_idx, 0xFFFF));
            }
            pMag[i-enqBufIdx] = _this->mMagicNum;

        }
        if (_this->mEnableSttPort) {
            _this->setTuning(pMag, TYPE_SET_TUNING, NULL, NULL);
        }

        #if (TEST_ZHDR_HW == 1)
        _this->setZHdrTuning(pMag);
        #endif

        free(pMag);

        for (i = 0; i < enqQ.mvOut.size(); i++) {
            if(_this->mSecEnable)
            MY_LOGD("S_%d[enque] dma:0x%x sof:%d sec PA:%lx crop:%d_%d_%d_%d size:%dx%d mag:0x%x",\
                    _this->mSensorIdx, enqQ.mvOut.at(i).mPortID.index, sof_idx, enqQ.mvOut.at(i).mPa,\
                    enqQ.mvOut.at(i).FrameBased.mCropRect.p.x,enqQ.mvOut.at(i).FrameBased.mCropRect.p.y,\
                    enqQ.mvOut.at(i).FrameBased.mCropRect.s.w,enqQ.mvOut.at(i).FrameBased.mCropRect.s.h,\
                    enqQ.mvOut.at(i).FrameBased.mDstSize.w,enqQ.mvOut.at(i).FrameBased.mDstSize.h,\
                    enqQ.mvOut.at(i).FrameBased.mMagicNum_tuning);
            else
            MY_LOGD("S_%d[enque] dma:0x%x sof:%d PA:%lx crop:%d_%d_%d_%d size:%dx%d mag:0x%x",\
                    _this->mSensorIdx, enqQ.mvOut.at(i).mPortID.index, sof_idx, enqQ.mvOut.at(i).mBuffer->getBufPA(0),\
                    enqQ.mvOut.at(i).FrameBased.mCropRect.p.x,enqQ.mvOut.at(i).FrameBased.mCropRect.p.y,\
                    enqQ.mvOut.at(i).FrameBased.mCropRect.s.w,enqQ.mvOut.at(i).FrameBased.mCropRect.s.h,\
                    enqQ.mvOut.at(i).FrameBased.mDstSize.w,enqQ.mvOut.at(i).FrameBased.mDstSize.h,\
                    enqQ.mvOut.at(i).FrameBased.mMagicNum_tuning);
        }

        if(_this->mSensorIdx == 0 && _this->mEnqCount == 10) {
            if(_this->m_uniSwitch) {
                MUINT32 switchState=0;
                _this->mpNPipe->sendCommand(ENPipeCmd_GET_UNI_SWITCH_STATE, (MUINTPTR)&switchState, 0, 0);
                MY_LOGD("switchState=%d", switchState);
                if(switchState == 0) {
                    _this->mpNPipe->sendCommand(ENPipeCmd_UNI_SWITCH, 0, 0, 0);
                }
            }
        } else if(_this->mSensorIdx == 1 && _this->mEnqCount == 20) {
            if(_this->m_uniSwitch) {
                MUINT32 switchState=0;
                _this->mpNPipe->sendCommand(ENPipeCmd_GET_UNI_SWITCH_STATE, (MUINTPTR)&switchState, 0, 0);
                MY_LOGD("switchState=%d", switchState);
                if(switchState == 0) {
                    _this->mpNPipe->sendCommand(ENPipeCmd_UNI_SWITCH, 0, 0, 0);
                }
            }
        }

        if (_this->mpNPipe->enque(enqQ) == MTRUE) {
            #if (TEST_AE_HIGHSPEED == 1)
            static MUINT32 _last_sof = 0;
            MUINT32 sof_diff = (sof_idx > _last_sof)? (sof_idx - _last_sof): (_last_sof - sof_idx);
            if (sof_diff >= (_this->mBurstQNum*2)) {
                NSImageio::NSIspio::ISPIO_REG_CFG reg1;
                list<NSImageio::NSIspio::ISPIO_REG_CFG> input;

                reg1.Addr = 0x0D70;
                reg1.Data = _this->mEnqCount;
                input.push_back(reg1);

                _this->mpNPipe->sendCommand(ENPipeCmd_HIGHSPEED_AE, 2, (MINTPTR)&input, 1);
            }
            _last_sof = sof_idx;
            #endif

            _this->mDumpNotify.dumpRegister();
            _this->mEnqCount += _this->mBurstQNum;
        }
        else {
            MY_LOGD("S_%d[enque] NG try again", _this->mSensorIdx);
        }

        #if 0
        _this->mpNPipe->sendCommand(ENPipeCmd_SET_HW_PATH_CFG, (MUINTPTR)eCamHwPathCfg_One_TG, 0, 0);
        #endif
    }

    MY_LOGD("exit enque_loop");

    ::sem_post(&_this->m_semEnqThd);

    return NULL;
}


MVOID* hwsync_loop(void* arg)
{
    NPipeUT*  _this = (NPipeUT*)arg;
    MINT32 ret = 0;
    IHalSensorList*     pHalSensorList;
    IHalSensor*         pSensorHalObj;
    MUINT32 index = 0;
    MUINT32 sof_idx;
    int i = 0, j = 0;
    MUINT32 sensorNum = 2;
    MUINT32 sensorSen[2] = {0,0};
    MUINT32 expTime[2], frmTime[2];
    MINT32 userKey = 0;
    MUINT32 *ptr_expTime = expTime;
    MUINT32 *ptr_frmTime = frmTime;
    MUINT32 senDev;

#if 1
    ::pthread_detach(::pthread_self());

    _this->m_aeCyclePeriod = HWSYNC_AE_CYCLE_PERIOD;

    pHalSensorList = IHalSensorList::get();
    pSensorHalObj = pHalSensorList->createSensor("HWSyncDrvThread", 0); //get sensor handler
    if (pSensorHalObj == NULL) {
        LOG_ERR("mpSensorHalObj is NULL");
        return NULL;
    }

    HWSyncDrv* pHwSyncDrv = HWSyncDrv::createInstance();
    pHwSyncDrv->init(HW_SYNC_USER_AE, 0, _this->m_aeCyclePeriod);

    while (!_this->m_bStop) {
        // wait vsync
        _this->mpNPipe->wait(EPipeSignal_VSYNC, EPipeSignal_ClearWait, userKey, 200);

        _this->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sof_idx, 0, 0);
        LOG_INF("@SOF_%d, hwsync_loop", sof_idx);

        switch(i)
        {
            case 0:
                {
                    senDev = NSCam::SENSOR_DEV_MAIN;
                    expTime[0] = 30009;
                    expTime[1] = 30000;
                    LOG_DBG("senDev(%d), expTime[0]=%d, expTime[1]=%d, frmTime[0]=%d, frmTime[1]=%d",senDev, expTime[0],expTime[1],frmTime[0],frmTime[1]);
                    ret = pHwSyncDrv->sendCommand(HW_SYNC_CMD_GET_FRAME_TIME, senDev, (MUINTPTR)ptr_expTime, (MUINTPTR)ptr_frmTime);

                    if(ret > 0){
                    LOG_INF("[MAIN1] frmTime(%d) fps, expTime(%d) us", frmTime[0], expTime[0]);
                    LOG_INF("[MAIN2] frmTime(%d) fps, expTime(%d) us", frmTime[1], expTime[1]);
                    }
                    else{
                    LOG_ERR("sendCommand Fail");
                    }

                    if(frmTime[0] != 0) {
                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_MAIN, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, \
                                                    (MINTPTR)&expTime[0],(MINTPTR)&frmTime[0], 0);

                    }
                    else{
                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_MAIN, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_TIME, \
                                                    (MINTPTR)&expTime[0], 0, 0);
                    }

                    if(frmTime[1] != 0) {

                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_SUB, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, \
                                                    (MINTPTR)&expTime[1],(MINTPTR)&frmTime[1], 0);
                    }
                    else{
                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_SUB, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_TIME, \
                                                    (MINTPTR)&expTime[1],0, 0);
                    }


                }
                break;
            default:
                break;
        }

        i++;

        if((MUINT32)i == _this->m_aeCyclePeriod)
            i = 0;

    }

    pHwSyncDrv->uninit(HW_SYNC_USER_AE, 0);
    pHwSyncDrv->destroyInstance();

    ::sem_post(&_this->m_semHwsyncThd);
#endif

    return NULL;
}

MVOID* tg_loop(void* arg)
{
    NPipeUT*        _this = (NPipeUT*)arg;
    MUINT32         tglineno = 1000, nRet;
    MINT32          userKey = 0;
    MUINTPTR        bufAddr;

    MY_LOGD("start tg_loop");

    ::pthread_detach(::pthread_self());

    MUINT32 sdma = 0;
    PortID port;

    if(!_this->mpSttPipe){
        MY_LOGD("S_%d mpSttPipe=NULL\n", _this->mSensorIdx);
        goto EXIT_TG_LOOP;
    }

    //get any one stt opened port
    for (sdma = 0; sdma < __MAXDMAO_STT; sdma++) {
        if (_this->mEnableSttPort & (1 << sdma)) {
            break;
        }
    }
    if (sdma == __MAXDMAO_STT){
        MY_LOGD("S_%d no STT port enable\n", _this->mSensorIdx);
        goto EXIT_TG_LOOP;
    }

    port = sttPortIDMap[sdma];

    /*set TG interrupt*/
    MY_LOGD("S_%d set TG lineno(%d)\n", _this->mSensorIdx, tglineno);
    _this->mpSttPipe->sendCommand(ESPipeCmd_SET_TG_INT_LINE,(MINTPTR)tglineno, 0, 0);

    for ( ; MFALSE == _this->m_bStop; ) {
        /*wait tg irq*/
        MY_LOGD("S_%d wait TG interrupt\n", _this->mSensorIdx);
        nRet = _this->mpSttPipe->wait(EPipeSignal_TG_INT, EPipeSignal_ClearWait, userKey, 200);
        if(nRet == MFALSE) {
            MY_LOGD("S_%d wait TG int fail", _this->mSensorIdx);
            break;
        }
        /*get cur buf addr*/
        _this->mpSttPipe->sendCommand(ESPipeCmd_GET_STT_CUR_BUF,(MINTPTR)port, (MINTPTR)&bufAddr,0);
        MY_LOGD("S_%d: dma(x%x)cur_addr(%p)\n", _this->mSensorIdx, port.index, (MUINT8*)bufAddr);
    }

EXIT_TG_LOOP:

    MY_LOGD("S_%d exit tg_loop, stop", _this->mSensorIdx);
    ::sem_post(&_this->m_semTgIntThd);

    return NULL;
}

MVOID* camsv_loop(void* arg)
{
    NPipeUT*    _this = (NPipeUT*)arg;
    MUINT32     mnum = 0, dumpBufSize = 0, i;
    MUINTPTR    dumpBufVA = 0;

    ::pthread_detach(::pthread_self());

    MY_LOGD("S_%d: camsvThread: %d", _this->mSensorIdx, gettid());

    for (mnum = 0; MFALSE == _this->m_bStop; mnum++) {
        for (i = _this->mCamsvIdx ; i < _this->mCamsvIdx + _this->mCamsvCnt ; i++){
            PortID _dq_portID;
            QBufInfo _dq_rQBufInfo;
            QBufInfo _eq_rQBufInfo;

            _dq_portID.index = NSImageio::NSIspio::EPortIndex_IMGO;

            if (_this->mpCamsvSttPipe[i]->deque(_dq_portID, _dq_rQBufInfo) == MFALSE) {
                MY_LOGD("##############################");
                MY_LOGD("# error:deque fail (%d)      #", i);
                MY_LOGD("##############################");
#if 0
                MUINT32 _cnt = 0;
                while (_this->mSuspendState == 1) {
                    if (_this->m_bStop == MTRUE) {
                        break;
                    }
                    if ((_cnt & 0x7F) == 0) {
                        MY_LOGD("stt[%d] under suspending: %d",dmao,_cnt);
                    }
                    usleep(10000);
                    _cnt++;
                }
#endif
                break;
            }

            if (_dq_rQBufInfo.mvOut.size()) {
                _this->mCamsvBufVa[i] = _dq_rQBufInfo.mvOut.at(0).mVa;
                _this->mCamsvBufSize[i] = _dq_rQBufInfo.mvOut.at(0).mSize;

                MY_LOGD("camsv magic# 0x%08x pa:0x%" PRIXPTR " va:%" PRIXPTR " size:%d\n",
                    _dq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning,
                    _dq_rQBufInfo.mvOut.at(0).mPa,
                    _dq_rQBufInfo.mvOut.at(0).mVa,
                    _dq_rQBufInfo.mvOut.at(0).mSize);
            }
            else {
                MY_LOGD("camsv _dq_portID size is 0\n");
            }

            _eq_rQBufInfo.mvOut.resize(1);
            _eq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning = mnum; // CAMSV : pass magic number

            if (_this->mpCamsvSttPipe[i]->enque(_eq_rQBufInfo) == MFALSE) {
                MY_LOGD("##############################");
                MY_LOGD("# error:enque fail (%d)      #", i);
                MY_LOGD("##############################");
                break;
            }
        }
    }

    ::sem_post(&_this->m_semCamsvThd);

    return NULL;
}


NPipeUT::NPipeUT()
: mpNPipe(NULL)
, mpSttPipe(NULL)
, tuningMgr(NULL)
, mpImemDrv(NULL)
, mCamsvIdx(0)
, mCamsvCnt(0)
, mTgSize(MSize(0,0))
, mEisNotify(this)
, mRssNotify(this)
, mSgg2Notify(this)
, mLcsNotify(this)
, mDumpNotify(this)
, mSensorIdx(0)
, m_enablePort(0)
, mEnableSttPort(0)
, mStaticEnqCnt(1)
, mEnqCount(0)
, mDeqCount(0)
, mFailCount(0)
, mLoopCount(0)
, mSuspendTest(0)
, mSuspendFrm(0)
, mSuspendFlow(0)
, mSuspendStart(0)
, mResumeStart(0)
, mSuspendState(0)
, mSofUserKey(0)
, m_bStop(MFALSE)
, m_aeCyclePeriod(HWSYNC_AE_CYCLE_PERIOD)
, m_ufeo_en(MFALSE)
, m_ufgo_en(MFALSE)
, mFrameNum(8)
, mBurstQNum(1)
, mPortBufDepth(0)
, mMagicNum(0)
, m_bEfuse(MFALSE)
, mEfuseIdx(0)
, mEnableTgInt(MFALSE)
, mbCamsvEn(MFALSE)
, m_uniSwitch(MFALSE)
, mIQNotify(this)
, mbIQNotify(MTRUE)
, mRRZCBNotify(this)
, mTuningCBNotify(this)
, mAwbCBNotify(this)
, mArgc(0)
, mArgv(NULL)
, routineEnq(enque_loop)
, routineDeq(deque_loop)
, routineHwsync(hwsync_loop)
, routineTg(tg_loop)
, routineCamsv(camsv_loop)
, mbConfig(MFALSE)
, mbEnque(MFALSE)
, mSttEnqCnt(0)
, mSttDeqCnt(0)
, mbRanUF(MFALSE)
, mScaleSel(0)
, mScaleFactor(100)
{
    MUINT32 i;

    for (i = 0; i < __MAXDMAO_STT; i++) {
        routineSttProc[i] = sttProcRoutine[i];
    }

    for (i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++) {
        mpCamsvSttPipe[i] = NULL;
        mCamsvBufVa[i] = NULL;
        mCamsvBufSize[i] = NULL;
    }

    memset((void*)mpImgBuffer, 0, sizeof(mpImgBuffer));
    memset((void*)mImemBuf, 0, sizeof(mImemBuf));
    memset((void*)mpHeap, 0, sizeof(mpHeap));
    memset((void*)mpSttBuf, 0, sizeof(mpSttBuf));

    memset(&m_DeqThd, 0, sizeof(m_DeqThd));
    memset(&m_EnqThd, 0, sizeof(m_EnqThd));
    memset(&m_HwsyncThd, 0, sizeof(m_HwsyncThd));
    memset(&mSttThd, 0, sizeof(mSttThd));
    memset(&m_TgIntThd, 0, sizeof(m_TgIntThd));
    memset(&m_CamsvThd, 0, sizeof(m_CamsvThd));

    memset(&m_semSwitchThd, 0, sizeof(m_semSwitchThd));
    memset(&m_semDeqThd, 0, sizeof(m_semDeqThd));
    memset(&m_semHwsyncThd, 0, sizeof(m_semHwsyncThd));
    memset(&m_semSttProc, 0, sizeof(m_semSttProc));
    memset(&m_semTgIntThd, 0, sizeof(m_semTgIntThd));
    memset(&m_semEnqThd, 0, sizeof(m_semEnqThd));
    memset(&m_semCamsvThd, 0, sizeof(m_semCamsvThd));
    m_Dualpd.Raw = MFALSE;

}

NPipeUT* NPipeUT::create(void)
{
    return new NPipeUT();
}
void NPipeUT::destroy(void)
{
    delete this;
}

void NPipeUT::startThread(void)
{

    if (this->routineEnq) {
        ::sem_init(&m_semEnqThd, 0, 0);
        pthread_create(&m_EnqThd, NULL, this->routineEnq, this);
    }
    else {
        MY_LOGE("error: lack of enque routine");
    }

    if (this->routineDeq) {
        ::sem_init(&m_semDeqThd, 0, 0);
        pthread_create(&m_DeqThd, NULL, this->routineDeq, this);
    }
    else {
        MY_LOGE("error: lack of deque routine");
    }

    if (this->mbCamsvEn) {
        if (this->routineCamsv) {
            ::sem_init(&m_semCamsvThd, 0, 0);
            pthread_create(&m_CamsvThd, NULL, this->routineCamsv, this);
        }
        else {
            MY_LOGE("error: lack of camsv routine");
        }
    }

    if (mEnableSttPort) {
        MUINT32 sdma = 0;

        for (sdma = 0; sdma < __MAXDMAO_STT; sdma++) {
            if (mEnableSttPort & (1 << sdma)) {
                if (this->routineSttProc[sdma]) {
                    ::sem_init(&m_semSttProc[sdma], 0, 0);
                    pthread_create(&mSttThd[sdma], NULL, this->routineSttProc[sdma], this);
                }
                else {
                    MY_LOGE("error: lack of stt routine: dma:0x%x", sdma);
                }
            }
        }

        //tg interrupt
        if (mEnableTgInt){
            if (this->routineTg) {
                ::sem_init(&m_semTgIntThd, 0, 0);
                pthread_create(&m_TgIntThd, NULL, this->routineTg, this);
            }
            else {
                MY_LOGE("error: lack of tg routine: dma:0x%x", sdma);
            }
        }
    }

}

void NPipeUT::stopThread(void)
{
    MY_LOGD("wait thread stop");

    if (mEnableSttPort) {
        MUINT32 sdma = 0;
        for (sdma = 0; sdma < __MAXDMAO_STT; sdma++) {
            if (mEnableSttPort & (1 << sdma)) {
                MY_LOGD("S_%d: Statistic wait stop : %d", this->mSensorIdx, sdma);
                ::sem_wait(&m_semSttProc[sdma]);
            }
        }
    }
    if (this->mbCamsvEn) {
        ::sem_wait(&m_semCamsvThd);
    }

    MY_LOGD("Wait deq thread exit");
    ::sem_wait(&m_semDeqThd);
    MY_LOGD("Wait enq thread exit");
    ::sem_wait(&m_semEnqThd);

    if (mEnableTgInt){
        MY_LOGD("Wait tg thread exit");
        ::sem_wait(&m_semTgIntThd);
    }
}

void NPipeUT::startHwsyncThread(void)
{
#ifdef HWSYNC_USE_AE_THREAD
    if (this->routineHwsync) {
        ::sem_init(&m_semHwsyncThd, 0, 0);
        pthread_create(&m_HwsyncThd, NULL, this->routineHwsync, this);
    }
    else {
        MY_LOGE("error: lack of Hwsync routine");
    }
#endif
}

void NPipeUT::stopHwsyncThread(void)
{
#ifdef HWSYNC_USE_AE_THREAD
    MY_LOGD("wait hwsync stop");
    ::sem_wait(&m_semHwsyncThd);
#endif
}

MVOID NPipeUT::setTuning(MUINT32* pMagic, TUNING_TYPE_ENUM tuningType, MVOID* pIn, MVOID* pOut)
{
    if(0 == mEnableSttPort){
        return;
    }

    if(tuningType == TYPE_SET_TUNING){
        tuningMgr->dequeBuffer((MINT32*)pMagic);
    }

    for(MUINT32 i=0;i<this->mBurstQNum;i++){

        if(this->mEnableSttPort & __AFO_ENABLE){
            MUINT32 tmp = 0;
            MUINT32 h_size, win_h_size;
            MUINT32 v_size, win_v_size;
            MUINT32 start_x = 0, start_y = 0;
            MUINT32 afo_xsize = 0 , afo_ysize = 0;
            MUINT32 af_blk_sz;
            CAM_REG_AF_CON af_con;
            CAM_REG_AF_CON2 af_con2;
            BIN_INPUT_INFO *pDataIn;
            Tuning_CFG *pCfg;
            cam_reg_t *pReg;

#define AF_WIN_NUM_X  128
#define AF_WIN_NUM_Y  128
#define AF_WIN_SZ     16
#define AF_WIN_SZ_EXT 40

            if(tuningType == TYPE_SET_TUNING){
                this->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO,(MINTPTR)&h_size, (MINTPTR)&v_size, i);
            }
            else{
                pDataIn = (BIN_INPUT_INFO *)pIn;
                h_size = pDataIn->TarBinOut_W;
                v_size = pDataIn->TarBinOut_H;
            }

            MY_LOGI("[AFO] h_size:%d v_size:%d", h_size, v_size);

            //mutual exclusive function
            af_con.Bits.AF_H_GONLY = 1;
            af_con.Bits.AF_V_GONLY = 0;
            af_con.Bits.AF_V_AVG_LVL = 2; // could only be 2 or 3
            af_con2.Bits.AF_DS_EN = 0;

            //af_ext_stat_en always on
            af_blk_sz = AF_WIN_SZ_EXT;
            af_con.Bits.AF_EXT_STAT_EN = 1;

            //win_h_size
            win_h_size = (h_size - start_x) / AF_WIN_NUM_X;
            if (win_h_size > 128) {
                win_h_size = 128;
            }
            else {//min constraint
                if((af_con.Bits.AF_V_AVG_LVL == 3) && (af_con.Bits.AF_V_GONLY == 1)){
                    win_h_size = (win_h_size < 32)? (32):(win_h_size);
                }
                else if((af_con.Bits.AF_V_AVG_LVL == 3) && (af_con.Bits.AF_V_GONLY == 0)){
                    win_h_size = (win_h_size < 16)? (16):(win_h_size);
                }
                else if((af_con.Bits.AF_V_AVG_LVL == 2) && (af_con.Bits.AF_V_GONLY == 1)){
                    win_h_size = (win_h_size < 16)? (16):(win_h_size);
                }
                else{
                    win_h_size = (win_h_size < 8)? (8):(win_h_size);
                }
            }
            if (af_con.Bits.AF_V_GONLY == 1)
                win_h_size = win_h_size / 4 * 4; // 4 alignment
            else
                win_h_size = win_h_size / 2 * 2; // 2 alignment

            //win_v_size
            win_v_size = (v_size - start_y) / AF_WIN_NUM_Y;
            if (win_v_size > 128)
                win_v_size = 128;
            else
                win_v_size = (win_v_size < 1)? (1):(win_v_size);

            //afo_xsize, afo_ysize
            afo_xsize = AF_WIN_NUM_X * af_blk_sz;
            afo_ysize = AF_WIN_NUM_Y;

            if(tuningType == TYPE_SET_TUNING){
                //af
                tuningMgr->updateEngine(eTuningMgrFunc_AF, MTRUE, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_SIZE, h_size, i); //case for no frontal binning

                //sgg5
                tuningMgr->updateEngine(eTuningMgrFunc_SGG5, MTRUE, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG5_PGN, 0x200, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG5_GMRC_1, 0xffffffff, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG5_GMRC_2, 0xffffffff, i);

                //sgg1
                tuningMgr->updateEngine(eTuningMgrFunc_SGG1, MTRUE, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG1_PGN, 0x200, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG1_GMRC_1, 0xffffffff, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG1_GMRC_2, 0xffffffff, i);

                //offset
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_VLD, (start_y<<16|start_x), i);

                //window num
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_BLK_1, ((AF_WIN_NUM_Y<<16)|AF_WIN_NUM_X), i);

                //window size
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_BLK_0, ((win_v_size<<16)|win_h_size), i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_BLK_2, ((win_v_size<<16)|win_h_size), i);

                //afo
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AFO_XSIZE, afo_xsize, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AFO_YSIZE, afo_ysize, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AFO_STRIDE, afo_xsize, i);

                //af_con, af_con2
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_CON, af_con.Raw, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_CON2, af_con2.Raw, i);

                //af_th
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_TH_2, 0, i);
            }
            else{
                pCfg = (Tuning_CFG *)pOut;
                pReg = (cam_reg_t *)pCfg->pIspReg;

                //af
                pCfg->Bypass.Bits.AF = 0;
                pReg->CAM_AF_SIZE.Raw = h_size;

                //sgg5
                pReg->CAM_SGG5_PGN.Raw = 0x200;
                pReg->CAM_SGG5_GMRC_1.Raw = 0xffffffff;
                pReg->CAM_SGG5_GMRC_2.Raw = 0xffffffff;

                //sgg1
                pReg->CAM_SGG1_PGN.Raw = 0x200;
                pReg->CAM_SGG1_GMRC_1.Raw = 0xffffffff;
                pReg->CAM_SGG1_GMRC_2.Raw = 0xffffffff;

                //offset
                pReg->CAM_AF_VLD.Raw = (start_y<<16|start_x);

                //window num
                pReg->CAM_AF_BLK_1.Raw = ((AF_WIN_NUM_Y<<16)|AF_WIN_NUM_X);

                //window size
                pReg->CAM_AF_BLK_0.Raw = ((win_v_size<<16)|win_h_size);
                pReg->CAM_AF_BLK_2.Raw = ((win_v_size<<16)|win_h_size);

                //afo
                pReg->CAM_AFO_XSIZE.Raw = afo_xsize;
                pReg->CAM_AFO_YSIZE.Raw = afo_ysize;
                pReg->CAM_AFO_STRIDE.Raw = afo_xsize;

                //af_con, af_con2
                pReg->CAM_AF_CON.Raw = af_con.Raw;
                pReg->CAM_AF_CON2.Raw = af_con2.Raw;

                //af_th
                pReg->CAM_AF_TH_2.Raw = 0;
            }
        }
    ////////////////////////////////////////////////////////////////////////////////////////
    //AAO
        if(this->mEnableSttPort & __AAO_ENABLE){
            tuningMgr->updateEngine(eTuningMgrFunc_AA, MTRUE, i);

            MUINT32 ae_hst_sel,aa_in_h,aa_in_v,aa_start_x,aa_start_y,_cnt=0;
            MUINT32 aa_isize_h,aa_isize_v;
            MUINT32 aa_win_size_h,aa_win_size_v;
            MUINT32 aa_xsize = 0;
            MUINT32 awb_pix_cnt;
            BIN_INPUT_INFO *pDataIn;
            Tuning_CFG *pCfg;
            cam_reg_t *pReg;

#undef AA_WIN_H
#undef AA_WIN_V
#define AA_WIN_H    120
#define AA_WIN_V    90

#define HDR_ST_EN     1
#define AE_OE_EN      1
#define TSF_ST_EN     1
#define LIN_ST_EN     1
#define HST_DMAO_EN   1

            if(tuningType == TYPE_SET_TUNING){
                this->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO,(MINTPTR)&aa_in_h, (MINTPTR)&aa_in_v, i);
            }
            else{
                pDataIn = (BIN_INPUT_INFO *)pIn;
                aa_in_h = pDataIn->TarQBNOut_W;
                aa_in_v = pDataIn->TarBinOut_H;
            }

            MY_LOGI("[AAO] tuningtype:%d aa_in_h:%d aa_in_v:%d", tuningType, aa_in_h, aa_in_v);

            //aa win/pit size, win size <= pit size
            aa_win_size_h = (aa_in_h / AA_WIN_H) /2 * 2;
            aa_win_size_v = (aa_in_v / AA_WIN_V) /2 * 2;

            //aa start coordinate
            aa_start_x = (aa_in_h - aa_win_size_h * AA_WIN_H);
            aa_start_y = (aa_in_v - aa_win_size_v * AA_WIN_V);

            if(aa_win_size_h < 4)
                MY_LOGE("h-min is 4 , error\n");
            if(aa_win_size_v < 2)
                MY_LOGE("v-min is 2 , error\n");

            aa_xsize = (AA_WIN_H*AA_WIN_V*4); // awb , 4 bytes each win
            aa_xsize += (AA_WIN_H*AA_WIN_V); // ae , 1 byte each win

            _cnt=0;
            if(LIN_ST_EN){
                _cnt |= 0x10;
                aa_xsize += (AA_WIN_H*AA_WIN_V*12); // 12 bytes each win
            }
            if(HST_DMAO_EN){
                _cnt |= 0x8;
                aa_xsize += (3 * 256) * 6; // 3 byte per bin for 6 hist
            }
            if(HDR_ST_EN){
                _cnt |= 0x4;
                aa_xsize += (AA_WIN_H*AA_WIN_V); // 1 byte each win
            }
            if(AE_OE_EN){
                _cnt |= 0x2;
                aa_xsize += (AA_WIN_H*AA_WIN_V); // 1 byte each win
            }
            if(TSF_ST_EN){
                _cnt |= 0x1;
                aa_xsize += (AA_WIN_H*AA_WIN_V*4); // 4 bytes each win
            }

            //pix cnt
            awb_pix_cnt = (1<<24) / (aa_win_size_h * aa_win_size_v) *2;

            if(tuningType == TYPE_SET_TUNING){
                //win number
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_NUM, (AA_WIN_H | (AA_WIN_V<<16)), i);

                //win size
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_SIZE, (aa_win_size_h|(aa_win_size_v<<16)), i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_PIT,  (aa_win_size_h|(aa_win_size_v<<16)), i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_ORG, (aa_start_x|(aa_start_y<<16)), i);

                if(HST_DMAO_EN){
                    TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_PIX_HST_CTL, 0x3F, i);
                    TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_PIX_HST_SET, 0x1111, i);
                    TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_PIX_HST_SET_1, 0x11, i);
                }

                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_STAT_EN, _cnt, i);

                //awb
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_GAIN1_0, 0x02000200, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_GAIN1_1, 0x00000200, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_LMT1_0, 0x0fff0fff, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_LMT1_1, 0x00000fff, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_LOW_THR, 0x00000000, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_HI_THR, 0x00ffffff, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_RC_CNV_4, 0x02000200, i);

                //pix cnt
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_PIXEL_CNT0, awb_pix_cnt*2, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_PIXEL_CNT1, awb_pix_cnt, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_PIXEL_CNT2, awb_pix_cnt*2, i);

                //ae
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_GAIN2_0, 0x00090200, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_GAIN2_1, 0x00000000, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_LMT2_0, 0x00000000, i);

                //aao
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AAO_XSIZE, aa_xsize, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AAO_YSIZE, 1, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AAO_STRIDE, aa_xsize, i);
            }
            else{
                pCfg = (Tuning_CFG *)pOut;
                pReg = (cam_reg_t *)pCfg->pIspReg;

                //aa
                pCfg->Bypass.Bits.AA = 0;

                //win number
                pReg->CAM_AWB_WIN_NUM.Raw = (AA_WIN_H | (AA_WIN_V<<16));

                //win size
                pReg->CAM_AWB_WIN_SIZE.Raw = (aa_win_size_h|(aa_win_size_v<<16));
                pReg->CAM_AWB_WIN_PIT.Raw = (aa_win_size_h|(aa_win_size_v<<16));
                pReg->CAM_AWB_WIN_ORG.Raw = (aa_start_x|(aa_start_y<<16));

                if(HST_DMAO_EN){
                    pReg->CAM_AE_PIX_HST_CTL.Raw = 0x3F;
                    pReg->CAM_AE_PIX_HST_SET.Raw = 0x1111;
                    pReg->CAM_AE_PIX_HST_SET_1.Raw = 0x11;
                }

                //cnt
                pReg->CAM_AE_STAT_EN.Raw = _cnt;

                //awb
                pReg->CAM_AWB_GAIN1_0.Raw = 0x02000200;
                pReg->CAM_AWB_GAIN1_1.Raw = 0x00000200;
                pReg->CAM_AWB_LMT1_0.Raw = 0x0fff0fff;
                pReg->CAM_AWB_LMT1_1.Raw = 0x00000fff;
                pReg->CAM_AWB_LOW_THR.Raw = 0x00000000;
                pReg->CAM_AWB_HI_THR.Raw = 0x00ffffff;
                pReg->CAM_AWB_RC_CNV_4.Raw = 0x02000200;

                //pix cnt
                pReg->CAM_AWB_PIXEL_CNT0.Raw = awb_pix_cnt * 2;
                pReg->CAM_AWB_PIXEL_CNT1.Raw = awb_pix_cnt;
                pReg->CAM_AWB_PIXEL_CNT2.Raw = awb_pix_cnt * 2;

                //ae
                pReg->CAM_AE_GAIN2_0.Raw = 0x00090200;
                pReg->CAM_AE_GAIN2_1.Raw = 0x00000000;
                pReg->CAM_AE_LMT2_0.Raw = 0x00000000;

                //aao
                pReg->CAM_AAO_XSIZE.Raw = aa_xsize;
                pReg->CAM_AAO_YSIZE.Raw = 0x1;
                pReg->CAM_AAO_STRIDE.Raw = aa_xsize;
            }
        }
////////////////////////////////////////////////////////////////////////////////////////
    //PSO
        if(this->mEnableSttPort & __PSO_ENABLE){
            tuningMgr->updateEngine(eTuningMgrFunc_PS, MTRUE, i);

            MUINT32 aa_in_h,aa_in_v,aa_start_x,aa_start_y;
            MUINT32 aa_isize_h,aa_isize_v;
            MUINT32 aa_win_size_h,aa_win_size_v;
            MUINT32 aa_xsize = 0;
            MUINT32 awb_pix_cnt;
            MUINT32 awb_pix_cnt_2;
            MUINT32 v_align, div;
            CAM_REG_PS_HST_ROI_X roi_x;
            CAM_REG_PS_HST_ROI_Y roi_y;
            CAM_REG_PS_HST_CFG hst_cfg;
            CAM_REG_PS_DATA_TYPE ps_type;
            CAM_REG_RMG_HDR_CFG hdr_cfg;
            CAM_REG_PS_AE_YCOEF0 y_coef0;
            CAM_REG_PS_AE_YCOEF1 y_coef1;
            BIN_INPUT_INFO *pDataIn;
            Tuning_CFG *pCfg;
            cam_reg_t *pReg;

#define AA_WIN_H        120
#define AA_WIN_V        90
#define DATA_PATTERN    eCAM_NORMAL
#define RMBN_SEL        0
#define HST_EN          1
#define PS_EXP_MODE     0

            if(tuningType == TYPE_SET_TUNING){
                this->mpNPipe->sendCommand(ENPipeCmd_GET_RMB_INFO, (MINTPTR)&aa_in_h, (MINTPTR)&aa_in_v, i);
            }
            else{
                pDataIn = (BIN_INPUT_INFO *)pIn;
                aa_in_h = pDataIn->TarRMBOut_W;
                aa_in_v = pDataIn->TarBinOut_H;
            }

            MY_LOGI("[PSO] tuningType:%d aa_in_h:%d aa_in_v:%d", tuningType, aa_in_h, aa_in_v);

            //
            switch(DATA_PATTERN){
                case eCAM_NORMAL:
                case eCAM_DUAL_PIX:
                case eCAM_4CELL:
                    ps_type.Bits.SENSOR_TYPE = 0;
                    v_align = 2;
                    break;
                case eCAM_IVHDR:
                case eCAM_4CELL_IVHDR:
                    switch(RMBN_SEL){
                        case 1:
                        case 2:
                            //RMG
                            hdr_cfg.Raw = 0xFFA000E2 | 0x01/*IHDR_EN*/;

                            //
                            ps_type.Bits.SENSOR_TYPE = 1;
                            ps_type.Bits.G_LE_FIRST = hdr_cfg.Bits.RMG_IHDR_LE_FIRST;
                            break;
                        case 0:
                        case 3:
                            ps_type.Bits.SENSOR_TYPE = 0;
                            break;
                        default:
                            MY_LOGE("illegal rmbn_sel value");
                            ps_type.Bits.SENSOR_TYPE = 0;
                            break;
                    }
                    v_align = 4;
                    break;
                case eCAM_ZVHDR:
                case eCAM_4CELL_ZVHDR:
                    switch(RMBN_SEL){
                        case 1:
                        case 2:
                            //RMG
                            hdr_cfg.Raw = 0xFFA000E2 | 0x10/*ZHDR_EN*/;

                            //
                            ps_type.Bits.SENSOR_TYPE = 2;
                            ps_type.Bits.G_LE_FIRST = hdr_cfg.Bits.RMG_ZHDR_GLE;
                            ps_type.Bits.R_LE_FIRST = hdr_cfg.Bits.RMG_ZHDR_RLE;
                            ps_type.Bits.B_LE_FIRST = hdr_cfg.Bits.RMG_ZHDR_BLE;
                            break;
                        case 0:
                        case 3:
                            ps_type.Bits.SENSOR_TYPE = 0;
                            break;
                        default:
                            MY_LOGE("illegal rmbn_sel value");
                            ps_type.Bits.SENSOR_TYPE = 0;
                            break;
                    }
                    v_align = 4;
                    break;
                default:
                    MY_LOGE("illegal data pattern value");
                    v_align = 2;
                    break;
            }

            //aa win/pit size, win size <= pit size
            aa_win_size_h = (aa_in_h / AA_WIN_H) / 4 * 4; // 4-alignment
            aa_win_size_v = (aa_in_v / AA_WIN_V) / v_align * v_align; // 2 or 4-alignment

            //aa start coordinate
            aa_start_x = (aa_in_h - aa_win_size_h * AA_WIN_H) / 4 * 4; // 4-aLignment
            aa_start_y = (aa_in_v - aa_win_size_v * AA_WIN_V) / v_align * v_align; // 2 or 4-alignment

            if(aa_win_size_h < 4)
                MY_LOGE("h-min is 4 , error\n");
            if(aa_win_size_v < v_align)
                MY_LOGE("v-min is %d , error\n", v_align);

            switch(DATA_PATTERN){
                case eCAM_IVHDR:
                case eCAM_4CELL_IVHDR:
                    div = 8;
                    break;
                case eCAM_ZVHDR:
                case eCAM_4CELL_ZVHDR:
                    div = 8;
                    break;
                default:
                    div = 4;
                    break;
            }

            awb_pix_cnt = awb_pix_cnt_2 = (1<<24) / (aa_win_size_h * aa_win_size_v / div);

            switch(DATA_PATTERN){
                case eCAM_ZVHDR:
                case eCAM_4CELL_ZVHDR:
                    awb_pix_cnt_2 = awb_pix_cnt_2 * 2;
                    break;
                default:
                    break;
            }

            //ae
            ps_type.Bits.EXP_MODE = PS_EXP_MODE;
            hst_cfg.Bits.PS_HST_SEP_G = 0;
            hst_cfg.Bits.PS_CCU_HST_END_Y = AA_WIN_V;
            if(HST_EN){
                hst_cfg.Bits.PS_HST_EN = 1;
            }

            //roi
            roi_x.Bits.PS_X_HI = aa_in_h / 4;
            roi_x.Bits.PS_X_LOW = 0;
            roi_y.Bits.PS_Y_HI = aa_in_v / v_align;
            roi_y.Bits.PS_Y_LOW = 0;

            //coef
            y_coef0.Bits.AE_YCOEF_R = 4899;
            y_coef0.Bits.AE_YCOEF_GR = 4809;
            y_coef1.Bits.AE_YCOEF_GB = 4809;
            y_coef1.Bits.AE_YCOEF_B = 1867;

            //pso
            aa_xsize = (AA_WIN_H * AA_WIN_V * 4 * 2); // awb , 2 bytes each channel and 4 channels each win
            aa_xsize += (AA_WIN_H * AA_WIN_V * 2); // ae , 2 bytes each win

            switch(DATA_PATTERN){
                case eCAM_IVHDR:
                case eCAM_4CELL_IVHDR:
                case eCAM_ZVHDR:
                case eCAM_4CELL_ZVHDR:
                    switch(ps_type.Bits.EXP_MODE){
                        case 0:
                        case 1:
                            if(hst_cfg.Bits.PS_HST_EN){
                                aa_xsize += 256 * 3; // 3 bytes each bin, total 256 bin
                            }
                            break;
                        case 2:
                            if(hst_cfg.Bits.PS_HST_EN){
                                aa_xsize += 256 * 3; // 3 bytes each bin, total 256 bin
                            }

                            aa_xsize *= 2; // le + se output
                            break;
                    }
                    break;
                default:
                    if(hst_cfg.Bits.PS_HST_EN){
                        aa_xsize += 1024 * 3; // 3 bytes each bin, total 1024 bin
                    }
                    break;
            }

            if(tuningType == TYPE_SET_TUNING){
                switch(DATA_PATTERN){
                    case eCAM_NORMAL:
                    case eCAM_DUAL_PIX:
                    case eCAM_4CELL:
                        break;
                    case eCAM_IVHDR:
                    case eCAM_4CELL_IVHDR:
                        switch(RMBN_SEL){
                            case 1:
                            case 2:
                                tuningMgr->updateEngine(eTuningMgrFunc_RMG, MTRUE, i);
                                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG, hdr_cfg.Raw, i);
                                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_GAIN, 0x00400040, i);
                                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG2, 0x00000400, i);
                                break;
                            case 0:
                            case 3:
                                break;
                            default:
                                break;
                        }
                        break;
                    case eCAM_ZVHDR:
                    case eCAM_4CELL_ZVHDR:
                        switch(RMBN_SEL){
                            case 1:
                            case 2:
                                //RMG
                                tuningMgr->updateEngine(eTuningMgrFunc_RMG, MTRUE, i);
                                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG, hdr_cfg.Raw, i);
                                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_GAIN, 0x00400040, i);
                                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG2, 0x00000400, i);
                                break;
                            case 0:
                            case 3:
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }

                //awb
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_WIN_NUM, (AA_WIN_H | (AA_WIN_V<<16)), i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_WIN_SIZE, (aa_win_size_h| (aa_win_size_v<<16)), i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_WIN_PIT, (aa_win_size_h| (aa_win_size_v<<16)), i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_WIN_ORG, (aa_start_x| (aa_start_y<<16)), i);

                //pix cnt
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_PIXEL_CNT0, awb_pix_cnt, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_PIXEL_CNT3, awb_pix_cnt, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_PIXEL_CNT1, awb_pix_cnt_2, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AWB_PIXEL_CNT2, awb_pix_cnt_2, i);

                //
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_DATA_TYPE, ps_type.Raw, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_HST_CFG, hst_cfg.Raw, i);

                //roi
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_HST_ROI_X, roi_x.Raw, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_HST_ROI_Y, roi_y.Raw, i);

                //coef
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AE_YCOEF0, y_coef0.Raw, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PS_AE_YCOEF1, y_coef1.Raw, i);

                //pso
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PSO_XSIZE, aa_xsize, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PSO_YSIZE, 1, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PSO_STRIDE, aa_xsize, i);
            }
            else{
                pCfg = (Tuning_CFG *)pOut;
                pReg = (cam_reg_t *)pCfg->pIspReg;

                //ps
                pCfg->Bypass.Bits.PS = 0;

                switch(DATA_PATTERN){
                    case eCAM_NORMAL:
                    case eCAM_DUAL_PIX:
                    case eCAM_4CELL:
                        break;
                    case eCAM_IVHDR:
                    case eCAM_4CELL_IVHDR:
                        switch(RMBN_SEL){
                            case 1:
                            case 2:
                                pReg->CAM_RMG_HDR_CFG.Raw = hdr_cfg.Raw;
                                pReg->CAM_RMG_HDR_GAIN.Raw = 0x00400040;
                                pReg->CAM_RMG_HDR_CFG2.Raw = 0x00400040;
                                break;
                            case 0:
                            case 3:
                                break;
                            default:
                                break;
                        }
                        break;
                    case eCAM_ZVHDR:
                    case eCAM_4CELL_ZVHDR:
                        switch(RMBN_SEL){
                            case 1:
                            case 2:
                                //RMG
                                pReg->CAM_RMG_HDR_CFG.Raw = hdr_cfg.Raw;
                                pReg->CAM_RMG_HDR_GAIN.Raw = 0x00400040;
                                pReg->CAM_RMG_HDR_CFG2.Raw = 0x00400040;
                                break;
                            case 0:
                            case 3:
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }

                //awb
                pReg->CAM_PS_AWB_WIN_NUM.Raw = (AA_WIN_H | (AA_WIN_V<<16));
                pReg->CAM_PS_AWB_WIN_SIZE.Raw = (aa_win_size_h| (aa_win_size_v<<16));
                pReg->CAM_PS_AWB_WIN_PIT.Raw = (aa_win_size_h| (aa_win_size_v<<16));
                pReg->CAM_PS_AWB_WIN_ORG.Raw = (aa_start_x| (aa_start_y<<16));

                //pix cnt
                pReg->CAM_PS_AWB_PIXEL_CNT0.Raw = awb_pix_cnt;
                pReg->CAM_PS_AWB_PIXEL_CNT3.Raw = awb_pix_cnt;
                pReg->CAM_PS_AWB_PIXEL_CNT1.Raw = awb_pix_cnt_2;
                pReg->CAM_PS_AWB_PIXEL_CNT2.Raw = awb_pix_cnt_2;

                //
                pReg->CAM_PS_DATA_TYPE.Raw = ps_type.Raw;
                pReg->CAM_PS_HST_CFG.Raw = hst_cfg.Raw;

                //roi
                pReg->CAM_PS_HST_ROI_X.Raw = roi_x.Raw;
                pReg->CAM_PS_HST_ROI_Y.Raw = roi_y.Raw;

                //coef
                pReg->CAM_PS_AE_YCOEF0.Raw = y_coef0.Raw;
                pReg->CAM_PS_AE_YCOEF1.Raw = y_coef1.Raw;

                //pso
                pReg->CAM_PSO_XSIZE.Raw = aa_xsize;
                pReg->CAM_PSO_YSIZE.Raw = 0x1;
                pReg->CAM_PSO_STRIDE.Raw = aa_xsize;
            }
        }
    ////////////////////////////////////////////////////////////////////////////////////////
    //PDO
        if(this->mEnableSttPort & __PDO_ENABLE){
#define SPECIAL_TOKEN 0xc000
            MUINT32 xsize = 0, ysize = 0;

            //bypass tuning callback
            if(tuningType == TYPE_TUNING_CB){
                goto BYPASS_PDO;
            }

            //native pd
            if (!this->m_Dualpd.Bits.bDual) {
                MY_LOGD("native PD");

                tuningMgr->updateEngine(eTuningMgrFunc_PDE, MTRUE, i);
                tuningMgr->updateEngineFD(eTuningMgrFunc_PDE, i, m_bpciBuf.memID, m_bpciBuf.virtAddr);
                //pdi
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDI_BASE_ADDR, m_bpciBuf.phyAddr, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDI_XSIZE, m_bpciBuf.size, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDI_YSIZE, 1, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDI_STRIDE, m_bpciBuf.size, i);
                MY_LOGD("PDI_virtAddr:0x%x PDI_phyAddr:0x%x PDI_size:%d", m_bpciBuf.virtAddr, m_bpciBuf.phyAddr, m_bpciBuf.size);

                //pdo
                //TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_BASE_ADDR,m_bpciBuf.virtAddr,0); // doesn't need to cfg pdo addr here.
                //use full TG size here is just easiest example,
                //theoratically, PDO size msut be smaller than TG size,
                //e.g.: PDO width = 1/8 TG_W,  PDO height = 1/4 TG_H
                xsize = mTgSize.w;
                xsize *= 2;//PDO is 2byte per pix
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_XSIZE,xsize,i);

                unsigned short int* ptr = (unsigned short int*)m_bpciBuf.virtAddr;
                MUINT32 n_2bytejump = 0;
                for(MUINT32 i = 0 ; i < m_bpciBuf.size ; )
                {
                    if(((*ptr)&SPECIAL_TOKEN)==SPECIAL_TOKEN){
                         ysize++;
                         //jump ofset & cnum
                         n_2bytejump = 3;
                         i += (n_2bytejump*2);
                         ptr += n_2bytejump;
                         //jump number of pnum
                         n_2bytejump = (*ptr) + 1 + 1;//+ 1 for next special token , +1 for pnum indexing is start from 0
                         i += (n_2bytejump*2);
                         ptr += n_2bytejump;
                    }
                    else{
                        i += 2;
                        ptr = ptr + 1;
                    }
                }

                ysize = (ysize > 0) ? (ysize): (ysize+1);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_YSIZE,ysize,i);
                //pdo stride must be 16-alignment
                if((xsize %16) != 0)
                    xsize = (xsize + 15) / 16 * 16;
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_STRIDE,xsize,i);
            } else {
                MY_LOGD("dual PD");
                //dual pd
                tuningMgr->updateEngine(eTuningMgrFunc_PBN, MTRUE, i);
                //
                if(this->m_Dualpd.Bits.Density == MTRUE){
                    MY_LOGD("High Density dual PD");
                    //
                    MY_LOGE("not supported yet!");
                }
                else{
                    MY_LOGD("Low Density dual PD");
                    //
                    TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_PBN_TYPE,PBN_TYPE,0,i);
                    TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_PBN_LST,PBN_LST,1,i);
                    TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_PBN_VSIZE,PBN_VSIZE,mTgSize.h,i);

                    //using TG as input size when config, so using TG as output size
                    xsize = (this->mTgSize.w>>4)<<1;  // output 2 pixs per 16 pixs input
                    xsize = xsize * 2;                // 1 pair of L/R per bayer pix
                    xsize = xsize * 2;                // 2 byte per pix
                    ysize = (this->mTgSize.h>>3);     // output 1 line per 8 line
                }
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_XSIZE,xsize,i);//PDO is 2byte per pix
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_YSIZE,ysize,i);
                MY_LOGD("PDO SIZE: (%d_%d)", xsize, ysize);
            }
#undef SPECIAL_TOKEN
        }

BYPASS_PDO:
        if (this->mEnableSttPort & __FLKO_ENABLE) {
#define FLK_WIN_H   2
#define FLK_WIN_V   2

            //input size w/h: ref SGG_SEL
            //SGG_SEL: 0: from BMX, 1 from TG
            MUINT32 h_size, v_size;
            MUINT32 xsize = 0;
            BIN_INPUT_INFO *pDataIn;
            Tuning_CFG *pCfg;
            cam_reg_t *pReg;

            if(tuningType == TYPE_SET_TUNING){
                this->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO,(MINTPTR)&h_size,(MINTPTR)&v_size,i);
            }
            else{
                pDataIn = (BIN_INPUT_INFO *)pIn;
                h_size = pDataIn->TarQBNOut_W;
                v_size = pDataIn->TarBinOut_H;
            }

            MY_LOGI("[FLKO] tuningType:%d h_size:%d v_size:%d", tuningType, h_size, v_size);

            //win size
            h_size = (h_size / FLK_WIN_H) / 2 * 2;
            v_size = (v_size / FLK_WIN_V) / 2 * 2;

            //xsize
            xsize = (FLK_WIN_H * FLK_WIN_V * v_size * 2);

            if(tuningType == TYPE_SET_TUNING){
                //flk
                tuningMgr->updateEngine(eTuningMgrFunc_FLK, MTRUE, i);

                //offset
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_FLK_A_OFST, 0, i);

                //win num
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_FLK_A_NUM, ((FLK_WIN_V<<4)|FLK_WIN_H), i);

                //win size
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_FLK_A_SIZE, ((v_size<<16)|h_size), i);

                //flko
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_FLKO_XSIZE, xsize, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_FLKO_YSIZE, 1, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_FLKO_STRIDE, xsize, i);

                //sgg3
                tuningMgr->updateEngine(eTuningMgrFunc_SGG3, MTRUE, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG3_PGN, 0x200, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG3_GMRC_1, 0x10080402, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG3_GMRC_2, 0x804020, i);
            }
            else{
                pCfg = (Tuning_CFG *)pOut;
                pReg = (cam_reg_t *)pCfg->pIspReg;

                //flk
                pCfg->Bypass.Bits.FLK = 0;

                //offset
                pReg->CAM_FLK_A_OFST.Raw = 0;

                //win num
                pReg->CAM_FLK_A_NUM.Raw = ((FLK_WIN_V<<4)|FLK_WIN_H);

                //win size
                pReg->CAM_FLK_A_SIZE.Raw = ((v_size<<16)|h_size);

                //flko
                pReg->CAM_FLKO_XSIZE.Raw = xsize;
                pReg->CAM_FLKO_YSIZE.Raw = 1;
                pReg->CAM_FLKO_STRIDE.Raw = xsize;

                //sgg3
                pReg->CAM_SGG3_PGN.Raw = 0x200;
                pReg->CAM_SGG3_GMRC_1.Raw = 0x10080402;
                pReg->CAM_SGG3_GMRC_2.Raw = 0x804020;
            }
        }

    }

    if(tuningType == TYPE_SET_TUNING){
        tuningMgr->enqueBuffer();
    }
}

void NPipeUT::setZHdrTuning(MUINT32* pMagic)
{
    pMagic;
#if (TEST_ZHDR_HW == 1)

    tuningMgr->dequeBuffer((MINT32*)pMagic);

    for(MUINT32 i=0;i<this->mBurstQNum;i++){

        MY_LOGD("zvHdr(%d,%d) M(%d)", mTgSize.w, mTgSize.h, pMagic[i]);


        tuningMgr->updateEngine(eTuningMgrFunc_DBS, MTRUE, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SIGMA,      0x00150005, i); /* 0x1A004B40 */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_0,    0x00010204, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_1,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_2,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_3,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_CTL,        0x0840F000, i);
                                                                /* DBS_EDGE: 0xF */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_CTL_2,      0x00000FFA, i); /*TBD*/
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SIGMA_2,    0x0f0f0f0f, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_YGN,        0x20202020, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_Y12,     0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_Y34,     0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_G12,     0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_G34,     0x00000000, i);

        tuningMgr->updateEngine(eTuningMgrFunc_RMG, MTRUE, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG,    (0xFFA000E2|0x10/*ZHDR_EN*/), i); /* 0x1A0045A0 */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_GAIN,   0x00400040, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG2,   0x00000400, i);

        tuningMgr->updateEngine(eTuningMgrFunc_RMM, MTRUE, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_OSC,        0xF01143EC, i); /* 0x1A0045C0 */
                                                                /* RMM_EDGE: 0xF */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_MC,         0x00040245, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_REVG_1,     0x04000400, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_REVG_2,     0x04000400, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_LEOS,       0x00000200, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_MC2,        0x00050080, i); /*TBD*/
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_DIFF_LB,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_MA,         0x08088408, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_TUNE,       0x30800013, i);
    }
    tuningMgr->enqueBuffer();
#endif
}


void TuningNotifyImp_EIS::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
#define EIS_WIN_H   4
#define EIS_WIN_V   8
#define EIS_OFST_X  17
#define EIS_OFST_Y  17
#define MAX_STEP_H  16
#define MAX_STEP_V  8
#define EIS_XSIZE   0xFF
#define EIS_YSIZE   0x0
    LMV_INPUT_INFO  eis_in = *(LMV_INPUT_INFO*)pIn;
    LMV_CFG  lmv_cfg;
    MUINT32 win_h = 4, win_v = 8;
    MUINT32 ofst_x=0, ofst_y=0;
    MUINT32 FL_ofst_x = 0,FL_ofst_y=0;
    MUINT32 in_size_h, in_size_v;
    MUINT32 pix_mode = 0, bFG = MFALSE, eis_pipe_mode = 0;
    MUINT32 step_h = 16,step_v = 8, win_size_h, win_size_v, op_h=1, op_v=1;
    MUINT32 first_frm_flag = ((0 == mpNPipeUtObj->mDeqCount)? 1: 0);

    MY_LOGD("EIS info: 1stFrm(%d) RMXOut(%d,%d) HBinOut(%d,%d) TG(%d,%d) Fmt(x%x) PixMod(%d)",\
            first_frm_flag, eis_in.sRMXOut.w, eis_in.sRMXOut.h, eis_in.sHBINOut.w, eis_in.sHBINOut.h,\
            eis_in.sTGOut.w, eis_in.sTGOut.h, eis_in.bYUVFmt, eis_in.pixMode);

    //max window number
    win_h = (win_h > EIS_WIN_H)?(EIS_WIN_H):(win_h);
    win_v = (win_v > EIS_WIN_V)?(EIS_WIN_V):(win_v);

    //ofst,ofst have min constraint
    ofst_x = (ofst_x < EIS_OFST_X)?(EIS_OFST_X):(ofst_x);
    ofst_y = (ofst_y < EIS_OFST_Y)?(EIS_OFST_Y):(ofst_y);

    //floating ofset, current frame ofst from previous frame, normally set 0.

    pix_mode    = eis_in.pixMode; //TBD
    in_size_h   = eis_in.sHBINOut.w;
    in_size_v   = eis_in.sHBINOut.h;

    step_h = (step_h > MAX_STEP_H)?(MAX_STEP_H):(step_h);
    step_v = (step_v > MAX_STEP_V)?(MAX_STEP_V):(step_v);

    while( (in_size_h/op_h) <= ((step_h*16) + ofst_x)){
        step_h--;
    }

    win_size_h = ((in_size_h/op_h) - (step_h*16) - ofst_x)/win_h;
    while(win_size_h < ((step_h + 1) * 16 + 2)){
        if( op_h> 1)
            op_h = op_h>>1;
        else{
            if(step_h > 1)
                step_h--;
            else{
                if(win_h > 1)
                    win_h -=1;
                else{
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    MY_LOGE("EIS H WINDOW SIZE FAIL\n");
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    exit(1);
                }
            }
        }
        //win_size_h = ((((in_size_h - ofst_x)/op_h)/16) - step_h) / win_h ;
        win_size_h = ((in_size_h/op_h) - (step_h*16) - ofst_x)/win_h;
    }

    win_size_v = ((in_size_v/op_v) - (step_v*16) - ofst_y)/win_v;
    while(win_size_v < ((step_v + 1) * 16 + 2)){
        if(op_v > 1)
            op_v = op_v>>1;
        else{
            if(step_v > 1)
                step_v--;
            else{
                if(win_v > 1)
                    win_v -=1;
                else{
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    MY_LOGE("EIS V WINDOW SIZE FAIL\n");
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    exit(1);
                }
            }
        }
        //win_size_v = ((((in_size_v - ofst_y)/op_v)/16) - step_v) / win_v ;
        win_size_v = ((in_size_v/op_v) - (step_v*16) - ofst_y)/win_v;
    }

    lmv_cfg.bypassLMV = MFALSE;
    lmv_cfg.enLMV     = MTRUE;
    /* IN ISP4.0 ALWAYS USE GSE to replace SUBG_EN */
    /* 0x3550 TBD */ lmv_cfg.cfg_lmv_prep_me_ctrl1 = ((win_v<<28)|(win_h<<25)|(step_v<<21)|(0<<16/*knee1 RSTVAL*/)
                                |(0<<12/*knee2 RSTVAL*/)|(step_h<<8)|(0/*SUBG_EN*/<<5)|(op_v<<3)|(op_h));
    /* 0x3554 */ lmv_cfg.cfg_lmv_prep_me_ctrl2 = 0x0000506C/*RSTVAL*/;
    /* 0x3558 TBD */ lmv_cfg.cfg_lmv_lmv_th = 0/*RSTVAL*/;
    /* 0x355C */ lmv_cfg.cfg_lmv_fl_offset = (((FL_ofst_x&0xFFF)<<16) | (FL_ofst_y&0xFFF));
    /* 0x3560 */ lmv_cfg.cfg_lmv_mb_offset = (((ofst_x&0xFFF)<<16) | (ofst_y&0xFFF));
    /* 0x3564 */ lmv_cfg.cfg_lmv_mb_interval = (((win_size_h&0xFFF)<<16)| (win_size_v&0xFFF));
    /* 0x3568 */ lmv_cfg.cfg_lmv_gmv    = 0; /* RO, GMV */
    /* 0x356C TBD */ lmv_cfg.cfg_lmv_err_ctrl = 0x000F0000/*RSTVAL*/;
    /* 0x3570 TBD*/ lmv_cfg.cfg_lmv_image_ctrl = ((eis_pipe_mode/*RSTVAL*/<<31)|((in_size_h&0x3FFF)<<16)
                                |(in_size_v&0x3FFF));

    *(LMV_CFG*)pOut = lmv_cfg;
}

MSize TuningNotifyImp_EIS::queryEisOutSize()
{
    return MSize(256, 1);
}

void TuningNotifyImp_RSS::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
#define RSS_DRV_COEFFICIENT_TABLE  (15)      //recommeded by HW
#define RSS_DRV_RZ_UINT            (1048576) //2^20
#define RSSO_HSIZE                 (288)
#define RSSO_VSIZE                 (511)

    RSS_CFG rss_cfg;
    RSS_INPUT_INFO rss_in = *(RSS_INPUT_INFO *)pIn;
    MUINT32    Mm1_H, Mm1_V, Nm1_H, Nm1_V;
    MUINT32    pCoeffStep_H, pCoeffStep_V;
    MUINT32    xsize, ysize;

    //calculate rsso_xsize
    xsize = RSSO_HSIZE;
    ysize = (xsize * rss_in.rrz_out_h) / rss_in.rrz_out_w;

    Mm1_H = rss_in.rss_in_w - 1;
    Mm1_V = rss_in.rss_in_h - 1;
    Nm1_H = xsize - 1;
    Nm1_V = ysize - 1;

    //calculate coeffstep
    pCoeffStep_H = (MUINT32)((Nm1_H * RSS_DRV_RZ_UINT + Mm1_H - 1) / Mm1_H);
    pCoeffStep_V = (MUINT32)((Nm1_V * RSS_DRV_RZ_UINT + Mm1_V - 1) / Mm1_V);

    rss_cfg.bypassRSS= MFALSE;
    rss_cfg.enRSS= MTRUE;
    rss_cfg.cfg_rss_in_img = (rss_in.rss_in_h << 16) | (rss_in.rss_in_w);
    rss_cfg.cfg_rss_out_img = (ysize << 16) | (xsize);
    rss_cfg.cfg_rss_hori_step = pCoeffStep_H;
    rss_cfg.cfg_rss_vert_step = pCoeffStep_V;
    rss_cfg.cfg_rss_hori_int_ofst = 0;
    rss_cfg.cfg_rss_hori_sub_ofst = 0;
    rss_cfg.cfg_rss_vert_int_ofst = 0;
    rss_cfg.cfg_rss_vert_sub_ofst = 0;
    rss_cfg.cfg_rss_ctrl_hori_tbl_sel = RSS_DRV_COEFFICIENT_TABLE;
    rss_cfg.cfg_rss_ctrl_vert_tbl_sel = RSS_DRV_COEFFICIENT_TABLE;
    rss_cfg.cfg_rss_ctrl_hori_en = MTRUE;
    rss_cfg.cfg_rss_ctrl_vert_en = MTRUE;

    *(RSS_CFG*)pOut = rss_cfg;
}

MSize TuningNotifyImp_RSS::queryRssOutSize()
{
    return MSize(288, 511);
}

void TuningNotifyImp_SGG2::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    LMV_SGG_CFG sgg2_cfg;
    (void)pIn;

    sgg2_cfg.bSGG2_Bypass   = 1; //TBD
    sgg2_cfg.bSGG2_EN       = 1; //TBD
    sgg2_cfg.PGN  = 0x10/*RSTVAL*/;
    sgg2_cfg.GMRC_1 = 0x10080402/*RSTVAL*/;
    sgg2_cfg.GMRC_2 = 0x00804020/*RSTVAL*/;

    //TBD CAM_UNI_TOP_MOD_EN |= SGG2_A_EN ???

    *(LMV_SGG_CFG*)pOut = sgg2_cfg;
}

void TuningNotifyImp_LCS::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    LCS_INPUT_INFO  lcs_input = *(LCS_INPUT_INFO*)pIn;
    LCS_REG_CFG     lcs_cfg;
    MSize           lcs_in = queryLcsInSize(mpNPipeUtObj->mTgSize);
    MSize           lcs_out = queryLcsOutSize(mpNPipeUtObj->mTgSize);
    MUINT32         lrzr_x, lrzr_y;

#if 0//get HBIN info by sendCommand
    MUINT32 binW = 0;
    MUINT32 binH = 0;
    mpNPipeUtObj->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO, (MINTPTR)(&binW), (MINTPTR)(&binH), 0);
    MY_LOGD("LCS HBIN info: %d_%d", binW,binH);
    if( binW <  lcs_in.w)
        lcs_in.w = binW;
    if( binH <  lcs_in.h)
        lcs_in.h = binH;
#else//get HBIN info by parameter
    lcs_in.w = lcs_input.sHBINOut.w;
    lcs_in.h = lcs_input.sHBINOut.h;
#endif

    lrzr_x = ((lcs_out.w-1)*1048576)/((lcs_in.w>>1)-1);
    lrzr_y = ((lcs_out.h-1)*1048576)/((lcs_in.h>>1)-1);

    MY_LOGD("LCS info: in_%d_%d out_%d_%d lrzr_%d_%d", lcs_in.w, lcs_in.h, lcs_out.w, lcs_out.h,
            lrzr_x, lrzr_y);

    lcs_cfg.bLCS_EN = MTRUE;
    lcs_cfg.bLCS_Bypass = MFALSE;
    lcs_cfg.u4LCSO_Stride = 768;
    lcs_cfg._LCS_REG.LCS_CON = (((lcs_out.h&0x1FF)<<20) | ((lcs_out.w&0x1FF)<<8));
    lcs_cfg._LCS_REG.LCS_ST  = 0; // LCS_START_I=0, LCS_START_J=0
    lcs_cfg._LCS_REG.LCS_AWS = (((lcs_in.h&0x1FFF)<<16) | (lcs_in.w&0x1FFF));
    lcs_cfg._LCS_REG.LCS_FLR = 0x00010000;/*RSTVAL*/
    lcs_cfg._LCS_REG.LCS_LRZR_1 = (lrzr_x & 0xFFFFF);
    lcs_cfg._LCS_REG.LCS_LRZR_2 = (lrzr_y & 0xFFFFF);
    lcs_cfg._LCS_REG.LCS_SATU1 = 0x0FFF0FFF;//default
    lcs_cfg._LCS_REG.LCS_SATU2 = 0x0FFF0FFF;//default
    lcs_cfg._LCS_REG.LCS_GAIN_1 = 0x02000200;//default
    lcs_cfg._LCS_REG.LCS_GAIN_2 = 0x02000200;//default
    lcs_cfg._LCS_REG.LCS_OFST_1 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_OFST_2 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_1 = 0x00000200;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_2 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_3 = 0x00000200;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_4 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_5 = 0x00090200;//default

    *(LCS_REG_CFG*)pOut = lcs_cfg;
}

MSize TuningNotifyImp_LCS::queryLcsOutSize(MSize TgSize)
{
    (void)TgSize;
    return MSize(8, 8);
}

MSize TuningNotifyImp_LCS::queryLcsInSize(MSize TgSize)
{
    return MSize(TgSize);
}

void TuningNotifyImp_IQ::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    if(mpNPipeUtObj->mbIQNotify == MFALSE){
        mpNPipeUtObj->mbIQNotify = MTRUE;
        MY_LOGI("[IQ Notify] Once");
    }
    else{
        MY_LOGI("[IQ Notify] Duplicate");
    }
}

void TuningNotifyImp_RRZCB::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    MY_LOGD("[RrzCB Notify] enRrzPort:%d pIn:0%x pOut:0x%x", (mpNPipeUtObj->m_enablePort&__RRZO_ENABLE), pIn, pOut);

    BIN_INPUT_INFO *pDataIn = (BIN_INPUT_INFO *)pIn;
    MY_LOGI("TgW:%d TgH:%d BinM:%d TarBinEn:%d TarBinW:%d TarBinH:%d CurBinW:%d CurBinH:%d Mag:%d TarQBNW:%d TarRMBW:%d CurQBNW:%d CurRMBW:%d",\
            pDataIn->TgOut_W, pDataIn->TgOut_H, pDataIn->Bin_MD, pDataIn->TarBin_EN, pDataIn->TarBinOut_W, pDataIn->TarBinOut_H, pDataIn->CurBinOut_W,\
            pDataIn->CurBinOut_H, pDataIn->Magic, pDataIn->TarQBNOut_W, pDataIn->TarRMBOut_W, pDataIn->CurQBNOut_W, pDataIn->CurRMBOut_W);


#if 1
    if(mpNPipeUtObj->m_enablePort & __RRZO_ENABLE){
        BIN_INPUT_INFO *pDataIn = (BIN_INPUT_INFO *)pIn;
        RRZ_REG_CFG *pDataOut = (RRZ_REG_CFG *)pOut;
        IImageBuffer *pIBuf;
        MRect _cropsize;
        MSize _size, _rrz;

        //get image format
        if(mpNPipeUtObj->mpImgBuffer[__RRZO].size() <= 0){
            MY_LOGE("[RrzCB Notify] RRZO buffer size is zero");
            return;
        }
        else{
            pIBuf = mpNPipeUtObj->mpImgBuffer[__RRZO].at(0); // the img format are all ths same, just use the first one
        }

        //get bin size
        _size.w = pDataIn->TarBinOut_W;
        _size.h = pDataIn->TarBinOut_H;

        //get crop/scale size
        mpNPipeUtObj->m_crop.PatchPipeLineIn(_size);
        mpNPipeUtObj->m_crop.GetCropInfo(PORT_RRZO, (EImageFormat)pIBuf->getImgFormat(), _cropsize);
        mpNPipeUtObj->m_crop.GetScaleInfo(PORT_RRZO, (EImageFormat)pIBuf->getImgFormat(), _rrz, mpNPipeUtObj->mScaleSel, mpNPipeUtObj->mScaleFactor);

        pDataOut->bRRZ_Bypass = MFALSE;
        pDataOut->src_x = _cropsize.p.x;
        pDataOut->src_y = _cropsize.p.y;
        pDataOut->src_w = _cropsize.s.w;
        pDataOut->src_h = _cropsize.s.h;
        pDataOut->tar_w = _rrz.w;
        pDataOut->tar_h = _rrz.h;
    }
#endif
}

void TuningNotifyImp_TuningCB::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    MY_LOGD("[TuningCB Notify] sttPort:%d pIn:0%x pOut:0x%x", mpNPipeUtObj->mEnableSttPort, pIn, pOut);

    BIN_INPUT_INFO *pDataIn = (BIN_INPUT_INFO *)pIn;
    MY_LOGI("TgW:%d TgH:%d BinM:%d TarBinEn:%d TarBinW:%d TarBinH:%d CurBinW:%d CurBinH:%d Mag:%d TarQBNW:%d TarRMBW:%d CurQBNW:%d CurRMBW:%d",\
            pDataIn->TgOut_W, pDataIn->TgOut_H, pDataIn->Bin_MD, pDataIn->TarBin_EN, pDataIn->TarBinOut_W, pDataIn->TarBinOut_H, pDataIn->CurBinOut_W,\
            pDataIn->CurBinOut_H, pDataIn->Magic, pDataIn->TarQBNOut_W, pDataIn->TarRMBOut_W, pDataIn->CurQBNOut_W, pDataIn->CurRMBOut_W);

#if 1
    if(mpNPipeUtObj->mEnableSttPort){
        mpNPipeUtObj->setTuning(NULL, TYPE_TUNING_CB, pIn, pOut);
    }
#endif
}

void TuningNotifyImp_Dump::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    Mutex::Autolock lock(mCntLock);
    CQ_DUMP_INFO *Reg_in = (CQ_DUMP_INFO*)pIn;

    if(Reg_in == NULL){
        MY_LOGE("pIn is NULL\n");
        return;
    }
    if(Reg_in->size == 0 || Reg_in->pReg == NULL) {
        MY_LOGE("size=%d, pReg=%x\n", Reg_in->size, Reg_in->pReg);
        return;
    }
    MY_LOGD("[RegDump Notify] m#(%d), reg size(%d)\n", Reg_in->magic, Reg_in->size);

    if (!this->mReg){
        this->mReg = (MUINT32 *)malloc(sizeof(MUINT32)*Reg_in->size);
        if(!this->mReg){
            MY_LOGE("allocate dump mem, size=%d fail\n", Reg_in->size);
            return;
        }
    }
    this->mSize = Reg_in->size;
    memcpy(this->mReg, Reg_in->pReg, sizeof(MUINT32)*Reg_in->size);
    mCnt++;
}

void TuningNotifyImp_Dump::dumpRegister()
{
    Mutex::Autolock lock(mCntLock);

    MUINT32* ptr;
    char _tmpchr[16] = "\0";
    char _chr[256] = "\0";

    if(!this->mReg || !mCnt) {
        return;
    }

    MY_LOGD("+");
    ptr = (MUINT32 *)this->mReg;
    mCnt--;

    for(MUINT32 i=0x0;i<this->mSize;i+=0x20) {
        _chr[0] = '\0';
        snprintf(_chr, sizeof(_chr),"0x%4x: ",i);
        for(MUINT32 j=i;j<(i+0x20);j+=0x4){
            if(j >= this->mSize)
                break;
            _tmpchr[0] = '\0';
            snprintf(_tmpchr, sizeof(_tmpchr),"0x%8x-",ptr[(j>>2)]);
            strncat(_chr,_tmpchr, strlen(_tmpchr));
        }
        MY_LOGD("%s",_chr);
    }
    MY_LOGD("-");
}

void TuningNotifyImp_AWB::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    MY_LOGD("[AwbCB Notify] enAaoPort:%d pIn:0%x pOut:0x%x", (mpNPipeUtObj->m_enablePort&__AAO_ENABLE), pIn, pOut);

    BIN_INPUT_INFO *pDataIn = (BIN_INPUT_INFO *)pIn;
    Tuning_CFG *pCfg;
    cam_reg_t *pReg;

    MY_LOGI("TgW:%d TgH:%d BinM:%d TarBinEn:%d TarBinW:%d TarBinH:%d CurBinW:%d CurBinH:%d Mag:%d TarQBNW:%d TarRMBW:%d CurQBNW:%d CurRMBW:%d",\
            pDataIn->TgOut_W, pDataIn->TgOut_H, pDataIn->Bin_MD, pDataIn->TarBin_EN, pDataIn->TarBinOut_W, pDataIn->TarBinOut_H, pDataIn->CurBinOut_W,\
            pDataIn->CurBinOut_H, pDataIn->Magic, pDataIn->TarQBNOut_W, pDataIn->TarRMBOut_W, pDataIn->CurQBNOut_W, pDataIn->CurRMBOut_W);


#if 1
    if(mpNPipeUtObj->mEnableSttPort & __AAO_ENABLE){
        pCfg = (Tuning_CFG *)pOut;
        pReg = (cam_reg_t *)pCfg->pIspReg;

        pCfg->Bypass.Bits.RPG = 0;
        pCfg->Bypass.Bits.AA = 0;

        //RPG, test value, no meaning
        pReg->CAM_RPG_OFST_1.Raw = 0xFFFF;
        pReg->CAM_RPG_OFST_2.Raw = 0xFFFF;

        //AWB, test value, no meaning
        pReg->CAM_AWB_L0_X.Raw = 0xFFFF;
        pReg->CAM_AWB_L0_Y.Raw = 0xFFFF;
    }
#endif
}

