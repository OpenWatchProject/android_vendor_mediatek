/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <drv_defs.h>
#include <drv_error.h>
#include <drv_fwk.h>
#include <drv_fwk_macro.h>
//#include <stdio.h>
//#include <string.h>

#include <drfod_api.h>

#include "drStd.h"
//#include "DrApi/DrApi.h"

#define LOG_TAG "fod_drv"
#include <fod_tdrv_ioctl.h>

#include "fod_tdrv.h"

#define FOD_DRV_ID DRIVER_ID
#define DRVTAG LOG_TAG

#ifndef LOW_DWORD
#define LOW_DWORD(x) ((unsigned int)((x)&0xFFFFFFFF))
#endif

/* Add sizeof() checking to get rid of "right shift count >= width of type"
 * warning */
#ifndef HIGH_DWORD
#define HIGH_DWORD(x) \
    (sizeof(x) > 4 ? (unsigned int)(((x) >> 32) & 0xFFFFFFFF) : 0U)
#endif

void *shm_mem = NULL;
uint64_t sapu_tee_shm_pa = 0;
uint32_t sapu_tee_shm_size = 0;
uint32_t sapu_tee_buf_offset[2] = {0, 0};
uint32_t sapu_tee_prop_offset[2] = {0, 0};

uint8_t *my_mem1_va = NULL;
uint8_t *my_mem2_va = NULL;
uint64_t my_mem1_pa = 0;
uint32_t my_mem1_sz = 0;
uint64_t my_mem2_pa = 0;
uint32_t my_mem2_sz = 0;

#ifdef VPU_M4U_ENABLE
#ifdef USE_NORMAL_M4U
	#define SDSP_VPU0_ELF_MVA       0x82600000  //max 8M-64K
	#define SDSP_VPU1_ELF_MVA       0x82E00000  //max 8M-64K
	#define SDSP_VPU0_DTA_MVA       0x83600000  //max 48M
#else
	#define SDSP_VPU0_ELF_MVA       0x3C000000  //max 8M-64K
	#define SDSP_VPU1_ELF_MVA       0x3C400000  //max 8M-64K
	#define SDSP_VPU0_DTA_MVA       0x38000000  //max 48M
#endif
const uint64_t TEE_VPU_SHM_MVA = SDSP_VPU0_DTA_MVA;
#endif
uint64_t SRC_OFFSET[2] = { 0x00000000, 0x00400000 };
uint64_t DST_OFFSET[2] = { 0x00200000, 0x00600000 };

