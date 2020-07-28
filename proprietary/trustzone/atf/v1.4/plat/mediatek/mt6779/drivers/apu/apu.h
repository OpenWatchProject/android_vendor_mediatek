#ifndef __APU_H__
#define __APU_H__

#include <platform_def.h>



/******************************************************************************
 *
 * REGISTER ADDRESS DEFINATION

 ******************************************************************************/

#define APUSYS_VCORE_CONFIG_BASE	0x19020000
/*******************************************************************************************/
/* APUSYS VCORE CG*/
#define APUSYS_VCORE_CG_SET          ((unsigned int *)(APUSYS_VCORE_CONFIG_BASE+0x4))
#define APUSYS_VCORE_CG_CLR          ((unsigned int *)(APUSYS_VCORE_CONFIG_BASE+0x8))

void apu_vcore_cg_ctl(int poweron);

#endif /* __APU_H__ */

