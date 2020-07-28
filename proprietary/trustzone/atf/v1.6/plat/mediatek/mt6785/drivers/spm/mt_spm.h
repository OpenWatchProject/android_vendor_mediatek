#ifndef __MT_SPM_H__
#define __MT_SPM_H__

#include <stdio.h>
#include <stdint.h>

#define true 1
#define false 0

#define SPM_FW_NO_RESUME 1
#define MCUSYS_MTCMOS_ON 0
#define WAKEUP_LOG_ON	 0

extern uint32_t __spmfw_idx;
extern uint32_t is_ext_buck;
extern uint32_t __ddr_max_freq;

/* EN SPM INFRA DEBUG OUT */
#define DEBUGSYS_DEBUG_EN_REG (DBGSYS_DEM_BASE + 0x94)

/* INFRA_AO_DEBUG_CON */
#define INFRA_AO_DBG_CON0 (INFRACFG_AO_BASE + 0x500)
#define INFRA_AO_DBG_CON1 (INFRACFG_AO_BASE + 0x504)
#define INFRA_AO_DBG_CON2 (INFRACFG_AO_BASE + 0x508)
#define INFRA_AO_DBG_CON3 (INFRACFG_AO_BASE + 0x50C)

/* Setting the SPM settle time*/
#define SPM_SYSCLK_SETTLE       0x34	/* 3ms */

enum {
	SPM_FIRMWARE_STATUS_NOT_LOADED = 0,
	SPM_FIRMWARE_STATUS_LOADED,
	SPM_FIRMWARE_STATUS_LOADED_KICKED,
};

enum {
	SPM_ARGS_SPMFW_IDX_KICK = 0,
	SPM_ARGS_SPMFW_INIT,
	SPM_ARGS_SUSPEND,
	SPM_ARGS_SUSPEND_FINISH,
	SPM_ARGS_SODI,
	SPM_ARGS_SODI_FINISH,
	SPM_ARGS_DPIDLE,
	SPM_ARGS_DPIDLE_FINISH,
	SPM_ARGS_PCM_WDT,
	SPM_ARGS_SUSPEND_CALLBACK,
	SPM_ARGS_IDLE_DRAM,
	SPM_ARGS_IDLE_DRAM_FINISH,
	SPM_ARGS_IDLE_SYSPLL,
	SPM_ARGS_IDLE_SYSPLL_FINISH,
	SPM_ARGS_IDLE_BUS26M,
	SPM_ARGS_IDLE_BUS26M_FINISH,
	SPM_ARGS_NUM,
};

typedef enum {
	WR_NONE = 0,
	WR_UART_BUSY = 1,
	WR_ABORT = 2,
	WR_PCM_TIMER = 3,
	WR_WAKE_SRC = 4,
	WR_DVFSRC = 5,
	WR_TWAM = 6,
	WR_PMSR = 7,
	WR_SPM_ACK_CHK = 8,
	WR_UNKNOWN = 9,
} wake_reason_t;

void spm_lock_init(void);
void spm_lock_get(void);
void spm_lock_release(void);
void spm_boot_init(void);

void spm_suspend_storage_mask_set(void);
void spm_suspend_args(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4);
uint64_t spm_load_firmware_status(void);
void spm_irq0_handler(uint64_t x1, uint64_t x2);
void spm_ap_mdsrc_req(uint64_t x1);
void spm_pwr_ctrl_args(uint64_t x1, uint64_t x2, uint64_t x3);
uint64_t spm_get_pwr_ctrl_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_dcs_s1_setting(uint64_t x1, uint64_t x2);
void spm_dummy_read(uint64_t x1, uint64_t x2);
void spm_args(uint64_t x1, uint64_t x2, uint64_t x3);

void spm_pcm_wdt(uint64_t enable, uint64_t time);
void spm_suspend(void);
void spm_suspend_finish(void);
void spm_suspend_callback(uint64_t user, uint64_t is_resume);

void spm_legacy_sleep_wfi(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_config_spmfw_base(uint64_t addr, uint64_t size);

void spm_set_bootaddr(unsigned long bootaddr);
void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue);
void spm_go_to_vcorefs(uint64_t spm_flags);
void dvfsrc_md_ddr_turbo(int en);
void dvfsrc_restore_req(void);
void dvfsrc_backup_req(void);
#endif /* __MT_SPM_H__ */
