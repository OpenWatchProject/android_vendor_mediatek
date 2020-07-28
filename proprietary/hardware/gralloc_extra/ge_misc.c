/*
 * Copyright (C) 2011-2014 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <unistd.h>

#define LOG_TAG "GE"
#include <cutils/log.h>

#include <ui/gralloc_extra.h>
#include <ged/ged_ge.h>
#include "ge_private.h"

#define GE_INCLUDE_CONFIG
#include "ge_config.h"

static int ion_custom_ioctl(int fd, unsigned int cmd, void* arg)
{
	struct ion_custom_data custom_data;
	custom_data.cmd = cmd;
	custom_data.arg = (unsigned long) arg;

	int ret = ioctl(fd, ION_IOC_CUSTOM, &custom_data);
	if (ret < 0) {
		ALOGE("ion_custom_ioctl %zx failed with code %d: %s\n", ION_IOC_CUSTOM,
		      ret, strerror(errno));
		return -errno;
	}
	return ret;
}

static int _ge_get(GEFD ge_fd, ge_info *gei, void *out)
{
	return ged_ge_get(ge_fd, gei->id, gei->offset, gei->size, out);
}
static int _ge_set(GEFD ge_fd, ge_info *gei, void *in)
{
	return ged_ge_set(ge_fd, gei->id, gei->offset, gei->size, in);
}

static int _ge_ion_get_phys(int ion_client, ion_user_handle_t ion_hnd, ge_sec_hnd_t *pphys)
{
	int ret;
	ion_sys_data_t sys_data;

	sys_data.sys_cmd = ION_SYS_GET_PHYS;
	sys_data.get_phys_param.handle = ion_hnd;

	ret = ion_custom_ioctl(ion_client, ION_CMD_SYSTEM, &sys_data);
	if (ret)
	{
		ALOGE("IOCTL[ION_IOC_CUSTOM] Get Phys failed!\n");
		return 1;
	}

#ifdef DEBUG
	ALOGI("[ge_ion_get_phys] phy_addr = 0x%x, len = %d, pphys(0x%x)\n", sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len,pphys);
#endif

	if (!ret && pphys) {
		*pphys = sys_data.get_phys_param.phy_addr;
	}

	return ret;
}

static int _ge_alloc_sec_hwc(int ion_client, size_t size, int usage, ion_user_handle_t* pion_hnd, GRALLOC_EXTRA_SECURE_BUFFER_TYPE type)
{
	int ret;
	ion_user_handle_t ion_hnd;
	unsigned int heap_mask = ION_HEAP_MULTIMEDIA_SEC_MASK;
	int ion_flags = 0;

	if ( (usage & GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN) == GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN )
	{
		ion_flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
	}

	if (type == GRALLOC_EXTRA_SECURE_BUFFER_TYPE_ZERO)
	{
		ion_flags |= ION_FLAG_MM_HEAP_INIT_ZERO;
	}

	/* require 4096 aligment */
	ret = ion_alloc(ion_client, size, 4096, heap_mask, ion_flags, &ion_hnd);

	if ( !ret && pion_hnd) *pion_hnd = ion_hnd;
	return ret;
}

static int _ge_free_sec_hwc(int ion_client, ion_user_handle_t ion_hnd)
{
	return ion_free(ion_client, ion_hnd);
}

#define MM_DBG_NAME_LEN (sizeof(((gralloc_extra_ion_debug_t *)0)->name)/sizeof(char))

