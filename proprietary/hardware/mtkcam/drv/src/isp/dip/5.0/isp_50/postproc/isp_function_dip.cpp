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
#define MAX_TPIPE_HEIGHT                 (8190)

// tpipe irq mode
#define TPIPE_IRQ_FRAME     (0)
#define TPIPE_IRQ_LINE      (1)
#define TPIPE_IRQ_TPIPE     (2)

///////////////////////////////////////
//temp for EP
#define EP_ISP_ONLY_FRAME_MODE 0
//////////////////////////////////////

#define CHECK_CTL_RAW_RGB_EN_BITS      32
#define CHECK_CTL_RAW_RGB2_EN_BITS      3

#define CHECK_CTL_YUV_EN_BITS          31
#define CHECK_CTL_YUV2_EN_BITS          9


const IspTopModuleMappingTable gIspModuleMappingTableRbgEn[CHECK_CTL_RAW_RGB_EN_BITS]
 ={ {DIP_X_REG_CTL_RGB_EN_UNP ,        DIP_A_UNP,    IspDrv_B::setUnp},
    {DIP_X_REG_CTL_RGB_EN_UFD ,        DIP_A_UFD,    IspDrv_B::setUfd},
    {DIP_X_REG_CTL_RGB_EN_PGN ,        DIP_A_PGN,    IspDrv_B::setPgn},
    {DIP_X_REG_CTL_RGB_EN_SL2 ,        DIP_A_SL2,    IspDrv_B::setSl2},
    {DIP_X_REG_CTL_RGB_EN_UDM ,        DIP_A_UDM,    IspDrv_B::setUdm},
    {DIP_X_REG_CTL_RGB_EN_LCE ,        DIP_A_LCE,    IspDrv_B::setLce},
    {DIP_X_REG_CTL_RGB_EN_G2G ,        DIP_A_G2G,    IspDrv_B::setG2g},
    {DIP_X_REG_CTL_RGB_EN_GGM ,        DIP_A_GGM,    IspDrv_B::setGgm},
    {DIP_X_REG_CTL_RGB_EN_GDR1,        DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_GDR2,        DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_DBS2,        DIP_A_DBS2,   IspDrv_B::setDbs2},
    {DIP_X_REG_CTL_RGB_EN_OBC2,        DIP_A_OBC2,   IspDrv_B::setOb2},
    {DIP_X_REG_CTL_RGB_EN_RMG2,        DIP_A_RMG2,   IspDrv_B::setRmg2},
    {DIP_X_REG_CTL_RGB_EN_BNR2,        DIP_A_BNR2,   IspDrv_B::setBnr2},
    {DIP_X_REG_CTL_RGB_EN_RMM2,        DIP_A_RMM2,   IspDrv_B::setRmm2},
    {DIP_X_REG_CTL_RGB_EN_LSC2,        DIP_A_LSC2,   IspDrv_B::setLsc2},
    {DIP_X_REG_CTL_RGB_EN_RCP2,        DIP_A_RCP2,   IspDrv_B::setRcp2},
    {DIP_X_REG_CTL_RGB_EN_PAK2,        DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_RNR ,        DIP_A_RNR,    IspDrv_B::setRnr},
    {DIP_X_REG_CTL_RGB_EN_SL2G,        DIP_A_SL2G,   IspDrv_B::setSl2g},
    {DIP_X_REG_CTL_RGB_EN_SL2H,        DIP_A_SL2H,   IspDrv_B::setSl2h},
    {DIP_X_REG_CTL_RGB_EN_BIT21,       DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_SL2K,        DIP_A_SL2K,   IspDrv_B::setSl2k},
    {DIP_X_REG_CTL_RGB_EN_FLC,         DIP_A_FLC,    IspDrv_B::setFlc},
    {DIP_X_REG_CTL_RGB_EN_FLC2,        DIP_A_FLC2,   IspDrv_B::setFlc2},
    {DIP_X_REG_CTL_RGB_EN_SMX1,        DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_PAKG2,       DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_G2G2,        DIP_A_G2G2,   IspDrv_B::setG2g2},
    {DIP_X_REG_CTL_RGB_EN_GGM2,        DIP_A_GGM2,   IspDrv_B::setGgm2},
    {DIP_X_REG_CTL_RGB_EN_WSHIFT,      DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_WSYNC,       DIP_A_DUMMY,  NULL},
    {DIP_X_REG_CTL_RGB_EN_MDPCROP2,    DIP_A_DUMMY,  NULL}};

const IspTopModuleMappingTable gIspModuleMappingTableRbg2En[CHECK_CTL_RAW_RGB2_EN_BITS]
 ={ {DIP_X_REG_CTL_RGB2_EN_ADBS2 ,     DIP_A_ADBS2,  IspDrv_B::setAdbs2},
    {DIP_X_REG_CTL_RGB2_EN_DCPN2 ,     DIP_A_DCPN2,  IspDrv_B::setDcpn2},
    {DIP_X_REG_CTL_RGB2_EN_CPN2 ,      DIP_A_CPN2,   IspDrv_B::setCpn2}};


const IspTopModuleMappingTable gIspModuleMappingTableYuvEn[CHECK_CTL_YUV_EN_BITS]
 ={ {DIP_X_REG_CTL_YUV_EN_MFB ,        DIP_A_MFB,   IspDrv_B::setMfb},
    {DIP_X_REG_CTL_YUV_EN_C02B ,       DIP_A_C02B,  IspDrv_B::setC02b},
    {DIP_X_REG_CTL_YUV_EN_C24 ,        DIP_A_C24,   IspDrv_B::setC24},
    {DIP_X_REG_CTL_YUV_EN_G2C ,        DIP_A_G2C,   IspDrv_B::setG2c},
    {DIP_X_REG_CTL_YUV_EN_C42 ,        DIP_A_C42,   IspDrv_B::setC42},
    {DIP_X_REG_CTL_YUV_EN_NBC ,        DIP_A_ANR,   IspDrv_B::setNbc},
    {DIP_X_REG_CTL_YUV_EN_NBC2 ,       DIP_A_ANR2,  IspDrv_B::setNbc2},
    {DIP_X_REG_CTL_YUV_EN_PCA ,        DIP_A_PCA,   IspDrv_B::setPca},
    {DIP_X_REG_CTL_YUV_EN_SEEE ,       DIP_A_SEEE,  IspDrv_B::setSeee},
    {DIP_X_REG_CTL_YUV_EN_COLOR ,      DIP_A_COLOR, IspDrv_B::setColor},
    {DIP_X_REG_CTL_YUV_EN_NR3D ,       DIP_A_NR3D,  IspDrv_B::setNr3d},
    {DIP_X_REG_CTL_YUV_EN_SL2B ,       DIP_A_SL2B,  IspDrv_B::setSl2b},
    {DIP_X_REG_CTL_YUV_EN_SL2C ,       DIP_A_SL2C,  IspDrv_B::setSl2c},
    {DIP_X_REG_CTL_YUV_EN_SL2D ,       DIP_A_SL2D,  IspDrv_B::setSl2d},
    {DIP_X_REG_CTL_YUV_EN_SL2E ,       DIP_A_SL2E,  IspDrv_B::setSl2e},
    {DIP_X_REG_CTL_YUV_EN_SRZ1 ,       DIP_A_SRZ1,  IspDrv_B::setSrz1},
    {DIP_X_REG_CTL_YUV_EN_SRZ2 ,       DIP_A_SRZ2,  IspDrv_B::setSrz2},
    {DIP_X_REG_CTL_YUV_EN_CRZ ,        DIP_A_CRZ,   NULL},
    {DIP_X_REG_CTL_YUV_EN_MIX1 ,       DIP_A_MIX1,  IspDrv_B::setMix1},
    {DIP_X_REG_CTL_YUV_EN_MIX2 ,       DIP_A_MIX2,  IspDrv_B::setMix2},
    {DIP_X_REG_CTL_YUV_EN_MIX3 ,       DIP_A_MIX3,  IspDrv_B::setMix3},
    {DIP_X_REG_CTL_YUV_EN_CRSP ,       DIP_A_CRSP,  IspDrv_B::setCrsp},
    {DIP_X_REG_CTL_YUV_EN_C24B,        DIP_A_C24B,  IspDrv_B::setC24b},
    {DIP_X_REG_CTL_YUV_EN_MDPCROP,     DIP_A_DUMMY, NULL},
    {DIP_X_REG_CTL_YUV_EN_C02,         DIP_A_C02,   IspDrv_B::setC02},
    {DIP_X_REG_CTL_YUV_EN_FE,          DIP_A_FE,    IspDrv_B::setFe},
    {DIP_X_REG_CTL_YUV_EN_MFBW,        DIP_A_DUMMY, IspDrv_B::setMfbw},
    {DIP_X_REG_CTL_YUV_EN_PLNW1,       DIP_A_DUMMY, NULL},
    {DIP_X_REG_CTL_YUV_EN_PLNR1,       DIP_A_DUMMY, NULL},
    {DIP_X_REG_CTL_YUV_EN_PLNW2,       DIP_A_DUMMY, NULL},
    {DIP_X_REG_CTL_YUV_EN_PLNR2,       DIP_A_DUMMY, NULL}};

const IspTopModuleMappingTable gIspModuleMappingTableYuv2En[CHECK_CTL_YUV2_EN_BITS]
 ={ {DIP_X_REG_CTL_YUV2_EN_FM ,       DIP_A_FM,    IspDrv_B::setFm},
    {DIP_X_REG_CTL_YUV2_EN_SRZ3 ,     DIP_A_SRZ3,  IspDrv_B::setSrz3},
    {DIP_X_REG_CTL_YUV2_EN_SRZ4 ,     DIP_A_SRZ4,  IspDrv_B::setSrz4},
    {DIP_X_REG_CTL_YUV2_EN_SRZ5 ,     DIP_A_SRZ5,  IspDrv_B::setSrz5},
    {DIP_X_REG_CTL_YUV2_EN_HFG  ,     DIP_A_HFG,   IspDrv_B::setHfg},
    {DIP_X_REG_CTL_YUV2_EN_SL2I ,     DIP_A_SL2I,  IspDrv_B::setSl2i},
    {DIP_X_REG_CTL_YUV2_EN_NDG ,      DIP_A_NDG,   IspDrv_B::setNdg},
    {DIP_X_REG_CTL_YUV2_EN_NDG2 ,     DIP_A_NDG2,  IspDrv_B::setNdg2},
    {DIP_X_REG_CTL_YUV2_EN_MIX4 ,     DIP_A_MIX4,  IspDrv_B::setMix4}};


MINT32
IspDrv_B::
setUnp(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UNP_OFST, 0x00);

    return 0;
}

MINT32
IspDrv_B::
setUfd(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;

    MUINT32 fgModeRegBit;
    fgModeRegBit = (imageioPackage.isp_top_ctl.FMT_SEL.Bits.FG_MODE&0x01);

    if (fgModeRegBit == 1)
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_UFDG_CON,UFOD_SEL,0x0);
    }
    else
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_UFDG_CON,UFOD_SEL,0x1);
    }

    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_UFDG_CON,UFDG_TCCT_BYP,0x1); // need always set to 1

    if (imageioPackage.pUfdParam != NULL)
    {
        DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_UFDG_CON,UFDG_BOND_MODE,imageioPackage.pUfdParam->UFDG_BOND_MODE);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_UFDG_AU2_CON,UFDG_AU2_SIZE,imageioPackage.pUfdParam->UFDG_AU2_SIZE);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UFDG_ADDRESS_CON, imageioPackage.pUfdParam->UFDG_BITSTREAM_OFST_ADDR);
        DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_UFDG_BS_AU_CON,UFDG_BS_AU_START,imageioPackage.pUfdParam->UFDG_BS_AU_START);
    }
    else
    {
        DIP_FUNC_INF("imageioPackage.pUfdParam is NULL!");
    }

    return 0;
}

MINT32
IspDrv_B::
setRcp2(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setSMX1(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;
    return 0;
}

MINT32
IspDrv_B::
setPAKG2(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;
    return 0;
}


MINT32
IspDrv_B::
setDbs2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DBS2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_DBS2_SIGMA,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }
    return 0;
}

MINT32
IspDrv_B::
setOb2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_OBC2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_OBC2_OFFST0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setBnr2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE)
	{
        MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_BNR2,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_BNR2_BPC_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
	return 0;
}

MINT32
IspDrv_B::
setRmg2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_RMG2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_RMG2_HDR_CFG,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setRmm2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_RMM2,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_RMM2_OSC,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}

    return 0;
}

MINT32
IspDrv_B::
setLsc2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
	    MUINT32 addr,size;
    	imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LSC2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_LSC2_CTL1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setAdbs2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
	    MUINT32 addr,size;
    	imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_ADBS2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_ADBS2_CTL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setDcpn2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
	    MUINT32 addr,size;
    	imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_DCPN2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_DCPN_HDR_EN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setCpn2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
	    MUINT32 addr,size;
    	imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_CPN2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_CPN_HDR_CTL_EN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}


MINT32
IspDrv_B::
setPgn(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_PGN,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_PGN_SATU_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setUdm(DIP_ISP_PIPE &imageioPackage)
{
    MUINT32 fgModeRegBit;
    fgModeRegBit = (imageioPackage.isp_top_ctl.FMT_SEL.Bits.FG_MODE&0x01)<<10;

    if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_UDM,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_UDM_INTP_CRS,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }
    else {
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INTP_CRS, 0x0002F004);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INTP_NAT, 0x1430053F);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INTP_AUG, 0x00500500);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_LUMA_LUT1, 0x052A30DC);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_LUMA_LUT2, 0x02A9124F);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_SL_CTL, 0x0039B4A0);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HFTD_CTL,  0x0A529400);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_NR_STR, 0x81028000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_NR_ACT,  0x00000050);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HF_STR, 0x84210000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HF_ACT1,  0x46FF1EFF);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HF_ACT2, 0x001EFF55);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_CLIP,  0x00942064);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_DSB,  0x00000000|fgModeRegBit);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_TILE_EDGE,  0x0000000F);
            //new add
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_P1_ACT,  0x000000FF);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_LR_RAT,  0x00000418);
            //new add
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HFTD_CTL2,  0x00000019);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_EST_CTL,  0x00000035);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_SPARE_2,  0x00000000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_SPARE_3,  0x00000000);
            //new add
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INT_CTL,  0x00000035);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_EE,  0x00000410);
    }

    DIP_FUNC_DBG("fdmode bit/reg/udm_dsb(0x%x/0x%x/0x%x)",imageioPackage.isp_top_ctl.FMT_SEL.Bits.FG_MODE, fgModeRegBit, 0x007FA800|fgModeRegBit);

    return 0;
}

MINT32
IspDrv_B::
setG2g(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){

        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_G2G,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_G2G_CNV_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CNV_1, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CNV_2, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CNV_3, 0x02000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CNV_4, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CNV_5, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CNV_6, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G_CTRL,  0x00000009);
    }
    return 0;
}

MINT32
IspDrv_B::
setG2g2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){

        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_G2G2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_1, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_2, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_3, 0x02000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_4, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_5, 0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CNV_6, 0x00000200);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2G2_CTRL,  0x00000009);
    }
    return 0;
}


MINT32
IspDrv_B::
setLce(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LCE,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_LCE25_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}

	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_LCE25_IMAGE_SIZE, LCE_IMAGE_WD, imageioPackage.DMAImgi.dma_cfg.size.w);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_LCE25_IMAGE_SIZE, LCE_IMAGE_HT, imageioPackage.DMAImgi.dma_cfg.size.h);

    return 0;
}

