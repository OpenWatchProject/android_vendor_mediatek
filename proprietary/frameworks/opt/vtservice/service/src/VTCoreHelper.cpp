/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#include "VTCoreHelper.h"
#include "VTMALStub.h"
#include "VTMsgDispatcher.h"
#include "ImsMa.h"

// for socket
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// for system property
#include <cutils/properties.h>
#include <cutils/sockets.h>
//for fd open
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

using namespace android;

namespace VTService {

    extern vt_srv_msg_struct g_vt_srv_msg_hdr;
    extern sp<IVTClient> gVTSClient;
    extern vt_srv_cntx_struct g_vt;
    Mutex g_vt_SendMsgLock;

    VTCoreHelper::VTCoreHelper() {
        VT_LOGW("[SRV] [VTCoreHelper] VTCoreHelper create");
        mDummyMa = new VTMALStub(VTMALStub::MA_INVALID_MODE, 1000, 0, VT_DEFAULT_OP_ID, NULL);
        mDummyRequestProfile = new VideoProfile(VT_SRV_VIDEO_STATE_BIDIRECTIONAL, VT_SRV_VIDEO_QTY_DEFAULT);
        mConSceFd = -1;

        for (int i = 0; i < VT_SRV_SIM_NR; i++) {
            mMap.mSimOpTable[i] = VT_DEFAULT_OP_ID;
            mMap.mSimHandoverTable[i] = VT_FALSE;
        }
    };

    void VTCoreHelper::logString(const char * str, VT_SRV_LOG_LV level) {
        char msg[VT_SRV_STR_LEN];

        const char *prefix = "[SRV] [OPERATION]";

        snprintf(msg, VT_SRV_STR_LEN, "%s %s", prefix, str);

        if (VT_SRV_LOG_E == level) {
            VT_LOGE("%s", msg);
        } else if (VT_SRV_LOG_W == level) {
            VT_LOGW("%s", msg);
        } else {
            VT_LOGI("%s", msg);
        }
    }

    void VTCoreHelper::logAction(const char * action, const char * des, int data, const char * data2, VT_SRV_LOG_LV level) {
        char msg[VT_SRV_STR_LEN];

        const char *prefix = "[SRV] [OPERATION]";

        if (!strcmp(des, "id")) {

            if (!strcmp(data2, "")) {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s", prefix, data, action);
            } else {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s (%s)", prefix, data, action, data2);
            }

        } else {

            if (data == VT_IVD) {
                snprintf(msg, VT_SRV_STR_LEN, "%s %s (%s : %s)", prefix, action, des, data2);
            } else {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s (%s : %s)", prefix, data, action, des, data2);
            }
        }

        if (VT_SRV_LOG_E == level) {
            VT_LOGE("%s", msg);
        } else if (VT_SRV_LOG_W == level) {
            VT_LOGW("%s", msg);
        } else {
            VT_LOGI("%s", msg);
        }
    }

    void VTCoreHelper::logFlow(const char * flow, const char * des, int data, const char * data2, VT_SRV_LOG_LV level) {
        char msg[VT_SRV_STR_LEN];

        const char *prefix = "[SRV] [CALL FLOW]";

        if (!strcmp(des, "id")) {

            if (!strcmp(data2, "")) {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s", prefix, data, flow);
            } else {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s (%s)", prefix, data, flow, data2);
            }

        } else {

            if (data == VT_IVD) {
                snprintf(msg, VT_SRV_STR_LEN, "%s %s (%s : %s)", prefix, flow, des, data2);
            } else {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s (%s : %s)", prefix, data, flow, des, data2);
            }
        }

        if (VT_SRV_LOG_E == level) {
            VT_LOGE("%s", msg);
        } else if (VT_SRV_LOG_W == level) {
            VT_LOGW("%s", msg);
        } else {
            VT_LOGI("%s", msg);
        }
    }

    void VTCoreHelper::logMsg(
            int notify,
            const char * des,
            int id,
            int data1,
            int data2,
            int data3,
            const char* obj1,
            const char* obj2,
            VT_SRV_LOG_LV level) {
        char msg[VT_SRV_STR_LEN];
        char msg2[VT_SRV_STR_LEN];
        char type[VT_SRV_STR_LEN];

        const char *prefix = "[SRV] [MA NOTIFY]";

        if (notify == VT_SRV_NOTIFY_RECEIVE_FIRSTFRAME)
            snprintf(type, sizeof(type), "Receive First Frame");
        else if (notify == VT_SRV_NOTIFY_SNAPSHOT_DONE)
            snprintf(type, sizeof(type), "Snapshot done");
        else if (notify == VT_SRV_NOTIFY_REC_INFO_UNKNOWN)
            snprintf(type, sizeof(type), "Recorder Event Info <unkonwn>");
        else if (notify == VT_SRV_NOTIFY_REC_INFO_REACH_MAX_DURATION)
            snprintf(type, sizeof(type), "Recorder Event Info <reach max duration>");
        else if (notify == VT_SRV_NOTIFY_REC_INFO_REACH_MAX_FILESIZE)
            snprintf(type, sizeof(type), "Recorder Event Info <reach max file size>");
        else if (notify == VT_SRV_NOTIFY_REC_INFO_NO_I_FRAME)
            snprintf(type, sizeof(type), "Recorder Event Info <no I frame>");
        else if (notify == VT_SRV_NOTIFY_REC_INFO_COMPLETE)
            snprintf(type, sizeof(type), "Recorder Event Info <complete>");
        else if (notify == VT_SRV_NOTIFY_CALL_END)
            snprintf(type, sizeof(type), "Call end");
        else if (notify == VT_SRV_NOTIFY_CALL_ABNORMAL_END)
            snprintf(type, sizeof(type), "Call abnormal end");
        else if (notify == VT_SRV_START_COUNTER)
            snprintf(type, sizeof(type), "Start Counter");
        else if (notify == VT_SRV_PEER_CAMERA_OPEN)
            snprintf(type, sizeof(type), "peer camera open");
        else if (notify == VT_SRV_PEER_CAMERA_CLOSE)
            snprintf(type, sizeof(type), "peer camera close");
        else if (notify == VT_SRV_NOTIFY_RECV_SESSION_CONFIG_REQ)
            snprintf(type, sizeof(type), "Receive session config request");
        else if (notify == VT_SRV_NOTIFY_RECV_SESSION_CONFIG_RSP)
            snprintf(type, sizeof(type), "Receive session config respounse");
        else if (notify == VT_SRV_NOTIFY_HANDLE_CALL_SESSION_EVT)
            snprintf(type, sizeof(type), "Receive call event");
        else if (notify == VT_SRV_NOTIFY_PEER_SIZE_CHANGED)
            snprintf(type, sizeof(type), "Peer size change");
        else if (notify == VT_SRV_NOTIFY_LOCAL_SIZE_CHANGED)
            snprintf(type, sizeof(type), "Local size change");
        else if (notify == VT_SRV_NOTIFY_DATA_USAGE_CHANGED)
            snprintf(type, sizeof(type), "Data usage change");
        else if (notify == VT_SRV_NOTIFY_BAD_DATA_BITRATE)
            snprintf(type, sizeof(type), "RTP data rate is bad");
        else if (notify == VT_SRV_NOTIFY_DATA_BITRATE_RECOVER)
            snprintf(type, sizeof(type), "RTP data rate is recovered");
        else if (notify == VT_SRV_NOTIFY_RECV_ENHANCE_SESSION_IND)
            snprintf(type, sizeof(type), "Receive enhance session indication");
        else if (notify == VT_SRV_NOTIFY_GET_CAP)
            snprintf(type, sizeof(type), "Get camera capability");
        else if (notify == VT_SRV_NOTIFY_LOCAL_BUFFER)
            snprintf(type, sizeof(type), "Record buffer received");
        else if (notify == VT_SRV_NOTIFY_UPLINK_STATE_CHANGE)
            snprintf(type, sizeof(type), "MA uplink state change");
        else if (notify == VT_SRV_NOTIFY_DEFAULT_LOCAL_SIZE)
            snprintf(type, sizeof(type), "Default local size is received");
        else if (notify == VT_SRV_ERROR_SERVICE)
            snprintf(type, sizeof(type), "Service error");
        else if (notify == VT_SRV_ERROR_SERVER_DIED)
            snprintf(type, sizeof(type), "Service Die");
        else if (notify == VT_SRV_ERROR_CAMERA_CRASHED)
            snprintf(type, sizeof(type), "Camera crash Error");
        else if (notify == VT_SRV_ERROR_CODEC)
            snprintf(type, sizeof(type), "Codec Error");
        else if (notify == VT_SRV_ERROR_REC)
            snprintf(type, sizeof(type), "Recording Error");
        else if (notify == VT_SRV_ERROR_CAMERA_SET_IGNORED)
            snprintf(type, sizeof(type), "Set camera ignore");
        else if (notify == VT_SRV_NOTIFY_AVPF_TMMBR_MBR_DL)
            snprintf(type, sizeof(type), "AVPF TMMBR MBR DL");
        else if (notify == VT_SRV_NOTIFY_RESTART_CAMERA)
            snprintf(type, sizeof(type), "restart camera");
        else if (notify == VT_SRV_NOTIFY_DATA_CHANGE_PAUSE)
            snprintf(type, sizeof(type), "data pause");
        else if (notify == VT_SRV_NOTIFY_DATA_CHANGE_RESUME)
            snprintf(type, sizeof(type), "data resume");
        else if (notify == VT_SRV_NOTIFY_SET_ANBR)
            snprintf(type, sizeof(type), "set ANBR");
        else
            snprintf(type, sizeof(type), "Unkonwn");

        if (!strcmp(des, "")) {
            snprintf(msg, VT_SRV_STR_LEN, "%s %s (id = %d) (arg1 = %d, arg2 = %d, arg3 = %d)",
                    prefix, type, id, data1, data2, data3);
            snprintf(msg2, VT_SRV_STR_LEN, "%s                   (obj1 = %s, obj2 = %s)",
                    prefix, obj1, obj2);
        } else {
            snprintf(msg, VT_SRV_STR_LEN, "%s %s (id = %d) (%s : arg1 = %d, arg2 = %d, arg3 = %d)",
                    prefix, type, id, des, data1, data2, data3);
            snprintf(msg2, VT_SRV_STR_LEN, "%s                   (obj1 = %s, obj2 = %s)",
                    prefix, obj1, obj2);
        }

        if (VT_SRV_LOG_E == level) {
            VT_LOGE("%s", msg);
            VT_LOGE("%s", msg2);
        } else if (VT_SRV_LOG_W == level) {
            VT_LOGW("%s", msg);
            VT_LOGW("%s", msg2);
        } else {
            VT_LOGI("%s", msg);
            VT_LOGI("%s", msg2);
        }
    }

