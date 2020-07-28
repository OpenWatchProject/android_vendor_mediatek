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


#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "device.h"
#include "drTuiHal.h"


//MTK Add for MTK common TUI driver layer API
#include "mtk_common_tpd.h"

#define TAG "[TUI PLAT TOUCH] "

/* Stack for ISR thread */
DECLARE_STACK(touchThreadStack, 2048);

// defines for generic touch screen
#define TOUCH_EINT_NUM  1
#define TOUCH_INTR_NUM  191

#define MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH32 0x8200020C
#define MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH64 0xC200020C
#define MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH32 0x8200020D
#define MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH64 0xC200020D

int is_extra_mem_inited();

static drTuiHalError_t touchRead(drTouchState_t *state, drTouchCoordinates_t *tsXY)
{
    // TODO: Read touch event on i2c bus
    //MTK Add +++++++++
	struct tpd_data data = {0};
	int err = 0;

	if((tpd_tui_common_obj.vendor_tpd_read_data == NULL)){
		drDbgPrintLnf(TAG "touchRead vendor_tpd_read_data=NULL...");
		return E_TUI_HAL_INTERNAL;
		}
	err = tpd_tui_common_obj.vendor_tpd_read_data(&data);
	if(err)
		return E_TUI_HAL_INTERNAL;

	tsXY->xOffset = data.position_x;
	tsXY->yOffset = data.position_y;
	*state = data.finger_states;

    return TUI_HAL_OK;
}

drTuiHalError_t tuiHalTouchGetInfo(drTouchInfo_ptr touchSize)
{
    drDbgPrintLnf(TAG "%s", __func__);

    // TODO: return real metrics
    touchSize->width = 1080;
    touchSize->height = 1920;

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
    drTouchCoordinates_t tsXY = {0xFFFF,0xFFFF};
    uint32_t touching = 0;
    uint32_t intNum = 0;
    drTouchState_t state = PEN_UP;
	int ret = 0;

    drDbgPrintLnf(TAG "%s: TOUCH thread is running.", __func__);


	wait_for_extra_mem_inited();

    // Attach touch interrupt to SWd
    if ( DRAPI_OK != drApiIntrAttach(TOUCH_INTR_NUM, INTR_MODE_FALLING_EDGE) ) {
        drDbgPrintLnf(TAG "%s:%i touch interrupt attach error", __func__, __LINE__);
        drApiThreadSleep(TIME_INFINITE);
    }

	drDbgPrintLnf(TAG "%s: TOUCH thread is running2.", __func__);

    // Touch detection loop
    for (;;) {
        /* drDbgPrintLnf(TAG "touch detection loop..."); */

        // Unlock Touch hardware
        drTouchUnlock();

        // Wait for touch IRQ
        if ( DRAPI_OK != drApiWaitForIntr(TOUCH_INTR_NUM, TIME_INFINITE, &intNum) ) {
            drDbgPrintLnf(TAG "%s:%i touch interrupt detach error", __func__, __LINE__);
            drApiThreadSleep(TIME_INFINITE);
        }
        if(intNum != TOUCH_INTR_NUM){
			drDbgPrintLnf(TAG "%s:touch interrupt number is %d", __func__, intNum);
            continue;
        }

        // Lock touch hardware
        drTouchLock();

		// TODO: Acknowledge touch IRQ
		touching = 1;
        // Read touch events
        do {
            // Read touch event on i2c bus
        	ret = touchRead(&state, &tsXY);
			if(ret < 0) {
				break;
			}
            // Report touch event
            drTouchReport(state, tsXY);

            // TODO: Check if still touching
			touching = 0;

        } while (touching);
    } // end for(;;)
}

drTuiHalError_t tuiHalTouchOpen()
{
    //drTuiHalError_t retHal = E_TUI_HAL_INTERNAL;
    drTuiHalError_t retHal = TUI_HAL_OK;
    drApiResult_t r;
	atfCallRegCtx_t atfReg;

    drDbgPrintLnf(TAG "%s", __func__);

	/* Set Direct-EINT */
	atfReg.r[0] = MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH32;
	atfReg.r[1] = TOUCH_EINT_NUM;
	atfReg.r[2] = TOUCH_INTR_NUM;
	r = drApiCallTrustedFirmware(&atfReg);
    drDbgPrintLnf(TAG "%s drApiCallTrustedFirmware %d %d ret=%d r0=%x r1=%x r2=%x r3=%x", __func__,
		TOUCH_EINT_NUM, TOUCH_INTR_NUM, r, atfReg.r[0], atfReg.r[1], atfReg.r[2], atfReg.r[3]);

#if TUI_LOCK_I2C
	/* Lock i2c */
	tui_lock_i2c(0, 1);
#endif

    // TODO: Initialize touch module
#if TUI_ENABLE_TOUCH
    gt1xx_driver_init();
#endif

#if TUI_ENABLE_TOUCH
    // Start thread
    clearStack(touchThreadStack);
    r = drApiStartThread(DRIVER_THREAD_NO_TOUCHH,           // thread number
            FUNC_PTR(touchThreadEntry),     // function
            getStackTop(touchThreadStack),  // stack
            TOUCHH_PRIORITY,                // priority
            DRIVER_THREAD_NO_EXCH);         // exeption handler
    if (r == DRAPI_OK) {
        retHal = TUI_HAL_OK;
    } else {
        drDbgPrintLnf(TAG "%s: drApiStartThread failed, error %x", __func__, r);
    }
#endif


    return retHal;
}

/**
 * @brief  Release all resources related to the TSP and kill the Touch thread.
 * @return drTuiHalError_t
 */
drTuiHalError_t tuiHalTouchClose()
{
    //drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drTuiHalError_t ret = TUI_HAL_OK;
	atfCallRegCtx_t atfReg;
	drApiResult_t r;

    drDbgPrintLnf(TAG "%s", __func__);

    // TODO: Reset Touch device
    // to avoid NWd to read data related to the last coordinates of the user finger.

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
    drDbgPrintLnf(TAG "%s drApiCallTrustedFirmware %d %d ret=%d r0=%x r1=%x r2=%x r3=%x", __func__,
		TOUCH_EINT_NUM, TOUCH_INTR_NUM, r, atfReg.r[0], atfReg.r[1], atfReg.r[2], atfReg.r[3]);

    return ret;
}