MINT32
IspDrv_B::
setGgm(DIP_ISP_PIPE &imageioPackage)
{

	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_GGM,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_GGM_LUT,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    else
    {
        imageioPackage.pDrvDip->writeReg( 0x00002180, 0x00000000); /*0x00002180  DIP_A_GGM_LUT[0] */
        imageioPackage.pDrvDip->writeReg( 0x00002184, 0x00200802); /*0x00002184  DIP_A_GGM_LUT[1] */
        imageioPackage.pDrvDip->writeReg( 0x00002188, 0x00401004); /*0x00002188  DIP_A_GGM_LUT[2] */
        imageioPackage.pDrvDip->writeReg( 0x0000218C, 0x00601806); /*0x0000218C  DIP_A_GGM_LUT[3] */
        imageioPackage.pDrvDip->writeReg( 0x00002190, 0x00802008); /*0x00002190  DIP_A_GGM_LUT[4] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002194, 0x00a0280a); /*0x00002194  DIP_A_GGM_LUT[5] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002198, 0x00c0300c); /*0x00002198  DIP_A_GGM_LUT[6] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000219C, 0x00e0380e); /*0x0000219C  DIP_A_GGM_LUT[7] */ 
        imageioPackage.pDrvDip->writeReg( 0x000021A0, 0x01004010); /*0x000021A0  DIP_A_GGM_LUT[8] */  
        imageioPackage.pDrvDip->writeReg( 0x000021A4, 0x01204812); /*0x000021A4  DIP_A_GGM_LUT[9] */  
        imageioPackage.pDrvDip->writeReg( 0x000021A8, 0x01405014); /*0x000021A8  DIP_A_GGM_LUT[10] */  
        imageioPackage.pDrvDip->writeReg( 0x000021AC, 0x01605816); /*0x000021AC  DIP_A_GGM_LUT[11] */  
        imageioPackage.pDrvDip->writeReg( 0x000021B0, 0x01806018); /*0x000021B0  DIP_A_GGM_LUT[12] */   
        imageioPackage.pDrvDip->writeReg( 0x000021B4, 0x01a0681a); /*0x000021B4  DIP_A_GGM_LUT[13] */   
        imageioPackage.pDrvDip->writeReg( 0x000021B8, 0x01c0701c); /*0x000021B8  DIP_A_GGM_LUT[14] */   
        imageioPackage.pDrvDip->writeReg( 0x000021BC, 0x01e0781e); /*0x000021BC  DIP_A_GGM_LUT[15] */   
        imageioPackage.pDrvDip->writeReg( 0x000021C0, 0x02008020); /*0x000021C0  DIP_A_GGM_LUT[16] */  
        imageioPackage.pDrvDip->writeReg( 0x000021C4, 0x02208822); /*0x000021C4  DIP_A_GGM_LUT[17] */  
        imageioPackage.pDrvDip->writeReg( 0x000021C8, 0x02409024); /*0x000021C8  DIP_A_GGM_LUT[18] */  
        imageioPackage.pDrvDip->writeReg( 0x000021CC, 0x02609826); /*0x000021CC  DIP_A_GGM_LUT[19] */  
        imageioPackage.pDrvDip->writeReg( 0x000021D0, 0x0280a028); /*0x000021D0  DIP_A_GGM_LUT[20] */   
        imageioPackage.pDrvDip->writeReg( 0x000021D4, 0x02a0a82a); /*0x000021D4  DIP_A_GGM_LUT[21] */   
        imageioPackage.pDrvDip->writeReg( 0x000021D8, 0x02c0b02c); /*0x000021D8  DIP_A_GGM_LUT[22] */   
        imageioPackage.pDrvDip->writeReg( 0x000021DC, 0x02e0b82e); /*0x000021DC  DIP_A_GGM_LUT[23] */   
        imageioPackage.pDrvDip->writeReg( 0x000021E0, 0x0300c030); /*0x000021E0  DIP_A_GGM_LUT[24] */    
        imageioPackage.pDrvDip->writeReg( 0x000021E4, 0x0320c832); /*0x000021E4  DIP_A_GGM_LUT[25] */    
        imageioPackage.pDrvDip->writeReg( 0x000021E8, 0x0340d034); /*0x000021E8  DIP_A_GGM_LUT[26] */    
        imageioPackage.pDrvDip->writeReg( 0x000021EC, 0x0360d836); /*0x000021EC  DIP_A_GGM_LUT[27] */    
        imageioPackage.pDrvDip->writeReg( 0x000021F0, 0x0380e038); /*0x000021F0  DIP_A_GGM_LUT[28] */     
        imageioPackage.pDrvDip->writeReg( 0x000021F4, 0x03a0e83a); /*0x000021F4  DIP_A_GGM_LUT[29] */     
        imageioPackage.pDrvDip->writeReg( 0x000021F8, 0x03c0f03c); /*0x000021F8  DIP_A_GGM_LUT[30] */     
        imageioPackage.pDrvDip->writeReg( 0x000021FC, 0x03e0f83e); /*0x000021FC  DIP_A_GGM_LUT[31] */     
        imageioPackage.pDrvDip->writeReg( 0x00002200, 0x04010040); /*0x00002200  DIP_A_GGM_LUT[32] */  
        imageioPackage.pDrvDip->writeReg( 0x00002204, 0x04210842); /*0x00002204  DIP_A_GGM_LUT[33] */  
        imageioPackage.pDrvDip->writeReg( 0x00002208, 0x04411044); /*0x00002208  DIP_A_GGM_LUT[34] */  
        imageioPackage.pDrvDip->writeReg( 0x0000220C, 0x04611846); /*0x0000220C  DIP_A_GGM_LUT[35] */  
        imageioPackage.pDrvDip->writeReg( 0x00002210, 0x04812048); /*0x00002210  DIP_A_GGM_LUT[36] */  
        imageioPackage.pDrvDip->writeReg( 0x00002214, 0x04a1284a); /*0x00002214  DIP_A_GGM_LUT[37] */  
        imageioPackage.pDrvDip->writeReg( 0x00002218, 0x04c1304c); /*0x00002218  DIP_A_GGM_LUT[38] */  
        imageioPackage.pDrvDip->writeReg( 0x0000221C, 0x04e1384e); /*0x0000221C  DIP_A_GGM_LUT[39] */  
        imageioPackage.pDrvDip->writeReg( 0x00002220, 0x05014050); /*0x00002220  DIP_A_GGM_LUT[40] */   
        imageioPackage.pDrvDip->writeReg( 0x00002224, 0x05214852); /*0x00002224  DIP_A_GGM_LUT[41] */   
        imageioPackage.pDrvDip->writeReg( 0x00002228, 0x05415054); /*0x00002228  DIP_A_GGM_LUT[42] */   
        imageioPackage.pDrvDip->writeReg( 0x0000222C, 0x05615856); /*0x0000222C  DIP_A_GGM_LUT[43] */   
        imageioPackage.pDrvDip->writeReg( 0x00002230, 0x05816058); /*0x00002230  DIP_A_GGM_LUT[44] */    
        imageioPackage.pDrvDip->writeReg( 0x00002234, 0x05a1685a); /*0x00002234  DIP_A_GGM_LUT[45] */    
        imageioPackage.pDrvDip->writeReg( 0x00002238, 0x05c1705c); /*0x00002238  DIP_A_GGM_LUT[46] */    
        imageioPackage.pDrvDip->writeReg( 0x0000223C, 0x05e1785e); /*0x0000223C  DIP_A_GGM_LUT[47] */    
        imageioPackage.pDrvDip->writeReg( 0x00002240, 0x06018060); /*0x00002240  DIP_A_GGM_LUT[48] */     
        imageioPackage.pDrvDip->writeReg( 0x00002244, 0x06218862); /*0x00002244  DIP_A_GGM_LUT[49] */     
        imageioPackage.pDrvDip->writeReg( 0x00002248, 0x06419064); /*0x00002248  DIP_A_GGM_LUT[50] */     
        imageioPackage.pDrvDip->writeReg( 0x0000224C, 0x06619866); /*0x0000224C  DIP_A_GGM_LUT[51] */     
        imageioPackage.pDrvDip->writeReg( 0x00002250, 0x0681a068); /*0x00002250  DIP_A_GGM_LUT[52] */      
        imageioPackage.pDrvDip->writeReg( 0x00002254, 0x06a1a86a); /*0x00002254  DIP_A_GGM_LUT[53] */      
        imageioPackage.pDrvDip->writeReg( 0x00002258, 0x06c1b06c); /*0x00002258  DIP_A_GGM_LUT[54] */      
        imageioPackage.pDrvDip->writeReg( 0x0000225C, 0x06e1b86e); /*0x0000225C  DIP_A_GGM_LUT[55] */      
        imageioPackage.pDrvDip->writeReg( 0x00002260, 0x0701c070); /*0x00002260  DIP_A_GGM_LUT[56] */  
        imageioPackage.pDrvDip->writeReg( 0x00002264, 0x0721c872); /*0x00002264  DIP_A_GGM_LUT[57] */  
        imageioPackage.pDrvDip->writeReg( 0x00002268, 0x0741d074); /*0x00002268  DIP_A_GGM_LUT[58] */  
        imageioPackage.pDrvDip->writeReg( 0x0000226C, 0x0761d876); /*0x0000226C  DIP_A_GGM_LUT[59] */  
        imageioPackage.pDrvDip->writeReg( 0x00002270, 0x0781e078); /*0x00002270  DIP_A_GGM_LUT[60] */  
        imageioPackage.pDrvDip->writeReg( 0x00002274, 0x07a1e87a); /*0x00002274  DIP_A_GGM_LUT[61] */  
        imageioPackage.pDrvDip->writeReg( 0x00002278, 0x07c1f07c); /*0x00002278  DIP_A_GGM_LUT[62] */  
        imageioPackage.pDrvDip->writeReg( 0x0000227C, 0x07e1f87e); /*0x0000227C  DIP_A_GGM_LUT[63] */  
        imageioPackage.pDrvDip->writeReg( 0x00002280, 0x08020080); /*0x00002280  DIP_A_GGM_LUT[64] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002284, 0x08421084); /*0x00002284  DIP_A_GGM_LUT[65] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002288, 0x08822088); /*0x00002288  DIP_A_GGM_LUT[66] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000228C, 0x08c2308c); /*0x0000228C  DIP_A_GGM_LUT[67] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002290, 0x09024090); /*0x00002290  DIP_A_GGM_LUT[68] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002294, 0x09425094); /*0x00002294  DIP_A_GGM_LUT[69] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002298, 0x09826098); /*0x00002298  DIP_A_GGM_LUT[70] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000229C, 0x09c2709c); /*0x0000229C  DIP_A_GGM_LUT[71] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022A0, 0x0a0280a0); /*0x000022A0  DIP_A_GGM_LUT[72] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022A4, 0x0a4290a4); /*0x000022A4  DIP_A_GGM_LUT[73] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022A8, 0x0a82a0a8); /*0x000022A8  DIP_A_GGM_LUT[74] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022AC, 0x0ac2b0ac); /*0x000022AC  DIP_A_GGM_LUT[75] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022B0, 0x0b02c0b0); /*0x000022B0  DIP_A_GGM_LUT[76] */  
        imageioPackage.pDrvDip->writeReg( 0x000022B4, 0x0b42d0b4); /*0x000022B4  DIP_A_GGM_LUT[77] */  
        imageioPackage.pDrvDip->writeReg( 0x000022B8, 0x0b82e0b8); /*0x000022B8  DIP_A_GGM_LUT[78] */  
        imageioPackage.pDrvDip->writeReg( 0x000022BC, 0x0bc2f0bc); /*0x000022BC  DIP_A_GGM_LUT[79] */  
        imageioPackage.pDrvDip->writeReg( 0x000022C0, 0x0c0300c0); /*0x000022C0  DIP_A_GGM_LUT[80] */   
        imageioPackage.pDrvDip->writeReg( 0x000022C4, 0x0c4310c4); /*0x000022C4  DIP_A_GGM_LUT[81] */   
        imageioPackage.pDrvDip->writeReg( 0x000022C8, 0x0c8320c8); /*0x000022C8  DIP_A_GGM_LUT[82] */   
        imageioPackage.pDrvDip->writeReg( 0x000022CC, 0x0cc330cc); /*0x000022CC  DIP_A_GGM_LUT[83] */   
        imageioPackage.pDrvDip->writeReg( 0x000022D0, 0x0d0340d0); /*0x000022D0  DIP_A_GGM_LUT[84] */   
        imageioPackage.pDrvDip->writeReg( 0x000022D4, 0x0d4350d4); /*0x000022D4  DIP_A_GGM_LUT[85] */   
        imageioPackage.pDrvDip->writeReg( 0x000022D8, 0x0d8360d8); /*0x000022D8  DIP_A_GGM_LUT[86] */   
        imageioPackage.pDrvDip->writeReg( 0x000022DC, 0x0dc370dc); /*0x000022DC  DIP_A_GGM_LUT[87] */   
        imageioPackage.pDrvDip->writeReg( 0x000022E0, 0x0e0380e0); /*0x000022E0  DIP_A_GGM_LUT[88] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022E4, 0x0e4390e4); /*0x000022E4  DIP_A_GGM_LUT[89] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022E8, 0x0e83a0e8); /*0x000022E8  DIP_A_GGM_LUT[90] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022EC, 0x0ec3b0ec); /*0x000022EC  DIP_A_GGM_LUT[91] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022F0, 0x0f03c0f0); /*0x000022F0  DIP_A_GGM_LUT[92] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022F4, 0x0f43d0f4); /*0x000022F4  DIP_A_GGM_LUT[93] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022F8, 0x0f83e0f8); /*0x000022F8  DIP_A_GGM_LUT[94] */ 
        imageioPackage.pDrvDip->writeReg( 0x000022FC, 0x0fc3f0fc); /*0x000022FC  DIP_A_GGM_LUT[95] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002300, 0x10040100); /*0x00002300  DIP_A_GGM_LUT[96] */  
        imageioPackage.pDrvDip->writeReg( 0x00002304, 0x10842108); /*0x00002304  DIP_A_GGM_LUT[97] */  
        imageioPackage.pDrvDip->writeReg( 0x00002308, 0x11044110); /*0x00002308  DIP_A_GGM_LUT[98] */  
        imageioPackage.pDrvDip->writeReg( 0x0000230C, 0x11846118); /*0x0000230C  DIP_A_GGM_LUT[99] */  
        imageioPackage.pDrvDip->writeReg( 0x00002310, 0x12048120); /*0x00002310  DIP_A_GGM_LUT[100] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002314, 0x1284a128); /*0x00002314  DIP_A_GGM_LUT[101] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002318, 0x1304c130); /*0x00002318  DIP_A_GGM_LUT[102] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000231C, 0x1384e138); /*0x0000231C  DIP_A_GGM_LUT[103] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002320, 0x14050140); /*0x00002320  DIP_A_GGM_LUT[104] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002324, 0x14852148); /*0x00002324  DIP_A_GGM_LUT[105] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002328, 0x15054150); /*0x00002328  DIP_A_GGM_LUT[106] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000232C, 0x15856158); /*0x0000232C  DIP_A_GGM_LUT[107] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002330, 0x16058160); /*0x00002330  DIP_A_GGM_LUT[108] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002334, 0x1685a168); /*0x00002334  DIP_A_GGM_LUT[109] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002338, 0x1705c170); /*0x00002338  DIP_A_GGM_LUT[110] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000233C, 0x1785e178); /*0x0000233C  DIP_A_GGM_LUT[111] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002340, 0x18060180); /*0x00002340  DIP_A_GGM_LUT[112] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002344, 0x18862188); /*0x00002344  DIP_A_GGM_LUT[113] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002348, 0x19064190); /*0x00002348  DIP_A_GGM_LUT[114] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000234C, 0x19866198); /*0x0000234C  DIP_A_GGM_LUT[115] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002350, 0x1a0681a0); /*0x00002350  DIP_A_GGM_LUT[116] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002354, 0x1a86a1a8); /*0x00002354  DIP_A_GGM_LUT[117] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002358, 0x1b06c1b0); /*0x00002358  DIP_A_GGM_LUT[118] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000235C, 0x1b86e1b8); /*0x0000235C  DIP_A_GGM_LUT[119] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002360, 0x1c0701c0); /*0x00002360  DIP_A_GGM_LUT[120] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002364, 0x1c8721c8); /*0x00002364  DIP_A_GGM_LUT[121] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002368, 0x1d0741d0); /*0x00002368  DIP_A_GGM_LUT[122] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000236C, 0x1d8761d8); /*0x0000236C  DIP_A_GGM_LUT[123] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002370, 0x1e0781e0); /*0x00002370  DIP_A_GGM_LUT[124] */  
        imageioPackage.pDrvDip->writeReg( 0x00002374, 0x1e87a1e8); /*0x00002374  DIP_A_GGM_LUT[125] */  
        imageioPackage.pDrvDip->writeReg( 0x00002378, 0x1f07c1f0); /*0x00002378  DIP_A_GGM_LUT[126] */  
        imageioPackage.pDrvDip->writeReg( 0x0000237C, 0x1f87e1f8); /*0x0000237C  DIP_A_GGM_LUT[127] */  
        imageioPackage.pDrvDip->writeReg( 0x00002380, 0x20080200); /*0x00002380  DIP_A_GGM_LUT[128] */  
        imageioPackage.pDrvDip->writeReg( 0x00002384, 0x20882208); /*0x00002384  DIP_A_GGM_LUT[129] */  
        imageioPackage.pDrvDip->writeReg( 0x00002388, 0x21084210); /*0x00002388  DIP_A_GGM_LUT[130] */  
        imageioPackage.pDrvDip->writeReg( 0x0000238C, 0x21886218); /*0x0000238C  DIP_A_GGM_LUT[131] */  
        imageioPackage.pDrvDip->writeReg( 0x00002390, 0x22088220); /*0x00002390  DIP_A_GGM_LUT[132] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002394, 0x2288a228); /*0x00002394  DIP_A_GGM_LUT[133] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002398, 0x2308c230); /*0x00002398  DIP_A_GGM_LUT[134] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000239C, 0x2388e238); /*0x0000239C  DIP_A_GGM_LUT[135] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023A0, 0x24090240); /*0x000023A0  DIP_A_GGM_LUT[136] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023A4, 0x24892248); /*0x000023A4  DIP_A_GGM_LUT[137] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023A8, 0x25094250); /*0x000023A8  DIP_A_GGM_LUT[138] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023AC, 0x25896258); /*0x000023AC  DIP_A_GGM_LUT[139] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023B0, 0x26098260); /*0x000023B0  DIP_A_GGM_LUT[140] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023B4, 0x2689a268); /*0x000023B4  DIP_A_GGM_LUT[141] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023B8, 0x2709c270); /*0x000023B8  DIP_A_GGM_LUT[142] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023BC, 0x2789e278); /*0x000023BC  DIP_A_GGM_LUT[143] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023C0, 0x280a0280); /*0x000023C0  DIP_A_GGM_LUT[144] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023C4, 0x288a2288); /*0x000023C4  DIP_A_GGM_LUT[145] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023C8, 0x290a4290); /*0x000023C8  DIP_A_GGM_LUT[146] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023CC, 0x298a6298); /*0x000023CC  DIP_A_GGM_LUT[147] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023D0, 0x2a0a82a0); /*0x000023D0  DIP_A_GGM_LUT[148] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023D4, 0x2a8aa2a8); /*0x000023D4  DIP_A_GGM_LUT[149] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023D8, 0x2b0ac2b0); /*0x000023D8  DIP_A_GGM_LUT[150] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023DC, 0x2b8ae2b8); /*0x000023DC  DIP_A_GGM_LUT[151] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023E0, 0x2c0b02c0); /*0x000023E0  DIP_A_GGM_LUT[152] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023E4, 0x2c8b22c8); /*0x000023E4  DIP_A_GGM_LUT[153] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023E8, 0x2d0b42d0); /*0x000023E8  DIP_A_GGM_LUT[154] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023EC, 0x2d8b62d8); /*0x000023EC  DIP_A_GGM_LUT[155] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023F0, 0x2e0b82e0); /*0x000023F0  DIP_A_GGM_LUT[156] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023F4, 0x2e8ba2e8); /*0x000023F4  DIP_A_GGM_LUT[157] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023F8, 0x2f0bc2f0); /*0x000023F8  DIP_A_GGM_LUT[158] */ 
        imageioPackage.pDrvDip->writeReg( 0x000023FC, 0x2f8be2f8); /*0x000023FC  DIP_A_GGM_LUT[159] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002400, 0x300c0300); /*0x00002400  DIP_A_GGM_LUT[160] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002404, 0x308c2308); /*0x00002404  DIP_A_GGM_LUT[161] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002408, 0x310c4310); /*0x00002408  DIP_A_GGM_LUT[162] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000240C, 0x318c6318); /*0x0000240C  DIP_A_GGM_LUT[163] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002410, 0x320c8320); /*0x00002410  DIP_A_GGM_LUT[164] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002414, 0x328ca328); /*0x00002414  DIP_A_GGM_LUT[165] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002418, 0x330cc330); /*0x00002418  DIP_A_GGM_LUT[166] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000241C, 0x338ce338); /*0x0000241C  DIP_A_GGM_LUT[167] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002420, 0x340d0340); /*0x00002420  DIP_A_GGM_LUT[168] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002424, 0x348d2348); /*0x00002424  DIP_A_GGM_LUT[169] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002428, 0x350d4350); /*0x00002428  DIP_A_GGM_LUT[170] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000242C, 0x358d6358); /*0x0000242C  DIP_A_GGM_LUT[171] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002430, 0x360d8360); /*0x00002430  DIP_A_GGM_LUT[172] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002434, 0x368da368); /*0x00002434  DIP_A_GGM_LUT[173] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002438, 0x370dc370); /*0x00002438,   DIP_A_GGM_LUT[174] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000243C, 0x378de378); /*0x0000243C,   DIP_A_GGM_LUT[175] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002440, 0x380e0380); /*0x00002440,   DIP_A_GGM_LUT[176] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002444, 0x388e2388); /*0x00002444,   DIP_A_GGM_LUT[177] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002448, 0x390e4390); /*0x00002448,   DIP_A_GGM_LUT[178] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000244C, 0x398e6398); /*0x0000244C,   DIP_A_GGM_LUT[179] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002450, 0x3a0e83a0); /*0x00002450,   DIP_A_GGM_LUT[180] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002454, 0x3a8ea3a8); /*0x00002454,   DIP_A_GGM_LUT[182] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002458, 0x3b0ec3b0); /*0x00002458,   DIP_A_GGM_LUT[182] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000245C, 0x3b8ee3b8); /*0x0000245C,   DIP_A_GGM_LUT[183] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002460, 0x3c0f03c0); /*0x00002460,   DIP_A_GGM_LUT[184] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002464, 0x3c8f23c8); /*0x00002464,   DIP_A_GGM_LUT[185] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002468, 0x3d0f43d0); /*0x00002468,   DIP_A_GGM_LUT[186] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000246C, 0x3d8f63d8); /*0x0000246C,   DIP_A_GGM_LUT[187] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002470, 0x3e0f83e0); /*0x00002470,   DIP_A_GGM_LUT[188] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002474, 0x3e0f83e0); /*0x00002474,   DIP_A_GGM_LUT[189] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002478, 0x3e0f83e0); /*0x00002478,   DIP_A_GGM_LUT[190] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000247C, 0x3e0f83e0); /*0x0000247C,   DIP_A_GGM_LUT[191] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002480, 0x100307FF); /* 0x15024480: DIP_X_GGM_CTRL */
    }
    return 0;
}

