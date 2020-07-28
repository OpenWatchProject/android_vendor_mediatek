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
#define TAG "[TUI HAL FB] "

#include "drStd.h"
#include "mt_emi_mpu.h"
#include "drTuiHal.h"
#include "drTuiCore.h"
#include "display_tui.h"
#include "ddp_log.h"
#include "DrApi/DrApi.h"

static int buffer_cnt = 0;
static int protect_cnt = 0;
static int mpu_protected = 0;
static int extra_mem_inited = 0;

#define BITS_PER_CHANNEL 8
#define BITS_PER_PIXEL 32
#define BYTES_PER_PIXEL 4

#define EXTRA_MEM_SIZE	0x200000
#define BUFFER_COUNT 3

void set_and_map_i2c_dma_pa(uint64_t physAddr, uint32_t size);
int cmdq_tui_set_base_pa(uint32_t physAddr, uint32_t size);

int tui_init_extra_pa(uint64_t physAddr)
{
	//set_cmdq_pa(physAddr, 0x100000);
	set_and_map_i2c_dma_pa(physAddr+0x100000, 0x100000);

	cmdq_tui_set_base_pa(physAddr+0x200000, 0x100000);
	return 0;
}

int is_extra_mem_inited(void)
{
	return extra_mem_inited;
}

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
drTuiHalError_t tuiHalFBOpenV2(tuiFbInfoV2_t *pFbInfo)
{
    drTuiHalError_t ret = TUI_HAL_OK;
    pFbInfo->xRes = X_RES;
    pFbInfo->yRes = Y_RES;
    pFbInfo->density = 480;
    pFbInfo->bitsPerPixel = BITS_PER_PIXEL;
    pFbInfo->stride= X_RES*BYTES_PER_PIXEL;
    pFbInfo->type = TUI_HAL_FB_TYPE_RGBA;
    pFbInfo->redChannelDepth.length = BITS_PER_CHANNEL;
    pFbInfo->greenChannelDepth.length = BITS_PER_CHANNEL;
    pFbInfo->blueChannelDepth.length = BITS_PER_CHANNEL;
    pFbInfo->alphaChannelDepth.length = BITS_PER_CHANNEL;
    pFbInfo->redChannelDepth.offset = 0;
    pFbInfo->greenChannelDepth.offset = 8;
    pFbInfo->blueChannelDepth.offset = 16;
    pFbInfo->alphaChannelDepth.offset = 24;
    pFbInfo->memSecureState = TUI_FB_PUBLIC;
    pFbInfo->fbPhysAddr = PHYS_INVALID_ADDRESS;
    pFbInfo->fbSize = X_RES*BYTES_PER_PIXEL*Y_RES;
    TUI_LOGD("%s (%d x %d) bpp=%d \n", __func__, pFbInfo->xRes , pFbInfo->yRes, pFbInfo->bitsPerPixel);
    return ret;
}

drTuiHalError_t tuiHalScreenInfoUpdate(tuiFbInfoV2_t *pFbInfo)
{
    drTuiHalError_t ret = TUI_HAL_OK;
    (void)pFbInfo;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}

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
                                  uint32_t size)
{
	threadid_t touch_tid;
	drApiResult_t r;
	drTuiHalError_t ret = TUI_HAL_OK;
#if TUI_ENABLE_MPU
    struct emi_region_info_t region_info;
#endif

    TUI_LOGD("%s addr=0x%x, size=%d\n", __func__, physAddr, size);
    /*first address will be passed in third call*/
    if (protect_cnt == 2) {
#if TUI_ENABLE_MPU
        region_info.region = TUI_MEMORY_MPU_REGION_ID;
        region_info.start = physAddr;
        region_info.end = physAddr + (X_RES*BYTES_PER_PIXEL*Y_RES)*BUFFER_COUNT + EXTRA_MEM_SIZE -1;

        TUI_LOGD("%s region=%d, addr_start=0x%x, addr_end=0x%x, x=%d, y=%d, buffer count=%d, extra mem=0x%x\n", 
                __func__, region_info.region, region_info.start, region_info.end, (X_RES*BYTES_PER_PIXEL), Y_RES, BUFFER_COUNT, EXTRA_MEM_SIZE);

        /* Enble MPU protection */
        SET_ACCESS_PERMISSION(region_info.apc, EMI_MPU_UNLOCK, \
            EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, \
            EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, \
            EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, \
            EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_SEC_RW);
        emi_mpu_set_protection(&region_info);
#endif
        tui_init_extra_pa(physAddr+(X_RES*BYTES_PER_PIXEL*Y_RES)* BUFFER_COUNT);
        extra_mem_inited = 1;
        mpu_protected = 1;
    }
    if(protect_cnt < 3)
        protect_cnt++;

    return ret;
}