    void VTCoreHelper::logMap(const char * action, const char * des, int data, VT_SRV_LOG_LV level) {
        char msg[VT_SRV_STR_LEN];

        const char *prefix = "[SRV]   [MAP OPERATION] ";

        if (!strcmp(des, "id")) {

            snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s", prefix, data, action);

        } else {

            if (data == VT_IVD) {
                snprintf(msg, VT_SRV_STR_LEN, "%s %s (%s)", prefix, action, des);
            } else {
                snprintf(msg, VT_SRV_STR_LEN, "%s[ID=%d] %s (%s)", prefix, data, action, des);
            }
        }

        if (VT_SRV_LOG_E == level) {
            VT_LOGE("%s", msg);
        } else if (VT_SRV_LOG_W == level) {
            VT_LOGW("%s", msg);
        } else {
            VT_LOGI("%s", msg);
        }
    }


    int VTCoreHelper::getUsedNr(void) {
        Mutex::Autolock mapLock(mMapLock);

        //logMap("getUsedNr", "count", mMap.mUsedSize, VT_SRV_LOG_I);
        return mMap.mUsedSize;
    }

    status_t VTCoreHelper::init(void) {
        Mutex::Autolock mapLock(mMapLock);

        VT_LOGW("[SRV] [VTCoreHelper] VTCoreHelper init");

        mMap.mUsedSize = 0;
        mMap.mDefaultLocalW = 320;
        mMap.mDefaultLocalH = 240;
        mMap.mIsSetSensorInfo = VT_FALSE;

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            mMap.mUsed[i] = 0;
            mMap.mTable[i].mUaVideoConfig = NULL;
            mMap.mTable[i].mMaConfig = NULL;
            mMap.mTable[i].mSessionReqProfile = NULL;
            clearElement(&mMap.mTable[i]);
        }