MINT32
IspDrv_B::
setGgm2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_GGM2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_GGM2_LUT,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    else
    {
        imageioPackage.pDrvDip->writeReg( 0x00002640, 0x00000000); /*0x15024640  DIP_A_GGM2_LUT[0] */
        imageioPackage.pDrvDip->writeReg( 0x00002644, 0x00200802); /*0x15024644  DIP_A_GGM2_LUT[1] */
        imageioPackage.pDrvDip->writeReg( 0x00002648, 0x00401004); /*0x15024648  DIP_A_GGM2_LUT[2] */
        imageioPackage.pDrvDip->writeReg( 0x0000264C, 0x00601806); /*0x1502464C  DIP_A_GGM2_LUT[3] */
        imageioPackage.pDrvDip->writeReg( 0x00002650, 0x00802008); /*0x15024650  DIP_A_GGM2_LUT[4] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002654, 0x00a0280a); /*0x15024654  DIP_A_GGM2_LUT[5] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002658, 0x00c0300c); /*0x15024658  DIP_A_GGM2_LUT[6] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000265C, 0x00e0380e); /*0x1502465C  DIP_A_GGM2_LUT[7] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002660, 0x01004010); /*0x15024660  DIP_A_GGM2_LUT[8] */  
        imageioPackage.pDrvDip->writeReg( 0x00002664, 0x01204812); /*0x15024664  DIP_A_GGM2_LUT[9] */  
        imageioPackage.pDrvDip->writeReg( 0x00002668, 0x01405014); /*0x15024668  DIP_A_GGM2_LUT[10] */  
        imageioPackage.pDrvDip->writeReg( 0x0000266C, 0x01605816); /*0x1502466C  DIP_A_GGM2_LUT[11] */  
        imageioPackage.pDrvDip->writeReg( 0x00002670, 0x01806018); /*0x15024670  DIP_A_GGM2_LUT[12] */   
        imageioPackage.pDrvDip->writeReg( 0x00002674, 0x01a0681a); /*0x15024674  DIP_A_GGM2_LUT[13] */   
        imageioPackage.pDrvDip->writeReg( 0x00002678, 0x01c0701c); /*0x15024678  DIP_A_GGM2_LUT[14] */   
        imageioPackage.pDrvDip->writeReg( 0x0000267C, 0x01e0781e); /*0x1502467C  DIP_A_GGM2_LUT[15] */   
        imageioPackage.pDrvDip->writeReg( 0x00002680, 0x02008020); /*0x15024680  DIP_A_GGM2_LUT[16] */  
        imageioPackage.pDrvDip->writeReg( 0x00002684, 0x02208822); /*0x15024684  DIP_A_GGM2_LUT[17] */  
        imageioPackage.pDrvDip->writeReg( 0x00002688, 0x02409024); /*0x15024688  DIP_A_GGM2_LUT[18] */  
        imageioPackage.pDrvDip->writeReg( 0x0000268C, 0x02609826); /*0x1502468C  DIP_A_GGM2_LUT[19] */  
        imageioPackage.pDrvDip->writeReg( 0x00002690, 0x0280a028); /*0x15024690  DIP_A_GGM2_LUT[20] */   
        imageioPackage.pDrvDip->writeReg( 0x00002694, 0x02a0a82a); /*0x15024694  DIP_A_GGM2_LUT[21] */   
        imageioPackage.pDrvDip->writeReg( 0x00002698, 0x02c0b02c); /*0x15024698  DIP_A_GGM2_LUT[22] */   
        imageioPackage.pDrvDip->writeReg( 0x0000269C, 0x02e0b82e); /*0x1502469C  DIP_A_GGM2_LUT[23] */   
        imageioPackage.pDrvDip->writeReg( 0x000026A0, 0x0300c030); /*0x150246A0  DIP_A_GGM2_LUT[24] */    
        imageioPackage.pDrvDip->writeReg( 0x000026A4, 0x0320c832); /*0x150246A4  DIP_A_GGM2_LUT[25] */    
        imageioPackage.pDrvDip->writeReg( 0x000026A8, 0x0340d034); /*0x150246A8  DIP_A_GGM2_LUT[26] */    
        imageioPackage.pDrvDip->writeReg( 0x000026AC, 0x0360d836); /*0x150246AC  DIP_A_GGM2_LUT[27] */    
        imageioPackage.pDrvDip->writeReg( 0x000026B0, 0x0380e038); /*0x150246B0  DIP_A_GGM2_LUT[28] */     
        imageioPackage.pDrvDip->writeReg( 0x000026B4, 0x03a0e83a); /*0x150246B4  DIP_A_GGM2_LUT[29] */     
        imageioPackage.pDrvDip->writeReg( 0x000026B8, 0x03c0f03c); /*0x150246B8  DIP_A_GGM2_LUT[30] */     
        imageioPackage.pDrvDip->writeReg( 0x000026BC, 0x03e0f83e); /*0x150246BC  DIP_A_GGM2_LUT[31] */     
        imageioPackage.pDrvDip->writeReg( 0x000026C0, 0x04010040); /*0x150246C0  DIP_A_GGM2_LUT[32] */  
        imageioPackage.pDrvDip->writeReg( 0x000026C4, 0x04210842); /*0x150246C4  DIP_A_GGM2_LUT[33] */  
        imageioPackage.pDrvDip->writeReg( 0x000026C8, 0x04411044); /*0x150246C8  DIP_A_GGM2_LUT[34] */  
        imageioPackage.pDrvDip->writeReg( 0x000026CC, 0x04611846); /*0x150246CC  DIP_A_GGM2_LUT[35] */  
        imageioPackage.pDrvDip->writeReg( 0x000026D0, 0x04812048); /*0x150246D0  DIP_A_GGM2_LUT[36] */  
        imageioPackage.pDrvDip->writeReg( 0x000026D4, 0x04a1284a); /*0x150246D4  DIP_A_GGM2_LUT[37] */  
        imageioPackage.pDrvDip->writeReg( 0x000026D8, 0x04c1304c); /*0x150246D8  DIP_A_GGM2_LUT[38] */  
        imageioPackage.pDrvDip->writeReg( 0x000026DC, 0x04e1384e); /*0x150246DC  DIP_A_GGM2_LUT[39] */  
        imageioPackage.pDrvDip->writeReg( 0x000026E0, 0x05014050); /*0x150246E0  DIP_A_GGM2_LUT[40] */   
        imageioPackage.pDrvDip->writeReg( 0x000026E4, 0x05214852); /*0x150246E4  DIP_A_GGM2_LUT[41] */   
        imageioPackage.pDrvDip->writeReg( 0x000026E8, 0x05415054); /*0x150246E8  DIP_A_GGM2_LUT[42] */   
        imageioPackage.pDrvDip->writeReg( 0x000026EC, 0x05615856); /*0x150246EC  DIP_A_GGM2_LUT[43] */   
        imageioPackage.pDrvDip->writeReg( 0x000026F0, 0x05816058); /*0x150246F0  DIP_A_GGM2_LUT[44] */    
        imageioPackage.pDrvDip->writeReg( 0x000026F4, 0x05a1685a); /*0x150246F4  DIP_A_GGM2_LUT[45] */    
        imageioPackage.pDrvDip->writeReg( 0x000026F8, 0x05c1705c); /*0x150246F8  DIP_A_GGM2_LUT[46] */    
        imageioPackage.pDrvDip->writeReg( 0x000026FC, 0x05e1785e); /*0x150246FC  DIP_A_GGM2_LUT[47] */    
        imageioPackage.pDrvDip->writeReg( 0x00002700, 0x06018060); /*0x15024700  DIP_A_GGM2_LUT[48] */     
        imageioPackage.pDrvDip->writeReg( 0x00002704, 0x06218862); /*0x15024704  DIP_A_GGM2_LUT[49] */     
        imageioPackage.pDrvDip->writeReg( 0x00002708, 0x06419064); /*0x15024708  DIP_A_GGM2_LUT[50] */     
        imageioPackage.pDrvDip->writeReg( 0x0000270C, 0x06619866); /*0x1502470C  DIP_A_GGM2_LUT[51] */     
        imageioPackage.pDrvDip->writeReg( 0x00002710, 0x0681a068); /*0x15024710  DIP_A_GGM2_LUT[52] */      
        imageioPackage.pDrvDip->writeReg( 0x00002714, 0x06a1a86a); /*0x15024714  DIP_A_GGM2_LUT[53] */      
        imageioPackage.pDrvDip->writeReg( 0x00002718, 0x06c1b06c); /*0x15024718  DIP_A_GGM2_LUT[54] */      
        imageioPackage.pDrvDip->writeReg( 0x0000271C, 0x06e1b86e); /*0x1502471C  DIP_A_GGM2_LUT[55] */      
        imageioPackage.pDrvDip->writeReg( 0x00002720, 0x0701c070); /*0x15024720  DIP_A_GGM2_LUT[56] */  
        imageioPackage.pDrvDip->writeReg( 0x00002724, 0x0721c872); /*0x15024724  DIP_A_GGM2_LUT[57] */  
        imageioPackage.pDrvDip->writeReg( 0x00002728, 0x0741d074); /*0x15024728  DIP_A_GGM2_LUT[58] */  
        imageioPackage.pDrvDip->writeReg( 0x0000272C, 0x0761d876); /*0x1502472C  DIP_A_GGM2_LUT[59] */  
        imageioPackage.pDrvDip->writeReg( 0x00002730, 0x0781e078); /*0x15024730  DIP_A_GGM2_LUT[60] */  
        imageioPackage.pDrvDip->writeReg( 0x00002734, 0x07a1e87a); /*0x15024734  DIP_A_GGM2_LUT[61] */  
        imageioPackage.pDrvDip->writeReg( 0x00002738, 0x07c1f07c); /*0x15024738  DIP_A_GGM2_LUT[62] */  
        imageioPackage.pDrvDip->writeReg( 0x0000273C, 0x07e1f87e); /*0x1502473C  DIP_A_GGM2_LUT[63] */  
        imageioPackage.pDrvDip->writeReg( 0x00002740, 0x08020080); /*0x15024740  DIP_A_GGM2_LUT[64] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002744, 0x08421084); /*0x15024744  DIP_A_GGM2_LUT[65] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002748, 0x08822088); /*0x15024748  DIP_A_GGM2_LUT[66] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000274C, 0x08c2308c); /*0x1502474C  DIP_A_GGM2_LUT[67] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002750, 0x09024090); /*0x15024750  DIP_A_GGM2_LUT[68] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002754, 0x09425094); /*0x15024754  DIP_A_GGM2_LUT[69] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002758, 0x09826098); /*0x15024758  DIP_A_GGM2_LUT[70] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000275C, 0x09c2709c); /*0x1502475C  DIP_A_GGM2_LUT[71] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002760, 0x0a0280a0); /*0x15024760  DIP_A_GGM2_LUT[72] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002764, 0x0a4290a4); /*0x15024764  DIP_A_GGM2_LUT[73] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002768, 0x0a82a0a8); /*0x15024768  DIP_A_GGM2_LUT[74] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000276C, 0x0ac2b0ac); /*0x1502476C  DIP_A_GGM2_LUT[75] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002770, 0x0b02c0b0); /*0x15024770  DIP_A_GGM2_LUT[76] */  
        imageioPackage.pDrvDip->writeReg( 0x00002774, 0x0b42d0b4); /*0x15024774  DIP_A_GGM2_LUT[77] */  
        imageioPackage.pDrvDip->writeReg( 0x00002778, 0x0b82e0b8); /*0x15024778  DIP_A_GGM2_LUT[78] */  
        imageioPackage.pDrvDip->writeReg( 0x0000277C, 0x0bc2f0bc); /*0x1502477C  DIP_A_GGM2_LUT[79] */  
        imageioPackage.pDrvDip->writeReg( 0x00002780, 0x0c0300c0); /*0x15024780  DIP_A_GGM2_LUT[80] */   
        imageioPackage.pDrvDip->writeReg( 0x00002784, 0x0c4310c4); /*0x15024784  DIP_A_GGM2_LUT[81] */   
        imageioPackage.pDrvDip->writeReg( 0x00002788, 0x0c8320c8); /*0x15024788  DIP_A_GGM2_LUT[82] */   
        imageioPackage.pDrvDip->writeReg( 0x0000278C, 0x0cc330cc); /*0x1502478C  DIP_A_GGM2_LUT[83] */   
        imageioPackage.pDrvDip->writeReg( 0x00002790, 0x0d0340d0); /*0x15024790  DIP_A_GGM2_LUT[84] */   
        imageioPackage.pDrvDip->writeReg( 0x00002794, 0x0d4350d4); /*0x15024794  DIP_A_GGM2_LUT[85] */   
        imageioPackage.pDrvDip->writeReg( 0x00002798, 0x0d8360d8); /*0x15024798  DIP_A_GGM2_LUT[86] */   
        imageioPackage.pDrvDip->writeReg( 0x0000279C, 0x0dc370dc); /*0x1502479C  DIP_A_GGM2_LUT[87] */   
        imageioPackage.pDrvDip->writeReg( 0x000027A0, 0x0e0380e0); /*0x150247A0  DIP_A_GGM2_LUT[88] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027A4, 0x0e4390e4); /*0x150247A4  DIP_A_GGM2_LUT[89] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027A8, 0x0e83a0e8); /*0x150247A8  DIP_A_GGM2_LUT[90] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027AC, 0x0ec3b0ec); /*0x150247AC  DIP_A_GGM2_LUT[91] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027B0, 0x0f03c0f0); /*0x150247B0  DIP_A_GGM2_LUT[92] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027B4, 0x0f43d0f4); /*0x150247B4  DIP_A_GGM2_LUT[93] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027B8, 0x0f83e0f8); /*0x150247B8  DIP_A_GGM2_LUT[94] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027BC, 0x0fc3f0fc); /*0x150247BC  DIP_A_GGM2_LUT[95] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027C0, 0x10040100); /*0x150247C0  DIP_A_GGM2_LUT[96] */  
        imageioPackage.pDrvDip->writeReg( 0x000027C4, 0x10842108); /*0x150247C4  DIP_A_GGM2_LUT[97] */  
        imageioPackage.pDrvDip->writeReg( 0x000027C8, 0x11044110); /*0x150247C8  DIP_A_GGM2_LUT[98] */  
        imageioPackage.pDrvDip->writeReg( 0x000027CC, 0x11846118); /*0x150247CC  DIP_A_GGM2_LUT[99] */  
        imageioPackage.pDrvDip->writeReg( 0x000027D0, 0x12048120); /*0x150247D0  DIP_A_GGM2_LUT[100] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027D4, 0x1284a128); /*0x150247D4  DIP_A_GGM2_LUT[101] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027D8, 0x1304c130); /*0x150247D8  DIP_A_GGM2_LUT[102] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027DC, 0x1384e138); /*0x150247DC  DIP_A_GGM2_LUT[103] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027E0, 0x14050140); /*0x150247E0  DIP_A_GGM2_LUT[104] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027E4, 0x14852148); /*0x150247E4  DIP_A_GGM2_LUT[105] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027E8, 0x15054150); /*0x150247E8  DIP_A_GGM2_LUT[106] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027EC, 0x15856158); /*0x150247EC  DIP_A_GGM2_LUT[107] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027F0, 0x16058160); /*0x150247F0  DIP_A_GGM2_LUT[108] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027F4, 0x1685a168); /*0x150247F4  DIP_A_GGM2_LUT[109] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027F8, 0x1705c170); /*0x150247F8  DIP_A_GGM2_LUT[110] */ 
        imageioPackage.pDrvDip->writeReg( 0x000027FC, 0x1785e178); /*0x150247FC  DIP_A_GGM2_LUT[111] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002800, 0x18060180); /*0x15024800  DIP_A_GGM2_LUT[112] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002804, 0x18862188); /*0x15024804  DIP_A_GGM2_LUT[113] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002808, 0x19064190); /*0x15024808  DIP_A_GGM2_LUT[114] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000280C, 0x19866198); /*0x1502480C  DIP_A_GGM2_LUT[115] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002810, 0x1a0681a0); /*0x15024810  DIP_A_GGM2_LUT[116] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002814, 0x1a86a1a8); /*0x15024814  DIP_A_GGM2_LUT[117] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002818, 0x1b06c1b0); /*0x15024818  DIP_A_GGM2_LUT[118] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000281C, 0x1b86e1b8); /*0x1502481C  DIP_A_GGM2_LUT[119] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002820, 0x1c0701c0); /*0x15024820  DIP_A_GGM2_LUT[120] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002824, 0x1c8721c8); /*0x15024824  DIP_A_GGM2_LUT[121] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002828, 0x1d0741d0); /*0x15024828  DIP_A_GGM2_LUT[122] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000282C, 0x1d8761d8); /*0x1502482C  DIP_A_GGM2_LUT[123] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002830, 0x1e0781e0); /*0x15024830  DIP_A_GGM2_LUT[124] */  
        imageioPackage.pDrvDip->writeReg( 0x00002834, 0x1e87a1e8); /*0x15024834  DIP_A_GGM2_LUT[125] */  
        imageioPackage.pDrvDip->writeReg( 0x00002838, 0x1f07c1f0); /*0x15024838  DIP_A_GGM2_LUT[126] */  
        imageioPackage.pDrvDip->writeReg( 0x0000283C, 0x1f87e1f8); /*0x1502483C  DIP_A_GGM2_LUT[127] */  
        imageioPackage.pDrvDip->writeReg( 0x00002840, 0x20080200); /*0x15024840  DIP_A_GGM2_LUT[128] */  
        imageioPackage.pDrvDip->writeReg( 0x00002844, 0x20882208); /*0x15024844  DIP_A_GGM2_LUT[129] */  
        imageioPackage.pDrvDip->writeReg( 0x00002848, 0x21084210); /*0x15024848  DIP_A_GGM2_LUT[130] */  
        imageioPackage.pDrvDip->writeReg( 0x0000284C, 0x21886218); /*0x1502484C  DIP_A_GGM2_LUT[131] */  
        imageioPackage.pDrvDip->writeReg( 0x00002850, 0x22088220); /*0x15024850  DIP_A_GGM2_LUT[132] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002854, 0x2288a228); /*0x15024854  DIP_A_GGM2_LUT[133] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002858, 0x2308c230); /*0x15024858  DIP_A_GGM2_LUT[134] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000285C, 0x2388e238); /*0x1502485C  DIP_A_GGM2_LUT[135] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002860, 0x24090240); /*0x15024860  DIP_A_GGM2_LUT[136] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002864, 0x24892248); /*0x15024864  DIP_A_GGM2_LUT[137] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002868, 0x25094250); /*0x15024868  DIP_A_GGM2_LUT[138] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000286C, 0x25896258); /*0x1502486C  DIP_A_GGM2_LUT[139] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002870, 0x26098260); /*0x15024870  DIP_A_GGM2_LUT[140] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002874, 0x2689a268); /*0x15024874  DIP_A_GGM2_LUT[141] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002878, 0x2709c270); /*0x15024878  DIP_A_GGM2_LUT[142] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000287C, 0x2789e278); /*0x1502487C  DIP_A_GGM2_LUT[143] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002880, 0x280a0280); /*0x15024880  DIP_A_GGM2_LUT[144] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002884, 0x288a2288); /*0x15024884  DIP_A_GGM2_LUT[145] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002888, 0x290a4290); /*0x15024888  DIP_A_GGM2_LUT[146] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000288C, 0x298a6298); /*0x1502488C  DIP_A_GGM2_LUT[147] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002890, 0x2a0a82a0); /*0x15024890  DIP_A_GGM2_LUT[148] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002894, 0x2a8aa2a8); /*0x15024894  DIP_A_GGM2_LUT[149] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002898, 0x2b0ac2b0); /*0x15024898  DIP_A_GGM2_LUT[150] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000289C, 0x2b8ae2b8); /*0x1502489C  DIP_A_GGM2_LUT[151] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028A0, 0x2c0b02c0); /*0x150248A0  DIP_A_GGM2_LUT[152] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028A4, 0x2c8b22c8); /*0x150248A4  DIP_A_GGM2_LUT[153] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028A8, 0x2d0b42d0); /*0x150248A8  DIP_A_GGM2_LUT[154] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028AC, 0x2d8b62d8); /*0x150248AC  DIP_A_GGM2_LUT[155] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028B0, 0x2e0b82e0); /*0x150248B0  DIP_A_GGM2_LUT[156] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028B4, 0x2e8ba2e8); /*0x150248B4  DIP_A_GGM2_LUT[157] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028B8, 0x2f0bc2f0); /*0x150248B8  DIP_A_GGM2_LUT[158] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028BC, 0x2f8be2f8); /*0x150248BC  DIP_A_GGM2_LUT[159] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028C0, 0x300c0300); /*0x150248C0  DIP_A_GGM2_LUT[160] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028C4, 0x308c2308); /*0x150248C4  DIP_A_GGM2_LUT[161] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028C8, 0x310c4310); /*0x150248C8  DIP_A_GGM2_LUT[162] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028CC, 0x318c6318); /*0x150248CC  DIP_A_GGM2_LUT[163] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028D0, 0x320c8320); /*0x150248D0  DIP_A_GGM2_LUT[164] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028D4, 0x328ca328); /*0x150248D4  DIP_A_GGM2_LUT[165] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028D8, 0x330cc330); /*0x150248D8  DIP_A_GGM2_LUT[166] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028DC, 0x338ce338); /*0x150248DC  DIP_A_GGM2_LUT[167] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028E0, 0x340d0340); /*0x150248E0  DIP_A_GGM2_LUT[168] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028E4, 0x348d2348); /*0x150248E4  DIP_A_GGM2_LUT[169] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028E8, 0x350d4350); /*0x150248E8  DIP_A_GGM2_LUT[170] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028EC, 0x358d6358); /*0x150248EC  DIP_A_GGM2_LUT[171] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028F0, 0x360d8360); /*0x150248F0  DIP_A_GGM2_LUT[172] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028F4, 0x368da368); /*0x150248F4  DIP_A_GGM2_LUT[173] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028F8, 0x370dc370); /*0x150248F8,   DIP_A_GGM2_LUT[174] */ 
        imageioPackage.pDrvDip->writeReg( 0x000028FC, 0x378de378); /*0x150248FC,   DIP_A_GGM2_LUT[175] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002900, 0x380e0380); /*0x15024900,   DIP_A_GGM2_LUT[176] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002904, 0x388e2388); /*0x15024904,   DIP_A_GGM2_LUT[177] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002908, 0x390e4390); /*0x15024908,   DIP_A_GGM2_LUT[178] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000290C, 0x398e6398); /*0x1502490C,   DIP_A_GGM2_LUT[179] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002910, 0x3a0e83a0); /*0x15024910,   DIP_A_GGM2_LUT[180] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002914, 0x3a8ea3a8); /*0x15024914,   DIP_A_GGM2_LUT[182] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002918, 0x3b0ec3b0); /*0x15024918,   DIP_A_GGM2_LUT[182] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000291C, 0x3b8ee3b8); /*0x1502491C,   DIP_A_GGM2_LUT[183] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002920, 0x3c0f03c0); /*0x15024920,   DIP_A_GGM2_LUT[184] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002924, 0x3c8f23c8); /*0x15024924,   DIP_A_GGM2_LUT[185] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002928, 0x3d0f43d0); /*0x15024928,   DIP_A_GGM2_LUT[186] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000292C, 0x3d8f63d8); /*0x1502492C,   DIP_A_GGM2_LUT[187] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002930, 0x3e0f83e0); /*0x15024930,   DIP_A_GGM2_LUT[188] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002934, 0x3e0f83e0); /*0x15024934,   DIP_A_GGM2_LUT[189] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002938, 0x3e0f83e0); /*0x15024938,   DIP_A_GGM2_LUT[190] */ 
        imageioPackage.pDrvDip->writeReg( 0x0000293C, 0x3e0f83e0); /*0x1502493C,   DIP_A_GGM2_LUT[191] */ 
        imageioPackage.pDrvDip->writeReg( 0x00002940, 0x100307FF); /* 0x15024940: DIP_A_GGM2_CTRL */


    }
    return 0;
}

