/**
* @file    mtk_etc_atf.h
* @brief   Driver header for embedded transient control
*/

#ifndef _MTK_ETC_
#define _MTK_ETC_

#include <debug.h>
#include <mmio.h>

/* LOG */
#define ETC_DEBUG_LOG_ON	1
#define ETC_TAG	"[ETC]"
#define etc_err(fmt, ...)	ERROR(ETC_TAG""fmt, __VA_ARGS__)
#define etc_warn(fmt, ...)	WARN(ETC_TAG""fmt, __VA_ARGS__)
#define etc_info(fmt,...)	INFO(ETC_TAG""fmt, __VA_ARGS__)
#if ETC_DEBUG_LOG_ON
#define etc_dbg(...)		etc_info(__VA_ARGS__)
#else
#define etc_dbg(...)
#endif

#define ETC_LOADER		0x0012A294
#define ETC_BASEADDR		(0x10202000)

#define ATC_ID			(ETC_BASEADDR + 0x600)
#define ATC_VOUT		(ETC_BASEADDR + 0x60C)
#define ATC_SW_DORMANT		(ETC_BASEADDR + 0x610)
#define ATC_VALARM_VERR		(ETC_BASEADDR + 0x614)
#define ATC_VALARM_STATUS	(ETC_BASEADDR + 0x618)
#define ATC_OVER_CTRL		(ETC_BASEADDR + 0x61C)
#define ATC_DROOP_CTRL		(ETC_BASEADDR + 0x620)
#define ATC_DROOP_VERR		(ETC_BASEADDR + 0x624)
#define ATC_LDO_CTRL		(ETC_BASEADDR + 0x62C)
#define ATC_DVC_WRITE		(ETC_BASEADDR + 0x634)
#define ATC_FORCE_DROOP_OFF	(ETC_BASEADDR + 0x638)
#define ATC_WAIT_TIME		(ETC_BASEADDR + 0x640)
#define ATC_LDO_WAIT_TIME	(ETC_BASEADDR + 0x644)
#define ATC_MISC2_CTRL		(ETC_BASEADDR + 0x648)
#define ATC_VMON_CTRL		(ETC_BASEADDR + 0x64C)
#define ATC_LIMIT_STATUS	(ETC_BASEADDR + 0x650)
#define ATC_ACT_EN		(ETC_BASEADDR + 0x654)
#define ATC_VMON_STATUS		(ETC_BASEADDR + 0x658)
#define ATC_TDMUX_STATUS	(ETC_BASEADDR + 0x65C)
#define ATC_TEST_ACCESS		(ETC_BASEADDR + 0x670)
#define ATC_TDMUX_CTRL		(ETC_BASEADDR + 0x674)
#define ATC_TDMUX		(ETC_BASEADDR + 0x67C)
#define ATC_DVC0_LO		(ETC_BASEADDR + 0x680)
#define ATC_DVC0_HI		(ETC_BASEADDR + 0x684)
#define ATC_DVC1_LO		(ETC_BASEADDR + 0x688)
#define ATC_DVC1_HI		(ETC_BASEADDR + 0x68C)
#define ATC_DVC2_LO		(ETC_BASEADDR + 0x690)
#define ATC_DVC2_HI             (ETC_BASEADDR + 0x694)
#define ATC_DVC3_LO		(ETC_BASEADDR + 0x698)
#define ATC_DVC3_HI             (ETC_BASEADDR + 0x69C)
#define ATC_DVC0_LO_RD          (ETC_BASEADDR + 0x6A0)
#define ATC_DVC0_HI_RD          (ETC_BASEADDR + 0x6A4)
#define ATC_DVC1_LO_RD          (ETC_BASEADDR + 0x6A8)
#define ATC_DVC1_HI_RD          (ETC_BASEADDR + 0x6AC)
#define ATC_DVC2_LO_RD          (ETC_BASEADDR + 0x6B0)
#define ATC_DVC2_HI_RD          (ETC_BASEADDR + 0x6B4)
#define ATC_DVC3_LO_RD          (ETC_BASEADDR + 0x6B8)
#define ATC_DVC3_HI_RD          (ETC_BASEADDR + 0x6BC)
#define ATC_DVC_LO_RD		(ETC_BASEADDR + 0x6C0)
#define ATC_DVC_HI_RD		(ETC_BASEADDR + 0x6C4)
#define ATC_ANA_TRIM1           (ETC_BASEADDR + 0x6E0)
#define ATC_ANA_TRIM2           (ETC_BASEADDR + 0x6E4)
#define ATC_DVC_LO_RUN_RD       (ETC_BASEADDR + 0x6E8)
#define ATC_DVC_HI_RUN_RD       (ETC_BASEADDR + 0x6EC)

/* Vout = 0.4V + (0.003125 * ATC_VOUT) */
#define ETC_V_BASE	(400000)
#define ETC_STEP	(3125)

#define VMAX		(1000000)
#define VMIN		(600000)

extern int etc_init(void);
extern int etc_voltage_change(unsigned int new_vout);
extern int etc_power_off(void);
extern int etc_dormant_ctrl(unsigned int onOff);
extern int etc_reg_read(unsigned int addr);
extern int etc_ank(unsigned int ank, unsigned int vol);
#endif