        return VT_SRV_RET_OK;
    }

    status_t VTCoreHelper::add(int id, sp<VTMALStub> ma_ptr) {
        Mutex::Autolock mapLock(mMapLock);

        // check if the table is full
        if (mMap.mUsedSize >= VT_SRV_MA_NR) {
            logMap("add", "table full", id, VT_SRV_LOG_E);
            return VT_SRV_RET_ERR_FULL;
        }

        // check if the id has already in table
        if (VT_SRV_RET_OK == checkElement(id)) {
            logMap("add", "entry exist", id, VT_SRV_LOG_E);
            return VT_SRV_RET_ERR_ALREADY_EXIST;
        }

        // find a empty entry and add it
        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (!mMap.mUsed[i]) {
                mMap.mUsed[i] = 1;
                clearElement(&mMap.mTable[i]);

                mMap.mTable[i].mId                  = id;
                mMap.mTable[i].mSimId                   = ma_ptr->getSimId();
                mMap.mTable[i].mPtr                     = ma_ptr;
                mMap.mTable[i].mUaVideoConfig           = new VT_IMCB_CONFIG();
                mMap.mTable[i].mMaConfig                = new media_config_t();

                if (VTMALStub::MA_NORMAL_MODE_3G == ma_ptr->getMode()) {
                    mMap.mTable[i].mMode                = VT_SRV_CALL_3G;
                } else if (VTMALStub::MA_NORMAL_MODE_4G == ma_ptr->getMode()) {
                    mMap.mTable[i].mMode                = VT_SRV_CALL_4G;
                } else {
                    mMap.mTable[i].mMode                = VT_SRV_CALL_NONE;
                }

                mMap.mUsedSize++;

                logMap("add", "add element", id, VT_SRV_LOG_I);

                dump();

                return VT_SRV_RET_OK;
            }
        }
        return VT_SRV_RET_ERR;
    }

    status_t VTCoreHelper::del(int id) {
        Mutex::Autolock mapLock(mMapLock);

        // check if the table is empty
        if (mMap.mUsedSize == 0) {
            logMap("delete", "table empty", id, VT_SRV_LOG_E);
            return VT_SRV_RET_ERR_EMPTY;
        }

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                mMap.mUsed[i] = 0;
                mMap.mUsedSize--;
                clearElement(&mMap.mTable[i]);

                logMap("delete", "delete", id, VT_SRV_LOG_I);

                dump();

                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("delete", "entry not exist", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    sp<VTMALStub> VTCoreHelper::get(int id) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id) && VT_SRV_CALL_NONE != mMap.mTable[i].mMode) {
                //logMap("get", "found", id, VT_SRV_LOG_I);
                return mMap.mTable[i].mPtr;
            }
        }
        logMap("get", "not found", id, VT_SRV_LOG_E);
        return mDummyMa;
    }

    sp<VTMALStub> VTCoreHelper::getFromIndex(int idx) {
        Mutex::Autolock mapLock(mMapLock);

        if (mMap.mUsed[idx] && VT_SRV_CALL_NONE != mMap.mTable[idx].mMode) {
                //logMap("getFromIndex", "found", idx, VT_SRV_LOG_I);
                return mMap.mTable[idx].mPtr;
        }

        logMap("getFromIndex", "not found", idx, VT_SRV_LOG_E);
        return mDummyMa;
    }

    int VTCoreHelper::getId(int idx) {
        return mMap.mTable[idx].mId;
    }

    status_t VTCoreHelper::check(int id) {

        Mutex::Autolock mapLock(mMapLock);

        // check if the table is empty
        if (mMap.mUsedSize == 0) {
            logMap("check", "table empty", id, VT_SRV_LOG_E);
            return VT_SRV_RET_ERR_NOT_EXIST;
        }

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                logMap("check", "entry exist", id, VT_SRV_LOG_E);
                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("check", "entry not exist", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    int VTCoreHelper::isUsed(int idx) {
        return mMap.mUsed[idx];
    }

    int VTCoreHelper::getOperatorId(int simId) {
        if (0 <= simId && simId < VT_SRV_SIM_NR) {
            return mMap.mSimOpTable[simId];
        } else {
            VT_LOGE("[SRV]   [MAP OPERATION] [SIM ID=%d] getOperatorId (not found)", simId);
            return VT_DEFAULT_OP_ID;
        }
    }

    status_t VTCoreHelper::setOperatorId(int simId, int opId) {
        if (0 <= simId && simId < VT_SRV_SIM_NR) {
            VT_LOGD("[SRV]   [MAP OPERATION] [SIM ID=%d] setOperatorId (set opId:%d)", simId, opId);
            mMap.mSimOpTable[simId] = opId;
            return VT_SRV_RET_OK;
        } else {
            VT_LOGE("[SRV]   [MAP OPERATION] [SIM ID=%d] setOperatorId (not found)", simId);
        }
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    int VTCoreHelper::getHandoverStateBySim(int simId) {
        if (0 <= simId && simId < VT_SRV_SIM_NR) {
            return mMap.mSimHandoverTable[simId];
        } else {
            VT_LOGE("[SRV]   [MAP OPERATION] [SIM ID=%d] getHandOverStateBySim (not found)", simId);
            return VT_FALSE;
        }
    }

    status_t VTCoreHelper::setHandoverStateBySim(int simId, int state) {
        if (0 <= simId && simId < VT_SRV_SIM_NR) {
            VT_LOGD("[SRV]   [MAP OPERATION] [SIM ID=%d] setHandOverStateBySim (set state:%d)", simId, state);
            mMap.mSimHandoverTable[simId] = state;
            return VT_SRV_RET_OK;
        } else {
            VT_LOGE("[SRV]   [MAP OPERATION] [SIM ID=%d] setHandOverStateBySim (not found)", simId);
        }
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    status_t VTCoreHelper::setMode(int id, VT_SRV_CALL_MODE mode) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                VT_SRV_CALL_MODE preMode = mMap.mTable[i].mMode;

                mMap.mTable[i].mMode = mode;

                logMap("setMode", mCallModeString[mode], id, VT_SRV_LOG_I);
                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("setMode", "not found", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    VT_SRV_CALL_MODE VTCoreHelper::getMode(int id) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                //logMap("getMode", mStateString[mMap.mTable[i].mState], id, VT_SRV_LOG_I);
                return mMap.mTable[i].mMode;
            }
        }

        // cannot find a entry mapping the id
        logMap("getMode", "not found", id, VT_SRV_LOG_E);
        return VT_SRV_CALL_NONE;
    }

    status_t VTCoreHelper::setState(int id, VT_SRV_MA_STATE state) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                VT_SRV_MA_STATE preState = mMap.mTable[i].mState;

                mMap.mTable[i].mState = state;

                dump();

                logMap("setState", mStateString[state], id, VT_SRV_LOG_I);
                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("    setState", "not found", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    VT_SRV_MA_STATE VTCoreHelper::getState(int id) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                //logMap("getState", mStateString[mMap.mTable[i].mState], id, VT_SRV_LOG_I);
                return mMap.mTable[i].mState;
            }
        }

        // cannot find a entry mapping the id
        logMap("    getState", "not found", id, VT_SRV_LOG_E);
        return VT_SRV_MA_STATE_STOPED;
    }

    status_t VTCoreHelper::getParam(int id, VT_SRV_PARAM type, void** param) {
        Mutex::Autolock mapLock(mMapLock);

        if (type > VT_SRV_PARAM_END) {
            logMap("getParam", "type not exist", id, VT_SRV_LOG_E);
            return VT_SRV_RET_ERR_INVALID_PARAM;
        }

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                if (type == VT_SRV_PARAM_UA_CONFIG) {
                    VT_IMCB_CONFIG * p = mMap.mTable[i].mUaVideoConfig;
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "UA config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_MA_CONFIG) {
                    media_config_t * p = mMap.mTable[i].mMaConfig;
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "MA config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_SURFACE_STATE) {
                    VT_SRV_SURFACE_STATE * p = &(mMap.mTable[i].mSetSurfaceState);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Set surface state", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_SESSION_REQ) {
                    VT_IMCB_REQ * p = &(mMap.mTable[i].mSessionReq);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Session request", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_REMOTE_SESSION_REQ) {
                    VT_IMCB_IND * p = &(mMap.mTable[i].mRemoteSessionReq);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Remote session request", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_PEER_UI_CONFIG) {
                    vt_srv_call_ui_config_struct * p = &(mMap.mTable[i].mPeerUiConfig);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Peer UI config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_LOCAL_UI_CONFIG) {
                    vt_srv_call_ui_config_struct * p = &(mMap.mTable[i].mLocalUiConfig);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Local UI config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_FORCE_STOP) {
                    bool * p = &(mMap.mTable[i].mIsForceStop);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Is force stop", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_MA_CRASH) {
                    bool * p = &(mMap.mTable[i].mIsMaCrash);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Is MA crash", id, VT_SRV_LOG_I);
                }  else if (type == VT_SRV_PARAM_UPDATE_INFO) {
                    vt_srv_call_update_info_struct * p = &(mMap.mTable[i].mUpdateInfo);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Update info", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_CANCEL_SESSION_REQ) {
                    VT_IMCB_CANCEL_REQ * p = &(mMap.mTable[i].mSessionCancelReq);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Cancel session request", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_FORCE_CANCEL) {
                    VT_BOOL * p = &(mMap.mTable[i].mIsForceCancel);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Is force cancel", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_RECV_UPDATE) {
                    VT_BOOL * p = &(mMap.mTable[i].mIsRecvUpdate);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Is downgrade", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_CALL_STATE) {
                    int * p = &(mMap.mTable[i].mCallState);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "call state", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_DURING_EARLY_MEDIA) {
                    VT_BOOL * p = &(mMap.mTable[i].mIsDuringEarlyMedia);
                    *param = reinterpret_cast<void*>(p);
                    logMap("getParam", "Is during early media", id, VT_SRV_LOG_I);
                }
                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("    getParam", "entry not found", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    status_t VTCoreHelper::setParam(int id, VT_SRV_PARAM type, void* param) {
        Mutex::Autolock mapLock(mMapLock);

        if (type > VT_SRV_PARAM_END) {
            logMap("setParam", "type not exist", id, VT_SRV_LOG_E);
            return VT_SRV_RET_ERR_INVALID_PARAM;
        }

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                if (type == VT_SRV_PARAM_UA_CONFIG) {
                    memcpy(mMap.mTable[i].mUaVideoConfig, param, sizeof(VT_IMCB_CONFIG));
                    logMap("setParam", "UA Config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_MA_CONFIG) {
                    memcpy(mMap.mTable[i].mMaConfig, param, sizeof(media_config_t));
                    logMap("setParam", "MA Config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_SURFACE_STATE) {
                    mMap.mTable[i].mSetSurfaceState = (*(reinterpret_cast<VT_SRV_SURFACE_STATE*>(param)));
                    logMap("setParam", "Set surface state", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_SESSION_REQ) {
                    memcpy(&mMap.mTable[i].mSessionReq, param, sizeof(VT_IMCB_REQ));
                    logMap("setParam", "Session request", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_REMOTE_SESSION_REQ) {
                    memcpy(&mMap.mTable[i].mRemoteSessionReq, param, sizeof(VT_IMCB_IND));
                    logMap("setParam", "Remote session request", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_PEER_UI_CONFIG) {
                    memcpy(&mMap.mTable[i].mPeerUiConfig, param, sizeof(vt_srv_call_ui_config_struct));
                    logMap("setParam", "Peer UI config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_LOCAL_UI_CONFIG) {
                    memcpy(&mMap.mTable[i].mLocalUiConfig, param, sizeof(vt_srv_call_ui_config_struct));
                    logMap("setParam", "Peer UI config", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_FORCE_STOP) {
                    memcpy(&mMap.mTable[i].mIsForceStop, param, sizeof(bool));
                    logMap("setParam", "Is force stop", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_MA_CRASH) {
                    memcpy(&mMap.mTable[i].mIsMaCrash, param, sizeof(bool));
                    logMap("setParam", "Is MA crash", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_UPDATE_INFO) {
                    memcpy(&mMap.mTable[i].mUpdateInfo, param, sizeof(vt_srv_call_update_info_struct));
                    logMap("setParam", "Update info", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_CANCEL_SESSION_REQ) {
                    memcpy(&mMap.mTable[i].mSessionCancelReq, param, sizeof(VT_IMCB_CANCEL_REQ));
                    logMap("setParam", "Cancel session request", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_FORCE_CANCEL) {
                    memcpy(&mMap.mTable[i].mIsForceCancel, param, sizeof(VT_BOOL));
                    logMap("setParam", "Is force stop", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_RECV_UPDATE) {
                    memcpy(&mMap.mTable[i].mIsRecvUpdate, param, sizeof(VT_BOOL));
                    logMap("setParam", "Is downgrade", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_CALL_STATE) {
                    memcpy(&mMap.mTable[i].mCallState, param, sizeof(int));
                    logMap("setParam", "call state", id, VT_SRV_LOG_I);
                } else if (type == VT_SRV_PARAM_IS_DURING_EARLY_MEDIA) {
                    memcpy(&mMap.mTable[i].mIsDuringEarlyMedia, param, sizeof(VT_BOOL));
                    logMap("setParam", "Is during early media", id, VT_SRV_LOG_I);
                }

                dump();

                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("setParam", "entry not exist", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    bool VTCoreHelper::isCallConnected(int id) {
        int *callState;
        getParam(id, VT_SRV_PARAM_CALL_STATE, reinterpret_cast<void **>(&callState));
        if (*callState == VT_CALL_STATE_CONNECTED ||
            *callState == VT_CALL_STATE_ACTIVE ||
            *callState == VT_CALL_STATE_HELD ||
            *callState == VT_CALL_STATE_REMOTE_HOLD ||
            *callState == VT_CALL_STATE_REMOTE_RESUME) {
            return true;
        }
        return false;
    }

    sp<VTMALStub> VTCoreHelper::pop() {
        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (mMap.mUsed[i]) {
                return mMap.mTable[i].mPtr;
            }
        }
        return NULL;
    }

    VT_SRV_RET VTCoreHelper::checkElement(int id) {
        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                return VT_SRV_RET_OK;
            }
        }
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    void VTCoreHelper::clearElement(vt_srv_call_table_entry_struct *e) {

        logMap("clearElement", "", VT_IVD, VT_SRV_LOG_I);

        e->mId                      = VT_IVD;
        e->mSimId                       = VT_IVD;
        e->mPtr                         = NULL;
        e->mMode                        = VT_SRV_CALL_NONE;
        e->mState                       = VT_SRV_MA_STATE_STOPED;

        if (e->mUaVideoConfig != NULL) {
            delete e->mUaVideoConfig;
            e->mUaVideoConfig = NULL;
        }

        if (e->mMaConfig != NULL) {
            delete e->mMaConfig;
            e->mMaConfig = NULL;
        }

        e->mSetSurfaceState             = VT_SRV_SURFACE_STATE_INIT;

        memset(&(e->mSessionReq)        , 0, sizeof(VT_IMCB_REQ));
        e->mSessionReqProfile           = NULL;
        memset(&(e->mRemoteSessionReq)  , 0, sizeof(VT_IMCB_IND));
        memset(&(e->mSessionCancelReq)  , 0, sizeof(VT_IMCB_CANCEL_REQ));

        e->mIsForceStop                 = VT_FALSE;
        e->mIsMaCrash                   = VT_FALSE;

        e->mHasReceiveInit              = VT_FALSE;
        e->mIsForceCancel               = VT_FALSE;
        e->mIsRecvUpdate                = VT_FALSE;
        e->mCallState                   = VT_CALL_STATE_UNKNOWN;
        e->mIsDuringEarlyMedia          = VT_FALSE;
    }

    VT_SRV_RET VTCoreHelper::isMatch(int idx, int id) {
        if (mMap.mUsed[idx]) {
            if (mMap.mTable[idx].mId == id) {
                return VT_SRV_RET_OK;
            }
        }
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    void VTCoreHelper::dump(void) {

        if (isDebug()) {

            for (int i = 0; i < VT_SRV_MA_NR; i++) {
                if (-1 != mMap.mTable[i].mId) {

                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (---- MA info %d -----)", i);
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (id                %d )", mMap.mTable[i].mId);
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (SIM id            %d )", mMap.mTable[i].mSimId);
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (ptr               %d )", mMap.mTable[i].mPtr.get());
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (state             %s )", getStateString(mMap.mTable[i].mState));
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (surface state     %s )", getSurfaceString(mMap.mTable[i].mSetSurfaceState));
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (IsForceStop       %d )", mMap.mTable[i].mIsForceStop);
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (has receive init  %d )", mMap.mTable[i].mHasReceiveInit);
                    VT_LOGW("[SRV]   [MAP DUMP DATA]     (---------------------)");
                }
            }
            VT_LOGW("[SRV]   [MAP DUMP DATA]     Current call count : %d", mMap.mUsedSize);
        }
    }

    String8 VTCoreHelper::packFromVdoProfile(sp<VideoProfile> videoProfile) {
        char config[VT_SRV_STR_LEN] = "";

        snprintf(
                config,
                VT_SRV_STR_LEN,
                "mVideoState=%d;mQuality=%d",
                videoProfile->getState(),
                videoProfile->getQty());

        return String8(config);
    }

    sp<VideoProfile> VTCoreHelper::unPackToVdoProfile(String8 flattened) {
        int state = VT_SRV_VIDEO_STATE_BIDIRECTIONAL;
        int qty = VT_SRV_VIDEO_QTY_DEFAULT;

        char* pch;
        char* internal;
        char config[VT_SRV_STR_LEN] = "";

        snprintf(config, VT_SRV_STR_LEN, "%s", flattened.string());

        VT_LOGW("[SRV] [UN-PACK P]     (flattened = %s)", config);

        pch = strtok_r(config, ";", &internal);

        while (pch != NULL) {
            char* div = strstr(pch, "=");
            int len = strlen(pch);
            char key[VT_SRV_STR_LEN] = "";
            char value[VT_SRV_STR_LEN] = "";

            if (NULL == div) {
                continue;
            }

            strncpy(key, pch, static_cast<int>(div - pch));
            strncpy(value, (div + 1), len - (static_cast<int>(div - pch) + 1));

            VT_LOGW("[SRV] [UN-PACK P]     (key = %s)", key);
            VT_LOGW("[SRV] [UN-PACK P]     (value = %s)", value);

            if (!strcmp(key, "mVideoState")) {
                state = atoi(value);
            } else if (!strcmp(key, "mQuality")) {
                qty = atoi(value);
            }
            if (isDebug()) {
                VT_LOGW("[SRV] [UN-PACK P]     (state = %d)", state);
                VT_LOGW("[SRV] [UN-PACK P]     (qty = %d)", qty);
            }
            pch = strtok_r(NULL, ";", &internal);
        }

        return new VideoProfile(state, qty);
    }

    sp<VideoProfile> VTCoreHelper::getProfileFromCnf(VT_IMCB_CNF * confirm) {
            int mode = 0;

            VT_LOGW("[SRV] [getProfileFromCnf]     (mode = %d, cam = %d)", confirm->mode, confirm->camera_direction);

            if (confirm->mode == 1) {
                mode |= confirm->camera_direction;
                if (confirm->camera_direction == VT_DIR_INACTIVE) {
                    mode |= 0x4;
                }
            }

            sp<VideoProfile> Vp = new VideoProfile(mode, QUALITY_DEFAULT, confirm->result);

            return Vp;
    }

    sp<VideoProfile> VTCoreHelper::getProfileFromInd(VT_IMCB_IND * indication) {
            int mode = 0;

            VT_LOGW("[SRV] [getProfileFromInd]     (mode = %d, cam = %d)", indication->mode, indication->camera_direction);

            if (indication->mode == 1) {
                mode |= indication->camera_direction;
                if (indication->camera_direction == VT_DIR_INACTIVE) {
                    mode |= 0x4;
                }
            }

            sp<VideoProfile> Vp = new VideoProfile(mode, QUALITY_DEFAULT);

            return Vp;
    }

    void VTCoreHelper::getReqFromProfile(VT_IMCB_REQ * req, sp<VideoProfile> Vp, int id) {
        // public static final int AUDIO_ONLY = 0x0;
        // public static final int TX_ENABLED = 0x1;
        // public static final int RX_ENABLED = 0x2;
        // public static final int BIDIRECTIONAL = TX_ENABLED | RX_ENABLED;
        // public static final int PAUSED = 0x4;

        req->call_id = GET_CALL_ID(id);
        req->sim_slot_id = GET_SIM_ID(id);

        if (Vp->getState() == 0) {
            req->mode = 0;
            req->camera_direction = 0;
        } else {
            req->mode = 1;

            if ((Vp->getState() & VT_SRV_VIDEO_STATE_PAUSED)) {
                req->camera_direction = 0;
            } else {
                req->camera_direction = Vp->getState();
            }
        }
    }

    void VTCoreHelper::getRspFromProfile(VT_IMCB_RSP * rsp, sp<VideoProfile> Vp, int id) {
        sp<VTMALStub> ma = get(id);

        // public static final int AUDIO_ONLY = 0x0;
        // public static final int TX_ENABLED = 0x1;
        // public static final int RX_ENABLED = 0x2;
        // public static final int BIDIRECTIONAL = TX_ENABLED | RX_ENABLED;
        // public static final int PAUSED = 0x4;

        rsp->call_id = GET_CALL_ID(id);
        rsp->sim_slot_id = GET_SIM_ID(id);

        if (Vp->getState() == 0) {
            rsp->mode = 0;
            rsp->camera_direction = 0;
        } else {
            rsp->mode = 1;

            if ((Vp->getState() & VT_SRV_VIDEO_STATE_PAUSED)) {
                rsp->camera_direction = 0;
            } else {
                rsp->camera_direction = Vp->getState();
            }
        }
    }

    const char* VTCoreHelper::getStateStringWithId(int id) {
        VT_SRV_MA_STATE s = getState(id);
        return getStateString(s);
    }

    const char* VTCoreHelper::getStateString(int state) {
        if (VT_SRV_MA_STATE_STOPED <= state && VT_SRV_MA_STATE_END > state) {
            return mStateString[state];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getErrorString(int errorCode) {
        if (VT_SRV_RET_OK <= errorCode && VT_SRV_RET_END > errorCode) {
            return mRetString[errorCode];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getModeString(int mode) {
        if (VT_SRV_DATA_PATH_NONE <= mode && VT_SRV_DATA_PATH_END > mode) {
            return mDataModeString[mode];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getCamString(int cam) {
        if (-1 <= cam && 2 > cam) {
            return mCameraString[cam + 1];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getHideMeString(int type) {
        if (VT_SRV_HIDE_ME_DISABLE <= type && VT_SRV_HIDE_ME_END > type) {
            return mHideMeString[type];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getHideYouString(int type) {
        if (VT_SRV_HIDE_YOU_DISABLE <= type && VT_SRV_HIDE_YOU_END > type) {
            return mHideYouString[type];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getUIModeString(int type) {
        if (VT_SRV_UI_MODE_FG <= type && VT_SRV_UI_MODE_END > type) {
            return mUIModeString[type];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getSurfaceString(int state) {
        if (VT_SRV_SURFACE_STATE_INIT <= state && VT_SRV_SURFACE_STATE_END > state) {
            return mSurfaceStateString[state];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getSnapshotString(int type) {
        if (VT_SRV_SNAPSHOT_LOCAL <= type && VT_SRV_SNAPSHOT_END > type) {
            return mSnapshotString[type];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getRecordString(int type) {
        if (VT_SRV_RECORD_VIDEO <= type && VT_SRV_RECORD_END > type) {
            return mRecordString[type];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getUaMsgString(int type) {
        if (MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND == type) {
            return mMsgGetCapString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND == type) {
            return mMsgCallEndString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND == type) {
            return mMsgConfiginitString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND == type) {
            return mMsgConfigUpdateString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_REQ == type) {
            return mMsgModifySessionReqString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF == type) {
            return mMsgModifySessionCnfString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND == type) {
            return mMsgModifySessionIndString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_RSP == type) {
            return mMsgModifySessionRspString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_REQ == type) {
            return mMsgModifySessionCancelReqString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND == type) {
            return mMsgModifySessionCancelIndString;
        } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_CNF == type) {
            return mMsgModifySessionCancelCnfString;
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getVideoStateString(int state) {
        if (0 <= state && 8 > state) {
            return mVideoStateString[state];
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getCallModeString(int mode) {
       if (0 <= mode && 2 > mode) {
           return mCallTypeString[mode];
       } else {
           return mEmptyString;
       }
    }

    const char* VTCoreHelper::getCallStateString(int callState) {
        if (VT_CALL_STATE_UNKNOWN == callState) {
            return mCallStateUnknownString;
        } else if (VT_CALL_STATE_SETUP == callState) {
            return mCallStateSetupString;
        } else if (VT_CALL_STATE_ALERT== callState) {
            return mCallStateAlertString;
        } else if (VT_CALL_STATE_CONNECTED == callState) {
            return mCallStateConnectedString;
        } else if (VT_CALL_STATE_HELD == callState) {
            return mCallStateHeldString;
        } else if (VT_CALL_STATE_ACTIVE == callState) {
            return mCallStateActiveString;
        } else if (VT_CALL_STATE_DISCONNETED == callState) {
            return mCallStateDisconnetedString;
        } else if (VT_CALL_STATE_MO_DISCONNECTING == callState) {
            return mCallStateMoDisconnectingString;
        } else if (VT_CALL_STATE_REMOTE_HOLD == callState) {
            return mCallStateRemoteHoldString;
        } else if (VT_CALL_STATE_REMOTE_RESUME == callState) {
            return mCallStateRemoteResumeString;
        } else {
            return mEmptyString;
        }
    }

    const char* VTCoreHelper::getOnOffString(int state) {
       if (0 <= state && 2 > state) {
           return mOnOffString[state];
       } else {
           return mEmptyString;
       }
    }

    const char* VTCoreHelper::getLocalCameraStateString(int state) {
       if (0 <= state && 2 > state) {
           return mLocalCameraStateString[state];
       } else {
           return mEmptyString;
       }
    }

    const char* VTCoreHelper::getPeerCameraStateString(int state) {
       if (0 <= state && 2 > state) {
           return mPeerCameraStateString[state];
       } else {
           return mEmptyString;
       }
    }

    const char* VTCoreHelper::getHOString(bool state) {
       if (VT_FALSE == state) {
           return mHOStateString[0];
       } else if (VT_TRUE == state) {
           return mHOStateString[1];
       } else {
           return mEmptyString;
       }
    }

    const char* VTCoreHelper::getSessionModifyResultString(int result) {
        if (0 <= result && 11 > result) {
            return mSessionModifyResultString[result];
        } else {
            return mEmptyString;
        }
    }

    VT_SRV_RET VTCoreHelper::checkRange(int value, int start, int end) {
        if (value >= start && value <= end) {
            return VT_SRV_RET_OK;
        } else {
            VT_LOGE("[SRV] [checkRange]     %d not in [%d, %d]", value, start, end);
            return VT_SRV_RET_ERR_INVALID_PARAM;
        }
    }

    VT_BOOL VTCoreHelper::isDebug() {
        char prop_val[100] = {0};

        property_get("persist.vendor.log.tag.tel_dbg", prop_val, "0");

        if (atoi(prop_val) == 1) {
            return VT_TRUE;
        } else {
            return VT_FALSE;
        }
    }

    VT_BOOL VTCoreHelper::isImsVideoCallon() {
        char vilte_prop_val[100] = {0};
        char viwifi_prop_val[100] = {0};

        property_get("persist.vendor.vilte_support", vilte_prop_val, "0");
        property_get("persist.vendor.viwifi_support", viwifi_prop_val, "0");

        if (atoi(vilte_prop_val) == 1 || atoi(viwifi_prop_val) == 1) {
            VT_LOGD("[SRV] isImsVideoCallon : True");
            return VT_TRUE;
        } else {
            VT_LOGD("[SRV] isImsVideoCallon : False");
            return VT_FALSE;
        }
    }

    VT_BOOL VTCoreHelper::isIMCBrun() {
        char prop_val[100] = {0}; // "stopped" or "running"

        property_get("init.svc.vendor.volte_imcb", prop_val, "running");

        if (!strcmp(prop_val, "running")) {
            VT_LOGD("[SRV] isIMCBrun : True");
            return VT_TRUE;
        } else {
            VT_LOGD("[SRV] isIMCBrun : False");
            return VT_FALSE;
        }
    }

    VT_BOOL VTCoreHelper::isVoLTEon() {
        char prop_val[100] = {0};

        property_get("persist.vendor.volte_support", prop_val, "0");

        if (atoi(prop_val) == 1) {
            VT_LOGD("[SRV] isVoLTEon : True");
            return VT_TRUE;
        } else {
            VT_LOGD("[SRV] isVoLTEon : False");
            return VT_FALSE;
        }
    }

    void VTCoreHelper::getStringProperty(const char* key, char* prop_val) {
        property_get(key, prop_val, "0");
        VT_LOGD("[SRV] getStringProperty: %s = %s, length is %d", key, prop_val, strlen(prop_val));
    }

    int VTCoreHelper::getIntProperty(const char* key) {
        char prop_val[100] = {0};
        property_get(key, prop_val, "0");
        VT_LOGD("[SRV] getIntProperty: %s = %d", key, atoi(prop_val));
        return atoi(prop_val);
    }

    VT_BOOL VTCoreHelper::isCancelRequest(sp<VideoProfile> videoProfile) {
        VT_BOOL isCancel = (VT_BOOL) (videoProfile->getState() == VT_SRV_VIDEO_STATE_CANCEL ||
            videoProfile->getState() == VT_SRV_VIDEO_STATE_CANCEL_TIMEOUT);
        return isCancel;
    }

    sp<VideoProfile> VTCoreHelper::getProfileFromCancelInd(VT_IMCB_CANCEL_IND * ind) {
        sp<VideoProfile> Vp = new VideoProfile(VT_SRV_VIDEO_STATE_CANCEL, QUALITY_DEFAULT);
        return Vp;
    }

    sp<VideoProfile> VTCoreHelper::getProfileFromCancelCnf(VT_IMCB_CANCEL_CNF * cnf) {
        sp<VideoProfile> Vp = new VideoProfile(VT_SRV_VIDEO_STATE_CANCEL, QUALITY_DEFAULT, cnf->result);
        return Vp;
    }

    sp<VideoProfile> VTCoreHelper::geteEarlyMediaProfileFromState(int state) {
        sp<VideoProfile> Vp = new VideoProfile(state, QUALITY_DEFAULT);
        return Vp;
    }

    int VTCoreHelper::getLabOperator() {
        return getIntProperty("persist.vendor.vt.lab_op_code");
    }

    int VTCoreHelper::getCurrentOperator(int simId) {

        VT_LOGI("[SRV] getCurrentOperator start");

        int multiPs = getIntProperty("ro.vendor.mtk_data_config");
        int op = 0;
        int labOp = getLabOperator();
        char operatorPropetyString[50] = "persist.vendor.radio.sim.opid";

        if (0 != simId) {
            snprintf(operatorPropetyString, 50, "%s_%d", "persist.vendor.radio.sim.opid", simId);
        }

        VT_LOGI("[SRV] getCurrentOperator operatorPropetyString : %s", operatorPropetyString);
        VT_LOGI("[SRV] getCurrentOperator multiPs : %d", multiPs);
        VT_LOGI("[SRV] getCurrentOperator labOp : %d", labOp);

        // If lab OP exist,
        // it mean we use test sim and cannot know which operator is from mnc/mcc directly
        // Need to check special property which is setup in EM
        if (0 != labOp) {
            return labOp;
        }

        // If sim state absent, return default OP
        if (VT_SIM_ID_ABSENT == simId) {
            VT_LOGI("[SRV] getCurrentOperator finish : SIM absent, return default op code = %d", VT_DEFAULT_OP_ID);
            return VT_DEFAULT_OP_ID;
        }

        // In legacy MD, if multiPs is off, will only update op code on "persist.radio.sim.opid"
        // so the property we got will not exist forever
        char prop_val[100] = {0};
        property_get(operatorPropetyString, prop_val, "-1");
        int isOpproperCodeExist = atoi(prop_val);
        int count = 0;

        do {
            // multiPs on
            if (multiPs == 1) {

                op = getIntProperty(operatorPropetyString);

            // multiPs off
            } else {

                if (-1 == isOpproperCodeExist) {
                    op = getIntProperty("persist.radio.sim.opid");
                } else {
                    op = getIntProperty(operatorPropetyString);
                }
            }

            // Only non-legacy MD will provide this value
            if (op == VT_NO_MATCHED_OP_ID) {
                VT_LOGI("[SRV] getCurrentOperator cannot match opid:%d, return default op code = %d", op, VT_DEFAULT_OP_ID);
                op = VT_DEFAULT_OP_ID;
                break;
            }

            usleep(500 * 1000);
            count++;

        } while (op == 0 && count <= 10);

        // SIM state is loaded but still cannot get operator id (id = 0) for 5 secs.
        // It means MCC/MNC is not found in mapping table, directly use default operator id
        if (op == 0) {
            VT_LOGI("[SRV] getCurrentOperator cannot match opid:%d, return default op code = %d", op, VT_DEFAULT_OP_ID);
            op = VT_DEFAULT_OP_ID;
        }

        VT_LOGI("[SRV] getCurrentOperator finish : op code = %d", op);

        return op;
    }

    void VTCoreHelper::setDefaultLocalSize(int w, int h) {
        Mutex::Autolock mapLock(mMapLock);
        mMap.mDefaultLocalW = w;
        mMap.mDefaultLocalH = h;
    }

    void VTCoreHelper::getDefaultLocalSize(int *w, int *h) {
        Mutex::Autolock mapLock(mMapLock);
        *w = mMap.mDefaultLocalW;
        *h = mMap.mDefaultLocalH;
    }

    void VTCoreHelper::setIsSetSensorInfo(bool isSet) {
        Mutex::Autolock mapLock(mMapLock);
        mMap.mIsSetSensorInfo = isSet;
    }

    bool VTCoreHelper::getIsSetSensorInfo() {
        Mutex::Autolock mapLock(mMapLock);
        return mMap.mIsSetSensorInfo;
    }

    status_t VTCoreHelper::setRequestVdoProfile(int id, sp<VideoProfile> vdo_profile) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                mMap.mTable[i].mSessionReqProfile = vdo_profile;

                logMap("setRequestVdoProfile",
                        getVideoStateString(vdo_profile->getState()),
                        id, VT_SRV_LOG_I);
                return VT_SRV_RET_OK;
            }
        }

        // cannot find a entry mapping the id
        logMap("    setRequestVdoProfile", "not found", id, VT_SRV_LOG_E);
        return VT_SRV_RET_ERR_NOT_EXIST;
    }

    sp<VideoProfile> VTCoreHelper::getRequestVdoProfile(int id) {
        Mutex::Autolock mapLock(mMapLock);

        for (int i = 0; i < VT_SRV_MA_NR; i++) {
            if (VT_SRV_RET_OK == isMatch(i, id)) {
                logMap("getRequestVdoProfile",
                        getVideoStateString(mMap.mTable[i].mSessionReqProfile->getState()),
                        id, VT_SRV_LOG_I);
                return mMap.mTable[i].mSessionReqProfile;
            }
        }

        // cannot find a entry mapping the id
        logMap("    getRequestVdoProfile", "not found", id, VT_SRV_LOG_E);
        return mDummyRequestProfile;
    }

    int VTCoreHelper::getNetworkInfoIndex(int network_id) {
        // check if the table is empty
        if (mMap.mNetworkInfoTable.size() == 0) {
            logMap("checkNetworkInfRecordExist", "table empty", network_id, VT_SRV_LOG_E);
            return VT_INVALID_INDEX;
        }

        for (int i = 0; i < mMap.mNetworkInfoTable.size(); i++) {
            if (mMap.mNetworkInfoTable[i].mNetworkId == network_id) {
                logMap("checkNetworkInfRecordExist", "entry exist", network_id, VT_SRV_LOG_E);
                return i;
            }
        }

        // cannot find a entry mapping the id
        logMap("check", "entry not exist", network_id, VT_SRV_LOG_E);
        return VT_INVALID_INDEX;
    }

    void VTCoreHelper::addNetworkInfoRecord(int network_id, String8 if_name) {
        Mutex::Autolock mapLock(mMapLock);

        vt_srv_network_info_struct net_info;
        net_info.mNetworkId = network_id;
        snprintf(net_info.mIfName, VT_MAX_IF_NAME_LENGTH, "%s", if_name.string());

        int idx = getNetworkInfoIndex(network_id);

        if (idx == VT_INVALID_INDEX) {
            VT_LOGI("[SRV]   [MAP OPERATION]  [ID=%d] AddNetworkInfoRecord ifname=%s",
                    network_id, (char *)if_name.string());

            mMap.mNetworkInfoTable.push_back(net_info);

        } else {
            VT_LOGI("[SRV]   [MAP OPERATION] [ID=%d] UpdateNetworkInfoRecord ifname=%s",
                    network_id, (char *)if_name.string());

            mMap.mNetworkInfoTable.replaceAt(net_info, idx);
        }
    }

    void VTCoreHelper::removeNetworkInfoRecord(int network_id) {
        Mutex::Autolock mapLock(mMapLock);

        int idx = getNetworkInfoIndex(network_id);

        if (idx == VT_INVALID_INDEX) {
            logMap("RemoveNetworkInfoRecord", "fail, not exist", network_id, VT_SRV_LOG_I);

        } else {
            logMap("RemoveNetworkInfoRecord", "Success", network_id, VT_SRV_LOG_I);
            mMap.mNetworkInfoTable.erase(mMap.mNetworkInfoTable.begin()+idx);
        }
    }

    int VTCoreHelper::getNetworkIdByIfName(String8 if_name) {
        Mutex::Autolock mapLock(mMapLock);

        // check if the table is empty
        if (mMap.mNetworkInfoTable.size() == 0) {
            logMap("checkNetworkInfRecordExist", "table empty", VT_IVD, VT_SRV_LOG_E);
            return VT_INVALID_NETWORK_ID;
        }

        for (int i = 0; i < mMap.mNetworkInfoTable.size(); i++) {
            if (!strcmp(mMap.mNetworkInfoTable[i].mIfName, if_name.string())) {
                logMap("checkNetworkInfRecordExist", "entry exist", mMap.mNetworkInfoTable[i].mNetworkId, VT_SRV_LOG_I);
                return mMap.mNetworkInfoTable[i].mNetworkId;
            }
        }

        // cannot find a entry mapping the if_name
        logMap("check", "entry not exist", VT_IVD, VT_SRV_LOG_E);
        return VT_INVALID_NETWORK_ID;
    }

    void VideoTelephonyDeathReceiver::serviceDied(uint64_t cookie, const wp<IBase>& who) {
            VT_LOGW("serviceDied start: %d", cookie);

            int ret = pthread_kill(g_vt_hidl_thd, SIGUSR1);
            if (!ret) {
                VT_LOGI("kill VT_HIDL_Thread success, wait for leaving...");
                usleep(500 * 1000);
                g_vt_hidl_thd = NULL;
                g_vt.hidlService = NULL;

                VT_LOGI("Recreate VT_HIDL_Thread");
                VT_Connect(g_vt_callback);
            } else {
                VT_LOGW("Cannot stop VT_HIDL_Thread, ret:%d (%s)", ret, strerror(ret));
            }

            VT_LOGW("serviceDied finish");
    }

    int VT_Connect(VT_Callback cb) {

        VT_LOGI("[SRV] [VT THREAD] VT_Connect start");

        if (NULL == g_vt_srv_msg_hdr.mDispatcher.get()) {
            g_vt_srv_msg_hdr.mDispatcher = new VTMsgDispatcher(cb);
        } else {
             VT_LOGI("[SRV] [VT THREAD] VTMsgDispatcher already existed");
        }

        g_vt_callback = cb;

        // Thread for HiDL message
        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&g_vt_hidl_thd, &(attr), VT_HIDL_Thread, NULL);
        pthread_attr_destroy(&attr);
        if (ret < 0) {
            VT_LOGE("[SRV] [VT THREAD] Fail to create thread");
            return VT_SRV_RET_ERR;
        }

        VT_LOGI("[SRV] [VT THREAD] VT_Connect finish");

        return VT_SRV_RET_OK;
    }


    /* send VT configuration to VT HiDL */
    int VT_Send(int target, int type, void *data, int len) {

        VT_LOGI("[SRV] [VT THREAD] VT_Send start, type = %d, len:%d", type, len);

        Mutex::Autolock sendLock(g_vt_SendMsgLock);

        if (NULL == g_vt_hidl_write_fmq.get()) {
            VT_LOGI("[SRV] [VT THREAD] VT_Send fmq not ready");
            return VT_SRV_RET_ERR;
        }

        android::hardware::EventFlag* efGroup = nullptr;
        std::atomic<uint32_t> mFw;
        android::status_t status = android::hardware::EventFlag::createEventFlag(&mFw, &efGroup);

        VT_LOGI("[SRV] [VT THREAD] VT_Send before type");

        g_vt_hidl_write_fmq->writeBlocking(
                (uint8_t*)&type,
                4,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        VT_LOGI("[SRV] [VT THREAD] VT_Send before len");

        g_vt_hidl_write_fmq->writeBlocking(
                (uint8_t*)&len,
                4,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        VT_LOGI("[SRV] [VT THREAD] VT_Send before data");

        g_vt_hidl_write_fmq->writeBlocking(
                (uint8_t*)data,
                len,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        status = android::hardware::EventFlag::deleteEventFlag(&efGroup);

        VT_LOGI("[SRV] [VT THREAD] VT_Send end");

        return VT_SRV_RET_OK;
    }

    static void VT_Bind() {

        VT_LOGI("[SRV] [VT THREAD] VT_Bind start");

        if (!VTCoreHelper::isImsVideoCallon()) {
            return;
        }

        // get HiDL
        g_vt.hidlService = IVideoTelephony::getService();

        while (NULL == g_vt.hidlService.get()) {
            usleep(500 * 1000);
            VT_LOGW("[SRV] [THREAD] get hild fail, try again");
            g_vt.hidlService = IVideoTelephony::getService();
        }
        VT_LOGI("[SRV] [THREAD] get hild success");

        if (NULL == g_vt_hidl_death_receiver.get()) {
            g_vt_hidl_death_receiver = new VideoTelephonyDeathReceiver();
        }
        g_vt.hidlService->linkToDeath(g_vt_hidl_death_receiver, 1);

        // HiDL FMQ config
        g_vt.hidlService->configureFmqMode(1);
        g_vt.hidlService->configureFmqSyncRead(VT_Config_ReaderQ_Callback);
        g_vt.hidlService->configureFmqSyncWrite(VT_Config_WriterQ_Callback);

        VT_LOGI("[SRV] [VT THREAD] VT_Bind start finish");

    }

    static void VT_Config_ReaderQ_Callback(bool ret, const vtHiDLFmq::Descriptor& mqDesc) {

        if(ret == true) {

            g_vt_hidl_read_fmq.reset(new vtHiDLFmq(mqDesc));
            if (!g_vt_hidl_read_fmq->isValid()) {
                VT_LOGI("[SRV] [VT CONFIG] Create Reader Q fail");
            }

        } else {
            VT_LOGI("[SRV] [VT CONFIG] Get Reader Q fail");
        }
    }


    static void VT_Config_WriterQ_Callback(bool ret, const vtHiDLFmq::Descriptor& mqDesc) {

        if(ret == true) {

            g_vt_hidl_write_fmq.reset(new vtHiDLFmq(mqDesc));
            if (!g_vt_hidl_write_fmq->isValid()) {
                VT_LOGI("[SRV] [VT CONFIG] Create Writer Q fail");
            }

        } else {
            VT_LOGI("[SRV] [VT CONFIG] Get Writer Q fail");
        }
    }

    static void VT_HIDL_Thread_Exit_Handler(int sig) {
        VT_LOGI("[SRV] [VT THREAD] [VT_HIDL_Thread] this signal is %d, exit thread \n", sig);
        pthread_exit(0);
    }

    static void *VT_HIDL_Thread(void *arg) {

        if (!VTCoreHelper::isImsVideoCallon()) {
            return 0;
        }

        VT_Bind();

        // Create SIGUSR1 signal handle to leave thread
        struct sigaction actions;
        memset(&actions, 0, sizeof(actions));
        sigemptyset(&actions.sa_mask);
        actions.sa_flags = 0;
        actions.sa_handler = VT_HIDL_Thread_Exit_Handler;
        sigaction(SIGUSR1,&actions,NULL);

        while (1) {

            int msg_type;
            int recv_length = 0;
            unsigned char* outBuffer = NULL;
            bool ret;

            VT_LOGD("[SRV] [VT THREAD] [VT_HIDL_Thread] read from IVT");

            android::hardware::EventFlag* efGroup = nullptr;
            std::atomic<uint32_t> mFw;
            android::status_t status = android::hardware::EventFlag::createEventFlag(&mFw, &efGroup);

            ret = g_vt_hidl_read_fmq->readBlocking(
                    (uint8_t*) (&msg_type),
                    4,
                    static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                    static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                    VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                    efGroup*/);

            if (!ret) {
                VT_LOGE("[SRV] [VT THREAD] [VT_HIDL_Thread] recv fail (type)");

                if (NULL != g_vt.core.get() && NULL != g_vt.hidlService.get()) {
                    g_vt.core->clearAll();
                }

                restartProcess();
            }

            VT_LOGI("[SRV] [VT THREAD] [VT_HIDL_Thread] msg_type : %d", msg_type);

            ret = g_vt_hidl_read_fmq->readBlocking(
                    (uint8_t*) (&recv_length),
                    4,
                    static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                    static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                    VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                    efGroup*/);

            if (!ret) {
                VT_LOGE("[SRV] [VT THREAD] [VT_HIDL_Thread] recv fail (len)");

                if (NULL != g_vt.core.get() && NULL != g_vt.hidlService.get()) {
                    g_vt.core->clearAll();
                }

                restartProcess();
            }

            VT_LOGI("[SRV] [VT THREAD] [VT_HIDL_Thread] recv_length : %d", recv_length);

            outBuffer = (unsigned char* ) malloc((sizeof(char) * recv_length) + 1);

            ret = g_vt_hidl_read_fmq->readBlocking(
                    (uint8_t*) (outBuffer),
                    recv_length,
                    static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                    static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                    VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                    efGroup*/);

            if (!ret) {
                VT_LOGE("[SRV] [VT THREAD] [VT_HIDL_Thread] recv fail (data)");

                if (NULL != g_vt.core.get() && NULL != g_vt.hidlService.get()) {
                    g_vt.core->clearAll();
                }

                free(outBuffer);
                restartProcess();
            }

            VT_LOGI("[SRV] [VT THREAD] [VT_HIDL_Thread] outBuffer");

            g_vt_srv_msg_hdr.mDispatcher->dispatchMessage(msg_type, outBuffer, recv_length);

            free(outBuffer);

            status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
        }

        VT_LOGI("[SRV] [VT THREAD] [VT_HIDL_Thread] end");

        return 0;

    }

    static void restartProcess(void) {
        return;
    }
}