MINT32
IspDrv_B::
setWshift(DIP_ISP_PIPE &imageioPackage)
{
    return 0;
}

MINT32
IspDrv_B::
setWsync(DIP_ISP_PIPE &imageioPackage)
{
    return 0;
}

MINT32
IspDrv_B::
setC24(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C24_TILE_EDGE, 0x0f);
    return 0;
}
MINT32
IspDrv_B::
setC02b(DIP_ISP_PIPE &imageioPackage)
{
    // TODO: under check about C02/C02B
    //INTERP_MODE of C02 and C02B = 1 <==request from algo team.
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C02B_CON, 0x01f);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C02B_CROP_CON1, 0x00);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C02B_CROP_CON2, 0x00);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02B_CROP_CON1,C02_CROP_XSTART,0x0);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02B_CROP_CON1,C02_CROP_XEND,imageioPackage.DMAImgi.dma_cfg.size.xsize-1);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02B_CROP_CON2,C02_CROP_YSTART,0x0);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02B_CROP_CON2,C02_CROP_YEND,imageioPackage.DMAImgi.dma_cfg.size.h-1);

    return 0;
}
MINT32
IspDrv_B::
setMfb(DIP_ISP_PIPE &imageioPackage)
{
#if 0
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_MFB,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_MFB_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}

	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_CON,BLD_MODE,imageioPackage.mfb_cfg.bld_mode);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_LL_CON3,BLD_LL_OUT_XSIZE,imageioPackage.mfb_cfg.mfb_out.w);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_LL_CON3,BLD_LL_OUT_YSIZE,imageioPackage.mfb_cfg.mfb_out.h);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_LL_CON3,BLD_LL_OUT_XOFST,imageioPackage.mfb_cfg.out_xofst);
#endif
    return 0;
}

MINT32
IspDrv_B::
setG2c(DIP_ISP_PIPE &imageioPackage)
{
    DIP_FUNC_DBG("setG2c in (%d)",imageioPackage.isApplyTuning);
    if(imageioPackage.isApplyTuning == MTRUE) {
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_G2C,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_G2C_CONV_0A,size,(MUINT32*)imageioPackage.pTuningIspReg);
    } else {
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CONV_0A, 0x012D0099);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CONV_0B, 0x0000003A);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CONV_1A, 0x075607AA);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CONV_1B, 0x00000100);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CONV_2A, 0x072A0100);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CONV_2B, 0x000007D6);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_SHADE_CON_1,  0x0118000E);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_SHADE_CON_2, 0x0074B740);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_SHADE_CON_3,  0x00000133);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_SHADE_TAR, 0x079F0A5A);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_SHADE_SP,  0x00000000);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CFC_CON_1, 0x03f70080);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CFC_CON_2,  0x1CE539CE);
    }

    return 0;
}

MINT32
IspDrv_B::
setC42(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C42_CON, 0xf0);
    return 0;
}

MINT32
IspDrv_B::
setSl2(DIP_ISP_PIPE &imageioPackage)
{
//3A will calculate and set correct value

	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSl2b(DIP_ISP_PIPE &imageioPackage)
{
//3A will calculate and set correct value
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2B,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2B_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSl2c(DIP_ISP_PIPE &imageioPackage)
{
//3A will calculate and set correct value
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2C,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2C_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}

    return 0;
}

MINT32
IspDrv_B::
setSl2d(DIP_ISP_PIPE &imageioPackage)
{
//3A will calculate and set correct value
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2D,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2D_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}
MINT32
IspDrv_B::
setSl2e(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2E,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2E_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}
MINT32
IspDrv_B::
setSl2k(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2K,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2K_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSl2g(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2G,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2G_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSl2h(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2H,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2H_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSl2i(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SL2I,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2I_CEN,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setRnr(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_RNR,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_RNR_CON1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setHfg(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_HFG,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_HFG_CON_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setFlc(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_FLC,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_FLC_OFFST0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}
MINT32
IspDrv_B::
setFlc2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_FLC2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_FLC2_OFFST0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setMfbw(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;
// MFB and MFBW should turn on and off together
    return 0;
}

MINT32
IspDrv_B::
setNbc(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_ANR,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_NBC_ANR_TBL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setNbc2(DIP_ISP_PIPE &imageioPackage)
{
//ANR2 and Bok : should choose one
	if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_ANR2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_NBC2_ANR2_CON1,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setNdg(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NDG,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_NDG_RAN_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}
MINT32
IspDrv_B::
setNdg2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NDG2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_NDG2_RAN_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSrz1(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_CONTROL,imageioPackage.srz1Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_IN_IMG,SRZ_IN_WD,imageioPackage.srz1Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_IN_IMG,SRZ_IN_HT,imageioPackage.srz1Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz1Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz1Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_HORI_STEP,imageioPackage.srz1Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_VERT_STEP,imageioPackage.srz1Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_HORI_INT_OFST,imageioPackage.srz1Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_HORI_SUB_OFST,imageioPackage.srz1Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_VERT_INT_OFST,imageioPackage.srz1Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_VERT_SUB_OFST,imageioPackage.srz1Cfg.crop.floatY);

    return 0;
}
MINT32
IspDrv_B::
setSrz2(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_CONTROL,imageioPackage.srz2Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ2_IN_IMG,SRZ_IN_WD,imageioPackage.srz2Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ2_IN_IMG,SRZ_IN_HT,imageioPackage.srz2Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ2_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz2Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ2_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz2Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_HORI_STEP,imageioPackage.srz2Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_VERT_STEP,imageioPackage.srz2Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_HORI_INT_OFST,imageioPackage.srz2Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_HORI_SUB_OFST,imageioPackage.srz2Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_VERT_INT_OFST,imageioPackage.srz2Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ2_VERT_SUB_OFST,imageioPackage.srz2Cfg.crop.floatY);

    return 0;
}
MINT32
IspDrv_B::
setSrz3(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_CONTROL,imageioPackage.srz3Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ3_IN_IMG,SRZ_IN_WD,imageioPackage.srz3Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ3_IN_IMG,SRZ_IN_HT,imageioPackage.srz3Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ3_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz3Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ3_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz3Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_HORI_STEP,imageioPackage.srz3Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_VERT_STEP,imageioPackage.srz3Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_HORI_INT_OFST,imageioPackage.srz3Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_HORI_SUB_OFST,imageioPackage.srz3Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_VERT_INT_OFST,imageioPackage.srz3Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ3_VERT_SUB_OFST,imageioPackage.srz3Cfg.crop.floatY);
    return 0;
}
MINT32
IspDrv_B::
setSrz4(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_CONTROL,imageioPackage.srz4Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ4_IN_IMG,SRZ_IN_WD,imageioPackage.srz4Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ4_IN_IMG,SRZ_IN_HT,imageioPackage.srz4Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ4_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz4Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ4_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz4Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_HORI_STEP,imageioPackage.srz4Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_VERT_STEP,imageioPackage.srz4Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_HORI_INT_OFST,imageioPackage.srz4Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_HORI_SUB_OFST,imageioPackage.srz4Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_VERT_INT_OFST,imageioPackage.srz4Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ4_VERT_SUB_OFST,imageioPackage.srz4Cfg.crop.floatY);

    return 0;
}

MINT32
IspDrv_B::
setSrz5(DIP_ISP_PIPE &imageioPackage)
{
#if 0
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_CONTROL,imageioPackage.srz5Cfg.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ5_IN_IMG,SRZ_IN_WD,imageioPackage.srz5Cfg.inout_size.in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ5_IN_IMG,SRZ_IN_HT,imageioPackage.srz5Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ5_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz5Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ5_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz5Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_HORI_STEP,imageioPackage.srz5Cfg.h_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_VERT_STEP,imageioPackage.srz5Cfg.v_step);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_HORI_INT_OFST,imageioPackage.srz5Cfg.crop.x);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_HORI_SUB_OFST,imageioPackage.srz5Cfg.crop.floatX);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_VERT_INT_OFST,imageioPackage.srz5Cfg.crop.y);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ5_VERT_SUB_OFST,imageioPackage.srz5Cfg.crop.floatY);
#endif
    return 0;
}

MINT32
IspDrv_B::
setMix1(DIP_ISP_PIPE &imageioPackage)
{
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_Y_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_Y_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_UV_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_UV_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_WT_SEL,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_B0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_B1,0xff);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_0,MIX1_DT,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_1,MIX1_M0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX1_CTRL_1,MIX1_M1,0xff);
	//DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MIX1_SPARE,imageioPackage.pTuningIspReg->DIP_X_MIX1_SPARE.Raw);

    return 0;
}

MINT32
IspDrv_B::
setMix2(DIP_ISP_PIPE &imageioPackage)
{
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_Y_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_Y_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_UV_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_UV_DEFAULT,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_WT_SEL,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_B0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_B1,0xff);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_0,MIX2_DT,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_1,MIX2_M0,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MIX2_CTRL_1,MIX2_M1,0xff);
	//DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MIX2_SPARE,imageioPackage.pTuningIspReg->DIP_X_MIX2_SPARE.Raw);

    return 0;
}

MINT32
IspDrv_B::
setMix3(DIP_ISP_PIPE &imageioPackage)
{

	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_MIX3,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_MIX3_CTRL_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setMix4(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_MIX4,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_MIX4_CTRL_0,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}


MINT32
IspDrv_B::
setPca(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_PCA,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_PCA_TBL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}

    return 0;
}

MINT32
IspDrv_B::
setSeee(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){

		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_SEEE,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SEEE_CTRL,size,(MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setCrsp(DIP_ISP_PIPE &imageioPackage)
{

	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CTRL,CRSP_HORI_EN,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CTRL,CRSP_VERT_EN,0x1);//the same with crsp_en

	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_STEP_OFST,CRSP_STEP_X,0x4);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_STEP_OFST,CRSP_STEP_Y,0x4);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_STEP_OFST,CRSP_OFST_X,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_STEP_OFST,CRSP_OFST_Y,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_OUT_IMG,CRSP_WD,imageioPackage.crspCfg.out.w);//desImgW);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_OUT_IMG,CRSP_HT,imageioPackage.crspCfg.out.h);//desImgH);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CTRL,CRSP_CROP_EN,0x1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CROP_X,CRSP_CROP_STR_X,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CROP_X,CRSP_CROP_END_X,imageioPackage.crspCfg.out.w-1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CROP_Y,CRSP_CROP_STR_Y,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CROP_Y,CRSP_CROP_END_Y,imageioPackage.crspCfg.out.h-1);

    return 0;
}

MINT32
IspDrv_B::
setNr3d(DIP_ISP_PIPE &imageioPackage)
{

	if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NR3D,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_CAM_TNR_ENG_CON,size,(MUINT32*)imageioPackage.pTuningIspReg);
        if ((imageioPackage.pTuningIspReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN & 0x1) == 0x00)
        {
            DIP_FUNC_ERR("Nr3d Top Enable but nr3d module enable is false!!! DIP_X_CAM_TNR_ENG_CON:%x",imageioPackage.pTuningIspReg->DIP_X_CAM_TNR_ENG_CON.Raw);
        }
        if ((imageioPackage.pTuningIspReg->DIP_X_MDP_TNR_TNR_ENABLE.Bits.NR3D_TNR_Y_EN & 0x1) == 0x0)
        {
            DIP_FUNC_ERR("No Noise effect!! DIP_X_MDP_TNR_TNR_ENABLE:%x",imageioPackage.pTuningIspReg->DIP_X_MDP_TNR_TNR_ENABLE.Raw);
        }
	}
	return 0;
}

MINT32
IspDrv_B::
setColor(DIP_ISP_PIPE &imageioPackage)
{

	if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
        if ((imageioPackage.pTuningIspReg->DIP_X_CAM_COLOR_START.Bits.COLOR_DISP_COLOR_START & 0x1) > 0)
        {
            imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_COLOR,addr,size);
            DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_CAM_COLOR_CFG_MAIN,size,(MUINT32*)imageioPackage.pTuningIspReg);
        }
        else
        {
            DIP_FUNC_ERR("Color Top Enable but color module enable is false!!! DIP_X_CAM_COLOR_START:%x",imageioPackage.pTuningIspReg->DIP_X_CAM_COLOR_START.Raw);
        }
	}
	return 0;
}


MINT32
IspDrv_B::
setFe(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE)
    {
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_FE,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_FE_CTRL1,size,(MUINT32*)imageioPackage.pTuningIspReg);
    }
    else
    {//default set fe_mode as 1
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_FE_CTRL1, 0xAD);
    }
    return 0;
}

MINT32
IspDrv_B::
setC24b(DIP_ISP_PIPE &imageioPackage)
{
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C24B_TILE_EDGE, 0xf); //default(0x0f)
    return 0;
}

MINT32
IspDrv_B::
setC02(DIP_ISP_PIPE &imageioPackage)
{
    // TODO: under check about C02/C02B
    //INTERP_MODE of C02 and C02B = 1 <==request from algo team.
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02_CON,C02_TPIPE_EDGE,0x1F); //default 0xf, tpipemain would update it
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02_CROP_CON1,C02_CROP_XSTART,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02_CROP_CON1,C02_CROP_XEND,imageioPackage.DMAVipi.dma_cfg.size.xsize-1);//.srcImgW-1);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02_CROP_CON2,C02_CROP_YSTART,0x0);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02_CROP_CON2,C02_CROP_YEND,imageioPackage.DMAVipi.dma_cfg.size.h-1);//srcImgH-1);

    return 0;
}

