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
#define ISP_CQ_APB_INST             0x0
#define ISP_CQ_NOP_INST             0x1
#define ISP_DRV_CQ_END_TOKEN        0x1C000000
#define ISP_CQ_INST_BIT             (26)//bit26 of cmq cmd is INST, it will be ISP_CQ_APB_INST or ISP_CQ_NOP_INST

#define ISP_DRV_CQ_DUMMY_TOKEN(Module)   ((ISP_CQ_NOP_INST<<ISP_CQ_INST_BIT)|((CQ_DUMMY_REG + Module*CAM_BASE_RANGE) & CQ_DES_RANGE))
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
    if(x ==ISP_DRV_CQ_THRE0)      _cqidx = _dma_cq0i_;\
    else if(x ==ISP_DRV_CQ_THRE1) _cqidx = _dma_cq1i_;\
    else if(x ==ISP_DRV_CQ_THRE2) _cqidx = _dma_cq2i_;\
    else if(x ==ISP_DRV_CQ_THRE3) _cqidx = _dma_cq3i_;\
    else if(x ==ISP_DRV_CQ_THRE4) _cqidx = _dma_cq4i_;\
    else if(x ==ISP_DRV_CQ_THRE5) _cqidx = _dma_cq5i_;\
    else if(x ==ISP_DRV_CQ_THRE6) _cqidx = _dma_cq6i_;\
    else if(x ==ISP_DRV_CQ_THRE7) _cqidx = _dma_cq7i_;\
    else if(x ==ISP_DRV_CQ_THRE8) _cqidx = _dma_cq8i_;\
    else if(x ==ISP_DRV_CQ_THRE9) _cqidx = _dma_cq9i_;\
    else if(x ==ISP_DRV_CQ_THRE10) _cqidx = _dma_cq10i_;\
    else if(x ==ISP_DRV_CQ_THRE11) _cqidx = _dma_cq11i_;\
    else if(x ==ISP_DRV_CQ_THRE12) _cqidx = _dma_cq12i_;\
    else\
        _cqidx = _dma_max_wr_;\
    _cqidx;\
})

ISP_DRV_CQ_MODULE_INFO_STRUCT mIspCQModuleInfo[CAM_CAM_MODULE_MAX]    =
{
    {TWIN_CQ0_BASEADDRESS,           0x0198, 0x2034,  1   }, //this is for twin's master cam use only , use master cam to program salve cam's cq baseaddress
    {TWIN_CQ1_BASEADDRESS,           0x01A4, 0x2038,  1   }, //this is for twin's master cam use only , use master cam to program salve cam's cq baseaddress
    {TWIN_CQ10_BASEADDRESS,         0x0210, 0x203C,  1   }, //this is for twin's master cam use only , use master cam to program salve cam's cq baseaddress
#if TWIN_CQ_SW_WORKAROUND
    {CAM_CQ0_EN_,                   0x0194, 0x0194,  1   }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ0_TRIG_,                 0x0000, 0x0000,  1   }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ1_EN_,                   0x01A0, 0x01A0,  1   }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ1_TRIG_,                 0x0000, 0x0000,  1   }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ10_EN_,                   0x020C, 0x020C,  1   }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    {CAM_CQ10_TRIG_,                 0x0000, 0x0000,  1   }, //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
