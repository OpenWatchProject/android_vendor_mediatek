#include <stdio.h>
#include "platform.h"
#include "plat_private.h"   //for atf_arg_t_ptr
#include "devapc.h"

#if !defined(SPD_no)
//For TEE
static DEVICE_INFO D_APC0_Devices_TEE[] = {
    /*0*/
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 },
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 },
    {"INFRA_AO_IO_CFG",                         E_L0,  E_L0 },
    {"INFRA_AO_PERICFG",                        E_L0,  E_L0 },
    {"INFRA_AO_DRAM_CONTROLLER",                E_L0,  E_L3 },
    {"INFRA_AO_GPIO",                           E_L0,  E_L0 },
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 },
    {"INFRA_AO_TOP_RGU",                        E_L0,  E_L0 },
    {"INFRA_AO_APXGPT",                         E_L0,  E_L3 },
    {"INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3 },

     /*10*/
    {"INFRA_AO_SEJ",                            E_L1,  E_L2 },
    {"INFRA_AO_AP_CIRQ_EINT",                   E_L0,  E_L3 },
    {"INFRA_AO_APMIXEDSYS_ FHCTL",              E_L0,  E_L0 },
    {"INFRA_AO_PMIC_WRAP",                      E_L0,  E_L0 },
    {"INFRA_AO_DEVICE_APC_AO",                  E_L1,  E_L3 },
    {"INFRA_AO_DDRPHY",                         E_L0,  E_L3 },
    {"INFRA_AO_KEYPAD",                         E_L0,  E_L3 },
    {"INFRA_AO_TOP_MISC",                       E_L0,  E_L0 },
    {"INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3 },
    {"INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3 },

     /*20*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 },
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 },
    {"INFRA_AO_AES_TOP0",                       E_L3,  E_L0 },
    {"INFRA_AO_AES_TOP1",                       E_L3,  E_L0 },
    {"INFRA_AO_MDEM_TEMP_SHARE",                E_L0,  E_L0 },
    {"INFRA_AO_SLEEP_CONTROL_PROCESSOR",        E_L0,  E_L0 },
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L0,  E_L3 },
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 },
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 },
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 },

     /*30*/
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 },
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 },
    {"INFRASYS_EFUSEC",                         E_L2,  E_L0 },
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 },
    {"INFRASYS_DEBUG_TRACKER",                  E_L0,  E_L3 },
    {"INFRASYS_CCI0_AP",                        E_L0,  E_L0 },
    {"INFRASYS_CCI0_MD",                        E_L0,  E_L0 },
    {"INFRASYS_CCI1_AP",                        E_L0,  E_L0 },
    {"INFRASYS_CCI1_MD",                        E_L0,  E_L0 },
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 },

    /*40*/
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 },
    {"INFRASYS_TRNG",                           E_L1,  E_L3 },
    {"INFRASYS_GCPU",                           E_L1,  E_L3 },
    {"INFRASYS_MD_CCIF_MD1",                    E_L0,  E_L0 },
    {"INFRASYS_GCE",                            E_L0,  E_L3 },
    {"INFRASYS_MD_CCIF_MD2",                    E_L0,  E_L0 },
    {"INFRASYS_RESERVE",                        E_L0,  E_L3 },
    {"INFRASYS_ANA_MIPI_DSI0",                  E_L0,  E_L0 },
    {"INFRASYS_ANA_MIPI_DSI1",                  E_L0,  E_L0 },
    {"INFRASYS_ANA_MIPI_CSI0",                  E_L0,  E_L0 },

    /*50*/
    {"INFRASYS_ANA_MIPI_CSI1",                  E_L0,  E_L0 },
    {"INFRASYS_ANA_MIPI_CSI2",                  E_L0,  E_L0 },
    {"INFRASYS_GPU_RSA",                        E_L0,  E_L3 },
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 },
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 },
    {"INFRASYS_MDSYS_INTF",                     E_L0,  E_L0 },
    {"INFRASYS_BPI_BIS_SLV0",                   E_L0,  E_L0 },
    {"INFRASYS_BPI_BIS_SLV1",                   E_L0,  E_L0 },
    {"INFRASYS_BPI_BIS_SLV2",                   E_L0,  E_L0 },
    {"INFRAYS_EMI_MPU_REG",                     E_L1,  E_L3 },

    /*60*/
    {"INFRAYS_DVFS_PROC",                       E_L0,  E_L3 },
    {"INFRASYS_DEBUGTOP",                       E_L0,  E_L0 },
    {"DMA",                                     E_L0,  E_L3 },
    {"AUXADC",                                  E_L0,  E_L0 },
    {"UART0",                                   E_L0,  E_L3 },
    {"UART1",                                   E_L0,  E_L3 },
    {"UART2",                                   E_L0,  E_L3 },
    {"UART3",                                   E_L0,  E_L3 },
    {"PWM",                                     E_L0,  E_L3 },
    {"I2C0",                                    E_L0,  E_L3 },

    /*70*/
    {"I2C1",                                    E_L0,  E_L3 },
    {"I2C2",                                    E_L0,  E_L3 },
    {"SPI0",                                    E_L0,  E_L3 }, /* For FPC protection */
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 },
    {"BTIF",                                    E_L0,  E_L3 },
    {"IRTX",                                    E_L0,  E_L3 },
    {"DISP_PWM",                                E_L0,  E_L3 },
    {"I2C3",                                    E_L0,  E_L3 },
    {"SPI1",                                    E_L0,  E_L3 },
    {"I2C4",                                    E_L0,  E_L3 },

    /*80*/
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"USB2.0",                                  E_L0,  E_L0 },
    {"USB2.0 SIF",                              E_L0,  E_L0 },
    {"AUDIO",                                   E_L0,  E_L0 }, /* Notice: The security configure index and violation index are not the same from index 84 to index 91 */
    {"MSDC0",                                   E_L0,  E_L3 },
    {"MSDC1",                                   E_L0,  E_L3 },
    {"MSDC2",                                   E_L0,  E_L3 },
    {"MSDC3",                                   E_L0,  E_L3 },
    {"USB3.0",                                  E_L0,  E_L0 },

    /*90*/
    {"USB3.0 SIF",                              E_L0,  E_L0 },
    {"USB3.0 SIF2",                             E_L0,  E_L0 },
    {"WCN AHB SLAVE",                           E_L0,  E_L0 },
    {"MD_PERIPHERALS",                          E_L0,  E_L3 },
    {"MD2_PERIPHERALS",                         E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"G3D_CONFIG_0",                            E_L0,  E_L3 },
    {"G3D_CONFIG_1",                            E_L0,  E_L3 },

    /*100*/
    {"G3D_CONFIG_2",                            E_L0,  E_L3 },
    {"G3D_CONFIG_3",                            E_L0,  E_L3 },
    {"G3D_CONFIG_4",                            E_L0,  E_L3 },
    {"G3D_POWER_CONTROL",                       E_L0,  E_L3 },
    {"MALI_CONFIG",                             E_L0,  E_L3 },
    {"MMSYS_CONFIG",                            E_L0,  E_L3 },
    {"MDP_RDMA",                                E_L0,  E_L3 },
    {"MDP_RSZ0",                                E_L0,  E_L3 },
    {"MDP_RSZ1",                                E_L0,  E_L3 },
    {"MDP_WDMA",                                E_L0,  E_L3 },

    /*110*/
    {"MDP_WROT",                                E_L0,  E_L3 },
    {"MDP_TDSHP",                               E_L0,  E_L3 },
    {"MDP_COLOR",                               E_L0,  E_L3 },
    {"DISP_OVL0",                               E_L0,  E_L3 },
    {"DISP_OVL1",                               E_L0,  E_L3 },
    {"DISP_RDMA0",                              E_L0,  E_L3 },
    {"DISP_RDMA1",                              E_L0,  E_L3 },
    {"DISP_WDMA0",                              E_L0,  E_L3 },
    {"DISP_COLOR",                              E_L0,  E_L3 },
    {"DISP_CCORR",                              E_L0,  E_L3 },

    /*120*/
    {"DISP_AAL",                                E_L0,  E_L3 },
    {"DISP_GAMMA",                              E_L0,  E_L3 },
    {"DISP_DITHER",                             E_L0,  E_L3 },
    {"DSI",                                     E_L0,  E_L3 },
    {"DPI",                                     E_L0,  E_L3 },
    {"MM_MUTEX",                                E_L0,  E_L3 },
    {"SMI_LARB0",                               E_L0,  E_L3 },
    {"SMI_COMMON",                              E_L0,  E_L3 },
    {"DISP_WDMA1",                              E_L0,  E_L3 },
    {"DISP_OVL0_2L",                            E_L0,  E_L3 },

    /*130*/
    {"DISP_OVL1_2L",                            E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 },
    {"SMI_LARB2",                               E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"CAM0",                                    E_L0,  E_L3 },
    {"CAM1",                                    E_L0,  E_L3 },
    {"CAM2",                                    E_L0,  E_L3 },

    /*140*/
    {"CAM3",                                    E_L0,  E_L3 },
    {"SENINF",                                  E_L0,  E_L3 },
    {"CAMSV",                                   E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"FD",                                      E_L0,  E_L3 },
    {"MIPI_RX",                                 E_L0,  E_L3 },
    {"CAM0",                                    E_L0,  E_L3 },
    {"CAM2",                                    E_L0,  E_L3 },
    {"CAM3",                                    E_L0,  E_L3 },
    {"VDEC_GLOBAL_CON",                         E_L0,  E_L3 },

    /*150*/
    {"SMI_LARB1",                               E_L0,  E_L3 },
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 },
    {"VENC_GLOBAL_CON",                         E_L0,  E_L3 },
    {"SMI_LARB3",                               E_L0,  E_L3 },
    {"VENC",                                    E_L0,  E_L3 },
    {"JPEGENC",                                 E_L0,  E_L3 },
    {"JPEGDEC",                                 E_L0,  E_L3 },

};

