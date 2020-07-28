/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include "VTMALStub.h"
#include "VTCore.h"
#include "VTDef.h"

using namespace android;
using android::INVALID_OPERATION;
using android::ALREADY_EXISTS;

namespace VTService{

    int VTMALStub::MA_INVALID_MODE      = 0;
    int VTMALStub::MA_PASS_MODE         = 1;
    int VTMALStub::MA_NORMAL_MODE_3G    = 2;
    int VTMALStub::MA_NORMAL_MODE_4G    = 3;

    VTMALStub::VTMALStub(int mode, int id, int sim_id, int opid, const sp<VTCore>& core) {

        mMode = mode;
        mVTCore = core;

        getLogger()->logAction("VTMALStub", "id", id, "VTMALStub", VT_SRV_LOG_I);
        VT_LOGI("[SRV] [OPERATION][ID=%d] VTMALStub (sim_id = %d)", id, sim_id);

        mId = id;
        mSimId = sim_id;
        mOpId = opid;

        if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "4G mode", VT_SRV_LOG_W);
            mImsMa = new ImsMa(mSimId, mOpId);
        }

        mPeerSurface = NULL;
        mLocalSurface = NULL;
        mCurDegree = -1;
    }

    status_t VTMALStub::Init(ma_datapath_t type) {

        getLogger()->logAction("VTMALStub", "id", mId, "Init (Begin)", VT_SRV_LOG_W);

        if (MA_INVALID_MODE == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Init (mode = invalid) (Finish)", VT_SRV_LOG_W);
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Init (mode = 4G)", VT_SRV_LOG_W);

            mImsMa->Init(type);

            mMsgHdlr = new MsgHandler(mVTCore, mId, mImsMa, 1);
            mMsgHdlr->exchangeMsg();

        } else {
            getLogger()->logAction("VTMALStub", "id", mId, "Init (mode = pass or 3G)", VT_SRV_LOG_W);
        }

        getLogger()->logAction("VTMALStub", "id", mId, "Init (Finish)", VT_SRV_LOG_W);
        return VT_SRV_RET_OK;

    }

    status_t VTMALStub::Start(ma_datapath_t type) {

        getLogger()->logAction("VTMALStub", "id", mId, "Start (Begin)", VT_SRV_LOG_W);

        if (MA_INVALID_MODE == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Start (mode = invalid) (Finish)", VT_SRV_LOG_W);
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Start (mode = 4G)", VT_SRV_LOG_W);

            if (type == MA_SOURCE) {
                getLogger()->logAction("VTMALStub", "id", mId, "Start (SOURCE)", VT_SRV_LOG_W);
            } else if (type == MA_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Start (SINK)", VT_SRV_LOG_W);
            } else if (type == MA_SOURCE_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Start (SOURCE_SINK)", VT_SRV_LOG_W);
            }

            if (mImsMa->Start(type)) {
                getLogger()->logAction("VTMALStub", "id", mId, "Start, error (Finish)", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            }

            if (type & MA_SOURCE) {
                mVTCore->notifyCallback(
                    getId(),
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_START_REC,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                //Set new bufferProducer to VTSource
                sp<IGraphicBufferProducer> outBufferProducer = NULL;
                getBufferQueueProducer(&outBufferProducer);

                mVTCore->notifyCallback(
                    getId(),
                    VT_SRV_NOTIFY_LOCAL_BUFFER,
                    0,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    outBufferProducer);

            }

        } else {
            getLogger()->logAction("VTMALStub", "id", mId, "Start (mode = pass or 3G)", VT_SRV_LOG_W);
        }

        getLogger()->logAction("VTMALStub", "id", mId, "Start (Finish)", VT_SRV_LOG_W);
        return VT_SRV_RET_OK;
    }

    status_t VTMALStub::Stop(ma_datapath_t type) {

        getLogger()->logAction("VTMALStub", "id", mId, "Stop (Begin)", VT_SRV_LOG_W);

        if (MA_INVALID_MODE == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Stop (mode = invalid) (Finish)", VT_SRV_LOG_W);
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Stop (mode = 4G)", VT_SRV_LOG_W);

            if (type == MA_SOURCE) {
                getLogger()->logAction("VTMALStub", "id", mId, "Stop (SOURCE)", VT_SRV_LOG_W);
            } else if (type == MA_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Stop (SINK)", VT_SRV_LOG_W);
            } else if (type == MA_SOURCE_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Stop (SOURCE_SINK)", VT_SRV_LOG_W);
            }

            if (mImsMa->Stop(type)) {
                getLogger()->logAction("VTMALStub", "id", mId, "Stop, error (Finish)", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            }
            mPeerSurface = NULL;
            mLocalSurface = NULL;

        } else {
            getLogger()->logAction("VTMALStub", "id", mId, "Stop (mode = pass or 3G)", VT_SRV_LOG_W);
        }

        getLogger()->logAction("VTMALStub", "id", mId, "Stop (Finish)", VT_SRV_LOG_W);
        return VT_SRV_RET_OK;
    }

    status_t VTMALStub::Pause(ma_datapath_t type,imsma_pause_resume_params_t* params) {

        getLogger()->logAction("VTMALStub", "id", mId, "Pause (Begin)", VT_SRV_LOG_W);

        if (MA_INVALID_MODE == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Pause (mode = invalid) (Finish)", VT_SRV_LOG_W);
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Pause (mode = 4G)", VT_SRV_LOG_W);

            if (type == MA_SOURCE) {
                getLogger()->logAction("VTMALStub", "id", mId, "Pause (SOURCE)", VT_SRV_LOG_W);
            } else if (type == MA_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Pause (SINK)", VT_SRV_LOG_W);
            } else if (type == MA_SOURCE_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Pause (SOURCE_SINK)", VT_SRV_LOG_W);
            }

            if (mImsMa->Pause(type, params)) {
                getLogger()->logAction("VTMALStub", "id", mId, "Pause, error (Finish)", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            }
        } else {
            getLogger()->logAction("VTMALStub", "id", mId, "Pause (mode = pass or 3G)", VT_SRV_LOG_W);
        }

        getLogger()->logAction("VTMALStub", "id", mId, "Pause (Finish)", VT_SRV_LOG_W);
        return VT_SRV_RET_OK;
    }

    status_t VTMALStub::Resume(ma_datapath_t type,imsma_pause_resume_params_t* params) {

        getLogger()->logAction("VTMALStub", "id", mId, "Resume (Begin)", VT_SRV_LOG_W);

        if (MA_INVALID_MODE == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Resume (mode = invalid) (Finish)", VT_SRV_LOG_W);
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Resume (mode = 4G)", VT_SRV_LOG_W);

            if (type == MA_SOURCE) {
                getLogger()->logAction("VTMALStub", "id", mId, "Resume (SOURCE)", VT_SRV_LOG_W);
            } else if (type == MA_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Resume (SINK)", VT_SRV_LOG_W);
            } else if (type == MA_SOURCE_SINK) {
                getLogger()->logAction("VTMALStub", "id", mId, "Resume (SOURCE_SINK)", VT_SRV_LOG_W);
            }

            if (mImsMa->Resume(type, params)) {
                getLogger()->logAction("VTMALStub", "id", mId, "Resume, error (Finish)", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            }

        } else {
            getLogger()->logAction("VTMALStub", "id", mId, "Resume (mode = pass or 3G)", VT_SRV_LOG_W);
        }

        getLogger()->logAction("VTMALStub", "id", mId, "Resume (Finish)", VT_SRV_LOG_W);
        return VT_SRV_RET_OK;
    }

    status_t VTMALStub::Reset(ma_datapath_t type) {

        getLogger()->logAction("VTMALStub", "id", mId, "Reset (Begin)", VT_SRV_LOG_W);

        RFX_UNUSED(type);

        if (MA_INVALID_MODE == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Reset (mode = invalid) (Finish)", VT_SRV_LOG_W);
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            getLogger()->logAction("VTMALStub", "id", mId, "Reset (mode = 4G)", VT_SRV_LOG_W);
            mImsMa->Reset(type);
            mMsgHdlr = NULL;
            mImsMa.clear();
            mImsMa = NULL;

        } else {
            getLogger()->logAction("VTMALStub", "id", mId, "Reset (mode = pass or 3G)", VT_SRV_LOG_W);
        }

        getLogger()->logAction("VTMALStub", "id", mId, "Reset (Finish)", VT_SRV_LOG_W);
        return VT_SRV_RET_OK;
    }

    status_t VTMALStub::InitMediaConfig(media_config_t *config) {

        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            int ret = mImsMa->InitMediaConfig(config);

            if (ret == NO_ERROR) {
                return VT_SRV_RET_OK;

            } else if (ret == ERROR_BIND_PORT) {
                getLogger()->logAction("VTMALStub", "id", mId, "InitMediaConfig, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_BIND_PORT;

            } else {
                getLogger()->logAction("VTMALStub", "id", mId, "InitMediaConfig, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            }

        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::UpdateMediaConfig(media_config_t *config) {

        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->UpdateMediaConfig(config)) {
                getLogger()->logAction("VTMALStub", "id", mId, "UpdateMediaConfig, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::UpdateCodecConfig(uint8_t id, video_codec_fmtp_t *video_fmtp) {

        RFX_UNUSED(id);
        RFX_UNUSED(video_fmtp);

        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::SetCameraSensor(int32_t index) {

        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->setCurrentCameraId(index)) {
                getLogger()->logAction("VTMALStub", "id", mId, "setCurrentCameraId, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::setUIMode(int isFg) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            return VT_SRV_RET_OK;

        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::setHandoverState(bool state) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->setHandoverState(state)) {
                getLogger()->logAction("VTMALStub", "id", mId, "setHandoverState, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::SetPeerSurface(const sp<Surface>& peer_surface) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            mPeerSurface = peer_surface;
            if (mImsMa->SetPeerSurface(peer_surface)) {
                getLogger()->logAction("VTMALStub", "id", mId, "SetPeerSurface, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::SetLocalSurface(const sp<Surface>& local_surface) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            mLocalSurface = local_surface;
            return VT_SRV_RET_OK;

        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->getBufferQueueProducer(outBufferProducer)) {
                getLogger()->logAction("VTMALStub", "id", mId, "getBufferQueueProducer, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::SnapShot(const char* url, snapshot_mode_t mode) {

        RFX_UNUSED(url);
        RFX_UNUSED(mode);

        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::SnapShot(sp<IMemory>& buffer, snapshot_mode_t mode) {

        RFX_UNUSED(buffer);
        RFX_UNUSED(mode);

        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else {
            return VT_SRV_RET_OK;
        }
    }

    imsma_turn_off_video_mode_t VTMALStub::GetTurnOffVideoByLocalState() {
        if (MA_INVALID_MODE == mMode) {
            return MA_TURN_OFF_VIDEO_DISABLE;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            return mImsMa->GetTurnOffVideoByLocalState();

        } else {
            return MA_TURN_OFF_VIDEO_DISABLE;
        }
    }

    imsma_turn_off_video_mode_t VTMALStub::GetTurnOffVideoByPeerState() {
        if (MA_INVALID_MODE == mMode) {
            return MA_TURN_OFF_VIDEO_DISABLE;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            return mImsMa->GetTurnOffVideoByPeerState();

        } else {
            return MA_TURN_OFF_VIDEO_DISABLE;
        }
    }

    status_t VTMALStub::SetRecordParameters(record_quality_t quality,  char* file_name) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->SetRecordParameters(quality, file_name)) {
                getLogger()->logAction("VTMALStub", "id", mId, "SetRecordParameters, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::StartRecord(record_mode_t mode) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->StartRecord(mode)) {
                getLogger()->logAction("VTMALStub", "id", mId, "StartRecord, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::StopRecord(void) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->StopRecord()) {
                getLogger()->logAction("VTMALStub", "id", mId, "StopRecord, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::SetVTQuality(VT_Quality quality) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->SetVTQuality(quality)) {
                getLogger()->logAction("VTMALStub", "id", mId, "SetVTQuality, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::setDeviceRotationDegree(int32_t degree) {
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            // Set device orientation only on changed
            if (mCurDegree == degree) {
                return VT_SRV_RET_OK;
            } else {
                mCurDegree = degree;
            }

            if (mImsMa->setDeviceRotationDegree(degree)) {
                getLogger()->logAction("VTMALStub", "id", mId, "setDeviceRotationDegree, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
    }

    status_t VTMALStub::notifyRanBitRate(bool isUL, int32_t bitrate) {
        return VT_SRV_RET_OK;
        #if 0
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->notifyRanBitrate(isUL, bitrate)) {
                getLogger()->logAction("VTMALStub", "id", mId, "notifyRanBitRate, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
        #endif
    }

    status_t VTMALStub::setRANEnable(bool enable) {
        return VT_SRV_RET_OK;
        #if 0
        if (MA_INVALID_MODE == mMode) {
            return VT_SRV_RET_ERR_MA;

        } else if (MA_NORMAL_MODE_4G == mMode) {
            if (mImsMa->setRANEnable(enable)) {
                getLogger()->logAction("VTMALStub", "id", mId, "setRANEnable, error", VT_SRV_LOG_E);
                return VT_SRV_RET_ERR_MA;
            } else {
                return VT_SRV_RET_OK;
            }
        } else {
            return VT_SRV_RET_OK;
        }
        #endif
    }

    sp<VTCoreHelper> VTMALStub::getLogger() {
        return mVTCore->mHelper;
    }

    ImsMaHandler::ImsMaHandler(sp<ImsMa> owner, sp<VTCore> user, int id) {
        mUser = user;
        mMa = owner;
        mId = id;
        mDataResumeSent = false;
    }

    void ImsMaHandler::finalization(void) {
        mUser.clear();
    }

    void ImsMaHandler::exchangeMsg() {

        mLooper = new ALooper();
        mLooper->setName("LTEVT_MA_MSG_HDLR");
        mLooper->start();
        mLooper->registerHandler(this);

        // set MA message callback as notifier
        sp<AMessage> notify;
        notify = new AMessage(kWhatImsMANotify , this);
        if (mMa->SetNotify(notify)) {
            VT_LOGI("[ImsMaHandler] exchangeMsg fail");
        }
    }

    void ImsMaHandler::onMessageReceived(const sp<AMessage> &msg) {
        int32_t reason;

        VT_LOGI("[SRV] [MA MSG HL] onMessageReceived");

        CHECK(msg->findInt32("reason", &reason));
        switch (reason) {
            case kWhatMACodecError: {
                mUser->notifyCallback(mId, VT_SRV_ERROR_CODEC);
            }
            break;
            case kWhatMASnapShotDone: {
                mUser->notifyCallback(mId, VT_SRV_NOTIFY_SNAPSHOT_DONE);
            }
            break;
            case kWhatMARecordNotify: {
                int32_t info;
                CHECK(msg->findInt32("info", &info));

                if (info == MA_REC_INFO_UNKNOWN) {
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_REC_INFO_UNKNOWN);
                } else if (info == MA_REC_INFO_REACH_MAX_DURATION) {
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_REC_INFO_REACH_MAX_DURATION);
                } else if (info == MA_REC_INFO_REACH_MAX_FILESIZE) {
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_REC_INFO_REACH_MAX_FILESIZE);
                } else if (info == MA_REC_INFO_NO_I_FRAME) {
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_REC_INFO_NO_I_FRAME);
                } else if (info == MA_REC_INFO_COMPLETE) {
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_REC_INFO_COMPLETE);
                } else {
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_REC_INFO_UNKNOWN);
                }
            }
            break;
            case kWhatPeerResolutionDegree: {
                int32_t w, h, r;
                CHECK(msg->findInt32("width", &w));
                CHECK(msg->findInt32("height", &h));
                CHECK(msg->findInt32("degree", &r));

                vt_srv_call_ui_config_struct config;
                config.mWidth = w;
                config.mHeight = h;
                config.mRotation = r;

                VTCore::mHelper->setParam(mId, VT_SRV_PARAM_PEER_UI_CONFIG, reinterpret_cast<void *>(&config));

                mUser->notifyCallback(mId, VT_SRV_NOTIFY_PEER_SIZE_CHANGED, w, h, r, String8(""), String8(""), NULL);
            }
            break;
            case kWhatLocalResolutionDegree: {
                int32_t w, h, r;
                CHECK(msg->findInt32("width", &w));
                CHECK(msg->findInt32("height", &h));
                CHECK(msg->findInt32("degree", &r));

                vt_srv_call_ui_config_struct config;
                config.mWidth = w;
                config.mHeight = h;
                config.mRotation = r;

                VTCore::mHelper->setParam(mId, VT_SRV_PARAM_LOCAL_UI_CONFIG, reinterpret_cast<void *>(&config));

                mUser->notifyCallback(mId, VT_SRV_NOTIFY_LOCAL_SIZE_CHANGED, w, h, r, String8(""), String8(""), NULL);
            }
            break;
            case kWhatLocalError_DropBitrate_Fail: {
                mUser->notifyCallback(mId, VT_SRV_NOTIFY_BAD_DATA_BITRATE);
            }
            break;
            case kWhatUpdateMbrDl: {
                int32_t dl;
                CHECK(msg->findInt32("mbr_dl", &dl));
                mUser->notifyCallback(mId, VT_SRV_NOTIFY_AVPF_TMMBR_MBR_DL, dl, 0);
            }
            break;
            case kWhatUpdateDebugInfo: {
                int32_t package_lost;
                CHECK(msg->findInt32("info", &package_lost));
                mUser->notifyCallback(mId, VT_SRV_NOTIFY_PACKET_LOSS_RATE, package_lost, 0);
            }
            break;
            case kWhatPeerDisplayStatus: {
                int32_t data;
                CHECK(msg->findInt32("show", &data));
                if (data == 0) {
                    if (mDataResumeSent) {
                        mUser->notifyCallback(mId, VT_SRV_NOTIFY_DATA_CHANGE_PAUSE);
                        mDataResumeSent = false;
                    } else {
                        VT_LOGI("[SRV] [MA MSG HL] not resumed, skip pause");
                    }
                } else if (data == 1) {
                    bool isCallConnected;
                    isCallConnected = VTCore::mHelper->isCallConnected(mId);
                    if (!isCallConnected) {
                        //for video ringtone, only send if call not  connected yet
                        mUser->notifyCallback(mId, VT_SRV_NOTIFY_RECEIVE_FIRSTFRAME);
                    }
                    mUser->notifyCallback(mId, VT_SRV_NOTIFY_DATA_CHANGE_RESUME);
                    mDataResumeSent = true;
                }
            }
            break;
            case kWhatLocalRestartCamera: {
                mUser->notifyCallback(mId, VT_SRV_NOTIFY_RESTART_CAMERA);
            }
            break;
            #if 0
            case kWhatSendANBRQ: {
                int32_t is_ul;
                int32_t ebi;
                int32_t bitrate;
                CHECK(msg->findInt32("is_ul", &is_ul));
                CHECK(msg->findInt32("ebi", &ebi));
                CHECK(msg->findInt32("bitrate", &bitrate));
                mUser->notifyCallback(mId, VT_SRV_NOTIFY_SET_ANBR, is_ul, ebi, bitrate, String8(""), String8(""), NULL);
                break;
            }
            #endif
        }
    }

    MsgHandler::MsgHandler(sp<VTCore> user, int id, sp<ImsMa> imsMa, int mode) {

        mMode = mode;
        if (mode == 1) {
            mImsHdlr = new ImsMaHandler(imsMa, user, id);
            mMode = 1;
        }
    }

   void MsgHandler::finalization(void) {
       if (mMode == 1) {
           mImsHdlr->finalization();
       }
   }

   void MsgHandler::exchangeMsg() {
       if (mMode == 1) {
           mImsHdlr->exchangeMsg();
       }
   }
}
