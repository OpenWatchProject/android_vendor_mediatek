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
#ifndef IVT_INTERFACE_H
#define IVT_INTERFACE_H

#include "VTDef.h"
#include <Surface.h>
#include <binder/IMemory.h>
#include <utils/String8.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

using namespace android;

namespace VTService {

class IVTInterface: virtual public RefBase {
 public:
  virtual status_t initialization(int mode, int id, int sim_id)=0;
  virtual status_t finalization(int id)=0;
  virtual status_t setCamera(int id, int cam)=0;
  virtual status_t setPreviewSurface(int id, const sp<VTSurface> & surface)=0;
  virtual status_t setDisplaySurface(int id, const sp<VTSurface> & surface)=0;
  virtual status_t setCameraParameters(int sim_id, int sensorCnt, sensor_info_vilte_t *sensor)=0;
  virtual status_t setCameraParametersOnly(int sensorCnt, sensor_info_vilte_t *sensor)=0;
  virtual status_t setCameraParametersWithSim(int sim_id, int major_sim_id, int sensorCnt, sensor_info_vilte_t *sensor)=0;
  virtual status_t setDeviceOrientation(int id, int rotation)=0;
  virtual status_t setUIMode(int id, VT_SRV_UI_MODE mode)=0;

  virtual status_t requestSessionModify(int id, const String8 &  config)=0;
  virtual status_t responseSessionModify(int id, const String8 &  config)=0;

  virtual status_t snapshot(int id, VT_SRV_SNAPSHOT_TYPE type, String8 savingImgURI)=0;
  virtual status_t startRecording(int id, VT_SRV_RECORD_TYPE type, String8 path, int maxSize)=0;
  virtual status_t stopRecording(int id)=0;

  virtual status_t switchFeature(int id, int feature, int isOn)=0;
  virtual status_t updateNetworkTable(bool is_add, int network_id, String8 if_name)=0;

  virtual ~IVTInterface(){};
// ### You don't need a ; after a }
};

class IVTNotifyCallback: virtual public RefBase {
 public:
  virtual void notifyCallback(
      int id,
      int msg,
      int arg1,
      int arg2,
      int arg3,
      const String8 & obj1,
      const String8 & obj2,
      const sp<IGraphicBufferProducer> & obj3)=0;

  virtual ~IVTNotifyCallback(){};
// ### You don't need a ; after a }
};

#define DECLARE_IVTINTERFACE \
    virtual status_t initialization(int mode, int id, int sim_id); \
    virtual status_t finalization(int id); \
    virtual status_t setCamera(int id, int cam); \
    virtual status_t setPreviewSurface(int id, const sp<VTSurface> & surface); \
    virtual status_t setDisplaySurface(int id, const sp<VTSurface> & surface); \
    virtual status_t setCameraParameters(int sim_id, int sensorCnt, sensor_info_vilte_t *sensor); \
    virtual status_t setCameraParametersOnly(int sensorCnt, sensor_info_vilte_t *sensor); \
    virtual status_t setCameraParametersWithSim(int sim_id, int major_sim_id, int sensorCnt, sensor_info_vilte_t *sensor); \
    virtual status_t setDeviceOrientation(int id, int rotation); \
    virtual status_t setUIMode(int id, VT_SRV_UI_MODE mode); \
    virtual status_t requestSessionModify(int id, const String8 &  config); \
    virtual status_t responseSessionModify(int id, const String8 &  config); \
    virtual status_t switchFeature(int id, int feature, int isOn); \
    virtual status_t snapshot(int id, VT_SRV_SNAPSHOT_TYPE type, String8 savingImgURI); \
    virtual status_t startRecording(int id, VT_SRV_RECORD_TYPE type, String8 path, int maxSize); \
    virtual status_t stopRecording(int id); \
    virtual status_t updateNetworkTable(bool is_add, int network_id, String8 if_name);
}  // namespace VTService

#endif
