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
#include <Surface.h>
#include "VTClient.h"

extern void notify(
    int id,
    int msgType,
    int arg1,
    int arg2,
    int arg3,
    const char* obj1,
    const char* obj2,
    const sp<IGraphicBufferProducer>& obj3);

namespace VTService {

    sp<IVTClient> gVTClient;

    void VTClient::DeathNotifier::binderDied(const wp<IBinder>& who) {
        Mutex::Autolock _l(VTClient::mLock);

        VT_LOGE("[VTC] binderDied");

        RFX_UNUSED(who);

        if (gVTClient.get() != 0) {
            gVTClient->notifyCallback(
                    -1,
                    VT_SRV_ERROR_SERVER_DIED,
                    0,
                    0,
                    0,
                    String8(""),
                    String8(""),
                    NULL);
        }
        VTClient::mVTService.clear();
    }

    sp<IVTService> VTClient::mVTService;
    Mutex VTClient::mLock;
    sp<VTClient::DeathNotifier> VTClient::mDeathNotifier;

    void VTClient::binderDied(const wp<IBinder>& who) {

        RFX_UNUSED(who);

        notifyCallback(
                -1,
                VT_SRV_ERROR_SERVER_DIED,
                0,
                0,
                0,
                String8(""),
                String8(""),
                NULL);
    }

    const sp<IVTService>& VTClient::getVTService() {
        VT_LOGD("[VTC] getVTService+");

        Mutex::Autolock _l(mLock);
        if (mVTService.get() == 0) {
            sp<IServiceManager> sm = defaultServiceManager();
            sp<IBinder> binder;
            do {
                VT_LOGI("[VTC] before getVTService");
                binder = sm->getService(String16("media.VTS"));
                VT_LOGI("[VTC] after getVTService");
                if (binder != 0)
                    break;
                usleep(500000);  // 0.5 s
            } while (true);

            if (mDeathNotifier == NULL) {
                VT_LOGI("[VTC] Add DeathNotifier");
                mDeathNotifier = new DeathNotifier();
            }
            binder->linkToDeath(mDeathNotifier);
            mVTService = interface_cast<IVTService>(binder);
        }
        VT_LOGD("[VTC] getVTService-");
        return mVTService;
    }

    sp<VTClient> VTClient::getVTClient() {

        VT_LOGD("[VTC] getVTClient");

        sp<VTClient> c = new VTClient();
        gVTClient= c;
        c->mStatus = UNKNOWN_ERROR;
        c->mVTService = getVTService();
        if (c->mVTService != 0) {
            VT_LOGD("[VTC] mVTService != 0");
            c->mVTService->setupVTService(gVTClient);
        } else {
            VT_LOGD("[VTC] mVTService == 0");
            c.clear();
            VT_LOGD("[VTC] clear client");
        }
        return c;
    }

    void VTClient::releaseVTClient() {

        VT_LOGD("[VTC] releaseVTClient");

        if (mVTService != 0) {
            mVTService->releaseVTService();
            mVTService = 0;
        }
        gVTClient = NULL;
    }

    status_t VTClient::initialization(int mode, int id, int sim_id) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] initialization : no service");
            return NO_INIT;
        }

        return c->initialization(mode, id, sim_id);
    }

    status_t VTClient::finalization(int id) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] finalization : no service");
            return NO_INIT;
        }

        return c->finalization(id);
    }

    status_t VTClient::setCamera(int id, int cam) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setCamera : no service");
            return NO_INIT;
        }

        return c->setCamera(id, cam);
    }

    status_t VTClient::setPreviewSurface(int id, const sp<VTSurface> & surface) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setPreviewSurface : no service");
            return NO_INIT;
        }

        return c->setPreviewSurface(id, surface);
    }

    status_t VTClient::setDisplaySurface(int id, const sp<VTSurface> & surface) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setDisplaySurface : no service");
            return NO_INIT;
        }

        return c->setDisplaySurface(id, surface);
    }

    status_t VTClient::setCameraParameters(int sim_id, int sensorCnt, sensor_info_vilte_t *sensor) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setCameraParameters : no service");
            return NO_INIT;
        }

        return c->setCameraParameters(sim_id, sensorCnt, sensor);
    }

    status_t VTClient::setCameraParametersWithSim(int sim_id, int major_sim_id, int sensorCnt, sensor_info_vilte_t *sensor) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setCameraParametersWithSim : no service");
            return NO_INIT;
        }

        return c->setCameraParametersWithSim(sim_id, major_sim_id, sensorCnt, sensor);
    }

    status_t VTClient::setCameraParametersOnly(int sensorCnt, sensor_info_vilte_t *sensor) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setCameraParametersOnly : no service");
            return NO_INIT;
        }

        return c->setCameraParametersOnly(sensorCnt, sensor);
    }

    status_t VTClient::setDeviceOrientation(int id, int rotation) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setDeviceOrientation : no service");
            return NO_INIT;
        }

        return c->setDeviceOrientation(id, rotation);
    }

    status_t VTClient::setUIMode(int id, VT_SRV_UI_MODE mode) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] setUIMode : no service");
            return NO_INIT;
        }

        return c->setUIMode(id, mode);
    }

    status_t VTClient::requestSessionModify(int id, const String8 &  config) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] requestSessionModify : no service");
            return NO_INIT;
        }

        return c->requestSessionModify(id, config);
    }

    status_t VTClient::responseSessionModify(int id, const String8 &  config) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] responseSessionModify : no service");
            return NO_INIT;
        }

        return c->responseSessionModify(id, config);
    }

    status_t VTClient::snapshot(int id, VT_SRV_SNAPSHOT_TYPE type, String8 savingImgURI) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] snapshot : no service");
            return NO_INIT;
        }

        return c->snapshot(id, type, savingImgURI);
    }

    status_t VTClient::startRecording(int id, VT_SRV_RECORD_TYPE type, String8 path, int maxSize) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] startRecording : no service");
            return NO_INIT;
        }

        return c->startRecording(id, type, path, maxSize);
    }

    status_t VTClient::stopRecording(int id) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] stopRecording : no service");
            return NO_INIT;
        }

        return c->stopRecording(id);
    }

    status_t VTClient::switchFeature(int id, int feature, int isOn) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] stopRecording : no service");
            return NO_INIT;
        }

        return c->switchFeature(id, feature, isOn);
    }

    status_t VTClient::updateNetworkTable(bool is_add, int network_id, String8 if_name) {
        sp<IVTService> c = mVTService;
        if (c == 0) {
            VT_LOGI("[VTC] updateNetworkTable : no service");
            return NO_INIT;
        }

        return c->updateNetworkTable(is_add, network_id, if_name);
    }

    void VTClient::notifyCallback(
            int32_t id,
            int32_t msgType,
            int32_t arg1,
            int32_t arg2,
            int32_t arg3,
            const String8 & obj1,
            const String8 & obj2,
            const sp<IGraphicBufferProducer> & obj3) {
        VT_LOGI("[VTC] notifyCallback");
        notify(
            id,
            msgType,
            arg1,
            arg2,
            arg3,
            obj1.string(),
            obj2.string(),
            obj3);
    }
}
