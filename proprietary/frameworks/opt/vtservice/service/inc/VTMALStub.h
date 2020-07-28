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

#ifndef VT_MAL_STUB_H
#define VT_MAL_STUB_H

#include "VTDef.h"
#include "VTCore.h"

// for 4G VT
#include "ImsMa.h"

using namespace android;

namespace VTService {

class VTMALStub: virtual public RefBase {
 public:
  static int MA_INVALID_MODE;
  static int MA_PASS_MODE;
  static int MA_NORMAL_MODE_3G;
  static int MA_NORMAL_MODE_4G;

  VTMALStub(int mode, int id, int sim_id, int opid, const sp<VTCore>& core);
  virtual ~VTMALStub(){};

  status_t Init(ma_datapath_t type);
  status_t Start(ma_datapath_t type);
  status_t Stop(ma_datapath_t type);
  status_t Pause(ma_datapath_t type,imsma_pause_resume_params_t* params);
  status_t Resume(ma_datapath_t type,imsma_pause_resume_params_t* params);
  status_t Reset(ma_datapath_t type);
  status_t InitMediaConfig(media_config_t *config);
  status_t UpdateMediaConfig(media_config_t *config);
  status_t UpdateCodecConfig(uint8_t id, video_codec_fmtp_t *video_fmtp);
  status_t SetCameraParameters(const String8& params);
  status_t SetCameraSensor(int32_t index);
  status_t SetPeerSurface(const sp<Surface>& peer_surface);
  status_t getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer);
  status_t SetLocalSurface(const sp<Surface>& local_surface);
  status_t setUIMode(int isFg);
  status_t setHandoverState(bool state);
  status_t SnapShot(const char* url, snapshot_mode_t mode);
  status_t SnapShot(sp<IMemory>& buffer, snapshot_mode_t mode);
  imsma_turn_off_video_mode_t GetTurnOffVideoByLocalState();
  imsma_turn_off_video_mode_t GetTurnOffVideoByPeerState();
  status_t SetRecordParameters(record_quality_t quality,  char* file_name);
  status_t StartRecord(record_mode_t mode);
  status_t StopRecord(void);
  status_t SetVTQuality(VT_Quality quality);
  status_t setDeviceRotationDegree(int32_t degree);
  status_t notifyRanBitRate(bool isUL,int32_t bitrate);
  status_t setRANEnable(bool enable);

  int getId() {return mId;};
  int getSimId() {return mSimId;};
  int getMode() {return mMode;};

  sp<VTCoreHelper> getLogger();


 private:

  sp<ImsMa>                 mImsMa;

  sp<VTCore>                mVTCore;

  sp<MsgHandler>            mMsgHdlr;

  int                       mId;
  int                       mSimId;
  int                       mOpId;
  int                       mMode;

  sp<VTSurface>             mPeerSurface;
  sp<VTSurface>             mLocalSurface;

  Mutex                     mLock;

  int32_t                   mCurDegree;
};

struct ImsMaHandler: public AHandler {
 public:
  ImsMaHandler(sp<ImsMa> owner, sp<VTCore> user, int id);
  virtual ~ImsMaHandler(){};
  void finalization(void);
  void exchangeMsg();

 protected:
  virtual void onMessageReceived(const sp<AMessage> &msg);
  void reply(const sp<AMessage> &msg);

 private:
  sp<VTCore>                mUser;
  sp<ImsMa>                 mMa;
  int                       mId;
  sp<ALooper>               mLooper;
  bool                      mDataResumeSent;

  DISALLOW_EVIL_CONSTRUCTORS(ImsMaHandler);
};

class MsgHandler: virtual public RefBase {

  public:
   MsgHandler(sp<VTCore> user, int id, sp<ImsMa> imsMa, int mode);
   ~MsgHandler(){};
   void finalization(void);
   void exchangeMsg();

  private:
   sp<ImsMaHandler>         mImsHdlr;
   int                      mMode;
};

}
#endif
