/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __TEEI_PRIVATE_H__
#define __TEEI_PRIVATE_H__

#include <context.h>
#include <arch.h>
#include <psci.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>

/*******************************************************************************
 * Secure Payload PM state information e.g. SP is suspended, uninitialised etc
 ******************************************************************************/
#define TEEI_STATE_OFF      0
#define TEEI_STATE_ON       1
#define TEEI_STATE_SUSPEND  2

/*******************************************************************************
 * Secure Payload execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define TEEI_AARCH32        MODE_RW_32
#define TEEI_AARCH64        MODE_RW_64

/*******************************************************************************
 * The SPD should know the type of Secure Payload.
 ******************************************************************************/
#define TEEI_TYPE_UP        PSCI_TOS_NOT_UP_MIG_CAP
#define TEEI_TYPE_UPM       PSCI_TOS_UP_MIG_CAP
#define TEEI_TYPE_MP        PSCI_TOS_NOT_PRESENT_MP

/*******************************************************************************
 * Secure Payload migrate type information as known to the SPD. We assume that
 * the SPD is dealing with an MP Secure Payload.
 ******************************************************************************/
#define TEEI_MIGRATE_INFO       TEEI_TYPE_MP

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define TEEI_CORE_COUNT     PLATFORM_CORE_COUNT
#define TEEI_PM_ENABLE          1
/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define TSPD_C_RT_CTX_X19       0x0
#define TSPD_C_RT_CTX_X20       0x8
#define TSPD_C_RT_CTX_X21       0x10
#define TSPD_C_RT_CTX_X22       0x18
#define TSPD_C_RT_CTX_X23       0x20
#define TSPD_C_RT_CTX_X24       0x28
#define TSPD_C_RT_CTX_X25       0x30
#define TSPD_C_RT_CTX_X26       0x38
#define TSPD_C_RT_CTX_X27       0x40
#define TSPD_C_RT_CTX_X28       0x48
#define TSPD_C_RT_CTX_X29       0x50
#define TSPD_C_RT_CTX_X30       0x58
#define TSPD_C_RT_CTX_SIZE      0x60
#define TSPD_C_RT_CTX_ENTRIES       (TSPD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, TSPD_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(sizeof(c_rt_regs_t) == TSPD_C_RT_CTX_SIZE, assert_spd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'state'    - collection of flags to track SP state e.g. on/off
 * 'mpidr'    - mpidr to associate a context with a cpu
 * 'c_rt_ctx' - stack address to restore C runtime context from after returning
 *              from a synchronous entry into the SP.
 * 'cpu_ctx'  - space to maintain SP architectural state
 ******************************************************************************/

struct teei_context {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
} __aligned(PLATFORM_CACHE_LINE_SIZE);

#define KEY_LEN 32

enum device_type {
	MT_UNUSED = 0,
	MT_UART16550 = 1,
	MT_SEC_GPT,
	MT_SEC_WDT,
	MT_SEC_CRYPTO,
};
struct tee_dev_t {
	uint32_t dev_type;
	uint64_t base_addr;
	uint32_t intr_num;
	uint32_t apc_num;
	uint32_t param[3];
} __packed;

struct  tee_arg_t {
	unsigned int magic;
	unsigned int length;
	unsigned long long version;

	unsigned long long secDRamBase;
	unsigned long long secDRamSize;
	unsigned long long gic_distributor_base;
	unsigned long long gic_cpuinterface_base;
	unsigned int gic_version;

	unsigned int total_number_spi;
	unsigned int ssiq_number[5];
	struct tee_dev_t tee_dev[5];

	/*NOTICE: pay attention to big/little endian when manupulate flowing flag
	 *
	 *    +-+-+-+-+-+-+-+-+
	 * 0x7|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x6|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x5|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x4|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x3|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x2|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x1|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 * 0x0|7|6|5|4|3|2|1|0|
	 *    +-+-+-+-+-+-+-+-+
	 *
	 *byte 0x0:
	 *          bit 1,0 represent log path;
	 *          bit 4,3,2 represent log level;
	 *          bit 5 represent full or mini version of soter ;
	 *          bit 6 represent verify method(public key or rpmb key);
	 *byte left:
	 *          reserved and not used in version 0x00010001U
	 *
	 */
	unsigned long long flags;
#define flags_mask_verify_mode   0x00000000000000C0
#define flags_mask_log_level     0x000000000000001c
#define flags_mask_log_patch     0x0000000000000003
#define flags_mask_soter_type    0x0000000000000020
} __packed;

struct tee_keys_t {
	uint32_t  magic;        /* 0x434d4254*/
	uint32_t  version;        /* VERSION*/
	uint8_t rpmb_key[KEY_LEN]; /* RPMB*/
	uint8_t hw_id[KEY_LEN];
};

