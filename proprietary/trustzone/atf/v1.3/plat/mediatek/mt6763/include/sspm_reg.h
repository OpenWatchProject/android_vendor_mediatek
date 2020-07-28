#ifndef __SSPM_REG_H__
#define __SSPM_REG_H__

#include "platform_def.h"

#define SSPM_CFGREG_RSV_RW_REG0        (SSPM_CFGREG_BASE + 0x0100)
#define SSPM_CFGREG_ACAO_INT_SET       (SSPM_CFGREG_BASE + 0x00D8)
#define SSPM_CFGREG_ACAO_INT_CLR       (SSPM_CFGREG_BASE + 0x00DC)

#define STANDBYWFI_EN(n)               (1 << (n +  8))
#define GIC_IRQOUT_EN(n)               (1 << (n +  0))

#define NF_MCDI_MBOX                            9
#define MCDI_MBOX_CLUSTER_0_CAN_POWER_OFF       0
#define MCDI_MBOX_CLUSTER_1_CAN_POWER_OFF       1
#define MCDI_MBOX_CLUSTER_2_CAN_POWER_OFF       2
#define MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE     3
#define MCDI_MBOX_CLUSTER_1_ATF_ACTION_DONE     4
#define MCDI_MBOX_CLUSTER_2_ATF_ACTION_DONE     5
#define MCDI_MBOX_PAUSE_ACTION                  6
#define MCDI_MBOX_CPU_CLUSTER_PWR_STAT          7
#define MCDI_MBOX_ACTION_STAT                   8

#endif /* __SSPM_REG_H__ */
