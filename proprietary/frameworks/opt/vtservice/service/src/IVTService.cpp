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
#include <utils/Log.h>
#include <stdint.h>
#include "IVTService.h"
#include "VTDef.h"

namespace VTService {

class BpVTService: public BpInterface<IVTService> {
 public:
  explicit BpVTService(const sp<IBinder> & impl)
      :BpInterface<IVTService>(impl) {
  }

  status_t setupVTService(const sp<IVTClient> & client) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeStrongBinder(client->asBinder(client));
      remote()->transact(SETUP_SERVICE, data, &reply);
      return reply.readInt32();
  }

  void releaseVTService() {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      remote()->transact(RELEASE_SERVICE, data, &reply);
  }

  status_t initialization(int mode, int id, int sim_id) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(mode);
      data.writeInt32(id);
      data.writeInt32(sim_id);
      remote()->transact(INITIALIZATION, data, &reply);
      return reply.readInt32();
  }

  status_t finalization(int id) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      remote()->transact(FINALIZATION, data, &reply);
      return reply.readInt32();
  }

  status_t setCamera(int id, int cam) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(cam);
      remote()->transact(SET_CAMERA, data, &reply);
      return reply.readInt32();
  }

  status_t setPreviewSurface(int id, const sp<VTSurface> & surface) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      if (surface != NULL) {
          data.writeInt32(id | 0x100);
          sp<IGraphicBufferProducer> localGraphicBuffer = surface->getIGraphicBufferProducer();
          data.writeStrongBinder(localGraphicBuffer->asBinder(localGraphicBuffer));
      } else {
          data.writeInt32(id);
      }
      remote()->transact(SET_LOCAL_SURFACE, data, &reply);
      return reply.readInt32();
  }

  status_t setDisplaySurface(int id, const sp<VTSurface> & surface) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      if (surface != NULL) {
          data.writeInt32(id | 0x100);
          sp<IGraphicBufferProducer> peerGraphicBuffer = surface->getIGraphicBufferProducer();
          data.writeStrongBinder(peerGraphicBuffer->asBinder(peerGraphicBuffer));
      } else {
          data.writeInt32(id);
      }
      remote()->transact(SET_PEER_SURFACE, data, &reply);
      return reply.readInt32();
  }

  status_t setCameraParameters(int sim_id, int sensorCnt, sensor_info_vilte_t *sensor) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(sim_id);
      data.writeInt32(sensorCnt);
      if (sensorCnt != 0) {
          data.write(sensor, sizeof(sensor_info_vilte_t)*sensorCnt);
      }
      remote()->transact(SET_CAMERA_PARAM, data, &reply);
      return reply.readInt32();
  }

  status_t setCameraParametersWithSim(int sim_id, int major_sim_id, int sensorCnt, sensor_info_vilte_t *sensor) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(sim_id);
      data.writeInt32(major_sim_id);
      data.writeInt32(sensorCnt);
      if (sensorCnt != 0) {
          data.write(sensor, sizeof(sensor_info_vilte_t)*sensorCnt);
      }
      remote()->transact(SET_CAMERA_PARAM_WITH_SIM, data, &reply);
      return reply.readInt32();
  }

  status_t setCameraParametersOnly(int sensorCnt, sensor_info_vilte_t *sensor) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(sensorCnt);
      if (sensorCnt != 0) {
          data.write(sensor, sizeof(sensor_info_vilte_t)*sensorCnt);
      }
      remote()->transact(SET_CAMERA_PARAM_ONLY, data, &reply);
      return reply.readInt32();
  }

  status_t setDeviceOrientation(int id, int rotation) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(rotation);
      remote()->transact(SET_ROTATION, data, &reply);
      return reply.readInt32();
  }

  status_t setUIMode(int id, VT_SRV_UI_MODE mode) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(mode);
      remote()->transact(SET_UI_MODE, data, &reply);
      return reply.readInt32();
  }

  status_t requestSessionModify(int id, const String8 & config) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeString8(config);
      remote()->transact(REQ_SESSION_MODIFY, data, &reply);
      return reply.readInt32();
  }

  status_t responseSessionModify(int id, const String8 & config) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeString8(config);
      remote()->transact(RES_SESSION_MODIFY, data, &reply);
      return reply.readInt32();
  }

  status_t snapshot(int id, VT_SRV_SNAPSHOT_TYPE type, String8 savingImgURI) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(type);
      data.writeString8(savingImgURI);
      remote()->transact(SNAPSHOT, data, &reply);
      return reply.readInt32();
  }

  status_t startRecording(int id, VT_SRV_RECORD_TYPE type, String8 path, int maxSize) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(type);
      data.writeString8(path);
      data.writeInt32(maxSize);
      remote()->transact(START_RECORD, data, &reply);
      return reply.readInt32();
  }

  status_t stopRecording(int id) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      remote()->transact(STOP_RECORD, data, &reply);
      return reply.readInt32();
  }

  status_t switchFeature(int id, int feature, int isOn) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(feature);
      data.writeInt32(isOn);
      remote()->transact(SWITCH_FEATURE, data, &reply);
      return reply.readInt32();
  }

  status_t updateNetworkTable(bool is_add, int network_id, String8 if_name) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTService::getInterfaceDescriptor());
      data.writeBool(is_add);
      data.writeInt32(network_id);
      data.writeString8(if_name);
      remote()->transact(UPDATE_NETWORK_TABLE, data, &reply);
      return reply.readInt32();
  }

};

IMPLEMENT_META_INTERFACE(VTService, "android.hardware.IVTService");

status_t BnVTService::onTransact(
        uint32_t code,
        const Parcel& data,
        Parcel * reply,
        uint32_t flags) {
    return BBinder::onTransact(code, data, reply, flags);
}

}  // namespace VTService