#endif
    {CAM_CTL_EN_,                   0x4004, 0x0004,  2   }, // CAM_CTL_EN
    {CAM_CTL_FMT_,                  0x400C, 0x000C,  1   }, // CAM_CTL_FMT_SEL
    {CAM_CTL_SEL_,                  0x4010, 0x0010,  1   }, // CAM_CTL_SEL
    {CAM_CTL_MISC_,                 0x4014, 0x0014,  1   }, // CAM_CTL_MISC, for dtwin
    {CAM_CTL_EN2_,                  0x4018, 0x0018,  1   }, // CAM_CTL_EN2
    {CAM_CTL_INT_EN_,               0x4020, 0x0020,  1   }, // CAM_CTL_RAW_INT_EN
    {CAM_CTL_INT2_EN_,              0x4030, 0x0030,  1   }, // CAM_CTL_RAW_INT2_EN
    {CAM_CTL_DONE_SEL_,             0x4044, 0x0044,  3   }, // CAM_CTL_AB_DONE_SEL
    {CAM_TWIN_INFO,                 0x4050, 0x0050,  2   }, // CAM_CTL_TWIN_STATUS
    {CAM_CQ_COUNTER,                0x4058, 0x0058,  1   }, // CAM_CTL_SPARE2
    {CAM_CTL_SW_DONE_SEL_,          0x405C, 0x005C,  1   }, // CAM_CTL_SW_PASS1_DONE
    {CAM_CTL_INT3_EN_,              0x40C0, 0x00C0,  1   }, // CAM_CTL_RAW_INT3_EN
    {CAM_FBC_IMGO_,                 0X4110, 0X0110,  1   }, // CAM_FBC_IMGO_CTL1
    {CAM_FBC_RRZO_,                 0X4118, 0X0118,  1   }, // CAM_FBC_RRZO_CTL1
    {CAM_FBC_UFEO_,                 0X4120, 0X0120,  1   }, // CAM_FBC_UFEO_CTL1
    {CAM_FBC_LCSO_,                 0X4128, 0X0128,  1   }, // CAM_FBC_LCSO_CTL1
    {CAM_FBC_AFO_,                  0x4130, 0x0130,  1   }, // CAM_FBC_AFO_CTL1
    {CAM_FBC_AAO_,                  0X4138, 0X0138,  1   }, // CAM_FBC_AAO_CTL1
    {CAM_FBC_PDO_,                  0X4140, 0X0140,  1   }, // CAM_FBC_PDO_CTL1
    {CAM_FBC_PSO_,                  0X4148, 0X0148,  1   }, // CAM_FBC_PSO_CTL1
    {CAM_FBC_FLKO_,                 0x4150, 0x0150,  1   }, // CAM_FBC_FLKO_CTL1
    {CAM_FBC_LMVO_,                 0x4158, 0x0158,  1   }, // CAM_FBC_LMVO_CTL1
    {CAM_FBC_RSSO_,                 0x4160, 0x0160,  1   }, // CAM_FBC_RSSO_CTL1
    {CAM_FBC_UFGO_,                 0x4168, 0x0168,  1   }, // CAM_FBC_UFGO_CTL1
    {CAM_CTL_DMA_V_FLIP_,           0x500C, 0x100C,  1   }, // CAM_VERTICAL_FLIP_EN
    {CAM_CTL_SPECIAL_FUN_,          0x5018, 0x1018,  1   }, // CAM_SPECIAL_FUN_EN
    {CAM_DMA_IMGO_BA,               0x5020, 0x1020,  1   }, // CAM_IMGO_BASE_ADDR
    {CAM_DMA_IMGO_BA_OFST,          0x5028, 0x1028,  1   }, // CAM_IMGO_OFST_ADDR
    {CAM_DMA_IMGO_,                 0x5030, 0x1030,  3   }, // CAM_IMGO_XSIZE
    {CAM_DMA_IMGO_CROP_,            0x5048, 0x1048,  1   }, // CAM_IMGO_CROP
    {CAM_DMA_RRZO_BA,               0x5050, 0x1050,  1   }, // CAM_RRZO_BASE_ADDR
    {CAM_DMA_RRZO_BA_OFST,          0x5058, 0x1058,  1   }, // CAM_RRZO_OFST_ADDR
    {CAM_DMA_RRZO_,                 0x5060, 0x1060,  3   }, // CAM_RRZO_XSIZE
    {CAM_DMA_RRZO_CROP_,            0x5078, 0x1078,  1   }, // CAM_RRZO_CROP
    {CAM_DMA_AAO_BA,                0x5080, 0x1080,  1   }, // CAM_AAO_BASE_ADDR
    {CAM_DMA_AAO_,                  0x5090, 0x1090,  3   }, // CAM_AAO_XSIZE
    {CAM_DMA_AFO_BA,                0x50B0, 0x10B0,  1   }, // CAM_AFO_BASE_ADDR
    {CAM_DMA_AFO_BA_OFST,           0x50B8, 0x10B8,  1   }, // CAM_AFO_OFST_ADDR
    {CAM_DMA_AFO_,                  0x50C0, 0x10C0,  3   }, // CAM_AFO_XSIZE
    {CAM_DMA_LCSO_BA,               0x50E0, 0x10E0,  1   }, // CAM_LCSO_BASE_ADDR
    {CAM_DMA_LCSO_BA_OFST,          0x50E8, 0x10E8,  1   }, // CAM_LCSO_OFST_ADDR
    {CAM_DMA_LCSO_,                 0x50F0, 0x10F0,  3   }, // CAM_LCSO_XSIZE
    {CAM_DMA_UFEO_BA,               0x5110, 0x1110,  1   }, // CAM_UFEO_BASE_ADDR
    {CAM_DMA_UFEO_BA_OFST,          0x5118, 0x1118,  1   }, // CAM_UFEO_OFST_ADDR
    {CAM_DMA_UFEO_,                 0x5120, 0x1120,  3   }, // CAM_UFEO_XSIZE
    {CAM_DMA_PDO_BA,                0x5140, 0x1140,  1   }, // CAM_PDO_BASE_ADDR
    {CAM_DMA_PDO_BA_OFST,           0x5148, 0x1148,  1   }, // CAM_PDO_OFST_ADDR
    {CAM_DMA_PDO_,                  0x5150, 0x1150,  3   }, // CAM_PDO_XSIZE
    {CAM_DMA_PSO_BA,                0x5260, 0x1260,  1   }, // CAM_PSO_BASE_ADDR
    {CAM_DMA_PSO_BA_OFST,           0x5264, 0x1264,  1   }, // CAM_PSO_OFST_ADDR
    {CAM_DMA_PSO_,                  0x5270, 0x1270,  3   }, // CAM_PSO_XSIZE
    {CAM_DMA_FLKO_BA,               0x52C0, 0x12C0,  1   }, // CAM_FLKO_BASE_ADDR
    {CAM_DMA_FLKO_BA_OFST,          0x52C4, 0x12C4,  1   }, // CAM_FLKO_OFST_ADDR
    {CAM_DMA_FLKO_,                 0x52D0, 0x12D0,  3   }, // CAM_FLKO_XSIZE
    {CAM_DMA_LMVO_BA,               0x5290, 0x1290,  1   }, // CAM_LMVO_BASE_ADDR
    {CAM_DMA_LMVO_BA_OFST,          0x5294, 0x1294,  1   }, // CAM_LMVO_OFST_ADDR
    {CAM_DMA_LMVO_,                 0x52A0, 0x12A0,  3   }, // CAM_LMVO_XSIZE
    {CAM_DMA_RSSO_A_BA,             0x52F0, 0x12F0,  1   }, // CAM_RSSO_A_BASE_ADDR
    {CAM_DMA_RSSO_A_BA_OFST,        0x52F4, 0x12F4,  1   }, // CAM_RSSO_A_OFST_ADDR
    {CAM_DMA_RSSO_A_,               0x5300, 0x1300,  3   }, // CAM_RSSO_A_XSIZE
    {CAM_DMA_UFGO_BA,               0x5320, 0x1320,  1   }, // CAM_UFGO_BASE_ADDR
    {CAM_DMA_UFGO_BA_OFST,          0x5324, 0x1324,  1   }, // CAM_UFGO_OFST_ADDR
    {CAM_DMA_UFGO_,                 0x5330, 0x1330,  3   }, // CAM_UFGO_XSIZE
    {CAM_DMA_BPCI_,                 0x5170, 0x1170,  7   }, // CAM_BPCI_BASE_ADDR
    {CAM_DMA_CACI_,                 0x51A0, 0x11A0,  7   }, // CAM_CACI_BASE_ADDR
    {CAM_DMA_LSCI_,                 0x51D0, 0x11D0,  7   }, // CAM_LSCI_BASE_ADDR
    {CAM_DMA_PDI_,                  0x5230, 0x1230,  7   }, // CAM_PDI_BASE_ADDR
    {CAM_DMA_RSV1_,                 0x53B0, 0x13B0,  1   }, // CAM_DMA_RSV1, for smi mask, dtwin
    {CAM_MAGIC_NUM_,                0x53B8, 0x13B8,  1   }, // CAM_MAGIC_NUM
    {CAM_DATA_PAT_,                 0x53BC, 0x13BC,  1   }, // CAM_DATA_PATTEN
    {CAM_DMA_RSV6_,                 0x53C4, 0x13C4,  1   }, // CAM_DMA_RSV6, for smi mask, dtwin
    {CAM_DMA_FH_EN_,                0x5400, 0x1400,  1   }, // CAM_DMA_FRAME_HEADER_EN
    {CAM_DMA_FH_IMGO_,              0x5404, 0x1404,  1   }, // CAM_IMGO_FH_BASE_ADDR
    {CAM_DMA_FH_IMGO_SPARE_,        0x5434, 0x1434,  11  }, // CAM_IMGO_FH_SPARE_2
    {CAM_DMA_FH_RRZO_,              0x5408, 0x1408,  1   }, // CAM_RRZO_FH_BASE_ADDR
    {CAM_DMA_FH_RRZO_SPARE_,        0x5474, 0x1474,  11  }, // CAM_RRZO_FH_SPARE_2
    {CAM_DMA_FH_AAO_,               0x540C, 0x140C,  1   }, // CAM_AAO_FH_BASE_ADDR
    {CAM_DMA_FH_AAO_SPARE_CQ0_,     0x54B4, 0x14B4,  8   }, // CAM_AAO_FH_SPARE_2
    {CAM_DMA_FH_AAO_SPARE_,         0x54D4, 0x14D4,  3  }, // CAM_AAO_FH_SPARE_10
    {CAM_DMA_FH_AFO_,               0x5410, 0x1410,  1   }, // CAM_AFO_FH_BASE_ADDR
    {CAM_DMA_FH_AFO_SPARE_CQ0_,     0x54F4, 0x14F4,  8   }, // CAM_AFO_FH_SPARE_2
    {CAM_DMA_FH_AFO_SPARE_,         0x5514, 0x1514,  3  }, // CAM_AFO_FH_SPARE_10
    {CAM_DMA_FH_LCSO_,              0x5414, 0x1414,  1   }, // CAM_LCSO_FH_BASE_ADDR
    {CAM_DMA_FH_LCSO_SPARE_,        0x5534, 0x1534,  11  }, // CAM_LCSO_FH_SPARE_2
    {CAM_DMA_FH_UFEO_,              0x5418, 0x1418,  1   }, // CAM_UFEO_FH_BASE_ADDR
    {CAM_DMA_FH_UFEO_SPARE_,        0x5574, 0x1574,  11  }, // CAM_UFEO_FH_SPARE_2
    {CAM_DMA_FH_PDO_,               0x541C, 0x141C,  1   }, // CAM_PDO_FH_BASE_ADDR
    {CAM_DMA_FH_PDO_SPARE_CQ0_,     0x55B4, 0x15B4,  8   }, // CAM_PDO_FH_SPARE_2
    {CAM_DMA_FH_PDO_SPARE_,         0x55D4, 0x15D4,  3   }, // CAM_PDO_FH_SPARE_10
    {CAM_DMA_FH_PSO_,               0x5420, 0x1420,  1   }, // CAM_PSO_FH_BASE_ADDR
    {CAM_DMA_FH_PSO_SPARE_CQ0_,     0x5600, 0x1600,  4   }, // CAM_PSO_FH_SPARE_5
    {CAM_DMA_FH_PSO_SPARE_,         0x5614, 0x1614,  4  }, // CAM_PSO_FH_SPARE_9
    {CAM_DMA_FH_FLKO_,              0x5428, 0x1428,  1   }, // CAM_FLKO_FH_BASE_ADDR
    {CAM_DMA_FH_FLKO_SPARE_CQ0_,    0x5674, 0x1674,  7   }, // CAM_FLKO_FH_SPARE_2
    {CAM_DMA_FH_FLKO_SPARE_,        0x5694, 0x1694,  4  }, // CAM_FLKO_FH_SPARE_9
    {CAM_DMA_FH_LMVO_,              0x5424, 0x1424,  1   }, // CAM_LMVO_FH_BASE_ADDR
    {CAM_DMA_FH_LMVO_SPARE_,        0x5634, 0x1634,  7  }, // CAM_LMVO_FH_SPARE_2
    {CAM_DMA_FH_LMVO_SPARE_2,      0x5654, 0x1654,  4  }, // CAM_LMVO_FH_SPARE_9
    {CAM_DMA_FH_RSSO_A_,            0x542C, 0x142C,  1   }, // CAM_RSSO_A_FH_BASE_ADDR
    {CAM_DMA_FH_RSSO_A_SPARE_,      0x56B4, 0x16B4,  3  }, // CAM_RSSO_A_FH_SPARE_2
    {CAM_DMA_FH_RSSO_A_SPARE_2,    0x56E0, 0x16E0,  4  }, // CAM_RSSO_A_FH_SPARE_5
    {CAM_DMA_FH_RSSO_A_SPARE_3,    0x56F4, 0x16F4,  4  }, // CAM_RSSO_A_FH_SPARE_9
    {CAM_DMA_FH_UFGO_,              0x5430, 0x1430,  1   }, // CAM_UFGO_FH_BASE_ADDR
    {CAM_DMA_FH_UFGO_SPARE_,        0x5714, 0x1714,  11  }, // CAM_UFGO_FH_SPARE_2
    {CAM_CQ_THRE0_ADDR_,            0x4198, 0x0198,  1   }, // CAM_CQ_THR0_BASEADDR
    {CAM_CQ_THRE0_SIZE_,            0x419C, 0x019C,  1   }, // CAM_CQ_THR0_DESC_SIZE
    {CAM_CQ_THRE1_ADDR_,            0x41A4, 0x01A4,  1   }, // CAM_CQ_THR1_BASEADDR
    {CAM_CQ_THRE1_SIZE_,            0x41A8, 0x01A8,  1   }, // CAM_CQ_THR1_DESC_SIZE
    {CAM_CQ_THRE2_ADDR_,            0x41B0, 0x01B0,  1   }, // CAM_CQ_THR2_BASEADDR
    {CAM_CQ_THRE2_SIZE_,            0x41B4, 0x01B4,  1   }, // CAM_CQ_THR2_DESC_SIZE
    {CAM_CQ_THRE3_ADDR_,            0x41BC, 0x01BC,  1   }, // CAM_CQ_THR3_BASEADDR
    {CAM_CQ_THRE3_SIZE_,            0x41C0, 0x01C0,  1   }, // CAM_CQ_THR3_DESC_SIZE
    {CAM_CQ_THRE4_ADDR_,            0x41C8, 0x01C8,  1   }, // CAM_CQ_THR4_BASEADDR
    {CAM_CQ_THRE4_SIZE_,            0x41CC, 0x01CC,  1   }, // CAM_CQ_THR4_DESC_SIZE
    {CAM_CQ_THRE5_ADDR_,            0x41D4, 0x01D4,  1   }, // CAM_CQ_THR5_BASEADDR
    {CAM_CQ_THRE5_SIZE_,            0x41D8, 0x01D8,  1   }, // CAM_CQ_THR5_DESC_SIZE
    {CAM_CQ_THRE6_ADDR_,            0x41E0, 0x01E0,  1   }, // CAM_CQ_THR6_BASEADDR
    {CAM_CQ_THRE6_SIZE_,            0x41E4, 0x01E4,  1   }, // CAM_CQ_THR6_DESC_SIZE
    {CAM_CQ_THRE7_ADDR_,            0x41EC, 0x01EC,  1   }, // CAM_CQ_THR7_BASEADDR
    {CAM_CQ_THRE7_SIZE_,            0x41F0, 0x01F0,  1   }, // CAM_CQ_THR7_DESC_SIZE
    {CAM_CQ_THRE8_ADDR_,            0x41F8, 0x01F8,  1   }, // CAM_CQ_THR8_BASEADDR
    {CAM_CQ_THRE8_SIZE_,            0x41FC, 0x01FC,  1   }, // CAM_CQ_THR8_DESC_SIZE
    {CAM_CQ_THRE9_ADDR_,            0x4204, 0x0204,  1   }, // CAM_CQ_THR9_BASEADDR
    {CAM_CQ_THRE9_SIZE_,            0x4208, 0x0208,  1   }, // CAM_CQ_THR9_DESC_SIZE
    {CAM_CQ_THRE10_ADDR_,           0x4210, 0x0210,  1   }, // CAM_CQ_THR10_BASEADDR
    {CAM_CQ_THRE10_SIZE_,           0x4214, 0x0214,  1   }, // CAM_CQ_THR10_DESC_SIZE
    {CAM_CQ_THRE11_ADDR_,           0x421C, 0x021C,  1   }, // CAM_CQ_THR11_BASEADDR
    {CAM_CQ_THRE11_SIZE_,           0x4220, 0x0220,  1   }, // CAM_CQ_THR11_DESC_SIZE
    {CAM_CQ_THRE12_ADDR_,           0x4228, 0x0228,  1   }, // CAM_CQ_THR12_BASEADDR
    {CAM_CQ_THRE12_SIZE_,           0x422C, 0x022C,  1   }, // CAM_CQ_THR12_DESC_SIZE
    {CAM_ISP_DMX_,                  0x4330, 0x0330,  3   }, // CAM_DMX_CTL
    {CAM_ISP_PBN_,                  0x4AA0, 0x0AA0,  3   }, // CAM_PBN_TYPE
    {CAM_ISP_DBN_,                  0x4A90, 0x0A90,  2   }, // CAM_DBN_GAIN
    {CAM_ISP_BIN_,                  0x4A70, 0x0A70,  3   }, // CAM_BIN_CTL
    {CAM_ISP_DBS_,                  0x4AC0, 0x0AC0,  13  }, // CAM_DBS_SIGMA
    {CAM_ISP_OBC_,                  0x43F0, 0x03F0,  8   }, // CAM_OBC_OFFST0
    {CAM_ISP_RMG_,                  0x4350, 0x0350,  3   }, // CAM_RMG_HDR_CFG
    {CAM_ISP_BNR_,                  0x4420, 0x0420,  32  }, // CAM_BNR_BPC_CON
    {CAM_ISP_STM_,                  0x44A0, 0x04A0,  2   }, // CAM_STM_CFG0
    {CAM_ISP_SCM_,                  0x44B0, 0x04B0,  2   }, // CAM_SCM_CFG0
    {CAM_ISP_RMM_,                  0x4390, 0x0390,  9   }, // CAM_RMM_OSC
    {CAM_ISP_BMX_,                  0x4580, 0x0580,  3   }, // CAM_BMX_CTL
    {CAM_ISP_LSC_,                  0x45D0, 0x05D0,  11   }, // CAM_LSC_CTL1 //2 new registers
    {CAM_ISP_RCP_,                  0x4800, 0x0800,  2   }, // CAM_RCP_CROP_CON1
    {CAM_ISP_RPG_,                  0x44C0, 0x04C0,  6   }, // CAM_RPG_SATU_1
    {CAM_ISP_RRZ_,                  0x44E0, 0x04E0,  14  }, // CAM_RRZ_CTL //2 new registers
    {CAM_ISP_RMX_,                  0x4540, 0x0540,  3   }, // CAM_RMX_CTL
    {CAM_ISP_SGG5_,                 0x4CB0, 0x0CB0,  3   }, // CAM_SGG5_PGN
    {CAM_ISP_UFE_,                  0x4D20, 0x0D20,  1   }, // CAM_UFE_CON
    {CAM_ISP_RCP3_,                 0x4AB0, 0x0AB0,  2   }, // CAM_RCP3_CROP_CON1
    {CAM_ISP_SGM_,                  0x4FA0, 0x0FA0,  12  }, // CAM_SGM_R_OFST_TABLE0_3
    {CAM_ISP_CPG_,                  0x49E0, 0x09E0,  6   }, // CAM_CPG_SATU_1
    {CAM_ISP_QBN1_,                 0x49D0, 0x09D0,  1   }, // CAM_QBN1_MODE
    {CAM_ISP_PS_AWB_,             0x4BD0, 0x0BD0,  8   }, // CAM_PS_AWB_WIN_ORG
    {CAM_ISP_PS_AE_,                0x4BF0, 0x0BF0,  6   }, // CAM_PS_AE_YCOEF0
    {CAM_ISP_SL2F_,                  0x4B00, 0x0B00,  12  }, // CAM_SL2F_CEN
    {CAM_ISP_PCP_CROP_,             0x4B80, 0x0B80,  2   }, // CAM_PCP_CROP_CON1
    {CAM_ISP_SGG2_,                 0x4B90, 0x0B90,  3   }, // CAM_REG_SGG2_PGN
    {CAM_ISP_SL2J_,                 0x4B40, 0x0B40,  12  }, // CAM_SL2J_CEN
    {CAM_ISP_HLR_,                  0x4710, 0x0710,  12  }, // CAM_HLR_CFG
    {CAM_ISP_PSB_,                  0x4BA0, 0x0BA0,  2   }, // CAM_PSB_CON
    {CAM_ISP_PDE_,                  0x4BB0, 0x0BB0,  2   }, // CAM_PDE_TBLI1
    {CAM_ISP_LCS_,                  0x4790, 0x0790,  18  }, // CAM_LCS25_CON
    {CAM_ISP_AE_,                   0x4900, 0x0900,  29  }, // CAM_REG_AE_GAIN2_0 can't access 4974, this register is for CCU only (AE on CCU)
    {CAM_ISP_VBN_,                  0x4A30, 0x0A30,  4   }, // CAM_VBN_GAIN
    {CAM_ISP_AMX_,                  0x4A50, 0x0A50,  3   }, // CAM_AMX_CTL
    {CAM_ISP_AWB_,                  0x4830, 0x0830,  42  }, // CAM_AWB_WIN_ORG
    {CAM_ISP_SGG1_,                 0x4810, 0x0810,  3   }, // CAM_SGG1_PGN
    {CAM_ISP_AF_,                   0x4610, 0x0610,  47  }, // CAM_AF_CON
    {CAM_ISP_UFEG_,                 0x45C0, 0x05C0,  1   }, // CAM_UFEG_CON
    {CAM_ISP_RMB_,                  0x4BC0, 0x0BC0,  1   }, // CAM_RMB_MODE
    {CAM_ISP_SGG3_,                 0x4C40, 0x0C40,  3   }, // CAM_SGG3_PGN
    {CAM_ISP_FLK_A_,                 0x4C50, 0x0C50,  4  }, // CAM_FLK_A_CON
    {CAM_ISP_LMV_,                    0x4C70, 0x0C70,  9  }, // CAM_EIS_PREP_ME_CTRL1   EIS rename to LMV
    {CAM_ISP_HDS_,                  0x4CC0, 0x0CC0,  1   }, // CAM_HDS_MODE
    {CAM_ISP_RSS_A_,                0x4CE0, 0x0CE0,  9  }, // CAM_RSS_A_CONTROL
    {CAM_ISP_ADBS_,                0x4D50, 0x0D50,  21  }, // CAM_ADBS_CTL               new module on cannon
    {CAM_ISP_DCPN_,                0x4DB0, 0x0DB0,  8   }, // CAM_DCPN_HDR_EN         new module on cannon
    {CAM_ISP_CPN_,                 0x4DF0, 0x0DF0,  16   }, // CAM_CPN_HDR_CTL_EN   new module on cannon
    {UNI_CAM_XMX_SET_,         0x13000, 0x0204,  1 }, // UNI_CAM_XMX_SET_ SET RAWA~D XMX
    {UNI_CAM_XMX_CLR_,         0x33000, 0x0208,  1 }, // UNI_CAM_XMX_CLR_ CLR RAWA~D XMX
    {UNI_CTL_EN_,                   0x3010, 0x0010,  2   }, // CAM_UNI_TOP_MOD_EN
    {UNI_CTL_FMT_,                  0x301C, 0x001C,  1   }, // CAM_UNI_TOP_FMT_SEL
    {UNI_DMA_V_FLIP_,               0x3104, 0x0104,  1   }, // CAM_UNI_VERTICAL_FLIP_EN
    {UNI_DMA_RAWI_,                 0x3120, 0x0120,  7   }, // CAM_UNI_RAWI_BASE_ADDR
    {UNI_ISP_UNP2_,                 0x3200, 0x0200,  1   }, // CAM_UNI_UNP2_A_OFST
    {CAM_DUMMY_,                    0x4FFC, 0x0FFC,  1   },
    {CAM_CTL_FBC_RCNT_INC_,         0x4060, 0x0060,  1   }, // CAM_CTL_FBC_RCNT_INC //must be allocated after all FBC_en descirptor
    {CAM_CQ_EN_,                    0x4190, 0x0190,  1   }, // CAM_CQ_EN //control
    {CAM_NEXT_Thread_,              0x14198, 0x0198,  1   }, // CAM_CQ_THR0_BASEADDR special descriptor for CQ-thre0 link-list, must at the end of descriptor  , or apb will be crashed
    {CAM_CAM_END_,                  0x0000, 0x0000,  1   },
