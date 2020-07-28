/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "test_GenericContainer"

#include <atomic>
#include <chrono>
#include <sstream>
#include <string>
#include <thread>

#include <cutils/compiler.h>

#include <mtkcam/utils/hw/IGenericContainer.h>
#include <mtkcam/utils/std/Log.h>

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
struct MyData
{
    size_t value;
};
using ContainerDataT = std::shared_ptr<MyData>;
using ContainerT = NSCam::IGenericContainer<ContainerDataT>;


/******************************************************************************
 *
 ******************************************************************************/
static std::string toString(MyData const& o)
{
    return std::to_string(o.value);
}

static std::string toString(std::shared_ptr<MyData> const& o)
{
    return toString(*o);
}

template <class T>
static std::string toString(std::list<T> const& o)
{
    std::string os;
    os += "[" + std::to_string(o.size()) + "]";
    os += "{";
    for (auto const& v : o) {
        os += " ";
        os += toString(v);
    }
    os += " }";
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
static std::atomic_bool gTesting{false};


/******************************************************************************
 *
 ******************************************************************************/
void reader(uint64_t timestamp, uint32_t subkey)
{
    // 1. Make a reader or a writer with the same 'timestamp' and 'subkey' for retrieving the same container.
    MY_LOGI("+ timestamp:%#" PRIx64 " subkey:%#x", timestamp, subkey);
    std::shared_ptr<ContainerT::Reader> reader = ContainerT::makeReader("Reader"/*for debug*/, timestamp, subkey);
    MY_LOGE_IF(reader==nullptr, "Bad reader");

    while ( gTesting )
    {
        std::ostringstream msg;
        {
            // 2. Read the latest one from the container.
            ContainerDataT data;
            if ( reader->cloneBack(data) ) {
                msg << " Clone the latest=" << toString(data);
            }
            else {
                msg << " Clone nothing" << "(size:" << reader->size() << " capacity:" << reader->capacity() << ")";
            }
        }
        {
            std::list<ContainerDataT> data;
            if ( reader->clone(data) ) {
                msg << " Clone all=" << toString(data);
            }
            else {
                msg << " Clone nothing" << "(size:" << reader->size() << " capacity:" << reader->capacity() << ")";
            }
        }
        MY_LOGI("%s", msg.str().c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    MY_LOGI("- timestamp:%#" PRIx64 " subkey:%#x", timestamp, subkey);
}


/******************************************************************************
 *
 ******************************************************************************/
void writer(uint64_t timestamp, uint32_t subkey)
{
    // 1. Make a reader or a writer with the same 'timestamp' and 'subkey' for retrieving the same container.
    MY_LOGI("+ timestamp:%#" PRIx64 " subkey:%#x", timestamp, subkey);
    std::shared_ptr<ContainerT::Writer> writer = ContainerT::makeWriter("Writer"/*for debug*/, timestamp, subkey);
    MY_LOGE_IF(writer==nullptr, "Bad writer");

    // 2. Reserve the max. number of data in the container
    writer->reserve(3);
    MY_LOGI("capacity:%zu size:%zu", writer->capacity(), writer->size());

    for (size_t i = 0; gTesting != false; i++) {
        // 3. Writer data to the container.
        auto data = std::make_shared<MyData>(MyData{.value = i,});
        writer->push_back(data);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    MY_LOGI("- timestamp:%#" PRIx64 " subkey:%#x", timestamp, subkey);
}


/******************************************************************************
 *
 ******************************************************************************/
int main(int /*argc*/, char** /*argv*/)
{
    android::LogPrinter printer(LOG_TAG);

    uint64_t const timestamps[] = {123456, 999999};

    MY_LOGI("+\n");

    // create Writer -> create Reader -> 2sec -> exit
    {
        int64_t timestamp = timestamps[0];
        const uint32_t ContainerSubKey = static_cast<uint32_t>(NSCam::IGenericContainerKey::VENDOR_START);

        MY_LOGI("+ TEST timestamp:%s", std::to_string(timestamp).c_str());

        gTesting = true;
        std::thread threadWriter(writer, timestamp, ContainerSubKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::thread threadReader(reader, timestamp, ContainerSubKey);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        gTesting = false;
        threadReader.join();
        threadWriter.join();

        if (auto manager = NSCam::IGenericContainerManager::get()) {
            manager->dumpState(printer);
        }
        MY_LOGI("- TEST timestamp:%s", std::to_string(timestamp).c_str());
    }

    // create Reader -> create Writer -> 2sec -> exit
    {
        int64_t timestamp = timestamps[1];
        const uint32_t ContainerSubKey = static_cast<uint32_t>(NSCam::IGenericContainerKey::VENDOR_START) + 0x100;

        MY_LOGI("+ TEST timestamp:%s", std::to_string(timestamp).c_str());

        gTesting = true;
        std::thread threadReader(reader, timestamp, ContainerSubKey);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::thread threadWriter(writer, timestamp, ContainerSubKey);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        gTesting = false;
        threadReader.join();
        threadWriter.join();

        if (auto manager = NSCam::IGenericContainerManager::get()) {
            manager->dumpState(printer);
        }
        MY_LOGI("- TEST timestamp:%s", std::to_string(timestamp).c_str());
    }

    {
        MY_LOGI("cleanup");
        if (auto manager = NSCam::IGenericContainerManager::get()) {
            for (size_t i = 0; i < sizeof(timestamps)/sizeof(timestamps[0]) ; i++) {
                manager->cleanup(
                    NSCam::IGenericContainerManager::Key{.k0=timestamps[i],}
                );
            }
            manager->dumpState(printer);
        }
    }

    MY_LOGI("-\n");
    return 0;
}

