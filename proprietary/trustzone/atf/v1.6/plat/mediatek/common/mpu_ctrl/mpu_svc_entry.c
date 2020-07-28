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

#include <mpu_ctrl/mpu_api.h>
#include <mpu_ctrl/mpu_def.h>
#include <mpu_ctrl/mpu_error.h>

#define LOG_TAG "mpu_svc"
#include <mpu_ctrl/mpu_dbg.h>

#include <plat_mpu_zone.h>

#define SIZE_64K 0x00010000

/* Cut lower 16-bits for 64KB alignment.
 * So that we can use 32-bit variable to carry 48-bit physical address range.
 */
#define MPU_PHYSICAL_ADDR_SHIFT_BITS (16)

static inline uint64_t get_decoded_phys_addr(uint32_t addr)
{
	return (((uint64_t)addr) << MPU_PHYSICAL_ADDR_SHIFT_BITS);
}

static inline uint32_t get_decoded_zone_id(uint32_t info)
{
	return ((info & 0xFFFF0000) >> 16);
}

static inline uint32_t get_decoded_ops(uint32_t info)
{
	return (info & 0x0000FFFF);
}

static inline bool is_set_request(uint32_t info)
{
	if (get_decoded_ops(info) == 0)
		return false;
	return true;
}

static inline bool is_valid_size(uint32_t size)
{
	if (size == 0)
		return false;
	if (size < SIZE_64K)
		return false;
	if ((size % SIZE_64K) != 0)
		return false;
	return true;
}

static struct mpu_origin_zone_entry *
get_mpu_zone_entry(enum MPU_REQ_ORIGIN origin,
		   enum MPU_REQ_ORIGIN_ZONE_ID zone_id)
{
	struct mpu_origin_zone_entry *zone_table;
	uint32_t entry_sz, entry_idx;

	MPU_TRACE_FN();

	switch (origin) {
	case MPU_REQ_ORIGIN_GZ:
		zone_table = &mpu_origin_zone_gz_mappings[0];
		entry_sz = MPU_ORIGIN_ZONE_GZ_ENTRY_SIZE;
		break;
	case MPU_REQ_ORIGIN_TEE:
		zone_table = &mpu_origin_zone_tee_mappings[0];
		entry_sz = MPU_ORIGIN_ZONE_TEE_ENTRY_SIZE;
		break;
	case MPU_REQ_ORIGIN_ATF:
		zone_table = &mpu_origin_zone_atf_mappings[0];
		entry_sz = MPU_ORIGIN_ZONE_ATF_ENTRY_SIZE;
		break;
	default:
		MPU_ERR_FN("unknown origin=%d\n", origin);
		return NULL;
	}

	for (entry_idx = 0; entry_idx < entry_sz; entry_idx++) {
		if (zone_table[entry_idx].zone_id == zone_id)
			return &zone_table[entry_idx];
	}

	MPU_DBG("zone entry not found, zid=%d, ori=%d\n", zone_id, origin);
	return NULL;
}

static uint64_t sip_origin_mpu_feat_request(enum MPU_REQ_ORIGIN origin,
					    uint64_t phys_addr, uint64_t size,
					    uint32_t zone_id,
					    enum MPU_FEAT_ID feat_id,
					    bool req_set, bool req_lock)
{
	struct mpu_origin_zone_entry *zone_entry =
		get_mpu_zone_entry(origin, zone_id);

	MPU_TRACE_FN();
	MPU_DBG_ENG(
		"origin=0x%x phys=0x%llx, sz=0x%llx, zinf=0x%x, req_set=%d\n",
		origin, phys_addr, size, zone_id, req_set);

	if (!is_valid_size(size)) {
		MPU_ERR("origin=0x%x invalid zone size=0x%llx\n", origin, size);
		return MPU_ERROR_SVC_INVALID_ZONE_SIZE;
	}

	if (zone_entry == NULL) {
		MPU_ERR("origin=0x%x entry is not found, id=%d\n", origin,
			zone_id);
		return MPU_ERROR_SVC_ZONE_ENTRY_NOT_FOUND;
	}

	if (zone_entry->active == false) {
		MPU_ERR("origin=0x%x entry is not active, zid=%d, uid=%d\n",
			origin, zone_id, zone_entry->unique_id);
		return MPU_ERROR_SVC_ZONE_ENTRY_NOT_ACTIVE;
	}

	return sip_mpu_request(zone_entry->unique_id, origin, feat_id,
			       phys_addr, size, req_set, req_lock);
}

static uint64_t sip_origin_mpu_request(enum MPU_REQ_ORIGIN origin,
				       uint32_t addr, uint32_t size,
				       uint32_t zone_info)
{
	uint64_t phys_addr = get_decoded_phys_addr(addr);
	bool req_set = is_set_request(zone_info);
	bool req_lock = false;
	enum MPU_REQ_ORIGIN_ZONE_ID zone_id = get_decoded_zone_id(zone_info);

	return sip_origin_mpu_feat_request(origin, phys_addr, size, zone_id,
					   MPU_FEAT_INVALID, req_set, req_lock);
}

uint64_t sip_tee_mpu_request(uint32_t addr, uint32_t size, uint32_t zone_info)
{
	return sip_origin_mpu_request(MPU_REQ_ORIGIN_TEE, addr, size,
				      zone_info);
}

uint64_t sip_gz_mpu_request(uint32_t addr, uint32_t size, uint32_t zone_info)
{
	return sip_origin_mpu_request(MPU_REQ_ORIGIN_GZ, addr, size, zone_info);
}

uint64_t sip_atf_mpu_request(uint64_t addr, uint64_t size, uint32_t zone_id,
			     bool req_set, bool req_lock)
{
	return sip_origin_mpu_feat_request(MPU_REQ_ORIGIN_ATF, addr, size,
					   zone_id, MPU_FEAT_INVALID, req_set,
					   req_lock);
}

uint64_t sip_feat_mpu_request(enum MPU_REQ_ORIGIN_ZONE_ID zone_id,
			      enum MPU_FEAT_ID feat_id, uint64_t addr,
			      uint64_t size)
{
	bool req_set = true;
	bool req_lock = false;

	return sip_origin_mpu_feat_request(MPU_REQ_ORIGIN_ATF, addr, size,
					   zone_id, feat_id, req_set, req_lock);
}
