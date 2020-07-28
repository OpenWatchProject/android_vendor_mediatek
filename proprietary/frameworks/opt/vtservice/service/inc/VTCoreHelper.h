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
#ifndef VT_CORE_HELPER_H
#define VT_CORE_HELPER_H

#include "VTDef.h"
#include "IVTService.h"
#include "IVTClient.h"
#include "IVTInterface.h"

// for UT
#include <cutils/properties.h>

// for HiDL
#include <hidl/MQDescriptor.h>
#include <fmq/MessageQueue.h>
#include <vendor/mediatek/hardware/videotelephony/1.0/IVideoTelephony.h>

using namespace android;

// for HiDL
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::videotelephony::V1_0::IVideoTelephony;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::MessageQueue;
using ::android::hardware::kSynchronizedReadWrite;
using ::android::hardware::kUnsynchronizedWrite;
using ::android::hardware::MQDescriptorSync;
using ::android::hardware::MQDescriptorUnsync;
using ::android::hardware::hidl_death_recipient;

namespace VTService {

class VideoProfile: virtual public RefBase {
 public:
  VideoProfile(int state, int qty) {
      mQty = qty;
      mState = state;
      mResult = VT_SRV_IMCB_SESSION_MODIFY_OK;
  }

  VideoProfile(int state, int qty, unsigned char result) {
      mQty = qty;
      mState = state;
      mResult = result;
  }

  int getQty() {
      return mQty;
  }

  int getState() {
      return mState;
  }

  int getResult() {
      return mResult;
  }

  virtual ~VideoProfile() {}

 private:
  int mQty;
  int mState;
  unsigned char mResult;
};

struct VideoTelephonyDeathReceiver : hidl_death_recipient {
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
};

class VTCoreHelper: virtual public RefBase {
 public:
  VTCoreHelper();
  virtual ~VTCoreHelper() {};

  // Log related operation
  void logString(
          const char * str,
          VT_SRV_LOG_LV level);
  void logAction(
          const char * action,
          const char * des,
          int data,
          const char * data2,
          VT_SRV_LOG_LV level);
  void logFlow(
          const char * flow,
          const char * des,
          int data,
          const char * data2,
          VT_SRV_LOG_LV level);
  void logMap(
          const char * action,
          const char * des,
          int data,
          VT_SRV_LOG_LV level);
  void logMsg(
          int notify,
          const char * des,
          int id,
          int data1,
          int data2,
          int data3,
          const char* obj1,
          const char* obj2,
          VT_SRV_LOG_LV level);

  // Map related operation
  int getUsedNr();
  status_t init(void);
  status_t add(int id, sp<VTMALStub> ma_ptr);
  status_t del(int id);
  sp<VTMALStub> get(int id);
  sp<VTMALStub> getFromIndex(int idx);
  int getId(int idx);
  status_t check(int id);
  int isUsed(int idx);

  int getOperatorId(int simId);
  status_t setOperatorId(int simId, int opId);

  int getHandoverStateBySim(int simId);
  status_t setHandoverStateBySim(int simId, int state);

  status_t setMode(int id, VT_SRV_CALL_MODE mode);
  VT_SRV_CALL_MODE getMode(int id);
  status_t setState(int id, VT_SRV_MA_STATE state);
  VT_SRV_MA_STATE getState(int id);
  status_t getParam(int id, VT_SRV_PARAM type, void** param);
  status_t setParam(int id, VT_SRV_PARAM type, void* param);
  bool isCallConnected(int id);
  sp<VTMALStub> pop();

  String8 packFromVdoProfile(sp<VideoProfile> VideoProfile);
  sp<VideoProfile> unPackToVdoProfile(String8 flattened);

  sp<VideoProfile> getProfileFromCnf(VT_IMCB_CNF * request);
  sp<VideoProfile> getProfileFromInd(VT_IMCB_IND * indication);
  void getReqFromProfile(VT_IMCB_REQ * req, sp<VideoProfile> Vp, int id);
  void getRspFromProfile(VT_IMCB_RSP * rsp, sp<VideoProfile> Vp, int id);

