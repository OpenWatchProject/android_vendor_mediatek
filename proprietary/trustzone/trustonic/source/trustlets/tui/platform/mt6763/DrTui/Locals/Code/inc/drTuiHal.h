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

#ifndef DRTUIHAL_H_
#define DRTUIHAL_H_

#include "drStd.h"
#include "drError.h"
#include "drTuiCommon.h"
#include "dciTui.h"

/* Declare the error type as a pointer to a struct, so that the compiler can do
 * type checking and emit a warning when we are assign or compare two different
 * error types.
 */
typedef struct drTuiHalError { int dummy; } * drTuiHalError_t;

/**
 * Driver fatal error codes.
 */
#define TUI_HAL_OK                  ((drTuiHalError_t) 0)  /**< Success */
#define E_TUI_HAL_INTERNAL          ((drTuiHalError_t) 1) /**< Internal error */
#define E_TUI_HAL_BAD_PARAMETERS    ((drTuiHalError_t) 2) /**< Bad parameters */
#define E_TUI_HAL_BUSY              ((drTuiHalError_t) 3) /**< Error peripheral is Busy */
#define E_TUI_HAL_IO                ((drTuiHalError_t) 4) /**< IO error */
#define E_TUI_HAL_NO_MEMORY         ((drTuiHalError_t) 5) /**< Not enough memory available */
#define E_TUI_HAL_MAP               ((drTuiHalError_t) 6) /**< Mapping error */

/*
 * TUI HAL VERSION
 */
#define TUI_HAL_VERSION_MAJOR   (2u)
#define TUI_HAL_VERSION_MINOR   (1u)

#define TUI_HAL_VERSION(major, minor) \
        (((major & 0x0000ffff) << 16) | (minor & 0x0000ffff))

/**
 * @brief Get the TUI HAL version number.
 *
 * @param pVersion HAL version number.
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
uint32_t tuiHalGetVersion(void);

///////////////////////////////////////////////////////
//////////              Board                //////////
///////////////////////////////////////////////////////

/**
 * @brief   Initialize board info
 *
 * @return  TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalBoardInit(void);

/**
 * @brief Copy the cmd on DCI, send a Notification to the NWd and wait for a
 * response. If a response is received it is copied to rsp.
 * @param cmd Pointer to a HAL command struct which will be send to NWd.
 * @param rsp Pointer to a HAL response struct which will hold the response from
 * NWd.
 * @return E_DR_INTERNAL An error happened on the DCI interface.
 *         E_DR_OK       The command has been sent on DCI and a response has
 *                       been received and is available in rsp.
 */
drError_t drTuiCoreDciSendAndWait(struct tui_hal_cmd_t *cmd,
                                  struct tui_hal_rsp_t *rsp);

///////////////////////////////////////////////////////
//////////              Touch                //////////
///////////////////////////////////////////////////////
#define TOUCHH_PRIORITY             (MAX_PRIORITY-1)
#define DRIVER_THREAD_NO_TOUCHH     4
#define DRIVER_THREAD_NO_VSYNC      5

/*
 * Information about the touchscreen coordinate system.
 * This coordinate system must be in the same orientation than the display.
 */
typedef struct
{
  uint32_t    width;            /**< Width in pixels. */
  uint32_t    height;           /**< Height in pixels. */
  uint32_t    eventQueueCnt;    /**< max entries of SW TUI queue desired until
                                  losing oldest touch events. Suggest pow(2) ==
                                  eventQueueCnt. */
} drTouchInfoV2_t;

/*
 * X and Y coordinate of a touch event.
 */
typedef struct{
    uint32_t    x;    /**< Coordinate in X in pixels. */
    uint32_t    y;    /**< Coordinate in Y in pixels. */
} drTouchCoordinatesV2_t;

/**
 * Touch event status.
 */
typedef enum {
    PEN_UP = 0, /**< Finger doesn't touch the screen anymore. */
    PEN_DOWN,   /**< Finger start to touch the screen. */
    PEN_HOLD,   /**< Finger is kept and moves on the screen. */
} drTouchState_t;

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
drTuiHalError_t tuiHalTouchGetInfoV2(drTouchInfoV2_t *pTouchInfo);

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
drTuiHalError_t tuiHalTouchOpen(void);

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
drTuiHalError_t tuiHalTouchClose(void);

drTuiHalError_t tuiHalHandleNwdMessage(uint32_t);

///////////////////////////////////////////////////////
//////////           DISPLAY                 //////////
///////////////////////////////////////////////////////

