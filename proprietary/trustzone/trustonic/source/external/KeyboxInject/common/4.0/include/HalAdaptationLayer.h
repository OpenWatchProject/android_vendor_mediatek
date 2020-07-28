/*
 * Copyright (C) 2016 MediaTek Inc.
 */

#ifndef _HalAdaptationLayer_H_
#define _HalAdaptationLayer_H_

#include <memory>
#include <stdlib.h>
#include <hardware/hardware.h>
#include "kmsetkey.h"


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
	static int32_t attest_key_check(const uint8_t *peakb, const uint32_t peakb_len);

	HalAdaptationLayer(hw_module_t* module);

private:
	// Struct is non-copyable and not default constructible
	HalAdaptationLayer();
	HalAdaptationLayer(const HalAdaptationLayer&);
	HalAdaptationLayer& operator=(const HalAdaptationLayer&);
};

#endif /* _HalAdaptationLayer_H_ */
