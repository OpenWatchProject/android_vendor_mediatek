/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
/**
 * Low level touch API stubs for generic platform
 */
#define TAG "[PLAT TOUCH]"

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "device.h"
#include "drTuiHal.h"
#include "drTuiCore.h"

//MTK Add for MTK common TUI driver layer API
#include "mtk_common_tpd.h"
#include "log.h"

/* Stack for ISR thread */
#define TOUCH_STACK_SIZE    4096    // allocated rounding to 4 kB pages


// defines for generic touch screen

#define TOUCH_INTR_NUM  203
#define TOUCH_EINT_NUM  1

#define HW_QUEUED_EVENTS_MAX    4
#define MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH32 0x8200020C
#define MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH64 0xC200020C
#define MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH32 0x8200020D
#define MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH64 0xC200020D

int is_extra_mem_inited();

static drTuiHalError_t touchRead(drTouchState_t *state, drTouchCoordinatesV2_t *tsXY)
{
    // TODO: Read touch event on i2c bus
    //MTK Add +++++++++
	struct tpd_data data = {0};
	int err = 0;

	if ((tpd_tui_common_obj.vendor_tpd_read_data == NULL)) {
		TUI_LOGE("touchRead vendor_tpd_read_data=NULL...\n");
		return E_TUI_HAL_INTERNAL;
	}
	err = tpd_tui_common_obj.vendor_tpd_read_data(&data);
	if(err)
		return E_TUI_HAL_INTERNAL;

	tsXY->x = data.position_x;
	tsXY->y = data.position_y;
	*state = data.finger_states;

    return TUI_HAL_OK;
}

/**
 * @brief           Get information on the touchscreen.
 *
 * This function is called by the driver core main thread after
 * tuiHalTouchOpen(). It must fill the structure information related to the
 * touchscreen.
 *
 * @param pTouchInfo Pointer on the touch info struct to fill.
 *
 * @return          TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalTouchGetInfoV2(drTouchInfoV2_t *pTouchInfo)
{
    TUI_LOGD("%s\n", __func__);

    // TODO: return real metrics
    pTouchInfo->width = 1080;
    pTouchInfo->height = 1920;
    /*
     * TODO-[2015-06-04]-[brybuc01]: size the queue according to the controller
     * sampling rate to let the CORE queue touch events during N seconds.
     *
     * Arbitrary for reference HAL/Arndale.
     */
    pTouchInfo->eventQueueCnt = HW_QUEUED_EVENTS_MAX * 2;

    return TUI_HAL_OK;
}

int wait_for_extra_mem_inited(void)
{
	do {
		if (!is_extra_mem_inited())
			drApiThreadSleep(50);
		else
			break;
	} while(true);
	return 0;
}

/**
 * Touch thread entry
 */
_THREAD_ENTRY void touchThreadEntry(void)
{
	drTouchCoordinatesV2_t tsXY = {0xFFFF,0xFFFF};
	uint32_t touching = 0;
	uint32_t intNum = 0;
	drTouchState_t state = PEN_UP;

	TUI_LOGD("%s: TOUCH thread is running\n", __func__);

	wait_for_extra_mem_inited();

	// Attach touch interrupt to SWd
	if ( DRAPI_OK != drApiIntrAttach(TOUCH_INTR_NUM, INTR_MODE_FALLING_EDGE) ) {
		TUI_LOGE("%s:%i touch interrupt attach error\n", __func__, __LINE__);
		drApiThreadSleep(TIME_INFINITE);
	}

	// Touch detection loop
	for (;;) {
		TUI_LOGV("touch detection loop...\n");
		// Unlock Touch hardware
		drTouchUnlock();

		// Wait for touch touch notification
		if ( DRAPI_OK != drApiWaitForIntr(TOUCH_INTR_NUM, TIME_INFINITE, &intNum) ) {
			TUI_LOGE("%s:%i touch interrupt detach error\n", __func__, __LINE__);
			drApiThreadSleep(TIME_INFINITE);
		}

		if(intNum != TOUCH_INTR_NUM){
			TUI_LOGE("%s:touch interrupt number is %d\n", __func__, intNum);
			continue;
		}
		TUI_LOGD("received touch event\n");

		// Lock touch hardware
		drTouchLock();

		// TODO: Acknowledge touch IRQ
		touching = 1;
		// Read touch events
		do {
			// Read touch event on i2c bus
			if (touchRead(&state, &tsXY) != TUI_HAL_OK) {
				break;
			}
			// Report touch event
			TUI_LOGV("report event(%u,%u)\n", tsXY.x, tsXY.y);
			drTouchReportV2(state, &tsXY);

			// TODO: Check if still touching
			touching = 0;

		} while (touching);

		TUI_LOGV("%s:%d ACKnowledging\n", __func__, __LINE__);
		//completeNwdMessage();

	} // end for(;;)
}

