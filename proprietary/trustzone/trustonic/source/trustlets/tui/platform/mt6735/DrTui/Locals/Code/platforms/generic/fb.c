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

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "DrApi/DrApiPlat.h"
#include "drTuiHal.h"
#include "display_tui.h"
#include "ddp_log.h"

#define LOG_TAG "[TUI_HAL_FB] "
#define TAG "[TUI_HAL_FB] "

static int buffer_cnt = 0;
static int protect_cnt = 0;
static int mpu_protected = 0;
static int extra_mem_inited = 0;

#define X_RES 1080
#define Y_RES 1920
#define BITS_PER_CHANNEL 8
#define BITS_PER_PIXEL 32
#define BYTES_PER_PIXEL 4

#define EXTRA_MEM_SIZE	0x200000

void set_and_map_i2c_dma_pa(uint32_t physAddr, uint32_t size);
int cmdq_tui_set_base_pa(uint32_t physAddr, uint32_t size);

static uint64_t alloc_fb()
{
    uint64_t fb_pa = 0;

    fb_pa = 0xf7000000 + buffer_cnt*X_RES*Y_RES*BYTES_PER_PIXEL;
    buffer_cnt++;

    DDPMSG("alloc_fb(%d)=0x%x", buffer_cnt, fb_pa);
	return fb_pa;
}

int tui_init_extra_pa(uint32_t physAddr)
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

/*
 * Initialize the FrameBuffer structure info.
 */
drTuiHalError_t tuiHalFBOpen(tuiFbInfo_t *pFbInfo) {
    drTuiHalError_t ret = TUI_HAL_OK;

    pFbInfo->xRes = X_RES;
    pFbInfo->yRes = Y_RES;
    pFbInfo->density = 480;
    pFbInfo->bitsPerPixel = BITS_PER_PIXEL;
    pFbInfo->lineLength = X_RES*BYTES_PER_PIXEL;
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
    DDPMSG("%s %d x %d bpp=%d ", __func__, pFbInfo->xRes , pFbInfo->yRes, pFbInfo->bitsPerPixel);
    return ret;
}



/**
 *  tuiHalDisplayProtectFB()
 *      uint32_t phys_base :    Physical address of memory region to be secured. Must be aligned on size. Only 32-bit supported.
 *      uint32_t size : size of the region (must be 2^n)
 *        It seems we must configure ALL TZASC modules identically (one per CPU?). Cf SLSI DRM secdrv example.
 *        Check availability only on 1st module, assuming all others are sync'ed.
 *        Note 1: Assuming 32-bit address ONLY (discard TZASC_HI)
 *        Note 2: Dealing with regions only, no subregion.
 *        Note 3: Assume "default" general config (no "init" function implemented by us).
 */
drTuiHalError_t tuiHalFBProtect(uint32_t physAddr, uint32_t size) {
	threadid_t touch_tid;
	drApiResult_t r;
    drTuiHalError_t ret = TUI_HAL_OK;
	uint32_t addr_start, addr_end;
	uint32_t mpu_attr;
    DDPMSG("%s addr=0x%x, size=0x%x", __func__, physAddr, size);

	// Secure memory has been protected in preloader.
#if 0
	if (protect_cnt == 0) {
		addr_start = physAddr;
		addr_end = physAddr + (X_RES*BYTES_PER_PIXEL*Y_RES)*2 + EXTRA_MEM_SIZE -1;

		/* Enble MPU protection */
		mpu_attr = SET_ACCESS_PERMISSON_8D(EMI_MPU_UNLOCK, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN,
				EMI_MPU_FORBIDDEN, EMI_MPU_SEC_RW, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_FORBIDDEN, EMI_MPU_SEC_RW);
		emi_mpu_set_region_protection(addr_start, addr_end, TUI_MEMORY_MPU_REGION_ID, mpu_attr);
		mpu_protected = 1;
	}
#endif

	protect_cnt++;

	if (protect_cnt == 2) {
		tui_init_extra_pa(physAddr+size);
		extra_mem_inited = 1;
	}

    return ret;
}

void clean_mem(uint32_t addr_start, uint32_t size)
{
	drApiResult_t ret;
	uint8_t *va = NULL;

	ret = drApiMapPhysicalBuffer((uint64_t)addr_start, size, MAP_READABLE | MAP_WRITABLE | MAP_UNCACHED, &va);
	if (ret == DRAPI_OK) {
		memset(va, 0, size);
		drApiUnmapBuffer((void *)va);
	} else {
		drApiLogPrintf("TUI-memory map failed: 0x%x\n", addr_start);
	}
}


