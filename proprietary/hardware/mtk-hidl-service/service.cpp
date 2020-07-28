/*
 * Copyright 2016 The Android Open Source Project
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

#define LOG_TAG "mtk-hidl-service"

#include <android/hardware/memtrack/1.0/IMemtrack.h>
#include <android/hardware/vibrator/1.0/IVibrator.h>
//#include <android/hardware/light/2.0/ILight.h>
#include <android/hardware/thermal/1.0/IThermal.h>
#include <hidl/LegacySupport.h>
#include <log/log.h>
#ifdef MTK_GPS_SUPPORT
#include <lbs_hidl_service.h>
#include <android/hardware/gnss/2.0/IGnss.h>
#endif
#include <android/hardware/graphics/allocator/2.0/IAllocator.h>
#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>

using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::memtrack::V1_0::IMemtrack;
using ::android::hardware::vibrator::V1_0::IVibrator;
//using ::android::hardware::light::V2_0::ILight;
using ::android::hardware::thermal::V1_0::IThermal;
#ifdef MTK_GPS_SUPPORT
using ::android::hardware::gnss::V2_0::IGnss;
#endif
using ::android::hardware::graphics::allocator::V2_0::IAllocator;
using ::vendor::mediatek::hardware::gpu::V1_0::IGraphicExt;

using ::android::hardware::registerPassthroughServiceImplementation;
using ::android::OK;
using ::android::status_t;


#define register(service) do { \
    status_t err = registerPassthroughServiceImplementation<service>(); \
    if (err != OK) { \
        ALOGE("Err %d while registering " #service, err); \
    } \
} while(false)



void* mtkHidlService(void *data)
{
    int ret = 0;
    ALOGI("mtkHidlService - data:%p", data);

    configureRpcThreadpool(1, true /* will call join */);

    register(IMemtrack);

    register(IVibrator);
    //register(ILight);
    register(IThermal);

    register(IAllocator);
    register(IGraphicExt);

#ifdef MTK_GPS_SUPPORT
    register(IGnss);
    ::vendor::mediatek::hardware::lbs::V1_0::implementation::cpp_main();
#endif

    joinRpcThreadpool();

    ALOGI("mtkHidlService - ret:%d", ret);

    return NULL;
}

int main() {
    ALOGE("Threadpool start run process!");
    pthread_t daemonThread, serviceThread;
    pthread_attr_t attr;

    /* service */
    pthread_attr_init(&attr);
    pthread_create(&serviceThread, &attr, mtkHidlService, NULL);

    pthread_join(serviceThread, NULL);

    ALOGE("Threadpool exited!!!");
    return -1;
}
