/*
 * Copyright (c) 2016, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include "HalAdaptationLayer.h"
#include "module.h"
#include <memory>
#include <cutils/log.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <log/log.h>
#include <cutils/log.h>
#include <kmsetkey_ipc.h>

extern struct kmsetkey_module HAL_MODULE_INFO_SYM;

extern "C" {

/******************************************************************************/
__attribute__((visibility("default")))
int nv_kmsetkey_open( const struct hw_module_t* module, const char* id,
					struct hw_device_t** device)
{
	ALOGI("opening nv kmsetkey device.\n");

	if ( id == NULL )
		return -EINVAL;

	// Make sure we initialize only if module provided is known
	if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
			(module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
			(module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
			(0!=memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
					   sizeof(KMSETKEY_HARDWARE_MODULE_NAME)-1)) )
	{
		return -EINVAL;
	}

	std::unique_ptr<HalAdaptationLayer> kmsetkey_device(
			new HalAdaptationLayer(const_cast<hw_module_t*>(module)));

	if (!kmsetkey_device)
	{
		ALOGE("Heap exhuasted. Exiting...");
		return -ENOMEM;
	}

	*device = reinterpret_cast<hw_device_t*>(kmsetkey_device.release());
	ALOGI("Kmsetkey device created");
	return 0;
}

/******************************************************************************/
__attribute__((visibility("default")))
int nv_kmsetkey_close(hw_device_t *hw)
{
	if (hw == NULL)
		return 0; // Nothing to close closed

	HalAdaptationLayer* gk = reinterpret_cast<HalAdaptationLayer*>(hw);
	if (NULL == gk)
	{
		ALOGE("Kmsetkey not initialized.");
		return -ENODEV;
	}

	delete gk;
	return 0;
}
} // extern "C"


/* -------------------------------------------------------------------------
   Implementation of HalAdaptationLayer methods
   -------------------------------------------------------------------------*/

int32_t HalAdaptationLayer::attest_key_install(const uint8_t *peakb,
		const uint32_t peakb_len)
{
	int32_t rc;
	uint32_t payload_offset, payload_len, out_size;
	uint8_t out[MAX_MSG_SIZE];
	struct kmsetkey_msg *msg;
	int handle;

	ALOGI("enter attest keybox HAL!\n");

	rc = kmsetkey_connect(&handle);
	if (rc < 0) {
		ALOGE("kmsetkey_connect failed: %d\n", rc);
		return rc;
	}

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(handle, KEY_LEN, true, (uint8_t *)&peakb_len, sizeof(uint32_t), out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_LEN);
		goto exit;
	}

	for (payload_offset = 0; payload_offset < peakb_len; payload_offset += payload_len) {
		payload_len = peakb_len - payload_offset < MAX_MSG_SIZE - IPC_MSG_SIZE - sizeof(struct kmsetkey_msg) ? peakb_len - payload_offset : MAX_MSG_SIZE - IPC_MSG_SIZE - sizeof(struct kmsetkey_msg);
		out_size = MAX_MSG_SIZE;
		rc = kmsetkey_call(handle, KEY_BUF, payload_offset + payload_len < peakb_len ? false : true, peakb + payload_offset, payload_len, out, &out_size);
		if (rc < 0) {
			ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_BUF);
			goto exit;
		}
	}

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(handle, SET_KEY, true, NULL, 0, out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, SET_KEY);
	}

exit:
	kmsetkey_disconnect(&handle);
	return rc < 0 ? rc : 0;
}


/******************************************************************************/
HalAdaptationLayer::HalAdaptationLayer(hw_module_t* module)
{
	ALOGI("kmsetkey: HalAdaptationLayer constructor initialized.");
	/* -------------------------------------------------------------------------
	   Device description
	   -------------------------------------------------------------------------*/
	_device.common.tag = HARDWARE_MODULE_TAG;
	_device.common.version = 1;
	_device.common.module = module;
	_device.common.close = nv_kmsetkey_close;

	/* -------------------------------------------------------------------------
	   All function pointers used by the HAL module
	   -------------------------------------------------------------------------*/
	_device.attest_key_install = HalAdaptationLayer::attest_key_install;

}

/******************************************************************************/


