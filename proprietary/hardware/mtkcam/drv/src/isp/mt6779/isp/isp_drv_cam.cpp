/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IspDrv_CAM"

#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>
#include "isp_drv_cam.h"
#include <isp_drv_stddef.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv_CAM);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_CAM_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("[0x%x][%d_%d]:" fmt, this->m_Module,this->m_CQ,this->m_pageIdx,##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_CAM_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG("[0x%x][%d_%d]:" fmt, this->m_Module,this->m_CQ,this->m_pageIdx,##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_CAM_DbgLogEnable_INFO   ) { BASE_LOG_INF("[0x%x][%d_%d]:" fmt, this->m_Module,this->m_CQ,this->m_pageIdx,##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_CAM_DbgLogEnable_WARN   ) { BASE_LOG_WRN("[0x%x][%d_%d]:" fmt, this->m_Module,this->m_CQ,this->m_pageIdx,##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_CAM_DbgLogEnable_ERROR  ) { BASE_LOG_ERR("[0x%x][%d_%d]:" fmt, this->m_Module,this->m_CQ,this->m_pageIdx,##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_CAM_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


typedef enum{
    IspDrv_UNKNONW      = 0,
    IspDrv_Create       = 1,
    IspDrv_Init         = 2,
    IspDrv_Start        = 3,
    IspDrv_Stop         = 4,
    IspDrv_Uninit       = 5,
}E_IspDrvStatus;

typedef enum{
    OP_IspDrv_Destroy,
    OP_IspDrv_sig,         //wait/clr/reg signal
    OP_IspDrv_sig_sig,     //signal signal
    OP_IspDrv_CQ,          //CQ operation
    OP_IspDrv_Device,      //device driver ctrl
    OP_IspDrv_init,
    OP_IspDrv_start,
    OP_IspDrv_stop,
    OP_IspDrv_Uninit,
}E_IspDrvOP;

/**
CAM cq module info
*/
#define CQ_DMA_SW_WORKAROUND        1 //HW bug: CQ_lsci will seldom read the wrong DMA address. Remove CQ NOP cmd to prevent this corner case.
#define ISP_CQ_APB_INST             0x0
#define ISP_CQ_NOP_INST             0x1
#define ISP_DRV_CQ_END_TOKEN        0x1C000000
#define ISP_CQ_INST_BIT             (26)//bit26 of cmq cmd is INST, it will be ISP_CQ_APB_INST or ISP_CQ_NOP_INST

#if (CQ_DMA_SW_WORKAROUND == 1)
#define ISP_DRV_CQ_DUMMY_TOKEN(Module)   (DESCRIPTOR_TOKEN(CQ_DUMMY_REG, 1, (Module+1)*CAM_BASE_RANGE_WITH_INNER))
#define ISP_DRV_CQ_NOP_TOKEN(Module)     (DESCRIPTOR_TOKEN(CQ_NOP_REG, 1, (Module+1)*CAM_BASE_RANGE_WITH_INNER))
#else
#define ISP_DRV_CQ_DUMMY_TOKEN(Module)   ((ISP_CQ_NOP_INST<<ISP_CQ_INST_BIT)|((CQ_DUMMY_REG + Module*CAM_BASE_RANGE) & CQ_DES_RANGE))
#endif
#define ISP_CQ_IS_NOP(cq_cmd)            ((cq_cmd>>ISP_CQ_INST_BIT) & ISP_CQ_NOP_INST)
#define ISP_CQ_DUMMY_PA             0x88100000

#define DESCRIPTOR_TOKEN(reg_oft,reg_num,dram_oft)  ({\
    MUINT32 tmp;\
    tmp = ((reg_oft + dram_oft)&0xffff)  |  \
                (((reg_num-1)&0x3ff)<<16)  |  \
                ((ISP_CQ_APB_INST)<<26)  | \
                (((reg_oft + dram_oft)&0x70000)<<(29-16));\
    tmp;\
})

#define _CQ_ID_Mapping(x)({\
    ISP_WRDMA_ENUM _cqidx;\
    if(x ==ISP_DRV_CQ_THRE0)       _cqidx = _dma_cq0i_;\
    else if(x ==ISP_DRV_CQ_THRE1)  _cqidx = _dma_cq1i_;\
    else if(x ==ISP_DRV_CQ_THRE2)  _cqidx = _dma_cq2i_;\
    else if(x ==ISP_DRV_CQ_THRE3)  _cqidx = _dma_cq3i_;\
    else if(x ==ISP_DRV_CQ_THRE4)  _cqidx = _dma_cq4i_;\
    else if(x ==ISP_DRV_CQ_THRE5)  _cqidx = _dma_cq5i_;\
    else if(x ==ISP_DRV_CQ_THRE6)  _cqidx = _dma_cq6i_;\
    else if(x ==ISP_DRV_CQ_THRE7)  _cqidx = _dma_cq7i_;\
    else if(x ==ISP_DRV_CQ_THRE8)  _cqidx = _dma_cq8i_;\
    else if(x ==ISP_DRV_CQ_THRE9)  _cqidx = _dma_cq9i_;\
    else if(x ==ISP_DRV_CQ_THRE10) _cqidx = _dma_cq10i_;\
    else if(x ==ISP_DRV_CQ_THRE11) _cqidx = _dma_cq11i_;\
    else if(x ==ISP_DRV_CQ_THRE12) _cqidx = _dma_cq12i_;\
    else if(x ==ISP_DRV_CQ_THRE13) _cqidx = _dma_cq13i_;\
    else if(x ==ISP_DRV_CQ_THRE14) _cqidx = _dma_cq14i_;\
    else if(x ==ISP_DRV_CQ_THRE15) _cqidx = _dma_cq15i_;\
    else if(x ==ISP_DRV_CQ_THRE16) _cqidx = _dma_cq16i_;\
    else if(x ==ISP_DRV_CQ_THRE17) _cqidx = _dma_cq17i_;\
    else if(x ==ISP_DRV_CQ_THRE23) _cqidx = _dma_cq23i_;\
    else if(x ==ISP_DRV_CQ_THRE24) _cqidx = _dma_cq24i_;\
    else if(x ==ISP_DRV_CQ_THRE25) _cqidx = _dma_cq25i_;\
    else if(x ==ISP_DRV_CQ_THRE26) _cqidx = _dma_cq26i_;\
    else if(x ==ISP_DRV_CQ_THRE27) _cqidx = _dma_cq27i_;\
    else if(x ==ISP_DRV_CQ_THRE28) _cqidx = _dma_cq28i_;\
    else\
        _cqidx = _dma_max_wr_;\
    _cqidx;\
})

ISP_DRV_CQ_MODULE_INFO_STRUCT mIspCQModuleInfo[CAM_CAM_MODULE_MAX]    =
{
    {TWIN_CQ0_BASEADDRESS,        0x00208, 0x7C74, 1,  TWIN_CQ_THR0_ADDR_CQ  }, //this is for twin's master cam use only , use master cam to program salve cam's cq baseaddress
    {TWIN_CQ1_BASEADDRESS,        0x00214, 0x7C78, 1,  TWIN_CQ_THR1_ADDR_CQ  }, //this is for twin's master cam use only , use master cam to program salve cam's cq baseaddress
    {TWIN_CQ10_BASEADDRESS,       0x00280, 0x7C7C, 1,  TWIN_CQ_THR10_ADDR_CQ }, //this is for twin's master cam use only , use master cam to program salve cam's cq baseaddress
#if TWIN_CQ_SW_WORKAROUND
    {CAM_CQ0_EN_,                 0x10204, 0x0204, 1,  CAMCQ_R1_CAMCQ_CQ_THR0_CTL_CQ }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ0_TRIG_,               0x10054, 0x0054, 1,  CAMCTL_R1_CAMCTL_START_CQ     }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ1_EN_,                 0x10210, 0x0210, 1,  CAMCQ_R1_CAMCQ_CQ_THR1_CTL_CQ }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ1_TRIG_,               0x10054, 0x0054, 1,  CAMCTL_R1_CAMCTL_START_CQ     }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ10_EN_,                0x1027C, 0x027C, 1,  CAMCQ_R1_CAMCQ_CQ_THR10_CTL_CQ}, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ10_TRIG_,              0x10054, 0x0054, 1,  CAMCTL_R1_CAMCTL_START_CQ     }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
