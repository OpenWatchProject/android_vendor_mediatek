/**
* @file    mtk_drcc_atf.h
* @brief   Driver header for embedded transient control
*/

#ifndef _MTK_DRCC_
#define _MTK_DRCC_

#include <debug.h>
#include <mmio.h>

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

#if 1
	#define DRCC_BASEADDR 0x0C532000 /* For Cannon */
#else
	#define DRCC_BASEADDR 0x0C532000 /* For Sylvia */
#endif

#define DRCC_BASE DRCC_BASEADDR

#define DRCC_CONF0 (DRCC_BASE + 0x750)
#define DRCC_CONF1 (DRCC_BASE + 0x754)
#define DRCC_CONF2 (DRCC_BASE + 0x758)
#define DRCC_CONF3 (DRCC_BASE + 0x75C)
#define AMUX_OUT 0x104A0048

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

extern int drcc_init(void);
extern int drcc_enable(unsigned int onOff);
extern int drcc_trig(unsigned int onOff, unsigned int value);
extern int drcc_count(unsigned int onOff, unsigned int value);
extern int drcc_mode(unsigned int value);
extern int drcc_code(unsigned int value);
extern int drcc_hwgatepct(unsigned int value);
extern int drcc_vreffilt(unsigned int value);
extern int drcc_autocalibdelay(unsigned int value);
extern int drcc_forcetrim(unsigned int onOff, unsigned int value);
extern int drcc_protect(unsigned int value);
extern int drcc_reg_read(unsigned int addr);
extern int drcc_status(void);
extern int drcc_calib_rst(void);
#endif