  const char* getStateStringWithId(int id);
  const char* getStateString(int state);
  const char* getErrorString(int errorCode);
  const char* getModeString(int mode);
  const char* getCamString(int cam);
  const char* getHideMeString(int type);
  const char* getHideYouString(int type);
  const char* getUIModeString(int type);
  const char* getSurfaceString(int state);
  const char* getSnapshotString(int type);
  const char* getRecordString(int type);
  const char* getUaMsgString(int type);
  const char* getVideoStateString(int mode);
  const char* getCallModeString(int mode);
  const char* getCallStateString(int callState);
  const char* getOnOffString(int state);
  const char* getLocalCameraStateString(int state);
  const char* getPeerCameraStateString(int state);
  const char* getHOString(bool state);
  const char* getSessionModifyResultString(int result);

  VT_SRV_RET checkRange(int value, int start, int end);

  static VT_BOOL isDebug();

  static VT_BOOL isImsVideoCallon();
  static VT_BOOL isDynamicImsVideoCallon();
  static VT_BOOL isIMCBrun();
  static VT_BOOL isVoLTEon();

  static int getIntProperty(const char* key);
  static void getStringProperty(const char* key, char* prop_val);

  VT_BOOL isCancelRequest(sp<VideoProfile> Vp);
  sp<VideoProfile> getProfileFromCancelInd(VT_IMCB_CANCEL_IND * ind);
  sp<VideoProfile> getProfileFromCancelCnf(VT_IMCB_CANCEL_CNF * cnf);
  sp<VideoProfile> geteEarlyMediaProfileFromState(int state);

  int getLabOperator();
  int getCurrentOperator(int simId);

  void setDefaultLocalSize(int w, int h);
  void getDefaultLocalSize(int *w, int *h);

  void setIsSetSensorInfo(bool isSet);
  bool getIsSetSensorInfo();

  status_t setRequestVdoProfile(int id, sp<VideoProfile> vdo_profile);
  sp<VideoProfile> getRequestVdoProfile(int id);


  void addNetworkInfoRecord(int network_id, String8 if_name);
  void removeNetworkInfoRecord(int network_id);
  int getNetworkInfoIndex(int network_id);
  int getNetworkIdByIfName(String8 if_name);

 private:
  const char *mEmptyString = "";
  const char mCallModeString[VT_SRV_CALL_END][50] = {
      "Call mode : None",
      "Call mode : 3G",
      "Call mode : 4G"
  };
  const char mStateString[VT_SRV_MA_STATE_END][50] = {
      "Stoped state",
      "Opened state",
      "Inited state",
      "Pre-Started state",
      "Started state (Up : init / Down : pause)",
      "Started state (Up : pause / Down : init)",
      "Started state (Up : init / Down : start)",
      "Started state (Up : start / Down : init)",
      "Started state (Up : pause / Down : pause)",
      "Started state (Up : pause / Down : start)",
      "Started state (Up : start / Down : pause)",
      "Started state (Up : start / Down : start)",
      "Pre-Stop state",
      "Stoped video state (Up : start / Down : init)",
      "Stoped video state (Up : init / Down : start)",
      "Stoped voice state (Up : start / Down : open)",
      "Stoped voice state (Up : open / Down : start)"
  };
  const char mRetString[VT_SRV_RET_END][50] = {
      "Operation success",
      "Operation error : not found match MA)",
      "Operation error : state error",
      "Operation error : tabel is full",
      "Operation error : table has empty",
      "Operation error : entry has exist",
      "Operation error : entry has not exist",
      "Operation error : invalide param",
      "Operation error : No client",
      "Operation error : MA error",
      "Operation error : common error"
  };
  const char mDataModeString[VT_SRV_DATA_PATH_END][50] = {
      "Path : none",
      "Path : sink",
      "Path : source",
      "Path : source sink"
  };
  const char mCameraString[3][50] = {
      "Turn off camera",
      "camera id 0",
      "camera id 1"
  };
  const char mHideMeString[VT_SRV_HIDE_ME_END][50] = {
      "Hide me : Disable",
      "Hide me : Enable",
      "Hide me : Freeze",
      "Hide me : Picture"
  };
  const char mHideYouString[VT_SRV_HIDE_YOU_END][50] = {
      "Hide you : Enable",
      "Hide you : Disable"
  };
  const char mUIModeString[VT_SRV_UI_MODE_END][50] = {
      "foreground",
      "background"
  };
  const char mSurfaceStateString[VT_SRV_SURFACE_STATE_END][50] = {
      "Surface state : Both not set",
      "Surface state : Set local only",
      "Surface state : Set peer only",
      "Surface state : Both set"
  };
  const char mSnapshotString[VT_SRV_SNAPSHOT_END][50] = {
      "Snapshot local",
      "Snapshot peer",
      "Snapshot local & peer"
  };
  const char mRecordString[VT_SRV_RECORD_END][50] = {
      "Record video",
      "Record video / audio"
  };
  const char mVideoStateString[8][50] = {
      "Video State : audio",
      "Video State : video (Tx)",
      "Video State : video (Rx)",
      "Video State : video (TxRx)",
      "Video State : video (Pause)",
      "Video State : video (Tx | Pause)",
      "Video State : video (Rx | Pause)",
      "Video State : video (TxRx | Pause)"
  };
  const char mCallTypeString[2][50] = {
      "Call Mode : voice",
      "Call Mode : video"
  };
  const char mOnOffString[2][50] = {
      "OFF",
      "ON"
  };
  const char mLocalCameraStateString[2][50] = {
      "Local Camera State : ON",
      "Local Camera State : OFF"
  };
  const char mPeerCameraStateString[2][50] = {
      "Peer Camera State : ON",
      "Peer Camera State : OFF"
  };
  const char mHOStateString[2][50] = {
      "False",
      "True"
  };
  const char mSessionModifyResultString[11][50] = {
      "Success",
      "Fail : Wrong Video Dir",
      "Fail : Internal Error",
      "Fail : BW Modify Failed",
      "Fail : Not Active State",
      "Fail : Local Rel",
      "Fail : Is Hold",
      "Fail : No Need",
      "Fail : Invalid Para",
      "Fail : Time out",
      "Fail : Reject by remote"
  };

