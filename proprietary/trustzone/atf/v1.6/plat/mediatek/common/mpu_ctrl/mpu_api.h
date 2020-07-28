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

#ifndef __MPU_API_H__
#define __MPU_API_H__

#include <stdint.h>
#include <mtk_plat_common.h>
#include <mpu_ctrl/mpu_def.h>

uint64_t mpu_init(void);

void mpu_mblock_cfg(struct boot_tag_mem *mem);

uint64_t sip_tee_mpu_request(uint32_t addr, uint32_t size, uint32_t zone_info);
uint64_t sip_gz_mpu_request(uint32_t addr, uint32_t size, uint32_t zone_info);
uint64_t sip_atf_mpu_request(uint64_t addr, uint64_t size, uint32_t zone_id,
			     bool req_set, bool req_lock);
uint64_t sip_feat_mpu_request(enum MPU_REQ_ORIGIN_ZONE_ID zone_id,
			      enum MPU_FEAT_ID feat_id, uint64_t addr,
			      uint64_t size);
uint64_t sip_mpu_request(enum MPU_UNIQUE_ID unique_id,
			 enum MPU_REQ_ORIGIN origin, enum MPU_FEAT_ID feat_id,
			 uint64_t addr, uint64_t size, bool is_set,
			 bool is_lock);
uint64_t sip_mpu_request_ree_perm_check(uint32_t phy_id);

#endif /* __MPU_API_H__ */
