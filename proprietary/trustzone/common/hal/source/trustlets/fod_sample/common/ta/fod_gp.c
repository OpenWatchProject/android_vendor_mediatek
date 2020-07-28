/*
 * Copyright (c) 2018 MediaTek Inc.
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

#include <drv_call.h>
#include <tee_taStd.h>

#include <drSecMemApi.h>
#include <drfod_api.h>
#include <tee_internal_api.h>

#define LOG_TAG "fod_ta"

#define UNUSED(x) (void)x

/* clang-format off */
#define ASSERT_PARAM_TYPE(pt) \
    do { \
        if ((pt) != nParamTypes) \
        return TEE_ERROR_BAD_PARAMETERS; \
    } while (0)
/* clang-format on */

#ifndef HIGH_DWORD
#define HIGH_DWORD(x) \
    (sizeof(x) > 4 ? (unsigned int)(((x) >> 32) & 0xFFFFFFFF) : 0U)
#endif

#ifndef LOW_DWORD
#define LOW_DWORD(x) ((unsigned int)((x)&0xFFFFFFFF))
#endif

#define IMAGE_WIDTH (1280)
#define IMAGE_HEIGHT (720)
#define IMAGE_HD_SIZE (IMAGE_WIDTH*IMAGE_HEIGHT)

uint64_t SRC_OFFSET[2] = { 0x00000000, 0x00400000 };
uint64_t DST_OFFSET[2] = { 0x00200000, 0x00600000 };

/*
#define SRC0_OFFSET 0x00000000
#define DST0_OFFSET 0x00200000
#define SRC1_OFFSET 0x00400000
#define DST1_OFFSET 0x00600000
*/

#define SECURE_M4U_SDSP_TEE_SHM_START_MVA	0x38000000
#define SECURE_M4U_SDSP_FIRMWARE_START_MVA	0x3C000000

DECLARE_TRUSTED_APPLICATION_MAIN_STACK(4096);
void *src_mem = NULL;
void *dst_mem = NULL;
uint32_t vpu_prop_offset[2] = {0, 0};

TEE_Result TA_EXPORT TA_CreateEntryPoint(void)
{
    msee_ta_logd("[%s] %d\n", __FUNCTION__, __LINE__);
    return TEE_SUCCESS;
}

void TA_EXPORT TA_DestroyEntryPoint(void)
{
    msee_ta_logd("[%s] %d\n", __FUNCTION__, __LINE__);
}

TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(uint32_t nParamTypes,
                                              TEE_Param pParams[4],
                                              void** ppSessionContext)
{
    UNUSED(nParamTypes);
    UNUSED(pParams);
    UNUSED(ppSessionContext);
    msee_ta_logd("[%s] %d\n", __FUNCTION__, __LINE__);

    return TEE_SUCCESS;
}

void TA_EXPORT TA_CloseSessionEntryPoint(void* pSessionContext)
{
    UNUSED(pSessionContext);
    msee_ta_logd("[%s] %d\n", __FUNCTION__, __LINE__);
}

#define PMEM_64BIT_PHYS_SHIFT (10)
#define HANDLE_TO_PA(handle)                       \
    (((uint64_t)handle << PMEM_64BIT_PHYS_SHIFT) & \
     ~((1 << PMEM_64BIT_PHYS_SHIFT) - 1))

