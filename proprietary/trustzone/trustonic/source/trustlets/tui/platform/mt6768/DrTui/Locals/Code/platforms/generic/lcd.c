/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#define TAG "[TUI HAL DISP] "

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drTuiHal.h"
#include "display_tui.h"
#include "ddp_log.h"

#define LOG_DEBUG 1
#define LOG_USER 0
#define LOG_LEVEL LOG_USER

/**
 * @brief Protect display controller
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayProtectController(void)
{
    drTuiHalError_t ret = TUI_HAL_OK;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintLnf(TAG "%s", __func__);
#endif
    return ret;
}

/**
 * @brief Unprotect display controller
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayUnprotectController(void)
{
    drTuiHalError_t ret = TUI_HAL_OK;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintLnf(TAG "%s", __func__);
#endif
    return ret;
}

/**
 * @brief   Map the video controller register into the driver address space.
 *
 * @return  TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayMapController(void)
{
    drTuiHalError_t ret = TUI_HAL_OK;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintLnf(TAG "%s", __func__);
#endif
#if TUI_ENABLE_DISPLAY
    disp_tui_init();
#endif
    return ret;
}

/**
 * @brief   Unmap the video controller register from the driver address space.
 *
 * @return  TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayUnMapController(void)
{
    drTuiHalError_t ret = TUI_HAL_OK;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintLnf(TAG "%s", __func__);
#endif
    return ret;
}

/**
 * @brief                       Initialize the display system for the Secure World.
 *
 * @param tuiFrameBufferPa      64-bits physical address of the initial framebuffer.
 * @param tuiFrameBufferSize    Framebuffer size.
 *
 * @return                      TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayInitialize64(uint64_t tuiFrameBufferPa,
                                          uint32_t tuiFrameBufferSize)
{
    drTuiHalError_t ret = TUI_HAL_OK;
    int tmp = 0;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintf(TAG "%s pa_h=0x%x, pa_l=0x%x, size=0x%x\n", __func__ , 
                        tuiFrameBufferPa >> 32, tuiFrameBufferPa & 0xffff, tuiFrameBufferSize);
#endif
#if TUI_ENABLE_DISPLAY
    tmp = disp_tui_enter();
    if(tmp)
    	ret = E_TUI_HAL_BUSY;
#endif
    return ret;
}

drTuiHalError_t tuiHalDisplayInitialize64Ex(uint32_t width, uint32_t height)
{
    drTuiHalError_t ret = TUI_HAL_OK;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintLnf(TAG "%s (%u x %u)", __func__, width, height);
#endif
    return ret;
}

/**
 * @brief Release the display system.
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayUninitialize(void)
{
    drTuiHalError_t ret = TUI_HAL_OK;
    int tmp = 0;
#if LOG_LEVEL >= LOG_DEBUG
    drDbgPrintLnf(TAG "%s", __func__);
#endif
#if TUI_ENABLE_DISPLAY
    tmp = disp_tui_leave();
    if(tmp)
    	ret = E_TUI_HAL_BUSY;
 #endif
    return ret;
}
int __attribute__((weak)) disp_tui_init()
{
	return 0;
}

int __attribute__((weak)) disp_tui_enter()
{
	return 0;
}

int __attribute__((weak)) disp_tui_leave()
{
	return 0;
}
