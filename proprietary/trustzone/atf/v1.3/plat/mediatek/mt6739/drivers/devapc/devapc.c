#include <debug.h>
#include <devapc.h>
#include <platform.h>
#include <plat_private.h>


#if !defined(SPD_no)

/* For all the TEE projects */

static const struct INFRA_PERI_DEVICE_INFO D_APC_INFRA_Devices_TEE[] = {
	/*              module,                                  AP permission,          MD permission (MD1 & C2K) */

	/* 0 */
	DAPC_INFRA_ATTR("INFRA_AO_TOPCKGEN",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",         E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("IO_CFG_REG",                            E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_ PERICFG",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_EFUSE_AO_DEBUG",               E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_GPIO",                         E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_SLEEP_CONTROLLER",             E_NO_PROTECTION, E_SEC_RW_NONSEC_R_ONLY),
	DAPC_INFRA_ATTR("INFRA_AO_TOPRGU",                       E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_APXGPT",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),

	/* 10 */
	DAPC_INFRA_ATTR("INFRA_AO_SEJ",                          E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_AP_CIRQ_EINT",                 E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_APMIXEDSYS",                   E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_PMIC_WRAP",                    E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_DEVICE_APC_AO_INFRA_PERI",     E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_SLEEP_CONTROLLER_MD",          E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_KEYPAD",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_TOP_MISC",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_ DVFS_CTRL_PROC",              E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_MBIST_AO_REG",                 E_NO_PROTECTION, E_FORBIDDEN),

	/* 20 */
	DAPC_INFRA_ATTR("INFRA_AO_CLDMA_AO_AP",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_AES_TOP_0",                    E_FORBIDDEN, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_SYS_TIMER",                    E_NO_PROTECTION, E_SEC_RW_NONSEC_R_ONLY),
	DAPC_INFRA_ATTR("INFRA_AO_MDEM_TEMP_SHARE",              E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_DEVICE_APC_AO_MD",             E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_SECURITY_AO",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_TOPCKGEN_REG",                 E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_DEVICE_APC_AO_MM",             E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_DRAMC_REG",                    E_NO_PROTECTION, E_FORBIDDEN),

	/* 30 */
	DAPC_INFRA_ATTR("INFRA_AO_DDRPHY_REG",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG0",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG1",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG2",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG3",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_SYS_CIRQ",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MM_IOMMU",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_EFUSE_PDN_DEBUG",              E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DEVICE_APC",                   E_NO_PROTECTION, E_FORBIDDEN),

	/* 40 */
	DAPC_INFRA_ATTR("INFRASYS_DBG_TRACKER",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_CCIF0_AP",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_CCIF0_MD",                     E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_CCIF1_MD",                     E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_CLDMA_MD",                     E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_MBIST",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_INFRA_PDN_REGISTER",           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_TRNG",                         E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DX_CC",                        E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),

	/* 50 */
	DAPC_INFRA_ATTR("INFRASYS_CQ_DMA",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_SRAMROM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUPM_REG",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUPM_SRAM",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUPM_SRAM",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_EMI",                          E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_CHN_EMI",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_CLDMA_PDN_AP",                 E_NO_PROTECTION, E_NO_PROTECTION),

	/* 60 */
	DAPC_INFRA_ATTR("INFRASYS_CLDMA_PDN_MD",                 E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_DRAMC_NAO",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_EMI_MPU",                      E_SEC_RW_ONLY,   E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DVFS_PROC",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_GCE",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_MCUCFG",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DBUGSYS",                      E_NO_PROTECTION, E_FORBIDDEN),

	/* 70 */
	DAPC_INFRA_ATTR("PERISYS_APDMA",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_AUXADC",                        E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_UART0",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UART1",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UART2",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UART3",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_PWM",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C0",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C1",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C2",                          E_NO_PROTECTION, E_FORBIDDEN),

	/* 80 */
	DAPC_INFRA_ATTR("PERISYS_SPI0",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_PTP",                           E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_BTIF",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_IRTX",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_DISP_PWM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C3",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_SPI1",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C4",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_SPI2",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_SPI3",                          E_NO_PROTECTION, E_FORBIDDEN),

	/* 90 */
	DAPC_INFRA_ATTR("PERISYS_I2C1_IMM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C2_IMM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C5",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C5_IMM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_NFI",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_NFIECC",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_USB",                           E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_USB_2.0_SUB",                   E_FORBIDDEN, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_MSDC0",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_MSDC1",                         E_NO_PROTECTION, E_FORBIDDEN),

	/* 100 */
	DAPC_INFRA_ATTR("PERISYS_MSDC2",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_MSDC3",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UFS",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISUS_USB3.0_SIF",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISUS_USB3.0_SIF2",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_USB_2.0_SIF",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_AUDIO",                         E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("EAST_EFUSE",                            E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("EAST_ CSI0_TOP_AO",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("EAST_ CSI1_TOP_AO",                     E_NO_PROTECTION, E_FORBIDDEN),

	/* 110 */
	DAPC_INFRA_ATTR("EAST_ RESERVE",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_MSDC1",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_RESERVE_0",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_RESERVE_1",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_RESERVE_2",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_MIPI_TX_CONFIG",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_RESERVE_0",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_RESERVE_1",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_RESERVE_2",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("NORTH_USBSIF_TOP",                      E_NO_PROTECTION, E_NO_PROTECTION),

	/* 120 */
	DAPC_INFRA_ATTR("NORTH_MSDC0",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("NORTH_RESERVE_0",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("NORTH_RESERVE_1",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_CONN",                          E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_RESERVE",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_RESERVE",                       E_NO_PROTECTION, E_FORBIDDEN),
};

#else

/* For Non-TEE projects */

static const struct INFRA_PERI_DEVICE_INFO D_APC_INFRA_Devices[] = {
	/*              module,                                  AP permission,          MD permission (MD1 & C2K) */

	/* 0 */
	DAPC_INFRA_ATTR("INFRA_AO_TOPCKGEN",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",         E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("IO_CFG_REG",                            E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_ PERICFG",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_EFUSE_AO_DEBUG",               E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_GPIO",                         E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_SLEEP_CONTROLLER",             E_NO_PROTECTION, E_SEC_RW_NONSEC_R_ONLY),
	DAPC_INFRA_ATTR("INFRA_AO_TOPRGU",                       E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_APXGPT",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),

	/* 10 */
	DAPC_INFRA_ATTR("INFRA_AO_SEJ",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_AP_CIRQ_EINT",                 E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_APMIXEDSYS",                   E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_PMIC_WRAP",                    E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_DEVICE_APC_AO_INFRA_PERI",     E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_SLEEP_CONTROLLER_MD",          E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_KEYPAD",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_TOP_MISC",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_ DVFS_CTRL_PROC",              E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_MBIST_AO_REG",                 E_NO_PROTECTION, E_FORBIDDEN),

	/* 20 */
	DAPC_INFRA_ATTR("INFRA_AO_CLDMA_AO_AP",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_AES_TOP_0",                    E_FORBIDDEN, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_SYS_TIMER",                    E_NO_PROTECTION, E_SEC_RW_NONSEC_R_ONLY),
	DAPC_INFRA_ATTR("INFRA_AO_MDEM_TEMP_SHARE",              E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRA_AO_DEVICE_APC_AO_MD",             E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_SECURITY_AO",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_TOPCKGEN_REG",                 E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_DEVICE_APC_AO_MM",             E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_DRAMC_REG",                    E_NO_PROTECTION, E_FORBIDDEN),

	/* 30 */
	DAPC_INFRA_ATTR("INFRA_AO_DDRPHY_REG",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG0",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG1",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG2",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUSYS_REG3",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_SYS_CIRQ",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MM_IOMMU",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_EFUSE_PDN_DEBUG",              E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DEVICE_APC",                   E_NO_PROTECTION, E_FORBIDDEN),

	/* 40 */
	DAPC_INFRA_ATTR("INFRASYS_DBG_TRACKER",                  E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_CCIF0_AP",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_CCIF0_MD",                     E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_CCIF1_MD",                     E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_CLDMA_MD",                     E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_MBIST",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_INFRA_PDN_REGISTER",           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_TRNG",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DX_CC",                        E_SEC_RW_ONLY, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),

	/* 50 */
	DAPC_INFRA_ATTR("INFRASYS_CQ_DMA",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_SRAMROM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUPM_REG",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUPM_SRAM",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_MCUPM_SRAM",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_EMI",                          E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_CHN_EMI",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_CLDMA_PDN_AP",                 E_NO_PROTECTION, E_NO_PROTECTION),

	/* 60 */
	DAPC_INFRA_ATTR("INFRASYS_CLDMA_PDN_MD",                 E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("INFRASYS_DRAMC_NAO",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_RESERVE",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_EMI_MPU",                      E_SEC_RW_ONLY,   E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DVFS_PROC",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_GCE",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRA_AO_MCUCFG",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("INFRASYS_DBUGSYS",                      E_NO_PROTECTION, E_FORBIDDEN),

	/* 70 */
	DAPC_INFRA_ATTR("PERISYS_APDMA",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_AUXADC",                        E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_UART0",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UART1",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UART2",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UART3",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_PWM",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C0",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C1",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C2",                          E_NO_PROTECTION, E_FORBIDDEN),

	/* 80 */
	DAPC_INFRA_ATTR("PERISYS_SPI0",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_PTP",                           E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_BTIF",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_IRTX",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_DISP_PWM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C3",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_SPI1",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C4",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_SPI2",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_SPI3",                          E_NO_PROTECTION, E_FORBIDDEN),

	/* 90 */
	DAPC_INFRA_ATTR("PERISYS_I2C1_IMM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C2_IMM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C5",                          E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_I2C5_IMM",                      E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_NFI",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_NFIECC",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_USB",                           E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_USB_2.0_SUB",                   E_FORBIDDEN, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_MSDC0",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_MSDC1",                         E_NO_PROTECTION, E_FORBIDDEN),

	/* 100 */
	DAPC_INFRA_ATTR("PERISYS_MSDC2",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_MSDC3",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_UFS",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISUS_USB3.0_SIF",                    E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISUS_USB3.0_SIF2",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_USB_2.0_SIF",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_AUDIO",                         E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("EAST_EFUSE",                            E_SEC_RW_NONSEC_R_ONLY, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("EAST_ CSI0_TOP_AO",                     E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("EAST_ CSI1_TOP_AO",                     E_NO_PROTECTION, E_FORBIDDEN),

	/* 110 */
	DAPC_INFRA_ATTR("EAST_ RESERVE",                         E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_MSDC1",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_RESERVE_0",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_RESERVE_1",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("SOUTH_RESERVE_2",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_MIPI_TX_CONFIG",                   E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_RESERVE_0",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_RESERVE_1",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("WEST_RESERVE_2",                        E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("NORTH_USBSIF_TOP",                      E_NO_PROTECTION, E_NO_PROTECTION),

	/* 120 */
	DAPC_INFRA_ATTR("NORTH_MSDC0",                           E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("NORTH_RESERVE_0",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("NORTH_RESERVE_1",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_CONN",                          E_NO_PROTECTION, E_NO_PROTECTION),
	DAPC_INFRA_ATTR("PERISYS_RESERVE",                       E_NO_PROTECTION, E_FORBIDDEN),
	DAPC_INFRA_ATTR("PERISYS_RESERVE",                       E_NO_PROTECTION, E_FORBIDDEN),
};

#endif

static const struct MM_MD_DEVICE_INFO D_APC_MM_Devices[] = {
	/*            module,                                 AP permission            */

	/* 0 */
	DAPC_MM_ATTR("GPU",                         E_NO_PROTECTION),
	DAPC_MM_ATTR("GPU CONFIG",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("GPU RESERVE",                 E_NO_PROTECTION),
	DAPC_MM_ATTR("MFG_OTHERS",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("MMSYS_CONFIG",                E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_MUTEX",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("SMI_COMMON",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("SMI_LARB0",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("MDP_RDMA0",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("MDP_RSZ0",                    E_NO_PROTECTION),

	/* 10 */
	DAPC_MM_ATTR("MDP_RSZ1",                    E_NO_PROTECTION),
	DAPC_MM_ATTR("MDP_WDMA0",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("MDP_WROT0",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("MDP_TDSHP0",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_OVL0",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_RDMA0",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_WDMA0",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_COLOR0",                 E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_CCORR0",                 E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_AAL0",                   E_NO_PROTECTION),

	/* 20 */
	DAPC_MM_ATTR("DISP_GAMMA0",                 E_NO_PROTECTION),
	DAPC_MM_ATTR("DISP_DITHER0",                E_NO_PROTECTION),
	DAPC_MM_ATTR("DSI0",                        E_NO_PROTECTION),
	DAPC_MM_ATTR("DBI0",                        E_NO_PROTECTION),
	DAPC_MM_ATTR("DSI0",                        E_NO_PROTECTION),
	DAPC_MM_ATTR("DPI",                         E_NO_PROTECTION),
	DAPC_MM_ATTR("MM_MUTEX",                    E_NO_PROTECTION),
	DAPC_MM_ATTR("SMI_LARB0",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("SMI_LARB1",                   E_NO_PROTECTION),
	DAPC_MM_ATTR("SMI_COMMON",                  E_NO_PROTECTION),

	/* 30 */
	DAPC_MM_ATTR("IMGSYS_CONFIG",               E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_SMI_LARB2",            E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_DISP_A0",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_DISP_A1",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_DISP_A2",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_RESERVE",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_RESERVE",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_VAD",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_DPE",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_RSC",                  E_NO_PROTECTION),

	/* 40 */
	DAPC_MM_ATTR("IMGSYS_RESERVE",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_FDVT",                 E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_GEPF",                 E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_RESERVE",              E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_DFP",                  E_NO_PROTECTION),
	DAPC_MM_ATTR("IMGSYS_RESERVE",              E_NO_PROTECTION),
	DAPC_MM_ATTR("VCODESYS_VENC_GLOBAL_CON",    E_NO_PROTECTION),
	DAPC_MM_ATTR("VCODESYS_SMI_LARB1",          E_NO_PROTECTION),
	DAPC_MM_ATTR("VCODESYS_VENC",               E_NO_PROTECTION),
	DAPC_MM_ATTR("VCODESYS_JPGENC",             E_NO_PROTECTION),

	/* 50 */
	DAPC_MM_ATTR("VCODESYS_VDEC_FULL_TOP",      E_NO_PROTECTION),
	DAPC_MM_ATTR("VCODESYS_MBIST_CTRL",         E_NO_PROTECTION),
};

static const struct MM_MD_DEVICE_INFO D_APC_MD_Devices[] = {
	/*            module,                                 AP permission            */

	/* 0 */
	DAPC_MD_ATTR("MDPERISYS_1",                 E_NO_PROTECTION),
	DAPC_MD_ATTR("MDPERISYS_2/MDTOP",           E_NO_PROTECTION),
	DAPC_MD_ATTR("MDMCUAPB",                    E_NO_PROTECTION),
	DAPC_MD_ATTR("MDCORESYS",                   E_NO_PROTECTION),
	DAPC_MD_ATTR("MDINFRA_APB_1",               E_NO_PROTECTION),
	DAPC_MD_ATTR("MDINFRA_APB_2",               E_SEC_RW_NONSEC_R_ONLY),
	DAPC_MD_ATTR("MML2",                        E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),

	/* 10 */
	DAPC_MD_ATTR("MD_INFRA",                    E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("-",                           E_FORBIDDEN),
	DAPC_MD_ATTR("uSIP Peripheral",             E_FORBIDDEN),
	DAPC_MD_ATTR("modeml1_ao_top_pwr_wrap",     E_FORBIDDEN),
	DAPC_MD_ATTR("md2gsys_pwr_wrap",            E_FORBIDDEN),
	DAPC_MD_ATTR("rxdfesys_pwr_wrap",           E_FORBIDDEN),

	/* 20 */
	DAPC_MD_ATTR("cssys_pwr_wrap",              E_FORBIDDEN),
	DAPC_MD_ATTR("txsys_pwr_wrap",              E_FORBIDDEN),
	DAPC_MD_ATTR("bigramsys (mem)",             E_FORBIDDEN),
	DAPC_MD_ATTR("md32scq share (mem)",         E_FORBIDDEN),
	DAPC_MD_ATTR("md32scq_vu01 (mem)",          E_FORBIDDEN),
	DAPC_MD_ATTR("peripheral (reg)",            E_FORBIDDEN),
	DAPC_MD_ATTR("rakesys_pwr_wrap",            E_FORBIDDEN),
	DAPC_MD_ATTR("rakesys_pwr_wrap",            E_FORBIDDEN),
	DAPC_MD_ATTR("brpsys_pwr_wrap",             E_FORBIDDEN),
	DAPC_MD_ATTR("brpsys_pwr_wrap",             E_FORBIDDEN),

	/* 30 */
	DAPC_MD_ATTR("dmcsys_pwr_wrap",             E_FORBIDDEN),
	DAPC_MD_ATTR("dmcsys_pwr_wrap",             E_FORBIDDEN),
};

/* SSPM does not exist in Zion, so set_master_transaction did not use. */
#if 0
static void set_master_transaction(DAPC_MASTER_TYPE master_type,
	unsigned int master_index, E_TRANSACTION transaction_type)
{
	unsigned int *base = NULL;
	unsigned int master_register_index = 0;
	unsigned int master_set_index = 0;

	master_register_index = master_index / (MOD_NO_IN_1_DEVAPC * 2);
	master_set_index = master_index % (MOD_NO_IN_1_DEVAPC * 2);

	if (master_type == E_DAPC_INFRA_MASTER && master_index <= MASTER_INFRA_MAX_INDEX)
		base = (unsigned int *)((size_t)DEVAPC_INFRA_MAS_SEC_0 + master_register_index * 4);
	else
		return;

	if (base != NULL) {
		if (transaction_type == NON_SECURE_TRANSACTION)
			devapc_writel(devapc_readl(base) & (0xFFFFFFFF ^ (0x1 << master_set_index)), base);
		else if (transaction_type == SECURE_TRANSACTION)
			devapc_writel(devapc_readl(base) | (0x1 << master_set_index), base);
	}
}
#endif

static void set_module_apc(enum DAPC_SLAVE_TYPE slave_type, unsigned int module,
	enum E_MASK_DOM domain_num, enum APC_ATTR permission_control)
{
	unsigned int *base = NULL;
	unsigned int apc_index = 0;
	unsigned int apc_set_index = 0;
	unsigned int clr_bit = 0;
	unsigned int set_bit = 0;

	if (permission_control != E_NO_PROTECTION && permission_control != E_SEC_RW_ONLY
			&& permission_control != E_SEC_RW_NONSEC_R_ONLY && permission_control != E_FORBIDDEN) {
		return;
	}

	apc_index = module / MOD_NO_IN_1_DEVAPC;
	apc_set_index = module % MOD_NO_IN_1_DEVAPC;
	clr_bit = 0xFFFFFFFF ^ (0x3 << (apc_set_index * 2));
	set_bit = permission_control << (apc_set_index * 2);

	/* Note: "clear_vio_status" is moved outside the function because
	 * config index and violation index may not be the same
	 */

	/* Note: "unmask_module_irq" is changed to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
	 * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded. Otherwise, nobody can
	 *           handle the violations before Device APC kernel driver is loaded.
	 */

	/* Do boundary check */

	if (slave_type == E_DAPC_INFRA_SLAVE && module <= SLAVE_INFRA_MAX_INDEX && domain_num <= E_DOMAIN_7)
		base = (unsigned int *)((size_t)DEVAPC_INFRA_D0_APC_0 + domain_num * 0x100 + apc_index * 4);
	else if (slave_type == E_DAPC_MM_SLAVE && module <= SLAVE_MM_MAX_INDEX && domain_num <= E_DOMAIN_3)
		base = (unsigned int *)((size_t)DEVAPC_MM_D0_APC_0 + domain_num * 0x100 + apc_index * 4);
	else if (slave_type == E_DAPC_MD_SLAVE && module <= SLAVE_MD_MAX_INDEX && domain_num <= E_DOMAIN_3)
		base = (unsigned int *)((size_t)DEVAPC_MD_D0_APC_0 + domain_num * 0x100 + apc_index * 4);
	else
		return;

	if (base != NULL) {
		devapc_writel(devapc_readl(base) & clr_bit, base);
		devapc_writel(devapc_readl(base) | set_bit, base);
	}
}

int start_devapc(void)
{
	unsigned int module_index = 0;

	int i = 0; /* register number */
#if !defined(SPD_no)
	unsigned int infra_tee_size = sizeof(D_APC_INFRA_Devices_TEE)/sizeof(struct INFRA_PERI_DEVICE_INFO);
#else
	unsigned int infra_size = sizeof(D_APC_INFRA_Devices)/sizeof(struct INFRA_PERI_DEVICE_INFO);
#endif
	unsigned int mm_size = sizeof(D_APC_MM_Devices)/sizeof(struct MM_MD_DEVICE_INFO);
	unsigned int md_size = sizeof(D_APC_MD_Devices)/sizeof(struct MM_MD_DEVICE_INFO);

	/* Enable Devapc */
	/* Lock DAPC to secure access only */
	devapc_writel(0x80000001, DEVAPC_INFRA_APC_CON);
	devapc_writel(0x80000001, DEVAPC_MM_APC_CON);
	devapc_writel(0x80000001, DEVAPC_MD_APC_CON);
	devapc_writel(0x80000000, DEVAPC_PD_INFRA_APC_CON);


	/* Initial Permission */
#if !defined(SPD_no)
	INFO("[DAPC] Walk TEE\n");
	for (module_index = 0; module_index < infra_tee_size; module_index++) {
		set_module_apc(E_DAPC_INFRA_SLAVE, module_index, E_DOMAIN_0,
			D_APC_INFRA_Devices_TEE[module_index].d0_permission);
		set_module_apc(E_DAPC_INFRA_SLAVE, module_index, E_DOMAIN_1,
			D_APC_INFRA_Devices_TEE[module_index].d1_permission);
		set_module_apc(E_DAPC_INFRA_SLAVE, module_index, E_DOMAIN_7,
			E_FORBIDDEN);
	}
#else
	INFO("[DAPC] Walk Non-TEE (ATF only)\n");
	for (module_index = 0; module_index < infra_size; module_index++) {
		set_module_apc(E_DAPC_INFRA_SLAVE, module_index, E_DOMAIN_0,
			D_APC_INFRA_Devices[module_index].d0_permission);
		set_module_apc(E_DAPC_INFRA_SLAVE, module_index, E_DOMAIN_1,
			D_APC_INFRA_Devices[module_index].d1_permission);
		set_module_apc(E_DAPC_INFRA_SLAVE, module_index, E_DOMAIN_7,
			E_FORBIDDEN);
	}
#endif

	/* [Important] You must open the access permission of AP=>MD1_PERIPHERALS
	 * and AP=>C2K_PERIPHERALS because it is the first level protection.
	 * Below is the MD 2nd level protection.
	 */

	for (module_index = 0; module_index < mm_size; module_index++) {
		set_module_apc(E_DAPC_MM_SLAVE, module_index, E_DOMAIN_0,
			D_APC_MM_Devices[module_index].d0_permission);
		set_module_apc(E_DAPC_MM_SLAVE, module_index, E_DOMAIN_3,
			E_FORBIDDEN);  /* Set others to forbidden */
	}

	for (module_index = 0; module_index < md_size; module_index++) {
		set_module_apc(E_DAPC_MD_SLAVE, module_index, E_DOMAIN_0,
			D_APC_MD_Devices[module_index].d0_permission);
		set_module_apc(E_DAPC_MD_SLAVE, module_index, E_DOMAIN_3,
			E_FORBIDDEN);  /* Set others to forbidden */
	}


	/* Print logs for debugging */

	/* d: domain, i: register number */
	for (i = 0; i < 12; i++) {
		INFO("[DAPC] (INFRA)D0_APC_%d = 0x%x, (INFRA)D1_APC_%d = 0x%x, (INFRA)D7_APC_%d = 0x%x\n",
				i, *(unsigned int *)((size_t)DEVAPC_INFRA_D0_APC_0 + i * 4),
				i, *(unsigned int *)((size_t)DEVAPC_INFRA_D0_APC_0 + 0x100 + i * 4),
				i, *(unsigned int *)((size_t)DEVAPC_INFRA_D0_APC_0 + 0x700 + i * 4));
	}

	for (i = 0; i < 6; i++) {
		INFO("[DAPC] (MM)D0_APC_%d = 0x%x, (MM)D3_APC_%d = 0x%x\n",
				i, *(unsigned int *)((size_t)DEVAPC_MM_D0_APC_0 + i * 4),
				i, *(unsigned int *)((size_t)DEVAPC_MM_D0_APC_0 + 0x300 + i * 4));
	}

	for (i = 0; i < 2; i++) {
		INFO("[DAPC] (MD)D0_APC_%d = 0x%x, (MD)D3_APC_%d = 0x%x\n",
				i, *(unsigned int *)((size_t)DEVAPC_MD_D0_APC_0 + i * 4),
				i, *(unsigned int *)((size_t)DEVAPC_MD_D0_APC_0 + 0x300 + i * 4));
	}

	INFO("[DAPC] (INFRA)MAS_SEC_0 = 0x%x\n", *DEVAPC_INFRA_MAS_SEC_0);

	for (i = 0; i < 10; i++) {
		INFO("[DAPC] (INFRA)VIO_STA_%d=0x%x\n", i,
			*(unsigned int *)((size_t)DEVAPC_PD_INFRA_VIO_STA_0 + i * 4));
	}

	INFO("\n[DAPC] MM_SEC_EN_0: 0x%x, MM_SEC_EN_1: 0x%x\n",
			*DEVAPC_MM_SEC_EN_0, *DEVAPC_MM_SEC_EN_1);


	/* Set Level 2 Secure CG (INFRACFG_AO) */

	devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON0) | SEJ_CG_PROTECT_BIT, INFRA_AO_SEC_CG_CON0);
	devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON1) | TRNG_CG_PROTECT_BIT, INFRA_AO_SEC_CG_CON1);
	devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON1) | DEVAPC_CG_PROTECT_BIT, INFRA_AO_SEC_CG_CON1);
	devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON1) | DXCC_SEC_CORE_CG_PROTECT_BIT, INFRA_AO_SEC_CG_CON1);
	devapc_writel(devapc_readl(INFRA_AO_SEC_CG_CON1) | DXCC_AO_CG_PROTECT_BIT, INFRA_AO_SEC_CG_CON1);

	INFO("[DAPC] INFRA_AO_SEC_CON = 0x%x\n", *INFRA_AO_SEC_CON);

	INFO("[DAPC] INFRA_AO_SEC_CG_CON 0: 0x%x, 1: 0x%x, 2: 0x%x, 3: 0x%x\n",
			*INFRA_AO_SEC_CG_CON0, *INFRA_AO_SEC_CG_CON1, *INFRA_AO_SEC_CG_CON2,
			*INFRA_AO_SEC_CG_CON3);

	INFO("[DAPC] Init Done\n");

	INFO("[DAPC] INFRA_APC_CON = 0x%x, MM_APC_CON = 0x%x, MD_APC_CON = 0x%x\n",
			*DEVAPC_INFRA_APC_CON, *DEVAPC_MM_APC_CON, *DEVAPC_MD_APC_CON);

	return 0;
}