int m4u_alloc_mva_sec_noipc_vpu(int port, uint64_t pa, unsigned int size, unsigned int *pMva);

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(void* pSessionContext,
                                                uint32_t nCommandID,
                                                uint32_t nParamTypes,
                                                TEE_Param pParams[4])
{
    int ret = TEE_SUCCESS;
	int drv_ret = 0;
	uint32_t i;
    UNUSED(pSessionContext);
	uint32_t buf_offset;
	uint32_t buf_num;
	uint32_t prop_offset;
	uint32_t prop_size;
	uint64_t shm_paddr = 0;
	uint64_t shm_size = 0;
#ifndef USE_NORMAL_M4U
	uint32_t shm_mva = 0;
#endif
	uint64_t sdsp_paddr = 0;
	uint64_t sdsp_size = 0;
#ifndef USE_NORMAL_M4U
	uint32_t sdsp_mva = 0;
#endif
	uint32_t my_prop[2] = {0, 0};

	uint64_t my_mem1_pa, my_mem1_sz, my_mem2_pa, my_mem2_sz;

    ASSERT_PARAM_TYPE(TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_VALUE_INOUT, TEE_PARAM_TYPE_VALUE_INOUT,
        TEE_PARAM_TYPE_VALUE_INOUT, TEE_PARAM_TYPE_VALUE_INOUT));

    msee_ta_logd("command is 0x%x\n", nCommandID);

    switch (nCommandID) {
	case 0x2019:
		src_mem = TEE_Malloc(IMAGE_HD_SIZE, 0);
		if (src_mem==NULL) {
			msee_ta_loge("TEE_Malloc src fail\n");
			drv_ret = -1;
			break;
		} else {
			msee_ta_logd("TEE_Malloc src done(%p)\n", src_mem);
		}

		dst_mem = TEE_Malloc(IMAGE_HD_SIZE, 0);
		if (dst_mem==NULL) {
			msee_ta_loge("TEE_Malloc dst fail\n");
			drv_ret = -1;
			break;
		} else {
			msee_ta_logd("TEE_Malloc dst done(%p)\n", dst_mem);
		}

		if (1) {
			memset(dst_mem, 0, IMAGE_HD_SIZE);
			//for (i=0 ; i<IMAGE_HD_SIZE ; i=i+4) {
			for (i=0 ; i<8192 ; i=i+4) {
				TEE_GenerateRandom((void *)(src_mem+i), 4);
				if (i<16) {
 					msee_ta_loge("[%d]0x%02x\n", i, *(uint32_t *)(src_mem+i));
				}
			}
		}

		ret = drRemappedMemPoolQueryByType(&shm_paddr, &shm_size, MEM_SDSP_SHARED);
		if (ret!=0) {
			msee_ta_loge("drRemappedMemPoolQueryByType fail(%d) for MEM_SDSP_SHARED\n", ret);
			break;
		}

		msee_ta_logd("shm_paddr/size(0x%08x%08x/0x%08x%08x)\n",
			HIGH_DWORD(shm_paddr), LOW_DWORD(shm_paddr), HIGH_DWORD(shm_size), LOW_DWORD(shm_size));


		vpu_prop_offset[0] = (uint32_t)shm_size-(0*4096)-(1*1024);
		vpu_prop_offset[1] = (uint32_t)shm_size-(1*4096)-(1*1024);
		msee_ta_logd("prop_offset(0x%08x/0x%08x)\n", vpu_prop_offset[0], vpu_prop_offset[1]);
		if (shm_size<0x800000) {
			msee_ta_logd("renew buf_offset\n");
			SRC_OFFSET[0] = 0x00000000;
			DST_OFFSET[0] = 0x00100000;
			SRC_OFFSET[1] = 0x00200000;
			DST_OFFSET[1] = 0x00300000;
		}



		ret = fod_tdrv_setup_shm_pa(shm_paddr, (uint32_t)shm_size);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_setup_shm_pa fail(%d)\n", ret);
			break;
		}

		ret = fod_tdrv_mmap_shm(0, 0, 0, &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_mmap_shm fail(%d)\n", ret);
			break;
		}

		msee_ta_logd("patten %p %p\n", src_mem, dst_mem);
		ret = fod_tdrv_copy_2_shm((uintptr_t)src_mem, IMAGE_HD_SIZE, SRC_OFFSET[0], &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_copy_2_shm src0 fail(%d)\n", ret);
			break;
		}

		msee_ta_logd("patten %p %p\n", src_mem, dst_mem);
		ret = fod_tdrv_copy_2_shm((uintptr_t)src_mem, IMAGE_HD_SIZE, SRC_OFFSET[1], &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_copy_2_shm src1 fail(%d)\n", ret);
			break;
		}


		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 0, 0x168);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
			break;
		} else {
			msee_ta_logd("buf0_offset(0x%x) buf_num(%u)\n", buf_offset, buf_num);
			msee_ta_logd("prop0_offset(0x%x) prop_size(%u)\n", prop_offset, prop_size);
			pParams[0].value.a = buf_offset;
			pParams[0].value.b = buf_num;
			pParams[1].value.a = prop_offset;
			pParams[1].value.b = prop_size;
		}

		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 1, 0x168);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
			break;
		} else {
			msee_ta_logd("buf1_offset(0x%x) buf_num(%u)\n", buf_offset, buf_num);
			msee_ta_logd("prop1_offset(0x%x) prop_size(%u)\n", prop_offset, prop_size);
			pParams[2].value.a = buf_offset;
			pParams[2].value.b = buf_num;
			pParams[3].value.a = prop_offset;
			pParams[3].value.b = prop_size;
		}

		break;

    case 0x10101:
		src_mem = TEE_Malloc(IMAGE_HD_SIZE, 0);
		if (src_mem==NULL) {
			msee_ta_loge("TEE_Malloc src fail\n");
			drv_ret = -1;
			break;
		} else {
			msee_ta_logd("TEE_Malloc src done(%p)\n", src_mem);
		}

		dst_mem = TEE_Malloc(IMAGE_HD_SIZE, 0);
		if (dst_mem==NULL) {
			msee_ta_loge("TEE_Malloc dst fail\n");
			drv_ret = -1;
			break;
		} else {
			msee_ta_logd("TEE_Malloc dst done(%p)\n", dst_mem);
		}

		if (1) {
			memset(dst_mem, 0, IMAGE_HD_SIZE);
			//for (i=0 ; i<IMAGE_HD_SIZE ; i=i+4) {
			for (i=0 ; i<8192 ; i=i+4) {
				TEE_GenerateRandom((void *)(src_mem+i), 4);
				if (i<16) {
 					msee_ta_loge("[%d]0x%02x\n", i, *(uint32_t *)(src_mem+i));
				}
			}
		}

		ret = drMemPoolQueryByType(&shm_paddr, &shm_size, MEM_SDSP_SHARED);
		if (ret!=0) {
			msee_ta_loge("drMemPoolQueryByType fail(%d) for MEM_SDSP_SHARED\n", ret);
			break;
		}