MINT32
IspDrv_B::
setFm(DIP_ISP_PIPE &imageioPackage)
{
// TODO: under check about dmgi, depi and mfbo
    if (imageioPackage.isApplyTuning == MTRUE)
    {
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_FM_SIZE,imageioPackage.pTuningIspReg->DIP_X_FM_SIZE.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_FM_TH_CON0,imageioPackage.pTuningIspReg->DIP_X_FM_TH_CON0.Raw);
    }
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DMGI_CON,0x10000019);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DMGI_CON2,0x00160010);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DMGI_CON3,0x00000000);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DEPI_CON,0x20000020);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DEPI_CON2,0x001B000F);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DEPI_CON3,0x001F000C);
//take care about the below setting. (pak2o)
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
    MINT32 i, j, k, l;
    MUINT32 ctrl_start = 0;
    MUINT32 cnt = 0;
    MUINT32 reset = 0;    
    MUINTPTR tmpVaddr = 0, tmpPaddr = 0;
    Mutex::Autolock lock(mLock);

    DIP_FUNC_INF("DIP: +, userName(%s), mInitCount(%d)", userName, this->mInitCount);


    m_iDipMetEn = ::property_get_int32("vendor.cam.dipmetlog.enable", 0);

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
        MUINT32 moduleIdx;
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
    //Read cm
    ctrl_start = DIP_READ_PHY_REG(m_pDrvDipPhy,DIP_X_CTL_START);
    DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIP_X_CTL_SW_CTL, 0x4);
    do{
        reset = DIP_READ_PHY_REG(m_pDrvDipPhy,DIP_X_CTL_SW_CTL);
        if ((reset & 0x2) == 0x2) //reset done.
        {
            break;
        }
        usleep(1000);
        cnt++;
        if (cnt == 10) //10ms
        {
            break; //to avoid while(1).
        }
    }
    while(1);
    DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIP_X_CTL_SW_CTL, 0x2);
    DIP_FUNC_INF("cnt:0x%x, ctrl_start:0x%x, reset:0x%x", cnt, ctrl_start, reset);
    DIP_WRITE_PHY_REG(m_pDrvDipPhy,DIP_X_CTL_TOP_DCM_DIS, 0x1);

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
    MINT32 i, j, k, l;
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
    MUINT32 ufd_en;

    DIP_FUNC_DBG("[DIP_DMA (%lu)] moduleIdx(%d)",\
        this->id(), dipidx);

    pDrvDip = m_pIspDrvShell->m_pDrvDip[dipidx];

    switch(this->id())
    {
        //in-dma
        case ENUM_DIP_DMA_TDRI:
            ///kk test need to add a function to get tile base address from isp_function
            DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_TDRI_BASE_ADDR,0x00);  //kk test
            DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_TDRI_OFST_ADDR,0x00);
            break;
        case ENUM_DIP_DMA_IMGI:
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGI_CON,0x80000080); //default:0x80000080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGI_CON2,0x00800080); //default:0x00800080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGI_CON3,0x00800080); //default:0x00800080
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,IMGI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //
            DIP_FUNC_DBG("[imgi]addr(0x%lx),xsize(0x%lx),h(%ld),stride(%ld)",\
               (unsigned long)(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),\
               (this->dma_cfg.size.xsize - 1),(this->dma_cfg.size.h - 1),this->dma_cfg.size.stride);
            DIP_FUNC_DBG("[imgi]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",\
                this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);
            break;
        case ENUM_DIP_DMA_IMGBI:
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGBI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGBI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGBI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGBI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGBI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGBI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGBI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGBI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGBI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGBI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGBI_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGBI_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGBI_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,IMGBI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            //
            DIP_FUNC_DBG("[imgbi]addr(0x%lx),xsize(0x%lx),h(%ld),stride(%ld)",\
               (unsigned long)(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),\
               (this->dma_cfg.size.xsize - 1),(this->dma_cfg.size.h - 1),this->dma_cfg.size.stride);
            DIP_FUNC_DBG("[imgbi]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",\
                this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);

            break;
        case ENUM_DIP_DMA_IMGCI:
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGCI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGCI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGCI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_IMGCI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGCI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGCI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGCI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGCI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGCI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMGCI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGCI_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGCI_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMGCI_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,IMGCI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            DIP_FUNC_DBG("[imgci]addr(0x%lx),xsize(0x%lx),h(%ld),stride(%ld)",\
               (unsigned long)(this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr),\
               (this->dma_cfg.size.xsize - 1),(this->dma_cfg.size.h - 1),this->dma_cfg.size.stride);
            DIP_FUNC_DBG("[imgci]format_en(%d),format(0x%x),bus_size_en(%d),bus_size(%d)",\
                this->dma_cfg.format_en,this->dma_cfg.format,this->dma_cfg.bus_size_en,this->dma_cfg.bus_size);
            break;
        case ENUM_DIP_DMA_UFDI:
            DIP_FUNC_DBG("[ufdi]xsize(0x%lx),h(%ld)",this->dma_cfg.size.xsize,this->dma_cfg.size.h);
            DIP_WRITE_REG(pDrvDip, DIP_X_UFDI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_UFDI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_UFDI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_UFDI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_UFDI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_UFDI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_UFDI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_UFDI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_UFDI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_UFDI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_UFDI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_UFDI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG( m_pIspDrvShell->m_pDrvDipPhy,DIP_X_UFDI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,UFDI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_LCEI:
            DIP_WRITE_REG(pDrvDip, DIP_X_LCEI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_LCEI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_LCEI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_LCEI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_LCEI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_LCEI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_LCEI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_LCEI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_LCEI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_LCEI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_LCEI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_LCEI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_LCEI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,LCEI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_VIPI:
            DIP_WRITE_REG(pDrvDip, DIP_X_VIPI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_VIPI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_VIPI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_VIPI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIPI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIPI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIPI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIPI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIPI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIPI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIPI_CON,0x80000080); //default:0x80000080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIPI_CON2,0x00800080); //default:0x00800080
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIPI_CON3,0x00800080); //default:0x00800080
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,VIPI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_VIP2I:
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP2I_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP2I_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP2I_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP2I_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP2I_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP2I_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP2I_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP2I_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP2I_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP2I_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIP2I_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIP2I_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIP2I_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,VIP2I_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_VIP3I:
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP3I_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP3I_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP3I_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_VIP3I_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP3I_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP3I_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP3I_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP3I_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP3I_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_VIP3I_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIP3I_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIP3I_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_VIP3I_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,VIP3I_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_DMGI:
            DIP_WRITE_REG(pDrvDip, DIP_X_DMGI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_DMGI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_DMGI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_DMGI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_DMGI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DMGI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DMGI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DMGI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DMGI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DMGI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_DMGI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_DMGI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_DMGI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,DMGI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        case ENUM_DIP_DMA_DEPI:
            DIP_WRITE_REG(pDrvDip, DIP_X_DEPI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
            DIP_WRITE_REG(pDrvDip, DIP_X_DEPI_OFST_ADDR, 0x00);
            DIP_WRITE_REG(pDrvDip, DIP_X_DEPI_XSIZE, (this->dma_cfg.size.xsize - 1));
            DIP_WRITE_REG(pDrvDip, DIP_X_DEPI_YSIZE, (this->dma_cfg.size.h - 1));
            DIP_WRITE_BITS(pDrvDip, DIP_X_DEPI_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DEPI_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DEPI_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DEPI_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DEPI_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_DEPI_STRIDE,SWAP,this->dma_cfg.swap);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_DEPI_CON,0x80000020); //default:0x80000020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_DEPI_CON2,0x00200020); //default:0x00200020
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_DEPI_CON3,0x00200020); //default:0x00200020
            //
            DIP_WRITE_BITS(pDrvDip, DIP_X_VERTICAL_FLIP_EN,DEPI_V_FLIP_EN,this->dma_cfg.v_flip_en);
            break;
        //out-dma
        case ENUM_DIP_DMA_PAK2O:
            DIP_WRITE_REG(pDrvDip,DIP_X_PAK2O_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_PAK2O_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_PAK2O_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_PAK2O_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_PAK2O_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_PAK2O_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_PAK2O_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_PAK2O_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_PAK2O_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_PAK2O_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_PAK2O_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_PAK2O_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_PAK2O_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_IMG3O:
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3O_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3O_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3O_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3O_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3O_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3O_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3O_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3O_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3O_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3O_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3O_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3O_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3O_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_IMG3BO:
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3BO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3BO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3BO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3BO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3BO_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3BO_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3BO_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3BO_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3BO_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3BO_CON,0x80000020); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3BO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3BO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3BO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_IMG3CO:
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3CO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3CO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3CO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3CO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3CO_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3CO_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3CO_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3CO_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG3CO_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3CO_CON,0x80000020); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3CO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3CO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG3CO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_FEO:
            DIP_WRITE_REG(pDrvDip,DIP_X_FEO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_FEO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_FEO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_FEO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip,DIP_X_FEO_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_FEO_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_FEO_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_FEO_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_FEO_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);

            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_FEO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_FEO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_FEO_CON3,0x00400040); //default:0x00400040
            break;
        case ENUM_DIP_DMA_IMG2O:
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2O_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2O_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2O_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2O_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2O_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2O_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2O_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2O_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2O_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG2O_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG2O_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG2O_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2O_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        case ENUM_DIP_DMA_IMG2BO:
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2BO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2BO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2BO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2BO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2BO_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2BO_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2BO_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2BO_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_IMG2BO_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG2BO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG2BO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG2BO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_IMG2BO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            break;
        default:
            DIP_FUNC_ERR("[Error]Not support this dip thread(%d)",this->id());
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
            cqThrModuleId2 = DIP_A_IMGI;
            break;
        case ENUM_DIP_DMA_IMGBI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_IMGBI;
            break;
        case ENUM_DIP_DMA_IMGCI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_IMGCI;
            break;
        case ENUM_DIP_DMA_UFDI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_UFDI;
            break;
        case ENUM_DIP_DMA_LCEI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_LCEI;
            break;
        case ENUM_DIP_DMA_VIPI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_VIPI;
            break;
        case ENUM_DIP_DMA_VIP2I:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_VIP2I;
            break;
        case ENUM_DIP_DMA_VIP3I:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_VIP3I;
            break;
        case ENUM_DIP_DMA_DMGI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_DMGI;
            break;
        case ENUM_DIP_DMA_DEPI:
            cqThrModuleId = DIP_A_VECTICAL_FLIP;
            cqThrModuleId2 = DIP_A_DEPI;
            break;
        //out-dma
        case ENUM_DIP_DMA_PAK2O:
            cqThrModuleId = DIP_A_PAK2O;
            cqThrModuleId2 = DIP_A_PAK2O_CROP;
            break;
        case ENUM_DIP_DMA_IMG3O:
            cqThrModuleId = DIP_A_IMG3O;
            cqThrModuleId2 = DIP_A_IMG3O_CRSP;
            break;
        case ENUM_DIP_DMA_IMG3BO:
            cqThrModuleId = DIP_A_IMG3BO;
            cqThrModuleId2 = DIP_A_IMG3BO_CRSP;
            break;
        case ENUM_DIP_DMA_IMG3CO:
            cqThrModuleId = DIP_A_IMG3CO;
            cqThrModuleId2 = DIP_A_IMG3CO_CRSP;
            break;
        case ENUM_DIP_DMA_FEO:
            cqThrModuleId = DIP_A_FEO;
            cqThrModuleId2 = 0xff;
            break;
        case ENUM_DIP_DMA_IMG2O:
            cqThrModuleId = DIP_A_IMG2O;
            cqThrModuleId2 = DIP_A_IMG2O_CRSP;
            break;
        case ENUM_DIP_DMA_IMG2BO:
            cqThrModuleId = DIP_A_IMG2BO;
            cqThrModuleId2 = DIP_A_IMG2BO_CRSP;
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

    DIP_X_REG_CTL_YUV_EN *pYuvEn;
    DIP_X_REG_CTL_YUV2_EN *pYuv2En;
    DIP_X_REG_CTL_RGB_EN *pRgbEn;
    DIP_X_REG_CTL_RGB2_EN *pRgb2En;
    DIP_X_REG_CTL_DMA_EN *pDmaEn;
    DIP_X_REG_CTL_FMT_SEL *pFmtSel;
    DIP_X_REG_CTL_PATH_SEL *pPathSel;
    DIP_X_REG_CTL_MISC_SEL *pMiscSel;
    DIP_X_REG_CTL_TDR_CTL *pTdrCtl;
    DIP_X_REG_NBC_ANR_CON1 *pNBCAnrCon;
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
    pTdri->drvinfo.VirtRegPa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegPhyAddr();
    pTdri->drvinfo.VirtRegVa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegVirAddr();
    pTdri->drvinfo.dupCqIdx = this->dupCqIdx;
    pTdri->drvinfo.burstCqIdx = this->burstQueIdx;
    pTdri->drvinfo.cqIdx = (E_ISP_DIP_CQ)this->dipTh;
    pTdri->drvinfo.debugRegDump = 0;

    pTdri->drvinfo.regCount = this->regCount;
    ::memcpy(pTdri->drvinfo.ReadAddrList, this->pReadAddrList, this->regCount*sizeof(int));

    // DIP settings for GCE cmd
    if (this->isSecureFra == 1)
    {
        pTdri->drvinfo.dip_ctl_yuv_en = this->isp_top_ctl.YUV_EN.Raw;
        pTdri->drvinfo.dip_ctl_yuv2_en = this->isp_top_ctl.YUV2_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb_en = this->isp_top_ctl.RGB_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb2_en = this->isp_top_ctl.RGB2_EN.Raw;
        pTdri->drvinfo.dip_ctl_dma_en = this->isp_top_ctl.DMA_EN.Raw;
        pTdri->drvinfo.dip_ctl_fmt_sel = this->isp_top_ctl.FMT_SEL.Raw;
        pTdri->drvinfo.dip_ctl_path_sel = this->isp_top_ctl.PATH_SEL.Raw;
        pTdri->drvinfo.dip_ctl_misc_sel = this->isp_top_ctl.MISC_EN.Raw;
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
        pTdri->drvinfo.dip_pak2o_base_addr = this->DMAPak2o.dma_cfg.memBuf.base_pAddr + this->DMAPak2o.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_pak2o_size[0] = this->DMAPak2o.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_pak2o_size[1] = pTdri->drvinfo.dip_pak2o_size[2] = 0;
        if (this->DMAImgi.dma_cfg.secureTag != 0)
        {
            pTdri->drvinfo.dip_imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr;
            if (this->isp_top_ctl.RGB_EN.Bits.BNR2_EN == 1) // BPCI table
            {
                pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgbi.dma_cfg.memBuf.base_pAddr;
                pTdri->drvinfo.dip_imgbi_base_vaddr = this->DMAImgbi.dma_cfg.memBuf.base_vAddr;
                //pTdri->drvinfo.dip_imgi_size = this->DMAImgi.dma_cfg.memBuf.size;
                //pTdri->drvinfo.dip_imgbi_size = this->DMAImgbi.dma_cfg.memBuf.size;
            }
            else
            {
                pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgi.dma_cfg.memBuf.size;
                pTdri->drvinfo.dip_imgbi_base_vaddr = 0;
                //pTdri->drvinfo.dip_imgi_size = this->DMAImgi.dma_cfg.memBuf.size + this->DMAImgbi.dma_cfg.memBuf.size + this->DMAImgci.dma_cfg.memBuf.size;
                //pTdri->drvinfo.dip_imgbi_size = pTdri->drvinfo.dip_imgi_size;
            }
            if (this->isp_top_ctl.RGB_EN.Bits.LSC2_EN == 1) // Shading table
            {
                pTdri->drvinfo.dip_imgci_base_addr = this->DMAImgci.dma_cfg.memBuf.base_pAddr;
                pTdri->drvinfo.dip_imgci_base_vaddr = this->DMAImgci.dma_cfg.memBuf.base_vAddr;
                //pTdri->drvinfo.dip_imgci_size = this->DMAImgci.dma_cfg.memBuf.size;
            }
            else
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
        pTdri->drvinfo.dip_pak2o_secure_tag = this->DMAPak2o.dma_cfg.secureTag;
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
        pTdri->drvinfo.dip_ctl_fmt_sel = this->isp_top_ctl.FMT_SEL.Raw;
        pTdri->drvinfo.dip_ctl_path_sel = this->isp_top_ctl.PATH_SEL.Raw;
        pTdri->drvinfo.dip_ctl_misc_sel = this->isp_top_ctl.MISC_EN.Raw;
        pTdri->drvinfo.dip_img2o_base_addr = this->DMAImg2o.dma_cfg.memBuf.base_pAddr + this->DMAImg2o.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img2bo_base_addr = this->DMAImg2bo.dma_cfg.memBuf.base_pAddr + this->DMAImg2bo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img3o_base_addr = this->DMAImg3o.dma_cfg.memBuf.base_pAddr + this->DMAImg3o.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img3bo_base_addr = this->DMAImg3bo.dma_cfg.memBuf.base_pAddr + this->DMAImg3bo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_img3co_base_addr = this->DMAImg3co.dma_cfg.memBuf.base_pAddr + this->DMAImg3co.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_feo_base_addr = this->DMAFeo.dma_cfg.memBuf.base_pAddr + this->DMAFeo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_pak2o_base_addr = this->DMAPak2o.dma_cfg.memBuf.base_pAddr + this->DMAPak2o.dma_cfg.memBuf.ofst_addr;
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
        if (this->isp_top_ctl.RGB_EN.Bits.UFD_EN == 1)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_1 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx1iPa = SmxPaddr;
            pTdri->drvinfo.smx1iOft = SmxOft;
        }
        if (this->isp_top_ctl.RGB_EN.Bits.MDPCROP2_EN == 1)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_4 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx4iPa = SmxPaddr;
            pTdri->drvinfo.smx4iOft = SmxOft;
        }
        if (this->isp_top_ctl.YUV_EN.Bits.NBC2_EN == 1)
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_2 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx2iPa = SmxPaddr;
            pTdri->drvinfo.smx2iOft = SmxOft;
        }
        if ((this->isp_top_ctl.YUV_EN.Bits.MDPCROP_EN == 1) || (this->isp_top_ctl.YUV_EN.Bits.NR3D_EN == 1))
        {
            m_pIspDrvShell->m_pDipRingBuf->getSMXIOBuffer((E_ISP_DIP_CQ)this->dipTh, DIP_DMA_SMXI_3 ,SmxVaddr,SmxPaddr,SmxOft);
            pTdri->drvinfo.smx3iPa = SmxPaddr;
            pTdri->drvinfo.smx3iOft = SmxOft;
        }
    }

    DIP_FUNC_DBG("VirCqPa(0x%lx),dupCqIdx(%d)",pTdri->drvinfo.DesCqPa,pTdri->drvinfo.dupCqIdx);

    if (this->tdr_ctl&DIP_X_REG_CTL_TDR_RN)
    {
        //TPIPE Mode
        // TODO: hungwen modify
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_CTL,this->tdr_ctl);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TILE,this->tdr_tpipe);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TCM_EN,this->tdr_tcm_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TCM2_EN,this->tdr_tcm2_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TCM3_EN,this->tdr_tcm3_en);
    }
    else
    {
        //Frame Mode
        // TODO: hungwen modify
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_CTL,0x0);
        m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x2;
        return ret;
    }

    //
    pYuvEn = (DIP_X_REG_CTL_YUV_EN*)&(this->isp_top_ctl.YUV_EN);
    pYuv2En = (DIP_X_REG_CTL_YUV2_EN*)&(this->isp_top_ctl.YUV2_EN);
    pRgbEn = (DIP_X_REG_CTL_RGB_EN*)&(this->isp_top_ctl.RGB_EN);
    pRgb2En = (DIP_X_REG_CTL_RGB2_EN*)&(this->isp_top_ctl.RGB2_EN);
    pDmaEn = (DIP_X_REG_CTL_DMA_EN*)&(this->isp_top_ctl.DMA_EN);
    pFmtSel = (DIP_X_REG_CTL_FMT_SEL*)&(this->isp_top_ctl.FMT_SEL);
    pPathSel = (DIP_X_REG_CTL_PATH_SEL*)&(this->isp_top_ctl.PATH_SEL);
    pMiscSel = (DIP_X_REG_CTL_MISC_SEL*)&(this->isp_top_ctl.MISC_EN);
    pTdrCtl = (DIP_X_REG_CTL_TDR_CTL*)&(this->tdr_ctl);
    if(this->isApplyTuning==MTRUE) {
        tuningIspReg = this->pTuningIspReg;
        pNBCAnrCon = (DIP_X_REG_NBC_ANR_CON1*)&(this->pTuningIspReg->DIP_X_NBC_ANR_CON1);
    } else {
        memset((void*)&tmpTuningIspReg, 0x00, sizeof(dip_x_reg_t));
        tuningIspReg = &tmpTuningIspReg;
    }
    DIP_FUNC_DBG("pYuvEn(0x%lx)",(unsigned long)pYuvEn);
    //
    pTdri->top.pixel_id = pFmtSel->Bits.PIX_ID;
    pTdri->top.cam_in_fmt = pFmtSel->Bits.IMGI_FMT;
    pTdri->top.ctl_extension_en = pTdrCtl->Bits.CTL_EXTENSION_EN;
    pTdri->top.fg_mode = pFmtSel->Bits.FG_MODE;
    pTdri->top.ufo_imgi_en = pRgbEn->Bits.UFD_EN;
    pTdri->top.ufdi_fmt = pFmtSel->Bits.UFDI_FMT;
    pTdri->top.vipi_fmt = pFmtSel->Bits.VIPI_FMT;
    pTdri->top.img3o_fmt = pFmtSel->Bits.IMG3O_FMT;
    pTdri->top.img2o_fmt = pFmtSel->Bits.IMG2O_FMT;
    pTdri->top.pak2_fmt = pMiscSel->Bits.PAK2_FMT;
    pTdri->top.adl_en = 0;
    pTdri->top.imgi_en = pDmaEn->Bits.IMGI_EN;
    pTdri->top.imgbi_en = pDmaEn->Bits.IMGBI_EN;
    pTdri->top.imgci_en = pDmaEn->Bits.IMGCI_EN;
    pTdri->top.ufdi_en = pDmaEn->Bits.UFDI_EN;
    pTdri->top.unp_en = pRgbEn->Bits.UNP_EN;
    pTdri->top.ufd_en = pRgbEn->Bits.UFD_EN;
    pTdri->top.ufd_crop_en = 0; // Paul will remove it later.
    pTdri->top.bnr_en = pRgbEn->Bits.BNR2_EN;
    pTdri->top.lsc2_en = pRgbEn->Bits.LSC2_EN;
    pTdri->top.rcp2_en = pRgbEn->Bits.RCP2_EN;
    pTdri->top.pak2_en = pRgbEn->Bits.PAK2_EN;
    pTdri->top.c24_en = pYuvEn->Bits.C24_EN;
    pTdri->top.sl2_en = pRgbEn->Bits.SL2_EN;
    pTdri->top.rnr_en = pRgbEn->Bits.RNR_EN;
    pTdri->top.udm_en = pRgbEn->Bits.UDM_EN;
    pTdri->top.vipi_en = pDmaEn->Bits.VIPI_EN;
    pTdri->top.vip2i_en = pDmaEn->Bits.VIP2I_EN;
    pTdri->top.vip3i_en = pDmaEn->Bits.VIP3I_EN;
    pTdri->top.pak2o_en = pDmaEn->Bits.PAK2O_EN;
    pTdri->top.g2c_en = pYuvEn->Bits.G2C_EN;
    pTdri->top.c42_en = pYuvEn->Bits.C42_EN;
    pTdri->top.sl2b_en = pYuvEn->Bits.SL2B_EN;
    pTdri->top.nbc_en = pYuvEn->Bits.NBC_EN;
    pTdri->top.dmgi_en = pDmaEn->Bits.DMGI_EN;
    pTdri->top.depi_en = pDmaEn->Bits.DEPI_EN;
    pTdri->top.nbc2_en = pYuvEn->Bits.NBC2_EN;
    pTdri->top.srz1_en = pYuvEn->Bits.SRZ1_EN;
    pTdri->top.mix1_en = pYuvEn->Bits.MIX1_EN;
    pTdri->top.srz2_en = pYuvEn->Bits.SRZ2_EN;
    pTdri->top.mix2_en = pYuvEn->Bits.MIX2_EN;
    pTdri->top.sl2c_en = pYuvEn->Bits.SL2C_EN;
    pTdri->top.sl2d_en = pYuvEn->Bits.SL2D_EN;
    pTdri->top.sl2e_en = pYuvEn->Bits.SL2E_EN;
    pTdri->top.sl2g_en = pRgbEn->Bits.SL2G_EN;
    pTdri->top.sl2h_en = pRgbEn->Bits.SL2H_EN;
    pTdri->top.sl2i_en = pYuv2En->Bits.SL2I_EN;
    pTdri->top.hfg_en = pYuv2En->Bits.HFG_EN;
    // TODO: ndg is tuning module?
    pTdri->top.ndg_en = pYuv2En->Bits.NDG_EN;
    pTdri->top.ndg2_en = pYuv2En->Bits.NDG2_EN;
    pTdri->top.seee_en = pYuvEn->Bits.SEEE_EN;
    pTdri->top.lcei_en = pDmaEn->Bits.LCEI_EN;
    pTdri->top.lce_en = pRgbEn->Bits.LCE_EN;
    pTdri->top.mix3_en = pYuvEn->Bits.MIX3_EN;
    pTdri->top.mix4_en = pYuv2En->Bits.MIX4_EN;
    pTdri->top.crz_en = pYuvEn->Bits.CRZ_EN;
    pTdri->top.img2o_en = pDmaEn->Bits.IMG2O_EN;
    pTdri->top.img2bo_en = pDmaEn->Bits.IMG2BO_EN;
    pTdri->top.fe_en = pYuvEn->Bits.FE_EN;
    pTdri->top.feo_en = pDmaEn->Bits.FEO_EN;
    pTdri->top.c02_en = pYuvEn->Bits.C02_EN;
    pTdri->top.c02b_en = pYuvEn->Bits.C02B_EN;
    pTdri->top.nr3d_en = pYuvEn->Bits.NR3D_EN;
    pTdri->top.color_en = pYuvEn->Bits.COLOR_EN;
    pTdri->top.crsp_en = pYuvEn->Bits.CRSP_EN;
    pTdri->top.img3o_en = pDmaEn->Bits.IMG3O_EN;
    pTdri->top.img3bo_en = pDmaEn->Bits.IMG3BO_EN;
    pTdri->top.img3co_en = pDmaEn->Bits.IMG3CO_EN;
    pTdri->top.c24b_en = pYuvEn->Bits.C24B_EN;
    pTdri->top.mdp_crop_en = pYuvEn->Bits.MDPCROP_EN;
    pTdri->top.mdp_crop2_en = pRgbEn->Bits.MDPCROP2_EN;
    pTdri->top.plnr1_en = pYuvEn->Bits.PLNR1_EN;
    pTdri->top.plnr2_en = pYuvEn->Bits.PLNR2_EN;
    pTdri->top.plnw1_en = pYuvEn->Bits.PLNW1_EN;
    pTdri->top.plnw2_en = pYuvEn->Bits.PLNW2_EN;
    // TODO: adbs2 is tuning module?
    pTdri->top.adbs2_en = pRgb2En->Bits.ADBS2_EN;
    pTdri->top.dbs2_en = pRgbEn->Bits.DBS2_EN;
    pTdri->top.obc2_en = pRgbEn->Bits.OBC2_EN;
    pTdri->top.rmm2_en = pRgbEn->Bits.RMM2_EN;
    pTdri->top.rmg2_en = pRgbEn->Bits.RMG2_EN;
    pTdri->top.gdr1_en = pRgbEn->Bits.GDR1_EN;
    pTdri->top.gdr2_en = pRgbEn->Bits.GDR2_EN;
    pTdri->top.bnr2_en = pRgbEn->Bits.BNR2_EN;
    pTdri->top.fm_en = pYuv2En->Bits.FM_EN;
    pTdri->top.srz3_en = pYuv2En->Bits.SRZ3_EN;
    pTdri->top.srz4_en = pYuv2En->Bits.SRZ4_EN;
    pTdri->top.g2g2_en = pRgbEn->Bits.G2G2_EN;
    pTdri->top.ggm2_en = pRgbEn->Bits.GGM2_EN;
    // TODO: wsync and wshift is tuning module?
    pTdri->top.wsync_en = pRgbEn->Bits.WSYNC_EN;
    pTdri->top.wshift_en = pRgbEn->Bits.WSHIFT_EN;
    pTdri->top.interlace_mode = 0; //INTERLACE_MODE
    pTdri->top.pgn_sel = pPathSel->Bits.PGN_SEL;
    pTdri->top.g2g_sel = pPathSel->Bits.G2G_SEL;
    pTdri->top.g2c_sel = pPathSel->Bits.G2C_SEL;
    pTdri->top.srz1_sel = pPathSel->Bits.SRZ1_SEL;
    pTdri->top.mix1_sel = pPathSel->Bits.MIX1_SEL;
    pTdri->top.crz_sel = pPathSel->Bits.CRZ_SEL;
    pTdri->top.nr3d_sel = pPathSel->Bits.NR3D_SEL;
    pTdri->top.fe_sel = pPathSel->Bits.FE_SEL;
    pTdri->top.mdp_sel = pPathSel->Bits.MDP_SEL;
    pTdri->top.pca_en = pYuvEn->Bits.PCA_EN;
    pTdri->top.pgn_en = pRgbEn->Bits.PGN_EN;
    pTdri->top.g2g_en = pRgbEn->Bits.G2G_EN;
    pTdri->top.flc_en = pRgbEn->Bits.FLC_EN;
    pTdri->top.flc2_en = pRgbEn->Bits.FLC2_EN;
    pTdri->top.ggm_en = pRgbEn->Bits.GGM_EN;
    // TODO: dcpn2 and cpn2 is tuning module?
    pTdri->top.dcpn2_en = pRgb2En->Bits.DCPN2_EN;
    pTdri->top.cpn2_en = pRgb2En->Bits.CPN2_EN;
    pTdri->top.nbc_sel = pPathSel->Bits.NBC_SEL;
    pTdri->top.rcp2_sel = pPathSel->Bits.RCP2_SEL;
    // TODO: Pak2o sel ?
    pTdri->top.pak2o_sel = pMiscSel->Bits.PAK2O_SEL;
    pTdri->top.crsp_sel = pPathSel->Bits.CRSP_SEL;
 
    pTdri->top.imgi_sel = pPathSel->Bits.IMGI_SEL;
    pTdri->top.ggm_sel = pPathSel->Bits.GGM_SEL;
    pTdri->top.wpe_sel = pPathSel->Bits.WPE_SEL;
    pTdri->top.feo_sel = pPathSel->Bits.FEO_SEL;
    pTdri->top.g2g2_sel = pPathSel->Bits.G2G2_SEL;
    pTdri->top.nbc_gmap_ltm_mode = pMiscSel->Bits.NBC_GMAP_LTM_MODE;
	pTdri->top.wuv_mode = pMiscSel->Bits.WUV_MODE;
    pTdri->top.pakg2_en = pRgbEn->Bits.PAKG2_EN;

    pTdri->top.smx1_en = pRgbEn->Bits.SMX1_EN;
    pTdri->top.smx1i_en = pDmaEn->Bits.SMX1I_EN;
    pTdri->top.smx1o_en = pDmaEn->Bits.SMX1O_EN;
    pTdri->top.smx2_en = pYuv2En->Bits.SMX2_EN;
    pTdri->top.smx2i_en = pDmaEn->Bits.SMX2I_EN;
    pTdri->top.smx2o_en = pDmaEn->Bits.SMX2O_EN;
    pTdri->top.smx3_en = pYuv2En->Bits.SMX3_EN;
    pTdri->top.smx3i_en = pDmaEn->Bits.SMX3I_EN;
    pTdri->top.smx3o_en = pDmaEn->Bits.SMX3O_EN;
    pTdri->top.smx4_en = pRgb2En->Bits.SMX4_EN;
    pTdri->top.smx4i_en = pDmaEn->Bits.SMX4I_EN;
    pTdri->top.smx4o_en = pDmaEn->Bits.SMX4O_EN;

    //
    pTdri->sw.log_en = 1;
    pTdri->sw.src_width = DMAImgi.dma_cfg.size.w;
    pTdri->sw.src_height = DMAImgi.dma_cfg.size.h;
    pTdri->sw.tpipe_sel_mode = 0x1; /* Paul suggested setting 1 for this */
    pTdri->sw.tpipe_irq_mode = TPIPE_IRQ_TPIPE;
    pTdri->sw.tpipe_width = MAX_TPIPE_WIDTH;
    pTdri->sw.tpipe_height = MAX_TPIPE_HEIGHT;
    // TODO: how to handel the ufd ??
    if (pUfdParam != NULL)
    {
        pTdri->ufd.ufd_bs2_au_start = pUfdParam->UFDG_BS_AU_START;//tuningIspReg->DIP_X_UFDG_BS_AU_CON.Bits.UFDG_BS_AU_START;
        pTdri->ufd.ufd_bond_mode = pUfdParam->UFDG_BOND_MODE;//tuningIspReg->DIP_X_UFDG_CON.Bits.UFDG_BOND_MODE;
    }
    else
    {
    	DIP_FUNC_DBG("pUfdParam is NULL!");
        pTdri->ufd.ufd_bs2_au_start = 0;
        pTdri->ufd.ufd_bond_mode = 0;
    }
    pTdri->ufd.ufd_sel = (pFmtSel->Bits.FG_MODE==1)?0:1;//tuningIspReg->DIP_X_UFDG_CON.Bits.UFOD_SEL;
    //
    pTdri->imgi.imgi_v_flip_en = DMAImgi.dma_cfg.v_flip_en;
    pTdri->imgi.imgi_stride = DMAImgi.dma_cfg.size.stride;
    // TODO: how to handel imgbi offset ?/
    pTdri->imgbi.imgbi_v_flip_en = DMAImgbi.dma_cfg.v_flip_en;
    pTdri->imgbi.imgbi_offset = DMAImgbi.dma_cfg.offset.x;
    pTdri->imgbi.imgbi_xsize = DMAImgbi.dma_cfg.size.xsize - 1;
    pTdri->imgbi.imgbi_ysize = DMAImgbi.dma_cfg.size.h - 1;
    pTdri->imgbi.imgbi_stride = DMAImgbi.dma_cfg.size.stride;
    //
    pTdri->imgci.imgci_v_flip_en = DMAImgci.dma_cfg.v_flip_en;
    pTdri->imgci.imgci_stride = DMAImgci.dma_cfg.size.stride;
    //
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
    //
    pTdri->bnr.bpc_en = tuningIspReg->DIP_X_BNR2_BPC_CON.Bits.BPC_EN;
    pTdri->bnr.bpc_tbl_en = tuningIspReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN;
    //
    pTdri->rmg.rmg_ihdr_en = tuningIspReg->DIP_X_RMG2_HDR_CFG.Bits.RMG_IHDR_EN;
    pTdri->rmg.rmg_zhdr_en = tuningIspReg->DIP_X_RMG2_HDR_CFG.Bits.RMG_ZHDR_EN;
    //
    pTdri->lsc2.extend_coef_mode= tuningIspReg->DIP_X_LSC2_CTL1.Bits.LSC_EXTEND_COEF_MODE;
    pTdri->lsc2.sdblk_width = tuningIspReg->DIP_X_LSC2_CTL2.Bits.LSC_SDBLK_WIDTH;
    pTdri->lsc2.sdblk_xnum = tuningIspReg->DIP_X_LSC2_CTL2.Bits.LSC_SDBLK_XNUM;
    pTdri->lsc2.sdblk_last_width = tuningIspReg->DIP_X_LSC2_LBLOCK.Bits.LSC_SDBLK_lWIDTH;
    pTdri->lsc2.sdblk_height = tuningIspReg->DIP_X_LSC2_CTL3.Bits.LSC_SDBLK_HEIGHT;
    pTdri->lsc2.sdblk_ynum = tuningIspReg->DIP_X_LSC2_CTL3.Bits.LSC_SDBLK_YNUM;
    pTdri->lsc2.sdblk_last_height = tuningIspReg->DIP_X_LSC2_LBLOCK.Bits.LSC_SDBLK_lHEIGHT;
    //
    pTdri->sl2.sl2_hrz_comp = tuningIspReg->DIP_X_SL2_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2.sl2_vrz_comp = tuningIspReg->DIP_X_SL2_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->udm.bayer_bypass = tuningIspReg->DIP_X_UDM_INTP_CRS.Bits.UDM_BYP;
    //
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
    //
    if(pTdri->top.vip2i_en)
    {
        pTdri->vip2i.vip2i_v_flip_en = DMAVip2i.dma_cfg.v_flip_en;
        pTdri->vip2i.vip2i_xsize = DMAVip2i.dma_cfg.size.xsize - 1;
        pTdri->vip2i.vip2i_ysize = DMAVip2i.dma_cfg.size.h - 1;
        pTdri->vip2i.vip2i_stride = DMAVip2i.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->vip2i, 0x00, sizeof(ISP_TPIPE_CONFIG_VIP2I_STRUCT));
    }
    //
    if(pTdri->top.vip3i_en)
    {
        pTdri->vip3i.vip3i_v_flip_en = DMAVip3i.dma_cfg.v_flip_en;
        pTdri->vip3i.vip3i_xsize = DMAVip3i.dma_cfg.size.xsize - 1;
        pTdri->vip3i.vip3i_ysize = DMAVip3i.dma_cfg.size.h - 1;
        pTdri->vip3i.vip3i_stride = DMAVip3i.dma_cfg.size.stride;
    }
    else
    {
        memset((void*)&pTdri->vip3i, 0x00, sizeof(ISP_TPIPE_CONFIG_VIP3I_STRUCT));
    }
    //
    //pTdri->mfb.bld_deblock_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_LL_DB_EN;
    //pTdri->mfb.bld_brz_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_LL_BRZ_EN;
    //pTdri->mfb.bld_mbd_wt_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_MBD_WT_EN;
    //
    if(pTdri->top.pak2o_en)
    {
        pTdri->pak2o.pak2o_stride = DMAPak2o.dma_cfg.size.stride;
        pTdri->pak2o.pak2o_xoffset = DMAPak2o.dma_cfg.offset.x;
        pTdri->pak2o.pak2o_yoffset = DMAPak2o.dma_cfg.offset.y;
        pTdri->pak2o.pak2o_xsize = DMAPak2o.dma_cfg.size.xsize - 1;
        pTdri->pak2o.pak2o_ysize = DMAPak2o.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->pak2o, 0x00, sizeof(ISP_TPIPE_CONFIG_PAK2O_STRUCT));
    }