static int _ge_get_ion_debug(int ion_client, ion_user_handle_t ion_hnd, ge_ion_debug_t *pdebug_info)
{
	struct ion_mm_data mm_data;

	mm_data.mm_cmd = ION_MM_GET_DEBUG_INFO;
	mm_data.buf_debug_info_param.handle = ion_hnd;

	if (ion_custom_ioctl(ion_client, ION_CMD_MULTIMEDIA, &mm_data))
	{
		ALOGE("ion_custom_ioctl fail, hnd = 0x%x", ion_hnd);
		return 1;
	}

	pdebug_info->data[0] = mm_data.buf_debug_info_param.value1;
	pdebug_info->data[1] = mm_data.buf_debug_info_param.value2;
	pdebug_info->data[2] = mm_data.buf_debug_info_param.value3;
	pdebug_info->data[3] = mm_data.buf_debug_info_param.value4;

	strncpy(pdebug_info->name, mm_data.buf_debug_info_param.dbg_name, MM_DBG_NAME_LEN);

	return 0;
}

static int _ge_set_ion_debug(int ion_client, ion_user_handle_t ion_hnd, gralloc_extra_ion_debug_t *pdebug_info)
{
	struct ion_mm_data mm_data;

	mm_data.mm_cmd = ION_MM_SET_DEBUG_INFO;
	mm_data.buf_debug_info_param.handle = ion_hnd;

	mm_data.buf_debug_info_param.value1 = pdebug_info->data[0];
	mm_data.buf_debug_info_param.value2 = pdebug_info->data[1];
	mm_data.buf_debug_info_param.value3 = pdebug_info->data[2];
	mm_data.buf_debug_info_param.value4 = pdebug_info->data[3];

	strncpy(mm_data.buf_debug_info_param.dbg_name, pdebug_info->name, MM_DBG_NAME_LEN);

	if (ion_custom_ioctl(ion_client, ION_CMD_MULTIMEDIA, &mm_data))
	{
		ALOGE("ion_custom_ioctl fail, hnd = 0x%x", ion_hnd);
		return 1;
	}

	return 0;
}

static int _ge_get_ion_client(void)
{
	static int _ion_client = -1;

	if (_ion_client == -1)
	{
		_ion_client = ion_open();
	}

	return _ion_client;
}

static int _get_ion_hnd_from_buffer_handle(int ion_client, buffer_handle_t hnd, ion_user_handle_t *out_ion_hnd)
{
	int ion_fd = -1;

	gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_ION_FD, &ion_fd);

	return ion_import(ion_client, ion_fd, out_ion_hnd);
}

static int _put_ion_hnd(int ion_client, ion_user_handle_t ion_hnd)
{
	return ion_free(ion_client, ion_hnd);
}

static int _ge_query_ion_debug(buffer_handle_t hnd, GEFD UNUSED(ge_fd), void *out)
{
	int err;
	int ion_client = _ge_get_ion_client();
	ion_user_handle_t ion_hnd = 0;

	err = _get_ion_hnd_from_buffer_handle(ion_client, hnd, &ion_hnd);

	if (!err) err = _ge_get_ion_debug(ion_client, ion_hnd, out);

	_put_ion_hnd(ion_client, ion_hnd);

	return err;
}
static int _ge_perform_ion_debug(buffer_handle_t hnd, GEFD UNUSED(ge_fd), void *in)
{
	int err;
	int ion_client = _ge_get_ion_client();
	ion_user_handle_t ion_hnd = 0;

	err = _get_ion_hnd_from_buffer_handle(ion_client, hnd, &ion_hnd);

	if (!err) err = _ge_set_ion_debug(ion_client, ion_hnd, in);

	_put_ion_hnd(ion_client, ion_hnd);

	return err;
}

