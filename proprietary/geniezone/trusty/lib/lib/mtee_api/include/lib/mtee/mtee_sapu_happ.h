#ifndef __MTEE_SAPU_HAPP_H
#define __MTEE_SAPU_HAPP_H
#include <sys/types.h>
#include <tz_private/mtee_sys_fd.h>
__BEGIN_CDECLS

void Sapu_TEE_DSP_Shm_Store(uint64_t pa, uint32_t size);
void Sapu_TEE_DSP_Shm_Static(uint64_t pa, uint32_t size);
void Sapu_TEE_DSP_Shm_Release(void);

#define SAPU_LOG_SHM_SIZE	0x10000	//64K

#if CFG_GZ_SECURE_DSP_WITH_M4U
/*
 *total (vpu0 elf + vpu1 elf) max 16M
 *in each vpu elf's final 64K as log buf mva
 */
#define SDSP_VPU0_ELF_MVA		0x3c000000	//max 64M
#define SDSP_VPU0_DTA_MVA		0x38000000	//max 64M
#define TEE_VPU_SHM_MVA			SDSP_VPU0_DTA_MVA
#endif

/* Command for system */
typedef enum {
	MTEE_SAPU_ACQUIRE_SAPU_MTEE_SHM	= 0x1,
	MTEE_SAPU_RELEASE_SAPU_MTEE_SHM	= 0x2,
	MTEE_SAPU_SDSP_RUN				= 0x3,
	MTEE_SAPU_LOG_DUMP				= 0x4,
	MTEE_SAPU_BOOTUP				= 0x5,
	MTEE_SAPU_GET_TEE_MEM_INFO		= 0x6,
	MTEE_SAPU_ACQUIRE_SAPU_TEE_SHM	= 0x7,
	MTEE_SAPU_RELEASE_SAPU_TEE_SHM	= 0x8,
	MTEE_SAPU_SDSP_CORE1_RUN		= 0x9,
	MTEE_SAPU_SDSP_CORE2_RUN		= 0xA,
	MTEE_SAPU_vDSP_SHM_OFFSET		= 0xB,
} MTEE_SAPU_CMD;

typedef struct {
	uint32_t data;
	uint32_t size;
} sapu_mtee_shm_t;

typedef struct {
	uint32_t data;
	uint32_t size;
} sapu_tee_shm_t;

typedef struct {
	uint32_t sdsp1_handle;	//reserved
	uint32_t sdsp1_cmd;		// use INFO17
	uint32_t sdsp1_result;	// use INFO18
	uint32_t sdsp2_handle;	//reserved
	uint32_t sdsp2_cmd;
	uint32_t sdsp2_result;
} sapu_reg_info_t ;

typedef struct {
	uint32_t sdsp1_handle;	// reserved
	uint32_t sdsp1_cmd;		// use INFO17
	uint32_t sdsp1_result;	// use INFO18
	uint32_t sdsp2_handle;	//
	uint32_t sdsp2_cmd;
	uint32_t sdsp2_result;
	uint32_t sdsp3_handle;	// reserved
	uint32_t sdsp3_cmd;		// use INFO17
	uint32_t sdsp3_result;	// use INFO18
	uint32_t sdsp4_handle;	// reserved
	uint32_t sdsp4_cmd;
	uint32_t sdsp4_result;
} sapu_reg_v2_info_t ;

typedef struct {
	uint32_t sdsp1_buf_num;
	uint32_t sdsp1_prop_size;
	uint32_t sdsp2_buf_num;
	uint32_t sdsp2_prop_size;
	uint32_t sdsp3_buf_num;
	uint32_t sdsp3_prop_size;
	uint32_t sdsp4_buf_num;
	uint32_t sdsp4_prop_size;
	uint32_t sdsp1_handle;
	uint32_t sdsp1_reg_cmd;
	uint32_t sdsp1_reg_result;
	uint32_t sdsp2_handle;
	uint32_t sdsp2_reg_cmd;
	uint32_t sdsp2_reg_result;
	uint32_t sdsp3_handle;
	uint32_t sdsp3_reg_cmd;
	uint32_t sdsp3_reg_result;
	uint32_t sdsp4_handle;
	uint32_t sdsp4_reg_cmd;
	uint32_t sdsp4_reg_result;
	uint32_t reserved;
	uint64_t basePA;
	bool	 sw_op_enabled;
} sapu_cmd_area_info_t;

typedef uint8_t vpu_id_t;

struct vpu_plane {
	uint32_t stride;         /* if buffer type is image */
	uint32_t length;
	uint64_t ptr;            /* mva which is accessible by VPU */
};

typedef struct {
	vpu_id_t port_id;
	uint8_t format;
	uint8_t plane_count;
	uint32_t width;
	uint32_t height;
	struct vpu_plane planes[3];
} vpu_buffer_t;

typedef struct {
	uint32_t cmd;
	uint32_t result;
	uint32_t count;
} vpu_prop_t;

__END_CDECLS

#endif

