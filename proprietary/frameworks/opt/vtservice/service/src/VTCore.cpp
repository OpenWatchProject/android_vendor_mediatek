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
#include <sys/prctl.h>
#include "VTCore.h"
#include "VTCoreHelper.h"
#include "VTAVSync.h"

// for get user ID
#include <unistd.h>
#include <sys/types.h>

using namespace android;
using android::INVALID_OPERATION;
using android::ALREADY_EXISTS;

namespace VTService {

    vt_srv_cntx_struct g_vt;
    sp<VTCoreHelper> VTCore::mHelper = NULL;

    VTCore::VTCore(const sp<IVTServiceCallBack> & user) {
        mHelper = new VTCoreHelper();

        mHelper->logFlow("create", "", VT_IVD, "", VT_SRV_LOG_I);

        // init cntx
        mUser     = user;
        g_vt.core = this;

        if (mHelper->isVoLTEon()) {
            g_vt.avSync = new VTAVSync();
        }

        mHelper->init();

        mUserID   = getuid();
        mHelper->logFlow("create", "User ID is : ", mUserID, "", VT_SRV_LOG_I);

        // init call flow locks
        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            mCallFlowLocks[i] = new Mutex();
        }
    }

    VTCore::~VTCore() {
        mUser     = NULL;
        mHelper   = NULL;
        g_vt.core = NULL;
        g_vt.avSync = NULL;

        mHelper->logFlow("~VTCore ", " error ~VTCore() here, abort to get backtrace", VT_IVD, "", VT_SRV_LOG_E);
        abort();
    }

    status_t VTCore::initialization(int mode, int id, int sim_id) {
        if (!mHelper->getIsSetSensorInfo()) {
            mHelper->logFlow("initialization", "id", id, "not set sensor information yet", VT_SRV_LOG_I);

            notifyCallback(
            id,
            VT_SRV_NOTIFY_GET_SENSOR_INFO,
            0,
            0,
            0,
            String8(""),
            String8(""),
            NULL);
        }

        return open((VT_SRV_CALL_MODE)mode, id, sim_id);
    }

    status_t VTCore::finalization(int id) {
        mHelper->logFlow("finalization", "id", id, "start", VT_SRV_LOG_I);

        close(id, MA_SOURCE_SINK);

        mHelper->logFlow("finalization", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;
    }

    status_t VTCore::open(VT_SRV_CALL_MODE mode, const int id, const int sim_id) {
        Mutex::Autolock malLock(*getCallFlowLock(id));

        mHelper->logFlow("open", "id", id, "start", VT_SRV_LOG_I);

        if (VT_SRV_CALL_4G == mode) {

            mHelper->logFlow("open", "id", id, "4G mode", VT_SRV_LOG_I);

            // ====================================================================
            // create MA , add to map, init MA
            sp<VTMALStub> currentMa = NULL;
            int opId = mHelper->getOperatorId(sim_id);

            int labOp = mHelper->getLabOperator();
            if (0 != labOp) {
                opId = labOp;
            }

            mHelper->logFlow("open", "id", id, "real MA", VT_SRV_LOG_I);
            currentMa = new VTMALStub(VTMALStub::MA_NORMAL_MODE_4G, id, sim_id, opId, this);

            mLastRet = currentMa->Init(MA_SOURCE_SINK);
            if (mLastRet) goto error;

            // every operation need to after ImsMa::Init()
            // add() will set Msg to MA
            mLastRet = mHelper->add(id, currentMa);
            if (mLastRet) goto error;

            //Notify default local size to VTP for preview
            int defaultW, defaultH;
            mHelper->getDefaultLocalSize(&defaultW, &defaultH);

            notifyCallback(
                id,
                VT_SRV_NOTIFY_DEFAULT_LOCAL_SIZE,
                defaultW,
                defaultH,
                0,
                String8(""),
                String8(""),
                NULL);

        } else {
            goto error;
        }

        // init force stop param
        // ====================================================================
        bool *isForceStop;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_STOP, reinterpret_cast<void **>(&isForceStop));
        if (mLastRet) goto error;

        (*isForceStop) = VT_FALSE;

        // init MA crash param
        // ====================================================================
        bool *isMaCrash;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_MA_CRASH, reinterpret_cast<void **>(&isMaCrash));
        if (mLastRet) goto error;

        (*isMaCrash) = VT_FALSE;

        // init call connected param
        // ====================================================================
        int *callState;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_CALL_STATE, reinterpret_cast<void **>(&callState));
        if (mLastRet) goto error;

        (*callState) = VT_CALL_STATE_UNKNOWN;


        // init call param
        // ====================================================================
        vt_srv_call_update_info_struct *info;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
        if (mLastRet) goto error;

        info->mIsHold               = VT_FALSE;
        info->mIsResume             = VT_FALSE;
        info->mIsHeld               = VT_FALSE;
        info->mIsResumed            = VT_FALSE;
        info->mVdoRtpPort           = VT_FALSE;
        info->mVdoRtcpPort          = VT_FALSE;
        info->mIsTxInPause          = VT_FALSE;
        info->mIsRxInPause          = VT_FALSE;

        mLastRet = mHelper->setParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void *>(info));
        if (mLastRet) goto error;
        // ====================================================================

        mHelper->setState(id, VT_SRV_MA_STATE_OPENED);

        mHelper->logFlow("open", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "open");
            return mLastRet;
    }

    status_t VTCore::init(const int id) {
        Mutex::Autolock malLock(*getCallFlowLock(id));

        media_config_t* ma_config;

        mHelper->logFlow("init", "id", id, "start", VT_SRV_LOG_I);

        VT_SRV_MA_STATE state = mHelper->getState(id);
        mLastRet = mHelper->checkRange(state, VT_SRV_MA_STATE_OPENED, VT_SRV_MA_STATE_INITED);
        if (mLastRet) goto error;

        // ====================================================================
        // if voice call, no need to InitMediaConfig(), we set it until update to video call
        VT_IMCB_CONFIG * ua;
        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
        if (mLastRet) goto error;

        if (ua->setting.mode == 0) {
            mHelper->logFlow("init", "id", id, "voice call", VT_SRV_LOG_I);
            mHelper->logFlow("init", "id", id, "finish", VT_SRV_LOG_I);
            return VT_SRV_RET_OK;
        }
        // ====================================================================

        mLastRet = init_internal(id);
        if (mLastRet) goto error;

        mHelper->logFlow("init", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "init");
            return mLastRet;
    }

    status_t VTCore::init_internal(const int id) {
        mHelper->logFlow("init internal", "id", id, "start", VT_SRV_LOG_I);

        VT_SRV_CALL_MODE mode = mHelper->getMode(id);

        if (VT_SRV_CALL_4G == mode) {
            media_config_t* ma_config;

            // ====================================================================
            // Config MA
            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_MA_CONFIG, reinterpret_cast<void **>(&ma_config));
            if (mLastRet) goto error;

            VT_SRV_MA_STATE state = mHelper->getState(id);

            // If alreay inited (MD silent redial)
            if (VT_SRV_MA_STATE_INITED == state) {
                mLastRet = mHelper->get(id)->UpdateMediaConfig(ma_config);
            } else {
                mLastRet = mHelper->get(id)->InitMediaConfig(ma_config);
            }
            if (mLastRet) goto error;
            // ===================================================================

            vt_srv_call_update_info_struct *info;
            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
            if (mLastRet) goto error;

        } else {
            goto error;
        }

        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_INITED);
        if (mLastRet) goto error;

        mHelper->logFlow("init internal", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "init internal");
            return mLastRet;
    }

    status_t VTCore::update(const int id) {
        Mutex::Autolock malLock(*getCallFlowLock(id));

        mHelper->logFlow("update", "id", id, "start", VT_SRV_LOG_I);

        if (VT_SRV_MA_STATE_OPENED == mHelper->getState(id)) {

            VT_IMCB_CONFIG * ua;

            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
            if (mLastRet) goto error;

            if (ua->setting.mode != 0) {
                // it mean not init and uddate directly
                // it is possible for
                //   1. if no bandwidth
                //   2. voice call firstly and then upgrade to video call
                init_internal(id);
            } else {
                // it mean a initial volte and still volte, do nothing
                return VT_SRV_RET_OK;
            }

        } else if (VT_SRV_MA_STATE_STOPED == mHelper->getState(id) ||
                   VT_SRV_MA_STATE_PRE_STOP == mHelper->getState(id)) {
            // it mean the call has end up
            return VT_SRV_RET_OK;
        }

        mLastRet = updateCallMode(id);
        if (mLastRet) goto error;

        mHelper->logFlow("update", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "update");
            return mLastRet;
    }

    status_t VTCore::updateCallMode(const int id) {
        VT_IMCB_CONFIG * ua;
        media_config_t* ma_config;
        vt_srv_call_update_info_struct *info;

        mHelper->logFlow("updateCallMode", "id", id, "start", VT_SRV_LOG_I);

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
        if (mLastRet) goto error;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_MA_CONFIG, reinterpret_cast<void **>(&ma_config));
        if (mLastRet) goto error;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
        if (mLastRet) goto error;

        mHelper->logFlow("updateCallMode", "ua->setting.mode", id, mHelper->getCallModeString(ua->setting.mode), VT_SRV_LOG_I);
        mHelper->logFlow("updateCallMode", "info->mIsHold", id, mHelper->getOnOffString(info->mIsHold), VT_SRV_LOG_I);
        mHelper->logFlow("updateCallMode", "info->mIsHeld", id, mHelper->getOnOffString(info->mIsHeld), VT_SRV_LOG_I);
        mHelper->logFlow("updateCallMode", "info->mIsResume", id, mHelper->getOnOffString(info->mIsResume), VT_SRV_LOG_I);
        mHelper->logFlow("updateCallMode", "info->mIsResumed", id, mHelper->getOnOffString(info->mIsResumed), VT_SRV_LOG_I);
        mHelper->logFlow("updateCallMode", "info->mIsTxInPause", id, mHelper->getOnOffString(info->mIsTxInPause), VT_SRV_LOG_I);
        mHelper->logFlow("updateCallMode", "info->mIsRxInPause", id, mHelper->getOnOffString(info->mIsTxInPause), VT_SRV_LOG_I);

        // ====================================================================
        if (ua->setting.mode == 0) {
            // if become VoLTE call, just close MA
            mLastRet = close_internal(id, MA_DOWNGRADE);
            if (mLastRet) goto error;

            return VT_SRV_RET_OK;
        } else {
            // We should only set config to MA when ViLTE call
            // or the parameter may be mess and cause error.
            mLastRet = mHelper->get(id)->UpdateMediaConfig(ma_config);
            if (mLastRet) goto error;

            // if still ViLTE Call, update MA accoring to direction
            mLastRet = updateTxRxMode(id, ua->config.rtp_direction, info);
            if (mLastRet) goto error;
        }
        // ====================================================================

        mHelper->logFlow("updateCallMode", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "updateCallMode");
            return mLastRet;
    }

    status_t VTCore::updateTxRxMode(const int id, int new_mode, vt_srv_call_update_info_struct *info) {
        mHelper->logFlow("updateTxRxMode", "id", id, "start", VT_SRV_LOG_I);
        mHelper->logFlow("updateTxRxMode", "new_mode", id, mHelper->getVideoStateString(new_mode), VT_SRV_LOG_I);

        VT_SRV_CALL_MODE mode = mHelper->getMode(id);
        sp<VTMALStub> currentMa = mHelper->get(id);
        VT_SRV_MA_STATE curr_state = mHelper->getState(id);

        mHelper->logFlow("updateTxRxMode", "curr_state", id, mHelper->getStateString(curr_state), VT_SRV_LOG_I);

        // ====================================================================
        // wait for set all surface done
        // we design the Surface status to be the same with MA mode
        // so we can go on until mode == surface status
        VT_SRV_SURFACE_STATE * setSfState;
        int loop_counter = 0;
        int isTxSurfaceeady = 0;
        int isRxSurfaceeady = 0;

        do {
            // init force stop param
            // ====================================================================
            bool *isForceStop;
            bool *isMaCrash;
            VT_BOOL *isForceCancel;
            VT_BOOL *isDowngraded;

            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_STOP, reinterpret_cast<void **>(&isForceStop));
            if (mLastRet || (*isForceStop) == VT_TRUE) {
                mHelper->logFlow("updateTxRxMode", "id", id, "Force close!!!", VT_SRV_LOG_I);
                mHelper->logFlow("updateTxRxMode", "id", id, "finish", VT_SRV_LOG_I);
                return VT_SRV_RET_OK;
            }

            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_MA_CRASH, reinterpret_cast<void **>(&isMaCrash));
            if (mLastRet || (*isMaCrash) == VT_TRUE) {
                mHelper->logFlow("updateTxRxMode", "id", id, "MA crash!!!", VT_SRV_LOG_I);
                mHelper->logFlow("updateTxRxMode", "id", id, "finish", VT_SRV_LOG_I);
                return VT_SRV_RET_OK;
            }

            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_CANCEL, reinterpret_cast<void **>(&isForceCancel));
            if ((VT_SRV_RET_OK == mLastRet) && (*isForceCancel) == VT_TRUE) {
                mHelper->logFlow("updateTxRxMode", "Force cancel!!!", VT_IVD, "", VT_SRV_LOG_I);
                *isForceCancel = VT_FALSE;
                return VT_SRV_RET_OK;
            }

            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_SURFACE_STATE, reinterpret_cast<void **>(&setSfState));
            if (mLastRet) goto error;

            mHelper->logFlow("updateTxRxMode", "id", id, mHelper->getSurfaceString(*setSfState), VT_SRV_LOG_I);

            usleep(50 * 1000);

            // ready mean :
            //    1. no need surface  => !(new_mode & VT_SRV_SURFACE_STATE_LOCAL)
            //    2. has set surface  => (*setSfState & VT_SRV_SURFACE_STATE_LOCAL)
            if (((*setSfState) & VT_SRV_SURFACE_STATE_LOCAL) ||
                !(new_mode & VT_SRV_SURFACE_STATE_LOCAL)) {
                isTxSurfaceeady = 1;
            }

            // ready mean :
            //    1. no need surface  => !(new_mode & VT_SRV_SURFACE_STATE_PEER)
            //    2. has set surface  => (setSfState & VT_SRV_SURFACE_STATE_PEER)
            if (((*setSfState) & VT_SRV_SURFACE_STATE_PEER) ||
                !(new_mode  & VT_SRV_SURFACE_STATE_PEER)) {
                isRxSurfaceeady = 1;
            }

            // We only try 5 times for most of normal case.
            // For the case surface set slowly or not to set,
            // we just do what we can do now and do retrigerUpdata when surface is set
            loop_counter++;

        } while (loop_counter < 3 && (new_mode & (*setSfState)) != new_mode); // all surface need in this mode is set

        if (VT_SRV_CALL_4G == mode) {

            // We need to make sure that
            // pause/resume operation cannot overlapping with operation (ex: setCamera) between different MAs
            // or the camera status inside MA may get into a mess
            mMAOperationLock.lock();


            mHelper->logFlow("updateTxRxMode", "id", id, "other action update", VT_SRV_LOG_I);

            imsma_pause_resume_params_t PauseInfo;
            imsma_pause_resume_params_t PauseByPeerInfo;

            PauseInfo.turnOffVideo.direction = MA_TURN_OFF_VIDEO_BY_LOCAL;
            PauseInfo.mode = MA_PAUSE_RESUME_NORMAL;

            PauseByPeerInfo.turnOffVideo.direction = MA_TURN_OFF_VIDEO_BY_PEER;
            PauseByPeerInfo.mode = MA_PAUSE_RESUME_NORMAL;

            bool oldTxIsOn = ((curr_state == VT_SRV_MA_STATE_START_UD) || (curr_state == VT_SRV_MA_STATE_START_UI) || (curr_state == VT_SRV_MA_STATE_START_UN));
            bool newTxIsOn = ((new_mode == VT_SRV_DATA_PATH_SOURCE) || (new_mode == VT_SRV_DATA_PATH_SOURCE_SINK));
            bool oldRxIsOn = ((curr_state == VT_SRV_MA_STATE_START_UD) || (curr_state == VT_SRV_MA_STATE_START_ID) || (curr_state == VT_SRV_MA_STATE_START_ND));
            bool newRxIsOn = ((new_mode == VT_SRV_DATA_PATH_SINK) || (new_mode == VT_SRV_DATA_PATH_SOURCE_SINK));

            mHelper->logFlow("updateCallMode", "oldTxIsOn", id, mHelper->getOnOffString(oldTxIsOn), VT_SRV_LOG_I);
            mHelper->logFlow("updateCallMode", "newTxIsOn", id, mHelper->getOnOffString(newTxIsOn), VT_SRV_LOG_I);
            mHelper->logFlow("updateCallMode", "oldRxIsOn", id, mHelper->getOnOffString(oldRxIsOn), VT_SRV_LOG_I);
            mHelper->logFlow("updateCallMode", "newRxIsOn", id, mHelper->getOnOffString(newRxIsOn), VT_SRV_LOG_I);

            if (oldTxIsOn && !newTxIsOn) {

                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_PAUSE_REC,
                    MA_PAUSE_RESUME_HOLD,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                info->mIsTxInPause = VT_TRUE;
            }

            if (info->mIsHold) {
                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_PAUSE_REC,
                    MA_PAUSE_RESUME_HOLD,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                info->mIsTxInPause = VT_TRUE;
            }

            if ((!oldTxIsOn && newTxIsOn)) {

                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_RESUME_REC,
                    MA_PAUSE_RESUME_HOLD,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                info->mIsTxInPause = VT_FALSE;
            }

            if (info->mIsResume) {

                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_RESUME_REC,
                    MA_PAUSE_RESUME_HOLD,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                info->mIsTxInPause = VT_FALSE;
            }

            if (oldRxIsOn && !newRxIsOn) {
                info->mIsRxInPause = VT_TRUE;
            }
            if (!oldTxIsOn && newTxIsOn) {
                info->mIsRxInPause = VT_FALSE;
            }

            if (VT_SRV_MA_STATE_INITED == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {
                    // do nothing
                } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UI);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ID);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UI);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ID);
                        if (mLastRet) goto error;

                    }
                }

            } else if (VT_SRV_MA_STATE_START_IN == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {
                    // do nothing
                } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isRxSurfaceeady) {
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ID);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {


                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ID);
                        if (mLastRet) goto error;
                    }
                }

            } else if (VT_SRV_MA_STATE_START_NI == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {
                    // do nothing
                } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UI);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UI);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;

                    }
                }

            } else if (VT_SRV_MA_STATE_START_ID == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {

                    if (isRxSurfaceeady) {
                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_IN);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_IN);
                        if (mLastRet) goto error;
                    }

                } else if (VT_SRV_DATA_PATH_SINK == new_mode) {
                    // do nothing
                } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SOURCE);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;
                    }
                }

            } else if (VT_SRV_MA_STATE_START_UI == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NI);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {
                    // do nothing
                 } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NI);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isRxSurfaceeady) {

                        mLastRet = currentMa->Start(MA_SINK);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;
                    }
                 }

            } else if (VT_SRV_MA_STATE_START_NN == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {
                     // do nothing
                 } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isRxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;
                    }

                 }

            } else if (VT_SRV_MA_STATE_START_ND == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {

                    if (isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NN);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NN);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SINK == new_mode) {
                    // do nothing
                 } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isTxSurfaceeady) {

                        mLastRet = currentMa->Resume(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;
                    }
                 }

            } else if (VT_SRV_MA_STATE_START_UN == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {

                    if (isTxSurfaceeady) {
                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NN);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {
                    // do nothing
                 } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {
                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NN);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {

                    if (isRxSurfaceeady) {
                        mLastRet = currentMa->Resume(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UD);
                        if (mLastRet) goto error;
                    }
                 }

            } else if (VT_SRV_MA_STATE_START_UD == curr_state) {
                if (VT_SRV_DATA_PATH_NONE == new_mode) {

                    if (isTxSurfaceeady && isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;
                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_NN);
                        if (mLastRet) goto error;

                    } else if (isTxSurfaceeady) {
                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;

                    } else if (isRxSurfaceeady) {

                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE == new_mode) {

                    if (isRxSurfaceeady) {
                        mLastRet = currentMa->Pause(MA_SINK, &PauseByPeerInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_UN);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SINK == new_mode) {

                    if (isTxSurfaceeady) {
                        mLastRet = currentMa->Pause(MA_SOURCE, &PauseInfo);
                        if (mLastRet) goto error;

                        mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_START_ND);
                        if (mLastRet) goto error;
                    }

                 } else if (VT_SRV_DATA_PATH_SOURCE_SINK == new_mode) {
                    // do nothing
                 }
            }

            mHelper->logFlow("updateTxRxMode", "id", id, "finish", VT_SRV_LOG_I);
            mMAOperationLock.unlock();

        } else {
            goto error;
        }

        return VT_SRV_RET_OK;

        error:
            if (VT_SRV_CALL_4G == mode) {
                mMAOperationLock.unlock();
            }
            notifyError(id, "updateTxRxMode");
            return mLastRet;
    }

    status_t VTCore::deinit(const int id) {
        Mutex::Autolock malLock(*getCallFlowLock(id));

        mHelper->logFlow("deinit", "id", id, "start", VT_SRV_LOG_I);

        VT_SRV_MA_STATE state = mHelper->getState(id);

        mLastRet = mHelper->checkRange(state, VT_SRV_MA_STATE_OPENED, VT_SRV_MA_STATE_END - 1);

        if (mLastRet == VT_SRV_RET_OK) {
            Mutex::Autolock malLock(mMAOperationLock);

            mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_PRE_STOP);
            if (mLastRet) goto error;

            mLastRet = mHelper->get(id)->Stop(MA_SOURCE_SINK);
            if (mLastRet) goto error;

            notifyCallback(
                id,
                VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                VT_SRV_VTCAM_STATE_STOP_REC_AND_PREVIEW,
                0,
                0,
                String8(""),
                String8(""),
                NULL);

            mHelper->logFlow("deinit", "id", id, "Stop done", VT_SRV_LOG_I);

            if (VT_SRV_MA_STATE_OPENED == state) {
                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_OPENED);
            } else {
                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_INITED);
            }
            if (mLastRet) goto error;

            mHelper->logFlow("deinit", "id", id, "Update state", VT_SRV_LOG_I);

        } else {
            mHelper->logFlow("deinit", "id", id, "this call has removed", VT_SRV_LOG_I);
        }

        mHelper->logFlow("deinit", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "deinit");
            return mLastRet;
    }

    status_t VTCore::close(const int id, int close_mode) {

        mHelper->logFlow("close", "id", id, "start", VT_SRV_LOG_I);

        // if still blocked at update
        // use this flag to break out to prevent from dead lock
        // need to set before lock
        bool *isForceStop;
        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_STOP, reinterpret_cast<void **>(&isForceStop));
        if (mLastRet) {
            mHelper->logFlow("close", "id", id, "this call has removed", VT_SRV_LOG_E);
            return VT_SRV_RET_OK;
        }
        (*isForceStop) = VT_TRUE;

        Mutex::Autolock malLock(*getCallFlowLock(id));
        return close_internal(id, close_mode);
    }

    status_t VTCore::close_internal(const int id, int close_mode) {
        Mutex::Autolock malLock(mMAOperationLock);

        mHelper->logFlow("close internal", "id", id, "start", VT_SRV_LOG_I);
        mHelper->logFlow("close internal", "id", id, mHelper->getVideoStateString(close_mode), VT_SRV_LOG_I);

        VT_SRV_CALL_MODE mode = mHelper->getMode(id);
        VT_SRV_MA_STATE state = mHelper->getState(id);

        mLastRet = mHelper->checkRange(state, VT_SRV_MA_STATE_OPENED, VT_SRV_MA_STATE_END - 1);

        if (mLastRet) {
            //When camera crash happen, call state will set to STOPED but not delete record in mHelper.
            //Need delete MA record here.
            if (VT_SRV_RET_OK == mHelper->check(id) && VT_SRV_MA_STATE_STOPED == state) {
                vt_srv_call_update_info_struct *info;

                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
                if (mLastRet) goto error;

                mLastRet = mHelper->del(id);
                if (mLastRet) goto error;

                mHelper->logFlow("close internal", "current call count", mHelper->getUsedNr(), "", VT_SRV_LOG_I);

                notifyCallback(id, VT_SRV_NOTIFY_CALL_END);
            } else {
                mHelper->logFlow("close internal", "id", id, "has close or deleted, do nothing just skip", VT_SRV_LOG_E);
                mHelper->logFlow("close internal", "id", id, "finish", VT_SRV_LOG_I);
            }

            return VT_SRV_RET_OK;
        }

        if (VT_SRV_CALL_4G == mode) {
            if (close_mode != MA_DOWNGRADE) {
                if (close_mode == MA_SOURCE_SINK) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_PRE_STOP);
                    if (mLastRet) goto error;
                }
                mLastRet = mHelper->get(id)->Stop((ma_datapath_t) close_mode);
                if (mLastRet) goto error;

                if (close_mode & MA_SOURCE) {
                    notifyCallback(
                        id,
                        VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                        VT_SRV_VTCAM_STATE_STOP_REC_AND_PREVIEW,
                        0,
                        0,
                        String8(""),
                        String8(""),
                        NULL);
                }

            } else {
                mLastRet = mHelper->get(id)->Stop(MA_SOURCE_SINK);
                if (mLastRet) goto error;

                // No need stop preview when downgrade, InCallUI will set camera null to stop preview
                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_STOP_REC,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);
            }

            mHelper->logFlow("close internal", "id", id, "Stop done", VT_SRV_LOG_I);

            if (close_mode == MA_SOURCE) {

                // video call once, and already set local surface null
                if (mHelper->getState(id) == VT_SRV_MA_STATE_STOPED_D) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_INITED);
                    if (mLastRet) goto error;

                // keep voice call, and already set local surface null
                } else if (mHelper->getState(id) == VT_SRV_MA_STATE_VOICE_STOPED_D) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_OPENED);
                    if (mLastRet) goto error;

                // video call once
                } else if (mHelper->getState(id) > VT_SRV_MA_STATE_OPENED) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_STOPED_U);
                    if (mLastRet) goto error;

                // keep voice call
                } else {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_VOICE_STOPED_U);
                    if (mLastRet) goto error;
                }

            } else if (close_mode == MA_SINK) {

                // video call once, and already set peer surface null
                if (mHelper->getState(id) == VT_SRV_MA_STATE_STOPED_U) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_INITED);
                    if (mLastRet) goto error;

                // keep voice call, and already set peer surface null
                } else if (mHelper->getState(id) == VT_SRV_MA_STATE_VOICE_STOPED_U) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_OPENED);
                    if (mLastRet) goto error;

                // video call once
                } else if (mHelper->getState(id) > VT_SRV_MA_STATE_OPENED) {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_STOPED_D);
                    if (mLastRet) goto error;

                // keep voice call
                } else {
                    mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_VOICE_STOPED_D);
                    if (mLastRet) goto error;
                }

            } else if (close_mode == MA_SOURCE_SINK) {
                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_STOPED);
                if (mLastRet) goto error;
            } else if (close_mode == MA_DOWNGRADE) {
                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_INITED);
                if (mLastRet) goto error;

                VT_BOOL *isRecvUpdate;
                mHelper->getParam(id, VT_SRV_PARAM_IS_RECV_UPDATE, reinterpret_cast<void **>(&isRecvUpdate));
                (*isRecvUpdate) = VT_FALSE;
            }

            mHelper->logFlow("close internal", "id", id, "Update state", VT_SRV_LOG_I);

            if (VT_SRV_MA_STATE_STOPED == mHelper->getState(id)) {

                vt_srv_call_update_info_struct *info;

                mLastRet = mHelper->get(id)->Reset(MA_SOURCE_SINK);
                if (mLastRet) goto error;

                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
                if (mLastRet) goto error;

                mLastRet = mHelper->del(id);
                if (mLastRet) goto error;

                notifyCallback(id, VT_SRV_NOTIFY_CALL_END);

            } else {
                mHelper->logFlow("close internal", "id", id, "reset IsForceStop", VT_SRV_LOG_I);

                // for downgrade case, there will be update event after this
                // so we need reset the flag or update will skip foever.
                bool *isForceStop;
                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_STOP, reinterpret_cast<void **>(&isForceStop));
                if (mLastRet) goto error;

                (*isForceStop) = VT_FALSE;
            }

        } else {
            goto error;
        }

        mHelper->logFlow("close internal", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            // no matter what kind of error, need to clear entry or the next call won't fail
            mLastRet = mHelper->del(id);

            notifyError(id, "close");
            return mLastRet;
    }

    status_t VTCore::clearAll(void) {
        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            Mutex::Autolock malLock(*mCallFlowLocks[i]);
        }

        Mutex::Autolock malLock(mMAOperationLock);

        mHelper->logFlow("clearAll", "id", VT_IVD, "", VT_SRV_LOG_I);

        sp<VTMALStub> currentMa = NULL;
        vt_srv_call_update_info_struct *info;

        while (1) {
            currentMa = mHelper->pop();

            if (currentMa.get() == NULL) {
                break;
            }

            int id = currentMa->getId();

            if (VT_SRV_CALL_NONE != mHelper->getMode(currentMa->getId()) &&
                VT_SRV_MA_STATE_STOPED != mHelper->getState(currentMa->getId()) &&
                VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(currentMa->getId())) {

                mLastRet = currentMa->Stop(MA_SOURCE_SINK);
                if (mLastRet) goto error;

                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_STOP_REC_AND_PREVIEW,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                mLastRet = currentMa->Reset(MA_SOURCE_SINK);
                if (mLastRet) goto error;
            }

            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
            if (mLastRet) goto error;

            mHelper->del(id);

            mHelper->logFlow("clearAll", "current call count", mHelper->getUsedNr(), "", VT_SRV_LOG_I);

            notifyCallback(id, VT_SRV_NOTIFY_CALL_END);
        }

        mLastRet = mHelper->init();
        if (mLastRet) goto error;

        return VT_SRV_RET_OK;

        error:
            notifyError(0, "clearAll");
            return mLastRet;
    }

    Mutex* VTCore::getCallFlowLock(int id) {
        int callId = GET_CALL_ID(id);

        mLastRet = mHelper->checkRange(callId, 1, VT_SRV_MA_NR);
        if (mLastRet) {
            mHelper->logFlow("getCallFlowLock", "id", callId, "this call id is invalid", VT_SRV_LOG_E);
            // Should be never happen, if then return the first lock.
            return mCallFlowLocks[0];
        }

        return mCallFlowLocks[callId - 1];
    }

    status_t VTCore::setHandoverState(bool state) {
        Mutex::Autolock malLock(mMAOperationLock);

        int idx = 0;
        mHelper->logAction("setHandoverState", "id", VT_IVD, mHelper->getHOString(state), VT_SRV_LOG_I);

        //set handover for all calls
        for (idx = 0; idx < VT_SRV_MA_NR; idx++) {
            if (mHelper->isUsed(idx)) {
                mLastRet = mHelper->getFromIndex(idx)->setHandoverState(state);
                if (mLastRet) goto error;
            }
        }
        return VT_SRV_RET_OK;

        error:
            notifyError(mHelper->getId(idx), "setHandoverState");
            return mLastRet;
    }

    status_t VTCore::setHandoverStateBySimId(int sim_id, bool state) {
        Mutex::Autolock malLock(mMAOperationLock);

        int idx = 0;
        mHelper->logAction("setHandoverStateBySimId", "id", sim_id, mHelper->getHOString(state), VT_SRV_LOG_I);

        //set handover for all calls
        for (idx = 0; idx < VT_SRV_MA_NR; idx++) {
            if (mHelper->isUsed(idx)) {
                if (sim_id == GET_SIM_ID(mHelper->getId(idx))) {
                    mLastRet = mHelper->getFromIndex(idx)->setHandoverState(state);
                    if (mLastRet) goto error;
                }
            }
        }
        return VT_SRV_RET_OK;

        error:
            notifyError(mHelper->getId(idx), "setHandoverState");
            return mLastRet;
    }

    status_t VTCore::setHandoverStateByNetworkId(int network_id, bool state) {
        Mutex::Autolock malLock(mMAOperationLock);

        int idx = 0;
        mHelper->logAction("setHandoverStateByNetworkId", "id", network_id, mHelper->getHOString(state), VT_SRV_LOG_I);

        VT_IMCB_CONFIG * ua;
        int id = 0;

        //set handover for all calls
        for (idx = 0; idx < VT_SRV_MA_NR; idx++) {
            if (mHelper->isUsed(idx)) {
                id = mHelper->getId(idx);
                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
                if (mLastRet) goto error;

                if (ua->setting.network_id == network_id) {
                    mLastRet = mHelper->getFromIndex(idx)->setHandoverState(state);
                    if (mLastRet) goto error;
                }
            }
        }
        return VT_SRV_RET_OK;

        error:
            notifyError(id, "setHandoverStateByNetworkId");
            return mLastRet;
    }

    status_t VTCore::setCamera(int id, int cam) {
        mHelper->logAction("setCamera", "id", id, mHelper->getCamString(cam), VT_SRV_LOG_I);

        if (VT_SRV_MA_STATE_PRE_STOP == mHelper->getState(id) ||
            VT_SRV_MA_STATE_STOPED == mHelper->getState(id)) {
            mHelper->logAction("setCamera", "id", id, "Call has stopped", VT_SRV_LOG_I);
            return VT_SRV_RET_OK;
        }

        mLastRet = mHelper->get(id)->SetCameraSensor(cam);
        if (mLastRet) goto error;

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "setCamera");
            return mLastRet;
    }

    status_t VTCore::setPreviewSurface(int id, const sp<VTSurface> & surface) {

        VT_SRV_SURFACE_STATE* state;
        VT_SRV_SURFACE_STATE pre_state;
        vt_srv_call_update_info_struct *info;
        VT_BOOL *isRecvUpdate;

        mHelper->logAction("setPreviewSurface", "id", id, "", VT_SRV_LOG_I);

        if (VT_SRV_MA_STATE_PRE_STOP == mHelper->getState(id) ||
            VT_SRV_MA_STATE_STOPED == mHelper->getState(id)) {
            mHelper->logAction("setPreviewSurface", "id", id, "Call has stopped", VT_SRV_LOG_I);
            return VT_SRV_RET_OK;
        }

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_SURFACE_STATE, reinterpret_cast<void **>(&state));
        if (mLastRet) goto error;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
        if (mLastRet) goto error;

        mHelper->logAction("setPreviewSurface", "state (before)", id, mHelper->getSurfaceString((*state)), VT_SRV_LOG_I);

        pre_state = (*state);

        if (NULL != surface.get()) {

            // The resume and stop operation may come at the same time.
            // We need to add flow lock to guarantee the correct order.
            mMAOperationLock.lock();

            mLastRet = mHelper->get(id)->SetLocalSurface(surface);
            if (mLastRet) {
                mMAOperationLock.unlock();
                goto error;
            }

            (*state) = (VT_SRV_SURFACE_STATE) ((static_cast<int>(*state)) | VT_SRV_SURFACE_STATE_LOCAL);

            mHelper->logAction("setPreviewSurface", "state (after)", id, mHelper->getSurfaceString((*state)), VT_SRV_LOG_I);

            // We Pause MA instead od Stop MA when clear surrface
            // Because it may happen during conference but not call end or dowgrade
            // We want to keep use MA and keep memory of previous operation
            // so we need to resume it when set surface
            imsma_pause_resume_params_t updateInfo;
            updateInfo.mode = MA_PAUSE_RESUME_HOLD;
            updateInfo.hold.direction = MA_HOLD_BY_LOCAL;

            // we skip the 1st set surface and the case set surface continuoisly
            if (VT_SRV_MA_STATE_INITED < mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_IN != mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_ID != mHelper->getState(id) &&
                VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(id) &&
                VT_TRUE != info->mIsTxInPause &&
                pre_state != (*state)) {
                mLastRet = mHelper->get(id)->Resume(MA_SOURCE, &updateInfo);
            }

            mMAOperationLock.unlock();

            // we don't update state here, because it is not rtp direction change
            // if we modify state, it may cause additional operation when UA notify the direction change
            // so we just change silencely and only need to make sure the pair of set/reset surface

        } else {
            (*state) = (VT_SRV_SURFACE_STATE) ((static_cast<int>(*state)) & ~VT_SRV_SURFACE_STATE_LOCAL);

            mHelper->logAction("setPreviewSurface", "state (after)", id, mHelper->getSurfaceString((*state)), VT_SRV_LOG_I);

            // The pause and stop operation may come at the same time.
            // We need to add flow lock to guarantee the correct order.
            mMAOperationLock.lock();

            // We Pause MA instead od Stop MA when clear surrface
            // Because it may happen during conference but not call end or dowgrade
            // We want to keep use MA and keep memory of previous operation
            //
            // Now MTK app will not clear surface when conference call.
            // It will clean surface only when call end
            // We keep this to prevent 3rd app still clean surface when conference call
            imsma_pause_resume_params_t updateInfo;
            updateInfo.mode = MA_PAUSE_RESUME_HOLD;
            updateInfo.hold.direction = MA_HOLD_BY_LOCAL;

            // We skip the case MA has stop or hold call and set surface continuoisly
            if (VT_SRV_MA_STATE_INITED < mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_IN != mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_ID != mHelper->getState(id) &&
                VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(id) &&
                VT_TRUE != info->mIsTxInPause &&
                pre_state != (*state)) {
                mLastRet = mHelper->get(id)->Pause(MA_SOURCE, &updateInfo);
            }

            mMAOperationLock.unlock();

            // we don't update state here, because it is not rtp direction change
            // if we modify state, it may cause additional operation when UA notify the direction change
            // so we just change silencely and only need to make sure the pair of set/reset surface
        }

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_RECV_UPDATE, reinterpret_cast<void **>(&isRecvUpdate));
        if (mLastRet) {
            goto error;
        }
        if (pre_state != (*state) && (*isRecvUpdate)) {
            retriggerUpdate(id);
        }

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "setPreviewSurface");
            return mLastRet;
    }

    status_t VTCore::setDisplaySurface(int id, const sp<VTSurface> & surface) {

        VT_SRV_SURFACE_STATE* state;
        VT_SRV_SURFACE_STATE pre_state;
        vt_srv_call_update_info_struct *info;
        VT_BOOL *isRecvUpdate;

        mHelper->logAction("setDisplaySurface", "id", id, "", VT_SRV_LOG_I);

        if (VT_SRV_MA_STATE_PRE_STOP == mHelper->getState(id) ||
            VT_SRV_MA_STATE_STOPED == mHelper->getState(id)) {
            mHelper->logAction("setDisplaySurface", "id", id, "MA has stopped", VT_SRV_LOG_I);
            return VT_SRV_RET_OK;
        }

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_SURFACE_STATE, reinterpret_cast<void **>(&state));
        if (mLastRet) goto error;

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
        if (mLastRet) goto error;

        mHelper->logAction("setDisplaySurface", "state (before)", id, mHelper->getSurfaceString((*state)), VT_SRV_LOG_I);

        pre_state = (*state);

        if (NULL != surface.get()) {

            // The resume and stop operation may come at the same time.
            // We need to add flow lock to guarantee the correct order.
            mMAOperationLock.lock();

            mLastRet = mHelper->get(id)->SetPeerSurface(surface);
            if (mLastRet) {
                mMAOperationLock.unlock();
                goto error;
            }

            (*state) = (VT_SRV_SURFACE_STATE) ((static_cast<int>(*state)) | VT_SRV_SURFACE_STATE_PEER);

            mHelper->logAction("setDisplaySurface", "state (after)", id, mHelper->getSurfaceString((*state)), VT_SRV_LOG_I);

            // We Pause MA instead od Stop MA when clear surrface
            // Because it may happen during conference but not call end or dowgrade
            // We want to keep use MA and keep memory of previous operation
            // so we need to resume it when set surface
            imsma_pause_resume_params_t updateInfo;
            updateInfo.mode = MA_PAUSE_RESUME_HOLD;
            updateInfo.hold.direction = MA_HOLD_BY_LOCAL;
            updateInfo.normal_pause_resume_extra_bitControl = MA_EXT_BIT_CTRL_NONE;

            // we skip the 1st set surface and the case set surface continuoisly
            if (VT_SRV_MA_STATE_INITED < mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_NI != mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_UI != mHelper->getState(id) &&
                VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(id) &&
                VT_TRUE != info->mIsRxInPause &&
                pre_state != (*state)) {
                mLastRet = mHelper->get(id)->Resume(MA_SINK, &updateInfo);
            }

            mMAOperationLock.unlock();

            // we don't update state here, because it is not rtp direction change
            // if we modify state, it may cause additional operation when UA notify the direction change
            // so we just change silencely and only need to make sure the pair of set/reset surface

        } else {
            (*state) = (VT_SRV_SURFACE_STATE) ((static_cast<int>(*state)) & ~VT_SRV_SURFACE_STATE_PEER);

            mHelper->logAction("setDisplaySurface", "state (after)", id, mHelper->getSurfaceString((*state)), VT_SRV_LOG_I);

            // The pause and stop operation may come at the same time.
            // We need to add flow lock to guarantee the correct order.
            mMAOperationLock.lock();

            // We Pause MA instead od Stop MA when clear surrface
            // Because it may happen during conference but not call end or dowgrade
            // We want to keep use MA and keep memory of previous operation
            //
            // Now MTK app will not clear surface when conference call.
            // It will clean surface only when call end
            // We keep this to prevent 3rd app still clean surface when conference call
            imsma_pause_resume_params_t updateInfo;
            updateInfo.mode = MA_PAUSE_RESUME_HOLD;
            updateInfo.hold.direction = MA_HOLD_BY_LOCAL;

            // We skip the case MA has stop or hold call and set surface continuoisly
            if (VT_SRV_MA_STATE_INITED < mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_NI != mHelper->getState(id) &&
                VT_SRV_MA_STATE_START_UI != mHelper->getState(id) &&
                VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(id) &&
                VT_TRUE != info->mIsRxInPause &&
                pre_state != (*state)) {
                mLastRet = mHelper->get(id)->Pause(MA_SINK, &updateInfo);
            }

            mMAOperationLock.unlock();

            // we don't update state here, because it is not rtp direction change
            // if we modify state, it may cause additional operation when UA notify the direction change
            // so we just change silencely and only need to make sure the pair of set/reset surface
        }

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_RECV_UPDATE, reinterpret_cast<void **>(&isRecvUpdate));
        if (mLastRet) {
            goto error;
        }
        if (pre_state != (*state) && (*isRecvUpdate)) {
            retriggerUpdate(id);
        }

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "setDisplaySurface");
            return mLastRet;
    }


    status_t VTCore::setCameraParameters(int sim_id, int sensorCnt, sensor_info_vilte_t *sensor) {
        mHelper->logAction("setCameraParameters", "id", sim_id, "", VT_SRV_LOG_I);
        VT_LOGI("sim_id:%d, sensorCnt:%d", sim_id, sensorCnt);

        for(int i = 0; i < sensorCnt; i++) {
            VT_LOGI("sensor[%d]: id=%d, width=%d, height=%d, degree=%d, facing=%d, hal=%d",
                i, sensor[i].index, sensor[i].max_width, sensor[i].max_height, sensor[i].degree, sensor[i].facing, sensor[i].hal);
        }

        ImsMa::setSensorParameters(sensor, sensorCnt);
        mHelper->setIsSetSensorInfo(VT_TRUE);

        VT_IMCB_CAP ua;
        memset(&ua, 0, sizeof(VT_IMCB_CAP));

        if (0 == sensorCnt) {
            // If cannot get camera cap, just response empty ua cap
            ua.sim_slot_id = sim_id;
        } else {
            int opId = mHelper->getOperatorId(sim_id);
            ua.operator_code = opId;

            int labOp = mHelper->getLabOperator();
            if (0 != labOp) {
                opId = labOp;
            }

            vt_rtp_codec_2_ua(VT_SRV_CALL_4G, &ua, sim_id, opId);
        }

        // ================================================================
        // send MSG to UA via IPC
        // ================================================================
        int ret = VT_Send(
                    VT_SRV_CALL_4G,
                    MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_RSP,
                    reinterpret_cast<void *>(&ua),
                    sizeof(VT_IMCB_CAP));
        if (ret) goto error;

        return VT_SRV_RET_OK;

        error:
            mHelper->logAction("setCameraParameters", "id", sim_id, "send fail", VT_SRV_LOG_E);
            return mLastRet;
    }

    status_t VTCore::setCameraParametersWithSim(int sim_id, int major_sim_id, int sensorCnt, sensor_info_vilte_t *sensor) {
        mHelper->logAction("setCameraParametersWithSim", "id", sim_id, "", VT_SRV_LOG_I);
        VT_LOGI("sim_id:%d, major_sim_id:%d, sensorCnt:%d", sim_id, major_sim_id, sensorCnt);

        for(int i = 0; i < sensorCnt; i++) {
            VT_LOGI("sensor[%d]: id=%d, width=%d, height=%d, degree=%d, facing=%d, hal=%d",
                i, sensor[i].index, sensor[i].max_width, sensor[i].max_height, sensor[i].degree, sensor[i].facing, sensor[i].hal);
        }

        ImsMa::setSensorParameters(sensor, sensorCnt);
        mHelper->setIsSetSensorInfo(VT_TRUE);

        VT_IMCB_CAP ua;
        memset(&ua, 0, sizeof(VT_IMCB_CAP));

        if (0 == sensorCnt) {
            // If cannot get camera cap, just response empty ua cap
            ua.sim_slot_id = sim_id;
        } else {
            int opId = mHelper->getCurrentOperator(major_sim_id);
            mHelper->setOperatorId(major_sim_id, opId);

            vt_rtp_codec_2_ua(VT_SRV_CALL_4G, &ua, sim_id, opId);
        }

        // ================================================================
        // send MSG to UA via IPC
        // ================================================================
        int ret = VT_Send(
                    VT_SRV_CALL_4G,
                    MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_RSP,
                    reinterpret_cast<void *>(&ua),
                    sizeof(VT_IMCB_CAP));
        if (ret) goto error;

        return VT_SRV_RET_OK;

        error:
            mHelper->logAction("setCameraParametersWithSim", "id", sim_id, "send fail", VT_SRV_LOG_E);
            return mLastRet;
    }

    status_t VTCore::setCameraParametersOnly(int sensorCnt, sensor_info_vilte_t *sensor) {
        mHelper->logAction("setCameraParametersToMA", "id", VT_IVD, "", VT_SRV_LOG_I);
        VT_LOGI("sensorCnt:%d", sensorCnt);

        for(int i = 0; i < sensorCnt; i++) {
            VT_LOGI("sensor[%d]: id=%d, width=%d, height=%d, degree=%d, facing=%d, hal=%d",
                i, sensor[i].index, sensor[i].max_width, sensor[i].max_height, sensor[i].degree, sensor[i].facing, sensor[i].hal);
        }

        ImsMa::setSensorParameters(sensor, sensorCnt);
        mHelper->setIsSetSensorInfo(VT_TRUE);

        return VT_SRV_RET_OK;
    }

    status_t VTCore::setDeviceOrientation(int id, int rotation) {
        mHelper->logAction("setDeviceOrientation", "id", id, "", VT_SRV_LOG_I);

        // we need check call state, if target call going to stop flow,
        // should not set device orientation.
        if (VT_SRV_MA_STATE_PRE_STOP == mHelper->getState(id) ||
            VT_SRV_MA_STATE_STOPED == mHelper->getState(id)) {
            mHelper->logAction("setDeviceOrientation", "id", id, "MA has stopped", VT_SRV_LOG_I);
            return VT_SRV_RET_OK;
        }

        mLastRet = mHelper->get(id)->setDeviceRotationDegree(rotation);
        if (mLastRet) goto error;

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "setDeviceOrientation");
            return mLastRet;
    }

    status_t VTCore::setUIMode(int id, VT_SRV_UI_MODE mode) {

        mHelper->logAction("setUIMode", "id", id, mHelper->getUIModeString(mode), VT_SRV_LOG_I);

        if (mode == VT_SRV_UI_MODE_FG) {

            mHelper->get(id)->setUIMode(1);

            vt_srv_call_ui_config_struct *config;
            mLastRet = mHelper->getParam(id, VT_SRV_PARAM_PEER_UI_CONFIG, reinterpret_cast<void **>(&config));
            if (mLastRet) goto error;

            if (config->mWidth == 0 || config->mHeight == 0) {
                mHelper->logAction("setUIMode", "id", id, "MA not notify yet, ignore", VT_SRV_LOG_W);
            } else {
                notifyCallback(
                        id,
                        VT_SRV_NOTIFY_PEER_SIZE_CHANGED,
                        config->mWidth,
                        config->mHeight,
                        config->mRotation,
                        String8(""),
                        String8(""),
                        NULL);
            }
        } else if (mode == VT_SRV_UI_MODE_BG) {
            mHelper->get(id)->setUIMode(0);

        } else if(mode == VT_SRV_UI_MODE_FULL_SCREEN) {
            ImsMa::setLowPowerMode(VT_TRUE);

        } else if(mode == VT_SRV_UI_MODE_NORMAL_SCREEN) {
            ImsMa::setLowPowerMode(VT_FALSE);
        }

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "setUIMode");
            return mLastRet;
    }

    status_t VTCore::requestCancelSessionModify(int id, sp<VideoProfile> Vp) {
        VT_IMCB_CANCEL_REQ request;
        memset(&request, 0, sizeof(request));

        mHelper->logAction("requestCancelSessionModify", "id", id, "", VT_SRV_LOG_I);

        int cause = (Vp->getState() == VT_SRV_VIDEO_STATE_CANCEL)? 0:1;

        request.call_id = GET_CALL_ID(id);
        request.sim_slot_id = GET_SIM_ID(id);
        request.cause = cause;

        mLastRet = mHelper->setParam(id, VT_SRV_PARAM_CANCEL_SESSION_REQ, reinterpret_cast<void *>(&request));
        if (mLastRet) goto error;

        mLastRet = VT_Send(
                        VT_SRV_CALL_4G,
                        MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_REQ,
                        reinterpret_cast<void *>(&request),
                        sizeof(VT_IMCB_CANCEL_REQ));

        if (mLastRet) goto error;

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "requestCancelSessionModify");
            return mLastRet;
    }

    status_t VTCore::requestSessionModify(int id, const String8 &  config) {
        mHelper->logAction("requestSessionModify", "id", id, "", VT_SRV_LOG_I);

        bool *isForceStop;
        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_STOP, reinterpret_cast<void **>(&isForceStop));
        if ((VT_SRV_RET_OK == mLastRet) && (*isForceStop) == VT_TRUE) {
            mHelper->logAction("requestSessionModify", "id", id, "isForceStopd, ignore session modify", VT_SRV_LOG_W);
            return VT_SRV_RET_OK;
        }

        sp<VideoProfile> Vp = mHelper->unPackToVdoProfile(config);
        if (mHelper->isCancelRequest(Vp)) {
            return requestCancelSessionModify(id, Vp);
        }

        mHelper->logAction("requestSessionModify", "id", id, mHelper->getVideoStateString(Vp->getState()), VT_SRV_LOG_I);

        VT_BOOL *isForceCancel;
        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_FORCE_CANCEL, reinterpret_cast<void **>(&isForceCancel));
        if ((VT_SRV_RET_OK == mLastRet) && (*isForceCancel) == VT_TRUE) {
            mHelper->logFlow("requestSessionModify", "clear force cancel!", VT_IVD, "", VT_SRV_LOG_I);
            *isForceCancel = VT_FALSE;
        }

        VT_IMCB_REQ request;
        memset(&request, 0, sizeof(VT_IMCB_REQ));

        mHelper->getReqFromProfile(&request, Vp, id);

        mLastRet = mHelper->setRequestVdoProfile(id, Vp);
        if (mLastRet) goto error;

        mLastRet = mHelper->setParam(id, VT_SRV_PARAM_SESSION_REQ, reinterpret_cast<void *>(&request));
        if (mLastRet) goto error;

        mLastRet = VT_Send(
                        VT_SRV_CALL_4G,
                        MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_REQ,
                        reinterpret_cast<void *>(&request),
                        sizeof(VT_IMCB_REQ));
        if (mLastRet) goto error;

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "requestSessionModify");
            return mLastRet;
    }

    status_t VTCore::responseSessionModify(int id, const String8 &  config) {
        mHelper->logAction("responseSessionModify", "id", id, "", VT_SRV_LOG_I);

        sp<VideoProfile> Vp = mHelper->unPackToVdoProfile(config);

        mHelper->logAction("responseSessionModify", "id", id, mHelper->getVideoStateString(Vp->getState()), VT_SRV_LOG_I);

        VT_IMCB_RSP response;
        memset(&response, 0, sizeof(VT_IMCB_RSP));

        mHelper->getRspFromProfile(&response, Vp, id);

        mLastRet = VT_Send(
                        VT_SRV_CALL_4G,
                        MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_RSP,
                        reinterpret_cast<void *>(&response),
                        sizeof(VT_IMCB_RSP));
        if (mLastRet) goto error;

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "responseSessionModify");
            return mLastRet;
    }

    status_t VTCore::snapshot(int id, VT_SRV_SNAPSHOT_TYPE type, String8 savingImgURI) {

        return VT_SRV_RET_OK;
    }

    status_t VTCore::startRecording(int id, VT_SRV_RECORD_TYPE type, String8 path, int maxSize) {

        return VT_SRV_RET_OK;
    }

    status_t VTCore::stopRecording(int id) {

        return VT_SRV_RET_OK;
    }

    status_t VTCore::retriggerUpdate(int id) {
        Mutex::Autolock malLock(*getCallFlowLock(id));

        VT_IMCB_CONFIG * ua;
        vt_srv_call_update_info_struct *info;

        mHelper->logFlow("retriggerUpdate", "id", id, "start", VT_SRV_LOG_I);

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
        if (mLastRet) goto error;


        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
        if (mLastRet) goto error;

        mHelper->logFlow("retriggerUpdate", "ua->setting.mode", id, mHelper->getCallModeString(ua->setting.mode), VT_SRV_LOG_I);
        mHelper->logFlow("retriggerUpdate", "info->mIsHold", id, mHelper->getOnOffString(info->mIsHold), VT_SRV_LOG_I);
        mHelper->logFlow("retriggerUpdate", "info->mIsHeld", id, mHelper->getOnOffString(info->mIsHeld), VT_SRV_LOG_I);
        mHelper->logFlow("retriggerUpdate", "info->mIsResume", id, mHelper->getOnOffString(info->mIsResume), VT_SRV_LOG_I);
        mHelper->logFlow("retriggerUpdate", "info->mIsResumed", id, mHelper->getOnOffString(info->mIsResumed), VT_SRV_LOG_I);
        mHelper->logFlow("retriggerUpdate", "info->mIsTxInPause", id, mHelper->getOnOffString(info->mIsTxInPause), VT_SRV_LOG_I);
        mHelper->logFlow("retriggerUpdate", "info->mIsRxInPause", id, mHelper->getOnOffString(info->mIsRxInPause), VT_SRV_LOG_I);

        if (ua->setting.mode != 0) {

            mLastRet = updateTxRxMode(id, ua->config.rtp_direction, info);
            if (mLastRet) goto error;
        }

        mHelper->logFlow("updateCallMode", "id", id, "finish", VT_SRV_LOG_I);

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "updateCallMode");
            return mLastRet;
    }

    status_t VTCore::switchFeature(int id, int feature, int isOn) {
        VT_IMCB_CONFIG * ua;

        mHelper->logAction("switchFeature", "id", id, "", VT_SRV_LOG_I);
        mHelper->logAction("switchFeature", "isOn", isOn, "", VT_SRV_LOG_I);

        //Do nothing when VoLTE
        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
        if (mLastRet) goto error;
        if (ua->setting.mode == 0) {
            return VT_SRV_RET_OK;
        }

        // LTE case
        if (TAG_VILTE_MOBILE == feature) {

            if (isOn) {

                // do nothing

            } else {

               // now we do nothing here
            }

        // WiFi case
        } else if (TAG_VILTE_WIFI== feature) {

            if (isOn) {

                // do nothing

            } else {

                // do nothing
            }
        }

        return VT_SRV_RET_OK;

        error:
            notifyError(id, "switchFeature");
            return mLastRet;
    }

    status_t VTCore::updateNetworkTable(bool is_add, int network_id, String8 if_name) {

        if (is_add) {
            VT_LOGI("[SRV] [OPERATION][NETWORK_ID=%d] Add if_name:%s", network_id, (char *)if_name.string());
            mHelper->addNetworkInfoRecord(network_id, if_name);

        } else {
            VT_LOGI("[SRV] [OPERATION][NETWORK_ID=%d] Remove", network_id);
            mHelper->removeNetworkInfoRecord(network_id);
        }

        return VT_SRV_RET_OK;
    }

    void VTCore::notifyError(int id, const char* action) {

        if (mLastRet == VT_SRV_RET_ERR_MA || mLastRet == VT_SRV_RET_ERR_BIND_PORT) {

            if (mLastRet == VT_SRV_RET_ERR_MA) {
                mLastNotify = VT_SRV_ERROR_CAMERA_CRASHED;
            } else if(mLastRet == VT_SRV_RET_ERR_BIND_PORT) {
                mLastNotify = VT_SRV_ERROR_BIND_PORT;
            }

            if (VT_SRV_MA_STATE_STOPED != mHelper->getState(id) &&
                    VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(id)) {

                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_PRE_STOP);

                mHelper->get(id)->Stop(MA_SOURCE_SINK);

                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_STOP_REC_AND_PREVIEW,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                mHelper->get(id)->Reset(MA_SOURCE_SINK);

                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_STOPED);

                // if still blocked at update
                // use this flag to break out to prevent from dead lock
                bool *isMaCrash;
                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_MA_CRASH, reinterpret_cast<void **>(&isMaCrash));
                if (mLastRet) {
                    mHelper->logFlow("close", "id", id, "this call MA has crash", VT_SRV_LOG_E);
                }
                (*isMaCrash) = VT_TRUE;

                mHelper->setMode(id, VT_SRV_CALL_NONE);
            }

        } else if (mLastRet == VT_SRV_RET_ERR_NOT_EXIST) {
             // The case that VTS call end and cannot get call record.
             // VTP will do finalize by receiving call end from IMS FWK.
             // So VTS don't need to notify VTP to do more handing.
            mHelper->logFlow(action, "id", id, mHelper->getErrorString(mLastRet), VT_SRV_LOG_E);
            return;

        } else {
            mLastNotify = VT_SRV_ERROR_SERVICE;
        }

        mHelper->logFlow(action, "id", id, mHelper->getErrorString(mLastRet), VT_SRV_LOG_E);
        notifyCallback(id, mLastNotify);
    }

    void VTCore::notifyCallback(int32_t id, int32_t msgType) {
        notifyCallback(
                id,
                msgType,
                0,
                0,
                0,
                String8(""),
                String8(""),
                NULL);
    }

    void VTCore::notifyCallback(
            int32_t id,
            int32_t msgType,
            int32_t arg1,
            int32_t arg2,
            int32_t arg3,
            const String8 & obj1,
            const String8 & obj2,
            const sp<IGraphicBufferProducer> & obj3) {

        if (msgType > VT_SRV_ERROR_BASE) {
            Mutex::Autolock ntyLock(mNotifyErrorLock);
        } else {
            Mutex::Autolock ntyLock(mNotifyLock);
        }

        mHelper->logMsg(
                    msgType,
                    "",
                    id,
                    arg1,
                    arg2,
                    arg3,
                    obj1.string(),
                    obj2.string(),
                    VT_SRV_LOG_I);

        if (msgType == VT_SRV_NOTIFY_AVPF_TMMBR_MBR_DL) {
            VT_IMCB_BW *BW = &g_vt.ma_bw;

            BW->call_id         = GET_CALL_ID(id);
            BW->sim_slot_id     = GET_SIM_ID(id);
            BW->video_bandwidth = arg1;

            VT_Send(
                    VT_SRV_CALL_4G,
                    MSG_ID_WRAP_IMSVT_IMCB_MOD_BW_REQ,
                    reinterpret_cast<void *>(&g_vt.ma_bw),
                    sizeof(VT_IMCB_BW));
            return;

        } else if (msgType == VT_SRV_ERROR_CODEC ||
                   msgType == VT_SRV_ERROR_CAMERA_CRASHED ||
                   msgType == VT_SRV_ERROR_BIND_PORT) {

            if (VT_SRV_MA_STATE_STOPED != mHelper->getState(id) &&
                    VT_SRV_MA_STATE_PRE_STOP != mHelper->getState(id)) {

                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_PRE_STOP);

                mHelper->get(id)->Stop(MA_SOURCE_SINK);

                notifyCallback(
                    id,
                    VT_SRV_NOTIFY_UPLINK_STATE_CHANGE,
                    VT_SRV_VTCAM_STATE_STOP_REC_AND_PREVIEW,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);

                mHelper->get(id)->Reset(MA_SOURCE_SINK);

                mLastRet = mHelper->setState(id, VT_SRV_MA_STATE_STOPED);

                // if still blocked at update
                // use this flag to break out to prevent from dead lock
                bool *isMaCrash;
                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_IS_MA_CRASH, reinterpret_cast<void **>(&isMaCrash));
                if (mLastRet) {
                    mHelper->logFlow("close", "id", id, "this call MA has crash", VT_SRV_LOG_E);
                }
                (*isMaCrash) = VT_TRUE;

                mHelper->setMode(id, VT_SRV_CALL_NONE);
                mLastNotify = msgType;
            }
        } else if (msgType == VT_SRV_NOTIFY_SET_ANBR) {

            VT_ANBR_REQ ANBR_REQ;
            VT_IMCB_CONFIG * ua;

            ANBR_REQ.call_id = GET_CALL_ID(id);
            ANBR_REQ.sim_slot_id = GET_SIM_ID(id);

            ANBR_REQ.anbr_config.is_ul = arg1;
            ANBR_REQ.anbr_config.ebi = arg2;
            ANBR_REQ.anbr_config.bitrate = arg3;

            //Do nothing when VoLTE
            if (ANBR_REQ.anbr_config.ebi == 0xff) {
                //invalid ebi, means NR network, get valid bearer id and pdu session id
                mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void **>(&ua));
                ANBR_REQ.anbr_config.bearer_id = ua->setting.bearer_id;
                ANBR_REQ.anbr_config.pdu_session_id = ua->setting.pdu_session_id;
            } else {
                //ebi valid, LTE netwokr, set bearer_id and pdu_session_id to invalid
                ANBR_REQ.anbr_config.bearer_id = 0xff;
                ANBR_REQ.anbr_config.pdu_session_id = 0xff;
            }

            ANBR_REQ.anbr_config.ext_param = 0; //for future usage

            VT_Send(
                    VT_SRV_CALL_4G,
                    MSG_ID_MD_ANBR_REPORT_REQ,
                    reinterpret_cast<void *>(&ANBR_REQ),
                    sizeof(VT_ANBR_REQ));
            return;
        }

        if (mUser == 0) {
            mHelper->logAction("notifyCallback", "id", id, "User is NULL", VT_SRV_LOG_E);
            return;
        }

        mUser->notifyCallback(id, msgType, arg1, arg2, arg3, obj1, obj2, obj3);
        return;
    }

    void VTCore::notifyCallback(
       int id,
       int msg,
       int arg1,
       int arg2) {

        notifyCallback(id, msg, arg1, arg2, 0, String8(""), String8(""), NULL);
    }

    void VTCore::resetUpdateInfo(int id) {
            return;
    }

    void VTCore::getUpdateInfo(int id, VT_IMCB_UPD* pre_config, VT_IMCB_UPD* new_config) {
        Mutex::Autolock malLock(*getCallFlowLock(id));

        mHelper->logAction("getUpdateInfo", "id", id, "start", VT_SRV_LOG_I);

        vt_srv_call_update_info_struct *info;

        if (VT_SRV_MA_STATE_STOPED == mHelper->getState(id) ||
            VT_SRV_MA_STATE_PRE_STOP == mHelper->getState(id)) {
            mHelper->logAction("getUpdateInfo", "id", id, "MA has stopped", VT_SRV_LOG_I);
            return;
        }

        mLastRet = mHelper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
        if (mLastRet) goto error;

        mHelper->logAction("getUpdateInfo", "old bHold", id, mHelper->getOnOffString(pre_config->config.bHold), VT_SRV_LOG_I);
        mHelper->logAction("getUpdateInfo", "new bHold", id, mHelper->getOnOffString(new_config->config.bHold), VT_SRV_LOG_I);
        mHelper->logAction("getUpdateInfo", "old bHeld", id, mHelper->getOnOffString(pre_config->config.bHeld), VT_SRV_LOG_I);
        mHelper->logAction("getUpdateInfo", "new bHeld", id, mHelper->getOnOffString(new_config->config.bHeld), VT_SRV_LOG_I);

        // set default value or it may be change on one condition and reset on the other case
        info->mIsHold = VT_FALSE;
        info->mIsResume = VT_FALSE;

        // ===================================================================
        // check local hold Call
        if (!pre_config->config.bHold && new_config->config.bHold) {
            info->mIsHold = VT_TRUE;
        }

        if(strcmp(pre_config->config.remote_rtp_address, "0.0.0.0") &&
           !strcmp(new_config->config.remote_rtp_address, "0.0.0.0")) {
            info->mIsHold = VT_TRUE;
        }
        // ===================================================================

        // ===================================================================
        // check local resume Call
        if (pre_config->config.bHold && !new_config->config.bHold) {
            info->mIsResume = VT_TRUE;
        }

        if(!strcmp(pre_config->config.remote_rtp_address, "0.0.0.0") &&
           strcmp(new_config->config.remote_rtp_address, "0.0.0.0")) {
            info->mIsResume = VT_TRUE;
        }
        // ===================================================================

        // ===================================================================
        // check remote hold Call
        if (!pre_config->config.bHeld && new_config->config.bHeld) {
            info->mIsHeld = VT_TRUE;
        } else {
            info->mIsHeld = VT_FALSE;
        }
        // ===================================================================

        // ===================================================================
        // check remote resume Call
        if (pre_config->config.bHeld && !new_config->config.bHeld) {
            info->mIsResumed = VT_TRUE;
        } else {
            info->mIsResumed = VT_FALSE;
        }
        // ===================================================================

        mLastRet = mHelper->setParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void *>(info));
        if (mLastRet) goto error;

        mHelper->logAction("getUpdateInfo", "id", id, "finish", VT_SRV_LOG_E);

        return;

        error:
            notifyError(id, "getUpdateInfo");
            return;
    }

    void vt_callback(int type, void *data, int len) {
        sp<VTCore> core = g_vt.core;
        sp<VTCoreHelper> helper = VTCore::mHelper;
        int id = -1;
        int opid = 0;
        int ret = VT_SRV_RET_OK;
        int notify_type = VT_SRV_ERROR_SERVICE;

        // get cap case
        if (type == MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND) {

            VT_IMCB_CAPIND CapInd;

            memset(&CapInd, 0, sizeof(VT_IMCB_CAPIND));
            memcpy(&CapInd, reinterpret_cast<VT_IMCB_CAPIND *>(data), len);

            id = CapInd.sim_slot_id;
            opid = CapInd.operator_code;

            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND", VT_SRV_LOG_W);
            VT_LOGW("[SRV] [OPERATION][ID=%d] vt_callback (MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND) opid = %d", id, opid);

            if (opid > 0) {
                helper->setOperatorId(id, opid);

                core->notifyCallback(
                    id,
                    VT_SRV_NOTIFY_GET_CAP,
                    0,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);
            } else {

                core->notifyCallback(
                    id,
                    VT_SRV_NOTIFY_GET_CAP_WITH_SIM,
                    0, //Default get camera 0
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);
           }

        // init config case
        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND) {

            media_config_t ma;
            VT_IMCB_INIT   ua;

            memset(&ma, 0, sizeof(media_config_t));
            memset(&ua, 0, sizeof(VT_IMCB_INIT));

            // ================================================================
            // copy to ua config
            // ================================================================
            memcpy(&ua, reinterpret_cast<VT_IMCB_INIT *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(ua.setting.sim_slot_id, ua.config.call_id);
            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND", VT_SRV_LOG_W);

            // ================================================================
            // Wait for CC FWK create provider
            // or this call will fail
            // ================================================================
            int count = 0;
            while (count < 100 && helper->check(id) != VT_SRV_RET_OK) {
                helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND", id, "this id is not init yet, wait for CC FWK", VT_SRV_LOG_E);
                usleep(200 * 1000);
                count++;
            }

            // ================================================================
            // copy ua config to ma config
            // ================================================================
            vt_ua_2_rtp(VT_SRV_CALL_4G, &ua, &(ma.rtp_rtcp_cap));
            vt_ua_2_codec(VT_SRV_CALL_4G, &ua, &(ma.codec_param));

            // store non-zero value for close
            // Both rtp/rtcp port will be 0 when voice call, not be 0 when ViLTE call
            if (0 != ua.config.local_rtp_port && 0 != ua.config.local_rtcp_port) {
                vt_srv_call_update_info_struct *info;

                ret = helper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
                if (ret) goto error;

                info->mVdoRtpPort = ua.config.local_rtp_port;
                info->mVdoRtcpPort = ua.config.local_rtcp_port;

                ret = helper->setParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void *>(info));
                if (ret) goto error;
            }

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND, mode = ", id, helper->getCallModeString(ua.setting.mode), VT_SRV_LOG_W);

            // ================================================================
            // update ua config to table
            // ================================================================
            ret = helper->setParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void *>(&ua));
            if (ret) goto error;

            // ================================================================
            // update ma config to table
            // ================================================================
            ret = helper->setParam(id, VT_SRV_PARAM_MA_CONFIG, reinterpret_cast<void *>(&ma));
            if (ret) goto error;

            core->init(id);

        // update config case
        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND) {

            media_config_t ma;
            VT_IMCB_UPD ua;
            VT_IMCB_UPD *pre_ua;
            VT_BOOL isRecvUpdate = VT_TRUE;

            memset(&ma, 0, sizeof(media_config_t));
            memset(&ua, 0, sizeof(VT_IMCB_UPD));

            // ================================================================
            // copy to ua config
            // ================================================================
            memcpy(&ua, reinterpret_cast<VT_IMCB_UPD *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(ua.setting.sim_slot_id, ua.config.call_id);
            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND", VT_SRV_LOG_W);

            bool *isForceStop;
            ret = helper->getParam(id, VT_SRV_PARAM_IS_FORCE_STOP, reinterpret_cast<void **>(&isForceStop));

            if (ret) goto error;

            if ((*isForceStop) == VT_TRUE) {
                helper->logAction("vt_callback", "id", id, "(MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND : isForceStopd !)", VT_SRV_LOG_W);
                return;
            }

            bool *isMaCrash;
            ret = helper->getParam(id, VT_SRV_PARAM_IS_MA_CRASH, reinterpret_cast<void **>(&isMaCrash));
            if (ret) goto error;

            if ((*isMaCrash) == VT_TRUE) {
                helper->logAction("vt_callback", "id", id, "(MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND : isMaCrash !)", VT_SRV_LOG_W);
                return;
            }

            // for early media case, it may start only one way
            // it should be always send_recv after call is connected really.
            helper->logAction("vt_callback", "rtp_direction (before)", id, helper->getVideoStateString(ua.config.rtp_direction), VT_SRV_LOG_W);
            helper->logAction("vt_callback", "early_media_direction", id, helper->getVideoStateString(ua.setting.early_media_direction), VT_SRV_LOG_W);

            VT_BOOL *isDuringEarlyMedia;
            helper->getParam(id, VT_SRV_PARAM_IS_DURING_EARLY_MEDIA, reinterpret_cast<void **>(&isDuringEarlyMedia));

            if (!helper->isCallConnected(id) && ua.setting.early_media_direction != 0 && ua.config.rtp_direction != 0) {
                helper->logAction("vt_callback", "id", id, "notify virtual early media enter", VT_SRV_LOG_W);

                //check if isDuringEarlyMedia is already set ture, if true, don't send again
                if ((*isDuringEarlyMedia) == VT_FALSE) {

                    (*isDuringEarlyMedia) = VT_TRUE;

                    sp<VideoProfile> Vp = helper->geteEarlyMediaProfileFromState(ua.config.rtp_direction);
                    core->notifyCallback(
                        id,
                        VT_SRV_NOTIFY_RECV_ENHANCE_SESSION_IND,
                        VT_SRV_SESSION_IND_EARLY_MEDIA,
                        1, //early media start
                        0,
                        helper->packFromVdoProfile(Vp),
                        String8(""),
                        NULL);
                }

            // If call connected or early media direction is audio only
            // set during early media as false
            } else if ((*isDuringEarlyMedia) == VT_TRUE) {
                helper->logAction("vt_callback", "id", id, "notify virtual early media leave", VT_SRV_LOG_W);

                (*isDuringEarlyMedia) = VT_FALSE;

                core->notifyCallback(
                    id,
                    VT_SRV_NOTIFY_RECV_ENHANCE_SESSION_IND,
                    VT_SRV_SESSION_IND_EARLY_MEDIA,
                    0, //early media stop
                    0,
                    String8(""),
                    String8(""),
                    NULL);
            }

            ua.config.rtp_direction = ua.config.rtp_direction & ua.setting.early_media_direction;
            helper->logAction("vt_callback", "rtp_direction (after)", id, helper->getVideoStateString(ua.config.rtp_direction), VT_SRV_LOG_W);

            // ================================================================
            // copy ua config to ma config
            // ================================================================
            vt_ua_2_rtp(VT_SRV_CALL_4G, &ua, &(ma.rtp_rtcp_cap));
            vt_ua_2_codec(VT_SRV_CALL_4G, &ua, &(ma.codec_param));

            // store non-zero value for close
            // Both rtp/rtcp port will be 0 when voice call, not be 0 when ViLTE call
            if (0 != ua.config.local_rtp_port && 0 != ua.config.local_rtcp_port) {
                vt_srv_call_update_info_struct *info;

                ret = helper->getParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void **>(&info));
                if (ret) goto error;

                info->mVdoRtpPort = ua.config.local_rtp_port;
                info->mVdoRtcpPort = ua.config.local_rtcp_port;

                ret = helper->setParam(id, VT_SRV_PARAM_UPDATE_INFO, reinterpret_cast<void *>(info));
                if (ret) goto error;
            }

            // ================================================================
            // compare with old config to know what's difference
            // ================================================================
            ret  = helper->getParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void**>(&pre_ua));

            core->getUpdateInfo(id, pre_ua, &ua);

            // ================================================================
            // update ua config to table
            // ================================================================
            ret = helper->setParam(id, VT_SRV_PARAM_UA_CONFIG, reinterpret_cast<void *>(&ua));
            if (ret) goto error;

            // ================================================================
            // update ma config to table
            // ================================================================
            ret = helper->setParam(id, VT_SRV_PARAM_MA_CONFIG, reinterpret_cast<void *>(&ma));
            if (ret) goto error;

            ret = helper->setParam(id, VT_SRV_PARAM_IS_RECV_UPDATE, reinterpret_cast<void *>(&isRecvUpdate));
            if (ret) goto error;

            core->update(id);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND) {

            VT_IMCB_DEINIT ua;

            memset(&ua, 0, sizeof(VT_IMCB_DEINIT));

            // ================================================================
            // copy to ua config
            // ================================================================
            memcpy(&ua, reinterpret_cast<VT_IMCB_DEINIT *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(ua.setting.sim_slot_id, ua.config.call_id);
            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND", VT_SRV_LOG_W);

            core->deinit(id);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND) {

            VT_IMCB_IND Indication;

            memset(&Indication, 0, sizeof(VT_IMCB_IND));
            memcpy(&Indication, reinterpret_cast<VT_IMCB_IND *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(Indication.sim_slot_id, Indication.call_id);
            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND", VT_SRV_LOG_W);

            VT_BOOL *isForceCancel;
            int ret = helper->getParam(id, VT_SRV_PARAM_IS_FORCE_CANCEL, reinterpret_cast<void **>(&isForceCancel));
            if ((VT_SRV_RET_OK == ret) && (*isForceCancel) == VT_TRUE) {
                helper->logFlow("MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND", "clear force cancel!", VT_IVD, "", VT_SRV_LOG_I);
                *isForceCancel = VT_FALSE;
            }

            sp<VideoProfile> Vp = helper->getProfileFromInd(&Indication);

            helper->logAction("vt_callback", "Vp->getState", id, helper->getVideoStateString(Vp->getState()), VT_SRV_LOG_W);

            ret = helper->setParam(id, VT_SRV_PARAM_REMOTE_SESSION_REQ, reinterpret_cast<void *>(&Indication));
            if (ret) goto error;

            core->notifyCallback(
                id,
                VT_SRV_NOTIFY_RECV_SESSION_CONFIG_REQ,
                0,
                0,
                0,
                helper->packFromVdoProfile(Vp),
                String8(""),
                NULL);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF) {

            VT_IMCB_CNF confrim;

            memset(&confrim, 0, sizeof(VT_IMCB_CNF));
            memcpy(&confrim, reinterpret_cast<VT_IMCB_CNF *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(confrim.sim_slot_id, confrim.call_id);
            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF", VT_SRV_LOG_W);

            sp<VideoProfile> spVp_confirm = helper->getProfileFromCnf(&confrim);

            helper->logAction("vt_callback", "spVp_confirm->getState()", id, helper->getVideoStateString(spVp_confirm->getState()), VT_SRV_LOG_W);
            helper->logAction("vt_callback", "spVp_confirm->getResult()", id, helper->getSessionModifyResultString(spVp_confirm->getResult()), VT_SRV_LOG_W);

            sp<VideoProfile> spVp_request = helper->getRequestVdoProfile(id);

            helper->logAction("vt_callback", "spVp_request->getState", id, helper->getVideoStateString(spVp_request->getState()), VT_SRV_LOG_W);

            if (VT_SRV_IMCB_SESSION_MODIFY_OK == spVp_confirm->getResult()) {
                spVp_confirm = spVp_request;
                helper->logAction("vt_callback", "spVp_confirm->getState()", id, helper->getVideoStateString(spVp_confirm->getState()), VT_SRV_LOG_W);
                helper->logAction("vt_callback", "spVp_confirm->getResult()", id, helper->getSessionModifyResultString(spVp_confirm->getResult()), VT_SRV_LOG_W);
            }

            core->notifyCallback(
                id,
                VT_SRV_NOTIFY_RECV_SESSION_CONFIG_RSP,
                spVp_confirm->getResult(),
                0,
                0,
                helper->packFromVdoProfile(spVp_request),
                helper->packFromVdoProfile(spVp_confirm),
                NULL);

            spVp_request = NULL;
            spVp_confirm = NULL;

        }  else if (type == MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_START_IND) {

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_START_IND", VT_IVD, "", VT_SRV_LOG_W);

            VT_IMCB_HOSTART hostStart;

            memset(&hostStart, 0, sizeof(VT_IMCB_HOSTART));
            memcpy(&hostStart, reinterpret_cast<VT_IMCB_HOSTART *>(data), len);

            core->setHandoverState(VT_TRUE);

        }  else if (type == MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_STOP_IND) {

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_STOP_IND", VT_IVD, "", VT_SRV_LOG_W);

            VT_IMCB_HOSTOP hostStop;

            memset(&hostStop, 0, sizeof(VT_IMCB_HOSTOP));
            memcpy(&hostStop, reinterpret_cast<VT_IMCB_HOSTOP *>(data), len);

            core->setHandoverState(VT_FALSE);

        }  else if (type == MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_BEGIN_IND) {

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_BEGIN_IND", VT_IVD, "", VT_SRV_LOG_W);

            VT_IMCB_PDN_HOBEGIN hostBegin;

            memset(&hostBegin, 0, sizeof(VT_IMCB_PDN_HOBEGIN));
            memcpy(&hostBegin, reinterpret_cast<VT_IMCB_PDN_HOBEGIN *>(data), len);

            core->setHandoverStateByNetworkId(hostBegin.network_id , VT_TRUE);

        }  else if (type == MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_END_IND) {

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_END_IND", VT_IVD, "", VT_SRV_LOG_W);

            VT_IMCB_PDN_HOEND hostStop;

            memset(&hostStop, 0, sizeof(VT_IMCB_HOSTOP));
            memcpy(&hostStop, reinterpret_cast<VT_IMCB_HOSTOP *>(data), len);

            core->setHandoverStateByNetworkId(hostStop.network_id , VT_FALSE);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND) {

            VT_IMCB_CANCEL_IND ind;

            memset(&ind, 0, sizeof(VT_IMCB_CANCEL_IND));
            memcpy(&ind, reinterpret_cast<VT_IMCB_CANCEL_IND *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(ind.sim_slot_id, ind.call_id);

            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_MODIFY_CANCEL_IND", VT_SRV_LOG_W);

            sp<VideoProfile> Vp = helper->getProfileFromCancelInd(&ind);

            helper->logFlow("Cancel", "id", id, "", VT_SRV_LOG_I);

            VT_BOOL *mIsForceCancel;
            ret = helper->getParam(id, VT_SRV_PARAM_IS_FORCE_CANCEL, reinterpret_cast<void **>(&mIsForceCancel));
            if (ret) {
                helper->logFlow("Cancel", "this call has removed, id", id, "", VT_SRV_LOG_E);
                return;
            }

            (*mIsForceCancel) = VT_TRUE;

            core->notifyCallback(
                id,
                VT_SRV_NOTIFY_RECV_ENHANCE_SESSION_IND,
                VT_SRV_SESSION_IND_CANCEL,
                0,
                0,
                helper->packFromVdoProfile(Vp),
                String8(""),
                NULL);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_CNF) {

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_MODIFY_CANCEL_CNF", VT_IVD, "", VT_SRV_LOG_W);

            VT_IMCB_CANCEL_CNF confirm;

            memset(&confirm, 0, sizeof(VT_IMCB_CANCEL_CNF));
            memcpy(&confirm, reinterpret_cast<VT_IMCB_CANCEL_CNF *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(confirm.sim_slot_id, confirm.call_id);

            sp<VideoProfile> Vp = helper->getProfileFromCancelCnf(&confirm);

            helper->logAction("vt_callback", "Vp->getResult", Vp->getResult(), "", VT_SRV_LOG_W);

            VT_IMCB_CANCEL_REQ * request;
            ret = helper->getParam(id, VT_SRV_PARAM_CANCEL_SESSION_REQ, reinterpret_cast<void **>(&request));
            if (ret) goto error;

            core->notifyCallback(
                id,
                VT_SRV_NOTIFY_RECV_SESSION_CONFIG_RSP,
                Vp->getResult(),
                0,
                0,
                helper->packFromVdoProfile(Vp),
                helper->packFromVdoProfile(Vp),
                NULL);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_EVENT_LOCAL_BW_READY_IND) {

            helper->logAction("vt_callback", "MSG_ID_WRAP_IMSVT_IMCB_EVENT_LOCAL_BW_READY_IND", VT_IVD, "", VT_SRV_LOG_W);

            VT_IMCB_UPGRADE_BW_READY_IND bw_ind;

            memset(&bw_ind, 0, sizeof(VT_IMCB_UPGRADE_BW_READY_IND));
            memcpy(&bw_ind, reinterpret_cast<VT_IMCB_UPGRADE_BW_READY_IND *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(bw_ind.sim_slot_id, bw_ind.call_id);

            core->notifyCallback(
                id,
                VT_SRV_NOTIFY_BW_READY_IND);

        } else if (type == MSG_ID_WRAP_IMSVT_IMCB_CALL_PROGRESS_IND) {

            VT_IMCB_CALL_PROGRESS_IND ind;

            memset(&ind, 0, sizeof(VT_IMCB_CALL_PROGRESS_IND));
            memcpy(&ind, reinterpret_cast<VT_IMCB_CALL_PROGRESS_IND *>(data), len);

            id = CONSTRUCT_SIM_CALL_ID(ind.sim_slot_id, ind.call_id);

            helper->logAction("vt_callback", "id", id, "MSG_ID_WRAP_IMSVT_IMCB_CALL_PROGRESS_IND", VT_SRV_LOG_W);

            int* callState;
            ret = helper->getParam(id, VT_SRV_PARAM_CALL_STATE, reinterpret_cast<void **>(&callState));
            if (ret) goto error;

            (*callState) = ind.ecpi;
            helper->logAction("vt_callback", "callState", id, helper->getCallStateString(*callState), VT_SRV_LOG_W);

        } else if (type == MSG_ID_MD_ANBR_CONFIG_UPDATE_IND) {

            helper->logAction("vt_callback", "MSG_ID_MD_ANBR_CONFIG_UPDATE_IND", VT_IVD, "", VT_SRV_LOG_W);
            VT_ANBR_CONFIG   anbr;

            memset(&anbr, 0, sizeof(VT_ANBR_CONFIG));

            // ================================================================
            // copy to ua config
            // ================================================================
            memcpy(&anbr, reinterpret_cast<VT_ANBR_CONFIG *>(data), len);

            VT_LOGI("[SRV] [VT_ANBR_CONFIG] config=%d, ebi=%d, is_ul=%d, bitrate=%d, bearer_id=%d, pdu_session_id=%d, ext_param=%d",
                    anbr.config, anbr.ebi, anbr.is_ul, anbr.bitrate, anbr.bearer_id, anbr.pdu_session_id, anbr.ext_param);
            //call MA API to set ebi, is_ul and bitrate

            for (int idx = 0; idx < VT_SRV_MA_NR; idx++) {
                if (helper->isUsed(idx)) {
                    if (anbr.config == 0) {
                        ret = helper->getFromIndex(idx)->setRANEnable(false);
                    } else if (anbr.config == 1) {
                        ret = helper->getFromIndex(idx)->setRANEnable(true);
                    } else if (anbr.config == 2) {
                        ret = helper->getFromIndex(idx)->notifyRanBitRate(anbr.is_ul, anbr.bitrate);
                    }
                    if (ret) goto error;
                }
            }
        } else if (type == MSG_ID_MD_INTER_RAT_STATUS_IND) {

            helper->logAction("vt_callback", "MSG_ID_MD_INTER_RAT_STATUS_IND", VT_IVD, "", VT_SRV_LOG_W);
            VT_IRAT_STRUCT irat;

            memset(&irat, 0, sizeof(VT_IRAT_STRUCT));

            // ================================================================
            // copy to ua config
            // ================================================================
            memcpy(&irat, reinterpret_cast<VT_IRAT_STRUCT *>(data), len);

            VT_LOGI("[SRV] [MSG_ID_MD_INTER_RAT_STATUS_IND] sim_slot_id=%d, irat_status=%d, is_successful=%d",
                    irat.sim_slot_id, irat.irat_status, irat.is_successful);

            /* +EIRAT:<irat_status>[,<is_successful>]
             *
             * <irat_status>: integer.
             * 0                    Idle (inter-RAT end)
             * 1                    Inter-RAT from LTE to GSM start
             * 2                    Inter-RAT from LTE to UMTS start
             * 3                    Inter-RAT from GSM to LTE start
             * 4                    Inter-RAT from UMTS to LTE start
             * 5                    Inter-RAT from LTE to GSM_UMTS(TBD) start
             * 6                    Inter-RAT from GSM_UMTS(TBD) to LTE start
             * 7                    Inter-RAT from NR to GSM start
             * 8                    Inter-RAT from NR to UMTS start
             * 9                    Inter-RAT from NR to GSM_UMTS(TBD) start
             * 10                   Inter-RAT from NR to LTE start
             * 11                   Inter-RAT from GSM to NR start
             * 12                   Inter-RAT from UMTS to NR start
             * 13                   Inter-RAT from GSM_UMTS(TBD) to NR start
             * 14                   Inter-RAT from LTE to NR start
             * 15                   Inter-RAT from GSM to UMTS start
             * 16                   Inter-RAT from UMTS to GSM start
             * 17                   Inter-RAT from GSM_UMTS(TBD) to GSM start
             * 18                   Inter-RAT from GSM to GSM_UMTS(TBD) start
             * 19                   Inter-RAT from GSM_UMTS(TBD) to UMTS start
             * 20                   Inter-RAT from UMTS to GSM_UMTS(TBD) start
             * <is_successful>: integer. Present only when <irat_status> is 0
             * 0, Inter-RAT procedure failed; 1, Inter-RAT procefure is successful
             *
             */

            /* Set handover state when EPS FB
             * Ex: EIRAT: 10 -> start
             *     EIRAT: 0, 0/1 -> stop
             */
            if (irat.irat_status == 0 &&
                (irat.is_successful == 0 || irat.is_successful == 1) &&
                helper->getHandoverStateBySim(irat.sim_slot_id) == VT_TRUE) {

                helper->setHandoverStateBySim(irat.sim_slot_id, VT_FALSE);
                core->setHandoverStateBySimId(irat.sim_slot_id, VT_FALSE);

            } else if (irat.irat_status == 10 &&
                helper->getHandoverStateBySim(irat.sim_slot_id) == VT_FALSE) {

                helper->setHandoverStateBySim(irat.sim_slot_id, VT_TRUE);
                core->setHandoverStateBySimId(irat.sim_slot_id, VT_TRUE);
            }

        }

        return;

        error:
            helper->logFlow("vt_callback", "id", id, helper->getErrorString(ret), VT_SRV_LOG_E);
            core->notifyCallback(id, notify_type);
            return;
    }

    void vt_rtp_codec_2_ua(int mode, VT_IMCB_CAP * ua, int ssid, int opid) {

        if (mode == VT_SRV_CALL_4G) {
            ua->sim_slot_id = ssid;

            sp<VTCoreHelper> helper = VTCore::mHelper;

            // ====================================================================
            // rtcp part
            // ====================================================================
            rtp_rtcp_capability_t *rtpCap;
            int rtp_num = ImsMa::getRtpRtcpCapability(&rtpCap, opid);

            // ====================================================================
            // codec part
            // ====================================================================
            video_codec_fmtp_t *codecCap;
            int codec_num = ImsMa::getCodecCapability(&codecCap, NULL, opid, QUALITY_DEFAULT);

            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] codec_num = %d", codec_num);

            //Save default local size W/H for preview, must always notify w>h
            int w = 320, h = 240;
            if (codec_num != 0 && codecCap != NULL) {
                w = MAX(codecCap[0].width, codecCap[0].height);
                h = MIN(codecCap[0].width, codecCap[0].height);
            }
            helper->setDefaultLocalSize(w, h);
            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] Save default local size W=%d, H=%d)", w, h);

            // ====================================================================
            // SPS part
            // ====================================================================
            uint32_t levelCapNumbers;
            video_codec_level_fmtp_t *codeclevelcap;

            ImsMa::getCodecParameterSets(VIDEO_H264, &levelCapNumbers, &codeclevelcap, opid, QUALITY_FINE);

            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] (H264) levelCapNumbers = %d (need <= %d)", levelCapNumbers, VT_MAX_PS_NUM);

            for (int i = 0; i < levelCapNumbers && i < VT_MAX_PS_NUM; i++) {
                ua->h264_ps[i].profile_level_id             = codeclevelcap[i].codec_level_fmtp.h264_codec_level_fmtp.profile_level_id;
                memcpy(ua->h264_ps[i].sprop_parameter_sets  , codeclevelcap[i].codec_level_fmtp.h264_codec_level_fmtp.sprop_parameter_sets, VT_MAX_SDP_PARAMETER_SET_LENGTH);
            }

            ImsMa::getCodecParameterSets(VIDEO_HEVC, &levelCapNumbers, &codeclevelcap, opid, QUALITY_FINE);

            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] (H265) levelCapNumbers = %d (need <= %d)", levelCapNumbers, VT_MAX_PS_NUM);

            for (int i = 0; i < levelCapNumbers && i < VT_MAX_PS_NUM; i++) {
                ua->hevc_ps[i].profile_id                   = codeclevelcap[i].codec_level_fmtp.hevc_codec_level_fmtp.profile_id;
                ua->hevc_ps[i].level_id                     = codeclevelcap[i].codec_level_fmtp.hevc_codec_level_fmtp.level_id;
                memcpy(ua->hevc_ps[i].sprop_vps             , codeclevelcap[i].codec_level_fmtp.hevc_codec_level_fmtp.sprop_vps, VT_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(ua->hevc_ps[i].sprop_sps             , codeclevelcap[i].codec_level_fmtp.hevc_codec_level_fmtp.sprop_sps, VT_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(ua->hevc_ps[i].sprop_pps             , codeclevelcap[i].codec_level_fmtp.hevc_codec_level_fmtp.sprop_pps, VT_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(ua->hevc_ps[i].sprop_sei             , codeclevelcap[i].codec_level_fmtp.hevc_codec_level_fmtp.sprop_sei, VT_MAX_SDP_PARAMETER_SET_LENGTH);
            }

            // ====================================================================
            // bit rate table part
            // ====================================================================
            unsigned int tableCnt;
            video_media_bitrate_t *bitrateTable;
            ImsMa::getCodecBitrateTable(&tableCnt, &bitrateTable, opid);

            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] bit rate table Cnt     = %d (need <= %d)", tableCnt, VT_MAX_LEVEL_SIZE);

            for (int k = 0; k < tableCnt && k < VT_MAX_LEVEL_SIZE; k++) {
                ua->bitrate_info[k].format                  = bitrateTable[k].format;
                ua->bitrate_info[k].profile                 = bitrateTable[k].profile;
                ua->bitrate_info[k].level                   = bitrateTable[k].level;
                ua->bitrate_info[k].minbitrate              = bitrateTable[k].minbitrate;
                ua->bitrate_info[k].bitrate                 = bitrateTable[k].bitrate;
            }

            // ====================================================================
            // merge rtp/codec with type
            // ====================================================================
            int final_num = 0;
            for (int i = 0; i < codec_num; i++) {
                for (int j =0; j < rtp_num; j++) {
                    if (codecCap[i].format == rtpCap[j].mime_Type) {

                        VT_LOGI("[SRV] [vt_rtp_codec_2_ua] find match type = %d, count = %d", rtpCap[j].mime_Type, final_num);

                        vt_srv_video_capability_t *cap = &(ua->video_cap[final_num]);

                        cap->mime_Type = codecCap[i].format;
                        if (cap->mime_Type == VIDEO_H264) {
                            cap->codec_cap.h264_codec.profile_level_id                  = codecCap[i].codec_fmtp.h264_codec_fmtp.profile_level_id;
                            cap->codec_cap.h264_codec.max_recv_level                    = codecCap[i].codec_fmtp.h264_codec_fmtp.max_recv_level;
                            cap->codec_cap.h264_codec.redundant_pic_cap                 = codecCap[i].codec_fmtp.h264_codec_fmtp.redundant_pic_cap;
                            memcpy(cap->codec_cap.h264_codec.sprop_parameter_sets       , codecCap[i].codec_fmtp.h264_codec_fmtp.sprop_parameter_sets       , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                            memcpy(cap->codec_cap.h264_codec.sprop_level_parameter_sets , codecCap[i].codec_fmtp.h264_codec_fmtp.sprop_level_parameter_sets , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                            cap->codec_cap.h264_codec.max_mbps                          = codecCap[i].codec_fmtp.h264_codec_fmtp.max_mbps;
                            cap->codec_cap.h264_codec.max_smbps                         = codecCap[i].codec_fmtp.h264_codec_fmtp.max_smbps;
                            cap->codec_cap.h264_codec.max_fs                            = codecCap[i].codec_fmtp.h264_codec_fmtp.max_fs;
                            cap->codec_cap.h264_codec.max_cpb                           = codecCap[i].codec_fmtp.h264_codec_fmtp.max_cpb;
                            cap->codec_cap.h264_codec.max_dpb                           = codecCap[i].codec_fmtp.h264_codec_fmtp.max_dpb;
                            cap->codec_cap.h264_codec.max_br                            = codecCap[i].codec_fmtp.h264_codec_fmtp.max_br;
                            cap->codec_cap.h264_codec.max_rcmd_nalu_size                = codecCap[i].codec_fmtp.h264_codec_fmtp.max_rcmd_nalu_size;
                            cap->codec_cap.h264_codec.sar_understood                    = codecCap[i].codec_fmtp.h264_codec_fmtp.sar_understood;
                            cap->codec_cap.h264_codec.sar_supported                     = codecCap[i].codec_fmtp.h264_codec_fmtp.sar_supported;
                            cap->codec_cap.h264_codec.in_band_parameter_sets            = 0;
                            cap->codec_cap.h264_codec.level_asymmetry_allowed           = 0;

                            cap->codec_cap.h264_codec.framerate                         = codecCap[i].fps;
                            cap->codec_cap.h264_codec.frame_width                       = codecCap[i].width;
                            cap->codec_cap.h264_codec.frame_height                      = codecCap[i].height;
                            for (int k = 0; k < VT_MAX_IMAGE_NUM; k++ ) {
                                cap->codec_cap.h264_codec.image_send[k].x               = codecCap[i].image_send[k].x;
                                cap->codec_cap.h264_codec.image_send[k].y               = codecCap[i].image_send[k].y;
                                memcpy(cap->codec_cap.h264_codec.image_send[k].sar      , codecCap[i].image_send[k].sar, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.h264_codec.image_send[k].par      , codecCap[i].image_send[k].par, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.h264_codec.image_send[k].q        , codecCap[i].image_send[k].q,   VT_MAX_SDP_EGBYTE_LENGTH);
                            }
                            for (int k = 0; k < VT_MAX_IMAGE_NUM; k++ ) {
                                cap->codec_cap.h264_codec.image_recv[k].x               = codecCap[i].image_recv[k].x;
                                cap->codec_cap.h264_codec.image_recv[k].y               = codecCap[i].image_recv[k].y;
                                memcpy(cap->codec_cap.h264_codec.image_recv[k].sar      , codecCap[i].image_recv[k].sar, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.h264_codec.image_recv[k].par      , codecCap[i].image_recv[k].par, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.h264_codec.image_recv[k].q        , codecCap[i].image_recv[k].q,   VT_MAX_SDP_EGBYTE_LENGTH);
                            }

                        } else if (cap->mime_Type == VIDEO_HEVC) {
                            cap->codec_cap.hevc_codec.profile_space                     = codecCap[i].codec_fmtp.hevc_codec_fmtp.profile_space;
                            cap->codec_cap.hevc_codec.profile_id                        = codecCap[i].codec_fmtp.hevc_codec_fmtp.profile_id;
                            cap->codec_cap.hevc_codec.tier_flag                         = codecCap[i].codec_fmtp.hevc_codec_fmtp.tier_flag;
                            cap->codec_cap.hevc_codec.level_id                          = codecCap[i].codec_fmtp.hevc_codec_fmtp.level_id;

                            for (int k = 0; k < 6; k++) {
                                cap->codec_cap.hevc_codec.interop_constraints[k]        = codecCap[i].codec_fmtp.hevc_codec_fmtp.interop_constraints[k];
                            }
                            for (int k = 0; k < 4; k++) {
                                cap->codec_cap.hevc_codec.profile_comp_ind[k]           = codecCap[i].codec_fmtp.hevc_codec_fmtp.profile_comp_ind[k];
                            }

                            cap->codec_cap.hevc_codec.sprop_sub_layer_id                = codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_sub_layer_id;
                            cap->codec_cap.hevc_codec.recv_sub_layer_id                 = codecCap[i].codec_fmtp.hevc_codec_fmtp.recv_sub_layer_id;
                            cap->codec_cap.hevc_codec.max_recv_level_id                 = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_recv_level_id;
                            cap->codec_cap.hevc_codec.tx_mode                           = codecCap[i].codec_fmtp.hevc_codec_fmtp.tx_mode;

                            memcpy(cap->codec_cap.hevc_codec.sprop_vps                  , codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_vps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                            memcpy(cap->codec_cap.hevc_codec.sprop_sps                  , codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_sps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                            memcpy(cap->codec_cap.hevc_codec.sprop_pps                  , codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_pps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                            memcpy(cap->codec_cap.hevc_codec.sprop_sei                  , codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_sei, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);

                            cap->codec_cap.hevc_codec.max_lsr                           = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_lsr;
                            cap->codec_cap.hevc_codec.max_lps                           = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_lps;
                            cap->codec_cap.hevc_codec.max_cpb                           = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_cpb;
                            cap->codec_cap.hevc_codec.max_dpb                           = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_dpb;
                            cap->codec_cap.hevc_codec.max_br                            = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_br;
                            cap->codec_cap.hevc_codec.max_tr                            = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_tr;
                            cap->codec_cap.hevc_codec.max_tc                            = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_tc;
                            cap->codec_cap.hevc_codec.max_fps                           = codecCap[i].codec_fmtp.hevc_codec_fmtp.max_fps;
                            cap->codec_cap.hevc_codec.sprop_max_don_diff                = codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_max_don_diff;
                            cap->codec_cap.hevc_codec.sprop_depack_buf_nalus            = codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_depack_buf_nalus;
                            cap->codec_cap.hevc_codec.sprop_depack_buf_bytes            = codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_depack_buf_bytes;
                            cap->codec_cap.hevc_codec.depack_buf_cap                    = codecCap[i].codec_fmtp.hevc_codec_fmtp.depack_buf_cap;
                            cap->codec_cap.hevc_codec.sprop_seg_id                      = codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_seg_id;
                            cap->codec_cap.hevc_codec.sprop_spatial_seg_idc             = codecCap[i].codec_fmtp.hevc_codec_fmtp.sprop_spatial_seg_idc;

                            memcpy(cap->codec_cap.hevc_codec.dec_parallel_cap           , codecCap[i].codec_fmtp.hevc_codec_fmtp.dec_parallel_cap, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                            memcpy(cap->codec_cap.hevc_codec.include_dph                , codecCap[i].codec_fmtp.hevc_codec_fmtp.include_dph     , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);

                            cap->codec_cap.hevc_codec.framerate                         = codecCap[i].fps;
                            cap->codec_cap.hevc_codec.frame_width                       = codecCap[i].width;
                            cap->codec_cap.hevc_codec.frame_height                      = codecCap[i].height;
                            for (int k = 0; k < VT_MAX_IMAGE_NUM; k++ ) {
                                cap->codec_cap.hevc_codec.image_send[k].x               = codecCap[i].image_send[k].x;
                                cap->codec_cap.hevc_codec.image_send[k].y               = codecCap[i].image_send[k].y;
                                memcpy(cap->codec_cap.hevc_codec.image_send[k].sar      , codecCap[i].image_send[k].sar, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.hevc_codec.image_send[k].par      , codecCap[i].image_send[k].par, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.hevc_codec.image_send[k].q        , codecCap[i].image_send[k].q,   VT_MAX_SDP_EGBYTE_LENGTH);
                            }
                            for (int k = 0; k < VT_MAX_IMAGE_NUM; k++ ) {
                                cap->codec_cap.hevc_codec.image_recv[k].x               = codecCap[i].image_recv[k].x;
                                cap->codec_cap.hevc_codec.image_recv[k].y               = codecCap[i].image_recv[k].y;
                                memcpy(cap->codec_cap.hevc_codec.image_recv[k].sar      , codecCap[i].image_recv[k].sar, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.hevc_codec.image_recv[k].par      , codecCap[i].image_recv[k].par, VT_MAX_SDP_EGBYTE_LENGTH);
                                memcpy(cap->codec_cap.hevc_codec.image_recv[k].q        , codecCap[i].image_recv[k].q,   VT_MAX_SDP_EGBYTE_LENGTH);
                            }
                        }

                        cap->media_type                  = rtpCap[j].media_type;
                        cap->mime_Type                   = rtpCap[j].mime_Type;
                        cap->rtp_profile                 = rtpCap[j].rtp_profile;
                        cap->sample_rate                 = rtpCap[j].sample_rate;
                        cap->packetize_mode              = rtpCap[j].packetize_mode;
                        cap->rtcp_rsize                  = rtpCap[j].rtcp_reduce_size;

                        cap->rtcp_fb_param_num           = rtpCap[j].rtcp_fb_param_num;
                        memcpy(cap->rtcp_fb_type         , rtpCap[j].rtcp_fb_type, sizeof(rtcp_fb_param_type_t) * VT_MAX_RTCP_FB_SIZE);

                        cap->extmap_num = rtpCap[j].rtp_header_extension_num;
                        VT_LOGI("[SRV] [vt_rtp_codec_2_ua] extmap_num             = %d", cap->extmap_num);
                        for (int k = 0; k < cap->extmap_num; k++) {
                            cap->extmap[k].ext_id        = rtpCap[j].rtp_ext_map[k].extension_id;
                            cap->extmap[k].direction     = rtpCap[j].rtp_ext_map[k].direction;
                            memcpy(cap->extmap[k].ext_uri, rtpCap[j].rtp_ext_map[k].extension_uri, VT_MAX_EXTMAP_URI_SIZE);
                            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] ext_id                 = %d", cap->extmap[k].ext_id);
                            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] direction              = %d", cap->extmap[k].direction);
                            VT_LOGI("[SRV] [vt_rtp_codec_2_ua] ext_uri                = %s", cap->extmap[k].ext_uri);
                        }
                        VT_LOGI("[SRV] [vt_rtp_codec_2_ua] cap.media_type         = %d", cap->media_type);
                        VT_LOGI("[SRV] [vt_rtp_codec_2_ua] cap.mime_Type          = %d", cap->mime_Type);

                        final_num++;
                    }
                }
            }
            ua->video_cap_num = final_num;

        }
    }

    void vt_ua_2_rtp(int mode, VT_IMCB_CONFIG * ua, rtp_rtcp_config_t * rtp) {

        if (mode == VT_SRV_CALL_4G) {
            sp<VTCoreHelper> helper = VTCore::mHelper;

            // ====================================================================
            // rtcp part
            // ====================================================================
            vt_srv_imcb_msg_config_t *cap = &(ua->config);

            VT_LOGI("[SRV] [vt_ua_2_rtp] media_type              = %d", cap->media_type);
            VT_LOGI("[SRV] [vt_ua_2_rtp] mime_Type               = %d", cap->mime_Type);
            VT_LOGI("[SRV] [vt_ua_2_rtp] packetize_mode          = %d", cap->packetize_mode);
            VT_LOGI("[SRV] [vt_ua_2_rtp] rtcp_sender_bandwidth   = %d", cap->video_b_rs);
            VT_LOGI("[SRV] [vt_ua_2_rtp] rtcp_receiver_bandwidth = %d", cap->video_b_rr);
            VT_LOGI("[SRV] [vt_ua_2_rtp] ebi                     = %d", ua->setting.ebi);
            VT_LOGI("[SRV] [vt_ua_2_rtp] network_id              = %d", ua->setting.network_id);
            VT_LOGI("[SRV] [vt_ua_2_rtp] if_name                 = %s", ua->setting.if_name);
            VT_LOGI("[SRV] [vt_ua_2_rtp] rtp_direction           = %d", cap->rtp_direction);
            VT_LOGI("[SRV] [vt_ua_2_rtp] remote_addr_type        = %d", cap->remote_addr_type);
            VT_LOGI("[SRV] [vt_ua_2_rtp] local_rtp_port          = %d", cap->local_rtp_port);
            VT_LOGI("[SRV] [vt_ua_2_rtp] local_rtcp_port         = %d", cap->local_rtcp_port);

            // cap->call_id;
            // cap->camera_direction;

            rtp->media_type                         = cap->media_type;
            rtp->mime_Type                          = cap->mime_Type;
            rtp->rtp_payload_type                   = cap->video_payload_type;
            rtp->rtp_profile                        = cap->rtp_profile;
            rtp->sample_rate                        = cap->sample_rate;
            rtp->rtp_packet_bandwidth               = cap->video_b_as;
            rtp->packetize_mode                     = cap->packetize_mode;
            //rtp->rtp_header_extension_num;          = ;
            //rtp->rtp_ext_map[0]                     = ;
            rtp->rtcp_sender_bandwidth              = cap->video_b_rs;
            rtp->rtcp_receiver_bandwidth            = cap->video_b_rr;
            rtp->rtcp_reduce_size                   = cap->rtcp_rsize;

            rtp->rtcp_fb_param_num                  = cap->rtcp_fb_param_num;
            memcpy(rtp->rtcp_fb_type                , cap->rtcp_fb_type, sizeof(vt_srv_rtcp_fb_param_type_t) * VT_MAX_RTCP_FB_SIZE);

            rtp->network_info.ebi                   = ua->setting.ebi;
            rtp->network_info.interface_type        = ua->setting.video_type;
            rtp->network_info.dscp                  = cap->video_dscp;
            rtp->network_info.soc_priority          = cap->video_soc_priority;

            rtp->network_info.remote_rtp_port       = cap->remote_rtp_port;
            rtp->network_info.remote_rtcp_port      = cap->remote_rtcp_port;
            rtp->network_info.local_rtp_port        = cap->local_rtp_port;
            rtp->network_info.local_rtcp_port       = cap->local_rtcp_port;
            memcpy(rtp->network_info.remote_rtp_address       , cap->remote_rtp_address,    VT_ADDR_LENGTH);
            memcpy(rtp->network_info.remote_rtcp_address      , cap->remote_rtcp_address,   VT_ADDR_LENGTH);
            memcpy(rtp->network_info.local_rtp_address        , cap->local_address,         VT_ADDR_LENGTH);
            memcpy(rtp->network_info.local_rtcp_address       , cap->local_address,         VT_ADDR_LENGTH);
            rtp->network_info.remote_addr_type      = cap->remote_addr_type;
            rtp->network_info.local_addr_type       = cap->remote_addr_type;

            if (ua->setting.network_id == VT_INVALID_NETWORK_ID) {
                String8 if_name8 = String8((char*)ua->setting.if_name);
                rtp->network_info.network_id = helper->getNetworkIdByIfName(if_name8);
                VT_LOGI("[SRV] [vt_ua_2_rtp] update network_id = %d", rtp->network_info.network_id);

            } else {
                rtp->network_info.network_id = ua->setting.network_id;
            }

            memcpy(rtp->network_info.ifname, ua->setting.if_name, VT_MAX_IF_NAME_LENGTH);
            rtp->network_info.uid                   = g_vt.core->mUserID;
            rtp->network_info.rtp_direction         = cap->rtp_direction;

            // Video over LTE case
            int id = CONSTRUCT_SIM_CALL_ID(ua->setting.sim_slot_id, ua->config.call_id);

            if (ua->setting.video_type == 0) {

                rtp->network_info.tag = TAG_VILTE_MOBILE + id;

            // Video over Wifi case
            } else {

                rtp->network_info.tag = TAG_VILTE_WIFI + id;
            }

            for (int i = 0; i < VILTE_BIND_FD_NUMBER; i++) {
                rtp->network_info.socket_fds[i]     = -1;
            }

            rtp->network_info.MBR_DL                = ua->setting.nw_assigned_dl_bw;
            rtp->network_info.MBR_UL                = ua->setting.nw_assigned_ul_bw;

            uint32_t rtp_header_extension_num;
            rtp_ext_map_t rtp_ext_map[1];//now only CVO support

            rtp->rtp_header_extension_num           = 1;
            for (int i = 0; i < rtp->rtp_header_extension_num; i++) {
                rtp->rtp_ext_map[i].extension_id    = cap->extmap.ext_id ;
                rtp->rtp_ext_map[i].direction       = cap->extmap.direction;
                memcpy(rtp->rtp_ext_map[i].extension_uri, cap->extmap.ext_uri, VT_MAX_EXTMAP_URI_SIZE);
                VT_LOGI("[SRV] [vt_ua_2_rtp] extension_id            = %d", cap->extmap.ext_id);
                VT_LOGI("[SRV] [vt_ua_2_rtp] direction               = %d", cap->extmap.direction);
                VT_LOGI("[SRV] [vt_ua_2_rtp] extension_uri           = %s", cap->extmap.ext_uri);
            }

        }
    }

    void vt_ua_2_codec(int mode, VT_IMCB_CONFIG * ua, video_codec_fmtp_t * codec) {

        if (mode == VT_SRV_CALL_4G) {

            // ====================================================================
            // codec part
            // ====================================================================
            vt_srv_video_codec_cap_cfg_t *codec_cfg = &(ua->config.codec_cfg);

            VT_LOGI("[SRV] [vt_ua_2_codec] format                = %d", ua->config.mime_Type);
            VT_LOGI("[SRV] [vt_ua_2_codec] profile_level_id      = %d", codec_cfg->h264_codec.profile_level_id);

            codec->format                           = (video_format_t) ua->config.mime_Type;
            codec->fps                              = -1;

            if (VIDEO_H264 == codec->format) {

                VT_LOGI("[SRV] [vt_ua_2_codec] format                = H264");
                VT_LOGI("[SRV] [vt_ua_2_codec] profile_level_id      = %d", codec_cfg->h264_codec.profile_level_id);

                codec->codec_fmtp.h264_codec_fmtp.profile_level_id                 = codec_cfg->h264_codec.profile_level_id;
                codec->codec_fmtp.h264_codec_fmtp.max_recv_level                   = codec_cfg->h264_codec.max_recv_level;
                codec->codec_fmtp.h264_codec_fmtp.redundant_pic_cap                = codec_cfg->h264_codec.redundant_pic_cap;

                memcpy(codec->codec_fmtp.h264_codec_fmtp.sprop_parameter_sets      , codec_cfg->h264_codec.sprop_parameter_sets,       VT_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.h264_codec_fmtp.sprop_level_parameter_sets, codec_cfg->h264_codec.sprop_level_parameter_sets, VT_MAX_SDP_PARAMETER_SET_LENGTH);

                codec->codec_fmtp.h264_codec_fmtp.max_mbps                         = codec_cfg->h264_codec.max_mbps;
                codec->codec_fmtp.h264_codec_fmtp.max_smbps                        = codec_cfg->h264_codec.max_smbps;
                codec->codec_fmtp.h264_codec_fmtp.max_fs                           = codec_cfg->h264_codec.max_fs;
                codec->codec_fmtp.h264_codec_fmtp.max_cpb                          = codec_cfg->h264_codec.max_cpb;
                codec->codec_fmtp.h264_codec_fmtp.max_dpb                          = codec_cfg->h264_codec.max_dpb;
                codec->codec_fmtp.h264_codec_fmtp.max_br                           = codec_cfg->h264_codec.max_br;

                codec->codec_fmtp.h264_codec_fmtp.max_rcmd_nalu_size               = codec_cfg->h264_codec.max_rcmd_nalu_size;
                codec->codec_fmtp.h264_codec_fmtp.sar_understood                   = codec_cfg->h264_codec.sar_understood;
                codec->codec_fmtp.h264_codec_fmtp.sar_supported                    = codec_cfg->h264_codec.sar_supported;

                VT_LOGI("[SRV] [vt_ua_2_codec] video_type            = %d", ua->setting.video_type);
                VT_LOGI("[SRV] [vt_ua_2_codec] video_b_as            = %d", ua->config.video_b_as);
                VT_LOGI("[SRV] [vt_ua_2_codec] nw_assigned_ul_bw     = %d", ua->setting.nw_assigned_ul_bw);

                // Video over LTE case
                if (ua->setting.video_type == 0) {

                    // if nw_assigned_ul_bw = 0, just use another one
                    if (ua->setting.nw_assigned_ul_bw != 0) {
                        if (ua->setting.nw_assigned_ul_bw > ua->config.video_b_as) {
                            codec->codec_fmtp.h264_codec_fmtp.video_b_as           = ua->config.video_b_as;
                        } else {
                            codec->codec_fmtp.h264_codec_fmtp.video_b_as           = ua->setting.nw_assigned_ul_bw;
                        }
                    } else {
                        codec->codec_fmtp.h264_codec_fmtp.video_b_as               = ua->config.video_b_as;
                    }

                // Video over Wifi case
                } else {
                    codec->codec_fmtp.h264_codec_fmtp.video_b_as                   = ua->config.video_b_as;
                }

            } else if (VIDEO_HEVC == codec->format) {

                VT_LOGI("[SRV] [vt_ua_2_codec] format                = H265");
                VT_LOGI("[SRV] [vt_ua_2_codec] profile_id            = %d", codec_cfg->hevc_codec.profile_id);

                codec->codec_fmtp.hevc_codec_fmtp.profile_space                    = codec_cfg->hevc_codec.profile_space;
                codec->codec_fmtp.hevc_codec_fmtp.tier_flag                        = codec_cfg->hevc_codec.tier_flag;
                codec->codec_fmtp.hevc_codec_fmtp.profile_id                       = codec_cfg->hevc_codec.profile_id;
                codec->codec_fmtp.hevc_codec_fmtp.level_id                         = codec_cfg->hevc_codec.level_id;

                for (int i = 0; i < 6;i++) {
                    codec->codec_fmtp.hevc_codec_fmtp.interop_constraints[i]       = codec_cfg->hevc_codec.interop_constraints[i];
                }

                for (int i = 0; i < 4;i++) {
                    codec->codec_fmtp.hevc_codec_fmtp.profile_comp_ind[i]          = codec_cfg->hevc_codec.profile_comp_ind[i];
                }

                codec->codec_fmtp.hevc_codec_fmtp.sprop_sub_layer_id               = codec_cfg->hevc_codec.sprop_sub_layer_id;
                codec->codec_fmtp.hevc_codec_fmtp.recv_sub_layer_id                = codec_cfg->hevc_codec.recv_sub_layer_id;
                codec->codec_fmtp.hevc_codec_fmtp.max_recv_level_id                = codec_cfg->hevc_codec.max_recv_level_id;
                codec->codec_fmtp.hevc_codec_fmtp.tx_mode                          = codec_cfg->hevc_codec.tx_mode;

                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_vps                 , codec_cfg->hevc_codec.sprop_vps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_sps                 , codec_cfg->hevc_codec.sprop_sps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_pps                 , codec_cfg->hevc_codec.sprop_pps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_sei                 , codec_cfg->hevc_codec.sprop_sei, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);

                codec->codec_fmtp.hevc_codec_fmtp.max_lsr                          = codec_cfg->hevc_codec.max_lsr;
                codec->codec_fmtp.hevc_codec_fmtp.max_lps                          = codec_cfg->hevc_codec.max_lps;
                codec->codec_fmtp.hevc_codec_fmtp.max_cpb                          = codec_cfg->hevc_codec.max_cpb;
                codec->codec_fmtp.hevc_codec_fmtp.max_dpb                          = codec_cfg->hevc_codec.max_dpb;
                codec->codec_fmtp.hevc_codec_fmtp.max_br                           = codec_cfg->hevc_codec.max_br;
                codec->codec_fmtp.hevc_codec_fmtp.max_tr                           = codec_cfg->hevc_codec.max_tr;
                codec->codec_fmtp.hevc_codec_fmtp.max_tc                           = codec_cfg->hevc_codec.max_tc;
                codec->codec_fmtp.hevc_codec_fmtp.max_fps                          = codec_cfg->hevc_codec.max_fps;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_max_don_diff               = codec_cfg->hevc_codec.sprop_max_don_diff;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_depack_buf_nalus           = codec_cfg->hevc_codec.sprop_depack_buf_nalus;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_depack_buf_bytes           = codec_cfg->hevc_codec.sprop_depack_buf_bytes;
                codec->codec_fmtp.hevc_codec_fmtp.depack_buf_cap                   = codec_cfg->hevc_codec.depack_buf_cap;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_seg_id                     = codec_cfg->hevc_codec.sprop_seg_id;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_spatial_seg_idc            = codec_cfg->hevc_codec.sprop_spatial_seg_idc;

                memcpy(codec->codec_fmtp.hevc_codec_fmtp.dec_parallel_cap          , codec_cfg->hevc_codec.dec_parallel_cap , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.include_dph               , codec_cfg->hevc_codec.include_dph      , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);

                VT_LOGI("[SRV] [vt_ua_2_codec] video_type            = %d", ua->setting.video_type);
                VT_LOGI("[SRV] [vt_ua_2_codec] video_b_as            = %d", ua->config.video_b_as);
                VT_LOGI("[SRV] [vt_ua_2_codec] nw_assigned_ul_bw     = %d", ua->setting.nw_assigned_ul_bw);

                // Video over LTE case
                if (ua->setting.video_type == 0) {

                    // if nw_assigned_ul_bw = 0, just use another one
                    if (ua->setting.nw_assigned_ul_bw == 0) {
                        if (ua->setting.nw_assigned_ul_bw > ua->config.video_b_as) {
                            codec->codec_fmtp.hevc_codec_fmtp.video_b_as           = ua->config.video_b_as;
                        } else {
                            codec->codec_fmtp.hevc_codec_fmtp.video_b_as           = ua->setting.nw_assigned_ul_bw;
                        }
                    } else {
                        codec->codec_fmtp.hevc_codec_fmtp.video_b_as               = ua->config.video_b_as;
                    }

                // Video over Wifi case
                } else {
                    codec->codec_fmtp.hevc_codec_fmtp.video_b_as                   = ua->config.video_b_as;
                }
            }

        }
    }

    void vt_ut_ua_2_rtp(int mode, VT_IMCB_CAP * ua, rtp_rtcp_config_t * rtp) {

        if (mode == VT_SRV_CALL_4G) {

            // ====================================================================
            // rtcp part
            // ====================================================================
            vt_srv_video_capability_t *cap = &(ua->video_cap[0]);

            VT_LOGI("[SRV] [vt_ut_ua_2_rtp] cap.mime_Type        = %d", cap->mime_Type);
            VT_LOGI("[SRV] [vt_ut_ua_2_rtp] cap.media_type       = %d", cap->media_type);

            rtp->media_type                         = cap->media_type;
            rtp->mime_Type                          = cap->mime_Type;
            rtp->rtp_payload_type                   = 56;
            rtp->rtp_profile                        = IMSMA_RTP_AVPF;
            rtp->sample_rate                        = cap->sample_rate;
            //rtp->rtp_packet_bandwidth               = cap->video_b_as;
            rtp->packetize_mode                     = IMSMA_NON_INTERLEAVED_MODE;
            //rtp->rtcp_sender_bandwidth              = cap->video_b_rs;
            //rtp->rtcp_receiver_bandwidth            = cap->video_b_rr;
            rtp->rtcp_reduce_size                   = cap->rtcp_rsize;

            rtp->rtcp_fb_param_num                  = cap->rtcp_fb_param_num;
            memcpy(rtp->rtcp_fb_type                , cap->rtcp_fb_type, sizeof(VT_MAX_RTCP_FB_SIZE) * sizeof(vt_srv_rtcp_fb_param_type_t));

            rtp->network_info.ebi                   = -1;
            rtp->network_info.interface_type        = -1;
            rtp->network_info.dscp                  = -1;
            rtp->network_info.soc_priority          = -1;

            rtp->network_info.remote_rtp_port       = 40000;
            rtp->network_info.remote_rtcp_port      = 40001;
            rtp->network_info.local_rtp_port        = 40000;
            rtp->network_info.local_rtcp_port       = 40001;
            rtp->network_info.remote_rtp_address[0] = 127;
            rtp->network_info.remote_rtp_address[1] = 0;
            rtp->network_info.remote_rtp_address[2] = 0;
            rtp->network_info.remote_rtp_address[3] = 1;
            rtp->network_info.remote_rtcp_address[0] = 127;
            rtp->network_info.remote_rtcp_address[1] = 0;
            rtp->network_info.remote_rtcp_address[2] = 0;
            rtp->network_info.remote_rtcp_address[3] = 1;
            rtp->network_info.local_rtp_address[0] = 127;
            rtp->network_info.local_rtp_address[1] = 0;
            rtp->network_info.local_rtp_address[2] = 0;
            rtp->network_info.local_rtp_address[3] = 1;
            rtp->network_info.local_rtcp_address[0] = 127;
            rtp->network_info.local_rtcp_address[1] = 0;
            rtp->network_info.local_rtcp_address[2] = 0;
            rtp->network_info.local_rtcp_address[3] = 1;
            rtp->network_info.remote_addr_type      = 0;
            rtp->network_info.local_addr_type       = 0;
            rtp->network_info.network_id            = 0;
            rtp->network_info.uid                   = g_vt.core->mUserID;
            rtp->network_info.rtp_direction         = -1;
            rtp->network_info.tag                   = 0xFF000000;

            for (int i = 0; i < VILTE_BIND_FD_NUMBER; i++) {
                rtp->network_info.socket_fds[i] = -1;
            }

            rtp->rtp_header_extension_num           = 1;
            for (int i = 0; i < rtp->rtp_header_extension_num; i++) {
                rtp->rtp_ext_map[i].extension_id    = cap->extmap[i].ext_id ;
                rtp->rtp_ext_map[i].direction       = cap->extmap[i].direction;
                memcpy(rtp->rtp_ext_map[i].extension_uri, cap->extmap[i].ext_uri, VT_MAX_EXTMAP_URI_SIZE);
            }

            rtp->network_info.MBR_DL                = 0;
            rtp->network_info.MBR_DL                = 0;

        }
    }

    void vt_ut_ua_2_codec(int mode, VT_IMCB_CAP * ua, video_codec_fmtp_t * codec) {

        if (mode == VT_SRV_CALL_4G) {

            // ====================================================================
            // codec part
            // ====================================================================
            vt_srv_video_codec_cap_cfg_t *codec_cfg = &(ua->video_cap[0].codec_cap);

            codec->format                           = (video_format_t) ua->video_cap[0].mime_Type;
            codec->fps                              = 15;

            if (VIDEO_H264 == codec->format) {
                codec->codec_fmtp.h264_codec_fmtp.profile_level_id                 = codec_cfg->h264_codec.profile_level_id;
                codec->codec_fmtp.h264_codec_fmtp.max_recv_level                   = codec_cfg->h264_codec.max_recv_level;
                codec->codec_fmtp.h264_codec_fmtp.redundant_pic_cap                = codec_cfg->h264_codec.redundant_pic_cap;

                memcpy(codec->codec_fmtp.h264_codec_fmtp.sprop_parameter_sets      , codec_cfg->h264_codec.sprop_parameter_sets,       VT_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.h264_codec_fmtp.sprop_level_parameter_sets, codec_cfg->h264_codec.sprop_level_parameter_sets, VT_MAX_SDP_PARAMETER_SET_LENGTH);

                codec->codec_fmtp.h264_codec_fmtp.max_mbps                         = codec_cfg->h264_codec.max_mbps;
                codec->codec_fmtp.h264_codec_fmtp.max_smbps                        = codec_cfg->h264_codec.max_smbps;
                codec->codec_fmtp.h264_codec_fmtp.max_fs                           = codec_cfg->h264_codec.max_fs;
                codec->codec_fmtp.h264_codec_fmtp.max_cpb                          = codec_cfg->h264_codec.max_cpb;
                codec->codec_fmtp.h264_codec_fmtp.max_dpb                          = codec_cfg->h264_codec.max_dpb;
                codec->codec_fmtp.h264_codec_fmtp.max_br                           = codec_cfg->h264_codec.max_br;

                codec->codec_fmtp.h264_codec_fmtp.max_rcmd_nalu_size               = codec_cfg->h264_codec.max_rcmd_nalu_size;
                codec->codec_fmtp.h264_codec_fmtp.sar_understood                   = codec_cfg->h264_codec.sar_understood;
                codec->codec_fmtp.h264_codec_fmtp.sar_supported                    = codec_cfg->h264_codec.sar_supported;
                codec->codec_fmtp.h264_codec_fmtp.video_b_as                       = 384;
                //codec_cfg->h264_codec.in_band_parameter_sets;
                //codec_cfg->h264_codec.level_asymmetry_allowed;

            } else if (VIDEO_HEVC == codec->format) {

                codec->codec_fmtp.hevc_codec_fmtp.profile_space                    = codec_cfg->hevc_codec.profile_space;
                codec->codec_fmtp.hevc_codec_fmtp.tier_flag                        = codec_cfg->hevc_codec.tier_flag;
                codec->codec_fmtp.hevc_codec_fmtp.profile_id                       = codec_cfg->hevc_codec.profile_id;
                codec->codec_fmtp.hevc_codec_fmtp.level_id                         = codec_cfg->hevc_codec.level_id;

                for (int i = 0; i < 6;i++) {
                    codec->codec_fmtp.hevc_codec_fmtp.interop_constraints[i]       = codec_cfg->hevc_codec.interop_constraints[i];
                }

                for (int i = 0; i < 4;i++) {
                    codec->codec_fmtp.hevc_codec_fmtp.profile_comp_ind[i]          = codec_cfg->hevc_codec.profile_comp_ind[i];
                }

                codec->codec_fmtp.hevc_codec_fmtp.sprop_sub_layer_id               = codec_cfg->hevc_codec.sprop_sub_layer_id;
                codec->codec_fmtp.hevc_codec_fmtp.recv_sub_layer_id                = codec_cfg->hevc_codec.recv_sub_layer_id;
                codec->codec_fmtp.hevc_codec_fmtp.max_recv_level_id                = codec_cfg->hevc_codec.max_recv_level_id;
                codec->codec_fmtp.hevc_codec_fmtp.tx_mode                          = codec_cfg->hevc_codec.tx_mode;

                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_vps                 , codec_cfg->hevc_codec.sprop_vps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_sps                 , codec_cfg->hevc_codec.sprop_sps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_pps                 , codec_cfg->hevc_codec.sprop_pps, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.sprop_sei                 , codec_cfg->hevc_codec.sprop_sei, VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);

                codec->codec_fmtp.hevc_codec_fmtp.max_lsr                          = codec_cfg->hevc_codec.max_lsr;
                codec->codec_fmtp.hevc_codec_fmtp.max_lps                          = codec_cfg->hevc_codec.max_lps;
                codec->codec_fmtp.hevc_codec_fmtp.max_cpb                          = codec_cfg->hevc_codec.max_cpb;
                codec->codec_fmtp.hevc_codec_fmtp.max_dpb                          = codec_cfg->hevc_codec.max_dpb;
                codec->codec_fmtp.hevc_codec_fmtp.max_br                           = codec_cfg->hevc_codec.max_br;
                codec->codec_fmtp.hevc_codec_fmtp.max_tr                           = codec_cfg->hevc_codec.max_tr;
                codec->codec_fmtp.hevc_codec_fmtp.max_tc                           = codec_cfg->hevc_codec.max_tc;
                codec->codec_fmtp.hevc_codec_fmtp.max_fps                          = codec_cfg->hevc_codec.max_fps;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_max_don_diff               = codec_cfg->hevc_codec.sprop_max_don_diff;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_depack_buf_nalus           = codec_cfg->hevc_codec.sprop_depack_buf_nalus;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_depack_buf_bytes           = codec_cfg->hevc_codec.sprop_depack_buf_bytes;
                codec->codec_fmtp.hevc_codec_fmtp.depack_buf_cap                   = codec_cfg->hevc_codec.depack_buf_cap;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_seg_id                     = codec_cfg->hevc_codec.sprop_seg_id;
                codec->codec_fmtp.hevc_codec_fmtp.sprop_spatial_seg_idc            = codec_cfg->hevc_codec.sprop_spatial_seg_idc;

                memcpy(codec->codec_fmtp.hevc_codec_fmtp.dec_parallel_cap          , codec_cfg->hevc_codec.dec_parallel_cap , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                memcpy(codec->codec_fmtp.hevc_codec_fmtp.include_dph               , codec_cfg->hevc_codec.include_dph      , VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
                codec->codec_fmtp.hevc_codec_fmtp.video_b_as                       = 384;
            }

        }
    }
}