//    {CAM_CAM_MODULE_MAX,            0x0000, 0x0000,  1   },
};

ISP_DRV_CQ_MODULE_INFO_STRUCT mIspCQModuleInfo_SpecialCQ[ISP_DRV_CAM_BASIC_CQ_NUM]    =
{
    {CAM_CQ_THRE0_ADDR_CQONLY,      0x14198, 0x2000,  1   }, //special sw ofset
    {CAM_CQ_THRE1_ADDR_CQONLY,      0x141A4, 0x2004,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE2_ADDR_CQONLY,      0x141B0, 0x2008,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE3_ADDR_CQONLY,      0x141BC, 0x200C,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE4_ADDR_CQONLY,      0x141C8, 0x2010,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE5_ADDR_CQONLY,      0x141D4, 0x2014,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE6_ADDR_CQONLY,      0x141E0, 0x2018,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE7_ADDR_CQONLY,      0x141EC, 0x201C,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE8_ADDR_CQONLY,      0x141F8, 0x2020,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE9_ADDR_CQONLY,      0x14204, 0x2024,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE10_ADDR_CQONLY,    0x14210, 0x2028,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE11_ADDR_CQONLY,    0x1421C, 0x202C,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
    {CAM_CQ_THRE12_ADDR_CQONLY,    0x14228, 0x2030,  1   }, //special sw ofset, this value is an none-sense value,real offset value will be patched at createinstance
};


ISP_DRV_CAM**** gPageTbl = NULL;
android::Mutex  gMutex;



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
        case CAM_A_TRIPLE_B:
            m_HWModule = CAM_B;
            break;
        case CAM_A_TWIN_C://this is a virtual hw
        case CAM_B_TWIN_C:
        case CAM_A_TRIPLE_C:
            m_HWModule = CAM_C;
            break;
        default:
            m_HWModule = module;
            break;
    }
    m_pIspDrvImp = IspDrvImp::createInstance(m_HWModule);

    m_CQ = cq;
    m_pageIdx = pageIdx;

    BASE_LOG_DBG("getpid[0x%08x],gettid[0x%08x]\n", getpid() ,gettid());
}


