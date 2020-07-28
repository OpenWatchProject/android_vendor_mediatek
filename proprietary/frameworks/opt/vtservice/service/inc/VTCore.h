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
#ifndef VT_CORE_H
#define VT_CORE_H

#include "VTDef.h"
#include "IVTService.h"
#include "IVTClient.h"
#include "IVTInterface.h"
#include "VTCoreHelper.h"
#include "VTMALStub.h"
#include "VTAVSync.h"

// for HiDL
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

namespace VTService {

void vt_callback(int type, void *data, int len);
void vt_rtp_codec_2_ua(int mode, VT_IMCB_CAP * ua, int ssid, int opid);
void vt_ua_2_rtp(int mode, VT_IMCB_CONFIG * ua, rtp_rtcp_config_t * rtp);
void vt_ua_2_codec(int mode, VT_IMCB_CONFIG * ua, video_codec_fmtp_t * codec);
void vt_ut_ua_2_rtp(int mode, VT_IMCB_CAP * ua, rtp_rtcp_config_t * rtp);
void vt_ut_ua_2_codec(int mode, VT_IMCB_CAP * ua, video_codec_fmtp_t * codec);

class IVTServiceCallBack:virtual public RefBase{
 public:
  virtual sp<IVTClient> getClient()=0;
  virtual void notifyCallback(
          int32_t id,
          int32_t msgType,
          int32_t arg1,
          int32_t arg2,
          int32_t arg3,
          const String8 & obj1,
          const String8 & obj2,
          const sp<IGraphicBufferProducer> & obj3)=0;
};

class VTCore: public IVTInterface, virtual public RefBase{
 public:
  explicit VTCore(const sp<IVTServiceCallBack> & user);
  VTCore();
  virtual ~VTCore();

  DECLARE_IVTINTERFACE

  status_t open(VT_SRV_CALL_MODE mode, const int id, const int sim_id);
  status_t init(const int id);
  status_t init_internal(const int id);
  status_t update(const int id);
  status_t updateCallMode(const int id);
  status_t updateTxRxMode(const int id, int new_mode, vt_srv_call_update_info_struct *info);
  status_t deinit(const int id);
  status_t close(const int id, int close_mode);
  status_t close_internal(const int id, int close_mode);
  status_t clearAll(void);
  status_t setHandoverState(bool state);
  status_t setHandoverStateBySimId(int sim_id, bool state);
  status_t setHandoverStateByNetworkId(int network_id, bool state);
  status_t requestCancelSessionModify(const int id, sp<VideoProfile> Vp);

  void notifyError(int id, const char* action);

  virtual void notifyCallback(
          int32_t id,
          int32_t msgType,
          int32_t arg1,
          int32_t arg2,
          int32_t arg3,
          const String8 & obj1,
          const String8 & obj2,
          const sp<IGraphicBufferProducer> & obj3);

  virtual void notifyCallback(
          int32_t id,
          int32_t msgType,
          int32_t arg1,
          int32_t arg2);

  void notifyCallback(
          int32_t id,
          int32_t msgType);

  void getUpdateInfo(int id, VT_IMCB_UPD* pre_config, VT_IMCB_UPD* new_config);
  void resetUpdateInfo(int id);

  sp<IVTServiceCallBack>                  mUser;
  static sp<VTCoreHelper>                 mHelper;

  unsigned int                            mUserID;

 private:
  int                                     mLastRet      = VT_SRV_RET_ERR;
  int                                     mLastNotify   = VT_SRV_ERROR_SERVICE;

  // The map hold the locks for open/init/start/stop/close VTService, App may call them at the same time
  // The different call has the dedicated different lock
  Mutex*                                  mCallFlowLocks[VT_SRV_MA_NR];
  // Lock for Notifycallback(), multiple MA may pass notify to VTcore at the same time
  mutable Mutex                           mNotifyLock;
  // Lock for Notifycallback(), only for error notify event
  mutable Mutex                           mNotifyErrorLock;
  // Lock for MA operation which need to sync each others bwtween MAs
  mutable Mutex                           mMAOperationLock;

  Mutex* getCallFlowLock(int id);
  status_t retriggerUpdate(int id);
};

}  // namespace VTService

#endif