/**
 * @brief Protect display controller
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayProtectController(void);

/**
 * @brief Unprotect display controller
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayUnprotectController(void);

/**
 * @brief   Map the video controller register into the driver address space.
 *
 * @return  TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayMapController(void);

/**
 * @brief   Unmap the video controller register from the driver address space.
 *
 * @return  TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayUnMapController(void);

/**
 * @brief                       Initialize the display system for the Secure World.
 *
 * @param tuiFrameBufferPa      64-bits physical address of the initial framebuffer.
 * @param tuiFrameBufferSize    Framebuffer size.
 *
 * @return                      TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayInitialize64(uint64_t tuiFrameBufferPa,
                                          uint32_t tuiFrameBufferSize);

/**
 * @brief Release the display system.
 *
 * @return TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalDisplayUninitialize(void);


///////////////////////////////////////////////////////
//////////         FRAMEBUFFER               //////////
///////////////////////////////////////////////////////

typedef struct {
    uint32_t    width;  /**< Image width in pixels. */
    uint32_t    height; /**< Image height in pixels. */
    uint32_t    depth;  /**< Image pixel bits per pixel. */
    uint8_t     *data;  /**< Image data buffer. */
    uint32_t    size;   /**< Image data buffer size. */
} tuiImageV2_t;

typedef enum {
    TUI_ORIENTATION_0 = 0,
    TUI_ORIENTATION_90,
    TUI_ORIENTATION_180,
    TUI_ORIENTATION_270
} tuiOrientation_t;

/*
 * Define how and where is a channel in a pixel.
 */
typedef struct {
    uint32_t    offset; /**< Index of the lowest bit of the channel into a
                          pixel. */
    uint32_t    length; /**< Number of bits of the channel. */
} tuiFbChannel_t;

/*
 * Define how pixels are packed
 * The typical format is BGRA:
 * It is stored like this in memory (little-endian):
 * |A|R|G|B|
 * The blue channel sample is stored at the lowest address.
 */
typedef enum {
    TUI_HAL_FB_TYPE_RGB = 0,
    TUI_HAL_FB_TYPE_BGR,
    TUI_HAL_FB_TYPE_RGBA,
    TUI_HAL_FB_TYPE_BGRA
} tuiFbPixelType_t;

/**< Buffer security state. */
typedef enum {
    TUI_FB_PUBLIC = 0, /**< For non-secure RAM, typically NWD memory */
    TUI_FB_PROTECTED,  /**< Memory protected by TZASC. Only used by the TUI
                         CORE library. */
    TUI_FB_SECURE,     /**< For secure RAM, typically SWD memory */
} tuiFbSecurity_t;

/**< Framebuffer info structure. */
typedef struct{
    /* Fields set by HAL */
    uint32_t            xRes;             /**<  Framebuffer width in pixels. */
    uint32_t            yRes;             /**<  Framebuffer height in pixels. */
    uint32_t            density;          /**<  Framebuffer density in ppi. */
    uint32_t            bitsPerPixel;     /**<  Framebuffer bits per pixel. */
    tuiFbChannel_t      redChannelDepth;  /**<  Framebuffer red channel depth
                                            in pixels. */
    tuiFbChannel_t      greenChannelDepth;/**<  Framebuffer green channel depth
                                            in pixels. */
    tuiFbChannel_t      blueChannelDepth; /**<  Framebuffer blue channel depth
                                            in pixels. */
    tuiFbChannel_t      alphaChannelDepth;/**<  Framebuffer alpha channel depth
                                            in pixels. */
    uint32_t            stride;           /**<  Number of bytes from one row of
                                            pixels in memory to the next row of
                                            pixels in memory. */
    tuiFbPixelType_t    type;             /**<  Type of pixels. */

    uint64_t            fbPhysAddr;       /**<  Framebuffer physical address.*/
    tuiFbSecurity_t     memSecureState;   /**<  State of the memory. */
    uint32_t            fbSize;           /**<  Size of a FrameBuffer in
                                              bytes. */

    /* Fields set by drTui */
    uint8_t             *fbVirtAddr;      /**<  Framebuffer virtual address. */
    uint32_t            padding;
} tuiFbInfoV2_t;

/**
 * @brief           Initialize the FrameBuffer structure info.
 *
 * REQUIREMENTS:
 *
 * For secure RAM, fbPhysAddr MUST be set to a value different from
 * PHYS_INVALID_ADDRESS and memSecureStat set to TUI_FB_SECURE by the
 * integrator.
 * For NWd allocated memory, fbPhysAddr MUST be set to PHYS_INVALID_ADDRESS and
 * memSecureStat to TUI_FB_PUBLIC.
 *
 * fbSize MUST contain at least a frame of the screen size, stride*yRes.
 *
 * @param pFbInfo   Pointer to the framebuffer info structure to fill.
 *
 * @return
 */