static int fod_invoke_command(unsigned int sid, unsigned int cmd,
                               unsigned long args)
{
    int fwk_ret = DRV_FWK_API_OK;
    int drv_ret = DRV_FWK_API_OK;
    struct fod_tdrv_params* ta_params = NULL;
    (void)sid;
	void *source_mem;
	void *dest_mem;
	MSEE_Result msee_ret;
	uint32_t core;
	//taskid_t taskid;
	/*
	threadid_t ipcClient = NILTHREAD;
    message_t  ipcMsg    = MSG_RD;
    uint32_t   ipcData   = 0;
	*/
	//drApiResult_t drApi_ret;

    msee_logd("sid:%d, cmd:%d, args:0x%x\n", sid, cmd, (unsigned int)args);

    fwk_ret =
        msee_map_user((void*)&ta_params, (void*)args,
                      sizeof(struct fod_tdrv_params), MSEE_MAP_USER_DEFAULT);
    if (fwk_ret != DRV_FWK_API_OK) {
        msee_logd("map_user failed %d\n", fwk_ret);
        return fwk_ret;
    }

	msee_logd("%s cmd:%d, pa(0x%08x%08x) va(0x%x) size(0x%x) offset(0x%x) buf_offset(0x%x) buf_num(0x%x), core(%u)\n", __func__, cmd,
		HIGH_DWORD(ta_params->mem_info.shm_pa), LOW_DWORD(ta_params->mem_info.shm_pa),
		ta_params->mem_info.tapp_mem_va, ta_params->mem_info.tapp_mem_size, ta_params->mem_info.shm_offset,
		ta_params->mem_info.buf_offset, ta_params->mem_info.buf_num, ta_params->mem_info.core);

	core = ta_params->mem_info.core;

    switch (cmd) {
	case FOD_TDRV_GET_SHM_PA:
		sapu_tee_shm_pa = ta_params->mem_info.shm_pa;
		sapu_tee_shm_size = ta_params->mem_info.tapp_mem_size;
		sapu_tee_buf_offset[0] = ta_params->mem_info.tapp_mem_size - (SAPU_TEE_BUF_SIZE + SAPU_TEE_PROP_SIZE);
		sapu_tee_prop_offset[0] = ta_params->mem_info.tapp_mem_size - SAPU_TEE_PROP_SIZE;
		sapu_tee_buf_offset[1] = ta_params->mem_info.tapp_mem_size - (SAPU_TEE_BUF_SIZE + SAPU_TEE_PROP_SIZE)*2;
		sapu_tee_prop_offset[1] = ta_params->mem_info.tapp_mem_size - (SAPU_TEE_BUF_SIZE + SAPU_TEE_PROP_SIZE) - SAPU_TEE_PROP_SIZE;
		msee_logd("shm_mem_pa/size(0x%08x%08x/0x%x)\n", HIGH_DWORD(sapu_tee_shm_pa), LOW_DWORD(sapu_tee_shm_pa), sapu_tee_shm_size);
		msee_logd("buf0/prop offset(0x%x/0x%x)\n", sapu_tee_buf_offset[0], sapu_tee_prop_offset[0]);
		msee_logd("buf1/prop offset(0x%x/0x%x)\n", sapu_tee_buf_offset[1], sapu_tee_prop_offset[1]);
		if (sapu_tee_shm_size<0x800000) {
			msee_logd("renew buf_offset\n");
			SRC_OFFSET[0] = 0x00000000;
			DST_OFFSET[0] = 0x00100000;
			SRC_OFFSET[1] = 0x00200000;
			DST_OFFSET[1] = 0x00300000;
		}
		break;

    case FOD_TDRV_MMAP_SHM:
		if (sapu_tee_shm_pa==0 || sapu_tee_shm_size==0) {
			drv_ret = -1;
			break;
		}

#ifdef WITHOUT_DSP
		drv_ret = msee_mmap_region(sapu_tee_shm_pa, (void**)&shm_mem, sapu_tee_shm_size,
									MSEE_MAP_READABLE|MSEE_MAP_WRITABLE|MSEE_MAP_ALLOW_NONSECURE/*|MSEE_MAP_UNCACHED*/);
#else
		drv_ret = msee_mmap_region(sapu_tee_shm_pa, (void**)&shm_mem, sapu_tee_shm_size,
									MSEE_MAP_READABLE|MSEE_MAP_WRITABLE/*|MSEE_MAP_UNCACHED*/);
#endif
		if (drv_ret==0) {
			msee_logd("shm_mem(%p)\n", shm_mem);
			if (1) {
				memset(shm_mem, 0x77, sapu_tee_shm_size);
			}
		}
        break;

    case FOD_TDRV_UNMAP_SHM:
		if (shm_mem!=NULL) {
			drv_ret = msee_unmmap_region(shm_mem, sapu_tee_shm_size);
			shm_mem = NULL;
			sapu_tee_shm_pa = 0;
			sapu_tee_shm_size = 0;
			sapu_tee_buf_offset[0] = 0;
			sapu_tee_prop_offset[0] = 0;
			sapu_tee_buf_offset[1] = 0;
			sapu_tee_prop_offset[1] = 0;
		}
		else
			drv_ret = -1;
        break;

	case FOD_TDRV_COPY_TO_SHM:
		msee_ret = msee_map_user((void **)&source_mem, (void *)ta_params->mem_info.tapp_mem_va, ta_params->mem_info.tapp_mem_size, MSEE_MAP_READABLE|MSEE_MAP_WRITABLE);
		if (msee_ret) {
			drv_ret = -1;
			msee_logd("msee_map_user fail(%u)\n", msee_ret);
			break;
		} else {
			msee_logd("msee_map_user done(%u)\n", msee_ret);
		}

		memcpy(shm_mem+ta_params->mem_info.shm_offset, source_mem, ta_params->mem_info.tapp_mem_size);
#ifndef WITHOUT_DSP
		msee_clean_invalidate_dcache_range((unsigned long)shm_mem, (unsigned long)sapu_tee_shm_size);
#endif
		msee_ret = msee_unmap_user(source_mem);
		if (msee_ret) {
			drv_ret = -1;
			msee_logd("msee_unmap_user fail(%u)\n", msee_ret);
			break;
		} else {
			msee_logd("msee_unmap_user done(%u)\n", msee_ret);
		}
		break;

	case FOD_TDRV_COPY_FROM_SHM:
		//msee_clean_invalidate_dcache_all();
		msee_ret = msee_map_user((void **)&dest_mem, (void *)ta_params->mem_info.tapp_mem_va, ta_params->mem_info.tapp_mem_size, MSEE_MAP_READABLE|MSEE_MAP_WRITABLE);
		if (msee_ret) {
			drv_ret = -1;
			msee_logd("msee_map_user fail(%u)\n", msee_ret);
			break;
		} else {
			msee_logd("msee_map_user done(%u)\n", msee_ret);
		}

		memcpy(dest_mem, shm_mem+ta_params->mem_info.shm_offset, ta_params->mem_info.tapp_mem_size);
		msee_logd("COPY_FROM_SHM 0-3 0x%x\n", *(uint32_t *)(shm_mem+ta_params->mem_info.shm_offset));
		msee_logd("COPY_FROM_SHM 4-7 0x%x\n", *(uint32_t *)(shm_mem+ta_params->mem_info.shm_offset+4));

		msee_ret = msee_unmap_user(dest_mem);
		if (msee_ret) {
			drv_ret = -1;
			msee_logd("msee_unmap_user fail(%u)\n", msee_ret);
			break;
		} else {
			msee_logd("msee_unmap_user done(%u)\n", msee_ret);
		}
		break;

	case FOD_TDRV_SETUP_SHM_CMD:
		if (ta_params->mem_info.prop_cmd==0x169) {
			vpu_prop_t *prop_info;
			prop_info = (vpu_prop_t *)(shm_mem+sapu_tee_prop_offset[core]);
			prop_info->cmd = ta_params->mem_info.prop_cmd;
			prop_info->result = 7;
			msee_clean_invalidate_dcache_range((unsigned long)shm_mem, (unsigned long)sapu_tee_shm_size);
			ta_params->mem_info.buf_offset = sapu_tee_buf_offset[core];
			ta_params->mem_info.buf_num = 2;
			ta_params->mem_info.prop_offset = sapu_tee_prop_offset[core];
			ta_params->mem_info.prop_size = sizeof(vpu_prop_t);
			msee_loge("FOD_TDRV_SETUP_SHM_CMD (0x%x)\n", prop_info->cmd);
			break;
		}

		if (ta_params->mem_info.prop_cmd==0x16a) {
			vpu_prop_t *prop_info;
			prop_info = (vpu_prop_t *)(shm_mem+sapu_tee_prop_offset[core]);
			prop_info->cmd = ta_params->mem_info.prop_cmd;
			prop_info->result = 7;
			msee_clean_invalidate_dcache_range((unsigned long)shm_mem, (unsigned long)sapu_tee_shm_size);
			ta_params->mem_info.buf_offset = sapu_tee_buf_offset[core];
			ta_params->mem_info.buf_num = 2;
			ta_params->mem_info.prop_offset = sapu_tee_prop_offset[core];
			ta_params->mem_info.prop_size = sizeof(vpu_prop_t);
			msee_loge("FOD_TDRV_SETUP_SHM_CMD (0x%x)\n", prop_info->cmd);
			break;
		}

		if (shm_mem!=NULL && sapu_tee_buf_offset[core]!=0) {
			uint32_t i;
			vpu_buffer_t *buf_info;
			vpu_prop_t *prop_info;
			buf_info = (vpu_buffer_t *)(shm_mem+sapu_tee_buf_offset[core]);

			i = 0;
			buf_info[i].port_id = core;
			buf_info[i].format = VPU_DATA_FMT_IMG_Y;
			buf_info[i].plane_count = 1;
			buf_info[i].width = IMAGE_WIDTH;
			buf_info[i].height = IMAGE_HEIGHT;
#ifdef VPU_M4U_ENABLE
			buf_info[i].planes[0].ptr = TEE_VPU_SHM_MVA+SRC_OFFSET[core];
#else
			buf_info[i].planes[0].ptr = (uint64_t)sapu_tee_shm_pa+SRC_OFFSET[core];
#endif
			buf_info[i].planes[0].length = (uint32_t)IMAGE_HD_SIZE;
			buf_info[i].planes[0].stride = IMAGE_WIDTH;
			msee_logd("buf_info[%d]%p\n", i, &buf_info[i]);

			i++;
			buf_info[i].port_id = core;
			buf_info[i].format = VPU_DATA_FMT_IMG_Y;
			buf_info[i].plane_count = 1;
			buf_info[i].width = IMAGE_WIDTH;
			buf_info[i].height = IMAGE_HEIGHT;
#ifdef VPU_M4U_ENABLE
			buf_info[i].planes[0].ptr = TEE_VPU_SHM_MVA+DST_OFFSET[core];
#else
			buf_info[i].planes[0].ptr = (uint64_t)sapu_tee_shm_pa+DST_OFFSET[core];
#endif
			buf_info[i].planes[0].length = (uint32_t)IMAGE_HD_SIZE;
			buf_info[i].planes[0].stride = IMAGE_WIDTH;
			msee_logd("buf_info[%d]%p\n", i, &buf_info[i]);

#ifdef TRUSTONIC_TEE_ENABLED
			assert(i<((1024*3)/64));
#endif

			prop_info = (vpu_prop_t *)(shm_mem+sapu_tee_prop_offset[core]);
			prop_info->cmd = ta_params->mem_info.prop_cmd;
			prop_info->result = 7;
#ifndef WITHOUT_DSP
			msee_clean_invalidate_dcache_range((unsigned long)shm_mem, (unsigned long)sapu_tee_shm_size);
#endif
		} else {
			drv_ret = -1;
		}
		ta_params->mem_info.buf_offset = sapu_tee_buf_offset[core];
		ta_params->mem_info.buf_num = 2;
		ta_params->mem_info.prop_offset = sapu_tee_prop_offset[core];
		ta_params->mem_info.prop_size = sizeof(vpu_prop_t);
		break;

	case FOD_TDRV_MMAP1_STATIC_GZ_MTEE_SHM:
		my_mem1_pa = ta_params->mem_info.shm_pa;
		my_mem1_sz = ta_params->mem_info.tapp_mem_size;
		drv_ret = msee_mmap_region(my_mem1_pa, (void**)&my_mem1_va, my_mem1_sz,
						MSEE_MAP_READABLE|MSEE_MAP_WRITABLE|MSEE_MAP_ALLOW_NONSECURE/*|MSEE_MAP_UNCACHED*/);
		msee_logd("my_mem1_pa va(%p) (0x%08x%08x), size(0x%x)\n", my_mem1_va, HIGH_DWORD(my_mem1_pa), LOW_DWORD(my_mem1_pa), my_mem1_sz);
		break;

	case FOD_TDRV_MMAP2_STATIC_GZ_MTEE_SHM:
		my_mem2_pa = ta_params->mem_info.shm_pa;
		my_mem2_sz = ta_params->mem_info.tapp_mem_size;
		drv_ret = msee_mmap_region(my_mem2_pa, (void**)&my_mem2_va, my_mem2_sz,
						MSEE_MAP_READABLE|MSEE_MAP_WRITABLE|MSEE_MAP_ALLOW_NONSECURE/*|MSEE_MAP_UNCACHED*/);
		msee_logd("my_mem2_pa va(%p) (0x%08x%08x), size(0x%x)\n", my_mem2_va, HIGH_DWORD(my_mem2_pa), LOW_DWORD(my_mem2_pa), my_mem2_sz);
		break;

	case FOD_TDRV_TEST_STATIC_GZ_MTEE_SHM:
		if (my_mem1_va==NULL || my_mem2_va==NULL) {
			msee_loge("no my_mem_va\n");
			drv_ret = -1;
			break;
		}
		//msee_loge("+data=0x%x for my_mem1_va\n", my_mem1_va[0]);
		memset(my_mem1_va, my_mem1_va[0]+1, my_mem1_sz);
		msee_loge("-data=0x%x for my_mem1_va\n", my_mem1_va[0]);
		//msee_loge("+data=0x%x for my_mem2_va\n", my_mem2_va[0]);
		memset(my_mem2_va, my_mem2_va[0]+1, my_mem2_sz);
		msee_loge("-data=0x%x for my_mem2_va\n", my_mem2_va[0]);
		//msee_loge("data-=0x%x\n", my_mem_va[0]);
		break;

	case FOD_TDRV_UMAP_STATIC_GZ_MTEE_SHM:
		if (my_mem1_va==NULL) {
			msee_loge("no my_mem1_va\n");
		} else {
			drv_ret = msee_unmmap_region(my_mem1_va, my_mem1_sz);
			if (!drv_ret) {
				my_mem1_va = NULL;
				my_mem1_pa = 0;
				my_mem1_sz = 0;
				msee_logd("test1 msee_unmmap_region done\n");
			} else {
				msee_loge("test1 msee_unmmap_region fail\n");
				drv_ret = -1;
				break;
			}
		}

		if (my_mem2_va==NULL) {
			msee_loge("no my_mem2_va\n");
		} else {
			drv_ret = msee_unmmap_region(my_mem2_va, my_mem2_sz);
			if (!drv_ret) {
				my_mem2_va = NULL;
				my_mem2_pa = 0;
				my_mem2_sz = 0;
				msee_logd("test2 msee_unmmap_region done\n");
			} else {
				msee_loge("test2 msee_unmmap_region fail\n");
				drv_ret = -1;
				break;
			}
		}
		break;

    default:
        msee_logd("command is not supported %d", cmd);
        drv_ret = DRV_FWK_API_INVALIDATE_PARAMETERS;
        break;
    }

    ta_params->drv_ret = drv_ret;
    fwk_ret = msee_unmap_user(ta_params);
    if (fwk_ret != DRV_FWK_API_OK) {
        msee_logd("unmap_user failed %d\n", fwk_ret);
        return fwk_ret;
    }

    return DRV_FWK_API_OK;
}

static int fod_open_session(unsigned int sid, unsigned long args)
{
    (void)sid;
    (void)args;
    msee_logd("%s %d\n", __func__, __LINE__);
    return 0;
}

static int fod_close_session(unsigned int sid)
{
    (void)sid;
    msee_logd("%s %d\n", __func__, __LINE__);
    return 0;
}

static int fod_init(const struct msee_drv_device* dev)
{
    (void)dev;
    msee_logd("%s %d\n", __func__, __LINE__);

    return 0;
}

DECLARE_DRIVER_MODULE(DRVTAG, FOD_DRV_ID, fod_init, fod_invoke_command, NULL,
                      fod_open_session, fod_close_session);
