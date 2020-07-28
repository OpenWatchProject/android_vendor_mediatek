#ifndef __MT_SPM_H__
#define __MT_SPM_H__

#include <stdio.h>

#ifndef bool
typedef unsigned char bool;
#endif

#define true 1
#define false 0

extern __uint32_t __spmfw_idx;
extern __uint32_t is_ext_buck;

enum {
	SPMFW_LP4X_2CH = 0,
	SPMFW_LP4X_1CH,
	SPMFW_LP3_1CH,
};

enum {
	SPM_ARGS_SPMFW_IDX = 0,
	SPM_ARGS_SUSPEND,
	SPM_ARGS_SUSPEND_FINISH,
	SPM_ARGS_SODI,
	SPM_ARGS_SODI_FINISH,
	SPM_ARGS_DPIDLE,
	SPM_ARGS_DPIDLE_FINISH,
	SPM_ARGS_PCM_WDT,
	SPM_ARGS_NUM,
};

typedef enum {
	WR_NONE = 0,
	WR_UART_BUSY = 1,
	WR_PCM_ASSERT = 2,
	WR_PCM_TIMER = 3,
	WR_WAKE_SRC = 4,
	WR_UNKNOWN = 5,
} wake_reason_t;

void spm_lock_init(void);
void spm_lock_get(void);
void spm_lock_release(void);
void spm_boot_init(void);

void spm_suspend_args(__uint64_t x1, __uint64_t x2, __uint64_t x3);
__uint64_t spm_load_firmware_status();
void spm_irq0_handler(__uint64_t x1);
void spm_ap_mdsrc_req(__uint64_t x1);
void spm_pwr_ctrl_args(__uint64_t x1, __uint64_t x2, __uint64_t x3);
__uint64_t spm_get_pwr_ctrl_args(__uint64_t x1, __uint64_t x2, __uint64_t x3);
void spm_dcs_s1_setting(__uint64_t x1, __uint64_t x2);
void spm_dummy_read(__uint64_t x1, __uint64_t x2);
void spm_args(__uint64_t x1, __uint64_t x2, __uint64_t x3);

void spm_pcm_wdt(uint64_t enable, uint64_t time);
void spm_suspend(void);
void spm_suspend_finish(void);

void spm_legacy_sleep_wfi(__uint64_t x1, __uint64_t x2, __uint64_t x3);

#endif /* __MT_SPM_H__ */