#if 0
    if(pTdri->top.mfbo_en)
    {
        pTdri->mfbo.mfbo_stride = DMAMfbo.dma_cfg.size.stride;
        pTdri->mfbo.mfbo_xoffset = DMAMfbo.dma_cfg.offset.x;
        pTdri->mfbo.mfbo_yoffset = DMAMfbo.dma_cfg.offset.y;
        pTdri->mfbo.mfbo_xsize = DMAMfbo.dma_cfg.size.xsize - 1;
        pTdri->mfbo.mfbo_ysize = DMAMfbo.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->mfbo, 0x00, sizeof(ISP_TPIPE_CONFIG_MFBO_STRUCT));
    }
#endif
    //
    pTdri->g2c.g2c_shade_en = tuningIspReg->DIP_X_G2C_SHADE_CON_1.Bits.G2C_SHADE_EN;
    pTdri->g2c.g2c_shade_xmid = tuningIspReg->DIP_X_G2C_SHADE_TAR.Bits.G2C_SHADE_XMID;
    pTdri->g2c.g2c_shade_ymid = tuningIspReg->DIP_X_G2C_SHADE_TAR.Bits.G2C_SHADE_YMID;
    pTdri->g2c.g2c_shade_var = tuningIspReg->DIP_X_G2C_SHADE_CON_1.Bits.G2C_SHADE_VAR;
    //
    pTdri->sl2b.sl2b_hrz_comp = tuningIspReg->DIP_X_SL2B_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2b.sl2b_vrz_comp = tuningIspReg->DIP_X_SL2B_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->nbc.anr_eny = tuningIspReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_ENY;
    pTdri->nbc.anr_enc = tuningIspReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_ENC;
    pTdri->nbc.anr_ltm_link = tuningIspReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_LTM_LINK;
    //
    pTdri->nbc2.anr2_eny = tuningIspReg->DIP_X_NBC2_ANR2_CON1.Bits.NBC2_ANR2_ENY;
    pTdri->nbc2.anr2_enc = tuningIspReg->DIP_X_NBC2_ANR2_CON1.Bits.NBC2_ANR2_ENC;
    pTdri->nbc2.anr2_scale_mode = tuningIspReg->DIP_X_NBC2_ANR2_CON1.Bits.NBC2_ANR2_SCALE_MODE;
    pTdri->nbc2.anr2_mode = tuningIspReg->DIP_X_NBC2_ANR2_CON1.Bits.NBC2_ANR2_MODE;
    pTdri->nbc2.anr2_bok_mode = tuningIspReg->DIP_X_NBC2_BOK_CON.Bits.NBC2_BOK_MODE;
    pTdri->nbc2.anr2_bok_pf_en = tuningIspReg->DIP_X_NBC2_BOK_CON.Bits.NBC2_BOK_PF_EN;
    pTdri->nbc2.abf_en = tuningIspReg->DIP_X_NBC2_ABF_CON1.Bits.NBC2_ABF_EN;
    //
    pTdri->sl2c.sl2c_hrz_comp = tuningIspReg->DIP_X_SL2C_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2c.sl2c_vrz_comp = tuningIspReg->DIP_X_SL2C_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->sl2d.sl2d_hrz_comp = tuningIspReg->DIP_X_SL2D_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2d.sl2d_vrz_comp = tuningIspReg->DIP_X_SL2D_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->sl2e.sl2e_hrz_comp = tuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2e.sl2e_vrz_comp = tuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->sl2g.sl2g_hrz_comp = tuningIspReg->DIP_X_SL2G_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2g.sl2g_vrz_comp = tuningIspReg->DIP_X_SL2G_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->sl2h.sl2h_hrz_comp = tuningIspReg->DIP_X_SL2H_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2h.sl2h_vrz_comp = tuningIspReg->DIP_X_SL2H_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->sl2i.sl2i_hrz_comp = tuningIspReg->DIP_X_SL2I_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2i.sl2i_vrz_comp = tuningIspReg->DIP_X_SL2I_RZ.Bits.SL2_VRZ_COMP;
    //
    //pTdri->sl2k.sl2k_hrz_comp = tuningIspReg->DIP_X_SL2K_RZ.Bits.SL2_HRZ_COMP;
    //pTdri->sl2k.sl2k_vrz_comp = tuningIspReg->DIP_X_SL2K_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->hfg.non_block_base = 0;   // need check with DE (It's cmodel parameter, don't care.)
    //
    pTdri->ndg.non_block_base = 0;
    pTdri->ndg2.non_block_base = 0;
    //
    // TODO: how to handel seee ??
    pTdri->seee.se_edge = tuningIspReg->DIP_X_SEEE_TOP_CTRL.Bits.SEEE_OUT_EDGE_SEL;

    //
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

    //LCE25_SLM_SIZE
    pTdri->lce.lce_lc_tone = tuningIspReg->DIP_X_LCE25_TM_PARA0.Bits.LCE_LC_TONE;
    pTdri->lce.lce_bc_mag_kubnx = tuningIspReg->DIP_X_LCE25_ZR.Bits.LCE_BCMK_X;
    pTdri->lce.lce_slm_width = tuningIspReg->DIP_X_LCE25_SLM_SIZE.Bits.LCE_SLM_WD;
    pTdri->lce.lce_bc_mag_kubny = tuningIspReg->DIP_X_LCE25_ZR.Bits.LCE_BCMK_Y;
    pTdri->lce.lce_slm_height = tuningIspReg->DIP_X_LCE25_SLM_SIZE.Bits.LCE_SLM_HT;
    pTdri->lce.lce_full_slm_width = tuningIspReg->DIP_X_LCE25_SLM.Bits.LCE_TPIPE_SLM_WD;
    pTdri->lce.lce_full_slm_height = tuningIspReg->DIP_X_LCE25_SLM.Bits.LCE_TPIPE_SLM_HT;
    pTdri->lce.lce_full_xoff = tuningIspReg->DIP_X_LCE25_OFFSET.Bits.LCE_TPIPE_OFFSET_X;
    pTdri->lce.lce_full_yoff = tuningIspReg->DIP_X_LCE25_OFFSET.Bits.LCE_TPIPE_OFFSET_Y;
    pTdri->lce.lce_full_out_height = tuningIspReg->DIP_X_LCE25_OUT.Bits.LCE_TPIPE_OUT_HT;
    //
    if(pTdri->top.crz_en)
    {
        pTdri->cdrz.cdrz_input_crop_width = crzPipe.crz_crop.w;
        pTdri->cdrz.cdrz_input_crop_height = crzPipe.crz_crop.h;
        pTdri->cdrz.cdrz_output_width = crzPipe.crz_out.w;
        pTdri->cdrz.cdrz_output_height = crzPipe.crz_out.h;
        pTdri->cdrz.cdrz_luma_horizontal_integer_offset = crzPipe.crz_crop.x;
        pTdri->cdrz.cdrz_luma_horizontal_subpixel_offset = crzPipe.crz_crop.floatX;
        pTdri->cdrz.cdrz_luma_vertical_integer_offset = crzPipe.crz_crop.y;
        pTdri->cdrz.cdrz_luma_vertical_subpixel_offset = crzPipe.crz_crop.floatY;
        pTdri->cdrz.cdrz_horizontal_luma_algorithm = crzPipe.hAlgo;
        pTdri->cdrz.cdrz_vertical_luma_algorithm = crzPipe.vAlgo;
        pTdri->cdrz.cdrz_horizontal_coeff_step = crzPipe.hCoeffStep;
        pTdri->cdrz.cdrz_vertical_coeff_step = crzPipe.vCoeffStep;
    }
    else
    {
        memset((void*)&pTdri->cdrz, 0x00, sizeof(ISP_TPIPE_CONFIG_CDRZ_STRUCT));
    }
    //
    if(pTdri->top.img2o_en)
    {
        pTdri->img2o.img2o_stride = DMAImg2o.dma_cfg.size.stride;
        pTdri->img2o.img2o_xoffset = DMAImg2o.dma_cfg.offset.x;
        pTdri->img2o.img2o_yoffset = DMAImg2o.dma_cfg.offset.y;
        pTdri->img2o.img2o_xsize = DMAImg2o.dma_cfg.size.xsize - 1;
        pTdri->img2o.img2o_ysize = DMAImg2o.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->img2o, 0x00, sizeof(ISP_TPIPE_CONFIG_IMG2O_STRUCT));
    }
    //
    if(pTdri->top.img2bo_en)
    {
        pTdri->img2bo.img2bo_stride = DMAImg2bo.dma_cfg.size.stride;
        pTdri->img2bo.img2bo_xoffset = DMAImg2bo.dma_cfg.offset.x;
        pTdri->img2bo.img2bo_yoffset = DMAImg2bo.dma_cfg.offset.y;
        pTdri->img2bo.img2bo_xsize = DMAImg2bo.dma_cfg.size.xsize - 1;
        pTdri->img2bo.img2bo_ysize = DMAImg2bo.dma_cfg.size.h - 1;
    }
    else
    {
        memset((void*)&pTdri->img2bo, 0x00, sizeof(ISP_TPIPE_CONFIG_IMG2BO_STRUCT));
    }
    //
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
    //
    if(pTdri->top.srz2_en)
    {
        pTdri->srz2.srz_input_crop_width = srz2Cfg.inout_size.in_w;
        pTdri->srz2.srz_input_crop_height = srz2Cfg.inout_size.in_h;
        pTdri->srz2.srz_output_width = srz2Cfg.inout_size.out_w;
        pTdri->srz2.srz_output_height = srz2Cfg.inout_size.out_h;
        pTdri->srz2.srz_luma_horizontal_integer_offset = srz2Cfg.crop.x;
        pTdri->srz2.srz_luma_horizontal_subpixel_offset = srz2Cfg.crop.floatX;
        pTdri->srz2.srz_luma_vertical_integer_offset = srz2Cfg.crop.y;
        pTdri->srz2.srz_luma_vertical_subpixel_offset = srz2Cfg.crop.floatY;
        pTdri->srz2.srz_horizontal_coeff_step = srz2Cfg.h_step;
        pTdri->srz2.srz_vertical_coeff_step = srz2Cfg.v_step;
    }
    else
    {
        memset((void*)&pTdri->srz2, 0x00, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    }
    //
    if(pTdri->top.srz3_en)
    {
        pTdri->srz3.srz_input_crop_width = srz3Cfg.inout_size.in_w;
        pTdri->srz3.srz_input_crop_height = srz3Cfg.inout_size.in_h;
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
    //
    if(pTdri->top.srz4_en)
    {
        if ((pYuvEn->Bits.NBC_EN == 1) && (pNBCAnrCon->Bits.NBC_ANR_LTM_LINK == 1)) //From TC Comment
        {
            pTdri->srz4.srz_input_crop_height = srz4Cfg.crop.h;
            pTdri->srz4.srz_input_crop_width = srz4Cfg.crop.w;
        }
        else
        {
            pTdri->srz4.srz_input_crop_width = srz4Cfg.inout_size.in_w;
            pTdri->srz4.srz_input_crop_height = srz4Cfg.inout_size.in_h;
        }
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
    //
    //
    pTdri->fe.fe_mode = tuningIspReg->DIP_X_FE_CTRL1.Bits.FE_MODE;
    //
    pTdri->feo.feo_stride = DMAFeo.dma_cfg.size.stride;
    //
    pTdri->nr3d.nr3d_on_en = tuningIspReg->DIP_X_NR3D_ON_CON.Bits.NR3D_ON_EN;
    pTdri->nr3d.nr3d_on_xoffset = tuningIspReg->DIP_X_NR3D_ON_OFF.Bits.NR3D_ON_OFST_X;
    pTdri->nr3d.nr3d_on_yoffset = tuningIspReg->DIP_X_NR3D_ON_OFF.Bits.NR3D_ON_OFST_Y;
    pTdri->nr3d.nr3d_on_width = tuningIspReg->DIP_X_NR3D_ON_SIZ.Bits.NR3D_ON_WD;
    pTdri->nr3d.nr3d_on_height = tuningIspReg->DIP_X_NR3D_ON_SIZ.Bits.NR3D_ON_HT;
    //
    pTdri->crsp.crsp_ystep = 4;  /* Paul suggested setting 0x04 for this that same as rome */
    pTdri->crsp.crsp_xoffset = 0x00;  /* Paul suggested setting 0x00 for this that same as rome */
    pTdri->crsp.crsp_yoffset = 0x01;  /* Paul suggested setting 0x01 for this that same as rome */
    //
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
    //
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
    //
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
    //
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
    //
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
    //
    pTdri->pca.pca_cfc_en = tuningIspReg->DIP_X_PCA_CON1.Bits.PCA_CFC_EN;
    pTdri->pca.pca_cnv_en = tuningIspReg->DIP_X_PCA_CON1.Bits.PCA_CNV_EN;

    //Smx1, TC says that you can take the smx buffer as one-dimension array.
    if(pTdri->top.smx1i_en)
    {
        pTdri->smx1i.smx1i_v_flip_en = 0;
        pTdri->smx1i.smx1i_xsize = (MAX_SMX1_BUF_SIZE - 1);
        pTdri->smx1i.smx1i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx1i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX1I_STRUCT));
    }

    if(pTdri->top.smx1o_en)
    {
        pTdri->smx1o.smx1o_xsize = (MAX_SMX1_BUF_SIZE - 1);
        pTdri->smx1o.smx1o_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx1o, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX1O_STRUCT));
    }
    //Smx2
    if(pTdri->top.smx2i_en)
    {
        pTdri->smx2i.smx2i_v_flip_en = 0;
        pTdri->smx2i.smx2i_xsize = (MAX_SMX2_BUF_SIZE - 1);
        pTdri->smx2i.smx2i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx2i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX2I_STRUCT));
    }

    if(pTdri->top.smx2o_en)
    {
        pTdri->smx2o.smx2o_xsize = (MAX_SMX2_BUF_SIZE - 1);
        pTdri->smx2o.smx2o_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx2o, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX2O_STRUCT));
    }

    //Smx3
    if(pTdri->top.smx3i_en)
    {
        pTdri->smx3i.smx3i_v_flip_en = 0;
        pTdri->smx3i.smx3i_xsize = (MAX_SMX3_BUF_SIZE - 1);
        pTdri->smx3i.smx3i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx3i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX3I_STRUCT));
    }

    if(pTdri->top.smx3o_en)
    {
        pTdri->smx3o.smx3o_xsize = (MAX_SMX3_BUF_SIZE - 1);
        pTdri->smx3o.smx3o_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx3o, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX3O_STRUCT));
    }
    //Smx4
    if(pTdri->top.smx4i_en)
    {
        pTdri->smx4i.smx4i_v_flip_en = 0;
        pTdri->smx4i.smx4i_xsize = (MAX_SMX4_BUF_SIZE - 1);
        pTdri->smx4i.smx4i_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx4i, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX4I_STRUCT));
    }

    if(pTdri->top.smx4o_en)
    {
        pTdri->smx4o.smx4o_xsize = (MAX_SMX4_BUF_SIZE - 1);
        pTdri->smx4o.smx4o_ysize = 0;
    }
    else
    {
        memset((void*)&pTdri->smx4o, 0x00, sizeof(ISP_TPIPE_CONFIG_SMX4O_STRUCT));
    }
    //TC says smxo_sel support zero, if smxo_sel is the other value, tile driver will report error.
    pTdri->smx1.smx1o_sel = 0;
    pTdri->smx2.smx2o_sel = 0;
    pTdri->smx3.smx3o_sel = 0;
    pTdri->smx4.smx4o_sel = 0;
