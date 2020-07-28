/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define LOG_TAG "VpuStream"

#include <sys/mman.h>
#include <ion/ion.h>        // stardard ion
#include <linux/ion_drv.h>  // for ION_CMDS, ION_CACHE_SYNC_TYPE, define for ion_mm_data_t
#include <ion.h>            // interface for mtk ion
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdint.h>
#include "vpuImpl.h"
#include "vpuCommon.h"

#ifdef __ANDROID__
#include <cutils/properties.h>  // For property_get().
#endif

#ifndef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#endif

#ifdef __ANDROID__
#include <utils/Trace.h>
#else
#define ATRACE_CALL(...)
#define ATRACE_BEGIN(...)
#define ATRACE_END(...)
#endif

#define PROPERTY_DEBUG_LOGLEVEL "debug.vpustream.loglevel"

#ifdef __ANDROID__
#include <android/log.h>
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGV_IF(_x, ...)  do { if (_x) ALOGV(__VA_ARGS__); } while (0)
#define ALOGD_IF(_x, ...)  do { if (_x) ALOGD(__VA_ARGS__); } while (0)
#define VPU_LOGV(fmt, arg...) ALOGV_IF(getVpuLogLevel(), "(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define VPU_LOGD(fmt, arg...) ALOGD_IF(getVpuLogLevel(), "(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define VPU_LOGI(fmt, arg...) ALOGI("(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define VPU_LOGW(fmt, arg...) ALOGW("(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define VPU_LOGE(fmt, arg...) ALOGE("(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#else
#define VPU_LOGV printf
#define VPU_LOGD printf
#define VPU_LOGI printf
#define VPU_LOGW printf
#define VPU_LOGE printf
#endif

#define UNUSED(x) ((void)(x))

const size_t gVpuPropTypeSize[5] = {
        sizeof(uint8_t),   // eTypeChar
        sizeof(int32_t),   // eTypeInt32
        sizeof(float),     // eTypeFloat
        sizeof(int64_t),  // eTypeInt64
        sizeof(double),  // eTypeDouble
};

const char *gVpuPropTypeName[5] = {
        "byte",  // eTypeChar
        "int32",  // eTypeInt32
        "float",  // eTypeFloat
        "int64",  // eTypeInt64
        "double",  // eTypeDouble
};


// for EARA supportw
#include "earaUtil.h"

#define DEFAULT_CALLER_NAME "defaultcaller"
//#define VPU_DUMP_SETT_INFO