/**
 *  tuiHalDisplayUnprotectFB()
 *      uint32_t physAddr : Physical address of memory region to be secured. Must be aligned on size. Only 32-bit supported.
 *      uint32_t size :   size of the region
 */
drTuiHalError_t tuiHalFBUnprotect(uint32_t physAddr, uint32_t size) {
    drTuiHalError_t ret = TUI_HAL_OK;
	uint32_t addr_start, addr_end;
	uint32_t mpu_attr;
	uint32_t total_mem_size;
    DDPMSG("%s addr=0x%x, size=0x%x", __func__, physAddr, size);

	// Secure memory has been protected in preloader.
#if 0
	if (protect_cnt && mpu_protected) {
		addr_start = physAddr;
		total_mem_size = (X_RES*BYTES_PER_PIXEL*Y_RES)*2 + EXTRA_MEM_SIZE;
		addr_end = physAddr + total_mem_size - 1;
		/* Clean memory */
		DDPMSG("%s addr_start=0x%x, addr_end=0x%x total=0x%x", __func__, addr_start, addr_end, total_mem_size);
		clean_mem(addr_start, total_mem_size);
		//memset((void*)addr_start, 0, total_mem_size);

		/* Disable MPU protection */
		mpu_attr = SET_ACCESS_PERMISSON_8D(EMI_MPU_UNLOCK, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION,
				EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION,
				EMI_MPU_NO_PROTECTION, EMI_MPU_NO_PROTECTION);
		emi_mpu_set_region_protection(addr_start, addr_end, TUI_MEMORY_MPU_REGION_ID, mpu_attr);
		mpu_protected = 0;
	}
#endif

    buffer_cnt = 0;
	protect_cnt = 0;
	extra_mem_inited = 0;

    return ret;
}

void tuiHalPrintFBInfo(tuiFbInfo_t *pFbInfo, const char *msg)
{
	DDPMSG("%s fb_info is =======>\n", __func__);
	DDPMSG("x=%d,y=%d,density=%d,bpp=%d,pitch=%d,type=%d,sec=%d,size=%d,phys=0x%x,va=0x%p\n",
		pFbInfo->xRes, pFbInfo->yRes, pFbInfo->density, pFbInfo->bitsPerPixel,
		pFbInfo->lineLength, pFbInfo->type, pFbInfo->memSecureState,
		pFbInfo->fbSize, (uint32_t)pFbInfo->fbPhysAddr, pFbInfo->fbVirtAddr);

}


drTuiHalError_t do_tuiHalFBImageBlt(tuiFbInfo_t *pFbInfo, tuiImage_t *pImage, tuiOrientation_t orientation)
{

	int x, y;
	int w, h;
	uint8_t *dst, *src;
	int dst_pitch, src_pitch;
	int Bpp = pFbInfo->bitsPerPixel / 8;

	ASSERT(orientation == TUI_ORIENTATION_0);
	ASSERT(pFbInfo->bitsPerPixel == pImage->depth);

	dst_pitch = pFbInfo->lineLength;
	src_pitch = pImage->width * pImage->depth/8;
	dst = pFbInfo->fbVirtAddr + pImage->org.xOffset * Bpp +	pImage->org.yOffset * dst_pitch;
	src = pImage->data;
	w = pImage->width * Bpp;
	h = pImage->height;

	for(y=0; y<pImage->height; y++) {

		/* use 32 bit copy to optimise */
		for(x=0; x<w; x++) {
			dst[x] = src[x];
		}
		dst += dst_pitch;
		src += src_pitch;
	}

	drApiCacheDataCleanAll();

	return TUI_HAL_OK;
}


/*
 * Copy an image to the framebuffer.
 */
drTuiHalError_t tuiHalFBImageBlt(tuiFbInfo_t *pFbInfo, tuiImage_t *pImage, tuiOrientation_t orientation)
{
    drTuiHalError_t ret = TUI_HAL_OK;
	tuiHalPrintFBInfo(pFbInfo, __func__);
	DDPMSG("%s img: x_y=(%d,%d),w=%d,h=%d,dep=%d,va=0x%p,size=0x%x,orient=%d\n",
		__func__, pImage->org.xOffset, pImage->org.yOffset,
		pImage->width, pImage->height, pImage->depth,
		pImage->data, pImage->size, orientation);

	do_tuiHalFBImageBlt(pFbInfo, pImage, orientation);
	disp_tui_pan_display(pFbInfo);

#if 0
	int i;
	for(i=0; i<1920/10; i++)
	{
		disp_tui_pan_display(i * 10 * pFbInfo->lineLength);
		DDPMSG("tui trigger done %d\n", i);
	}
#endif
    return ret;
}