#ifndef USE_NORMAL_M4U
		ret = m4u_alloc_mva_sec_noipc_vpu(151, shm_paddr, (unsigned int)shm_size, (unsigned int *)&shm_mva);
		if (ret!=0 || shm_mva!=SECURE_M4U_SDSP_TEE_SHM_START_MVA) {
			msee_ta_loge("MEM_SDSP_TEE mva fail(%d), , mva(0x%x)\n", ret, shm_mva);
			break;
		}
		msee_ta_logd("shm_paddr/size(0x%08x%08x/0x%08x%08x), mva(0x%x)\n",
			HIGH_DWORD(shm_paddr), LOW_DWORD(shm_paddr), HIGH_DWORD(shm_size), LOW_DWORD(shm_size), shm_mva);
#else
		msee_ta_logd("shm_paddr/size(0x%08x%08x/0x%08x%08x)\n",
			HIGH_DWORD(shm_paddr), LOW_DWORD(shm_paddr), HIGH_DWORD(shm_size), LOW_DWORD(shm_size));
#endif

		vpu_prop_offset[0] = (uint32_t)shm_size-(0*4096)-(1*1024);
		vpu_prop_offset[1] = (uint32_t)shm_size-(1*4096)-(1*1024);
		msee_ta_logd("prop_offset(0x%08x/0x%08x)\n", vpu_prop_offset[0], vpu_prop_offset[1]);
		if (shm_size<0x800000) {
			msee_ta_logd("renew buf_offset\n");
			SRC_OFFSET[0] = 0x00000000;
			DST_OFFSET[0] = 0x00100000;
			SRC_OFFSET[1] = 0x00200000;
			DST_OFFSET[1] = 0x00300000;
		}

		ret = drMemPoolQueryByType(&sdsp_paddr, &sdsp_size, MEM_SDSP_FIRMWARE);
		if (ret!=0) {
			msee_ta_loge("drMemPoolQueryByType fail(%d) for MEM_SDSP_FIRMWARE\n", ret);
			break;
		}
#ifndef USE_NORMAL_M4U
		ret = m4u_alloc_mva_sec_noipc_vpu(151, sdsp_paddr, (unsigned int)sdsp_size, (unsigned int *)&sdsp_mva);
		if (ret!=0 || sdsp_mva!=SECURE_M4U_SDSP_FIRMWARE_START_MVA) {
			msee_ta_loge("MEM_SDSP_FIRMWARE mva fail(%d), mva(0x%x)\n", ret, sdsp_mva);
			break;
		}
		msee_ta_logd("sdsp_paddr/size(0x%08x%08x/0x%08x%08x), mva(0x%x)\n",
			HIGH_DWORD(sdsp_paddr), LOW_DWORD(sdsp_paddr), HIGH_DWORD(sdsp_size), LOW_DWORD(sdsp_size), sdsp_mva);
