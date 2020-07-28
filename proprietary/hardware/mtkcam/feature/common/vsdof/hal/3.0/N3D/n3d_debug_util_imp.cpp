/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "N3DDebugUtil"

#include "n3d_debug_util_imp.h"
#include <lz4.h>
#include <thread>
#include <mtkcam/utils/std/Log.h>
#include <vsdof/hal/ProfileUtil.h>

#include <mutex>
#include <condition_variable>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

//NOTICE: property has 31 characters limitation
//vendor.STEREO.log.hal.n3d [0: disable] [1: enable]
#define LOG_PERPERTY  PROPERTY_ENABLE_LOG".hal.n3d_debug"
#define DUMP_PROPERTY "vendor.STEREO.n3d_debug_dump"

#define SINGLE_LINE_LOG 0
#if (1==SINGLE_LINE_LOG)
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD(fmt, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI(fmt, ##arg)
#else
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD("[%s]" fmt, __func__, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI("[%s]" fmt, __func__, ##arg)
#endif
#define FAST_LOGW(fmt, arg...)  __fastLogger.FastLogW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define FAST_LOGE(fmt, arg...)  __fastLogger.FastLogE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define FAST_LOG_PRINT  __fastLogger.print()

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START          MY_LOGD_IF(__LOG_ENABLED, "+")
#define FUNC_END            MY_LOGD_IF(__LOG_ENABLED, "-")

#define INIT_INFO_TAG   "InitInfo"
#define PROC_INFO_TAG   "ProcInfo"
#define RESULT_TAG      "Result"

using namespace rapidjson;

Mutex N3DDebugUtil::__instanceLock;
N3DDebugUtil *N3DDebugUtil::__pvInstance = NULL;
N3DDebugUtil *N3DDebugUtil::__capInstance = NULL;

Mutex N3DDebugUtilImp::__ioLock;

mutex               __mtx;
condition_variable  __cv;

// Defined in n3d_hal_kernel.dump.cpp
extern void __addIntValue(Value &v, const char *key, int value, Document::AllocatorType& allocator);
extern void __addFloatValue(Value &v, const char *key, float value, Document::AllocatorType& allocator);

bool __compress(const char *name, char *buf, size_t &len)
{
    bool isCompressed = false;
    int maxDstSize = LZ4_compressBound(len);
    char *dst = new char[maxDstSize];
    size_t dstSize = 0;
    dstSize = LZ4_compress_default(buf, dst, len, maxDstSize);

    if(dstSize > 0 &&
       dstSize < len)
    {
        MY_LOGD("Compress %s from %zu to %zu(%.2f%%)", name, len, dstSize, dstSize*100.0f/len);
        len = dstSize;
        ::memcpy(buf, dst, dstSize);
        isCompressed = true;
    } else {
        MY_LOGD("Compress %s failed(%zu -> %zu)", name, len, dstSize);
        isCompressed = false;
    }

    delete [] dst;

    return isCompressed;
}

//=============================================
//  N3DDebugBuffer_T
//=============================================
N3DDebugBuffer_T::N3DDebugBuffer_T(std::string n, NSCam::IImageBuffer *img)
{
    name = n;
    if(img) {
        format = img->getImgFormat();
        bufferSize = img->getImgSize();
        bufferSizeInBytes = 0;

        for(size_t p = 0; p < img->getPlaneCount(); p++) {
            bufferSizeInBytes += img->getBufSizeInBytes(p);
        }

        buffer = new(std::nothrow) MUINT8[bufferSizeInBytes];
        if(buffer) {
            size_t planeBytes = 0;
            size_t offset     = 0;
            void *planeAddr   = NULL;
            for(size_t p = 0; p < img->getPlaneCount(); p++) {
                planeBytes = img->getBufSizeInBytes(p);
                planeAddr = (void*)img->getBufVA(p);
                if(planeBytes > 0 &&
                   nullptr != planeAddr)
                {
                    ::memcpy((MUINT8*)buffer+offset, planeAddr, planeBytes);
                    offset += planeBytes;
                }
            }
        }

        // MY_LOGD("Copy image %s(%zu bytes)", name.c_str(), bufferSizeInBytes);
    }
}

N3DDebugBuffer_T::N3DDebugBuffer_T(std::string n, void *buf, size_t len, NSCam::MSize sz, int f)
{
    name = n;
    format = f;
    if(nullptr != buf &&
       len > 0)
    {
        bufferSizeInBytes = len;
        bufferSize = sz;
        buffer = new(std::nothrow) MUINT8[bufferSizeInBytes];
        if(buffer) {
            // MY_LOGD("Copy buffer %s(%zu bytes, size %dx%d fotmat %d)", name.c_str(), bufferSizeInBytes, sz.w, sz.h, f);
            ::memcpy(buffer, buf, len);
        } else {
            MY_LOGE("Fail to alloc %zu bytes for %s", bufferSizeInBytes, n.c_str());
        }
    }
}

bool
N3DDebugBuffer_T::compress()
{
    char message[128];
    sprintf(message, "Compress %s", name.c_str());
    AutoProfileUtil profile(LOG_TAG, message);

    isCompressed = __compress(name.c_str(), (char *)buffer, bufferSizeInBytes);
    return isCompressed;
}

//=============================================
//  N3DDebugUtil
//=============================================
N3DDebugUtilImp::N3DDebugUtilImp()
    : __LOG_ENABLED(StereoSettingProvider::isLogEnabled(LOG_PERPERTY))
    , __COMPRESS_ENABLED(!!property_get_int32(ENABLE_COMPRESS_BUFFER_PROPERTY, COMPRESS_STEREO_DEBUG_BUFFER))
    , __DUMP_ENABLED(!!property_get_int32(DUMP_PROPERTY, 0))
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);
}

N3DDebugUtilImp::~N3DDebugUtilImp()
{
    Mutex::Autolock lock(__ioLock);

    for(auto &item : __debugBufferMap) {
        item.second.reset();
    }
    __debugBufferMap.clear();

    for(auto &t : __compressThreadList) {
        if(t.joinable()) {
            // MY_LOGD_IF(__LOG_ENABLED, "Wait compress thread to stop...");
            t.join();
        }
    }
    __compressThreadList.clear();

    if(__headerBuffer) {
        delete [] __headerBuffer;
        __headerBuffer = NULL;
        __headerSize = 0;
    }
}

N3DDebugUtil *
N3DDebugUtil::getInstance(ENUM_STEREO_SCENARIO eScenario)
{
    Mutex::Autolock lock(__instanceLock);

    switch(eScenario)
    {
        case eSTEREO_SCENARIO_CAPTURE:
        default:
            if(NULL == __capInstance) {
                __capInstance = new N3DDebugUtilImp();
            }

            return __capInstance;
            break;
        case eSTEREO_SCENARIO_PREVIEW:
        case eSTEREO_SCENARIO_RECORD:
            if(NULL == __pvInstance) {
                __pvInstance = new N3DDebugUtilImp();
            }

            return __pvInstance;
            break;
    }
}

void
N3DDebugUtil::destroyInstance(ENUM_STEREO_SCENARIO eScenario)
{
    Mutex::Autolock lock(__instanceLock);

    switch(eScenario)
    {
        case eSTEREO_SCENARIO_CAPTURE:
        default:
            if(__capInstance) {
                delete __capInstance;
                __capInstance = NULL;
            }
            break;
        case eSTEREO_SCENARIO_PREVIEW:
        case eSTEREO_SCENARIO_RECORD:
            if(__pvInstance) {
                delete __pvInstance;
                __pvInstance = NULL;
            }
            break;
    }
}

void
N3DDebugUtilImp::addBuffer(N3DDebugBuffer_T &&src)
{
    if(!__DUMP_ENABLED) {
        return;
    }

    Mutex::Autolock lock(__ioLock);
    // MY_LOGD_IF(__LOG_ENABLED, "Add buffer(%zu) %s, src %p", __debugBufferMap.size(), src.name.c_str(), &src);
    __rawBufferSize += src.bufferSizeInBytes;
    string key = src.name;
    __debugBufferMap.insert(std::make_pair(key, std::move(src)));

    if(__COMPRESS_ENABLED)
    {
        __compressThreadList.push_back(std::thread(
        [&, key]() mutable {
            __debugBufferMap[key].compress();
        }));
    }
}

size_t
N3DDebugUtilImp::getDebugBufferSize()
{
    Mutex::Autolock lock(__ioLock);
    return __getDebugBufferSize();
}

size_t
N3DDebugUtilImp::__getDebugBufferSize()
{
    if(0 == __debugBufferSize) {
        const size_t INT_SIZE = sizeof(int);
        __debugBufferSize = INT_SIZE * 2;
        __rawBufferSize += __debugBufferSize;
        for(auto &item : __debugBufferMap) {
            __debugBufferSize += item.second.bufferSizeInBytes;
        }

        string header = __prepareDebugHeader();
        MY_LOGD_IF(__LOG_ENABLED, "Debug header: %s", header.c_str());
        __headerSize = header.length()+1;
        __rawBufferSize += __headerSize;
        if(__headerBuffer) {
            delete [] __headerBuffer;
            __headerBuffer = NULL;
        }
        __headerBuffer = new char[__headerSize];
        ::memcpy(__headerBuffer, header.c_str(), __headerSize);
        if(__COMPRESS_ENABLED) {
            __compress("Header", __headerBuffer, __headerSize);
        }

        __debugBufferSize += __headerSize;
    }

    MY_LOGD_IF((__COMPRESS_ENABLED && __LOG_ENABLED),
               "Debug buffer size: %zu bytes(raw data: %zu bytes, compress rate: %.2f%%",
               __debugBufferSize, __rawBufferSize, __debugBufferSize*100.0f/__rawBufferSize);

    return __debugBufferSize;
}

size_t
N3DDebugUtilImp::getDebugBuffer(void *output)
{
    AutoProfileUtil profile(LOG_TAG, "getDebugBuffer");
    Mutex::Autolock lock(__ioLock);

    if(__COMPRESS_ENABLED) {
        for(auto &t : __compressThreadList) {
            if(t.joinable()) {
                // MY_LOGD_IF(__LOG_ENABLED, "Wait compress thread to stop...");
                t.join();
            }
        }
    }

    size_t retSize = 0;
    if(NULL == output) {
        MY_LOGD("Skip to copy debug buffer");
        for(auto &item : __debugBufferMap) {
            item.second.reset();
        }
    } else {
        retSize = __getDebugBufferSize();  //will generate header abd size

        const size_t INT_SIZE = sizeof(int);
        MUINT8 *target = (MUINT8 *)output;
        ::memcpy(target, &__debugBufferSize, INT_SIZE);
        target += INT_SIZE;
        ::memcpy(target, &__headerSize, INT_SIZE);
        target += INT_SIZE;
        ::memcpy(target, __headerBuffer, __headerSize);
        target += __headerSize;

        for(auto &item : __debugBufferMap) {
            if(NULL != item.second.buffer &&
               item.second.bufferSizeInBytes > 0)
            {
                ::memcpy(target, item.second.buffer, item.second.bufferSizeInBytes);
                target += item.second.bufferSizeInBytes;
            }

            item.second.reset();
        }
    }

    __debugBufferMap.clear();
    __compressThreadList.clear();
    __rawBufferSize = 0;
    __debugBufferSize = 0;

    delete [] __headerBuffer;
    __headerBuffer = NULL;
    __headerSize = 0;

    return retSize;
}

void
N3DDebugUtilImp::setInitInfo(STEREO_KERNEL_SET_ENV_INFO_STRUCT &initInfo)
{
    Value(kObjectType).Swap(__initInfoDoc); //reset
    Document::AllocatorType& allocator = __initInfoDoc.GetAllocator();
    Value initValue(kObjectType);

    #define AddIntInitValue(key, value) initValue.AddMember(key, Value().SetInt(value), allocator)
    #define AddFloatInitValue(key, value) initValue.AddMember(key, Value().SetFloat(value), allocator)

    AddIntInitValue("scenario", initInfo.scenario);

    //iio_main
    Value iio_main_value(kObjectType);
    __addIntValue(iio_main_value, "inp_w", initInfo.iio_main.inp_w, allocator);
    __addIntValue(iio_main_value, "inp_h", initInfo.iio_main.inp_h, allocator);
    __addIntValue(iio_main_value, "out_w", initInfo.iio_main.out_w, allocator);
    __addIntValue(iio_main_value, "out_h", initInfo.iio_main.out_h, allocator);
    __addIntValue(iio_main_value, "src_w", initInfo.iio_main.src_w, allocator);
    __addIntValue(iio_main_value, "src_h", initInfo.iio_main.src_h, allocator);
    __addIntValue(iio_main_value, "wpe_w", initInfo.iio_main.wpe_w, allocator);
    __addIntValue(iio_main_value, "wpe_h", initInfo.iio_main.wpe_h, allocator);
    initValue.AddMember("iio_main", iio_main_value, allocator);

    Value iio_auxi_value(kObjectType);
    __addIntValue(iio_auxi_value, "inp_w", initInfo.iio_auxi.inp_w, allocator);
    __addIntValue(iio_auxi_value, "inp_h", initInfo.iio_auxi.inp_h, allocator);
    __addIntValue(iio_auxi_value, "out_w", initInfo.iio_auxi.out_w, allocator);
    __addIntValue(iio_auxi_value, "out_h", initInfo.iio_auxi.out_h, allocator);
    __addIntValue(iio_auxi_value, "src_w", initInfo.iio_auxi.src_w, allocator);
    __addIntValue(iio_auxi_value, "src_h", initInfo.iio_auxi.src_h, allocator);
    __addIntValue(iio_auxi_value, "wpe_w", initInfo.iio_auxi.wpe_w, allocator);
    __addIntValue(iio_auxi_value, "wpe_h", initInfo.iio_auxi.wpe_h, allocator);
    initValue.AddMember("iio_auxi", iio_auxi_value, allocator);

    AddIntInitValue("geo_info", initInfo.geo_info);

    Value geoValues(kArrayType);
    STEREO_KERNEL_GEO_INFO_STRUCT geo_img[MAX_GEO_LEVEL];
    for(int level = 0; level < MAX_GEO_LEVEL; level++) {
        STEREO_KERNEL_GEO_INFO_STRUCT &geoInfo = initInfo.geo_img[level];

        Value value(kObjectType);
        Value img_main_value(kObjectType);
        __addIntValue(img_main_value, "width", geoInfo.img_main.width, allocator);
        __addIntValue(img_main_value, "height", geoInfo.img_main.height, allocator);
        __addIntValue(img_main_value, "depth", geoInfo.img_main.depth, allocator);
        __addIntValue(img_main_value, "stride", geoInfo.img_main.stride, allocator);
        __addIntValue(img_main_value, "format", geoInfo.img_main.format, allocator);
        __addIntValue(img_main_value, "act_width", geoInfo.img_main.act_width, allocator);
        __addIntValue(img_main_value, "act_height", geoInfo.img_main.act_height, allocator);
        __addIntValue(img_main_value, "offset_x", geoInfo.img_main.offset_x, allocator);
        __addIntValue(img_main_value, "offset_y", geoInfo.img_main.offset_y, allocator);
        value.AddMember("img_main", img_main_value, allocator);

        Value img_auxi_value(kObjectType);
        __addIntValue(img_auxi_value, "width", geoInfo.img_auxi.width, allocator);
        __addIntValue(img_auxi_value, "height", geoInfo.img_auxi.height, allocator);
        __addIntValue(img_auxi_value, "depth", geoInfo.img_auxi.depth, allocator);
        __addIntValue(img_auxi_value, "stride", geoInfo.img_auxi.stride, allocator);
        __addIntValue(img_auxi_value, "format", geoInfo.img_auxi.format, allocator);
        __addIntValue(img_auxi_value, "act_width", geoInfo.img_auxi.act_width, allocator);
        __addIntValue(img_auxi_value, "act_height", geoInfo.img_auxi.act_height, allocator);
        __addIntValue(img_auxi_value, "offset_x", geoInfo.img_auxi.offset_x, allocator);
        __addIntValue(img_auxi_value, "offset_y", geoInfo.img_auxi.offset_y, allocator);
        value.AddMember("img_auxi", img_auxi_value, allocator);

        geoValues.PushBack(value.Move(), allocator);
    }
    initValue.AddMember("geo_img", geoValues.Move(), allocator);

    Value pho_img_value(kObjectType);
    __addIntValue(pho_img_value, "width", initInfo.pho_img.width, allocator);
    __addIntValue(pho_img_value, "height", initInfo.pho_img.height, allocator);
    __addIntValue(pho_img_value, "depth", initInfo.pho_img.depth, allocator);
    __addIntValue(pho_img_value, "stride", initInfo.pho_img.stride, allocator);
    __addIntValue(pho_img_value, "format", initInfo.pho_img.format, allocator);
    __addIntValue(pho_img_value, "act_width", initInfo.pho_img.act_width, allocator);
    __addIntValue(pho_img_value, "act_height", initInfo.pho_img.act_height, allocator);
    __addIntValue(pho_img_value, "offset_x", initInfo.pho_img.offset_x, allocator);
    __addIntValue(pho_img_value, "offset_y", initInfo.pho_img.offset_y, allocator);
    initValue.AddMember("pho_img", pho_img_value, allocator);

    AddFloatInitValue("fov_main.h", initInfo.fov_main[0]);
    AddFloatInitValue("fov_main.v", initInfo.fov_main[0]);

    AddFloatInitValue("fov_auxi.h", initInfo.fov_auxi[0]);
    AddFloatInitValue("fov_auxi.v", initInfo.fov_auxi[0]);
    AddFloatInitValue("baseline", initInfo.baseline);

    AddIntInitValue("system_cfg", initInfo.system_cfg);

    // af_init_main;
    // af_init_auxi;

    Value flow_main_value(kObjectType);
    __addIntValue(flow_main_value, "pixel_array_width", initInfo.flow_main.pixel_array_width, allocator);
    __addIntValue(flow_main_value, "pixel_array_height", initInfo.flow_main.pixel_array_height, allocator);
    __addIntValue(flow_main_value, "sensor_offset_x0", initInfo.flow_main.sensor_offset_x0, allocator);
    __addIntValue(flow_main_value, "sensor_offset_y0", initInfo.flow_main.sensor_offset_y0, allocator);
    __addIntValue(flow_main_value, "sensor_size_w0", initInfo.flow_main.sensor_size_w0, allocator);
    __addIntValue(flow_main_value, "sensor_size_h0", initInfo.flow_main.sensor_size_h0, allocator);
    __addIntValue(flow_main_value, "sensor_scale_w", initInfo.flow_main.sensor_scale_w, allocator);
    __addIntValue(flow_main_value, "sensor_scale_h", initInfo.flow_main.sensor_scale_h, allocator);
    __addIntValue(flow_main_value, "sensor_offset_x1", initInfo.flow_main.sensor_offset_x1, allocator);
    __addIntValue(flow_main_value, "sensor_offset_y1", initInfo.flow_main.sensor_offset_y1, allocator);
    __addIntValue(flow_main_value, "sensor_size_w1", initInfo.flow_main.sensor_size_w1, allocator);
    __addIntValue(flow_main_value, "sensor_size_h1", initInfo.flow_main.sensor_size_h1, allocator);
    __addIntValue(flow_main_value, "tg_offset_x", initInfo.flow_main.tg_offset_x, allocator);
    __addIntValue(flow_main_value, "tg_offset_y", initInfo.flow_main.tg_offset_y, allocator);
    __addIntValue(flow_main_value, "tg_size_w", initInfo.flow_main.tg_size_w, allocator);
    __addIntValue(flow_main_value, "tg_size_h", initInfo.flow_main.tg_size_h, allocator);
    __addIntValue(flow_main_value, "rrz_offset_x", initInfo.flow_main.rrz_offset_x, allocator);
    __addIntValue(flow_main_value, "rrz_offset_y", initInfo.flow_main.rrz_offset_y, allocator);
    __addIntValue(flow_main_value, "rrz_usage_width", initInfo.flow_main.rrz_usage_width, allocator);
    __addIntValue(flow_main_value, "rrz_usage_height", initInfo.flow_main.rrz_usage_height, allocator);
    __addIntValue(flow_main_value, "rrz_out_width", initInfo.flow_main.rrz_out_width, allocator);
    __addIntValue(flow_main_value, "rrz_out_height", initInfo.flow_main.rrz_out_height, allocator);
    __addIntValue(flow_main_value, "mdp_rotate", initInfo.flow_main.mdp_rotate, allocator);
    __addIntValue(flow_main_value, "mdp_offset_x", initInfo.flow_main.mdp_offset_x, allocator);
    __addIntValue(flow_main_value, "mdp_offset_y", initInfo.flow_main.mdp_offset_y, allocator);
    __addIntValue(flow_main_value, "mdp_usage_width", initInfo.flow_main.mdp_usage_width, allocator);
    __addIntValue(flow_main_value, "mdp_usage_height", initInfo.flow_main.mdp_usage_height, allocator);
    initValue.AddMember("flow_main", flow_main_value, allocator);

    Value flow_auxi_value(kObjectType);
    __addIntValue(flow_auxi_value, "pixel_array_width", initInfo.flow_auxi.pixel_array_width, allocator);
    __addIntValue(flow_auxi_value, "pixel_array_height", initInfo.flow_auxi.pixel_array_height, allocator);
    __addIntValue(flow_auxi_value, "sensor_offset_x0", initInfo.flow_auxi.sensor_offset_x0, allocator);
    __addIntValue(flow_auxi_value, "sensor_offset_y0", initInfo.flow_auxi.sensor_offset_y0, allocator);
    __addIntValue(flow_auxi_value, "sensor_size_w0", initInfo.flow_auxi.sensor_size_w0, allocator);
    __addIntValue(flow_auxi_value, "sensor_size_h0", initInfo.flow_auxi.sensor_size_h0, allocator);
    __addIntValue(flow_auxi_value, "sensor_scale_w", initInfo.flow_auxi.sensor_scale_w, allocator);
    __addIntValue(flow_auxi_value, "sensor_scale_h", initInfo.flow_auxi.sensor_scale_h, allocator);
    __addIntValue(flow_auxi_value, "sensor_offset_x1", initInfo.flow_auxi.sensor_offset_x1, allocator);
    __addIntValue(flow_auxi_value, "sensor_offset_y1", initInfo.flow_auxi.sensor_offset_y1, allocator);
    __addIntValue(flow_auxi_value, "sensor_size_w1", initInfo.flow_auxi.sensor_size_w1, allocator);
    __addIntValue(flow_auxi_value, "sensor_size_h1", initInfo.flow_auxi.sensor_size_h1, allocator);
    __addIntValue(flow_auxi_value, "tg_offset_x", initInfo.flow_auxi.tg_offset_x, allocator);
    __addIntValue(flow_auxi_value, "tg_offset_y", initInfo.flow_auxi.tg_offset_y, allocator);
    __addIntValue(flow_auxi_value, "tg_size_w", initInfo.flow_auxi.tg_size_w, allocator);
    __addIntValue(flow_auxi_value, "tg_size_h", initInfo.flow_auxi.tg_size_h, allocator);
    __addIntValue(flow_auxi_value, "rrz_offset_x", initInfo.flow_auxi.rrz_offset_x, allocator);
    __addIntValue(flow_auxi_value, "rrz_offset_y", initInfo.flow_auxi.rrz_offset_y, allocator);
    __addIntValue(flow_auxi_value, "rrz_usage_width", initInfo.flow_auxi.rrz_usage_width, allocator);
    __addIntValue(flow_auxi_value, "rrz_usage_height", initInfo.flow_auxi.rrz_usage_height, allocator);
    __addIntValue(flow_auxi_value, "rrz_out_width", initInfo.flow_auxi.rrz_out_width, allocator);
    __addIntValue(flow_auxi_value, "rrz_out_height", initInfo.flow_auxi.rrz_out_height, allocator);
    __addIntValue(flow_auxi_value, "mdp_rotate", initInfo.flow_auxi.mdp_rotate, allocator);
    __addIntValue(flow_auxi_value, "mdp_offset_x", initInfo.flow_auxi.mdp_offset_x, allocator);
    __addIntValue(flow_auxi_value, "mdp_offset_y", initInfo.flow_auxi.mdp_offset_y, allocator);
    __addIntValue(flow_auxi_value, "mdp_usage_width", initInfo.flow_auxi.mdp_usage_width, allocator);
    __addIntValue(flow_auxi_value, "mdp_usage_height", initInfo.flow_auxi.mdp_usage_height, allocator);
    initValue.AddMember("flow_auxi", flow_auxi_value, allocator);

    AddIntInitValue("working_buffer_size", initInfo.working_buffer_size);

    __initInfoDoc.AddMember(INIT_INFO_TAG, initValue, allocator);
}

void
N3DDebugUtilImp::setProcInfo(STEREO_KERNEL_SET_PROC_INFO_STRUCT &procInfo)
{
    Value(kObjectType).Swap(__procInfoDoc); //reset

    Document::AllocatorType& allocator = __procInfoDoc.GetAllocator();
    Value procValue(kObjectType);

    #define AddIntProcValue(key, value) procValue.AddMember(key, Value().SetInt(value), allocator)
    #define AddFloatProcValue(key, value) procValue.AddMember(key, Value().SetFloat(value), allocator)

    Value af_main_value(kObjectType);
    __addIntValue(af_main_value, "dac_i", procInfo.af_main.dac_i, allocator);
    __addIntValue(af_main_value, "dac_v", procInfo.af_main.dac_v, allocator);
    __addFloatValue(af_main_value, "dac_c", procInfo.af_main.dac_c, allocator);
    Value af_main_dac_w(kArrayType);
    for(int i = 0; i < 4; i++) {
        af_main_dac_w.PushBack(Value().SetInt(procInfo.af_main.dac_w[i]), allocator);
    }
    af_main_value.AddMember("dac_w", af_main_dac_w.Move(), allocator);
    procValue.AddMember("af_main", af_main_value, allocator);

    Value af_auxi_value(kObjectType);
    __addIntValue(af_auxi_value, "dac_i", procInfo.af_auxi.dac_i, allocator);
    __addIntValue(af_auxi_value, "dac_v", procInfo.af_auxi.dac_v, allocator);
    __addFloatValue(af_auxi_value, "dac_c", procInfo.af_auxi.dac_c, allocator);
    Value af_auxi_dac_w(kArrayType);
    for(int i = 0; i < 4; i++) {
        af_auxi_dac_w.PushBack(Value().SetInt(procInfo.af_auxi.dac_w[i]), allocator);
    }
    af_auxi_value.AddMember("dac_w", af_auxi_dac_w.Move(), allocator);
    procValue.AddMember("af_auxi", af_auxi_value, allocator);

    //TODO: Rollback after N3D updates interface
    // Value af_init_main_value(kObjectType);
    // __addIntValue(af_init_main_value, "dac_mcr", procInfo.af_init_main.dac_mcr, allocator);
    // __addIntValue(af_init_main_value, "dac_inf", procInfo.af_init_main.dac_inf, allocator);
    // __addIntValue(af_init_main_value, "dac_str", procInfo.af_init_main.dac_str, allocator);
    // __addIntValue(af_init_main_value, "dist_mcr", procInfo.af_init_main.dist_mcr, allocator);
    // __addIntValue(af_init_main_value, "dist_inf", procInfo.af_init_main.dist_inf, allocator);
    // procValue.AddMember("af_init_main", af_init_main_value, allocator);

    // Value af_init_auxi_value(kObjectType);
    // __addIntValue(af_init_auxi_value, "dac_mcr", procInfo.af_init_auxi.dac_mcr, allocator);
    // __addIntValue(af_init_auxi_value, "dac_inf", procInfo.af_init_auxi.dac_inf, allocator);
    // __addIntValue(af_init_auxi_value, "dac_str", procInfo.af_init_auxi.dac_str, allocator);
    // __addIntValue(af_init_auxi_value, "dist_mcr", procInfo.af_init_auxi.dist_mcr, allocator);
    // __addIntValue(af_init_auxi_value, "dist_inf", procInfo.af_init_auxi.dist_inf, allocator);
    // procValue.AddMember("af_init_auxi", af_init_auxi_value, allocator);

    Value eisValues(kArrayType);
    for(int i = 0; i < 4; i++) {
        eisValues.PushBack(Value().SetInt(procInfo.eis[i]), allocator);
    }
    procValue.AddMember("eis", eisValues.Move(), allocator);

    AddIntProcValue("runtime_cfg", procInfo.runtime_cfg);

    AddIntProcValue("buffer_number", procInfo.buffer_number);

    //TODO: Rollback after N3D updates interface
    // AddIntProcValue("GPU_FMT_IS_YUV", procInfo.GPU_FMT_IS_YUV);
    // AddIntProcValue("GPU_BUF_IS_SRC", procInfo.GPU_BUF_IS_SRC);
    // AddIntProcValue("source_color_domain", procInfo.source_color_domain);
    // AddIntProcValue("SPLIT_MASK", procInfo.SPLIT_MASK);

    __procInfoDoc.AddMember(PROC_INFO_TAG, procValue, allocator);
}

void
N3DDebugUtilImp::setResultInfo(STEREO_KERNEL_RESULT_STRUCT &result)
{
    Value(kObjectType).Swap(__resultInfoDoc); //reset

    Document::AllocatorType& allocator = __resultInfoDoc.GetAllocator();
    Value resultValue(kObjectType);

    Value out_n_values(kArrayType);
    for(int i = 0; i < 7; i++) {
        out_n_values.PushBack(Value().SetInt(result.out_n[i]), allocator);
    }
    __resultInfoDoc.AddMember("out_n", out_n_values.Move(), allocator);


    // out_p
    // STEREO_KERNEL_OUTPUT_FEFM
    // STEREO_KERNEL_OUTPUT_RECT
    // STEREO_KERNEL_OUTPUT_RECT_CAP
    // STEREO_KERNEL_OUTPUT_DEPTH
    // STEREO_KERNEL_OUTPUT_3A
    // STEREO_KERNEL_OUTPUT_VERIFY
    // STEREO_KERNEL_OUTPUT_SCENE_INFO

    // void*  out_debug ;

    __resultInfoDoc.AddMember(RESULT_TAG, resultValue, allocator);
}

string
N3DDebugUtilImp::__prepareDebugHeader()
{
    string headerString;

    Document document(kObjectType);
    Document::AllocatorType& allocator = document.GetAllocator();

    __addInitInfoToDocument(document);
    __addProcToDocument(document);

//    "buffer_list": [
//        {
//          "name": "BID_P2A_FE1B_INPUT",
//          "image_format": "Y8",
//          "image_size": "544x288",
//          "buffer_size": 156672,
//          "compressed": 1
//        },
//        ...
//      ]

    Value buffer_list(kArrayType);
    char strImageSize[__debugBufferMap.size()][12]; //11111x22222
    int index = 0;
    for(auto &item : __debugBufferMap) {
        Value buffer_info(kObjectType);
        Value buffer_name(kObjectType);
        buffer_info.AddMember(EXTRA_DATA_NAME, StringRef(item.second.name.c_str(), item.second.name.length()), allocator);
        switch(item.second.format) {
            case eImgFmt_Y8:
                buffer_info.AddMember(EXTRA_DATA_IMAGE_FORMAT, "Y8", allocator);
                break;
            case eImgFmt_YV12:
                buffer_info.AddMember(EXTRA_DATA_IMAGE_FORMAT, "YV12", allocator);
                break;
            case eImgFmt_BLOB:
            default:
                buffer_info.AddMember(EXTRA_DATA_IMAGE_FORMAT, "Blob", allocator);
                break;
        }

        sprintf(strImageSize[index], "%dx%d", item.second.bufferSize.w, item.second.bufferSize.h);
        buffer_info.AddMember(EXTRA_DATA_IMAGE_SIZE, StringRef(strImageSize[index], strlen(strImageSize[index])), allocator);
        buffer_info.AddMember(EXTRA_DATA_BUFFER_SIZE, item.second.bufferSizeInBytes, allocator);
        buffer_info.AddMember(EXTRA_DATA_COMPRESSED, (int)item.second.isCompressed, allocator);
        buffer_list.PushBack(buffer_info.Move(), allocator);

        index++;
    }
    document.AddMember(EXTRA_DATA_BUFFER_LIST, buffer_list, allocator);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

    const char *header = sb.GetString();
    if(header) {
        headerString = header;
    }

    if(__LOG_ENABLED) {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        writer.SetFormatOptions(kFormatSingleLineArray);
        document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

        const char *src = sb.GetString();
        if(src) {
            const size_t STR_LEN = strlen(src);
            if(STR_LEN > 0) {
                if(STR_LEN < FastLogger::MAX_LOG_SIZE) {
                    __fastLogger.FastLogD("%s", src);
                } else {
                    int restLen = STR_LEN;
                    const int LINE_SIZE = FastLogger::MAX_LOG_SIZE-1;
                    char lineBuffer[LINE_SIZE+1];
                    size_t copySize = 0;
                    while(restLen > 0) {
                        if(restLen < LINE_SIZE) {
                            strncpy(lineBuffer, src, restLen);
                            lineBuffer[restLen] = 0;
                            copySize = restLen;
                        } else {
                            copySize = LINE_SIZE;
                            for(const char *dst = src + LINE_SIZE-1; *dst != '\n' && copySize > 0; --dst, --copySize);

                            //if no newline in buffer, just copy it...
                            if(0 == copySize) {
                                copySize = LINE_SIZE;
                            }
                            strncpy(lineBuffer, src, copySize);
                            lineBuffer[copySize] = 0;
                        }

                        __fastLogger.FastLogD("%s", lineBuffer);
                        restLen -= copySize;
                        src += copySize;
                    }
                }

                __fastLogger.print();
            }
        }
    }

    return headerString;
}

void
N3DDebugUtilImp::__addInitInfoToDocument(Document &doc)
{
    Document::AllocatorType& allocator = doc.GetAllocator();
    Value initValue(kObjectType);
    initValue.CopyFrom(__initInfoDoc[INIT_INFO_TAG], allocator);
    doc.AddMember(INIT_INFO_TAG, initValue.Move(), allocator);
}

void
N3DDebugUtilImp::__addProcToDocument(Document &doc)
{
    Document::AllocatorType& allocator = doc.GetAllocator();
    Value procValue(kObjectType);
    procValue.CopyFrom(__procInfoDoc[PROC_INFO_TAG], allocator);
    doc.AddMember(PROC_INFO_TAG, procValue.Move(), allocator);
}
