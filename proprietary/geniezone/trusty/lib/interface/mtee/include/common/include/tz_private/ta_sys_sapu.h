/*
* Copyright (c) 2015 MediaTek Inc.
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

/** Commands for TA memory **/

#ifndef __TRUSTZONE_TA_SYS_SAPU__
#define __TRUSTZONE_TA_SYS_SAPU__

#include "tz_cross/gz_version.h"


/**
 * Acquire sAPU MTEE shared memory
 * Get sAPU MTEE shared memory
 *
 * @param size		put sAPU TEE shared memory size
 * @return    		return sAPU MTEE shared memory va in HA space
 */
void *SAPU_AcquireSapuMteeShm(u32 *size);


/**
 * Release sAPU MTEE shared memory
 *
 * @param ptr		get va by use SAPU_AcquireSapuMteeShm
 * @param size		get size by use SAPU_AcquireSapuMteeShm
 * @return    		0 is success. others is fail
 */
int SAPU_ReleaseSapuMteeShm(void **ptr, u32 *size);


/**
 * Run sDSP operation
 *
 * @param sdsp1_buf_offset		DSP1 buf offset from the head of sAPU TEE shared memory
 * @param sdsp1_buf_num			DSP1 buf num
 * @param sdsp1_prop_offset		DSP1 prop offset from the head of sAPU TEE shared memory
 * @param sdsp1_prop_size		DSP1 prop size
 * @param sdsp2_buf_offset		DSP2 buf offset from the head of sAPU TEE shared memory
 * @param sdsp2_buf_num			DSP2 buf num
 * @param sdsp2_prop_offset		DSP2 prop offset from the head of sAPU TEE shared memory
 * @param sdsp2_prop_size		DSP2 prop size
 * @param reserved				reserved
 * @param sw_op_enabled			false is for sDSP, true  is for SW_OP
 * @param reg_info				reg_info for HA issue/get vpu cmd/result
 * @return    					0 is success. others is fail
 */
int SAPU_SDSP_Run(	u32 sdsp1_buf_offset, u32 sdsp1_buf_num, u32 sdsp1_prop_offset, u32 sdsp1_prop_size,
					u32 sdsp2_buf_offset, u32 sdsp2_buf_num, u32 sdsp2_prop_offset, u32 sdsp2_prop_size,
					u32 reserved, bool sw_op_enabled, sapu_reg_info_t *reg_info);


/**
 * Run sDSP operation V2
 *
 * @param sdsp1_buf_num			DSP1 buf num
 * @param sdsp1_prop_size		DSP1 prop size
 * @param sdsp2_buf_num			DSP2 buf num
 * @param sdsp2_prop_size		DSP2 prop size
 * @param sdsp3_buf_num			DSP3 buf num
 * @param sdsp3_prop_size		DSP3 prop size
 * @param sdsp4_buf_num			DSP4 buf num
 * @param sdsp4_prop_size		DSP4 prop size
 * @param reserved				reserved
 * @param sw_op_enabled			false is for sDSP, true  is for SW_OP
 * @param reg_v2_info			reg_info for HA issue/get vpu cmd/result
 * @return    					0 is success. others is fail
 */
int SAPU_v2_SDSP_Run(	u32 sdsp1_buf_num, u32 sdsp1_prop_size, u32 sdsp2_buf_num, u32 sdsp2_prop_size,
						u32 sdsp3_buf_num, u32 sdsp3_prop_size, u32 sdsp4_buf_num, u32 sdsp4_prop_size,
						u32 reserved, bool sw_op_enabled, sapu_reg_v2_info_t *reg_v2_info);

/**
 * Run sDSP1 operation
 *
 * @param sdsp1_buf_offset		DSP1 buf offset from the head of sAPU TEE shared memory
 * @param sdsp1_buf_num			DSP1 buf num
 * @param sdsp1_prop_offset		DSP1 prop offset from the head of sAPU TEE shared memory
 * @param sdsp1_prop_size		DSP1 prop size
 * @param reserved				reserved
 * @param reg_info				reg_info for HA issue/get vpu cmd/result
 * @return    					0 is success. others is fail
 */
int SAPU_SDSP_Core1_Run(u32 sdsp1_buf_offset, u32 sdsp1_buf_num, u32 sdsp1_prop_offset, u32 sdsp1_prop_size,
						u32 reserved, sapu_reg_info_t *reg_info);

/**
 * Run sDSP2 operation
 *
 * @param sdsp1_buf_offset		DSP1 buf offset from the head of sAPU TEE shared memory
 * @param sdsp1_buf_num			DSP1 buf num
 * @param sdsp1_prop_offset		DSP1 prop offset from the head of sAPU TEE shared memory
 * @param sdsp1_prop_size		DSP1 prop size
 * @param reserved				reserved
 * @param reg_info				reg_info for HA issue/get vpu cmd/result
 * @return    					0 is success. others is fail
 */
int SAPU_SDSP_Core2_Run(u32 sdsp2_buf_offset, u32 sdsp2_buf_num, u32 sdsp2_prop_offset, u32 sdsp2_prop_size,
						u32 reserved, sapu_reg_info_t *reg_info);

/**
 * sAPU log dump
 *
 * @param mask					how many VPU log need dump, VPU1(1<<0), VPU2(1<<1), VPU3(1<<2)....
 * @return                      0 is success. others is fail
 */
int SAPU_Log_Dump(u32 mask);



/**
 * sAPU VPU BOOTUP
 *
 * @return                      0 is success. others is fail
 */
int SAPU_SDSP_BOOTUP(void);

/**
 * map sdsp1_buf & sdsp1_prop_offset to HA
 * @param sdsp1_buf_offset		DSP1 buf offset from the head of sAPU TEE shared memory
 * @param sdsp1_buf_num			DSP1 buf num
 * @param sdsp1_prop_offset		DSP1 prop offset from the head of sAPU TEE shared memory
 * @param sdsp1_prop_size		DSP1 prop size
 *
 * *VA1 = sdsp1_buf VA
 * *VA2 = sdsp1_prop VA
 * *basePA = shm PA
 */
int SAPU_GetTeeMemInfo(uint32_t *VA1, uint32_t *VA2, u32 sdsp1_buf_offset, u32 sdsp1_buf_num, u32 sdsp1_prop_offset, u32 sdsp1_prop_size, uint64_t *basePA);


/**
 * Acquire sAPU TEE shared memory
 * Get sAPU TEE shared memory
 *
 * @param size		put sAPU TEE shared memory size
 * @return    		return sAPU TEE shared memory va in HA space
 */
void *SAPU_AcquireSapuTeeShm(u32 *size);


/**
 * Release sAPU TEE shared memory
 *
 * @param ptr		get va by use SAPU_AcquireSapuTeeShm
 * @param size		get size by use SAPU_AcquireSapuTeeShm
 * @return    		0 is success. others is fail
 */
int SAPU_ReleaseSapuTeeShm(void **ptr, u32 *size);

/*get the offset of ptr(TA PA) base on TEE-MTEE share mem PA*/
uint64_t SAPU_vDSP_Shm_offset(uint64_t ptr);

#endif /* __TRUSTZONE_TA_SYSMEM__ */