#if 0
    pTdri->smx1.smx1o_sel = tuningIspReg->DIP_X_SMX1_CTL.Bits.SMXO_SEL;
    pTdri->smx2.smx2o_sel = tuningIspReg->DIP_X_SMX2_CTL.Bits.SMXO_SEL;
    pTdri->smx3.smx3o_sel = tuningIspReg->DIP_X_SMX3_CTL.Bits.SMXO_SEL;
    pTdri->smx4.smx4o_sel = tuningIspReg->DIP_X_SMX4_CTL.Bits.SMXO_SEL;
#endif

EXIT:
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
    pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableOft = pDipWBuf->tpipeTableOft;

    DIP_FUNC_DBG("moduleIdx(%d), pDipWBuf(0x%x), m_pIspDescript_phy(0x%x), m_pIspDescript_vir(0x%x), m_pIspVirRegAddr_pa(0x%x), m_pIspVirRegAddr_va(0x%x)",
        this->moduleIdx, pDipWBuf, pDipWBuf->m_pIspDescript_phy, pDipWBuf->m_pIspDescript_vir, pDipWBuf->m_pIspVirRegAddr_pa, pDipWBuf->m_pIspVirRegAddr_va);

    //initiate cq descriptor buffer
    for(int m=0;m<(DIP_A_MODULE_MAX - 1);m++){
        pDipWBuf->m_pIspDescript_vir->u.cmd = ISP_DRV_CQ_DUMMY_TOKEN;
        pDipWBuf->m_pIspDescript_vir->v_reg_addr = 0;
    }

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
    MINT32 ret = MTRUE;
    MBOOL crz_result = MTRUE;
    MINT32 result = MTRUE;
    MUINTPTR SmxVaddr = 0,SmxPaddr = 0;
    MUINT32 i;
    int bDCMEn;
    bDCMEn = ::property_get_int32("vendor.camera.DCM.disable", 0);
    ISP_TPIPE_CONFIG_STRUCT *pTdri = NULL;
    
    DIP_FUNC_DBG("[DIP_TOP_CTRL]moduleIdx(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d),isApplyTuning(%d)",\
        this->moduleIdx,this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx,this->isApplyTuning);

    DIP_FUNC_DBG("YUV_EN(0x%08x),YUV2_EN(0x%08x),RGB_EN(0x%08x),FMT_SEL(0x%08x),PATH_SEL(0x%08x)",\
        this->isp_top_ctl.YUV_EN.Raw,this->isp_top_ctl.YUV2_EN.Raw,this->isp_top_ctl.RGB_EN.Raw,\
        this->isp_top_ctl.FMT_SEL.Raw,this->isp_top_ctl.PATH_SEL.Raw);

    // TODO: hungwen modify
    this->pDrvDip = m_pIspDrvShell->m_pDrvDip[this->moduleIdx];
    this->pDrvDipPhy = this->m_pIspDrvShell->m_pDrvDipPhy;

    /********************************************************************************************************************/
    // set registers to hardware register directly
    //when mfb dcm is enabled, the scenario realted mfb will happen seldom hang in last tile. 
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV_DCM_DIS, 0x00000001);//kk test (read clear or write one clear??)
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV2_DCM_DIS, 0x00000008);//kk test (read clear or write one clear??)
    //Disable SMX DCM & UFD/UFDI/IMGI DCM
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_RGB_DCM_DIS, 0x02000002);
    ///DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV2_DCM_DIS, 0x00000300);
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_DMA_DCM_DIS, 0x03FC0009);

    if (bDCMEn)
    {
        DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV_DCM_DIS, 0xffffffff);
        DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV2_DCM_DIS, 0xffffffff);
        DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_RGB_DCM_DIS, 0xffffffff);
        DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_DMA_DCM_DIS, 0xffffffff);
        DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_TOP_DCM_DIS, 0xffffffff);
        DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_RGB2_DCM_DIS, 0xffffffff);
    }

    /* Move to mIspDipInitReg init setting
    DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_INT_EN, 0x0);//disable to save cpu usage
    DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_CQ_INT_EN, 0x0);//disable to save cpu usage
    DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_CQ_INT2_EN, 0x0);//disable to save cpu usage
    DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_CQ_INT3_EN, 0x0);//disable to save cpu usage
    */

    // set registers to dip thread
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_DMA_EN, this->isp_top_ctl.DMA_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_YUV_EN, this->isp_top_ctl.YUV_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_YUV2_EN, this->isp_top_ctl.YUV2_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_RGB_EN, this->isp_top_ctl.RGB_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_RGB2_EN, this->isp_top_ctl.RGB2_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_FMT_SEL, this->isp_top_ctl.FMT_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_PATH_SEL, this->isp_top_ctl.PATH_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_MISC_SEL, this->isp_top_ctl.MISC_EN.Raw);

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
            DIP_A__CTL_RGB_EN=%d,DIP_A__CTL_RGB2_EN=%d,DIP_A__CTL_DMA_EN=%d",\
            this->isp_top_ctl.YUV_EN.Raw, this->isp_top_ctl.YUV2_EN.Raw,\
            this->isp_top_ctl.RGB_EN.Raw, this->isp_top_ctl.RGB2_EN.Raw, this->isp_top_ctl.DMA_EN.Raw);
        }
    }
 
      _CQ_CONFIG_INFO_ cqcfginfo;
    cqcfginfo.moduleIdx=this->moduleIdx;
    cqcfginfo.dipTh=this->dipTh;
    cqcfginfo.dipCQ_dupIdx=this->dupCqIdx;
    cqcfginfo.dipCQ_burstIdx=this->burstQueIdx;
    cqcfginfo.dipCQ_ringIdx=this->RingBufIdx;
    cqcfginfo.isDipOnly=this->isDipOnly;
    cqcfginfo.pDipWBuf= this->m_pDipWBuf;
    pthread_mutex_lock(&mCQCfgInfoMutex);
    mFrameCQCfgInfoList.push_back(cqcfginfo);
    pthread_mutex_unlock(&mCQCfgInfoMutex);

    DIP_FUNC_DBG("cqcfginfo.pDipWBuf (0x%08x), desphy:0x%x,desvir:0x%x,virispphy:0x%x,virispvir:0x%x", cqcfginfo.pDipWBuf, cqcfginfo.pDipWBuf->m_pIspDescript_phy, cqcfginfo.pDipWBuf->m_pIspDescript_vir
        ,cqcfginfo.pDipWBuf->m_pIspVirRegAddr_pa, cqcfginfo.pDipWBuf->m_pIspVirRegAddr_va);

    
    //DIP_RAW_PIPE Domain
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].default_func!=NULL)) {
            gIspModuleMappingTableRbgEn[i].default_func((*this));
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
    if (this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_LCE)
    {
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_CTL_MISC_SEL, NBC_GMAP_LTM_MODE,0x1);
    }
    else
    {
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_CTL_MISC_SEL, NBC_GMAP_LTM_MODE,0x0);
    }

    //UFDI
    if ((this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_UFD))
    {
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_SPECIAL_FUN_EN, UFO_IMGI_EN,0x1);
    }
    else
    {
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_SPECIAL_FUN_EN, UFO_IMGI_EN,0x0);
    }


    //NR3D
    if ((!(this->isp_top_ctl.YUV_EN.Raw & DIP_X_REG_CTL_YUV_EN_NR3D)) && (this->pTuningIspReg!=NULL))
    {
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_CAM_TNR_ENG_CON, NR3D_CAM_TNR_EN, this->pTuningIspReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN);
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_MDP_TNR_TNR_ENABLE, NR3D_TNR_Y_EN, this->pTuningIspReg->DIP_X_MDP_TNR_TNR_ENABLE.Bits.NR3D_TNR_Y_EN);
    }

    //Color
    if ((!(this->isp_top_ctl.YUV_EN.Raw & DIP_X_REG_CTL_YUV_EN_COLOR)) && (this->pTuningIspReg!=NULL))
    {
        DIP_WRITE_BITS(this->pDrvDip, DIP_X_CAM_COLOR_START, COLOR_DISP_COLOR_START,this->pTuningIspReg->DIP_X_CAM_COLOR_START.Bits.COLOR_DISP_COLOR_START);
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


int DIP_ISP_PIPE::enable(void* pParam)
{
    MINT32 errSta;
    MINT32 ret = MTRUE;
    MUINT32 frameflag=0;
    DipWorkingBuffer* pDipWBuf;
    MUINT32 ref_ringbufidx=0;
    //
     _CQ_CONFIG_INFO_ cqcfginfo;
    list<_CQ_CONFIG_INFO_>::iterator it;
    pthread_mutex_lock(&mCQCfgInfoMutex);
    it = mFrameCQCfgInfoList.begin();
    cqcfginfo.moduleIdx=(*it).moduleIdx;
    cqcfginfo.moduleIdx=(*it).moduleIdx;
    cqcfginfo.dipTh=(*it).dipTh;
    cqcfginfo.dipCQ_dupIdx=(*it).dipCQ_dupIdx;
    cqcfginfo.dipCQ_burstIdx=(*it).dipCQ_burstIdx;
    cqcfginfo.dipCQ_ringIdx=(*it).dipCQ_ringIdx;
    cqcfginfo.isDipOnly=(*it).isDipOnly;
    cqcfginfo.pDipWBuf=(*it).pDipWBuf;
    mFrameCQCfgInfoList.pop_front();
    pthread_mutex_unlock(&mCQCfgInfoMutex);
    pParam;
    ref_ringbufidx=cqcfginfo.dipCQ_ringIdx;
    pDipWBuf = m_pIspDrvShell->m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)cqcfginfo.dipTh, cqcfginfo.dipCQ_ringIdx);

    this->m_pIspDrvShell->m_pDipRingBuf->skipTpipeCalculatebyIdx((E_ISP_DIP_CQ)cqcfginfo.dipTh,frameflag,ref_ringbufidx,cqcfginfo.pDipWBuf);
    cqcfginfo.pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = frameflag;

    DIP_FUNC_DBG("thiscq(%d), cqcfginfo.dipCQ_ringIdx(0x%x), frameflag(0x%x), ref_ringbufidx(%d)",cqcfginfo.dipTh, cqcfginfo.dipCQ_ringIdx, frameflag, ref_ringbufidx);

    ret=m_pIspDrvShell->m_pMdpMgr->updateDpStreamMap(frameflag,cqcfginfo.dipTh, cqcfginfo.dipCQ_ringIdx, ref_ringbufidx);

    errSta = m_pIspDrvShell->m_pMdpMgr->startMdp((*cqcfginfo.pDipWBuf->m_pMdpCfg));
    if(MDPMGR_NO_ERROR != errSta){
        DIP_FUNC_ERR("[Error]pass2 TH(%d) m_pIspDrvShell->m_pMdpMgr->startMdp fail(%d)",cqcfginfo.dipTh,errSta);
        ret = -1;
        goto EXIT;
    }