#else

//For ATF
static DEVICE_INFO D_APC0_Devices[] = {
    /*0*/
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 },
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 },
    {"INFRA_AO_IO_CFG",                         E_L0,  E_L0 },
    {"INFRA_AO_PERICFG",                        E_L0,  E_L0 },
    {"INFRA_AO_DRAM_CONTROLLER",                E_L0,  E_L3 },
    {"INFRA_AO_GPIO",                           E_L0,  E_L0 },
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 },
    {"INFRA_AO_TOP_RGU",                        E_L0,  E_L0 },
    {"INFRA_AO_APXGPT",                         E_L0,  E_L3 },
    {"INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3 },

     /*10*/
    {"INFRA_AO_SEJ",                            E_L0,  E_L2 },
    {"INFRA_AO_AP_CIRQ_EINT",                   E_L0,  E_L3 },
    {"INFRA_AO_APMIXEDSYS_ FHCTL",              E_L0,  E_L0 },
    {"INFRA_AO_PMIC_WRAP",                      E_L0,  E_L0 },
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 },
    {"INFRA_AO_DDRPHY",                         E_L0,  E_L3 },
    {"INFRA_AO_KEYPAD",                         E_L0,  E_L3 },
    {"INFRA_AO_TOP_MISC",                       E_L0,  E_L0 },
    {"INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3 },
    {"INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3 },

     /*20*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 },
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 },
    {"INFRA_AO_AES_TOP0",                       E_L3,  E_L0 },
    {"INFRA_AO_AES_TOP1",                       E_L3,  E_L0 },
    {"INFRA_AO_MDEM_TEMP_SHARE",                E_L0,  E_L0 },
    {"INFRA_AO_SLEEP_CONTROL_PROCESSOR",        E_L0,  E_L0 },
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L0,  E_L3 },
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 },
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 },
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 },

     /*30*/
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 },
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 },
    {"INFRASYS_EFUSEC",                         E_L2,  E_L0 },
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 },
    {"INFRASYS_DEBUG_TRACKER",                  E_L0,  E_L3 },
    {"INFRASYS_CCI0_AP",                        E_L0,  E_L0 },
    {"INFRASYS_CCI0_MD",                        E_L0,  E_L0 },
    {"INFRASYS_CCI1_AP",                        E_L0,  E_L0 },
    {"INFRASYS_CCI1_MD",                        E_L0,  E_L0 },
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 },

    /*40*/
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 },
    {"INFRASYS_TRNG",                           E_L0,  E_L3 },
    {"INFRASYS_GCPU",                           E_L0,  E_L3 },
    {"INFRASYS_MD_CCIF_MD1",                    E_L0,  E_L0 },
    {"INFRASYS_GCE",                            E_L0,  E_L3 },
    {"INFRASYS_MD_CCIF_MD2",                    E_L0,  E_L0 },
    {"INFRASYS_RESERVE",                        E_L0,  E_L3 },
    {"INFRASYS_ANA_MIPI_DSI0",                  E_L0,  E_L0 },
    {"INFRASYS_ANA_MIPI_DSI1",                  E_L0,  E_L0 },
    {"INFRASYS_ANA_MIPI_CSI0",                  E_L0,  E_L0 },

    /*50*/
    {"INFRASYS_ANA_MIPI_CSI1",                  E_L0,  E_L0 },
    {"INFRASYS_ANA_MIPI_CSI2",                  E_L0,  E_L0 },
    {"INFRASYS_GPU_RSA",                        E_L0,  E_L3 },
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 },
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 },
    {"INFRASYS_MDSYS_INTF",                     E_L0,  E_L0 },
    {"INFRASYS_BPI_BIS_SLV0",                   E_L0,  E_L0 },
    {"INFRASYS_BPI_BIS_SLV1",                   E_L0,  E_L0 },
    {"INFRASYS_BPI_BIS_SLV2",                   E_L0,  E_L0 },
    {"INFRAYS_EMI_MPU_REG",                     E_L1,  E_L3 },

    /*60*/
    {"INFRAYS_DVFS_PROC",                       E_L0,  E_L3 },
    {"INFRASYS_DEBUGTOP",                       E_L0,  E_L0 },
    {"DMA",                                     E_L0,  E_L3 },
    {"AUXADC",                                  E_L0,  E_L0 },
    {"UART0",                                   E_L0,  E_L3 },
    {"UART1",                                   E_L0,  E_L3 },
    {"UART2",                                   E_L0,  E_L3 },
    {"UART3",                                   E_L0,  E_L3 },
    {"PWM",                                     E_L0,  E_L3 },
    {"I2C0",                                    E_L0,  E_L3 },

    /*70*/
    {"I2C1",                                    E_L0,  E_L3 },
    {"I2C2",                                    E_L0,  E_L3 },
    {"SPI0",                                    E_L0,  E_L3 },
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 },
    {"BTIF",                                    E_L0,  E_L3 },
    {"IRTX",                                    E_L0,  E_L3 },
    {"DISP_PWM",                                E_L0,  E_L3 },
    {"I2C3",                                    E_L0,  E_L3 },
    {"SPI1",                                    E_L0,  E_L3 },
    {"I2C4",                                    E_L0,  E_L3 },

    /*80*/
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"USB2.0",                                  E_L0,  E_L0 },
    {"USB2.0 SIF",                              E_L0,  E_L0 },
    {"AUDIO",                                   E_L0,  E_L0 }, /* Notice: The security configure index and violation index are not the same from index 84 to index 91 */
    {"MSDC0",                                   E_L0,  E_L3 },
    {"MSDC1",                                   E_L0,  E_L3 },
    {"MSDC2",                                   E_L0,  E_L3 },
    {"MSDC3",                                   E_L0,  E_L3 },
    {"USB3.0",                                  E_L0,  E_L0 },

    /*90*/
    {"USB3.0 SIF",                              E_L0,  E_L0 },
    {"USB3.0 SIF2",                             E_L0,  E_L0 },
    {"WCN AHB SLAVE",                           E_L0,  E_L0 },
    {"MD_PERIPHERALS",                          E_L0,  E_L3 },
    {"MD2_PERIPHERALS",                         E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"G3D_CONFIG_0",                            E_L0,  E_L3 },
    {"G3D_CONFIG_1",                            E_L0,  E_L3 },

    /*100*/
    {"G3D_CONFIG_2",                            E_L0,  E_L3 },
    {"G3D_CONFIG_3",                            E_L0,  E_L3 },
    {"G3D_CONFIG_4",                            E_L0,  E_L3 },
    {"G3D_POWER_CONTROL",                       E_L0,  E_L3 },
    {"MALI_CONFIG",                             E_L0,  E_L3 },
    {"MMSYS_CONFIG",                            E_L0,  E_L3 },
    {"MDP_RDMA",                                E_L0,  E_L3 },
    {"MDP_RSZ0",                                E_L0,  E_L3 },
    {"MDP_RSZ1",                                E_L0,  E_L3 },
    {"MDP_WDMA",                                E_L0,  E_L3 },

    /*110*/
    {"MDP_WROT",                                E_L0,  E_L3 },
    {"MDP_TDSHP",                               E_L0,  E_L3 },
    {"MDP_COLOR",                               E_L0,  E_L3 },
    {"DISP_OVL0",                               E_L0,  E_L3 },
    {"DISP_OVL1",                               E_L0,  E_L3 },
    {"DISP_RDMA0",                              E_L0,  E_L3 },
    {"DISP_RDMA1",                              E_L0,  E_L3 },
    {"DISP_WDMA0",                              E_L0,  E_L3 },
    {"DISP_COLOR",                              E_L0,  E_L3 },
    {"DISP_CCORR",                              E_L0,  E_L3 },

    /*120*/
    {"DISP_AAL",                                E_L0,  E_L3 },
    {"DISP_GAMMA",                              E_L0,  E_L3 },
    {"DISP_DITHER",                             E_L0,  E_L3 },
    {"DSI",                                     E_L0,  E_L3 },
    {"DPI",                                     E_L0,  E_L3 },
    {"MM_MUTEX",                                E_L0,  E_L3 },
    {"SMI_LARB0",                               E_L0,  E_L3 },
    {"SMI_COMMON",                              E_L0,  E_L3 },
    {"DISP_WDMA1",                              E_L0,  E_L3 },
    {"DISP_OVL0_2L",                            E_L0,  E_L3 },

    /*130*/
    {"DISP_OVL1_2L",                            E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 },
    {"SMI_LARB2",                               E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"CAM0",                                    E_L0,  E_L3 },
    {"CAM1",                                    E_L0,  E_L3 },
    {"CAM2",                                    E_L0,  E_L3 },

    /*140*/
    {"CAM3",                                    E_L0,  E_L3 },
    {"SENINF",                                  E_L0,  E_L3 },
    {"CAMSV",                                   E_L0,  E_L3 },
    {"RESERVE",                                 E_L0,  E_L3 },
    {"FD",                                      E_L0,  E_L3 },
    {"MIPI_RX",                                 E_L0,  E_L3 },
    {"CAM0",                                    E_L0,  E_L3 },
    {"CAM2",                                    E_L0,  E_L3 },
    {"CAM3",                                    E_L0,  E_L3 },
    {"VDEC_GLOBAL_CON",                         E_L0,  E_L3 },

    /*150*/
    {"SMI_LARB1",                               E_L0,  E_L3 },
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 },
    {"VENC_GLOBAL_CON",                         E_L0,  E_L3 },
    {"SMI_LARB3",                               E_L0,  E_L3 },
    {"VENC",                                    E_L0,  E_L3 },
    {"JPEGENC",                                 E_L0,  E_L3 },
    {"JPEGDEC",                                 E_L0,  E_L3 },
};