drTuiHalError_t tuiHalFBOpenV2(tuiFbInfoV2_t *pFbInfo);

/**
 * @brief           Protect the frameBuffer.
 *
 * This function is called at TUI session opening if the framebuffer is marked
 * as non-secure memory (memSecureStat set to TUI_FB_PUBLIC). Particularly this
 * may happen if the framebuffer is recovered from the normal world.
 *
 * This function should do the following operations:
 *
 * Check that the designated region consists entirely of normal world memory. If
 * not, return a failure status.
 *
 * Change the TZASC configuration to make the buffer locked for secure access
 * only.
 *
 * These two steps must be done atomically. Failing to do it atomically may lead
 * to race conditions if the framebuffer is shared with other secure services,
 * like DRM.
 *
 * @param physAddr  64-bit physical address of the framebuffer.
 * @param size      Framebuffer size.
 *
 * @return
 */
drTuiHalError_t tuiHalFBProtectV2(uint64_t physAddr,
                                  uint32_t size);

/**
 * @brief           Unprotect the frameBuffer.
 *
 * This function is called at TUI session closing. It must change the TZASC
 * configuration to make the buffer available for non-secure access.
 *
 * @param physAddr  64-bit physical address of the framebuffer.
 * @param size      Framebuffer size.
 *
 * @return
 */
drTuiHalError_t tuiHalFBUnprotectV2(uint64_t physAddr,
                                    uint32_t size);


/*
 * Temporary Workaround for Screen Resolution change
 */
drTuiHalError_t tuiHalScreenInfoUpdate(tuiFbInfoV2_t *pFbInfo);
drTuiHalError_t tuiHalDisplayInitialize64Ex(uint32_t width, uint32_t height);


/**
 * @brief           Post the FB and wait until it is displayed.
 *
 * @param pFbInfo   FB to be posted and displayed.
 * @param copied    Output parameter indicating if the post was done by copy or
 * by page flipping. True value indicates that the buffer was copied.  False
 * indicates that the buffer was not copied, but flipped.  In this latter case,
 * the buffer pointed to by pFbInfo has become the the front buffer and the
 * core is guaranteed not to draw into it.
 *
 * @return          TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalFBPost(const tuiFbInfoV2_t *pFbInfo);

/**
 * @brief HAL function to copy a clipping rectangle from an image to a place in
 * the framebuffer.
 *
 * @param pFbInfo       Pointer on the framebuffer to bitblit the image
 * @param pImage        Pointer on the image to bitblit
 * @param orientation   Orientation of the bitblit
 * @param dx            X coordinate of the top-left corner of the image inside
 * the screen coordinate system.
 * @param dy            Y coordinate of the top-left corner of the image inside
 * the screen coordinate system.
 * @param sx            X coordinate of the top left corner of the clipping
 * rectangle inside the image coordinate system.
 * @param sy            Y coordinate of the top left corner of the clipping
 * rectangle inside the image coordinate system.
 * @param sWidth        Width in pixels of the clipping rectangle.
 * @param sHeight       Height in pixels of the clipping rectangle.
 *
 * @return              TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalFBImageBltV2(const tuiFbInfoV2_t *pFbInfo,
                                   const tuiImageV2_t *pImage,
                                   tuiOrientation_t orientation,
                                   uint32_t dx,
                                   uint32_t dy,
                                   uint32_t sx,
                                   uint32_t sy,
                                   uint32_t sWidth,
                                   uint32_t sHeight);

/**
 * @brief           Function to copy a rectangle area of the src frameBuffer to a
 * place in the dst frameBuffer.
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
 * @param width    Width in pixels of the area to copy.
 * @param height   Height in pixels of the area to copy.

 * @return          TUI_HAL_OK upon success or any specific error.
 */
drTuiHalError_t tuiHalFBCopyArea(const tuiFbInfoV2_t *pDstFB,
                                 const tuiFbInfoV2_t *pSrcFB,
                                 uint32_t dx,
                                 uint32_t dy,
                                 uint32_t sx,
                                 uint32_t sy,
                                 uint32_t width,
                                 uint32_t height);

/**
 * @brief           Function to draw a rectangle filled with color to the dst
 * FrameBuffer.
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
drTuiHalError_t tuiHalFBFillRect(const tuiFbInfoV2_t *pDstFB,
                                 uint32_t x,
                                 uint32_t y,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t fgColor);

#endif /* DRTUIHAL_H_ */
