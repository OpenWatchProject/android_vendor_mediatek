#include <debug.h>
#include <devapc.h>
#include <platform.h>
#include <plat_private.h>   //for atf_arg_t_ptr


#if !defined(SPD_no)
//For TEE
const static DEVICE_INFO D_APC0_Devices_TEE[] = {
    /* 0 */
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_IO_CFG",                         E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_PERICFG",                        E_L1,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_GPIO",                           E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L2),
    DAPC_ATTR("INFRA_AO_TOP_RGU",                        E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_APXGPT",                         E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3),

    /* 10 */
    DAPC_ATTR("INFRA_AO_SEJ",                            E_L1,  E_L2),
    DAPC_ATTR("INFRA_AO_AP_CIRQ_EINT",                   E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_APMIXEDSYS_FHCTL",               E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_PMIC_WRAP",                      E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_DEVICE_APC_AO",                  E_L1,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_KEYPAD",                         E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_TOP_MISC",                       E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3),

    /* 20 */
    DAPC_ATTR("INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_AES_TOP0",                       E_L3,  E_L0),
    DAPC_ATTR("INFRA_AO_AES_TOP1",                       E_L3,  E_L0),
    DAPC_ATTR("INFRA_AO_MDEM_TEMP_SHARE",                E_L0,  E_L2),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG",              E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG1",             E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG2",             E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG3",             E_L0,  E_L3),

    /* 30 */
    DAPC_ATTR("INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_EFUSEC",                         E_L2,  E_L0),
    DAPC_ATTR("INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_DEBUG_TRACKER",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_CCI0_AP",                        E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_CCI0_MD",                        E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_CCI1_AP",                        E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_CCI1_MD",                        E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3),

    /* 40 */
    DAPC_ATTR("INFRASYS_CONTROL_REG",                    E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_TRNG",                           E_L1,  E_L3),
    DAPC_ATTR("INFRASYS_GCPU",                           E_L1,  E_L3),
    DAPC_ATTR("INFRASYS_MD_CCIF_MD1",                    E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_GCE",                            E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MD_CCIF_MD2",                    E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_ANA_MIPI_DSI0",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_ANA_MIPI_DSI1",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_ANA_MIPI_CSI0",                  E_L0,  E_L3),

    /* 50 */
    DAPC_ATTR("INFRASYS_ANA_MIPI_CSI1",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_GPU_RSA",                        E_L1,  E_L3),
    DAPC_ATTR("INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_MDSYS_INTF",                     E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BPI_BIS_SLV0",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BPI_BIS_SLV1",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BPI_BIS_SLV2",                   E_L0,  E_L3),
    DAPC_ATTR("INFRAYS_EMI_MPU_REG",                     E_L1,  E_L3),

    /* 60 */
    DAPC_ATTR("INFRAYS_DVFS_PROC",                       E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP0_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP1_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP2_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP3_configuration",            E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP0_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP1_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP2_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP3_configuration",            E_L0,  E_L3),

    /* 70 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("PERI_GPU_SMI_COMMON",                     E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_DEBUGTOP",                       E_L0,  E_L3),
    DAPC_ATTR("DMA",                                     E_L0,  E_L3),
    DAPC_ATTR("AUXADC",                                  E_L0,  E_L0),
    DAPC_ATTR("UART0",                                   E_L0,  E_L3),
    DAPC_ATTR("UART1",                                   E_L0,  E_L3),
    DAPC_ATTR("UART2",                                   E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 80 */
    DAPC_ATTR("PWM",                                     E_L0,  E_L3),
    DAPC_ATTR("I2C0",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C1",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C2",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI0",                                    E_L0,  E_L3),
    DAPC_ATTR("PTP_THERMAL_CTL",                         E_L0,  E_L0),
    DAPC_ATTR("BTIF",                                    E_L0,  E_L3),
    DAPC_ATTR("IRTX",                                    E_L0,  E_L3),
    DAPC_ATTR("DISP_PWM",                                E_L0,  E_L3),
    DAPC_ATTR("I2C3",                                    E_L0,  E_L3),

    /* 90 */
    DAPC_ATTR("SPI1",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C4",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI2",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI3",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C1_IMM",                                E_L0,  E_L3),
    DAPC_ATTR("I2C2_IMM",                                E_L0,  E_L3),
    DAPC_ATTR("I2C5",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C5_IMM",                                E_L0,  E_L3),
    DAPC_ATTR("SPI4",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI5",                                    E_L0,  E_L3),

    /* 100 */
    DAPC_ATTR("USB2.0",                                  E_L0,  E_L3),
    DAPC_ATTR("USB2.0_SIF",                              E_L0,  E_L3),
    DAPC_ATTR("MSDC0",                                   E_L0,  E_L3),
    DAPC_ATTR("MSDC1",                                   E_L0,  E_L3),
    DAPC_ATTR("MSDC2",                                   E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("USB3.0",                                  E_L0,  E_L3),
    DAPC_ATTR("USB3.0_SIF",                              E_L0,  E_L3),
    DAPC_ATTR("USB3.0_SIF2",                             E_L0,  E_L3),
    DAPC_ATTR("AUDIO",                                   E_L0,  E_L0),

    /* 110 */
    DAPC_ATTR("WCN_AHB_SLAVE",                           E_L0,  E_L0),
    DAPC_ATTR("MD_PERIPHERALS",                          E_L0,  E_L0),
    DAPC_ATTR("MD2_PERIPHERALS",                         E_L0,  E_L0),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_0",                            E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_1",                            E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_2",                            E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_3",                            E_L0,  E_L3),

    /* 120 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("G3D_POWER_CONTROL",                       E_L0,  E_L3),
    DAPC_ATTR("MALI_CONFIG",                             E_L0,  E_L3),
    DAPC_ATTR("MMSYS_CONFIG",                            E_L0,  E_L3),
    DAPC_ATTR("MDP_RDMA0",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_RDMA1",                               E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("MDP_RSZ1",                                E_L0,  E_L3),
    DAPC_ATTR("MDP_RSZ2",                                E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 130 */
    DAPC_ATTR("MDP_WROT0",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_WROT1",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_TDSHP",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_COLOR",                               E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL0",                               E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL1",                               E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL0_2L",                            E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL1_2L",                            E_L0,  E_L3),
    DAPC_ATTR("DISP_RDMA0",                              E_L0,  E_L3),
    DAPC_ATTR("DISP_RDMA1",                              E_L0,  E_L3),

    /* 140 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_WDMA0",                              E_L0,  E_L3),
    DAPC_ATTR("DISP_WDMA1",                              E_L0,  E_L3),
    DAPC_ATTR("DISP_COLOR0",                             E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_CCORR0",                             E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_AAL0",                               E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_GAMMA0",                             E_L0,  E_L3),

    /* 150 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_DITHER0",                            E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DSI_UFOE",                                E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DSI_SPLIT",                               E_L0,  E_L3),
    DAPC_ATTR("DSI0",                                    E_L0,  E_L3),
    DAPC_ATTR("DSI1",                                    E_L0,  E_L3),
    DAPC_ATTR("DPI0",                                    E_L0,  E_L3),

    /* 160 */
    DAPC_ATTR("MM_MUTEX",                                E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB0",                               E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB4",                               E_L0,  E_L3),
    DAPC_ATTR("SMI_COMMON",                              E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("IMGSYS_CONFIG",                           E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB5",                               E_L0,  E_L3),
    DAPC_ATTR("DIP_A0",                                  E_L0,  E_L3),
    DAPC_ATTR("DIP_A1",                                  E_L0,  E_L3),

    /* 170 */
    DAPC_ATTR("DIP_A2",                                  E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("VAD",                                     E_L0,  E_L3),
    DAPC_ATTR("DPE",                                     E_L0,  E_L3),
    DAPC_ATTR("RSC",                                     E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("FDVT",                                    E_L0,  E_L3),
    DAPC_ATTR("GEPF",                                    E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 180 */
    DAPC_ATTR("DFP",                                     E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("VDEC_GLOBAL_CON",                         E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB1",                               E_L0,  E_L3),
    DAPC_ATTR("VDEC_FULL_TOP",                           E_L0,  E_L3),
    DAPC_ATTR("VENC_GLOBAL_CON",                         E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB3",                               E_L0,  E_L3),
    DAPC_ATTR("VENC",                                    E_L0,  E_L3),
    DAPC_ATTR("JPEGENC",                                 E_L0,  E_L3),
    DAPC_ATTR("JPEGDEC",                                 E_L0,  E_L3),

    /* 190 */
    DAPC_ATTR("CAMSYS_TOP",                              E_L0,  E_L3),
    DAPC_ATTR("LARB2",                                   E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP",                                 E_L0,  E_L3),
    DAPC_ATTR("CAM_A",                                   E_L0,  E_L3),
    DAPC_ATTR("CAM_B",                                   E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP_SET",                             E_L0,  E_L3),
    DAPC_ATTR("CAM_A_SET",                               E_L0,  E_L3),
    DAPC_ATTR("CAM_B_SET",                               E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP_INNER",                           E_L0,  E_L3),
    DAPC_ATTR("CAM_A_INNER",                             E_L0,  E_L3),

    /* 200 */
    DAPC_ATTR("CAM_B_INNER",                             E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP_CLR",                             E_L0,  E_L3),
    DAPC_ATTR("CAM_A_CLR",                               E_L0,  E_L3),
    DAPC_ATTR("CAM_B_CLR",                               E_L0,  E_L3),
    DAPC_ATTR("SENINF_A",                                E_L0,  E_L3),
    DAPC_ATTR("SENINF_B",                                E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 210 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_A",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_B",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_C",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_D",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_E",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_F",                                 E_L0,  E_L3),
    DAPC_ATTR("TSF",                                     E_L0,  E_L3),
    DAPC_ATTR("CAMSYS_OTHERS",                           E_L0,  E_L3),

    /* 220 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
};

#else

//For ATF
const static DEVICE_INFO D_APC0_Devices[] = {
    /* 0 */
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_CLOCK_GENERATOR",      E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_INFRASYS_CONFIG_REGS",           E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_IO_CFG",                         E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_PERICFG",                        E_L1,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_GPIO",                           E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_TOP_LEVEL_SLP_MANAGER",          E_L0,  E_L2),
    DAPC_ATTR("INFRA_AO_TOP_RGU",                        E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_APXGPT",                         E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3),

    /* 10 */
    DAPC_ATTR("INFRA_AO_SEJ",                            E_L0,  E_L2),
    DAPC_ATTR("INFRA_AO_AP_CIRQ_EINT",                   E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_APMIXEDSYS_FHCTL",               E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_PMIC_WRAP",                      E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_DEVICE_APC_AO",                  E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_KEYPAD",                         E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_TOP_MISC",                       E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3),
    DAPC_ATTR("INFRA_AO_RESERVE_REGION",                 E_L0,  E_L3),

    /* 20 */
    DAPC_ATTR("INFRA_AO_CLDMA_AO_TOP_AP",                E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_CLDMA_AO_TOP_MD",                E_L0,  E_L0),
    DAPC_ATTR("INFRA_AO_AES_TOP0",                       E_L3,  E_L0),
    DAPC_ATTR("INFRA_AO_AES_TOP1",                       E_L3,  E_L0),
    DAPC_ATTR("INFRA_AO_MDEM_TEMP_SHARE",                E_L0,  E_L2),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG",              E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG1",             E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG2",             E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MCUSYS_CONFIG_REG3",             E_L0,  E_L3),

    /* 30 */
    DAPC_ATTR("INFRASYS_SYSTEM_CIRQ",                    E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MM_IOMMU_CONFIGURATION",         E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_EFUSEC",                         E_L2,  E_L0),
    DAPC_ATTR("INFRASYS_DEVICE_APC_MONITOR",             E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_DEBUG_TRACKER",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_CCI0_AP",                        E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_CCI0_MD",                        E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_CCI1_AP",                        E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_CCI1_MD",                        E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MBIST_CONTROL_REG",              E_L0,  E_L3),

    /* 40 */
    DAPC_ATTR("INFRASYS_CONTROL_REG",                    E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_TRNG",                           E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_GCPU",                           E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MD_CCIF_MD1",                    E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_GCE",                            E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_MD_CCIF_MD2",                    E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BOOTROM/SRAM",                   E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_ANA_MIPI_DSI0",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_ANA_MIPI_DSI1",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_ANA_MIPI_CSI0",                  E_L0,  E_L3),

    /* 50 */
    DAPC_ATTR("INFRASYS_ANA_MIPI_CSI1",                  E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_EMI_BUS_INTERFACE",              E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_GPU_RSA",                        E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_CLDMA_PDN_AP",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_CLDMA_PDN_MD",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_MDSYS_INTF",                     E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BPI_BIS_SLV0",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BPI_BIS_SLV1",                   E_L0,  E_L0),
    DAPC_ATTR("INFRASYS_BPI_BIS_SLV2",                   E_L0,  E_L3),
    DAPC_ATTR("INFRAYS_EMI_MPU_REG",                     E_L1,  E_L3),

    /* 60 */
    DAPC_ATTR("INFRAYS_DVFS_PROC",                       E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP0_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP1_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP2_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH0_TOP3_configuration",            E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP0_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP1_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP2_configuration",            E_L0,  E_L3),
    DAPC_ATTR("DRAMC_CH1_TOP3_configuration",            E_L0,  E_L3),

    /* 70 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("PERI_GPU_SMI_COMMON",                     E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("INFRASYS_DEBUGTOP",                       E_L0,  E_L3),
    DAPC_ATTR("DMA",                                     E_L0,  E_L3),
    DAPC_ATTR("AUXADC",                                  E_L0,  E_L0),
    DAPC_ATTR("UART0",                                   E_L0,  E_L3),
    DAPC_ATTR("UART1",                                   E_L0,  E_L3),
    DAPC_ATTR("UART2",                                   E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 80 */
    DAPC_ATTR("PWM",                                     E_L0,  E_L3),
    DAPC_ATTR("I2C0",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C1",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C2",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI0",                                    E_L0,  E_L3),
    DAPC_ATTR("PTP_THERMAL_CTL",                         E_L0,  E_L0),
    DAPC_ATTR("BTIF",                                    E_L0,  E_L3),
    DAPC_ATTR("IRTX",                                    E_L0,  E_L3),
    DAPC_ATTR("DISP_PWM",                                E_L0,  E_L3),
    DAPC_ATTR("I2C3",                                    E_L0,  E_L3),

    /* 90 */
    DAPC_ATTR("SPI1",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C4",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI2",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI3",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C1_IMM",                                E_L0,  E_L3),
    DAPC_ATTR("I2C2_IMM",                                E_L0,  E_L3),
    DAPC_ATTR("I2C5",                                    E_L0,  E_L3),
    DAPC_ATTR("I2C5_IMM",                                E_L0,  E_L3),
    DAPC_ATTR("SPI4",                                    E_L0,  E_L3),
    DAPC_ATTR("SPI5",                                    E_L0,  E_L3),

    /* 100 */
    DAPC_ATTR("USB2.0",                                  E_L0,  E_L3),
    DAPC_ATTR("USB2.0_SIF",                              E_L0,  E_L3),
    DAPC_ATTR("MSDC0",                                   E_L0,  E_L3),
    DAPC_ATTR("MSDC1",                                   E_L0,  E_L3),
    DAPC_ATTR("MSDC2",                                   E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("USB3.0",                                  E_L0,  E_L3),
    DAPC_ATTR("USB3.0_SIF",                              E_L0,  E_L3),
    DAPC_ATTR("USB3.0_SIF2",                             E_L0,  E_L3),
    DAPC_ATTR("AUDIO",                                   E_L0,  E_L0),

    /* 110 */
    DAPC_ATTR("WCN_AHB_SLAVE",                           E_L0,  E_L0),
    DAPC_ATTR("MD_PERIPHERALS",                          E_L0,  E_L0),
    DAPC_ATTR("MD2_PERIPHERALS",                         E_L0,  E_L0),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_0",                            E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_1",                            E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_2",                            E_L0,  E_L3),
    DAPC_ATTR("G3D_CONFIG_3",                            E_L0,  E_L3),

    /* 120 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("G3D_POWER_CONTROL",                       E_L0,  E_L3),
    DAPC_ATTR("MALI_CONFIG",                             E_L0,  E_L3),
    DAPC_ATTR("MMSYS_CONFIG",                            E_L0,  E_L3),
    DAPC_ATTR("MDP_RDMA0",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_RDMA1",                               E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("MDP_RSZ1",                                E_L0,  E_L3),
    DAPC_ATTR("MDP_RSZ2",                                E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 130 */
    DAPC_ATTR("MDP_WROT0",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_WROT1",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_TDSHP",                               E_L0,  E_L3),
    DAPC_ATTR("MDP_COLOR",                               E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL0",                               E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL1",                               E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL0_2L",                            E_L0,  E_L3),
    DAPC_ATTR("DISP_OVL1_2L",                            E_L0,  E_L3),
    DAPC_ATTR("DISP_RDMA0",                              E_L0,  E_L3),
    DAPC_ATTR("DISP_RDMA1",                              E_L0,  E_L3),

    /* 140 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_WDMA0",                              E_L0,  E_L3),
    DAPC_ATTR("DISP_WDMA1",                              E_L0,  E_L3),
    DAPC_ATTR("DISP_COLOR0",                             E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_CCORR0",                             E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_AAL0",                               E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_GAMMA0",                             E_L0,  E_L3),

    /* 150 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DISP_DITHER0",                            E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DSI_UFOE",                                E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("DSI_SPLIT",                               E_L0,  E_L3),
    DAPC_ATTR("DSI0",                                    E_L0,  E_L3),
    DAPC_ATTR("DSI1",                                    E_L0,  E_L3),
    DAPC_ATTR("DPI0",                                    E_L0,  E_L3),

    /* 160 */
    DAPC_ATTR("MM_MUTEX",                                E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB0",                               E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB4",                               E_L0,  E_L3),
    DAPC_ATTR("SMI_COMMON",                              E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("IMGSYS_CONFIG",                           E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB5",                               E_L0,  E_L3),
    DAPC_ATTR("DIP_A0",                                  E_L0,  E_L3),
    DAPC_ATTR("DIP_A1",                                  E_L0,  E_L3),

    /* 170 */
    DAPC_ATTR("DIP_A2",                                  E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("VAD",                                     E_L0,  E_L3),
    DAPC_ATTR("DPE",                                     E_L0,  E_L3),
    DAPC_ATTR("RSC",                                     E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("FDVT",                                    E_L0,  E_L3),
    DAPC_ATTR("GEPF",                                    E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 180 */
    DAPC_ATTR("DFP",                                     E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("VDEC_GLOBAL_CON",                         E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB1",                               E_L0,  E_L3),
    DAPC_ATTR("VDEC_FULL_TOP",                           E_L0,  E_L3),
    DAPC_ATTR("VENC_GLOBAL_CON",                         E_L0,  E_L3),
    DAPC_ATTR("SMI_LARB3",                               E_L0,  E_L3),
    DAPC_ATTR("VENC",                                    E_L0,  E_L3),
    DAPC_ATTR("JPEGENC",                                 E_L0,  E_L3),
    DAPC_ATTR("JPEGDEC",                                 E_L0,  E_L3),

    /* 190 */
    DAPC_ATTR("CAMSYS_TOP",                              E_L0,  E_L3),
    DAPC_ATTR("LARB2",                                   E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP",                                 E_L0,  E_L3),
    DAPC_ATTR("CAM_A",                                   E_L0,  E_L3),
    DAPC_ATTR("CAM_B",                                   E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP_SET",                             E_L0,  E_L3),
    DAPC_ATTR("CAM_A_SET",                               E_L0,  E_L3),
    DAPC_ATTR("CAM_B_SET",                               E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP_INNER",                           E_L0,  E_L3),
    DAPC_ATTR("CAM_A_INNER",                             E_L0,  E_L3),

    /* 200 */
    DAPC_ATTR("CAM_B_INNER",                             E_L0,  E_L3),
    DAPC_ATTR("CAM_TOP_CLR",                             E_L0,  E_L3),
    DAPC_ATTR("CAM_A_CLR",                               E_L0,  E_L3),
    DAPC_ATTR("CAM_B_CLR",                               E_L0,  E_L3),
    DAPC_ATTR("SENINF_A",                                E_L0,  E_L3),
    DAPC_ATTR("SENINF_B",                                E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),

    /* 210 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_A",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_B",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_C",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_D",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_E",                                 E_L0,  E_L3),
    DAPC_ATTR("CAMSV_F",                                 E_L0,  E_L3),
    DAPC_ATTR("TSF",                                     E_L0,  E_L3),
    DAPC_ATTR("CAMSYS_OTHERS",                           E_L0,  E_L3),

    /* 220 */
    DAPC_ATTR("RESERVE",                                 E_L0,  E_L3),
};

#endif

const static MD_MODULE_INFO D_MD_Devices[] = {
    /*device_name                    read_permission   write_permission*/
    /* 0 */
    MD_ATTR("MDCFGCTL_TOP",                            E_NonSec,    E_NonSec),
    MD_ATTR("MDUART0_TOP",                             E_Sec,       E_Sec),
    MD_ATTR("MDGDMA_TOP",                              E_NonSec,    E_Sec),
    MD_ATTR("MDGPTM_TOP",                              E_Sec,       E_Sec),
    MD_ATTR("USIM1_TOP",                               E_Sec,       E_Sec),
    MD_ATTR("USIM2_TOP",                               E_Sec,       E_Sec),
    MD_ATTR("MDPERISYS_MISC_REG",                      E_NonSec,    E_NonSec),
    MD_ATTR("MDCIRQ_TOP",                              E_Sec,       E_Sec),
    MD_ATTR("MD_DBGSYS1_TOP",                          E_NonSec,    E_NonSec),
    MD_ATTR("MD_DBGSYS2_TOP",                          E_NonSec,    E_NonSec),

    /* 10 */
    MD_ATTR("MD_DBGSYS3_TOP",                          E_NonSec,    E_NonSec),
    MD_ATTR("MDPAR_DBGMON_TOP",                        E_Sec,       E_Sec),
    MD_ATTR("MDPERISYS_CLK_CTL_TOP",                   E_Sec,       E_Sec),
    MD_ATTR("MD_TOPSM",                                E_NonSec,    E_NonSec),
    MD_ATTR("MD_OSTIMER",                              E_NonSec,    E_NonSec),
    MD_ATTR("MDRGU_TOP",                               E_NonSec,    E_NonSec),
    MD_ATTR("I2C_TOP",                                 E_Sec,       E_Sec),
    MD_ATTR("MD_EINT_TOP",                             E_Sec,       E_Sec),
    MD_ATTR("MDSYS_CLKCTL_TOP",                        E_NonSec,    E_NonSec),
    MD_ATTR("MD_GLOBAL_CON_TOP_DCM",                   E_NonSec,    E_NonSec),

    /* 20 */
    MD_ATTR("MDTOP_PLLMIXED_TOP",                      E_NonSec,    E_NonSec),
    MD_ATTR("MD_CLKSW_TOP",                            E_NonSec,    E_NonSec),
    MD_ATTR("ARM7_OSTIMER",                            E_Sec,       E_Sec),
    MD_ATTR("MD_LITE_GPTIMER",                         E_Sec,       E_Sec),
    MD_ATTR("PS2ARM7_PCCIF",                           E_Sec,       E_Sec),
    MD_ATTR("ARM72PS_ARM7",                            E_Sec,       E_Sec),
    MD_ATTR("MDPERISYS_MBIST_CONFIG",                  E_Sec,       E_Sec),
    MD_ATTR("SDF_TOP",                                 E_Sec,       E_Sec),
    MD_ATTR("PSMCU_MISC_REG",                          E_NonSec,    E_Sec),
    MD_ATTR("PSMCU_BUSMON_TOP",                        E_Sec,       E_Sec),

    /* 30 */
    MD_ATTR("PSMCU_ETM_PC_MONITOR_TOP",                E_NonSec,    E_NonSec),
    MD_ATTR("PSMCUSYS_MBIST_CONFIG",                   E_Sec,       E_Sec),
    MD_ATTR("ELM_TOP",                                 E_NonSec,    E_Sec),
    MD_ATTR("ABM/ASM",                                 E_NonSec,    E_Sec),
    MD_ATTR("SOE",                                     E_Sec,       E_Sec),
    MD_ATTR("BUSMON",                                  E_Sec,       E_Sec),
    MD_ATTR("MDUART1_TOP",                             E_Sec,       E_Sec),
    MD_ATTR("MDUART2_TOP",                             E_Sec,       E_Sec),
    MD_ATTR("MDINFRASYS_MBIST_CONFIG",                 E_Sec,       E_Sec),
    MD_ATTR("MDSYS_MBIST_CONFIG",                      E_Sec,       E_Sec),

    /* 40 */
    MD_ATTR("MDSMICFG",                                E_NonSec,    E_Sec),
    MD_ATTR("MDINFRA_MISC_REG",                        E_NonSec,    E_Sec),
    MD_ATTR("MD_BUS_RECORDER",                         E_NonSec,    E_Sec),
    MD_ATTR("PPC_TOP",                                 E_Sec,       E_Sec),
    MD_ATTR("ARM7_WDT_TOP",                            E_Sec,       E_Sec),
    MD_ATTR("ARM7_MBIST_CONFIG",                       E_Sec,       E_Sec),
    MD_ATTR("ARM7_CIRQ",                               E_Sec,       E_Sec),
    MD_ATTR("ARM7_MISC_REG",                           E_Sec,       E_Sec),
    MD_ATTR("MDL1_RGU",                                E_NonSec,    E_NonSec),
    MD_ATTR("MDL1_OSTIMER",                            E_NonSec,    E_Sec),

    /* 50 */
    MD_ATTR("MDL1_MODEM_TOPSM",                        E_NonSec,    E_Sec),
    MD_ATTR("MDL1_AO_CONFG",                           E_NonSec,    E_NonSec),
    MD_ATTR("BUS_MON",                                 E_NonSec,    E_NonSec),
    MD_ATTR("PC_MON",                                  E_NonSec,    E_NonSec),
    MD_ATTR("L1MCU_BUS_CFG",                           E_NonSec,    E_Sec),
    MD_ATTR("RESERVE",                                 E_NonSec,    E_NonSec),
    MD_ATTR("RESERVE",                                 E_NonSec,    E_NonSec),
    MD_ATTR("RESERVE",                                 E_NonSec,    E_NonSec),
    MD_ATTR("RESERVE",                                 E_NonSec,    E_NonSec),
    MD_ATTR("RESERVE",                                 E_NonSec,    E_NonSec),

    /* 60 */
    MD_ATTR("MD1_OTHERS",                              E_Sec,       E_Sec),
    MD_ATTR("CG",                                      E_NonSec,    E_NonSec),
    MD_ATTR("PLLCFG",                                  E_NonSec,    E_NonSec),
    MD_ATTR("C2K_OTHERS",                              E_NonSec,    E_Sec),
};


static void clear_vio_status(unsigned int module)
{
    unsigned int apc_index = 0;
    unsigned int apc_bit_index = 0;

    apc_index = module / (MOD_NO_IN_1_DEVAPC*2);
    apc_bit_index = module % (MOD_NO_IN_1_DEVAPC*2);

    *((volatile unsigned int*)((size_t)DEVAPC_D0_VIO_STA_0 + (apc_index * 4))) = (0x1 << apc_bit_index);
}

static int set_master_transaction(unsigned int master_index, unsigned int transaction_type)
{
    volatile unsigned int* base = NULL;
    unsigned int set_bit = 0;
    unsigned int master_register_index = 0;
    unsigned int master_set_index = 0;

    if (master_index > MASTER_MAX_INDEX)
        return -1;

    master_register_index = master_index / (MOD_NO_IN_1_DEVAPC*2);
    master_set_index = master_index % (MOD_NO_IN_1_DEVAPC*2);

    base = (volatile unsigned int*) ((size_t)DEVAPC_MAS_SEC_0 + master_register_index * 4);

    if (transaction_type == 0){
        set_bit = 0xFFFFFFFF ^ (1 << master_set_index);
        writel(readl(base) & set_bit, base);
    } else if (transaction_type == 1) {
        set_bit = 1 << master_set_index;
        writel(readl(base) | set_bit, base);
    } else
        return -2;

    return 0;
}

#if  CFG_MICROTRUST_TEE_SUPPORT
#else
static
#endif
void set_module_apc(unsigned int module, E_MASK_DOM domain_num, APC_ATTR permission_control)
{
    volatile unsigned int* base = NULL;

    unsigned int clr_bit = 0xFFFFFFFF ^ (0x3 << ((module % MOD_NO_IN_1_DEVAPC) * 2));
    unsigned int set_bit = permission_control << ((module % MOD_NO_IN_1_DEVAPC) * 2);

    clear_vio_status(module);

    /* Note: "unmask_module_irq" is changed to set in Kernel to avoid time gap between DEVAPC and EMI_MPU.
     * Remember: NEVER unmask_module_irq before the Device APC Kernel driver is loaded
     */

    if (domain_num == E_DOMAIN_0)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D0_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_1)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D1_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_2)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D2_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_3)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D3_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_4)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D4_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_5)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D5_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_6)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D6_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }
    else if (domain_num == E_DOMAIN_7)
    {
        base = (volatile unsigned int*) ((size_t)DEVAPC_D7_APC_0 + (module/MOD_NO_IN_1_DEVAPC) * 4);
    }

    if (base != NULL) {
        writel(readl(base) & clr_bit, base);
        writel(readl(base) | set_bit, base);
    }
}

static void set_mdsys_apc(unsigned int module, MDAPC_ATTR read_permission, MDAPC_ATTR write_permission)
{
    volatile unsigned int* base_read = NULL;
    unsigned int set_bit_read = 0;
    volatile unsigned int* base_write = NULL;
    unsigned int set_bit_write = 0;

    base_read = (volatile unsigned int*) ((size_t)DEVAPC_MD_SEC_EN_R_0 + (module/(MOD_NO_IN_1_DEVAPC*2)) * 4);
    if (read_permission == E_NonSec)
    {
        set_bit_read = 0xFFFFFFFF ^ (1 << (module % (MOD_NO_IN_1_DEVAPC*2)));
        writel(readl(base_read) & set_bit_read, base_read);
    }
    else
    {
        set_bit_read = 1 << (module % (MOD_NO_IN_1_DEVAPC*2));
        writel(readl(base_read) | set_bit_read, base_read);
    }

    base_write = (volatile unsigned int*) ((size_t)DEVAPC_MD_SEC_EN_0 + (module/(MOD_NO_IN_1_DEVAPC*2)) * 4);
    if (write_permission == E_NonSec)
    {
        set_bit_write = 0xFFFFFFFF ^ (1 << (module % (MOD_NO_IN_1_DEVAPC*2)));
        writel(readl(base_write) & set_bit_write, base_write);
    }
    else
    {
        set_bit_write = 1 << (module % (MOD_NO_IN_1_DEVAPC*2));
        writel(readl(base_write) | set_bit_write, base_write);
    }
}

int start_devapc(void)
{
    int module_index = 0;

    //d: domain, i: register number
    int i = 0;
    int d = 0;

    /* Enable Devapc */
    writel(0x80000000, DEVAPC_VIO_DBG0); // clear apc0 dbg info if any

    writel(readl(DEVAPC_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), DEVAPC_APC_CON);

    writel(readl(DEVAPC_PD_APC_CON) & (0xFFFFFFFF ^ (1<<2)), DEVAPC_PD_APC_CON);

    /* Set Necessary Masters to Secure Transaction */
    set_master_transaction(MASTER_SPM, SECURE_TRANSACTION); /* SPM_PDN will access MCUSYS in Normal World, but MCUSYS is secure r/w and non-secure read-only. So this setting is necessary. */

    /* Initial Permission */

#if !defined(SPD_no)
    INFO("[DAPC] Walk TEE\n");
    for (module_index = 0; module_index < (sizeof(D_APC0_Devices_TEE)/sizeof(DEVICE_INFO)); module_index++)
    {
        set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices_TEE[module_index].d0_permission);

        set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices_TEE[module_index].d1_permission);
    }
#else
    INFO("[DAPC] Walk ATF\n");
    for (module_index = 0; module_index < (sizeof(D_APC0_Devices)/sizeof(DEVICE_INFO)); module_index++)
    {
        set_module_apc(module_index, E_DOMAIN_0 , D_APC0_Devices[module_index].d0_permission);

        set_module_apc(module_index, E_DOMAIN_1 , D_APC0_Devices[module_index].d1_permission);
    }
#endif

    /* [Important] You must open the access permission of AP=>MD1_PERIPHERALS and AP=>C2K_PERIPHERALS because
     *             it is the first level protection. Below is the MD 2nd level protection.*/
    INFO("[DAPC] Setting MDSYS\n");
    for (module_index = 0; module_index < (sizeof(D_MD_Devices)/sizeof(MD_MODULE_INFO)); module_index++)
    {
        set_mdsys_apc(module_index, D_MD_Devices[module_index].read_permission, D_MD_Devices[module_index].write_permission);
    }

    //d: domain, i: register number
    for (d=0; d<=1; d++) {
        for (i=0; i<=13; i++) {
            INFO("[DAPC] D%d_APC_%d = 0x%x\n", d, i, *((volatile unsigned int*)((size_t)DEVAPC_D0_APC_0 + (d * 0x100) + (i * 4))));
        }
    }

    INFO("[DAPC] MAS_SEC_0 = 0x%x\n", *DEVAPC_MAS_SEC_0);

    for (i=0; i<=7; i++) {
        INFO("[DAPC] VIO_MASK_%d=0x%x\n", i, *(DEVAPC_D0_VIO_MASK_0 + i));
    }

    for (i=0; i<=7; i++) {
        INFO("[DAPC] VIO_STA_%d=0x%x\n", i, *(DEVAPC_D0_VIO_STA_0 + i));
    }

    INFO("\n[DAPC] Current MM_EN 0:0x%x, 1:0x%x, 2:0x%x\n",
       *DEVAPC_MM_SEC_EN_0, *DEVAPC_MM_SEC_EN_1, *DEVAPC_MM_SEC_EN_2);

    INFO("[DAPC] Current MD_EN 0:0x%x, 1:0x%x\n",
       *DEVAPC_MD_SEC_EN_0, *DEVAPC_MD_SEC_EN_1);

    INFO("[DAPC] Current MD_EN_R 0:0x%x, 1:0x%x\n",
       *DEVAPC_MD_SEC_EN_R_0, *DEVAPC_MD_SEC_EN_R_1);

    /*Lock DAPC to secure access only*/
    writel(readl(DEVAPC_APC_CON) | (0x1), DEVAPC_APC_CON);

    /*Set Level 2 secure*/
    writel(readl(INFRA_AO_SEC_CG_CON0) | (SEJ_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON0);
    writel(readl(INFRA_AO_SEC_CG_CON1) | (TRNG_CG_PROTECT_BIT), INFRA_AO_SEC_CG_CON1);

    INFO("[DAPC] Init Done\n");
    INFO("[DAPC] APC_CON = 0x%x\n", *DEVAPC_APC_CON);

    return 0;
}