#endif

#if 0
static void unmask_module_irq(unsigned int module)
{

    /* Change to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */
}
#endif

static void clear_vio_status(unsigned int module)
{
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;

    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

    *((unsigned int*)((size_t)DEVAPC_D0_VIO_STA_0 + (apc_index * 4))) = (0x1 << apc_bit_index);
}

void clear_sramrom_violation(void)
{
    /* Clear the internal SRAMROM violation only when internal SRAMROM has violation */
    if (*SRAMROM_SEC_VIO_STA & 0x1) {
        printf("[DEVAPC] (Before)SRAMROM VIO_STA: 0x%x, VIO_ADDR: 0x%x\n", *SRAMROM_SEC_VIO_STA, *SRAMROM_SEC_VIO_ADDR);

        /* You can clear Device APC slave:SRAMROM violation only after you clear the violation in internal SRAMROM register */
        writel(0x1, SRAMROM_SEC_VIO_CLR);

        printf("[DEVAPC] (After)SRAMROM VIO_STA: 0x%x, VIO_ADDR: 0x%x\n", *SRAMROM_SEC_VIO_STA, *SRAMROM_SEC_VIO_ADDR);
    }

    clear_vio_status(SLAVE_SRAMROM_INDEX);
}

int set_master_transaction(unsigned int master_index ,unsigned int transaction_type)
{
    volatile unsigned int* base = 0;
    unsigned int set_bit = 0;


    base = (unsigned int*) ((size_t)DEVAPC_MAS_SEC_0);

    if (master_index > 31)
        return -1;

    if (transaction_type == 0){
        set_bit = ~(1 << master_index);
        writel(readl(base) & set_bit, base);
    }
    else if (transaction_type == 1){
        set_bit = 1 << master_index;
        writel(readl(base) | set_bit, base);
    }
    else
        return -2;

    return 0;
}