/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoImp::
VpuAlgoImp(VpuStream *stream)
{
    mStream = stream;
    memset(&mNative, 0, sizeof(vpu_algo_t));
    memset(mPorts, 0, sizeof(mPorts));
    mSize = 0;
    mNative.info.ptr = (uint64_t) malloc(1024);
    mNative.info.length = 1024;
    mbPortBuild = false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
VpuAlgoImp::
getProperty(
    const char *keyName,
    VpuProperty &prop
)
{
    UNUSED(keyName);
    UNUSED(prop);
    VPU_LOGW("deprecated function");
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuPort *
VpuAlgoImp::
getPort(
    const char *name
)
{
    UNUSED(name);
    VPU_LOGW("deprecated function");
    return NULL;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoImp::
~VpuAlgoImp()
{
    VPU_LOGV("delete algo: 0x%lx, algo name %s", (unsigned long)this, mNative.name);
    if (mNative.info.ptr != 0)
    {
        free((void *) mNative.info.ptr);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequest *
VpuAlgoImp::
acquire()
{
    return new VpuRequestImp(this);
}
/******************************************************************************
 *
 ******************************************************************************/
void
VpuAlgoImp::
release(VpuRequest *request)
{
    delete request;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequestImp::
VpuRequestImp(VpuAlgoImp *algo)
    : mAlgo(algo),
      mBufferCount(0)

{
    memset(mRequestBuffer, 0, sizeof(mRequestBuffer));

    memset(&mNative, 0, sizeof(vpu_request_t));
    mNative.status  = VPU_REQ_STATUS_INVALID;
    mNative.power_param.opp_step = ePowerOppUnrequest;
    mNative.power_param.boost_value = ePowerBoostUnrequest;

    mStream = algo->getStream();

    mSetting = nullptr;

    /* vpu 4.0: assign algo by name */
    strncpy(mNative.algo, mAlgo->getName(), ALGO_NAMELEN - 1);
    VPU_LOGV("algo name %s", mNative.algo);  // debug

    // for EARA support
    mModule = eModuleTypeCv;
    mExecTime = 0;
    mBandwidth = 0;
    mFastestTime = 0;
    mEaraParam.boost_value = ePowerBoostUnrequest;
    mJobPriority = 0;
    mSuggestTime = EARA_NOCONTROL_SUGGESTTIME;

    mId = 0;
}

/******************************************************************************
 *
 ******************************************************************************/
void VpuRequestImp::releaseSettBuf(void)
{
    if (mStream == nullptr || mSetting == nullptr)
        return;

    mStream->memFree(mSetting);
    mSetting = nullptr;
}

bool
VpuRequestImp::
prepareSettBuf(
    int size
)
{
    bool ret = true;

    if (!size)
    {
        VPU_LOGW("Property size is 0");
        return false;
    }

    // apusysEngine::memAlloc(size_t size, uint32_t align, APUSYS_USER_MEM_E type)
    mSetting = mStream->memAlloc(size);

    if (mSetting == nullptr) {
        VPU_LOGE("mSetting: apusys memAlloc failed");
        return false;
    }

    VPU_LOGI("mSetting: iova 0x%x, va 0x%lx, size %d",
        mSetting->iova, (unsigned long)mSetting->va, mSetting->size);
    VPU_LOGI("+ buffer size: %d/%d", mNative.buffer_count, mBufferCount);
    /* vpu 4.0: assign algo by name */
    strncpy(mNative.algo, mAlgo->getName(), ALGO_NAMELEN);
    VPU_LOGV("- buffer size: %d/%d", mNative.buffer_count, mBufferCount);
    mNative.sett_ptr     = mSetting->iova;
    mNative.sett_length  = mSetting->size;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
addBuffer(
    VpuBuffer &buffer
)
{
    bool result = true;
    int ret = 0;

    int buf_index = mBufferCount;
    vpu_request_buffer_t *req_buf = &mRequestBuffer[buf_index];
    vpu_buffer_t         *buf_n   = &mNative.buffers[buf_index];

    buf_n->format      = buffer.format == eFormatData      ? VPU_BUF_FORMAT_DATA     :
                         buffer.format == eFormatImageY8   ? VPU_BUF_FORMAT_IMG_Y8   :
                         buffer.format == eFormatImageYV12 ? VPU_BUF_FORMAT_IMG_YV12 :
                         buffer.format == eFormatImageNV21 ? VPU_BUF_FORMAT_IMG_NV21 : VPU_BUF_FORMAT_IMG_YUY2;
    buf_n->width       = buffer.width;
    buf_n->height      = buffer.height;
    buf_n->port_id     = buffer.port_id;
    buf_n->plane_count = buffer.planeCount;
    req_buf->plane_count = buffer.planeCount;

    VPU_LOGE("buf%d: port_id: %d, planes: %d",
        mBufferCount, buf_n->port_id, buf_n->plane_count); // debug

    if(mBufferCount == 32)
    {
        VPU_LOGE("vpu buffer number would be larger than (%d) after this", mBufferCount);
        return false;
    }

    if(buffer.planeCount > 3)
    {
        VPU_LOGE("vpu wrong planeCount(%d)", buffer.planeCount);
        return false;
    }
    else
    {
        for (unsigned int i = 0; i < buffer.planeCount; i++)
        {
            buf_n->planes[i].stride = buffer.planes[i].stride;
            buf_n->planes[i].length = buffer.planes[i].length;
            buf_n->planes[i].ptr    = -1;
            req_buf->plane[i].share_fd   = buffer.planes[i].fd;
            req_buf->plane[i].offset     = buffer.planes[i].offset;
            req_buf->plane[i].m = nullptr;
        }

        mNative.buffer_count = ++mBufferCount;
        VPU_LOGD("buffer count: (%d/%d)",
            mNative.buffer_count, mBufferCount);

        VPU_LOGD("mmapMVA+ (%d/%d)", buf_n->port_id, buffer.port_id);
        ret = mmapMVA(); //map mva one time
        VPU_LOGD("mmapMVA- (%d)", ret);
        if(ret != 0) {
            result = false;
        }
    }

    return result;
}


/******************************************************************************
 *
 ******************************************************************************/
void
VpuRequestImp::
addBuffer(
    VpuPort *port,
    VpuBuffer &buffer
)
{
    UNUSED(port);
    UNUSED(buffer);
    VPU_LOGW("4.0 not support this function");
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
getProperty(
    const char *keyName,
    VpuProperty &prop
)
{
    UNUSED(keyName);
    UNUSED(prop);
    VPU_LOGW("4.0 do not support this function in 2.0 version");
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
setProperty(
    const char *keyName,
    const VpuProperty &prop
)
{
    UNUSED(keyName);
    UNUSED(prop);
    VPU_LOGW("3.0 not support this function");
    return false;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
setProperty(
    void* sett_ptr,
    int size
)
{
    VPU_LOGD("+, size(%d)", size);
    bool ret;
    if(sett_ptr!=NULL)
    {
        //
        ret = prepareSettBuf(size);
        if(ret)
        {
            VPU_LOGD("1.1");
            memcpy((void *) (mSetting->va), (void*)(sett_ptr), size);
            VPU_LOGD("1.2");
            #ifdef VPU_DUMP_SETT_INFO
            for(int i=0;i<size/sizeof(int);i++)
            {
                VPU_LOGV("(%d), value: (0x%x/0x%x)", i, *((int*)sett_ptr+i), *((int*)(mSetting->va)+i));
            }
            #endif
        //
        //need to cache flush before hw run
        if(this->cacheSync())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
            VPU_LOGE("prepareSettBuf fail");
            return false;
        }
    }
    else
    {
        VPU_LOGE("setting_ptr is null");
        return false;
    }
}

bool
VpuRequestImp::
getProperty(
    void* sett_ptr,
    int size
)
{
    VPU_LOGD("+, size(%d)", size);

    if((sett_ptr == NULL) ||
       (mSetting == nullptr) ||
       (mSetting->va == 0) ||
       (size < 0) ||
       ((unsigned)size > mNative.sett_length))
    {
        return false;
    }

    if(!this->cacheInvalid())
    {
        return false;
    }

    memcpy((void*)(sett_ptr), (void *) (mSetting->va), size);
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
setExtraParam(
    VpuExtraParam extra_param
)
{
    VPU_LOGD("%s+ 0x%x/%d/0x%x",__func__, extra_param.opp_step, extra_param.boost_value, extra_param.bw);

    mNative.power_param.opp_step = extra_param.opp_step;
    mNative.power_param.boost_value = extra_param.boost_value;
    mNative.power_param.bw = extra_param.bw;

    VPU_LOGV("[earaVpu] setExtraParam: eara boost value = %d/%d", extra_param.boost_value, extra_param.eara_param.boost_value);
    memcpy(&mEaraParam, &extra_param.eara_param, sizeof(VpuEaraParam));

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuRequestImp::
~VpuRequestImp()
{
    VPU_LOGI("delete request: 0x%lx, request algo: %s", (unsigned long)this, mNative.algo);

    if (mStream != nullptr) {
        VPU_LOGI("unmap buffers request Id: %lx, request algo: %s", (unsigned long)this->mId, mNative.algo);
        munmapMVA();
        releaseSettBuf();
        mStream = nullptr;
    }
}

static int gVpuLogLevel=0;

static inline void initVpuLogLevel(void)
{
    char prop[100];

#ifdef __ANDROID__
    property_get(PROPERTY_DEBUG_LOGLEVEL, prop, "0");
    gVpuLogLevel = atoi(prop);
#else
    gVpuLogLevel = 0;
#endif

    return;
}

int getVpuLogLevel(void)
{
    return gVpuLogLevel;
}

void
VpuRequestImp::
getInferenceInfo(struct VpuInferenceInfo &info)
{
    ATRACE_CALL();
    info.execTime = mExecTime;
    info.bandwidth = mBandwidth;
}

bool
VpuRequestImp::
earaBegin()
{
    uint64_t suggestTime = EARA_NOCONTROL_SUGGESTTIME;
    int jobId = getJobIdFromPointer(this);
    uint8_t boostValue = 100;

    if(isEaraDisabled())
    {
        return false;
    }

    ATRACE_CALL();

    //libvpu.so only control cv request for EARA
    if(mModule == eModuleTypeCv)
    {
        if(earaNotifyJobBegin(jobId, suggestTime))
        {
            VPU_LOGW("[earaVpu] notify eara begin failed(%d)",jobId);
            return false;
        }

        // for EARA test
#ifdef VPU_EARA_TEST
        {
            uint64_t earaTime = 0;
            earaTime = getEaraSuggestTime();
            if(earaTime == -1)
            {
                VPU_LOGD("[earaVpu] test eara dont control");
                VPU_LOGE("[earaVpu] notify eara begin failed(%d)",jobId);
                return false;
            }
            else
            {
                suggestTime = earaTime;
                VPU_LOGD("[earaVpu] test eara boostValue(%d)",boostValue);
            }
        }
#endif

        mSuggestTime = suggestTime;

        // if first time to run this request,
        if(mFastestTime == 0)
        {
            boostValue = 100;
        }
        else
        {
            //calculate boost value(0~100) for driver to decide opp table
            if(mSuggestTime == 0)
            {
                boostValue = 100;
            }
            else if(mSuggestTime == EARA_NOCONTROL_SUGGESTTIME)
            {
                boostValue = ePowerBoostUnrequest;
            }
            else
            {
                boostValue = mFastestTime * 100 / mSuggestTime;
                if(boostValue > 100)
                {
                    boostValue = 100;
                }
                else if(boostValue < 0)
                {
                    boostValue = 0;
                }
            }
        }

#ifdef VPU_EARA_TEST
        mNative.priority = (uint8_t)getEaraPriority();
#else
        mNative.priority = (uint8_t)mJobPriority;
#endif
        if (mNative.priority >= VPU_REQ_MAX_NUM_PRIORITY)
            mNative.priority = 0;

        if(boostValue != ePowerBoostUnrequest)
        {
            uint8_t tmpBoostValue = calcBoostvalueFromOpp(mNative.power_param.opp_step);
            /* opp set */
            if(tmpBoostValue < boostValue)
            {
                boostValue = tmpBoostValue;
            }
        }

        mNative.power_param.boost_value = boostValue;

        VPU_LOGD("[earaVpu] CV: update suggestTime(%d: %lu/%lu), opp(%d), boostValue(%d), priority(%d)",
            jobId, (unsigned long)mFastestTime, (unsigned long)mSuggestTime,
            mNative.power_param.opp_step, boostValue, mNative.priority);

        return true;
    }
    else
    {
        // for NN application, boost_value != ePowerBoostUnrequest mean eara want to control
        if(mEaraParam.boost_value != ePowerBoostUnrequest)
        {
            VPU_LOGD("[earaVpu] NN: (%d/%d)", mEaraParam.job_priority, mEaraParam.boost_value);

            mNative.priority = (uint8_t)mEaraParam.job_priority;
            boostValue = mEaraParam.boost_value;
            mNative.power_param.boost_value = boostValue;

            return true;
        }
    }

    return false;
}

bool
VpuRequestImp::
earaEnd(int errorStatus)
{
    int jobId = getJobIdFromPointer(this);
    int32_t jobPriority = 0;
    int32_t boostVal = 100;

    if(isEaraDisabled())
    {
        return false;
    }

    ATRACE_CALL();

    mExecTime = mNative.busy_time;
    mBandwidth = mNative.bandwidth;

    if(mModule == eModuleTypeCv)
    {
        //The first request should set boost value = 100, assume its bust time is fastest time
        if(mFastestTime == 0)
        {
            if(mNative.power_param.boost_value >= 0 && mNative.power_param.boost_value <= 100)
            {
                mFastestTime = mExecTime*mNative.power_param.boost_value/100;
            }
            VPU_LOGD("[earaVpu] update mFastestTime(%d/%ld/%ld)", jobId,
                (unsigned long)mFastestTime, (unsigned long)mExecTime);
        }

        // get boost value from current setting
        if(mNative.power_param.boost_value >= 0 && mNative.power_param.boost_value <= 100)
        {
            boostVal = mNative.power_param.boost_value;
        }
        else
        {
            boostVal = calcBoostvalueFromOpp(mNative.power_param.opp_step);
            if(boostVal == ePowerBoostUnrequest)
            {
                boostVal = -1;
            }
        }

        if(earaNotifyJobEnd(jobId, mExecTime, mBandwidth, errorStatus, jobPriority, boostVal))
        {
            VPU_LOGW("[earaVpu] notify eara end failed(%d)",jobId);
            return false;
        }

        if(mSuggestTime == EARA_NOCONTROL_SUGGESTTIME)
        {
            return false;
        }

        if (jobPriority >= VPU_REQ_MAX_NUM_PRIORITY) {
            VPU_LOGW("[earaVpu] invalid priority %d, reset to zero.", jobPriority);
            jobPriority = 0;
        }

        //VPU_LOGD("[earaVpu] update jobPriority(%d/%d)",jobId, jobPriority);
        mJobPriority = jobPriority;

        return true;
    }

    return false;
}

void
VpuRequestImp::
initEaraInfo(VpuModuleType module)
{
    ATRACE_CALL();
    if(module >= eModuleTypeNone || module < 0)
    {
        mModule = eModuleTypeCv;
    }
    else
    {
        mModule = module;
    }

    VPU_LOGD("[earaVpu] module = %s", mModule==0?"CV":"NN");
}

void
VpuRequestImp::
clearEaraInfo()
{
    mNative.power_param.opp_step = ePowerOppUnrequest;
    mNative.power_param.boost_value = ePowerBoostUnrequest;
    mNative.power_param.bw = 0x0;

    // for EARA support
    mModule = eModuleTypeCv;
    mExecTime = 0;
    mBandwidth = 0;
    mFastestTime = 0;
    mEaraParam.boost_value = ePowerBoostUnrequest;
    mJobPriority = 0;
    mSuggestTime = EARA_NOCONTROL_SUGGESTTIME;

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuUtil::
VpuUtil()
{
}
/******************************************************************************
 *
 ******************************************************************************/
uint32_t
VpuUtil::
mapPhyAddr(
    int fd
)
{
    UNUSED(fd);
    VPU_LOGW("deprecated function");
    return 0;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuUtil::
~VpuUtil()
{
}
/******************************************************************************
 *
 ******************************************************************************/
VpuUtil*
VpuUtil::getInstance()
{
    static VpuUtil vpuUtil;
    return &vpuUtil;
}
/******************************************************************************
 *
 ******************************************************************************/
int
VpuRequestImp::
mmapMVA()
{
    ATRACE_CALL();
    int buf_idx, plane_idx;
    vpu_request_buffer_t *req_buf;
    vpu_buffer_t         *buf_n;
    int cnt = 0;

    if (mStream == nullptr) {
        VPU_LOGE("mStream was invalid, unable to import buffers");
        return -1;
    }

    for (buf_idx = 0 ; buf_idx < mNative.buffer_count ; buf_idx++)
    {
        buf_n   = &mNative.buffers[buf_idx];
        req_buf = &mRequestBuffer [buf_idx];
        for (plane_idx = 0 ; plane_idx < buf_n->plane_count ; plane_idx++)
        {
            if(req_buf->plane[plane_idx].m != nullptr)
            {}
            else
            {
                //a. get handle from IonBufFd and increase handle ref count
                IApusysMem *m;
                m = mStream->memImport(
                    req_buf->plane[plane_idx].share_fd, // shared fd
                    buf_n->planes[plane_idx].length);   // size

                if (m == nullptr) {
                    VPU_LOGE("memImport fail(%d/%d), fd(0x%0x)",
                        buf_idx, plane_idx,
                        req_buf->plane[plane_idx].share_fd);
                    return -1;
                }

                req_buf->plane[plane_idx].m = m;
                req_buf->plane[plane_idx].mva = m->iova;
                buf_n->planes[plane_idx].ptr = m->iova + req_buf->plane[plane_idx].offset;
                VPU_LOGV("memImport: buffer[%d], plane[%d], mva 0x%lx",
                    buf_idx, plane_idx, (unsigned long)buf_n->planes[plane_idx].ptr);
            }
            mNative.buf_ion_infos[cnt] = (uint64_t)req_buf->plane[plane_idx].share_fd;
            VPU_LOGD("cnt: %d, share_fd: %ld, 0x%x", cnt, (unsigned long)mNative.buf_ion_infos[cnt],
                req_buf->plane[plane_idx].share_fd);
            cnt++;
        }
    }

    return 0;
}
/******************************************************************************
 *
 ******************************************************************************/
void
VpuRequestImp::
munmapMVA()
{
    ATRACE_CALL();
    int buf_idx, plane_idx;
    vpu_request_buffer_t *req_buf;
    vpu_buffer_t         *buf_n;

    for (buf_idx = 0 ; buf_idx < mNative.buffer_count ; buf_idx++)
    {
        buf_n   = &mNative.buffers[buf_idx];
        req_buf = &mRequestBuffer [buf_idx];
        for (plane_idx = 0 ; plane_idx < buf_n->plane_count ; plane_idx++)
        {
            if (buf_n->planes[plane_idx].ptr != (uint64_t)-1)
            {
                // decrease handle ref count
                if (!mStream->memUnImport(req_buf->plane[plane_idx].m))
                    VPU_LOGE("memUnImport: buffer[%d], plane[%d]: failed",
                        buf_idx, plane_idx);

                buf_n->planes[plane_idx].ptr = -1;
                req_buf->plane[plane_idx].m = nullptr;
                VPU_LOGV("memUnImport: buffer[%d], plane[%d]",
                    buf_idx, plane_idx);
            }
        }
        buf_n->plane_count = 0;
    }
    mNative.buffer_count = 0;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
cacheSync()
{
    return mStream->memSync(mSetting);
}

bool
VpuRequestImp::
cacheInvalid()
{
    return mStream->memInvalidate(mSetting);
}

/******************************************************************************
 *
 ******************************************************************************/
VpuStreamImp::
VpuStreamImp(const char* callername)
{
    UNUSED(callername);
    mReqId = 0;
    //for EARA support
    startEaraInf(); //earaComm

}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgo *
VpuStreamImp::
getAlgo(
    const char *name
)
{
    ATRACE_CALL();
    VPU_LOGI("getAlgo(%s) in", name);

    std::lock_guard<std::mutex> gLock(mAlgosLock);
    auto iter = mAlgos.find(name);
    if (iter == mAlgos.end())    // not in Pool
    {
        // TODO: query algo existence from device
        VpuAlgoImp *algo = new VpuAlgoImp(this);
        vpu_algo_t *algo_n = algo->getNative();
        strncpy(algo_n->name, name, sizeof(algo_n->name));
        algo_n->name[sizeof(algo_n->name) -1] = '\0';
        VPU_LOGV("algo: 0x%lx, algo name %s, stream: %lx",
            (unsigned long)algo,
            algo_n->name,
            (unsigned long)algo->mStream);
        mAlgos[std::string(name)] = algo;;
        return algo;
    }
    else
    {
        VpuAlgoImp *algo = static_cast<VpuAlgoImp *>(iter->second);
        VPU_LOGV("found algo %s, stream: %lx", name,
            (unsigned long)algo->mStream);
        algo->mStream = this;
        return algo;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequest *
VpuStreamImp::
acquire(
    VpuAlgo *algo,
    VpuModuleType module = eModuleTypeCv
)
{
    VpuRequest * req = NULL;

    if(algo == NULL)
    {
        return NULL;
    }

    req = ((VpuAlgoImp *)algo)->acquire();
    if(req == NULL)
    {
        return NULL;
    }

    //for EARA support
    ((VpuRequestImp *)req)->initEaraInfo(module);

    return req;
}

/******************************************************************************
 *
 ******************************************************************************/
void
VpuStreamImp::
release(
    VpuRequest *request
)
{
    if(request == nullptr)
        return;

    VPU_LOGI("[vpu] release req");
    auto *reqi = static_cast<VpuRequestImp *>(request);

    reqi->munmapMVA();
    reqi->getAlgo()->release(request);
    reqi->clearEaraInfo();
    reqi->mStream = nullptr;


}
/******************************************************************************
 *
 ******************************************************************************/
bool
VpuStreamImp::
enque(
    VpuRequest *request,
    unsigned int core_index
)
{
    bool ret;

    UNUSED(core_index);
    VPU_LOGI("deprecated function");

    std::unique_lock <std::mutex> l(mRequestMutex);

    ret = runReq(request, true);

    if (ret) {
        mRequests.push_back(request);
        mRequestCond.notify_one();
    }

    return ret;
}

bool
VpuStreamImp::
runReq(
    VpuRequest *request,
    bool async
)
{
    bool ret = false;

    ATRACE_CALL();
    VpuRequestImp *reqi = static_cast<VpuRequestImp *>(request);
    vpu_request_t *req_n = reqi->getNative();

    VPU_LOGI("[vpu] enque algo: %s, Id: %ld",
        req_n->algo, reqi->mId);

    {
        int i, j ;
        VPU_LOGD("buffer_count = %d, sett_ptr 0x%lx, sett_length %d", \
            req_n->buffer_count, (unsigned long)req_n->sett_ptr, req_n->sett_length);
        if(req_n->buffer_count == 0)
        {
            VPU_LOGW("buffer count = 0"); //testcase_max_power do not need buffers
        }
        for (i = 0 ; i < req_n->buffer_count; i++)
        {
            VPU_LOGD("buffer[%d]: plane_count = %d, port_id %d, w %d, h %d, fmt %d", i, req_n->buffers[i].plane_count, req_n->buffers[i].port_id, req_n->buffers[i].width, req_n->buffers[i].height, req_n->buffers[i].format);
            for (j = 0 ; j < req_n->buffers[i].plane_count; j++)
            {
                VPU_LOGD("plane[%d]: ptr 0x%lx, length %d, stride %d", j, (unsigned long)req_n->buffers[i].planes[j].ptr, req_n->buffers[i].planes[j].length, req_n->buffers[i].planes[j].stride);
            }
        }
    }

    //for EARA support
    ((VpuRequestImp *) request)->earaBegin();

    // APUSYS sendcmd
    do {
        reqi->mCmd = new apusysCmd(this);
        reqi->mReq = memAlloc(sizeof(vpu_request_t));
        std::vector<int> dep;

        if (reqi->mReq == nullptr) {
            VPU_LOGE("mReq: apusys memAlloc failed");
            ret = -ENOMEM;
            break;
        }

        memcpy((void *)reqi->mReq->va, req_n, sizeof(vpu_request_t));
        memSync(reqi->mReq);
        VPU_LOGD("addSubCmd(cmd) mva:%x, kva:%lx, va:%lx, size:%d",
            reqi->mReq->iova, (unsigned long)reqi->mReq->kva,
            (unsigned long)reqi->mReq->va, reqi->mReq->size);
        reqi->mCmd->addSubCmd(reqi->mReq, APUSYS_DEVICE_VPU, dep, 3000);
        // cmd->constructCmd(); // private function

        if (async) {
            ret = this->runCmdAsync(reqi->mCmd);
        } else {
            ret = this->runCmd(reqi->mCmd);
            delete reqi->mCmd;
            reqi->mCmd = nullptr;
            memFree(reqi->mReq);
            reqi->mReq = nullptr;
        }

        if (!ret) {
            VPU_LOGE("apusys runCmd%s failed",
                async ? "async" : "");
            break;
        }
    } while (0);

    return ret;
}

bool
VpuStreamImp::
waitReq(
    VpuRequest *request
)
{
    VpuRequestImp *reqi = static_cast<VpuRequestImp *>(request);
    bool ret;

    if (reqi->mCmd == nullptr) {
        VPU_LOGD("already finished");
        return true;
    }

    ret = this->waitCmd(reqi->mCmd);

    if (!ret)
        VPU_LOGE("apusys waitCmd failed");

    delete reqi->mCmd;
    reqi->mCmd = nullptr;

    if (reqi->mReq) {
        memFree(reqi->mReq);
        reqi->mReq = nullptr;
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuStreamImp::
tryEnque(
    VpuRequest *request
)
{
    UNUSED(request);
    VPU_LOGW("deprecated function");
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequest *
VpuStreamImp::
deque()
{
    ATRACE_CALL();
    bool result = true;
    VpuRequest *req = nullptr;
    {
        std::unique_lock <std::mutex> l(mRequestMutex);
        if (mRequests.size() == 0)
        {
            mRequestCond.wait(l);
        }
        req = mRequests.front();
        mRequests.erase(mRequests.begin());
    }

    if (req) {
        result = waitReq(req);
        // for EARA support
        VpuRequestImp *reqi = static_cast<VpuRequestImp *>(req);
        reqi->earaEnd((int)result);
    }

    return req;
}
/******************************************************************************
 *
 ******************************************************************************/
void
VpuStreamImp::
flush()
{
    ATRACE_CALL();
    // TODO: how to flush requests from APUSYS ?
}
/******************************************************************************
 *
 ******************************************************************************/
bool
VpuStreamImp::
setPower(
    VpuPower& pwr,
    unsigned int core_index
)
{
    UNUSED(core_index);
    return apusysEngine::setPower(APUSYS_DEVICE_VPU, pwr.boost_value);
}
/******************************************************************************
 *
 ******************************************************************************/
bool
VpuStreamImp::getVpuStatus(
    std::vector<VpuStatus>& vpu_status_info
)
{
    for(int i = vpu_status_info.size()-1 ; i>=0; i--)
    {
        vpu_status_info.pop_back();
    }

    /* individual cores */
    for(int i = 0 ; i < MAX_VPU_CORE_NUM ; i++)
    {
        VpuStatus status;
        status.core_index = static_cast<VpuCoreIndex>(i);
        status.available = true;
        status.waiting_requests_num = 0;
        vpu_status_info.push_back(status);
    }

    /* common pool */
    if((int)vpu_status_info.size() > 0)
    {
        VpuStatus status;
        status.core_index = static_cast<VpuCoreIndex>(eIndex_NONE);
        status.available = true;
        status.waiting_requests_num = 0;
        vpu_status_info.push_back(status);
    }

    return true;
}

struct vpuMemBufferImpl :VpuMemBuffer {
public:
    IApusysMem *m;
};

/******************************************************************************
 * stream lib 3.0, support allocate memory for VPU
 ******************************************************************************/
VpuMemBuffer *VpuStreamImp::allocMem(size_t size, uint32_t align, enum vpuMemType type)
{
    vpuMemBufferImpl *b;

    UNUSED(align);
    UNUSED(type);

    b = new vpuMemBufferImpl;
    if (b == nullptr)  {
        VPU_LOGE("alloc buffer failed\n");
        return nullptr;
    }

    // TODO: allocate memory upon type
    // apusysEngine::memAlloc(size_t size, uint32_t align, APUSYS_USER_MEM_E type)
    b->m = this->memAlloc(size);

    if (b->m == nullptr){
        VPU_LOGE("apusys memAlloc failed\n");
        delete b;
        return nullptr;
    }

    return b;
}

bool VpuStreamImp::freeMem(VpuMemBuffer *buf)
{
    bool ret = true;

    vpuMemBufferImpl *b = static_cast<vpuMemBufferImpl *>(buf);

    if (b) {
        if (b->m)
            // bool apusysEngine::memFree(IApusysMem * mem)
            ret = this->memFree(b->m);
        delete b;
    }
    return ret;
}

bool VpuStreamImp::syncMem(VpuMemBuffer *buf)
{
    vpuMemBufferImpl *b = static_cast<vpuMemBufferImpl *>(buf);

    if ((b == nullptr) || (b->m == nullptr))
        return false;

    // bool apusysEngine::memSync(IApusysMem * mem)
    return this->memSync(b->m);
}

/******************************************************************************
 * stream lib 4.0, support dynamic load algo
 ******************************************************************************/
VpuCreateAlgo VpuStreamImp::createAlgo(const char *name, char *buf, uint32_t len, uint32_t core)
{
    // TODO: add createAlgo for Dynamic load algo
    UNUSED(name);
    UNUSED(buf);
    UNUSED(len);
    UNUSED(core);
    return 0;
}

bool VpuStreamImp::freeAlgo(VpuCreateAlgo algoFd)
{
    // TODO: add freeAlgo for Dynamic load algo
    UNUSED(algoFd);
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuStreamImp::
~VpuStreamImp()
{
    //for EARA support
    closeEaraInf();
    std::vector<VpuRequest *>::iterator iter;

    VPU_LOGI("destory stream instance\n");

    std::unique_lock <std::mutex> l(mRequestMutex);

    // Release queued requests
    while(!mRequests.empty())
    {
        VpuRequestImp *req;
        iter = mRequests.begin();
        req = static_cast<VpuRequestImp *>(*iter);
        VPU_LOGI("unmap buffers of request: 0x%lx, request algo: %s",
            (unsigned long)req, req->getAlgo()->getName());
        req->munmapMVA();
        req->releaseSettBuf();
        req->mStream = nullptr;
        mRequests.erase(iter);
    }

    // Release all algos
    for (auto iter = mAlgos.begin(); iter != mAlgos.end(); ) {
        delete iter->second;
        iter = mAlgos.erase(iter);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuStream *
VpuStream::
createInstance(void)
{
    return createInstance(DEFAULT_CALLER_NAME);
}

VpuStream *
VpuStream::
createInstance(const char* callername)
{
    /* get eara disable from property */
    initEaraDisable();
    /* init vpu stream log level from property */
    initVpuLogLevel();

#ifdef HAVE_EFUSE_BOND
    {
        #define DEV_IOC_MAGIC       'd'
        #define READ_DEV_DATA       _IOR(DEV_IOC_MAGIC,  1, unsigned int)

        typedef struct {
            unsigned int entry_num;
            unsigned int data[100];
        } DEVINFO_S;

        DEVINFO_S devinfo;
        int ret = 0;
        int fd = 0;

        int bonding_info=0;
        int mDisVPU = 0; /* vpu efuse value*/
        bool vpu_hw_support = true;
        fd = open("/proc/device-tree/chosen/atag,devinfo", O_RDONLY);
        if(fd < 0)
        {
            VPU_LOGE("/proc/device-tree/chosen/atag,devinfo kernel open fail, errno(%d):%s",errno,strerror(errno));
            return NULL;
        }
        else
        {
            if ((ret = read(fd, (void *)&devinfo, sizeof(DEVINFO_S))) < 0)
            {
                VPU_LOGE("Get Devinfo Data Fail:%d\n", ret);
                return NULL;
            }
            else
            {
                switch (HAVE_EFUSE_BOND_VERSION) {
                case 71:
                bonding_info = devinfo.data[3];
                    mDisVPU = (bonding_info & 0xC00)>>10;
                    if(mDisVPU == 0x3)
                        vpu_hw_support = false;
                    break;
                default:
                    break;
                }
            }
            VPU_LOGI("version(%d), Data:0x%x,0x%x, %d\n", HAVE_EFUSE_BOND_VERSION,
                bonding_info, mDisVPU, vpu_hw_support);
            close(fd);
        }
        if(vpu_hw_support)
        {
            return new VpuStreamImp(callername);
        }
        else
        {
            VPU_LOGE("Do not support VPU HW:0x%x,0x%x, %d\n", bonding_info, mDisVPU, vpu_hw_support);
            return NULL;
        }
    }

#else
    VPU_LOGD("vpu user createInstance: %s", callername);
    return new VpuStreamImp(callername);
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
vpuMapPhyAddr(
    int fd
)
{
    UNUSED(fd);
    VPU_LOGW("deprecated function");
    return 0;
}

