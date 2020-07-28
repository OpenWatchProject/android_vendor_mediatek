#ifndef __LAST_EMI_H__
#define __LAST_EMI_H__

#include <platform_def.h>

#define EMI_AXI_BIST_ADR0		(EMI_CEN_BASE + 0x98c)
#define EMI_AXI_BIST_ADR1		(EMI_CEN_BASE + 0x990)
#define EMI_AXI_BIST_ADR2		(EMI_CEN_BASE + 0x994)

uint64_t sip_last_emi_reset(uint64_t start, uint64_t end);

#endif /* end of __LAST_EMI_H__ */
