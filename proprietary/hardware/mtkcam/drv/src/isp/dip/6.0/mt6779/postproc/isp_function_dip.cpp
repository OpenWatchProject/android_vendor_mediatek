#define LOG_TAG "iio/ifunc_dip"

#include "isp_function_dip.h"
#include <mtkcam/def/PriorityDefs.h>
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
//thread
#include <utils/threads.h>
#include <utils/StrongPointer.h>
//tpipe
#include <tpipe_config.h>
//

#include <cutils/properties.h>  // For property_get().
// TODO: hungwen
#include <stdlib.h>

using namespace std;


#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#define  DEBUG_ISP_ONLY     MFALSE

DECLARE_DBG_LOG_VARIABLE(function_Dip);
//EXTERN_DBG_LOG_VARIABLE(function);

// Clear previous define, use our own define.
#undef DIP_FUNC_VRB
#undef DIP_FUNC_DBG
#undef DIP_FUNC_INF
#undef DIP_FUNC_WRN
#undef DIP_FUNC_ERR
#undef DIP_FUNC_AST
#define DIP_FUNC_VRB(fmt, arg...)        do { if (function_Dip_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define DIP_FUNC_DBG(fmt, arg...)        do { if (function_Dip_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define DIP_FUNC_INF(fmt, arg...)        do { if (function_Dip_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define DIP_FUNC_WRN(fmt, arg...)        do { if (function_Dip_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define DIP_FUNC_ERR(fmt, arg...)        do { if (function_Dip_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define DIP_FUNC_AST(cond, fmt, arg...)  do { if (function_Dip_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

// the size only affect tpipe table
#define MAX_TPIPE_WIDTH                  (0) /* 0: read tile width from INIT_PROPERTY_LUT; 1: MAX_TILE_WIDTH_HW; Others(>1): unified HW tile width */
#define MAX_TPIPE_HEIGHT                 (8192)

// tpipe irq mode
#define TPIPE_IRQ_FRAME     (0)
#define TPIPE_IRQ_LINE      (1)
#define TPIPE_IRQ_TPIPE     (2)

///////////////////////////////////////
//temp for EP
#define EP_ISP_ONLY_FRAME_MODE 0
//////////////////////////////////////

#define CHECK_CTL_RAW_RGB_EN_BITS      23
#define CHECK_CTL_RAW_RGB2_EN_BITS     14

#define CHECK_CTL_YUV_EN_BITS          29
#define CHECK_CTL_YUV2_EN_BITS         32

const IspTopModuleMappingTable gIspModuleMappingTableRbgEn[CHECK_CTL_RAW_RGB_EN_BITS]
 ={ {DIPCTL_RGB_EN1_UNP_D1 ,       DIP_A_UNP_D1,     NULL},
    {DIPCTL_RGB_EN1_UFD_D1 ,       DIP_A_UFD_D1,     IspDrv_B::setUFD_D1},
    {DIPCTL_RGB_EN1_SMT_D1 ,       DIP_A_SMT_D1,     IspDrv_B::setSMT_D1},
    {DIPCTL_RGB_EN1_PAKG_D1 ,      DIP_A_DUMMY,      NULL},
    {DIPCTL_RGB_EN1_BPC_D1 ,       DIP_A_BPC_D1,     IspDrv_B::setBPC_D1},
    {DIPCTL_RGB_EN1_OBC_D1,        DIP_A_OBC_D1,     IspDrv_B::setOBC_D1},
    {DIPCTL_RGB_EN1_ZFUS_D1 ,      DIP_A_ZFUS_D1,    IspDrv_B::setZFUS_D1},
    {DIPCTL_RGB_EN1_DGN_D1 ,       DIP_A_DGN_D1,     IspDrv_B::setDGN_D1},
    {DIPCTL_RGB_EN1_LSC_D1 ,       DIP_A_LSC_D1,     IspDrv_B::setLSC_D1},
    {DIPCTL_RGB_EN1_WB_D1  ,       DIP_A_WB_D1,      IspDrv_B::setWB_D1},
    {DIPCTL_RGB_EN1_HLR_D1 ,       DIP_A_HLR_D1,     IspDrv_B::setHLR_D1},
    {DIPCTL_RGB_EN1_LTM_D1 ,       DIP_A_LTM_D1,     IspDrv_B::setLTM_D1},
    {DIPCTL_RGB_EN1_DM_D1  ,       DIP_A_DM_D1,      IspDrv_B::setDM_D1},
    {DIPCTL_RGB_EN1_LDNR_D1,       DIP_A_LDNR_D1,    IspDrv_B::setLDNR_D1},
    {DIPCTL_RGB_EN1_CRP_D2 ,       DIP_A_CRP_D2,     IspDrv_B::setCRP_D2},
    {DIPCTL_RGB_EN1_PAK_D1 ,       DIP_A_PAK_D1,     NULL},
    {DIPCTL_RGB_EN1_WIF_D1 ,       DIP_A_DUMMY,      NULL},     //TODO, justin-yt under confirm
    {DIPCTL_RGB_EN1_WIF_D2 ,       DIP_A_DUMMY,      NULL},
    {DIPCTL_RGB_EN1_GDR_D1 ,       DIP_A_DUMMY,      NULL},     //TODO, justin-yt under confirm
    {DIPCTL_RGB_EN1_ALIGN_D1 ,     DIP_A_DUMMY,      NULL},     //TODO, justin-yt under confirm
    {DIPCTL_RGB_EN1_SLK_D1 ,       DIP_A_SLK_D1,     IspDrv_B::setSLK_D1},
    {DIPCTL_RGB_EN1_SLK_D6 ,       DIP_A_SLK_D6,     IspDrv_B::setSLK_D6},
    {DIPCTL_RGB_EN1_WIF_D3 ,       DIP_A_DUMMY,      NULL}};

const IspTopModuleMappingTable gIspModuleMappingTableRbg2En[CHECK_CTL_RAW_RGB2_EN_BITS]
 ={ {DIPCTL_RGB_EN2_FLC_D1,        DIP_A_FLC_D1,     IspDrv_B::setFLC_D1},
    {DIPCTL_RGB_EN2_CCM_D1,        DIP_A_CCM_D1,     IspDrv_B::setCCM_D1},
    {DIPCTL_RGB_EN2_GGM_D1,        DIP_A_GGM_D1,     IspDrv_B::setGGM_D1},
    {DIPCTL_RGB_EN2_WSYNC_D1,      DIP_A_WSYNC_D1,   IspDrv_B::setWSYNC_D1},
    {DIPCTL_RGB_EN2_CCM_D2,        DIP_A_CCM_D2,     IspDrv_B::setCCM_D2},
    {DIPCTL_RGB_EN2_GGM_D2,        DIP_A_GGM_D2,     IspDrv_B::setGGM_D2},
    {DIPCTL_RGB_EN2_SMT_D4,        DIP_A_SMT_D4,     IspDrv_B::setSMT_D4},
    {DIPCTL_RGB_EN2_UNP_D4,        DIP_A_DUMMY,      NULL},
    {DIPCTL_RGB_EN2_PAK_D4,        DIP_A_DUMMY,      NULL},
    {DIPCTL_RGB_EN2_MCRP_D2,       DIP_A_MCRP_D2,    IspDrv_B::setMCRP_D2},
    {DIPCTL_RGB_EN2_GGM_D4,        DIP_A_DUMMY,      NULL},   // no GGM_D4
    {DIPCTL_RGB_EN2_G2C_D4,        DIP_A_DUMMY,      NULL},   // no G2C_D4,   //TODO, justin-yt under confirm
    {DIPCTL_RGB_EN2_C42_D4,        DIP_A_DUMMY,      NULL},   // no C42_D4
    {DIPCTL_RGB_EN2_CRP_D4,        DIP_A_DUMMY,      NULL}};  // no CRP_D4


const IspTopModuleMappingTable gIspModuleMappingTableYuvEn[CHECK_CTL_YUV_EN_BITS]
 ={ {DIPCTL_YUV_EN1_G2CX_D1,       DIP_A_G2CX_D1,    IspDrv_B::setG2CX_D1},
    {DIPCTL_YUV_EN1_C42_D1,        DIP_A_C42_D1,     IspDrv_B::setC42_D1},
    {DIPCTL_YUV_EN1_MIX_D3,        DIP_A_MIX_D3,     IspDrv_B::setMIX_D3},
    {DIPCTL_YUV_EN1_YNR_D1,        DIP_A_YNR_D1,     IspDrv_B::setYNR_D1},
    {DIPCTL_YUV_EN1_NDG_D1,        DIP_A_NDG_D1,     IspDrv_B::setNDG_D1},
    {DIPCTL_YUV_EN1_MIX_D1,        DIP_A_MIX_D1,     IspDrv_B::setMIX_D1},
    {DIPCTL_YUV_EN1_C24_D3,        DIP_A_C24_D3,     IspDrv_B::setC24_D3},
    {DIPCTL_YUV_EN1_C2G_D1,        DIP_A_C2G_D1,     IspDrv_B::setC2G_D1},
    {DIPCTL_YUV_EN1_IGGM_D1,       DIP_A_IGGM_D1,    IspDrv_B::setIGGM_D1},
    {DIPCTL_YUV_EN1_CCM_D3,        DIP_A_CCM_D3,     IspDrv_B::setCCM_D3},
    {DIPCTL_YUV_EN1_LCE_D1,        DIP_A_LCE_D1,     IspDrv_B::setLCE_D1},
    {DIPCTL_YUV_EN1_GGM_D3,        DIP_A_GGM_D3,     IspDrv_B::setGGM_D3},
    {DIPCTL_YUV_EN1_DCE_D1,        DIP_A_DCE_D1,     IspDrv_B::setDCE_D1},
    {DIPCTL_YUV_EN1_DCES_D1,       DIP_A_DCES_D1,    IspDrv_B::setDCES_D1},
    {DIPCTL_YUV_EN1_G2C_D1,        DIP_A_G2C_D1,     IspDrv_B::setG2C_D1},
    {DIPCTL_YUV_EN1_C42_D2,        DIP_A_C42_D2,     IspDrv_B::setC42_D2},
    {DIPCTL_YUV_EN1_EE_D1 ,        DIP_A_EE_D1,      IspDrv_B::setEE_D1},
    {DIPCTL_YUV_EN1_SMT_D2,        DIP_A_SMT_D2,     IspDrv_B::setSMT_D2},
    {DIPCTL_YUV_EN1_UNP_D2,        DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN1_PAK_D2,        DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN1_CNR_D1,        DIP_A_CNR_D1,     IspDrv_B::setCNR_D1},
    {DIPCTL_YUV_EN1_NDG_D2,        DIP_A_NDG_D2,     IspDrv_B::setNDG_D2},
    {DIPCTL_YUV_EN1_SMT_D3,        DIP_A_SMT_D3,     IspDrv_B::setSMT_D3},
    {DIPCTL_YUV_EN1_UNP_D3,        DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN1_PAK_D3,        DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN1_COLOR_D1,      DIP_A_COLOR_D1,   IspDrv_B::setCOLOR_D1},
    {DIPCTL_YUV_EN1_MIX_D2,        DIP_A_MIX_D2,     IspDrv_B::setMIX_D2},
    {DIPCTL_YUV_EN1_MIX_D4,        DIP_A_MIX_D4,     IspDrv_B::setMIX_D4},
    {DIPCTL_YUV_EN1_CRP_D1,        DIP_A_CRP_D1,     IspDrv_B::setCRP_D1}};

const IspTopModuleMappingTable gIspModuleMappingTableYuv2En[CHECK_CTL_YUV2_EN_BITS]
 ={ {DIPCTL_YUV_EN2_NR3D_D1 ,      DIP_A_NR3D_D1,    IspDrv_B::setNR3D_D1},
    {DIPCTL_YUV_EN2_CRSP_D1,       DIP_A_CRSP_D1,    IspDrv_B::setCRSP_D1},
    {DIPCTL_YUV_EN2_PLNW_D1,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_PAK_D6 ,       DIP_A_PAK_D6,     NULL},
    {DIPCTL_YUV_EN2_PAK_D7 ,       DIP_A_PAK_D7,     NULL},
    {DIPCTL_YUV_EN2_PAK_D8 ,       DIP_A_PAK_D8,     NULL},
    {DIPCTL_YUV_EN2_C24_D2 ,       DIP_A_C24_D2,     IspDrv_B::setC24_D2},
    {DIPCTL_YUV_EN2_MCRP_D1,       DIP_A_MCRP_D1,    IspDrv_B::setMCRP_D1},
    {DIPCTL_YUV_EN2_CRZ_D1 ,       DIP_A_CRZ_D1,     IspDrv_B::setCRZ_D1},
    {DIPCTL_YUV_EN2_PLNW_D2,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_SRZ_D1 ,       DIP_A_SRZ_D1,     IspDrv_B::setSRZ_D1},
    {DIPCTL_YUV_EN2_SRZ_D3 ,       DIP_A_SRZ_D3,     IspDrv_B::setSRZ_D3},
    {DIPCTL_YUV_EN2_SRZ_D4 ,       DIP_A_SRZ_D4,     IspDrv_B::setSRZ_D4},
    {DIPCTL_YUV_EN2_UNP_D6 ,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_UNP_D7 ,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_UNP_D8 ,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_PLNR_D2,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_C02_D2 ,       DIP_A_C02_D2,     IspDrv_B::setC02_D2},
    {DIPCTL_YUV_EN2_C24_D1 ,       DIP_A_C24_D1,     IspDrv_B::setC24_D1},
    {DIPCTL_YUV_EN2_UNP_D9 ,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_UNP_D10,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_UNP_D11,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_PLNR_D1,       DIP_A_DUMMY,      NULL},
    {DIPCTL_YUV_EN2_C02_D1 ,       DIP_A_C02_D1,     IspDrv_B::setC02_D1},
    {DIPCTL_YUV_EN2_SLK_D2 ,       DIP_A_SLK_D2,     IspDrv_B::setSLK_D2},
    {DIPCTL_YUV_EN2_SLK_D3 ,       DIP_A_SLK_D3,     IspDrv_B::setSLK_D3},
    {DIPCTL_YUV_EN2_SLK_D4 ,       DIP_A_SLK_D4,     IspDrv_B::setSLK_D4},
    {DIPCTL_YUV_EN2_SLK_D5 ,       DIP_A_SLK_D5,     IspDrv_B::setSLK_D5},
    {DIPCTL_YUV_EN2_FM_D1  ,       DIP_A_FM_D1,      IspDrv_B::setFM_D1},
    {DIPCTL_YUV_EN2_LPCNR_D1,      DIP_A_LPCNR_D1,   IspDrv_B::setLPCNR_D1},
    {DIPCTL_YUV_EN2_BS_D1  ,       DIP_A_BS_D1,      IspDrv_B::setBS_D1},
    {DIPCTL_YUV_EN2_DFE_D1 ,       DIP_A_DFE_D1,     IspDrv_B::setDFE_D1}};

MINT32
IspDrv_B::
setUNP_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUFD_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;

    MUINT32 fgModeRegBit;
    fgModeRegBit = (imageioPackage.isp_top_ctl.MISC_SEL.Bits.DIPCTL_FG_MODE&0x01);

    if (fgModeRegBit == 1)
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,UFD_D1A_UFD_CON,UFD_SEL,0x0);
    }
    else
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,UFD_D1A_UFD_CON,UFD_SEL,0x1);
    }

    DIP_WRITE_BITS(imageioPackage.pDrvDip,UFD_D1A_UFD_CON,UFD_TCCT_BYP,0x1);

    if (imageioPackage.pUfdParam != NULL)
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,UFD_D1A_UFD_BS2_AU_CON,UFD_BOND_MODE,imageioPackage.pUfdParam->UFDG_BOND_MODE);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,UFD_D1A_UFD_AU2_CON,UFD_AU2_SIZE,imageioPackage.pUfdParam->UFDG_AU2_SIZE);
        DIP_WRITE_REG(imageioPackage.pDrvDip, UFD_D1A_UFD_ADDRESS_CON1, imageioPackage.pUfdParam->UFDG_BITSTREAM_OFST_ADDR);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,UFD_D1A_UFD_BS2_AU_CON,UFD_BS2_AU_START,imageioPackage.pUfdParam->UFDG_BS_AU_START);
    }
    else
    {
        DIP_FUNC_DBG("imageioPackage.pUfdParam is NULL!");
    }

    return 0;
}

MINT32
IspDrv_B::
setSMT_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setBPC_D1(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE)
    {
        MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_BPC_D1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, BPC_D1A_BPC_BPC_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}



MINT32
IspDrv_B::
setOBC_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_OBC_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, OBC_D1A_OBC_CTL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setZFUS_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_ZFUS_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, ZFUS_D1A_ZFUS_OSC,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setDGN_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DGN_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DGN_D1A_DGN_GN0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}



MINT32
IspDrv_B::
setLSC_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
	    MUINT32 addr,size;
    	imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LSC_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, LSC_D1A_LSC_CTL1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setWB_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE)
	{
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_WB_D1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, WB_D1A_WB_GAIN_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}


MINT32
IspDrv_B::
setHLR_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE)
	{
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_HLR_D1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, HLR_D1A_HLR_EST_Y0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;

}

MINT32
IspDrv_B::
setLTM_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE)
	{
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LTM_D1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, LTM_D1A_LTM_CTRL,size,(MUINT32*)imageioPackage.pTuningIspReg);
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LTMTC_D1_A ,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, LTMTC_D1A_LTMTC_CURVE, size,(MUINT32*)imageioPackage.pTuningIspReg);
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LTMTC_D1_B ,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, LTMTC_D1A_LTMTC_CURVE[900], size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}



MINT32
IspDrv_B::
setDM_D1(DIP_ISP_PIPE &imageioPackage)
{
    MUINT32 fgModeRegBit;
    fgModeRegBit = (imageioPackage.isp_top_ctl.MISC_SEL.Bits.DIPCTL_FG_MODE&0x01)<<10;   // OK5 Justin, FMT_SEL there are two SEL(1/2) in ISP6.0


    if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DM_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DM_D1A_DM_INTP_CRS,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }
    else {
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_INTP_CRS, 0x0002F004);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_INTP_NAT, 0x1430053F);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_INTP_AUG, 0x00500500);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_LUMA_LUT1, 0x052A30DC);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_LUMA_LUT2, 0x02A9124F);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_SL_CTL, 0x0039B4A0);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_HFTD_CTL,  0x0A529400);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_NR_STR, 0x81028000);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_NR_ACT,  0x00000050);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_HF_STR, 0x84210000);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_HF_ACT1,  0x46FF1EFF);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_HF_ACT2, 0x001EFF55);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_CLIP,  0x00942064);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_DSB,  0x00000000|fgModeRegBit);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_TILE_EDGE,	0x0000000F);
		//new add
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_P1_ACT,  0x000000FF);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_LR_RAT,  0x00000418);
		//new add
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_HFTD_CTL2,	0x00000019);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_EST_CTL,  0x00000035);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_SPARE_2,  0x00000000);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_SPARE_3,  0x00000000);
		//new add
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_INT_CTL,  0x00000035);
		DIP_WRITE_REG(imageioPackage.pDrvDip, DM_D1A_DM_EE,  0x00000410);
    }

    DIP_FUNC_DBG("fdmode bit/reg/udm_dsb(0x%x/0x%x/0x%x)",imageioPackage.isp_top_ctl.MISC_SEL.Bits.DIPCTL_FG_MODE, fgModeRegBit, 0x007FA800|fgModeRegBit);

    return 0;
}



MINT32
IspDrv_B::
setLDNR_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LDNR_D1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, LDNR_D1A_LDNR_CON1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}


MINT32
IspDrv_B::
setCRP_D2(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setPAK_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setSLK_D1(DIP_ISP_PIPE &imageioPackage)
{
	//3A will calculate and set correct value
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SLK_D1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, SLK_D1A_SLK_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setSLK_D6(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SLK_D6,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, SLK_D6A_SLK_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setFLC_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_FLC_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, FLC_D1A_FLC_OFST_RB,size,(MUINT32*)imageioPackage.pTuningIspReg);  //OK Justin, FLC_OFST_G / FLC_OFST_RB? no DIP_X_FLC_OFFST0
	}
    return 0;
}



MINT32
IspDrv_B::
setCCM_D1(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){

        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_CCM_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_1, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_2, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_3, 0x02000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_4, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_5, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CNV_6, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CTRL,  0x00000009);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CFC_CTRL1, 0x0FDF0020);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D1A_CCM_CFC_CTRL2, 0x1CE729CE);
    }
    return 0;
}

MINT32
IspDrv_B::
setGGM_D1(DIP_ISP_PIPE &imageioPackage)  //Todo, Justin ,  change addr
{

	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_GGM_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, GGM_D1A_GGM_LUT,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    else
    {
            imageioPackage.pDrvDip->writeReg( 0x000030c0, 0x00000000); /*0x150240c0,   GGM_D1A_GGM_LUT[0] */
            imageioPackage.pDrvDip->writeReg( 0x000030c4, 0x00200802); /*0x150240c4,   GGM_D1A_GGM_LUT[1] */
            imageioPackage.pDrvDip->writeReg( 0x000030c8, 0x00401004); /*0x150240c8,   GGM_D1A_GGM_LUT[2] */
            imageioPackage.pDrvDip->writeReg( 0x000030cc, 0x00601806); /*0x150240cc,   GGM_D1A_GGM_LUT[3] */
            imageioPackage.pDrvDip->writeReg( 0x000030d0, 0x00802008); /*0x150240d0,   GGM_D1A_GGM_LUT[4] */
            imageioPackage.pDrvDip->writeReg( 0x000030d4, 0x00a0280a); /*0x150240d4,   GGM_D1A_GGM_LUT[5] */
            imageioPackage.pDrvDip->writeReg( 0x000030d8, 0x00c0300c); /*0x150240d8,   GGM_D1A_GGM_LUT[6] */
            imageioPackage.pDrvDip->writeReg( 0x000030dc, 0x00e0380e); /*0x150240dc,   GGM_D1A_GGM_LUT[7] */
            imageioPackage.pDrvDip->writeReg( 0x000030e0, 0x01004010); /*0x150240e0,   GGM_D1A_GGM_LUT[8] */
            imageioPackage.pDrvDip->writeReg( 0x000030e4, 0x01204812); /*0x150240e4,   GGM_D1A_GGM_LUT[9] */
            imageioPackage.pDrvDip->writeReg( 0x000030e8, 0x01405014); /*0x150240e8,   GGM_D1A_GGM_LUT[10] */
            imageioPackage.pDrvDip->writeReg( 0x000030ec, 0x01605816); /*0x150240ec,   GGM_D1A_GGM_LUT[11] */
            imageioPackage.pDrvDip->writeReg( 0x000030f0, 0x01806018); /*0x150240f0,   GGM_D1A_GGM_LUT[12] */
            imageioPackage.pDrvDip->writeReg( 0x000030f4, 0x01a0681a); /*0x150240f4,   GGM_D1A_GGM_LUT[13] */
            imageioPackage.pDrvDip->writeReg( 0x000030f8, 0x01c0701c); /*0x150240f8,   GGM_D1A_GGM_LUT[14] */
            imageioPackage.pDrvDip->writeReg( 0x000030fc, 0x01e0781e); /*0x150240fc,   GGM_D1A_GGM_LUT[15] */
            imageioPackage.pDrvDip->writeReg( 0x00003100, 0x02008020); /*0x15024100,   GGM_D1A_GGM_LUT[16] */
            imageioPackage.pDrvDip->writeReg( 0x00003104, 0x02208822); /*0x15024104,   GGM_D1A_GGM_LUT[17] */
            imageioPackage.pDrvDip->writeReg( 0x00003108, 0x02409024); /*0x15024108,   GGM_D1A_GGM_LUT[18] */
            imageioPackage.pDrvDip->writeReg( 0x0000310c, 0x02609826); /*0x1502410c,   GGM_D1A_GGM_LUT[19] */
            imageioPackage.pDrvDip->writeReg( 0x00003110, 0x0280a028); /*0x15024110,   GGM_D1A_GGM_LUT[20] */
            imageioPackage.pDrvDip->writeReg( 0x00003114, 0x02a0a82a); /*0x15024114,   GGM_D1A_GGM_LUT[21] */
            imageioPackage.pDrvDip->writeReg( 0x00003118, 0x02c0b02c); /*0x15024118,   GGM_D1A_GGM_LUT[22] */
            imageioPackage.pDrvDip->writeReg( 0x0000311c, 0x02e0b82e); /*0x1502411c,   GGM_D1A_GGM_LUT[23] */
            imageioPackage.pDrvDip->writeReg( 0x00003120, 0x0300c030); /*0x15024120,   GGM_D1A_GGM_LUT[24] */
            imageioPackage.pDrvDip->writeReg( 0x00003124, 0x0320c832); /*0x15024124,   GGM_D1A_GGM_LUT[25] */
            imageioPackage.pDrvDip->writeReg( 0x00003128, 0x0340d034); /*0x15024128,   GGM_D1A_GGM_LUT[26] */
            imageioPackage.pDrvDip->writeReg( 0x0000312c, 0x0360d836); /*0x1502412c,   GGM_D1A_GGM_LUT[27] */
            imageioPackage.pDrvDip->writeReg( 0x00003130, 0x0380e038); /*0x15024130,   GGM_D1A_GGM_LUT[28] */
            imageioPackage.pDrvDip->writeReg( 0x00003134, 0x03a0e83a); /*0x15024134,   GGM_D1A_GGM_LUT[29] */
            imageioPackage.pDrvDip->writeReg( 0x00003138, 0x03c0f03c); /*0x15024138,   GGM_D1A_GGM_LUT[30] */
            imageioPackage.pDrvDip->writeReg( 0x0000313c, 0x03e0f83e); /*0x1502413c,   GGM_D1A_GGM_LUT[31] */
            imageioPackage.pDrvDip->writeReg( 0x00003140, 0x04010040); /*0x15024140,   GGM_D1A_GGM_LUT[32] */
            imageioPackage.pDrvDip->writeReg( 0x00003144, 0x04210842); /*0x15024144,   GGM_D1A_GGM_LUT[33] */
            imageioPackage.pDrvDip->writeReg( 0x00003148, 0x04411044); /*0x15024148,   GGM_D1A_GGM_LUT[34] */
            imageioPackage.pDrvDip->writeReg( 0x0000314c, 0x04611846); /*0x1502414c,   GGM_D1A_GGM_LUT[35] */
            imageioPackage.pDrvDip->writeReg( 0x00003150, 0x04812048); /*0x15024150,   GGM_D1A_GGM_LUT[36] */
            imageioPackage.pDrvDip->writeReg( 0x00003154, 0x04a1284a); /*0x15024154,   GGM_D1A_GGM_LUT[37] */
            imageioPackage.pDrvDip->writeReg( 0x00003158, 0x04c1304c); /*0x15024158,   GGM_D1A_GGM_LUT[38] */
            imageioPackage.pDrvDip->writeReg( 0x0000315c, 0x04e1384e); /*0x1502415c,   GGM_D1A_GGM_LUT[39] */
            imageioPackage.pDrvDip->writeReg( 0x00003160, 0x05014050); /*0x15024160,   GGM_D1A_GGM_LUT[40] */
            imageioPackage.pDrvDip->writeReg( 0x00003164, 0x05214852); /*0x15024164,   GGM_D1A_GGM_LUT[41] */
            imageioPackage.pDrvDip->writeReg( 0x00003168, 0x05415054); /*0x15024168,   GGM_D1A_GGM_LUT[42] */
            imageioPackage.pDrvDip->writeReg( 0x0000316c, 0x05615856); /*0x1502416c,   GGM_D1A_GGM_LUT[43] */
            imageioPackage.pDrvDip->writeReg( 0x00003170, 0x05816058); /*0x15024170,   GGM_D1A_GGM_LUT[44] */
            imageioPackage.pDrvDip->writeReg( 0x00003174, 0x05a1685a); /*0x15024174,   GGM_D1A_GGM_LUT[45] */
            imageioPackage.pDrvDip->writeReg( 0x00003178, 0x05c1705c); /*0x15024178,   GGM_D1A_GGM_LUT[46] */
            imageioPackage.pDrvDip->writeReg( 0x0000317c, 0x05e1785e); /*0x1502417c,   GGM_D1A_GGM_LUT[47] */
            imageioPackage.pDrvDip->writeReg( 0x00003180, 0x06018060); /*0x15024180,   GGM_D1A_GGM_LUT[48] */
            imageioPackage.pDrvDip->writeReg( 0x00003184, 0x06218862); /*0x15024184,   GGM_D1A_GGM_LUT[49] */
            imageioPackage.pDrvDip->writeReg( 0x00003188, 0x06419064); /*0x15024188,   GGM_D1A_GGM_LUT[50] */
            imageioPackage.pDrvDip->writeReg( 0x0000318c, 0x06619866); /*0x1502418c,   GGM_D1A_GGM_LUT[51] */
            imageioPackage.pDrvDip->writeReg( 0x00003190, 0x0681a068); /*0x15024190,   GGM_D1A_GGM_LUT[52] */
            imageioPackage.pDrvDip->writeReg( 0x00003194, 0x06a1a86a); /*0x15024194,   GGM_D1A_GGM_LUT[53] */
            imageioPackage.pDrvDip->writeReg( 0x00003198, 0x06c1b06c); /*0x15024198,   GGM_D1A_GGM_LUT[54] */
            imageioPackage.pDrvDip->writeReg( 0x0000319c, 0x06e1b86e); /*0x1502419c,   GGM_D1A_GGM_LUT[55] */
            imageioPackage.pDrvDip->writeReg( 0x000031a0, 0x0701c070); /*0x150241a0,   GGM_D1A_GGM_LUT[56] */
            imageioPackage.pDrvDip->writeReg( 0x000031a4, 0x0721c872); /*0x150241a4,   GGM_D1A_GGM_LUT[57] */
            imageioPackage.pDrvDip->writeReg( 0x000031a8, 0x0741d074); /*0x150241a8,   GGM_D1A_GGM_LUT[58] */
            imageioPackage.pDrvDip->writeReg( 0x000031ac, 0x0761d876); /*0x150241ac,   GGM_D1A_GGM_LUT[59] */
            imageioPackage.pDrvDip->writeReg( 0x000031b0, 0x0781e078); /*0x150241b0,   GGM_D1A_GGM_LUT[60] */
            imageioPackage.pDrvDip->writeReg( 0x000031b4, 0x07a1e87a); /*0x150241b4,   GGM_D1A_GGM_LUT[61] */
            imageioPackage.pDrvDip->writeReg( 0x000031b8, 0x07c1f07c); /*0x150241b8,   GGM_D1A_GGM_LUT[62] */
            imageioPackage.pDrvDip->writeReg( 0x000031bc, 0x07e1f87e); /*0x150241bc,   GGM_D1A_GGM_LUT[63] */
            imageioPackage.pDrvDip->writeReg( 0x000031c0, 0x08020080); /*0x150241c0,   GGM_D1A_GGM_LUT[64] */
            imageioPackage.pDrvDip->writeReg( 0x000031c4, 0x08421084); /*0x150241c4,   GGM_D1A_GGM_LUT[65] */
            imageioPackage.pDrvDip->writeReg( 0x000031c8, 0x08822088); /*0x150241c8,   GGM_D1A_GGM_LUT[66] */
            imageioPackage.pDrvDip->writeReg( 0x000031cc, 0x08c2308c); /*0x150241cc,   GGM_D1A_GGM_LUT[67] */
            imageioPackage.pDrvDip->writeReg( 0x000031d0, 0x09024090); /*0x150241d0,   GGM_D1A_GGM_LUT[68] */
            imageioPackage.pDrvDip->writeReg( 0x000031d4, 0x09425094); /*0x150241d4,   GGM_D1A_GGM_LUT[69] */
            imageioPackage.pDrvDip->writeReg( 0x000031d8, 0x09826098); /*0x150241d8,   GGM_D1A_GGM_LUT[70] */
            imageioPackage.pDrvDip->writeReg( 0x000031dc, 0x09c2709c); /*0x150241dc,   GGM_D1A_GGM_LUT[71] */
            imageioPackage.pDrvDip->writeReg( 0x000031e0, 0x0a0280a0); /*0x150241e0,   GGM_D1A_GGM_LUT[72] */
            imageioPackage.pDrvDip->writeReg( 0x000031e4, 0x0a4290a4); /*0x150241e4,   GGM_D1A_GGM_LUT[73] */
            imageioPackage.pDrvDip->writeReg( 0x000031e8, 0x0a82a0a8); /*0x150241e8,   GGM_D1A_GGM_LUT[74] */
            imageioPackage.pDrvDip->writeReg( 0x000031ec, 0x0ac2b0ac); /*0x150241ec,   GGM_D1A_GGM_LUT[75] */
            imageioPackage.pDrvDip->writeReg( 0x000031f0, 0x0b02c0b0); /*0x150241f0,   GGM_D1A_GGM_LUT[76] */
            imageioPackage.pDrvDip->writeReg( 0x000031f4, 0x0b42d0b4); /*0x150241f4,   GGM_D1A_GGM_LUT[77] */
            imageioPackage.pDrvDip->writeReg( 0x000031f8, 0x0b82e0b8); /*0x150241f8,   GGM_D1A_GGM_LUT[78] */
            imageioPackage.pDrvDip->writeReg( 0x000031fc, 0x0bc2f0bc); /*0x150241fc,   GGM_D1A_GGM_LUT[79] */
            imageioPackage.pDrvDip->writeReg( 0x00003200, 0x0c0300c0); /*0x15024200,   GGM_D1A_GGM_LUT[80] */
            imageioPackage.pDrvDip->writeReg( 0x00003204, 0x0c4310c4); /*0x15024204,   GGM_D1A_GGM_LUT[81] */
            imageioPackage.pDrvDip->writeReg( 0x00003208, 0x0c8320c8); /*0x15024208,   GGM_D1A_GGM_LUT[82] */
            imageioPackage.pDrvDip->writeReg( 0x0000320c, 0x0cc330cc); /*0x1502420c,   GGM_D1A_GGM_LUT[83] */
            imageioPackage.pDrvDip->writeReg( 0x00003210, 0x0d0340d0); /*0x15024210,   GGM_D1A_GGM_LUT[84] */
            imageioPackage.pDrvDip->writeReg( 0x00003214, 0x0d4350d4); /*0x15024214,   GGM_D1A_GGM_LUT[85] */
            imageioPackage.pDrvDip->writeReg( 0x00003218, 0x0d8360d8); /*0x15024218,   GGM_D1A_GGM_LUT[86] */
            imageioPackage.pDrvDip->writeReg( 0x0000321c, 0x0dc370dc); /*0x1502421c,   GGM_D1A_GGM_LUT[87] */
            imageioPackage.pDrvDip->writeReg( 0x00003220, 0x0e0380e0); /*0x15024220,   GGM_D1A_GGM_LUT[88] */
            imageioPackage.pDrvDip->writeReg( 0x00003224, 0x0e4390e4); /*0x15024224,   GGM_D1A_GGM_LUT[89] */
            imageioPackage.pDrvDip->writeReg( 0x00003228, 0x0e83a0e8); /*0x15024228,   GGM_D1A_GGM_LUT[90] */
            imageioPackage.pDrvDip->writeReg( 0x0000322c, 0x0ec3b0ec); /*0x1502422c,   GGM_D1A_GGM_LUT[91] */
            imageioPackage.pDrvDip->writeReg( 0x00003230, 0x0f03c0f0); /*0x15024230,   GGM_D1A_GGM_LUT[92] */
            imageioPackage.pDrvDip->writeReg( 0x00003234, 0x0f43d0f4); /*0x15024234,   GGM_D1A_GGM_LUT[93] */
            imageioPackage.pDrvDip->writeReg( 0x00003238, 0x0f83e0f8); /*0x15024238,   GGM_D1A_GGM_LUT[94] */
            imageioPackage.pDrvDip->writeReg( 0x0000323c, 0x0fc3f0fc); /*0x1502423c,   GGM_D1A_GGM_LUT[95] */
            imageioPackage.pDrvDip->writeReg( 0x00003240, 0x10040100); /*0x15024240,   GGM_D1A_GGM_LUT[96] */
            imageioPackage.pDrvDip->writeReg( 0x00003244, 0x10842108); /*0x15024244,   GGM_D1A_GGM_LUT[97] */
            imageioPackage.pDrvDip->writeReg( 0x00003248, 0x11044110); /*0x15024248,   GGM_D1A_GGM_LUT[98] */
            imageioPackage.pDrvDip->writeReg( 0x0000324c, 0x11846118); /*0x1502424c,   GGM_D1A_GGM_LUT[99] */
            imageioPackage.pDrvDip->writeReg( 0x00003250, 0x12048120); /*0x15024250,   GGM_D1A_GGM_LUT[100] */
            imageioPackage.pDrvDip->writeReg( 0x00003254, 0x1284a128); /*0x15024254,   GGM_D1A_GGM_LUT[101] */
            imageioPackage.pDrvDip->writeReg( 0x00003258, 0x1304c130); /*0x15024258,   GGM_D1A_GGM_LUT[102] */
            imageioPackage.pDrvDip->writeReg( 0x0000325c, 0x1384e138); /*0x1502425c,   GGM_D1A_GGM_LUT[103] */
            imageioPackage.pDrvDip->writeReg( 0x00003260, 0x14050140); /*0x15024260,   GGM_D1A_GGM_LUT[104] */
            imageioPackage.pDrvDip->writeReg( 0x00003264, 0x14852148); /*0x15024264,   GGM_D1A_GGM_LUT[105] */
            imageioPackage.pDrvDip->writeReg( 0x00003268, 0x15054150); /*0x15024268,   GGM_D1A_GGM_LUT[106] */
            imageioPackage.pDrvDip->writeReg( 0x0000326c, 0x15856158); /*0x1502426c,   GGM_D1A_GGM_LUT[107] */
            imageioPackage.pDrvDip->writeReg( 0x00003270, 0x16058160); /*0x15024270,   GGM_D1A_GGM_LUT[108] */
            imageioPackage.pDrvDip->writeReg( 0x00003274, 0x1685a168); /*0x15024274,   GGM_D1A_GGM_LUT[109] */
            imageioPackage.pDrvDip->writeReg( 0x00003278, 0x1705c170); /*0x15024278,   GGM_D1A_GGM_LUT[110] */
            imageioPackage.pDrvDip->writeReg( 0x0000327c, 0x1785e178); /*0x1502427c,   GGM_D1A_GGM_LUT[111] */
            imageioPackage.pDrvDip->writeReg( 0x00003280, 0x18060180); /*0x15024280,   GGM_D1A_GGM_LUT[112] */
            imageioPackage.pDrvDip->writeReg( 0x00003284, 0x18862188); /*0x15024284,   GGM_D1A_GGM_LUT[113] */
            imageioPackage.pDrvDip->writeReg( 0x00003288, 0x19064190); /*0x15024288,   GGM_D1A_GGM_LUT[114] */
            imageioPackage.pDrvDip->writeReg( 0x0000328c, 0x19866198); /*0x1502428c,   GGM_D1A_GGM_LUT[115] */
            imageioPackage.pDrvDip->writeReg( 0x00003290, 0x1a0681a0); /*0x15024290,   GGM_D1A_GGM_LUT[116] */
            imageioPackage.pDrvDip->writeReg( 0x00003294, 0x1a86a1a8); /*0x15024294,   GGM_D1A_GGM_LUT[117] */
            imageioPackage.pDrvDip->writeReg( 0x00003298, 0x1b06c1b0); /*0x15024298,   GGM_D1A_GGM_LUT[118] */
            imageioPackage.pDrvDip->writeReg( 0x0000329c, 0x1b86e1b8); /*0x1502429c,   GGM_D1A_GGM_LUT[119] */
            imageioPackage.pDrvDip->writeReg( 0x000032a0, 0x1c0701c0); /*0x150242a0,   GGM_D1A_GGM_LUT[120] */
            imageioPackage.pDrvDip->writeReg( 0x000032a4, 0x1c8721c8); /*0x150242a4,   GGM_D1A_GGM_LUT[121] */
            imageioPackage.pDrvDip->writeReg( 0x000032a8, 0x1d0741d0); /*0x150242a8,   GGM_D1A_GGM_LUT[122] */
            imageioPackage.pDrvDip->writeReg( 0x000032ac, 0x1d8761d8); /*0x150242ac,   GGM_D1A_GGM_LUT[123] */
            imageioPackage.pDrvDip->writeReg( 0x000032b0, 0x1e0781e0); /*0x150242b0,   GGM_D1A_GGM_LUT[124] */
            imageioPackage.pDrvDip->writeReg( 0x000032b4, 0x1e87a1e8); /*0x150242b4,   GGM_D1A_GGM_LUT[125] */
            imageioPackage.pDrvDip->writeReg( 0x000032b8, 0x1f07c1f0); /*0x150242b8,   GGM_D1A_GGM_LUT[126] */
            imageioPackage.pDrvDip->writeReg( 0x000032bc, 0x1f87e1f8); /*0x150242bc,   GGM_D1A_GGM_LUT[127] */
            imageioPackage.pDrvDip->writeReg( 0x000032c0, 0x20080200); /*0x150242c0,   GGM_D1A_GGM_LUT[128] */
            imageioPackage.pDrvDip->writeReg( 0x000032c4, 0x20882208); /*0x150242c4,   GGM_D1A_GGM_LUT[129] */
            imageioPackage.pDrvDip->writeReg( 0x000032c8, 0x21084210); /*0x150242c8,   GGM_D1A_GGM_LUT[130] */
            imageioPackage.pDrvDip->writeReg( 0x000032cc, 0x21886218); /*0x150242cc,   GGM_D1A_GGM_LUT[131] */
            imageioPackage.pDrvDip->writeReg( 0x000032d0, 0x22088220); /*0x150242d0,   GGM_D1A_GGM_LUT[132] */
            imageioPackage.pDrvDip->writeReg( 0x000032d4, 0x2288a228); /*0x150242d4,   GGM_D1A_GGM_LUT[133] */
            imageioPackage.pDrvDip->writeReg( 0x000032d8, 0x2308c230); /*0x150242d8,   GGM_D1A_GGM_LUT[134] */
            imageioPackage.pDrvDip->writeReg( 0x000032dc, 0x2388e238); /*0x150242dc,   GGM_D1A_GGM_LUT[135] */
            imageioPackage.pDrvDip->writeReg( 0x000032e0, 0x24090240); /*0x150242e0,   GGM_D1A_GGM_LUT[136] */
            imageioPackage.pDrvDip->writeReg( 0x000032e4, 0x24892248); /*0x150242e4,   GGM_D1A_GGM_LUT[137] */
            imageioPackage.pDrvDip->writeReg( 0x000032e8, 0x25094250); /*0x150242e8,   GGM_D1A_GGM_LUT[138] */
            imageioPackage.pDrvDip->writeReg( 0x000032ec, 0x25896258); /*0x150242ec,   GGM_D1A_GGM_LUT[139] */
            imageioPackage.pDrvDip->writeReg( 0x000032f0, 0x26098260); /*0x150242f0,   GGM_D1A_GGM_LUT[140] */
            imageioPackage.pDrvDip->writeReg( 0x000032f4, 0x2689a268); /*0x150242f4,   GGM_D1A_GGM_LUT[141] */
            imageioPackage.pDrvDip->writeReg( 0x000032f8, 0x2709c270); /*0x150242f8,   GGM_D1A_GGM_LUT[142] */
            imageioPackage.pDrvDip->writeReg( 0x000032fc, 0x2789e278); /*0x150242fc,   GGM_D1A_GGM_LUT[143] */
            imageioPackage.pDrvDip->writeReg( 0x00003300, 0x280a0280); /*0x15024300,   GGM_D1A_GGM_LUT[144] */
            imageioPackage.pDrvDip->writeReg( 0x00003304, 0x288a2288); /*0x15024304,   GGM_D1A_GGM_LUT[145] */
            imageioPackage.pDrvDip->writeReg( 0x00003308, 0x290a4290); /*0x15024308,   GGM_D1A_GGM_LUT[146] */
            imageioPackage.pDrvDip->writeReg( 0x0000330c, 0x298a6298); /*0x1502430c,   GGM_D1A_GGM_LUT[147] */
            imageioPackage.pDrvDip->writeReg( 0x00003310, 0x2a0a82a0); /*0x15024310,   GGM_D1A_GGM_LUT[148] */
            imageioPackage.pDrvDip->writeReg( 0x00003314, 0x2a8aa2a8); /*0x15024314,   GGM_D1A_GGM_LUT[149] */
            imageioPackage.pDrvDip->writeReg( 0x00003318, 0x2b0ac2b0); /*0x15024318,   GGM_D1A_GGM_LUT[150] */
            imageioPackage.pDrvDip->writeReg( 0x0000331c, 0x2b8ae2b8); /*0x1502431c,   GGM_D1A_GGM_LUT[151] */
            imageioPackage.pDrvDip->writeReg( 0x00003320, 0x2c0b02c0); /*0x15024320,   GGM_D1A_GGM_LUT[152] */
            imageioPackage.pDrvDip->writeReg( 0x00003324, 0x2c8b22c8); /*0x15024324,   GGM_D1A_GGM_LUT[153] */
            imageioPackage.pDrvDip->writeReg( 0x00003328, 0x2d0b42d0); /*0x15024328,   GGM_D1A_GGM_LUT[154] */
            imageioPackage.pDrvDip->writeReg( 0x0000332c, 0x2d8b62d8); /*0x1502432c,   GGM_D1A_GGM_LUT[155] */
            imageioPackage.pDrvDip->writeReg( 0x00003330, 0x2e0b82e0); /*0x15024330,   GGM_D1A_GGM_LUT[156] */
            imageioPackage.pDrvDip->writeReg( 0x00003334, 0x2e8ba2e8); /*0x15024334,   GGM_D1A_GGM_LUT[157] */
            imageioPackage.pDrvDip->writeReg( 0x00003338, 0x2f0bc2f0); /*0x15024338,   GGM_D1A_GGM_LUT[158] */
            imageioPackage.pDrvDip->writeReg( 0x0000333c, 0x2f8be2f8); /*0x1502433c,   GGM_D1A_GGM_LUT[159] */
            imageioPackage.pDrvDip->writeReg( 0x00003340, 0x300c0300); /*0x15024340,   GGM_D1A_GGM_LUT[160] */
            imageioPackage.pDrvDip->writeReg( 0x00003344, 0x308c2308); /*0x15024344,   GGM_D1A_GGM_LUT[161] */
            imageioPackage.pDrvDip->writeReg( 0x00003348, 0x310c4310); /*0x15024348,   GGM_D1A_GGM_LUT[162] */
            imageioPackage.pDrvDip->writeReg( 0x0000334c, 0x318c6318); /*0x1502434c,   GGM_D1A_GGM_LUT[163] */
            imageioPackage.pDrvDip->writeReg( 0x00003350, 0x320c8320); /*0x15024350,   GGM_D1A_GGM_LUT[164] */
            imageioPackage.pDrvDip->writeReg( 0x00003354, 0x328ca328); /*0x15024354,   GGM_D1A_GGM_LUT[165] */
            imageioPackage.pDrvDip->writeReg( 0x00003358, 0x330cc330); /*0x15024358,   GGM_D1A_GGM_LUT[166] */
            imageioPackage.pDrvDip->writeReg( 0x0000335c, 0x338ce338); /*0x1502435c,   GGM_D1A_GGM_LUT[167] */
            imageioPackage.pDrvDip->writeReg( 0x00003360, 0x340d0340); /*0x15024360,   GGM_D1A_GGM_LUT[168] */
            imageioPackage.pDrvDip->writeReg( 0x00003364, 0x348d2348); /*0x15024364,   GGM_D1A_GGM_LUT[169] */
            imageioPackage.pDrvDip->writeReg( 0x00003368, 0x350d4350); /*0x15024368,   GGM_D1A_GGM_LUT[170] */
            imageioPackage.pDrvDip->writeReg( 0x0000336c, 0x358d6358); /*0x1502436c,   GGM_D1A_GGM_LUT[171] */
            imageioPackage.pDrvDip->writeReg( 0x00003370, 0x360d8360); /*0x15024370,   GGM_D1A_GGM_LUT[172] */
            imageioPackage.pDrvDip->writeReg( 0x00003374, 0x368da368); /*0x15024374,   GGM_D1A_GGM_LUT[173] */
            imageioPackage.pDrvDip->writeReg( 0x00003378, 0x370dc370); /*0x15024378,   GGM_D1A_GGM_LUT[174] */
            imageioPackage.pDrvDip->writeReg( 0x0000337c, 0x378de378); /*0x1502437c,   GGM_D1A_GGM_LUT[175] */
            imageioPackage.pDrvDip->writeReg( 0x00003380, 0x380e0380); /*0x15024380,   GGM_D1A_GGM_LUT[176] */
            imageioPackage.pDrvDip->writeReg( 0x00003384, 0x388e2388); /*0x15024384,   GGM_D1A_GGM_LUT[177] */
            imageioPackage.pDrvDip->writeReg( 0x00003388, 0x390e4390); /*0x15024388,   GGM_D1A_GGM_LUT[178] */
            imageioPackage.pDrvDip->writeReg( 0x0000338c, 0x398e6398); /*0x1502438c,   GGM_D1A_GGM_LUT[179] */
            imageioPackage.pDrvDip->writeReg( 0x00003390, 0x3a0e83a0); /*0x15024390,   GGM_D1A_GGM_LUT[180] */
            imageioPackage.pDrvDip->writeReg( 0x00003394, 0x3a8ea3a8); /*0x15024394,   GGM_D1A_GGM_LUT[181] */
            imageioPackage.pDrvDip->writeReg( 0x00003398, 0x3b0ec3b0); /*0x15024398,   GGM_D1A_GGM_LUT[182] */
            imageioPackage.pDrvDip->writeReg( 0x0000339c, 0x3b8ee3b8); /*0x1502439c,   GGM_D1A_GGM_LUT[183] */
            imageioPackage.pDrvDip->writeReg( 0x000033a0, 0x3c0f03c0); /*0x150243a0,   GGM_D1A_GGM_LUT[184] */
            imageioPackage.pDrvDip->writeReg( 0x000033a4, 0x3c8f23c8); /*0x150243a4,   GGM_D1A_GGM_LUT[185] */
            imageioPackage.pDrvDip->writeReg( 0x000033a8, 0x3d0f43d0); /*0x150243a8,   GGM_D1A_GGM_LUT[186] */
            imageioPackage.pDrvDip->writeReg( 0x000033ac, 0x3d8f63d8); /*0x150243ac,   GGM_D1A_GGM_LUT[187] */
            imageioPackage.pDrvDip->writeReg( 0x000033b0, 0x3e0f83e0); /*0x150243b0,   GGM_D1A_GGM_LUT[188] */
            imageioPackage.pDrvDip->writeReg( 0x000033b4, 0x3e0f83e0); /*0x150243b4,   GGM_D1A_GGM_LUT[189] */
            imageioPackage.pDrvDip->writeReg( 0x000033b8, 0x3e0f83e0); /*0x150243b8,   GGM_D1A_GGM_LUT[190] */
            imageioPackage.pDrvDip->writeReg( 0x000033bc, 0x3e0f83e0); /*0x150243bc,   GGM_D1A_GGM_LUT[191] */
            imageioPackage.pDrvDip->writeReg( 0x000033c0, 0x100307FF); /*0x150243c0,   GGM_D1A_GGM_CTRL     */
            imageioPackage.pDrvDip->writeReg( 0x000033c4, 0x0);        /*0x150243c4,   GGM_D1A_GGM_SRAM_PINGPONG */   // TODO: GGM_D1A_GGM_SRAM_PINGPONG new

    }
    return 0;
}



MINT32
IspDrv_B::
setWSYNC_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setCCM_D2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){

        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_CCM_D2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_1, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_2, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_3, 0x02000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_4, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_5, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CNV_6, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CTRL,  0x00000009);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CFC_CTRL1, 0x0FDF0020);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D2A_CCM_CFC_CTRL2, 0x1CE729CE);
    }
    return 0;
}



MINT32
IspDrv_B::
setGGM_D2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_GGM_D2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, GGM_D2A_GGM_LUT,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    else
    {
        imageioPackage.pDrvDip->writeReg( 0x00003400, 0x00000000); /*0x15024400,   GGM_D2A_GGM_LUT[0] */
        imageioPackage.pDrvDip->writeReg( 0x00003404, 0x00200802); /*0x15024404,   GGM_D2A_GGM_LUT[1] */
        imageioPackage.pDrvDip->writeReg( 0x00003408, 0x00401004); /*0x15024408,   GGM_D2A_GGM_LUT[2] */
        imageioPackage.pDrvDip->writeReg( 0x0000340c, 0x00601806); /*0x1502440c,   GGM_D2A_GGM_LUT[3] */
        imageioPackage.pDrvDip->writeReg( 0x00003410, 0x00802008); /*0x15024410,   GGM_D2A_GGM_LUT[4] */
        imageioPackage.pDrvDip->writeReg( 0x00003414, 0x00a0280a); /*0x15024414,   GGM_D2A_GGM_LUT[5] */
        imageioPackage.pDrvDip->writeReg( 0x00003418, 0x00c0300c); /*0x15024418,   GGM_D2A_GGM_LUT[6] */
        imageioPackage.pDrvDip->writeReg( 0x0000341c, 0x00e0380e); /*0x1502441c,   GGM_D2A_GGM_LUT[7] */
        imageioPackage.pDrvDip->writeReg( 0x00003420, 0x01004010); /*0x15024420,   GGM_D2A_GGM_LUT[8] */
        imageioPackage.pDrvDip->writeReg( 0x00003424, 0x01204812); /*0x15024424,   GGM_D2A_GGM_LUT[9] */
        imageioPackage.pDrvDip->writeReg( 0x00003428, 0x01405014); /*0x15024428,   GGM_D2A_GGM_LUT[10] */
        imageioPackage.pDrvDip->writeReg( 0x0000342c, 0x01605816); /*0x1502442c,   GGM_D2A_GGM_LUT[11] */
        imageioPackage.pDrvDip->writeReg( 0x00003430, 0x01806018); /*0x15024430,   GGM_D2A_GGM_LUT[12] */
        imageioPackage.pDrvDip->writeReg( 0x00003434, 0x01a0681a); /*0x15024434,   GGM_D2A_GGM_LUT[13] */
        imageioPackage.pDrvDip->writeReg( 0x00003438, 0x01c0701c); /*0x15024438,   GGM_D2A_GGM_LUT[14] */
        imageioPackage.pDrvDip->writeReg( 0x0000343c, 0x01e0781e); /*0x1502443c,   GGM_D2A_GGM_LUT[15] */
        imageioPackage.pDrvDip->writeReg( 0x00003440, 0x02008020); /*0x15024440,   GGM_D2A_GGM_LUT[16] */
        imageioPackage.pDrvDip->writeReg( 0x00003444, 0x02208822); /*0x15024444,   GGM_D2A_GGM_LUT[17] */
        imageioPackage.pDrvDip->writeReg( 0x00003448, 0x02409024); /*0x15024448,   GGM_D2A_GGM_LUT[18] */
        imageioPackage.pDrvDip->writeReg( 0x0000344c, 0x02609826); /*0x1502444c,   GGM_D2A_GGM_LUT[19] */
        imageioPackage.pDrvDip->writeReg( 0x00003450, 0x0280a028); /*0x15024450,   GGM_D2A_GGM_LUT[20] */
        imageioPackage.pDrvDip->writeReg( 0x00003454, 0x02a0a82a); /*0x15024454,   GGM_D2A_GGM_LUT[21] */
        imageioPackage.pDrvDip->writeReg( 0x00003458, 0x02c0b02c); /*0x15024458,   GGM_D2A_GGM_LUT[22] */
        imageioPackage.pDrvDip->writeReg( 0x0000345c, 0x02e0b82e); /*0x1502445c,   GGM_D2A_GGM_LUT[23] */
        imageioPackage.pDrvDip->writeReg( 0x00003460, 0x0300c030); /*0x15024460,   GGM_D2A_GGM_LUT[24] */
        imageioPackage.pDrvDip->writeReg( 0x00003464, 0x0320c832); /*0x15024464,   GGM_D2A_GGM_LUT[25] */
        imageioPackage.pDrvDip->writeReg( 0x00003468, 0x0340d034); /*0x15024468,   GGM_D2A_GGM_LUT[26] */
        imageioPackage.pDrvDip->writeReg( 0x0000346c, 0x0360d836); /*0x1502446c,   GGM_D2A_GGM_LUT[27] */
        imageioPackage.pDrvDip->writeReg( 0x00003470, 0x0380e038); /*0x15024470,   GGM_D2A_GGM_LUT[28] */
        imageioPackage.pDrvDip->writeReg( 0x00003474, 0x03a0e83a); /*0x15024474,   GGM_D2A_GGM_LUT[29] */
        imageioPackage.pDrvDip->writeReg( 0x00003478, 0x03c0f03c); /*0x15024478,   GGM_D2A_GGM_LUT[30] */
        imageioPackage.pDrvDip->writeReg( 0x0000347c, 0x03e0f83e); /*0x1502447c,   GGM_D2A_GGM_LUT[31] */
        imageioPackage.pDrvDip->writeReg( 0x00003480, 0x04010040); /*0x15024480,   GGM_D2A_GGM_LUT[32] */
        imageioPackage.pDrvDip->writeReg( 0x00003484, 0x04210842); /*0x15024484,   GGM_D2A_GGM_LUT[33] */
        imageioPackage.pDrvDip->writeReg( 0x00003488, 0x04411044); /*0x15024488,   GGM_D2A_GGM_LUT[34] */
        imageioPackage.pDrvDip->writeReg( 0x0000348c, 0x04611846); /*0x1502448c,   GGM_D2A_GGM_LUT[35] */
        imageioPackage.pDrvDip->writeReg( 0x00003490, 0x04812048); /*0x15024490,   GGM_D2A_GGM_LUT[36] */
        imageioPackage.pDrvDip->writeReg( 0x00003494, 0x04a1284a); /*0x15024494,   GGM_D2A_GGM_LUT[37] */
        imageioPackage.pDrvDip->writeReg( 0x00003498, 0x04c1304c); /*0x15024498,   GGM_D2A_GGM_LUT[38] */
        imageioPackage.pDrvDip->writeReg( 0x0000349c, 0x04e1384e); /*0x1502449c,   GGM_D2A_GGM_LUT[39] */
        imageioPackage.pDrvDip->writeReg( 0x000034a0, 0x05014050); /*0x150244a0,   GGM_D2A_GGM_LUT[40] */
        imageioPackage.pDrvDip->writeReg( 0x000034a4, 0x05214852); /*0x150244a4,   GGM_D2A_GGM_LUT[41] */
        imageioPackage.pDrvDip->writeReg( 0x000034a8, 0x05415054); /*0x150244a8,   GGM_D2A_GGM_LUT[42] */
        imageioPackage.pDrvDip->writeReg( 0x000034ac, 0x05615856); /*0x150244ac,   GGM_D2A_GGM_LUT[43] */
        imageioPackage.pDrvDip->writeReg( 0x000034b0, 0x05816058); /*0x150244b0,   GGM_D2A_GGM_LUT[44] */
        imageioPackage.pDrvDip->writeReg( 0x000034b4, 0x05a1685a); /*0x150244b4,   GGM_D2A_GGM_LUT[45] */
        imageioPackage.pDrvDip->writeReg( 0x000034b8, 0x05c1705c); /*0x150244b8,   GGM_D2A_GGM_LUT[46] */
        imageioPackage.pDrvDip->writeReg( 0x000034bc, 0x05e1785e); /*0x150244bc,   GGM_D2A_GGM_LUT[47] */
        imageioPackage.pDrvDip->writeReg( 0x000034c0, 0x06018060); /*0x150244c0,   GGM_D2A_GGM_LUT[48] */
        imageioPackage.pDrvDip->writeReg( 0x000034c4, 0x06218862); /*0x150244c4,   GGM_D2A_GGM_LUT[49] */
        imageioPackage.pDrvDip->writeReg( 0x000034c8, 0x06419064); /*0x150244c8,   GGM_D2A_GGM_LUT[50] */
        imageioPackage.pDrvDip->writeReg( 0x000034cc, 0x06619866); /*0x150244cc,   GGM_D2A_GGM_LUT[51] */
        imageioPackage.pDrvDip->writeReg( 0x000034d0, 0x0681a068); /*0x150244d0,   GGM_D2A_GGM_LUT[52] */
        imageioPackage.pDrvDip->writeReg( 0x000034d4, 0x06a1a86a); /*0x150244d4,   GGM_D2A_GGM_LUT[53] */
        imageioPackage.pDrvDip->writeReg( 0x000034d8, 0x06c1b06c); /*0x150244d8,   GGM_D2A_GGM_LUT[54] */
        imageioPackage.pDrvDip->writeReg( 0x000034dc, 0x06e1b86e); /*0x150244dc,   GGM_D2A_GGM_LUT[55] */
        imageioPackage.pDrvDip->writeReg( 0x000034e0, 0x0701c070); /*0x150244e0,   GGM_D2A_GGM_LUT[56] */
        imageioPackage.pDrvDip->writeReg( 0x000034e4, 0x0721c872); /*0x150244e4,   GGM_D2A_GGM_LUT[57] */
        imageioPackage.pDrvDip->writeReg( 0x000034e8, 0x0741d074); /*0x150244e8,   GGM_D2A_GGM_LUT[58] */
        imageioPackage.pDrvDip->writeReg( 0x000034ec, 0x0761d876); /*0x150244ec,   GGM_D2A_GGM_LUT[59] */
        imageioPackage.pDrvDip->writeReg( 0x000034f0, 0x0781e078); /*0x150244f0,   GGM_D2A_GGM_LUT[60] */
        imageioPackage.pDrvDip->writeReg( 0x000034f4, 0x07a1e87a); /*0x150244f4,   GGM_D2A_GGM_LUT[61] */
        imageioPackage.pDrvDip->writeReg( 0x000034f8, 0x07c1f07c); /*0x150244f8,   GGM_D2A_GGM_LUT[62] */
        imageioPackage.pDrvDip->writeReg( 0x000034fc, 0x07e1f87e); /*0x150244fc,   GGM_D2A_GGM_LUT[63] */
        imageioPackage.pDrvDip->writeReg( 0x00003500, 0x08020080); /*0x15024500,   GGM_D2A_GGM_LUT[64] */
        imageioPackage.pDrvDip->writeReg( 0x00003504, 0x08421084); /*0x15024504,   GGM_D2A_GGM_LUT[65] */
        imageioPackage.pDrvDip->writeReg( 0x00003508, 0x08822088); /*0x15024508,   GGM_D2A_GGM_LUT[66] */
        imageioPackage.pDrvDip->writeReg( 0x0000350c, 0x08c2308c); /*0x1502450c,   GGM_D2A_GGM_LUT[67] */
        imageioPackage.pDrvDip->writeReg( 0x00003510, 0x09024090); /*0x15024510,   GGM_D2A_GGM_LUT[68] */
        imageioPackage.pDrvDip->writeReg( 0x00003514, 0x09425094); /*0x15024514,   GGM_D2A_GGM_LUT[69] */
        imageioPackage.pDrvDip->writeReg( 0x00003518, 0x09826098); /*0x15024518,   GGM_D2A_GGM_LUT[70] */
        imageioPackage.pDrvDip->writeReg( 0x0000351c, 0x09c2709c); /*0x1502451c,   GGM_D2A_GGM_LUT[71] */
        imageioPackage.pDrvDip->writeReg( 0x00003520, 0x0a0280a0); /*0x15024520,   GGM_D2A_GGM_LUT[72] */
        imageioPackage.pDrvDip->writeReg( 0x00003524, 0x0a4290a4); /*0x15024524,   GGM_D2A_GGM_LUT[73] */
        imageioPackage.pDrvDip->writeReg( 0x00003528, 0x0a82a0a8); /*0x15024528,   GGM_D2A_GGM_LUT[74] */
        imageioPackage.pDrvDip->writeReg( 0x0000352c, 0x0ac2b0ac); /*0x1502452c,   GGM_D2A_GGM_LUT[75] */
        imageioPackage.pDrvDip->writeReg( 0x00003530, 0x0b02c0b0); /*0x15024530,   GGM_D2A_GGM_LUT[76] */
        imageioPackage.pDrvDip->writeReg( 0x00003534, 0x0b42d0b4); /*0x15024534,   GGM_D2A_GGM_LUT[77] */
        imageioPackage.pDrvDip->writeReg( 0x00003538, 0x0b82e0b8); /*0x15024538,   GGM_D2A_GGM_LUT[78] */
        imageioPackage.pDrvDip->writeReg( 0x0000353c, 0x0bc2f0bc); /*0x1502453c,   GGM_D2A_GGM_LUT[79] */
        imageioPackage.pDrvDip->writeReg( 0x00003540, 0x0c0300c0); /*0x15024540,   GGM_D2A_GGM_LUT[80] */
        imageioPackage.pDrvDip->writeReg( 0x00003544, 0x0c4310c4); /*0x15024544,   GGM_D2A_GGM_LUT[81] */
        imageioPackage.pDrvDip->writeReg( 0x00003548, 0x0c8320c8); /*0x15024548,   GGM_D2A_GGM_LUT[82] */
        imageioPackage.pDrvDip->writeReg( 0x0000354c, 0x0cc330cc); /*0x1502454c,   GGM_D2A_GGM_LUT[83] */
        imageioPackage.pDrvDip->writeReg( 0x00003550, 0x0d0340d0); /*0x15024550,   GGM_D2A_GGM_LUT[84] */
        imageioPackage.pDrvDip->writeReg( 0x00003554, 0x0d4350d4); /*0x15024554,   GGM_D2A_GGM_LUT[85] */
        imageioPackage.pDrvDip->writeReg( 0x00003558, 0x0d8360d8); /*0x15024558,   GGM_D2A_GGM_LUT[86] */
        imageioPackage.pDrvDip->writeReg( 0x0000355c, 0x0dc370dc); /*0x1502455c,   GGM_D2A_GGM_LUT[87] */
        imageioPackage.pDrvDip->writeReg( 0x00003560, 0x0e0380e0); /*0x15024560,   GGM_D2A_GGM_LUT[88] */
        imageioPackage.pDrvDip->writeReg( 0x00003564, 0x0e4390e4); /*0x15024564,   GGM_D2A_GGM_LUT[89] */
        imageioPackage.pDrvDip->writeReg( 0x00003568, 0x0e83a0e8); /*0x15024568,   GGM_D2A_GGM_LUT[90] */
        imageioPackage.pDrvDip->writeReg( 0x0000356c, 0x0ec3b0ec); /*0x1502456c,   GGM_D2A_GGM_LUT[91] */
        imageioPackage.pDrvDip->writeReg( 0x00003570, 0x0f03c0f0); /*0x15024570,   GGM_D2A_GGM_LUT[92] */
        imageioPackage.pDrvDip->writeReg( 0x00003574, 0x0f43d0f4); /*0x15024574,   GGM_D2A_GGM_LUT[93] */
        imageioPackage.pDrvDip->writeReg( 0x00003578, 0x0f83e0f8); /*0x15024578,   GGM_D2A_GGM_LUT[94] */
        imageioPackage.pDrvDip->writeReg( 0x0000357c, 0x0fc3f0fc); /*0x1502457c,   GGM_D2A_GGM_LUT[95] */
        imageioPackage.pDrvDip->writeReg( 0x00003580, 0x10040100); /*0x15024580,   GGM_D2A_GGM_LUT[96] */
        imageioPackage.pDrvDip->writeReg( 0x00003584, 0x10842108); /*0x15024584,   GGM_D2A_GGM_LUT[97] */
        imageioPackage.pDrvDip->writeReg( 0x00003588, 0x11044110); /*0x15024588,   GGM_D2A_GGM_LUT[98] */
        imageioPackage.pDrvDip->writeReg( 0x0000358c, 0x11846118); /*0x1502458c,   GGM_D2A_GGM_LUT[99] */
        imageioPackage.pDrvDip->writeReg( 0x00003590, 0x12048120); /*0x15024590,   GGM_D2A_GGM_LUT[100] */
        imageioPackage.pDrvDip->writeReg( 0x00003594, 0x1284a128); /*0x15024594,   GGM_D2A_GGM_LUT[101] */
        imageioPackage.pDrvDip->writeReg( 0x00003598, 0x1304c130); /*0x15024598,   GGM_D2A_GGM_LUT[102] */
        imageioPackage.pDrvDip->writeReg( 0x0000359c, 0x1384e138); /*0x1502459c,   GGM_D2A_GGM_LUT[103] */
        imageioPackage.pDrvDip->writeReg( 0x000035a0, 0x14050140); /*0x150245a0,   GGM_D2A_GGM_LUT[104] */
        imageioPackage.pDrvDip->writeReg( 0x000035a4, 0x14852148); /*0x150245a4,   GGM_D2A_GGM_LUT[105] */
        imageioPackage.pDrvDip->writeReg( 0x000035a8, 0x15054150); /*0x150245a8,   GGM_D2A_GGM_LUT[106] */
        imageioPackage.pDrvDip->writeReg( 0x000035ac, 0x15856158); /*0x150245ac,   GGM_D2A_GGM_LUT[107] */
        imageioPackage.pDrvDip->writeReg( 0x000035b0, 0x16058160); /*0x150245b0,   GGM_D2A_GGM_LUT[108] */
        imageioPackage.pDrvDip->writeReg( 0x000035b4, 0x1685a168); /*0x150245b4,   GGM_D2A_GGM_LUT[109] */
        imageioPackage.pDrvDip->writeReg( 0x000035b8, 0x1705c170); /*0x150245b8,   GGM_D2A_GGM_LUT[110] */
        imageioPackage.pDrvDip->writeReg( 0x000035bc, 0x1785e178); /*0x150245bc,   GGM_D2A_GGM_LUT[111] */
        imageioPackage.pDrvDip->writeReg( 0x000035c0, 0x18060180); /*0x150245c0,   GGM_D2A_GGM_LUT[112] */
        imageioPackage.pDrvDip->writeReg( 0x000035c4, 0x18862188); /*0x150245c4,   GGM_D2A_GGM_LUT[113] */
        imageioPackage.pDrvDip->writeReg( 0x000035c8, 0x19064190); /*0x150245c8,   GGM_D2A_GGM_LUT[114] */
        imageioPackage.pDrvDip->writeReg( 0x000035cc, 0x19866198); /*0x150245cc,   GGM_D2A_GGM_LUT[115] */
        imageioPackage.pDrvDip->writeReg( 0x000035d0, 0x1a0681a0); /*0x150245d0,   GGM_D2A_GGM_LUT[116] */
        imageioPackage.pDrvDip->writeReg( 0x000035d4, 0x1a86a1a8); /*0x150245d4,   GGM_D2A_GGM_LUT[117] */
        imageioPackage.pDrvDip->writeReg( 0x000035d8, 0x1b06c1b0); /*0x150245d8,   GGM_D2A_GGM_LUT[118] */
        imageioPackage.pDrvDip->writeReg( 0x000035dc, 0x1b86e1b8); /*0x150245dc,   GGM_D2A_GGM_LUT[119] */
        imageioPackage.pDrvDip->writeReg( 0x000035e0, 0x1c0701c0); /*0x150245e0,   GGM_D2A_GGM_LUT[120] */
        imageioPackage.pDrvDip->writeReg( 0x000035e4, 0x1c8721c8); /*0x150245e4,   GGM_D2A_GGM_LUT[121] */
        imageioPackage.pDrvDip->writeReg( 0x000035e8, 0x1d0741d0); /*0x150245e8,   GGM_D2A_GGM_LUT[122] */
        imageioPackage.pDrvDip->writeReg( 0x000035ec, 0x1d8761d8); /*0x150245ec,   GGM_D2A_GGM_LUT[123] */
        imageioPackage.pDrvDip->writeReg( 0x000035f0, 0x1e0781e0); /*0x150245f0,   GGM_D2A_GGM_LUT[124] */
        imageioPackage.pDrvDip->writeReg( 0x000035f4, 0x1e87a1e8); /*0x150245f4,   GGM_D2A_GGM_LUT[125] */
        imageioPackage.pDrvDip->writeReg( 0x000035f8, 0x1f07c1f0); /*0x150245f8,   GGM_D2A_GGM_LUT[126] */
        imageioPackage.pDrvDip->writeReg( 0x000035fc, 0x1f87e1f8); /*0x150245fc,   GGM_D2A_GGM_LUT[127] */
        imageioPackage.pDrvDip->writeReg( 0x00003600, 0x20080200); /*0x15024600,   GGM_D2A_GGM_LUT[128] */
        imageioPackage.pDrvDip->writeReg( 0x00003604, 0x20882208); /*0x15024604,   GGM_D2A_GGM_LUT[129] */
        imageioPackage.pDrvDip->writeReg( 0x00003608, 0x21084210); /*0x15024608,   GGM_D2A_GGM_LUT[130] */
        imageioPackage.pDrvDip->writeReg( 0x0000360c, 0x21886218); /*0x1502460c,   GGM_D2A_GGM_LUT[131] */
        imageioPackage.pDrvDip->writeReg( 0x00003610, 0x22088220); /*0x15024610,   GGM_D2A_GGM_LUT[132] */
        imageioPackage.pDrvDip->writeReg( 0x00003614, 0x2288a228); /*0x15024614,   GGM_D2A_GGM_LUT[133] */
        imageioPackage.pDrvDip->writeReg( 0x00003618, 0x2308c230); /*0x15024618,   GGM_D2A_GGM_LUT[134] */
        imageioPackage.pDrvDip->writeReg( 0x0000361c, 0x2388e238); /*0x1502461c,   GGM_D2A_GGM_LUT[135] */
        imageioPackage.pDrvDip->writeReg( 0x00003620, 0x24090240); /*0x15024620,   GGM_D2A_GGM_LUT[136] */
        imageioPackage.pDrvDip->writeReg( 0x00003624, 0x24892248); /*0x15024624,   GGM_D2A_GGM_LUT[137] */
        imageioPackage.pDrvDip->writeReg( 0x00003628, 0x25094250); /*0x15024628,   GGM_D2A_GGM_LUT[138] */
        imageioPackage.pDrvDip->writeReg( 0x0000362c, 0x25896258); /*0x1502462c,   GGM_D2A_GGM_LUT[139] */
        imageioPackage.pDrvDip->writeReg( 0x00003630, 0x26098260); /*0x15024630,   GGM_D2A_GGM_LUT[140] */
        imageioPackage.pDrvDip->writeReg( 0x00003634, 0x2689a268); /*0x15024634,   GGM_D2A_GGM_LUT[141] */
        imageioPackage.pDrvDip->writeReg( 0x00003638, 0x2709c270); /*0x15024638,   GGM_D2A_GGM_LUT[142] */
        imageioPackage.pDrvDip->writeReg( 0x0000363c, 0x2789e278); /*0x1502463c,   GGM_D2A_GGM_LUT[143] */
        imageioPackage.pDrvDip->writeReg( 0x00003640, 0x280a0280); /*0x15024640,   GGM_D2A_GGM_LUT[144] */
        imageioPackage.pDrvDip->writeReg( 0x00003644, 0x288a2288); /*0x15024644,   GGM_D2A_GGM_LUT[145] */
        imageioPackage.pDrvDip->writeReg( 0x00003648, 0x290a4290); /*0x15024648,   GGM_D2A_GGM_LUT[146] */
        imageioPackage.pDrvDip->writeReg( 0x0000364c, 0x298a6298); /*0x1502464c,   GGM_D2A_GGM_LUT[147] */
        imageioPackage.pDrvDip->writeReg( 0x00003650, 0x2a0a82a0); /*0x15024650,   GGM_D2A_GGM_LUT[148] */
        imageioPackage.pDrvDip->writeReg( 0x00003654, 0x2a8aa2a8); /*0x15024654,   GGM_D2A_GGM_LUT[149] */
        imageioPackage.pDrvDip->writeReg( 0x00003658, 0x2b0ac2b0); /*0x15024658,   GGM_D2A_GGM_LUT[150] */
        imageioPackage.pDrvDip->writeReg( 0x0000365c, 0x2b8ae2b8); /*0x1502465c,   GGM_D2A_GGM_LUT[151] */
        imageioPackage.pDrvDip->writeReg( 0x00003660, 0x2c0b02c0); /*0x15024660,   GGM_D2A_GGM_LUT[152] */
        imageioPackage.pDrvDip->writeReg( 0x00003664, 0x2c8b22c8); /*0x15024664,   GGM_D2A_GGM_LUT[153] */
        imageioPackage.pDrvDip->writeReg( 0x00003668, 0x2d0b42d0); /*0x15024668,   GGM_D2A_GGM_LUT[154] */
        imageioPackage.pDrvDip->writeReg( 0x0000366c, 0x2d8b62d8); /*0x1502466c,   GGM_D2A_GGM_LUT[155] */
        imageioPackage.pDrvDip->writeReg( 0x00003670, 0x2e0b82e0); /*0x15024670,   GGM_D2A_GGM_LUT[156] */
        imageioPackage.pDrvDip->writeReg( 0x00003674, 0x2e8ba2e8); /*0x15024674,   GGM_D2A_GGM_LUT[157] */
        imageioPackage.pDrvDip->writeReg( 0x00003678, 0x2f0bc2f0); /*0x15024678,   GGM_D2A_GGM_LUT[158] */
        imageioPackage.pDrvDip->writeReg( 0x0000367c, 0x2f8be2f8); /*0x1502467c,   GGM_D2A_GGM_LUT[159] */
        imageioPackage.pDrvDip->writeReg( 0x00003680, 0x300c0300); /*0x15024680,   GGM_D2A_GGM_LUT[160] */
        imageioPackage.pDrvDip->writeReg( 0x00003684, 0x308c2308); /*0x15024684,   GGM_D2A_GGM_LUT[161] */
        imageioPackage.pDrvDip->writeReg( 0x00003688, 0x310c4310); /*0x15024688,   GGM_D2A_GGM_LUT[162] */
        imageioPackage.pDrvDip->writeReg( 0x0000368c, 0x318c6318); /*0x1502468c,   GGM_D2A_GGM_LUT[163] */
        imageioPackage.pDrvDip->writeReg( 0x00003690, 0x320c8320); /*0x15024690,   GGM_D2A_GGM_LUT[164] */
        imageioPackage.pDrvDip->writeReg( 0x00003694, 0x328ca328); /*0x15024694,   GGM_D2A_GGM_LUT[165] */
        imageioPackage.pDrvDip->writeReg( 0x00003698, 0x330cc330); /*0x15024698,   GGM_D2A_GGM_LUT[166] */
        imageioPackage.pDrvDip->writeReg( 0x0000369c, 0x338ce338); /*0x1502469c,   GGM_D2A_GGM_LUT[167] */
        imageioPackage.pDrvDip->writeReg( 0x000036a0, 0x340d0340); /*0x150246a0,   GGM_D2A_GGM_LUT[168] */
        imageioPackage.pDrvDip->writeReg( 0x000036a4, 0x348d2348); /*0x150246a4,   GGM_D2A_GGM_LUT[169] */
        imageioPackage.pDrvDip->writeReg( 0x000036a8, 0x350d4350); /*0x150246a8,   GGM_D2A_GGM_LUT[170] */
        imageioPackage.pDrvDip->writeReg( 0x000036ac, 0x358d6358); /*0x150246ac,   GGM_D2A_GGM_LUT[171] */
        imageioPackage.pDrvDip->writeReg( 0x000036b0, 0x360d8360); /*0x150246b0,   GGM_D2A_GGM_LUT[172] */
        imageioPackage.pDrvDip->writeReg( 0x000036b4, 0x368da368); /*0x150246b4,   GGM_D2A_GGM_LUT[173] */
        imageioPackage.pDrvDip->writeReg( 0x000036b8, 0x370dc370); /*0x150246b8,   GGM_D2A_GGM_LUT[174] */
        imageioPackage.pDrvDip->writeReg( 0x000036bc, 0x378de378); /*0x150246bc,   GGM_D2A_GGM_LUT[175] */
        imageioPackage.pDrvDip->writeReg( 0x000036c0, 0x380e0380); /*0x150246c0,   GGM_D2A_GGM_LUT[176] */
        imageioPackage.pDrvDip->writeReg( 0x000036c4, 0x388e2388); /*0x150246c4,   GGM_D2A_GGM_LUT[177] */
        imageioPackage.pDrvDip->writeReg( 0x000036c8, 0x390e4390); /*0x150246c8,   GGM_D2A_GGM_LUT[178] */
        imageioPackage.pDrvDip->writeReg( 0x000036cc, 0x398e6398); /*0x150246cc,   GGM_D2A_GGM_LUT[179] */
        imageioPackage.pDrvDip->writeReg( 0x000036d0, 0x3a0e83a0); /*0x150246d0,   GGM_D2A_GGM_LUT[180] */
        imageioPackage.pDrvDip->writeReg( 0x000036d4, 0x3a8ea3a8); /*0x150246d4,   GGM_D2A_GGM_LUT[181] */
        imageioPackage.pDrvDip->writeReg( 0x000036d8, 0x3b0ec3b0); /*0x150246d8,   GGM_D2A_GGM_LUT[182] */
        imageioPackage.pDrvDip->writeReg( 0x000036dc, 0x3b8ee3b8); /*0x150246dc,   GGM_D2A_GGM_LUT[183] */
        imageioPackage.pDrvDip->writeReg( 0x000036e0, 0x3c0f03c0); /*0x150246e0,   GGM_D2A_GGM_LUT[184] */
        imageioPackage.pDrvDip->writeReg( 0x000036e4, 0x3c8f23c8); /*0x150246e4,   GGM_D2A_GGM_LUT[185] */
        imageioPackage.pDrvDip->writeReg( 0x000036e8, 0x3d0f43d0); /*0x150246e8,   GGM_D2A_GGM_LUT[186] */
        imageioPackage.pDrvDip->writeReg( 0x000036ec, 0x3d8f63d8); /*0x150246ec,   GGM_D2A_GGM_LUT[187] */
        imageioPackage.pDrvDip->writeReg( 0x000036f0, 0x3e0f83e0); /*0x150246f0,   GGM_D2A_GGM_LUT[188] */
        imageioPackage.pDrvDip->writeReg( 0x000036f4, 0x3e0f83e0); /*0x150246f4,   GGM_D2A_GGM_LUT[189] */
        imageioPackage.pDrvDip->writeReg( 0x000036f8, 0x3e0f83e0); /*0x150246f8,   GGM_D2A_GGM_LUT[190] */
        imageioPackage.pDrvDip->writeReg( 0x000036fc, 0x3e0f83e0); /*0x150246fc,   GGM_D2A_GGM_LUT[191] */
        imageioPackage.pDrvDip->writeReg( 0x00003700, 0x100307FF); /*0x15024700,   GGM_D2A_GGM_CTRL */
        imageioPackage.pDrvDip->writeReg( 0x00003704, 0x0);        /*0x15024704,   GGM_D2A_GGM_SRAM_PINGPONG */   // TODO: GGM_D2A_GGM_SRAM_PINGPONG new
    }
    return 0;
}

MINT32
IspDrv_B::
setSMT_D4(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUNP_D4(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setPAK_D4(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setMCRP_D2(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setG2CX_D1(DIP_ISP_PIPE &imageioPackage)
{
    DIP_FUNC_DBG("setG2c in (%d)",imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning == MTRUE) {
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_G2CX_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_0A,size,(MUINT32*)imageioPackage.pTuningIspReg);
    } else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_0A,     0x012D0099);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_0B,     0x0000003A);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_1A,     0x075607AA);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_1B,     0x00000100);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_2A,     0x072A0100);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CONV_2B,     0x000007D6);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_SHADE_CON_1, 0x0118000E);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_SHADE_CON_2, 0x0074B740);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_SHADE_CON_3, 0x00000133);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_SHADE_TAR,   0x079F0A5A);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_SHADE_SP,    0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CFC_CON_1,   0x03f70080);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2CX_D1A_G2CX_CFC_CON_2,   0x1CE539CE);
    }

    return 0;
}


MINT32
IspDrv_B::
setC42_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isp_top_ctl.RGB2_EN.Bits.DIPCTL_SW_P2_IDENDITY_EN)
	{
		DIP_WRITE_REG(imageioPackage.pDrvDip, C42_D1A_C42_CON, 0xf1);
	}
	else
	{
		DIP_WRITE_REG(imageioPackage.pDrvDip, C42_D1A_C42_CON, 0xf0);
	}
	return 0;
}

MINT32
IspDrv_B::
setMIX_D3(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_MIX_D3,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, MIX_D3A_MIX_CTRL0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	} else {
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_Y_EN,0x1);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_Y_DEFAULT,0x0);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_UV_EN,0x1);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_UV_DEFAULT,0x0);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_WT_SEL,0x0);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_B0,0x0);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_B1,0xff);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL0,MIX_DT,0x1);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL1,MIX_M0,0x0);
		DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D3A_MIX_CTRL1,MIX_M1,0xff);
	}
    return 0;
}



MINT32
IspDrv_B::
setYNR_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_YNR_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, YNR_D1A_YNR_TBL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setNDG_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NDG_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, NDG_D1A_NDG_RAN_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}




MINT32
IspDrv_B::
setMIX_D1(DIP_ISP_PIPE &imageioPackage)
{
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_Y_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_Y_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_UV_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_UV_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_WT_SEL,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_B0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_B1,0xff);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL0,MIX_DT,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL1,MIX_M0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D1A_MIX_CTRL1,MIX_M1,0xff);
	//DIP_WRITE_REG(imageioPackage.pDrvDip,MIX_D1A_MIX_SPARE,imageioPackage.pTuningIspReg->MIX_D1A_MIX_SPARE.Raw);

    return 0;
}

MINT32
IspDrv_B::
setC24_D3(DIP_ISP_PIPE &imageioPackage)
{
	DIP_WRITE_REG(imageioPackage.pDrvDip, C24_D3A_C24_TILE_EDGE, 0x0f);  //TODO, justin-yt under confirm
	return 0;
}

MINT32
IspDrv_B::
setC2G_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE)
	{
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_C2G_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_0A,size,(MUINT32*)imageioPackage.pTuningIspReg);
	} else {
		DIP_WRITE_REG(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_0A,     0x00000200);
		DIP_WRITE_REG(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_0B,     0x000002ce);
		DIP_WRITE_REG(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_1A,     0x07500200);
		DIP_WRITE_REG(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_1B,     0x00000692);
		DIP_WRITE_REG(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_2A,     0x038b0200);
		DIP_WRITE_REG(imageioPackage.pDrvDip, C2G_D1A_C2G_CONV_2B,     0x00000000);
	}
	return 0;
}


MINT32
IspDrv_B::
setIGGM_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_IGGM_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, IGGM_D1A_IGGM_LUT_RG,size,(MUINT32*)imageioPackage.pTuningIspReg);
	} else {
		imageioPackage.pDrvDip->writeReg( 0x000050c0, 0x2d8050e); /*0x150260c0,   IGGM_D1A_IGGM_LUT_RG[0] */
		imageioPackage.pDrvDip->writeReg( 0x000050c4, 0xa4602fd); /*0x150260c4,   IGGM_D1A_IGGM_LUT_RG[1] */
		imageioPackage.pDrvDip->writeReg( 0x000050c8, 0xcab04d0); /*0x150260c8,   IGGM_D1A_IGGM_LUT_RG[2] */
		imageioPackage.pDrvDip->writeReg( 0x000050cc, 0x25700e6); /*0x150260cc,   IGGM_D1A_IGGM_LUT_RG[3] */
		imageioPackage.pDrvDip->writeReg( 0x000050d0, 0xa370f17); /*0x150260d0,   IGGM_D1A_IGGM_LUT_RG[4] */
		imageioPackage.pDrvDip->writeReg( 0x000050d4, 0x38907f7); /*0x150260d4,   IGGM_D1A_IGGM_LUT_RG[5] */
		imageioPackage.pDrvDip->writeReg( 0x000050d8, 0xccd0852); /*0x150260d8,   IGGM_D1A_IGGM_LUT_RG[6] */
		imageioPackage.pDrvDip->writeReg( 0x000050dc, 0xacc02f6); /*0x150260dc,   IGGM_D1A_IGGM_LUT_RG[7] */
		imageioPackage.pDrvDip->writeReg( 0x000050e0, 0x5a90021); /*0x150260e0,   IGGM_D1A_IGGM_LUT_RG[8] */
		imageioPackage.pDrvDip->writeReg( 0x000050e4, 0x1b903a8); /*0x150260e4,   IGGM_D1A_IGGM_LUT_RG[9] */
		imageioPackage.pDrvDip->writeReg( 0x000050e8, 0xae90422); /*0x150260e8,   IGGM_D1A_IGGM_LUT_RG[10] */
		imageioPackage.pDrvDip->writeReg( 0x000050ec, 0x9e400a3); /*0x150260ec,   IGGM_D1A_IGGM_LUT_RG[11] */
		imageioPackage.pDrvDip->writeReg( 0x000050f0, 0x2490568); /*0x150260f0,   IGGM_D1A_IGGM_LUT_RG[12] */
		imageioPackage.pDrvDip->writeReg( 0x000050f4, 0x909054b); /*0x150260f4,   IGGM_D1A_IGGM_LUT_RG[13] */
		imageioPackage.pDrvDip->writeReg( 0x000050f8, 0x47c0a5a); /*0x150260f8,   IGGM_D1A_IGGM_LUT_RG[14] */
		imageioPackage.pDrvDip->writeReg( 0x000050fc, 0xbe406e9); /*0x150260fc,   IGGM_D1A_IGGM_LUT_RG[15] */
		imageioPackage.pDrvDip->writeReg( 0x00005100, 0x5d60a69); /*0x15026100,   IGGM_D1A_IGGM_LUT_RG[16] */
		imageioPackage.pDrvDip->writeReg( 0x00005104, 0x5cf02f5); /*0x15026104,   IGGM_D1A_IGGM_LUT_RG[17] */
		imageioPackage.pDrvDip->writeReg( 0x00005108, 0xe420e48); /*0x15026108,   IGGM_D1A_IGGM_LUT_RG[18] */
		imageioPackage.pDrvDip->writeReg( 0x0000510c, 0x444024b); /*0x1502610c,   IGGM_D1A_IGGM_LUT_RG[19] */
		imageioPackage.pDrvDip->writeReg( 0x00005110, 0xf900000); /*0x15026110,   IGGM_D1A_IGGM_LUT_RG[20] */
		imageioPackage.pDrvDip->writeReg( 0x00005114, 0x74d06bb); /*0x15026114,   IGGM_D1A_IGGM_LUT_RG[21] */
		imageioPackage.pDrvDip->writeReg( 0x00005118, 0x26a0ad4); /*0x15026118,   IGGM_D1A_IGGM_LUT_RG[22] */
		imageioPackage.pDrvDip->writeReg( 0x0000511c, 0x29a0ca4); /*0x1502611c,   IGGM_D1A_IGGM_LUT_RG[23] */
		imageioPackage.pDrvDip->writeReg( 0x00005120, 0xa470f72); /*0x15026120,   IGGM_D1A_IGGM_LUT_RG[24] */
		imageioPackage.pDrvDip->writeReg( 0x00005124, 0x18d0f92); /*0x15026124,   IGGM_D1A_IGGM_LUT_RG[25] */
		imageioPackage.pDrvDip->writeReg( 0x00005128, 0xacd0338); /*0x15026128,   IGGM_D1A_IGGM_LUT_RG[26] */
		imageioPackage.pDrvDip->writeReg( 0x0000512c, 0x25d0b04); /*0x1502612c,   IGGM_D1A_IGGM_LUT_RG[27] */
		imageioPackage.pDrvDip->writeReg( 0x00005130, 0xe9d0352); /*0x15026130,   IGGM_D1A_IGGM_LUT_RG[28] */
		imageioPackage.pDrvDip->writeReg( 0x00005134, 0x6f0fe2 ); /*0x15026134,   IGGM_D1A_IGGM_LUT_RG[29] */
		imageioPackage.pDrvDip->writeReg( 0x00005138, 0x8080648); /*0x15026138,   IGGM_D1A_IGGM_LUT_RG[30] */
		imageioPackage.pDrvDip->writeReg( 0x0000513c, 0x75801d0); /*0x1502613c,   IGGM_D1A_IGGM_LUT_RG[31] */
		imageioPackage.pDrvDip->writeReg( 0x00005140, 0xac8059f); /*0x15026140,   IGGM_D1A_IGGM_LUT_RG[32] */
		imageioPackage.pDrvDip->writeReg( 0x00005144, 0xca10fba); /*0x15026144,   IGGM_D1A_IGGM_LUT_RG[33] */
		imageioPackage.pDrvDip->writeReg( 0x00005148, 0x9cf0067); /*0x15026148,   IGGM_D1A_IGGM_LUT_RG[34] */
		imageioPackage.pDrvDip->writeReg( 0x0000514c, 0xa300e45); /*0x1502614c,   IGGM_D1A_IGGM_LUT_RG[35] */
		imageioPackage.pDrvDip->writeReg( 0x00005150, 0x2ff0a76); /*0x15026150,   IGGM_D1A_IGGM_LUT_RG[36] */
		imageioPackage.pDrvDip->writeReg( 0x00005154, 0x5300e0a); /*0x15026154,   IGGM_D1A_IGGM_LUT_RG[37] */
		imageioPackage.pDrvDip->writeReg( 0x00005158, 0x290b5e ); /*0x15026158,   IGGM_D1A_IGGM_LUT_RG[38] */
		imageioPackage.pDrvDip->writeReg( 0x0000515c, 0x9930081); /*0x1502615c,   IGGM_D1A_IGGM_LUT_RG[39] */
		imageioPackage.pDrvDip->writeReg( 0x00005160, 0xfa60efd); /*0x15026160,   IGGM_D1A_IGGM_LUT_RG[40] */
		imageioPackage.pDrvDip->writeReg( 0x00005164, 0x3a0edf ); /*0x15026164,   IGGM_D1A_IGGM_LUT_RG[41] */
		imageioPackage.pDrvDip->writeReg( 0x00005168, 0x3870741); /*0x15026168,   IGGM_D1A_IGGM_LUT_RG[42] */
		imageioPackage.pDrvDip->writeReg( 0x0000516c, 0xef906c9); /*0x1502616c,   IGGM_D1A_IGGM_LUT_RG[43] */
		imageioPackage.pDrvDip->writeReg( 0x00005170, 0x5020fc9); /*0x15026170,   IGGM_D1A_IGGM_LUT_RG[44] */
		imageioPackage.pDrvDip->writeReg( 0x00005174, 0xc2f0e4e); /*0x15026174,   IGGM_D1A_IGGM_LUT_RG[45] */
		imageioPackage.pDrvDip->writeReg( 0x00005178, 0xba70f16); /*0x15026178,   IGGM_D1A_IGGM_LUT_RG[46] */
		imageioPackage.pDrvDip->writeReg( 0x0000517c, 0xdef0f92); /*0x1502617c,   IGGM_D1A_IGGM_LUT_RG[47] */
		imageioPackage.pDrvDip->writeReg( 0x00005180, 0x3560a42); /*0x15026180,   IGGM_D1A_IGGM_LUT_RG[48] */
		imageioPackage.pDrvDip->writeReg( 0x00005184, 0x3690753); /*0x15026184,   IGGM_D1A_IGGM_LUT_RG[49] */
		imageioPackage.pDrvDip->writeReg( 0x00005188, 0xcc102a2); /*0x15026188,   IGGM_D1A_IGGM_LUT_RG[50] */
		imageioPackage.pDrvDip->writeReg( 0x0000518c, 0x7210e5f); /*0x1502618c,   IGGM_D1A_IGGM_LUT_RG[51] */
		imageioPackage.pDrvDip->writeReg( 0x00005190, 0xa400052); /*0x15026190,   IGGM_D1A_IGGM_LUT_RG[52] */
		imageioPackage.pDrvDip->writeReg( 0x00005194, 0xf4e0de1); /*0x15026194,   IGGM_D1A_IGGM_LUT_RG[53] */
		imageioPackage.pDrvDip->writeReg( 0x00005198, 0xc8601f3); /*0x15026198,   IGGM_D1A_IGGM_LUT_RG[54] */
		imageioPackage.pDrvDip->writeReg( 0x0000519c, 0x62b067d); /*0x1502619c,   IGGM_D1A_IGGM_LUT_RG[55] */
		imageioPackage.pDrvDip->writeReg( 0x000051a0, 0xdd30f8e); /*0x150261a0,   IGGM_D1A_IGGM_LUT_RG[56] */
		imageioPackage.pDrvDip->writeReg( 0x000051a4, 0x7c60d70); /*0x150261a4,   IGGM_D1A_IGGM_LUT_RG[57] */
		imageioPackage.pDrvDip->writeReg( 0x000051a8, 0x1630098); /*0x150261a8,   IGGM_D1A_IGGM_LUT_RG[58] */
		imageioPackage.pDrvDip->writeReg( 0x000051ac, 0xe7708ef); /*0x150261ac,   IGGM_D1A_IGGM_LUT_RG[59] */
		imageioPackage.pDrvDip->writeReg( 0x000051b0, 0x100418 ); /*0x150261b0,   IGGM_D1A_IGGM_LUT_RG[60] */
		imageioPackage.pDrvDip->writeReg( 0x000051b4, 0x2ad05b6); /*0x150261b4,   IGGM_D1A_IGGM_LUT_RG[61] */
		imageioPackage.pDrvDip->writeReg( 0x000051b8, 0x82600bb); /*0x150261b8,   IGGM_D1A_IGGM_LUT_RG[62] */
		imageioPackage.pDrvDip->writeReg( 0x000051bc, 0x6aa016d); /*0x150261bc,   IGGM_D1A_IGGM_LUT_RG[63] */
		imageioPackage.pDrvDip->writeReg( 0x000051c0, 0xe3100fc); /*0x150261c0,   IGGM_D1A_IGGM_LUT_RG[64] */
		imageioPackage.pDrvDip->writeReg( 0x000051c4, 0xbf905a0); /*0x150261c4,   IGGM_D1A_IGGM_LUT_RG[65] */
		imageioPackage.pDrvDip->writeReg( 0x000051c8, 0x34f05b8); /*0x150261c8,   IGGM_D1A_IGGM_LUT_RG[66] */
		imageioPackage.pDrvDip->writeReg( 0x000051cc, 0x4dc0200); /*0x150261cc,   IGGM_D1A_IGGM_LUT_RG[67] */
		imageioPackage.pDrvDip->writeReg( 0x000051d0, 0x465073e); /*0x150261d0,   IGGM_D1A_IGGM_LUT_RG[68] */
		imageioPackage.pDrvDip->writeReg( 0x000051d4, 0x8470eb7); /*0x150261d4,   IGGM_D1A_IGGM_LUT_RG[69] */
		imageioPackage.pDrvDip->writeReg( 0x000051d8, 0x40eb2  ); /*0x150261d8,   IGGM_D1A_IGGM_LUT_RG[70] */
		imageioPackage.pDrvDip->writeReg( 0x000051dc, 0x4ce017a); /*0x150261dc,   IGGM_D1A_IGGM_LUT_RG[71] */
		imageioPackage.pDrvDip->writeReg( 0x000051e0, 0x68502fc); /*0x150261e0,   IGGM_D1A_IGGM_LUT_RG[72] */
		imageioPackage.pDrvDip->writeReg( 0x000051e4, 0x65d0906); /*0x150261e4,   IGGM_D1A_IGGM_LUT_RG[73] */
		imageioPackage.pDrvDip->writeReg( 0x000051e8, 0xd1c0176); /*0x150261e8,   IGGM_D1A_IGGM_LUT_RG[74] */
		imageioPackage.pDrvDip->writeReg( 0x000051ec, 0xc9e0572); /*0x150261ec,   IGGM_D1A_IGGM_LUT_RG[75] */
		imageioPackage.pDrvDip->writeReg( 0x000051f0, 0x75c0299); /*0x150261f0,   IGGM_D1A_IGGM_LUT_RG[76] */
		imageioPackage.pDrvDip->writeReg( 0x000051f4, 0x1c807a6); /*0x150261f4,   IGGM_D1A_IGGM_LUT_RG[77] */
		imageioPackage.pDrvDip->writeReg( 0x000051f8, 0xa0f0a2d); /*0x150261f8,   IGGM_D1A_IGGM_LUT_RG[78] */
		imageioPackage.pDrvDip->writeReg( 0x000051fc, 0x53100c9); /*0x150261fc,   IGGM_D1A_IGGM_LUT_RG[79] */
		imageioPackage.pDrvDip->writeReg( 0x00005200, 0x5930078); /*0x15026200,   IGGM_D1A_IGGM_LUT_RG[80] */
		imageioPackage.pDrvDip->writeReg( 0x00005204, 0xc940683); /*0x15026204,   IGGM_D1A_IGGM_LUT_RG[81] */
		imageioPackage.pDrvDip->writeReg( 0x00005208, 0xbd70602); /*0x15026208,   IGGM_D1A_IGGM_LUT_RG[82] */
		imageioPackage.pDrvDip->writeReg( 0x0000520c, 0x41c0fa1); /*0x1502620c,   IGGM_D1A_IGGM_LUT_RG[83] */
		imageioPackage.pDrvDip->writeReg( 0x00005210, 0x9b10a31); /*0x15026210,   IGGM_D1A_IGGM_LUT_RG[84] */
		imageioPackage.pDrvDip->writeReg( 0x00005214, 0xe060eed); /*0x15026214,   IGGM_D1A_IGGM_LUT_RG[85] */
		imageioPackage.pDrvDip->writeReg( 0x00005218, 0x1450155); /*0x15026218,   IGGM_D1A_IGGM_LUT_RG[86] */
		imageioPackage.pDrvDip->writeReg( 0x0000521c, 0x9af01e8); /*0x1502621c,   IGGM_D1A_IGGM_LUT_RG[87] */
		imageioPackage.pDrvDip->writeReg( 0x00005220, 0x24c0139); /*0x15026220,   IGGM_D1A_IGGM_LUT_RG[88] */
		imageioPackage.pDrvDip->writeReg( 0x00005224, 0x5fa021f); /*0x15026224,   IGGM_D1A_IGGM_LUT_RG[89] */
		imageioPackage.pDrvDip->writeReg( 0x00005228, 0xa910dcc); /*0x15026228,   IGGM_D1A_IGGM_LUT_RG[90] */
		imageioPackage.pDrvDip->writeReg( 0x0000522c, 0xe6a0870); /*0x1502622c,   IGGM_D1A_IGGM_LUT_RG[91] */
		imageioPackage.pDrvDip->writeReg( 0x00005230, 0x4640c01); /*0x15026230,   IGGM_D1A_IGGM_LUT_RG[92] */
		imageioPackage.pDrvDip->writeReg( 0x00005234, 0x62b076d); /*0x15026234,   IGGM_D1A_IGGM_LUT_RG[93] */
		imageioPackage.pDrvDip->writeReg( 0x00005238, 0xbf7013c); /*0x15026238,   IGGM_D1A_IGGM_LUT_RG[94] */
		imageioPackage.pDrvDip->writeReg( 0x0000523c, 0xc6a0a8e); /*0x1502623c,   IGGM_D1A_IGGM_LUT_RG[95] */
		imageioPackage.pDrvDip->writeReg( 0x00005240, 0xde0ea8 ); /*0x15026240,   IGGM_D1A_IGGM_LUT_RG[96] */
		imageioPackage.pDrvDip->writeReg( 0x00005244, 0xb1c0fa3); /*0x15026244,   IGGM_D1A_IGGM_LUT_RG[97] */
		imageioPackage.pDrvDip->writeReg( 0x00005248, 0xb6f0927); /*0x15026248,   IGGM_D1A_IGGM_LUT_RG[98] */
		imageioPackage.pDrvDip->writeReg( 0x0000524c, 0xad802ff); /*0x1502624c,   IGGM_D1A_IGGM_LUT_RG[99] */
		imageioPackage.pDrvDip->writeReg( 0x00005250, 0x3870793); /*0x15026250,   IGGM_D1A_IGGM_LUT_RG[100] */
		imageioPackage.pDrvDip->writeReg( 0x00005254, 0x42003dc); /*0x15026254,   IGGM_D1A_IGGM_LUT_RG[101] */
		imageioPackage.pDrvDip->writeReg( 0x00005258, 0xc750698); /*0x15026258,   IGGM_D1A_IGGM_LUT_RG[102] */
		imageioPackage.pDrvDip->writeReg( 0x0000525c, 0x37a0ecc); /*0x1502625c,   IGGM_D1A_IGGM_LUT_RG[103] */
		imageioPackage.pDrvDip->writeReg( 0x00005260, 0xfb9092e); /*0x15026260,   IGGM_D1A_IGGM_LUT_RG[104] */
		imageioPackage.pDrvDip->writeReg( 0x00005264, 0x4820946); /*0x15026264,   IGGM_D1A_IGGM_LUT_RG[105] */
		imageioPackage.pDrvDip->writeReg( 0x00005268, 0x5bd0e79); /*0x15026268,   IGGM_D1A_IGGM_LUT_RG[106] */
		imageioPackage.pDrvDip->writeReg( 0x0000526c, 0x53d0747); /*0x1502626c,   IGGM_D1A_IGGM_LUT_RG[107] */
		imageioPackage.pDrvDip->writeReg( 0x00005270, 0x6e507d5); /*0x15026270,   IGGM_D1A_IGGM_LUT_RG[108] */
		imageioPackage.pDrvDip->writeReg( 0x00005274, 0xc4e0d17); /*0x15026274,   IGGM_D1A_IGGM_LUT_RG[109] */
		imageioPackage.pDrvDip->writeReg( 0x00005278, 0xb330c41); /*0x15026278,   IGGM_D1A_IGGM_LUT_RG[110] */
		imageioPackage.pDrvDip->writeReg( 0x0000527c, 0x140044d); /*0x1502627c,   IGGM_D1A_IGGM_LUT_RG[111] */
		imageioPackage.pDrvDip->writeReg( 0x00005280, 0x35a08b7); /*0x15026280,   IGGM_D1A_IGGM_LUT_RG[112] */
		imageioPackage.pDrvDip->writeReg( 0x00005284, 0x7ec06ac); /*0x15026284,   IGGM_D1A_IGGM_LUT_RG[113] */
		imageioPackage.pDrvDip->writeReg( 0x00005288, 0x5880e42); /*0x15026288,   IGGM_D1A_IGGM_LUT_RG[114] */
		imageioPackage.pDrvDip->writeReg( 0x0000528c, 0x6000027); /*0x1502628c,   IGGM_D1A_IGGM_LUT_RG[115] */
		imageioPackage.pDrvDip->writeReg( 0x00005290, 0xc480c74); /*0x15026290,   IGGM_D1A_IGGM_LUT_RG[116] */
		imageioPackage.pDrvDip->writeReg( 0x00005294, 0xcb70bf8); /*0x15026294,   IGGM_D1A_IGGM_LUT_RG[117] */
		imageioPackage.pDrvDip->writeReg( 0x00005298, 0xaf80c05); /*0x15026298,   IGGM_D1A_IGGM_LUT_RG[118] */
		imageioPackage.pDrvDip->writeReg( 0x0000529c, 0x3d90cd3); /*0x1502629c,   IGGM_D1A_IGGM_LUT_RG[119] */
		imageioPackage.pDrvDip->writeReg( 0x000052a0, 0x9c809c4); /*0x150262a0,   IGGM_D1A_IGGM_LUT_RG[120] */
		imageioPackage.pDrvDip->writeReg( 0x000052a4, 0x33d0220); /*0x150262a4,   IGGM_D1A_IGGM_LUT_RG[121] */
		imageioPackage.pDrvDip->writeReg( 0x000052a8, 0xbfa060a); /*0x150262a8,   IGGM_D1A_IGGM_LUT_RG[122] */
		imageioPackage.pDrvDip->writeReg( 0x000052ac, 0x85005db); /*0x150262ac,   IGGM_D1A_IGGM_LUT_RG[123] */
		imageioPackage.pDrvDip->writeReg( 0x000052b0, 0xa8802f2); /*0x150262b0,   IGGM_D1A_IGGM_LUT_RG[124] */
		imageioPackage.pDrvDip->writeReg( 0x000052b4, 0x8150259); /*0x150262b4,   IGGM_D1A_IGGM_LUT_RG[125] */
		imageioPackage.pDrvDip->writeReg( 0x000052b8, 0x4b0cc4 ); /*0x150262b8,   IGGM_D1A_IGGM_LUT_RG[126] */
		imageioPackage.pDrvDip->writeReg( 0x000052bc, 0x54b0fb1); /*0x150262bc,   IGGM_D1A_IGGM_LUT_RG[127] */
		imageioPackage.pDrvDip->writeReg( 0x000052c0, 0x6d02a9 ); /*0x150262c0,   IGGM_D1A_IGGM_LUT_RG[128] */
		imageioPackage.pDrvDip->writeReg( 0x000052c4, 0x3110413); /*0x150262c4,   IGGM_D1A_IGGM_LUT_RG[129] */
		imageioPackage.pDrvDip->writeReg( 0x000052c8, 0xb290624); /*0x150262c8,   IGGM_D1A_IGGM_LUT_RG[130] */
		imageioPackage.pDrvDip->writeReg( 0x000052cc, 0x10d0391); /*0x150262cc,   IGGM_D1A_IGGM_LUT_RG[131] */
		imageioPackage.pDrvDip->writeReg( 0x000052d0, 0xde70a10); /*0x150262d0,   IGGM_D1A_IGGM_LUT_RG[132] */
		imageioPackage.pDrvDip->writeReg( 0x000052d4, 0x49a01f5); /*0x150262d4,   IGGM_D1A_IGGM_LUT_RG[133] */
		imageioPackage.pDrvDip->writeReg( 0x000052d8, 0xa78069f); /*0x150262d8,   IGGM_D1A_IGGM_LUT_RG[134] */
		imageioPackage.pDrvDip->writeReg( 0x000052dc, 0x6e60591); /*0x150262dc,   IGGM_D1A_IGGM_LUT_RG[135] */
		imageioPackage.pDrvDip->writeReg( 0x000052e0, 0xa0b0a10); /*0x150262e0,   IGGM_D1A_IGGM_LUT_RG[136] */
		imageioPackage.pDrvDip->writeReg( 0x000052e4, 0xf8c0ddc); /*0x150262e4,   IGGM_D1A_IGGM_LUT_RG[137] */
		imageioPackage.pDrvDip->writeReg( 0x000052e8, 0xcf20488); /*0x150262e8,   IGGM_D1A_IGGM_LUT_RG[138] */
		imageioPackage.pDrvDip->writeReg( 0x000052ec, 0xf8b05ef); /*0x150262ec,   IGGM_D1A_IGGM_LUT_RG[139] */
		imageioPackage.pDrvDip->writeReg( 0x000052f0, 0x30906cf); /*0x150262f0,   IGGM_D1A_IGGM_LUT_RG[140] */
		imageioPackage.pDrvDip->writeReg( 0x000052f4, 0xd5f0da6); /*0x150262f4,   IGGM_D1A_IGGM_LUT_RG[141] */
		imageioPackage.pDrvDip->writeReg( 0x000052f8, 0x5c10263); /*0x150262f8,   IGGM_D1A_IGGM_LUT_RG[142] */
		imageioPackage.pDrvDip->writeReg( 0x000052fc, 0xc620745); /*0x150262fc,   IGGM_D1A_IGGM_LUT_RG[143] */
		imageioPackage.pDrvDip->writeReg( 0x00005300, 0xea40a23); /*0x15026300,   IGGM_D1A_IGGM_LUT_RG[144] */
		imageioPackage.pDrvDip->writeReg( 0x00005304, 0xacd0156); /*0x15026304,   IGGM_D1A_IGGM_LUT_RG[145] */
		imageioPackage.pDrvDip->writeReg( 0x00005308, 0x24f0fe8); /*0x15026308,   IGGM_D1A_IGGM_LUT_RG[146] */
		imageioPackage.pDrvDip->writeReg( 0x0000530c, 0xadc0038); /*0x1502630c,   IGGM_D1A_IGGM_LUT_RG[147] */
		imageioPackage.pDrvDip->writeReg( 0x00005310, 0x37a0cbf); /*0x15026310,   IGGM_D1A_IGGM_LUT_RG[148] */
		imageioPackage.pDrvDip->writeReg( 0x00005314, 0x13606bb); /*0x15026314,   IGGM_D1A_IGGM_LUT_RG[149] */
		imageioPackage.pDrvDip->writeReg( 0x00005318, 0xf1a05db); /*0x15026318,   IGGM_D1A_IGGM_LUT_RG[150] */
		imageioPackage.pDrvDip->writeReg( 0x0000531c, 0xc0d08eb); /*0x1502631c,   IGGM_D1A_IGGM_LUT_RG[151] */
		imageioPackage.pDrvDip->writeReg( 0x00005320, 0xecf0eec); /*0x15026320,   IGGM_D1A_IGGM_LUT_RG[152] */
		imageioPackage.pDrvDip->writeReg( 0x00005324, 0x3c10179); /*0x15026324,   IGGM_D1A_IGGM_LUT_RG[153] */
		imageioPackage.pDrvDip->writeReg( 0x00005328, 0xa8f04fc); /*0x15026328,   IGGM_D1A_IGGM_LUT_RG[154] */
		imageioPackage.pDrvDip->writeReg( 0x0000532c, 0x69e0c1f); /*0x1502632c,   IGGM_D1A_IGGM_LUT_RG[155] */
		imageioPackage.pDrvDip->writeReg( 0x00005330, 0x2df0a70); /*0x15026330,   IGGM_D1A_IGGM_LUT_RG[156] */
		imageioPackage.pDrvDip->writeReg( 0x00005334, 0x82505de); /*0x15026334,   IGGM_D1A_IGGM_LUT_RG[157] */
		imageioPackage.pDrvDip->writeReg( 0x00005338, 0x81d0d44); /*0x15026338,   IGGM_D1A_IGGM_LUT_RG[158] */
		imageioPackage.pDrvDip->writeReg( 0x0000533c, 0xf7c0ddd); /*0x1502633c,   IGGM_D1A_IGGM_LUT_RG[159] */
		imageioPackage.pDrvDip->writeReg( 0x00005340, 0xec100b9); /*0x15026340,   IGGM_D1A_IGGM_LUT_RG[160] */
		imageioPackage.pDrvDip->writeReg( 0x00005344, 0x2c10226); /*0x15026344,   IGGM_D1A_IGGM_LUT_RG[161] */
		imageioPackage.pDrvDip->writeReg( 0x00005348, 0xf6c00ac); /*0x15026348,   IGGM_D1A_IGGM_LUT_RG[162] */
		imageioPackage.pDrvDip->writeReg( 0x0000534c, 0x58301f4); /*0x1502634c,   IGGM_D1A_IGGM_LUT_RG[163] */
		imageioPackage.pDrvDip->writeReg( 0x00005350, 0x66d0e9b); /*0x15026350,   IGGM_D1A_IGGM_LUT_RG[164] */
		imageioPackage.pDrvDip->writeReg( 0x00005354, 0xc0307e2); /*0x15026354,   IGGM_D1A_IGGM_LUT_RG[165] */
		imageioPackage.pDrvDip->writeReg( 0x00005358, 0x3d70779); /*0x15026358,   IGGM_D1A_IGGM_LUT_RG[166] */
		imageioPackage.pDrvDip->writeReg( 0x0000535c, 0x61c05da); /*0x1502635c,   IGGM_D1A_IGGM_LUT_RG[167] */
		imageioPackage.pDrvDip->writeReg( 0x00005360, 0xd8d0c32); /*0x15026360,   IGGM_D1A_IGGM_LUT_RG[168] */
		imageioPackage.pDrvDip->writeReg( 0x00005364, 0x26602af); /*0x15026364,   IGGM_D1A_IGGM_LUT_RG[169] */
		imageioPackage.pDrvDip->writeReg( 0x00005368, 0xf8509d7); /*0x15026368,   IGGM_D1A_IGGM_LUT_RG[170] */
		imageioPackage.pDrvDip->writeReg( 0x0000536c, 0xc730309); /*0x1502636c,   IGGM_D1A_IGGM_LUT_RG[171] */
		imageioPackage.pDrvDip->writeReg( 0x00005370, 0xedf0445); /*0x15026370,   IGGM_D1A_IGGM_LUT_RG[172] */
		imageioPackage.pDrvDip->writeReg( 0x00005374, 0xd2e0454); /*0x15026374,   IGGM_D1A_IGGM_LUT_RG[173] */
		imageioPackage.pDrvDip->writeReg( 0x00005378, 0xb270dce); /*0x15026378,   IGGM_D1A_IGGM_LUT_RG[174] */
		imageioPackage.pDrvDip->writeReg( 0x0000537c, 0xcda0fde); /*0x1502637c,   IGGM_D1A_IGGM_LUT_RG[175] */
		imageioPackage.pDrvDip->writeReg( 0x00005380, 0x1f80f30); /*0x15026380,   IGGM_D1A_IGGM_LUT_RG[176] */
		imageioPackage.pDrvDip->writeReg( 0x00005384, 0x88104a9); /*0x15026384,   IGGM_D1A_IGGM_LUT_RG[177] */
		imageioPackage.pDrvDip->writeReg( 0x00005388, 0x1450f8e); /*0x15026388,   IGGM_D1A_IGGM_LUT_RG[178] */
		imageioPackage.pDrvDip->writeReg( 0x0000538c, 0xfb801af); /*0x1502638c,   IGGM_D1A_IGGM_LUT_RG[179] */
		imageioPackage.pDrvDip->writeReg( 0x00005390, 0xdad012b); /*0x15026390,   IGGM_D1A_IGGM_LUT_RG[180] */
		imageioPackage.pDrvDip->writeReg( 0x00005394, 0x5b60cc5); /*0x15026394,   IGGM_D1A_IGGM_LUT_RG[181] */
		imageioPackage.pDrvDip->writeReg( 0x00005398, 0x711007d); /*0x15026398,   IGGM_D1A_IGGM_LUT_RG[182] */
		imageioPackage.pDrvDip->writeReg( 0x0000539c, 0x2b9066c); /*0x1502639c,   IGGM_D1A_IGGM_LUT_RG[183] */
		imageioPackage.pDrvDip->writeReg( 0x000053a0, 0x5a20c47); /*0x150263a0,   IGGM_D1A_IGGM_LUT_RG[184] */
		imageioPackage.pDrvDip->writeReg( 0x000053a4, 0xa1309de); /*0x150263a4,   IGGM_D1A_IGGM_LUT_RG[185] */
		imageioPackage.pDrvDip->writeReg( 0x000053a8, 0x9c10662); /*0x150263a8,   IGGM_D1A_IGGM_LUT_RG[186] */
		imageioPackage.pDrvDip->writeReg( 0x000053ac, 0x299074b); /*0x150263ac,   IGGM_D1A_IGGM_LUT_RG[187] */
		imageioPackage.pDrvDip->writeReg( 0x000053b0, 0x4200b72); /*0x150263b0,   IGGM_D1A_IGGM_LUT_RG[188] */
		imageioPackage.pDrvDip->writeReg( 0x000053b4, 0xc150386); /*0x150263b4,   IGGM_D1A_IGGM_LUT_RG[189] */
		imageioPackage.pDrvDip->writeReg( 0x000053b8, 0x2ef0250); /*0x150263b8,   IGGM_D1A_IGGM_LUT_RG[190] */
		imageioPackage.pDrvDip->writeReg( 0x000053bc, 0x9030b73); /*0x150263bc,   IGGM_D1A_IGGM_LUT_RG[191] */


		imageioPackage.pDrvDip->writeReg( 0x000054c0, 0x3b3); /*0x150264c0,   IGGM_D1A_IGGM_LUT_B[0] */
		imageioPackage.pDrvDip->writeReg( 0x000054c4, 0xe77); /*0x150264c4,   IGGM_D1A_IGGM_LUT_B[1] */
		imageioPackage.pDrvDip->writeReg( 0x000054c8, 0x969); /*0x150264c8,   IGGM_D1A_IGGM_LUT_B[2] */
		imageioPackage.pDrvDip->writeReg( 0x000054cc, 0x872); /*0x150264cc,   IGGM_D1A_IGGM_LUT_B[3] */
		imageioPackage.pDrvDip->writeReg( 0x000054d0, 0x755); /*0x150264d0,   IGGM_D1A_IGGM_LUT_B[4] */
		imageioPackage.pDrvDip->writeReg( 0x000054d4, 0xd6e); /*0x150264d4,   IGGM_D1A_IGGM_LUT_B[5] */
		imageioPackage.pDrvDip->writeReg( 0x000054d8, 0xa2f); /*0x150264d8,   IGGM_D1A_IGGM_LUT_B[6] */
		imageioPackage.pDrvDip->writeReg( 0x000054dc, 0x9fa); /*0x150264dc,   IGGM_D1A_IGGM_LUT_B[7] */
		imageioPackage.pDrvDip->writeReg( 0x000054e0, 0x4f ); /*0x150264e0,   IGGM_D1A_IGGM_LUT_B[8] */
		imageioPackage.pDrvDip->writeReg( 0x000054e4, 0xfee); /*0x150264e4,   IGGM_D1A_IGGM_LUT_B[9] */
		imageioPackage.pDrvDip->writeReg( 0x000054e8, 0xa53); /*0x150264e8,   IGGM_D1A_IGGM_LUT_B[10] */
		imageioPackage.pDrvDip->writeReg( 0x000054ec, 0x936); /*0x150264ec,   IGGM_D1A_IGGM_LUT_B[11] */
		imageioPackage.pDrvDip->writeReg( 0x000054f0, 0xa3 ); /*0x150264f0,   IGGM_D1A_IGGM_LUT_B[12] */
		imageioPackage.pDrvDip->writeReg( 0x000054f4, 0xf3d); /*0x150264f4,   IGGM_D1A_IGGM_LUT_B[13] */
		imageioPackage.pDrvDip->writeReg( 0x000054f8, 0x6bb); /*0x150264f8,   IGGM_D1A_IGGM_LUT_B[14] */
		imageioPackage.pDrvDip->writeReg( 0x000054fc, 0xca0); /*0x150264fc,   IGGM_D1A_IGGM_LUT_B[15] */
		imageioPackage.pDrvDip->writeReg( 0x00005500, 0xb0a); /*0x15026500,   IGGM_D1A_IGGM_LUT_B[16] */
		imageioPackage.pDrvDip->writeReg( 0x00005504, 0x565); /*0x15026504,   IGGM_D1A_IGGM_LUT_B[17] */
		imageioPackage.pDrvDip->writeReg( 0x00005508, 0x52f); /*0x15026508,   IGGM_D1A_IGGM_LUT_B[18] */
		imageioPackage.pDrvDip->writeReg( 0x0000550c, 0x5dd); /*0x1502650c,   IGGM_D1A_IGGM_LUT_B[19] */
		imageioPackage.pDrvDip->writeReg( 0x00005510, 0x13 ); /*0x15026510,   IGGM_D1A_IGGM_LUT_B[20] */
		imageioPackage.pDrvDip->writeReg( 0x00005514, 0x404); /*0x15026514,   IGGM_D1A_IGGM_LUT_B[21] */
		imageioPackage.pDrvDip->writeReg( 0x00005518, 0x99a); /*0x15026518,   IGGM_D1A_IGGM_LUT_B[22] */
		imageioPackage.pDrvDip->writeReg( 0x0000551c, 0x72a); /*0x1502651c,   IGGM_D1A_IGGM_LUT_B[23] */
		imageioPackage.pDrvDip->writeReg( 0x00005520, 0xaf9); /*0x15026520,   IGGM_D1A_IGGM_LUT_B[24] */
		imageioPackage.pDrvDip->writeReg( 0x00005524, 0x8ef); /*0x15026524,   IGGM_D1A_IGGM_LUT_B[25] */
		imageioPackage.pDrvDip->writeReg( 0x00005528, 0xdec); /*0x15026528,   IGGM_D1A_IGGM_LUT_B[26] */
		imageioPackage.pDrvDip->writeReg( 0x0000552c, 0xa6c); /*0x1502652c,   IGGM_D1A_IGGM_LUT_B[27] */
		imageioPackage.pDrvDip->writeReg( 0x00005530, 0x123); /*0x15026530,   IGGM_D1A_IGGM_LUT_B[28] */
		imageioPackage.pDrvDip->writeReg( 0x00005534, 0x852); /*0x15026534,   IGGM_D1A_IGGM_LUT_B[29] */
		imageioPackage.pDrvDip->writeReg( 0x00005538, 0xd7a); /*0x15026538,   IGGM_D1A_IGGM_LUT_B[30] */
		imageioPackage.pDrvDip->writeReg( 0x0000553c, 0xd29); /*0x1502653c,   IGGM_D1A_IGGM_LUT_B[31] */
		imageioPackage.pDrvDip->writeReg( 0x00005540, 0xa18); /*0x15026540,   IGGM_D1A_IGGM_LUT_B[32] */
		imageioPackage.pDrvDip->writeReg( 0x00005544, 0xe77); /*0x15026544,   IGGM_D1A_IGGM_LUT_B[33] */
		imageioPackage.pDrvDip->writeReg( 0x00005548, 0x7fa); /*0x15026548,   IGGM_D1A_IGGM_LUT_B[34] */
		imageioPackage.pDrvDip->writeReg( 0x0000554c, 0x19b); /*0x1502654c,   IGGM_D1A_IGGM_LUT_B[35] */
		imageioPackage.pDrvDip->writeReg( 0x00005550, 0x1ec); /*0x15026550,   IGGM_D1A_IGGM_LUT_B[36] */
		imageioPackage.pDrvDip->writeReg( 0x00005554, 0x885); /*0x15026554,   IGGM_D1A_IGGM_LUT_B[37] */
		imageioPackage.pDrvDip->writeReg( 0x00005558, 0x82e); /*0x15026558,   IGGM_D1A_IGGM_LUT_B[38] */
		imageioPackage.pDrvDip->writeReg( 0x0000555c, 0x62e); /*0x1502655c,   IGGM_D1A_IGGM_LUT_B[39] */
		imageioPackage.pDrvDip->writeReg( 0x00005560, 0xc7c); /*0x15026560,   IGGM_D1A_IGGM_LUT_B[40] */
		imageioPackage.pDrvDip->writeReg( 0x00005564, 0xc76); /*0x15026564,   IGGM_D1A_IGGM_LUT_B[41] */
		imageioPackage.pDrvDip->writeReg( 0x00005568, 0xbd5); /*0x15026568,   IGGM_D1A_IGGM_LUT_B[42] */
		imageioPackage.pDrvDip->writeReg( 0x0000556c, 0xcb7); /*0x1502656c,   IGGM_D1A_IGGM_LUT_B[43] */
		imageioPackage.pDrvDip->writeReg( 0x00005570, 0x607); /*0x15026570,   IGGM_D1A_IGGM_LUT_B[44] */
		imageioPackage.pDrvDip->writeReg( 0x00005574, 0x7b9); /*0x15026574,   IGGM_D1A_IGGM_LUT_B[45] */
		imageioPackage.pDrvDip->writeReg( 0x00005578, 0x5c5); /*0x15026578,   IGGM_D1A_IGGM_LUT_B[46] */
		imageioPackage.pDrvDip->writeReg( 0x0000557c, 0xab8); /*0x1502657c,   IGGM_D1A_IGGM_LUT_B[47] */
		imageioPackage.pDrvDip->writeReg( 0x00005580, 0x39f); /*0x15026580,   IGGM_D1A_IGGM_LUT_B[48] */
		imageioPackage.pDrvDip->writeReg( 0x00005584, 0xde1); /*0x15026584,   IGGM_D1A_IGGM_LUT_B[49] */
		imageioPackage.pDrvDip->writeReg( 0x00005588, 0xbfa); /*0x15026588,   IGGM_D1A_IGGM_LUT_B[50] */
		imageioPackage.pDrvDip->writeReg( 0x0000558c, 0x3f5); /*0x1502658c,   IGGM_D1A_IGGM_LUT_B[51] */
		imageioPackage.pDrvDip->writeReg( 0x00005590, 0x54b); /*0x15026590,   IGGM_D1A_IGGM_LUT_B[52] */
		imageioPackage.pDrvDip->writeReg( 0x00005594, 0xa48); /*0x15026594,   IGGM_D1A_IGGM_LUT_B[53] */
		imageioPackage.pDrvDip->writeReg( 0x00005598, 0x734); /*0x15026598,   IGGM_D1A_IGGM_LUT_B[54] */
		imageioPackage.pDrvDip->writeReg( 0x0000559c, 0xb6d); /*0x1502659c,   IGGM_D1A_IGGM_LUT_B[55] */
		imageioPackage.pDrvDip->writeReg( 0x000055a0, 0x332); /*0x150265a0,   IGGM_D1A_IGGM_LUT_B[56] */
		imageioPackage.pDrvDip->writeReg( 0x000055a4, 0x724); /*0x150265a4,   IGGM_D1A_IGGM_LUT_B[57] */
		imageioPackage.pDrvDip->writeReg( 0x000055a8, 0xbbf); /*0x150265a8,   IGGM_D1A_IGGM_LUT_B[58] */
		imageioPackage.pDrvDip->writeReg( 0x000055ac, 0xa6d); /*0x150265ac,   IGGM_D1A_IGGM_LUT_B[59] */
		imageioPackage.pDrvDip->writeReg( 0x000055b0, 0xbca); /*0x150265b0,   IGGM_D1A_IGGM_LUT_B[60] */
		imageioPackage.pDrvDip->writeReg( 0x000055b4, 0x73e); /*0x150265b4,   IGGM_D1A_IGGM_LUT_B[61] */
		imageioPackage.pDrvDip->writeReg( 0x000055b8, 0xe37); /*0x150265b8,   IGGM_D1A_IGGM_LUT_B[62] */
		imageioPackage.pDrvDip->writeReg( 0x000055bc, 0x638); /*0x150265bc,   IGGM_D1A_IGGM_LUT_B[63] */
		imageioPackage.pDrvDip->writeReg( 0x000055c0, 0x612); /*0x150265c0,   IGGM_D1A_IGGM_LUT_B[64] */
		imageioPackage.pDrvDip->writeReg( 0x000055c4, 0x5ab); /*0x150265c4,   IGGM_D1A_IGGM_LUT_B[65] */
		imageioPackage.pDrvDip->writeReg( 0x000055c8, 0xd66); /*0x150265c8,   IGGM_D1A_IGGM_LUT_B[66] */
		imageioPackage.pDrvDip->writeReg( 0x000055cc, 0x91d); /*0x150265cc,   IGGM_D1A_IGGM_LUT_B[67] */
		imageioPackage.pDrvDip->writeReg( 0x000055d0, 0xbcc); /*0x150265d0,   IGGM_D1A_IGGM_LUT_B[68] */
		imageioPackage.pDrvDip->writeReg( 0x000055d4, 0xdb1); /*0x150265d4,   IGGM_D1A_IGGM_LUT_B[69] */
		imageioPackage.pDrvDip->writeReg( 0x000055d8, 0xb03); /*0x150265d8,   IGGM_D1A_IGGM_LUT_B[70] */
		imageioPackage.pDrvDip->writeReg( 0x000055dc, 0x94d); /*0x150265dc,   IGGM_D1A_IGGM_LUT_B[71] */
		imageioPackage.pDrvDip->writeReg( 0x000055e0, 0xc14); /*0x150265e0,   IGGM_D1A_IGGM_LUT_B[72] */
		imageioPackage.pDrvDip->writeReg( 0x000055e4, 0x8b4); /*0x150265e4,   IGGM_D1A_IGGM_LUT_B[73] */
		imageioPackage.pDrvDip->writeReg( 0x000055e8, 0x1e2); /*0x150265e8,   IGGM_D1A_IGGM_LUT_B[74] */
		imageioPackage.pDrvDip->writeReg( 0x000055ec, 0xdfc); /*0x150265ec,   IGGM_D1A_IGGM_LUT_B[75] */
		imageioPackage.pDrvDip->writeReg( 0x000055f0, 0x5  ); /*0x150265f0,   IGGM_D1A_IGGM_LUT_B[76] */
		imageioPackage.pDrvDip->writeReg( 0x000055f4, 0xecf); /*0x150265f4,   IGGM_D1A_IGGM_LUT_B[77] */
		imageioPackage.pDrvDip->writeReg( 0x000055f8, 0xaf4); /*0x150265f8,   IGGM_D1A_IGGM_LUT_B[78] */
		imageioPackage.pDrvDip->writeReg( 0x000055fc, 0x7fe); /*0x150265fc,   IGGM_D1A_IGGM_LUT_B[79] */
		imageioPackage.pDrvDip->writeReg( 0x00005600, 0x2e9); /*0x15026600,   IGGM_D1A_IGGM_LUT_B[80] */
		imageioPackage.pDrvDip->writeReg( 0x00005604, 0xed2); /*0x15026604,   IGGM_D1A_IGGM_LUT_B[81] */
		imageioPackage.pDrvDip->writeReg( 0x00005608, 0xb71); /*0x15026608,   IGGM_D1A_IGGM_LUT_B[82] */
		imageioPackage.pDrvDip->writeReg( 0x0000560c, 0x51f); /*0x1502660c,   IGGM_D1A_IGGM_LUT_B[83] */
		imageioPackage.pDrvDip->writeReg( 0x00005610, 0x574); /*0x15026610,   IGGM_D1A_IGGM_LUT_B[84] */
		imageioPackage.pDrvDip->writeReg( 0x00005614, 0x9a5); /*0x15026614,   IGGM_D1A_IGGM_LUT_B[85] */
		imageioPackage.pDrvDip->writeReg( 0x00005618, 0x5d8); /*0x15026618,   IGGM_D1A_IGGM_LUT_B[86] */
		imageioPackage.pDrvDip->writeReg( 0x0000561c, 0x88f); /*0x1502661c,   IGGM_D1A_IGGM_LUT_B[87] */
		imageioPackage.pDrvDip->writeReg( 0x00005620, 0x32c); /*0x15026620,   IGGM_D1A_IGGM_LUT_B[88] */
		imageioPackage.pDrvDip->writeReg( 0x00005624, 0xa1e); /*0x15026624,   IGGM_D1A_IGGM_LUT_B[89] */
		imageioPackage.pDrvDip->writeReg( 0x00005628, 0x342); /*0x15026628,   IGGM_D1A_IGGM_LUT_B[90] */
		imageioPackage.pDrvDip->writeReg( 0x0000562c, 0x8bc); /*0x1502662c,   IGGM_D1A_IGGM_LUT_B[91] */
		imageioPackage.pDrvDip->writeReg( 0x00005630, 0x89f); /*0x15026630,   IGGM_D1A_IGGM_LUT_B[92] */
		imageioPackage.pDrvDip->writeReg( 0x00005634, 0x44b); /*0x15026634,   IGGM_D1A_IGGM_LUT_B[93] */
		imageioPackage.pDrvDip->writeReg( 0x00005638, 0x2  ); /*0x15026638,   IGGM_D1A_IGGM_LUT_B[94] */
		imageioPackage.pDrvDip->writeReg( 0x0000563c, 0x10 ); /*0x1502663c,   IGGM_D1A_IGGM_LUT_B[95] */
		imageioPackage.pDrvDip->writeReg( 0x00005640, 0x39d); /*0x15026640,   IGGM_D1A_IGGM_LUT_B[96] */
		imageioPackage.pDrvDip->writeReg( 0x00005644, 0x9d2); /*0x15026644,   IGGM_D1A_IGGM_LUT_B[97] */
		imageioPackage.pDrvDip->writeReg( 0x00005648, 0x7e8); /*0x15026648,   IGGM_D1A_IGGM_LUT_B[98] */
		imageioPackage.pDrvDip->writeReg( 0x0000564c, 0xc02); /*0x1502664c,   IGGM_D1A_IGGM_LUT_B[99] */
		imageioPackage.pDrvDip->writeReg( 0x00005650, 0xae8); /*0x15026650,   IGGM_D1A_IGGM_LUT_B[100] */
		imageioPackage.pDrvDip->writeReg( 0x00005654, 0x60b); /*0x15026654,   IGGM_D1A_IGGM_LUT_B[101] */
		imageioPackage.pDrvDip->writeReg( 0x00005658, 0xc18); /*0x15026658,   IGGM_D1A_IGGM_LUT_B[102] */
		imageioPackage.pDrvDip->writeReg( 0x0000565c, 0xe0 ); /*0x1502665c,   IGGM_D1A_IGGM_LUT_B[103] */
		imageioPackage.pDrvDip->writeReg( 0x00005660, 0x762); /*0x15026660,   IGGM_D1A_IGGM_LUT_B[104] */
		imageioPackage.pDrvDip->writeReg( 0x00005664, 0xc64); /*0x15026664,   IGGM_D1A_IGGM_LUT_B[105] */
		imageioPackage.pDrvDip->writeReg( 0x00005668, 0xf51); /*0x15026668,   IGGM_D1A_IGGM_LUT_B[106] */
		imageioPackage.pDrvDip->writeReg( 0x0000566c, 0x2ec); /*0x1502666c,   IGGM_D1A_IGGM_LUT_B[107] */
		imageioPackage.pDrvDip->writeReg( 0x00005670, 0x215); /*0x15026670,   IGGM_D1A_IGGM_LUT_B[108] */
		imageioPackage.pDrvDip->writeReg( 0x00005674, 0x5b2); /*0x15026674,   IGGM_D1A_IGGM_LUT_B[109] */
		imageioPackage.pDrvDip->writeReg( 0x00005678, 0xea3); /*0x15026678,   IGGM_D1A_IGGM_LUT_B[110] */
		imageioPackage.pDrvDip->writeReg( 0x0000567c, 0xd41); /*0x1502667c,   IGGM_D1A_IGGM_LUT_B[111] */
		imageioPackage.pDrvDip->writeReg( 0x00005680, 0x536); /*0x15026680,   IGGM_D1A_IGGM_LUT_B[112] */
		imageioPackage.pDrvDip->writeReg( 0x00005684, 0xccf); /*0x15026684,   IGGM_D1A_IGGM_LUT_B[113] */
		imageioPackage.pDrvDip->writeReg( 0x00005688, 0x7e7); /*0x15026688,   IGGM_D1A_IGGM_LUT_B[114] */
		imageioPackage.pDrvDip->writeReg( 0x0000568c, 0x195); /*0x1502668c,   IGGM_D1A_IGGM_LUT_B[115] */
		imageioPackage.pDrvDip->writeReg( 0x00005690, 0x2ab); /*0x15026690,   IGGM_D1A_IGGM_LUT_B[116] */
		imageioPackage.pDrvDip->writeReg( 0x00005694, 0xbd1); /*0x15026694,   IGGM_D1A_IGGM_LUT_B[117] */
		imageioPackage.pDrvDip->writeReg( 0x00005698, 0x7b ); /*0x15026698,   IGGM_D1A_IGGM_LUT_B[118] */
		imageioPackage.pDrvDip->writeReg( 0x0000569c, 0x2ba); /*0x1502669c,   IGGM_D1A_IGGM_LUT_B[119] */
		imageioPackage.pDrvDip->writeReg( 0x000056a0, 0x5b7); /*0x150266a0,   IGGM_D1A_IGGM_LUT_B[120] */
		imageioPackage.pDrvDip->writeReg( 0x000056a4, 0xec5); /*0x150266a4,   IGGM_D1A_IGGM_LUT_B[121] */
		imageioPackage.pDrvDip->writeReg( 0x000056a8, 0x19a); /*0x150266a8,   IGGM_D1A_IGGM_LUT_B[122] */
		imageioPackage.pDrvDip->writeReg( 0x000056ac, 0xb7e); /*0x150266ac,   IGGM_D1A_IGGM_LUT_B[123] */
		imageioPackage.pDrvDip->writeReg( 0x000056b0, 0xcab); /*0x150266b0,   IGGM_D1A_IGGM_LUT_B[124] */
		imageioPackage.pDrvDip->writeReg( 0x000056b4, 0x98c); /*0x150266b4,   IGGM_D1A_IGGM_LUT_B[125] */
		imageioPackage.pDrvDip->writeReg( 0x000056b8, 0x8ba); /*0x150266b8,   IGGM_D1A_IGGM_LUT_B[126] */
		imageioPackage.pDrvDip->writeReg( 0x000056bc, 0x6b7); /*0x150266bc,   IGGM_D1A_IGGM_LUT_B[127] */
		imageioPackage.pDrvDip->writeReg( 0x000056c0, 0xca4); /*0x150266c0,   IGGM_D1A_IGGM_LUT_B[128] */
		imageioPackage.pDrvDip->writeReg( 0x000056c4, 0x6d8); /*0x150266c4,   IGGM_D1A_IGGM_LUT_B[129] */
		imageioPackage.pDrvDip->writeReg( 0x000056c8, 0x7d2); /*0x150266c8,   IGGM_D1A_IGGM_LUT_B[130] */
		imageioPackage.pDrvDip->writeReg( 0x000056cc, 0xdd1); /*0x150266cc,   IGGM_D1A_IGGM_LUT_B[131] */
		imageioPackage.pDrvDip->writeReg( 0x000056d0, 0xa65); /*0x150266d0,   IGGM_D1A_IGGM_LUT_B[132] */
		imageioPackage.pDrvDip->writeReg( 0x000056d4, 0x20e); /*0x150266d4,   IGGM_D1A_IGGM_LUT_B[133] */
		imageioPackage.pDrvDip->writeReg( 0x000056d8, 0x38c); /*0x150266d8,   IGGM_D1A_IGGM_LUT_B[134] */
		imageioPackage.pDrvDip->writeReg( 0x000056dc, 0x57a); /*0x150266dc,   IGGM_D1A_IGGM_LUT_B[135] */
		imageioPackage.pDrvDip->writeReg( 0x000056e0, 0x2cf); /*0x150266e0,   IGGM_D1A_IGGM_LUT_B[136] */
		imageioPackage.pDrvDip->writeReg( 0x000056e4, 0x1d8); /*0x150266e4,   IGGM_D1A_IGGM_LUT_B[137] */
		imageioPackage.pDrvDip->writeReg( 0x000056e8, 0x8f1); /*0x150266e8,   IGGM_D1A_IGGM_LUT_B[138] */
		imageioPackage.pDrvDip->writeReg( 0x000056ec, 0x937); /*0x150266ec,   IGGM_D1A_IGGM_LUT_B[139] */
		imageioPackage.pDrvDip->writeReg( 0x000056f0, 0x2bf); /*0x150266f0,   IGGM_D1A_IGGM_LUT_B[140] */
		imageioPackage.pDrvDip->writeReg( 0x000056f4, 0xca6); /*0x150266f4,   IGGM_D1A_IGGM_LUT_B[141] */
		imageioPackage.pDrvDip->writeReg( 0x000056f8, 0x966); /*0x150266f8,   IGGM_D1A_IGGM_LUT_B[142] */
		imageioPackage.pDrvDip->writeReg( 0x000056fc, 0x7f2); /*0x150266fc,   IGGM_D1A_IGGM_LUT_B[143] */
		imageioPackage.pDrvDip->writeReg( 0x00005700, 0x2ba); /*0x15026700,   IGGM_D1A_IGGM_LUT_B[144] */
		imageioPackage.pDrvDip->writeReg( 0x00005704, 0xb9a); /*0x15026704,   IGGM_D1A_IGGM_LUT_B[145] */
		imageioPackage.pDrvDip->writeReg( 0x00005708, 0x39c); /*0x15026708,   IGGM_D1A_IGGM_LUT_B[146] */
		imageioPackage.pDrvDip->writeReg( 0x0000570c, 0xc93); /*0x1502670c,   IGGM_D1A_IGGM_LUT_B[147] */
		imageioPackage.pDrvDip->writeReg( 0x00005710, 0xf92); /*0x15026710,   IGGM_D1A_IGGM_LUT_B[148] */
		imageioPackage.pDrvDip->writeReg( 0x00005714, 0x32b); /*0x15026714,   IGGM_D1A_IGGM_LUT_B[149] */
		imageioPackage.pDrvDip->writeReg( 0x00005718, 0x305); /*0x15026718,   IGGM_D1A_IGGM_LUT_B[150] */
		imageioPackage.pDrvDip->writeReg( 0x0000571c, 0xccc); /*0x1502671c,   IGGM_D1A_IGGM_LUT_B[151] */
		imageioPackage.pDrvDip->writeReg( 0x00005720, 0xa16); /*0x15026720,   IGGM_D1A_IGGM_LUT_B[152] */
		imageioPackage.pDrvDip->writeReg( 0x00005724, 0xc4e); /*0x15026724,   IGGM_D1A_IGGM_LUT_B[153] */
		imageioPackage.pDrvDip->writeReg( 0x00005728, 0x6c4); /*0x15026728,   IGGM_D1A_IGGM_LUT_B[154] */
		imageioPackage.pDrvDip->writeReg( 0x0000572c, 0x379); /*0x1502672c,   IGGM_D1A_IGGM_LUT_B[155] */
		imageioPackage.pDrvDip->writeReg( 0x00005730, 0x46e); /*0x15026730,   IGGM_D1A_IGGM_LUT_B[156] */
		imageioPackage.pDrvDip->writeReg( 0x00005734, 0x61f); /*0x15026734,   IGGM_D1A_IGGM_LUT_B[157] */
		imageioPackage.pDrvDip->writeReg( 0x00005738, 0x865); /*0x15026738,   IGGM_D1A_IGGM_LUT_B[158] */
		imageioPackage.pDrvDip->writeReg( 0x0000573c, 0x945); /*0x1502673c,   IGGM_D1A_IGGM_LUT_B[159] */
		imageioPackage.pDrvDip->writeReg( 0x00005740, 0xa27); /*0x15026740,   IGGM_D1A_IGGM_LUT_B[160] */
		imageioPackage.pDrvDip->writeReg( 0x00005744, 0x943); /*0x15026744,   IGGM_D1A_IGGM_LUT_B[161] */
		imageioPackage.pDrvDip->writeReg( 0x00005748, 0x15e); /*0x15026748,   IGGM_D1A_IGGM_LUT_B[162] */
		imageioPackage.pDrvDip->writeReg( 0x0000574c, 0x3c3); /*0x1502674c,   IGGM_D1A_IGGM_LUT_B[163] */
		imageioPackage.pDrvDip->writeReg( 0x00005750, 0xe93); /*0x15026750,   IGGM_D1A_IGGM_LUT_B[164] */
		imageioPackage.pDrvDip->writeReg( 0x00005754, 0x859); /*0x15026754,   IGGM_D1A_IGGM_LUT_B[165] */
		imageioPackage.pDrvDip->writeReg( 0x00005758, 0x80a); /*0x15026758,   IGGM_D1A_IGGM_LUT_B[166] */
		imageioPackage.pDrvDip->writeReg( 0x0000575c, 0x11c); /*0x1502675c,   IGGM_D1A_IGGM_LUT_B[167] */
		imageioPackage.pDrvDip->writeReg( 0x00005760, 0xf82); /*0x15026760,   IGGM_D1A_IGGM_LUT_B[168] */
		imageioPackage.pDrvDip->writeReg( 0x00005764, 0xfea); /*0x15026764,   IGGM_D1A_IGGM_LUT_B[169] */
		imageioPackage.pDrvDip->writeReg( 0x00005768, 0xb87); /*0x15026768,   IGGM_D1A_IGGM_LUT_B[170] */
		imageioPackage.pDrvDip->writeReg( 0x0000576c, 0x463); /*0x1502676c,   IGGM_D1A_IGGM_LUT_B[171] */
		imageioPackage.pDrvDip->writeReg( 0x00005770, 0x6a2); /*0x15026770,   IGGM_D1A_IGGM_LUT_B[172] */
		imageioPackage.pDrvDip->writeReg( 0x00005774, 0x115); /*0x15026774,   IGGM_D1A_IGGM_LUT_B[173] */
		imageioPackage.pDrvDip->writeReg( 0x00005778, 0x945); /*0x15026778,   IGGM_D1A_IGGM_LUT_B[174] */
		imageioPackage.pDrvDip->writeReg( 0x0000577c, 0x952); /*0x1502677c,   IGGM_D1A_IGGM_LUT_B[175] */
		imageioPackage.pDrvDip->writeReg( 0x00005780, 0x97 ); /*0x15026780,   IGGM_D1A_IGGM_LUT_B[176] */
		imageioPackage.pDrvDip->writeReg( 0x00005784, 0x2a2); /*0x15026784,   IGGM_D1A_IGGM_LUT_B[177] */
		imageioPackage.pDrvDip->writeReg( 0x00005788, 0x2a5); /*0x15026788,   IGGM_D1A_IGGM_LUT_B[178] */
		imageioPackage.pDrvDip->writeReg( 0x0000578c, 0x7c0); /*0x1502678c,   IGGM_D1A_IGGM_LUT_B[179] */
		imageioPackage.pDrvDip->writeReg( 0x00005790, 0x359); /*0x15026790,   IGGM_D1A_IGGM_LUT_B[180] */
		imageioPackage.pDrvDip->writeReg( 0x00005794, 0x281); /*0x15026794,   IGGM_D1A_IGGM_LUT_B[181] */
		imageioPackage.pDrvDip->writeReg( 0x00005798, 0xf89); /*0x15026798,   IGGM_D1A_IGGM_LUT_B[182] */
		imageioPackage.pDrvDip->writeReg( 0x0000579c, 0x614); /*0x1502679c,   IGGM_D1A_IGGM_LUT_B[183] */
		imageioPackage.pDrvDip->writeReg( 0x000057a0, 0x862); /*0x150267a0,   IGGM_D1A_IGGM_LUT_B[184] */
		imageioPackage.pDrvDip->writeReg( 0x000057a4, 0x4f ); /*0x150267a4,   IGGM_D1A_IGGM_LUT_B[185] */
		imageioPackage.pDrvDip->writeReg( 0x000057a8, 0xf69); /*0x150267a8,   IGGM_D1A_IGGM_LUT_B[186] */
		imageioPackage.pDrvDip->writeReg( 0x000057ac, 0xd84); /*0x150267ac,   IGGM_D1A_IGGM_LUT_B[187] */
		imageioPackage.pDrvDip->writeReg( 0x000057b0, 0xb30); /*0x150267b0,   IGGM_D1A_IGGM_LUT_B[188] */
		imageioPackage.pDrvDip->writeReg( 0x000057b4, 0x64 ); /*0x150267b4,   IGGM_D1A_IGGM_LUT_B[189] */
		imageioPackage.pDrvDip->writeReg( 0x000057b8, 0xe74); /*0x150267b8,   IGGM_D1A_IGGM_LUT_B[190] */
		imageioPackage.pDrvDip->writeReg( 0x000057bc, 0xfd7); /*0x150267bc,   IGGM_D1A_IGGM_LUT_B[191] */

		imageioPackage.pDrvDip->writeReg( 0x000058c4,0x1FFF); /*0x150268C4,   IGGM_D1A_IGGM_CTRL       */
	}
    return 0;
}


MINT32
IspDrv_B::
setCCM_D3(DIP_ISP_PIPE &imageioPackage)   //TODO: check CCM D3
{
    if (imageioPackage.isApplyTuning == MTRUE){

        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_CCM_D3,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_1, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_2, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_3, 0x02000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_4, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_5, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CNV_6, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CTRL,  0x00000009);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CFC_CTRL1, 0x0FDF0020);
        DIP_WRITE_REG(imageioPackage.pDrvDip, CCM_D3A_CCM_CFC_CTRL2, 0x1CE729CE);
    }
    return 0;
}

MINT32
IspDrv_B::
setGGM_D3(DIP_ISP_PIPE &imageioPackage)   //TODO: need check default value
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_GGM_D3,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, GGM_D3A_GGM_LUT,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    else
    {
        imageioPackage.pDrvDip->writeReg( 0x00005a80, 0x00000000); /*0x15026a80,   GGM_D3A_GGM_LUT[0] */
		imageioPackage.pDrvDip->writeReg( 0x00005a84, 0x00200802); /*0x15026a84,   GGM_D3A_GGM_LUT[1] */
		imageioPackage.pDrvDip->writeReg( 0x00005a88, 0x00401004); /*0x15026a88,   GGM_D3A_GGM_LUT[2] */
		imageioPackage.pDrvDip->writeReg( 0x00005a8c, 0x00601806); /*0x15026a8c,   GGM_D3A_GGM_LUT[3] */
		imageioPackage.pDrvDip->writeReg( 0x00005a90, 0x00802008); /*0x15026a90,   GGM_D3A_GGM_LUT[4] */
		imageioPackage.pDrvDip->writeReg( 0x00005a94, 0x00a0280a); /*0x15026a94,   GGM_D3A_GGM_LUT[5] */
		imageioPackage.pDrvDip->writeReg( 0x00005a98, 0x00c0300c); /*0x15026a98,   GGM_D3A_GGM_LUT[6] */
		imageioPackage.pDrvDip->writeReg( 0x00005a9c, 0x00e0380e); /*0x15026a9c,   GGM_D3A_GGM_LUT[7] */
		imageioPackage.pDrvDip->writeReg( 0x00005aa0, 0x01004010); /*0x15026aa0,   GGM_D3A_GGM_LUT[8] */
		imageioPackage.pDrvDip->writeReg( 0x00005aa4, 0x01204812); /*0x15026aa4,   GGM_D3A_GGM_LUT[9] */
		imageioPackage.pDrvDip->writeReg( 0x00005aa8, 0x01405014); /*0x15026aa8,   GGM_D3A_GGM_LUT[10] */
		imageioPackage.pDrvDip->writeReg( 0x00005aac, 0x01605816); /*0x15026aac,   GGM_D3A_GGM_LUT[11] */
		imageioPackage.pDrvDip->writeReg( 0x00005ab0, 0x01806018); /*0x15026ab0,   GGM_D3A_GGM_LUT[12] */
		imageioPackage.pDrvDip->writeReg( 0x00005ab4, 0x01a0681a); /*0x15026ab4,   GGM_D3A_GGM_LUT[13] */
		imageioPackage.pDrvDip->writeReg( 0x00005ab8, 0x01c0701c); /*0x15026ab8,   GGM_D3A_GGM_LUT[14] */
		imageioPackage.pDrvDip->writeReg( 0x00005abc, 0x01e0781e); /*0x15026abc,   GGM_D3A_GGM_LUT[15] */
		imageioPackage.pDrvDip->writeReg( 0x00005ac0, 0x02008020); /*0x15026ac0,   GGM_D3A_GGM_LUT[16] */
		imageioPackage.pDrvDip->writeReg( 0x00005ac4, 0x02208822); /*0x15026ac4,   GGM_D3A_GGM_LUT[17] */
		imageioPackage.pDrvDip->writeReg( 0x00005ac8, 0x02409024); /*0x15026ac8,   GGM_D3A_GGM_LUT[18] */
		imageioPackage.pDrvDip->writeReg( 0x00005acc, 0x02609826); /*0x15026acc,   GGM_D3A_GGM_LUT[19] */
		imageioPackage.pDrvDip->writeReg( 0x00005ad0, 0x0280a028); /*0x15026ad0,   GGM_D3A_GGM_LUT[20] */
		imageioPackage.pDrvDip->writeReg( 0x00005ad4, 0x02a0a82a); /*0x15026ad4,   GGM_D3A_GGM_LUT[21] */
		imageioPackage.pDrvDip->writeReg( 0x00005ad8, 0x02c0b02c); /*0x15026ad8,   GGM_D3A_GGM_LUT[22] */
		imageioPackage.pDrvDip->writeReg( 0x00005adc, 0x02e0b82e); /*0x15026adc,   GGM_D3A_GGM_LUT[23] */
		imageioPackage.pDrvDip->writeReg( 0x00005ae0, 0x0300c030); /*0x15026ae0,   GGM_D3A_GGM_LUT[24] */
		imageioPackage.pDrvDip->writeReg( 0x00005ae4, 0x0320c832); /*0x15026ae4,   GGM_D3A_GGM_LUT[25] */
		imageioPackage.pDrvDip->writeReg( 0x00005ae8, 0x0340d034); /*0x15026ae8,   GGM_D3A_GGM_LUT[26] */
		imageioPackage.pDrvDip->writeReg( 0x00005aec, 0x0360d836); /*0x15026aec,   GGM_D3A_GGM_LUT[27] */
		imageioPackage.pDrvDip->writeReg( 0x00005af0, 0x0380e038); /*0x15026af0,   GGM_D3A_GGM_LUT[28] */
		imageioPackage.pDrvDip->writeReg( 0x00005af4, 0x03a0e83a); /*0x15026af4,   GGM_D3A_GGM_LUT[29] */
		imageioPackage.pDrvDip->writeReg( 0x00005af8, 0x03c0f03c); /*0x15026af8,   GGM_D3A_GGM_LUT[30] */
		imageioPackage.pDrvDip->writeReg( 0x00005afc, 0x03e0f83e); /*0x15026afc,   GGM_D3A_GGM_LUT[31] */
		imageioPackage.pDrvDip->writeReg( 0x00005b00, 0x04010040); /*0x15026b00,   GGM_D3A_GGM_LUT[32] */
		imageioPackage.pDrvDip->writeReg( 0x00005b04, 0x04210842); /*0x15026b04,   GGM_D3A_GGM_LUT[33] */
		imageioPackage.pDrvDip->writeReg( 0x00005b08, 0x04411044); /*0x15026b08,   GGM_D3A_GGM_LUT[34] */
		imageioPackage.pDrvDip->writeReg( 0x00005b0c, 0x04611846); /*0x15026b0c,   GGM_D3A_GGM_LUT[35] */
		imageioPackage.pDrvDip->writeReg( 0x00005b10, 0x04812048); /*0x15026b10,   GGM_D3A_GGM_LUT[36] */
		imageioPackage.pDrvDip->writeReg( 0x00005b14, 0x04a1284a); /*0x15026b14,   GGM_D3A_GGM_LUT[37] */
		imageioPackage.pDrvDip->writeReg( 0x00005b18, 0x04c1304c); /*0x15026b18,   GGM_D3A_GGM_LUT[38] */
		imageioPackage.pDrvDip->writeReg( 0x00005b1c, 0x04e1384e); /*0x15026b1c,   GGM_D3A_GGM_LUT[39] */
		imageioPackage.pDrvDip->writeReg( 0x00005b20, 0x05014050); /*0x15026b20,   GGM_D3A_GGM_LUT[40] */
		imageioPackage.pDrvDip->writeReg( 0x00005b24, 0x05214852); /*0x15026b24,   GGM_D3A_GGM_LUT[41] */
		imageioPackage.pDrvDip->writeReg( 0x00005b28, 0x05415054); /*0x15026b28,   GGM_D3A_GGM_LUT[42] */
		imageioPackage.pDrvDip->writeReg( 0x00005b2c, 0x05615856); /*0x15026b2c,   GGM_D3A_GGM_LUT[43] */
		imageioPackage.pDrvDip->writeReg( 0x00005b30, 0x05816058); /*0x15026b30,   GGM_D3A_GGM_LUT[44] */
		imageioPackage.pDrvDip->writeReg( 0x00005b34, 0x05a1685a); /*0x15026b34,   GGM_D3A_GGM_LUT[45] */
		imageioPackage.pDrvDip->writeReg( 0x00005b38, 0x05c1705c); /*0x15026b38,   GGM_D3A_GGM_LUT[46] */
		imageioPackage.pDrvDip->writeReg( 0x00005b3c, 0x05e1785e); /*0x15026b3c,   GGM_D3A_GGM_LUT[47] */
		imageioPackage.pDrvDip->writeReg( 0x00005b40, 0x06018060); /*0x15026b40,   GGM_D3A_GGM_LUT[48] */
		imageioPackage.pDrvDip->writeReg( 0x00005b44, 0x06218862); /*0x15026b44,   GGM_D3A_GGM_LUT[49] */
		imageioPackage.pDrvDip->writeReg( 0x00005b48, 0x06419064); /*0x15026b48,   GGM_D3A_GGM_LUT[50] */
		imageioPackage.pDrvDip->writeReg( 0x00005b4c, 0x06619866); /*0x15026b4c,   GGM_D3A_GGM_LUT[51] */
		imageioPackage.pDrvDip->writeReg( 0x00005b50, 0x0681a068); /*0x15026b50,   GGM_D3A_GGM_LUT[52] */
		imageioPackage.pDrvDip->writeReg( 0x00005b54, 0x06a1a86a); /*0x15026b54,   GGM_D3A_GGM_LUT[53] */
		imageioPackage.pDrvDip->writeReg( 0x00005b58, 0x06c1b06c); /*0x15026b58,   GGM_D3A_GGM_LUT[54] */
		imageioPackage.pDrvDip->writeReg( 0x00005b5c, 0x06e1b86e); /*0x15026b5c,   GGM_D3A_GGM_LUT[55] */
		imageioPackage.pDrvDip->writeReg( 0x00005b60, 0x0701c070); /*0x15026b60,   GGM_D3A_GGM_LUT[56] */
		imageioPackage.pDrvDip->writeReg( 0x00005b64, 0x0721c872); /*0x15026b64,   GGM_D3A_GGM_LUT[57] */
		imageioPackage.pDrvDip->writeReg( 0x00005b68, 0x0741d074); /*0x15026b68,   GGM_D3A_GGM_LUT[58] */
		imageioPackage.pDrvDip->writeReg( 0x00005b6c, 0x0761d876); /*0x15026b6c,   GGM_D3A_GGM_LUT[59] */
		imageioPackage.pDrvDip->writeReg( 0x00005b70, 0x0781e078); /*0x15026b70,   GGM_D3A_GGM_LUT[60] */
		imageioPackage.pDrvDip->writeReg( 0x00005b74, 0x07a1e87a); /*0x15026b74,   GGM_D3A_GGM_LUT[61] */
		imageioPackage.pDrvDip->writeReg( 0x00005b78, 0x07c1f07c); /*0x15026b78,   GGM_D3A_GGM_LUT[62] */
		imageioPackage.pDrvDip->writeReg( 0x00005b7c, 0x07e1f87e); /*0x15026b7c,   GGM_D3A_GGM_LUT[63] */
		imageioPackage.pDrvDip->writeReg( 0x00005b80, 0x08020080); /*0x15026b80,   GGM_D3A_GGM_LUT[64] */
		imageioPackage.pDrvDip->writeReg( 0x00005b84, 0x08421084); /*0x15026b84,   GGM_D3A_GGM_LUT[65] */
		imageioPackage.pDrvDip->writeReg( 0x00005b88, 0x08822088); /*0x15026b88,   GGM_D3A_GGM_LUT[66] */
		imageioPackage.pDrvDip->writeReg( 0x00005b8c, 0x08c2308c); /*0x15026b8c,   GGM_D3A_GGM_LUT[67] */
		imageioPackage.pDrvDip->writeReg( 0x00005b90, 0x09024090); /*0x15026b90,   GGM_D3A_GGM_LUT[68] */
		imageioPackage.pDrvDip->writeReg( 0x00005b94, 0x09425094); /*0x15026b94,   GGM_D3A_GGM_LUT[69] */
		imageioPackage.pDrvDip->writeReg( 0x00005b98, 0x09826098); /*0x15026b98,   GGM_D3A_GGM_LUT[70] */
		imageioPackage.pDrvDip->writeReg( 0x00005b9c, 0x09c2709c); /*0x15026b9c,   GGM_D3A_GGM_LUT[71] */
		imageioPackage.pDrvDip->writeReg( 0x00005ba0, 0x0a0280a0); /*0x15026ba0,   GGM_D3A_GGM_LUT[72] */
		imageioPackage.pDrvDip->writeReg( 0x00005ba4, 0x0a4290a4); /*0x15026ba4,   GGM_D3A_GGM_LUT[73] */
		imageioPackage.pDrvDip->writeReg( 0x00005ba8, 0x0a82a0a8); /*0x15026ba8,   GGM_D3A_GGM_LUT[74] */
		imageioPackage.pDrvDip->writeReg( 0x00005bac, 0x0ac2b0ac); /*0x15026bac,   GGM_D3A_GGM_LUT[75] */
		imageioPackage.pDrvDip->writeReg( 0x00005bb0, 0x0b02c0b0); /*0x15026bb0,   GGM_D3A_GGM_LUT[76] */
		imageioPackage.pDrvDip->writeReg( 0x00005bb4, 0x0b42d0b4); /*0x15026bb4,   GGM_D3A_GGM_LUT[77] */
		imageioPackage.pDrvDip->writeReg( 0x00005bb8, 0x0b82e0b8); /*0x15026bb8,   GGM_D3A_GGM_LUT[78] */
		imageioPackage.pDrvDip->writeReg( 0x00005bbc, 0x0bc2f0bc); /*0x15026bbc,   GGM_D3A_GGM_LUT[79] */
		imageioPackage.pDrvDip->writeReg( 0x00005bc0, 0x0c0300c0); /*0x15026bc0,   GGM_D3A_GGM_LUT[80] */
		imageioPackage.pDrvDip->writeReg( 0x00005bc4, 0x0c4310c4); /*0x15026bc4,   GGM_D3A_GGM_LUT[81] */
		imageioPackage.pDrvDip->writeReg( 0x00005bc8, 0x0c8320c8); /*0x15026bc8,   GGM_D3A_GGM_LUT[82] */
		imageioPackage.pDrvDip->writeReg( 0x00005bcc, 0x0cc330cc); /*0x15026bcc,   GGM_D3A_GGM_LUT[83] */
		imageioPackage.pDrvDip->writeReg( 0x00005bd0, 0x0d0340d0); /*0x15026bd0,   GGM_D3A_GGM_LUT[84] */
		imageioPackage.pDrvDip->writeReg( 0x00005bd4, 0x0d4350d4); /*0x15026bd4,   GGM_D3A_GGM_LUT[85] */
		imageioPackage.pDrvDip->writeReg( 0x00005bd8, 0x0d8360d8); /*0x15026bd8,   GGM_D3A_GGM_LUT[86] */
		imageioPackage.pDrvDip->writeReg( 0x00005bdc, 0x0dc370dc); /*0x15026bdc,   GGM_D3A_GGM_LUT[87] */
		imageioPackage.pDrvDip->writeReg( 0x00005be0, 0x0e0380e0); /*0x15026be0,   GGM_D3A_GGM_LUT[88] */
		imageioPackage.pDrvDip->writeReg( 0x00005be4, 0x0e4390e4); /*0x15026be4,   GGM_D3A_GGM_LUT[89] */
		imageioPackage.pDrvDip->writeReg( 0x00005be8, 0x0e83a0e8); /*0x15026be8,   GGM_D3A_GGM_LUT[90] */
		imageioPackage.pDrvDip->writeReg( 0x00005bec, 0x0ec3b0ec); /*0x15026bec,   GGM_D3A_GGM_LUT[91] */
		imageioPackage.pDrvDip->writeReg( 0x00005bf0, 0x0f03c0f0); /*0x15026bf0,   GGM_D3A_GGM_LUT[92] */
		imageioPackage.pDrvDip->writeReg( 0x00005bf4, 0x0f43d0f4); /*0x15026bf4,   GGM_D3A_GGM_LUT[93] */
		imageioPackage.pDrvDip->writeReg( 0x00005bf8, 0x0f83e0f8); /*0x15026bf8,   GGM_D3A_GGM_LUT[94] */
		imageioPackage.pDrvDip->writeReg( 0x00005bfc, 0x0fc3f0fc); /*0x15026bfc,   GGM_D3A_GGM_LUT[95] */
		imageioPackage.pDrvDip->writeReg( 0x00005c00, 0x10040100); /*0x15026c00,   GGM_D3A_GGM_LUT[96] */
		imageioPackage.pDrvDip->writeReg( 0x00005c04, 0x10842108); /*0x15026c04,   GGM_D3A_GGM_LUT[97] */
		imageioPackage.pDrvDip->writeReg( 0x00005c08, 0x11044110); /*0x15026c08,   GGM_D3A_GGM_LUT[98] */
		imageioPackage.pDrvDip->writeReg( 0x00005c0c, 0x11846118); /*0x15026c0c,   GGM_D3A_GGM_LUT[99] */
		imageioPackage.pDrvDip->writeReg( 0x00005c10, 0x12048120); /*0x15026c10,   GGM_D3A_GGM_LUT[100] */
		imageioPackage.pDrvDip->writeReg( 0x00005c14, 0x1284a128); /*0x15026c14,   GGM_D3A_GGM_LUT[101] */
		imageioPackage.pDrvDip->writeReg( 0x00005c18, 0x1304c130); /*0x15026c18,   GGM_D3A_GGM_LUT[102] */
		imageioPackage.pDrvDip->writeReg( 0x00005c1c, 0x1384e138); /*0x15026c1c,   GGM_D3A_GGM_LUT[103] */
		imageioPackage.pDrvDip->writeReg( 0x00005c20, 0x14050140); /*0x15026c20,   GGM_D3A_GGM_LUT[104] */
		imageioPackage.pDrvDip->writeReg( 0x00005c24, 0x14852148); /*0x15026c24,   GGM_D3A_GGM_LUT[105] */
		imageioPackage.pDrvDip->writeReg( 0x00005c28, 0x15054150); /*0x15026c28,   GGM_D3A_GGM_LUT[106] */
		imageioPackage.pDrvDip->writeReg( 0x00005c2c, 0x15856158); /*0x15026c2c,   GGM_D3A_GGM_LUT[107] */
		imageioPackage.pDrvDip->writeReg( 0x00005c30, 0x16058160); /*0x15026c30,   GGM_D3A_GGM_LUT[108] */
		imageioPackage.pDrvDip->writeReg( 0x00005c34, 0x1685a168); /*0x15026c34,   GGM_D3A_GGM_LUT[109] */
		imageioPackage.pDrvDip->writeReg( 0x00005c38, 0x1705c170); /*0x15026c38,   GGM_D3A_GGM_LUT[110] */
		imageioPackage.pDrvDip->writeReg( 0x00005c3c, 0x1785e178); /*0x15026c3c,   GGM_D3A_GGM_LUT[111] */
		imageioPackage.pDrvDip->writeReg( 0x00005c40, 0x18060180); /*0x15026c40,   GGM_D3A_GGM_LUT[112] */
		imageioPackage.pDrvDip->writeReg( 0x00005c44, 0x18862188); /*0x15026c44,   GGM_D3A_GGM_LUT[113] */
		imageioPackage.pDrvDip->writeReg( 0x00005c48, 0x19064190); /*0x15026c48,   GGM_D3A_GGM_LUT[114] */
		imageioPackage.pDrvDip->writeReg( 0x00005c4c, 0x19866198); /*0x15026c4c,   GGM_D3A_GGM_LUT[115] */
		imageioPackage.pDrvDip->writeReg( 0x00005c50, 0x1a0681a0); /*0x15026c50,   GGM_D3A_GGM_LUT[116] */
		imageioPackage.pDrvDip->writeReg( 0x00005c54, 0x1a86a1a8); /*0x15026c54,   GGM_D3A_GGM_LUT[117] */
		imageioPackage.pDrvDip->writeReg( 0x00005c58, 0x1b06c1b0); /*0x15026c58,   GGM_D3A_GGM_LUT[118] */
		imageioPackage.pDrvDip->writeReg( 0x00005c5c, 0x1b86e1b8); /*0x15026c5c,   GGM_D3A_GGM_LUT[119] */
		imageioPackage.pDrvDip->writeReg( 0x00005c60, 0x1c0701c0); /*0x15026c60,   GGM_D3A_GGM_LUT[120] */
		imageioPackage.pDrvDip->writeReg( 0x00005c64, 0x1c8721c8); /*0x15026c64,   GGM_D3A_GGM_LUT[121] */
		imageioPackage.pDrvDip->writeReg( 0x00005c68, 0x1d0741d0); /*0x15026c68,   GGM_D3A_GGM_LUT[122] */
		imageioPackage.pDrvDip->writeReg( 0x00005c6c, 0x1d8761d8); /*0x15026c6c,   GGM_D3A_GGM_LUT[123] */
		imageioPackage.pDrvDip->writeReg( 0x00005c70, 0x1e0781e0); /*0x15026c70,   GGM_D3A_GGM_LUT[124] */
		imageioPackage.pDrvDip->writeReg( 0x00005c74, 0x1e87a1e8); /*0x15026c74,   GGM_D3A_GGM_LUT[125] */
		imageioPackage.pDrvDip->writeReg( 0x00005c78, 0x1f07c1f0); /*0x15026c78,   GGM_D3A_GGM_LUT[126] */
		imageioPackage.pDrvDip->writeReg( 0x00005c7c, 0x1f87e1f8); /*0x15026c7c,   GGM_D3A_GGM_LUT[127] */
		imageioPackage.pDrvDip->writeReg( 0x00005c80, 0x20080200); /*0x15026c80,   GGM_D3A_GGM_LUT[128] */
		imageioPackage.pDrvDip->writeReg( 0x00005c84, 0x20882208); /*0x15026c84,   GGM_D3A_GGM_LUT[129] */
		imageioPackage.pDrvDip->writeReg( 0x00005c88, 0x21084210); /*0x15026c88,   GGM_D3A_GGM_LUT[130] */
		imageioPackage.pDrvDip->writeReg( 0x00005c8c, 0x21886218); /*0x15026c8c,   GGM_D3A_GGM_LUT[131] */
		imageioPackage.pDrvDip->writeReg( 0x00005c90, 0x22088220); /*0x15026c90,   GGM_D3A_GGM_LUT[132] */
		imageioPackage.pDrvDip->writeReg( 0x00005c94, 0x2288a228); /*0x15026c94,   GGM_D3A_GGM_LUT[133] */
		imageioPackage.pDrvDip->writeReg( 0x00005c98, 0x2308c230); /*0x15026c98,   GGM_D3A_GGM_LUT[134] */
		imageioPackage.pDrvDip->writeReg( 0x00005c9c, 0x2388e238); /*0x15026c9c,   GGM_D3A_GGM_LUT[135] */
		imageioPackage.pDrvDip->writeReg( 0x00005ca0, 0x24090240); /*0x15026ca0,   GGM_D3A_GGM_LUT[136] */
		imageioPackage.pDrvDip->writeReg( 0x00005ca4, 0x24892248); /*0x15026ca4,   GGM_D3A_GGM_LUT[137] */
		imageioPackage.pDrvDip->writeReg( 0x00005ca8, 0x25094250); /*0x15026ca8,   GGM_D3A_GGM_LUT[138] */
		imageioPackage.pDrvDip->writeReg( 0x00005cac, 0x25896258); /*0x15026cac,   GGM_D3A_GGM_LUT[139] */
		imageioPackage.pDrvDip->writeReg( 0x00005cb0, 0x26098260); /*0x15026cb0,   GGM_D3A_GGM_LUT[140] */
		imageioPackage.pDrvDip->writeReg( 0x00005cb4, 0x2689a268); /*0x15026cb4,   GGM_D3A_GGM_LUT[141] */
		imageioPackage.pDrvDip->writeReg( 0x00005cb8, 0x2709c270); /*0x15026cb8,   GGM_D3A_GGM_LUT[142] */
		imageioPackage.pDrvDip->writeReg( 0x00005cbc, 0x2789e278); /*0x15026cbc,   GGM_D3A_GGM_LUT[143] */
		imageioPackage.pDrvDip->writeReg( 0x00005cc0, 0x280a0280); /*0x15026cc0,   GGM_D3A_GGM_LUT[144] */
		imageioPackage.pDrvDip->writeReg( 0x00005cc4, 0x288a2288); /*0x15026cc4,   GGM_D3A_GGM_LUT[145] */
		imageioPackage.pDrvDip->writeReg( 0x00005cc8, 0x290a4290); /*0x15026cc8,   GGM_D3A_GGM_LUT[146] */
		imageioPackage.pDrvDip->writeReg( 0x00005ccc, 0x298a6298); /*0x15026ccc,   GGM_D3A_GGM_LUT[147] */
		imageioPackage.pDrvDip->writeReg( 0x00005cd0, 0x2a0a82a0); /*0x15026cd0,   GGM_D3A_GGM_LUT[148] */
		imageioPackage.pDrvDip->writeReg( 0x00005cd4, 0x2a8aa2a8); /*0x15026cd4,   GGM_D3A_GGM_LUT[149] */
		imageioPackage.pDrvDip->writeReg( 0x00005cd8, 0x2b0ac2b0); /*0x15026cd8,   GGM_D3A_GGM_LUT[150] */
		imageioPackage.pDrvDip->writeReg( 0x00005cdc, 0x2b8ae2b8); /*0x15026cdc,   GGM_D3A_GGM_LUT[151] */
		imageioPackage.pDrvDip->writeReg( 0x00005ce0, 0x2c0b02c0); /*0x15026ce0,   GGM_D3A_GGM_LUT[152] */
		imageioPackage.pDrvDip->writeReg( 0x00005ce4, 0x2c8b22c8); /*0x15026ce4,   GGM_D3A_GGM_LUT[153] */
		imageioPackage.pDrvDip->writeReg( 0x00005ce8, 0x2d0b42d0); /*0x15026ce8,   GGM_D3A_GGM_LUT[154] */
		imageioPackage.pDrvDip->writeReg( 0x00005cec, 0x2d8b62d8); /*0x15026cec,   GGM_D3A_GGM_LUT[155] */
		imageioPackage.pDrvDip->writeReg( 0x00005cf0, 0x2e0b82e0); /*0x15026cf0,   GGM_D3A_GGM_LUT[156] */
		imageioPackage.pDrvDip->writeReg( 0x00005cf4, 0x2e8ba2e8); /*0x15026cf4,   GGM_D3A_GGM_LUT[157] */
		imageioPackage.pDrvDip->writeReg( 0x00005cf8, 0x2f0bc2f0); /*0x15026cf8,   GGM_D3A_GGM_LUT[158] */
		imageioPackage.pDrvDip->writeReg( 0x00005cfc, 0x2f8be2f8); /*0x15026cfc,   GGM_D3A_GGM_LUT[159] */
		imageioPackage.pDrvDip->writeReg( 0x00005d00, 0x300c0300); /*0x15026d00,   GGM_D3A_GGM_LUT[160] */
		imageioPackage.pDrvDip->writeReg( 0x00005d04, 0x308c2308); /*0x15026d04,   GGM_D3A_GGM_LUT[161] */
		imageioPackage.pDrvDip->writeReg( 0x00005d08, 0x310c4310); /*0x15026d08,   GGM_D3A_GGM_LUT[162] */
		imageioPackage.pDrvDip->writeReg( 0x00005d0c, 0x318c6318); /*0x15026d0c,   GGM_D3A_GGM_LUT[163] */
		imageioPackage.pDrvDip->writeReg( 0x00005d10, 0x320c8320); /*0x15026d10,   GGM_D3A_GGM_LUT[164] */
		imageioPackage.pDrvDip->writeReg( 0x00005d14, 0x328ca328); /*0x15026d14,   GGM_D3A_GGM_LUT[165] */
		imageioPackage.pDrvDip->writeReg( 0x00005d18, 0x330cc330); /*0x15026d18,   GGM_D3A_GGM_LUT[166] */
		imageioPackage.pDrvDip->writeReg( 0x00005d1c, 0x338ce338); /*0x15026d1c,   GGM_D3A_GGM_LUT[167] */
		imageioPackage.pDrvDip->writeReg( 0x00005d20, 0x340d0340); /*0x15026d20,   GGM_D3A_GGM_LUT[168] */
		imageioPackage.pDrvDip->writeReg( 0x00005d24, 0x348d2348); /*0x15026d24,   GGM_D3A_GGM_LUT[169] */
		imageioPackage.pDrvDip->writeReg( 0x00005d28, 0x350d4350); /*0x15026d28,   GGM_D3A_GGM_LUT[170] */
		imageioPackage.pDrvDip->writeReg( 0x00005d2c, 0x358d6358); /*0x15026d2c,   GGM_D3A_GGM_LUT[171] */
		imageioPackage.pDrvDip->writeReg( 0x00005d30, 0x360d8360); /*0x15026d30,   GGM_D3A_GGM_LUT[172] */
		imageioPackage.pDrvDip->writeReg( 0x00005d34, 0x368da368); /*0x15026d34,   GGM_D3A_GGM_LUT[173] */
		imageioPackage.pDrvDip->writeReg( 0x00005d38, 0x370dc370); /*0x15026d38,   GGM_D3A_GGM_LUT[174] */
		imageioPackage.pDrvDip->writeReg( 0x00005d3c, 0x378de378); /*0x15026d3c,   GGM_D3A_GGM_LUT[175] */
		imageioPackage.pDrvDip->writeReg( 0x00005d40, 0x380e0380); /*0x15026d40,   GGM_D3A_GGM_LUT[176] */
		imageioPackage.pDrvDip->writeReg( 0x00005d44, 0x388e2388); /*0x15026d44,   GGM_D3A_GGM_LUT[177] */
		imageioPackage.pDrvDip->writeReg( 0x00005d48, 0x390e4390); /*0x15026d48,   GGM_D3A_GGM_LUT[178] */
		imageioPackage.pDrvDip->writeReg( 0x00005d4c, 0x398e6398); /*0x15026d4c,   GGM_D3A_GGM_LUT[179] */
		imageioPackage.pDrvDip->writeReg( 0x00005d50, 0x3a0e83a0); /*0x15026d50,   GGM_D3A_GGM_LUT[180] */
		imageioPackage.pDrvDip->writeReg( 0x00005d54, 0x3a8ea3a8); /*0x15026d54,   GGM_D3A_GGM_LUT[181] */
		imageioPackage.pDrvDip->writeReg( 0x00005d58, 0x3b0ec3b0); /*0x15026d58,   GGM_D3A_GGM_LUT[182] */
		imageioPackage.pDrvDip->writeReg( 0x00005d5c, 0x3b8ee3b8); /*0x15026d5c,   GGM_D3A_GGM_LUT[183] */
		imageioPackage.pDrvDip->writeReg( 0x00005d60, 0x3c0f03c0); /*0x15026d60,   GGM_D3A_GGM_LUT[184] */
		imageioPackage.pDrvDip->writeReg( 0x00005d64, 0x3c8f23c8); /*0x15026d64,   GGM_D3A_GGM_LUT[185] */
		imageioPackage.pDrvDip->writeReg( 0x00005d68, 0x3d0f43d0); /*0x15026d68,   GGM_D3A_GGM_LUT[186] */
		imageioPackage.pDrvDip->writeReg( 0x00005d6c, 0x3d8f63d8); /*0x15026d6c,   GGM_D3A_GGM_LUT[187] */
		imageioPackage.pDrvDip->writeReg( 0x00005d70, 0x3e0f83e0); /*0x15026d70,   GGM_D3A_GGM_LUT[188] */
		imageioPackage.pDrvDip->writeReg( 0x00005d74, 0x3e0f83e0); /*0x15026d74,   GGM_D3A_GGM_LUT[189] */
		imageioPackage.pDrvDip->writeReg( 0x00005d78, 0x3e0f83e0); /*0x15026d78,   GGM_D3A_GGM_LUT[190] */
		imageioPackage.pDrvDip->writeReg( 0x00005d7c, 0x3e0f83e0); /*0x15026d7c,   GGM_D3A_GGM_LUT[191] */
		imageioPackage.pDrvDip->writeReg( 0x00005d80, 0x100307FF); /*0x15026d80,   GGM_D3A_GGM_LUT[192] */
		imageioPackage.pDrvDip->writeReg( 0x00005d84, 0x0);        /*0x15026d84,   GGM_D3A_GGM_SRAM_PINGPONG */   // TODO: GGM_D3A_GGM_SRAM_PINGPONG new
    }
    return 0;
}


MINT32
IspDrv_B::
setDCE_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DCE_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DCE_D1A_DCE_TC_G1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setDCES_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DCES_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DCES_D1A_DCES_DC_CTRL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}


MINT32
IspDrv_B::
setG2C_D1(DIP_ISP_PIPE &imageioPackage)
{
    DIP_FUNC_DBG("setG2c in (%d)",imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning == MTRUE) {
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_G2C_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_0A,size,(MUINT32*)imageioPackage.pTuningIspReg);
    } else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_0A, 0x012D0099);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_0B, 0x0000003A);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_1A, 0x075607AA);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_1B, 0x00000100);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_2A, 0x072A0100);
        DIP_WRITE_REG(imageioPackage.pDrvDip, G2C_D1A_G2C_CONV_2B, 0x000007D6);
    }

    return 0;
}


MINT32
IspDrv_B::
setC42_D2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isp_top_ctl.RGB2_EN.Bits.DIPCTL_SW_P2_IDENDITY_EN)
	{
		DIP_WRITE_REG(imageioPackage.pDrvDip, C42_D2A_C42_CON, 0xf1);
	}
	else
	{
		DIP_WRITE_REG(imageioPackage.pDrvDip, C42_D2A_C42_CON, 0xf0);
	}

	return 0;
}

MINT32
IspDrv_B::
setEE_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_EE_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, EE_D1A_EE_CTRL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setSMT_D2(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUNP_D2(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setPAK_D2(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setCNR_D1(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_CNR_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, CNR_D1A_CNR_CNR_CON1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setNDG_D2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NDG_D2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, NDG_D2A_NDG_RAN_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setSMT_D3(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUNP_D3(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setPAK_D3(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setCOLOR_D1(DIP_ISP_PIPE &imageioPackage)  // OK
{
    if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
        if ((imageioPackage.pTuningIspReg->COLOR_D1A_COLOR_DIP_COLOR_START.Bits.COLOR_disp_color_start & 0x1) > 0)
        {
            imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_COLOR_D1,addr,size);
            DIP_WRITE_REGS(imageioPackage.pDrvDip, COLOR_D1A_COLOR_DIP_COLOR_CFG_MAIN,size,(MUINT32*)imageioPackage.pTuningIspReg);
        }
	}
	return 0;
}


MINT32
IspDrv_B::
setMIX_D2(DIP_ISP_PIPE &imageioPackage)
{
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_Y_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_Y_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_UV_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_UV_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_WT_SEL,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_B0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_B1,0xff);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL0,MIX_DT,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL1,MIX_M0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,MIX_D2A_MIX_CTRL1,MIX_M1,0xff);
	//DIP_WRITE_REG(imageioPackage.pDrvDip,MIX_D2A_MIX_SPARE,imageioPackage.pTuningIspReg->MIX_D2A_MIX_SPARE.Raw);

    return 0;
}

MINT32
IspDrv_B::
setMIX_D4(DIP_ISP_PIPE &imageioPackage)
{

	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_MIX_D4,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, MIX_D4A_MIX_CTRL0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setCRP_D1(DIP_ISP_PIPE &imageioPackage)
{

    DIP_WRITE_REG(imageioPackage.pDrvDip,CRP_D1A_CRP_X_POS,((imageioPackage.DMAImg3o.dma_cfg.size.w - 1) & 0xffff <<16));
    DIP_WRITE_REG(imageioPackage.pDrvDip,CRP_D1A_CRP_Y_POS,((imageioPackage.DMAImg3o.dma_cfg.size.h - 1) & 0xffff <<16));//ySize;
    return 0;
}


MINT32
IspDrv_B::
setNR3D_D1(DIP_ISP_PIPE &imageioPackage)  //TOD: tuning module name?
{

	if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NR3D_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, NR3D_D1A_NR3D_NR3D_ENG_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setCRSP_D1(DIP_ISP_PIPE &imageioPackage)
{

	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CTRL,CRSP_HORI_EN,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CTRL,CRSP_VERT_EN,0x1);//the same with crsp_en

	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_STEP_OFST,CRSP_STEP_X,0x4);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_STEP_OFST,CRSP_STEP_Y,0x4);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_STEP_OFST,CRSP_OFST_X,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_STEP_OFST,CRSP_OFST_Y,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_OUT_IMG,CRSP_WD,imageioPackage.crspCfg.out.w);//desImgW);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_OUT_IMG,CRSP_HT,imageioPackage.crspCfg.out.h);//desImgH);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CTRL,CRSP_CROP_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CROP_X,CRSP_CROP_XSTART,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CROP_X,CRSP_CROP_XEND,imageioPackage.crspCfg.out.w-1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CROP_Y,CRSP_CROP_YSTART,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,CRSP_D1A_CRSP_CROP_Y,CRSP_CROP_YEND,imageioPackage.crspCfg.out.h-1);

    return 0;
}

MINT32
IspDrv_B::
setPAK_D6(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setPAK_D7(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setPAK_D8(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setC24_D2(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip, C24_D2A_C24_TILE_EDGE, 0xf); //default(0x0f)
    return 0;
}

MINT32
IspDrv_B::
setMCRP_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setCRZ_D1(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setSRZ_D1(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_CONTROL,imageioPackage.srz1Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D1A_SRZ_IN_IMG,SRZ_IN_WD,imageioPackage.srz1Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D1A_SRZ_IN_IMG,SRZ_IN_HT,imageioPackage.srz1Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D1A_SRZ_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz1Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D1A_SRZ_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz1Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_HORI_STEP,imageioPackage.srz1Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_VERT_STEP,imageioPackage.srz1Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_HORI_INT_OFST,imageioPackage.srz1Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_HORI_SUB_OFST,imageioPackage.srz1Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_VERT_INT_OFST,imageioPackage.srz1Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D1A_SRZ_VERT_SUB_OFST,imageioPackage.srz1Cfg.crop.floatY);

    return 0;
}

MINT32
IspDrv_B::
setSRZ_D3(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_CONTROL,imageioPackage.srz3Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D3A_SRZ_IN_IMG,SRZ_IN_WD,imageioPackage.srz3Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D3A_SRZ_IN_IMG,SRZ_IN_HT,imageioPackage.srz3Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D3A_SRZ_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz3Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D3A_SRZ_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz3Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_HORI_STEP,imageioPackage.srz3Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_VERT_STEP,imageioPackage.srz3Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_HORI_INT_OFST,imageioPackage.srz3Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_HORI_SUB_OFST,imageioPackage.srz3Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_VERT_INT_OFST,imageioPackage.srz3Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D3A_SRZ_VERT_SUB_OFST,imageioPackage.srz3Cfg.crop.floatY);
    return 0;
}
MINT32
IspDrv_B::
setSRZ_D4(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_CONTROL,imageioPackage.srz4Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D4A_SRZ_IN_IMG,SRZ_IN_WD,imageioPackage.srz4Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D4A_SRZ_IN_IMG,SRZ_IN_HT,imageioPackage.srz4Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D4A_SRZ_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz4Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,SRZ_D4A_SRZ_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz4Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_HORI_STEP,imageioPackage.srz4Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_VERT_STEP,imageioPackage.srz4Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_HORI_INT_OFST,imageioPackage.srz4Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_HORI_SUB_OFST,imageioPackage.srz4Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_VERT_INT_OFST,imageioPackage.srz4Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,SRZ_D4A_SRZ_VERT_SUB_OFST,imageioPackage.srz4Cfg.crop.floatY);

    return 0;
}

MINT32
IspDrv_B::
setUNP_D6(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUNP_D7(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUNP_D8(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setC02_D2(DIP_ISP_PIPE &imageioPackage)
{
    // TODO: under check about C02/C02B
    //INTERP_MODE of C02 and C02B = 1 <==request from algo team.
    DIP_WRITE_REG(imageioPackage.pDrvDip, C02_D2A_C02_CON, 0x01f);
    DIP_WRITE_REG(imageioPackage.pDrvDip, C02_D2A_C02_CROP_CON1, 0x00);
    DIP_WRITE_REG(imageioPackage.pDrvDip, C02_D2A_C02_CROP_CON2, 0x00);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D2A_C02_CROP_CON1,C02_CROP_XSTART,0x0);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D2A_C02_CROP_CON1,C02_CROP_XEND,imageioPackage.DMAImgi.dma_cfg.size.xsize-1);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D2A_C02_CROP_CON2,C02_CROP_YSTART,0x0);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D2A_C02_CROP_CON2,C02_CROP_YEND,imageioPackage.DMAImgi.dma_cfg.size.h-1);

    return 0;
}

MINT32
IspDrv_B::
setC24_D1(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip, C24_D1A_C24_TILE_EDGE, 0x0f);
    return 0;
}

MINT32
IspDrv_B::
setUNP_D9(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setUNP_D10(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setUNP_D11(DIP_ISP_PIPE &imageioPackage)
{
    (void)imageioPackage;
    return 0;
}



MINT32
IspDrv_B::
setLCE_D1(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LCE_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, LCE_D1A_LCE_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}

	DIP_WRITE_BITS(imageioPackage.pDrvDip,LCE_D1A_LCE_IMAGE_SIZE, LCE_IMAGE_WD, imageioPackage.DMAImgi.dma_cfg.size.w);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,LCE_D1A_LCE_IMAGE_SIZE, LCE_IMAGE_HT, imageioPackage.DMAImgi.dma_cfg.size.h);

    return 0;
}


MINT32
IspDrv_B::
setC02_D1(DIP_ISP_PIPE &imageioPackage)
{
    // TODO(isp50): under check about C02/C02B
    //INTERP_MODE of C02 and C02B = 1 <==request from algo team.
	DIP_WRITE_REG(imageioPackage.pDrvDip, C02_D1A_C02_CON, 0x1f); //default 0x1f, tpipemain would update it, C02_INTERP_MODE = 1
	DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D1A_C02_CROP_CON1,C02_CROP_XSTART,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D1A_C02_CROP_CON1,C02_CROP_XEND,imageioPackage.DMAVipi.dma_cfg.size.xsize-1);//.srcImgW-1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D1A_C02_CROP_CON2,C02_CROP_YSTART,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,C02_D1A_C02_CROP_CON2,C02_CROP_YEND,imageioPackage.DMAVipi.dma_cfg.size.h-1);//srcImgH-1);

    return 0;
}

MINT32
IspDrv_B::
setSLK_D2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SLK_D2,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, SLK_D2A_SLK_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setSLK_D3(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SLK_D3,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, SLK_D3A_SLK_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setSLK_D4(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SLK_D4,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, SLK_D4A_SLK_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setSLK_D5(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SLK_D5,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, SLK_D5A_SLK_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}




MINT32
IspDrv_B::
setLPCNR_D1(DIP_ISP_PIPE &imageioPackage)
{

    if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LPCNR_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, LPCNR_D1A_LPCNR_TOP,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }
    else
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_PASS_MODE,0x1); // 0:pass1 & 1:pass2
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_LBIT_MODE,0x1); // 0:10b & 1:8b
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_lpcnr_reset,0x0);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_lpcnr_dcm_dis,0x0);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_eot_seq_mode,0x0);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_stat_sof_clr,0x0);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_lpcnr_core_en,0x1); // else for debug
        DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_TOP,LPCNR_lpcnr_path_sel,0x0); // else for debug
        DIP_WRITE_REG(imageioPackage.pDrvDip, LPCNR_D1A_LPCNR_ATPG,0x0);
    }
    DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_SIZE,LPCNR_IN_HSIZE,imageioPackage.DMADepi.dma_cfg.size.w/2);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,LPCNR_D1A_LPCNR_SIZE,LPCNR_IN_VSIZE,imageioPackage.DMADepi.dma_cfg.size.h);
    return 0;
}

MINT32
IspDrv_B::
setBS_D1(DIP_ISP_PIPE &imageioPackage) // Todo : Justin-yt Under confirm
{
    (void)imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setDFE_D1(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE)
    {
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DFE_D1,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DFE_D1A_DFE_FE_CTRL1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }
    else
    {//default set fe_mode as 1
        DIP_WRITE_REG(imageioPackage.pDrvDip,DFE_D1A_DFE_FE_CTRL1, 0xAD);
    }
    return 0;
}




MINT32
IspDrv_B::
setFM_D1(DIP_ISP_PIPE &imageioPackage)
{
// TODO(isp50): under check about dmgi, depi and mfbo
    if (imageioPackage.isApplyTuning == MTRUE)
    {
        DIP_WRITE_REG(imageioPackage.pDrvDip,FM_D1A_FM_SIZE_CTRL,imageioPackage.pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,FM_D1A_FM_TH_CON0_CTRL,imageioPackage.pTuningIspReg->FM_D1A_FM_TH_CON0_CTRL.Raw);
    }
    DIP_WRITE_REG(imageioPackage.pDrvDip,DMGI_D1A_DMGI_CON,0x10000019);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DMGI_D1A_DMGI_CON2,0x00160010);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DMGI_D1A_DMGI_CON3,0x00000000);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DEPI_D1A_DEPI_CON,0x20000020);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DEPI_D1A_DEPI_CON2,0x001B000F);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DEPI_D1A_DEPI_CON3,0x001F000C);
//(isp50)take care about the below setting. (pak2o)
    //DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MFBO_CON,0x80000040);
    //DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MFBO_CON2,0x000A0008);
    //DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MFBO_CON3,0x00080008);

    return 0;
}





/*/////////////////////////////////////////////////////////////////////////////
    IspDrvShellImp
/////////////////////////////////////////////////////////////////////////////*/
class IspDrvShellImp:public IspDrvShell
{
    public:
        IspDrvShellImp();

        static IspDrvShell*    getInstance();
        virtual void           destroyInstance(void);
        virtual MBOOL          init(const char* userName="", MUINT32 secTag=0);
        virtual MBOOL          uninit(const char* userName="");
        virtual DipRingBuffer* getDipRingBufMgr();

    protected:
        volatile MINT32 mInitCount;

};

//
IspDrvShell* IspDrvShell::createInstance()
{
    return IspDrvShellImp::getInstance();
}

IspDrvShell*
IspDrvShellImp::
getInstance()
{
    static IspDrvShellImp singleton;
    return &singleton;
}
//
void
IspDrvShellImp::
destroyInstance(void)
{

}

IspDrvShellImp::IspDrvShellImp()
{
    m_bInitAllocSmxBuf = true;
    mInitCount = 0;
}

MBOOL
IspDrvShellImp::
init(const char* userName, MUINT32 secTag)
{
    MINT32 ret = MTRUE;
    MINT32 i;
    MUINTPTR tmpVaddr = 0, tmpPaddr = 0;
    Mutex::Autolock lock(mLock);

    DIP_FUNC_INF("DIP: +, userName(%s), mInitCount(%d)", userName, this->mInitCount);
    m_iDipMetEn = ::property_get_int32("vendor.cam.dipmetlog.enable", 0);
    m_bDCMEn = ::property_get_int32("vendor.camera.DCM.disable", 1);
    m_dumpDipHwRegEn= ::property_get_int32("vendor.camera.dumpDipHwReg.enable", 0);
    m_dumpDipHwRegBySceID = ::property_get_int32("vendor.camera.dumpDipHwRegBySceID.enable", -1);
    m_dumpDipHwTpipeReg = ::property_get_int32("vendor.camera.dumpDipHwTpipeReg.enable", 1);
    if(this->mInitCount > 0)
    {
        android_atomic_inc(&this->mInitCount);
        ret = MTRUE;
        goto EXIT;
    }
    DBG_LOG_CONFIG(imageio, function_Dip);
    // do initial setting
    /*============================================
        imem driver
    =============================================*/
    m_pIMemDrv = DipIMemDrv::createInstance();
    DIP_FUNC_DBG("[m_pIMemDrv]m_pIMemDrv(%lx)",(unsigned long)m_pIMemDrv);
    if ( NULL == m_pIMemDrv ) {
        DIP_FUNC_ERR("[Error]DipIMemDrv::createInstance fail.");
        ret = MFALSE;
        goto EXIT;
    }
    m_pIMemDrv->init();


    /*============================================
        ispDrvDip & ispDrvDipPhy driver
    =============================================*/
    for(i=DIP_HW_A; i<DIP_HW_MAX; i++) {
        // generate IspDrvDipPhy obj
        m_pDrvDipPhy = (PhyDipDrv*)DipDrv::createInstance((DIP_HW_MODULE)i);
        DIP_FUNC_INF("DIP: init IspDrvShellImp");
        m_pDrvDipPhy->init("IspDrvShellImp");
        //
        // generate IspDrvDip obj
        // TODO: hungwen modify
        m_pDrvDip[i] = (VirDipDrv*)DipDrv::getVirDipDrvInstance((DIP_HW_MODULE)i, m_pDrvDipPhy->m_pIspRegMap);
        m_pDrvDip[i]->init("IspDrvShellImp");
    }

	// set registers to hardware register directly
    //when mfb dcm is enabled, the scenario realted mfb will happen seldom hang in last tile.
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV_DCM_DIS, 0x00000001);//kk test (read clear or write one clear??)
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV2_DCM_DIS, 0x00000008);//kk test (read clear or write one clear??)
    //Disable SMX DCM & UFD/UFDI/IMGI DCM
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_RGB_DCM_DIS, 0x02000002);
    ///DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV2_DCM_DIS, 0x00000300);
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_DMA_DCM_DIS, 0x03FC0009);
	if (m_bDCMEn)
    {
        DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_RGB_DCM_DIS1, 0xffffffff);
        DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_RGB_DCM_DIS2, 0xffffffff);
        DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_YUV_DCM_DIS1, 0xffffffff);
        DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_YUV_DCM_DIS2, 0xffffffff);
        DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_DMA_DCM_DIS1, 0xffffffff);
        DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_DMA_DCM_DIS2, 0xffffffff);
        //DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIPCTL_D1A_DIPCTL_TOP_DCM_DIS, 0xffffffff);
    }

    /*============================================
     control MDP buffer & obj
    =============================================*/
    { // control mdp object
        m_pMdpMgr = MdpMgr::createInstance();
        if(m_pMdpMgr == NULL){
            DIP_FUNC_ERR("[Error]fail to create MDP instance");
            ret = MFALSE;
            goto EXIT;
        }
        if(MDPMGR_NO_ERROR != m_pMdpMgr->init()){
            DIP_FUNC_ERR("[Error]m_pMdpMgr->init fail");
            ret = MFALSE;
            goto EXIT;
        }
    }

    m_pDipRingBuf = DipRingBuffer::createInstance((DIP_HW_MODULE)DIP_HW_A);
    if(m_pDipRingBuf)
    {
        ret=m_pDipRingBuf->init(m_bInitAllocSmxBuf, secTag);
    }
    //Initial Debug Dump Buffer
    m_pDipRingBuf->getTpipeBuf(tmpVaddr, tmpPaddr);
    m_pDrvDipPhy->setMemInfo(DIP_MEMORY_INFO_TPIPE_CMD, (unsigned long)tmpPaddr,(unsigned int *)tmpVaddr,MAX_ISP_TILE_TDR_HEX_NO);
    m_pDipRingBuf->getCmdqBuf(tmpVaddr, tmpPaddr);
    m_pDrvDipPhy->setMemInfo(DIP_MEMORY_INFO_CMDQ_CMD, (unsigned long)tmpPaddr,(unsigned int *)tmpVaddr,MAX_DIP_CMDQ_BUFFER_SIZE);

    /*============================================
     crz driver
    =============================================*/
    m_pDipCrzDrv = CrzDrv::CreateInstance();
    DIP_FUNC_DBG("[m_pCrzDrv]:0x%lx",(unsigned long)m_pDipCrzDrv);
    if ( NULL == m_pDipCrzDrv ) {
        DIP_FUNC_ERR("[Error]CrzDrv::CreateInstance cfail ");
        return -1;
    }
    m_pDipCrzDrv->Init();
    //
    android_atomic_inc(&this->mInitCount);

EXIT:
    DIP_FUNC_INF("-,ret(%d),mInitCount(%d)",ret,this->mInitCount);
    return ret;

}
//
MBOOL
IspDrvShellImp::
uninit(const char* userName)
{
    MINT32 ret = MTRUE;
    MINT32 subRet;
    MINT32 i;
    Mutex::Autolock lock(mLock);
    DIP_FUNC_INF("+,mInitCount(%d),userName(%s)",mInitCount,userName);
    //
    if(mInitCount <= 0)
    {
        // No more users
        ret = MTRUE;
        goto EXIT;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        ret = MTRUE;
        goto EXIT;
    }
    //
    // do un-initial setting
    /*============================================
        ispDrvDip & ispDrvDipPhy driver
    =============================================*/
    for(i=DIP_HW_A; i<DIP_HW_MAX; i++) {
        // release IspDrvDipPhy obj
        m_pDrvDipPhy->uninit("IspDrvShellImp");
        m_pDrvDipPhy->destroyInstance();
        //
        m_pDrvDip[i]->uninit("IspDrvShellImp");
        m_pDrvDip[i]->destroyInstance();
    }
    DIP_FUNC_INF("IspDrvShellImp uninit is done!");

    if(m_pDipRingBuf)
    {
        m_pDipRingBuf->uninit();
        m_pDipRingBuf = NULL;
    }

    /*============================================
      control MDP buffer & obj
    =============================================*/
    DIP_FUNC_INF("mdp douninit start");
    subRet=m_pMdpMgr->uninit();
    if(MDPMGR_NO_ERROR != subRet) {
        DIP_FUNC_ERR("[Error]m_pMdpMgr->uninit fail(%d)",subRet);
        ret = MFALSE;
        goto EXIT;
    }
    m_pMdpMgr->destroyInstance();
    m_pMdpMgr = NULL;
    DIP_FUNC_INF("mdp destoryInstance");

    /*============================================
     crz driver
    =============================================*/
    m_pDipCrzDrv->Uninit();
    m_pDipCrzDrv->DestroyInstance();

    /*============================================
        imem driver
    =============================================*/
    m_pIMemDrv->uninit();
    m_pIMemDrv->destroyInstance();
    m_pIMemDrv = NULL;


EXIT:
    DIP_FUNC_INF("-,ret(%d),mInitCount(%d)",ret,mInitCount);
    return ret;
}


DipRingBuffer* IspDrvShellImp::getDipRingBufMgr()
{
    return m_pDipRingBuf;
}


/*/////////////////////////////////////////////////////////////////////////////
    IspFunctionDip_B
  /////////////////////////////////////////////////////////////////////////////*/
IspDrvShell*        IspFunctionDip_B::m_pIspDrvShell = NULL;

//////
/*/////////////////////////////////////////////////////////////////////////////
    DMA
  /////////////////////////////////////////////////////////////////////////////*/
int DIP_DMA::config( unsigned int dipidx )
{
    MINT32 ret = MTRUE;
    //IspDrvDip*   pDrvDip;
    VirDipDrv*   pDrvDip;

    DIP_FUNC_DBG("[DIP_DMA (%lu)] moduleIdx(%d)",\
        this->id(), dipidx);

    pDrvDip = m_pIspDrvShell->m_pDrvDip[dipidx];

    switch(this->id())
    {
        //in-dma
        case ENUM_DIP_DMA_TDRI:
            ///kk test need to add a function to get tile base address from isp_function
            DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIPDMATOP_D1A_DIPDMATOP_TDRI_BASE_ADDR,0x00);  //kk test
            DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIPDMATOP_D1A_DIPDMATOP_TDRI_OFST_ADDR,0x00);
            break;
        case ENUM_DIP_DMA_IMGI:
            //DIP_WRITE_REG(pDrvDip, IMGI_D1A_IMGI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, IMGI_D1A_IMGI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, IMGI_D1A_IMGI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, IMGI_D1A_IMGI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, IMGI_D1A_IMGI_STRIDE,IMGI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, IMGI_D1A_IMGI_STRIDE,IMGI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, IMGI_D1A_IMGI_STRIDE,IMGI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGI_D1A_IMGI_CON,0x80000080); //default:0x80000080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGI_D1A_IMGI_CON2,0x00800080); //default:0x00800080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGI_D1A_IMGI_CON3,0x00800080); //default:0x00800080
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_IMGI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //UNP_D6
            DIP_WRITE_REG(pDrvDip, UNP_D6A_UNP_OFST, 0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D6A_UNP_CONT,UNP_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, UNP_D6A_UNP_CONT,UNP_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D6A_UNP_CONT,UNP_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, UNP_D6A_UNP_CONT,UNP_YUV_DNG,this->dma_cfg.yuv_dng);
            //
            DIP_FUNC_DBG("[imgi]addr(0x%lx),xsize(0x%lx),h(%ld),stride(%ld)",\
               (unsigned long)(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),\
               (this->dma_cfg.size.xsize - 1),(this->dma_cfg.size.h - 1),this->dma_cfg.size.stride);
            DIP_FUNC_DBG("[imgi]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",\
                this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);
            break;
        case ENUM_DIP_DMA_IMGBI:
            DIP_WRITE_REG(pDrvDip, IMGBI_D1A_IMGBI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, IMGBI_D1A_IMGBI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, IMGBI_D1A_IMGBI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, IMGBI_D1A_IMGBI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, IMGBI_D1A_IMGBI_STRIDE,IMGBI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, IMGBI_D1A_IMGBI_STRIDE,IMGBI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, IMGBI_D1A_IMGBI_STRIDE,IMGBI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGBI_D1A_IMGBI_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGBI_D1A_IMGBI_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGBI_D1A_IMGBI_CON3,0x00400040); //default:0x00400040
            //
            //UNP_D7
            DIP_WRITE_REG(pDrvDip, UNP_D7A_UNP_OFST, 0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D7A_UNP_CONT,UNP_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, UNP_D7A_UNP_CONT,UNP_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D7A_UNP_CONT,UNP_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, UNP_D7A_UNP_CONT,UNP_YUV_DNG,this->dma_cfg.yuv_dng);
            //
            DIP_FUNC_DBG("[imgbi]addr(0x%lx),xsize(0x%lx),h(%ld),stride(%ld)",\
               (unsigned long)(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),\
               (this->dma_cfg.size.xsize - 1),(this->dma_cfg.size.h - 1),this->dma_cfg.size.stride);
            DIP_FUNC_DBG("[imgbi]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",\
                this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);

            break;
        case ENUM_DIP_DMA_IMGCI:
            DIP_WRITE_REG(pDrvDip, IMGCI_D1A_IMGCI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, IMGCI_D1A_IMGCI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, IMGCI_D1A_IMGCI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, IMGCI_D1A_IMGCI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, IMGCI_D1A_IMGCI_STRIDE,IMGCI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, IMGCI_D1A_IMGCI_STRIDE,IMGCI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, IMGCI_D1A_IMGCI_STRIDE,IMGCI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGCI_D1A_IMGCI_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGCI_D1A_IMGCI_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMGCI_D1A_IMGCI_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_IMGCI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //UNP_D8
            DIP_WRITE_REG(pDrvDip, UNP_D8A_UNP_OFST, 0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D8A_UNP_CONT,UNP_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, UNP_D8A_UNP_CONT,UNP_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D8A_UNP_CONT,UNP_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, UNP_D8A_UNP_CONT,UNP_YUV_DNG,this->dma_cfg.yuv_dng);

            DIP_FUNC_DBG("[imgci]addr(0x%lx),xsize(0x%lx),h(%ld),stride(%ld)",\
               (unsigned long)(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),\
               (this->dma_cfg.size.xsize - 1),(this->dma_cfg.size.h - 1),this->dma_cfg.size.stride);
            DIP_FUNC_DBG("[imgci]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",\
                this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);
            break;
        case ENUM_DIP_DMA_UFDI:
            DIP_FUNC_DBG("[ufdi]xsize(0x%lx),h(%ld)",this->dma_cfg.size.xsize,this->dma_cfg.size.h);
            DIP_WRITE_REG(pDrvDip, UFDI_D1A_UFDI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, UFDI_D1A_UFDI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, UFDI_D1A_UFDI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, UFDI_D1A_UFDI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, UFDI_D1A_UFDI_STRIDE,UFDI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, UFDI_D1A_UFDI_STRIDE,UFDI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, UFDI_D1A_UFDI_STRIDE,UFDI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,UFDI_D1A_UFDI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,UFDI_D1A_UFDI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG( m_pIspDrvShell->m_pDrvDipPhy,UFDI_D1A_UFDI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_UFDI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_LCEI:
            DIP_WRITE_REG(pDrvDip, LCEI_D1A_LCEI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, LCEI_D1A_LCEI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, LCEI_D1A_LCEI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, LCEI_D1A_LCEI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, LCEI_D1A_LCEI_STRIDE,LCEI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, LCEI_D1A_LCEI_STRIDE,LCEI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, LCEI_D1A_LCEI_STRIDE,LCEI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,LCEI_D1A_LCEI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,LCEI_D1A_LCEI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,LCEI_D1A_LCEI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_LCEI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_VIPI:
            DIP_WRITE_REG(pDrvDip, VIPI_D1A_VIPI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, VIPI_D1A_VIPI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, VIPI_D1A_VIPI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, VIPI_D1A_VIPI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, VIPI_D1A_VIPI_STRIDE,VIPI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, VIPI_D1A_VIPI_STRIDE,VIPI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, VIPI_D1A_VIPI_STRIDE,VIPI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPI_D1A_VIPI_CON,0x80000080); //default:0x80000080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPI_D1A_VIPI_CON2,0x00800080); //default:0x00800080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPI_D1A_VIPI_CON3,0x00800080); //default:0x00800080
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_VIPI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //UNP_D9
            DIP_WRITE_REG(pDrvDip, UNP_D9A_UNP_OFST, 0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D9A_UNP_CONT,UNP_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, UNP_D9A_UNP_CONT,UNP_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D9A_UNP_CONT,UNP_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, UNP_D9A_UNP_CONT,UNP_YUV_DNG,this->dma_cfg.yuv_dng);
            break;
        case ENUM_DIP_DMA_VIP2I:
            DIP_WRITE_REG(pDrvDip, VIPBI_D1A_VIPBI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, VIPBI_D1A_VIPBI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, VIPBI_D1A_VIPBI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, VIPBI_D1A_VIPBI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, VIPBI_D1A_VIPBI_STRIDE,VIPBI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, VIPBI_D1A_VIPBI_STRIDE,VIPBI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, VIPBI_D1A_VIPBI_STRIDE,VIPBI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPBI_D1A_VIPBI_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPBI_D1A_VIPBI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPBI_D1A_VIPBI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_VIPBI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //UNP_D10
            DIP_WRITE_REG(pDrvDip, UNP_D10A_UNP_OFST, 0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D10A_UNP_CONT,UNP_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, UNP_D10A_UNP_CONT,UNP_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D10A_UNP_CONT,UNP_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, UNP_D10A_UNP_CONT,UNP_YUV_DNG,this->dma_cfg.yuv_dng);
            break;
        case ENUM_DIP_DMA_VIP3I:
            DIP_WRITE_REG(pDrvDip, VIPCI_D1A_VIPCI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, VIPCI_D1A_VIPCI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, VIPCI_D1A_VIPCI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, VIPCI_D1A_VIPCI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, VIPCI_D1A_VIPCI_STRIDE,VIPCI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, VIPCI_D1A_VIPCI_STRIDE,VIPCI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, VIPCI_D1A_VIPCI_STRIDE,VIPCI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPCI_D1A_VIPCI_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPCI_D1A_VIPCI_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,VIPCI_D1A_VIPCI_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_VIPCI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //UNP_D11
            DIP_WRITE_REG(pDrvDip, UNP_D11A_UNP_OFST, 0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D11A_UNP_CONT,UNP_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, UNP_D11A_UNP_CONT,UNP_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, UNP_D11A_UNP_CONT,UNP_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, UNP_D11A_UNP_CONT,UNP_YUV_DNG,this->dma_cfg.yuv_dng);
            break;
        case ENUM_DIP_DMA_DMGI:
            DIP_WRITE_REG(pDrvDip, DMGI_D1A_DMGI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DMGI_D1A_DMGI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DMGI_D1A_DMGI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DMGI_D1A_DMGI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DMGI_D1A_DMGI_STRIDE,DMGI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DMGI_D1A_DMGI_STRIDE,DMGI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DMGI_D1A_DMGI_STRIDE,DMGI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DMGI_D1A_DMGI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DMGI_D1A_DMGI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DMGI_D1A_DMGI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_DMGI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_DEPI:
            DIP_WRITE_REG(pDrvDip, DEPI_D1A_DEPI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DEPI_D1A_DEPI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DEPI_D1A_DEPI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DEPI_D1A_DEPI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DEPI_D1A_DEPI_STRIDE,DEPI_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DEPI_D1A_DEPI_STRIDE,DEPI_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DEPI_D1A_DEPI_STRIDE,DEPI_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DEPI_D1A_DEPI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DEPI_D1A_DEPI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DEPI_D1A_DEPI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_DEPI_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        //out-dma
        case ENUM_DIP_DMA_TIMGO:
            DIP_WRITE_REG(pDrvDip,TIMGO_D1A_TIMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,TIMGO_D1A_TIMGO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,TIMGO_D1A_TIMGO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,TIMGO_D1A_TIMGO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, TIMGO_D1A_TIMGO_STRIDE,TIMGO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, TIMGO_D1A_TIMGO_STRIDE,TIMGO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, TIMGO_D1A_TIMGO_STRIDE,TIMGO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,TIMGO_D1A_TIMGO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,TIMGO_D1A_TIMGO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,TIMGO_D1A_TIMGO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,TIMGO_D1A_TIMGO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            //PAK_D1
            DIP_WRITE_BITS(pDrvDip, PAK_D1A_PAK_CONT,PAK_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, PAK_D1A_PAK_CONT,PAK_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, PAK_D1A_PAK_CONT,PAK_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, PAK_D1A_PAK_CONT,PAK_YUV_DNG,this->dma_cfg.yuv_dng);
            DIP_WRITE_BITS(pDrvDip, PAK_D1A_PAK_CONT,PAK_OBIT,this->dma_cfg.pak_obit);

            DIP_WRITE_BITS(pDrvDip, DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN,DIPDMATOP_TIMGO_D1_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_IMG3O:
            DIP_WRITE_REG(pDrvDip,IMG3O_D1A_IMG3O_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,IMG3O_D1A_IMG3O_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,IMG3O_D1A_IMG3O_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,IMG3O_D1A_IMG3O_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, IMG3O_D1A_IMG3O_STRIDE,IMG3O_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, IMG3O_D1A_IMG3O_STRIDE,IMG3O_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, IMG3O_D1A_IMG3O_STRIDE,IMG3O_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3O_D1A_IMG3O_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3O_D1A_IMG3O_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3O_D1A_IMG3O_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,IMG3O_D1A_IMG3O_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            //PAK_D6
            DIP_WRITE_BITS(pDrvDip, PAK_D6A_PAK_CONT,PAK_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, PAK_D6A_PAK_CONT,PAK_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, PAK_D6A_PAK_CONT,PAK_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, PAK_D6A_PAK_CONT,PAK_YUV_DNG,this->dma_cfg.yuv_dng);
            DIP_WRITE_BITS(pDrvDip, PAK_D6A_PAK_CONT,PAK_OBIT,0xA);
            break;
        case ENUM_DIP_DMA_IMG3BO:
            DIP_WRITE_REG(pDrvDip,IMG3BO_D1A_IMG3BO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,IMG3BO_D1A_IMG3BO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,IMG3BO_D1A_IMG3BO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,IMG3BO_D1A_IMG3BO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, IMG3BO_D1A_IMG3BO_STRIDE,IMG3BO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, IMG3BO_D1A_IMG3BO_STRIDE,IMG3BO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, IMG3BO_D1A_IMG3BO_STRIDE,IMG3BO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3O_D1A_IMG3BO_CON,0x80000020); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3O_D1A_IMG3BO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3O_D1A_IMG3BO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,IMG3BO_D1A_IMG3BO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            //PAK_D7
            DIP_WRITE_BITS(pDrvDip, PAK_D7A_PAK_CONT,PAK_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, PAK_D7A_PAK_CONT,PAK_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, PAK_D7A_PAK_CONT,PAK_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, PAK_D7A_PAK_CONT,PAK_YUV_DNG,this->dma_cfg.yuv_dng);
            DIP_WRITE_BITS(pDrvDip, PAK_D7A_PAK_CONT,PAK_OBIT,0xA);
            break;
        case ENUM_DIP_DMA_IMG3CO:
            DIP_WRITE_REG(pDrvDip,IMG3CO_D1A_IMG3CO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,IMG3CO_D1A_IMG3CO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,IMG3CO_D1A_IMG3CO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,IMG3CO_D1A_IMG3CO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, IMG3CO_D1A_IMG3CO_STRIDE,IMG3CO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, IMG3CO_D1A_IMG3CO_STRIDE,IMG3CO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, IMG3CO_D1A_IMG3CO_STRIDE,IMG3CO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3CO_D1A_IMG3CO_CON,0x80000020); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3CO_D1A_IMG3CO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,IMG3CO_D1A_IMG3CO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,IMG3CO_D1A_IMG3CO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            //PAK_D8
            DIP_WRITE_BITS(pDrvDip, PAK_D8A_PAK_CONT,PAK_SWAP_ODR,this->dma_cfg.swap);
            DIP_WRITE_BITS(pDrvDip, PAK_D8A_PAK_CONT,PAK_UV_SIGN,0x00);
            DIP_WRITE_BITS(pDrvDip, PAK_D8A_PAK_CONT,PAK_YUV_BIT,this->dma_cfg.yuv_bit);
            DIP_WRITE_BITS(pDrvDip, PAK_D8A_PAK_CONT,PAK_YUV_DNG,this->dma_cfg.yuv_dng);
            DIP_WRITE_BITS(pDrvDip, PAK_D8A_PAK_CONT,PAK_OBIT,0xA);
            break;
        case ENUM_DIP_DMA_FEO:
            DIP_WRITE_REG(pDrvDip,FEO_D1A_FEO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,FEO_D1A_FEO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,FEO_D1A_FEO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,FEO_D1A_FEO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, FEO_D1A_FEO_STRIDE,FEO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, FEO_D1A_FEO_STRIDE,FEO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, FEO_D1A_FEO_STRIDE,FEO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);

            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,FEO_D1A_FEO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,FEO_D1A_FEO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,FEO_D1A_FEO_CON3,0x00400040); //default:0x00400040
            break;
        case ENUM_DIP_DMA_IMG2O:
            DIP_WRITE_REG(pDrvDip,CRZO_D1A_CRZO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,CRZO_D1A_CRZO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,CRZO_D1A_CRZO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,CRZO_D1A_CRZO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, CRZO_D1A_CRZO_STRIDE,CRZO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, CRZO_D1A_CRZO_STRIDE,CRZO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, CRZO_D1A_CRZO_STRIDE,CRZO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,CRZO_D1A_CRZO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,CRZO_D1A_CRZO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,CRZO_D1A_CRZO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,CRZO_D1A_CRZO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_IMG2BO:
            DIP_WRITE_REG(pDrvDip,CRZBO_D1A_CRZBO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,CRZBO_D1A_CRZBO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,CRZBO_D1A_CRZBO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,CRZBO_D1A_CRZBO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, CRZBO_D1A_CRZBO_STRIDE,CRZBO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, CRZBO_D1A_CRZBO_STRIDE,CRZBO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, CRZBO_D1A_CRZBO_STRIDE,CRZBO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,CRZBO_D1A_CRZBO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,CRZBO_D1A_CRZBO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,CRZBO_D1A_CRZBO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,CRZBO_D1A_CRZBO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_DCESO:
            DIP_WRITE_REG(pDrvDip,DCESO_D1A_DCESO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DCESO_D1A_DCESO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DCESO_D1A_DCESO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DCESO_D1A_DCESO_YSIZE,this->dma_cfg.size.h - 1);
            DIP_WRITE_BITS(pDrvDip, DCESO_D1A_DCESO_STRIDE,DCESO_STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DCESO_D1A_DCESO_STRIDE,DCESO_FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DCESO_D1A_DCESO_STRIDE,DCESO_BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DCESO_D1A_DCESO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DCESO_D1A_DCESO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DCESO_D1A_DCESO_CON3,0x00400040); //default:0x00400040
            //
            break;
        default:
            DIP_FUNC_ERR("[Error]Not support this dip thread(%lu)",this->id());
            ret = MFALSE;
            goto EXIT;
            break;
    }
EXIT:
    return ret;
}

int DIP_DMA::write2CQ(unsigned int dipidx)
{
    MINT32 ret = MTRUE;
    MUINT32 cqThrModuleId = 0xff;
    MUINT32 cqThrModuleId2 = 0xff;
    MUINT32 cqThrModuleId3 = 0xff;
    //
    DIP_FUNC_DBG("[DIP_DMA]moduleIdx(%d)",\
        dipidx);

    switch (this->id()){
        case ENUM_DIP_DMA_CQI:
            /* do nothing */
            goto EXIT;
            break;
        //in-dma
        case ENUM_DIP_DMA_TDRI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_TDRI;
            break;
        case ENUM_DIP_DMA_IMGI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_IMGI_D1;
            cqThrModuleId3 = DIP_A_UNP_D6;
            break;
        case ENUM_DIP_DMA_IMGBI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_IMGBI_D1;
            cqThrModuleId3 = DIP_A_UNP_D7;
            break;
        case ENUM_DIP_DMA_IMGCI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_IMGCI_D1;
            cqThrModuleId3 = DIP_A_UNP_D8;
            break;
        case ENUM_DIP_DMA_UFDI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_UFDI_D1;
            break;
        case ENUM_DIP_DMA_LCEI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_LCEI_D1;
            break;
        case ENUM_DIP_DMA_VIPI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_VIPI_D1;
            cqThrModuleId3 = DIP_A_UNP_D9;
            break;
        case ENUM_DIP_DMA_VIP2I:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_VIPBI_D1;
            cqThrModuleId3 = DIP_A_UNP_D10;
            break;
        case ENUM_DIP_DMA_VIP3I:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_VIPCI_D1;
            cqThrModuleId3 = DIP_A_UNP_D11;
            break;
        case ENUM_DIP_DMA_DMGI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_DMGI_D1;
            break;
        case ENUM_DIP_DMA_DEPI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_DEPI_D1;
            break;
        //out-dma
        case ENUM_DIP_DMA_TIMGO:
            cqThrModuleId = DIP_A_TIMGO_D1;
            cqThrModuleId2 = DIP_A_TIMGO_D1_CROP;
            cqThrModuleId3 = DIP_A_VECTICAL_FLIP;
            break;
        case ENUM_DIP_DMA_IMG3O:
            cqThrModuleId = DIP_A_IMG3O_D1;
            cqThrModuleId2 = DIP_A_IMG3O_D1_CROP;
            cqThrModuleId3 = DIP_A_PAK_D6;
            break;
        case ENUM_DIP_DMA_IMG3BO:
            cqThrModuleId = DIP_A_IMG3BO_D1;
            cqThrModuleId2 = DIP_A_IMG3BO_D1_CROP;
            cqThrModuleId3 = DIP_A_PAK_D7;
            break;
        case ENUM_DIP_DMA_IMG3CO:
            cqThrModuleId = DIP_A_IMG3CO_D1;
            cqThrModuleId2 = DIP_A_IMG3CO_D1_CROP;
            cqThrModuleId3 = DIP_A_PAK_D8;
            break;
        case ENUM_DIP_DMA_FEO:
            cqThrModuleId = DIP_A_FEO_D1;
            cqThrModuleId2 = 0xff;
            break;
        case ENUM_DIP_DMA_IMG2O:
            cqThrModuleId = DIP_A_CRZO_D1;
            cqThrModuleId2 = DIP_A_CRZO_D1_CROP;
            break;
        case ENUM_DIP_DMA_IMG2BO:
            cqThrModuleId = DIP_A_CRZBO_D1;
            cqThrModuleId2 = DIP_A_CRZBO_D1_CROP;
            break;
        case ENUM_DIP_DMA_DCESO:
            cqThrModuleId = DIP_A_DCESO_D1;
            cqThrModuleId2 = 0xff;
            break;
        default:
            DIP_FUNC_ERR("[Error]Not support this this->id(%ld)",this->id());
            ret = MFALSE;
            goto EXIT;
            break;
    }

    // TODO: hungwen modify
    m_pIspDrvShell->m_pDrvDip[dipidx]->cqAddModule(cqThrModuleId);
    if(cqThrModuleId2 != 0xff)
        m_pIspDrvShell->m_pDrvDip[dipidx]->cqAddModule(cqThrModuleId2);

    if(cqThrModuleId3 != 0xff)
        m_pIspDrvShell->m_pDrvDip[dipidx]->cqAddModule(cqThrModuleId3);


EXIT:
    return ret;
}

/*/////////////////////////////////////////////////////////////////////////////
    DIP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
*
********************************************************************************/
int DIP_ISP_PIPE::configTpipeData(void )
{
    int ret = 0;
    int i;
    MUINTPTR SmxVaddr = 0,SmxPaddr = 0;
	MUINT32 SmxOft = 0;

    DIPCTL_REG_D1A_DIPCTL_RGB_EN1 *pRgbEn;
    DIPCTL_REG_D1A_DIPCTL_RGB_EN2 *pRgb2En;
    DIPCTL_REG_D1A_DIPCTL_YUV_EN1 *pYuvEn;
    DIPCTL_REG_D1A_DIPCTL_YUV_EN2 *pYuv2En;
    DIPCTL_REG_D1A_DIPCTL_DMA_EN1 *pDmaEn;
    DIPCTL_REG_D1A_DIPCTL_DMA_EN2 *pDma2En;
    DIPCTL_REG_D1A_DIPCTL_MUX_SEL1 *pMuxSel;
    DIPCTL_REG_D1A_DIPCTL_MUX_SEL2 *pMux2Sel;
    DIPCTL_REG_D1A_DIPCTL_FMT_SEL1 *pFmtSel;
    DIPCTL_REG_D1A_DIPCTL_FMT_SEL2 *pFmt2Sel;
    DIPCTL_REG_D1A_DIPCTL_MISC_SEL *pMiscSel;
    DIPCTL_REG_D1A_DIPCTL_TDR_CTL *pTdrCtl;
    DIPCTL_REG_D1A_DIPCTL_TDR_SEL *pTdrSel;
    YNR_REG_D1A_YNR_CON1 *pYNRCon;

    dip_x_reg_t *tuningIspReg;
    dip_x_reg_t tmpTuningIspReg;

    UFDG_META_INFO *pUfdParam;
    pUfdParam = this->pUfdParam;

    ISP_TPIPE_CONFIG_STRUCT *pTdri = NULL;

	MUINT32 cqSecHdl, tpipeSecHdl, smxSecHdl;
    MUINT32 cqSecSize, tpipeSecSize, smxSecSize;
    MUINT32 cqDescSize;
    //


    // TODO: hungwen modify
    //pthread_mutex_lock(&mMdpCfgDataMutex);
    m_pDipWBuf->m_pMdpCfg->pDumpRegBuffer = this->pRegiAddr;
    pTdri = &m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo;
    //pthread_mutex_unlock(&mMdpCfgDataMutex);
    // TODO: hungwen modify
    pTdri->drvinfo.DesCqPa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getCQDescBufPhyAddr();
    pTdri->drvinfo.DesCqVa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getCQDescBufVirAddr();
    pTdri->drvinfo.pIspVirRegAddr_pa = m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegPhyAddr();
    pTdri->drvinfo.pIspVirRegAddr_va = m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegVirAddr();
	pTdri->drvinfo.VirtRegPa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegPhyAddr();
    pTdri->drvinfo.VirtRegVa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegVirAddr();
    pTdri->drvinfo.dupCqIdx = 0;
    pTdri->drvinfo.RingBufIdx = this->RingBufIdx;
    pTdri->drvinfo.burstCqIdx = this->burstQueIdx;
    pTdri->drvinfo.cqIdx = (E_ISP_DIP_CQ)this->dipTh;
    //Update IMGI Address
    pTdri->drvinfo.imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr + this->DMAImgi.dma_cfg.memBuf.ofst_addr;
    pTdri->drvinfo.debugRegDump = 0;

    pTdri->drvinfo.regCount = this->regCount;
    ::memcpy(pTdri->drvinfo.ReadAddrList, this->pReadAddrList, this->regCount*sizeof(int));

	// DIP settings for GCE cmd
	m_pDipWBuf->m_pMdpCfg->isSecureFra = this->isSecureFra;
	m_pDipWBuf->m_pMdpCfg->srcSecureTag = this->DMAImgi.dma_cfg.secureTag;
    if (this->isSecureFra == 1)
    {
        pTdri->drvinfo.dip_ctl_yuv_en = this->isp_top_ctl.YUV_EN.Raw;
        pTdri->drvinfo.dip_ctl_yuv2_en = this->isp_top_ctl.YUV2_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb_en = this->isp_top_ctl.RGB_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb2_en = this->isp_top_ctl.RGB2_EN.Raw;
        pTdri->drvinfo.dip_ctl_dma_en = this->isp_top_ctl.DMA_EN.Raw;
		pTdri->drvinfo.dip_ctl_dma2_en = this->isp_top_ctl.DMA2_EN.Raw;
        pTdri->drvinfo.dip_ctl_fmt_sel = this->isp_top_ctl.FMT_SEL.Raw;
		pTdri->drvinfo.dip_ctl_fmt2_sel = this->isp_top_ctl.FMT2_SEL.Raw;
        pTdri->drvinfo.dip_ctl_mux_sel = this->isp_top_ctl.MUX_SEL.Raw;
		pTdri->drvinfo.dip_ctl_mux2_sel = this->isp_top_ctl.MUX2_SEL.Raw;
        pTdri->drvinfo.dip_ctl_misc_sel = this->isp_top_ctl.MISC_SEL.Raw;
        if (this->DMAImg2o.dma_cfg.secureTag != 0)
        {
            pTdri->drvinfo.dip_img2o_base_addr = this->DMAImg2o.dma_cfg.memBuf.base_pAddr;
            pTdri->drvinfo.dip_img2bo_base_addr = this->DMAImg2o.dma_cfg.memBuf.size;	// stands for offset
        }
        else
        {
            pTdri->drvinfo.dip_img2o_base_addr = this->DMAImg2o.dma_cfg.memBuf.base_pAddr + this->DMAImg2o.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_img2bo_base_addr = this->DMAImg2bo.dma_cfg.memBuf.base_pAddr + this->DMAImg2bo.dma_cfg.memBuf.ofst_addr;
        }
        pTdri->drvinfo.dip_img2o_size[0] = this->DMAImg2o.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_img2o_size[1] = this->DMAImg2bo.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_img2o_size[2] = 0;
        if (this->DMAImg3o.dma_cfg.secureTag != 0)
        {
            pTdri->drvinfo.dip_img3o_base_addr = this->DMAImg3o.dma_cfg.memBuf.base_pAddr;
            pTdri->drvinfo.dip_img3bo_base_addr = this->DMAImg3o.dma_cfg.memBuf.size;
            pTdri->drvinfo.dip_img3co_base_addr = this->DMAImg3o.dma_cfg.memBuf.size + this->DMAImg3bo.dma_cfg.memBuf.size;
        }
        else
        {
            pTdri->drvinfo.dip_img3o_base_addr = this->DMAImg3o.dma_cfg.memBuf.base_pAddr + this->DMAImg3o.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_img3bo_base_addr = this->DMAImg3bo.dma_cfg.memBuf.base_pAddr + this->DMAImg3bo.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_img3co_base_addr = this->DMAImg3co.dma_cfg.memBuf.base_pAddr + this->DMAImg3co.dma_cfg.memBuf.ofst_addr;
        }
        pTdri->drvinfo.dip_img3o_size[0] = this->DMAImg3o.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_img3o_size[1] = this->DMAImg3bo.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_img3o_size[2] = this->DMAImg3co.dma_cfg.memBuf.size;;
        pTdri->drvinfo.dip_feo_base_addr = this->DMAFeo.dma_cfg.memBuf.base_pAddr + this->DMAFeo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_feo_size[0] = this->DMAFeo.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_feo_size[1] = pTdri->drvinfo.dip_feo_size[2] = 0;
        pTdri->drvinfo.dip_dceso_base_addr = this->DMADceso.dma_cfg.memBuf.base_pAddr + this->DMADceso.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_dceso_size[0] = this->DMADceso.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_dceso_size[1] = pTdri->drvinfo.dip_dceso_size[2] = 0;
		pTdri->drvinfo.dip_timgo_base_addr = this->DMATimgo.dma_cfg.memBuf.base_pAddr + this->DMATimgo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_timgo_size[0] = this->DMATimgo.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_timgo_size[1] = pTdri->drvinfo.dip_timgo_size[2] = 0;

        if (this->DMAImgi.dma_cfg.secureTag != 0)
        {
            pTdri->drvinfo.dip_imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr;
            if (this->isp_top_ctl.RGB_EN.Bits.DIPCTL_BPC_D1_EN == 1) // BPCI table
            {
                pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgbi.dma_cfg.memBuf.base_pAddr;
                pTdri->drvinfo.dip_imgbi_base_vaddr = this->DMAImgbi.dma_cfg.memBuf.base_vAddr;
                //pTdri->drvinfo.dip_imgi_size = this->DMAImgi.dma_cfg.memBuf.size;
                //pTdri->drvinfo.dip_imgbi_size = this->DMAImgbi.dma_cfg.memBuf.size;
            }
            else if (this->DMAImgbi.dma_cfg.secureTag != 0)
            {
                pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgi.dma_cfg.memBuf.size;
                pTdri->drvinfo.dip_imgbi_base_vaddr = 0;
                //pTdri->drvinfo.dip_imgi_size = this->DMAImgi.dma_cfg.memBuf.size + this->DMAImgbi.dma_cfg.memBuf.size + this->DMAImgci.dma_cfg.memBuf.size;
                //pTdri->drvinfo.dip_imgbi_size = pTdri->drvinfo.dip_imgi_size;
            }
            if (this->isp_top_ctl.RGB_EN.Bits.DIPCTL_LSC_D1_EN == 1) // Shading table
            {
                pTdri->drvinfo.dip_imgci_base_addr = this->DMAImgci.dma_cfg.memBuf.base_pAddr;
                pTdri->drvinfo.dip_imgci_base_vaddr = this->DMAImgci.dma_cfg.memBuf.base_vAddr;
                //pTdri->drvinfo.dip_imgci_size = this->DMAImgci.dma_cfg.memBuf.size;
            }
            else if (this->DMAImgci.dma_cfg.secureTag != 0)
            {
                pTdri->drvinfo.dip_imgci_base_addr = this->DMAImgi.dma_cfg.memBuf.size + this->DMAImgbi.dma_cfg.memBuf.size;
                pTdri->drvinfo.dip_imgci_base_vaddr = 0;
                //pTdri->drvinfo.dip_imgci_size = pTdri->drvinfo.dip_imgi_size;
            }
        }
        else
        {
            pTdri->drvinfo.dip_imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr + this->DMAImgi.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgbi.dma_cfg.memBuf.base_pAddr + this->DMAImgbi.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_imgci_base_addr = this->DMAImgci.dma_cfg.memBuf.base_pAddr + this->DMAImgci.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_imgbi_base_vaddr = 0;
            pTdri->drvinfo.dip_imgci_base_vaddr = 0;
        }
        pTdri->drvinfo.dip_imgi_size[0] = this->DMAImgi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_imgi_size[1] = this->DMAImgbi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_imgi_size[2] = this->DMAImgci.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_imgbi_size[0] = this->DMAImgbi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_imgbi_size[1] = pTdri->drvinfo.dip_imgbi_size[2] = 0;
        pTdri->drvinfo.dip_imgci_size[0] = this->DMAImgci.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_imgci_size[1] = pTdri->drvinfo.dip_imgci_size[2] = 0;
        if (this->DMAVipi.dma_cfg.secureTag != 0)
        {
            pTdri->drvinfo.dip_vipi_base_addr = this->DMAVipi.dma_cfg.memBuf.base_pAddr;
            pTdri->drvinfo.dip_vip2i_base_addr = this->DMAVipi.dma_cfg.memBuf.size;
            pTdri->drvinfo.dip_vip3i_base_addr = this->DMAVipi.dma_cfg.memBuf.size + this->DMAVip2i.dma_cfg.memBuf.size;
        }
        else
        {
            pTdri->drvinfo.dip_vipi_base_addr = this->DMAVipi.dma_cfg.memBuf.base_pAddr + this->DMAVipi.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_vip2i_base_addr = this->DMAVip2i.dma_cfg.memBuf.base_pAddr + this->DMAVip2i.dma_cfg.memBuf.ofst_addr;
            pTdri->drvinfo.dip_vip3i_base_addr = this->DMAVip3i.dma_cfg.memBuf.base_pAddr + this->DMAVip3i.dma_cfg.memBuf.ofst_addr;
        }
        pTdri->drvinfo.dip_vipi_size[0] = this->DMAVipi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_vipi_size[1] = this->DMAVip2i.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_vipi_size[2] = this->DMAVip3i.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_dmgi_base_addr = this->DMADmgi.dma_cfg.memBuf.base_pAddr + this->DMADmgi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_dmgi_base_vaddr = this->DMADmgi.dma_cfg.memBuf.base_vAddr + this->DMADmgi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_dmgi_size[0] = this->DMADmgi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_dmgi_size[1] = pTdri->drvinfo.dip_dmgi_size[2] = 0;
        pTdri->drvinfo.dip_depi_base_addr = this->DMADepi.dma_cfg.memBuf.base_pAddr + this->DMADepi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_depi_base_vaddr = this->DMADepi.dma_cfg.memBuf.base_vAddr + this->DMADepi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_depi_size[0] = this->DMADepi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_depi_size[1] = pTdri->drvinfo.dip_depi_size[2] = 0;
        pTdri->drvinfo.dip_lcei_base_addr = this->DMALcei.dma_cfg.memBuf.base_pAddr + this->DMALcei.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_lcei_base_vaddr = this->DMALcei.dma_cfg.memBuf.base_vAddr + this->DMALcei.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_lcei_size[0] = this->DMALcei.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_lcei_size[1] = pTdri->drvinfo.dip_lcei_size[2] = 0;
        pTdri->drvinfo.dip_ufdi_base_addr = this->DMAUfdi.dma_cfg.memBuf.base_pAddr + this->DMAUfdi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_ufdi_size[0] = this->DMAUfdi.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_ufdi_size[1] = pTdri->drvinfo.dip_ufdi_size[2] = 0;

        pTdri->drvinfo.dip_secure_tag = this->isSecureFra;
        pTdri->drvinfo.dip_img2o_secure_tag = this->DMAImg2o.dma_cfg.secureTag;
        pTdri->drvinfo.dip_img2bo_secure_tag = this->DMAImg2bo.dma_cfg.secureTag;
        pTdri->drvinfo.dip_img3o_secure_tag = this->DMAImg3o.dma_cfg.secureTag;
        pTdri->drvinfo.dip_img3bo_secure_tag = this->DMAImg3bo.dma_cfg.secureTag;
        pTdri->drvinfo.dip_img3co_secure_tag = this->DMAImg3co.dma_cfg.secureTag;
        pTdri->drvinfo.dip_feo_secure_tag = this->DMAFeo.dma_cfg.secureTag;
        pTdri->drvinfo.dip_dceso_secure_tag = this->DMADceso.dma_cfg.secureTag;
		pTdri->drvinfo.dip_timgo_secure_tag = this->DMATimgo.dma_cfg.secureTag;
        pTdri->drvinfo.dip_imgi_secure_tag = this->DMAImgi.dma_cfg.secureTag;
        pTdri->drvinfo.dip_imgbi_secure_tag = this->DMAImgbi.dma_cfg.secureTag;
        pTdri->drvinfo.dip_imgci_secure_tag = this->DMAImgci.dma_cfg.secureTag;
        pTdri->drvinfo.dip_vipi_secure_tag = this->DMAVipi.dma_cfg.secureTag;
        pTdri->drvinfo.dip_vip2i_secure_tag = this->DMAVip2i.dma_cfg.secureTag;
        pTdri->drvinfo.dip_vip3i_secure_tag = this->DMAVip3i.dma_cfg.secureTag;
        pTdri->drvinfo.dip_dmgi_secure_tag = this->DMADmgi.dma_cfg.secureTag;
        pTdri->drvinfo.dip_depi_secure_tag = this->DMADepi.dma_cfg.secureTag;
        pTdri->drvinfo.dip_lcei_secure_tag = this->DMALcei.dma_cfg.secureTag;
        pTdri->drvinfo.dip_ufdi_secure_tag = this->DMAUfdi.dma_cfg.secureTag;
        pTdri->drvinfo.DesCqOft = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getCQDescBufOft();
        pTdri->drvinfo.VirtRegOft = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegOft();
        m_pIspDrvShell->m_pDipRingBuf->getSecureBufHandle(cqSecHdl,tpipeSecHdl,smxSecHdl);
        m_pIspDrvShell->m_pDipRingBuf->getSecureBufSize(cqSecSize,tpipeSecSize,smxSecSize);
        m_pIspDrvShell->m_pDipRingBuf->getCqDescBufSize(cqDescSize);
        pTdri->drvinfo.cqSecHdl = cqSecHdl;
        pTdri->drvinfo.tpipeTableSecHdl = tpipeSecHdl;
        pTdri->drvinfo.smxSecHdl = smxSecHdl;
        pTdri->drvinfo.cqSecSize = cqSecSize;
        pTdri->drvinfo.tpipeTableSecSize = tpipeSecSize;
        pTdri->drvinfo.smxSecSize = smxSecSize;
        pTdri->drvinfo.DesCqSize = cqDescSize;
    }
    else
    {
        pTdri->drvinfo.dip_ctl_yuv_en = this->isp_top_ctl.YUV_EN.Raw;
        pTdri->drvinfo.dip_ctl_yuv2_en = this->isp_top_ctl.YUV2_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb_en = this->isp_top_ctl.RGB_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb2_en = this->isp_top_ctl.RGB2_EN.Raw;
        pTdri->drvinfo.dip_ctl_dma_en = this->isp_top_ctl.DMA_EN.Raw;
		pTdri->drvinfo.dip_ctl_dma2_en = this->isp_top_ctl.DMA2_EN.Raw;
        pTdri->drvinfo.dip_ctl_fmt_sel = this->isp_top_ctl.FMT_SEL.Raw;
		pTdri->drvinfo.dip_ctl_fmt2_sel = this->isp_top_ctl.FMT2_SEL.Raw;
        pTdri->drvinfo.dip_ctl_mux_sel = this->isp_top_ctl.MUX_SEL.Raw;
		pTdri->drvinfo.dip_ctl_mux2_sel = this->isp_top_ctl.MUX2_SEL.Raw;
        pTdri->drvinfo.dip_ctl_misc_sel = this->isp_top_ctl.MISC_SEL.Raw;
        pTdri->drvinfo.dip_img2o_base_addr = this->DMAImg2o.dma_cfg.memBuf.base_pAddr + this->DMAImg2o.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img2bo_base_addr = this->DMAImg2bo.dma_cfg.memBuf.base_pAddr + this->DMAImg2bo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img3o_base_addr = this->DMAImg3o.dma_cfg.memBuf.base_pAddr + this->DMAImg3o.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img3bo_base_addr = this->DMAImg3bo.dma_cfg.memBuf.base_pAddr + this->DMAImg3bo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img3co_base_addr = this->DMAImg3co.dma_cfg.memBuf.base_pAddr + this->DMAImg3co.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_feo_base_addr = this->DMAFeo.dma_cfg.memBuf.base_pAddr + this->DMAFeo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_dceso_base_addr = this->DMADceso.dma_cfg.memBuf.base_pAddr + this->DMADceso.dma_cfg.memBuf.ofst_addr;
		pTdri->drvinfo.dip_timgo_base_addr = this->DMATimgo.dma_cfg.memBuf.base_pAddr + this->DMATimgo.dma_cfg.memBuf.ofst_addr;
		pTdri->drvinfo.dip_imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr + this->DMAImgi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgbi.dma_cfg.memBuf.base_pAddr + this->DMAImgbi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_imgci_base_addr = this->DMAImgci.dma_cfg.memBuf.base_pAddr + this->DMAImgci.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_vipi_base_addr = this->DMAVipi.dma_cfg.memBuf.base_pAddr + this->DMAVipi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_vip2i_base_addr = this->DMAVip2i.dma_cfg.memBuf.base_pAddr + this->DMAVip2i.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_vip3i_base_addr = this->DMAVip3i.dma_cfg.memBuf.base_pAddr + this->DMAVip3i.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_dmgi_base_addr = this->DMADmgi.dma_cfg.memBuf.base_pAddr + this->DMADmgi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_depi_base_addr = this->DMADepi.dma_cfg.memBuf.base_pAddr + this->DMADepi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_lcei_base_addr = this->DMALcei.dma_cfg.memBuf.base_pAddr + this->DMALcei.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_ufdi_base_addr = this->DMAUfdi.dma_cfg.memBuf.base_pAddr + this->DMAUfdi.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_secure_tag = 0;
        pTdri->drvinfo.DesCqOft = 0;
        pTdri->drvinfo.VirtRegOft =0;
        pTdri->drvinfo.cqSecHdl = 0;
        pTdri->drvinfo.tpipeTableSecHdl = 0;
        pTdri->drvinfo.smxSecHdl = 0;
        pTdri->drvinfo.cqSecSize = 0;
        pTdri->drvinfo.tpipeTableSecSize = 0;
        pTdri->drvinfo.smxSecSize = 0;
        pTdri->drvinfo.DesCqSize = 0;
        pTdri->drvinfo.dip_imgbi_base_vaddr = 0;
        pTdri->drvinfo.dip_imgci_base_vaddr	= 0;
        pTdri->drvinfo.dip_dmgi_base_vaddr = 0;
        pTdri->drvinfo.dip_depi_base_vaddr = 0;
        pTdri->drvinfo.dip_lcei_base_vaddr = 0;
    }

    if (m_pIspDrvShell->m_bInitAllocSmxBuf == true)
    {
        for (i=0;i<DIP_DMA_SMXIO_MAX;i++)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, (DIP_DMA_SMXIO_ENUM)i ,SmxVaddr,SmxPaddr,SmxOft);
            switch (i)
            {
                case DIP_DMA_SMXI_1:
                    pTdri->drvinfo.smx1iPa = SmxPaddr;
					pTdri->drvinfo.smx1iOft = SmxOft;
                    break;
                case DIP_DMA_SMXI_2:
                    pTdri->drvinfo.smx2iPa = SmxPaddr;
					pTdri->drvinfo.smx2iOft = SmxOft;
                    break;
                case DIP_DMA_SMXI_3:
                    pTdri->drvinfo.smx3iPa = SmxPaddr;
					pTdri->drvinfo.smx3iOft = SmxOft;
                    break;
                case DIP_DMA_SMXI_4:
                    pTdri->drvinfo.smx4iPa = SmxPaddr;
					pTdri->drvinfo.smx4iOft = SmxOft;
                    break;
                default:
                    DIP_FUNC_ERR("no such smxio index:%d", i);
                    break;
            }

        }
    }
    else
    {
        if (this->isp_top_ctl.RGB_EN.Bits.DIPCTL_UFD_D1_EN == 1)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_1 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx1iPa = SmxPaddr;
			pTdri->drvinfo.smx1iOft = SmxOft;
        }
        if (this->isp_top_ctl.RGB2_EN.Bits.DIPCTL_MCRP_D2_EN == 1)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_4 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx4iPa = SmxPaddr;
			pTdri->drvinfo.smx4iOft = SmxOft;
        }
        if (this->isp_top_ctl.YUV_EN.Bits.DIPCTL_CNR_D1_EN == 1)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_2 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx2iPa = SmxPaddr;
			pTdri->drvinfo.smx2iOft = SmxOft;
        }
        if ((this->isp_top_ctl.YUV2_EN.Bits.DIPCTL_MCRP_D1_EN == 1) || (this->isp_top_ctl.YUV2_EN.Bits.DIPCTL_NR3D_D1_EN == 1))
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_3 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx3iPa = SmxPaddr;
			pTdri->drvinfo.smx3iOft = SmxOft;
        }
    }
    //smx setting
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],PAK_D2A_PAK_CONT,0x000A0100);
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],PAK_D3A_PAK_CONT,0x000A0100);
    //DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],PAK_D4A_PAK_CONT,0x000A0100);
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D1A_UNP_OFST, 0x0);
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D1A_UNP_CONT, 0x00000000); //TC: HW auto sync with imgi
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D2A_UNP_OFST, 0x0);
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D2A_UNP_CONT, 0x00000010);
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D3A_UNP_OFST, 0x0);
    DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D3A_UNP_CONT, 0x00000010);
    //DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D4A_UNP_OFST, 0x00);
    //DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx], UNP_D4A_UNP_CONT, 0x00000010);


    DIP_FUNC_DBG("VirCqPa(0x%lx),RingBufIdx(%d)",pTdri->drvinfo.DesCqPa,pTdri->drvinfo.RingBufIdx);

    if (this->tdr_ctl&DIPCTL_TDR_CTL_TDR_EN)
    {
        //TPIPE Mode
        // TODO: hungwen modify
        m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x0;
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_CTL,this->tdr_ctl);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_SEL,this->tdr_sel);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_TCM_EN1,this->tdr_tcm_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_TCM_EN2,this->tdr_tcm2_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_TCM_EN3,this->tdr_tcm3_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_TCM_EN4,this->tdr_tcm4_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_TCM_EN5,this->tdr_tcm5_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_TCM_EN6,this->tdr_tcm6_en);
    }
    else
    {
        //Frame Mode
        // TODO: hungwen modify
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIPCTL_D1A_DIPCTL_TDR_CTL,0x0);
        m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x2;
        return ret;
    }

    //
    pRgbEn = (DIPCTL_REG_D1A_DIPCTL_RGB_EN1*)&(this->isp_top_ctl.RGB_EN);
    pRgb2En = (DIPCTL_REG_D1A_DIPCTL_RGB_EN2*)&(this->isp_top_ctl.RGB2_EN);
    pYuvEn = (DIPCTL_REG_D1A_DIPCTL_YUV_EN1*)&(this->isp_top_ctl.YUV_EN);
    pYuv2En = (DIPCTL_REG_D1A_DIPCTL_YUV_EN2*)&(this->isp_top_ctl.YUV2_EN);
    pDmaEn = (DIPCTL_REG_D1A_DIPCTL_DMA_EN1*)&(this->isp_top_ctl.DMA_EN);
    pDma2En = (DIPCTL_REG_D1A_DIPCTL_DMA_EN2*)&(this->isp_top_ctl.DMA2_EN);
    pMuxSel = (DIPCTL_REG_D1A_DIPCTL_MUX_SEL1*)&(this->isp_top_ctl.MUX_SEL);
    pMux2Sel = (DIPCTL_REG_D1A_DIPCTL_MUX_SEL2*)&(this->isp_top_ctl.MUX2_SEL);
    pFmtSel = (DIPCTL_REG_D1A_DIPCTL_FMT_SEL1*)&(this->isp_top_ctl.FMT_SEL);
    pFmt2Sel = (DIPCTL_REG_D1A_DIPCTL_FMT_SEL2*)&(this->isp_top_ctl.FMT2_SEL);
    pMiscSel = (DIPCTL_REG_D1A_DIPCTL_MISC_SEL*)&(this->isp_top_ctl.MISC_SEL);
    pTdrCtl = (DIPCTL_REG_D1A_DIPCTL_TDR_CTL*)&(this->tdr_ctl);
    pTdrSel = (DIPCTL_REG_D1A_DIPCTL_TDR_SEL*)&(this->tdr_sel);

    if(this->isApplyTuning==MTRUE) {
        tuningIspReg = this->pTuningIspReg;
        pYNRCon = (YNR_REG_D1A_YNR_CON1*)&(this->pTuningIspReg->YNR_D1A_YNR_CON1);
    } else {
        memset((void*)&tmpTuningIspReg, 0x00, sizeof(dip_x_reg_t));
        tuningIspReg = &tmpTuningIspReg;
    }
    DIP_FUNC_DBG("pYuvEn(0x%lx)",(unsigned long)pYuvEn);

    //ISP_TPIPE_CONFIG_TOP_STRUCT
    pTdri->top.pixel_id = pMiscSel->Bits.DIPCTL_PIX_ID;
    pTdri->top.cam_in_fmt = pFmtSel->Bits.DIPCTL_IMGI_D1_FMT;
    pTdri->top.ctl_extension_en = pTdrSel->Bits.DIPCTL_TDR_CTL_EXT_EN;
    pTdri->top.fg_mode = pMiscSel->Bits.DIPCTL_FG_MODE;
    pTdri->top.pakg_fg_out = pMiscSel->Bits.DIPCTL_PAKG_D1_FG_OUT;
    pTdri->top.ufo_imgi_en = pRgbEn->Bits.DIPCTL_UFD_D1_EN;
    pTdri->top.ufdi_fmt = pFmtSel->Bits.DIPCTL_UFDI_D1_FMT;
    pTdri->top.vipi_fmt = pFmtSel->Bits.DIPCTL_VIPI_D1_FMT;
    pTdri->top.img3o_fmt = pFmt2Sel->Bits.DIPCTL_IMG3O_D1_FMT;
    pTdri->top.crzo_fmt = pFmt2Sel->Bits.DIPCTL_CRZO_D1_FMT;
    pTdri->top.timgo_fmt = pFmt2Sel->Bits.DIPCTL_TIMGO_D1_FMT;
    pTdri->top.crp2_sel = pMuxSel->Bits.DIPCTL_CRP_D2_SEL;
    pTdri->top.ynr_sel = pMuxSel->Bits.DIPCTL_YNR_D1_SEL;
    pTdri->top.timgo_sel = pMuxSel->Bits.DIPCTL_TIMGO_D1_SEL;
    pTdri->top.traw_sel = pMuxSel->Bits.DIPCTL_TRAW_SEL;
    pTdri->top.dgn_sel = pMuxSel->Bits.DIPCTL_DGN_D1_SEL;
    pTdri->top.flc_sel = pMuxSel->Bits.DIPCTL_FLC_D1_SEL;
    pTdri->top.g2cx_sel = pMuxSel->Bits.DIPCTL_G2CX_D1_SEL;
    pTdri->top.c24d1_sel = pMux2Sel->Bits.DIPCTL_C24_D1_SEL;
    pTdri->top.srz_sel = pMux2Sel->Bits.DIPCTL_SRZ_D1_SEL;
    pTdri->top.mix1_sel = pMuxSel->Bits.DIPCTL_MIX_D1_SEL;
    pTdri->top.crz_sel = pMux2Sel->Bits.DIPCTL_CRZ_D1_SEL;
    pTdri->top.nr3d_pre_i_sel = pMux2Sel->Bits.DIPCTL_NR3D_D1_PRE_I_SEL;
    pTdri->top.mix3_w_sel = pMux2Sel->Bits.DIPCTL_MIX_D3_W_SEL;
    pTdri->top.crsp_sel = pMuxSel->Bits.DIPCTL_CRSP_D1_SEL;
    pTdri->top.fe_sel = pMux2Sel->Bits.DIPCTL_FE_D1_SEL;
    pTdri->top.feo_sel = pMux2Sel->Bits.DIPCTL_FEO_D1_SEL;
    pTdri->top.imgi_en = pDmaEn->Bits.DIPCTL_IMGI_D1_EN;
    pTdri->top.imgbi_en = pDmaEn->Bits.DIPCTL_IMGBI_D1_EN;
    pTdri->top.imgci_en = pDmaEn->Bits.DIPCTL_IMGCI_D1_EN;
    pTdri->top.vipi_en = pDma2En->Bits.DIPCTL_VIPI_D1_EN;
    pTdri->top.vipbi_en = pDma2En->Bits.DIPCTL_VIPBI_D1_EN;
    pTdri->top.vipci_en = pDma2En->Bits.DIPCTL_VIPCI_D1_EN;
    pTdri->top.ufdi_en = pDmaEn->Bits.DIPCTL_UFDI_D1_EN;
    pTdri->top.lcei_en = pDmaEn->Bits.DIPCTL_LCEI_D1_EN;
    pTdri->top.timgo_en = pDmaEn->Bits.DIPCTL_TIMGO_D1_EN;
    pTdri->top.unp_en = pRgbEn->Bits.DIPCTL_UNP_D1_EN;
    pTdri->top.unp2_en = pYuvEn->Bits.DIPCTL_UNP_D2_EN;
    pTdri->top.unp3_en = pYuvEn->Bits.DIPCTL_UNP_D3_EN;
    pTdri->top.unp4_en = pRgb2En->Bits.DIPCTL_UNP_D4_EN;
    pTdri->top.unp6_en = pYuv2En->Bits.DIPCTL_UNP_D6_EN;
    pTdri->top.unp7_en = pYuv2En->Bits.DIPCTL_UNP_D7_EN;
    pTdri->top.unp8_en = pYuv2En->Bits.DIPCTL_UNP_D8_EN;
    pTdri->top.unp9_en = pYuv2En->Bits.DIPCTL_UNP_D9_EN;
    pTdri->top.unp10_en = pYuv2En->Bits.DIPCTL_UNP_D10_EN;
    pTdri->top.unp11_en = pYuv2En->Bits.DIPCTL_UNP_D11_EN;
    pTdri->top.bs_en = pYuv2En->Bits.DIPCTL_BS_D1_EN;
    pTdri->top.ufd_en = pRgbEn->Bits.DIPCTL_UFD_D1_EN;
    pTdri->top.bpc_en = pRgbEn->Bits.DIPCTL_BPC_D1_EN;
    pTdri->top.lsc_en = pRgbEn->Bits.DIPCTL_LSC_D1_EN;
    pTdri->top.wb_en = pRgbEn->Bits.DIPCTL_WB_D1_EN;
    pTdri->top.rcp_en = pYuvEn->Bits.DIPCTL_CRP_D1_EN;
    pTdri->top.rcp2_en = pRgbEn->Bits.DIPCTL_CRP_D2_EN;
    pTdri->top.pak_en = pRgbEn->Bits.DIPCTL_PAK_D1_EN;
    pTdri->top.pak2_en = pYuvEn->Bits.DIPCTL_PAK_D2_EN;
    pTdri->top.pak3_en = pYuvEn->Bits.DIPCTL_PAK_D3_EN;
    pTdri->top.pak4_en = pRgb2En->Bits.DIPCTL_PAK_D4_EN;
    pTdri->top.pak6_en = pYuv2En->Bits.DIPCTL_PAK_D6_EN;
    pTdri->top.pak7_en = pYuv2En->Bits.DIPCTL_PAK_D7_EN;
    pTdri->top.pak8_en = pYuv2En->Bits.DIPCTL_PAK_D8_EN;
    pTdri->top.c24_en = pYuv2En->Bits.DIPCTL_C24_D1_EN;
    pTdri->top.hlr_en = pRgbEn->Bits.DIPCTL_HLR_D1_EN;
    pTdri->top.dgn_en = pRgbEn->Bits.DIPCTL_DGN_D1_EN;
    pTdri->top.ltm_en = pRgbEn->Bits.DIPCTL_LTM_D1_EN;
    pTdri->top.dm_en = pRgbEn->Bits.DIPCTL_DM_D1_EN;
    pTdri->top.gdr_en = pRgbEn->Bits.DIPCTL_GDR_D1_EN;
    pTdri->top.ldnr_en = pRgbEn->Bits.DIPCTL_LDNR_D1_EN;
    pTdri->top.align_en = pRgbEn->Bits.DIPCTL_ALIGN_D1_EN;
    pTdri->top.flc_en = pRgb2En->Bits.DIPCTL_FLC_D1_EN;
    pTdri->top.ccm_en = pRgb2En->Bits.DIPCTL_CCM_D1_EN;
    pTdri->top.ccm2_en = pRgb2En->Bits.DIPCTL_CCM_D2_EN;
    pTdri->top.ggm_en = pRgb2En->Bits.DIPCTL_GGM_D1_EN;
    pTdri->top.ggm2_en = pRgb2En->Bits.DIPCTL_GGM_D2_EN;
    pTdri->top.wsync_en = pRgb2En->Bits.DIPCTL_WSYNC_D1_EN;
    pTdri->top.ggm4_en = pRgb2En->Bits.DIPCTL_GGM_D4_EN;
    pTdri->top.g2c4_en = pRgb2En->Bits.DIPCTL_G2C_D4_EN;
    pTdri->top.c42d4_en = pRgb2En->Bits.DIPCTL_C42_D4_EN;
    pTdri->top.crp4_en = pRgb2En->Bits.DIPCTL_CRP_D4_EN;
    pTdri->top.g2cx_en = pYuvEn->Bits.DIPCTL_G2CX_D1_EN;
    pTdri->top.c42_en = pYuvEn->Bits.DIPCTL_C42_D1_EN;
    pTdri->top.ynr_en = pYuvEn->Bits.DIPCTL_YNR_D1_EN;
    pTdri->top.dmgi_en = pDmaEn->Bits.DIPCTL_DMGI_D1_EN;
    pTdri->top.depi_en = pDmaEn->Bits.DIPCTL_DEPI_D1_EN;
    pTdri->top.cnr_en = pYuvEn->Bits.DIPCTL_CNR_D1_EN;
    pTdri->top.srz1_en = pYuv2En->Bits.DIPCTL_SRZ_D1_EN;
    pTdri->top.mix1_en = pYuvEn->Bits.DIPCTL_MIX_D1_EN;
    pTdri->top.mix2_en = pYuvEn->Bits.DIPCTL_MIX_D2_EN;
    pTdri->top.slk1_en = pRgbEn->Bits.DIPCTL_SLK_D1_EN;
    pTdri->top.slk2_en = pYuv2En->Bits.DIPCTL_SLK_D2_EN;
    pTdri->top.slk3_en = pYuv2En->Bits.DIPCTL_SLK_D3_EN;
    pTdri->top.slk4_en = pYuv2En->Bits.DIPCTL_SLK_D4_EN;
    pTdri->top.slk5_en = pYuv2En->Bits.DIPCTL_SLK_D5_EN;
    pTdri->top.slk6_en = pRgbEn->Bits.DIPCTL_SLK_D6_EN;
    pTdri->top.ndg_en = pYuvEn->Bits.DIPCTL_NDG_D1_EN;
    pTdri->top.ndg2_en = pYuvEn->Bits.DIPCTL_NDG_D2_EN;
    pTdri->top.c24d3_en = pYuvEn->Bits.DIPCTL_C24_D3_EN;
    pTdri->top.c24d2_en = pYuv2En->Bits.DIPCTL_C24_D2_EN;
    pTdri->top.c2g_en = pYuvEn->Bits.DIPCTL_C2G_D1_EN;
    pTdri->top.iggm_en = pYuvEn->Bits.DIPCTL_IGGM_D1_EN;
    pTdri->top.ccm3_en = pYuvEn->Bits.DIPCTL_CCM_D3_EN;
    pTdri->top.ggm3_en = pYuvEn->Bits.DIPCTL_GGM_D3_EN;
    pTdri->top.dce_en = pYuvEn->Bits.DIPCTL_DCE_D1_EN;
    pTdri->top.dces_en = pYuvEn->Bits.DIPCTL_DCES_D1_EN;
    pTdri->top.dceso_en = pDmaEn->Bits.DIPCTL_DCESO_D1_EN;
    pTdri->top.g2c_en = pYuvEn->Bits.DIPCTL_G2C_D1_EN;
    pTdri->top.c42d2_en = pYuvEn->Bits.DIPCTL_C42_D2_EN;
    pTdri->top.ee_en = pYuvEn->Bits.DIPCTL_EE_D1_EN;
    pTdri->top.lce_en = pYuvEn->Bits.DIPCTL_LCE_D1_EN;
    pTdri->top.mix3_en = pYuvEn->Bits.DIPCTL_MIX_D3_EN;
    pTdri->top.mix4_en = pYuvEn->Bits.DIPCTL_MIX_D4_EN;
    pTdri->top.crz_en = pYuv2En->Bits.DIPCTL_CRZ_D1_EN;
    pTdri->top.feo_en = pDmaEn->Bits.DIPCTL_FEO_D1_EN;
    pTdri->top.c02_en = pYuv2En->Bits.DIPCTL_C02_D1_EN;
    pTdri->top.c02d2_en = pYuv2En->Bits.DIPCTL_C02_D2_EN;
    pTdri->top.nr3d_en = pYuv2En->Bits.DIPCTL_NR3D_D1_EN;
    pTdri->top.color_en = pYuvEn->Bits.DIPCTL_COLOR_D1_EN;
    pTdri->top.crsp_en = pYuv2En->Bits.DIPCTL_CRSP_D1_EN;
    pTdri->top.img3o_en = pDma2En->Bits.DIPCTL_IMG3O_D1_EN;
    pTdri->top.img3bo_en = pDma2En->Bits.DIPCTL_IMG3BO_D1_EN;
    pTdri->top.img3co_en = pDma2En->Bits.DIPCTL_IMG3CO_D1_EN;
    pTdri->top.mcrp_en = pYuv2En->Bits.DIPCTL_MCRP_D1_EN;
    pTdri->top.mcrp2_en = pRgb2En->Bits.DIPCTL_MCRP_D2_EN;
    pTdri->top.plnr1_en = pYuv2En->Bits.DIPCTL_PLNR_D1_EN;
    pTdri->top.plnr2_en = pYuv2En->Bits.DIPCTL_PLNR_D2_EN;
    pTdri->top.plnw1_en = pYuv2En->Bits.DIPCTL_PLNW_D1_EN;
    pTdri->top.plnw2_en = pYuv2En->Bits.DIPCTL_PLNW_D2_EN;
    pTdri->top.crzo_en = pDmaEn->Bits.DIPCTL_CRZO_D1_EN;
    pTdri->top.crzbo_en = pDmaEn->Bits.DIPCTL_CRZBO_D1_EN;
    pTdri->top.dfe_en = pYuv2En->Bits.DIPCTL_DFE_D1_EN;
    pTdri->top.obc_en = pRgbEn->Bits.DIPCTL_OBC_D1_EN;
    pTdri->top.zfus_en = pRgbEn->Bits.DIPCTL_ZFUS_D1_EN;
    pTdri->top.srz3_en = pYuv2En->Bits.DIPCTL_SRZ_D3_EN;
    pTdri->top.srz4_en = pYuv2En->Bits.DIPCTL_SRZ_D4_EN;
    pTdri->top.smt1_en = pRgbEn->Bits.DIPCTL_SMT_D1_EN;
    pTdri->top.smt1i_en = pDmaEn->Bits.DIPCTL_SMTI_D1_EN;
    pTdri->top.smt1o_en = pDmaEn->Bits.DIPCTL_SMTO_D1_EN;
    pTdri->top.smt2_en = pYuvEn->Bits.DIPCTL_SMT_D2_EN;
    pTdri->top.smt2i_en = pDmaEn->Bits.DIPCTL_SMTI_D2_EN;
    pTdri->top.smt2o_en = pDmaEn->Bits.DIPCTL_SMTO_D2_EN;
    pTdri->top.smt3_en = pYuvEn->Bits.DIPCTL_SMT_D3_EN;
    pTdri->top.smt3i_en = pDmaEn->Bits.DIPCTL_SMTI_D3_EN;
    pTdri->top.smt3o_en = pDmaEn->Bits.DIPCTL_SMTO_D3_EN;
    pTdri->top.smt4_en = pRgb2En->Bits.DIPCTL_SMT_D4_EN;
    pTdri->top.smt4i_en = pDmaEn->Bits.DIPCTL_SMTI_D4_EN;
    pTdri->top.smt4o_en = pDmaEn->Bits.DIPCTL_SMTO_D4_EN;
    pTdri->top.adl_en = 0;//pDmaEn->Bits.DIPCTL_ADL_D1_EN
#if 0 //MFB part
    /* bld */
    pTdri->top.bld_ctl_extension_en = 0;
    pTdri->top.bld_en = 0;
    pTdri->top.bldi_en = 0;
    pTdri->top.bld2i_en = 0;
    pTdri->top.bld3i_en = 0;
    pTdri->top.bld4i_en = 0;
    pTdri->top.bldo_en = 0;
    pTdri->top.bld2o_en = 0;
    pTdri->top.mfb_srz_en = 0;
    pTdri->top.bldi_fmt = 0;
#endif
#if 0 //ADAS part
    /* oft */
    pTdri->top.oft_en = 0;//pDma2En->Bits.DIPCTL_OFT_D1_EN
    pTdri->top.oftl_en = 0;//pDma2En->Bits.DIPCTL_OFTL_D1_EN
#endif

    //ISP_TPIPE_CONFIG_SW_STRUCT
    pTdri->sw.log_en = 1;
    pTdri->sw.tpipe_sel_mode |= 0x1; /* For calculate DIP TPipe, use OR so MFB sel wont be erase */
    pTdri->sw.tpipe_irq_mode = TPIPE_IRQ_TPIPE;
    if (pTdri->top.mfb_en == 0)
    {
        pTdri->sw.src_width = DMAImgi.dma_cfg.size.w;
        pTdri->sw.src_height = DMAImgi.dma_cfg.size.h;
    }

    pTdri->sw.tpipe_width = MAX_TPIPE_WIDTH;
    pTdri->sw.tpipe_height = MAX_TPIPE_HEIGHT;
#if 0 //MFB part
    pTdri->sw.src_width_bld = 0;
    pTdri->sw.src_height_bld = 0;
    pTdri->sw.tpipe_width_bld = 0;
    pTdri->sw.tpipe_height_bld = 0;
    pTdri->sw.tpipe_dual_en = 0;
#endif

    //ISP_TPIPE_CONFIG_ADL_STRUCT
    pTdri->adl.adl_ctl_en = 0;//vpu-dl not support
    pTdri->adl.ipui_stride = 0;
    pTdri->adl.ipuo_stride = 0;

    //ISP_TPIPE_CONFIG_IMGI_STRUCT
    pTdri->imgi.imgi_v_flip_en = DMAImgi.dma_cfg.v_flip_en;
    pTdri->imgi.imgi_stride = DMAImgi.dma_cfg.size.stride;

    //ISP_TPIPE_CONFIG_IMGBI_STRUCT
    pTdri->imgbi.imgbi_offset = DMAImgbi.dma_cfg.offset.x;
    pTdri->imgbi.imgbi_xsize = DMAImgbi.dma_cfg.size.xsize - 1;
    pTdri->imgbi.imgbi_ysize = DMAImgbi.dma_cfg.size.h - 1;
    pTdri->imgbi.imgbi_stride = DMAImgbi.dma_cfg.size.stride;

    //ISP_TPIPE_CONFIG_IMGCI_STRUCT
    pTdri->imgci.imgci_v_flip_en = DMAImgci.dma_cfg.v_flip_en;
    pTdri->imgci.imgci_stride = DMAImgci.dma_cfg.size.stride;

    //ISP_TPIPE_CONFIG_VIPI_STRUCT
    if(pTdri->top.vipi_en)
    {
        pTdri->vipi.vipi_v_flip_en = DMAVipi.dma_cfg.v_flip_en;
        pTdri->vipi.vipi_xsize = DMAVipi.dma_cfg.size.xsize - 1;
        pTdri->vipi.vipi_ysize = DMAVipi.dma_cfg.size.h - 1;
        pTdri->vipi.vipi_stride = DMAVipi.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->vipi, 0x00, sizeof(ISP_TPIPE_CONFIG_VIPI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_VIPBI_STRUCT
    if(pTdri->top.vipbi_en)
    {
        pTdri->vipbi.vipbi_v_flip_en = DMAVip2i.dma_cfg.v_flip_en;
        pTdri->vipbi.vipbi_xsize = DMAVip2i.dma_cfg.size.xsize - 1;
        pTdri->vipbi.vipbi_ysize = DMAVip2i.dma_cfg.size.h - 1;
        pTdri->vipbi.vipbi_stride = DMAVip2i.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->vipbi, 0x00, sizeof(ISP_TPIPE_CONFIG_VIPBI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_VIPCI_STRUCT
    //
    if(pTdri->top.vipci_en)
    {
        pTdri->vipci.vipci_v_flip_en = DMAVip3i.dma_cfg.v_flip_en;
        pTdri->vipci.vipci_xsize = DMAVip3i.dma_cfg.size.xsize - 1;
        pTdri->vipci.vipci_ysize = DMAVip3i.dma_cfg.size.h - 1;
        pTdri->vipci.vipci_stride = DMAVip3i.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->vipci, 0x00, sizeof(ISP_TPIPE_CONFIG_VIPCI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_UFDI_STRUCT
    if(pTdri->top.ufdi_en)
    {
        pTdri->ufdi.ufdi_v_flip_en = DMAUfdi.dma_cfg.v_flip_en;
        pTdri->ufdi.ufdi_xsize = DMAUfdi.dma_cfg.size.xsize - 1;
        pTdri->ufdi.ufdi_ysize = DMAUfdi.dma_cfg.size.h - 1;
        pTdri->ufdi.ufdi_stride = DMAUfdi.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->ufdi, 0x00, sizeof(ISP_TPIPE_CONFIG_UFDI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_LCEI_STRUCT
    if(pTdri->top.lcei_en)
    {
        pTdri->lcei.lcei_v_flip_en = DMALcei.dma_cfg.v_flip_en;
        pTdri->lcei.lcei_xsize = DMALcei.dma_cfg.size.xsize - 1;
        pTdri->lcei.lcei_ysize = DMALcei.dma_cfg.size.h - 1;
        pTdri->lcei.lcei_stride = DMALcei.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->lcei, 0x00, sizeof(ISP_TPIPE_CONFIG_LCEI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_DMGI_STRUCT
    if(pTdri->top.dmgi_en)
    {
        pTdri->dmgi.dmgi_v_flip_en = DMADmgi.dma_cfg.v_flip_en;
        pTdri->dmgi.dmgi_stride = DMADmgi.dma_cfg.size.stride;
        pTdri->dmgi.dmgi_offset = DMADmgi.dma_cfg.offset.x; // for non-image data, use x-offset to regard as general offset
        pTdri->dmgi.dmgi_xsize = DMADmgi.dma_cfg.size.xsize - 1;
        pTdri->dmgi.dmgi_ysize = DMADmgi.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->dmgi, 0x00, sizeof(ISP_TPIPE_CONFIG_DMGI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_DEPI_STRUCT
    if(pTdri->top.depi_en)
    {
        pTdri->depi.depi_v_flip_en = DMADepi.dma_cfg.v_flip_en;
        pTdri->depi.depi_stride = DMADepi.dma_cfg.size.stride;
        pTdri->depi.depi_xsize = DMADepi.dma_cfg.size.xsize - 1;
        pTdri->depi.depi_ysize = DMADepi.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->depi, 0x00, sizeof(ISP_TPIPE_CONFIG_DEPI_STRUCT));
    }

    //ISP_TPIPE_CONFIG_TIMGO_STRUCT
    if(pTdri->top.timgo_en)
    {
        pTdri->timgo.timgo_stride = DMATimgo.dma_cfg.size.stride;
        pTdri->timgo.timgo_xoffset = DMATimgo.dma_cfg.offset.x;
        pTdri->timgo.timgo_yoffset = DMATimgo.dma_cfg.offset.y;
        pTdri->timgo.timgo_xsize = DMATimgo.dma_cfg.size.xsize - 1;
        pTdri->timgo.timgo_ysize = DMATimgo.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->timgo, 0x00, sizeof(ISP_TPIPE_CONFIG_TIMGO_STRUCT));
    }

    //ISP_TPIPE_CONFIG_DCESO_STRUCT
    if(pTdri->top.dceso_en)
    {
        pTdri->dceso.dceso_xsize = DMADceso.dma_cfg.size.xsize - 1;
        pTdri->dceso.dceso_ysize = DMADceso.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->dceso, 0x00, sizeof(ISP_TPIPE_CONFIG_DCESO_STRUCT));
    }

    //ISP_TPIPE_CONFIG_IMG3O_STRUCT
    if(pTdri->top.img3o_en)
    {
        pTdri->img3o.img3o_stride = DMAImg3o.dma_cfg.size.stride;
        pTdri->img3o.img3o_xoffset = DMAImg3o.dma_cfg.offset.x;
        pTdri->img3o.img3o_yoffset = DMAImg3o.dma_cfg.offset.y;
        pTdri->img3o.img3o_xsize = DMAImg3o.dma_cfg.size.xsize - 1;
        pTdri->img3o.img3o_ysize = DMAImg3o.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->img3o, 0x00, sizeof(ISP_TPIPE_CONFIG_IMG3O_STRUCT));
    }

    //ISP_TPIPE_CONFIG_IMG3BO_STRUCT
    if(pTdri->top.img3bo_en)
    {
        pTdri->img3bo.img3bo_stride = DMAImg3bo.dma_cfg.size.stride;
        pTdri->img3bo.img3bo_xsize = DMAImg3bo.dma_cfg.size.xsize - 1;
        pTdri->img3bo.img3bo_ysize = DMAImg3bo.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->img3bo, 0x00, sizeof(ISP_TPIPE_CONFIG_IMG3BO_STRUCT));
    }

    //ISP_TPIPE_CONFIG_IMG3CO_STRUCT
    if(pTdri->top.img3co_en)
    {
        pTdri->img3co.img3co_stride = DMAImg3co.dma_cfg.size.stride;
        pTdri->img3co.img3co_xsize = DMAImg3co.dma_cfg.size.xsize - 1;
        pTdri->img3co.img3co_ysize = DMAImg3co.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->img3co, 0x00, sizeof(ISP_TPIPE_CONFIG_IMG3CO_STRUCT));
    }

    //ISP_TPIPE_CONFIG_CRZO_STRUCT
    if(pTdri->top.crzo_en)
    {
        pTdri->crzo.crzo_stride = DMAImg2o.dma_cfg.size.stride;
        pTdri->crzo.crzo_xoffset = DMAImg2o.dma_cfg.offset.x;
        pTdri->crzo.crzo_yoffset = DMAImg2o.dma_cfg.offset.y;
        pTdri->crzo.crzo_xsize = DMAImg2o.dma_cfg.size.xsize - 1;
        pTdri->crzo.crzo_ysize = DMAImg2o.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->crzo, 0x00, sizeof(ISP_TPIPE_CONFIG_CRZO_STRUCT));
    }

    //ISP_TPIPE_CONFIG_CRZBO_STRUCT
    if(pTdri->top.crzbo_en)
    {
        pTdri->crzbo.crzbo_stride = DMAImg2bo.dma_cfg.size.stride;
        pTdri->crzbo.crzbo_xoffset = DMAImg2bo.dma_cfg.offset.x;
        pTdri->crzbo.crzbo_yoffset = DMAImg2bo.dma_cfg.offset.y;
        pTdri->crzbo.crzbo_xsize = DMAImg2bo.dma_cfg.size.xsize - 1;
        pTdri->crzbo.crzbo_ysize = DMAImg2bo.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->crzbo, 0x00, sizeof(ISP_TPIPE_CONFIG_CRZBO_STRUCT));
    }

    //ISP_TPIPE_CONFIG_FEO_STRUCT
    pTdri->feo.feo_stride = DMAFeo.dma_cfg.size.stride;

    //ISP_TPIPE_CONFIG_UFD_STRUCT
    if (pUfdParam != NULL)
    {
        pTdri->ufd.ufd_bs2_au_start = pUfdParam->UFDG_BS_AU_START;
        pTdri->ufd.ufd_bond_mode = pUfdParam->UFDG_BOND_MODE;
    }
    else
    {
        DIP_FUNC_DBG("pUfdParam is NULL!");
        pTdri->ufd.ufd_bs2_au_start = 0;
        pTdri->ufd.ufd_bond_mode = 0;
    }
    pTdri->ufd.ufd_sel = (pMiscSel->Bits.DIPCTL_FG_MODE == 1) ? 0 : 1;

    //ISP_TPIPE_CONFIG_LSC_STRUCT
    pTdri->lsc.extend_coef_mode = tuningIspReg->LSC_D1A_LSC_CTL1.Bits.LSC_EXTEND_COEF_MODE;
    pTdri->lsc.sdblk_width = tuningIspReg->LSC_D1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
    pTdri->lsc.sdblk_xnum = tuningIspReg->LSC_D1A_LSC_CTL2.Bits.LSC_SDBLK_XNUM;
    pTdri->lsc.sdblk_last_width = tuningIspReg->LSC_D1A_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH;
    pTdri->lsc.sdblk_height = tuningIspReg->LSC_D1A_LSC_CTL3.Bits.LSC_SDBLK_HEIGHT;
    pTdri->lsc.sdblk_ynum = tuningIspReg->LSC_D1A_LSC_CTL3.Bits.LSC_SDBLK_YNUM;
    pTdri->lsc.sdblk_last_height = tuningIspReg->LSC_D1A_LSC_LBLOCK.Bits.LSC_SDBLK_lHEIGHT;

    //ISP_TPIPE_CONFIG_LTM_STRUCT
    pTdri->ltm.blk_x_num = tuningIspReg->LTM_D1A_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM;
    pTdri->ltm.blk_y_num = tuningIspReg->LTM_D1A_LTM_BLK_NUM.Bits.LTM_BLK_Y_NUM;
    pTdri->ltm.blk_width = tuningIspReg->LTM_D1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
    pTdri->ltm.blk_height = tuningIspReg->LTM_D1A_LTM_BLK_SZ.Bits.LTM_BLK_HEIGHT;

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp2.yuv_bit = 1; //SMTI_D2 10b
    pTdri->unp2.yuv_dng = 0; //SMTI_D2 pak

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp3.yuv_bit = 1; //SMTI_D3 10b
    pTdri->unp3.yuv_dng = 0; //SMTI_D3 pak

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp4.yuv_bit = 1; //SMTI_D4 10b
    pTdri->unp4.yuv_dng = 0; //SMTI_D4 pak

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp6.yuv_bit = DMAImgi.dma_cfg.yuv_bit;
    pTdri->unp6.yuv_dng = DMAImgi.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp7.yuv_bit = DMAImgbi.dma_cfg.yuv_bit;
    pTdri->unp7.yuv_dng = DMAImgbi.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp8.yuv_bit = DMAImgci.dma_cfg.yuv_bit;
    pTdri->unp8.yuv_dng = DMAImgci.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp9.yuv_bit = DMAVipi.dma_cfg.yuv_bit;
    pTdri->unp9.yuv_dng = DMAVipi.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp10.yuv_bit = DMAVip2i.dma_cfg.yuv_bit;
    pTdri->unp10.yuv_dng = DMAVip2i.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_UNP_STRUCT
    pTdri->unp11.yuv_bit = DMAVip3i.dma_cfg.yuv_bit;
    pTdri->unp11.yuv_dng = DMAVip3i.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_PAK_STRUCT
    pTdri->pak2.yuv_bit = 1; //SMTO_D2 10b
    pTdri->pak2.yuv_dng = 0; //SMTO_D2 pak

    //ISP_TPIPE_CONFIG_PAK_STRUCT
    pTdri->pak3.yuv_bit = 1; //SMTO_D3 10b
    pTdri->pak3.yuv_dng = 0; //SMTO_D3 pak

    //ISP_TPIPE_CONFIG_PAK_STRUCT
    pTdri->pak4.yuv_bit = 1; //SMTO_D4 10b
    pTdri->pak4.yuv_dng = 0; //SMTO_D4 pak

    //ISP_TPIPE_CONFIG_PAK_STRUCT
    pTdri->pak6.yuv_bit = DMAImg3o.dma_cfg.yuv_bit;
    pTdri->pak6.yuv_dng = DMAImg3o.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_PAK_STRUCT
    pTdri->pak7.yuv_bit = DMAImg3bo.dma_cfg.yuv_bit;
    pTdri->pak7.yuv_dng = DMAImg3bo.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_PAK_STRUCT
    pTdri->pak8.yuv_bit = DMAImg3co.dma_cfg.yuv_bit;
    pTdri->pak8.yuv_dng = DMAImg3co.dma_cfg.yuv_dng;

    //ISP_TPIPE_CONFIG_DM_STRUCT
    pTdri->dm.dm_byp = tuningIspReg->DM_D1A_DM_INTP_CRS.Bits.DM_BYP;

    //ISP_TPIPE_CONFIG_G2CX_STRUCT
    pTdri->g2cx.g2cx_shade_en = tuningIspReg->G2CX_D1A_G2CX_SHADE_CON_1.Bits.G2CX_SHADE_EN;
    pTdri->g2cx.g2cx_shade_xmid = tuningIspReg->G2CX_D1A_G2CX_SHADE_TAR.Bits.G2CX_SHADE_XMID;
    pTdri->g2cx.g2cx_shade_ymid = tuningIspReg->G2CX_D1A_G2CX_SHADE_TAR.Bits.G2CX_SHADE_YMID;
    pTdri->g2cx.g2cx_shade_var = tuningIspReg->G2CX_D1A_G2CX_SHADE_CON_1.Bits.G2CX_SHADE_VAR;

    //ISP_TPIPE_CONFIG_YNR_STRUCT
    pTdri->ynr.eny = tuningIspReg->YNR_D1A_YNR_CON1.Bits.YNR_ENY;
    pTdri->ynr.enc = tuningIspReg->YNR_D1A_YNR_CON1.Bits.YNR_ENC;
    pTdri->ynr.video_mode = tuningIspReg->YNR_D1A_YNR_CON1.Bits.YNR_VIDEO_MODE;
    pTdri->ynr.skin_link = tuningIspReg->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK;
    pTdri->ynr.lce_link = tuningIspReg->YNR_D1A_YNR_CON1.Bits.YNR_LCE_LINK;

    //ISP_TPIPE_CONFIG_CNR_STRUCT
    pTdri->cnr.bpc_en = tuningIspReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_BPC_EN;
    pTdri->cnr.enc = tuningIspReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_CNR_ENC;
    pTdri->cnr.scale_mode = tuningIspReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_CNR_SCALE_MODE;
    pTdri->cnr.mode = tuningIspReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_MODE;
    pTdri->cnr.video_mode = tuningIspReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_VIDEO_MODE;
    pTdri->cnr.abf_en = tuningIspReg->CNR_D1A_CNR_ABF_CON1.Bits.CNR_ABF_EN;

    //ISP_TPIPE_CONFIG_SLK_STRUCT
    pTdri->slk1.slk_hrz_comp = tuningIspReg->SLK_D1A_SLK_RZ.Bits.SLK_HRZ_COMP;
    pTdri->slk1.slk_vrz_comp = tuningIspReg->SLK_D1A_SLK_RZ.Bits.SLK_VRZ_COMP;

    //ISP_TPIPE_CONFIG_SLK_STRUCT
    pTdri->slk2.slk_hrz_comp = tuningIspReg->SLK_D2A_SLK_RZ.Bits.SLK_HRZ_COMP;
    pTdri->slk2.slk_vrz_comp = tuningIspReg->SLK_D2A_SLK_RZ.Bits.SLK_VRZ_COMP;

    //ISP_TPIPE_CONFIG_SLK_STRUCT
    pTdri->slk3.slk_hrz_comp = tuningIspReg->SLK_D3A_SLK_RZ.Bits.SLK_HRZ_COMP;
    pTdri->slk3.slk_vrz_comp = tuningIspReg->SLK_D3A_SLK_RZ.Bits.SLK_VRZ_COMP;

    //ISP_TPIPE_CONFIG_SLK_STRUCT
    pTdri->slk4.slk_hrz_comp = tuningIspReg->SLK_D4A_SLK_RZ.Bits.SLK_HRZ_COMP;
    pTdri->slk4.slk_vrz_comp = tuningIspReg->SLK_D4A_SLK_RZ.Bits.SLK_VRZ_COMP;

    //ISP_TPIPE_CONFIG_SLK_STRUCT
    pTdri->slk5.slk_hrz_comp = tuningIspReg->SLK_D5A_SLK_RZ.Bits.SLK_HRZ_COMP;
    pTdri->slk5.slk_vrz_comp = tuningIspReg->SLK_D5A_SLK_RZ.Bits.SLK_VRZ_COMP;

    //ISP_TPIPE_CONFIG_SLK_STRUCT
    pTdri->slk6.slk_hrz_comp = tuningIspReg->SLK_D6A_SLK_RZ.Bits.SLK_HRZ_COMP;
    pTdri->slk6.slk_vrz_comp = tuningIspReg->SLK_D6A_SLK_RZ.Bits.SLK_VRZ_COMP;

    //ISP_TPIPE_CONFIG_EE_STRUCT
    pTdri->ee.ee_out_edge_sel = tuningIspReg->EE_D1A_EE_TOP_CTRL.Bits.EE_OUT_EDGE_SEL;

    //ISP_TPIPE_CONFIG_LCE_STRUCT
    pTdri->lce.lce_lc_tone = tuningIspReg->LCE_D1A_LCE_GLOBAL.Bits.LCE_LC_TONE;
    pTdri->lce.lce_bc_mag_kubnx = tuningIspReg->LCE_D1A_LCE_ZR.Bits.LCE_BCMK_X;
    pTdri->lce.lce_slm_width = tuningIspReg->LCE_D1A_LCE_SLM_SIZE.Bits.LCE_SLM_WD;
    pTdri->lce.lce_bc_mag_kubny = tuningIspReg->LCE_D1A_LCE_ZR.Bits.LCE_BCMK_Y;
    pTdri->lce.lce_slm_height = tuningIspReg->LCE_D1A_LCE_SLM_SIZE.Bits.LCE_SLM_HT;
    pTdri->lce.lce_full_xoff =  tuningIspReg->LCE_D1A_LCE_OFFSET.Bits.LCE_TPIPE_OFFSET_X;
    pTdri->lce.lce_full_yoff =  tuningIspReg->LCE_D1A_LCE_OFFSET.Bits.LCE_TPIPE_OFFSET_Y;
    pTdri->lce.lce_full_slm_width =  tuningIspReg->LCE_D1A_LCE_SLM.Bits.LCE_TPIPE_SLM_WD;
    pTdri->lce.lce_full_slm_height =  tuningIspReg->LCE_D1A_LCE_SLM.Bits.LCE_TPIPE_SLM_HT;
    pTdri->lce.lce_full_out_height =  tuningIspReg->LCE_D1A_LCE_OUT.Bits.LCE_TPIPE_OUT_HT;

    //ISP_TPIPE_CONFIG_DCES_STRUCT
    pTdri->dces.dces_crop_xstart = tuningIspReg->DCES_D1A_DCES_CROP_X.Bits.DCES_CROP_XSTART;
    pTdri->dces.dces_crop_xend = tuningIspReg->DCES_D1A_DCES_CROP_X.Bits.DCES_CROP_XEND;
    pTdri->dces.dces_crop_ystart = tuningIspReg->DCES_D1A_DCES_CROP_Y.Bits.DCES_CROP_YSTART;
    pTdri->dces.dces_crop_yend = tuningIspReg->DCES_D1A_DCES_CROP_Y.Bits.DCES_CROP_YEND;

    //ISP_TPIPE_CONFIG_SRZ_STRUCT
    if(pTdri->top.srz1_en)
    {
        pTdri->srz1.srz_input_crop_width = srz1Cfg.crop.w;
        pTdri->srz1.srz_input_crop_height = srz1Cfg.crop.h;
        pTdri->srz1.srz_output_width = srz1Cfg.inout_size.out_w;
        pTdri->srz1.srz_output_height = srz1Cfg.inout_size.out_h;
        pTdri->srz1.srz_luma_horizontal_integer_offset = srz1Cfg.crop.x;
        pTdri->srz1.srz_luma_horizontal_subpixel_offset = srz1Cfg.crop.floatX;
        pTdri->srz1.srz_luma_vertical_integer_offset = srz1Cfg.crop.y;
        pTdri->srz1.srz_luma_vertical_subpixel_offset = srz1Cfg.crop.floatY;
        pTdri->srz1.srz_horizontal_coeff_step = srz1Cfg.h_step;
        pTdri->srz1.srz_vertical_coeff_step = srz1Cfg.v_step;
    }
    else
    {
        memset((void*)&pTdri->srz1, 0x00, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    }

    //ISP_TPIPE_CONFIG_SRZ_STRUCT
    if(pTdri->top.srz3_en)
    {
        pTdri->srz3.srz_input_crop_width = srz3Cfg.crop.w;
        pTdri->srz3.srz_input_crop_height = srz3Cfg.crop.h;
        pTdri->srz3.srz_output_width = srz3Cfg.inout_size.out_w;
        pTdri->srz3.srz_output_height = srz3Cfg.inout_size.out_h;
        pTdri->srz3.srz_luma_horizontal_integer_offset = srz3Cfg.crop.x;
        pTdri->srz3.srz_luma_horizontal_subpixel_offset = srz3Cfg.crop.floatX;
        pTdri->srz3.srz_luma_vertical_integer_offset = srz3Cfg.crop.y;
        pTdri->srz3.srz_luma_vertical_subpixel_offset = srz3Cfg.crop.floatY;
        pTdri->srz3.srz_horizontal_coeff_step = srz3Cfg.h_step;
        pTdri->srz3.srz_vertical_coeff_step = srz3Cfg.v_step;
    }
    else
    {
        memset((void*)&pTdri->srz3, 0x00, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    }

    //ISP_TPIPE_CONFIG_SRZ_STRUCT
    if(pTdri->top.srz4_en)
    {
        if (pYuvEn->Bits.DIPCTL_MIX_D2_EN == 1)
        {
            pTdri->srz4.srz_input_crop_width = srz4Cfg.inout_size.in_w;
            pTdri->srz4.srz_input_crop_height = srz4Cfg.inout_size.in_h;
        }
        else
        {
            pTdri->srz4.srz_input_crop_height = srz4Cfg.crop.h;
            pTdri->srz4.srz_input_crop_width = srz4Cfg.crop.w;
        }
#if 0 //5.0 srz_d4
        if ((pYuvEn->Bits.DIPCTL_YNR_D1_EN == 1) && (pYNRCon->Bits.YNR_LCE_LINK == 1)) //From TC Comment
        {
            pTdri->srz4.srz_input_crop_height = srz4Cfg.crop.h;
            pTdri->srz4.srz_input_crop_width = srz4Cfg.crop.w;
        }
        else
        {
            pTdri->srz4.srz_input_crop_width = srz4Cfg.inout_size.in_w;
            pTdri->srz4.srz_input_crop_height = srz4Cfg.inout_size.in_h;
        }
#endif
        pTdri->srz4.srz_output_width = srz4Cfg.inout_size.out_w;
        pTdri->srz4.srz_output_height = srz4Cfg.inout_size.out_h;
        pTdri->srz4.srz_luma_horizontal_integer_offset = srz4Cfg.crop.x;
        pTdri->srz4.srz_luma_horizontal_subpixel_offset = srz4Cfg.crop.floatX;
        pTdri->srz4.srz_luma_vertical_integer_offset = srz4Cfg.crop.y;
        pTdri->srz4.srz_luma_vertical_subpixel_offset = srz4Cfg.crop.floatY;
        pTdri->srz4.srz_horizontal_coeff_step = srz4Cfg.h_step;
        pTdri->srz4.srz_vertical_coeff_step = srz4Cfg.v_step;
    }
    else
    {
        memset((void*)&pTdri->srz4, 0x00, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    }

    //ISP_TPIPE_CONFIG_CRZ_STRUCT
    if(pTdri->top.crz_en)
    {
        pTdri->crz.crz_input_crop_width = crzPipe.crz_crop.w;
        pTdri->crz.crz_input_crop_height = crzPipe.crz_crop.h;
        pTdri->crz.crz_output_width = crzPipe.crz_out.w;
        pTdri->crz.crz_output_height = crzPipe.crz_out.h;
        pTdri->crz.crz_luma_horizontal_integer_offset = crzPipe.crz_crop.x;
        pTdri->crz.crz_luma_horizontal_subpixel_offset = crzPipe.crz_crop.floatX;
        pTdri->crz.crz_luma_vertical_integer_offset = crzPipe.crz_crop.y;
        pTdri->crz.crz_luma_vertical_subpixel_offset = crzPipe.crz_crop.floatY;
        pTdri->crz.crz_horizontal_luma_algorithm = crzPipe.hAlgo;
        pTdri->crz.crz_vertical_luma_algorithm = crzPipe.vAlgo;
        pTdri->crz.crz_horizontal_coeff_step = crzPipe.hCoeffStep;
        pTdri->crz.crz_vertical_coeff_step = crzPipe.vCoeffStep;
    }
    else
    {
        memset((void*)&pTdri->crz, 0x00, sizeof(ISP_TPIPE_CONFIG_CRZ_STRUCT));
    }

    //ISP_TPIPE_CONFIG_DFE_STRUCT
    pTdri->dfe.dfe_mode = tuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_MODE;

    //ISP_TPIPE_CONFIG_NR3D_STRUCT
    pTdri->nr3d.nr3d_on_en = tuningIspReg->NR3D_D1A_NR3D_NR3D_ON_CON.Bits.NR3D_on_en;
    pTdri->nr3d.nr3d_snr_en = tuningIspReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_en;
    pTdri->nr3d.nr3d_on_xoffset = tuningIspReg->NR3D_D1A_NR3D_NR3D_ON_OFF.Bits.NR3D_nr3d_on_ofst_x;
    pTdri->nr3d.nr3d_on_yoffset = tuningIspReg->NR3D_D1A_NR3D_NR3D_ON_OFF.Bits.NR3D_nr3d_on_ofst_y;
    pTdri->nr3d.nr3d_on_width = tuningIspReg->NR3D_D1A_NR3D_NR3D_ON_SIZ.Bits.NR3D_nr3d_on_wd;
    pTdri->nr3d.nr3d_on_height = tuningIspReg->NR3D_D1A_NR3D_NR3D_ON_SIZ.Bits.NR3D_nr3d_on_ht;

    //ISP_TPIPE_CONFIG_CRSP_STRUCT
    pTdri->crsp.crsp_ystep = 4;  /* Paul suggested setting 0x04 for this that same as rome */
    pTdri->crsp.crsp_xoffset = 0x00;  /* Paul suggested setting 0x00 for this that same as rome */
    pTdri->crsp.crsp_yoffset = 0x01;  /* Paul suggested setting 0x01 for this that same as rome */

    //ISP_TPIPE_CONFIG_SMT1_STRUCT , TC says smxo_sel support zero, if smxo_sel is the other value, tile driver will report error.
    pTdri->smt1.smt1o_sel = 0;//tuningIspReg->SMT_D1A_SMT_CTL.Bits.SMT_SMTO_SEL

    //ISP_TPIPE_CONFIG_SMT2_STRUCT
    pTdri->smt2.smt2o_sel = 0;//tuningIspReg->SMT_D2A_SMT_CTL.Bits.SMT_SMTO_SEL

    //ISP_TPIPE_CONFIG_SMT3_STRUCT
    pTdri->smt3.smt3o_sel = 0;//tuningIspReg->SMT_D3A_SMT_CTL.Bits.SMT_SMTO_SEL

    //ISP_TPIPE_CONFIG_SMT4_STRUCT
    pTdri->smt4.smt4o_sel = 0;//tuningIspReg->SMT_D4A_SMT_CTL.Bits.SMT_SMTO_SEL

    //ISP_TPIPE_CONFIG_SMT1I_STRUCT , TC says that you can take the smx buffer as one-dimension array.
    if(pTdri->top.smt1i_en)
    {
        pTdri->smt1i.smt1i_v_flip_en = 0;
        pTdri->smt1i.smt1i_xsize = (MAX_SMX1_BUF_SIZE - 1);
        pTdri->smt1i.smt1i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smt1i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMT1I_STRUCT));
    }

    //ISP_TPIPE_CONFIG_SMT1O_STRUCT
    pTdri->smt1o.smt1o_xsize = (MAX_SMX1_BUF_SIZE - 1);
    pTdri->smt1o.smt1o_ysize = 0;

    //ISP_TPIPE_CONFIG_SMT2I_STRUCT
    if(pTdri->top.smt2i_en)
    {
        pTdri->smt2i.smt2i_v_flip_en = 0;
        pTdri->smt2i.smt2i_xsize = (MAX_SMX2_BUF_SIZE - 1);
        pTdri->smt2i.smt2i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smt2i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMT2I_STRUCT));
    }

    //ISP_TPIPE_CONFIG_SMT2O_STRUCT
    pTdri->smt2o.smt2o_xsize = (MAX_SMX2_BUF_SIZE - 1);
    pTdri->smt2o.smt2o_ysize = 0;

    //ISP_TPIPE_CONFIG_SMT3I_STRUCT
    if(pTdri->top.smt3i_en)
    {
        pTdri->smt3i.smt3i_v_flip_en = 0;
        pTdri->smt3i.smt3i_xsize = (MAX_SMX3_BUF_SIZE - 1);
        pTdri->smt3i.smt3i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smt3i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMT3I_STRUCT));
    }

    //ISP_TPIPE_CONFIG_SMT3O_STRUCT
    pTdri->smt3o.smt3o_xsize = (MAX_SMX3_BUF_SIZE - 1);
    pTdri->smt3o.smt3o_ysize = 0;

    //ISP_TPIPE_CONFIG_SMT4I_STRUCT
    if(pTdri->top.smt4i_en)
    {
        pTdri->smt4i.smt4i_v_flip_en = 0;
        pTdri->smt4i.smt4i_xsize = (MAX_SMX4_BUF_SIZE - 1);
        pTdri->smt4i.smt4i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smt4i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMT4I_STRUCT));
    }

    //ISP_TPIPE_CONFIG_SMT4O_STRUCT
    pTdri->smt4o.smt4o_xsize = (MAX_SMX4_BUF_SIZE - 1);
    pTdri->smt4o.smt4o_ysize = 0;

#if 0 //MFB part
    //ISP_TPIPE_CONFIG_BLDI_STRUCT
    int bldi_v_flip_en;
    int bldi_stride;

    //ISP_TPIPE_CONFIG_BLDBI_STRUCT
    int bldbi_v_flip_en;
    int bldbi_stride;

    //ISP_TPIPE_CONFIG_BLD2I_STRUCT
    int bld2i_v_flip_en;
    int bld2i_stride;

    //ISP_TPIPE_CONFIG_BLD2BI_STRUCT
    int bld2bi_v_flip_en;
    int bld2bi_stride;

    //ISP_TPIPE_CONFIG_BLD3I_STRUCT
    int bld3i_v_flip_en;
    int bld3i_stride;

    //ISP_TPIPE_CONFIG_BLD4I_STRUCT
    int bld4i_v_flip_en;
    int bld4i_xsize;
    int bld4i_ysize;
    int bld4i_stride;

    //ISP_TPIPE_CONFIG_BLD_STRUCT
    int bld_deblock_en;
    int bld_brz_en;
    int bld_mbd_wt_en;

    //ISP_TPIPE_CONFIG_BLDO_STRUCT
    int bldo_stride;
    int bldo_xoffset;
    int bldo_yoffset;
    int bldo_xsize;
    int bldo_ysize;

    //ISP_TPIPE_CONFIG_BLDBO_STRUCT
    int bldbo_stride;

    //ISP_TPIPE_CONFIG_BLD2O_STRUCT
    int bld2o_stride;
    int bld2o_xoffset;
    int bld2o_yoffset;
    int bld2o_xsize;
    int bld2o_ysize;

    //ISP_TPIPE_CONFIG_SRZ_STRUCT mfb_srz; ????
    if(pTdri->top.mfb_srz)
    {
        pTdri->mfb_srz.srz_input_crop_width = mfb_srz.crop.w;
        pTdri->mfb_srz.srz_input_crop_height = mfb_srz.crop.h;
        pTdri->mfb_srz.srz_output_width = mfb_srz.inout_size.out_w;
        pTdri->mfb_srz.srz_output_height = mfb_srz.inout_size.out_h;
        pTdri->mfb_srz.srz_luma_horizontal_integer_offset = mfb_srz.crop.x;
        pTdri->mfb_srz.srz_luma_horizontal_subpixel_offset = mfb_srz.crop.floatX;
        pTdri->mfb_srz.srz_luma_vertical_integer_offset = mfb_srz.crop.y;
        pTdri->mfb_srz.srz_luma_vertical_subpixel_offset = mfb_srz.crop.floatY;
        pTdri->mfb_srz.srz_horizontal_coeff_step = mfb_srz.h_step;
        pTdri->mfb_srz.srz_vertical_coeff_step = mfb_srz.v_step;
    }
    else
    {
        memset((void*)&pTdri->mfb_srz, 0x00, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    }

    //ISP_TPIPE_CONFIG_CRSPB_STRUCT crspb; mfb
    pTdri->crspb.crspb_ystep = 0;
    pTdri->crspb.crspb_xoffset = 0;
    pTdri->crspb.crspb_yoffset = 0;
#endif

#if 0 //ADAS part
    //ISP_TPIPE_CONFIG_OFT_STRUCT
    pTdri->oft.oft_crop_en = 0;
    pTdri->oft.oft_crz_en = 0;
    pTdri->oft.oft_dth_en = 0;
    int oft_bcvt_en;
    pTdri->oft.oft_c24_en = 0;
    pTdri->oft.oft_c2g_en = 0;
    pTdri->oft.oft_iggm_en = 0;
    pTdri->oft.oft_ccm_en = 0;
    pTdri->oft.oft_gce_en = 0;
    pTdri->oft.oft_g2c_en = 0;
    pTdri->oft.oft_c42_en = 0;
    pTdri->oft.oft_c20_en = 0;
    pTdri->oft.oft_r2b_en = 0;
    pTdri->oft.oft_hsv_en = 0;
    pTdri->oft.oft_lab_en = 0;
    // crz
    pTdri->oft.oft_crz_input_crop_width = 0;
    pTdri->oft.oft_crz_input_crop_height = 0;
    pTdri->oft.oft_crz_output_width = 0;
    pTdri->oft.oft_crz_output_height = 0;
    pTdri->oft.oft_crz_luma_horizontal_integer_offset = 0;
    pTdri->oft.oft_crz_luma_horizontal_subpixel_offset = 0;
    pTdri->oft.oft_crz_luma_vertical_integer_offset = 0;
    pTdri->oft.oft_crz_luma_vertical_subpixel_offset = 0;
    pTdri->oft.oft_crz_horizontal_luma_algorithm = 0;
    pTdri->oft.oft_crz_vertical_luma_algorithm = 0;
    pTdri->oft.oft_crz_horizontal_coeff_step = 0;
    pTdri->oft.oft_crz_vertical_coeff_step = 0;
    // r2b
    int oft_c20_xoffset;
    int oft_c20_yoffset;
    pTdri->oft.oft_r2b_ch0_stride = 0;
    pTdri->oft.oft_r2b_ch1_stride = 0;
    pTdri->oft.oft_r2b_ch2_stride = 0;
    int oft_r2b_stride;
    pTdri->oft.oft_r2b_xoffset = 0;
    pTdri->oft.oft_r2b_yoffset = 0;
    pTdri->oft.oft_r2b_xsize = 0;
    pTdri->oft.oft_r2b_ysize = 0;
    pTdri->oft.oft_r2b_format = 0;
    pTdri->oft.oft_r2b_plane = 0;
    pTdri->oft.oft_r2b_comp_mode = 0;
    pTdri->oft.oft_r2b_comp_en = 0;
    pTdri->oft.oft_r2b_out_depth = 0;
    int oft_ext_isp_format;

    //ISP_TPIPE_CONFIG_OFTL_STRUCT
    pTdri->oftl.oftl_crop_en = 0;
    pTdri->oftl.oftl_crz_en = 0;
    pTdri->oftl.oftl_dth_en = 0;
    int oftl_bcvt_en;
    pTdri->oftl.oftl_c24_en = 0;
    pTdri->oftl.oftl_c2g_en = 0;
    pTdri->oftl.oftl_iggm_en = 0;
    pTdri->oftl.oftl_ccm_en = 0;
    pTdri->oftl.oftl_gce_en = 0;
    pTdri->oftl.oftl_g2c_en = 0;
    pTdri->oftl.oftl_c42_en = 0;
    pTdri->oftl.oftl_c20_en = 0;
    pTdri->oftl.oftl_r2b_en = 0;
    //pTdri->oftl.oftl_hsv_en = 0;
    //pTdri->oftl.oftl_lab_en = 0;
    // crz
    pTdri->oftl.oftl_crz_input_crop_width = 0;
    pTdri->oftl.oftl_crz_input_crop_height = 0;
    pTdri->oftl.oftl_crz_output_width = 0;
    pTdri->oftl.oftl_crz_output_height = 0;
    pTdri->oftl.oftl_crz_luma_horizontal_integer_offset = 0;
    pTdri->oftl.oftl_crz_luma_horizontal_subpixel_offset = 0;
    pTdri->oftl.oftl_crz_luma_vertical_integer_offset = 0;
    pTdri->oftl.oftl_crz_luma_vertical_subpixel_offset = 0;
    pTdri->oftl.oftl_crz_horizontal_luma_algorithm = 0;
    pTdri->oftl.oftl_crz_vertical_luma_algorithm = 0;
    pTdri->oftl.oftl_crz_horizontal_coeff_step = 0;
    pTdri->oftl.oftl_crz_vertical_coeff_step = 0;
    // r2b
    int oftl_c20_xoffset;
    int oftl_c20_yoffset;
    pTdri->oftl.oftl_r2b_ch0_stride = 0;
    pTdri->oftl.oftl_r2b_ch1_stride = 0;
    pTdri->oftl.oftl_r2b_xoffset = 0;
    pTdri->oftl.oftl_r2b_yoffset = 0;
    pTdri->oftl.oftl_r2b_xsize = 0;
    pTdri->oftl.oftl_r2b_ysize = 0;
    pTdri->oftl.oftl_r2b_format = 0;
    pTdri->oftl.oftl_r2b_plane = 0;
    pTdri->oftl.oftl_r2b_comp_mode = 0;
    pTdri->oftl.oftl_r2b_comp_en = 0;
    pTdri->oftl.oftl_r2b_out_depth = 0;
#endif

    return ret;

}
int DIP_ISP_PIPE::ConfigDipPipe(DipWorkingBuffer* pDipWBuf)
{
    int i;
    int             ret_val = 0;
    IspFunctionDip_B**  isplist;
    int             ispcount;

    //Get Dip Working Buffer
    m_pDipWBuf = pDipWBuf;
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->setCQDescBufPhyAddr(pDipWBuf->m_pIspDescript_phy);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->setCQDescBufVirAddr(pDipWBuf->m_pIspDescript_vir);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->setIspVirRegPhyAddr(pDipWBuf->m_pIspVirRegAddr_pa);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->setIspVirRegVirAddr(pDipWBuf->m_pIspVirRegAddr_va);
	m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->setCQDescBufOft(pDipWBuf->m_IspDescript_oft);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->setIspVirRegOft(pDipWBuf->m_IspVirRegAddr_oft);
    //pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa = pDipWBuf->tpipeTableVa;
    //pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa = pDipWBuf->tpipeTablePa;
    //pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableOft = pDipWBuf->tpipeTableOft;

    //initiate cq descriptor buffer
    for(int m=0;m<(DIP_A_MODULE_MAX - 1);m++){
        pDipWBuf->m_pIspDescript_vir->u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
        pDipWBuf->m_pIspDescript_vir->v_reg_addr = 0;
    }

    DIP_FUNC_DBG("moduleIdx(%d), pDipWBuf(%p), m_pIspDescript_phy(%p), m_pIspDescript_vir(%p), m_pIspVirRegAddr_pa(%p), m_pIspVirRegAddr_va(%p), tpipeTableVa(%p), tpipeTablePa(%lx)",
        this->moduleIdx, pDipWBuf, pDipWBuf->m_pIspDescript_phy, pDipWBuf->m_pIspDescript_vir, pDipWBuf->m_pIspVirRegAddr_pa, pDipWBuf->m_pIspVirRegAddr_va, pDipWBuf->tpipeTableVa, pDipWBuf->tpipeTablePa);

    this->configTpipeData();

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            DIP_FUNC_DBG("[%d]<%s> bypass:",i,isplist[i]->name_Str());
            continue;
        }

        DIP_FUNC_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config(moduleIdx) ) < 0 )
        {
            DIP_FUNC_ERR("[ERROR] _config(%s) ",isplist[i]->name_Str());
            break;
        }

        if( ( ret_val = isplist[i]->write2CQ(moduleIdx) ) < 0 )
        {
            DIP_FUNC_ERR("[ERROR] _config dipTh(%d) ", this->dipTh);
            break;
        }

    }

    return MTRUE;

}

MBOOL DIP_ISP_PIPE::CalAlgoAndCStep(
    CRZ_DRV_MODE_ENUM       eFrameOrTpipeOrVrmrg,
    CRZ_DRV_RZ_ENUM         eRzName,
    MUINT32                 SizeIn_H,
    MUINT32                 SizeIn_V,
    MUINT32                 u4CroppedSize_H,
    MUINT32                 u4CroppedSize_V,
    MUINT32                 SizeOut_H,
    MUINT32                 SizeOut_V,
    CRZ_DRV_ALGO_ENUM       *pAlgo_H,
    CRZ_DRV_ALGO_ENUM       *pAlgo_V,
    MUINT32                 *pTable_H,
    MUINT32                 *pTable_V,
    MUINT32                 *pCoeffStep_H,
    MUINT32                 *pCoeffStep_V)
{
    return this->m_pIspDrvShell->m_pDipCrzDrv->CalAlgoAndCStep(eFrameOrTpipeOrVrmrg,
        eRzName,SizeIn_H,SizeIn_V,u4CroppedSize_H,u4CroppedSize_V,SizeOut_H,SizeOut_V,pAlgo_H,pAlgo_V,pTable_H,pTable_V,pCoeffStep_H,pCoeffStep_V);
}

int DIP_ISP_PIPE::config( unsigned int dipidx )
{
    (void)dipidx;
    MINT32 ret = MTRUE;
    MBOOL crz_result = MTRUE;
    MINT32 result = MTRUE;
    MUINT32 i;

    ISP_TPIPE_CONFIG_STRUCT *pTdri = NULL;
    DIP_FUNC_DBG("[DIP_TOP_CTRL]moduleIdx(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),RingBufIdx(%d),isApplyTuning(%d)",\
        this->moduleIdx,this->moduleIdx,this->dipTh,this->burstQueIdx,this->RingBufIdx,this->isApplyTuning);

    DIP_FUNC_DBG("YUV_EN(0x%08x),YUV2_EN(0x%08x),RGB_EN(0x%08x),FMT_SEL(0x%08x),MUX_SEL(0x%08x),MUX2_SEL(0x%08x)",\
        this->isp_top_ctl.YUV_EN.Raw,this->isp_top_ctl.YUV2_EN.Raw,this->isp_top_ctl.RGB_EN.Raw,\
        this->isp_top_ctl.FMT_SEL.Raw,this->isp_top_ctl.MUX_SEL.Raw,this->isp_top_ctl.MUX2_SEL.Raw);

    // TODO: hungwen modify
    this->pDrvDip = m_pIspDrvShell->m_pDrvDip[this->moduleIdx];
    this->pDrvDipPhy = this->m_pIspDrvShell->m_pDrvDipPhy;

    /********************************************************************************************************************/
    // set registers to dip thread
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_RGB_EN1, this->isp_top_ctl.RGB_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_RGB_EN2, this->isp_top_ctl.RGB2_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_YUV_EN1, this->isp_top_ctl.YUV_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_YUV_EN2, this->isp_top_ctl.YUV2_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_DMA_EN1, this->isp_top_ctl.DMA_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_DMA_EN2, this->isp_top_ctl.DMA2_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_MUX_SEL1, this->isp_top_ctl.MUX_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_MUX_SEL2, this->isp_top_ctl.MUX2_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_FMT_SEL1, this->isp_top_ctl.FMT_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_FMT_SEL2, this->isp_top_ctl.FMT2_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_MISC_SEL, this->isp_top_ctl.MISC_SEL.Raw);


    if (m_pIspDrvShell->m_iDipMetEn)
    {
        //MET Log Bufer
        pTdri = &m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo;
        if (pTdri != NULL){
            //pTdri->drvinfo.isp2mdpcfg.MET_String = m_pDipWBuf->m_pMetLogBuf;
            //pTdri->drvinfo.isp2mdpcfg.MET_String_length = MAX_METLOG_SIZE;
            pTdri->drvinfo.m_pMetLogBuf = m_pDipWBuf->m_pMetLogBuf;
            pTdri->drvinfo.m_MetLogBufSize = MAX_METLOG_SIZE;
            snprintf(pTdri->drvinfo.m_pMetLogBuf, MAX_METLOG_SIZE, "DIP_A__CTL_YUV_EN=%d,DIP_A__CTL_YUV2_EN=%d,\
            DIP_A__CTL_RGB_EN=%d,DIP_A__CTL_RGB2_EN=%d,DIP_A__CTL_DMA_EN=%d,DIP_A__CTL_DMA2_EN=%d",\
            this->isp_top_ctl.YUV_EN.Raw, this->isp_top_ctl.YUV2_EN.Raw,\
            this->isp_top_ctl.RGB_EN.Raw, this->isp_top_ctl.RGB2_EN.Raw, this->isp_top_ctl.DMA_EN.Raw, this->isp_top_ctl.DMA2_EN.Raw);
        }
    }

#if 0
      _CQ_CONFIG_INFO_ cqcfginfo;
    cqcfginfo.moduleIdx=this->moduleIdx;
    cqcfginfo.dipTh=this->dipTh;
    cqcfginfo.dipCQ_dupIdx=this->dupCqIdx;
    cqcfginfo.dipCQ_burstIdx=this->burstQueIdx;
    cqcfginfo.isDipOnly=this->isDipOnly;
    cqcfginfo.pDipWBuf= this->m_pDipWBuf;
    pthread_mutex_lock(&mCQCfgInfoMutex);
    mFrameCQCfgInfoList.push_back(cqcfginfo);
    pthread_mutex_unlock(&mCQCfgInfoMutex);
#endif

    DIP_FUNC_DBG("cqcfginfo.pDipWBuf (%p), desphy:%p,desvir:%p,virispphy:%p,virispvir:%p", m_pDipWBuf, m_pDipWBuf->m_pIspDescript_phy, m_pDipWBuf->m_pIspDescript_vir
        ,m_pDipWBuf->m_pIspVirRegAddr_pa, m_pDipWBuf->m_pIspVirRegAddr_va);


    //DIP_RAW_PIPE Domain
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].default_func!=NULL)) {
            gIspModuleMappingTableRbgEn[i].default_func((*this));
        }
        //BPC force flush
        if ((!(this->isp_top_ctl.RGB_EN.Raw & DIPCTL_RGB_EN1_BPC_D1)) && (gIspModuleMappingTableRbgEn[i].default_func!=NULL))
        {
            gIspModuleMappingTableRbgEn[i].default_func((*this)); // write BPC disable;
        }
    }

    for(i=0;i<CHECK_CTL_RAW_RGB2_EN_BITS;i++){
        if((gIspModuleMappingTableRbg2En[i].ctrlByte & this->isp_top_ctl.RGB2_EN.Raw)&&\
                (gIspModuleMappingTableRbg2En[i].default_func!=NULL)) {
            gIspModuleMappingTableRbg2En[i].default_func((*this));
        }
    }


    //DIP_YUV_PIPE
    for(i=0;i<CHECK_CTL_YUV_EN_BITS;i++){
        if((gIspModuleMappingTableYuvEn[i].ctrlByte & this->isp_top_ctl.YUV_EN.Raw)&&\
                (gIspModuleMappingTableYuvEn[i].default_func!=NULL)) {
            gIspModuleMappingTableYuvEn[i].default_func((*this));
        }
    }
    //
    for(i=0;i<CHECK_CTL_YUV2_EN_BITS;i++){
        if((gIspModuleMappingTableYuv2En[i].ctrlByte & this->isp_top_ctl.YUV2_EN.Raw)&&\
                (gIspModuleMappingTableYuv2En[i].default_func!=NULL)) {
            gIspModuleMappingTableYuv2En[i].default_func((*this));
        }
    }

    //LCE and HLR
#if 0
    if ((this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_LCE) || (this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_HLR2))
    {
        DIP_WRITE_REG(this->pDrvDip,DIP_X_HLR2_CFG,this->pTuningIspReg->DIP_X_HLR2_CFG.Raw);
    }
#endif
    //hw default 1, you need to modify it to zero when init and lce enable will the value chnage to 1, and lce disalbe chnage the value to 0
    if (this->isp_top_ctl.YUV_EN.Raw & DIPCTL_YUV_EN1_LCE_D1)
    {
        DIP_WRITE_BITS(this->pDrvDip, DIPCTL_D1A_DIPCTL_MISC_SEL, DIPCTL_YNR_GMAP_LTM_MODE,0x1);
    }
    else
    {
        DIP_WRITE_BITS(this->pDrvDip, DIPCTL_D1A_DIPCTL_MISC_SEL, DIPCTL_YNR_GMAP_LTM_MODE,0x0);
    }

    //UFDI
    if ((this->isp_top_ctl.RGB_EN.Raw & DIPCTL_RGB_EN1_UFD_D1))
    {
        DIP_WRITE_BITS(this->pDrvDip, DIPDMATOP_D1A_DIPDMATOP_SPECIAL_FUN_EN, DIPDMATOP_UFO_IMGI_D1_EN,0x1);
    }
    else
    {
        DIP_WRITE_BITS(this->pDrvDip, DIPDMATOP_D1A_DIPDMATOP_SPECIAL_FUN_EN, DIPDMATOP_UFO_IMGI_D1_EN,0x0);
    }

    if (m_pIspDrvShell->m_bDCMEn == 0) {
        //Workaround for DCES
        if (!(this->isp_top_ctl.YUV_EN.Raw & DIPCTL_YUV_EN1_DCES_D1))
        {
           DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_YUV_DCM_DIS1, 0x0);     
        }
    } else {
        if (!(this->isp_top_ctl.YUV_EN.Raw & DIPCTL_YUV_EN1_DCES_D1))
        {
           DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_YUV_DCM_DIS1, 0xffffdfff);     
        } else {
           DIP_WRITE_REG(this->pDrvDip, DIPCTL_D1A_DIPCTL_YUV_DCM_DIS1, 0xffffffff); 
        }
    }
    //NR3D
    if ((!(this->isp_top_ctl.YUV2_EN.Raw & DIPCTL_YUV_EN2_NR3D_D1)) && (this->pTuningIspReg!=NULL))
    {
        DIP_WRITE_BITS(this->pDrvDip, NR3D_D1A_NR3D_NR3D_ENG_CON, NR3D_yclnr_en, this->pTuningIspReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_yclnr_en); //YWtodo
        DIP_WRITE_BITS(this->pDrvDip, NR3D_D1A_NR3D_NR3D_TNR_ENABLE, NR3D_tnr_y_en, this->pTuningIspReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_y_en); //YWtodo
    }

    //Color
    if ((!(this->isp_top_ctl.YUV_EN.Raw & DIPCTL_YUV_EN1_COLOR_D1)) && (this->pTuningIspReg!=NULL))
    {
        DIP_WRITE_BITS(this->pDrvDip, COLOR_D1A_COLOR_DIP_COLOR_START, COLOR_disp_color_start,this->pTuningIspReg->COLOR_D1A_COLOR_DIP_COLOR_START.Bits.COLOR_disp_color_start);
    }

    if ( this->crzPipe.conf_cdrz ) {
        DIP_FUNC_DBG("CDRZ_Config ");
        CRZ_DRV_IMG_SIZE_STRUCT CrzsizeIn;
        CRZ_DRV_IMG_SIZE_STRUCT CrzsizeOut;
        CRZ_DRV_IMG_CROP_STRUCT Crzcrop;

        //CDRZ
        CrzsizeIn.Width = this->crzPipe.crz_in.w;
        CrzsizeIn.Height = this->crzPipe.crz_in.h;
        CrzsizeOut.Width = this->crzPipe.crz_out.w;
        CrzsizeOut.Height = this->crzPipe.crz_out.h;
        // crop
        Crzcrop.Width.Start = this->crzPipe.crz_crop.x;
        Crzcrop.Width.Size = this->crzPipe.crz_crop.w;
        Crzcrop.Height.Start = this->crzPipe.crz_crop.y;
        Crzcrop.Height.Size = this->crzPipe.crz_crop.h;

        result = this->m_pIspDrvShell->m_pDipCrzDrv->SetIspDrv(this->pDrvDip);
        crz_result = this->m_pIspDrvShell->m_pDipCrzDrv->CDRZ_Config(CRZ_DRV_MODE_TPIPE,CrzsizeIn,CrzsizeOut,Crzcrop);

        if (MFALSE == crz_result) {
            DIP_FUNC_ERR("[Error]CDRZ_Config fail");
            ret = MFALSE;
        }
    }

    return ret;
}


int DIP_ISP_PIPE::startMDP(MINT32 const p2CQ, MUINT32 const RingBufIdx, MINT32 const burstQIdx, MUINT32 const frameNum)
{
    MINT32 errSta;
    MINT32 ret = MTRUE;
    MUINT32 frameflag=0;
    DipRingBuffer* pDipRingBuffer=NULL;
    MUINT32 ref_ringbufidx=RingBufIdx;
    DipWorkingBuffer* pDipWBuf;
    //
#if 0
     _CQ_CONFIG_INFO_ cqcfginfo;
    list<_CQ_CONFIG_INFO_>::iterator it;
    pthread_mutex_lock(&mCQCfgInfoMutex);
    it = mFrameCQCfgInfoList.begin();
    cqcfginfo.moduleIdx=(*it).moduleIdx;
    cqcfginfo.dipTh=(*it).dipTh;
    cqcfginfo.dipCQ_dupIdx=(*it).dipCQ_dupIdx;
    cqcfginfo.dipCQ_burstIdx=(*it).dipCQ_burstIdx;
    cqcfginfo.isDipOnly=(*it).isDipOnly;
    cqcfginfo.pDipWBuf=(*it).pDipWBuf;
    mFrameCQCfgInfoList.pop_front();
    pthread_mutex_unlock(&mCQCfgInfoMutex);
    //pParam;
#endif

    pDipWBuf = m_pIspDrvShell->m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)p2CQ, RingBufIdx);
    pDipRingBuffer = this->m_pIspDrvShell->getDipRingBufMgr();
    if (pDipRingBuffer != NULL)
    {
        pDipRingBuffer->skipTpipeCalculatebyIdx((E_ISP_DIP_CQ)p2CQ,frameflag,ref_ringbufidx,pDipWBuf,frameNum);
        pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = frameflag;
        DIP_FUNC_DBG("thiscq(%d), frameflag(0x%x), ref_ringbufidx(%d)",p2CQ, frameflag, ref_ringbufidx);
    }
    else
    {
        DIP_FUNC_ERR("thiscq(%d), RingBufIdx(0x%x), frameflag(0x%x), ref_ringbufidx(%d)",p2CQ, RingBufIdx, frameflag, ref_ringbufidx);
        return -1;
    }
    pDipWBuf->m_pMdpCfg->MDPCallback = this->MDPCallback;
    pDipWBuf->m_pMdpCfg->MDPparam = this->MDPparam;
    ret=m_pIspDrvShell->m_pMdpMgr->updateDpStreamMap(frameflag,p2CQ, RingBufIdx, ref_ringbufidx);

    DIP_FUNC_DBG("[_enable] hwm(%d_%d_%d_%d_%p_desphy:%p,desvir:%p,virispphy:%p,virispvir:%p)",\
        p2CQ,RingBufIdx, burstQIdx, frameNum, pDipWBuf, pDipWBuf->m_pIspDescript_phy, pDipWBuf->m_pIspDescript_vir,
        pDipWBuf->m_pIspVirRegAddr_pa, pDipWBuf->m_pIspVirRegAddr_va);

    // TODO: hungwen modify
    //VirDipDrv* pDrvDip = (VirDipDrv*)m_pIspDrvShell->m_pDrvDip[cqcfginfo.moduleIdx];

    //errSta = m_pIspDrvShell->m_pMdpMgr->startMdp((*cqcfginfo.pDipWBuf->m_pMdpCfg));
    errSta = m_pIspDrvShell->m_pMdpMgr->startMdp((*pDipWBuf->m_pMdpCfg));
    if(MDPMGR_NO_ERROR != errSta){
        DIP_FUNC_ERR("[Error]p2CQ:(%d), RingBufIdx:(%d) pDipWBuf:(%p), m_pIspDrvShell->m_pMdpMgr->startMdp fail(%d)",p2CQ, RingBufIdx, pDipWBuf,errSta);
        ret = -1;
        goto EXIT;
    }


EXIT:
    DIP_FUNC_DBG("ret(%d)", ret);
    return ret;

}

int DIP_ISP_PIPE::enable( void* pParam )
{
    (void)pParam;
    return 0;
}

int DIP_ISP_PIPE::disable( void* pParam )
{
    (void)pParam;

    return 0;
}


int DIP_ISP_PIPE::write2CQ(unsigned int dipidx)
{
    (void)dipidx;
    MUINT32 i;

    DIP_FUNC_DBG("[DIP_TOP_CTRL]moduleIdx(%d),hwTh(%d),burstQueIdx(%d),RingBufIdx(%d)",\
        this->moduleIdx,this->dipTh,this->burstQueIdx,this->RingBufIdx);

    // TODO: hungwen modify
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx][this->dipTh][this->burstQueIdx][this->dupCqIdx]->cqAddModule(DIP_A_CTL);
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_EN); /*via gce*/
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_MUX);
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_FMT);

    //DIP_RAW_PIPE Domain
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].eCqThrFunc!=DIP_A_DUMMY)) {
            // TODO: hungwen modify
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableRbgEn[i].eCqThrFunc);
        }
    }
    if(DIPCTL_RGB_EN1_LTM_D1 & this->isp_top_ctl.RGB_EN.Raw){
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_LTMTC_D1_A);
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_LTMTC_D1_B);
    }

    for(i=0;i<CHECK_CTL_RAW_RGB2_EN_BITS;i++){
        if((gIspModuleMappingTableRbg2En[i].ctrlByte & this->isp_top_ctl.RGB2_EN.Raw)&&\
                (gIspModuleMappingTableRbg2En[i].eCqThrFunc!=DIP_A_DUMMY)) {
            // TODO: hungwen modify
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableRbg2En[i].eCqThrFunc);
        }
    }

    //DIP_YUV_PIPE
    for(i=0;i<CHECK_CTL_YUV_EN_BITS;i++){
        if((gIspModuleMappingTableYuvEn[i].ctrlByte & this->isp_top_ctl.YUV_EN.Raw)&&\
                (gIspModuleMappingTableYuvEn[i].eCqThrFunc!=DIP_A_DUMMY)) {
                // TODO: hungwen modify
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuvEn[i].eCqThrFunc);
        }
    }

    for(i=0;i<CHECK_CTL_YUV2_EN_BITS;i++){
        if((gIspModuleMappingTableYuv2En[i].ctrlByte & this->isp_top_ctl.YUV2_EN.Raw)&&\
                (gIspModuleMappingTableYuv2En[i].eCqThrFunc!=DIP_A_DUMMY)) {
                // TODO: hungwen modify
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuv2En[i].eCqThrFunc);
        }
    }
    //LCE and HLR
#if 0
    if ((this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_LCE) || (this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_HLR2))
    {
        // TODO: hungwen modify
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_HLR2);
        //m_pIspDrvShell->m_pDrvDip[this->moduleIdx][this->dipTh][this->burstQueIdx][this->dupCqIdx]->cqAddModule(DIP_A_HLR2);
    }
#endif
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_VECTICAL_FLIP);

    //UFDI
    // TODO: hungwen modify
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_SPECIAL_FUN);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_PAK_D2);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_PAK_D3);
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_PAK_D4);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_UNP_D2);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_UNP_D3);
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_UNP_D4);

    //Workaround for DCES
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_DCES_DCM);
    
    
    //NR3D
    if (!(this->isp_top_ctl.YUV2_EN.Raw & DIPCTL_YUV_EN2_NR3D_D1))
    {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_NR3D_D1);
    }

    //Color
    if (!(this->isp_top_ctl.YUV_EN.Raw & DIPCTL_YUV_EN1_COLOR_D1))
    {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_COLOR_D1);
    }

    //BPC disable flush BPC setting
    if (!(this->isp_top_ctl.RGB_EN.Raw & DIPCTL_RGB_EN1_BPC_D1))
    {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_BPC_D1);
    }

    //CRZ
    // TODO: hungwen modify
    if ( this->crzPipe.conf_cdrz ) {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CRZ_D1);
    }

    //TPIPE Ctrl
    // TODO: hungwen modify
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_TDR);

    return MTRUE;
}


/*/////////////////////////////////////////////////////////////////////////////
    DIP_MDP_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MBOOL DIP_MDP_PIPE::startVencLink( MINT32 fps, MINT32 wd,MINT32 ht)
{
    DIP_FUNC_DBG("+");
    MBOOL err = MTRUE;
    if(m_pIspDrvShell->m_pMdpMgr)
    {
        MUINT32 ret=MDPMGR_NO_ERROR;
        ret=m_pIspDrvShell->m_pMdpMgr->startVencLink(fps,wd,ht);
        if(ret !=MDPMGR_NO_ERROR)
        {
            DIP_FUNC_ERR("[Error]startVencLink fail");
            err=MFALSE;
        }
    }
    else
    {
        err = MFALSE;
        DIP_FUNC_ERR("[Error]m_pIspDrvShell->m_pMdpMgr = NULL");
    }
    DIP_FUNC_DBG("-");
    return err;
}
MBOOL DIP_MDP_PIPE::stopVencLink( )
{
    DIP_FUNC_DBG("+");
    MBOOL err = MTRUE;
    if(m_pIspDrvShell->m_pMdpMgr)
    {
        MUINT32 ret=MDPMGR_NO_ERROR;
        m_pIspDrvShell->m_pMdpMgr->stopVencLink();
        if(ret !=MDPMGR_NO_ERROR)
        {
            DIP_FUNC_ERR("[Error]stopVencLink fail");
            err=MFALSE;
        }
    }
    else
    {
        err = MFALSE;
        DIP_FUNC_ERR("[Error]m_pIspDrvShell->m_pMdpMgr = NULL");
    }
    DIP_FUNC_DBG("-");
    return err;
}
int DIP_MDP_PIPE::configMDP( DipWorkingBuffer* pDipWBuf )
{
    MINT32 ret = MTRUE;
    MBOOL enPureMdpOut = MFALSE;
    //MUINT32 based_dupCqidx=0;
    MUINT32 ref_cqidx=0;
    MUINT32 ref_dupCqidx=0;
    MUINT32 ref_burstCqidx=0;
    MUINT32 frameflag=0;
    //unsigned int* originalTdriAddr=NULL;
    MUINT32 cmprule=0x0;
    MBOOL same=MFALSE;
    MDPMGR_CFG_STRUCT *pMdpCfg = pDipWBuf->m_pMdpCfg;
    //
    DIP_FUNC_DBG("[DIP_MDP_PIPE] dipCtlDmaEn(0x%x), pMdpCfg(%p)",\
        this->dipCtlDmaEn, pMdpCfg);

    if (this->isDipOnly == MTRUE){
        pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx = (E_ISP_DIP_CQ)this->dipTh;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx = this->burstQueIdx;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx = this->RingBufIdx;
        pMdpCfg->drvScenario = this->drvScenario;  //kk test
        pMdpCfg->isWaitBuf = this->isWaitBuf;
        pMdpCfg->imgxoEnP2 = this->mdp_imgxo_p2_en;
        pMdpCfg->pDumpTuningData = (unsigned int *)this->pTuningIspReg; //QQ
        pMdpCfg->ispDmaEn = this->dip_dma_en;
        pMdpCfg->ispDma2En = this->dip_dma2_en;
        pMdpCfg->m_iDipMetEn = this->m_pIspDrvShell->m_iDipMetEn;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = this->framemode_total_in_w;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_out_w = this->framemode_total_out_w;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = this->framemode_h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = this->total_data_size;

        // initialize
        for(MINT32 i = 0; i < ISP_OUTPORT_NUM; i++){
            pMdpCfg->dstPortCfg[i] = 0;
        }
        return ret;
    }

//Ring Buffer
    // TODO: hungwen modify
    //pthread_mutex_lock(&mMdpCfgDataMutex);
    pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x0;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx = (E_ISP_DIP_CQ)this->dipTh;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx = this->burstQueIdx;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx = 0;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx = this->RingBufIdx;
    pMdpCfg->drvScenario = this->drvScenario;  //kk test
    pMdpCfg->isWaitBuf = this->isWaitBuf;
    pMdpCfg->imgxoEnP2 = this->mdp_imgxo_p2_en;
    pMdpCfg->pDumpTuningData = (unsigned int *)this->pTuningIspReg; //QQ
    pMdpCfg->ispDmaEn = this->dip_dma_en;
    pMdpCfg->ispDma2En = this->dip_dma2_en;
    pMdpCfg->m_iDipMetEn = this->m_pIspDrvShell->m_iDipMetEn;
    //pmqos (todo)
    pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = 0;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_out_w = 0;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = 0;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = 0;

    // initialize
    for(MINT32 i = 0; i < ISP_OUTPORT_NUM; i++){
        pMdpCfg->dstPortCfg[i] = 0;
    }

    // collect src image info for MdpMgr
    pMdpCfg->mdpSrcFmt    = this->p2MdpSrcFmt;
    pMdpCfg->mdpSrcW      = this->p2MdpSrcW;
    pMdpCfg->mdpSrcH      = this->p2MdpSrcH;
    pMdpCfg->mdpSrcYStride = this->p2MdpSrcYStride;
    pMdpCfg->mdpSrcUVStride = this->p2MdpSrcUVStride;
    pMdpCfg->mdpSrcBufSize = this->p2MdpSrcSize;
    pMdpCfg->mdpSrcCBufSize = this->p2MdpSrcCSize;
    pMdpCfg->mdpSrcVBufSize = this->p2MdpSrcVSize;
    pMdpCfg->mdpSrcPlaneNum = this->p2MdpSrcPlaneNum;
    pMdpCfg->srcVirAddr  = 0xffff/*(this->dma_cfg.memBuf.base_vAddr + this->dma_cfg.memBuf.ofst_addr)*/; //kk test
    pMdpCfg->srcPhyAddr  = (this->imgi_dma_cfg.memBuf.base_pAddr + this->imgi_dma_cfg.memBuf.ofst_addr);
	pMdpCfg->isSecureFra = this->isSecureFra;
    pMdpCfg->srcSecureTag = this->imgi_dma_cfg.secureTag;

    DIP_FUNC_DBG("[imgi]p2cq(%d),cq(%d),RingBufIdx(%d),burstQueIdx(%d)",this->dipTh,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx);

    DIP_FUNC_DBG("[mdp]Fmt(0x%x),W(%d),H(%d),stride(%d,%d),VA(0x%8x),PA(0x%8x),Size(0x%8x)",pMdpCfg->mdpSrcFmt, \
                pMdpCfg->mdpSrcW, pMdpCfg->mdpSrcH, \
                pMdpCfg->mdpSrcYStride, pMdpCfg->mdpSrcYStride, \
                pMdpCfg->srcVirAddr, \
                pMdpCfg->srcPhyAddr, pMdpCfg->mdpSrcBufSize);

    // WROTO
    if(DIPCTL_DMA_EN1_WROTO & this->dipCtlDmaEn)
    {
        pMdpCfg->dstPortCfg[ISP_MDP_DL_WROTO] = 1;
        memcpy(&pMdpCfg->dstDma[ISP_MDP_DL_WROTO], &this->wroto_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // WDMAO
    if(DIPCTL_DMA_EN1_WDMAO & this->dipCtlDmaEn)
    {
        pMdpCfg->dstPortCfg[ISP_MDP_DL_WDMAO] = 1;
        memcpy(&pMdpCfg->dstDma[ISP_MDP_DL_WDMAO], &this->wdmao_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    //VENCO
    if(DIPCTL_DMA_EN1_VENCO & this->dipCtlDmaEn)
    {
        pMdpCfg->dstPortCfg[ISP_MDP_DL_VENCO] = 1;
        memcpy(&pMdpCfg->dstDma[ISP_MDP_DL_VENCO], &this->venco_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // ISP IMGXO
    if(enPureMdpOut == MFALSE){
        DIP_FUNC_DBG("imgxo va(0x%lx),pa(0x%lx),size(%d)",(unsigned long)this->imgxo_out.memBuf.base_vAddr,(unsigned long)this->imgxo_out.memBuf.base_pAddr,this->imgxo_out.memBuf.size);
        pMdpCfg->dstPortCfg[ISP_ONLY_OUT_TPIPE] = 1;
        memcpy(&pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE], &this->imgxo_out, sizeof(MdpRotDMACfg));
    }
    // set iso value only for MDP PQ function
    pMdpCfg->mdpWDMAPQParam  = this->WDMAPQParam;
    pMdpCfg->mdpWROTPQParam   = this->WROTPQParam;

#if 0
    originalTdriAddr = pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa;

    //save for the last frame in enque package
    if(this->lastframe)
    {
        //this->lastframe_dipTh = this->dipTh;
        //this->lastframe_dupCqIdx = this->dupCqIdx;
        //this->lastframe_burstQueIdx = this->burstQueIdx;
        //::memcpy(&m_LastTpipeStructure, &(pMdpCfg->ispTpipeCfgInfo), \
        //    sizeof(ISP_TPIPE_CONFIG_STRUCT) - sizeof(ISP_DRIVER_CONFIG_STRUCT));
        //m_LastTpipeStructure.drvinfo.frameflag = frameflag; //mark to make sure we keep the last frame during run-time
        m_LastVencPortEn = pMdpCfg->dstPortCfg[ISP_MDP_DL_VENCO];
    }
#endif
    //pthread_mutex_unlock(&mMdpCfgDataMutex);

    DIP_FUNC_DBG("HM flag(0x%x, 0x%x_%d), thiscq(%d_%d_%d), refcq(%d_%d_%d), lst(%d)",frameflag, cmprule, same, this->dipTh, this->RingBufIdx,this->burstQueIdx,\
        ref_cqidx, ref_dupCqidx, ref_burstCqidx, this->lastframe);


    return ret;

}

/*/////////////////////////////////////////////////////////////////////////////
    DIP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
//
MINT32
DIP_BUF_CTRL::
dequeueMdpFrameEnd()
{
    if(MDPMGR_NO_ERROR != m_pIspDrvShell->m_pMdpMgr->dequeueFrameEnd((*m_pMdpMgrCfgData))){
        DIP_FUNC_ERR("dequeueMdpFrameEnd of DIP_BUF_CTRL is fail!!");
        return -1;
    }
    return 0;
}

EIspRetStatus
DIP_BUF_CTRL::
debugDump(DipWorkingBuffer* pDipWBuf, MUINT32 ispDmaEnBit, MUINT32 i, MUINT32 drvScenario){
    EIspRetStatus ret = eIspRetStatus_Success;
    // dump hardware debug information
    IspDumpDbgLogDipPackage p2DumpPackage;
    char* pWorkingBuffer =  NULL;
    dip_x_reg_t *pIspDipReg;
    DIP_FUNC_INF("start dump HW regs");
    MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192,1);
    ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
    // dump difference between tpipe and hardware registers
    pIspDipReg = (dip_x_reg_t*)this->m_pIspDrvShell->m_pDrvDipPhy->getCurHWRegValues();
    pWorkingBuffer = new char[tpipeWorkingSize];
    //

    //pthread_mutex_lock(&mMdpCfgDataMutex);
    if(pWorkingBuffer!=NULL) {
        if (pIspDipReg!=NULL) {
            tpipe_main_platform(&m_pMdpMgrCfgData->ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspDipReg, NULL);
        }
        delete[] pWorkingBuffer;
        pWorkingBuffer = NULL;
    } else {
        DIP_FUNC_ERR("[Error]pWorkingBuffer(0x%lx),pIspReg(0x%lx)",(unsigned long)pWorkingBuffer,(unsigned long)pIspDipReg);
    }
    // TODO: hungwen modify
    p2DumpPackage.IspDescriptVa = (unsigned int*)pDipWBuf->m_pIspDescript_vir;
    p2DumpPackage.IspVirRegAddrVa = pDipWBuf->m_pIspVirRegAddr_va;
    p2DumpPackage.tpipeTableVa = m_pMdpMgrCfgData->ispTpipeCfgInfo.drvinfo.tpipeTableVa;
    p2DumpPackage.tpipeTablePa = m_pMdpMgrCfgData->ispTpipeCfgInfo.drvinfo.tpipeTablePa;
    p2DumpPackage.pTuningQue= m_pMdpMgrCfgData->pDumpTuningData;
    //pthread_mutex_unlock(&mMdpCfgDataMutex);
    DIP_FUNC_INF("Dump HW, ispDmaEnBit=%d, i=%d, drvScenario=%d",ispDmaEnBit, i, drvScenario);
    this->m_pIspDrvShell->m_pDrvDipPhy->dumpDipHwReg(&p2DumpPackage,this->m_pIspDrvShell->m_dumpDipHwTpipeReg);
    return ret;
}


/*
description: move FILLED buffer from hw to sw list
called at passx_done
*/
EIspRetStatus
DIP_BUF_CTRL::
dequeHwBuf( MBOOL& isVencContained, DipWorkingBuffer* pDipWBuf)
{
    EIspRetStatus ret = eIspRetStatus_Success;
    MDPMGR_DEQUEUE_INDEX mdpQueueIdx;
    MBOOL isDequeMdp = MTRUE;
    MUINT32 mdpMgrCfgDataImgxoEnP2;
    MUINT32 ispDmaEn, ispDma2En, ispDmaEnAll;
    MUINT32 ispDmaEnBit;
    MDPMGR_CFG_STRUCT mdpMgrCfgStructData;

    //get property for dumpHwBuf

    if (pDipWBuf==NULL){
        DIP_FUNC_ERR("deque pDipWBuf is NULL!!");
        return eIspRetStatus_Failed;
    }
    m_pMdpMgrCfgData = pDipWBuf->m_pMdpCfg;
    mdpMgrCfgDataImgxoEnP2 = m_pMdpMgrCfgData->imgxoEnP2;
    ispDmaEn = (m_pMdpMgrCfgData->ispDmaEn & DEQUE_DMA_MASK);
    ispDma2En = (m_pMdpMgrCfgData->ispDma2En & DEQUE_DMA2_MASK);
    ispDmaEnAll = (ispDmaEn | ispDma2En); // YWtodo but img3o no overlap


    // MDP flow
    if ((ispDmaEn & DIPCTL_DMA_EN1_VENCO) == DIPCTL_DMA_EN1_VENCO){
        isVencContained = MTRUE;
    }
/*
#define DIPCTL_DMA_EN1_IMGI_D1                  (1UL<<0)
#define DIPCTL_DMA_EN1_IMGBI_D1                 (1UL<<1)
#define DIPCTL_DMA_EN1_IMGCI_D1                 (1UL<<2)
#define DIPCTL_DMA_EN1_CPI_D1                   (1UL<<3)
#define DIPCTL_DMA_EN1_CPBI_D1                  (1UL<<4)
#define DIPCTL_DMA_EN1_CRZO_D1                  (1UL<<5)
#define DIPCTL_DMA_EN1_CRZBO_D1                 (1UL<<6)
#define DIPCTL_DMA_EN1_SMTI_D1                  (1UL<<7)
#define DIPCTL_DMA_EN1_SMTI_D2                  (1UL<<8)
#define DIPCTL_DMA_EN1_SMTI_D3                  (1UL<<9)
#define DIPCTL_DMA_EN1_SMTI_D4                  (1UL<<10)
#define DIPCTL_DMA_EN1_SMTO_D1                  (1UL<<11)
#define DIPCTL_DMA_EN1_SMTO_D2                  (1UL<<12)
#define DIPCTL_DMA_EN1_SMTO_D3                  (1UL<<13)
#define DIPCTL_DMA_EN1_SMTO_D4                  (1UL<<14)
#define DIPCTL_DMA_EN1_TIMGO_D1                 (1UL<<15)
#define DIPCTL_DMA_EN1_UFDI_D1                  (1UL<<16)
#define DIPCTL_DMA_EN1_DMGI_D1                  (1UL<<17)
#define DIPCTL_DMA_EN1_DEPI_D1                  (1UL<<18)
#define DIPCTL_DMA_EN1_LCEI_D1                  (1UL<<19)
#define DIPCTL_DMA_EN1_DCESO_D1                 (1UL<<20)
#define DIPCTL_DMA_EN1_FEO_D1                   (1UL<<21)
#define DIPCTL_DMA_EN1_ADL_D1                   (1UL<<22)

#define DIPCTL_DMA_EN1_WROTO                    (1UL<<28)
#define DIPCTL_DMA_EN1_WDMAO                    (1UL<<29)
#define DIPCTL_DMA_EN1_JPEGO                    (1UL<<30)
#define DIPCTL_DMA_EN1_VENCO                    (1UL<<31)

#define DIPCTL_DMA_EN2_VIPI_D1                  (1UL<<0)
#define DIPCTL_DMA_EN2_VIPBI_D1                 (1UL<<1)
#define DIPCTL_DMA_EN2_VIPCI_D1                 (1UL<<2)
#define DIPCTL_DMA_EN2_IMG3O_D1                 (1UL<<3)
#define DIPCTL_DMA_EN2_IMG3BO_D1                (1UL<<4)
#define DIPCTL_DMA_EN2_IMG3CO_D1                (1UL<<5)
#define DIPCTL_DMA_EN2_CPI_D2                   (1UL<<6)
#define DIPCTL_DMA_EN2_CPBI_D2                  (1UL<<7)
#define DIPCTL_DMA_EN2_CPO_D1                   (1UL<<8)
#define DIPCTL_DMA_EN2_CPBO_D1                  (1UL<<9)
#define DIPCTL_DMA_EN2_OFT_D1                   (1UL<<10)
#define DIPCTL_DMA_EN2_OFTL_D1                  (1UL<<11)

*/

    // MFB DL Case don't have any DMAI, so add IMGI for dequeSrc
    if (pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.top.mfb_en == 1)
    {
        ispDmaEnAll |= DIPCTL_DMA_EN1_IMGI_D1;
    }

    for(int i = 31 ; i >= 0 ; i--) //DIPCTL_DMA_EN1_VENCO
    {
        isDequeMdp = MTRUE;
        ispDmaEnBit = ispDmaEnAll & (1 << i);
        if(ispDmaEnBit)
        {
            //
            switch(ispDmaEnBit){
                case DIPCTL_DMA_EN1_IMGI_D1:
                    mdpQueueIdx = MDPMGR_DEQ_SRC;
                    break;
                case DIPCTL_DMA_EN1_WROTO:
                    mdpQueueIdx = MDPMGR_DEQ_WROT;
                    break;
                case DIPCTL_DMA_EN1_WDMAO:
                    mdpQueueIdx = MDPMGR_DEQ_WDMA;
                    break;
                case DIPCTL_DMA_EN1_VENCO:
                    mdpQueueIdx = MDPMGR_DEQ_VENCO;
                    break;
                case DIPCTL_DMA_EN1_CRZO_D1: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIPCTL_DMA_EN1_CRZO_D1){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIPCTL_DMA_EN2_IMG3O_D1: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIPCTL_DMA_EN2_IMG3O_D1){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIPCTL_DMA_EN1_FEO_D1: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIPCTL_DMA_EN1_FEO_D1){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIPCTL_DMA_EN1_TIMGO_D1: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIPCTL_DMA_EN1_TIMGO_D1){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIPCTL_DMA_EN1_JPEGO:
                default:
                    isDequeMdp = MFALSE;
                    break;
            };

            if (this->m_pIspDrvShell->m_dumpDipHwRegEn || (this->m_pIspDrvShell->m_dumpDipHwRegBySceID == m_pMdpMgrCfgData->drvScenario))
            {
                debugDump(pDipWBuf,ispDmaEnBit,i,m_pMdpMgrCfgData->drvScenario);
            }
            
            if(isDequeMdp==MTRUE){
                MDPMGR_RETURN_TYPE mdpRet;
                mdpRet = (MDPMGR_RETURN_TYPE)m_pIspDrvShell->m_pMdpMgr->dequeueBuf(mdpQueueIdx, (*m_pMdpMgrCfgData));
                
                DIP_FUNC_DBG("mdpRet(%d)",mdpRet);
                if(mdpRet == MDPMGR_NO_ERROR){
                    ret = eIspRetStatus_Success;
                } else if(mdpRet == MDPMGR_VSS_NOT_READY){
                    ret = eIspRetStatus_VSS_NotReady;
                    break;
                } else {
                    debugDump(pDipWBuf,ispDmaEnBit,i,m_pMdpMgrCfgData->drvScenario);
                    ret = eIspRetStatus_Failed;

                    DIP_FUNC_ERR("[Error]dequeueBuf fail mdpRet(%d)",mdpRet);
                    break;
                    //goto EXIT;
                }
            }
        }
    }

    //
    return ret;
}