IspDrvVir* ISP_DRV_CAM::createInstance(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx,const char* userName)
{
    BASE_LOG_INF("module:0x%x,cq:0x%x,pageIdx:0x%x,userName:%s",module,cq,pageIdx,userName);
    if(userName == NULL){
        BASE_LOG_ERR("userName can't be NULL (cq:0x%x,pageidx:0x%x)",cq,pageIdx);
        return NULL;
    }

    switch(module){
        case CAM_A:
        case CAM_B:
        case CAM_C:
        case CAM_A_TWIN_B:
        case CAM_A_TWIN_C:
        case CAM_B_TWIN_C:
        case CAM_A_TRIPLE_B:
        case CAM_A_TRIPLE_C:
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
    else{
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

        LOG_INF("+ module:0x%x,cq:0x%x,pageIdx:0x%x,userName:%s\n",this->m_Module,this->m_CQ,this->m_pageIdx,this->m_useName);

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
                if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispVirRegBufInfo) ) {
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
                if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispCQDescBufInfo) ) {
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
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR0_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                case ISP_DRV_CQ_THRE1:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR1_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                case ISP_DRV_CQ_THRE10:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR10_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
                    break;
                case ISP_DRV_CQ_THRE11:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR11_DESC_SIZE,sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_CAM_MODULE_MAX);
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
                if ( this->m_pMemDrv->mapPhyAddr(&this->m_ispCQDescBufInfo) ) {
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
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR4_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE5:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR5_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE7:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR7_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE8:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR8_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
                    break;
                case ISP_DRV_CQ_THRE12:
                    CAM_WRITE_REG(this->getPhyObj(),CAM_CQ_THR12_DESC_SIZE,(RingBuf_MODULE_MAX+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT));
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
    MBOOL Result = MTRUE;
    MUINTPTR _tmpVirAddr,_tmpPhyAddr;
    MINT32 tmp=0;
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
    LOG_INF(" - E. m_UserCnt(%d), curUser(%s),module:0x%x,cq:0x%x,pageIdx:0x%x", this->m_UserCnt,userName,this->m_Module,this->m_CQ,this->m_pageIdx);

    // init ispdrvimp
    //to avoid iIspDrv userCnt over upper bound
    if(this->m_pIspDrvImp->init(userName) == MFALSE){
        Result = MFALSE;
        LOG_ERR("isp drv init fail\n");
        goto EXIT;
    }

    //
    if(this->m_UserCnt > 0)
    {
        tmp = android_atomic_inc(&this->m_UserCnt);
        LOG_INF(" - X. m_UserCnt: %d.", this->m_UserCnt);
        return Result;
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
        case ISP_DRV_CQ_THRE11:
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
        case ISP_DRV_CQ_THRE11:
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
    LOG_INF(" - X. ret: %d. mInitCount: %d.", Result, this->m_UserCnt);
    return Result;
}


MBOOL ISP_DRV_CAM::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MINT32 tmp=0;
    //MUINT32 bClr = 0;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_Uninit,__FUNCTION__) == MFALSE)
        return MFALSE;
    //
    LOG_INF(" - E. m_UserCnt(%d), curUser(%s)", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }
    //
    if(this->m_UserCnt <= 0)
    {
        // No more users
        goto EXIT;
    }
    // More than one user
    tmp = android_atomic_dec(&this->m_UserCnt);
    if(this->m_UserCnt > 0)    // If there are still users, exit.
    {
        goto EXIT;
    }

    //free cq descriptor
    if(this->m_ispCQDescBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispCQDescBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispCQDescBufInfo) != 0){
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
        LOG_INF("cq descriptor PA is already NULL");
    }
    this->m_pIspDescript_vir = NULL;
    this->m_pIspDescript_phy = NULL;

    //free virtual isp
    if(this->m_ispVirRegBufInfo.phyAddr != 0){
        if(this->m_pMemDrv->unmapPhyAddr(&this->m_ispVirRegBufInfo) == 0){
            if(this->m_pMemDrv->freeVirtBuf(&this->m_ispVirRegBufInfo) != 0){
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
EXIT:

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


MBOOL ISP_DRV_CAM::getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    MBOOL rst = MTRUE;
    if(this->FSM_CHK(OP_IspDrv_Device,__FUNCTION__) == MFALSE)
        return MFALSE;

    switch(eCmd){
        case _GET_SOF_CNT:
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
                MUINT32 dbg[3];
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
                dbg[2] = 0;
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
        case _SET_PM_QOS_INFO:
            if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_SET_PM_QOS_INFO, (ISP_PM_QOS_INFO_STRUCT*)pData) < 0){
                LOG_ERR("ISP_SET_DMAO_BW_INFO error\n");
                return MFALSE;
            }
            break;
        case _SET_PM_QOS_ON:
            {
                MUINT32 dbg[2];
                switch(this->m_HWModule){
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

                dbg[0] = 1;
                if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_SET_PM_QOS, (unsigned int*)dbg) < 0){
                    LOG_ERR("_SET_PM_QOS_ON error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_PM_QOS_RESET:
            {
                MUINT32 dbg[2];
                switch(this->m_HWModule){
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
                MUINT32 dbg[6];

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
                dbg[4] = this->readReg(0x10,this->m_HWModule);
                dbg[5] = this->readReg(0x18,this->m_HWModule);
                LOG_INF("[DAPC]dbg[0]:0x%x dbg[1]:0x%x dbg[2]:0x%x dbg[3]:0x%x dbg[4]:0x%x dbg[5]:0x%x",
                        dbg[0],dbg[1],dbg[2],dbg[3],dbg[4],dbg[5]);
                if(ioctl(this->m_pIspDrvImp->m_Fd, ISP_SET_SEC_DAPC_REG, (unsigned int*)dbg) < 0){
                    LOG_ERR("_SET_SEC_DAPC_REG error\n");
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
#if Vir_Buf_Order
            shift = 0x0;
#else
            shift = UNI_BASE_RANGE;
#endif
            break;
        case UNI_A:
#if Vir_Buf_Order
            shift = CAM_BASE_RANGE_SPECIAL;
#else
            shift = 0x0;
#endif
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
    MUINT32 vr_offset;
    MUINTPTR dummyaddr;

    LOG_DBG("[0x%x_0x%x_0x%x]moduleID:0x%x",this->m_HWModule,this->m_CQ,this->m_pageIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    switch(this->m_HWModule){
        case CAM_A:
            break;
        case CAM_B:
            if((mIspCQModuleInfo[moduleId].addr_ofst & CQ_DES_RANGE) < 0x4000)//uni module
                offset = 0x0;
            else
                offset = CAM_BASE_RANGE;//cam_b hw start at 0x6000,not 0x4000
            break;
        case CAM_C:
            if((mIspCQModuleInfo[moduleId].addr_ofst & CQ_DES_RANGE) < 0x4000)//uni module
                offset = 0x0;
            else
                offset = CAM_BASE_RANGE*2;//cam_c hw start at 0x8000,not 0x4000
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

                if (_cq >= ISP_DRV_CQ_NONE) {
                    LOG_ERR("unsupported hw module:0x%x, extModuleId: %d\n", this->m_Module, extModuleId);
                    return MFALSE;
                }

                dummyaddr = (MUINTPTR)((MUINTPTR)this->m_pIspVirRegAddr_pa + mIspCQModuleInfo_SpecialCQ[_cq].sw_addr_ofst);
                //
                if((mIspCQModuleInfo[moduleId].addr_ofst & CQ_DES_RANGE) < 0x4000)
                    dummyaddr += this->VirReg_OFFSET(UNI_A);
                else
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
            if(((mIspCQModuleInfo[moduleId].addr_ofst+cam_oft) & CQ_DES_RANGE) < 0x4000)
                dummyaddr += this->VirReg_OFFSET(UNI_A);
            else
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

    switch(slave_cam) {
        case CAM_A:
            return 0x4000; //cam_a hw start at 0x4000
            break;
        case CAM_B:
            return 0x4000 + CAM_BASE_RANGE; //cam_b hw start at 0x6000
            break;
        case CAM_C:
            return 0x4000 + CAM_BASE_RANGE*2; //cam_c hw start at 0x8000
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
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

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
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    this->m_pIspDescript_vir[moduleId].u.cmd |= (ISP_CQ_NOP_INST<<26);
    LOG_DBG("cmd:0x%x",this->m_pIspDescript_vir[moduleId].u.cmd);
    return MTRUE;
}


MBOOL ISP_DRV_CAM::cqApbModule(MUINT32 moduleId)
{
    LOG_DBG("[0x%x_0x%x]moduleID:0x%x",this->m_CQ,this->m_pageIdx,moduleId);
    //
    android::Mutex::Autolock lock(this->IspRegMutex);
    //
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    this->m_pIspDescript_vir[moduleId].u.cmd &= (0xFFFFFFFF - (ISP_CQ_NOP_INST<<26));
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

MUINT32 ISP_DRV_CAM::getCQDescBufSize(void)
{
    LOG_INF("[0x%x_0x%x]+ CQ size:0x%x",this->m_CQ,this->m_pageIdx,this->m_ispCQDescBufInfo.size);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return this->m_ispCQDescBufInfo.size;
}

MINT32 ISP_DRV_CAM::getCQIonFD(void)
{
    LOG_INF("[0x%x_0x%x]+ CQ memID:0x%x",this->m_CQ,this->m_pageIdx,this->m_ispCQDescBufInfo.memID);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return this->m_ispCQDescBufInfo.memID;
}

MUINT32* ISP_DRV_CAM::getIspVirRegVirAddr(void)
{
    LOG_DBG("[0x%x_0x%x]+ ",this->m_CQ,this->m_pageIdx);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return (MUINT32*)this->m_pIspVirRegAddr_va;
}

MUINT32 ISP_DRV_CAM::getIspVirRegSize(void)
{
    LOG_INF("[0x%x_0x%x]+ Virt size:0x%x",this->m_CQ,this->m_pageIdx,this->m_ispVirRegBufInfo.size);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return this->m_ispVirRegBufInfo.size;
}

MINT32 ISP_DRV_CAM::getIspVirIonFD(void)
{
    LOG_INF("[0x%x_0x%x]+ Virt memID:0x%x",this->m_CQ,this->m_pageIdx,this->m_ispVirRegBufInfo.memID);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return this->m_ispVirRegBufInfo.memID;
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

MBOOL ISP_DRV_CAM::readCQModuleInfo(MUINT32 moduleId, void **ptr, MUINT32 size)
{
    /* [in/out] ptr
       Parsing CQ descriptor for dump current virtual register setting, save register to **ptr
    */

    MBOOL Ret = MFALSE;
    MUINT32 mode, j, addrOffset, moduleSize, _addr = 0;
    MUINT32 *pIspRegMap;
    char _str[1024] = {"\0"};
    char _tmp[16] = {"\0"};

    LOG_DBG("[0x%x_0x%x]moduleID:0x%x",this->m_CQ,this->m_pageIdx,moduleId);

    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return MFALSE;

    typedef enum{
        CQ_MD_APB = ISP_CQ_APB_INST,
        CQ_MD_NOP = ISP_CQ_NOP_INST,
        CQ_MD_APB_SKIP,//inner or special for twin
        CQ_MD_DUMY,
    }E_CQ_MODE;

    pIspRegMap = (MUINT32 *)*ptr;
    if(pIspRegMap == NULL){
         LOG_ERR("moduleID:0x%x, mem *ptr=null",moduleId);
        return MFALSE;
    }

    //
    Ret = this->getCQModuleInfo(moduleId, addrOffset, moduleSize);
    if(Ret == MFALSE)
        return Ret;

    if ((mIspCQModuleInfo[moduleId].addr_ofst > 0x10000) || //inner cmd
        (mIspCQModuleInfo[moduleId].addr_ofst <  0x4000)){ //uni(0x3xxx) & special for twin(0x0xxx)
        mode = CQ_MD_APB_SKIP;
    } else if(this->m_pIspDescript_vir[moduleId].u.cmd == ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule)){
        mode = CQ_MD_DUMY;
    } else {
        mode = ISP_CQ_IS_NOP(this->m_pIspDescript_vir[moduleId].u.cmd);
    }

    if(mode != CQ_MD_APB){
        /* no need to reset vir. reg value
           1. sw_addr_ofst of uni and cam might be the same
           2. sw_addr_ofst of inner cmd ill be out of reg range
        */
        if(mode == CQ_MD_APB_SKIP)
            return MTRUE;

        //dummy or nop cmd
        for(j = 0 ;j < moduleSize; j++){
            _addr = addrOffset + (j*0x4);
            if (_addr >= size){
                LOG_ERR("addr(%x) > memory size(%x)\n", _addr, size);
                break;
            }
            pIspRegMap[(_addr>>2)] = 0x0;
        }

        return MTRUE;
    }

    LOG_DBG("[%d]:[0x%08x]:\n",moduleId, this->m_pIspDescript_vir[moduleId].u.cmd);
    for(j = 0 ;j < moduleSize; j++){
        _addr = addrOffset + (j*0x4);

        if (_addr >= size){
            LOG_ERR("addr(%x) > memory size(%x)\n", _addr, size);
            break;
        }
        pIspRegMap[(_addr>>2)] = this->readReg(_addr,CAM_A);

        //debug log
        if ( MTRUE == isp_drv_CAM_DbgLogEnable_DEBUG){
            snprintf(_tmp, sizeof(_tmp),"0x%08x-",pIspRegMap[(_addr>>2)]);
            strncat(_str,_tmp, (sizeof(_str)-strlen(_str)));
        }
    }

    //debug log
    if ( MTRUE == isp_drv_CAM_DbgLogEnable_DEBUG){
        LOG_DBG("%s\n",_str);
        _str[0] = '\0';
    }

    return MTRUE;
}

IspDrv* ISP_DRV_CAM::getPhyObj(void)
{
    if(this->FSM_CHK(OP_IspDrv_CQ,__FUNCTION__) == MFALSE)
        return NULL;

    return this->m_pIspDrvImp;
}
MBOOL ISP_DRV_CAM::dumpCQTable(void)
{
    LOG_INF("[0x%x_0x%x]+",this->m_CQ,this->m_pageIdx);
    char _str[1024] = {"\0"};
    char _tmp[16] = {"\0"};

    //cam_next_thread is using another moduleinfo table

    for(MUINT32 i=0;i<CAM_NEXT_Thread_;i++){
        if(this->m_pIspDescript_vir[i].u.cmd != ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule)) {
            LOG_INF("[%d]:[0x%08x]:",i,this->m_pIspDescript_vir[i].u.cmd);
            //LOG_CQ_VIRTUAL_TABLE(this->m_pIspVirRegAddr_va,i,mIspCQModuleInfo[i].reg_num);
            for(unsigned int j=0;j<mIspCQModuleInfo[i].reg_num;j++){
                if( (mIspCQModuleInfo[i].addr_ofst < 0x4000) && (mIspCQModuleInfo[i].addr_ofst >= 0x3000) )
                    sprintf(_tmp,"0x%08x-",this->readReg(mIspCQModuleInfo[i].sw_addr_ofst + (j*0x4),UNI_A));
                else
                    sprintf(_tmp,"0x%08x-",this->readReg(mIspCQModuleInfo[i].sw_addr_ofst + (j*0x4)));
                strncat(_str,_tmp,strlen(_tmp));
            }
            LOG_INF(" %s\n",_str);
            _str[0] = '\0';
        }
    }
    if(this->m_pIspDescript_vir[CAM_NEXT_Thread_].u.cmd != ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule)) {
        LOG_INF("[%d]:[0x%08x]:",CAM_NEXT_Thread_,this->m_pIspDescript_vir[CAM_NEXT_Thread_].u.cmd);
        //LOG_CQ_VIRTUAL_TABLE(this->m_pIspVirRegAddr_va,i,mIspCQModuleInfo[i].reg_num);
        for(unsigned int j=0;j<mIspCQModuleInfo_SpecialCQ[this->m_CQ].reg_num;j++){
            sprintf(_tmp,"0x%08x-",this->m_pIspVirRegAddr_va[(mIspCQModuleInfo_SpecialCQ[this->m_CQ].sw_addr_ofst >>2) + j]);
            strncat(_str,_tmp,strlen(_tmp));
        }
        LOG_INF(" %s\n",_str);
        _str[0] = '\0';
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

MBOOL ISP_DRV_CAM::DumpReg(MBOOL bPhy)
{
    char _tmpchr[16] = "\0";
    char _chr[256] = "\0";
    MUINT32 shift=0x0;

    switch(this->m_HWModule){
        case CAM_A:
            shift = 0x4000;
            break;
        case CAM_B:
            shift = 0x4000 + CAM_BASE_RANGE;
            break;
        case CAM_C:
            shift = 0x4000 + CAM_BASE_RANGE*2;
            break;
        default:
            break;
    }

    if(bPhy){
        LOG_ERR("###################\n");
        LOG_ERR("start dump phy reg\n");
        for(MUINT32 i=0x0;i<CAM_BASE_RANGE;i+=0x20){
            _chr[0] = '\0';
            sprintf(_chr,"0x%x: ",shift + i);
            for(MUINT32 j=i;j<(i+0x20);j+=0x4){
                if(j >= CAM_BASE_RANGE)
                    break;
                _tmpchr[0] = '\0';
                sprintf(_tmpchr,"0x%8x - ",this->m_pIspDrvImp->readReg(j));
                strncat(_chr,_tmpchr, strlen(_tmpchr));
            }
            LOG_WRN("%s\n",_chr);
        }
        LOG_ERR("###################\n");
    }
    else{
        LOG_ERR("###################\n");
        LOG_ERR("start dump vir reg(uni is inclueed)\n");

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

        //uni
        shift = 0x3000;
        for(MUINT32 i=0x0;i<UNI_BASE_RANGE;i+=0x20){
            _chr[0] = '\0';
            sprintf(_chr,"0x%8x: ",shift + i);
            for(MUINT32 j=i;j<(i+0x20);j+=0x4){
                if(j >= UNI_BASE_RANGE)
                    break;
                _tmpchr[0] = '\0';
                sprintf(_tmpchr,"0x%x - ",this->readReg(j,UNI_A));
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
        case ISP_DRV_CQ_THRE11:
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
        case UNI_A:
            legal_range = UNI_BASE_RANGE;
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
        case UNI_A:
            legal_range = UNI_BASE_RANGE;
            break;
        default:
            LOG_ERR("un-supported module_0x%x\n",caller);
            return MFALSE;
            break;
    }

    shift = this->VirReg_OFFSET(caller);

    if(Addr >= legal_range){
        LOG_ERR("over range(0x%x_0x%x)\n",sizeof(cam_reg_t),Addr);
        return MFALSE;
    }
    LOG_DBG("Data:0x%x",this->m_pIspVirRegAddr_va[(Addr+shift)>>2]);

    return this->m_pIspVirRegAddr_va[(Addr+shift)>>2];
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
        case UNI_A:
            legal_range = UNI_BASE_RANGE;
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
        case UNI_A:
            legal_range = UNI_BASE_RANGE;
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
        if(0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispVirRegBufInfo) ){
            LOG_ERR("virReg flush fail");
            return MFALSE;
        }
    }

    if( 0 != this->m_pMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH,&this->m_ispCQDescBufInfo) ){
        LOG_ERR("descriptor flush fail");
        return MFALSE;
    }

    LOG_DBG("-");//for flush performance tracking
    return MTRUE;
}

//support no replace function
MBOOL ISP_DRV_CAM::updateEnqCqRingBuf(CQ_RingBuf_ST *pBuf_ctrl)
{
    MINT32  Ret = MTRUE;
    MUINT32 i = 0, j = 0;
    MUINT32 offset[RingBuf_MODULE_MAX] = {0};
    MUINT32 offsetModule = 0; // for high speed
    MUINT32 moduleid[RingBuf_MODULE_MAX] = {0};
    MUINT32 cqModuleNum = 4;
    //
    #define RTBC_GET_PA_FROM_VA(va,bva,bpa) ( ( (unsigned long)(va) - (unsigned long)(bva) ) + (unsigned long)(bpa) )

    CQ_RTBC_RING_ST_CAM *pcqrtbcring_va  = (CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_vir;
    CQ_RTBC_RING_ST_CAM *pcqrtbcring_pa  = (CQ_RTBC_RING_ST_CAM*)this->m_pIspDescript_phy;


    if ( CAM_READ_BITS(this->getPhyObj(), CAM_TG_VF_CON,VFDATA_EN) == 0 ) {
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
                moduleid[0] = CAM_DMA_PSO_BA;
                moduleid[1] = CAM_DMA_FH_PSO_;
                moduleid[2] = CAM_DMA_FH_PSO_SPARE_;
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
                    offset[j] = CAM_BASE_RANGE;
                    break;
                case CAM_C:
                    offset[j] = CAM_BASE_RANGE*2;
                    break;
                default:
                    LOG_ERR("unsuppoted module:0x%x\n",this->m_HWModule);
                    return MFALSE;
                    break;
            }

           if(mIspCQModuleInfo[moduleid[j]].addr_ofst < 0x4000)//uni module
                offset[j] = 0x0;
        }

        switch(this->m_HWModule){
            case CAM_A:
                offsetModule = 0x0;
                break;
            case CAM_B:
                offsetModule = CAM_BASE_RANGE;
                break;
            case CAM_C:
                offsetModule = CAM_BASE_RANGE*2;
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
            //next thread addr
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.descriptor = \
                DESCRIPTOR_TOKEN(mIspCQModuleInfo_SpecialCQ[this->m_CQ].addr_ofst,mIspCQModuleInfo_SpecialCQ[this->m_CQ].reg_num,offsetModule);

            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.data = \
                (unsigned long)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].next_pa,pcqrtbcring_va,pcqrtbcring_pa);
        }
        else{
            pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_Thread_addr_inner.descriptor = \
                ISP_DRV_CQ_DUMMY_TOKEN(this->m_HWModule);

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
    MUINT32 shift;
    MUINT32 offset;
    MUINTPTR dummyaddr;
    switch(this->m_CQ){
        case ISP_DRV_CQ_THRE1:
        case ISP_DRV_CQ_THRE10:
        case ISP_DRV_CQ_THRE11:
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
            offset = 0x4000;
            break;
        case CAM_B:
            offset = 0x4000 + CAM_BASE_RANGE;//cam_b hw start at 0x6000,not 0x4000
            break;
        case CAM_C:
            offset = 0x4000 + CAM_BASE_RANGE*2;//cam_c hw start at 0x8000,not 0x4000
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
