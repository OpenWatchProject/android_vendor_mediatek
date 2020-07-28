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
#include "IVTClient.h"
#include "IVTInterface.h"

namespace VTService {

enum {
    TRANS_BEGIN = IBinder::FIRST_CALL_TRANSACTION,
    NOTIFY_CALLBACK,
};

class BpVTClient: public BpInterface<IVTClient> {
 public:
  explicit BpVTClient(const sp<IBinder>& impl):BpInterface<IVTClient>(impl) {}

  virtual void notifyCallback(
          int32_t id,
          int32_t msgType,
          int32_t arg1,
          int32_t arg2,
          int32_t arg3,
          const String8 & obj1,
          const String8 & obj2,
          const sp<IGraphicBufferProducer> & obj3) {
      Parcel data, reply;
      data.writeInterfaceToken(IVTClient::getInterfaceDescriptor());
      data.writeInt32(id);
      data.writeInt32(msgType);
      data.writeInt32(arg1);
      data.writeInt32(arg2);
      data.writeInt32(arg3);
      data.writeString8(obj1);
      data.writeString8(obj2);
      data.writeStrongBinder(obj3->asBinder(obj3));
      remote()->transact(NOTIFY_CALLBACK, data, &reply);
  }
};

IMPLEMENT_META_INTERFACE(VTClient, "android.hardware.IVTClient");

status_t BnVTClient::onTransact(uint32_t code, const Parcel& data, Parcel * reply, uint32_t flags) {
    switch (code) {
        case NOTIFY_CALLBACK: {
            CHECK_INTERFACE(IVTClient, data, reply);
            int32_t id = data.readInt32();
            int32_t msgType = data.readInt32();
            int32_t arg1 = data.readInt32();
            int32_t arg2 = data.readInt32();
            int32_t arg3 = data.readInt32();
            const String8 obj1(data.readString8());
            const String8 obj2(data.readString8());
            sp<IGraphicBufferProducer> obj3
                    = interface_cast<IGraphicBufferProducer> (data.readStrongBinder());

            notifyCallback(
                    id,
                    msgType,
                    arg1,
                    arg2,
                    arg3,
                    obj1,
                    obj2,
                    obj3);

            return NO_ERROR;
        }
        break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}