enum {
	TEEI_BOOT,
	TEEI_KERNEL_READY,
	TEEI_BUF_READY,
	TEEI_SERVICE_READY,
	TEEI_ALL_READY,
};

#define		SCR_EL3_S_AARCH64     (SCR_SIF_BIT | SCR_HCE_BIT | SCR_RES1_BITS | SCR_RW_BIT)
#define		SCR_EL3_S_IRQ_AARCH64		(SCR_SIF_BIT | SCR_HCE_BIT | SCR_RES1_BITS | SCR_IRQ_BIT | SCR_RW_BIT)
#define		SCR_EL3_S_AARCH32     (SCR_SIF_BIT | SCR_HCE_BIT | SCR_RES1_BITS) /* | SCR_IRQ_BIT)*/
#define		SCR_EL3_S_IRQ_AARCH32		(SCR_SIF_BIT | SCR_HCE_BIT | SCR_RES1_BITS | SCR_IRQ_BIT)
#define		SCR_EL3_NS      (SCR_SIF_BIT | SCR_HCE_BIT | SCR_RES1_BITS | SCR_NS_BIT | SCR_RW_BIT) /*| SCR_FIQ_BIT)*/
#define		SCR_EL3_NS_FIQ     (SCR_SIF_BIT | SCR_HCE_BIT | SCR_RES1_BITS | SCR_NS_BIT | SCR_RW_BIT | SCR_FIQ_BIT)

/* Magic for interface*/
#define TEEI_BOOTCFG_MAGIC (0x434d4254) /* String TBMC in little-endian*/

/*******************************************************************************
 * TEEI specific SMC ids
 ******************************************************************************/

/*This field id is fixed by arm*/
#define ID_FIELD_F_FAST_SMC_CALL            1
#define ID_FIELD_F_STANDARD_SMC_CALL        0
#define ID_FIELD_W_64                       1
#define ID_FIELD_W_32                       0
#define ID_FIELD_T_ARM_SERVICE             0
#define ID_FIELD_T_CPU_SERVICE              1
#define ID_FIELD_T_SIP_SERVICE                2
#define ID_FIELD_T_OEM_SERVICE            3
#define ID_FIELD_T_STANDARD_SERVICE          4

/*TA Call 48-49*/
#define ID_FIELD_T_TA_SERVICE0              48
#define ID_FIELD_T_TA_SERVICE1              49
/*TOS Call 50-63*/
#define ID_FIELD_T_TRUSTED_OS_SERVICE0      50
#define ID_FIELD_T_TRUSTED_OS_SERVICE1      51

#define ID_FIELD_T_TRUSTED_OS_SERVICE2      52
#define ID_FIELD_T_TRUSTED_OS_SERVICE3      53

#define MAKE_SMC_CALL_ID(F, W, T, FN) (((F)<<31)|((W)<<30)|((T)<<24)|(FN))

#define SMC_CALL_RTC_OK                 0x0
#define SMC_CALL_RTC_UNKNOWN_FUN        0xFFFFFFFF
#define SMC_CALL_RTC_MONITOR_NOT_READY  0xFFFFFFFE


/*For t side  Fast Call*/
#define T_BOOT_NT_OS            \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32,  ID_FIELD_T_TRUSTED_OS_SERVICE0, 0)
#define T_ACK_N_OS_READY    \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32,  ID_FIELD_T_TRUSTED_OS_SERVICE0, 1)
/*
 *#define T_GET_PARAM_IN        \
 *		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32,  ID_FIELD_T_TRUSTED_OS_SERVICE0, 2)
 *#define T_ACK_T_OS_FOREGROUND    \
 *		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 3)
 *#define T_ACK_T_OS_BACKSTAGE       \
 *		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 4)
 */
#define T_ACK_N_FAST_CALL    \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 5)
#define T_DUMP_STATE       \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 6)
#define T_ACK_N_INIT_FC_BUF   \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 7)
#define T_GET_BOOT_PARAMS      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 8)
#define T_WDT_FIQ_DUMP      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 9)
#define T_VUART_LOG_CALL      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 10)
#define T_SET_TUI_EINT      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 11)
#define T_CLR_TUI_EINT      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 12)
#define T_SET_DRM_INT      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 13)
#define T_CLR_DRM_INT		\
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 14)
#define T_BOOT_TA_MODE		\
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 17)
#define T_GET_RAND_SEED		\
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 50)
#define T_SW_GENERATE_INT                                                                          \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 18)
#define T_SET_DRM_PPI      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 19)
#define T_CLR_DRM_PPI		\
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE0, 20)

/*For t side  Standard Call*/
#define T_SCHED_NT              \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 0)
#define T_ACK_N_SYS_CTL     \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 1)
#define T_ACK_N_NQ              \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 2)
#define T_ACK_N_INVOKE_DRV  \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 3)
#define T_INVOKE_N_DRV      \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 4)
/*
 *#define T_RAISE_N_EVENT     \
 *		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 5)
 */
