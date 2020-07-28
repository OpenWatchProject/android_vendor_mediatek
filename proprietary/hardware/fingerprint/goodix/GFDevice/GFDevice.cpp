/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gf_fingerprint.h"
#include "GFDevice.h"

#define LOG_TAG "GFDevice"

namespace android {

    GFDevice* GFDevice::sInstance = NULL;

    GFDevice::GFDevice():
    mFingerprintBase(GoodixFingerprintBase()),
    mFingerprintExt(GoodixFingerprintExt()),
    mFingerprintTest(GoodixFingerprintTest()){
    }

    int32_t GFDevice::openHal() {
        mDevice = new gf_fingerprint_device_t();
        if (mDevice == NULL)
        {
            ALOGE("out of memory, g_device is NULL");
            return -EINVAL;
        }

        mDevice->notify = this->notify;

        if (GF_SUCCESS != gf_hal_open(mDevice)) {
            ALOGE("open goodix hal failed");
            delete(mDevice);
            mDevice = NULL;
            return -EINVAL;
        }
        return GF_SUCCESS;
    }

    int32_t GFDevice::closeHal() {
        ALOGE("fingerprint_close");
        gf_hal_close(mDevice);

        if(mDevice != NULL) {
            delete(mDevice);
            mDevice = NULL;
        }

        return GF_SUCCESS;
    }


    void GFDevice::notify(const gf_fingerprint_msg_t *msg){
	 
         gf_fingerprint_msg_t message = *msg;
	 message.type = msg->type;
	 switch (message.type) {
		 case GF_FINGERPRINT_ERROR:
		 case GF_FINGERPRINT_ACQUIRED:
		 case GF_FINGERPRINT_AUTHENTICATED:
                 case GF_FINGERPRINT_TEMPLATE_ENROLLING:
                 case GF_FINGERPRINT_TEMPLATE_REMOVED:
                 case GF_FINGERPRINT_ENUMERATED:
                      GoodixFingerprintBase::notify(&message);
			 break;
		 default:
			 ALOGE("invalid msg type: %u", message.type);
			 return;
        }

    }

    IGoodixFingerprintBase* GFDevice::getFingerprintBase(void) {
        return &mFingerprintBase;
    }

    IGoodixFingerprintTest* GFDevice::getFingerprintTest(void) {
        return &mFingerprintTest;
    }

    IGoodixFingerprintExt* GFDevice::getFingerprintExt(void) {
        return &mFingerprintExt;
    }
} // namespace android
