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

#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_ALWAYS
#endif

#include <utils/Trace.h>
#include <cutils/log.h>
#include <sys/mman.h>
#include <ion/ion.h>                // stardard ion
#include <linux/ion_drv.h>          // for ION_CMDS, ION_CACHE_SYNC_TYPE, define for ion_mm_data_t
#include <libion_mtk/include/ion.h> // interface for mtk ion
#include <errno.h>

#include "vpuImpl.h"

static int32_t gIonDevFD = -1;
/******************************************************************************
 *
 ******************************************************************************/
#if 1
#define CAM_LOGV ALOGV
#define CAM_LOGD ALOGD
#define CAM_LOGI ALOGI
#define CAM_LOGW ALOGW
#define CAM_LOGE ALOGE
#define CAM_LOGA ALOGA
#define CAM_LOGF ALOGF
#else
#define CAM_LOGV printf
#define CAM_LOGD printf
#define CAM_LOGI printf
#define CAM_LOGW printf
#define CAM_LOGE printf
#define CAM_LOGA printf
#define CAM_LOGF printf
#endif

#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoImp::
VpuAlgoImp()
{
    mAvaiableRequest.clear();
    mInUseRequest.clear();
    memset(&mNative, 0, sizeof(vpu_algo_t));
    memset(mPorts, 0, sizeof(mPorts));
    mSize = 0;
    mNative.info_ptr = (uint64_t) malloc(1024);
    mNative.info_length = 1024;
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
    vpu_algo_t *algo_n = &mNative;
    vpu_prop_desc_t *prop_desc;
    for (int i = 0; i < algo_n->info_desc_count; i++)
    {
        prop_desc = &algo_n->info_descs[i];
        if (strcmp(prop_desc->name, keyName) != 0)
        {
            continue;
        }
        prop.type   = prop_desc->type == VPU_PROP_TYPE_CHAR  ? eTypeChar  :
                      prop_desc->type == VPU_PROP_TYPE_INT32 ? eTypeInt32 :
                      prop_desc->type == VPU_PROP_TYPE_INT64 ? eTypeInt64 :
                      prop_desc->type == VPU_PROP_TYPE_FLOAT ? eTypeFloat : eTypeDouble;
        prop.count  = prop_desc->count;
        prop.data.c = (char *) (algo_n->info_ptr + prop_desc->offset);
        return true;
    }
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
    int i;
    vpu_algo_t *algo_n = &mNative;
    if (mbPortBuild == false)
    {
        vpu_port_t *port_n;
        for (int i = 0; i < algo_n->port_count; i++)
        {
            port_n = &algo_n->ports[i];
            MY_LOGV("  Port[%02d]: name=%s", port_n->id, port_n->name);
            mPorts[i].id   = port_n->id;
            mPorts[i].name = port_n->name;
            mPorts[i].portDirection = port_n->dir == VPU_PORT_DIR_IN  ? eDirectionIn :
                                      port_n->dir == VPU_PORT_DIR_OUT ? eDirectionOut : eDirectionInOut ;
            mPorts[i].portUsage     = port_n->usage == VPU_PORT_USAGE_IMAGE ? eImageBuffer : eDataBuffer ;
        }
        mbPortBuild = true;
    }

    VpuPort *pPort;
    for (i = 0; i < algo_n->port_count; i++)
    {
        pPort = &mPorts[i];
        if (strcmp(name, pPort->name) != 0) {
            continue;
        }
        MY_LOGV("port: 0x%lx", (unsigned long)pPort);
        return pPort;
    }
    MY_LOGV("port: NULL");
    return NULL;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoImp::
~VpuAlgoImp()
{
    vpuRequestList_t::iterator iter;
    MY_LOGV("delete algo: 0x%lx, algo name %s, id %d", (unsigned long)this, mNative.name, mNative.id);
    if (mNative.info_ptr != 0)
    {
        free((void *) mNative.info_ptr);
    }
    MY_LOGV("mAvaiableRequest: size %d, mInUseRequest: size %d", (int)mAvaiableRequest.size(), (int)mInUseRequest.size());
    for(iter = mAvaiableRequest.begin(); iter != mAvaiableRequest.end(); iter++)
        delete *iter;

    for(iter = mInUseRequest.begin(); iter != mInUseRequest.end(); iter++)
        delete *iter;

    mAvaiableRequest.clear();
    mInUseRequest.clear();
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequest *
VpuAlgoImp::
acquire()
{
    ATRACE_CALL();
    std::unique_lock <std::mutex> l(mRequestMutex);
    VpuRequestImp *req;
    if (!mAvaiableRequest.empty())
    {
        vpuRequestList_t::iterator iter = mAvaiableRequest.begin();
        mInUseRequest.push_back(*iter);
        req = *iter;
        mAvaiableRequest.erase(iter);
        req->reset();
    }
    else
    {
        req = new VpuRequestImp(this, mSize);
        mInUseRequest.push_back(req);
    }

    return req;
}
/******************************************************************************
 *
 ******************************************************************************/
void
VpuAlgoImp::
release(VpuRequest *request)
{
    ATRACE_CALL();
    std::unique_lock <std::mutex> l(mRequestMutex);
    mInUseRequest.remove((VpuRequestImp *)request);
    mAvaiableRequest.push_back((VpuRequestImp *)request);
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequestImp::
VpuRequestImp(VpuAlgoImp *algo, int size)
    : mAlgo(algo),
      mBufferCount(0)

{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    ion_user_handle_t   ion_handle;
    int                 share_fd;
    uint64_t            virtAddr;
    int err;

    ion_handle = -1;
    share_fd   = -1;
    virtAddr   =  0;
    sys_data.get_phys_param.phy_addr = 0;

    if (!size)
    {
        MY_LOGW("Property size is 0.");
        goto EXIT;
    }
    // a. alloc ion buffer
    ATRACE_BEGIN("vpuStream::ion_alloc_mm");
    if (ion_alloc_mm(gIonDevFD, size, 0, 0, &ion_handle))
    {
        ATRACE_END();
        MY_LOGE("alloc ion buffer fail!");
        goto EXIT;
    }
    ATRACE_END();
    MY_LOGV("ion_alloc: mSetting, ion_handle %d", ion_handle);
    // b. get ion share_fd
    ATRACE_BEGIN("vpuStream::ion_share");
    if (ion_share(gIonDevFD, ion_handle, &share_fd))
    {
        ATRACE_END();
        MY_LOGE("get ion share_fd fail!");
        goto EXIT;
    }
    ATRACE_END();
    // c. map virtual address
    MY_LOGV("ion_dev_fd %d, ion_handle %d, share_fd %d, size %d", gIonDevFD, ion_handle, share_fd, size);
    ATRACE_BEGIN("vpuStream::ion_mmap");
    virtAddr = (uint64_t)ion_mmap(gIonDevFD, NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, share_fd, 0);
    ATRACE_END();
    if (!virtAddr)
    {
        MY_LOGE("Cannot map ion buffer.");
        goto EXIT;
    }
    // d. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.handle      = ion_handle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_VPU;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 1;
    mm_data.config_buffer_param.reserve_iova_start = VPU_MVA_START;
    mm_data.config_buffer_param.reserve_iova_end   = VPU_MVA_END;
    ATRACE_BEGIN("vpuStream::config_ion_buffer");
    err = ion_custom_ioctl(gIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
    ATRACE_END();
    if(err == (-ION_ERROR_CONFIG_LOCKED))
    {
        MY_LOGW("ion_custom_ioctl Double config after map phy address");
    }
    else if(err != 0)
    {
        MY_LOGE("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
        goto EXIT;
    }

    //e. map physical address
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = ion_handle;
    sys_data.get_phys_param.phy_addr = M4U_PORT_VPU << 24 | ION_FLAG_GET_FIXED_PHYS;
    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
    ATRACE_BEGIN("vpuStream::getPhys");
    if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
        MY_LOGE("ion_custom_ioctl get_phys_param failed!");
    }
    ATRACE_END();
EXIT:
    mSetting.ion_handle = ion_handle;
    mSetting.share_fd   = share_fd;
    mSetting.va         = virtAddr;
    mSetting.mva        = sys_data.get_phys_param.phy_addr;
    MY_LOGV("mSetting: mva 0x%x, va 0x%lx, ion_handle %d, share_fd %d", mSetting.mva, (unsigned long)mSetting.va, mSetting.ion_handle, mSetting.share_fd);
    mNative.algo_id      = mAlgo->getId();
    mNative.sett_ptr     = mSetting.mva;
    mNative.sett_length  = size;

    mNative.buffer_count = 0;
    mNative.status  = VPU_REQ_STATUS_INVALID;
    mNative.priv    = 0;
    memset(mNative.buffers, 0, sizeof(mNative.buffers));
    memset(mRequestBuffer, 0, sizeof(mRequestBuffer));
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
    int buf_index = mBufferCount;
    vpu_request_buffer_t *req_buf = &mRequestBuffer[buf_index];
    vpu_buffer_t         *buf_n   = &mNative.buffers[buf_index];

    if (!port)
    {
        MY_LOGE("port is Null!");
        return;
    }
    buf_n->format      = buffer.format == eFormatData      ? VPU_BUF_FORMAT_DATA     :
                         buffer.format == eFormatImageY8   ? VPU_BUF_FORMAT_IMG_Y8   :
                         buffer.format == eFormatImageYV12 ? VPU_BUF_FORMAT_IMG_YV12 :
                         buffer.format == eFormatImageNV21 ? VPU_BUF_FORMAT_IMG_NV21 : VPU_BUF_FORMAT_IMG_YUY2;
    buf_n->width       = buffer.width;
    buf_n->height      = buffer.height;
    buf_n->port_id     = port->id;
    buf_n->plane_count = buffer.planeCount;
    req_buf->plane_count = buffer.planeCount;
    for (unsigned int i = 0; i < buffer.planeCount; i++)
    {
        buf_n->planes[i].stride = buffer.planes[i].stride;
        buf_n->planes[i].length = buffer.planes[i].length;
        buf_n->planes[i].ptr    = -1;
        req_buf->plane[i].share_fd   = buffer.planes[i].fd;
        req_buf->plane[i].offset     = buffer.planes[i].offset;
        req_buf->plane[i].ion_handle = -1;
    }

    mNative.buffer_count = ++mBufferCount;
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
VpuRequestImp::
addBuffer(
    VpuBuffer &buffer
)
{
	MY_LOGE("not support this function(addBuf) in 1.0");
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
    vpu_algo_t *algo_n = mAlgo->getNative();
    vpu_prop_desc_t *prop_desc_n;
    for (int i = 0; i < algo_n->sett_desc_count; i++) {
        prop_desc_n = &algo_n->sett_descs[i];
        if (strcmp(prop_desc_n->name, keyName) != 0) {
            continue;
        }
        prop.type   = prop_desc_n->type == VPU_PROP_TYPE_CHAR  ? eTypeChar  :
                      prop_desc_n->type == VPU_PROP_TYPE_INT32 ? eTypeInt32 :
                      prop_desc_n->type == VPU_PROP_TYPE_INT64 ? eTypeInt64 :
                      prop_desc_n->type == VPU_PROP_TYPE_FLOAT ? eTypeFloat : eTypeDouble;
        prop.count  = prop_desc_n->count;
        prop.data.c = (char *) (mSetting.va + prop_desc_n->offset);
        return true;
    }
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
    vpu_algo_t *algo_n = mAlgo->getNative();
    vpu_prop_desc_t *prop_desc_n;
    uint8_t type;
    type = prop.type == eTypeChar  ? VPU_PROP_TYPE_CHAR  :
           prop.type == eTypeInt32 ? VPU_PROP_TYPE_INT32 :
           prop.type == eTypeInt64 ? VPU_PROP_TYPE_INT64 :
           prop.type == eTypeFloat ? VPU_PROP_TYPE_FLOAT : VPU_PROP_TYPE_DOUBLE;
    for (int i = 0; i < algo_n->sett_desc_count; i++)
    {
        prop_desc_n = &algo_n->sett_descs[i];
        if (strcmp(prop_desc_n->name, keyName) != 0)
        {
            continue;
        }
        if (prop.count != prop_desc_n->count)
        {
            MY_LOGW("property[%s] the count doesen't match. native:%d, user:%d",
                   keyName, prop_desc_n->count, prop.count);
        }
        if (type != prop_desc_n->type)
        {
            MY_LOGW("property[%s] the type doesen't match. native:%d, user:%d",
                   keyName, prop_desc_n->type, prop.type);
        }
        int data_length = prop_desc_n->count * gVpuPropTypeSize[prop_desc_n->type];

        memcpy((void *) (mSetting.va + prop_desc_n->offset), prop.data.c, data_length);
        return true;
    }
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
	MY_LOGE("not support this function(setProp) in 1.0");
	return false;
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
	MY_LOGE("not support this function(setProp) in 1.0");
	return false;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuRequestImp::
~VpuRequestImp()
{
    MY_LOGV("delete request: 0x%lx, request algo_id %d", (unsigned long)this, mNative.algo_id);
    if (mNative.sett_length)
    {
        ion_munmap(gIonDevFD, (char*)mSetting.va, mNative.sett_length);
        ion_share_close(gIonDevFD, mSetting.share_fd);
        if (ion_free(gIonDevFD, mSetting.ion_handle))
        {
            MY_LOGE("ion_free failed(share_fd %d, ion_handle %d)!", mSetting.share_fd, mSetting.ion_handle);
        }
        MY_LOGV("ion_free: mSetting, ion_handle %d", mSetting.ion_handle);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoPool::
VpuAlgoPool()
{
    mvVpuAlgoMap.clear();
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoPool::
~VpuAlgoPool()
{
    vpuAlgoMap_t::iterator iter;
    for(iter = mvVpuAlgoMap.begin(); iter != mvVpuAlgoMap.end(); iter++)
    {
        delete iter->second;
    }
    mvVpuAlgoMap.clear();
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoPool*
VpuAlgoPool::getInstance()
{
    static VpuAlgoPool vpuAlgoPool;
    return &vpuAlgoPool;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgo*
VpuAlgoPool::
getAlgo(std::string name, int fd)
{
    ATRACE_CALL();
    int index;
    vpuAlgoMap_t::iterator iter;
    vpuRequestList_t listRequest;
    if ((iter = mvVpuAlgoMap.find(name)) == mvVpuAlgoMap.end())    // not in Pool
    {
        VpuAlgoImp *algo = new VpuAlgoImp();
        vpu_algo_t *algo_n = algo->getNative();
        strncpy(algo_n->name, name.c_str(), sizeof(algo_n->name));
        algo_n->name[sizeof(algo_n->name) -1] = '\0';
        MY_LOGV("algo: 0x%lx, algo name %s", (unsigned long)algo, algo_n->name);
        MY_LOGV("fd: %d, algo name %s", fd, algo_n->name);
        int ret = ioctl(fd, VPU_IOCTL_GET_ALGO_INFO, algo_n);
        if (ret < 0)
        {
            MY_LOGE("fail to get algo, %s, errno = %d", strerror(errno), errno);
            delete algo;
            return NULL;
        }
        MY_LOGV("Algo[%02d]: port count=%d", algo_n->id, algo_n->port_count);
        vpu_port_t *port_n;
        for (int i = 0; i < algo_n->port_count; i++)
        {
            port_n = &algo_n->ports[i];
            MY_LOGV("  Port[%02d]: name=%s", port_n->id, port_n->name);
        }

        vpu_prop_desc_t *prop_desc_n;
        int prop_length = 0;
        int data_length = 0;
#ifdef VPU_DUMP_ALGO_INFO
        for (int i = 0; i < algo_n->info_desc_count; i++)
        {
            prop_desc_n = &algo_n->info_descs[i];
            data_length = prop_desc_n->count * gVpuPropTypeSize[prop_desc_n->type];
            MY_LOGV("  Info[%02d]: name=%s, offset=%d, type=%s, count=%d",
                   prop_desc_n->id,
                   prop_desc_n->name,
                   prop_desc_n->offset,
                   gVpuPropTypeName[prop_desc_n->type],
                   prop_desc_n->count);

            for (int j = 0; j < data_length; j++)
            {
                if (j % 16 == 0)
                    MY_LOGV("            ");
                MY_LOGV("%02X ", *((unsigned char *)
                        (algo_n->info_ptr + prop_desc_n->offset + j)));
            }
            MY_LOGV("");
        }
#endif
        for (int i = 0; i < algo_n->sett_desc_count; i++)
        {
            prop_desc_n = &algo_n->sett_descs[i];
            data_length = prop_desc_n->count * gVpuPropTypeSize[prop_desc_n->type];
            prop_length += data_length;
#ifdef VPU_DUMP_ALGO_INFO
            MY_LOGV("  Sett[%02d]: name=%s, offset=%d, type=%s, count=%d",
                   prop_desc_n->id,
                   prop_desc_n->name,
                   prop_desc_n->offset,
                   gVpuPropTypeName[prop_desc_n->type],
                   prop_desc_n->count);
#endif
        }
        algo->setPropSize(prop_length);
        mvVpuAlgoMap.insert(vpuAlgoPair_t(name, algo));
        return algo;
    }
    else
    {
        return iter->second;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuUtil::
VpuUtil()
{
    if (gIonDevFD < 0)
    {
        gIonDevFD = mt_ion_open("VpuStream");
        if (gIonDevFD < 0)
        {
            MY_LOGE("mt_ion_open() return %d", gIonDevFD);
        }
    }
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
    ATRACE_CALL();
    std::unique_lock <std::mutex> l(mMapPhyMutex);
    uint32_t ret = -1;
    int err;
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    ion_user_handle_t   ion_handle;

    ATRACE_BEGIN("VpuUtil::ion_import");
    //a. get ion handle from IonBufFd and increase handle ref count
    if(ion_import(gIonDevFD, fd, &ion_handle))
    {
        ATRACE_END();
        MY_LOGE("ion_import fail, ion_handle(0x%x)", ion_handle);
        return ret;
    }
    ATRACE_END();
    MY_LOGV("ion_import: share_fd %d, ion_handle %d", fd, ion_handle);
    //b. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.handle      = ion_handle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_VPU ;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 1;
    mm_data.config_buffer_param.reserve_iova_start = VPU_MVA_START;
    mm_data.config_buffer_param.reserve_iova_end   = VPU_MVA_END;
    ATRACE_BEGIN("VpuUtil::config_ion_buffer");
    err = ion_custom_ioctl(gIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
    ATRACE_END();
    if(err == (-ION_ERROR_CONFIG_LOCKED))
    {
        MY_LOGW("ion_custom_ioctl Double config after map phy address");
    }
    else if(err != 0)
    {
        MY_LOGW("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
    }
    //c. map physical address
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = ion_handle;
    sys_data.get_phys_param.phy_addr = M4U_PORT_VPU << 24 | ION_FLAG_GET_FIXED_PHYS;
    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
    ATRACE_BEGIN("VpuUtil::getPhys");
    if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
        ATRACE_END();
        MY_LOGE("ion_custom_ioctl get_phys_param failed!");
        return ret;
    }
    ATRACE_END();

    ret = sys_data.get_phys_param.phy_addr;

    ATRACE_BEGIN("vpuStream::ion_free");
    //c. free ion handle
    if(ion_free(gIonDevFD, ion_handle))
    {
        MY_LOGE("ion_free fail");
    }
    ATRACE_END();
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuUtil::
~VpuUtil()
{
    ion_close(gIonDevFD);
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
void
VpuRequestImp::
mmapMVA()
{
    ATRACE_CALL();
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    ion_user_handle_t   ion_handle;
    int err, buf_idx, plane_idx;
    vpu_request_buffer_t *req_buf;
    vpu_buffer_t         *buf_n;

    for (buf_idx = 0 ; buf_idx < mNative.buffer_count ; buf_idx++)
    {
        buf_n   = &mNative.buffers[buf_idx];
        req_buf = &mRequestBuffer [buf_idx];
        for (plane_idx = 0 ; plane_idx < buf_n->plane_count ; plane_idx++)
        {
            //a. get handle from IonBufFd and increase handle ref count
            ATRACE_BEGIN("vpuStream::ion_import");
            if(ion_import(gIonDevFD, req_buf->plane[plane_idx].share_fd, &ion_handle))
            {
                ATRACE_END();
                MY_LOGE("ion_import fail, ion_handle(0x%x)", ion_handle);
                continue;
            }
            ATRACE_END();
            MY_LOGV("ion_import: buffer[%d], plane[%d], share_fd %d, ion_handle %d", buf_idx, plane_idx, req_buf->plane[plane_idx].share_fd, ion_handle);
            //b. config buffer
            mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
            mm_data.config_buffer_param.handle      = ion_handle;
            mm_data.config_buffer_param.eModuleID   = M4U_PORT_VPU ;
            mm_data.config_buffer_param.security    = 0;
            mm_data.config_buffer_param.coherent    = 1;
            mm_data.config_buffer_param.reserve_iova_start = VPU_MVA_START;
            mm_data.config_buffer_param.reserve_iova_end   = VPU_MVA_END;
            ATRACE_BEGIN("vpuStream::config_ion_buffer");
            err = ion_custom_ioctl(gIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
            ATRACE_END();
            if(err == (-ION_ERROR_CONFIG_LOCKED))
            {
                MY_LOGW("ion_custom_ioctl Double config after map phy address");
            }
            else if(err != 0)
            {
                MY_LOGW("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
            }
            //c. map physical address
            sys_data.sys_cmd = ION_SYS_GET_PHYS;
            sys_data.get_phys_param.handle = ion_handle;
            sys_data.get_phys_param.phy_addr = M4U_PORT_VPU << 24 | ION_FLAG_GET_FIXED_PHYS;
            sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
            ATRACE_BEGIN("vpuStream::getPhys");
            if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
            {
                ATRACE_END();
                MY_LOGE("ion_custom_ioctl get_phys_param failed!");
                continue;
            }
            ATRACE_END();
            req_buf->plane[plane_idx].ion_handle = ion_handle;
            req_buf->plane[plane_idx].mva        = sys_data.get_phys_param.phy_addr;
            buf_n->planes[plane_idx].ptr         = req_buf->plane[plane_idx].mva + req_buf->plane[plane_idx].offset;
            MY_LOGV("get_phys: buffer[%d], plane[%d], mva 0x%lx", buf_idx, plane_idx, (unsigned long)buf_n->planes[plane_idx].ptr);
        }
    }
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
                ATRACE_BEGIN("vpuStream::ion_free");
                if(ion_free(gIonDevFD, req_buf->plane[plane_idx].ion_handle))
                {
                    MY_LOGE("ion_free fail");
                }
                ATRACE_END();
                MY_LOGV("ion_free: buffer[%d], plane[%d], ion_handle %d", buf_idx, plane_idx, req_buf->plane[plane_idx].ion_handle);
                buf_n->planes[plane_idx].ptr = -1;
                req_buf->plane[plane_idx].ion_handle = -1;
            }
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuStreamImp::
VpuStreamImp()
{
    mFd = open("/dev/vpu", O_RDONLY);
    if (mFd < 0)
    {
      MY_LOGE("open vpu fail, return %d", mFd);
    }
    VpuUtil::getInstance();    // constructor
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgo *
VpuStreamImp::
getAlgo(
    char *name
)
{
    return VpuAlgoPool::getInstance()->getAlgo(std::string(name), mFd);
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequest *
VpuStreamImp::
acquire(
    VpuAlgo *algo
)
{
    return (algo == NULL ? NULL : ((VpuAlgoImp *)algo)->acquire());
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
    ((VpuRequestImp *)request)->getAlgo()->release(request);
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
    ATRACE_CALL();
    std::unique_lock <std::mutex> l(mRequestMutex);
    mRequests.push_back(request);
    mRequestCond.notify_one();
    ((VpuRequestImp *) request)->mmapMVA();
    vpu_request_t *req_n = ((VpuRequestImp *)request)->getNative();
{
    int i, j ;
    MY_LOGV("buffer_count = %d, sett_ptr 0x%lx, sett_length %d", req_n->buffer_count, (unsigned long)req_n->sett_ptr, req_n->sett_length);
    for (i = 0 ; i < req_n->buffer_count; i++)
    {
        MY_LOGV("buffer[%d]: plane_count = %d, port_id %d, w %d, h %d, fmt %d", i, req_n->buffers[i].plane_count, req_n->buffers[i].port_id, req_n->buffers[i].width, req_n->buffers[i].height, req_n->buffers[i].format);
        for (j = 0 ; j < req_n->buffers[i].plane_count; j++)
        {
            MY_LOGV("plane[%d]: ptr 0x%lx, length %d, stride %d", j, (unsigned long)req_n->buffers[i].planes[j].ptr, req_n->buffers[i].planes[j].length, req_n->buffers[i].planes[j].stride);
        }
    }
}
    int ret = ioctl(mFd, VPU_IOCTL_ENQUE_REQUEST, req_n);
    if (ret < 0) {
        MY_LOGE("fail to enque, %s, errno = %d", strerror(errno), errno);
        return false;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuRequest *
VpuStreamImp::
deque()
{
    ATRACE_CALL();
    VpuRequest *req;
    {
        std::unique_lock <std::mutex> l(mRequestMutex);
        if (mRequests.size() == 0)
        {
            mRequestCond.wait(l);
        }
        req = mRequests.front();
        mRequests.erase(mRequests.begin());
    }
    vpu_request_t *req_n = ((VpuRequestImp *) req)->getNative();
    int ret = ioctl(mFd, VPU_IOCTL_DEQUE_REQUEST, req_n);
    if (ret < 0) {
        MY_LOGE("fail to deque, %s, errno = %d", strerror(errno), errno);
        return NULL;
    }
    ((VpuRequestImp *) req)->munmapMVA();
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
    int ret = ioctl(mFd, VPU_IOCTL_FLUSH_REQUEST, 0);
    if (ret < 0)
    {
        MY_LOGE("fail to flush request, %s, errno = %d", strerror(errno), errno);
    }
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
    ATRACE_CALL();
    vpu_power_t vpuPower;
    vpuPower.mode = pwr.mode == ePowerModeOn ? VPU_POWER_MODE_ON : VPU_POWER_MODE_DYNAMIC;
    vpuPower.opp  = pwr.opp;
    int ret = ioctl(mFd, VPU_IOCTL_SET_POWER, &vpuPower);
    if (ret < 0) {
        MY_LOGE("fail to setPower, %s, errno = %d", strerror(errno), errno);
        return false;
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuStreamImp::getVpuStatus(
	std::vector<VpuStatus>& vpu_status_info
)
{
	MY_LOGE("not support this function(getVpuStatus) in 1.0");
	return false;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuStreamImp::
~VpuStreamImp()
{
    close(mFd);

    std::vector<VpuRequest *>::iterator iter;

    while(!mRequests.empty())
    {
        iter = mRequests.begin();
        ((VpuRequestImp *)(*iter))->munmapMVA();
        mRequests.erase(iter);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VpuStream *
VpuStream::
createInstance(void)
{
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
        fd = open("/proc/device-tree/chosen/atag,devinfo", O_RDONLY);
        if(fd < 0)
        {
            MY_LOGE("/proc/device-tree/chosen/atag,devinfo kernel open fail, errno(%d):%s",errno,strerror(errno));
			return NULL;
        }
        else
        {
            if ((ret = read(fd, (void *)&devinfo, sizeof(DEVINFO_S))) < 0)
            {
                MY_LOGE("Get Devinfo Data Fail:%d\n", ret);
				return NULL;
            }
            else
            {
                bonding_info = devinfo.data[3];
				mDisVPU = (bonding_info & 0x01000000)>>24;
            }
            MY_LOGI("Efuse Data:0x%x,0x%x\n", bonding_info, mDisVPU);

            close(fd);
        }
		if(!mDisVPU)
		{
			return new VpuStreamImp();
		}
		else
		{
			MY_LOGE("Efuse ... :0x%x,0x%x\n", bonding_info, mDisVPU);
			return NULL;
		}
    }
	
#else
	return new VpuStreamImp();
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
    return VpuUtil::getInstance()->mapPhyAddr(fd);
}
