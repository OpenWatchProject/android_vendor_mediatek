/**
* @file    mtk_picachu.h
* @brief   Driver header for PICACHU
*/

#ifndef _MTK_PICACHU_
#define _MTK_PICACHU_

#include <platform_def.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>

void picachu_cfg_boot_addr(void);
void picachu_cfg_default_boot_addr(void);

extern uint32_t fp0_maxtrans_slave_start(uint32_t);
#endif
