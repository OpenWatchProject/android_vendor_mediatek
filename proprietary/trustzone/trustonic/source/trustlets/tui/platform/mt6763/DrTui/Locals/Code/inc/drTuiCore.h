/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef DRTUICORE_H_
#define DRTUICORE_H_

#include "drStd.h"
#include "drError.h"
#include "drTuiHal.h"

///////////////////////////////////////////////////////
//////////              Core                //////////
///////////////////////////////////////////////////////
/**
 * @brief CORE function that can be used by the HAL in order ask the CORE to
 * cancel the TUI session if any.
 *
 * @return
 */
void drTuiCoreCancelSessionRequest(void);
void completeNwdMessage(void);

/**
 * @brief               CORE function that can be used by the HAL in order to
 * report touch events to the CORE.
 *
 * @param touchEvent    State that represent the finger.
 * @param coordinates   Coordinates of the finger.
 *
 * @return              TUI_HAL_OK upon success or any specific error.
 */
drError_t drTouchReportV2(drTouchState_t touchEvent,
                          drTouchCoordinatesV2_t *pCoordinates);

/**
 * @brief Release lock on the touchscreen hardware.
 *
 * Called from the touch thread to signal to the core driver that the
 * touch hardware is not being used anymore. It also means that the i2c
 * interrupt must not be attached anymore to the touch thread when calling
 * this.  The integrator must take care not to call this function while the
 * touch device state indicates that a finger is pressed. After calling this
 * the touch thread may be killed and the touch device reset.
 */
void drTouchUnlock(void);


/**
 * @brief CORE function to lock touchscreen
 *
 * Called from the touch thread to signal to the core driver that
 * the touch hardware is being used and it must not be interrupted or stopped.
 */
void drTouchLock(void);

/**
 * @brief           Generic default function to copy a rectangle area of the src
 * FrameBuffer to a place in the dst FrameBuffer.
 *
 * @param pDstFB    Destination framebuffer where to draw the rectangle area from the
 * src FB.
 * @param pSrcFB    Source framebuffer used to apply the rectangle area which is copied
 * into the dst framebuffer.
 * @param dx        X coordinate into the destination framebuffer coordinate
 * system of the top-left corner of the copy.
 * @param dy        Y coordinate into the destionation framebuffer coordinate
 * system of the top-left corner of the copy.
 * @param sx        X coordinate into the source framebuffer coordinate system
 * of the top-left corner of the area to copy.
 * @param sy        Y coordinate into the source framebuffer coordinate system
 * of the top-left corner of the area to copy.
 * @param sWidth    Width in pixels of the area to copy.
 * @param sHeight   Height in pixels of the area to copy.
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t drTuiFBCopyArea(const tuiFbInfoV2_t *pDstFB,
                                const tuiFbInfoV2_t *pSrcFB,
                                uint32_t dx,
                                uint32_t dy,
                                uint32_t sx,
                                uint32_t sy,
                                uint32_t sWidth,
                                uint32_t sHeight);

/**
 * @brief           Generic default function to draw a rectangle filled with
 * color to the dst FrameBuffer.
 *
 * @param pDstFB    Destination FB where to draw the filled rectangle
 * @param x         X coordinates of the rectangle's top-left corner
 * @param y         Y coordinates of the rectangle's top-left corner
 * @param width     Width of the rectangle to draw
 * @param height    Height of the rectangle to draw
 * @param fgColor   RGBA_8888 color to fill the rectangle with.
 *
 * @return          TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t drTuiFBFillRect(const tuiFbInfoV2_t *pDstFB,
                                uint32_t x,
                                uint32_t y,
                                uint32_t width,
                                uint32_t height,
                                uint32_t fgColor);

/**
 * @brief               Generic default function to draw a clipped rectangle
 * area of an image to the dst FrameBuffer.
 *
 * @param pDstFB        Destination FB where to draw the filled rectangle
 * @param pImage        Source image buffer of pixels to clip and blit into the
 * dst.
 * @param orientation   Orientation to apply to the image to bitblit.
 * @param dx            X coordinate of the top-left corner of the image inside
 * the screen coordinate system.
 * @param dy            Y coordinate of the top-left corner of the image inside
 * the screen coordinate system.
 * @param sx            X coordinate of the top-left corner of the clipping
 * rectangle in the source image buffer.
 * @param sy            Y coordinate of the top-left corner of the clipping
 * rectangle in the source image buffer.
 * @param sWidth        Width in pixels of the clipping rectangle.
 * @param sHeight       Height in pixels of the clipping rectangle.
 *
 * @return              TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t drTuiFBBlit(const tuiFbInfoV2_t *pDstFB,
                            const tuiImageV2_t *pImage,
                            tuiOrientation_t orientation,
                            uint32_t dx,
                            uint32_t dy,
                            uint32_t sx,
                            uint32_t sy,
                            uint32_t sWidth,
                            uint32_t sHeight);

#endif /* DRTUICORE_H_ */
