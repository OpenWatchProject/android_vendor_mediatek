/**
* @file    mtk_drcc_atf.h
* @brief   Driver header for embedded transient control
*/

#ifndef _MTK_DRCC_
#define _MTK_DRCC_

#include <debug.h>
#include <mmio.h>
#include <platform_def.h>

/* LOG */
#define DRCC_DEBUG_LOG_ON	1
#define DRCC_TAG	"[DRCC]"
#define drcc_err(fmt, ...)	ERROR(DRCC_TAG""fmt, __VA_ARGS__)
#define drcc_warn(fmt, ...)	WARN(DRCC_TAG""fmt, __VA_ARGS__)
#define drcc_info(fmt, ...)	INFO(DRCC_TAG""fmt, __VA_ARGS__)
#if DRCC_DEBUG_LOG_ON
#define drcc_dbg(...)		drcc_info(__VA_ARGS__)
#else
#define drcc_dbg(...)
#endif

/* #define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0])) */
#define DRCC_NUM PLATFORM_CORE_COUNT
#define AMUX_OUT 0x100056E0

#define DRCC_BASE 0x0C530000
#define CPU0_DRCC_A0_CONFIG	(DRCC_BASE + 0xB000)
#define CPU0_DRCC_CFG_REG0	(DRCC_BASE + 0x0280)
#define CPU0_DRCC_CFG_REG1	(DRCC_BASE + 0x0284)
#define CPU0_DRCC_CFG_REG2	(DRCC_BASE + 0x0288)
#define CPU0_DRCC_CFG_REG3	(DRCC_BASE + 0x028C)

#define CPU1_DRCC_A0_CONFIG	(DRCC_BASE + 0xB200)
#define CPU1_DRCC_CFG_REG0	(DRCC_BASE + 0x0A80)
#define CPU1_DRCC_CFG_REG1	(DRCC_BASE + 0x0A84)
#define CPU1_DRCC_CFG_REG2	(DRCC_BASE + 0x0A88)
#define CPU1_DRCC_CFG_REG3	(DRCC_BASE + 0x0A8C)

#define CPU2_DRCC_A0_CONFIG	(DRCC_BASE + 0xB400)
#define CPU2_DRCC_CFG_REG0	(DRCC_BASE + 0x1280)
#define CPU2_DRCC_CFG_REG1	(DRCC_BASE + 0x1284)
#define CPU2_DRCC_CFG_REG2	(DRCC_BASE + 0x1288)
#define CPU2_DRCC_CFG_REG3	(DRCC_BASE + 0x128C)

#define CPU3_DRCC_A0_CONFIG	(DRCC_BASE + 0xB600)
#define CPU3_DRCC_CFG_REG0	(DRCC_BASE + 0x1A80)
#define CPU3_DRCC_CFG_REG1	(DRCC_BASE + 0x1A84)
#define CPU3_DRCC_CFG_REG2	(DRCC_BASE + 0x1A88)
#define CPU3_DRCC_CFG_REG3	(DRCC_BASE + 0x1A8C)

#define CPU4_DRCC_A0_CONFIG	(DRCC_BASE + 0xB800)
#define CPU4_DRCC_CFG_REG0	(DRCC_BASE + 0x2280)
#define CPU4_DRCC_CFG_REG1	(DRCC_BASE + 0x2284)
#define CPU4_DRCC_CFG_REG2	(DRCC_BASE + 0x2288)
#define CPU4_DRCC_CFG_REG3	(DRCC_BASE + 0x228C)

#define CPU5_DRCC_A0_CONFIG	(DRCC_BASE + 0xBA00)
#define CPU5_DRCC_CFG_REG0	(DRCC_BASE + 0x2A80)
#define CPU5_DRCC_CFG_REG1	(DRCC_BASE + 0x2A84)
#define CPU5_DRCC_CFG_REG2	(DRCC_BASE + 0x2A88)
#define CPU5_DRCC_CFG_REG3	(DRCC_BASE + 0x2A8C)

#define CPU6_DRCC_A0_CONFIG	(DRCC_BASE + 0xBC00)
#define CPU6_DRCC_CFG_REG0	(DRCC_BASE + 0x3280)
#define CPU6_DRCC_CFG_REG1	(DRCC_BASE + 0x3284)
#define CPU6_DRCC_CFG_REG2	(DRCC_BASE + 0x3288)
#define CPU6_DRCC_CFG_REG3	(DRCC_BASE + 0x328C)

#define CPU7_DRCC_A0_CONFIG	(DRCC_BASE + 0xBE00)
#define CPU7_DRCC_CFG_REG0	(DRCC_BASE + 0x3A80)
#define CPU7_DRCC_CFG_REG1	(DRCC_BASE + 0x3A84)
#define CPU7_DRCC_CFG_REG2	(DRCC_BASE + 0x3A88)
#define CPU7_DRCC_CFG_REG3	(DRCC_BASE + 0x3A8C)

#define DRCC_ENABLE		0
#define DRCC_TRIGEN		0x0
#define DRCC_TRIGSEL		0x0
#define DRCC_COUNTEN		0x0
#define DRCC_COUNTSEL		0x0
#define DRCC_STANDBYWFIL2_MASK	0x0
#define DRCC_MODE		0
#define DRCC_CODE		0x38		/* 52 */
#define DRCC_HWGATEPCT		0x3		/* 50 % */
#define DRCC_VERFFILT		0x1		/* 2 Mhz */
#define DRCC_AUTOCALIBDELAY	0x0
#define DRCC_FORCETRIM		0x0
#define DRCC_FORCETRIMEN	0x0
#define DRCC_DISABLEAUTOPRTDURCALIB	0x0

extern int drcc_debug_init(void);
extern int drcc_enable(unsigned int onOff, unsigned int drcc_n);
extern int drcc_trig(unsigned int onOff, unsigned int value, unsigned int drcc_n);
extern int drcc_count(unsigned int onOff, unsigned int value, unsigned int drcc_n);
extern int drcc_mode(unsigned int value, unsigned int drcc_n);
extern int drcc_code(unsigned int value, unsigned int drcc_n);
extern int drcc_hwgatepct(unsigned int value, unsigned int drcc_n);
extern int drcc_vreffilt(unsigned int value, unsigned int drcc_n);
extern int drcc_autocalibdelay(unsigned int value, unsigned int drcc_n);
extern int drcc_forcetrim(unsigned int onOff, unsigned int value, unsigned int drcc_n);
extern int drcc_protect(unsigned int value, unsigned int drcc_n);
extern int drcc_reg_read(unsigned int addr);
#endif