void set_module_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control)
{
    volatile unsigned int* base = NULL;

    unsigned int clr_bit = 0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2);
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    clear_vio_status(module);

#if 0
    /* Change to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */
    unmask_module_irq(module);
#endif

    if (domain_num == E_DOMAIN_0)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D0_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_1)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D1_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_2)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D2_APC_0 + (module/16) * 4);
    }
    else if (domain_num == E_DOMAIN_3)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D3_APC_0 + (module/16) * 4);
    }

    if (base != NULL) {
        writel(readl(base) & ~clr_bit, base);
        writel(readl(base) | set_bit, base);
    }
}

int start_devapc()
{
    int module_index = 0;
	int i = 0;
    int d = 0;

    /* Enable Devapc */
    writel(0x80000000, DEVAPC_VIO_DBG0); // clear apc0 dbg info if any

    writel(readl(DEVAPC_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC_APC_CON);

    writel(readl(DEVAPC_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC_PD_APC_CON);

    clear_sramrom_violation();

    /* Set Necessary Masters to Secure Transaction */
    set_master_transaction(MASTER_SPM_PDN, SECURE_TRANSACTION); /* SPM_PDN will access MCUSYS in Normal World, but MCUSYS is secure r/w and non-secure read-only. So this setting is necessary. */


    /* Initial Permission */

#if !defined(SPD_no)
	printf("[DAPC] Walk TEE\n");
	for (module_index = 0; module_index < (sizeof(D_APC0_Devices_TEE)/sizeof(DEVICE_INFO)); module_index++)
	{
		set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices_TEE[module_index].d0_permission);

		set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices_TEE[module_index].d1_permission);
	}
#else
	printf("[DAPC] Walk ATF\n");
	for (module_index = 0; module_index < (sizeof(D_APC0_Devices)/sizeof(DEVICE_INFO)); module_index++)
	{
		set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices[module_index].d0_permission);

		set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices[module_index].d1_permission);
	}
