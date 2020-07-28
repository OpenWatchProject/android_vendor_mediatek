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
#include <ion.h> // interface for mtk ion
#include <errno.h>
#include <sys/types.h>

#include "vpuImpl.h"

static int32_t gIonDevFD = -1;
#define DEFAULT_CALLER_NAME "defaultcaller"
//#define VPU_DUMP_SETT_INFO
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
#define MY_LOGD(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
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

//#define VPU_FIX_M4U_MVA
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
	#if 0
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
	#endif
	MY_LOGW("keyName_%s_0x%x 2.0 not support this function", keyName, prop.count);
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
	#if 0
    int i;
    vpu_algo_t *algo_n = &mNative;
    if (mbPortBuild == false)
    {
        vpu_port_t *port_n;
        for (int i = 0; i < algo_n->port_count; i++)
        {
            port_n = &algo_n->ports[i];
            MY_LOGV("  Port[%02d]: name=%s, dir=%d, usage=%d", \
				port_n->id, port_n->name, port_n->dir, port_n->usage);
            mPorts[i].id   = port_n->id;
            mPorts[i].name = port_n->name;
            mPorts[i].portDirection = port_n->dir == VPU_PORT_DIR_IN  ? eDirectionIn :
                                      port_n->dir == VPU_PORT_DIR_OUT ? eDirectionOut : eDirectionInOut ;
            mPorts[i].portUsage     = port_n->usage == VPU_PORT_USAGE_IMAGE ? eImageBuffer : eDataBuffer ;
        }
        mbPortBuild = true;
    }

    VpuPort *pPort;
	#if 0 //CHRISTODO, work_around
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
	#else
	pPort = &mPorts[work_around_index];
	MY_LOGV(" %d, Port[%02d]: name=%s, dir=%d, usage=%d", work_around_index, \
				pPort->id, pPort->name, pPort->portDirection, pPort->portUsage);
	work_around_index++;
	return pPort;
	#endif
	#endif
	MY_LOGW("name=%s, 2.0 not support this function", name);
    return NULL;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuAlgoImp::
~VpuAlgoImp()
{
    vpuRequestList_t::iterator iter;
    MY_LOGV("delete algo: 0x%lx, algo name %s, id: 0x%lx", (unsigned long)this, mNative.name, (unsigned long)mNative.id);
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
	MY_LOGV("acquire +");
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
		MY_LOGV("acquire reset");
    }
    else
    {
        req = new VpuRequestImp(this);
        mInUseRequest.push_back(req);
    }
	MY_LOGV("acquire -");
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
VpuRequestImp(VpuAlgoImp *algo)
    : mAlgo(algo),
      mBufferCount(0)

