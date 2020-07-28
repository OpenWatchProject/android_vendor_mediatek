#ifndef __MTK_MCDI_H__
#define __MTK_MCDI_H__

#include <stdint.h>
#define DYNA_LOAD_MCUPM_PATH_SIZE	(128)
#define MCUPM_FW_VERSION_SIZE	(128)
#define MCUPM_BINARY_SIZE	(848)
#define MCUPM_FW_MAGIC	(0x53504D32)
#define MCDI_TAG "MCDI"
/* #define MCDI_INFO(fmt, ...) INFO(MCDI_TAG fmt, ##...) */

/* mcdi smc call event id */
enum {
	MCDI_ARGS_MCUPMFW_INIT = 0,
};

enum dyna_load_fw_index {
	MCUPM_FW_MCDI = 0,
	MCUPM_FW_MAX,
};

struct mcupm_fw_header {
	unsigned int magic;
	unsigned int size;
#ifdef MCUPM_FW_USE_PARTITION
	char name[58];
#else
	char name[72];
#endif
}__attribute__((packed));

struct mcupm_desc {
	char *version;			/* MCUPM code version */
	uint32_t pmem_words;	/* size of pmem */
	uint32_t total_words;	/* size of pmem + dmem */
	uint32_t pmem_start;	/* start addr of pmem */
	uint32_t dmem_start;	/* start addr of dmem */
}__attribute__((packed));

/* we parse raw binary into this struct */
struct dyna_load_mcupm_t {
	char path[DYNA_LOAD_MCUPM_PATH_SIZE];
	char version[MCUPM_FW_VERSION_SIZE];
	uint32_t *buf;
	struct mcupm_desc desc;
	int ready;
};

#ifndef MCUPM_FW_USE_PARTITION
struct bin {
	struct mcupm_fw_header header;
	uint32_t binary[MCUPM_BINARY_SIZE];
	struct mcupm_desc desc;
}__attribute__((packed));
#endif

extern struct dyna_load_mcupm_t dyna_load_mcupm[MCUPM_FW_MAX];
extern struct mcupm_desc mcdi_mcupm;
extern struct mcupm_fw_header header;
extern uint32_t mcdi_binary[MCUPM_BINARY_SIZE];
extern struct bin mcupm_bin;

extern void mcdi_mcupm_cluster_pwr_on_notify(unsigned long cluster);
extern void mcdi_mcupm_cluster_pwr_off_notify(unsigned long cluster);
extern void mcdi_mcupm_standbywfi_irq_enable(unsigned long cpu);
extern void mcdi_mcupm_wakeup_irq_enable(unsigned long cpu);
extern void mcdi_mcupm_boot_init(void);
extern void mcdi_args(uint64_t x1, uint64_t x2, uint64_t x3);
extern int mcdi_load_firmware_status(void);

#endif /* __MTK_MCDI_H__ */