#else
		msee_ta_logd("sdsp_paddr/size(0x%08x%08x/0x%08x%08x)\n",
			HIGH_DWORD(sdsp_paddr), LOW_DWORD(sdsp_paddr), HIGH_DWORD(sdsp_size), LOW_DWORD(sdsp_size));
#endif

		ret = fod_tdrv_setup_shm_pa(shm_paddr, (uint32_t)shm_size);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_map_shm fail(%d)\n", ret);
			break;
		}

		ret = fod_tdrv_mmap_shm(0, 0, 0, &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_map_shm fail(%d)\n", ret);
			break;
		}

		msee_ta_logd("patten %p %p\n", src_mem, dst_mem);
		ret = fod_tdrv_copy_2_shm((uintptr_t)src_mem, IMAGE_HD_SIZE, SRC_OFFSET[0], &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_copy_2_shm src0 fail(%d)\n", ret);
			break;
		}

		msee_ta_logd("patten %p %p\n", src_mem, dst_mem);
		ret = fod_tdrv_copy_2_shm((uintptr_t)src_mem, IMAGE_HD_SIZE, SRC_OFFSET[1], &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_copy_2_shm src1 fail(%d)\n", ret);
			break;
		}


		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 0, 0x168);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
			break;
		} else {
			msee_ta_logd("buf0_num(%u) prop_size(%u)\n", buf_num, prop_size);
			pParams[0].value.a = buf_num;
			pParams[0].value.b = prop_size;
		}

		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 1, 0x168);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
			break;
		} else {
			msee_ta_logd("buf1_num(%u) prop_size(%u)\n", buf_num, prop_size);
			pParams[1].value.a = buf_num;
			pParams[1].value.b = prop_size;
		}
		pParams[2].value.a = 0;
		pParams[2].value.b = 0;
		pParams[3].value.a = 0;
		pParams[3].value.b = 0;

        break;

	case 0x10102:
		ret = fod_tdrv_copy_from_shm((uintptr_t )my_prop, sizeof(my_prop), vpu_prop_offset[0], &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_copy_from_shm fail(%d)\n", ret);
			break;
		}
		msee_ta_loge("cmd(0x%x) result(%u) !!!!!\n", my_prop[0], my_prop[1]);
		if ( my_prop[0]==0x168 && my_prop[1]==100) {
			pParams[2].value.a = 100;
		} else {
			pParams[2].value.a = 59;
		}

		//msee_ta_logd("bef dst[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(dst_mem+0), *(uint32_t *)(dst_mem+4), *(uint32_t *)(dst_mem+8), *(uint32_t *)(dst_mem+12));
		memset(dst_mem, 0, IMAGE_HD_SIZE);
		if ( memcmp(dst_mem, src_mem, IMAGE_HD_SIZE)==0 ) {
			msee_ta_loge("before check data condition no expected !!!!!\n");
			msee_ta_loge("bef dst[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(dst_mem+0), *(uint32_t *)(dst_mem+4), *(uint32_t *)(dst_mem+8), *(uint32_t *)(dst_mem+12));
			pParams[2].value.b = 58;
			break;
		}
		ret = fod_tdrv_copy_from_shm((uintptr_t )dst_mem, IMAGE_HD_SIZE, DST_OFFSET[0], &drv_ret);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_copy_from_shm fail(%d)\n", ret);
			break;
		}
		if ( memcmp(dst_mem, src_mem, IMAGE_HD_SIZE)==0 ) {
			msee_ta_loge("after check data, copy data correct !!!!!\n");
			msee_ta_loge("aft [0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(dst_mem+0), *(uint32_t *)(dst_mem+4), *(uint32_t *)(dst_mem+8), *(uint32_t *)(dst_mem+12));
			pParams[2].value.b = 100;
		} else {
			msee_ta_loge("aft src[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(src_mem+0), *(uint32_t *)(src_mem+4), *(uint32_t *)(src_mem+8), *(uint32_t *)(src_mem+12));
			msee_ta_loge("aft dst[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint16_t *)(dst_mem+0), *(uint16_t *)(dst_mem+4), *(uint16_t *)(dst_mem+8), *(uint16_t *)(dst_mem+12));
			pParams[2].value.b = 59;
		}
		memset(dst_mem, 0, IMAGE_HD_SIZE);

		/*********************************************************************/
		ret = fod_tdrv_copy_from_shm((uintptr_t )my_prop, sizeof(my_prop), vpu_prop_offset[1], &drv_ret);
		if (ret!=0) {
			msee_ta_logd("fod_tdrv_copy_from_shm fail(%d)\n", ret);
			break;
		}
		msee_ta_loge("cmd(0x%x) result(%u) !!!!!\n", my_prop[0], my_prop[1]);
		if ( my_prop[0]==0x168 && my_prop[1]==100) {
			pParams[3].value.a = 100;
		} else {
			pParams[3].value.a = 59;
		}

		//msee_ta_logd("bef dst[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(dst_mem+0), *(uint32_t *)(dst_mem+4), *(uint32_t *)(dst_mem+8), *(uint32_t *)(dst_mem+12));
		if ( memcmp(dst_mem, src_mem, IMAGE_HD_SIZE)==0 ) {
			msee_ta_loge("before check data condition no expected !!!!!\n");
			msee_ta_loge("bef dst[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(dst_mem+0), *(uint32_t *)(dst_mem+4), *(uint32_t *)(dst_mem+8), *(uint32_t *)(dst_mem+12));
			pParams[3].value.b = 58;
			break;
		}
		ret = fod_tdrv_copy_from_shm((uintptr_t )dst_mem, IMAGE_HD_SIZE, DST_OFFSET[1], &drv_ret);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_copy_from_shm fail(%d)\n", ret);
			break;
		}
		if ( memcmp(dst_mem, src_mem, IMAGE_HD_SIZE)==0 ) {
			msee_ta_loge("after check data, copy data correct !!!!!\n");
			msee_ta_loge("aft [0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(dst_mem+0), *(uint32_t *)(dst_mem+4), *(uint32_t *)(dst_mem+8), *(uint32_t *)(dst_mem+12));
			pParams[3].value.b = 100;
		} else {
			msee_ta_loge("aft src[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint32_t *)(src_mem+0), *(uint32_t *)(src_mem+4), *(uint32_t *)(src_mem+8), *(uint32_t *)(src_mem+12));
			msee_ta_loge("aft dst[0-16]0x%x 0x%x 0x%x 0x%x\n", *(uint16_t *)(dst_mem+0), *(uint16_t *)(dst_mem+4), *(uint16_t *)(dst_mem+8), *(uint16_t *)(dst_mem+12));
			pParams[3].value.b = 59;
		}

		break;

	case 0x10169:
		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 0, 0x169);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
		} else {
			msee_ta_logd("0x169 buf0_num(%u) prop_size(%u)\n", buf_num, prop_size);
			pParams[0].value.a = buf_num;
			pParams[0].value.b = prop_size;
		}
		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 1, 0x169);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
		} else {
			msee_ta_logd("0x169 buf1_num(%u) prop_size(%u)\n", buf_num, prop_size);
			pParams[1].value.a = buf_num;
			pParams[1].value.b = prop_size;
		}
		pParams[2].value.a = 0;
		pParams[2].value.b = 0;
		pParams[3].value.a = 0;
		pParams[3].value.b = 0;
		break;

	case 0x1016a:
		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 0, 0x16a);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
		} else {
			msee_ta_logd("0x16a buf0_num(%u) prop_size(%u)\n", buf_num, prop_size);
			pParams[0].value.a = buf_num;
			pParams[0].value.b = prop_size;
		}
		ret = fod_tdrv_setup_shm_cmd(&buf_offset, &buf_num, &prop_offset, &prop_size, 1, 0x16a);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_setup_shm_cmd_data fail(%d)\n", ret);
		} else {
			msee_ta_logd("0x16a buf1_num(%u) prop_size(%u)\n", buf_num, prop_size);
			pParams[1].value.a = buf_num;
			pParams[1].value.b = prop_size;
		}
		pParams[2].value.a = 0;
		pParams[2].value.b = 0;
		pParams[3].value.a = 0;
		pParams[3].value.b = 0;
		break;

	case 0x10103:
		if (src_mem) {
			TEE_Free(src_mem);
			src_mem = NULL;
		}
		if (dst_mem) {
			TEE_Free(dst_mem);
			dst_mem = NULL;
		}
		ret = fod_tdrv_unmap_shm(0, 0, 0, &drv_ret);
		if (ret!=0) {
			msee_ta_loge("fod_tdrv_ummap_shm fail(%d)\n", ret);
			break;
		}
		break;

	case 0x10105:
		ret = drMemPoolQueryByType(&shm_paddr, &shm_size, MEM_SDSP_SHARED);
		if (ret!=0) {
			msee_ta_loge("drMemPoolQueryByType fail(%d) for MEM_SDSP_SHARED\n", ret);
			break;
		}
