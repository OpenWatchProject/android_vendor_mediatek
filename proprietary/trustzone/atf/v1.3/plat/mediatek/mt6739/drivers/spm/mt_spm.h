#ifndef __MT_SPM_H__
#define __MT_SPM_H__

#include <stdio.h>

#define true 1
#define false 0

enum {
	SPM_ARGS_SPMFW_IDX = 0,
	SPM_ARGS_SPMFW_INIT,
	SPM_ARGS_SUSPEND,
	SPM_ARGS_SUSPEND_FINISH,
	SPM_ARGS_SODI,
	SPM_ARGS_SODI_FINISH,
	SPM_ARGS_DPIDLE,
	SPM_ARGS_DPIDLE_FINISH,
	SPM_ARGS_PCM_WDT,
	SPM_ARGS_NUM,
};

enum {
	WR_NONE = 0,
	WR_UART_BUSY = 1,
	WR_PCM_ASSERT = 2,
	WR_PCM_TIMER = 3,
	WR_WAKE_SRC = 4,
	WR_UNKNOWN = 5,
};

void spm_lock_init(void);
void spm_lock_get(void);
void spm_lock_release(void);
void spm_boot_init(void);

void spm_suspend_args(uint64_t x1, uint64_t x2, uint64_t x3);
uint64_t spm_load_firmware_status(void);
void spm_irq0_handler(uint64_t x1);
void spm_ap_mdsrc_req(uint64_t x1);
void spm_pwr_ctrl_args(uint64_t x1, uint64_t x2, uint64_t x3);
uint64_t spm_get_pwr_ctrl_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_dcs_s1_setting(uint64_t x1, uint64_t x2);
void spm_dummy_read(uint64_t x1, uint64_t x2);
void spm_args(uint64_t x1, uint64_t x2, uint64_t x3);

void spm_pcm_wdt(uint64_t enable, uint64_t time);
void spm_suspend(void);
void spm_suspend_finish(void);

extern void gic_cpuif_init(void);
void spm_legacy_sleep_wfi(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_config_spmfw_base(uint64_t addr, uint64_t size);

#endif /* __MT_SPM_H__ */
