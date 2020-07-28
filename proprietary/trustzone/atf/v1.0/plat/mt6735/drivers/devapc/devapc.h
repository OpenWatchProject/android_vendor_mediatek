#ifndef _ATF_DAPC_H__
#define _ATF_DAPC_H__
/******************************************************************************
*
 * STRUCTURE DEFINATION
 
******************************************************************************/
 typedef enum
 {
     E_L0=0,
     E_L1,
     E_L2,
     E_L3,
     E_MAX_APC_ATTR,
     E_APC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
 }APC_ATTR;

 // domain index 
 typedef enum
 {
     E_DOMAIN_0 = 0,
     E_DOMAIN_1 ,
     E_DOMAIN_2 , 
     E_DOMAIN_3 ,
     E_MAX,
     E_MASK_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
 }E_MASK_DOM;
 
 
 typedef struct {
     const char          *device_name;
     APC_ATTR            d0_permission;
     APC_ATTR            d1_permission;
     APC_ATTR            d2_permission;
     APC_ATTR            d3_permission;
 } DEVICE_INFO;
/******************************************************************************
*
 * UTILITY DEFINATION
 
******************************************************************************/

#define READ_REGISTER_UINT32(reg) \
        (*(volatile unsigned int * const)(uintptr_t)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
        (*(volatile unsigned int * const)(uintptr_t)(reg)) = (val)

#define writel(VAL,REG)  WRITE_REGISTER_UINT32(REG,VAL)
#define readl(REG)       READ_REGISTER_UINT32(REG)

/******************************************************************************
*
 * REGISTER ADDRESS DEFINATION
 
******************************************************************************/
#define DEVAPC_AO_BASE         0x10007000
#define DEVAPC_PD_BASE         0x10207000
#define INFRACFG_AO_base       0x10000000

#define DEVAPC_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0000))
#define DEVAPC_D0_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0004))
#define DEVAPC_D0_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0008))
#define DEVAPC_D0_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x000C))
#define DEVAPC_D0_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0010))
#define DEVAPC_D0_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0014))
#define DEVAPC_D0_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0018))
#define DEVAPC_D0_APC_7            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x001C))

#define DEVAPC_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0100))
#define DEVAPC_D1_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0104))
#define DEVAPC_D1_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0108))
#define DEVAPC_D1_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x010C))
#define DEVAPC_D1_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0110))
#define DEVAPC_D1_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0114))
#define DEVAPC_D1_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0118))
#define DEVAPC_D1_APC_7            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x011C))

#define DEVAPC_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0200))
#define DEVAPC_D2_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0204))
#define DEVAPC_D2_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0208))
#define DEVAPC_D2_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x020C))
#define DEVAPC_D2_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0210))
#define DEVAPC_D2_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0214))
#define DEVAPC_D2_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0218))
#define DEVAPC_D2_APC_7            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x021C))

#define DEVAPC_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0300))
#define DEVAPC_D3_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0304))
#define DEVAPC_D3_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0308))
#define DEVAPC_D3_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x030C))
#define DEVAPC_D3_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0310))
#define DEVAPC_D3_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0314))
#define DEVAPC_D3_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0318))
#define DEVAPC_D3_APC_7            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x031C))

#if defined(MACH_TYPE_MT6735)

#define DEVAPC_D4_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0400))
#define DEVAPC_D4_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0404))
#define DEVAPC_D4_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0408))
#define DEVAPC_D4_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x040C))
#define DEVAPC_D4_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0410))
#define DEVAPC_D4_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0414))
#define DEVAPC_D4_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0418))

#define DEVAPC_D5_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0500))
#define DEVAPC_D5_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0504))
#define DEVAPC_D5_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0508))
#define DEVAPC_D5_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x050C))
#define DEVAPC_D5_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0510))
#define DEVAPC_D5_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0514))
#define DEVAPC_D5_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0518))

#define DEVAPC_D6_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0600))
#define DEVAPC_D6_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0604))
#define DEVAPC_D6_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0608))
#define DEVAPC_D6_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x060C))
#define DEVAPC_D6_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0610))
#define DEVAPC_D6_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0614))
#define DEVAPC_D6_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0618))

#define DEVAPC_D7_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0700))
#define DEVAPC_D7_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0704))
#define DEVAPC_D7_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0708))
#define DEVAPC_D7_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x070C))
#define DEVAPC_D7_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0710))
#define DEVAPC_D7_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0714))
#define DEVAPC_D7_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0718))


#define DEVAPC_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A00))
#define DEVAPC_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A04))
#define DEVAPC_MAS_SEC             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B00))

#elif defined(MACH_TYPE_MT6735M)

#define DEVAPC_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0400))
#define DEVAPC_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0404))
#define DEVAPC_MAS_SEC             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0500))

#elif defined(MACH_TYPE_MT6753)

#define DEVAPC_D4_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0400))
#define DEVAPC_D4_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0404))
#define DEVAPC_D4_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0408))
#define DEVAPC_D4_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x040C))
#define DEVAPC_D4_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0410))
#define DEVAPC_D4_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0414))
#define DEVAPC_D4_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0418))

#define DEVAPC_D5_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0500))
#define DEVAPC_D5_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0504))
#define DEVAPC_D5_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0508))
#define DEVAPC_D5_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x050C))
#define DEVAPC_D5_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0510))
#define DEVAPC_D5_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0514))
#define DEVAPC_D5_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0518))

#define DEVAPC_D6_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0600))
#define DEVAPC_D6_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0604))
#define DEVAPC_D6_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0608))
#define DEVAPC_D6_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x060C))
#define DEVAPC_D6_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0610))
#define DEVAPC_D6_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0614))
#define DEVAPC_D6_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0618))

#define DEVAPC_D7_APC_0            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0700))
#define DEVAPC_D7_APC_1            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0704))
#define DEVAPC_D7_APC_2            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0708))
#define DEVAPC_D7_APC_3            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x070C))
#define DEVAPC_D7_APC_4            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0710))
#define DEVAPC_D7_APC_5            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0714))
#define DEVAPC_D7_APC_6            ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0718))


#define DEVAPC_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A00))
#define DEVAPC_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0A04))
#define DEVAPC_MAS_SEC             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0B00))

#else

#error "Wrong MACH type"

#endif


#define DEVAPC_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F00))
#define DEVAPC_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F04))
#define DEVAPC_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F08))
#define DEVAPC_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F0C))
#define DEVAPC_APC_LOCK_3          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F10))
#define DEVAPC_APC_LOCK_4          ((volatile unsigned int*)(DEVAPC_AO_BASE+0x0F14))


#define DEVAPC_PD_APC_CON          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F00))
#define DEVAPC_D0_VIO_MASK_0       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0000))
#define DEVAPC_D0_VIO_MASK_1       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0004))
#define DEVAPC_D0_VIO_MASK_2       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0008))
#define DEVAPC_D0_VIO_MASK_3       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x000C))
#define DEVAPC_D0_VIO_MASK_4       ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0010))
#define DEVAPC_D0_VIO_STA_0        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0400))
#define DEVAPC_D0_VIO_STA_1        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0404))
#define DEVAPC_D0_VIO_STA_2        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0408))
#define DEVAPC_D0_VIO_STA_3        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x040C))
#define DEVAPC_D0_VIO_STA_4        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0410)) 

#define DEVAPC_VIO_DBG0            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0900))
#define DEVAPC_VIO_DBG1            ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0904))
#define DEVAPC_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F80))
#define DEVAPC_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F84))
#define DEVAPC_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC_PD_BASE+0x0F88))

#define INFRA_AO_SEC_CON           ((volatile unsigned int*)( INFRACFG_AO_base+0x0F00))
#define INFRA_RST_SEC_CON          ((volatile unsigned int*)( INFRACFG_AO_base+0x0F04))
#define INFRA_PDN_SEC_CON          ((volatile unsigned int*)( INFRACFG_AO_base+0x0F08))
/******************************************************************************
*
 * Variable DEFINATION
 
******************************************************************************/

#define SEJ_CG_PROTECT_BIT         ((0x1) << 19) /*Denali 10000040 INFRA_GLOBALCON_PDN0*/
#define TRNG_CG_PROTECT_BIT        ((0x1) << 2)  /*Denali 10000040 INFRA_GLOBALCON_PDN0*/

/* DOMAIN_SETUP */
#define DOMAIN_AP                       0
#define DOMAIN_MD1                      1
#define DOMAIN_MD2                      2
#define DOMAIN_MM                       3

#define MOD_NO_IN_1_DEVAPC             16

#endif