#endif

	//d: domain, i: register number
    for (d=0; d<=1; d++) {
        for (i=0; i<=10; i++) {
            printf("[DAPC] reg%d-%d = 0x%x\n", d, i, *((unsigned int*)((size_t)DEVAPC_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    printf("[DAPC] MAS_SEC_0 = 0x%x\n", *DEVAPC_MAS_SEC_0);

	printf("[DAPC] Current VIO_MASK 0:0x%x, 1:0x%x, 2:0x%x, 3:0x%x, 4:0x%x, 5:0x%x\n",
       *DEVAPC_D0_VIO_MASK_0, *DEVAPC_D0_VIO_MASK_1,
       *DEVAPC_D0_VIO_MASK_2, *DEVAPC_D0_VIO_MASK_3,
       *DEVAPC_D0_VIO_MASK_4, *DEVAPC_D0_VIO_MASK_5);

    printf("[DAPC] Current VIO_STA 0:0x%x, 1:0x%x, 2:0x%x, 3:0x%x, 4:0x%x, 5:0x%x\n",
       *DEVAPC_D0_VIO_STA_0, *DEVAPC_D0_VIO_STA_1,
       *DEVAPC_D0_VIO_STA_2, *DEVAPC_D0_VIO_STA_3,
       *DEVAPC_D0_VIO_STA_4, *DEVAPC_D0_VIO_STA_5);

    /*Lock DAPC to secure access only*/
    writel(readl(DEVAPC_APC_CON) | (0x1), DEVAPC_APC_CON);

    /*Set Level 2 secure*/
    writel(readl(INFRA_AO_SEC_CG_CON0) | (SEJ_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON0);
    writel(readl(INFRA_AO_SEC_CG_CON1) | (TRNG_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON1);

    printf("[DAPC] Init Done\n");
    printf("[DAPC] APC_CON = 0x%x\n", *DEVAPC_APC_CON);

    return 0;

}