static int _ge_perform_alloc_sec_hwc(buffer_handle_t hnd, GEFD ge_fd, GRALLOC_EXTRA_SECURE_BUFFER_TYPE* in)
{
	int err = -1;
	ge_sec_hwc_t sechwc;
	ge_info gei = GEI_BY_STRUCT(ge_sec_hwc_t);

	GRALLOC_EXTRA_SECURE_BUFFER_TYPE buf_type;
	buf_type = (in == NULL) ? GRALLOC_EXTRA_SECURE_BUFFER_TYPE_NORMAL : *in;

	_ge_get(ge_fd, &gei, &sechwc);

	if (sechwc.sec_hnd == 0)
	{
		int ion_client = _ge_get_ion_client();
		int size = 0, usage = 0;

		err = 0;
		err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
		err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_USAGE, &usage);

		err |= _ge_alloc_sec_hwc(ion_client, size, usage, &sechwc.ion_hnd, buf_type);

		if (!err)
		{
			_ge_ion_get_phys(ion_client, sechwc.ion_hnd, &sechwc.sec_hnd);
			sechwc.ge_fd = ge_fd;
			sechwc._64hnd = (uint64_t)hnd;
			_ge_set(ge_fd, &gei, &sechwc);
		}

	}

	return err;
}
static int _ge_perform_free_sec_hwc(buffer_handle_t hnd, GEFD ge_fd, void *UNUSED(in))
{
	ge_sec_hwc_t sechwc;
	ge_info gei = GEI_BY_STRUCT(ge_sec_hwc_t);

	_ge_get(ge_fd, &gei, &sechwc);

	if (sechwc.sec_hnd != 0 && sechwc._64hnd == (uint64_t)hnd)
	{
		int ion_client = _ge_get_ion_client();

		_ge_free_sec_hwc(ion_client, sechwc.ion_hnd);

		memset(&sechwc, 0, sizeof(ge_sec_hwc_t));
		_ge_set(ge_fd, &gei, &sechwc);
	}

	return 0;
}

static int _ge_retain(int UNUSED(ge_fd))
{
	/* do nothing */
	return 0;
}

GEFD ge_alloc(buffer_handle_t UNUSED(hnd))
{
	int ge_fd = ged_ge_alloc(region_num, region_sizes);

	_ge_retain(ge_fd);

	return ge_fd;
}

int ge_retain(buffer_handle_t hnd)
{
	ge_handle_t *ge_hnd = get_ge_handle(hnd);

	return (NULL != ge_hnd) ? _ge_retain(ge_hnd->ge_fd) : 0;
}

int ge_release(buffer_handle_t hnd)
{
	ge_handle_t *ge_hnd = get_ge_handle(hnd);

	return (NULL != ge_hnd) ? _ge_perform_free_sec_hwc(hnd, ge_hnd->ge_fd, NULL) : 0;
}

int ge_free(buffer_handle_t hnd)
{
	ge_handle_t *ge_hnd = get_ge_handle(hnd);

	return (NULL != ge_hnd) ? close(ge_hnd->ge_fd) : 0;
}

/*
 * In ge_query(), MAKE_GENERAL_CASE/MAKE_GENERAL_CASE2 are read macro.
 * In ge_perform(), MAKE_GENERAL_CASE/MAKE_GENERAL_CASE2 are write macro.
 */

/*
 * MAKE_GENERAL_CASE
 *
 * read/write info by struct
 * ex:
 *   MAKE_GENERAL_CASE(TIMESTAMP_INFO, ge_timestamp_info_t);
 *   get/set the struct "ge_timestamp_info_t"
 */
#define MAKE_GENERAL_CASE(attr, ge_struct) \
	case GE_PREFIX(attr): { \
		ge_info gei = GEI_BY_STRUCT(ge_struct); \
		return GE_GETSET_FUNC(ge_fd, &gei, GE_INOUT); \
	}
/*
 * MAKE_GENERAL_CASE2
 *
 * read/write info by struct member
 * ex:
 *   MAKE_GENERAL_CASE2(TIMESTAMP, ge_timestamp_info_t, timestamp);
 *   get/set the variable "ge_timestamp_info_t.timestamp"
 */
#define MAKE_GENERAL_CASE2(attr, ge_struct, member) \
	case GE_PREFIX(attr): { \
		ge_info gei = GEI_BY_MEMBER(ge_struct, member); \
		return GE_GETSET_FUNC(ge_fd, &gei, GE_INOUT); \
	}

