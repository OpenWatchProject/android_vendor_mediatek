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

#define TUI_LOCK_I2C		1
#define TUI_ENABLE_TOUCH	1

//MTK Add for MTK common TUI driver layer API
#include "mtk_common_tpd.h"

#define TAG "[TUI PLAT TOUCH]"
#include "log.h"

/* Stack for ISR thread */
DECLARE_STACK(touchThreadStack, 2048);

// defines for generic touch screen
#define TOUCH_INTR_NUM  187
#define I2C_INTR_NUM    118

int is_extra_mem_inited();

static drTuiHalError_t touchRead(drTouchState_t *state, drTouchCoordinates_t *tsXY)
{
    // TODO: Read touch event on i2c bus
    //MTK Add +++++++++
	struct tpd_data data = {0};
	int err = 0;

	if((tpd_tui_common_obj.vendor_tpd_read_data == NULL)){
		TUI_LOGE("touchRead vendor_tpd_read_data=NULL...\n");
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
    TUI_LOGD("%s\n", __func__);

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

    TUI_LOGD("%s: TOUCH thread is running.\n", __func__);


	wait_for_extra_mem_inited();

    // Attach touch interrupt to SWd
    if ( DRAPI_OK != drApiIntrAttach(TOUCH_INTR_NUM, INTR_MODE_FALLING_EDGE) ) {
        TUI_LOGE("%s:%i touch interrupt attach error\n", __func__, __LINE__);
        drApiThreadSleep(TIME_INFINITE);
    }

	TUI_LOGD("%s: TOUCH thread is running2.\n", __func__);

    // Touch detection loop
    for (;;) {
        TUI_LOGV("touch detection loop...\n");

        // Unlock Touch hardware
        drTouchUnlock();

        // Wait for touch IRQ
        if ( DRAPI_OK != drApiWaitForIntr(TOUCH_INTR_NUM, TIME_INFINITE, &intNum) ) {
            TUI_LOGE("%s:%i touch interrupt detach error\n", __func__, __LINE__);
            drApiThreadSleep(TIME_INFINITE);
        }
        if(intNum != TOUCH_INTR_NUM){
			TUI_LOGE("%s:touch interrupt number is %d\n", __func__, intNum);
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

    TUI_LOGI("%s\n", __func__);

#if TUI_LOCK_I2C
	/* Lock i2c */
	tui_lock_i2c(1, 1);
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
        TUI_LOGE("%s: drApiStartThread failed, error %x\n", __func__, r);
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

    TUI_LOGI("%s\n", __func__);

    // TODO: Reset Touch device
    // to avoid NWd to read data related to the last coordinates of the user finger.

    // Kill Touch thread

    if (DRAPI_OK == drApiStopThread(DRIVER_THREAD_NO_TOUCHH)) {
        ret = TUI_HAL_OK;
    }

#if TUI_LOCK_I2C
	/* Unlock i2c & disable DMA secure access */
	tui_lock_i2c(1, 0);
#endif
	unmap_i2c_dma_buffer();

    return ret;
}