EXIT:
    DIP_FUNC_DBG("ret(%d)", ret);
    return ret;

}

int DIP_ISP_PIPE::disable( void* pParam )
{
    pParam;

EXIT:
    return 0;
}


int DIP_ISP_PIPE::write2CQ(unsigned int dipidx)
{
    MUINT32 i;

    DIP_FUNC_DBG("[DIP_TOP_CTRL]moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d)",\
        this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx);

    // TODO: hungwen modify
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx][this->dipTh][this->burstQueIdx][this->dupCqIdx]->cqAddModule(DIP_A_CTL);
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL);
    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL2); //rgb2_enable

    //DIP_RAW_PIPE Domain
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].eCqThrFunc!=DIP_A_DUMMY)) {
            // TODO: hungwen modify
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableRbgEn[i].eCqThrFunc);
        }
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

    //UFDI
    // TODO: hungwen modify
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_SPECIAL_FUN);


    //NR3D
    if (!(this->isp_top_ctl.YUV_EN.Raw & DIP_X_REG_CTL_YUV_EN_NR3D))
    {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_NR3D);
    }

    //Color
    if (!(this->isp_top_ctl.YUV_EN.Raw & DIP_X_REG_CTL_YUV_EN_COLOR))
    {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_COLOR);
    }

    //CRZ
    // TODO: hungwen modify
    if ( this->crzPipe.conf_cdrz ) {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CRZ);
    }

    //TPIPE Ctrl
    // TODO: hungwen modify
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_TDR);
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_TDR2);

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
        ret=m_pIspDrvShell->m_pMdpMgr->stopVencLink();
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
    MUINT32 based_dupCqidx=0;
    MUINT32 ref_cqidx=0;
    MUINT32 ref_dupCqidx=0;
    MUINT32 ref_burstCqidx=0;
    MUINT32 frameflag=0;
    unsigned int* originalTdriAddr=NULL;
    MUINT32 cmprule=0x0;
    MBOOL same=MFALSE;
    MDPMGR_CFG_STRUCT *pMdpCfg = pDipWBuf->m_pMdpCfg;
    //
    DIP_FUNC_DBG("[DIP_MDP_PIPE] dipCtlDmaEn(0x%x), pMdpCfg(0x%x)",\
        this->dipCtlDmaEn, pMdpCfg);

    if (this->isDipOnly == MTRUE){
        pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx = (E_ISP_DIP_CQ)this->dipTh;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx = this->burstQueIdx;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx = this->dupCqIdx;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx = this->RingBufIdx;
        pMdpCfg->drvScenario = this->drvScenario;  //kk test
        pMdpCfg->isWaitBuf = this->isWaitBuf;
        pMdpCfg->imgxoEnP2 = this->mdp_imgxo_p2_en;
        pMdpCfg->pDumpTuningData = (unsigned int *)this->pTuningIspReg; //QQ
        pMdpCfg->ispDmaEn = this->dip_dma_en;
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
    pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx = this->dupCqIdx;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx = this->RingBufIdx;
    pMdpCfg->drvScenario = this->drvScenario;  //kk test
    pMdpCfg->isWaitBuf = this->isWaitBuf;
    pMdpCfg->imgxoEnP2 = this->mdp_imgxo_p2_en;
    pMdpCfg->pDumpTuningData = (unsigned int *)this->pTuningIspReg; //QQ
    pMdpCfg->ispDmaEn = this->dip_dma_en;
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

    DIP_FUNC_DBG("[imgi]p2cq(%d),cq(%d),dupCqIdx(%d),burstQueIdx(%d),RingBufIdx(%d)",this->dipTh,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx, pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx);

    DIP_FUNC_DBG("[mdp]Fmt(0x%x),W(%d),H(%d),stride(%d,%d),VA(0x%8x),PA(0x%8x),Size(0x%8x)",pMdpCfg->mdpSrcFmt, \
                pMdpCfg->mdpSrcW, pMdpCfg->mdpSrcH, \
                pMdpCfg->mdpSrcYStride, pMdpCfg->mdpSrcYStride, \
                pMdpCfg->srcVirAddr, \
                pMdpCfg->srcPhyAddr, pMdpCfg->mdpSrcBufSize);

    // WROTO
    if(DIP_X_REG_CTL_DMA_EN_WROTO & this->dipCtlDmaEn)
    {
        pMdpCfg->dstPortCfg[ISP_MDP_DL_WROTO] = 1;
        memcpy(&pMdpCfg->dstDma[ISP_MDP_DL_WROTO], &this->wroto_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    // WDMAO
    if(DIP_X_REG_CTL_DMA_EN_WDMAO & this->dipCtlDmaEn)
    {
        pMdpCfg->dstPortCfg[ISP_MDP_DL_WDMAO] = 1;
        memcpy(&pMdpCfg->dstDma[ISP_MDP_DL_WDMAO], &this->wdmao_out, sizeof(MdpRotDMACfg));
        enPureMdpOut = MTRUE;
    }
    //VENCO
    if(DIP_X_REG_CTL_DMA_EN_VENCO & this->dipCtlDmaEn)
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
    //if(this->pTuningIspReg != NULL) {
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

    DIP_FUNC_DBG("HM flag(0x%x, 0x%x_%d), thiscq(%d_%d_%d_0x%lx), refcq(%d_%d_%d), lst(%d)",frameflag, cmprule, same, this->dipTh, this->dupCqIdx,this->burstQueIdx,\
        (unsigned long)originalTdriAddr, ref_cqidx, ref_dupCqidx, ref_burstCqidx, this->lastframe);


EXIT:
    return ret;

}

/*/////////////////////////////////////////////////////////////////////////////
    DIP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
//
MINT32
DIP_BUF_CTRL::
dequeueMdpFrameEnd( MINT32 drvScenario, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx)
{
    DIP_FUNC_DBG("DequeMdpFrameEnd +, drvScenario: %d, Cq(%d)/BurstQIdx(%d)/DupCqIdx(%d)/moduleIdx(%d)", drvScenario, dequeCq, dequeBurstQIdx, dequeDupCqIdx, moduleIdx);
    if(EP_ISP_ONLY_FRAME_MODE)
    {
        DIP_FUNC_INF("no need deque frame end in isp_only frame mode, wait p2 done here");
        MUINT32 status=0x0;
        MBOOL getDone=false;

        DIP_WAIT_IRQ_ST irq;
        irq.Clear = DIP_IRQ_CLEAR_NONE;
        irq.Status = (1<<16); //wait p2 done
        irq.Timeout = 0x1000;
        irq.UserKey = 0x0;

        DIP_FUNC_INF("enter wait irq");
        m_pIspDrvShell->m_pDrvDipPhy->waitIrq(&irq);
        DIP_FUNC_INF("get p2 done\n");

    }
    else
    {
        MDPMGR_CFG_STRUCT_s mdpCfgdata;
        //pthread_mutex_lock(&mMdpCfgDataMutex);
        // TODO: hungwen
        mdpCfgdata = (*m_pMdpMgrCfgData);
        //pthread_mutex_unlock(&mMdpCfgDataMutex);

        if(MDPMGR_NO_ERROR != m_pIspDrvShell->m_pMdpMgr->dequeueFrameEnd(mdpCfgdata)){
            DIP_FUNC_ERR("m_pIspDrvShell->m_pMdpMgr->dequeueMdpFrameEnd for dispo fail");
            return -1;
        }
    }
    return 0;
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
    MUINT32 ispDmaEn;
    MUINT32 ispDmaEnBit;
    MDPMGR_CFG_STRUCT mdpMgrCfgStructData;

    //get property for dumpHwBuf
    int dumpDipHwRegEn= ::property_get_int32("vendor.camera.dumpDipHwReg.enable", 0);

    if (pDipWBuf==NULL){
        DIP_FUNC_ERR("deque pDipWBuf is NULL!!");
        return eIspRetStatus_Failed;
    }
    m_pMdpMgrCfgData = pDipWBuf->m_pMdpCfg;
    //pthread_mutex_lock(&mMdpCfgDataMutex);
    mdpMgrCfgDataImgxoEnP2 = m_pMdpMgrCfgData->imgxoEnP2;
    ispDmaEn = (m_pMdpMgrCfgData->ispDmaEn & DEQUE_DMA_MASK);
    //pthread_mutex_unlock(&mMdpCfgDataMutex);
    DIP_FUNC_DBG("module(%d_%d_%d), mdpEnP2(0x%x)",this->m_BufdipTh, this->m_BufdupCqIdx, this->m_BufburstQueIdx,\
		mdpMgrCfgDataImgxoEnP2);
    // MDP flow
    if ((ispDmaEn & DIP_X_REG_CTL_DMA_EN_VENCO) == DIP_X_REG_CTL_DMA_EN_VENCO){
        isVencContained = MTRUE;
    }
/*
#define DIP_X_REG_CTL_DMA_EN_IMGI                   (1L<<0)
#define DIP_X_REG_CTL_DMA_EN_IMGBI                (1L<<1)
#define DIP_X_REG_CTL_DMA_EN_IMGCI                   (1L<<2)
#define DIP_X_REG_CTL_DMA_EN_UFDI                   (1L<<3)
#define DIP_X_REG_CTL_DMA_EN_VIPI                   (1L<<4)
#define DIP_X_REG_CTL_DMA_EN_VIP2I                   (1L<<5)
#define DIP_X_REG_CTL_DMA_EN_VIP3I                   (1L<<6)
#define DIP_X_REG_CTL_DMA_EN_LCEI                   (1L<<7)

#define DIP_X_REG_CTL_DMA_EN_DEPI                   (1L<<8)
#define DIP_X_REG_CTL_DMA_EN_DMGI                   (1L<<9)
#define DIP_X_REG_CTL_DMA_EN_MFBO                   (1L<<10)
#define DIP_X_REG_CTL_DMA_EN_IMG2O                   (1L<<11)

#define DIP_X_REG_CTL_DMA_EN_IMG2BO                   (1L<<12)
#define DIP_X_REG_CTL_DMA_EN_IMG3O                   (1L<<13)
#define DIP_X_REG_CTL_DMA_EN_IMG3BO                   (1L<<14)
#define DIP_X_REG_CTL_DMA_EN_IMG3CO                   (1L<<15)

#define DIP_X_REG_CTL_DMA_EN_FEO                   (1L<<16)
#define DIP_X_REG_CTL_DMA_EN_ADL2                   (1L<<17)

#define DIP_X_REG_CTL_DMA_EN_WROTO                 (1UL<<28)
#define DIP_X_REG_CTL_DMA_EN_WDMAO                 (1UL<<29)
#define DIP_X_REG_CTL_DMA_EN_JPEGO                 (1UL<<30)
#define DIP_X_REG_CTL_DMA_EN_VENCO                 (1UL<<31)

*/

    for(int i = 31 ; i >= 0 ; i--) //DIP_X_REG_CTL_DMA_EN_VENCO
    {
        isDequeMdp = MTRUE;
        ispDmaEnBit = ispDmaEn & (1 << i);
        if(ispDmaEnBit)
        {
            //
            switch(ispDmaEnBit){
                case DIP_X_REG_CTL_DMA_EN_IMGI:
                    mdpQueueIdx = MDPMGR_DEQ_SRC;
                    break;
                case DIP_X_REG_CTL_DMA_EN_WROTO:
                    mdpQueueIdx = MDPMGR_DEQ_WROT;
                    break;
                case DIP_X_REG_CTL_DMA_EN_WDMAO:
                    mdpQueueIdx = MDPMGR_DEQ_WDMA;
                    break;
                case DIP_X_REG_CTL_DMA_EN_VENCO:
                    mdpQueueIdx = MDPMGR_DEQ_VENCO;
                    break;
                case DIP_X_REG_CTL_DMA_EN_IMG2O: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIP_X_REG_CTL_DMA_EN_IMG2O){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIP_X_REG_CTL_DMA_EN_IMG3O: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIP_X_REG_CTL_DMA_EN_IMG3O){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIP_X_REG_CTL_DMA_EN_FEO: /* for compatible MDP flow */
                    if(mdpMgrCfgDataImgxoEnP2 == DIP_X_REG_CTL_DMA_EN_FEO){
                        mdpQueueIdx = MDPMGR_DEQ_IMGXO;
                    }else{
                        isDequeMdp = MFALSE;
                    }
                    break;
                case DIP_X_REG_CTL_DMA_EN_JPEGO:
                default:
                    isDequeMdp = MFALSE;
                    break;
            };

            if (dumpDipHwRegEn)
            {
                // dump hardware debug information
                IspDumpDbgLogDipPackage p2DumpPackage;
                char* pWorkingBuffer =  NULL;
                dip_x_reg_t *pIspDipReg;

                MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
                ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
                // dump difference between tpipe and hardware registers
                pIspDipReg = (dip_x_reg_t*)this->m_pIspDrvShell->m_pDrvDipPhy->getCurHWRegValues();
                pWorkingBuffer = new char[tpipeWorkingSize];
                //

                //pthread_mutex_lock(&mMdpCfgDataMutex);
                if(pWorkingBuffer!=NULL) {
                    if (pIspDipReg!=NULL) {
                        tpipe_main_platform(&m_pMdpMgrCfgData->ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspDipReg, NULL, NULL, NULL);
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

                this->m_pIspDrvShell->m_pDrvDipPhy->dumpDipHwReg(&p2DumpPackage);
                //
            }

            if(isDequeMdp==MTRUE){
                MDPMGR_RETURN_TYPE mdpRet;
                //
                DIP_FUNC_DBG("moduleIdx(%d),dipTh(%d),burstQueIdx(%d),dupCqIdx(%d)", this->m_BufmoduleIdx,this->m_BufdipTh,this->m_BufburstQueIdx,this->m_BufdupCqIdx);
                //
                //pthread_mutex_lock(&mMdpCfgDataMutex);
                //::memcpy(&mdpMgrCfgStructData, m_pMdpMgrCfgData, sizeof(MDPMGR_CFG_STRUCT));
                //pthread_mutex_unlock(&mMdpCfgDataMutex);
                //
                mdpRet = (MDPMGR_RETURN_TYPE)m_pIspDrvShell->m_pMdpMgr->dequeueBuf(mdpQueueIdx, (*m_pMdpMgrCfgData));
                DIP_FUNC_DBG("mdpRet(%d)",mdpRet);
                if(mdpRet == MDPMGR_NO_ERROR){
                    m_pMdpMgrCfgData->isWaitBuf = MTRUE;
                    ret = eIspRetStatus_Success;
                } else if(mdpRet == MDPMGR_VSS_NOT_READY){
                    ret = eIspRetStatus_VSS_NotReady;
                    break;
                } else {
                    // dump hardware debug information
                    IspDumpDbgLogDipPackage p2DumpPackage;
                    char* pWorkingBuffer =  NULL;
                    dip_x_reg_t *pIspDipReg;
                    MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
                    ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
                    // dump difference between tpipe and hardware registers
                    pIspDipReg = (dip_x_reg_t*)this->m_pIspDrvShell->m_pDrvDipPhy->getCurHWRegValues();
                    pWorkingBuffer = new char[tpipeWorkingSize];
                    //

                    //pthread_mutex_lock(&mMdpCfgDataMutex);
                    if(pWorkingBuffer!=NULL) {
                        if(pIspDipReg!=NULL) {
                            tpipe_main_platform(&m_pMdpMgrCfgData->ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspDipReg, NULL, NULL, NULL);
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

                    this->m_pIspDrvShell->m_pDrvDipPhy->dumpDipHwReg(&p2DumpPackage);
                    //
                    ret = eIspRetStatus_Failed;

                    DIP_FUNC_ERR("[Error]dequeueBuf fail mdpRet(%d)",mdpRet);
                    break;
                    //goto EXIT;
                }
            }
        }
    }

    //
EXIT:
    return ret;
}


