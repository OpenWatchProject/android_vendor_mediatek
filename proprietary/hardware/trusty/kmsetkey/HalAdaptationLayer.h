/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
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

#ifndef _HalAdaptationLayer_H_
#define _HalAdaptationLayer_H_

#include <memory>
#include <stdlib.h>
#include <hardware/hardware.h>
#include "kmsetkey.h"

#undef  LOG_TAG
#define LOG_TAG "kmsetkey"


struct HalAdaptationLayer
{
    kmsetkey_device_t          _device;

/* -----------------------------------------------------------------------------
 * @brief   An interface for key injection from HIDL to HAL.
 *
 * @param   peakb: data buffer pointer
 * @param   peakb_len: data buffer length
 *
 * @returns:
 *          ERROR_NONE: Success
 *          An error code < 0 on failure
 -------------------------------------------------------------------------------- */
	static int32_t attest_key_install(const uint8_t *peakb, const uint32_t peakb_len);

    HalAdaptationLayer(hw_module_t* module);

private:
	// Struct is non-copyable and not default constructible
	HalAdaptationLayer();
	HalAdaptationLayer(const HalAdaptationLayer&);
	HalAdaptationLayer& operator=(const HalAdaptationLayer&);
};

#endif /* _HalAdaptationLayer_H_ */
