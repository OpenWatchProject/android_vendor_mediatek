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

#ifndef IGOODIX_GFDEVICE_H_
#define IGOODIX_GFDEVICE_H_

#include <errno.h>
#include <string.h>
#include <iostream>
#include <cutils/log.h>
#include <utils/RefBase.h>
#include <memory>
#include "IGoodixFingerprint.h"

namespace android {


class GFDevice {
    public:
        static GFDevice* getInstance()
        {
            if (sInstance == NULL)
            {
                sInstance = new GFDevice();
            }

            return sInstance;
        }

        IGoodixFingerprintBase* getFingerprintBase(void);
        IGoodixFingerprintTest* getFingerprintTest(void);
        IGoodixFingerprintExt* getFingerprintExt(void);
        int32_t openHal();
        int32_t closeHal();
        static void notify(const gf_fingerprint_msg_t *msg);
    private:
        explicit GFDevice();
        static GFDevice* sInstance;
        GoodixFingerprintBase mFingerprintBase;
        GoodixFingerprintTest mFingerprintTest;
        GoodixFingerprintExt mFingerprintExt;
        gf_fingerprint_device_t *mDevice;
};

} // namespace android

#endif // IGOODIX_GFDEVICE_H_