#endif
    {CAM_CTL_EN_,                 0x10004, 0x0004, 6  , CAMCTL_R1_CAMCTL_EN_CQ                           }, // CAMCTL_R1_CAMCTL_EN            0004~0018
    {CAM_CTL_SEL_,                0x1001C, 0x001C, 2  , CAMCTL_R1_CAMCTL_SEL_CQ                          }, // CAMCTL_R1_CAMCTL_SEL           001C~0020
    {CAM_CTL_FMT_,                0x10024, 0x0024, 3  , CAMCTL_R1_CAMCTL_FMT_SEL_CQ                      }, // CAMCTL_R1_CAMCTL_FMT_SEL       0024~002C
    {CAM_CTL_MISC_,               0x10030, 0x0030, 1  , CAMCTL_R1_CAMCTL_MISC_CQ                         }, // CAMCTL_R1_CAMCTL_MISC          0030       for db_load_Hold
    {CAM_CTL_SW_CTL_,             0x10034, 0x0034, 1  , CAMCTL_R1_CAMCTL_SW_CTL_CQ                       }, // CAMCTL_R1_CAMCTL_SW_CTL        0034
    {CAM_CTL_RAWI_TRIG_,          0x10038, 0x0038, 1  , CAMCTL_R1_CAMCTL_RAWI_TRIG_CQ                    }, // CAMCTL_R1_CAMCTL_RAWI_TRIG     0038
    {CAM_CTL_DONE_SEL_,           0x1003C, 0x003C, 1  , CAMCTL_R1_CAMCTL_DONE_SEL_CQ                     }, // CAMCTL_R1_CAMCTL_DONE_SEL      003C
    {CAM_CTL_DONE_SEL2_,          0x10040, 0x0040, 1  , CAMCTL_R1_CAMCTL_DONE_SEL2_CQ                    }, // CAMCTL_R1_CAMCTL_DONE_SEL2     0040
    {CAM_CTL_SW_DONE_SEL_,        0x10044, 0x0044, 1  , CAMCTL_R1_CAMCTL_SW_PASS1_DONE_CQ                }, // CAMCTL_R1_CAMCTL_SW_PASS1_DONE 0044
    {CAM_CTL_FBC_GROUP_,          0x10048, 0x0048, 1  , CAMCTL_R1_CAMCTL_FBC_GROUP_CQ                    }, // CAMCTL_R1_CAMCTL_FBC_GROUP     0048
    {CAM_CTL_HLR_LKMSB_,          0x10050, 0x0050, 1  , CAMCTL_R1_CAMCTL_HLR_LKMSB_CQ                    }, // CAMCTL_R1_CAMCTL_HLR_LKMSB     0050
    {CAM_CTL_DB_SUB_SEL_,         0x10068, 0x0068, 6  , CAMCTL_R1_CAMCTL_DB_SUB_SEL_CQ                   }, // CAMCTL_R1_CAMCTL_DB_SUB_SEL    0068~007C
    {CAM_CTL_INT_EN_,             0x100C0, 0x00C0, 1  , CAMCTL_R1_CAMCTL_INT_EN_CQ                       }, // CAMCTL_R1_CAMCTL_INT_EN        00C0
    {CAM_CTL_INT2_EN_,            0x100CC, 0x00CC, 1  , CAMCTL_R1_CAMCTL_INT2_EN_CQ                      }, // CAMCTL_R1_CAMCTL_INT2_EN       00CC
    {CAM_CTL_INT3_EN_,            0x100D8, 0x00D8, 1  , CAMCTL_R1_CAMCTL_INT3_EN_CQ                      }, // CAMCTL_R1_CAMCTL_INT3_EN       00D8
    {CAM_CTL_INT4_EN_,            0x100E4, 0x00E4, 1  , CAMCTL_R1_CAMCTL_INT4_EN_CQ                      }, // CAMCTL_R1_CAMCTL_INT4_EN       00E4
    {CAM_CTL_INT5_EN_,            0x100F0, 0x00F0, 1  , CAMCTL_R1_CAMCTL_INT5_EN_CQ                      }, // CAMCTL_R1_CAMCTL_INT5_EN       00F0
    {CAM_TWIN_INFO,               0x1008C, 0x008C, 1  , CAMCTL_R1_CAMCTL_TWIN_STATUS_CQ                  }, // CAMCTL_R1_CAMCTL_TWIN_STATUS   008C
    {CAM_CQ_COUNTER,              0x140AC, 0x40AC, 1  , CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER_CQ       }, // CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER   40AC
    {CAM_FBC_IMGO_,               0X10100, 0X0100, 1  , FBC_R1_FBC_IMGO_R1_CTL1_CQ                       }, // FBC_R1_FBC_IMGO_R1_CTL1        0100~0104
    {CAM_FBC_RRZO_,               0X10108, 0X0108, 1  , FBC_R1_FBC_RRZO_R1_CTL1_CQ                       }, // FBC_R1_FBC_RRZO_R1_CTL1        0108~010C
    {CAM_FBC_UFEO_,               0X10110, 0X0110, 1  , FBC_R1_FBC_UFEO_R1_CTL1_CQ                       }, // FBC_R1_FBC_UFEO_R1_CTL1        0110~0114
    {CAM_FBC_LCSO_,               0X10120, 0X0120, 1  , FBC_R1_FBC_LCESO_R1_CTL1_CQ                      }, // FBC_R1_FBC_LCESO_R1_CTL1       0120~0124
    {CAM_FBC_AFO_,                0x10130, 0x0130, 1  , FBC_R1_FBC_AFO_R1_CTL1_CQ                        }, // FBC_R1_FBC_AFO_R1_CTL1         0130~0134
    {CAM_FBC_AAO_,                0X10138, 0X0138, 1  , FBC_R1_FBC_AAO_R1_CTL1_CQ                        }, // FBC_R1_FBC_AAO_R1_CTL1         0138~013C
    {CAM_FBC_PDO_,                0X10140, 0X0140, 1  , FBC_R1_FBC_PDO_R1_CTL1_CQ                        }, // FBC_R1_FBC_PDO_R1_CTL1         0140~0144
    {CAM_FBC_FLKO_,               0x10150, 0x0150, 1  , FBC_R1_FBC_FLKO_R1_CTL1_CQ                       }, // FBC_R1_FBC_FLKO_R1_CTL1        0150~0154
    {CAM_FBC_LMVO_,               0x10158, 0x0158, 1  , FBC_R1_FBC_LMVO_R1_CTL1_CQ                       }, // FBC_R1_FBC_LMVO_R1_CTL1        0158~015C
    {CAM_FBC_RSSO_,               0x10160, 0x0160, 1  , FBC_R1_FBC_RSSO_R1_CTL1_CQ                       }, // FBC_R1_FBC_RSSO_R1_CTL1        0160~0164
    {CAM_FBC_UFGO_,               0x10118, 0x0118, 1  , FBC_R1_FBC_UFGO_R1_CTL1_CQ                       }, // FBC_R1_FBC_UFGO_R1_CTL1        0118~011C
    {CAM_FBC_TSFSO_,              0x10148, 0x0148, 1  , FBC_R1_FBC_TSFSO_R1_CTL1_CQ                      }, // FBC_R1_FBC_TSFSO_R1_CTL1       0148~014C
    {CAM_FBC_YUVO_,               0x10170, 0x0170, 1  , FBC_R1_FBC_YUVO_R1_CTL1_CQ                       }, // FBC_R1_FBC_YUVO_R1_CTL1        0170~0174
    {CAM_FBC_YUVBO_,              0x10178, 0x0178, 1  , FBC_R1_FBC_YUVBO_R1_CTL1_CQ                      }, // FBC_R1_FBC_YUVBO_R1_CTL1       0178~017C
    {CAM_FBC_YUVCO_,              0x10180, 0x0180, 1  , FBC_R1_FBC_YUVCO_R1_CTL1_CQ                      }, // FBC_R1_FBC_YUVCO_R1_CTL1       0180~0184
    {CAM_FBC_CRZO_,               0x10188, 0x0188, 1  , FBC_R1_FBC_CRZO_R1_CTL1_CQ                       }, // FBC_R1_FBC_CRZO_R1_CTL1        0188~018C
    {CAM_FBC_CRZBO_,              0x10190, 0x0190, 1  , FBC_R1_FBC_CRZBO_R1_CTL1_CQ                      }, // FBC_R1_FBC_CRZBO_R1_CTL1       0190~0194
    {CAM_FBC_CRZO_R2_,            0x10198, 0x0198, 1  , FBC_R1_FBC_CRZO_R2_CTL1_CQ                       }, // FBC_R1_FBC_CRZO_R2_CTL1        0198~019C
    {CAM_FBC_CRZBO_R2_,           0x101A0, 0x01A0, 1  , FBC_R1_FBC_CRZBO_R2_CTL1_CQ                      }, // FBC_R1_FBC_CRZBO_R2_CTL1       01A0~01A4
    {CAM_FBC_RSSO_R2_,            0x101C8, 0x01C8, 1  , FBC_R1_FBC_RSSO_R2_CTL1_CQ                       }, // FBC_R1_FBC_RSSO_R2_CTL1        01C8~01CC
    {CAM_FBC_LTMSO_,              0x101D0, 0x01D0, 1  , FBC_R1_FBC_LTMSO_R1_CTL1_CQ                      }, // FBC_R1_FBC_LTMSO_R1_CTL1       01D0~01D4
    {CAM_CTL_DMA_V_FLIP_,         0x1400C, 0x400C, 1  , CAMDMATOP1_R1_CAMDMATOP1_VERTICAL_FLIP_EN1_CQ    }, // CAMDMATOP1_R1_CAMDMATOP1_VERTICAL_FLIP_EN1 400C
    {CAM_CTL_SPECIAL_FUN_,        0x14018, 0x4018, 1  , CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1_CQ      }, // CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1   4018
    {CAM_DMA_IMGO_BA,             0x14720, 0x4720, 1  , IMGO_R1_IMGO_BASE_ADDR_CQ                        }, // IMGO_R1_IMGO_BASE_ADDR         4720
    {CAM_DMA_IMGO_BA_OFST,        0x14724, 0x4724, 1  , IMGO_R1_IMGO_OFST_ADDR_CQ                        }, // IMGO_R1_IMGO_OFST_ADDR         4724
    {CAM_DMA_IMGO_X_,             0x1472C, 0x472C, 1  , IMGO_R1_IMGO_XSIZE_CQ                            }, // IMGO_R1_IMGO_XSIZE             472C
    {CAM_DMA_IMGO_Y_,             0x14730, 0x4730, 1  , IMGO_R1_IMGO_YSIZE_CQ                            }, // IMGO_R1_IMGO_YSIZE             4730
    {CAM_DMA_IMGO_STRIDE_,        0x14734, 0x4734, 1  , IMGO_R1_IMGO_STRIDE_CQ                           }, // IMGO_R1_IMGO_STRIDE            4734
    {CAM_DMA_IMGO_CROP_,          0x14744, 0x4744, 1  , IMGO_R1_IMGO_CROP_CQ                             }, // IMGO_R1_IMGO_CROP              4744
    {CAM_DMA_RRZO_BA,             0x14800, 0x4800, 1  , RRZO_R1_RRZO_BASE_ADDR_CQ                        }, // RRZO_R1_RRZO_BASE_ADDR         4800
    {CAM_DMA_RRZO_BA_OFST,        0x14804, 0x4804, 1  , RRZO_R1_RRZO_OFST_ADDR_CQ                        }, // RRZO_R1_RRZO_OFST_ADDR         4804
    {CAM_DMA_RRZO_X_,             0x1480C, 0x480C, 1  , RRZO_R1_RRZO_XSIZE_CQ                            }, // RRZO_R1_RRZO_XSIZE             480C
    {CAM_DMA_RRZO_Y_,             0x14810, 0x4810, 1  , RRZO_R1_RRZO_YSIZE_CQ                            }, // RRZO_R1_RRZO_YSIZE             4810
    {CAM_DMA_RRZO_STRIDE_,        0x14814, 0x4814, 1  , RRZO_R1_RRZO_STRIDE_CQ                           }, // RRZO_R1_RRZO_STRIDE            4814
    {CAM_DMA_RRZO_CROP_,          0x14824, 0x4824, 1  , RRZO_R1_RRZO_CROP_CQ                             }, // RRZO_R1_RRZO_CROP              4824
    {CAM_DMA_AAO_BA,              0x143A0, 0x43A0, 1  , AAO_R1_AAO_BASE_ADDR_CQ                          }, // AAO_R1_AAO_BASE_ADDR           43A0
    {CAM_DMA_AAO_,                0x143AC, 0x43AC, 3  , AAO_R1_AAO_XSIZE_CQ                              }, // AAO_R1_AAO_XSIZE               43AC~43B4
    {CAM_DMA_AFO_BA,              0x14410, 0x4410, 1  , AFO_R1_AFO_BASE_ADDR_CQ                          }, // AFO_R1_AFO_BASE_ADDR           4410
    {CAM_DMA_AFO_BA_OFST,         0x14414, 0x4414, 1  , AFO_R1_AFO_OFST_ADDR_CQ                          }, // AFO_R1_AFO_OFST_ADDR           4414
    {CAM_DMA_AFO_,                0x1441C, 0x441C, 3  , AFO_R1_AFO_XSIZE_CQ                              }, // AFO_R1_AFO_XSIZE               441C~4424
    {CAM_DMA_LCSO_BA,             0x14560, 0x4560, 1  , LCESO_R1_LCESO_BASE_ADDR_CQ                      }, // LCESO_R1_LCESO_BASE_ADDR       4560
    {CAM_DMA_LCSO_BA_OFST,        0x14564, 0x4564, 1  , LCESO_R1_LCESO_OFST_ADDR_CQ                      }, // LCESO_R1_LCESO_OFST_ADDR       4564
    {CAM_DMA_LCSO_,               0x1456C, 0x456C, 3  , LCESO_R1_LCESO_XSIZE_CQ                          }, // LCESO_R1_LCESO_XSIZE           456C~4574
    {CAM_DMA_UFEO_BA,             0x14790, 0x4790, 1  , UFEO_R1_UFEO_BASE_ADDR_CQ                        }, // UFEO_R1_UFEO_BASE_ADDR         4790
    {CAM_DMA_UFEO_BA_OFST,        0x14794, 0x4794, 1  , UFEO_R1_UFEO_OFST_ADDR_CQ                        }, // UFEO_R1_UFEO_OFST_ADDR         4794
    {CAM_DMA_UFEO_,               0x1479C, 0x479C, 3  , UFEO_R1_UFEO_XSIZE_CQ                            }, // UFEO_R1_UFEO_XSIZE             479C~47A4
    {CAM_DMA_PDO_BA,              0x14160, 0x4160, 1  , PDO_R1_PDO_BASE_ADDR_CQ                          }, // PDO_R1_PDO_BASE_ADDR           4160
    {CAM_DMA_PDO_BA_OFST,         0x14164, 0x4164, 1  , PDO_R1_PDO_OFST_ADDR_CQ                          }, // PDO_R1_PDO_OFST_ADDR           4164
    {CAM_DMA_PDO_,                0x1416C, 0x416C, 3  , PDO_R1_PDO_XSIZE_CQ                              }, // PDO_R1_PDO_XSIZE               416C~4174
    {CAM_DMA_FLKO_BA,             0x14480, 0x4480, 1  , FLKO_R1_FLKO_BASE_ADDR_CQ                        }, // FLKO_R1_FLKO_BASE_ADDR         4480
    {CAM_DMA_FLKO_BA_OFST,        0x14484, 0x4484, 1  , FLKO_R1_FLKO_OFST_ADDR_CQ                        }, // FLKO_R1_FLKO_OFST_ADDR         4484
    {CAM_DMA_FLKO_,               0x1448C, 0x448C, 3  , FLKO_R1_FLKO_XSIZE_CQ                            }, // FLKO_R1_FLKO_XSIZE             448C~4494
    {CAM_DMA_LMVO_BA,             0x146B0, 0x46B0, 1  , LMVO_R1_LMVO_BASE_ADDR_CQ                        }, // LMVO_R1_LMVO_BASE_ADDR         46B0
    {CAM_DMA_LMVO_BA_OFST,        0x146B4, 0x46B4, 1  , LMVO_R1_LMVO_OFST_ADDR_CQ                        }, // LMVO_R1_LMVO_OFST_ADDR         46B4
    {CAM_DMA_LMVO_,               0x146BC, 0x46BC, 3  , LMVO_R1_LMVO_XSIZE_CQ                            }, // LMVO_R1_LMVO_XSIZE             46BC~46C4
    {CAM_DMA_RSSO_A_BA,           0x14640, 0x4640, 1  , RSSO_R1_RSSO_BASE_ADDR_CQ                        }, // RSSO_R1_RSSO_BASE_ADDR         4640
    {CAM_DMA_RSSO_A_BA_OFST,      0x14644, 0x4644, 1  , RSSO_R1_RSSO_OFST_ADDR_CQ                        }, // RSSO_R1_RSSO_OFST_ADDR         4644
    {CAM_DMA_RSSO_A_,             0x1464C, 0x464C, 3  , RSSO_R1_RSSO_XSIZE_CQ                            }, // RSSO_R1_RSSO_XSIZE             464C~4654
    {CAM_DMA_UFGO_BA,             0x14870, 0x4870, 1  , UFGO_R1_UFGO_BASE_ADDR_CQ                        }, // UFGO_R1_UFGO_BASE_ADDR         4870
    {CAM_DMA_UFGO_BA_OFST,        0x14874, 0x4874, 1  , UFGO_R1_UFGO_OFST_ADDR_CQ                        }, // UFGO_R1_UFGO_OFST_ADDR         4874
    {CAM_DMA_UFGO_,               0x1487C, 0x487C, 3  , UFGO_R1_UFGO_XSIZE_CQ                            }, // UFGO_R1_UFGO_XSIZE             487C~4884
    {CAM_DMA_TSFSO_BA_,           0x142C0, 0x42C0, 1  , TSFSO_R1_TSFSO_BASE_ADDR_CQ                      }, // TSFSO_R1_TSFSO_BASE_ADDR       42C0
    {CAM_DMA_TSFSO_BA_OFST_,      0x142C4, 0x42C4, 1  , TSFSO_R1_TSFSO_OFST_ADDR_CQ                      }, // TSFSO_R1_TSFSO_OFST_ADDR       42C4
    {CAM_DMA_TSFSO_,              0x142CC, 0x42CC, 3  , TSFSO_R1_TSFSO_XSIZE_CQ                          }, // TSFSO_R1_TSFSO_XSIZE           42CC~42D4
    {CAM_DMA_LTMSO_BA_,           0x144F0, 0x44F0, 1  , LTMSO_R1_LTMSO_BASE_ADDR_CQ                      }, // LTMSO_R1_LTMSO_BASE_ADDR       44F0
    {CAM_DMA_LTMSO_BA_OFST_,      0x144F4, 0x44F4, 1  , LTMSO_R1_LTMSO_OFST_ADDR_CQ                      }, // LTMSO_R1_LTMSO_OFST_ADDR       44F4
    {CAM_DMA_LTMSO_,              0x144FC, 0x44FC, 3  , LTMSO_R1_LTMSO_XSIZE_CQ                          }, // LTMSO_R1_LTMSO_XSIZE           44FC~4504
    {CAM_DMA_YUVO_BA_,            0x148E0, 0x48E0, 1  , YUVO_R1_YUVO_BASE_ADDR_CQ                        }, // YUVO_R1_YUVO_BASE_ADDR         48E0
    {CAM_DMA_YUVO_BA_OFST_,       0x148E4, 0x48E4, 1  , YUVO_R1_YUVO_OFST_ADDR_CQ                        }, // YUVO_R1_YUVO_OFST_ADDR         48E4
    {CAM_DMA_YUVO_,               0x148EC, 0x48EC, 3  , YUVO_R1_YUVO_XSIZE_CQ                            }, // YUVO_R1_YUVO_XSIZE             48EC~48F4
    {CAM_DMA_YUVO_CROP_,          0x14904, 0x4904, 1  , YUVO_R1_YUVO_CROP_CQ                             }, // YUVO_R1_YUVO_CROP              4904
    {CAM_DMA_YUVBO_BA_,           0x14950, 0x4950, 1  , YUVBO_R1_YUVBO_BASE_ADDR_CQ                      }, // YUVBO_R1_YUVBO_BASE_ADDR       4950
    {CAM_DMA_YUVBO_BA_OFST_,      0x14954, 0x4954, 1  , YUVBO_R1_YUVBO_OFST_ADDR_CQ                      }, // YUVBO_R1_YUVBO_OFST_ADDR       4954
    {CAM_DMA_YUVBO_,              0x1495C, 0x495C, 3  , YUVBO_R1_YUVBO_XSIZE_CQ                          }, // YUVBO_R1_YUVBO_XSIZE           495C~4964
    {CAM_DMA_YUVBO_CROP_,         0x14974, 0x4974, 1  , YUVBO_R1_YUVBO_CROP_CQ                           }, // YUVBO_R1_YUVBO_CROP            4974
    {CAM_DMA_YUVCO_BA_,           0x149C0, 0x49C0, 1  , YUVCO_R1_YUVCO_BASE_ADDR_CQ                      }, // YUVCO_R1_YUVCO_BASE_ADDR       49C0
    {CAM_DMA_YUVCO_BA_OFST_,      0x149C4, 0x49C4, 1  , YUVCO_R1_YUVCO_OFST_ADDR_CQ                      }, // YUVCO_R1_YUVCO_OFST_ADDR       49C4
    {CAM_DMA_YUVCO_,              0x149CC, 0x49CC, 1  , YUVCO_R1_YUVCO_XSIZE_CQ                          }, // YUVCO_R1_YUVCO_XSIZE           49CC~49D4
    {CAM_DMA_YUVCO_CROP_,         0x149E4, 0x49E4, 1  , YUVCO_R1_YUVCO_CROP_CQ                           }, // YUVCO_R1_YUVCO_CROP            49E4
    {CAM_DMA_CRZO_BA_,            0x14A30, 0x4A30, 1  , CRZO_R1_CRZO_BASE_ADDR_CQ                        }, // CRZO_R1_CRZO_BASE_ADDR         4A30
    {CAM_DMA_CRZO_BA_OFST_,       0x14A34, 0x4A34, 1  , CRZO_R1_CRZO_OFST_ADDR_CQ                        }, // CRZO_R1_CRZO_OFST_ADDR         4A34
    {CAM_DMA_CRZO,                0x14A3C, 0x4A3C, 3  , CRZO_R1_CRZO_XSIZE_CQ                            }, // CRZO_R1_CRZO_XSIZE             4A3C~4A44
    {CAM_DMA_CRZBO_BA_,           0x14AA0, 0x4AA0, 1  , CRZBO_R1_CRZBO_BASE_ADDR_CQ                      }, // CRZBO_R1_CRZBO_BASE_ADDR       4AA0
    {CAM_DMA_CRZBO_BA_OFST_,      0x14AA4, 0x4AA4, 1  , CRZBO_R1_CRZBO_OFST_ADDR_CQ                      }, // CRZBO_R1_CRZBO_OFST_ADDR       4AA4
    {CAM_DMA_CRZBO,               0x14AAC, 0x4AAC, 3  , CRZBO_R1_CRZBO_XSIZE_CQ                          }, // CRZBO_R1_CRZBO_XSIZE           4AAC~4AB4
    {CAM_DMA_CRZO_R2_BA_,         0x14B10, 0x4B10, 1  , CRZO_R2_CRZO_BASE_ADDR_CQ                        }, // CRZO_R2_CRZO_BASE_ADDR         4B10
    {CAM_DMA_CRZO_R2_BA_OFST_,    0x14B14, 0x4B14, 1  , CRZO_R2_CRZO_OFST_ADDR_CQ                        }, // CRZO_R2_CRZO_OFST_ADDR         4B14
    {CAM_DMA_CRZO_R2_,            0x14B1C, 0x4B1C, 3  , CRZO_R2_CRZO_XSIZE_CQ                            }, // CRZO_R2_CRZO_XSIZE             4B1C~4B24
    {CAM_DMA_CRZBO_R2_BA_,        0x14B80, 0x4B80, 1  , CRZBO_R2_CRZBO_BASE_ADDR_CQ                      }, // CRZBO_R2_CRZBO_BASE_ADDR       4B80
    {CAM_DMA_CRZBO_R2_BA_OFST_,   0x14B84, 0x4B84, 1  , CRZBO_R2_CRZBO_OFST_ADDR_CQ                      }, // CRZBO_R2_CRZBO_OFST_ADDR       4B84
    {CAM_DMA_CRZBO_R2_,           0x14B8C, 0x4B8C, 3  , CRZBO_R2_CRZBO_XSIZE_CQ                          }, // CRZBO_R2_CRZBO_XSIZE           4B8C~4B94
    {CAM_DMA_RSSO_R2_BA_,         0x14BF0, 0x4BF0, 1  , RSSO_R2_RSSO_BASE_ADDR_CQ                        }, // RSSO_R2_RSSO_BASE_ADDR         4BF0
    {CAM_DMA_RSSO_R2_BA_OFST_,    0x14BF4, 0x4BF4, 1  , RSSO_R2_RSSO_OFST_ADDR_CQ                        }, // RSSO_R2_RSSO_OFST_ADDR         4BF4
    {CAM_DMA_RSSO_R2_,            0x14BFC, 0x4BFC, 3  , RSSO_R2_RSSO_XSIZE_CQ                            }, // RSSO_R2_RSSO_XSIZE             4BFC~4C04
    {CAM_DMA_RAWI_,               0x14100, 0x4100, 6  , RAWI_R2_RAWI_BASE_ADDR_CQ                        }, // RAWI_R2_RAWI_BASE_ADDR         4100~4114
    {CAM_DMA_BPCI_,               0x141D0, 0x41D0, 6  , BPCI_R1_BPCI_BASE_ADDR_CQ                        }, // BPCI_R1_BPCI_BASE_ADDR         41D0~41E4
    {CAM_DMA_BPCI_R2_,            0x14200, 0x4200, 6  , BPCI_R2_BPCI_BASE_ADDR_CQ                        }, // BPCI_R2_BPCI_BASE_ADDR         4200~4214
    {CAM_DMA_UFDI_R2_,            0x14260, 0x4260, 6  , UFDI_R2_UFDI_BASE_ADDR_CQ                        }, // UFDI_R2_UFDI_BASE_ADDR         4260~4274
    {CAM_DMA_LSCI_,               0x14290, 0x4290, 6  , LSCI_R1_LSCI_BASE_ADDR_CQ                        }, // LSCI_R1_LSCI_BASE_ADDR         4290~42A4
    {CAM_DMA_PDI_,                0x14130, 0x4130, 6  , PDI_R1_PDI_BASE_ADDR_CQ                          }, // PDI_R1_PDI_BASE_ADDR           4130~4144
    {CAM_DMA_RSV1_,               0x10110, 0x0110, 1  , CAMADL_R1_CAMADL_DMA_RSV1_CQ                     }, // CAMADL_R1_CAMADL_DMA_RSV1      0110                  for smi mask, dtwin
    {CAM_MAGIC_NUM_,              0x140A4, 0x40A4, 1  , CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM_CQ        }, // CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM         40A4
    {CAM_DATA_PAT_,               0x140A8, 0x40A8, 1  , CAMDMATOP1_R1_CAMDMATOP1_DMA_DATA_PATTERN_CQ     }, // CAMDMATOP1_R1_CAMDMATOP1_DMA_DATA_PATTERN      40A8
    {CAM_DMA_RSV6_,               0x10124, 0x0124, 1  , CAMADL_R1_CAMADL_DMA_RSV6_CQ                     }, // CAMADL_R1_CAMADL_DMA_RSV6      0124                  for smi mask, dtwin
    {CAM_DMA_FH_EN_,              0x140BC, 0x40BC, 1  , CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1_CQ }, // CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1  40BC
    {CAM_DMA_FH_IMGO_,            0x1474C, 0x474C, 1  , IMGO_R1_IMGO_FH_BASE_ADDR_CQ                     }, // IMGO_R1_IMGO_FH_BASE_ADDR      474C
    {CAM_DMA_FH_IMGO_SPARE_,      0x14750, 0x4750, 8  , IMGO_R1_IMGO_FH_SPARE_2_CQ                       }, // IMGO_R1_IMGO_FH_SPARE_2        4750~476C
    {CAM_DMA_FH_IMGO_SPARE_CROP,  0x14770, 0x4770, 2  , IMGO_R1_IMGO_FH_SPARE_10_CQ                      }, // IMGO_R1_IMGO_FH_SPARE_10       4770~4774
    {CAM_DMA_FH_IMGO_SPARE_PA,    0x14778, 0x4778, 1  , IMGO_R1_IMGO_FH_SPARE_12_CQ                      }, // IMGO_R1_IMGO_FH_SPARE_12       4778
    {CAM_DMA_FH_RRZO_,            0x1482C, 0x482C, 1  , RRZO_R1_RRZO_FH_BASE_ADDR_CQ                     }, // RRZO_R1_RRZO_FH_BASE_ADDR      482C
    {CAM_DMA_FH_RRZO_SPARE_,      0x14830, 0x4830, 7  , RRZO_R1_RRZO_FH_SPARE_2_CQ                       }, // RRZO_R1_RRZO_FH_SPARE_2        4830~4848
    {CAM_DMA_FH_RRZO_SPARE_CROP,  0x1484C, 0x484C, 3  , RRZO_R1_RRZO_FH_SPARE_9_CQ                       }, // RRZO_R1_RRZO_FH_SPARE_9        484C~4854
    {CAM_DMA_FH_RRZO_SPARE_PA,    0x14858, 0x4858, 1  , RRZO_R1_RRZO_FH_SPARE_12_CQ                      }, // RRZO_R1_RRZO_FH_SPARE_12       4858
    {CAM_DMA_FH_AAO_,             0x143CC, 0x43CC, 1  , AAO_R1_AAO_FH_BASE_ADDR_CQ                       }, // AAO_R1_AAO_FH_BASE_ADDR        43CC
    {CAM_DMA_FH_AAO_SPARE_CQ0_,   0x143D0, 0x43D0, 8  , AAO_R1_AAO_FH_SPARE_2_CQ                         }, // AAO_R1_AAO_FH_SPARE_2          43D0~43EC
    {CAM_DMA_FH_AAO_SPARE_,       0x143F0, 0x43F0, 3  , AAO_R1_AAO_FH_SPARE_10_CQ                        }, // AAO_R1_AAO_FH_SPARE_10         43F0~43F8
    {CAM_DMA_FH_AFO_,             0x1443C, 0x443C, 1  , AFO_R1_AFO_FH_BASE_ADDR_CQ                       }, // AFO_R1_AFO_FH_BASE_ADDR        443C
    {CAM_DMA_FH_AFO_SPARE_CQ0_,   0x14440, 0x4440, 8  , AFO_R1_AFO_FH_SPARE_2_CQ                         }, // AFO_R1_AFO_FH_SPARE_2          4440~445C
    {CAM_DMA_FH_AFO_SPARE_,       0x14460, 0x4460, 3  , AFO_R1_AFO_FH_SPARE_10_CQ                        }, // AFO_R1_AFO_FH_SPARE_10         4460~4468
    {CAM_DMA_FH_LCSO_,            0x1458C, 0x458C, 1  , LCESO_R1_LCESO_FH_BASE_ADDR_CQ                   }, // LCESO_R1_LCESO_FH_BASE_ADDR    458C
    {CAM_DMA_FH_LCSO_SPARE_,      0x14590, 0x4590, 4  , LCESO_R1_LCESO_FH_SPARE_2_CQ                     }, // LCESO_R1_LCESO_FH_SPARE_2      4590~459C
    {CAM_DMA_FH_LCSO_SPARE_PA,    0x145B8, 0x45B8, 1  , LCESO_R1_LCESO_FH_SPARE_12_CQ                    }, // LCESO_R1_LCESO_FH_SPARE_12     45B8
    {CAM_DMA_FH_UFEO_,            0x147BC, 0x47BC, 1  , UFEO_R1_UFEO_FH_BASE_ADDR_CQ                     }, // UFEO_R1_UFEO_FH_BASE_ADDR      47BC
    {CAM_DMA_FH_UFEO_SPARE_,      0x147C0, 0x47C0, 8  , UFEO_R1_UFEO_FH_SPARE_2_CQ                       }, // UFEO_R1_UFEO_FH_SPARE_2        47C0~47DC
    {CAM_DMA_FH_UFEO_SPARE_PA,    0x147E8, 0x47E8, 1  , UFEO_R1_UFEO_FH_SPARE_12_CQ                      }, // UFEO_R1_UFEO_FH_SPARE_12       47E8
    {CAM_DMA_FH_PDO_,             0x1418C, 0x418C, 1  , PDO_R1_PDO_FH_BASE_ADDR_CQ                       }, // PDO_R1_PDO_FH_BASE_ADDR        418C
    {CAM_DMA_FH_PDO_SPARE_CQ0_,   0x14190, 0x4190, 8  , PDO_R1_PDO_FH_SPARE_2_CQ                         }, // PDO_R1_PDO_FH_SPARE_2          4190~41AC
    {CAM_DMA_FH_PDO_SPARE_,       0x141B0, 0x41B0, 3  , PDO_R1_PDO_FH_SPARE_10_CQ                        }, // PDO_R1_PDO_FH_SPARE_10         41B0~41B8
    {CAM_DMA_FH_FLKO_,            0x144AC, 0x44AC, 1  , FLKO_R1_FLKO_FH_BASE_ADDR_CQ                     }, // FLKO_R1_FLKO_FH_BASE_ADDR      44AC
    {CAM_DMA_FH_FLKO_SPARE_CQ0_,  0x144B0, 0x44B0, 7  , FLKO_R1_FLKO_FH_SPARE_2_CQ                       }, // FLKO_R1_FLKO_FH_SPARE_2        44B0~44C8
    {CAM_DMA_FH_FLKO_SPARE_,      0x144CC, 0x44CC, 4  , FLKO_R1_FLKO_FH_SPARE_9_CQ                       }, // FLKO_R1_FLKO_FH_SPARE_9        44CC~44D8
    {CAM_DMA_FH_LMVO_,            0x146DC, 0x46DC, 1  , LMVO_R1_LMVO_FH_BASE_ADDR_CQ                     }, // LMVO_R1_LMVO_FH_BASE_ADDR      46DC
    {CAM_DMA_FH_LMVO_SPARE_,      0x146E0, 0x46E0, 4  , LMVO_R1_LMVO_FH_SPARE_2_CQ                       }, // LMVO_R1_LMVO_FH_SPARE_2        46E0~46EC
    {CAM_DMA_FH_LMVO_SPARE_PA,    0x14708, 0x4708, 1  , LMVO_R1_LMVO_FH_SPARE_12_CQ                      }, // LMVO_R1_LMVO_FH_SPARE_12       4708
    {CAM_DMA_FH_RSSO_A_,          0x1466C, 0x466C, 1  , RSSO_R1_RSSO_FH_BASE_ADDR_CQ                     }, // RSSO_R1_RSSO_FH_BASE_ADDR      466C
    {CAM_DMA_FH_RSSO_A_SPARE_,    0x14670, 0x4670, 5  , RSSO_R1_RSSO_FH_SPARE_2_CQ                       }, // RSSO_R1_RSSO_FH_SPARE_2        4670~4680
    {CAM_DMA_FH_RSSO_A_SPARE_PA,  0x14698, 0x4698, 1  , RSSO_R1_RSSO_FH_SPARE_12_CQ                      }, // RSSO_R1_RSSO_FH_SPARE_12       4698
    {CAM_DMA_FH_UFGO_,            0x1489C, 0x489C, 1  , UFGO_R1_UFGO_FH_BASE_ADDR_CQ                     }, // UFGO_R1_UFGO_FH_BASE_ADDR      489C
    {CAM_DMA_FH_UFGO_SPARE_,      0x148A0, 0x48A0, 8  , UFGO_R1_UFGO_FH_SPARE_2_CQ                       }, // UFGO_R1_UFGO_FH_SPARE_2        48A0~48BC
    {CAM_DMA_FH_UFGO_SPARE_PA,    0x148C8, 0x48C8, 1  , UFGO_R1_UFGO_FH_SPARE_12_CQ                      }, // UFGO_R1_UFGO_FH_SPARE_12       48C8
    {CAM_DMA_FH_TSFSO_,           0x142EC, 0x42EC, 1  , TSFSO_R1_TSFSO_FH_BASE_ADDR_CQ                   }, // TSFSO_R1_TSFSO_FH_BASE_ADDR    42EC
    {CAM_DMA_FH_TSFSO_SPARE_CQ0_, 0x142F0, 0x42F0, 8  , TSFSO_R1_TSFSO_FH_SPARE_2_CQ                     }, // TSFSO_R1_TSFSO_FH_SPARE_2      42F0~430C
    {CAM_DMA_FH_TSFSO_SPARE_,     0x14310, 0x4310, 3  , TSFSO_R1_TSFSO_FH_SPARE_10_CQ                    }, // TSFSO_R1_TSFSO_FH_SPARE_10     4310~4314
    {CAM_DMA_FH_LTMSO_,           0x1451C, 0x451C, 1  , LTMSO_R1_LTMSO_FH_BASE_ADDR_CQ                   }, // LTMSO_R1_LTMSO_FH_BASE_ADDR    451C
    {CAM_DMA_FH_LTMSO_SPARE_,     0x14520, 0x4520, 11 , LTMSO_R1_LTMSO_FH_SPARE_2_CQ                     }, // LTMSO_R1_LTMSO_FH_SPARE_2      4520~4548
    {CAM_DMA_FH_YUVO_,            0x1490C, 0x490C, 1  , YUVO_R1_YUVO_FH_BASE_ADDR_CQ                     }, // YUVO_R1_YUVO_FH_BASE_ADDR      490C
    {CAM_DMA_FH_YUVO_SPARE_,      0x14910, 0x4910, 10 , YUVO_R1_YUVO_FH_SPARE_2_CQ                       }, // YUVO_R1_YUVO_FH_SPARE_2        4910~4934
    {CAM_DMA_FH_YUVO_SPARE_PA,    0x14938, 0x4938, 1  , YUVO_R1_YUVO_FH_SPARE_12_CQ                      }, // YUVO_R1_YUVO_FH_SPARE_12       4938
    {CAM_DMA_FH_YUVBO_,           0x1497C, 0x497C, 1  , YUVBO_R1_YUVBO_FH_BASE_ADDR_CQ                   }, // YUVBO_R1_YUVBO_FH_BASE_ADDR    497C
    {CAM_DMA_FH_YUVBO_SPARE_,     0x14980, 0x4980, 10 , YUVBO_R1_YUVBO_FH_SPARE_2_CQ                     }, // YUVBO_R1_YUVBO_FH_SPARE_2      4980~49A4
    {CAM_DMA_FH_YUVBO_SPARE_PA,   0x149A8, 0x49A8, 1  , YUVBO_R1_YUVBO_FH_SPARE_12_CQ                    }, // YUVBO_R1_YUVBO_FH_SPARE_12     49A8
    {CAM_DMA_FH_YUVCO_,           0x149EC, 0x49EC, 1  , YUVCO_R1_YUVCO_FH_BASE_ADDR_CQ                   }, // YUVCO_R1_YUVCO_FH_BASE_ADDR    49EC
    {CAM_DMA_FH_YUVCO_SPARE_,     0x149F0, 0x49F0, 10 , YUVCO_R1_YUVCO_FH_SPARE_2_CQ                     }, // YUVCO_R1_YUVCO_FH_SPARE_2      49F0~4A14
    {CAM_DMA_FH_YUVCO_SPARE_PA,   0x14A18, 0x4A18, 1  , YUVCO_R1_YUVCO_FH_SPARE_12_CQ                    }, // YUVCO_R1_YUVCO_FH_SPARE_12     4A18
    {CAM_DMA_FH_CRZO_,            0x14A5C, 0x4A5C, 1  , CRZO_R1_CRZO_FH_BASE_ADDR_CQ                     }, // CRZO_R1_CRZO_FH_BASE_ADDR      4A5C
    {CAM_DMA_FH_CRZO_SPARE_,      0x14A60, 0x4A60, 10 , CRZO_R1_CRZO_FH_SPARE_2_CQ                       }, // CRZO_R1_CRZO_FH_SPARE_2        4A60~4A84
    {CAM_DMA_FH_CRZO_SPARE_PA,    0x14A88, 0x4A88, 1  , CRZO_R1_CRZO_FH_SPARE_12_CQ                      }, // CRZO_R1_CRZO_FH_SPARE_12       4A88
    {CAM_DMA_FH_CRZBO_,           0x14ACC, 0x4ACC, 1  , CRZBO_R1_CRZBO_FH_BASE_ADDR_CQ                   }, // CRZBO_R1_CRZBO_FH_BASE_ADDR    4ACC
    {CAM_DMA_FH_CRZBO_SPARE_,     0x14AD0, 0x4AD0, 9  , CRZBO_R1_CRZBO_FH_SPARE_2_CQ                     }, // CRZBO_R1_CRZBO_FH_SPARE_2      4AD0~4AF0
    {CAM_DMA_FH_CRZBO_SPARE_PA,   0x14AF8, 0x4AF8, 1  , CRZBO_R1_CRZBO_FH_SPARE_12_CQ                    }, // CRZBO_R1_CRZBO_FH_SPARE_12     4AF8
    {CAM_DMA_FH_CRZO_R2_,         0x14B3C, 0x4B3C, 1  , CRZO_R2_CRZO_FH_BASE_ADDR_CQ                     }, // CRZO_R2_CRZO_FH_BASE_ADDR      4B3C
    {CAM_DMA_FH_CRZO_R2_SPARE_,   0x14B40, 0x4B40, 10 , CRZO_R2_CRZO_FH_SPARE_2_CQ                       }, // CRZO_R2_CRZO_FH_SPARE_2        4B40~4B64
    {CAM_DMA_FH_CRZO_R2_SPARE_PA, 0x14B68, 0x4B68, 1  , CRZO_R2_CRZO_FH_SPARE_12_CQ                      }, // CRZO_R2_CRZO_FH_SPARE_12       4B68
    {CAM_DMA_FH_CRZBO_R2_,        0x14BAC, 0x4BAC, 1  , CRZBO_R2_CRZBO_FH_BASE_ADDR_CQ                   }, // CRZBO_R2_CRZBO_FH_BASE_ADDR    4BAC
    {CAM_DMA_FH_CRZBO_R2_SPARE_,  0x14BB0, 0x4BB0, 9  , CRZBO_R2_CRZBO_FH_SPARE_2_CQ                     }, // CRZBO_R2_CRZBO_FH_SPARE_2      4BB0~4BD0
    {CAM_DMA_FH_CRZBO_R2_SPARE_PA,0x14BD8, 0x4BD8, 1  , CRZBO_R2_CRZBO_FH_SPARE_12_CQ                    }, // CRZBO_R2_CRZBO_FH_SPARE_12     4BD8
    {CAM_DMA_FH_RSSO_R2_,         0x14C1C, 0x4C1C, 1  , RSSO_R2_RSSO_FH_BASE_ADDR_CQ                     }, // RSSO_R2_RSSO_FH_BASE_ADDR      4C1C
    {CAM_DMA_FH_RSSO_R2_SPARE_,   0x14C20, 0x4C20, 11 , RSSO_R2_RSSO_FH_SPARE_2_CQ                       }, // RSSO_R2_RSSO_FH_SPARE_2        4C20~4C48
    {CAM_CQ_THRE0_ADDR_,          0x10208, 0x0208, 1  , CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR
    {CAM_CQ_THRE0_SIZE_,          0x1020C, 0x020C, 1  , CAMCQ_R1_CAMCQ_CQ_THR0_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR0_DESC_SIZE
    {CAM_CQ_THRE1_ADDR_,          0x10214, 0x0214, 1  , CAMCQ_R1_CAMCQ_CQ_THR1_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR1_BASEADDR
    {CAM_CQ_THRE1_SIZE_,          0x10218, 0x0218, 1  , CAMCQ_R1_CAMCQ_CQ_THR1_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR1_DESC_SIZE
    {CAM_CQ_THRE2_ADDR_,          0x10220, 0x0220, 1  , CAMCQ_R1_CAMCQ_CQ_THR2_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR2_BASEADDR
    {CAM_CQ_THRE2_SIZE_,          0x10224, 0x0224, 1  , CAMCQ_R1_CAMCQ_CQ_THR2_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR2_DESC_SIZE
    {CAM_CQ_THRE3_ADDR_,          0x1022C, 0x022C, 1  , CAMCQ_R1_CAMCQ_CQ_THR3_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR3_BASEADDR
    {CAM_CQ_THRE3_SIZE_,          0x10230, 0x0230, 1  , CAMCQ_R1_CAMCQ_CQ_THR3_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR3_DESC_SIZE
    {CAM_CQ_THRE4_ADDR_,          0x10238, 0x0238, 1  , CAMCQ_R1_CAMCQ_CQ_THR4_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR4_BASEADDR
    {CAM_CQ_THRE4_SIZE_,          0x1023C, 0x023C, 1  , CAMCQ_R1_CAMCQ_CQ_THR4_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR4_DESC_SIZE
    {CAM_CQ_THRE5_ADDR_,          0x10244, 0x0244, 1  , CAMCQ_R1_CAMCQ_CQ_THR5_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR5_BASEADDR
    {CAM_CQ_THRE5_SIZE_,          0x10248, 0x0248, 1  , CAMCQ_R1_CAMCQ_CQ_THR5_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR5_DESC_SIZE
    {CAM_CQ_THRE6_ADDR_,          0x10250, 0x0250, 1  , CAMCQ_R1_CAMCQ_CQ_THR6_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR6_BASEADDR
    {CAM_CQ_THRE6_SIZE_,          0x10254, 0x0254, 1  , CAMCQ_R1_CAMCQ_CQ_THR6_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR6_DESC_SIZE
    {CAM_CQ_THRE7_ADDR_,          0x1025C, 0x025C, 1  , CAMCQ_R1_CAMCQ_CQ_THR7_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR7_BASEADDR
    {CAM_CQ_THRE7_SIZE_,          0x10260, 0x0260, 1  , CAMCQ_R1_CAMCQ_CQ_THR7_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR7_DESC_SIZE
    {CAM_CQ_THRE8_ADDR_,          0x10268, 0x0268, 1  , CAMCQ_R1_CAMCQ_CQ_THR8_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR8_BASEADDR
    {CAM_CQ_THRE8_SIZE_,          0x1026C, 0x026C, 1  , CAMCQ_R1_CAMCQ_CQ_THR8_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR8_DESC_SIZE
    {CAM_CQ_THRE9_ADDR_,          0x10274, 0x0274, 1  , CAMCQ_R1_CAMCQ_CQ_THR9_BASEADDR_CQ               }, // CAMCQ_R1_CAMCQ_CQ_THR9_BASEADDR
    {CAM_CQ_THRE9_SIZE_,          0x10278, 0x0278, 1  , CAMCQ_R1_CAMCQ_CQ_THR9_DESC_SIZE_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR9_DESC_SIZE
    {CAM_CQ_THRE10_ADDR_,         0x10280, 0x0280, 1  , CAMCQ_R1_CAMCQ_CQ_THR10_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR10_BASEADDR
    {CAM_CQ_THRE10_SIZE_,         0x10284, 0x0284, 1  , CAMCQ_R1_CAMCQ_CQ_THR10_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR10_DESC_SIZE
    {CAM_CQ_THRE11_ADDR_,         0x1028C, 0x028C, 1  , CAMCQ_R1_CAMCQ_CQ_THR11_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR11_BASEADDR
    {CAM_CQ_THRE11_SIZE_,         0x10290, 0x0290, 1  , CAMCQ_R1_CAMCQ_CQ_THR11_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR11_DESC_SIZE
    {CAM_CQ_THRE12_ADDR_,         0x10298, 0x0298, 1  , CAMCQ_R1_CAMCQ_CQ_THR12_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR12_BASEADDR
    {CAM_CQ_THRE12_SIZE_,         0x1029C, 0x029C, 1  , CAMCQ_R1_CAMCQ_CQ_THR12_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR12_DESC_SIZE
    {CAM_CQ_THRE13_ADDR_,         0x102A4, 0x02A4, 1  , CAMCQ_R1_CAMCQ_CQ_THR13_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR13_BASEADDR
    {CAM_CQ_THRE13_SIZE_,         0x102A8, 0x02A8, 1  , CAMCQ_R1_CAMCQ_CQ_THR13_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR13_DESC_SIZE
    {CAM_CQ_THRE14_ADDR_,         0x102B0, 0x02B0, 1  , CAMCQ_R1_CAMCQ_CQ_THR14_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR14_BASEADDR
    {CAM_CQ_THRE14_SIZE_,         0x102B4, 0x02B4, 1  , CAMCQ_R1_CAMCQ_CQ_THR14_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR14_DESC_SIZE
    {CAM_CQ_THRE15_ADDR_,         0x102BC, 0x02BC, 1  , CAMCQ_R1_CAMCQ_CQ_THR15_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR15_BASEADDR
    {CAM_CQ_THRE15_SIZE_,         0x102C0, 0x02C0, 1  , CAMCQ_R1_CAMCQ_CQ_THR15_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR15_DESC_SIZE
    {CAM_CQ_THRE16_ADDR_,         0x102C8, 0x02C8, 1  , CAMCQ_R1_CAMCQ_CQ_THR16_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR16_BASEADDR
    {CAM_CQ_THRE16_SIZE_,         0x102CC, 0x02CC, 1  , CAMCQ_R1_CAMCQ_CQ_THR16_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR16_DESC_SIZE
    {CAM_CQ_THRE17_ADDR_,         0x102D4, 0x02D4, 1  , CAMCQ_R1_CAMCQ_CQ_THR17_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR17_BASEADDR
    {CAM_CQ_THRE17_SIZE_,         0x102D8, 0x02D8, 1  , CAMCQ_R1_CAMCQ_CQ_THR17_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR17_DESC_SIZE
    {CAM_CQ_THRE18_ADDR_,         0x102E0, 0x02E0, 1  , CAMCQ_R1_CAMCQ_CQ_THR18_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR18_BASEADDR
    {CAM_CQ_THRE18_SIZE_,         0x102E4, 0x02E4, 1  , CAMCQ_R1_CAMCQ_CQ_THR18_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR18_DESC_SIZE
    {CAM_CQ_THRE19_ADDR_,         0x102EC, 0x02EC, 1  , CAMCQ_R1_CAMCQ_CQ_THR19_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR19_BASEADDR
    {CAM_CQ_THRE19_SIZE_,         0x102F0, 0x02F0, 1  , CAMCQ_R1_CAMCQ_CQ_THR19_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR19_DESC_SIZE
    {CAM_CQ_THRE20_ADDR_,         0x102F8, 0x02F8, 1  , CAMCQ_R1_CAMCQ_CQ_THR20_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR20_BASEADDR
    {CAM_CQ_THRE20_SIZE_,         0x102FC, 0x02FC, 1  , CAMCQ_R1_CAMCQ_CQ_THR20_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR20_DESC_SIZE
    {CAM_CQ_THRE21_ADDR_,         0x10304, 0x0304, 1  , CAMCQ_R1_CAMCQ_CQ_THR21_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR21_BASEADDR
    {CAM_CQ_THRE21_SIZE_,         0x10308, 0x0308, 1  , CAMCQ_R1_CAMCQ_CQ_THR21_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR21_DESC_SIZE
    {CAM_CQ_THRE22_ADDR_,         0x10310, 0x0310, 1  , CAMCQ_R1_CAMCQ_CQ_THR22_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR22_BASEADDR
    {CAM_CQ_THRE22_SIZE_,         0x10314, 0x0314, 1  , CAMCQ_R1_CAMCQ_CQ_THR22_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR22_DESC_SIZE
    {CAM_CQ_THRE23_ADDR_,         0x1031C, 0x031C, 1  , CAMCQ_R1_CAMCQ_CQ_THR23_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR23_BASEADDR
    {CAM_CQ_THRE23_SIZE_,         0x10320, 0x0320, 1  , CAMCQ_R1_CAMCQ_CQ_THR23_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR23_DESC_SIZE
    {CAM_CQ_THRE24_ADDR_,         0x10328, 0x0328, 1  , CAMCQ_R1_CAMCQ_CQ_THR24_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR24_BASEADDR
    {CAM_CQ_THRE24_SIZE_,         0x1032C, 0x032C, 1  , CAMCQ_R1_CAMCQ_CQ_THR24_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR24_DESC_SIZE
    {CAM_CQ_THRE25_ADDR_,         0x10334, 0x0334, 1  , CAMCQ_R1_CAMCQ_CQ_THR25_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR25_BASEADDR
    {CAM_CQ_THRE25_SIZE_,         0x10338, 0x0338, 1  , CAMCQ_R1_CAMCQ_CQ_THR25_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR25_DESC_SIZE
    {CAM_CQ_THRE26_ADDR_,         0x10340, 0x0340, 1  , CAMCQ_R1_CAMCQ_CQ_THR26_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR26_BASEADDR
    {CAM_CQ_THRE26_SIZE_,         0x10344, 0x0344, 1  , CAMCQ_R1_CAMCQ_CQ_THR26_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR26_DESC_SIZE
    {CAM_CQ_THRE27_ADDR_,         0x1034C, 0x034C, 1  , CAMCQ_R1_CAMCQ_CQ_THR27_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR27_BASEADDR
    {CAM_CQ_THRE27_SIZE_,         0x10350, 0x0350, 1  , CAMCQ_R1_CAMCQ_CQ_THR27_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR27_DESC_SIZE
    {CAM_CQ_THRE28_ADDR_,         0x10358, 0x0358, 1  , CAMCQ_R1_CAMCQ_CQ_THR28_BASEADDR_CQ              }, // CAMCQ_R1_CAMCQ_CQ_THR28_BASEADDR
    {CAM_CQ_THRE28_SIZE_,         0x1035C, 0x035C, 1  , CAMCQ_R1_CAMCQ_CQ_THR28_DESC_SIZE_CQ             }, // CAMCQ_R1_CAMCQ_CQ_THR28_DESC_SIZE
    {CAM_ISP_PBN_,                0x11780, 0x1780, 3  , PBN_R1_PBN_PBN_TYPE_CQ                           }, // PBN_R1_PBN_PBN_TYPE            1780~1788
    {CAM_ISP_DBN_,                0x10D40, 0x0D40, 3  , DBN_R1_DBN_GAIN_CQ                               }, // DBN_R1_DBN_GAIN                0D40~0D48
    {CAM_ISP_BIN_,                0x10D80, 0x0D80, 6  , BIN_R1_BIN_CTL_CQ                                }, // BIN_R1_BIN_CTL                 0D80~0D94
    {CAM_ISP_OBC_,                0x104C0, 0x04C0, 28 , OBC_R1_OBC_CTL_CQ                                }, // OBC_R1_OBC_CTL                 04C0~052C
    {CAM_ISP_LSC_,                0x105C0, 0x05C0, 11 , LSC_R1_LSC_CTL1_CQ                               }, // LSC_R1_LSC_CTL1                05C0~05E8
    {CAM_ISP_RRZ_,                0x11100, 0x1100, 14 , RRZ_R1_RRZ_CTL_CQ                                }, // RRZ_R1_RRZ_CTL                 1100~1134
    {CAM_ISP_UFE_,                0x12780, 0x2780, 1  , UFE_R1_UFE_CON_CQ                                }, // UFE_R1_UFE_CON                 2780
    {CAM_ISP_QBIN_,               0x11580, 0x1580, 1  , QBIN_R1_QBIN_CTL_CQ                              }, // QBIN_R1_QBIN_CTL               1580        QBIN == QBN
    {CAM_ISP_PDE_,                0x11700, 0x1700, 2  , PDE_R1_PDE_TBLI1_CQ                              }, // PDE_R1_PDE_TBLI1               1700~1704
    {CAM_ISP_LCS_SIZE,            0x10804, 0x0804, 2  , LCES_R1_LCES_IN_SIZE_CQ                          }, // LCES_R1_LCES_IN_SIZE           0804~0808
    {CAM_ISP_LCS_LOG_,            0x1080C, 0x080C, 1  , LCES_R1_LCES_LOG_CQ                              }, // LCES_R1_LCES_LOG               080C
    {CAM_ISP_LCS_LRZR,            0x10810, 0x0810, 2  , LCES_R1_LCES_LRZRX_CQ                            }, // LCES_R1_LCES_LRZRX             0810~0814
    {CAM_ISP_LCS_FLC0_,           0x10820, 0x0820, 10 , LCES_R1_LCES_FLC0_CQ                             }, // LCES_R1_LCES_FLC0              0820~0844
    {CAM_ISP_AE_,                 0x10ADC, 0x0ADC, 43 , AA_R1_AA_AE_GAIN2_0_CQ                           }, // AA_R1_AA_AE_GAIN2_0            0ADC~0B84   ISP5.0 : can't access 4974, this register is for CCU only (AE on CCU)  0ADC~0B98
    {CAM_ISP_AE2_,                0x10B8C, 0x0B8C,  4 , AA_R1_AA_AE_SPARE_CQ                             }, // AA_R1_AA_AE_SPARE              0B8C~0B98
    {CAM_ISP_AWB_,                0x10A00, 0x0A00, 55 , AA_R1_AA_AWB_WIN_ORG_CQ                          }, // AA_R1_AA_AWB_WIN_ORG           0A00~0AD8
    {CAM_ISP_AF_,                 0x11380, 0x1380, 102, AF_R1_AF_CON_CQ                                  }, // AF_R1_AF_CON                   1380~1514
    {CAM_ISP_FLK_A_,              0x10C40, 0x0C40, 13 , FLK_R1_FLK_CON_CQ                                }, // FLK_R1_FLK_CON                 0C40~0C70
    {CAM_ISP_LMV_,                0x11300, 0x1300, 9  , LMV_R1_LMV_LMV_PREP_ME_CTRL1_CQ                  }, // LMV_R1_LMV_LMV_PREP_ME_CTRL1   1300~1320   EIS rename to LMV
    {CAM_ISP_HDS_,                0x11280, 0x1280, 1  , HDS_R1_HDS_HDS_MODE_CQ                           }, // HDS_R1_HDS_HDS_MODE            1280
    {CAM_ISP_RSS_A_,              0x112C0, 0x12C0, 9  , RSS_R1_RSS_CONTROL_CQ                            }, // RSS_R1_RSS_CONTROL             12C0~12E0
    {CAM_ISP_SEP_,                0x10380, 0x0380, 3  , SEP_R1_SEP_CTL_CQ                                }, // SEP_R1_SEP_CTL                 0380~0388
    {CAM_ISP_BPC_,                0x103C0, 0x03C0, 58 , BPC_R1_BPC_BPC_CON_CQ                            }, // BPC_R1_BPC_BPC_CON             03C0~04A4
    {CAM_ISP_DGN_,                0x10580, 0x0580, 5  , DGN_R1_DGN_GN0_CQ                                }, // DGN_R1_DGN_GN0                 0580~0590
    {CAM_ISP_WB_,                 0x10600, 0x0600, 3  , WB_R1_WB_GAIN_1_CQ                               }, // WB_R1_WB_GAIN_1                0600~0608
    {CAM_ISP_MRG_R7_,             0x10740, 0x0740, 3  , MRG_R7_MRG_CTL_CQ                                }, // MRG_R7_MRG_CTL                 0740~0748
    {CAM_ISP_MRG_R5_,             0x107C0, 0x07C0, 3  , MRG_R5_MRG_CTL_CQ                                }, // MRG_R5_MRG_CTL                 07C0~07C8
    {CAM_ISP_QBIN_R4_,            0x10880, 0x0880, 1  , QBIN_R4_QBIN_CTL_CQ                              }, // QBIN_R4_QBIN_CTL               0880        QBIN_R4 == QBN_R4
    {CAM_ISP_LTMS_,               0x10910, 0x0910, 30 , LTMS_R1_LTMS_CFG_CQ                              }, // LTMS_R1_LTMS_CFG               0910~0984
    {CAM_ISP_LTM_SECTION1,        0x10640, 0x0640, 5  , LTM_R1_LTM_CTRL_CQ                               }, // LTM_R1_LTM_CTRL                0640~0650
    {CAM_ISP_LTM_MAX_DIV,         0x10654, 0x0654, 1  , LTM_R1_LTM_MAX_DIV_CQ                            }, // LTM_R1_LTM_MAX_DIV             0654
    {CAM_ISP_LTM_SECTION2,        0x1065C, 0x065C, 48 , LTM_R1_LTM_CLIP_CQ                               }, // LTM_R1_LTM_CLIP                065C~0718
    {CAM_ISP_LTMTC_CURVE_1,       0x16000, 0x6000, 1008,LTMTC_R1_LTMTC_CURVE_CQ                          }, // LTMTC_R1_LTMTC_CURVE           6000~6FBC
    {CAM_ISP_LTMTC_CURVE_2,       0x16FC0, 0x6FC0, 774, LTMTC_R1_LTMTC_CURVE_CQ                          }, // LTMTC_R1_LTMTC_CURVE           6FC0~7BD4
    {CAM_ISP_LTM_PINGPONG_,       0x1071C, 0x071C, 1  , LTM_R1_LTM_SRAM_PINGPONG_CQ                      }, // LTM_R1_LTM_SRAM_PINGPONG       071C
    {CAM_ISP_HLR_,                0x11080, 0x1080, 17 , HLR_R1_HLR_EST_Y0_CQ                             }, // HLR_R1_HLR_EST_Y0              1080~10C0
    {CAM_ISP_MRG_,                0x109C0, 0x09C0, 3  , MRG_R1_MRG_CTL_CQ                                }, // MRG_R1_MRG_CTL                 09C0~09C8
    {CAM_ISP_AA_,                 0x10A00, 0x0A00, 103, AA_R1_AA_AWB_WIN_ORG_CQ                          }, // AA_R1_AA_AWB_WIN_ORG           0A00~0B98
    {CAM_ISP_CRP_R3_,             0x10C80, 0x0C80, 2  , CRP_R3_CRP_X_POS_CQ                              }, // CRP_R3_CRP_X_POS               0C80~0C84
    {CAM_ISP_PAK_,                0x10CC0, 0x0CC0, 1  , PAK_R1_PAK_CONT_CQ                               }, // PAK_R1_PAK_CONT                0CC0
    {CAM_ISP_UNP_R2_,             0x10D00, 0x0D00, 1  , UNP_R2_UNP_OFST_CQ                               }, // UNP_R2_UNP_OFST                0D00
    {CAM_ISP_FBND_,               0x10DC0, 0x0DC0, 1  , FBND_R1_FBND_CFG_CQ                              }, // FBND_R1_FBND_CFG               0DC0
    {CAM_ISP_BPC_R2_,             0x10E80, 0x0E80, 58 , BPC_R2_BPC_BPC_CON_CQ                            }, // BPC_R2_BPC_BPC_CON             0E80~0F64
    {CAM_ISP_OBC_R2_,             0x10F80, 0x0F80, 28 , OBC_R2_OBC_CTL_CQ                                }, // OBC_R2_OBC_CTL                 0F80~0FEC
    {CAM_ISP_UFG_,                0x11180, 0x1180, 1  , UFG_R1_UFG_CON_CQ                                }, // UFG_R1_UFG_CON                 1180
    {CAM_ISP_QBIN_R5_,            0x111C0, 0x11C0, 1  , QBIN_R5_QBIN_CTL_CQ                              }, // QBIN_R5_QBIN_CTL               11C0        QBIN_R5 == QBN_R5
    {CAM_ISP_GSE_,                0x11200, 0x1200, 3  , GSE_R1_GSE_GSE_00_CQ                             }, // GSE_R1_GSE_GSE_00              1200~1208
    {CAM_ISP_MRG_R8_,             0x11240, 0x1240, 3  , MRG_R8_MRG_CTL_CQ                                }, // MRG_R8_MRG_CTL                 1240~1248
    {CAM_ISP_CRP_R1_,             0x11340, 0x1340, 2  , CRP_R1_CRP_X_POS_CQ                              }, // CRP_R1_CRP_X_POS               1340~1344
    {CAM_ISP_MRG_R2_,             0x115C0, 0x15C0, 3  , MRG_R2_MRG_CTL_CQ                                }, // MRG_R2_MRG_CTL                 15C0~15C8
    {CAM_ISP_QBIN_R2_,            0x11600, 0x1600, 1  , QBIN_R2_QBIN_CTL_CQ                              }, // QBIN_R2_QBIN_CTL               1600        QBIN_R2 == QBN_R2
    {CAM_ISP_TSFS_,               0x11644, 0x1644, 7  , TSFS_R1_TSFS_ORG_CQ                              }, // TSFS_R1_TSFS_ORG               1644~165C
    {CAM_ISP_CRP_R8_,             0x11740, 0x1740, 2  , CRP_R8_CRP_X_POS_CQ                              }, // CRP_R8_CRP_X_POS               1740~1744
    {CAM_ISP_SLK_CEN,             0x117C0, 0x17C0, 5  , SLK_R1_SLK_CEN_CQ                                }, // SLK_R1_SLK_CEN                 17C0~17D0
    {CAM_ISP_SLK_XOFF,            0x117D4, 0x17D4, 2  , SLK_R1_SLK_XOFF_CQ                               }, // SLK_R1_SLK_XOFF                17D4~17D8
    {CAM_ISP_SLK_CON,             0x117DC, 0x17DC, 4  , SLK_R1_SLK_SLP_CON0_CQ                           }, // SLK_R1_SLK_SLP_CON0            17DC~17E8
    {CAM_ISP_SLK_SIZE,            0x117EC, 0x17EC, 1  , SLK_R1_SLK_SIZE_CQ                               }, // SLK_R1_SLK_SIZE                17EC
    {CAM_ISP_DM_,                 0x11800, 0x1800, 26 , DM_R1_DM_INTP_CRS_CQ                             }, // DM_R1_DM_INTP_CRS              1800~1864
    {CAM_ISP_CCM_,                0x11880, 0x1880, 9  , CCM_R1_CCM_CNV_1_CQ                              }, // CCM_R1_CCM_CNV_1               1880~18A0
    {CAM_ISP_GGM_,                0x118C0, 0x18C0, 193, GGM_R1_GGM_LUT_CQ                                }, // GGM_R1_GGM_LUT                 18C0~1BC0
    {CAM_ISP_GGM_PINGPONG_,       0x11BC4, 0x1BC4, 1  , GGM_R1_GGM_SRAM_PINGPONG_CQ                      }, // GGM_R1_GGM_SRAM_PINGPONG       1BC4
    {CAM_ISP_G2C_,                0x11C00, 0x1C00, 6  , G2C_R1_G2C_CONV_0A_CQ                            }, // G2C_R1_G2C_CONV_0A             1C00~1C14
    {CAM_ISP_C42_,                0x11C40, 0x1C40, 1  , C42_R1_C42_CON_CQ                                }, // C42_R1_C42_CON                 1C40
    {CAM_ISP_CRSP_,               0x11C80, 0x1C80, 5  , CRSP_R1_CRSP_CTRL_CQ                             }, // CRSP_R1_CRSP_CTRL              1C80~1C90
    {CAM_ISP_PAK_R3_,             0x11D00, 0x1D00, 1  , PAK_R3_PAK_CONT_CQ                               }, // PAK_R3_PAK_CONT                1D00
    {CAM_ISP_PAK_R4_,             0x11D40, 0x1D40, 1  , PAK_R4_PAK_CONT_CQ                               }, // PAK_R4_PAK_CONT                1D40
    {CAM_ISP_MRG_R3_,             0x12740, 0x2740, 3  , MRG_R3_MRG_CTL_CQ                                }, // MRG_R3_MRG_CTL                 2740~2748
    {CAM_ISP_MRG_R4_,             0x127C0, 0x27C0, 3  , MRG_R4_MRG_CTL_CQ                                }, // MRG_R4_MRG_CTL                 27C0~27C8
    {CAM_ISP_MRG_R6_,             0x12800, 0x2800, 3  , MRG_R6_MRG_CTL_CQ                                }, // MRG_R6_MRG_CTL                 2800~2808
    {CAM_ISP_QBIN_R3_,            0x12840, 0x2840, 1  , QBIN_R3_QBIN_CTL_CQ                              }, // QBIN_R3_QBIN_CTL               2840         QBIN_R3 == QBN_R3
    {CAM_ISP_MRG_R10_,            0x12880, 0x2880, 3  , MRG_R10_MRG_CTL_CQ                               }, // MRG_R10_MRG_CTL                2880~2888
    {CAM_ISP_CRZ_,                0x128C0, 0x28C0, 14 , CRZ_R1_CRZ_CONTROL_CQ                            }, // CRZ_R1_CRZ_CONTROL             28C0~28F4
    {CAM_ISP_SLK_R2_CEN,          0x12980, 0x2980, 5  , SLK_R2_SLK_CEN_CQ                                }, // SLK_R2_SLK_CEN                 2980~2990
    {CAM_ISP_SLK_R2_XOFF,         0x12994, 0x2994, 2  , SLK_R2_SLK_XOFF_CQ                               }, // SLK_R2_SLK_XOFF                2994~2998
    {CAM_ISP_SLK_R2_CON,          0x1299C, 0x299C, 4  , SLK_R2_SLK_SLP_CON0_CQ                           }, // SLK_R2_SLK_SLP_CON0            299C~29A8
    {CAM_ISP_SLK_R2_SIZE,         0x129AC, 0x29AC, 1  , SLK_R2_SLK_SIZE_CQ                               }, // SLK_R2_SLK_SIZE                29AC
    {CAM_ISP_MRG_R11_,            0x12A80, 0x2A80, 3  , MRG_R11_MRG_CTL_CQ                               }, // MRG_R11_MRG_CTL                2A80~2A88
    {CAM_ISP_MRG_R12_,            0x12AC0, 0x2AC0, 3  , MRG_R12_MRG_CTL_CQ                               }, // MRG_R12_MRG_CTL                2AC0~2AC8
    {CAM_ISP_UFD_R2_,             0x12B40, 0x2B40, 14 , UFD_R2_UFD_CON_CQ                                }, // UFD_R2_UFD_CON                 2B40~2B74
    {CAM_ISP_CRP_R4_,             0x12B80, 0x2B80, 2  , CRP_R4_CRP_X_POS_CQ                              }, // CRP_R4_CRP_X_POS               2B80~2B84
    {CAM_ISP_MRG_R13_,            0x12BC0, 0x2BC0, 3  , MRG_R13_MRG_CTL_CQ                               }, // MRG_R13_MRG_CTL                2BC0~2BC8
    {CAM_ISP_MRG_R14_,            0x12C00, 0x2C00, 3  , MRG_R14_MRG_CTL_CQ                               }, // MRG_R14_MRG_CTL                2C00~2C08
    {CAM_ISP_BS_,                 0x12C40, 0x2C40, 1  , BS_R1_BS_SPARE0_CQ                               }, // BS_R1_BS_SPARE0                2C40
    {CAM_ISP_BS_R2_,              0x12C80, 0x2C80, 1  , BS_R2_BS_SPARE0_CQ                               }, // BS_R2_BS_SPARE0                2C80
    {CAM_ISP_BS_R3_,              0x12CC0, 0x2CC0, 1  , BS_R3_BS_SPARE0_CQ                               }, // BS_R3_BS_SPARE0                2CC0
    {CAM_ISP_PAK_R5_,             0x12D00, 0x2D00, 1  , PAK_R5_PAK_CONT_CQ                               }, // PAK_R5_PAK_CONT                2D00
    {CAM_ISP_G2C_R2_,             0x12D40, 0x2D40, 6  , G2C_R2_G2C_CONV_0A_CQ                            }, // G2C_R2_G2C_CONV_0A             2D40~2D54
    {CAM_ISP_C42_R2_,             0x12D80, 0x2D80, 1  , C42_R2_C42_CON_CQ                                }, // C42_R2_C42_CON                 2D80
    {CAM_ISP_FLC_,                0x12DC0, 0x2DC0, 4  , FLC_R1_FLC_OFST_RB_CQ                            }, // FLC_R1_FLC_OFST_RB             2DC0~2DCC
    {CAM_ISP_CRSP_R2_,            0x12F00, 0x2F00, 5  , CRSP_R2_CRSP_CTRL_CQ                             }, // CRSP_R2_CRSP_CTRL              2F00~2F10
    {CAM_ISP_YNRS_,               0x13000, 0x3000, 295, YNRS_R1_YNRS_TBL_CQ                              }, // YNRS_R1_YNRS_TBL               3000~3498
    {CAM_ISP_YNRS_PINGPONG_,      0x1349C, 0x349C, 1  , YNRS_R1_YNRS_SRAM_PINGPONG_CQ                    }, // YNRS_R1_YNRS_SRAM_PINGPONG     349C
    {CAM_ISP_GGM_R2_,             0x13600, 0x3600, 193, GGM_R2_GGM_LUT_CQ                                }, // GGM_R2_GGM_LUT                 3600~3900
    {CAM_ISP_GGM_R2_PINGPONG_,    0x13904, 0x3904, 1  , GGM_R2_GGM_SRAM_PINGPONG_CQ                      }, // GGM_R2_GGM_SRAM_PINGPONG       3904
    {CAM_ISP_RSS_R2_,             0x13940, 0x3940, 9  , RSS_R2_RSS_CONTROL_CQ                            }, // RSS_R2_RSS_CONTROL             3940~3960
    {CAM_ISP_CRZ_R2_,             0x13980, 0x3980, 14 , CRZ_R2_CRZ_CONTROL_CQ                            }, // CRZ_R2_CRZ_CONTROL             3980~39B4
    {CAM_ISP_MOBC_R2_,            0x13A00, 0x3A00, 8  , MOBC_R2_MOBC_OFFST0_CQ                           }, // MOBC_R2_MOBC_OFFST0            3A00~3A1C
    {CAM_ISP_MOBC_R3_,            0x13A40, 0x3A40, 8  , MOBC_R3_MOBC_OFFST0_CQ                           }, // MOBC_R3_MOBC_OFFST0            3A40~3A5C
    {SV_DMA_IMGO_BA_A_0,          0x51A08, 0x50B0, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50B0
    {SV_DMA_IMGO_BA_A_1,          0x51A08, 0x50B4, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50B4
    {SV_DMA_IMGO_BA_A_2,          0x51A08, 0x50B8, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50B8
    {SV_DMA_IMGO_BA_A_3,          0x51A08, 0x50BC, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50BC
    {SV_DMA_IMGO_BA_A_4,          0x51A08, 0x50C0, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50C0
    {SV_DMA_IMGO_BA_A_5,          0x51A08, 0x50C4, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50C4
    {SV_DMA_IMGO_BA_A_6,          0x51A08, 0x50C8, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50C8
    {SV_DMA_IMGO_BA_A_7,          0x51A08, 0x50CC, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        50CC
    {SV_DMA_FH_IMGO_A_0,          0x51A10, 0x50D0, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50D0
    {SV_DMA_FH_IMGO_A_1,          0x51A10, 0x50D4, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50D4
    {SV_DMA_FH_IMGO_A_2,          0x51A10, 0x50D8, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50D8
    {SV_DMA_FH_IMGO_A_3,          0x51A10, 0x50DC, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50DC
    {SV_DMA_FH_IMGO_A_4,          0x51A10, 0x50E0, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50E0
    {SV_DMA_FH_IMGO_A_5,          0x51A10, 0x50E4, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50E4
    {SV_DMA_FH_IMGO_A_6,          0x51A10, 0x50E8, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50E8
    {SV_DMA_FH_IMGO_A_7,          0x51A10, 0x50EC, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       50EC
    {SV_DMA_FH_IMGO_SPARE_A_0,    0x51A18, 0x50F0, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    50F0
    {SV_DMA_FH_IMGO_SPARE_A_1,    0x51A18, 0x5120, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5120
    {SV_DMA_FH_IMGO_SPARE_A_2,    0x51A18, 0x5150, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5150
    {SV_DMA_FH_IMGO_SPARE_A_3,    0x51A18, 0x5180, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5180
    {SV_DMA_FH_IMGO_SPARE_A_4,    0x51A18, 0x51B0, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    51B0
    {SV_DMA_FH_IMGO_SPARE_A_5,    0x51A18, 0x51E0, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    51E0
    {SV_DMA_FH_IMGO_SPARE_A_6,    0x51A18, 0x5210, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5210
    {SV_DMA_FH_IMGO_SPARE_A_7,    0x51A18, 0x5240, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5240
    {SV_DMA_UFEO_BA_A_0,          0x51B08, 0x5270, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        5270
    {SV_DMA_UFEO_BA_A_1,          0x51B08, 0x5274, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        5274
    {SV_DMA_UFEO_BA_A_2,          0x51B08, 0x5278, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        5278
    {SV_DMA_UFEO_BA_A_3,          0x51B08, 0x527C, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        527C
    {SV_DMA_UFEO_BA_A_4,          0x51B08, 0x5280, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        5280
    {SV_DMA_UFEO_BA_A_5,          0x51B08, 0x5284, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        5284
    {SV_DMA_UFEO_BA_A_6,          0x51B08, 0x5288, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        5288
    {SV_DMA_UFEO_BA_A_7,          0x51B08, 0x528C, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        528C
    {SV_DMA_FH_UFEO_A_0,          0x51B10, 0x5290, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       5290
    {SV_DMA_FH_UFEO_A_1,          0x51B10, 0x5294, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       5294
    {SV_DMA_FH_UFEO_A_2,          0x51B10, 0x5298, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       5298
    {SV_DMA_FH_UFEO_A_3,          0x51B10, 0x529C, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       529C
    {SV_DMA_FH_UFEO_A_4,          0x51B10, 0x52A0, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       52A0
    {SV_DMA_FH_UFEO_A_5,          0x51B10, 0x52A4, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       52A4
    {SV_DMA_FH_UFEO_A_6,          0x51B10, 0x52A8, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       52A8
    {SV_DMA_FH_UFEO_A_7,          0x51B10, 0x52AC, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       52AC
    {SV_DMA_FH_UFEO_SPARE_A_0,    0x51B18, 0x52B0, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    52B0
    {SV_DMA_FH_UFEO_SPARE_A_1,    0x51B18, 0x52F8, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    52F8
    {SV_DMA_FH_UFEO_SPARE_A_2,    0x51B18, 0x5340, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5340
    {SV_DMA_FH_UFEO_SPARE_A_3,    0x51B18, 0x5388, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5388
    {SV_DMA_FH_UFEO_SPARE_A_4,    0x51B18, 0x53D0, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    53D0
    {SV_DMA_FH_UFEO_SPARE_A_5,    0x51B18, 0x5418, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5418
    {SV_DMA_FH_UFEO_SPARE_A_6,    0x51B18, 0x5460, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5460
    {SV_DMA_FH_UFEO_SPARE_A_7,    0x51B18, 0x54A8, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    54A8
    {SV_DMA_IMGO_BA_B_0,          0x52A08, 0x54F0, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        54F0
    {SV_DMA_IMGO_BA_B_1,          0x52A08, 0x54F4, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        54F4
    {SV_DMA_IMGO_BA_B_2,          0x52A08, 0x54F8, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        54F8
    {SV_DMA_IMGO_BA_B_3,          0x52A08, 0x54FC, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        54FC
    {SV_DMA_IMGO_BA_B_4,          0x52A08, 0x5500, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        5500
    {SV_DMA_IMGO_BA_B_5,          0x52A08, 0x5504, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        5504
    {SV_DMA_IMGO_BA_B_6,          0x52A08, 0x5508, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        5508
    {SV_DMA_IMGO_BA_B_7,          0x52A08, 0x550C, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_ADDR        550C
    {SV_DMA_FH_IMGO_B_0,          0x52A10, 0x5510, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       5510
    {SV_DMA_FH_IMGO_B_1,          0x52A10, 0x5514, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       5514
    {SV_DMA_FH_IMGO_B_2,          0x52A10, 0x5518, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       5518
    {SV_DMA_FH_IMGO_B_3,          0x52A10, 0x551C, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       551C
    {SV_DMA_FH_IMGO_B_4,          0x52A10, 0x5520, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       5520
    {SV_DMA_FH_IMGO_B_5,          0x52A10, 0x5524, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       5524
    {SV_DMA_FH_IMGO_B_6,          0x52A10, 0x5528, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       5528
    {SV_DMA_FH_IMGO_B_7,          0x52A10, 0x552C, 1  , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HADDR       552C
    {SV_DMA_FH_IMGO_SPARE_B_0,    0x52A18, 0x5530, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5530
    {SV_DMA_FH_IMGO_SPARE_B_1,    0x52A18, 0x5560, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5560
    {SV_DMA_FH_IMGO_SPARE_B_2,    0x52A18, 0x5590, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5590
    {SV_DMA_FH_IMGO_SPARE_B_3,    0x52A18, 0x55C0, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    55C0
    {SV_DMA_FH_IMGO_SPARE_B_4,    0x52A18, 0x55F0, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    55F0
    {SV_DMA_FH_IMGO_SPARE_B_5,    0x52A18, 0x5620, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5620
    {SV_DMA_FH_IMGO_SPARE_B_6,    0x52A18, 0x5650, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5650
    {SV_DMA_FH_IMGO_SPARE_B_7,    0x52A18, 0x5680, 12 , 1                                                }, // CAMSV_FBC_IMGO_ENQ_HEADER_0    5680
    {SV_DMA_UFEO_BA_B_0,          0x52B08, 0x56B0, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56B0
    {SV_DMA_UFEO_BA_B_1,          0x52B08, 0x56B4, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56B4
    {SV_DMA_UFEO_BA_B_2,          0x52B08, 0x56B8, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56B8
    {SV_DMA_UFEO_BA_B_3,          0x52B08, 0x56BC, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56BC
    {SV_DMA_UFEO_BA_B_4,          0x52B08, 0x56C0, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56C0
    {SV_DMA_UFEO_BA_B_5,          0x52B08, 0x56C4, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56C4
    {SV_DMA_UFEO_BA_B_6,          0x52B08, 0x56C8, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56C8
    {SV_DMA_UFEO_BA_B_7,          0x52B08, 0x56CC, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_ADDR        56CC
    {SV_DMA_FH_UFEO_B_0,          0x52B10, 0x56D0, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56D0
    {SV_DMA_FH_UFEO_B_1,          0x52B10, 0x56D4, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56D4
    {SV_DMA_FH_UFEO_B_2,          0x52B10, 0x56D8, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56D8
    {SV_DMA_FH_UFEO_B_3,          0x52B10, 0x56DC, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56DC
    {SV_DMA_FH_UFEO_B_4,          0x52B10, 0x56E0, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56E0
    {SV_DMA_FH_UFEO_B_5,          0x52B10, 0x56E4, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56E4
    {SV_DMA_FH_UFEO_B_6,          0x52B10, 0x56E8, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56E8
    {SV_DMA_FH_UFEO_B_7,          0x52B10, 0x56EC, 1  , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HADDR       56EC
    {SV_DMA_FH_UFEO_SPARE_B_0,    0x52B18, 0x56F0, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    56F0
    {SV_DMA_FH_UFEO_SPARE_B_1,    0x52B18, 0x5738, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5738
    {SV_DMA_FH_UFEO_SPARE_B_2,    0x52B18, 0x5780, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5780
    {SV_DMA_FH_UFEO_SPARE_B_3,    0x52B18, 0x57C8, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    57C8
    {SV_DMA_FH_UFEO_SPARE_B_4,    0x52B18, 0x5810, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5810
    {SV_DMA_FH_UFEO_SPARE_B_5,    0x52B18, 0x5858, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    5858
    {SV_DMA_FH_UFEO_SPARE_B_6,    0x52B18, 0x58A0, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    58A0
    {SV_DMA_FH_UFEO_SPARE_B_7,    0x52B18, 0x58E8, 18 , 1                                                }, // CAMSV_FBC_UFEO_ENQ_HEADER_0    58E8
    {SV_FBC_RCNT_INC_0,           0x5102C, 0x5930, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5930
    {SV_FBC_RCNT_INC_1,           0x5102C, 0x5934, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5934
    {SV_FBC_RCNT_INC_2,           0x5102C, 0x5938, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5938
    {SV_FBC_RCNT_INC_3,           0x5102C, 0x593C, 1  , 1                                                }, // CAMSV_IMGO_FBC                 593C
    {SV_FBC_RCNT_INC_4,           0x5102C, 0x5940, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5940
    {SV_FBC_RCNT_INC_5,           0x5102C, 0x5944, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5944
    {SV_FBC_RCNT_INC_6,           0x5102C, 0x5948, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5948
    {SV_FBC_RCNT_INC_7,           0x5102C, 0x594C, 1  , 1                                                }, // CAMSV_IMGO_FBC                 594C
    {SV_DMA_FH_IMGO_BASE_A_0,     0x51804, 0x5950, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5950
    {SV_DMA_FH_IMGO_BASE_A_1,     0x51804, 0x5954, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5954
    {SV_DMA_FH_IMGO_BASE_A_2,     0x51804, 0x5958, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5958
    {SV_DMA_FH_IMGO_BASE_A_3,     0x51804, 0x595C, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        595C
    {SV_DMA_FH_IMGO_BASE_A_4,     0x51804, 0x5960, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5960
    {SV_DMA_FH_IMGO_BASE_A_5,     0x51804, 0x5964, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5964
    {SV_DMA_FH_IMGO_BASE_A_6,     0x51804, 0x5968, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5968
    {SV_DMA_FH_IMGO_BASE_A_7,     0x51804, 0x596C, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        596C
    {SV_DMA_FH_IMGO_BASE_B_0,     0x52804, 0x5970, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5970
    {SV_DMA_FH_IMGO_BASE_B_1,     0x52804, 0x5974, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5974
    {SV_DMA_FH_IMGO_BASE_B_2,     0x52804, 0x5978, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5978
    {SV_DMA_FH_IMGO_BASE_B_3,     0x52804, 0x597C, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        597C
    {SV_DMA_FH_IMGO_BASE_B_4,     0x52804, 0x5980, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5980
    {SV_DMA_FH_IMGO_BASE_B_5,     0x52804, 0x5984, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5984
    {SV_DMA_FH_IMGO_BASE_B_6,     0x52804, 0x5988, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        5988
    {SV_DMA_FH_IMGO_BASE_B_7,     0x52804, 0x598C, 1  , 1                                                }, // CAMSV_IMGO_FH_BASE_ADDR        598C

    {SV_FBC_RCNT_INC_B_0,         0x5202C, 0x5990, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5990
    {SV_FBC_RCNT_INC_B_1,         0x5202C, 0x5994, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5994
    {SV_FBC_RCNT_INC_B_2,         0x5202C, 0x5998, 1  , 1                                                }, // CAMSV_IMGO_FBC                 5998
    {SV_FBC_RCNT_INC_B_3,         0x5202C, 0x599C, 1  , 1                                                }, // CAMSV_IMGO_FBC                 599C
    {SV_FBC_RCNT_INC_B_4,         0x5202C, 0x59A0, 1  , 1                                                }, // CAMSV_IMGO_FBC                 59A0
    {SV_FBC_RCNT_INC_B_5,         0x5202C, 0x59A4, 1  , 1                                                }, // CAMSV_IMGO_FBC                 59A4
    {SV_FBC_RCNT_INC_B_6,         0x5202C, 0x59A8, 1  , 1                                                }, // CAMSV_IMGO_FBC                 59A8
    {SV_FBC_RCNT_INC_B_7,         0x5202C, 0x59AC, 1  , 1                                                }, // CAMSV_IMGO_FBC                 59AC

    {SV_MODULE_EN_A_0,            0x517B4, 0x59B0, 1  , 1                                                }, // CAMSV1_CAMSV_0_DMA_RSV2        59B0
    {SV_MODULE_EN_A_1,            0x517B8, 0x59B4, 1  , 1                                                }, // CAMSV1_CAMSV_0_DMA_RSV3        59B4
    {SV_MODULE_EN_B_0,            0x517BC, 0x59B8, 1  , 1                                                }, // CAMSV1_CAMSV_0_DMA_RSV4        59B8
    {SV_MODULE_EN_B_1,            0x517C0, 0x59BC, 1  , 1                                                }, // CAMSV1_CAMSV_0_DMA_RSV5        59BC

    {SV_TG_SUB_PERIOD_A_0,        0x527B4, 0x59C0, 1  , 1                                                }, // CAMSV2_CAMSV_0_DMA_RSV2        59C0
    {SV_TG_SUB_PERIOD_A_1,        0x527B8, 0x59C4, 1  , 1                                                }, // CAMSV2_CAMSV_0_DMA_RSV3        59C4
    {SV_TG_SUB_PERIOD_B_0,        0x527BC, 0x59C8, 1  , 1                                                }, // CAMSV2_CAMSV_0_DMA_RSV4        59C8
    {SV_TG_SUB_PERIOD_B_1,        0x527C0, 0x59CC, 1  , 1                                                }, // CAMSV2_CAMSV_0_DMA_RSV5        59CC

    {SV_SPARE_0_A_0,              0x537B4, 0x59D0, 1  , 1                                                }, // CAMSV3_CAMSV_0_DMA_RSV2        59D0
    {SV_SPARE_0_A_1,              0x537B8, 0x59D4, 1  , 1                                                }, // CAMSV3_CAMSV_0_DMA_RSV3        59D4
    {SV_SPARE_0_B_0,              0x537BC, 0x59D8, 1  , 1                                                }, // CAMSV3_CAMSV_0_DMA_RSV4        59D8
    {SV_SPARE_0_B_1,              0x537C0, 0x59DC, 1  , 1                                                }, // CAMSV3_CAMSV_0_DMA_RSV5        59DC

    {SV_SPARE_1_A_0,              0x547B4, 0x59E0, 1  , 1                                                }, // CAMSV4_CAMSV_0_DMA_RSV2        59E0
    {SV_SPARE_1_A_1,              0x547B8, 0x59E4, 1  , 1                                                }, // CAMSV4_CAMSV_0_DMA_RSV3        59E4
    {SV_SPARE_1_B_0,              0x547BC, 0x59E8, 1  , 1                                                }, // CAMSV4_CAMSV_0_DMA_RSV4        59E8
    {SV_SPARE_1_B_1,              0x547C0, 0x59EC, 1  , 1                                                }, // CAMSV4_CAMSV_0_DMA_RSV5        59EC

    {SV_120FPS_CAMA_CHANGE,       0x557B4, 0x59F0, 1  , 1                                                }, // CAMSV5_CAMSV_0_DMA_RSV2        59F0
    {SV_120FPS_CAMB_CHANGE,       0x557B8, 0x59F4, 1  , 1                                                }, // CAMSV5_CAMSV_0_DMA_RSV3        59F4
    {CAM_DMA_EN_,                 0x10014, 0x59F8, 1  , 1                                                }, // CAM_DMA_EN                     59F8

    {CAM_DMA_CAMSV_STAGER_,       0x140C0, 0x40C0, 2  , CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2_CQ }, // CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2             40C0~40C4
    {CAM_DUMMY_,                  0x150B0, 0x50B0, 1  , 1                                                }, // rsv_50B0 No Use Register       50B0
    {CAM_CTL_FBC_RCNT_INC_,       0x1004C, 0x004C, 1  , CAMCTL_R1_CAMCTL_FBC_RCNT_INC_CQ                 }, // CAMCTL_R1_CAMCTL_FBC_RCNT_INC  004C        //must be allocated after all FBC_en descirptor
    {CAM_CQ_EN_,                  0x10200, 0x0200, 1  , CAMCQ_R1_CAMCQ_CQ_EN_CQ                          }, // CAMCQ_R1_CAMCQ_CQ_EN           0200        //control
    {CAM_NEXT_Thread_,            0x18208, 0x0208, 1  , 1                                                }, // CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR special descriptor for CQ-thre0 link-list, must at the end of descriptor  , or apb will be crashed
    {CAM_CAM_END_,                0x00000, 0x0000, 1  , 1                                                },
//  {CAM_CAM_MODULE_MAX,          0x00000, 0x0000, 1  ,},
};

ISP_DRV_CQ_MODULE_INFO_STRUCT mIspCQModuleInfo_SpecialCQ[ISP_DRV_CAM_BASIC_CQ_NUM]    =
{
    {CAM_CQ_THRE0_ADDR_CQONLY , 0x5a208, 0x7C00, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE1_ADDR_CQONLY , 0x5a214, 0x7C04, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE2_ADDR_CQONLY , 0x5a220, 0x7C08, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE3_ADDR_CQONLY , 0x5a22C, 0x7C0C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE4_ADDR_CQONLY , 0x5a238, 0x7C10, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE5_ADDR_CQONLY , 0x5a244, 0x7C14, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE6_ADDR_CQONLY , 0x5a250, 0x7C18, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE7_ADDR_CQONLY , 0x5a25C, 0x7C1C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE8_ADDR_CQONLY , 0x5a268, 0x7C20, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE9_ADDR_CQONLY , 0x5a274, 0x7C24, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE10_ADDR_CQONLY, 0x5a280, 0x7C28, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE11_ADDR_CQONLY, 0x5a28C, 0x7C2C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE12_ADDR_CQONLY, 0x5a298, 0x7C30, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE13_ADDR_CQONLY, 0x5a2A4, 0x7C34, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE14_ADDR_CQONLY, 0x5a2B0, 0x7C38, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE15_ADDR_CQONLY, 0x5a2BC, 0x7C3C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE16_ADDR_CQONLY, 0x5a2C8, 0x7C40, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE17_ADDR_CQONLY, 0x5a2D4, 0x7C44, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE18_ADDR_CQONLY, 0x5a2E0, 0x7C48, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE19_ADDR_CQONLY, 0x5a2EC, 0x7C4C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE20_ADDR_CQONLY, 0x5a2F8, 0x7C50, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE21_ADDR_CQONLY, 0x5a304, 0x7C54, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE22_ADDR_CQONLY, 0x5a310, 0x7C58, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE23_ADDR_CQONLY, 0x5a31C, 0x7C5C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE24_ADDR_CQONLY, 0x5a328, 0x7C60, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE25_ADDR_CQONLY, 0x5a334, 0x7C64, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE26_ADDR_CQONLY, 0x5a340, 0x7C68, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE27_ADDR_CQONLY, 0x5a34C, 0x7C6C, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE28_ADDR_CQONLY, 0x5a358, 0x7C70, 1, 1 }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
};


ISP_DRV_CAM**** gPageTbl = NULL;
android::Mutex  gMutex;

//For secure camera
vector<MUINT32>            ISP_DRV_CAM::m_DapcIdx;
vector<MUINT32>            ISP_DRV_CAM::m_DapcReg[PHY_CAM];
MBOOL                      ISP_DRV_CAM::m_SecOn = MFALSE;

ISP_DRV_CAM::ISP_DRV_CAM(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx)
{
    DBG_LOG_CONFIG(drv, isp_drv_CAM);
    //
    m_UserCnt = 0;
    m_FSM = IspDrv_UNKNONW;
    //imem
    m_pMemDrv = NULL;
    //
    m_pIspDescript_vir = NULL;
    m_pIspDescript_phy = NULL;
    //
    m_pIspVirRegAddr_va = NULL;
    m_pIspVirRegAddr_pa = NULL;

    m_HWmasterModule = CAM_MAX;
    m_Module = module;
    switch(module){
        case CAM_A_TWIN_B://this is a virtual hw
            m_HWModule = CAM_B;
            break;
        default:
            m_HWModule = module;
            break;
    }
    m_pIspDrvImp = IspDrvImp::createInstance(m_HWModule);

    m_CQ = cq;
    m_pageIdx = pageIdx;
    //
    m_currentDCIFsumsampleRecode = 0;

    BASE_LOG_DBG("getpid[0x%08x],gettid[0x%08x],[%d],[%d],[%d]\n", getpid() ,gettid(),m_Module,m_CQ,m_pageIdx);
}


IspDrvVir* ISP_DRV_CAM::createInstance(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx,const char* userName)
{
    if(userName == NULL){
        BASE_LOG_ERR("userName can't be NULL (cq:0x%x,pageidx:0x%x)",cq,pageIdx);
        return NULL;
    }

    std::string strUserName = userName;
    if ((strUserName.compare(0,10,"CmdQMgrImp") != 0) && (strUserName.compare(0,5,"dummy") != 0)) {
        BASE_LOG_INF("module:0x%x,cq:0x%x,pageIdx:0x%x,userName:%s",module,cq,pageIdx,userName);
    }

    switch(module){
        case CAM_A:
        case CAM_B:
        case CAM_C:
        case CAM_A_TWIN_B:
            break;
        default:
            BASE_LOG_ERR("unsupported module:0x%x\n",module);
            return NULL;
            break;
    }

    android::Mutex::Autolock lock(gMutex);
    if(gPageTbl == NULL){
        gPageTbl = (ISP_DRV_CAM****)malloc(sizeof(ISP_DRV_CAM***)*CAM_MAX);
        for(MUINT32 i=0;i<CAM_MAX;i++){
            gPageTbl[i] = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*ISP_DRV_CAM_BASIC_CQ_NUM);
            for(MUINT32 j=0;j<ISP_DRV_CAM_BASIC_CQ_NUM;j++){
                gPageTbl[i][j] = (ISP_DRV_CAM**)malloc(sizeof(ISP_DRV_CAM*)*Max_PageNum);
                for(int k=0;k<Max_PageNum;k++)
                    gPageTbl[i][j][k] = NULL;
            }
        }
    }

    if(gPageTbl[module][cq][pageIdx] == NULL){
        gPageTbl[module][cq][pageIdx] = new ISP_DRV_CAM(module,cq,pageIdx);
        strncpy(gPageTbl[module][cq][pageIdx]->m_useName,userName, sizeof(gPageTbl[module][cq][pageIdx]->m_useName)-1);
        gPageTbl[module][cq][pageIdx]->m_FSM = IspDrv_Create;
    }
    else {
        BASE_LOG_INF("this obj already created (cq:0x%x,idx:0x%x,userNmae:%s)",cq,pageIdx,userName);
    }

    return (IspDrvVir*)gPageTbl[module][cq][pageIdx];
}


void ISP_DRV_CAM::destroyInstance(void)
{
    if(this->m_UserCnt <=0){
        MUINT32 _cnt=0;

        if(this->FSM_CHK(OP_IspDrv_Destroy,__FUNCTION__) == MFALSE){
            LOG_ERR("destroy fail");
        }

        std::string strUserName = this->m_useName;
        if ((strUserName.compare(0,10,"CmdQMgrImp") != 0) && (strUserName.compare(0,5,"dummy") != 0)) {
            LOG_INF("+ module:0x%x,cq:0x%x,pageIdx:0x%x,userName:%s\n",this->m_Module,this->m_CQ,this->m_pageIdx,this->m_useName);
        }

        android::Mutex::Autolock lock(gMutex);

        gPageTbl[this->m_Module][this->m_CQ][this->m_pageIdx] = NULL;


        //check for free gPageTbl
        _cnt=0;
        for(MUINT32 i=0;i<CAM_MAX;i++){
            for(MUINT32 j=0;j<ISP_DRV_CAM_BASIC_CQ_NUM;j++){
                for(MUINT32 k=0;k<Max_PageNum;k++){
                    if(gPageTbl[i][j][k] == NULL)
                        _cnt++;
                    else
                        goto EXIT;
                }
            }
        }
        if(_cnt == (CAM_MAX * ISP_DRV_CAM_BASIC_CQ_NUM * Max_PageNum)){
            for(MUINT32 i=0;i<CAM_MAX;i++){
                for(MUINT32 j=0;j<ISP_DRV_CAM_BASIC_CQ_NUM;j++){
                    free(gPageTbl[i][j]);
                }
                free(gPageTbl[i]);
            }
        }
        free(gPageTbl);
        gPageTbl = NULL;


EXIT:
        //
        delete this;
    }
}


/**
    method for CQ0 . support dynamic descriptor operation, vir reg read/write .
    cq descriptor size is written via this api
*/
MBOOL ISP_DRV_CAM::CQ_Allocate_method1(MUINT32 step)
{
    MUINTPTR _tmpVirAddr,_tmpPhyAddr;
    switch(step){
        case 0:
            // allocate virCmdQ
            LOG_DBG("virQ rage:0x%zx",(CAM_BASE_RANGE_SPECIAL + UNI_BASE_RANGE));
            //va ptr chk, if not NULL, means imem was not freed also
            if(this->m_pIspVirRegAddr_va){
                LOG_ERR("va of VirReg is not null(0x%p)",this->m_pIspVirRegAddr_va);
                return MFALSE;
            }
            //pa ptr chk, if not NULL, means imem was not freed also
            if(this->m_pIspVirRegAddr_pa){
                LOG_ERR("pa of VirReg is not null(0x%p)",this->m_pIspVirRegAddr_pa);
                return MFALSE;
            }
            //imem allocate
            this->m_ispVirRegBufInfo.size = (((CAM_BASE_RANGE_SPECIAL + UNI_BASE_RANGE) + 0x3) & (~0x3)); //4byte alignment,round up
            this->m_ispVirRegBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);

            if ( this->m_pMemDrv->allocVirtBuf(&this->m_ispVirRegBufInfo) ){
                LOG_ERR("m_pMemDrv->allocVirtBuf fail");
                return MFALSE;
            }
            else{
                if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispVirRegBufInfo, MFALSE) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                    return MFALSE;
                }
            }
            //alignment chk
            if(( (_tmpVirAddr=this->m_ispVirRegBufInfo.virtAddr) % 0x4) != 0 ){
                LOG_INF("iMem virRegAddr not 4 bytes alignment(0x%p), round up",(void*)this->m_ispVirRegBufInfo.virtAddr);
                _tmpVirAddr = (this->m_ispVirRegBufInfo.virtAddr + 0x3) & (~0x3);
            }
            if(( (_tmpPhyAddr=this->m_ispVirRegBufInfo.phyAddr) % 0x4) != 0 ){
                LOG_INF("iMem phyRegAddr not 4 bytes alignment(0x%p), round up",(void*)this->m_ispVirRegBufInfo.phyAddr);
                _tmpPhyAddr = (this->m_ispVirRegBufInfo.phyAddr + 0x3) & (~0x3);
            }
            //va/pa 0 init
            memset((MUINT8*)_tmpVirAddr,0x0,(CAM_BASE_RANGE_SPECIAL + UNI_BASE_RANGE));
            this->m_pIspVirRegAddr_va = (MUINT32*)_tmpVirAddr;
            this->m_pIspVirRegAddr_pa = (MUINT32*)_tmpPhyAddr;
            LOG_INF("virtIspAddr:virt[%p]/phy[%p]",this->m_pIspVirRegAddr_va, this->m_pIspVirRegAddr_pa);

            //CQ descriptor
            //descriptor vir ptr chk,if not NULL, means imem was not freed also
            if(this->m_pIspDescript_vir){
                LOG_ERR("vir list of descriptor is not null(0x%p)",this->m_pIspDescript_vir);
                return MFALSE;
            }

            //descriptor phy ptr chk,if not NULL, means imem was not freed also
            if(this->m_pIspDescript_phy){
                LOG_ERR("phy list of descriptor is not null(0x%p)",this->m_pIspDescript_phy);
                return MFALSE;
            }

            //imem allcoate
            LOG_DBG("descriptor size:0x%zx",sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
            this->m_ispCQDescBufInfo.size = (((sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX) + 0x3) & (~0x3)); //4byte alignment,round up
            this->m_ispCQDescBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);
            if ( this->m_pMemDrv->allocVirtBuf(&this->m_ispCQDescBufInfo) ){
                LOG_ERR("m_pMemDrv->allocVirtBuf fail");
                return MFALSE;
            }
            else{
                if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispCQDescBufInfo, MFALSE) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                    return MFALSE;
                }
            }
            //alignment chk
            if(( (_tmpVirAddr=this->m_ispCQDescBufInfo.virtAddr) % 0x4) != 0 ){
                LOG_INF("iMem virDescriptAddr not 4 bytes alignment(0x%p), round up",(void*)this->m_ispCQDescBufInfo.virtAddr);
                _tmpVirAddr = (this->m_ispCQDescBufInfo.virtAddr + 0x3) & (~0x3);
            }
            if(( (_tmpPhyAddr=this->m_ispCQDescBufInfo.phyAddr) % 0x4) != 0 ){
                LOG_INF("iMem phyDescriptAddr not 4 bytes alignment(0x%p), round up",(void*)this->m_ispCQDescBufInfo.phyAddr);
                _tmpPhyAddr = (this->m_ispCQDescBufInfo.phyAddr + 0x3) & (~0x3);
            }
            this->m_pIspDescript_vir = (ISP_DRV_CQ_CMD_DESC_STRUCT*)_tmpVirAddr;
            this->m_pIspDescript_phy = (MUINT32*)_tmpPhyAddr;
            //
            //descriptor initial
            for(int m=0;m<(CAM_CAM_MODULE_MAX - 1);m++){
                this->m_pIspDescript_vir[m].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule);
                this->m_pIspDescript_vir[m].v_reg_addr = (MUINT32)((MUINTPTR)this->m_pIspVirRegAddr_pa & 0xffffffff);
            }
            this->m_pIspDescript_vir[CAM_CAM_END_].u.cmd = ISP_DRV_CQ_END_TOKEN;
            this->m_pIspDescript_vir[CAM_CAM_END_].v_reg_addr = (MUINT32)((MUINTPTR)this->m_pIspVirRegAddr_pa & 0xffffffff);

            LOG_INF("CQDescriptor:Virt[%p]/Phy[%p],size/num(%zd/%d)",\
                (MUINT8*)this->m_pIspDescript_vir,\
                (MUINT8*)this->m_pIspDescript_phy,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT),CAM_CAM_MODULE_MAX);
            break;
        case 1:
            //descriptor size
            //descriptor size is fixed to 1 dmao ba + 1 fh ba + 1 end token
            switch(this->m_CQ){
                case ISP_DRV_CQ_THRE0:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                case ISP_DRV_CQ_THRE1:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR1_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                case ISP_DRV_CQ_THRE10:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR10_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                case ISP_DRV_CQ_THRE28:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR28_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                default:
                    LOG_ERR("unsuppoted cq:0x%x\n",this->m_CQ);
                    return MFALSE;
                    break;
            }
            break;
    }
    return MTRUE;
}

/**
    method for CQ3/4/7/8 . can't support dynamic descriptor operation, vir reg read/write .
    vir reg and descriptor are mixed together.
*/
MBOOL ISP_DRV_CAM::CQ_Allocate_method2(MUINT32 step)
{
    MUINTPTR _tmpVirAddr,_tmpPhyAddr;

    switch(step){
        case 0:
            //CQ descriptor + vir reg
            //descriptor vir ptr chk,if not NULL, means imem was not freed also
            if(this->m_pIspDescript_vir){
                LOG_ERR("vir list of descriptor is not null(0x%p)",this->m_pIspDescript_vir);
                return MFALSE;
            }

            //descriptor phy ptr chk,if not NULL, means imem was not freed also
            if(this->m_pIspDescript_phy){
                LOG_ERR("phy list of descriptor is not null(0x%p)",this->m_pIspDescript_phy);
                return MFALSE;
            }

            //imem allcoate
            LOG_DBG("descriptor size:0x%zx",sizeof(CQ_RTBC_RING_ST_CAM));
            this->m_ispCQDescBufInfo.size = (((sizeof(CQ_RTBC_RING_ST_CAM)) + 0x3) & (~0x3)); //4byte alignment,round up
            this->m_ispCQDescBufInfo.useNoncache = (CmdQ_Cache==1)? (0):(1);
            if ( this->m_pMemDrv->allocVirtBuf(&this->m_ispCQDescBufInfo) ){
                LOG_ERR("m_pMemDrv->allocVirtBuf fail");
                return MFALSE;
            }
            else{
                if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispCQDescBufInfo, MFALSE) ) {
                    LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
                    return MFALSE;
                }
            }
            //alignment chk
            if(( (_tmpVirAddr=this->m_ispCQDescBufInfo.virtAddr) % 0x4) != 0 ){
                LOG_INF("iMem virDescriptAddr not 4 bytes alignment(0x%p), round up",(void*)this->m_ispCQDescBufInfo.virtAddr);
                _tmpVirAddr = (this->m_ispCQDescBufInfo.virtAddr + 0x3) & (~0x3);
            }
            if(( (_tmpPhyAddr=this->m_ispCQDescBufInfo.phyAddr) % 0x4) != 0 ){
                LOG_INF("iMem phyDescriptAddr not 4 bytes alignment(0x%p), round up",(void*)this->m_ispCQDescBufInfo.phyAddr);
                _tmpPhyAddr = (this->m_ispCQDescBufInfo.phyAddr + 0x3) & (~0x3);
            }
            this->m_pIspDescript_vir = (ISP_DRV_CQ_CMD_DESC_STRUCT*)_tmpVirAddr;
            this->m_pIspDescript_phy = (MUINT32*)_tmpPhyAddr;
            //
            //descriptor initial
            memset((MUINT8*)this->m_pIspDescript_vir, 0, sizeof(CQ_RTBC_RING_ST_CAM));

            LOG_INF("CQDescriptor:Virt[%p]/Phy[%p],size/num(%zd/%d)",\
                (MUINT8*)this->m_pIspDescript_vir,\
                (MUINT8*)this->m_pIspDescript_phy,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT),CAM_CAM_MODULE_MAX);
            break;
        case 1:
            //descriptor size + 1 is for END_TOKEN
            switch(this->m_CQ){
                case ISP_DRV_CQ_THRE4:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR4_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE5:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR5_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE7:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR7_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE8:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR8_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE12:
                    CAM_WRITE_REG(this->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR12_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                default:
                    LOG_ERR("unsuppoted cq:0x%x\n",this->m_CQ);
                    return MFALSE;
                    break;
            }
            break;
    }
    return MTRUE;
}


MBOOL ISP_DRV_CAM::init(const char* userName)
{
    MBOOL Result = MTRUE, bPrint = MTRUE;
    MINT32 tmp=0;
    MUINT32 userNameInitCnt = 0;
    std::string strUserName = userName;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_init,__FUNCTION__) == MFALSE)
        return MFALSE;

    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }

    //
    if ((strUserName.compare(0,10,"CmdQMgrImp") == 0) || (strUserName.compare(0,5,"dummy") == 0)) {
        bPrint = MFALSE;
    }

    if (bPrint) {
        LOG_INF(" - E. m_UserCnt(%d),createUser(%s),useUser(%s),"
                "module:0x%x,cq:0x%x,pageIdx:0x%x",
                this->m_UserCnt, this->m_useName, userName,
                this->m_Module, this->m_CQ, this->m_pageIdx);
    }

    auto itr = std::find(this->m_useNameInfo.begin(),
                         this->m_useNameInfo.end(),
                         userName);

    if (itr != m_useNameInfo.end()) {
        LOG_ERR("duplicate user name:%s", userName);
        return MTRUE;
    } else {
        this->m_useNameInfo.push_back(userName);
    }

    if(this->m_UserCnt > 0) {
        tmp = android_atomic_inc(&this->m_UserCnt);
        if (bPrint) {
            LOG_INF(" - X. m_UserCnt: %d. createUser(%s) useUser(%s)",
                    this->m_UserCnt, this->m_useName, userName);
        }
        userNameInitCnt = std::count(this->m_useNameInfo.begin(),
            this->m_useNameInfo.end(), userName);

        if(userNameInitCnt == 1) {
            if(this->m_pIspDrvImp->init(userName) == MFALSE) {
                Result = MFALSE;
                LOG_ERR("isp drv init fail\n");
                goto EXIT;
            }
        }
        return MTRUE;
    }

    // init ispdrvimp
    //to avoid iIspDrv userCnt over upper bound
    if(this->m_pIspDrvImp->init(userName) == MFALSE){
        Result = MFALSE;
        LOG_ERR("isp drv init fail\n");
        goto EXIT;
    }

    // init imem
    this->m_pMemDrv = IMemDrv::createInstance();
    if(this->m_pMemDrv->init() == MFALSE){
        Result = MFALSE;
        LOG_ERR("imem fail\n");
        goto EXIT;
    }

    //descriptor initial
    switch(this->m_CQ){
        case ISP_DRV_CQ_THRE3:
        case ISP_DRV_CQ_THRE4:
        case ISP_DRV_CQ_THRE5:
        case ISP_DRV_CQ_THRE6:
        case ISP_DRV_CQ_THRE7:
        case ISP_DRV_CQ_THRE8:
        case ISP_DRV_CQ_THRE9:
        case ISP_DRV_CQ_THRE12:
            if(this->CQ_Allocate_method2(0) == MFALSE){
                LOG_ERR("vir CQ_0x%x allocate fail\n",this->m_CQ);
                Result = MFALSE;
                goto EXIT;
            }
            break;
        case ISP_DRV_CQ_THRE0:
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE28:
        case ISP_DRV_CQ_THRE10:
            if(this->CQ_Allocate_method1(0) == MFALSE){
                LOG_ERR("vir CQ_0x%x allocate fail\n",this->m_CQ);
                Result = MFALSE;
                goto EXIT;
            }
            break;
        default:
            LOG_ERR("unsupported CQ:0x%x\n",this->m_CQ);
            Result = MFALSE;
            goto EXIT;
            break;
    }


    //
    tmp = android_atomic_inc(&this->m_UserCnt);
    //
    this->m_FSM = IspDrv_Init;

    //ion handle
    ISP_DEV_ION_NODE_STRUCT IonNode;
    IonNode.devNode = this->m_HWModule;
    IonNode.memID = this->m_ispCQDescBufInfo.memID;
    IonNode.dmaPort = _CQ_ID_Mapping(this->m_CQ);
    this->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);

    //cq descritpor size config
    //must put here , becuase of FSM chk
    switch(this->m_CQ){
        case ISP_DRV_CQ_THRE3:
        case ISP_DRV_CQ_THRE4:
        case ISP_DRV_CQ_THRE5:
        case ISP_DRV_CQ_THRE6:
        case ISP_DRV_CQ_THRE7:
        case ISP_DRV_CQ_THRE8:
        case ISP_DRV_CQ_THRE9:
        case ISP_DRV_CQ_THRE12:
            if(this->CQ_Allocate_method2(1) == MFALSE){
                LOG_ERR("vir CQ_0x%x allocate fail\n",this->m_CQ);
                Result = MFALSE;
                goto EXIT;
            }
            break;
        case ISP_DRV_CQ_THRE0:
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE28:
        case ISP_DRV_CQ_THRE10:
            if(this->CQ_Allocate_method1(1) == MFALSE){
                LOG_ERR("vir CQ_0x%x allocate fail\n",this->m_CQ);
                Result = MFALSE;
                goto EXIT;
            }
            //ion handle
            if(this->m_CQ == ISP_DRV_CQ_THRE0)
                IonNode.dmaPort = _dma_cq0i_vir;
            IonNode.memID = this->m_ispVirRegBufInfo.memID;
            this->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);
            break;
        default:
            LOG_ERR("unsupported CQ:0x%x\n",this->m_CQ);
            Result = MFALSE;
            goto EXIT;
            break;
    }
EXIT:

    //
    if (bPrint)
        LOG_INF(" - X. ret: %d. mInitCount: %d.", Result, this->m_UserCnt);

    return Result;
}


MBOOL ISP_DRV_CAM::uninit(const char* userName)
{
    MBOOL Result = MTRUE, bPrint = MTRUE;
    MINT32 tmp=0;
    MUINT32 useNameUninitCnt = 0;
    std::string strUserName = userName;
    //MUINT32 bClr = 0;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_Uninit,__FUNCTION__) == MFALSE)
        return MFALSE;
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }

    if ((strUserName.compare(0,10,"CmdQMgrImp") == 0) || (strUserName.compare(0,5,"dummy") == 0)) {
        bPrint = MFALSE;
    }

    if (bPrint)
        LOG_INF(" - E. m_UserCnt(%d), createUser(%s) curUser(%s)",
            this->m_UserCnt, this->m_useName, userName);
    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in isp_drv_cam , curUser(%s)",userName);
        return MFALSE;
    }

    // remove userName
    auto itr = std::find(this->m_useNameInfo.begin(),
                         this->m_useNameInfo.end(),
                         userName);

    if (itr != m_useNameInfo.end()) {
        m_useNameInfo.erase(itr);
    } else {
        LOG_ERR("No matching m_useNameInfo:%s\n",userName);

        char str[200] = {0,};
        char str1[200] = {0,};
        for (MUINT32 i = 0; i < m_useNameInfo.size(); i++) {
            sprintf(str, "%sm_useNameInfo(%d):%s, ", str1, i, m_useNameInfo.at(i).c_str());

            if (((i+1) % 6) == 0) {
                if (bPrint) {
                    LOG_INF("%s", str);
                }
                memset(str, 0, sizeof(str));
            }
        }
        if(str[0] != 0) {
            if (bPrint) {
                LOG_INF("%s", str);
            }
        }

        return MFALSE;
    }

    // More than one user
    tmp = android_atomic_dec(&this->m_UserCnt);
    if(this->m_UserCnt > 0)    // If there are still users, exit.
    {
        useNameUninitCnt = std::count(this->m_useNameInfo.begin(),
                                      this-> m_useNameInfo.end(),
                                      userName);
        if(useNameUninitCnt == 0) {
            this->m_pIspDrvImp->uninit(userName);
        }
        goto EXIT;
    }

    //free cq descriptor
    if(this->m_ispCQDescBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispCQDescBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispCQDescBufInfo, MFALSE) != 0){
                LOG_ERR("free descriptor fail");
                Result = MFALSE;
                goto EXIT;
            }
        }
        else{
            LOG_ERR("unmap descriptor fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    else{
        if (bPrint)
            LOG_INF("cq descriptor PA is already NULL");
    }
    this->m_pIspDescript_vir = NULL;
    this->m_pIspDescript_phy = NULL;

    //free virtual isp
    if(this->m_ispVirRegBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispVirRegBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispVirRegBufInfo, MFALSE) != 0){
                LOG_ERR("free virRegister fail");
                Result = MFALSE;
                goto EXIT;
            }
        }
        else{
            LOG_ERR("unmap virRegister fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    else{
        LOG_INF("cq virRegister PA is already NULL");
    }
    this->m_pIspVirRegAddr_va = NULL;
    this->m_pIspVirRegAddr_pa = NULL;

    //
    //IMEM
    this->m_pMemDrv->uninit();
    this->m_pMemDrv->destroyInstance();
    this->m_pMemDrv = NULL;

    this->m_pIspDrvImp->uninit(userName);
    this->m_pIspDrvImp->destroyInstance();

    //
    this->m_FSM = IspDrv_Uninit;
    //
    this->m_currentDCIFsumsampleRecode = 0;

    //For secure camera
    if(this->m_SecOn){
        for (MUINT32 i = 0; i < PHY_CAM; i++)
            this->m_DapcReg[i].clear();

        m_DapcIdx.clear();
        this->m_SecOn = 0;
    }
EXIT:

    if (bPrint)
        LOG_INF(" - X. ret: %d. mInitCount: %d.", Result, this->m_UserCnt);

    return Result;
}

MBOOL ISP_DRV_CAM::start(void)
{
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_start,__FUNCTION__) == MFALSE)
        return MFALSE;

    LOG_DBG("[0x%x_0x%x_0x%x]start -",this->m_Module,this->m_CQ,this->m_pageIdx);
    this->m_FSM = IspDrv_Start;
    return MTRUE;
}

MBOOL ISP_DRV_CAM::stop(void)
{
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_stop,__FUNCTION__) == MFALSE)
        return MFALSE;

    LOG_DBG("[0x%x_0x%x_0x%x]stop -",this->m_Module,this->m_CQ,this->m_pageIdx);
    this->m_FSM = IspDrv_Stop;
    return MTRUE;
}


MBOOL ISP_DRV_CAM::waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig,__FUNCTION__) == MFALSE)
        return MFALSE;
    return this->m_pIspDrvImp->waitIrq(pWaitIrq);
}

MBOOL ISP_DRV_CAM::waitIrq(ISP_WAIT_IRQ_STRUCT* pWaitIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig,__FUNCTION__) == MFALSE)
        return MFALSE;
    return this->m_pIspDrvImp->waitIrq(pWaitIrq);
}

MBOOL ISP_DRV_CAM::clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig,__FUNCTION__) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->clearIrq(pClearIrq);
}

MBOOL ISP_DRV_CAM::registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig,__FUNCTION__) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->registerIrq(pRegIrq);
}


MBOOL ISP_DRV_CAM::signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    LOG_INF("IRQ SIGNAL:module:0x%x, userKey:0x%x, status:0x%x",this->m_Module,pWaitIrq->UserKey,pWaitIrq->Status);
    ISP_WAIT_IRQ_STRUCT wait;

    if(this->FSM_CHK(OP_IspDrv_sig_sig,__FUNCTION__) == MFALSE)
        return MFALSE;

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(ISP_WAIT_IRQ_ST));
    switch(this->m_HWModule){
        case CAM_A:     wait.Type = ISP_IRQ_TYPE_INT_CAM_A_ST;
            break;
        case CAM_B:     wait.Type = ISP_IRQ_TYPE_INT_CAM_B_ST;
            break;
        case CAM_C:     wait.Type = ISP_IRQ_TYPE_INT_CAM_C_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWModule);
            return MFALSE;
            break;
    }

    if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_FLUSH_IRQ_REQUEST,&wait) < 0){
        LOG_ERR("signal IRQ fail(irq:0x%x,status:0x%x)",wait.Type,wait.EventInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL ISP_DRV_CAM::signalIrq(ISP_WAIT_IRQ_STRUCT* pWaitIrq)
{
    LOG_INF("IRQ SIGNAL:module:0x%x, userKey:0x%x, status:0x%x",pWaitIrq->Type,pWaitIrq->EventInfo.UserKey,pWaitIrq->EventInfo.Status);
    ISP_WAIT_IRQ_STRUCT wait;

    if(this->FSM_CHK(OP_IspDrv_sig_sig,__FUNCTION__) == MFALSE)
        return MFALSE;

    memcpy(&wait,pWaitIrq,sizeof(ISP_WAIT_IRQ_STRUCT));

    if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_FLUSH_IRQ_REQUEST,&wait) < 0){
        LOG_ERR("signal IRQ fail(irq:0x%x,status:0x%x)",wait.Type,wait.EventInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL ISP_DRV_CAM::getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    MBOOL rst = MTRUE;
    if(this->FSM_CHK(OP_IspDrv_Device,__FUNCTION__) == MFALSE)
        return MFALSE;

    switch(eCmd){
        case _GET_SOF_CNT:
            switch(*(MUINT32*)pData){
                case CAM_A:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_A_ST;
                    break;
                case CAM_B:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_B_ST;
                    break;
                case CAM_C:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_C_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_CUR_SOF,(unsigned char*)pData) < 0){
                LOG_ERR("dump sof fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_DMA_ERR:
            switch(this->m_HWModule){
                case CAM_A:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_A_ST;
                    break;
                case CAM_B:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_B_ST;
                    break;
                case CAM_C:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_C_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_DMA_ERR,(unsigned char*)pData) < 0){
                LOG_ERR("dump dma_err fail");
                rst = MFALSE;
            }
            break;
        case _GET_INT_ERR:
            {
                struct ISP_RAW_INT_STATUS IntStatus[ISP_IRQ_TYPE_AMOUNT];
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_INT_ERR,(struct ISP_RAW_INT_STATUS*)IntStatus) < 0){
                    LOG_ERR("dump int_err fail\n");
                    rst = MFALSE;
                }
                else{
                    switch(this->m_HWModule){
                        case CAM_A:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAM_A_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAM_B:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAM_B_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAM_C:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAM_C_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        default:
                            LOG_ERR("unsuported module:0x%x\n",this->m_HWModule);
                            break;
                    }
                }
            }
            break;
        case _GET_DROP_FRAME_STATUS:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_DROP_FRAME,(unsigned char*)pData) < 0){
                LOG_ERR("dump drop frame status fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_START_TIME:
            switch(*(MUINT32*)pData){
                case CAM_A:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_A_ST;
                    break;
                case CAM_B:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_B_ST;
                    break;
                case CAM_C:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_C_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_START_TIME,(unsigned char*)pData) < 0){
                LOG_ERR("get start time fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_VSYNC_CNT:
            switch(this->m_HWModule){
                case CAM_A:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_A_ST;
                    break;
                case CAM_B:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_B_ST;
                    break;
                case CAM_C:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAM_C_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_VSYNC_CNT,(unsigned char*)pData) < 0){
                LOG_ERR("_GET_VSYNC_CNT fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_SUPPORTED_ISP_CLOCKS:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_SUPPORTED_ISP_CLOCKS,(struct ISP_CLK_INFO*)pData) < 0){
                LOG_ERR("_GET_SUPPORTED_ISP_CLOCKS fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_CUR_ISP_CLOCK:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_CUR_ISP_CLOCK,(struct ISP_GET_CLK_INFO*)pData) < 0){
                LOG_ERR("_GET_CUR_ISP_CLOCK fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_GLOBAL_TIME:
            {
                MUINT64 time[_e_TS_max] = {0,};
                time[_e_mono_] = ((MUINT64*)pData)[_e_mono_];
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_GLOBAL_TIME,(unsigned long long*)time) < 0){
                    LOG_ERR("_GET_GLOBAL_TIME fail\n");
                    rst = MFALSE;
                }
                memcpy(pData, (void*)time, sizeof(MUINT64)*_e_TS_max);
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x\n",eCmd);
            return MFALSE;
        break;
    }
    return rst;
}

MBOOL ISP_DRV_CAM::setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    if(this->FSM_CHK(OP_IspDrv_Device,__FUNCTION__) == MFALSE)
        return MFALSE;

    switch(eCmd){
        case _SET_DBG_INT:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DEBUG_FLAG,(unsigned char*)pData) < 0){
                LOG_ERR("kernel log enable error\n");
                return MFALSE;
            }
            break;
        case _SET_VF_OFF:
            {
                MUINT32 dbg[2];
                switch(this->m_HWModule){
                    case CAM_A: dbg[1] = ISP_CAM_A_IDX;
                        break;
                    case CAM_B: dbg[1] = ISP_CAM_B_IDX;
                        break;
                    case CAM_C: dbg[1] = ISP_CAM_C_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_HWModule);
                        return MFALSE;
                }
                dbg[0] = 0;
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_VF_LOG,(unsigned int*)dbg) < 0){
                    LOG_ERR("_SET_VF_OFF error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_VF_ON:
            {
                MUINT32 dbg[2];
                switch(this->m_HWModule){
                    case CAM_A: dbg[1] = ISP_CAM_A_IDX;
                        break;
                    case CAM_B: dbg[1] = ISP_CAM_B_IDX;
                        break;
                    case CAM_C: dbg[1] = ISP_CAM_C_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_HWModule);
                        return MFALSE;
                }
                dbg[0] = 1;
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_VF_LOG,(unsigned char*)dbg) < 0){
                    LOG_ERR("_SET_VF_ON error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_BUF_CTRL:
            switch(this->m_HWModule){
                case CAM_A:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAM_A_ST;
                    break;
                case CAM_B:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAM_B_ST;
                    break;
                case CAM_C:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAM_C_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_BUFFER_CTRL,(ISP_BUFFER_CTRL_STRUCT*)pData) < 0){
                LOG_ERR("_set_buf_ctrl error,ctrl:0x%x\n",((ISP_BUFFER_CTRL_STRUCT*)pData)->ctrl);
                return MFALSE;
            }
            break;
        case _SET_RESET_HW_MOD:
            {
                MUINT32 resetModule = ISP_CAM_A_IDX;

                switch(this->m_HWModule){
                    case CAM_A:
                        resetModule = ISP_CAM_A_IDX;
                        break;
                    case CAM_B:
                        resetModule = ISP_CAM_B_IDX;
                        break;
                    case CAM_C:
                        resetModule = ISP_CAM_C_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_HWModule);
                        return MFALSE;
                }

                LOG_INF("Reset hw module: %d, corresponding resetModule: %d ......\n", this->m_HWModule, resetModule);
                if (ioctl(this->m_pIspDrvImp->m_Fd, ISP_RESET_BY_HWMODULE, &resetModule) < 0) {
                    LOG_ERR("Error: Fail reset hw module: %d, corresponding resetModule: %d\n", this->m_HWModule, resetModule);
                    return MFALSE;
                }

            }
            break;
        case _SET_LABR_MMU: {
            ISP_LARB_MMU_STRUCT *pLarbInfo = (ISP_LARB_MMU_STRUCT *)pData;

            if (pLarbInfo == NULL) {
                LOG_ERR("Invalid LARB INFO struct !\n");
                return MFALSE;
            }

            LOG_INF("Config MMU Larb=%d offset=0x%x val=0x%x\n", pLarbInfo->LarbNum, pLarbInfo->regOffset, pLarbInfo->regVal);

            if (ioctl(this->m_pIspDrvImp->m_Fd, ISP_LARB_MMU_CTL, (ISP_LARB_MMU_STRUCT*)pLarbInfo) < 0) {
                LOG_ERR("ISP_LARB_MMU_CTL fail...\n");
                return MFALSE;
            }

            }
            break;
        case _SET_ION_HANDLE:
            if(((ISP_DEV_ION_NODE_STRUCT*)pData)->memID <= 0) {
                LOG_ERR("_set_ion_handle error, memID(%d_%d)\n",
                    ((ISP_DEV_ION_NODE_STRUCT*)pData)->memID,
                    ((ISP_DEV_ION_NODE_STRUCT*)pData)->dmaPort);
                return MFALSE;
            }
            {
                ISP_DEV_ION_NODE_STRUCT node;
                node.dmaPort = ((ISP_DEV_ION_NODE_STRUCT*)pData)->dmaPort;
                node.memID = ((ISP_DEV_ION_NODE_STRUCT*)pData)->memID;

                switch(this->m_HWModule){
                    case CAM_A: node.devNode = (unsigned int)ISP_CAM_A_IDX;
                        break;
                    case CAM_B: node.devNode = (unsigned int)ISP_CAM_B_IDX;
                        break;
                    case CAM_C: node.devNode = (unsigned int)ISP_CAM_C_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:%d\n",((ISP_DEV_ION_NODE_STRUCT*)pData)->devNode);
                        break;
                }
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_ION_IMPORT,(ISP_DEV_ION_NODE_STRUCT*)&node) < 0){
                    LOG_ERR("_set_ion_handle error(%d_%d_%d)\n",
                        node.devNode,
                        node.memID,
                        node.dmaPort);
                    return MFALSE;
                }
                LOG_DBG("_set_ion_handle(%d_%d_%d)\n", node.devNode, node.memID, node.dmaPort);
            }
            break;
        case _SET_ION_FREE:
            if(((ISP_DEV_ION_NODE_STRUCT*)pData)->memID <= 0) {
                LOG_WRN("_set_ion_free error, memID(%d)\n",
                    ((ISP_DEV_ION_NODE_STRUCT*)pData)->memID);
                return MFALSE;
            }
            {
                ISP_DEV_ION_NODE_STRUCT node;
                node.dmaPort = ((ISP_DEV_ION_NODE_STRUCT*)pData)->dmaPort;
                node.memID = ((ISP_DEV_ION_NODE_STRUCT*)pData)->memID;

                switch(this->m_HWModule){
                    case CAM_A: node.devNode = (unsigned int)ISP_CAM_A_IDX;
                        break;
                    case CAM_B: node.devNode = (unsigned int)ISP_CAM_B_IDX;
                        break;
                    case CAM_C: node.devNode = (unsigned int)ISP_CAM_C_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:%d\n",((ISP_DEV_ION_NODE_STRUCT*)pData)->devNode);
                        break;
                }
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_ION_FREE,(ISP_DEV_ION_NODE_STRUCT*)&node) < 0){
                    LOG_WRN("_set_ion_free error(%d_%d_%d)\n",
                        node.devNode,
                        node.memID,
                        node.dmaPort);
                    return MFALSE;
                }
                LOG_DBG("_set_ion_free(%d_%d_%d)\n", node.devNode, node.memID, node.dmaPort);
            }
            break;
        case _SET_ION_FREE_BY_HWMODULE:
            MUINT32 module;
            switch(*pData){
                case CAM_A: module = ISP_CAM_A_IDX;
                    break;
                case CAM_B: module = ISP_CAM_B_IDX;
                    break;
                case CAM_C: module = ISP_CAM_C_IDX;
                    break;
                default:
                    LOG_ERR("unsupported module:%d\n",*pData);
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_ION_FREE_BY_HWMODULE,(ISP_DEV_ION_NODE_STRUCT*)&module) < 0){
                LOG_WRN("_ion_free_by_module error(%d)\n", (*pData));
                return MFALSE;
            }
            LOG_DBG("_ion_free_by_module(%d)\n", (*pData));
            break;
        case _RESET_VSYNC_CNT:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_RESET_VSYNC_CNT,NULL) < 0){
                LOG_ERR("_RESET_VSYNC_CNT error,ctrl:0x%x\n",((ISP_BUFFER_CTRL_STRUCT*)pData)->ctrl);
                return MFALSE;
            }
            break;
        case _SET_CQ_SW_PATCH:
            {
                ((ISP_MULTI_RAW_CONFIG*)pData)->HWmodule = this->m_HWModule;

                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_CQ_SW_PATCH,(ISP_MULTI_RAW_CONFIG*)pData) < 0){
                    LOG_ERR("CQ SW PATCH error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_DFS_UPDATE:
            if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_DFS_UPDATE, (unsigned int*)pData) < 0){
                LOG_ERR("Update DFS error(%d)\n", (*pData));
                return MFALSE;
            }
            break;
        case _SET_DFS_RESET:
            {
                MUINT32 camsys_pmqos = MTRUE;
                if (ioctl(this->m_Fd, ISP_DFS_CTRL, (MUINT8*)& camsys_pmqos) < 0) {
                    LOG_ERR("ISP_DFS_CTRL turn on error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_PM_QOS_INFO:
            if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_SET_PM_QOS_INFO, (ISP_PM_QOS_INFO_STRUCT*)pData) < 0){
                LOG_ERR("ISP_SET_DMAO_BW_INFO error\n");
                return MFALSE;
            }
            break;
        case _SET_PM_QOS_RESET:
            {
                MUINT32 dbg[2];
                switch(*pData){
                    case CAM_A:
                        dbg[1] = ISP_IRQ_TYPE_INT_CAM_A_ST;
                        break;
                    case CAM_B:
                        dbg[1] = ISP_IRQ_TYPE_INT_CAM_B_ST;
                        break;
                    case CAM_C:
                        dbg[1] = ISP_IRQ_TYPE_INT_CAM_C_ST;
                        break;
                    default:
                        LOG_ERR("unsuported module:0x%x\n",this->m_HWModule);
                        break;
                }

                dbg[0] = 0;
                if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_SET_PM_QOS, (unsigned int*)dbg) < 0){
                    LOG_ERR("_SET_PM_QOS_ON error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_SEC_DAPC_REG:
            {
                MUINT32 dbg[10];

                switch(this->m_HWModule){
                    case CAM_A: dbg[0] = ISP_CAM_A_IDX;
                        break;
                    case CAM_B: dbg[0] = ISP_CAM_B_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_HWModule);
                        return MFALSE;
                }

                dbg[1] = 1;
                dbg[2] = this->readReg(0x4,this->m_HWModule);
                dbg[3] = this->readReg(0x8,this->m_HWModule);
                dbg[4] = this->readReg(0xc,this->m_HWModule);
                dbg[5] = this->readReg(0x10,this->m_HWModule);
                dbg[6] = this->readReg(0x14,this->m_HWModule);
                dbg[7] = this->readReg(0x18,this->m_HWModule);
                dbg[8] = this->readReg(0x1c,this->m_HWModule);
                dbg[9] = this->readReg(0x20,this->m_HWModule);
                LOG_INF("[DAPC]dbg[0]:0x%x dbg[1]:0x%x dbg[2]:0x%x dbg[3]:0x%x dbg[4]:0x%x dbg[5]:0x%x",
                        dbg[0],dbg[1],dbg[2],dbg[3],dbg[4],dbg[5]);
                LOG_INF("[DAPC]dbg[6]:0x%x dbg[7]:0x%x dbg[8]:0x%x dbg[9]:0x%x",
                        dbg[6],dbg[7],dbg[8],dbg[9]);
                if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_SET_SEC_DAPC_REG, (unsigned int*)dbg) < 0){
                    LOG_ERR("_SET_SEC_DAPC_REG error\n");
                    return MFALSE;
                }

            }
            break;
        case _NOTE_CQTHR0_BASE:
        {
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_NOTE_CQTHR0_BASE,(unsigned int*)pData) < 0){
                LOG_ERR("_NOTE_CQTHR0_BASE fail\n");
                return MFALSE;
            }
        }
        break;
        default:
            LOG_ERR("unsupported cmd:0x%x",eCmd);
            return MFALSE;
        break;
    }
    return MTRUE;
}


MUINT32 ISP_DRV_CAM::VirReg_OFFSET(MUINT32 hwModule)
{
    MUINT32 shift = 0x0;
    switch(hwModule){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            shift = 0x0;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",hwModule);
            return 0;
            break;
    }

    return shift;
}

MBOOL ISP_DRV_CAM::cqAddModule(MUINT32 moduleId)
{
    switch(moduleId) {
        case TWIN_CQ0_BASEADDRESS:
        case TWIN_CQ1_BASEADDRESS:
        case TWIN_CQ10_BASEADDRESS:
#if TWIN_CQ_SW_WORKAROUND
        case CAM_CQ0_EN_:
        case CAM_CQ0_TRIG_:
        case CAM_CQ1_EN_:
        case CAM_CQ1_TRIG_:
        case CAM_CQ10_EN_:
        case CAM_CQ10_TRIG_:
#endif
            LOG_ERR("Err CQ_module(0x%x) in this function", moduleId);
            return MFALSE;
        default:
            break;
    }

    return cqAddModule(moduleId, ISP_DRV_CQ_NONE, PHY_CAM);
}

MBOOL ISP_DRV_CAM::cqAddModule(MUINT32 moduleId, ISP_HW_MODULE slave_cam)
{
    switch(moduleId) {
        case TWIN_CQ0_BASEADDRESS:
        case TWIN_CQ1_BASEADDRESS:
        case TWIN_CQ10_BASEADDRESS:
#if TWIN_CQ_SW_WORKAROUND
        case CAM_CQ0_EN_:
        case CAM_CQ0_TRIG_:
        case CAM_CQ1_EN_:
        case CAM_CQ1_TRIG_:
        case CAM_CQ10_EN_:
        case CAM_CQ10_TRIG_:
#endif
            break;
        default:
            LOG_ERR("Err CQ_module(0x%x) in this function", moduleId);
            return MFALSE;
    }

    return cqAddModule(moduleId, ISP_DRV_CQ_NONE, slave_cam);
}

MBOOL ISP_DRV_CAM::cqAddModule(MUINT32 moduleId, MUINT32 extModuleId, ISP_HW_MODULE slave_cam)
{
    int cmd;
    MUINT32 offset=0x0;
    MUINT32 cam_oft = 0x0;
    MUINTPTR dummyaddr;

    LOG_DBG("[0x%x_0x%x_0x%x]moduleID:0x%x",this->m_HWModule,this->m_CQ,this->m_pageIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if(mIspCQModuleInfo[moduleId].reg_set_by_cq != 1) {
        LOG_DBG("This moduleID(%d) don't set by CQ, Check the isp_reg_if_cam.h\n", moduleId);
        return MTRUE;
    }

    switch(this->m_HWModule){
        case CAM_A:
            break;
        case CAM_B:
                offset = CAM_BASE_RANGE * 2;//cam_b hw start at 0x20000
            break;
        case CAM_C:
                offset = CAM_BASE_RANGE * 4;//cam_c hw start at 0x30000
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWModule);
            return MFALSE;
            break;
    }

    switch(moduleId){
        case CAM_NEXT_Thread_:
            {
                /* For use CQ10 trigger CQ11 inner register */
                /* m_CQ is current CQ, i.e. CQ11, so another arg extModuleId is needed to pass CQ11 arg */
                MUINT32 _cq = (extModuleId == ISP_DRV_CQ_NONE) ? this->m_CQ : extModuleId;

                /*
                 * cam_a set/clr : 0x1a05a000/0x1a05b000
                 * cam_b set/clr : 0x1a05c000/0x1a05d000
                 * cam_c set/clr : 0x1a05e000/0x1a05f000
                 */
                switch(this->m_HWModule){
                case CAM_A:
                    offset = 0x0;
                    break;
                case CAM_B:
                    offset = CAM_BASE_RANGE_SETCLR * 2;
                    break;
                case CAM_C:
                    offset = CAM_BASE_RANGE_SETCLR * 4;
                    break;
                default:
                    LOG_ERR("unsupported hw module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                }

                if (_cq >= ISP_DRV_CQ_NONE) {
                    LOG_ERR("unsupported hw module:0x%x, extModuleId: %d\n", this->m_Module, extModuleId);
                    return MFALSE;
                }

                dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspCQModuleInfo_SpecialCQ[_cq].sw_addr_ofst);

                //
                dummyaddr += this->VirReg_OFFSET(this->m_HWModule);

                //
                cmd = DESCRIPTOR_TOKEN(mIspCQModuleInfo_SpecialCQ[_cq].addr_ofst,mIspCQModuleInfo_SpecialCQ[_cq].reg_num,offset);

                this->m_pIspDescript_vir[moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
                this->m_pIspDescript_vir[moduleId].u.cmd = cmd;
            }
            break;
        default:
            if (slave_cam < PHY_CAM) {
                cam_oft = this->twinPath_CQ_Ctrl(moduleId, slave_cam);
            }

            dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspCQModuleInfo[moduleId].sw_addr_ofst);

            //
            dummyaddr += this->VirReg_OFFSET(this->m_HWModule);

            //
            cmd = DESCRIPTOR_TOKEN((mIspCQModuleInfo[moduleId].addr_ofst+cam_oft),mIspCQModuleInfo[moduleId].reg_num,offset);

            this->m_pIspDescript_vir[moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
            this->m_pIspDescript_vir[moduleId].u.cmd = cmd;
            break;
    }

    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MUINT32 ISP_DRV_CAM::twinPath_CQ_Ctrl(MUINT32 moduleId, ISP_HW_MODULE slave_cam)
{
    //no racing issue for mIspCQModuleInfo when cam_a/cam_b runing with multi-thread.
    //at multi-sesnor case, cq0_en/cq0_trig won't be use.
    //at twin path, slave cam's module is cam_x_twin
    (void)moduleId;

    switch(slave_cam) {
        case CAM_A:
            return 0x10000; //cam_a hw start at 0x10000
            break;
        case CAM_B:
            return 0x10000 + CAM_BASE_RANGE * 2; //cam_b hw start at 0x20000
            break;
        case CAM_C:
            return 0x10000 + CAM_BASE_RANGE * 4; //cam_c hw start at 0x30000
            break;
        default:
            LOG_ERR("Wrong slave cam:0x%x", slave_cam);
            return 0x0;
            break;
    }
    return 0x0;
}

MBOOL ISP_DRV_CAM::cqDelModule(MUINT32 moduleId)
{
    LOG_DBG("[0x%x_0x%x_0x%x]moduleID:0x%x",this->m_Module,this->m_CQ,this->m_pageIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if(mIspCQModuleInfo[moduleId].reg_set_by_cq != 1) {
        LOG_DBG("This moduleID(%d) don't set by CQ, Check the isp_reg_if_cam.h\n", moduleId);
        return MTRUE;
    }

    this->m_pIspDescript_vir[moduleId].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule);
    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MBOOL ISP_DRV_CAM::cqNopModule(MUINT32 moduleId)
{
    LOG_DBG("[0x%x_0x%x]moduleID:0x%x",this->m_CQ,this->m_pageIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if(mIspCQModuleInfo[moduleId].reg_set_by_cq != 1) {
        LOG_DBG("This moduleID(%d) don't set by CQ, Check the isp_reg_if_cam.h\n", moduleId);
        return MTRUE;
    }

#if (CQ_DMA_SW_WORKAROUND == 1)
    this->m_pIspDescript_vir[moduleId].u.cmd = ISP_DRV_CQ_NOP_TOKEN(this->m_HWModule);
#else
    this->m_pIspDescript_vir[moduleId].u.cmd |= (ISP_CQ_NOP_INST<<26);
#endif
    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MBOOL ISP_DRV_CAM::cqApbModule(MUINT32 moduleId)
{
#if (CQ_DMA_SW_WORKAROUND == 1)
    if ((this->m_pIspDescript_vir[moduleId].u.cmd & CQ_DES_RANGE) != CQ_DUMMY_REG) {
        return cqAddModule(moduleId);
    }
#endif
    LOG_DBG("[0x%x_0x%x]moduleID:0x%x",this->m_CQ,this->m_pageIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if(mIspCQModuleInfo[moduleId].reg_set_by_cq != 1) {
        LOG_DBG("This moduleID(%d) don't set by CQ, Check the isp_reg_if_cam.h\n", moduleId);
        return MTRUE;
    }
#if (CQ_DMA_SW_WORKAROUND == 1)
    if ((this->m_pIspDescript_vir[moduleId].u.cmd & CQ_DES_RANGE) == CQ_DUMMY_REG) {
        LOG_DBG("[0x%x_0x%x]moduleID:0x%x",this->m_CQ,this->m_pageIdx,moduleId);
        this->m_pIspDescript_vir[moduleId].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN(moduleId);
    }
#else
    this->m_pIspDescript_vir[moduleId].u.cmd &= (0xFFFFFFFF - (ISP_CQ_NOP_INST<<26));
#endif
    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MUINT32* ISP_DRV_CAM::getCQDescBufPhyAddr(void)
{
    LOG_DBG("[0x%x_0x%x]+ ",this->m_CQ,this->m_pageIdx);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return (MUINT32*)this->m_pIspDescript_phy;
}

MUINT32* ISP_DRV_CAM::getCQDescBufVirAddr(void)
{
    LOG_DBG("[0x%x_0x%x]+ ",this->m_CQ,this->m_pageIdx);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return (MUINT32*)this->m_pIspDescript_vir;
}


MUINT32* ISP_DRV_CAM::getIspVirRegVirAddr(void)
{
    LOG_DBG("[0x%x_0x%x]+ ",this->m_CQ,this->m_pageIdx);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return (MUINT32*)this->m_pIspVirRegAddr_va;
}

MUINT32* ISP_DRV_CAM::getIspVirRegPhyAddr(void)
{
    LOG_DBG("[0x%x_0x%x]+ ",this->m_CQ,this->m_pageIdx);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return (MUINT32*)this->m_pIspVirRegAddr_pa;
}



MBOOL ISP_DRV_CAM::getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize)
{
    LOG_DBG("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    if(moduleId == CAM_NEXT_Thread_){
        addrOffset = mIspCQModuleInfo_SpecialCQ[this->m_CQ].sw_addr_ofst;
        moduleSize = mIspCQModuleInfo_SpecialCQ[this->m_CQ].reg_num;
    }
    else{
        addrOffset = mIspCQModuleInfo[moduleId].sw_addr_ofst;
        moduleSize = mIspCQModuleInfo[moduleId].reg_num;
    }
    return MTRUE;
}

MBOOL ISP_DRV_CAM::readPtrRegs(MUINT32 **ptr, MUINT32 offset, MUINT32 size,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    MUINT32 _addr = 0;
    MUINT32 *pIspRegMap = (MUINT32 *)*ptr;
    LOG_DBG("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    if(this->VirReg_ValidRW() == MFALSE)
        return MFALSE;

    switch(caller){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            legal_range = CAM_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    shift = this->VirReg_OFFSET(caller);

    for(unsigned int i=0; i<size; i++){
        _addr = offset + (i*0x4);
        if(_addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",_addr);
            return MFALSE;
        }
        pIspRegMap[(_addr>>2)] = this->m_pIspVirRegAddr_va[(_addr + shift)>>2];
        LOG_DBG("%d:addr:0x%x,data:0x%x",i,_addr,pIspRegMap[(_addr>>2)]);
    }
    return MTRUE;

}

IspDrv* ISP_DRV_CAM::getPhyObj(void)
{
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return this->m_pIspDrvImp;
}

MBOOL ISP_DRV_CAM::dumpCQRingTable(void)
{
    MUINT32 i = 0;
    CQ_RTBC_RING_ST_CAM *pcqrtbcring_va  = (CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_vir;

    if(pcqrtbcring_va == NULL){
        LOG_ERR("va is null:0x%p\n",pcqrtbcring_va);
        return 0;
    }
    if(pcqrtbcring_va->dma_ring_size > MAX_RING_SIZE){
        LOG_ERR("ring size out of range:0x%x\n",pcqrtbcring_va->dma_ring_size);
        return 0;
    }
    for(i=0;i<pcqrtbcring_va->dma_ring_size;i++){

        LOG_INF("cq:%d, next CQ(0x%x, 0x%x), inr(0x%x, 0x%x), table(0x%lx)",
            this->m_CQ,
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr.descriptor,
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr.data,
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.descriptor,
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.data,
            pcqrtbcring_va->rtbc_ring[i].next_pa);
    }

    return MTRUE;
}

MBOOL ISP_DRV_CAM::dumpCQTable(void)
{
/* 400 for ep max print char size
 *    MSG: libc: FORTIFY: vsprintf: prevented 521-byte write into 512-byte buffer
 */
#define __MAX_DUMP_STR_LENGTH__  (400)

    LOG_INF("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    string sstr;
    char _tmp[16] = {"\0"};

    //cam_next_thread is using another moduleinfo table

    for(MUINT32 i=0;i<CAM_NEXT_Thread_;i++){
        if(this->m_pIspDescript_vir[i].u.cmd != ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule)) {
            LOG_INF("[%d]:[0x%08x]:",i,this->m_pIspDescript_vir[i].u.cmd);
            //LOG_CQ_VIRTUAL_TABLE(this->m_pIspVirRegAddr_va,i,mIspCQModuleInfo[i].reg_num);
            sstr = "";
            for(unsigned int j=0;j<mIspCQModuleInfo[i].reg_num;j++){
                sprintf(_tmp,"0x%08x-",this->readReg(mIspCQModuleInfo[i].sw_addr_ofst + (j*0x4)));
                sstr += _tmp;
            }
            LOG_INF("[%d] cur size: %lu\n", i, (unsigned long)sstr.size());
            if (sstr.size() < __MAX_DUMP_STR_LENGTH__) {
                LOG_INF(" %s\n", sstr.c_str());
            }
            else {
                LOG_WRN("str too long %lu, truncate: %d\n",(unsigned long)sstr.size(), __MAX_DUMP_STR_LENGTH__);
                sstr.resize(__MAX_DUMP_STR_LENGTH__);
                LOG_INF(" %s\n", sstr.c_str());
            }
        }
    }
    if(this->m_pIspDescript_vir[CAM_NEXT_Thread_].u.cmd != ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule)) {
        LOG_INF("[%d]:[0x%08x]:",CAM_NEXT_Thread_,this->m_pIspDescript_vir[CAM_NEXT_Thread_].u.cmd);
        //LOG_CQ_VIRTUAL_TABLE(this->m_pIspVirRegAddr_va,i,mIspCQModuleInfo[i].reg_num);
        sstr = "";
        for(unsigned int j=0;j<mIspCQModuleInfo_SpecialCQ[this->m_CQ].reg_num;j++){
            sprintf(_tmp,"0x%08x-",this->m_pIspVirRegAddr_va[(mIspCQModuleInfo_SpecialCQ[this->m_CQ].sw_addr_ofst >>2) + j]);
            sstr += _tmp;
        }
        if (sstr.size() < __MAX_DUMP_STR_LENGTH__) {
            LOG_INF(" %s\n", sstr.c_str());
        }
        else {
            LOG_WRN("str too long %lu, truncate: %d\n",(unsigned long)sstr.size(), __MAX_DUMP_STR_LENGTH__);
            sstr.resize(__MAX_DUMP_STR_LENGTH__);
            LOG_INF(" %s\n", sstr.c_str());
        }
    }

    LOG_INF("-\n");
    return MTRUE;
}

MBOOL ISP_DRV_CAM::getCurObjInfo(ISP_HW_MODULE* p_module,E_ISP_CAM_CQ* p_cq,MUINT32* p_page)
{
    *p_module = this->m_HWModule;
    *p_cq = this->m_CQ;
    *p_page = this->m_pageIdx;
    return MTRUE;
}
/*
 * DumpReg():
 * Dump inner register.
 * outer register addr + REG_SIZE = inner register addr
 */
MBOOL ISP_DRV_CAM::DumpReg(MBOOL bPhy)
{
    char _tmpchr[16] = "\0";
    char _chr[256] = "\0";
    MUINT32 shift=0x0;

    switch(this->m_HWModule){
        case CAM_A:
            shift = 0x10000;
            break;
        case CAM_B:
            shift = 0x20000;
            break;
        case CAM_C:
            shift = 0x30000;
            break;
        default:
            break;
    }

    if(bPhy){
        LOG_ERR("###################\n");
        LOG_ERR("start dump inner phy reg\n");
        for(MUINT32 i=0x0;i<CAM_BASE_RANGE;i+=0x20){
            _chr[0] = '\0';
            sprintf(_chr,"0x%x: ",shift + i);
            for(MUINT32 j=i;j<(i+0x20);j+=0x4){
                if(j >= CAM_BASE_RANGE) {
                    break;
                }
                _tmpchr[0] = '\0';
                sprintf(_tmpchr,"0x%8x - ",this->m_pIspDrvImp->readInnerReg(j));
                strncat(_chr,_tmpchr, strlen(_tmpchr));
            }
            LOG_WRN("%s\n",_chr);
        }
        LOG_ERR("###################\n");
    }
    else{
        LOG_ERR("###################\n");
        LOG_ERR("start dump vir reg\n");

        for(MUINT32 i=0x0;i<CAM_BASE_RANGE_SPECIAL;i+=0x20){
            _chr[0] = '\0';
            sprintf(_chr,"0x%x: ",shift + i);
            for(MUINT32 j=i;j<(i+0x20);j+=0x4){
                if(j >= CAM_BASE_RANGE_SPECIAL)
                    break;
                _tmpchr[0] = '\0';
                sprintf(_tmpchr,"0x%x - ",this->readReg(j,this->m_HWModule));
                strncat(_chr,_tmpchr, strlen(_tmpchr));
            }
            LOG_WRN("%s\n",_chr);
        }
        LOG_ERR("###################\n");
    }
    return MTRUE;
}

MBOOL ISP_DRV_CAM::VirReg_ValidRW(void)
{

    switch(this->m_CQ){
        case ISP_DRV_CQ_THRE0:
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE10:
        case ISP_DRV_CQ_THRE28:
            break;
        default:
            LOG_ERR("can't support this CQ:0x%x\n",this->m_CQ);
            return MFALSE;
            break;
    }

    return MTRUE;
}

MBOOL ISP_DRV_CAM::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    if(this->VirReg_ValidRW() == MFALSE)
        return MFALSE;

    switch(caller){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            legal_range = CAM_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    shift = this->VirReg_OFFSET(caller);

    for(unsigned int i=0; i<Count; i++){
        if(pRegIo[i].Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",pRegIo[i].Addr);
            return MFALSE;
        }
        pRegIo[i].Data = this->m_pIspVirRegAddr_va[(pRegIo[i].Addr + shift)>>2];
        LOG_DBG("%d:addr:0x%x,data:0x%x",i,pRegIo[i].Addr,pRegIo[i].Data);
    }
    return MTRUE;
}


MUINT32 ISP_DRV_CAM::readReg(MUINT32 Addr,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x]Addr:0x%08X",this->m_CQ,this->m_pageIdx,Addr);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return 0;

    if(this->VirReg_ValidRW() == MFALSE)
        return 0;

    switch(caller){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            legal_range = CAM_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    shift = this->VirReg_OFFSET(caller);

    if(Addr >= legal_range){
        LOG_ERR("over range(0x%lx_0x%x)\n",(unsigned long)sizeof(cam_reg_t),Addr);
        return MFALSE;
    }
    LOG_DBG("Data:0x%x",this->m_pIspVirRegAddr_va[(Addr+shift)>>2]);

    return this->m_pIspVirRegAddr_va[(Addr+shift)>>2];
}

MUINT32 ISP_DRV_CAM::readInnerReg(MUINT32 Addr,MINT32 caller)
{
    (void)Addr;
    (void)caller;
    LOG_ERR("ISP_DRV_CAM no support readInnerReg()\n");
    return MFALSE;
}


MBOOL ISP_DRV_CAM::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    if(this->VirReg_ValidRW() == MFALSE)
        return MFALSE;

    switch(caller){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            legal_range = CAM_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    shift = this->VirReg_OFFSET(caller);

    for(unsigned int i=0; i<Count; i++){
        if(pRegIo[i].Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",pRegIo[i].Addr);
            return MFALSE;
        }
        this->m_pIspVirRegAddr_va[(pRegIo[i].Addr+shift)>>2] = pRegIo[i].Data;
        LOG_DBG("%d:addr:0x%x,data:0x%x",i,pRegIo[i].Addr,this->m_pIspVirRegAddr_va[(pRegIo[i].Addr+shift)>>2]);
    }
    return MTRUE;
}

MBOOL ISP_DRV_CAM::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MUINT32 shift;
    MUINT32 legal_range;
    LOG_DBG("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    if(this->VirReg_ValidRW() == MFALSE)
        return MFALSE;

    switch(caller){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            legal_range = CAM_BASE_RANGE_SPECIAL;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }
    shift = this->VirReg_OFFSET(caller);

    if(Addr >= legal_range){
        LOG_ERR("over range(0x%x)\n",Addr);
        return MFALSE;
    }
    this->m_pIspVirRegAddr_va[(Addr+shift)>>2] = Data;
    LOG_DBG("addr:0x%x,data:0x%x\n",Addr,this->m_pIspVirRegAddr_va[(Addr+shift)>>2]);

    return MTRUE;
}


MBOOL ISP_DRV_CAM::flushCmdQ(void)
{
    LOG_DBG("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;


    LOG_DBG("+");//for flush performance tracking

    if(this->m_ispVirRegBufInfo.phyAddr != 0){//CQ allocated method2 have no vir reg allocated.
        if(0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispVirRegBufInfo,MFALSE) ){
            LOG_ERR("virReg flush fail");
            return MFALSE;
        }
    }

    if( 0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispCQDescBufInfo,MFALSE) ){
        LOG_ERR("descriptor flush fail");
        return MFALSE;
    }

    LOG_DBG("-");//for flush performance tracking
    return MTRUE;
}

//support no replace function
MBOOL ISP_DRV_CAM::updateEnqCqRingBuf(CQ_RingBuf_ST *pBuf_ctrl)
{
    MUINT32 i = 0, j = 0;
    MUINT32 offset[RingBuf_MODULE_MAX] = {0};
    MUINT32 offsetModule = 0; // for high speed
    MUINT32 moduleid[RingBuf_MODULE_MAX] = {0};
    MUINT32 cqModuleNum = 4;
    //
    #define RTBC_GET_PA_FROM_VA(va,bva,bpa) ( ( (unsigned long)(va) - (unsigned long)(bva) ) + (unsigned long)(bpa) )

    CQ_RTBC_RING_ST_CAM *pcqrtbcring_va  = (CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_vir;
    CQ_RTBC_RING_ST_CAM *pcqrtbcring_pa  = (CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_phy;

    if ( CAM_READ_BITS(this->getPhyObj(), TG_R1_TG_VF_CON,TG_VFDATA_EN) == 0 ) {
        //
        i = pcqrtbcring_va->dma_ring_size;

        //
        pcqrtbcring_va->rtbc_ring[i].pNext = &pcqrtbcring_va->rtbc_ring[(i>0)?0:i];
        pcqrtbcring_va->rtbc_ring[i].next_pa = \
            (unsigned long)RTBC_GET_PA_FROM_VA(pcqrtbcring_va->rtbc_ring[i].pNext,pcqrtbcring_va,pcqrtbcring_pa);

        //
        switch(this->m_CQ){
            case ISP_DRV_CQ_THRE4:
                moduleid[0] = CAM_DMA_AAO_BA;
                moduleid[1] = CAM_DMA_FH_AAO_;
                moduleid[2] = CAM_DMA_FH_AAO_SPARE_;
                moduleid[3] = CAM_CQ_THRE4_ADDR_;
                cqModuleNum = 4;
                break;
            case ISP_DRV_CQ_THRE5:
                moduleid[0] = CAM_DMA_AFO_BA;
                moduleid[1] = CAM_DMA_FH_AFO_;
                moduleid[2] = CAM_DMA_FH_AFO_SPARE_;
                moduleid[3] = CAM_CQ_THRE5_ADDR_;
                cqModuleNum = 4;
                break;
            case ISP_DRV_CQ_THRE7:
                moduleid[0] = CAM_DMA_PDO_BA;
                moduleid[1] = CAM_DMA_FH_PDO_;
                moduleid[2] = CAM_DMA_FH_PDO_SPARE_;
                moduleid[3] = CAM_CQ_THRE7_ADDR_;
                cqModuleNum = 4;
                break;
            case ISP_DRV_CQ_THRE8:
                moduleid[0] = CAM_DMA_FLKO_BA;
                moduleid[1] = CAM_DMA_FH_FLKO_;
                moduleid[2] = CAM_DMA_FH_FLKO_SPARE_;
                moduleid[3] = CAM_CQ_THRE8_ADDR_;
                cqModuleNum = 4;
                break;
            case ISP_DRV_CQ_THRE12:
                moduleid[0] = CAM_DMA_TSFSO_BA_;
                moduleid[1] = CAM_DMA_FH_TSFSO_;
                moduleid[2] = CAM_DMA_FH_TSFSO_SPARE_;
                moduleid[3] = CAM_CQ_THRE12_ADDR_;
                cqModuleNum = 4;
                break;
            default:
                LOG_ERR("unsuppoted cq:0x%x\n",this->m_CQ);
                return MFALSE;
                break;
        }
        //

        for(j = 0; j < cqModuleNum; j++)
        {
            switch(this->m_HWModule){
                case CAM_A:
                    offset[j] = 0x0;
                    break;
                case CAM_B:
                    offset[j] = CAM_BASE_RANGE * 2;
                    break;
                case CAM_C:
                    offset[j] = CAM_BASE_RANGE * 4;
                    break;
                default:
                    LOG_ERR("unsuppoted module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }
        }

        switch(this->m_HWModule){
            case CAM_A:
                offsetModule = 0x0;
                break;
            case CAM_B:
                offsetModule = CAM_BASE_RANGE * 2;
                break;
            case CAM_C:
                offsetModule = CAM_BASE_RANGE * 4;
                break;
            default:
                LOG_ERR("unsuppoted module:0x%x\n",this->m_HWModule);
                return MFALSE;
                break;
        }
        //dmao ba
        //write only 1 base reg.  no ba_offset_reg

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma.descriptor = \
            DESCRIPTOR_TOKEN(mIspCQModuleInfo[moduleid[0]].addr_ofst,mIspCQModuleInfo[moduleid[0]].reg_num,offset[0]);


        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma.data = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_base_pa,pcqrtbcring_va,pcqrtbcring_pa);

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_base_pa = pBuf_ctrl->dma_PA;


        //dmao fh ba
        //write only 1 base reg.  no ba_offset_reg
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_fh.descriptor = \
            DESCRIPTOR_TOKEN(mIspCQModuleInfo[moduleid[1]].addr_ofst,mIspCQModuleInfo[moduleid[1]].reg_num,offset[1]);

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_fh.data = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_fh_base_pa,pcqrtbcring_va,pcqrtbcring_pa);

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_fh_base_pa = pBuf_ctrl->dma_FH_PA;

        //fh spare reg
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.fh_spare.descriptor = \
            DESCRIPTOR_TOKEN(mIspCQModuleInfo[moduleid[2]].addr_ofst,mIspCQModuleInfo[moduleid[2]].reg_num,offset[2]);

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.fh_spare.data = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_fh_spare[0],pcqrtbcring_va,pcqrtbcring_pa);


        //next thread addr
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr.descriptor = \
            DESCRIPTOR_TOKEN(mIspCQModuleInfo[moduleid[3]].addr_ofst,mIspCQModuleInfo[moduleid[3]].reg_num,offset[3]);

        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr.data = \
            (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].next_pa,pcqrtbcring_va,pcqrtbcring_pa);

        if(pBuf_ctrl->ctrl == BUF_CTRL_ENQUE_HIGH_SPEED){
            MUINT32 offsetModule_setclr;
            /*
             * cam_a set/clr : 0x1a05a000/0x1a05b000
             * cam_b set/clr : 0x1a05c000/0x1a05d000
             * cam_c set/clr : 0x1a05e000/0x1a05f000
             */
            switch(this->m_HWModule){
            case CAM_A:
                offsetModule_setclr = 0x0;
                break;
            case CAM_B:
                offsetModule_setclr = CAM_BASE_RANGE_SETCLR * 2;
                break;
            case CAM_C:
                offsetModule_setclr = CAM_BASE_RANGE_SETCLR * 4;
                break;
            default:
                LOG_ERR("unsupported hw module:0x%x\n",this->m_HWModule);
                return MFALSE;
            }
            //next thread addr
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.descriptor = \
                DESCRIPTOR_TOKEN(mIspCQModuleInfo_SpecialCQ[this->m_CQ].addr_ofst,mIspCQModuleInfo_SpecialCQ[this->m_CQ].reg_num,offsetModule_setclr);

            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.data = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].next_pa,pcqrtbcring_va,pcqrtbcring_pa);
        }
        else{
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.descriptor = \
                ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule);
#if (CQ_DMA_SW_WORKAROUND == 1)
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.data = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].next_pa,pcqrtbcring_va,pcqrtbcring_pa);
#endif
        }
        //end
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.end.descriptor = ISP_DRV_CQ_END_TOKEN;
        pcqrtbcring_va->rtbc_ring[i].cq_rtbc.end.data = 0;
        //
        pcqrtbcring_va->dma_ring_size++;
        //
        if (i>0) {

            pcqrtbcring_va->rtbc_ring[i-1].pNext = &pcqrtbcring_va->rtbc_ring[i];
            pcqrtbcring_va->rtbc_ring[i-1].next_pa = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i],pcqrtbcring_va,pcqrtbcring_pa);
            //
            pcqrtbcring_va->rtbc_ring[i-1].cq_rtbc.next_Thread_addr.data = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i-1].next_pa,pcqrtbcring_va,pcqrtbcring_pa);
        }

    }
    else{
        LOG_ERR("logic error, should not update cq ring when VF is ON\n");
        return MFALSE;
    }
    return MTRUE;
}


MUINT32* ISP_DRV_CAM::update_FH_Spare(CQ_RingBuf_ST *pBuf_ctrl)
{
    CQ_RTBC_RING_ST_CAM *pcqrtbcring_va  = (CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_vir;
    unsigned int i;
    if(pcqrtbcring_va == NULL){
        LOG_ERR("va is null:0x%p\n",pcqrtbcring_va);
        return NULL;
    }

    if(pcqrtbcring_va->dma_ring_size > MAX_RING_SIZE){
        LOG_ERR("ring size out of range:0x%x\n",pcqrtbcring_va->dma_ring_size);
        return NULL;
    }
    for(i=0;i<pcqrtbcring_va->dma_ring_size;i++){
        if(pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_base_pa == pBuf_ctrl->dma_PA)
            break;
    }

    if(i == pcqrtbcring_va->dma_ring_size){
        char _str[128];
        char _tmp[16];
        _tmp[0] = _str[0] = '\0';
        LOG_ERR("find no match pa:0x%x\n",pBuf_ctrl->dma_PA);
        LOG_ERR("current PA in CQ:\n");
        for(i=0;i<pcqrtbcring_va->dma_ring_size;i++){
            sprintf(_tmp,"0x%08x-",pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_base_pa);
            strncat(_str,_tmp, strlen(_tmp));
        }
        LOG_ERR("%s\n",_str);
        return NULL;
    }

    return pcqrtbcring_va->rtbc_ring[i].cq_rtbc.dma_fh_spare;
}

MUINT32 ISP_DRV_CAM::cqRingBuf(CQ_RingBuf_ST *pBuf_ctrl)
{
    LOG_DBG("cqRingBuf[0x%x_0x%x]:pa(0x%x/0x%x),ctrl(%d)\n", \
            this->m_CQ, \
            this->m_pageIdx, \
            pBuf_ctrl->dma_PA, \
            pBuf_ctrl->dma_FH_PA, \
            pBuf_ctrl->ctrl);

    //
    switch( pBuf_ctrl->ctrl ) {
        //
        case BUF_CTRL_ENQUE:
        case BUF_CTRL_ENQUE_HIGH_SPEED:
            if(this->updateEnqCqRingBuf(pBuf_ctrl) == MFALSE)
                return MFALSE;
            //
            break;
        case BUF_CTRL_CLEAR:
            memset((MUINT8*)this->m_pIspDescript_vir, 0, sizeof(CQ_RTBC_RING_ST_CAM));
            break;
        case SET_FH_SPARE:
            if( (pBuf_ctrl->pDma_fh_spare = this->update_FH_Spare(pBuf_ctrl)) == NULL)
                return MFALSE;
            break;
        case GET_RING_DEPTH:
            return ((CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_vir)->dma_ring_size;
            break;
        default:
            LOG_ERR("ERROR:ctrl id(%d)\n",pBuf_ctrl->ctrl);
            return MFALSE;
            break;
    }

    return MTRUE;
}

//this api support only cam.  (uni is not supported by this control path)
MBOOL ISP_DRV_CAM::CQ_SetContent(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count)
{
    MUINT32 offset;
    MUINTPTR dummyaddr;
    switch(this->m_CQ){
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE10:
        case ISP_DRV_CQ_THRE28:
            break;
        default:
            LOG_ERR("this operation is supported only in thread_%d\n",this->m_CQ);
            return MFALSE;
            break;
    }

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return 0;

    if(Count >= CAM_CAM_MODULE_MAX){
        LOG_ERR("over max reg number:0x%x_0x%x\n",Count,CAM_CAM_MODULE_MAX);
        return MFALSE;
    }

    switch(this->m_HWModule){
        case CAM_A:
            offset = 0x10000;
            break;
        case CAM_B:
            //cam_b hw start at 0x20000
            offset = 0x10000 + CAM_BASE_RANGE * 2;
            break;
        case CAM_C:
            //cam_c hw start at 0x30000
            offset = 0x10000 + CAM_BASE_RANGE * 4;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWModule);
            return MFALSE;
            break;
    }

    for(MUINT32 i=0;i<Count;i++){
        //descritpor, each register cfg is mapping to 1 descriptor.
        dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + pRegIo[i].Addr);
        dummyaddr += this->VirReg_OFFSET(this->m_HWModule);


        this->m_pIspDescript_vir[i].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
        this->m_pIspDescript_vir[i].u.cmd =  DESCRIPTOR_TOKEN(pRegIo[i].Addr,1,offset);

        this->writeRegs(pRegIo,Count,this->m_HWModule);
    }

    //clr remainded descriptor
    for(MUINT32 i = Count;i<CAM_CAM_END_;i++){
        this->m_pIspDescript_vir[i].u.cmd = ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule);

    }
    return MTRUE;
}

MBOOL ISP_DRV_CAM::FSM_CHK(MUINT32 op,const char* caller)
{
    MBOOL rst = MTRUE;
    switch(op){
        case OP_IspDrv_init:
            switch(this->m_FSM){
                case IspDrv_Create:
                case IspDrv_Init:
                    break;
                default:
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
            break;
        case OP_IspDrv_start:
            switch(this->m_FSM){
                case IspDrv_Init:
                case IspDrv_Stop:
                    break;
                default:
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
            break;
        case OP_IspDrv_stop:
            switch(this->m_FSM){
                case IspDrv_Start:
                    break;
                default:
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
            break;
        case OP_IspDrv_Uninit:
            switch(this->m_FSM){
                case IspDrv_Stop:
                case IspDrv_Init:
                case IspDrv_Start:
                    return MTRUE;
                    break;
                default:
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
            break;
        case OP_IspDrv_Destroy:
            switch(this->m_FSM){
                case IspDrv_Uninit:
                case IspDrv_Create:
                    break;
                default:
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
            break;
        case OP_IspDrv_sig:
            switch(this->m_FSM){
                case IspDrv_UNKNONW:
                case IspDrv_Create:
                case IspDrv_Uninit:
                    rst = MFALSE;
                    goto EXIT;
                    break;
                default:
                    break;
            }
            break;
        case OP_IspDrv_sig_sig:
            switch(this->m_FSM){
                case IspDrv_Create:
                case IspDrv_Uninit:
                    rst = MFALSE;
                    goto EXIT;
                    break;
                default:
                    break;
            }
            break;
        case OP_IspDrv_CQ:
        case OP_IspDrv_Device:
            switch(this->m_FSM){
                case IspDrv_Init:
                case IspDrv_Start:
                case IspDrv_Stop:
                    break;
                default:
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
            break;
        default:
            LOG_ERR("unsupport Operation:0x%x,%s",op,caller);
            rst = MFALSE;
            break;
    }
EXIT:
    if(rst == MFALSE){
        LOG_ERR("FSM error: op:0x%x, cur status:0x%x,%s",op,this->m_FSM,caller);
    }
    return rst;
}

MUINT32*  ISP_DRV_CAM::getHwRegAddr(void)
{
        LOG_ERR("ISP_DRV_CAM no support getHwRegAddr()\n");
        return NULL;
}

MBOOL
ISP_DRV_CAM::updateSVModule(
    E_CAM_MODULE moduleId,
    vector<MUINT32> reg_values,
    MUINT32 CamsvDCIFsubsample,
    ISP_HW_MODULE camsvHwModuleId)
{
    int cmd;
    MBOOL ret = MTRUE;
    MUINT32 offset=0x0;
    MUINTPTR dummyaddr;

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE) {
        LOG_ERR("updateSVModule fail\n");
        return MFALSE;
    }

    switch(camsvHwModuleId) {
      case CAMSV_0:
          break;                             //CAMSV_0 hw start at 0x1A051000
      case CAMSV_1:
              offset = CAMSV_BASE_RANGE * 1; //CAMSV_1 hw start at 0x1A052000
          break;
      case CAMSV_2:
              offset = CAMSV_BASE_RANGE * 2; //CAMSV_2 hw start at 0x1A053000
          break;
      case CAMSV_3:
              offset = CAMSV_BASE_RANGE * 3; //CAMSV_3 hw start at 0x1A054000
          break;
      case CAMSV_4:
              offset = CAMSV_BASE_RANGE * 4; //CAMSV_4 hw start at 0x1A055000
          break;
      case CAMSV_5:
              offset = CAMSV_BASE_RANGE * 5; //CAMSV_5 hw start at 0x1A056000
          break;
      case CAMSV_6:
              offset = CAMSV_BASE_RANGE * 6; //CAMSV_6 hw start at 0x1A057000
          break;
      case CAMSV_7:
              offset = CAMSV_BASE_RANGE * 7; //CAMSV_7 hw start at 0x1A058000
          break;
      default:
          LOG_ERR("unsupported HwModule:0x%x camsvHwModule:0x%x \n",
                  this->m_HWModule, camsvHwModuleId);
          return MFALSE;
    }

    // DBG purpose
    switch(moduleId) {
        case SV_FBC_RCNT_INC_0:
            this->m_currentDCIFsumsampleRecode |= (1L << 0);
            break;
        case SV_FBC_RCNT_INC_1:
            this->m_currentDCIFsumsampleRecode |= (1L << 1);
            break;
        case SV_FBC_RCNT_INC_2:
            this->m_currentDCIFsumsampleRecode |= (1L << 2);
            break;
        case SV_FBC_RCNT_INC_3:
            this->m_currentDCIFsumsampleRecode |= (1L << 3);
            break;
        case SV_FBC_RCNT_INC_4:
            this->m_currentDCIFsumsampleRecode |= (1L << 4);
            break;
        case SV_FBC_RCNT_INC_5:
            this->m_currentDCIFsumsampleRecode |= (1L << 5);
            break;
        case SV_FBC_RCNT_INC_6:
            this->m_currentDCIFsumsampleRecode |= (1L << 6);
            break;
        case SV_FBC_RCNT_INC_7:
            this->m_currentDCIFsumsampleRecode |= (1L << 7);
            break;
        default:
            break;
    }

    // DBG purpose
    switch(moduleId) {
        case SV_FBC_RCNT_INC_0:
        case SV_FBC_RCNT_INC_1:
        case SV_FBC_RCNT_INC_2:
        case SV_FBC_RCNT_INC_3:
        case SV_FBC_RCNT_INC_4:
        case SV_FBC_RCNT_INC_5:
        case SV_FBC_RCNT_INC_6:
        case SV_FBC_RCNT_INC_7:
            LOG_DBG("HWModule:0x%x "
                    "camsvHwModuleId:0x%x "
                    "SV_FBC_RCNT_INC_%d "
                    "m_currentDCIFsumsampleRecode(0x%x) "
                    "CamsvDCIFsubsample(0x%x) ",
                    this->m_HWModule,
                    camsvHwModuleId,
                    (moduleId - SV_FBC_RCNT_INC_0),
                    this->m_currentDCIFsumsampleRecode,
                    CamsvDCIFsubsample);

            if(this->m_currentDCIFsumsampleRecode ==
              ((1L << CamsvDCIFsubsample) - 1)) {
                this->m_currentDCIFsumsampleRecode = 0;

                LOG_DBG("HWModule:0x%x camsvHwModuleId:0x%x "
                        "Set currentDCIFsumsampleRecode = 0\n",
                        this->m_HWModule, camsvHwModuleId);
            }
            break;
        default:
            break;
    }

    switch(moduleId) {
        case SV_DMA_IMGO_BA_A_0:
        case SV_DMA_IMGO_BA_A_1:
        case SV_DMA_IMGO_BA_A_2:
        case SV_DMA_IMGO_BA_A_3:
        case SV_DMA_IMGO_BA_A_4:
        case SV_DMA_IMGO_BA_A_5:
        case SV_DMA_IMGO_BA_A_6:
        case SV_DMA_IMGO_BA_A_7:
        case SV_DMA_FH_IMGO_A_0:
        case SV_DMA_FH_IMGO_A_1:
        case SV_DMA_FH_IMGO_A_2:
        case SV_DMA_FH_IMGO_A_3:
        case SV_DMA_FH_IMGO_A_4:
        case SV_DMA_FH_IMGO_A_5:
        case SV_DMA_FH_IMGO_A_6:
        case SV_DMA_FH_IMGO_A_7:
        case SV_DMA_FH_IMGO_SPARE_A_0:
        case SV_DMA_FH_IMGO_SPARE_A_1:
        case SV_DMA_FH_IMGO_SPARE_A_2:
        case SV_DMA_FH_IMGO_SPARE_A_3:
        case SV_DMA_FH_IMGO_SPARE_A_4:
        case SV_DMA_FH_IMGO_SPARE_A_5:
        case SV_DMA_FH_IMGO_SPARE_A_6:
        case SV_DMA_FH_IMGO_SPARE_A_7:
        case SV_DMA_UFEO_BA_A_0:
        case SV_DMA_UFEO_BA_A_1:
        case SV_DMA_UFEO_BA_A_2:
        case SV_DMA_UFEO_BA_A_3:
        case SV_DMA_UFEO_BA_A_4:
        case SV_DMA_UFEO_BA_A_5:
        case SV_DMA_UFEO_BA_A_6:
        case SV_DMA_UFEO_BA_A_7:
        case SV_DMA_FH_UFEO_A_0:
        case SV_DMA_FH_UFEO_A_1:
        case SV_DMA_FH_UFEO_A_2:
        case SV_DMA_FH_UFEO_A_3:
        case SV_DMA_FH_UFEO_A_4:
        case SV_DMA_FH_UFEO_A_5:
        case SV_DMA_FH_UFEO_A_6:
        case SV_DMA_FH_UFEO_A_7:
        case SV_DMA_FH_UFEO_SPARE_A_0:
        case SV_DMA_FH_UFEO_SPARE_A_1:
        case SV_DMA_FH_UFEO_SPARE_A_2:
        case SV_DMA_FH_UFEO_SPARE_A_3:
        case SV_DMA_FH_UFEO_SPARE_A_4:
        case SV_DMA_FH_UFEO_SPARE_A_5:
        case SV_DMA_FH_UFEO_SPARE_A_6:
        case SV_DMA_FH_UFEO_SPARE_A_7:
        case SV_DMA_IMGO_BA_B_0:
        case SV_DMA_IMGO_BA_B_1:
        case SV_DMA_IMGO_BA_B_2:
        case SV_DMA_IMGO_BA_B_3:
        case SV_DMA_IMGO_BA_B_4:
        case SV_DMA_IMGO_BA_B_5:
        case SV_DMA_IMGO_BA_B_6:
        case SV_DMA_IMGO_BA_B_7:
        case SV_DMA_FH_IMGO_B_0:
        case SV_DMA_FH_IMGO_B_1:
        case SV_DMA_FH_IMGO_B_2:
        case SV_DMA_FH_IMGO_B_3:
        case SV_DMA_FH_IMGO_B_4:
        case SV_DMA_FH_IMGO_B_5:
        case SV_DMA_FH_IMGO_B_6:
        case SV_DMA_FH_IMGO_B_7:
        case SV_DMA_FH_IMGO_SPARE_B_0:
        case SV_DMA_FH_IMGO_SPARE_B_1:
        case SV_DMA_FH_IMGO_SPARE_B_2:
        case SV_DMA_FH_IMGO_SPARE_B_3:
        case SV_DMA_FH_IMGO_SPARE_B_4:
        case SV_DMA_FH_IMGO_SPARE_B_5:
        case SV_DMA_FH_IMGO_SPARE_B_6:
        case SV_DMA_FH_IMGO_SPARE_B_7:
        case SV_DMA_UFEO_BA_B_0:
        case SV_DMA_UFEO_BA_B_1:
        case SV_DMA_UFEO_BA_B_2:
        case SV_DMA_UFEO_BA_B_3:
        case SV_DMA_UFEO_BA_B_4:
        case SV_DMA_UFEO_BA_B_5:
        case SV_DMA_UFEO_BA_B_6:
        case SV_DMA_UFEO_BA_B_7:
        case SV_DMA_FH_UFEO_B_0:
        case SV_DMA_FH_UFEO_B_1:
        case SV_DMA_FH_UFEO_B_2:
        case SV_DMA_FH_UFEO_B_3:
        case SV_DMA_FH_UFEO_B_4:
        case SV_DMA_FH_UFEO_B_5:
        case SV_DMA_FH_UFEO_B_6:
        case SV_DMA_FH_UFEO_B_7:
        case SV_DMA_FH_UFEO_SPARE_B_0:
        case SV_DMA_FH_UFEO_SPARE_B_1:
        case SV_DMA_FH_UFEO_SPARE_B_2:
        case SV_DMA_FH_UFEO_SPARE_B_3:
        case SV_DMA_FH_UFEO_SPARE_B_4:
        case SV_DMA_FH_UFEO_SPARE_B_5:
        case SV_DMA_FH_UFEO_SPARE_B_6:
        case SV_DMA_FH_UFEO_SPARE_B_7:
        case SV_FBC_RCNT_INC_0:
        case SV_FBC_RCNT_INC_1:
        case SV_FBC_RCNT_INC_2:
        case SV_FBC_RCNT_INC_3:
        case SV_FBC_RCNT_INC_4:
        case SV_FBC_RCNT_INC_5:
        case SV_FBC_RCNT_INC_6:
        case SV_FBC_RCNT_INC_7:
        case SV_DMA_FH_IMGO_BASE_A_0:
        case SV_DMA_FH_IMGO_BASE_A_1:
        case SV_DMA_FH_IMGO_BASE_A_2:
        case SV_DMA_FH_IMGO_BASE_A_3:
        case SV_DMA_FH_IMGO_BASE_A_4:
        case SV_DMA_FH_IMGO_BASE_A_5:
        case SV_DMA_FH_IMGO_BASE_A_6:
        case SV_DMA_FH_IMGO_BASE_A_7:
        case SV_DMA_FH_IMGO_BASE_B_0:
        case SV_DMA_FH_IMGO_BASE_B_1:
        case SV_DMA_FH_IMGO_BASE_B_2:
        case SV_DMA_FH_IMGO_BASE_B_3:
        case SV_DMA_FH_IMGO_BASE_B_4:
        case SV_DMA_FH_IMGO_BASE_B_5:
        case SV_DMA_FH_IMGO_BASE_B_6:
        case SV_DMA_FH_IMGO_BASE_B_7:
        case SV_FBC_RCNT_INC_B_0:
        case SV_FBC_RCNT_INC_B_1:
        case SV_FBC_RCNT_INC_B_2:
        case SV_FBC_RCNT_INC_B_3:
        case SV_FBC_RCNT_INC_B_4:
        case SV_FBC_RCNT_INC_B_5:
        case SV_FBC_RCNT_INC_B_6:
        case SV_FBC_RCNT_INC_B_7:
        {

            if(mIspCQModuleInfo[moduleId].reg_num != reg_values.size()) {
                    LOG_ERR("reg_values input size error!!! \n");
                    return MFALSE;
            }

            vector<MUINT32>::iterator it_reg_values;
            MUINT32 inx = 0;
            for(it_reg_values = reg_values.begin(), inx = 0;
                it_reg_values < reg_values.end();
                ++inx,++it_reg_values) {

                this->m_pIspVirRegAddr_va[(mIspCQModuleInfo[moduleId].sw_addr_ofst >> 2) + inx ] =
                    *it_reg_values;
            }

            dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa +
                                    mIspCQModuleInfo[moduleId].sw_addr_ofst);
            //
            cmd = DESCRIPTOR_TOKEN((mIspCQModuleInfo[moduleId].addr_ofst),mIspCQModuleInfo[moduleId].reg_num,offset);

            this->m_pIspDescript_vir[moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer
            this->m_pIspDescript_vir[moduleId].u.cmd = cmd;

            ret = MTRUE;
            break;
        }
        default:
            LOG_ERR("updateSVModule no support this moduleId:0x%x "
                    "HWModule:0x%x camsvHwModuleId:0x%x\n ",
                    moduleId, this->m_HWModule, camsvHwModuleId);
            return MFALSE;
    }



    return ret;
}


MBOOL ISP_DRV_CAM::updateSVModule_120fps(
    E_CAM_MODULE moduleId,
    vector<MUINT32> reg_values,
    ISP_HW_MODULE camsvHwModuleId)
{
    int cmd;
    MBOOL ret = MTRUE;
    MUINT32 offset = 0x0;
    MUINTPTR dummyaddr;

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE) {
        LOG_ERR("updateSVModule fail\n");
        return MFALSE;
    }

    switch(moduleId){
        case SV_MODULE_EN_A_0:
        case SV_MODULE_EN_A_1:
        case SV_MODULE_EN_B_0:
        case SV_MODULE_EN_B_1:
        case SV_TG_SUB_PERIOD_A_0:
        case SV_TG_SUB_PERIOD_A_1:
        case SV_TG_SUB_PERIOD_B_0:
        case SV_TG_SUB_PERIOD_B_1:
        case SV_SPARE_0_A_0:
        case SV_SPARE_0_A_1:
        case SV_SPARE_0_B_0:
        case SV_SPARE_0_B_1:
        case SV_SPARE_1_A_0:
        case SV_SPARE_1_A_1:
        case SV_SPARE_1_B_0:
        case SV_SPARE_1_B_1:
        case SV_120FPS_CAMA_CHANGE:
        case SV_120FPS_CAMB_CHANGE:
        {
            if(mIspCQModuleInfo[moduleId].reg_num != reg_values.size()) {
                    LOG_ERR("reg_values input size error!!! \n");
                    return MFALSE;
            }

            vector<MUINT32>::iterator it_reg_values;
            MUINT32 inx = 0;

            for(it_reg_values = reg_values.begin(), inx = 0; it_reg_values < reg_values.end(); ++inx,++it_reg_values) {

                this->m_pIspVirRegAddr_va[(mIspCQModuleInfo[moduleId].sw_addr_ofst >> 2) + inx ] = *it_reg_values;
            }

            dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspCQModuleInfo[moduleId].sw_addr_ofst);
            this->m_pIspDescript_vir[moduleId].v_reg_addr = (MUINT32)dummyaddr & 0xFFFFFFFF; // >>2 for MUINT32* pointer

            cmd = DESCRIPTOR_TOKEN((mIspCQModuleInfo[moduleId].addr_ofst),
                                    mIspCQModuleInfo[moduleId].reg_num,
                                    offset);

            /* #define DESCRIPTOR_TOKEN(reg_oft, reg_num, dram_oft)({
                MUINT32 tmp;
                tmp = ((reg_oft + dram_oft)&0xffff) | (((reg_num-1)&0x3ff)<<16) | ((ISP_CQ_APB_INST)<<26) | (((reg_oft + dram_oft)&0x70000)<<(29-16));
                tmp;
            }) */

            this->m_pIspDescript_vir[moduleId].u.cmd = cmd;
            ret = MTRUE;
            break;
        }
        default:
            LOG_ERR("updateSVModule_120fps no support this moduleId:0x%x" "HWModule:0x%x camsvHwModuleId:0x%x\n", moduleId, this->m_HWModule, camsvHwModuleId);
            return MFALSE;
    }
    return ret;
}


//For secure camera
MBOOL ISP_DRV_CAM::setDapcReg(vector<MUINT32> reg_list,ISP_HW_MODULE module)
{
    MUINT32 init_value = 0x0;

    if (this->m_DapcReg[module].size() == 0){
        m_DapcIdx = reg_list;
        this->m_SecOn = MTRUE;

        LOG_DBG("m_SecOn:0x%x",this->m_SecOn);
    }else{
        this->m_DapcReg[module].clear();
    }

    for(MUINT32 i=0;i<m_DapcIdx.size();i++){
        this->m_DapcReg[module].push_back(init_value);
        LOG_DBG("Addr:0x%x data:0x%x",m_DapcIdx.at(i),this->m_DapcReg[module].at(i));
    }

    return MTRUE;
}

vector<MUINT32> ISP_DRV_CAM::getDapcReg(ISP_HW_MODULE module)
{
    LOG_DBG("m_DapcReg size:%lu",this->m_DapcReg[module].size());

    for(MUINT32 i=0;i<m_DapcIdx.size();i++){
        this->m_DapcReg[module].at(i) = this->readReg(m_DapcIdx.at(i),module);
        LOG_DBG("Addr:0x%x data:0x%x",m_DapcIdx.at(i),this->m_DapcReg[module].at(i));
    }

    return this->m_DapcReg[module];
}