#undef GE_PREFIX
#define GE_PREFIX(x) GRALLOC_EXTRA_GET_##x
#undef GE_GETSET_FUNC
#define GE_GETSET_FUNC _ge_get
#undef GE_INOUT
#define GE_INOUT out
int ge_query(buffer_handle_t hnd, GEFD ge_fd, GRALLOC_EXTRA_ATTRIBUTE_QUERY attr, void *out)
{
	switch (attr)
	{
	MAKE_GENERAL_CASE(SMVR_INFO, ge_smvr_info_t);

	MAKE_GENERAL_CASE(SF_INFO, ge_sf_info_t);

	MAKE_GENERAL_CASE(HWC_INFO, ge_hwc_info_t);

	MAKE_GENERAL_CASE2(SECURE_HANDLE_HWC, ge_sec_hwc_t, sec_hnd);

	MAKE_GENERAL_CASE(HDR_INFO, ge_hdr_info_t);

	MAKE_GENERAL_CASE(VIDEO_INFO, ge_video_info_t);

	case GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG: return _ge_query_ion_debug(hnd, ge_fd, out);

	MAKE_GENERAL_CASE2(ORIENTATION, ge_misc_info_t, orientation);

	MAKE_GENERAL_CASE2(TIMESTAMP, ge_timestamp_info_t, timestamp);
	MAKE_GENERAL_CASE(TIMESTAMP_INFO, ge_timestamp_info_t);
	MAKE_GENERAL_CASE(PQ_SCLTM_INFO, ge_pq_scltm_info_t);

	MAKE_GENERAL_CASE(PQ_MIRA_VISION_INFO, ge_pq_mira_vision_info_t);

	case GRALLOC_EXTRA_GET_ID: return ged_ge_query(ge_fd, GE_GET_ID, out);

	default:
		ALOGE("unkown attr %x", attr);
		/* abort to expose problem earlier */
		abort();
		return -1;
	}

	return 0;
}

#undef GE_PREFIX
#define GE_PREFIX(x) GRALLOC_EXTRA_SET_##x
#undef GE_GETSET_FUNC
#define GE_GETSET_FUNC _ge_set
#undef GE_INOUT
#define GE_INOUT in
int ge_perform(buffer_handle_t hnd, GEFD ge_fd, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attr, void *in)
{
	switch (attr)
	{
	MAKE_GENERAL_CASE(SMVR_INFO, ge_smvr_info_t);

	MAKE_GENERAL_CASE(SF_INFO, ge_sf_info_t);

	MAKE_GENERAL_CASE(HWC_INFO, ge_hwc_info_t);

	MAKE_GENERAL_CASE(HDR_INFO, ge_hdr_info_t);

	MAKE_GENERAL_CASE(VIDEO_INFO, ge_video_info_t);

	case GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG: return _ge_perform_ion_debug(hnd, ge_fd, in);

	case GRALLOC_EXTRA_ALLOC_SECURE_BUFFER_HWC: return _ge_perform_alloc_sec_hwc(hnd, ge_fd, in);

	case GRALLOC_EXTRA_FREE_SECURE_BUFFER_HWC: return _ge_perform_free_sec_hwc(hnd, ge_fd, in);

	MAKE_GENERAL_CASE2(ORIENTATION, ge_misc_info_t, orientation);

	MAKE_GENERAL_CASE2(TIMESTAMP, ge_timestamp_info_t, timestamp);
	MAKE_GENERAL_CASE(TIMESTAMP_INFO, ge_timestamp_info_t);
	MAKE_GENERAL_CASE(PQ_SCLTM_INFO, ge_pq_scltm_info_t);

	MAKE_GENERAL_CASE(PQ_MIRA_VISION_INFO, ge_pq_mira_vision_info_t);

	default:
		ALOGE("unkown attr %x", attr);
		/* abort to expose problem earlier */
		abort();
		return -1;
	}

	return 0;
}
