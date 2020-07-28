#include <stdio.h>
#include "platform.h"
#include "plat_private.h"   //for atf_arg_t_ptr
#include "devapc.h"


#if defined(MACH_TYPE_MT6735)

//For TEE
static DEVICE_INFO D_APC0_Devices_TEE[] = {
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_GPT",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_SEJ",                            E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_RSVD",                           E_L0,  E_L3 ,  E_L0, E_L0},
     /*10*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L2,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EFUSEC",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ,  E_L0, E_L0},
     /*20*/
    {"INFRASYS_MCU_BIU_CONFIGURATION",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_AP_MIXED_CONTROL_REG",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_MD_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"RSVD",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_TRNG",                           E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L0 ,  E_L0, E_L0},
     /*30*/
    {"INFRA_AO_TOP_LEVEL_REST_GENERATOR",       E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DDRPHY",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MIPI_RX_ANA",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCPU",                           E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCE",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_AP_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_MD_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 ,  E_L0, E_L0},
     /*40*/
    {"INFRASYS_MD2MD_CCIF0",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MD2MD_CCIF1",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MDSYSINTF",                      E_L0,  E_L3 ,  E_L0, E_L0},
    {"DEGBUGSYS",                               E_L0,  E_L0 ,  E_L0, E_L0},
    {"DMA",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"AUXADC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART3",                                   E_L0,  E_L3 ,  E_L0, E_L0},
     /*50*/
    {"PWM",                                     E_L0,  E_L0 ,  E_L0, E_L0},
    {"I2C0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C1",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C2",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"SPI0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"BTIF",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART4",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_PWM",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C3",                                    E_L0,  E_L3 ,  E_L0, E_L0},
     /*60*/
    {"IRDA",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"IR_TX",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB2.0",                                  E_L0,  E_L0 ,  E_L0, E_L0},
    {"USB2.0 SIF",                              E_L0,  E_L0 ,  E_L0, E_L0},
    {"AUDIO",                                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"MSDC0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB3.0",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"WCN_AHB_SLAVE",                           E_L0,  E_L0 ,  E_L0, E_L0},
     /*70*/
    {"MD2_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"MD3_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"G3D_CONFIG",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MALI",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"MMSYS_CONFIG",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ0",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ1",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WROT",                                E_L0,  E_L3 ,  E_L0, E_L0},
     /*80*/
    {"MDP_TDSHP",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_OVL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA0",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA1",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_WDMA",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_COLOR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_CCORR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_AAL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_GAMMA",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_DITHER",                             E_L0,  E_L3 ,  E_L0, E_L0},
     /*90*/
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DSI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"DPI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MM_MUTEX",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_COMMON",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MIPI_TX_CONFIG",                          E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SMI_LARB2",                        E_L0,  E_L3 ,  E_L0, E_L0},
     /*100*/
    {"IMGSYS_CAM1",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM2",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM3",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM4",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SENINF",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAMSV",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_FDVT",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM5",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM6",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM7",                             E_L0,  E_L3 ,  E_L0, E_L0},
     /*110*/
    {"VDECSYS_GLOBAL_CONFIGURATION",            E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC_GLOBAL_CON",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB3",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_ENC",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_DEC",                                E_L0,  E_L3 ,  E_L0, E_L0},
  };


//For ATF
static DEVICE_INFO D_APC0_Devices[] = {
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_GPT",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_SEJ",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_RSVD",                           E_L0,  E_L3 ,  E_L0, E_L0},
     /*10*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L2,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EFUSEC",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ,  E_L0, E_L0},
     /*20*/
    {"INFRASYS_MCU_BIU_CONFIGURATION",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_AP_MIXED_CONTROL_REG",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_MD_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"RSVD",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_TRNG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L0 ,  E_L0, E_L0},
     /*30*/
    {"INFRA_AO_TOP_LEVEL_REST_GENERATOR",       E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DDRPHY",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MIPI_RX_ANA",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCPU",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCE",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_AP_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_MD_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 ,  E_L0, E_L0},
     /*40*/
    {"INFRASYS_MD2MD_CCIF0",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MD2MD_CCIF1",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MDSYSINTF",                      E_L0,  E_L3 ,  E_L0, E_L0},
    {"DEGBUGSYS",                               E_L0,  E_L0 ,  E_L0, E_L0},
    {"DMA",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"AUXADC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART3",                                   E_L0,  E_L3 ,  E_L0, E_L0},
     /*50*/
    {"PWM",                                     E_L0,  E_L0 ,  E_L0, E_L0},
    {"I2C0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C1",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C2",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"SPI0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"BTIF",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART4",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_PWM",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C3",                                    E_L0,  E_L3 ,  E_L0, E_L0},
     /*60*/
    {"IRDA",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"IR_TX",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB2.0",                                  E_L0,  E_L0 ,  E_L0, E_L0},
    {"USB2.0 SIF",                              E_L0,  E_L0 ,  E_L0, E_L0},
    {"AUDIO",                                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"MSDC0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB3.0",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"WCN_AHB_SLAVE",                           E_L0,  E_L0 ,  E_L0, E_L0},
     /*70*/
    {"MD2_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"MD3_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"G3D_CONFIG",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MALI",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"MMSYS_CONFIG",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ0",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ1",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WROT",                                E_L0,  E_L3 ,  E_L0, E_L0},
     /*80*/
    {"MDP_TDSHP",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_OVL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA0",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA1",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_WDMA",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_COLOR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_CCORR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_AAL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_GAMMA",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_DITHER",                             E_L0,  E_L3 ,  E_L0, E_L0},
     /*90*/
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DSI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"DPI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MM_MUTEX",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_COMMON",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MIPI_TX_CONFIG",                          E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SMI_LARB2",                        E_L0,  E_L3 ,  E_L0, E_L0},
     /*100*/
    {"IMGSYS_CAM1",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM2",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM3",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM4",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SENINF",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAMSV",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_FDVT",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM5",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM6",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM7",                             E_L0,  E_L3 ,  E_L0, E_L0},
     /*110*/
    {"VDECSYS_GLOBAL_CONFIGURATION",            E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC_GLOBAL_CON",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB3",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_ENC",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_DEC",                                E_L0,  E_L3 ,  E_L0, E_L0},
};

#elif defined(MACH_TYPE_MT6735M)

//For TEE
static DEVICE_INFO D_APC0_Devices_TEE[] = {
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_GPT",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_SEJ",                            E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_RSVD",                           E_L0,  E_L3 ,  E_L0, E_L0},
     /*10*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L2,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EFUSEC",                         E_L0,  E_L0 ,  E_L0, E_L0},
     /*20*/
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MCU_BIU_CONFIGURATION",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_AP_MIXED_CONTROL_REG",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_MD_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"RSVD",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_TRNG",                           E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ,  E_L0, E_L0},
     /*30*/
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_REST_GENERATOR",       E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_DDRPHY",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MIPI_RX_ANA",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCPU",                           E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCE",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_AP_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_MD_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 ,  E_L0, E_L0},
     /*40*/
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MDSYSINTF",                      E_L0,  E_L3 ,  E_L0, E_L0},
    {"DEGBUGSYS",                               E_L0,  E_L0 ,  E_L0, E_L0},
    {"DMA",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"AUXADC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART3",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"PWM",                                     E_L0,  E_L0 ,  E_L0, E_L0},
     /*50*/
    {"I2C0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C1",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C2",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"SPI0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"BTIF",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_PWM",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C3",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"IRDA",                                    E_L0,  E_L3 ,  E_L0, E_L0},
     /*60*/
    {"IR_TX",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB2.0",                                  E_L0,  E_L0 ,  E_L0, E_L0},
    {"USB2.0 SIF",                              E_L0,  E_L0 ,  E_L0, E_L0},
    {"AUDIO",                                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"MSDC0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"RESERVE",                                 E_L0,  E_L3 ,  E_L0, E_L0},
    {"RESERVE",                                 E_L0,  E_L3 ,  E_L0, E_L0},
    {"WCN_AHB_SLAVE",                           E_L0,  E_L0 ,  E_L0, E_L0},
    {"MD_PERIPHERALS",                          E_L0,  E_L3 ,  E_L0, E_L0},
     /*70*/
    {"RESERVE",                                 E_L0,  E_L3 ,  E_L0, E_L0},
    {"G3D_CONFIG",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MALI",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"MMSYS_CONFIG",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ0",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ1",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WROT",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_TDSHP",                               E_L0,  E_L3 ,  E_L0, E_L0},
     /*80*/
    {"DISP_OVL0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_OVL1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA0",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA1",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_WDMA",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_COLOR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_CCORR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_AAL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_GAMMA",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_DITHER",                             E_L0,  E_L3 ,  E_L0, E_L0},
     /*90*/
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DPI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"DSI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MM_MUTEX",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_COMMON",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MIPI_TX_CONFIG",                          E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SMI_LARB2",                        E_L0,  E_L3 ,  E_L0, E_L0},
     /*100*/
    {"IMGSYS_CAM1",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM2",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SENINF",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPGENC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_GLOBAL_CON",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 ,  E_L0, E_L0},

};

//For ATF
static DEVICE_INFO D_APC0_Devices[] = {
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_GPT",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_SEJ",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_RSVD",                           E_L0,  E_L3 ,  E_L0, E_L0},
    /*10*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L2,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EFUSEC",                         E_L0,  E_L0 ,  E_L0, E_L0},
    /*20*/
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MCU_BIU_CONFIGURATION",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_AP_MIXED_CONTROL_REG",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_MD_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"RSVD",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_TRNG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ,  E_L0, E_L0},
    /*30*/
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_REST_GENERATOR",       E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_DDRPHY",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MIPI_RX_ANA",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCPU",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCE",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_AP_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_MD_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 ,  E_L0, E_L0},
    /*40*/
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MDSYSINTF",                      E_L0,  E_L3 ,  E_L0, E_L0},
    {"DEGBUGSYS",                               E_L0,  E_L0 ,  E_L0, E_L0},
    {"DMA",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"AUXADC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART3",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"PWM",                                     E_L0,  E_L0 ,  E_L0, E_L0},
    /*50*/
    {"I2C0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C1",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C2",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"SPI0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"BTIF",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_PWM",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C3",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"IRDA",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    /*60*/
    {"IR_TX",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB2.0",                                  E_L0,  E_L0 ,  E_L0, E_L0},
    {"USB2.0 SIF",                              E_L0,  E_L0 ,  E_L0, E_L0},
    {"AUDIO",                                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"MSDC0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"RESERVE",                                 E_L0,  E_L3 ,  E_L0, E_L0},
    {"RESERVE",                                 E_L0,  E_L3 ,  E_L0, E_L0},
    {"WCN_AHB_SLAVE",                           E_L0,  E_L0 ,  E_L0, E_L0},
    {"MD_PERIPHERALS",                          E_L0,  E_L3 ,  E_L0, E_L0},
    /*70*/
    {"RESERVE",                                 E_L0,  E_L3 ,  E_L0, E_L0},
    {"G3D_CONFIG",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MALI",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"MMSYS_CONFIG",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ0",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ1",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WROT",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_TDSHP",                               E_L0,  E_L3 ,  E_L0, E_L0},
    /*80*/
    {"DISP_OVL0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_OVL1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA0",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA1",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_WDMA",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_COLOR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_CCORR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_AAL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_GAMMA",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_DITHER",                             E_L0,  E_L3 ,  E_L0, E_L0},
    /*90*/
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DPI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"DSI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MM_MUTEX",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_COMMON",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MIPI_TX_CONFIG",                          E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SMI_LARB2",                        E_L0,  E_L3 ,  E_L0, E_L0},
    /*100*/
    {"IMGSYS_CAM1",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM2",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SENINF",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPGENC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_GLOBAL_CON",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 ,  E_L0, E_L0},
};

#elif defined(MACH_TYPE_MT6753)

//For TEE
static DEVICE_INFO D_APC0_Devices_TEE[] = {
    /*0*/
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_GPT",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_SEJ",                            E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_RSVD",                           E_L0,  E_L3 ,  E_L0, E_L0},
     /*10*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L2,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EFUSEC",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ,  E_L0, E_L0},
     /*20*/
    {"INFRASYS_MCU_BIU_CONFIGURATION",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_AP_MIXED_CONTROL_REG",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_MD_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"RSVD",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_TRNG",                           E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L0 ,  E_L0, E_L0},
     /*30*/
    {"INFRA_AO_TOP_LEVEL_REST_GENERATOR",       E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DDRPHY",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MIPI_RX_ANA",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCPU",                           E_L1,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCE",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_AP_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_MD_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 ,  E_L0, E_L0},
     /*40*/
    {"INFRASYS_MD2MD_CCIF0",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MD2MD_CCIF1",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MDSYSINTF",                      E_L0,  E_L3 ,  E_L0, E_L0},
    {"DEGBUGSYS",                               E_L0,  E_L0 ,  E_L0, E_L0},
    {"DMA",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"AUXADC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART3",                                   E_L0,  E_L3 ,  E_L0, E_L0},
     /*50*/
    {"PWM",                                     E_L0,  E_L0 ,  E_L0, E_L0},
    {"I2C0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C1",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C2",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"SPI0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"BTIF",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART4",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_PWM",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C3",                                    E_L0,  E_L3 ,  E_L0, E_L0},
     /*60*/
    {"IRDA",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"IR_TX",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C4",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB2.0",                                  E_L0,  E_L0 ,  E_L0, E_L0},
    {"USB2.0 SIF",                              E_L0,  E_L0 ,  E_L0, E_L0},
    {"AUDIO",                                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"MSDC0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB3.0",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    /*70*/
    {"WCN_AHB_SLAVE",                           E_L0,  E_L0 ,  E_L0, E_L0},
    {"MD2_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"MD3_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"G3D_CONFIG",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MALI",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"MMSYS_CONFIG",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ0",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ1",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    /*80*/
    {"MDP_WROT",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_TDSHP",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_OVL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA0",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA1",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_WDMA",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_COLOR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_CCORR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_AAL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_GAMMA",                              E_L0,  E_L3 ,  E_L0, E_L0},
    /*90*/
    {"DISP_DITHER",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DSI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"DPI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MM_MUTEX",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_COMMON",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MIPI_TX_CONFIG",                          E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    /*100*/
    {"IMGSYS_SMI_LARB2",                        E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM1",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM2",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM3",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM4",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SENINF",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAMSV",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_FDVT",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM5",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM6",                             E_L0,  E_L3 ,  E_L0, E_L0},
    /*110*/
    {"IMGSYS_CAM7",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDECSYS_GLOBAL_CONFIGURATION",            E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC_GLOBAL_CON",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB3",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_ENC",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_DEC",                                E_L0,  E_L3 ,  E_L0, E_L0},
  };


//For ATF
static DEVICE_INFO D_APC0_Devices[] = {
    /*0*/
    {"INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PMIC_WRAP_CONTROL_REG",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_PERISYS_CONFIG_REGS",            E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_KPAD_CONTROL_REG",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_GPT",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_APMCU_EINT_CONTROLLER",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_SEJ",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_RSVD",                           E_L0,  E_L3 ,  E_L0, E_L0},
     /*10*/
    {"INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MCUSYS_CONFIG_REG",              E_L2,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CONTROL_REG",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_EFUSEC",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3 ,  E_L0, E_L0},
     /*20*/
    {"INFRASYS_MCU_BIU_CONFIGURATION",          E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_AP_MIXED_CONTROL_REG",           E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_AP_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CA7_MD_CCIF",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"RSVD",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_TRNG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_GPIO1_CONTROLLER",               E_L0,  E_L0 ,  E_L0, E_L0},
     /*30*/
    {"INFRA_AO_TOP_LEVEL_REST_GENERATOR",       E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DDRPHY",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_DRAM_CONTROLLER",                E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_MIPI_RX_ANA",                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCPU",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_GCE",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_AP_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CCIF_MD_1",                      E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0 ,  E_L0, E_L0},
     /*40*/
    {"INFRASYS_MD2MD_CCIF0",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MD2MD_CCIF1",                    E_L0,  E_L0 ,  E_L0, E_L0},
    {"INFRASYS_MDSYSINTF",                      E_L0,  E_L3 ,  E_L0, E_L0},
    {"DEGBUGSYS",                               E_L0,  E_L0 ,  E_L0, E_L0},
    {"DMA",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"AUXADC",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART3",                                   E_L0,  E_L3 ,  E_L0, E_L0},
     /*50*/
    {"PWM",                                     E_L0,  E_L0 ,  E_L0, E_L0},
    {"I2C0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C1",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C2",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"SPI0",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"PTP_THERMAL_CTL",                         E_L0,  E_L0 ,  E_L0, E_L0},
    {"BTIF",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"UART4",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_PWM",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C3",                                    E_L0,  E_L3 ,  E_L0, E_L0},
     /*60*/
    {"IRDA",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"IR_TX",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"I2C4",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB2.0",                                  E_L0,  E_L0 ,  E_L0, E_L0},
    {"USB2.0 SIF",                              E_L0,  E_L0 ,  E_L0, E_L0},
    {"AUDIO",                                   E_L0,  E_L0 ,  E_L0, E_L0},
    {"MSDC0",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC1",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"MSDC2",                                   E_L0,  E_L3 ,  E_L0, E_L0},
    {"USB3.0",                                  E_L0,  E_L3 ,  E_L0, E_L0},
    /*70*/
    {"WCN_AHB_SLAVE",                           E_L0,  E_L0 ,  E_L0, E_L0},
    {"MD2_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"MD3_PERIPHERALS",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"G3D_CONFIG",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MALI",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"MMSYS_CONFIG",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ0",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_RSZ1",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_WDMA",                                E_L0,  E_L3 ,  E_L0, E_L0},
    /*80*/
    {"MDP_WROT",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MDP_TDSHP",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_OVL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA0",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_RDMA1",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_WDMA",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_COLOR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_CCORR",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_AAL",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DISP_GAMMA",                              E_L0,  E_L3 ,  E_L0, E_L0},
    /*90*/
    {"DISP_DITHER",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"DSI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"DPI",                                     E_L0,  E_L3 ,  E_L0, E_L0},
    {"Reserved",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"MM_MUTEX",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB0",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_COMMON",                              E_L0,  E_L3 ,  E_L0, E_L0},
    {"MIPI_TX_CONFIG",                          E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CONFIG",                           E_L0,  E_L3 ,  E_L0, E_L0},
    /*100*/
    {"IMGSYS_SMI_LARB2",                        E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM1",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM2",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM3",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM4",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_SENINF",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAMSV",                            E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_FDVT",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM5",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"IMGSYS_CAM6",                             E_L0,  E_L3 ,  E_L0, E_L0},
    /*110*/
    {"IMGSYS_CAM7",                             E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDECSYS_GLOBAL_CONFIGURATION",            E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB1",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VDEC_FULL_TOP",                           E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC_GLOBAL_CON",                         E_L0,  E_L3 ,  E_L0, E_L0},
    {"SMI_LARB3",                               E_L0,  E_L3 ,  E_L0, E_L0},
    {"VENC",                                    E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_ENC",                                E_L0,  E_L3 ,  E_L0, E_L0},
    {"JPEG_DEC",                                E_L0,  E_L3 ,  E_L0, E_L0},
};


#else
#error "Wrong MACH type"
#endif

static void unmask_module_irq(unsigned int module)
{
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;
    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

    switch (apc_index){
       case 0:
           *DEVAPC_D0_VIO_MASK_0 &= ~(0x1 << apc_bit_index);
           break;
       case 1:
           *DEVAPC_D0_VIO_MASK_1 &= ~(0x1 << apc_bit_index);
           break;
       case 2:
           *DEVAPC_D0_VIO_MASK_2 &= ~(0x1 << apc_bit_index);
           break;
       case 3:
           *DEVAPC_D0_VIO_MASK_3 &= ~(0x1 << apc_bit_index);
           break;
       case 4:
           *DEVAPC_D0_VIO_MASK_4 &= ~(0x1 << apc_bit_index);
           break;

       default:
           printf("UnMask_Module_IRQ : The setting is error, please check if domain master setting is correct or not !\n");
           break;
    }
}


void mask_module_irq(unsigned int module)
{
     unsigned int apc_index = 0;
     unsigned int apc_bit_index = 0;
     apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
     apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

     switch (apc_index){
       case 0:
           *DEVAPC_D0_VIO_MASK_0 |= (0x1 << apc_bit_index);
           break;
       case 1:
           *DEVAPC_D0_VIO_MASK_1 |= (0x1 << apc_bit_index);
           break;
       case 2:
           *DEVAPC_D0_VIO_MASK_2 |= (0x1 << apc_bit_index);
           break;
       case 3:
           *DEVAPC_D0_VIO_MASK_3 |= (0x1 << apc_bit_index);
           break;
       case 4:
           *DEVAPC_D0_VIO_MASK_4 |= (0x1 << apc_bit_index);
           break;
       default:
           printf("Mask_Module_IRQ : The setting is error, please check if domain master setting is correct or not !\n");
           break;
     }
}


 static void clear_vio_status(unsigned int module)
 {
     unsigned int apc_index = 0;
     unsigned int apc_bit_index = 0;
     apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
     apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

     switch (apc_index){
         case 0:
            *DEVAPC_D0_VIO_STA_0 = (0x1 << apc_bit_index);
            break;
         case 1:
            *DEVAPC_D0_VIO_STA_1 = (0x1 << apc_bit_index);
            break;
         case 2:
            *DEVAPC_D0_VIO_STA_2 = (0x1 << apc_bit_index);
             break;
         case 3:
            *DEVAPC_D0_VIO_STA_3 = (0x1 << apc_bit_index);
             break;
         case 4:
            *DEVAPC_D0_VIO_STA_4 = (0x1 << apc_bit_index);
             break;
         default:
            break;
     }
 }

int set_master_transaction(unsigned int master_index ,unsigned int transaction_type)
{
    volatile unsigned int* base = 0;
    unsigned int set_bit = 0;

    base = (unsigned int*) ((size_t)DEVAPC_MAS_SEC );

    if(master_index > 31)
        return -1;

    if(transaction_type == 0){
        set_bit = ~(1 << master_index);
        writel(readl(base) & set_bit, base);
    }
    else if(transaction_type == 1){
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
    unmask_module_irq(module);

    if(domain_num == E_DOMAIN_0)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D0_APC_0 + (module/16) *4);
    }
    else if(domain_num == E_DOMAIN_1)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D1_APC_0 + (module/16) *4);
    }
    else if(domain_num == E_DOMAIN_2)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D2_APC_0 + (module/16) *4);
    }
    else if(domain_num == E_DOMAIN_3)
    {
        base = (unsigned int*) ((size_t)DEVAPC_D3_APC_0 + (module/16) *4);
    }

    if (base != NULL) {
        writel(readl(base) & ~clr_bit, base);
        writel(readl(base) | set_bit, base);
    }
}

int start_devapc()
{

    int module_index = 0;

    /*Enable Devapc*/
    writel(0x80000000, DEVAPC_VIO_DBG0); // clear apc0 dbg info if any

    writel(readl(DEVAPC_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC_APC_CON);

    writel(readl(DEVAPC_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC_PD_APC_CON);

   /*Initial Permission*/

    atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;

    if (teearg->tee_support) {
        printf("[DAPC] Walk TEE\n");
        for (module_index = 0; module_index < (sizeof(D_APC0_Devices_TEE)/sizeof(DEVICE_INFO)); module_index++)
        {
            set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices_TEE[module_index].d0_permission);
            set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices_TEE[module_index].d1_permission);
        }
    } else {
        printf("[DAPC] Walk ATF\n");
        for (module_index = 0; module_index < (sizeof(D_APC0_Devices)/sizeof(DEVICE_INFO)); module_index++)
        {
            set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices[module_index].d0_permission);
            set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices[module_index].d1_permission);
        }
    }

    printf("[DAPC] d0-0=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_0);
    printf("[DAPC] d0-1=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_1);
    printf("[DAPC] d0-2=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_2);
    printf("[DAPC] d0-3=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_3);
    printf("[DAPC] d0-4=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_4);
    printf("[DAPC] d0-5=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_5);
    printf("[DAPC] d0-6=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_6);
    printf("[DAPC] d0-7=0x%x\n", *(volatile unsigned int*)DEVAPC_D0_APC_7);

    printf("[DAPC] d1-0=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_0);
    printf("[DAPC] d1-1=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_1);
    printf("[DAPC] d1-2=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_2);
    printf("[DAPC] d1-3=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_3);
    printf("[DAPC] d1-4=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_4);
    printf("[DAPC] d1-5=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_5);
    printf("[DAPC] d1-6=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_6);
    printf("[DAPC] d1-7=0x%x\n", *(volatile unsigned int*)DEVAPC_D1_APC_7);

    /*Lock DAPC to secure access only*/
    writel(readl(DEVAPC_APC_CON) |  (0x1), DEVAPC_APC_CON);

    /*Set Level 2 secure*/
    writel(readl(INFRA_PDN_SEC_CON) | (SEJ_CG_PROTECT_BIT), INFRA_PDN_SEC_CON );
    writel(readl(INFRA_PDN_SEC_CON) | (TRNG_CG_PROTECT_BIT), INFRA_PDN_SEC_CON );

    printf("[DAPC] ATF Init Done\n");

    return 0;

}