/**
 * @brief       Open, initialise and protect the touchscreen hardware.
 *
 * This function is called by the driver core main thread at session opening.
 * It must start a new thread dedicated to handle the touchscreen hardware
 * during the TUI session. When this call returns, the touchscreen must be
 * initlialized, protected and ready to report touch events.
 *
 * @return      TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalTouchOpen()
{
	drApiResult_t r;
	static uint8_t * touchThreadStack = NULL;
	atfCallRegCtx_t atfReg;

	TUI_LOGV("%s\n", __func__);

	/* Set Direct-EINT */
	atfReg.r[0] = MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH32;
	atfReg.r[1] = TOUCH_EINT_NUM;
	atfReg.r[2] = TOUCH_INTR_NUM;
	r = drApiCallTrustedFirmware(&atfReg);
	TUI_LOGI("%s drApiCallTrustedFirmware %d %d ret=%d r0=%x r1=%x r2=%x r3=%x\n", __func__,
			TOUCH_EINT_NUM, TOUCH_INTR_NUM, r, atfReg.r[0], atfReg.r[1], atfReg.r[2], atfReg.r[3]);

#if TUI_LOCK_I2C
	// TODO: Initialize i2c bus connected to the touch device
	/* Lock i2c */
	tui_lock_i2c(0, 1);
#endif

#if TUI_ENABLE_TOUCH
	// TODO: Initialize touch module
	gt1xx_driver_init();
#endif

#if TUI_ENABLE_TOUCH
	// allocate protected stack lazily:
	if (NULL == touchThreadStack) {
		tuiAllocStack (TAG, TOUCH_STACK_SIZE, &touchThreadStack);
	}

	// Start thread
	r = drApiStartThread (
			DRIVER_THREAD_NO_TOUCHH,    // thread number
			touchThreadEntry,           // function
			(stackTop_ptr) touchThreadStack,// stack
			TOUCHH_PRIORITY,            // priority
			DRIVER_THREAD_NO_EXCH       // exception handler
			);
	if (r != DRAPI_OK)
	{
		TUI_LOGE(TAG "%s: drApiStartThread failed, error %x\n", __func__, r);
		return E_TUI_HAL_INTERNAL;
	}
#endif
	return TUI_HAL_OK;
}

/**
 * @brief       Close, reset and unprotect the touchscreen hardware.
 *
 * This functions is called by the driver core main thread at TUI session
 * closing. It must kill the touch thread and release the touchscreen hardware.
 * Before returning this function must particularly wait until the device state
 * indicates that no finger is pressed and reset the input device so that it is
 * subsequently impossible to read any information about past touch events.
 *
 * This function must unprotect the touchscreen hardware before returning.
 *
 * @return      TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalTouchClose()
{
	//drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
	drTuiHalError_t ret = TUI_HAL_OK;
	atfCallRegCtx_t atfReg;
	drApiResult_t r;

	TUI_LOGV("%s\n", __func__);

	// TODO: Reset Touch device
	// to avoid NWd to read data related to the last coordinates of the user finger.

	// TODO: Release i2c

	// Kill Touch thread
	if (DRAPI_OK == drApiStopThread(DRIVER_THREAD_NO_TOUCHH)) {
		ret = TUI_HAL_OK;
	}

#if TUI_LOCK_I2C
	/* Unlock i2c & disable DMA secure access */
	tui_lock_i2c(0, 0);
#endif
	unmap_i2c_dma_buffer();

	// Clear DEINT
	atfReg.r[0] = MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH32;
	atfReg.r[1] = TOUCH_INTR_NUM;
	r = drApiCallTrustedFirmware(&atfReg);
	TUI_LOGI("%s drApiCallTrustedFirmware %d %d ret=%d r0=%x r1=%x r2=%x r3=%x", __func__,
			TOUCH_EINT_NUM, TOUCH_INTR_NUM, r, atfReg.r[0], atfReg.r[1], atfReg.r[2], atfReg.r[3]);

	return ret;
}
