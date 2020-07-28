#ifndef __PMIC_WRAP_PLATFORM_H__
#define __PMIC_WRAP_PLATFORM_H__

#define MTK_PLATFORM_MT6358      1
#define MTK_PLATFORM_MT6337      0
#if (MTK_PLATFORM_MT6337)
#define DUAL_PMICS
#endif

/***********  platform info, PMIC info ********************/
#define PMIC_WRAP_BASE (0xA0010000)
#ifndef TOPCKGEN_BASE
#define TOPCKGEN_BASE           (0xA0210000)
#endif
#define CKSYS_BASE              TOPCKGEN_BASE
#define INFRACFG_AO_REG_BASE    (0xA0000000)

#define PMIC_WRAP_REG_RANGE     (354)
#define PMIC_WRAP_WACS2_RDATA_OFFSET    0xA4
#define PMIC_WRAP_WACS2_VLDCLR_OFFSET   0xA8
#define PMIC_WRAP_INIT_DONE2_OFFSET     0x9C


/************* macro for spi clock config ******************************/
#define CLK_CFG_4_SET                       (TOPCKGEN_BASE+0x144)
#define CLK_CFG_4_CLR                       (TOPCKGEN_BASE+0x148)

#define CLK_SPI_CK_26M                       0xf3000000
#define MODULE_CLK_SEL                      (INFRACFG_AO_REG_BASE+0x098)
#define MODULE_SW_CG_0_SET                  (INFRACFG_AO_REG_BASE+0x080)
#define MODULE_SW_CG_0_CLR                  (INFRACFG_AO_REG_BASE+0x084)
#define INFRA_GLOBALCON_RST2_SET            (INFRACFG_AO_REG_BASE+0x140)
#define INFRA_GLOBALCON_RST2_CLR            (INFRACFG_AO_REG_BASE+0x144)

/****************P2P Reg ********************************/
#if (PMIC_WRAP_SCP)
#include <reg_PMIC_WRAP_P2P_mac.h>
#endif
/*****************************************************************/
#include <reg_PMIC_WRAP_mac.h>
/*******************macro for  regsister@PMIC *******************************/
#define PMIC_REG_BASE (0x0000)
#define DEW_READ_TEST        ((UINT32)(PMIC_REG_BASE+0x040e))
#define DEW_WRITE_TEST       ((UINT32)(PMIC_REG_BASE+0x0410))
#endif /*__PMIC_WRAP_PLATFORM_H__*/
