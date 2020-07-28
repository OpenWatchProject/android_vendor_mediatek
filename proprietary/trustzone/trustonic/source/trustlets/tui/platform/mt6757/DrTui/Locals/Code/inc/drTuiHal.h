/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
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

/**
 * Driver fatal error codes.
 */
/* Declare the error type as a pointer to a struct, so that the compiler can do
 * type checking and emit a warning when we are assign or compare two different
 * error types.
 */
typedef struct drTuiHalError { int dummy; } * drTuiHalError_t;

#define TUI_HAL_OK                  ((drTuiHalError_t) 0)  /**< Success */
#define E_TUI_HAL_INTERNAL          ((drTuiHalError_t) 1) /**< Internal error */
#define E_TUI_HAL_BAD_PARAMETERS    ((drTuiHalError_t) 2) /**< Bad parameters */
#define E_TUI_HAL_BUSY              ((drTuiHalError_t) 3) /**< Error peripheral is Busy */
#define E_TUI_HAL_IO                ((drTuiHalError_t) 4) /**< IO error */
#define E_TUI_HAL_NO_MEMORY         ((drTuiHalError_t) 5) /**< Error no memory available */
#define E_TUI_HAL_MAP               ((drTuiHalError_t) 6) /**< Error no memory available */

///////////////////////////////////////////////////////
//////////              Board                //////////
///////////////////////////////////////////////////////

/** Initialize board info */
drTuiHalError_t tuiHalBoardInit(void);

///////////////////////////////////////////////////////
//////////              Core                //////////
///////////////////////////////////////////////////////
extern void drTuiCoreCancelSessionRequest(void);

///////////////////////////////////////////////////////
//////////              Touch                //////////
///////////////////////////////////////////////////////
#define TOUCHH_PRIORITY             (MAX_PRIORITY-1)
#define DRIVER_THREAD_NO_TOUCHH     4

typedef struct
{
  uint32_t    width;
  uint32_t    height;
} drTouchInfo_t, *drTouchInfo_ptr;

typedef struct{
    uint32_t    xOffset;
    uint32_t    yOffset;
} drTouchCoordinates_t, *drTouchCoordinates_ptr;

typedef enum {
    PEN_UP = 0,
    PEN_DOWN  = 1,
    PEN_HOLD,
} drTouchState_t, *drTouchState_ptr;


drTuiHalError_t tuiHalTouchGetInfo(drTouchInfo_ptr touchSize);

drTuiHalError_t tuiHalTouchOpen(void);

drTuiHalError_t tuiHalTouchClose(void);

extern drError_t drTouchReport(drTouchState_t touchEvent, drTouchCoordinates_t coordinates);

extern void drTouchUnlock(void);

extern void drTouchLock(void);

///////////////////////////////////////////////////////
//////////           DISPLAY                 //////////
///////////////////////////////////////////////////////
typedef struct
{
  uint32_t    grayscaleBitDepth;
  uint32_t    redBitDepth;
  uint32_t    greenBitDepth;
  uint32_t    blueBitDepth;
  uint32_t    width;
  uint32_t    height;
  uint32_t    wDensity;
  uint32_t    hDensity;
} tuiScreenInfo_t, *tuiScreenInfo_ptr;

/** Protect display controller */
drTuiHalError_t tuiHalDisplayProtectController(void);

/** Unprotect display controller */
drTuiHalError_t tuiHalDisplayUnprotectController(void);

/** */
drTuiHalError_t tuiHalDisplayMapController(void);

/** */
drTuiHalError_t tuiHalDisplayUnMapController(void);

/** Backup NWd config (in a static struct), protect and reconfig DSS for TUI frame buffer) */
drTuiHalError_t tuiHalDisplayInitialize(uint32_t tuiFrameBufferPa, uint32_t tuiFrameBufferSize);

/** Restore NWd config and unprotect DSS */
drTuiHalError_t tuiHalDisplayUninitialize(void);


///////////////////////////////////////////////////////
//////////         FRAMEBUFFER               //////////
///////////////////////////////////////////////////////

#define TUI_FB_NUMBER       2

typedef struct {
    uint32_t    xOffset;
    uint32_t    yOffset;
} tuiCoordinates_t, *tuiCoordinates_ptr;

typedef struct {
    tuiCoordinates_t    org;
    uint32_t            width;
    uint32_t            height;
    uint32_t            depth;
    uint8_t             *data;
    uint32_t            size;
} tuiImage_t, *tuiImage_ptr;

typedef enum {
    TUI_ORIENTATION_0 = 0,
    TUI_ORIENTATION_90,
    TUI_ORIENTATION_180,
    TUI_ORIENTATION_270
} tuiOrientation_t, *tuiOrientation_ptr;
/*
 * Define how and where is a channel in a pixel
 */
typedef struct {
    uint32_t    offset;
    uint32_t    length;
} tuiFbChannel_t, *tuiFbChannel_ptr;

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
} tuiFbPixelType_t, *tuiFbPixelType_ptr;

typedef enum {
    TUI_FB_PUBLIC = 0,
    TUI_FB_PROTECTED,
    TUI_FB_SECURE,
} tuiFbSecurity_t;

typedef struct{
    /* Fields set by HAL */
    uint32_t            xRes;
    uint32_t            yRes;
    uint32_t            density;
    uint32_t            bitsPerPixel;
    tuiFbChannel_t      redChannelDepth;
    tuiFbChannel_t      greenChannelDepth;
    tuiFbChannel_t      blueChannelDepth;
    tuiFbChannel_t      alphaChannelDepth;
    uint32_t            lineLength; /* Length of a framebuffer line in bytes */
    tuiFbPixelType_t    type; /* Type of pixels */

    tuiFbSecurity_t     memSecureState;
    uint32_t            fbSize; /* Size of a FrameBuffer in bytes*/
    phys_addr_t         fbPhysAddr;

    /* Fields set by drTui */
    uint8_t             *fbVirtAddr;
} tuiFbInfo_t, *tuiFbInfo_ptr;

/** Initialize the FrameBuffer structure info.
 * REQUIREMENTS:
 * For secure RAM, fbPhysAddr MUST be set to a non-NULL value and secureLevel
 * set to TUI_FB_SECURE by the integrator.
 * For NWd allocated memory, fbPhysAddr MUST be set to NULL and secureLevel to
 * TUI_FB_PUBLIC
 * fbSize MUST contain at least a frame of the screen size, xRes*yRes*(bitsPerPixel/8)
 * memSize MUST contain 2 FrameBuffers
 * */
drTuiHalError_t tuiHalFBOpen(tuiFbInfo_t *pFbInfo);

/** Protect FrameBuffer */
drTuiHalError_t tuiHalFBProtect(uint64_t physAddr, uint32_t size);

/** Unrotect FrameBuffer */
drTuiHalError_t tuiHalFBUnprotect(uint64_t physAddr, uint32_t size);

/** Copy an image to the framebuffer. */
drTuiHalError_t tuiHalFBImageBlt(tuiFbInfo_t *pFbInfo, tuiImage_t *pImage, tuiOrientation_t orientation);


#endif /* DRTUIHAL_H_ */