void clean_mem(uint64_t addr_start, uint32_t size)
{
	drApiResult_t ret;
	uint8_t *va = NULL;

	ret = drApiMapPhysicalBuffer(addr_start, size, MAP_READABLE | MAP_WRITABLE | MAP_UNCACHED, (void **)&va);
	if (ret == DRAPI_OK) {
		memset(va, 0, size);
		drApiUnmapBuffer((void *)va);
	} else {
		drApiLogPrintf("TUI-memory map failed: 0x%08x%08x\n", VALUE_64(addr_start));
	}
}

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
                                    uint32_t size)
{
    drTuiHalError_t ret = TUI_HAL_OK;
    uint32_t total_mem_size;
#if TUI_ENABLE_MPU
    struct emi_region_info_t region_info;
#endif

    TUI_LOGD("%s addr=0x%x, size=0x%x\n", __func__, physAddr, size);
    /*first address will be passed in third call*/
    if (protect_cnt==1 && mpu_protected) {
#if TUI_ENABLE_MPU
        total_mem_size = (X_RES*BYTES_PER_PIXEL*Y_RES)*BUFFER_COUNT + EXTRA_MEM_SIZE;
        region_info.region = TUI_MEMORY_MPU_REGION_ID;
        region_info.start = physAddr;
        region_info.end = physAddr + total_mem_size - 1;
        /* Clean memory */
        TUI_LOGD("%s addr_start=0x%x, addr_end=0x%x total=0x%x\n", __func__, region_info.start, region_info.end, total_mem_size);
        clean_mem(region_info.start, total_mem_size);
        //memset((void*)region_info.start, 0, total_mem_size);

        /* Disable MPU protection */
        SET_ACCESS_PERMISSION(region_info.apc, EMI_MPU_UNLOCK, \
            EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, \
            EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, \
            EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, \
            EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION);
        emi_mpu_set_protection(&region_info);
#endif
        mpu_protected = 0;
        extra_mem_inited = 0;
    }
    if(protect_cnt)
        protect_cnt--;

    return ret;
}

void tuiHalPrintFBInfo(const tuiFbInfoV2_t *pFbInfo, const char *msg)
{
	TUI_LOGV("%s fb_info is =======>\n", __func__);
	TUI_LOGV("x=%d,y=%d,density=%d,bpp=%d,pitch=%d,type=%d,sec=%d,size=%d,phys=0x%08x%08x,va=0x%p\n",
		pFbInfo->xRes, pFbInfo->yRes, pFbInfo->density, pFbInfo->bitsPerPixel,
		pFbInfo->stride, pFbInfo->type, pFbInfo->memSecureState,
		pFbInfo->fbSize, VALUE_64(pFbInfo->fbPhysAddr), pFbInfo->fbVirtAddr);

}

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
                                   uint32_t sHeight)
{
	drTuiHalError_t ret = TUI_HAL_OK;

	tuiHalPrintFBInfo(pFbInfo, __func__);
	TUI_LOGV("%s img:w=%d,h=%d,dep=%d,va=0x%p,size=0x%x,orient=%d\n",
			__func__, pImage->width, pImage->height, pImage->depth,
			pImage->data, pImage->size, orientation);
	/* If no hardware implementation, use the generic software implementation*/
	ret = drTuiFBBlit(pFbInfo, pImage, orientation, dx, dy, sx, sy, sWidth,
			sHeight);
	/*TUI_LOGV("%s ,ret=%d\n", __func__, ret);*/
	return ret;
}

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
drTuiHalError_t tuiHalFBPost(const tuiFbInfoV2_t *pFbInfo)
{
    drTuiHalError_t ret = TUI_HAL_OK;
    TUI_LOGV("%s\n", __func__);
#if TUI_ENABLE_DISPLAY
    disp_tui_pan_display(pFbInfo);
#endif
    return ret;
}

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
                                 uint32_t height)
{
	TUI_LOGV("%s: %s:%d\n", __FILE__, __func__, __LINE__);

    // If no hardware implementation, use the generic software implementation
    return drTuiFBCopyArea(pDstFB, pSrcFB, dx, dy, sx, sy, width, height);
}

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
                                 uint32_t fgColor)
{
    TUI_LOGV("%s: %s:%d\n", __FILE__, __func__, __LINE__);

    // If no hardware implementation, use the generic software implementation
    return drTuiFBFillRect(pDstFB, x, y, width, height, fgColor);
}