#ifndef USE_NORMAL_M4U
		ret = m4u_alloc_mva_sec_noipc_vpu(151, shm_paddr, (unsigned int)shm_size, (unsigned int *)&shm_mva);
		if (ret!=0 || shm_mva!=SECURE_M4U_SDSP_TEE_SHM_START_MVA) {
			msee_ta_loge("MEM_SDSP_TEE mva fail(%d), , mva(0x%x)\n", ret, shm_mva);
			break;
		}
		msee_ta_logd("shm_paddr/size(0x%08x%08x/0x%08x%08x), mva(0x%x)\n",
			HIGH_DWORD(shm_paddr), LOW_DWORD(shm_paddr), HIGH_DWORD(shm_size), LOW_DWORD(shm_size), shm_mva);
#else
		msee_ta_logd("shm_paddr/size(0x%08x%08x/0x%08x%08x)\n",
			HIGH_DWORD(shm_paddr), LOW_DWORD(shm_paddr), HIGH_DWORD(shm_size), LOW_DWORD(shm_size));
#endif

		ret = drMemPoolQueryByType(&sdsp_paddr, &sdsp_size, MEM_SDSP_FIRMWARE);
		if (ret!=0) {
			msee_ta_loge("drMemPoolQueryByType fail(%d) for MEM_SDSP_FIRMWARE\n", ret);
			break;
		}
