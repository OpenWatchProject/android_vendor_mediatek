#include <string.h>
#include <mtk_plat_common.h>
#include <mpu_ctrl/mpu_def.h>
#include <mpu_ctrl/mpu_api.h>
#include <mpu_ctrl/mpu_mblock.h>
#include <debug.h>

#if defined(MTK_DEVMPU_SUPPORT)
#include <devmpu.h>
#define ALIGN_MASK_DEVMPU ((0x1ULL << DEVMPU_ALIGN_BITS) - 1)
#endif

#define UNUSED(x) (void)(x)

/* clang-format off */
#define MPU_MBLOCK_ATF		"atf-reserved"
#define MPU_MBLOCK_TEE		"tee-reserved"
#define MPU_MBLOCK_GZ		"gz"
#define MPU_MBLOCK_SDSP_S	"sapu-mtee-shm"
#define MPU_MBLOCK_GZTEE_S	"gz-tee-static-shm"
/* clang-format on */

static inline uint32_t get_mpu_atf_zone_id(struct reserved *mem)
{
	uint64_t pa = mem->start;
	uint64_t sz = mem->size;
	char *name = mem->name;

	/*
	 * as ATF might be protected by EMI MPU / DeviceMPU
	 * with 64KB/2MB alignment, the zone id is returned
	 * accordingly.
	 */
	if (strcmp(MPU_MBLOCK_ATF, name) == 0) {
#if defined(MTK_DEVMPU_SUPPORT)
		if (((pa & ALIGN_MASK_DEVMPU) == 0)
		&&  ((sz & ALIGN_MASK_DEVMPU) == 0))
			return MPU_REQ_ORIGIN_ATF_ZONE_ATF_DEVMPU;
#else
		UNUSED(pa);
		UNUSED(sz);
#endif
		return MPU_REQ_ORIGIN_ATF_ZONE_ATF_EMIMPU;
	}

	return
		(strcmp(MPU_MBLOCK_TEE,  name) == 0) ? MPU_REQ_ORIGIN_ATF_ZONE_TEE :
		(strcmp(MPU_MBLOCK_GZ,   name) == 0) ? MPU_REQ_ORIGIN_ATF_ZONE_GZ  :
		(strcmp(MPU_MBLOCK_SDSP_S,   name) == 0) ?
		MPU_REQ_ORIGIN_ATF_ZONE_SDSP_SHARED_VPU_TEE :
		(strcmp(MPU_MBLOCK_GZTEE_S,   name) == 0) ?
		MPU_REQ_ORIGIN_ATF_ZONE_MTEE_TEE_STATIC_SHARED :
		MPU_REQ_ORIGIN_ZONE_INVALID;
}

void mpu_mblock_cfg(struct boot_tag_mem *mem)
{
	uint64_t rc;

	uint32_t i;
	uint32_t atf_zone_id;

	struct mblock_info *mblock_info;
	struct reserved reserved_mem;

	uint64_t mem_pa;
	uint64_t mem_sz;

	if (mem == NULL) {
		ERROR("memory information is NULL\n");
		return;
	}

	mblock_info = &mem->mblock_info;
	if (mblock_info->mblock_version != MPU_MBLOCK_VERSION) {
		ERROR("unsupported mblock version %u, expect version %u\n",
				mblock_info->mblock_version, MPU_MBLOCK_VERSION);
		return;
	}

	for (i = 0; i < mblock_info->reserved_num; ++i) {

		/* prevent alignment fault */
		memcpy(&reserved_mem, &mblock_info->reserved_mem[i],
				sizeof(reserved_mem));

		atf_zone_id = get_mpu_atf_zone_id(&reserved_mem);
		if (atf_zone_id == MPU_REQ_ORIGIN_ZONE_INVALID)
			continue;

		/*
		 * as ATF log buffer is concated with TEE memory, the
		 * log buffer range should be excluded to avoid being
		 * restricted to secure world access only. The log is
		 * desirable to be accessible for NS world
		 */
		mem_pa = reserved_mem.start;
		mem_sz = reserved_mem.size;

		rc = sip_atf_mpu_request(mem_pa, mem_sz, atf_zone_id, true, true);
		if (rc) {
			ERROR("failed to config MPU, ATF zone=%u, pa=0x%lx, sz=0x%lx, rc=0x%lx\n",
					atf_zone_id, mem_pa, mem_sz, rc);
			continue;
		}
	}
}