#define T_ACK_N_BOOT_OK     \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 6)
#define T_INVOKE_N_LOAD_IMG \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 7)
#define T_ACK_N_KERNEL_OK               \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 8)
#define T_SCHED_NT_IRQ              \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 9)
#define T_NOTIFY_N_ERR             \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 10)
#define T_SCHED_NT_LOG              \
		MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE1, 11)

/*For nt side Fast Call*/
#define N_SWITCH_TO_T_OS_STAGE2   \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 0)
#define N_GET_PARAM_IN                                                                             \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 1)
#define N_INIT_T_FC_BUF                                                                            \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 2)
#define N_INVOKE_T_FAST_CALL                                                                       \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 3)
#define N_GET_T_FP_DEVICE_ID \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_64, ID_FIELD_T_TRUSTED_OS_SERVICE2, 11)

/*
 *#define NT_DUMP_STATE_32       \
 *MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_64, ID_FIELD_T_TRUSTED_OS_SERVICE2, 4)
 *#define N_ACK_N_FOREGROUND_32   \
 *MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_64, ID_FIELD_T_TRUSTED_OS_SERVICE2, 5)
 *#define N_ACK_N_BACKSTAGE_32     \
 *MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_64, ID_FIELD_T_TRUSTED_OS_SERVICE2, 6)
 */
#define N_INIT_T_BOOT_STAGE1                                                                       \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 7)
#define N_SWITCH_CORE                                                                              \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 8)
#define N_GET_NON_IRQ_NUM                                                                          \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 9)
#define N_GET_SE_OS_STATE                                                                          \
	MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 10)
#define N_GET_T_FP_DEVICE_ID_32 \
		MAKE_SMC_CALL_ID(ID_FIELD_F_FAST_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE2, 11)

/*For nt side Standard Call*/
#define NT_SCHED_T                                                                                 \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 0)
#define N_INVOKE_T_SYS_CTL                                                                         \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 1)
#define N_INVOKE_T_NQ                                                                              \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 2)
#define N_INVOKE_T_DRV                                                                             \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 3)
/*
 *#define N_RAISE_T_EVENT_32  \
 *MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 4)
 */
#define N_ACK_T_INVOKE_DRV                                                                         \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 5)
#define N_INVOKE_T_LOAD_TEE                                                                        \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 6)
#define N_ACK_T_LOAD_IMG                                                                           \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 7)
#define NT_CANCEL_T_TUI                                                                            \
	MAKE_SMC_CALL_ID(ID_FIELD_F_STANDARD_SMC_CALL, ID_FIELD_W_32, ID_FIELD_T_TRUSTED_OS_SERVICE3, 9)

/* MSM area definition
 * This structure is SPD owned area mapped as part of MSM
 */
struct msm_area_t {
	struct teei_context secure_context[TEEI_CORE_COUNT];
};

extern struct msm_area_t msm_area;

/* Context for each core. gp registers not used by SPD.*/
extern struct teei_context *secure_context;

/* teei power management handlers */
extern const spd_pm_ops_t teei_pm;
extern uint64_t teeiBootCoreMpidr;

extern uint32_t uart_apc_num;
extern uint32_t TEEI_STATE;
/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
extern void teei_setup_entry(cpu_context_t *ns_context, uint32_t call_offset, uint32_t regfileNro);
extern uint64_t teei_enter_sp(uint64_t *c_rt_ctx);
extern void __dead2 teei_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
extern uint64_t teei_synchronous_sp_entry(struct teei_context *tsp_ctx);
extern void __dead2 teei_synchronous_sp_exit(struct teei_context *teei_ctx, uint64_t ret, uint32_t save_sysregs);
extern uint32_t maskSWdRegister(uint64_t x);
extern int32_t teei_fastcall_setup(void);
/*extern void dump_state_and_die(void);*/
extern void teei_register_fiq_handler(void);
extern const char *teei_smc_call_id_to_string(uint32_t id);
extern uint64_t teei_fiq_handler(uint32_t id, uint32_t flags, void *handle, void *cookie);
extern int32_t teei_init_secure_context(struct teei_context *teei_ctx);
extern void irq_raise_softirq(unsigned int map, unsigned int irq);
extern void gicd_v3_do_wait_for_rwp(unsigned int gicd_base);

extern uint64_t teei_irq_handler(uint32_t id,
		uint32_t flags,
		void *handle,
		void *cookie);

/*fiq is handled by S-EL1*/
extern uint64_t teei_fiq_handler(uint32_t id,
		uint32_t flags,
		void *handle,
		void *cookie);

#endif /*__ASSEMBLY__*/

#endif /* __TEEI_PRIVATE_H__ */