#ifndef USE_NORMAL_M4U
		ret = m4u_alloc_mva_sec_noipc_vpu(151, sdsp_paddr, (unsigned int)sdsp_size, (unsigned int *)&sdsp_mva);
		if (ret!=0 || sdsp_mva!=SECURE_M4U_SDSP_FIRMWARE_START_MVA) {
			msee_ta_loge("MEM_SDSP_FIRMWARE mva fail(%d), mva(0x%x)\n", ret, sdsp_mva);
			break;
		}
		msee_ta_logd("sdsp_paddr/size(0x%08x%08x/0x%08x%08x), mva(0x%x)\n",
			HIGH_DWORD(sdsp_paddr), LOW_DWORD(sdsp_paddr), HIGH_DWORD(sdsp_size), LOW_DWORD(sdsp_size), sdsp_mva);
#else
		msee_ta_logd("sdsp_paddr/size(0x%08x%08x/0x%08x%08x)\n",
			HIGH_DWORD(sdsp_paddr), LOW_DWORD(sdsp_paddr), HIGH_DWORD(sdsp_size), LOW_DWORD(sdsp_size));
#endif
		break;

	case 0x10107:
		ret = drSecmemQueryRemappedGzTeeShmByName("for-ut-test1", &my_mem1_pa, &my_mem1_sz);
		if(ret) {
			msee_ta_loge("drSecmemQueryRemappedGzTeeShmByName failed \n");
			break;
                }

		ret = fod_tdrv_mmap1_StaticGzMteeShm(my_mem1_pa, my_mem1_sz);
		if(ret) {
			msee_ta_loge("fod_tdrv_mmap1_StaticGzMteeShm failed \n");
			break;
                }
		break;

	case 0x10108:
		ret = drSecmemQueryRemappedGzTeeShmByName("for-ut-test2", &my_mem2_pa, &my_mem2_sz);
		if(ret) {
			msee_ta_loge("drSecmemQueryRemappedGzTeeShmByName failed \n");
			break;
                }

		ret = fod_tdrv_mmap2_StaticGzMteeShm(my_mem2_pa, my_mem2_sz);
		if(ret) {
			msee_ta_loge("fod_tdrv_mmap1_StaticGzMteeShm failed \n");
			break;
                }
		break;

	case 0x10109:
		ret = fod_tdrv_test_StaticGzMteeShm(0, 0);
		break;

	case 0x1010a:
		ret = fod_tdrv_umap_StaticGzMteeShm(0, 0);
		break;

    default:
        msee_ta_loge("command is not supported 0x%x\n", nCommandID);
        break;
    }

    if (ret != 0) {
        return TEE_ERROR_GENERIC;
    }

    return TEE_SUCCESS;
}