{
    memset(mRequestBuffer, 0, sizeof(mRequestBuffer));
	mSetting.share_fd= -1;
	mSetting.ion_handle = -1;
	mSetting.mva = 0x0;
	mSetting.va = 0x0;
	/*mSetting.size = 0x0;*/

	mNative.frame_magic = 0;
	mNative.buffer_count = 0;
	mNative.status  = VPU_REQ_STATUS_INVALID;
	mNative.priv    = 0;
	memset(mNative.buffers, 0, sizeof(mNative.buffers));
	mNative.user_id = NULL;
	mNative.request_id = (unsigned long)(&mNative);
	mNative.requested_core = eIndex_NONE;
	mNative.occupied_core = eIndex_NONE;

	mNative.power_param.opp_step = 0xff;
	mNative.power_param.freq_step = 0xff;
	mNative.power_param.bw = 0x0;

	for(int i = 0;i<MAX_VPU_CORE_NUM;i++)
	{
		mNative.algo_id[i] = mAlgo->getId(i);
	}
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VpuRequestImp::
prepareSettBuf(
	int size
)
{
	MY_LOGD("+ (0x%x)", mSetting.ion_handle);
	bool ret = true;
	if(mSetting.ion_handle > -1)
	{
		return ret;
	}
	else
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
				ret = false;
	        goto EXIT;
	    }
	    // a. alloc ion buffer
	    ATRACE_BEGIN("vpuStream::ion_alloc_mm");
	    if (ion_alloc_mm(gIonDevFD, size, 0, 0, &ion_handle))
	    {
	        ATRACE_END();
	        MY_LOGE("alloc ion buffer fail!");
				ret = false;
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
				ret = false;
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
				ret = false;
	        goto EXIT;
	    }
	    // d. config buffer

	    #ifdef VPU_FIX_M4U_MVA
	    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
	    mm_data.config_buffer_param.reserve_iova_start = VPU_MVA_START;
	    mm_data.config_buffer_param.reserve_iova_end   = VPU_MVA_END;
	    #else
		mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
		#endif
		mm_data.config_buffer_param.handle      = ion_handle;
	    mm_data.config_buffer_param.eModuleID   = M4U_PORT_VPU1;
	    mm_data.config_buffer_param.security    = 0;
	    mm_data.config_buffer_param.coherent    = 0;

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
	    #ifdef VPU_FIX_M4U_MVA
	    sys_data.get_phys_param.phy_addr = M4U_PORT_VPU1 << 24 | ION_FLAG_GET_FIXED_PHYS;
	    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
		#else
	    #endif
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
	    /*mSetting.size				= size;*/
	    MY_LOGI("mSetting: mva 0x%x, va 0x%lx, ion_handle %d, share_fd %d", mSetting.mva, (unsigned long)mSetting.va, mSetting.ion_handle, mSetting.share_fd);
		MY_LOGV("+ buffer size: %d/%d", mNative.buffer_count, mBufferCount);
		for(int i = 0;i<MAX_VPU_CORE_NUM;i++)
		{
			mNative.algo_id[i]      = mAlgo->getId(i);
		}
		MY_LOGV("- buffer size: %d/%d", mNative.buffer_count, mBufferCount);
	    mNative.sett_ptr     = mSetting.mva;
	    mNative.sett_length  = size;

		return ret;
	}
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

	if(mBufferCount == 32)
	{
		MY_LOGE("vpu buffer number would be larger than (%d) after this\n", mBufferCount);
		return false;
	}

	if(buffer.planeCount > 3) {
		MY_LOGE("vpu wrong planeCount(%d)\n", buffer.planeCount);
		return false;
	} else {
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
		MY_LOGD("buffer count 0x%lx, (%d/%d)",(unsigned long)&mNative, \
			mNative.buffer_count, mBufferCount);
		//

		MY_LOGD("mmapMVA+ (%d/%d)", buf_n->port_id, buffer.port_id);
		ret = mmapMVA(); //map mva one time
		MY_LOGD("mmapMVA- (%d)", ret);
		if(ret != 0) {
			result = false;
		}
		//
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
#if 0
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

	//
	MY_LOGI("mmapMVA+");
	mmapMVA(); //map mva one time
	MY_LOGI("mmapMVA-");
	//

    return;
#else
	//to avoid build warning
	port = NULL;
	buffer.port_id = 0x0;
	MY_LOGW("2.0 not support this function");
	return ;
#endif
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
#if 0
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
#else
		MY_LOGW("keyName_%s_0x%x 2.0 do not support this function in 2.0 version", keyName, prop.count);
		return false;
#endif
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
#if 0
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
#endif
	MY_LOGW("keyName_%s_0x%x, 2.0 not support this function", keyName, prop.count);
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
	MY_LOGI("+, pre(0x%lx,0x%lx), size(%d)",(unsigned long)mSetting.va,\
		(unsigned long)sett_ptr, size);
	bool ret;
	if(sett_ptr!=NULL)
	{
		//
		ret = prepareSettBuf(size);
		if(ret)
		{
			MY_LOGD("1.1");
			memcpy((void *) (uintptr_t)(mSetting.va), (void*)(uintptr_t)(sett_ptr), size);
			MY_LOGD("1.2");
			#ifdef VPU_DUMP_SETT_INFO
			for(int i=0;i<size/sizeof(int);i++)
			{
				MY_LOGV("(%d), value: (0x%x/0x%x)", i, *((int*)sett_ptr+i), *((int*)(mSetting.va)+i));
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
			MY_LOGE("prepareSettBuf fail \n");
			return false;
		}
	}
	else
	{
		MY_LOGE("setting_ptr is null \n");
		return false;
	}
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
	MY_LOGD("+ 0x%x/0x%x/0x%x", extra_param.opp_step,\
		extra_param.freq_step, extra_param.bw);

	mNative.power_param.opp_step = extra_param.opp_step;
	mNative.power_param.freq_step = extra_param.freq_step; //no use, would delete soon
	mNative.power_param.bw = extra_param.bw;
	return true;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuRequestImp::
~VpuRequestImp()
{
    MY_LOGI("delete request: 0x%lx/0x%lx, request algo_id 0x%lx", 
		(unsigned long)this,(unsigned long)mNative.request_id, (unsigned long)mNative.algo_id);
    if (mNative.sett_length)
    {
        ion_munmap(gIonDevFD, (char*)mSetting.va, mNative.sett_length);
        ion_share_close(gIonDevFD, mSetting.share_fd);
        if (ion_free(gIonDevFD, mSetting.ion_handle))
        {
            MY_LOGE("ion_free failed(share_fd %d, ion_handle %d)!", mSetting.share_fd, mSetting.ion_handle);
        }
        MY_LOGI("ion_free: mSetting, ion_handle %d", mSetting.ion_handle);
		mSetting.share_fd= -1;
		mSetting.ion_handle = -1;
		mSetting.mva = 0x0;
		mSetting.va = 0x0;
		/*mSetting.size = 0;*/
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
    vpuAlgoMap_t::iterator iter;
    vpuRequestList_t listRequest;
	MY_LOGI("getAlgo(%s) in", name.c_str());
    if ((iter = mvVpuAlgoMap.find(name)) == mvVpuAlgoMap.end())    // not in Pool
    {
        VpuAlgoImp *algo = new VpuAlgoImp();
        vpu_algo_t *algo_n = algo->getNative();
        strncpy(algo_n->name, name.c_str(), sizeof(algo_n->name));
        algo_n->name[sizeof(algo_n->name) -1] = '\0';
        MY_LOGV("algo: 0x%lx, algo name %s", (unsigned long)algo, algo_n->name);
        MY_LOGV("fd: %d, algo name %s", fd, algo_n->name);
		int ret = ioctl(fd, VPU_IOCTL_LOAD_ALG_TO_POOL, algo_n);
        if (ret < 0)
        {
            MY_LOGE("fail to get algo, %s, errno = %d", strerror(errno), errno);
            delete algo;
            return NULL;
        }
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
        MY_LOGE("ion_import fail, share_fd(0x%x)", fd);
        return ret;
    }
    ATRACE_END();
    MY_LOGV("ion_import: share_fd %d, ion_handle %d", fd, ion_handle);
    //b. config buffer
    #ifdef VPU_FIX_M4U_MVA
	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.reserve_iova_start = VPU_MVA_START;
    mm_data.config_buffer_param.reserve_iova_end   = VPU_MVA_END;
	#else
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    #endif
	mm_data.config_buffer_param.handle      = ion_handle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_VPU1 ;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 0;

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
	#ifdef VPU_FIX_M4U_MVA
    sys_data.get_phys_param.phy_addr = M4U_PORT_VPU1 << 24 | ION_FLAG_GET_FIXED_PHYS;
    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
    #else
	#endif
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
int
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
	int cnt = 0;

    for (buf_idx = 0 ; buf_idx < mNative.buffer_count ; buf_idx++)
    {
        buf_n   = &mNative.buffers[buf_idx];
        req_buf = &mRequestBuffer [buf_idx];
        for (plane_idx = 0 ; plane_idx < buf_n->plane_count ; plane_idx++)
        {
        	if(req_buf->plane[plane_idx].ion_handle != -1)
        	{}
			else
			{
	            //a. get handle from IonBufFd and increase handle ref count
	            ATRACE_BEGIN("vpuStream::ion_import");
	            if(ion_import(gIonDevFD, req_buf->plane[plane_idx].share_fd, &ion_handle))
	            {
	                ATRACE_END();
	                MY_LOGE("ion_import fail(%d/%d), fd(0x%0x)",
						buf_idx, plane_idx,
						req_buf->plane[plane_idx].share_fd);
	                //continue;
	                return -1;
	            }
	            ATRACE_END();
	            MY_LOGV("ion_import: buffer[%d], plane[%d], share_fd %d, ion_handle(0x%x)", buf_idx, plane_idx, req_buf->plane[plane_idx].share_fd, ion_handle);
	            //b. config buffer
	            #ifdef VPU_FIX_M4U_MVA
				mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
	            mm_data.config_buffer_param.reserve_iova_start = VPU_MVA_START;
	            mm_data.config_buffer_param.reserve_iova_end   = VPU_MVA_END;
				#else
	            mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	            #endif
				mm_data.config_buffer_param.handle      = ion_handle;
	            mm_data.config_buffer_param.eModuleID   = M4U_PORT_VPU1 ;
	            mm_data.config_buffer_param.security    = 0;
	            mm_data.config_buffer_param.coherent    = 0;

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
					return -1;
	            }
	            //c. map physical address
	            sys_data.sys_cmd = ION_SYS_GET_PHYS;
	            sys_data.get_phys_param.handle = ion_handle;
				#ifdef VPU_FIX_M4U_MVA
	            sys_data.get_phys_param.phy_addr = M4U_PORT_VPU1 << 24 | ION_FLAG_GET_FIXED_PHYS;
	            sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
	            #else
				#endif
	            ATRACE_BEGIN("vpuStream::getPhys");
	            if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
	            {
	                ATRACE_END();
	                MY_LOGE("ion_custom_ioctl get_phys_param failed!");
	                //continue;
	                return -1;
	            }
	            ATRACE_END();
	            req_buf->plane[plane_idx].ion_handle = ion_handle;
	            req_buf->plane[plane_idx].mva        = sys_data.get_phys_param.phy_addr;
	            buf_n->planes[plane_idx].ptr         = req_buf->plane[plane_idx].mva + req_buf->plane[plane_idx].offset;
	            MY_LOGV("get_phys: buffer[%d], plane[%d], mva 0x%lx", buf_idx, plane_idx, (unsigned long)buf_n->planes[plane_idx].ptr);
	        }
			mNative.buf_ion_infos[cnt] = (uint64_t)req_buf->plane[plane_idx].share_fd;
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
bool
VpuRequestImp::
cacheSync()
{
	MY_LOGV("cacheSync share_fd/(0x%x) +", mSetting.share_fd);
    struct ion_sys_data sys_data;
    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle=(ion_user_handle_t)mSetting.ion_handle;
    sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;
    sys_data.cache_sync_param.va = (void *)mSetting.va;
    sys_data.cache_sync_param.size = mNative.sett_length;/*mSetting.size;*/
	ATRACE_BEGIN("vpuStream:: cacheSync");
	if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
    	ATRACE_END();
        MY_LOGE("CAN NOT DO SYNC, share_fd/(0x%x)", mSetting.share_fd);
        return false;
    }
	else
	{
		ATRACE_END();
		MY_LOGV("-");
		return true;
	}
}

/******************************************************************************
 *
 ******************************************************************************/
VpuStreamImp::
VpuStreamImp(const char* callername)
{
    mFd = open("/dev/vpu", O_RDONLY);
    if (mFd < 0)
    {
      MY_LOGE("===================================================================\n");
	  MY_LOGE("===================================================================\n");
	  MY_LOGE("===================================================================\n");
      MY_LOGE("open vpu fail, return %d, errno(%d):%s\n", mFd, errno, strerror(errno));
	  MY_LOGE("===================================================================\n");
	  MY_LOGE("===================================================================\n");
	  MY_LOGE("===================================================================\n");
    }
	else
	{
		vpu_dev_debug_info dev_debug_info;
		dev_debug_info.dev_fd = mFd;
		strncpy(dev_debug_info.callername, callername, sizeof(dev_debug_info.callername));
		dev_debug_info.callername[sizeof(dev_debug_info.callername) -1] = '\0';
		int ret = ioctl(mFd, VPU_IOCTL_OPEN_DEV_NOTICE, &dev_debug_info);
        if (ret < 0)
        {
            MY_LOGE("fd(%d) fail to notice open device, %s, errno = %d",
				mFd, strerror(errno), errno);
        }
	}
	MY_LOGI(" user_%s open mFd (%d)", callername, mFd);
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
	if(request == NULL) {
		MY_LOGE("null request to release\n");
	} else {
		MY_LOGD("[vpu] release req_ref_0x%lx",
			(unsigned long)(((VpuRequestImp *)request)->getNative()->request_id));
		((VpuRequestImp *)request)->munmapMVA(); //unmap buffer first
	    ((VpuRequestImp *)request)->getAlgo()->release(request);
	}
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
    //((VpuRequestImp *) request)->mmapMVA();
    vpu_request_t *req_n = ((VpuRequestImp *)request)->getNative();
	if(req_n->frame_magic == 65536)
		req_n->frame_magic = 0;
	else
		req_n->frame_magic += 1;
	MY_LOGI("[vpu] enque algo:ref_%d/0x%lx/%d, expected core:0x%x, opp/bw(0x%x/0x%x)",
		req_n->algo_id[0], (unsigned long)req_n->request_id, req_n->frame_magic, core_index,
		req_n->power_param.opp_step, req_n->power_param.bw);
{
    int i, j ;
    MY_LOGV("buffer_count = %d, sett_ptr 0x%lx, sett_length %d", \
		req_n->buffer_count, (unsigned long)req_n->sett_ptr, req_n->sett_length);
	if(req_n->buffer_count == 0)
	{
		MY_LOGW("buffer count = 0, WRN"); //testcase_max_power do not need buffers
		//return false;
	}
    for (i = 0 ; i < req_n->buffer_count; i++)
    {
        MY_LOGV("buffer[%d]: plane_count = %d, port_id %d, w %d, h %d, fmt %d", i, req_n->buffers[i].plane_count, req_n->buffers[i].port_id, req_n->buffers[i].width, req_n->buffers[i].height, req_n->buffers[i].format);
        for (j = 0 ; j < req_n->buffers[i].plane_count; j++)
        {
            MY_LOGV("plane[%d]: ptr 0x%lx, length %d, stride %d", j, (unsigned long)req_n->buffers[i].planes[j].ptr, req_n->buffers[i].planes[j].length, req_n->buffers[i].planes[j].stride);
        }
    }
}
	req_n->requested_core = core_index;
    int ret = ioctl(mFd, VPU_IOCTL_ENQUE_REQUEST, req_n);
    if (ret < 0) {
        MY_LOGE("core(0x%x/0x%x) fail to enque, %s, errno = %d", \
			core_index, req_n->requested_core, strerror(errno), errno);
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
	bool result = true;
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

    // Set deque core index as MET log to ATRACE
    char met_deque_log[64] = {0};
    memset(met_deque_log,0,sizeof(met_deque_log));
    // cause ATRACE's vpu index start from 0, but occupied_core start from 1
    sprintf(met_deque_log,"deque from vpu%d",req_n->occupied_core-1);

    ATRACE_NAME(met_deque_log);

	MY_LOGI("[vpu] algo_ref_%d/0x%lx/%d, occupied core_index 0x%x/0x%x, sts(%d)",
		req_n->algo_id[0], (unsigned long)req_n->request_id,
		req_n->frame_magic, req_n->requested_core, req_n->occupied_core, req_n->status);

	switch(req_n->status) {
	case VPU_REQ_STATUS_SUCCESS:
		result = true;
		break;
	case VPU_REQ_STATUS_BUSY:
	case VPU_REQ_STATUS_TIMEOUT:
	case VPU_REQ_STATUS_INVALID:
	case VPU_REQ_STATUS_FLUSH:
	case VPU_REQ_STATUS_FAILURE:
	default:
		MY_LOGE("[vpu] algo_ref_%d_%d, ocp core_index 0x%x/0x%x, FAILED sts(%d)",
			req_n->algo_id[0], req_n->frame_magic, req_n->requested_core, req_n->occupied_core, req_n->status);
		result = false;
		break;
	}

    //((VpuRequestImp *) req)->munmapMVA();
	if(result) {
    	return req;
	} else {
		return NULL;
	}
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
	#if 0
    vpu_power_t vpuPower;
    vpuPower.mode = pwr.mode == ePowerModeOn ? VPU_POWER_MODE_ON : VPU_POWER_MODE_DYNAMIC;
    vpuPower.opp  = pwr.opp;
    int ret = ioctl(mFd, VPU_IOCTL_SET_POWER, &vpuPower);
    if (ret < 0) {
        MY_LOGE("fail to setPower, %s, errno = %d", strerror(errno), errno);
        return false;
    }
	#else
	struct vpu_power power_param;
	power_param.opp_step = pwr.opp;
	power_param.freq_step = 0xFF;
	power_param.bw = 0;
	power_param.core = core_index;
	MY_LOGD("[vpu] setPower opp_step_%d/core_%d", pwr.opp, core_index);
	int ret = ioctl(mFd, VPU_IOCTL_SET_POWER, &power_param);
    if (ret < 0) {
        MY_LOGE("fail to setPower, %s, errno = %d", strerror(errno), errno);
        return false;
    }
	#endif
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
	int ret = 0;
	for(int i = vpu_status_info.size()-1 ; i>=0; i--)
		vpu_status_info.pop_back();

	/* individual cores */
	#if 0
	unsigned int index_max = static_cast<unsigned int>(eIndex_MAX);
	printf("eIndex_MAX(0x%x/0x%x), max_core_num(%d) \n",index_max, eIndex_MAX,max_core_num);
	do{
		if((int)(index_max) > 0) {
			max_core_num += 1;
			index_max = index_max >> 1;
		} else {
			getmax=true;
		}
		printf("index_max(0x%x), getmax(%d), max_core_num(%d)\n",index_max, getmax, max_core_num);

	}while(!getmax);
	printf("after eIndex_MAX(0x%x/0x%x), max_core_num(%d) \n",index_max, eIndex_MAX,max_core_num);
	#endif
	for(int i = 0 ; i < MAX_VPU_CORE_NUM ; i++)
	{
		vpu_status_t vpu_status;
		vpu_status.vpu_core_index = i; //kernel pool-based index, 0, 1, 2 //(0x1 << i);
		ret = ioctl(mFd, VPU_IOCTL_GET_CORE_STATUS, &vpu_status);
		if(ret < 0) {
			if (errno == EINVAL) {
				/*hw not support this core*/
				MY_LOGW("warning to get vpu_%d status, %s, errno = %d\n", vpu_status.vpu_core_index,
					strerror(errno), errno);
				continue;
			} else {
				MY_LOGE("fail to get vpu_%d status, %s, errno = %d\n", vpu_status.vpu_core_index,
					strerror(errno), errno);
        		return false;
			}
		}
		VpuStatus status;
		status.core_index = static_cast<VpuCoreIndex>(vpu_status.vpu_core_index);
		status.available = vpu_status.vpu_core_available;
		status.waiting_requests_num = vpu_status.pool_list_size;
		vpu_status_info.push_back(status);
	}

	/* common pool */
	if((int)vpu_status_info.size() > 0) {
	vpu_status_t vpu_status;
	vpu_status.vpu_core_index = static_cast<int>(eIndex_NONE);
	ret = ioctl(mFd, VPU_IOCTL_GET_CORE_STATUS, &vpu_status);
	if(ret < 0) {
		MY_LOGE("fail to get vpu status, %s, errno = %d", strerror(errno), errno);
       	return false;
	}
	VpuStatus status;
	status.core_index = static_cast<VpuCoreIndex>(vpu_status.vpu_core_index);
	status.available = vpu_status.vpu_core_available;
	status.waiting_requests_num = vpu_status.pool_list_size;
	vpu_status_info.push_back(status);
	} else {
		MY_LOGE("do not support VPU HW..\n");
		return false;
	}

	return true;
}

/******************************************************************************
 *
 ******************************************************************************/
VpuStreamImp::
~VpuStreamImp()
{
	int fd;
	fd = mFd;

	MY_LOGI("close mFd (%d)",mFd);
	int ret = ioctl(mFd, VPU_IOCTL_CLOSE_DEV_NOTICE, &fd);
    if (ret < 0)
    {
    	MY_LOGE("fd(%d/%d) fail to notice close device, %s, errno = %d",
			fd, mFd, strerror(errno), errno);
    }

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
#if 0
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
            MY_LOGI("version(%d), Data:0x%x,0x%x, %d\n", HAVE_EFUSE_BOND_VERSION,
				bonding_info, mDisVPU, vpu_hw_support);
            close(fd);
        }
		if(vpu_hw_support)
		{
			return new VpuStreamImp(DEFAULT_CALLER_NAME);
		}
		else
		{
			MY_LOGE("Do not support VPU HW:0x%x,0x%x, %d\n", bonding_info, mDisVPU, vpu_hw_support);
			printf("Do not support VPU HW:0x%x,0x%x, %d\n", bonding_info, mDisVPU, vpu_hw_support);
			return NULL;
		}
    }

#else
	MY_LOGD("vpu user createInstance\n");
	return new VpuStreamImp(DEFAULT_CALLER_NAME);
#endif
#endif
	return createInstance(DEFAULT_CALLER_NAME);
}

VpuStream *
VpuStream::
createInstance(const char* callername)
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
		bool vpu_hw_support = true;
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
            MY_LOGI("version(%d), Data:0x%x,0x%x, %d", HAVE_EFUSE_BOND_VERSION,
				bonding_info, mDisVPU, vpu_hw_support);
            close(fd);
        }
		if(vpu_hw_support)
		{
			return new VpuStreamImp(callername);
		}
		else
		{
			MY_LOGE("Do not support VPU HW:0x%x,0x%x, %d\n", bonding_info, mDisVPU, vpu_hw_support);
			return NULL;
		}
    }

#else
	MY_LOGD("vpu user createInstance\n");
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
    return VpuUtil::getInstance()->mapPhyAddr(fd);
}