  const char *mMsgGetCapString           = "get capability request";
  const char *mMsgCallEndString          = "Call end";
  const char *mMsgConfiginitString       = "config init";
  const char *mMsgConfigUpdateString     = "config update";
  const char *mMsgModifySessionReqString = "modify session request";
  const char *mMsgModifySessionCnfString = "modify session confirm";
  const char *mMsgModifySessionIndString = "modify session indication";
  const char *mMsgModifySessionRspString = "modify session response";
  const char *mMsgModifySessionCancelReqString = "modify session cancel request";
  const char *mMsgModifySessionCancelIndString = "modify session cancel indication";
  const char *mMsgModifySessionCancelCnfString = "modify session cancel confirm";

  const char *mCallStateUnknownString         = "call state unknown";
  const char *mCallStateSetupString           = "call state setup";
  const char *mCallStateAlertString           = "call state alert";
  const char *mCallStateConnectedString       = "call state connected";
  const char *mCallStateHeldString            = "call state held";
  const char *mCallStateActiveString          = "call state active";
  const char *mCallStateDisconnetedString     = "call state disconnected";
  const char *mCallStateMoDisconnectingString = "call state MO disconnecting";
  const char *mCallStateRemoteHoldString      = "call state remote hold";
  const char *mCallStateRemoteResumeString    = "call state remote resume";

  VT_SRV_RET checkElement(int id);
  void clearElement(vt_srv_call_table_entry_struct *e);
  VT_SRV_RET isMatch(int idx, int id);
  VT_BOOL isHelding(int oldState, int newState);
  VT_BOOL isActiving(int oldState, int newState);

  void dump(void);

  int                             mConSceFd;

  sp<VTMALStub>                   mDummyMa;
  sp<VideoProfile>                mDummyRequestProfile;

  mutable Mutex                   mMapLock;

  vt_srv_call_table_struct        mMap;
};

static void restartProcess(void);

static void VT_Bind();

static void *VT_HIDL_Thread(void *arg);

static VT_Callback g_vt_callback;

static pthread_t  g_vt_hidl_thd = 0;
static sp<VideoTelephonyDeathReceiver> g_vt_hidl_death_receiver;

typedef MessageQueue<uint8_t, kSynchronizedReadWrite> vtHiDLFmq;

static void VT_Config_ReaderQ_Callback(bool ret, const vtHiDLFmq::Descriptor& mqDesc);
static void VT_Config_WriterQ_Callback(bool ret, const vtHiDLFmq::Descriptor& mqDesc);

static std::unique_ptr<vtHiDLFmq>   g_vt_hidl_write_fmq;
static std::unique_ptr<vtHiDLFmq>   g_vt_hidl_read_fmq;

}

#endif
