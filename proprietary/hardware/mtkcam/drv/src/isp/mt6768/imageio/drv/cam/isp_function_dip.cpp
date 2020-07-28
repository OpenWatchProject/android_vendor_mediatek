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
//#include "mdp_mgr.h"

#include <cutils/properties.h>  // For property_get().

//#include "m4u_lib.h"

//
//digital zoom
#define CAM_ISP_ZOOMRATIO_GAIN (100)


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


class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
//        if  (0==mIdx)
//        {
//            DIP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            DIP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};


#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 0   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 1   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT

///////////////////////////////////////
//temp for EP
#define EP_ISP_ONLY_FRAME_MODE 0
//////////////////////////////////////

#define CHECK_CTL_RAW_RGB_EN_BITS       23
#define CHECK_CTL_YUV_EN_BITS          30
#define CHECK_CTL_YUV2_EN_BITS          8


const IspTopModuleMappingTable gIspModuleMappingTableRbgEn[CHECK_CTL_RAW_RGB_EN_BITS]
 ={ {ENUM_DIP_CTL_RGB_EN_UNP , eIspModuleRaw, DIP_A_UNP,   IspDrv_B::setUnp},
    {ENUM_DIP_CTL_RGB_EN_UFD , eIspModuleRaw, DIP_A_UFD,   IspDrv_B::setUfd},
    {ENUM_DIP_CTL_RGB_EN_PGN , eIspModuleRaw, DIP_A_PGN,   IspDrv_B::setPgn},
    {ENUM_DIP_CTL_RGB_EN_SL2 , eIspModuleRaw, DIP_A_SL2,   IspDrv_B::setSl2},
    {ENUM_DIP_CTL_RGB_EN_UDM , eIspModuleRaw, DIP_A_UDM,   IspDrv_B::setUdm},
    {ENUM_DIP_CTL_RGB_EN_LCE , eIspModuleRgb, DIP_A_LCE,   IspDrv_B::setLce},
    {ENUM_DIP_CTL_RGB_EN_G2G , eIspModuleRgb, DIP_A_G2G,   IspDrv_B::setG2g},
    {ENUM_DIP_CTL_RGB_EN_GGM , eIspModuleRgb, DIP_A_GGM,   IspDrv_B::setGgm},
    {ENUM_DIP_CTL_RGB_EN_GDR1, eIspModuleRgb, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_RGB_EN_GDR2, eIspModuleRgb, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_RGB_EN_DBS2, eIspModuleRaw, DIP_A_DBS2,  IspDrv_B::setDbs2},
    {ENUM_DIP_CTL_RGB_EN_OBC2, eIspModuleRaw, DIP_A_OBC2,  IspDrv_B::setOb2},
    {ENUM_DIP_CTL_RGB_EN_RMG2, eIspModuleRaw, DIP_A_RMG2,  IspDrv_B::setRmg2},
    {ENUM_DIP_CTL_RGB_EN_BNR2, eIspModuleRaw, DIP_A_BNR2,  IspDrv_B::setBnr2},
    {ENUM_DIP_CTL_RGB_EN_RMM2, eIspModuleRaw, DIP_A_RMM2,  IspDrv_B::setRmm2},
    {ENUM_DIP_CTL_RGB_EN_LSC2, eIspModuleRaw, DIP_A_LSC2,  IspDrv_B::setLsc2},
    {ENUM_DIP_CTL_RGB_EN_RCP2, eIspModuleRaw, DIP_A_RCP2,  IspDrv_B::setRcp2},
    {ENUM_DIP_CTL_RGB_EN_PAK2, eIspModuleRaw, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_RGB_EN_RNR , eIspModuleRaw, DIP_A_RNR,   IspDrv_B::setRnr},
    {ENUM_DIP_CTL_RGB_EN_SL2G, eIspModuleRaw, DIP_A_SL2G,  IspDrv_B::setSl2g},
    {ENUM_DIP_CTL_RGB_EN_SL2H, eIspModuleRaw, DIP_A_SL2H,  IspDrv_B::setSl2h},
    {ENUM_DIP_CTL_RGB_EN_HLR2, eIspModuleRgb, DIP_A_HLR2_1,IspDrv_B::setHlr2},
    {ENUM_DIP_CTL_RGB_EN_SL2K, eIspModuleRgb, DIP_A_SL2K,  IspDrv_B::setSl2k}};


const IspTopModuleMapping3LayerTable gIspModuleMappingTableYuvEn[CHECK_CTL_YUV_EN_BITS]
 ={ {ENUM_DIP_CTL_YUV_EN_MFB ,      eIspModuleYuv, DIP_A_MFB,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setMfb},
    {ENUM_DIP_CTL_YUV_EN_C02B ,     eIspModuleYuv, DIP_A_C02B, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setC02b},
    {ENUM_DIP_CTL_YUV_EN_C24 ,      eIspModuleYuv, DIP_A_C24,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setC24},
    {ENUM_DIP_CTL_YUV_EN_G2C ,      eIspModuleYuv, DIP_A_G2C,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setG2c},
    {ENUM_DIP_CTL_YUV_EN_C42 ,      eIspModuleYuv, DIP_A_C42,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setC42},
    {ENUM_DIP_CTL_YUV_EN_NBC ,      eIspModuleYuv, DIP_A_ANR,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setNbc},
    {ENUM_DIP_CTL_YUV_EN_NBC2 ,     eIspModuleYuv, DIP_A_ANR2, DIP_A_CCR,   DIP_A_BOK,   DIP_A_ABF,   IspDrv_B::setNbc2},
    {ENUM_DIP_CTL_YUV_EN_PCA ,      eIspModuleYuv, DIP_A_PCA,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setPca},
    {ENUM_DIP_CTL_YUV_EN_SEEE ,     eIspModuleYuv, DIP_A_SEEE, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSeee},
    {ENUM_DIP_CTL_YUV_EN_NR3D ,     eIspModuleYuv, DIP_A_NR3D, DIP_A_COLOR, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setNr3d},
    {ENUM_DIP_CTL_YUV_EN_SL2B ,     eIspModuleYuv, DIP_A_SL2B, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSl2b},
    {ENUM_DIP_CTL_YUV_EN_SL2C ,     eIspModuleYuv, DIP_A_SL2C, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSl2c},
    {ENUM_DIP_CTL_YUV_EN_SL2D ,     eIspModuleYuv, DIP_A_SL2D, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSl2d},
    {ENUM_DIP_CTL_YUV_EN_SL2E ,     eIspModuleYuv, DIP_A_SL2E, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSl2e},
    {ENUM_DIP_CTL_YUV_EN_SRZ1 ,     eIspModuleYuv, DIP_A_SRZ1, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSrz1},
    {ENUM_DIP_CTL_YUV_EN_SRZ2 ,     eIspModuleYuv, DIP_A_SRZ2, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setSrz2},
    {ENUM_DIP_CTL_YUV_EN_CRZ ,      eIspModuleYuv, DIP_A_CRZ,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_YUV_EN_MIX1 ,     eIspModuleYuv, DIP_A_MIX1, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setMix1},
    {ENUM_DIP_CTL_YUV_EN_MIX2 ,     eIspModuleYuv, DIP_A_MIX2, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setMix2},
    {ENUM_DIP_CTL_YUV_EN_MIX3 ,     eIspModuleYuv, DIP_A_MIX3, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setMix3},
    {ENUM_DIP_CTL_YUV_EN_CRSP ,     eIspModuleYuv, DIP_A_CRSP, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setCrsp},
    {ENUM_DIP_CTL_YUV_EN_C24B,      eIspModuleYuv, DIP_A_C24B, DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setC24b},
    {ENUM_DIP_CTL_YUV_EN_MDPCROP,   eIspModuleYuv, DIP_A_DUMMY,DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_YUV_EN_C02,       eIspModuleYuv, DIP_A_C02,  DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setC02},
    {ENUM_DIP_CTL_YUV_EN_FE,        eIspModuleYuv, DIP_A_FE,   DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setFe},
    {ENUM_DIP_CTL_YUV_EN_MFBW,      eIspModuleYuv, DIP_A_DUMMY,DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, IspDrv_B::setMfbw},
    {ENUM_DIP_CTL_YUV_EN_PLNW1,     eIspModuleYuv, DIP_A_DUMMY,DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_YUV_EN_PLNR1,     eIspModuleYuv, DIP_A_DUMMY,DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_YUV_EN_PLNW2,     eIspModuleYuv, DIP_A_DUMMY,DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, NULL},
    {ENUM_DIP_CTL_YUV_EN_PLNR2,     eIspModuleYuv, DIP_A_DUMMY,DIP_A_DUMMY, DIP_A_DUMMY, DIP_A_DUMMY, NULL}};


const IspTopModuleMappingTable gIspModuleMappingTableYuv2En[CHECK_CTL_YUV2_EN_BITS]
 ={ {ENUM_DIP_CTL_YUV_EN_FM ,       eIspModuleYuv, DIP_A_FM,    IspDrv_B::setFm},
    {ENUM_DIP_CTL_YUV_EN_SRZ3 ,     eIspModuleYuv, DIP_A_SRZ3,  IspDrv_B::setSrz3},
    {ENUM_DIP_CTL_YUV_EN_SRZ4 ,     eIspModuleYuv, DIP_A_SRZ4,  IspDrv_B::setSrz4},
    {ENUM_DIP_CTL_YUV_EN_SRZ5 ,     eIspModuleYuv, DIP_A_SRZ5,  IspDrv_B::setSrz5},
    {ENUM_DIP_CTL_YUV_EN_HFG  ,     eIspModuleYuv, DIP_A_HFG,   IspDrv_B::setHfg},
    {ENUM_DIP_CTL_YUV_EN_SL2I ,     eIspModuleYuv, DIP_A_SL2I,  IspDrv_B::setSl2i},
    {ENUM_DIP_CTL_YUV_EN_NDG ,      eIspModuleYuv, DIP_A_NDG,   IspDrv_B::setNdg},
    {ENUM_DIP_CTL_YUV_EN_NDG2 ,     eIspModuleYuv, DIP_A_NDG2,  IspDrv_B::setNdg2}};



/****************************************************************************
* Global Variable
****************************************************************************/
list<_CQ_CONFIG_INFO_> gFrameCQCfgInfoList;
pthread_mutex_t mCQCfgInfoMutex = PTHREAD_MUTEX_INITIALIZER;

/****************************************************************************
*
****************************************************************************/
/*/////////////////////////////////////////////////////////////////////////////
    Isp driver base
/////////////////////////////////////////////////////////////////////////////*/
//IspDrvDip*          IspDrv_B::m_pDrvDip[ISP_DIP_MODULE_IDX_MAX][ISP_DRV_DIP_CQ_NUM][MAX_BURST_QUE_NUM][MAX_DUP_CQ_NUM];
IspDrvDip*          IspDrv_B::m_pDrvDip[ISP_DIP_MODULE_IDX_MAX];
IspDrvDipPhy*       IspDrv_B::m_pDrvDipPhy = NULL;
IMemDrv*            IspDrv_B::m_pIMemDrv = NULL;
CrzDrv*             IspDrv_B::m_pDipCrzDrv = NULL;
MdpMgr*             IspDrv_B::m_pMdpMgr = NULL;
DipRingBuffer*      IspDrv_B::m_pDipRingBuf = NULL;


MINT32
IspDrv_B::
setUnp(DIP_ISP_PIPE &imageioPackage)
{
    //DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_UNP_OFST, 0x00);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UNP_OFST, 0x00);
    
    return 0;
}

MINT32
IspDrv_B::
setUfd(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_UFD_CON, 0x00);
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_UFD_SIZE_CON, 0x00);
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_UFD_AU_CON, 0x00);
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_UFD_CROP_CON1, 0x00);
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_UFD_CROP_CON2, 0x00);

    return 0;
}

MINT32
IspDrv_B::
setRcp2(DIP_ISP_PIPE &imageioPackage)
{
    imageioPackage;
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_RCP2_CROP_CON1, 0x00);
//    DIP_WRITE_REG(imageioPackage.hwModule, imageioPackage.pDrvDip, DIP_X_RCP2_CROP_CON2, 0x00);

    return 0;
}

MINT32
IspDrv_B::
setDbs2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_SIGMA,imageioPackage.pTuningIspReg->DIP_X_DBS2_SIGMA.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_BSTBL_0,imageioPackage.pTuningIspReg->DIP_X_DBS2_BSTBL_0.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_BSTBL_1,imageioPackage.pTuningIspReg->DIP_X_DBS2_BSTBL_1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_BSTBL_2,imageioPackage.pTuningIspReg->DIP_X_DBS2_BSTBL_2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_BSTBL_3,imageioPackage.pTuningIspReg->DIP_X_DBS2_BSTBL_3.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_CTL,imageioPackage.pTuningIspReg->DIP_X_DBS2_CTL.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_CTL_2,imageioPackage.pTuningIspReg->DIP_X_DBS2_CTL_2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_SIGMA_2,imageioPackage.pTuningIspReg->DIP_X_DBS2_SIGMA_2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_YGN,imageioPackage.pTuningIspReg->DIP_X_DBS2_YGN.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_SL_Y12,imageioPackage.pTuningIspReg->DIP_X_DBS2_SL_Y12.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_SL_Y34,imageioPackage.pTuningIspReg->DIP_X_DBS2_SL_Y34.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_SL_G12,imageioPackage.pTuningIspReg->DIP_X_DBS2_SL_G12.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_DBS2_SL_G34,imageioPackage.pTuningIspReg->DIP_X_DBS2_SL_G34.Raw);

    }
    return 0;
}

MINT32
IspDrv_B::
setOb2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_OFFST0,imageioPackage.pTuningIspReg->DIP_X_OBC2_OFFST0.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_OFFST1,imageioPackage.pTuningIspReg->DIP_X_OBC2_OFFST1.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_OFFST2,imageioPackage.pTuningIspReg->DIP_X_OBC2_OFFST2.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_OFFST3,imageioPackage.pTuningIspReg->DIP_X_OBC2_OFFST3.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_GAIN0,imageioPackage.pTuningIspReg->DIP_X_OBC2_GAIN0.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_GAIN1,imageioPackage.pTuningIspReg->DIP_X_OBC2_GAIN1.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_GAIN2,imageioPackage.pTuningIspReg->DIP_X_OBC2_GAIN2.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_OBC2_GAIN3,imageioPackage.pTuningIspReg->DIP_X_OBC2_GAIN3.Raw);

	}
    return 0;
}

MINT32
IspDrv_B::
setBnr2(DIP_ISP_PIPE &imageioPackage)
{
	MUINT32 addr,size;
    if (imageioPackage.isApplyTuning == MTRUE)
    {
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_BNR2,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_BNR2_BPC_CON,size, (MUINT32*)imageioPackage.pTuningIspReg);
    }
    return 0;
}

MINT32
IspDrv_B::
setRmg2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMG2_HDR_CFG,imageioPackage.pTuningIspReg->DIP_X_RMG2_HDR_CFG.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMG2_HDR_GAIN,imageioPackage.pTuningIspReg->DIP_X_RMG2_HDR_GAIN.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMG2_HDR_CFG2,imageioPackage.pTuningIspReg->DIP_X_RMG2_HDR_CFG2.Raw);
    }
    return 0;
}

MINT32
IspDrv_B::
setRmm2(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_OSC,imageioPackage.pTuningIspReg->DIP_X_RMM2_OSC.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_MC,imageioPackage.pTuningIspReg->DIP_X_RMM2_MC.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_REVG_1,imageioPackage.pTuningIspReg->DIP_X_RMM2_REVG_1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_REVG_2,imageioPackage.pTuningIspReg->DIP_X_RMM2_REVG_2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_LEOS,imageioPackage.pTuningIspReg->DIP_X_RMM2_LEOS.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_MC2,imageioPackage.pTuningIspReg->DIP_X_RMM2_MC2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_DIFF_LB,imageioPackage.pTuningIspReg->DIP_X_RMM2_DIFF_LB.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_MA,imageioPackage.pTuningIspReg->DIP_X_RMM2_MA.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_RMM2_TUNE,imageioPackage.pTuningIspReg->DIP_X_RMM2_TUNE.Raw);
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
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_LSC2_CTL1,size, (MUINT32*)imageioPackage.pTuningIspReg);
    }
    return 0;
}


MINT32
IspDrv_B::
setPgn(DIP_ISP_PIPE &imageioPackage)
{
    if (imageioPackage.isApplyTuning == MTRUE){
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_PGN_SATU_1,imageioPackage.pTuningIspReg->DIP_X_PGN_SATU_1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_PGN_SATU_2,imageioPackage.pTuningIspReg->DIP_X_PGN_SATU_2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_PGN_GAIN_1,imageioPackage.pTuningIspReg->DIP_X_PGN_GAIN_1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_PGN_GAIN_2,imageioPackage.pTuningIspReg->DIP_X_PGN_GAIN_2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_PGN_OFST_1,imageioPackage.pTuningIspReg->DIP_X_PGN_OFST_1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_PGN_OFST_2,imageioPackage.pTuningIspReg->DIP_X_PGN_OFST_2.Raw);

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
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_UDM_INTP_CRS,size, (MUINT32*)imageioPackage.pTuningIspReg);
    }
    else {
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INTP_CRS, 0x38303060);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INTP_NAT, 0x1430063F);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_INTP_AUG, 0x00600600);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_LUMA_LUT1, 0x07FF0100);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_LUMA_LUT2, 0x02008020);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_SL_CTL, 0x003FFFE0);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HFTD_CTL,  0x08421000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_NR_STR, 0x81028000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_NR_ACT,  0x00000050);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HF_STR, 0x00000000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HF_ACT1,  0x145034DC);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_HF_ACT2, 0x0034FF55);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_CLIP,  0x00DF2064);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_DSB,  0x007FA800|fgModeRegBit);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_TILE_EDGE,  0x0000000F);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_DSL,  0x00000000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_LR_RAT,  0x0000000F);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_SPARE_2,  0x00000000);
            DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_UDM_SPARE_3,  0x00000000);

    }

    DIP_FUNC_DBG("fdmode bit/reg/udm_dsb(0x%x/0x%x/0x%x)",imageioPackage.isp_top_ctl.FMT_SEL.Bits.FG_MODE, fgModeRegBit, 0x007FA800|fgModeRegBit);

    return 0;
}

MINT32
IspDrv_B::
setG2g(DIP_ISP_PIPE &imageioPackage)
{
//QQ need-confirm

    if (imageioPackage.isApplyTuning == MTRUE){

        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_G2G,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_G2G_CNV_1,size, (MUINT32*)imageioPackage.pTuningIspReg);

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
setLce(DIP_ISP_PIPE &imageioPackage)
{
//QQ need-confirm with Holmes
    if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr,size;
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_LCE,addr,size);
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_LCE25_CON,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_GGM_LUT_RB,size, (MUINT32*)imageioPackage.pTuningIspReg);
	}
    else
    {
        imageioPackage.pDrvDip->writeReg(0x00001000, 0x08000800); /* 0x15023000: DIP_A_GGM_LUT_RB[0] */
        imageioPackage.pDrvDip->writeReg(0x00001004, 0x08020802); /* 0x15023004: DIP_A_GGM_LUT_RB[1] */
        imageioPackage.pDrvDip->writeReg(0x00001008, 0x08040804); /* 0x15023008: DIP_A_GGM_LUT_RB[2] */
        imageioPackage.pDrvDip->writeReg(0x0000100C, 0x08060806); /* 0x1502300C: DIP_A_GGM_LUT_RB[3] */
        imageioPackage.pDrvDip->writeReg(0x00001010, 0x08080808); /* 0x15023010: DIP_A_GGM_LUT_RB[4] */
        imageioPackage.pDrvDip->writeReg(0x00001014, 0x080A080A); /* 0x15023014: DIP_A_GGM_LUT_RB[5] */
        imageioPackage.pDrvDip->writeReg(0x00001018, 0x080C080C); /* 0x15023018: DIP_A_GGM_LUT_RB[6] */
        imageioPackage.pDrvDip->writeReg(0x0000101C, 0x080E080E); /* 0x1502301C: DIP_A_GGM_LUT_RB[7] */
        imageioPackage.pDrvDip->writeReg(0x00001020, 0x08100810); /* 0x15023020: DIP_A_GGM_LUT_RB[8] */
        imageioPackage.pDrvDip->writeReg(0x00001024, 0x08120812); /* 0x15023024: DIP_A_GGM_LUT_RB[9] */
        imageioPackage.pDrvDip->writeReg(0x00001028, 0x08140814); /* 0x15023028: DIP_A_GGM_LUT_RB[10] */
        imageioPackage.pDrvDip->writeReg(0x0000102C, 0x08160816); /* 0x1502302C: DIP_A_GGM_LUT_RB[11] */
        imageioPackage.pDrvDip->writeReg(0x00001030, 0x08180818); /* 0x15023030: DIP_A_GGM_LUT_RB[12] */
        imageioPackage.pDrvDip->writeReg(0x00001034, 0x081A081A); /* 0x15023034: DIP_A_GGM_LUT_RB[13] */
        imageioPackage.pDrvDip->writeReg(0x00001038, 0x081C081C); /* 0x15023038: DIP_A_GGM_LUT_RB[14] */
        imageioPackage.pDrvDip->writeReg(0x0000103C, 0x081E081E); /* 0x1502303C: DIP_A_GGM_LUT_RB[15] */
        imageioPackage.pDrvDip->writeReg(0x00001040, 0x08200820); /* 0x15023040: DIP_A_GGM_LUT_RB[16] */
        imageioPackage.pDrvDip->writeReg(0x00001044, 0x08220822); /* 0x15023044: DIP_A_GGM_LUT_RB[17] */
        imageioPackage.pDrvDip->writeReg(0x00001048, 0x08240824); /* 0x15023048: DIP_A_GGM_LUT_RB[18] */
        imageioPackage.pDrvDip->writeReg(0x0000104C, 0x08260826); /* 0x1502304C: DIP_A_GGM_LUT_RB[19] */
        imageioPackage.pDrvDip->writeReg(0x00001050, 0x08280828); /* 0x15023050: DIP_A_GGM_LUT_RB[20] */
        imageioPackage.pDrvDip->writeReg(0x00001054, 0x082A082A); /* 0x15023054: DIP_A_GGM_LUT_RB[21] */
        imageioPackage.pDrvDip->writeReg(0x00001058, 0x082C082C); /* 0x15023058: DIP_A_GGM_LUT_RB[22] */
        imageioPackage.pDrvDip->writeReg(0x0000105C, 0x082E082E); /* 0x1502305C: DIP_A_GGM_LUT_RB[23] */
        imageioPackage.pDrvDip->writeReg(0x00001060, 0x08300830); /* 0x15023060: DIP_A_GGM_LUT_RB[24] */
        imageioPackage.pDrvDip->writeReg(0x00001064, 0x08320832); /* 0x15023064: DIP_A_GGM_LUT_RB[25] */
        imageioPackage.pDrvDip->writeReg(0x00001068, 0x08340834); /* 0x15023068: DIP_A_GGM_LUT_RB[26] */
        imageioPackage.pDrvDip->writeReg(0x0000106C, 0x08360836); /* 0x1502306C: DIP_A_GGM_LUT_RB[27] */
        imageioPackage.pDrvDip->writeReg(0x00001070, 0x08380838); /* 0x15023070: DIP_A_GGM_LUT_RB[28] */
        imageioPackage.pDrvDip->writeReg(0x00001074, 0x083A083A); /* 0x15023074: DIP_A_GGM_LUT_RB[29] */
        imageioPackage.pDrvDip->writeReg(0x00001078, 0x083C083C); /* 0x15023078: DIP_A_GGM_LUT_RB[30] */
        imageioPackage.pDrvDip->writeReg(0x0000107C, 0x083E083E); /* 0x1502307C: DIP_A_GGM_LUT_RB[31] */
        imageioPackage.pDrvDip->writeReg(0x00001080, 0x08400840); /* 0x15023080: DIP_A_GGM_LUT_RB[32] */
        imageioPackage.pDrvDip->writeReg(0x00001084, 0x08420842); /* 0x15023084: DIP_A_GGM_LUT_RB[33] */
        imageioPackage.pDrvDip->writeReg(0x00001088, 0x08440844); /* 0x15023088: DIP_A_GGM_LUT_RB[34] */
        imageioPackage.pDrvDip->writeReg(0x0000108C, 0x08460846); /* 0x1502308C: DIP_A_GGM_LUT_RB[35] */
        imageioPackage.pDrvDip->writeReg(0x00001090, 0x08480848); /* 0x15023090: DIP_A_GGM_LUT_RB[36] */
        imageioPackage.pDrvDip->writeReg(0x00001094, 0x084A084A); /* 0x15023094: DIP_A_GGM_LUT_RB[37] */
        imageioPackage.pDrvDip->writeReg(0x00001098, 0x084C084C); /* 0x15023098: DIP_A_GGM_LUT_RB[38] */
        imageioPackage.pDrvDip->writeReg(0x0000109C, 0x084E084E); /* 0x1502309C: DIP_A_GGM_LUT_RB[39] */
        imageioPackage.pDrvDip->writeReg(0x000010A0, 0x08500850); /* 0x150230A0: DIP_A_GGM_LUT_RB[40] */
        imageioPackage.pDrvDip->writeReg(0x000010A4, 0x08520852); /* 0x150230A4: DIP_A_GGM_LUT_RB[41] */
        imageioPackage.pDrvDip->writeReg(0x000010A8, 0x08540854); /* 0x150230A8: DIP_A_GGM_LUT_RB[42] */
        imageioPackage.pDrvDip->writeReg(0x000010AC, 0x08560856); /* 0x150230AC: DIP_A_GGM_LUT_RB[43] */
        imageioPackage.pDrvDip->writeReg(0x000010B0, 0x08580858); /* 0x150230B0: DIP_A_GGM_LUT_RB[44] */
        imageioPackage.pDrvDip->writeReg(0x000010B4, 0x085A085A); /* 0x150230B4: DIP_A_GGM_LUT_RB[45] */
        imageioPackage.pDrvDip->writeReg(0x000010B8, 0x085C085C); /* 0x150230B8: DIP_A_GGM_LUT_RB[46] */
        imageioPackage.pDrvDip->writeReg(0x000010BC, 0x085E085E); /* 0x150230BC: DIP_A_GGM_LUT_RB[47] */
        imageioPackage.pDrvDip->writeReg(0x000010C0, 0x08600860); /* 0x150230C0: DIP_A_GGM_LUT_RB[48] */
        imageioPackage.pDrvDip->writeReg(0x000010C4, 0x08620862); /* 0x150230C4: DIP_A_GGM_LUT_RB[49] */
        imageioPackage.pDrvDip->writeReg(0x000010C8, 0x08640864); /* 0x150230C8: DIP_A_GGM_LUT_RB[50] */
        imageioPackage.pDrvDip->writeReg(0x000010CC, 0x08660866); /* 0x150230CC: DIP_A_GGM_LUT_RB[51] */
        imageioPackage.pDrvDip->writeReg(0x000010D0, 0x08680868); /* 0x150230D0: DIP_A_GGM_LUT_RB[52] */
        imageioPackage.pDrvDip->writeReg(0x000010D4, 0x086A086A); /* 0x150230D4: DIP_A_GGM_LUT_RB[53] */
        imageioPackage.pDrvDip->writeReg(0x000010D8, 0x086C086C); /* 0x150230D8: DIP_A_GGM_LUT_RB[54] */
        imageioPackage.pDrvDip->writeReg(0x000010DC, 0x086E086E); /* 0x150230DC: DIP_A_GGM_LUT_RB[55] */
        imageioPackage.pDrvDip->writeReg(0x000010E0, 0x08700870); /* 0x150230E0: DIP_A_GGM_LUT_RB[56] */
        imageioPackage.pDrvDip->writeReg(0x000010E4, 0x08720872); /* 0x150230E4: DIP_A_GGM_LUT_RB[57] */
        imageioPackage.pDrvDip->writeReg(0x000010E8, 0x08740874); /* 0x150230E8: DIP_A_GGM_LUT_RB[58] */
        imageioPackage.pDrvDip->writeReg(0x000010EC, 0x08760876); /* 0x150230EC: DIP_A_GGM_LUT_RB[59] */
        imageioPackage.pDrvDip->writeReg(0x000010F0, 0x08780878); /* 0x150230F0: DIP_A_GGM_LUT_RB[60] */
        imageioPackage.pDrvDip->writeReg(0x000010F4, 0x087A087A); /* 0x150230F4: DIP_A_GGM_LUT_RB[61] */
        imageioPackage.pDrvDip->writeReg(0x000010F8, 0x087C087C); /* 0x150230F8: DIP_A_GGM_LUT_RB[62] */
        imageioPackage.pDrvDip->writeReg(0x000010FC, 0x087E087E); /* 0x150230FC: DIP_A_GGM_LUT_RB[63] */
        imageioPackage.pDrvDip->writeReg(0x00001100, 0x10801080); /* 0x15023100: DIP_A_GGM_LUT_RB[64] */
        imageioPackage.pDrvDip->writeReg(0x00001104, 0x10841084); /* 0x15023104: DIP_A_GGM_LUT_RB[65] */
        imageioPackage.pDrvDip->writeReg(0x00001108, 0x10881088); /* 0x15023108: DIP_A_GGM_LUT_RB[66] */
        imageioPackage.pDrvDip->writeReg(0x0000110C, 0x108C108C); /* 0x1502310C: DIP_A_GGM_LUT_RB[67] */
        imageioPackage.pDrvDip->writeReg(0x00001110, 0x10901090); /* 0x15023110: DIP_A_GGM_LUT_RB[68] */
        imageioPackage.pDrvDip->writeReg(0x00001114, 0x10941094); /* 0x15023114: DIP_A_GGM_LUT_RB[69] */
        imageioPackage.pDrvDip->writeReg(0x00001118, 0x10981098); /* 0x15023118: DIP_A_GGM_LUT_RB[70] */
        imageioPackage.pDrvDip->writeReg(0x0000111C, 0x109C109C); /* 0x1502311C: DIP_A_GGM_LUT_RB[71] */
        imageioPackage.pDrvDip->writeReg(0x00001120, 0x10A010A0); /* 0x15023120: DIP_A_GGM_LUT_RB[72] */
        imageioPackage.pDrvDip->writeReg(0x00001124, 0x10A410A4); /* 0x15023124: DIP_A_GGM_LUT_RB[73] */
        imageioPackage.pDrvDip->writeReg(0x00001128, 0x10A810A8); /* 0x15023128: DIP_A_GGM_LUT_RB[74] */
        imageioPackage.pDrvDip->writeReg(0x0000112C, 0x10AC10AC); /* 0x1502312C: DIP_A_GGM_LUT_RB[75] */
        imageioPackage.pDrvDip->writeReg(0x00001130, 0x10B010B0); /* 0x15023130: DIP_A_GGM_LUT_RB[76] */
        imageioPackage.pDrvDip->writeReg(0x00001134, 0x10B410B4); /* 0x15023134: DIP_A_GGM_LUT_RB[77] */
        imageioPackage.pDrvDip->writeReg(0x00001138, 0x10B810B8); /* 0x15023138: DIP_A_GGM_LUT_RB[78] */
        imageioPackage.pDrvDip->writeReg(0x0000113C, 0x10BC10BC); /* 0x1502313C: DIP_A_GGM_LUT_RB[79] */
        imageioPackage.pDrvDip->writeReg(0x00001140, 0x10C010C0); /* 0x15023140: DIP_A_GGM_LUT_RB[80] */
        imageioPackage.pDrvDip->writeReg(0x00001144, 0x10C410C4); /* 0x15023144: DIP_A_GGM_LUT_RB[81] */
        imageioPackage.pDrvDip->writeReg(0x00001148, 0x10C810C8); /* 0x15023148: DIP_A_GGM_LUT_RB[82] */
        imageioPackage.pDrvDip->writeReg(0x0000114C, 0x10CC10CC); /* 0x1502314C: DIP_A_GGM_LUT_RB[83] */
        imageioPackage.pDrvDip->writeReg(0x00001150, 0x10D010D0); /* 0x15023150: DIP_A_GGM_LUT_RB[84] */
        imageioPackage.pDrvDip->writeReg(0x00001154, 0x10D410D4); /* 0x15023154: DIP_A_GGM_LUT_RB[85] */
        imageioPackage.pDrvDip->writeReg(0x00001158, 0x10D810D8); /* 0x15023158: DIP_A_GGM_LUT_RB[86] */
        imageioPackage.pDrvDip->writeReg(0x0000115C, 0x10DC10DC); /* 0x1502315C: DIP_A_GGM_LUT_RB[87] */
        imageioPackage.pDrvDip->writeReg(0x00001160, 0x10E010E0); /* 0x15023160: DIP_A_GGM_LUT_RB[88] */
        imageioPackage.pDrvDip->writeReg(0x00001164, 0x10E410E4); /* 0x15023164: DIP_A_GGM_LUT_RB[89] */
        imageioPackage.pDrvDip->writeReg(0x00001168, 0x10E810E8); /* 0x15023168: DIP_A_GGM_LUT_RB[90] */
        imageioPackage.pDrvDip->writeReg(0x0000116C, 0x10EC10EC); /* 0x1502316C: DIP_A_GGM_LUT_RB[91] */
        imageioPackage.pDrvDip->writeReg(0x00001170, 0x10F010F0); /* 0x15023170: DIP_A_GGM_LUT_RB[92] */
        imageioPackage.pDrvDip->writeReg(0x00001174, 0x10F410F4); /* 0x15023174: DIP_A_GGM_LUT_RB[93] */
        imageioPackage.pDrvDip->writeReg(0x00001178, 0x10F810F8); /* 0x15023178: DIP_A_GGM_LUT_RB[94] */
        imageioPackage.pDrvDip->writeReg(0x0000117C, 0x10FC10FC); /* 0x1502317C: DIP_A_GGM_LUT_RB[95] */
        imageioPackage.pDrvDip->writeReg(0x00001180, 0x21002100); /* 0x15023180: DIP_A_GGM_LUT_RB[96] */
        imageioPackage.pDrvDip->writeReg(0x00001184, 0x21082108); /* 0x15023184: DIP_A_GGM_LUT_RB[97] */
        imageioPackage.pDrvDip->writeReg(0x00001188, 0x21102110); /* 0x15023188: DIP_A_GGM_LUT_RB[98] */
        imageioPackage.pDrvDip->writeReg(0x0000118C, 0x21182118); /* 0x1502318C: DIP_A_GGM_LUT_RB[99] */
        imageioPackage.pDrvDip->writeReg(0x00001190, 0x21202120); /* 0x15023190: DIP_A_GGM_LUT_RB[100] */
        imageioPackage.pDrvDip->writeReg(0x00001194, 0x21282128); /* 0x15023194: DIP_A_GGM_LUT_RB[101] */
        imageioPackage.pDrvDip->writeReg(0x00001198, 0x21302130); /* 0x15023198: DIP_A_GGM_LUT_RB[102] */
        imageioPackage.pDrvDip->writeReg(0x0000119C, 0x21382138); /* 0x1502319C: DIP_A_GGM_LUT_RB[103] */
        imageioPackage.pDrvDip->writeReg(0x000011A0, 0x21402140); /* 0x150231A0: DIP_A_GGM_LUT_RB[104] */
        imageioPackage.pDrvDip->writeReg(0x000011A4, 0x21482148); /* 0x150231A4: DIP_A_GGM_LUT_RB[105] */
        imageioPackage.pDrvDip->writeReg(0x000011A8, 0x21502150); /* 0x150231A8: DIP_A_GGM_LUT_RB[106] */
        imageioPackage.pDrvDip->writeReg(0x000011AC, 0x21582158); /* 0x150231AC: DIP_A_GGM_LUT_RB[107] */
        imageioPackage.pDrvDip->writeReg(0x000011B0, 0x21602160); /* 0x150231B0: DIP_A_GGM_LUT_RB[108] */
        imageioPackage.pDrvDip->writeReg(0x000011B4, 0x21682168); /* 0x150231B4: DIP_A_GGM_LUT_RB[109] */
        imageioPackage.pDrvDip->writeReg(0x000011B8, 0x21702170); /* 0x150231B8: DIP_A_GGM_LUT_RB[110] */
        imageioPackage.pDrvDip->writeReg(0x000011BC, 0x21782178); /* 0x150231BC: DIP_A_GGM_LUT_RB[111] */
        imageioPackage.pDrvDip->writeReg(0x000011C0, 0x21802180); /* 0x150231C0: DIP_A_GGM_LUT_RB[112] */
        imageioPackage.pDrvDip->writeReg(0x000011C4, 0x21882188); /* 0x150231C4: DIP_A_GGM_LUT_RB[113] */
        imageioPackage.pDrvDip->writeReg(0x000011C8, 0x21902190); /* 0x150231C8: DIP_A_GGM_LUT_RB[114] */
        imageioPackage.pDrvDip->writeReg(0x000011CC, 0x21982198); /* 0x150231CC: DIP_A_GGM_LUT_RB[115] */
        imageioPackage.pDrvDip->writeReg(0x000011D0, 0x21A021A0); /* 0x150231D0: DIP_A_GGM_LUT_RB[116] */
        imageioPackage.pDrvDip->writeReg(0x000011D4, 0x21A821A8); /* 0x150231D4: DIP_A_GGM_LUT_RB[117] */
        imageioPackage.pDrvDip->writeReg(0x000011D8, 0x21B021B0); /* 0x150231D8: DIP_A_GGM_LUT_RB[118] */
        imageioPackage.pDrvDip->writeReg(0x000011DC, 0x21B821B8); /* 0x150231DC: DIP_A_GGM_LUT_RB[119] */
        imageioPackage.pDrvDip->writeReg(0x000011E0, 0x21C021C0); /* 0x150231E0: DIP_A_GGM_LUT_RB[120] */
        imageioPackage.pDrvDip->writeReg(0x000011E4, 0x21C821C8); /* 0x150231E4: DIP_A_GGM_LUT_RB[121] */
        imageioPackage.pDrvDip->writeReg(0x000011E8, 0x21D021D0); /* 0x150231E8: DIP_A_GGM_LUT_RB[122] */
        imageioPackage.pDrvDip->writeReg(0x000011EC, 0x21D821D8); /* 0x150231EC: DIP_A_GGM_LUT_RB[123] */
        imageioPackage.pDrvDip->writeReg(0x000011F0, 0x21E021E0); /* 0x150231F0: DIP_A_GGM_LUT_RB[124] */
        imageioPackage.pDrvDip->writeReg(0x000011F4, 0x21E821E8); /* 0x150231F4: DIP_A_GGM_LUT_RB[125] */
        imageioPackage.pDrvDip->writeReg(0x000011F8, 0x21F021F0); /* 0x150231F8: DIP_A_GGM_LUT_RB[126] */
        imageioPackage.pDrvDip->writeReg(0x000011FC, 0x21F821F8); /* 0x150231FC: DIP_A_GGM_LUT_RB[127] */
        imageioPackage.pDrvDip->writeReg(0x00001200, 0x82008200); /* 0x15023200: DIP_A_GGM_LUT_RB[128] */
        imageioPackage.pDrvDip->writeReg(0x00001204, 0x82208220); /* 0x15023204: DIP_A_GGM_LUT_RB[129] */
        imageioPackage.pDrvDip->writeReg(0x00001208, 0x82408240); /* 0x15023208: DIP_A_GGM_LUT_RB[130] */
        imageioPackage.pDrvDip->writeReg(0x0000120C, 0x82608260); /* 0x1502320C: DIP_A_GGM_LUT_RB[131] */
        imageioPackage.pDrvDip->writeReg(0x00001210, 0x82808280); /* 0x15023210: DIP_A_GGM_LUT_RB[132] */
        imageioPackage.pDrvDip->writeReg(0x00001214, 0x82A082A0); /* 0x15023214: DIP_A_GGM_LUT_RB[133] */
        imageioPackage.pDrvDip->writeReg(0x00001218, 0x82C082C0); /* 0x15023218: DIP_A_GGM_LUT_RB[134] */
        imageioPackage.pDrvDip->writeReg(0x0000121C, 0x82E082E0); /* 0x1502321C: DIP_A_GGM_LUT_RB[135] */
        imageioPackage.pDrvDip->writeReg(0x00001220, 0x83008300); /* 0x15023220: DIP_A_GGM_LUT_RB[136] */
        imageioPackage.pDrvDip->writeReg(0x00001224, 0x83208320); /* 0x15023224: DIP_A_GGM_LUT_RB[137] */
        imageioPackage.pDrvDip->writeReg(0x00001228, 0x83408340); /* 0x15023228: DIP_A_GGM_LUT_RB[138] */
        imageioPackage.pDrvDip->writeReg(0x0000122C, 0x83608360); /* 0x1502322C: DIP_A_GGM_LUT_RB[139] */
        imageioPackage.pDrvDip->writeReg(0x00001230, 0x83808380); /* 0x15023230: DIP_A_GGM_LUT_RB[140] */
        imageioPackage.pDrvDip->writeReg(0x00001234, 0x83A083A0); /* 0x15023234: DIP_A_GGM_LUT_RB[141] */
        imageioPackage.pDrvDip->writeReg(0x00001238, 0x83C083C0); /* 0x15023238: DIP_A_GGM_LUT_RB[142] */
        imageioPackage.pDrvDip->writeReg(0x0000123C, 0x7FE07FE0); /* 0x1502323C: DIP_A_GGM_LUT_RB[143] */
        imageioPackage.pDrvDip->writeReg(0x00001240, 0x00000800); /* 0x15023240: DIP_A_GGM_LUT_G[0] */
        imageioPackage.pDrvDip->writeReg(0x00001244, 0x00000802); /* 0x15023244: DIP_A_GGM_LUT_G[1] */
        imageioPackage.pDrvDip->writeReg(0x00001248, 0x00000804); /* 0x15023248: DIP_A_GGM_LUT_G[2] */
        imageioPackage.pDrvDip->writeReg(0x0000124C, 0x00000806); /* 0x1502324C: DIP_A_GGM_LUT_G[3] */
        imageioPackage.pDrvDip->writeReg(0x00001250, 0x00000808); /* 0x15023250: DIP_A_GGM_LUT_G[4] */
        imageioPackage.pDrvDip->writeReg(0x00001254, 0x0000080A); /* 0x15023254: DIP_A_GGM_LUT_G[5] */
        imageioPackage.pDrvDip->writeReg(0x00001258, 0x0000080C); /* 0x15023258: DIP_A_GGM_LUT_G[6] */
        imageioPackage.pDrvDip->writeReg(0x0000125C, 0x0000080E); /* 0x1502325C: DIP_A_GGM_LUT_G[7] */
        imageioPackage.pDrvDip->writeReg(0x00001260, 0x00000810); /* 0x15023260: DIP_A_GGM_LUT_G[8] */
        imageioPackage.pDrvDip->writeReg(0x00001264, 0x00000812); /* 0x15023264: DIP_A_GGM_LUT_G[9] */
        imageioPackage.pDrvDip->writeReg(0x00001268, 0x00000814); /* 0x15023268: DIP_A_GGM_LUT_G[10] */
        imageioPackage.pDrvDip->writeReg(0x0000126C, 0x00000816); /* 0x1502326C: DIP_A_GGM_LUT_G[11] */
        imageioPackage.pDrvDip->writeReg(0x00001270, 0x00000818); /* 0x15023270: DIP_A_GGM_LUT_G[12] */
        imageioPackage.pDrvDip->writeReg(0x00001274, 0x0000081A); /* 0x15023274: DIP_A_GGM_LUT_G[13] */
        imageioPackage.pDrvDip->writeReg(0x00001278, 0x0000081C); /* 0x15023278: DIP_A_GGM_LUT_G[14] */
        imageioPackage.pDrvDip->writeReg(0x0000127C, 0x0000081E); /* 0x1502327C: DIP_A_GGM_LUT_G[15] */
        imageioPackage.pDrvDip->writeReg(0x00001280, 0x00000820); /* 0x15023280: DIP_A_GGM_LUT_G[16] */
        imageioPackage.pDrvDip->writeReg(0x00001284, 0x00000822); /* 0x15023284: DIP_A_GGM_LUT_G[17] */
        imageioPackage.pDrvDip->writeReg(0x00001288, 0x00000824); /* 0x15023288: DIP_A_GGM_LUT_G[18] */
        imageioPackage.pDrvDip->writeReg(0x0000128C, 0x00000826); /* 0x1502328C: DIP_A_GGM_LUT_G[19] */
        imageioPackage.pDrvDip->writeReg(0x00001290, 0x00000828); /* 0x15023290: DIP_A_GGM_LUT_G[20] */
        imageioPackage.pDrvDip->writeReg(0x00001294, 0x0000082A); /* 0x15023294: DIP_A_GGM_LUT_G[21] */
        imageioPackage.pDrvDip->writeReg(0x00001298, 0x0000082C); /* 0x15023298: DIP_A_GGM_LUT_G[22] */
        imageioPackage.pDrvDip->writeReg(0x0000129C, 0x0000082E); /* 0x1502329C: DIP_A_GGM_LUT_G[23] */
        imageioPackage.pDrvDip->writeReg(0x000012A0, 0x00000830); /* 0x150232A0: DIP_A_GGM_LUT_G[24] */
        imageioPackage.pDrvDip->writeReg(0x000012A4, 0x00000832); /* 0x150232A4: DIP_A_GGM_LUT_G[25] */
        imageioPackage.pDrvDip->writeReg(0x000012A8, 0x00000834); /* 0x150232A8: DIP_A_GGM_LUT_G[26] */
        imageioPackage.pDrvDip->writeReg(0x000012AC, 0x00000836); /* 0x150232AC: DIP_A_GGM_LUT_G[27] */
        imageioPackage.pDrvDip->writeReg(0x000012B0, 0x00000838); /* 0x150232B0: DIP_A_GGM_LUT_G[28] */
        imageioPackage.pDrvDip->writeReg(0x000012B4, 0x0000083A); /* 0x150232B4: DIP_A_GGM_LUT_G[29] */
        imageioPackage.pDrvDip->writeReg(0x000012B8, 0x0000083C); /* 0x150232B8: DIP_A_GGM_LUT_G[30] */
        imageioPackage.pDrvDip->writeReg(0x000012BC, 0x0000083E); /* 0x150232BC: DIP_A_GGM_LUT_G[31] */
        imageioPackage.pDrvDip->writeReg(0x000012C0, 0x00000840); /* 0x150232C0: DIP_A_GGM_LUT_G[32] */
        imageioPackage.pDrvDip->writeReg(0x000012C4, 0x00000842); /* 0x150232C4: DIP_A_GGM_LUT_G[33] */
        imageioPackage.pDrvDip->writeReg(0x000012C8, 0x00000844); /* 0x150232C8: DIP_A_GGM_LUT_G[34] */
        imageioPackage.pDrvDip->writeReg(0x000012CC, 0x00000846); /* 0x150232CC: DIP_A_GGM_LUT_G[35] */
        imageioPackage.pDrvDip->writeReg(0x000012D0, 0x00000848); /* 0x150232D0: DIP_A_GGM_LUT_G[36] */
        imageioPackage.pDrvDip->writeReg(0x000012D4, 0x0000084A); /* 0x150232D4: DIP_A_GGM_LUT_G[37] */
        imageioPackage.pDrvDip->writeReg(0x000012D8, 0x0000084C); /* 0x150232D8: DIP_A_GGM_LUT_G[38] */
        imageioPackage.pDrvDip->writeReg(0x000012DC, 0x0000084E); /* 0x150232DC: DIP_A_GGM_LUT_G[39] */
        imageioPackage.pDrvDip->writeReg(0x000012E0, 0x00000850); /* 0x150232E0: DIP_A_GGM_LUT_G[40] */
        imageioPackage.pDrvDip->writeReg(0x000012E4, 0x00000852); /* 0x150232E4: DIP_A_GGM_LUT_G[41] */
        imageioPackage.pDrvDip->writeReg(0x000012E8, 0x00000854); /* 0x150232E8: DIP_A_GGM_LUT_G[42] */
        imageioPackage.pDrvDip->writeReg(0x000012EC, 0x00000856); /* 0x150232EC: DIP_A_GGM_LUT_G[43] */
        imageioPackage.pDrvDip->writeReg(0x000012F0, 0x00000858); /* 0x150232F0: DIP_A_GGM_LUT_G[44] */
        imageioPackage.pDrvDip->writeReg(0x000012F4, 0x0000085A); /* 0x150232F4: DIP_A_GGM_LUT_G[45] */
        imageioPackage.pDrvDip->writeReg(0x000012F8, 0x0000085C); /* 0x150232F8: DIP_A_GGM_LUT_G[46] */
        imageioPackage.pDrvDip->writeReg(0x000012FC, 0x0000085E); /* 0x150232FC: DIP_A_GGM_LUT_G[47] */
        imageioPackage.pDrvDip->writeReg(0x00001300, 0x00000860); /* 0x15023300: DIP_A_GGM_LUT_G[48] */
        imageioPackage.pDrvDip->writeReg(0x00001304, 0x00000862); /* 0x15023304: DIP_A_GGM_LUT_G[49] */
        imageioPackage.pDrvDip->writeReg(0x00001308, 0x00000864); /* 0x15023308: DIP_A_GGM_LUT_G[50] */
        imageioPackage.pDrvDip->writeReg(0x0000130C, 0x00000866); /* 0x1502330C: DIP_A_GGM_LUT_G[51] */
        imageioPackage.pDrvDip->writeReg(0x00001310, 0x00000868); /* 0x15023310: DIP_A_GGM_LUT_G[52] */
        imageioPackage.pDrvDip->writeReg(0x00001314, 0x0000086A); /* 0x15023314: DIP_A_GGM_LUT_G[53] */
        imageioPackage.pDrvDip->writeReg(0x00001318, 0x0000086C); /* 0x15023318: DIP_A_GGM_LUT_G[54] */
        imageioPackage.pDrvDip->writeReg(0x0000131C, 0x0000086E); /* 0x1502331C: DIP_A_GGM_LUT_G[55] */
        imageioPackage.pDrvDip->writeReg(0x00001320, 0x00000870); /* 0x15023320: DIP_A_GGM_LUT_G[56] */
        imageioPackage.pDrvDip->writeReg(0x00001324, 0x00000872); /* 0x15023324: DIP_A_GGM_LUT_G[57] */
        imageioPackage.pDrvDip->writeReg(0x00001328, 0x00000874); /* 0x15023328: DIP_A_GGM_LUT_G[58] */
        imageioPackage.pDrvDip->writeReg(0x0000132C, 0x00000876); /* 0x1502332C: DIP_A_GGM_LUT_G[59] */
        imageioPackage.pDrvDip->writeReg(0x00001330, 0x00000878); /* 0x15023330: DIP_A_GGM_LUT_G[60] */
        imageioPackage.pDrvDip->writeReg(0x00001334, 0x0000087A); /* 0x15023334: DIP_A_GGM_LUT_G[61] */
        imageioPackage.pDrvDip->writeReg(0x00001338, 0x0000087C); /* 0x15023338: DIP_A_GGM_LUT_G[62] */
        imageioPackage.pDrvDip->writeReg(0x0000133C, 0x0000087E); /* 0x1502333C: DIP_A_GGM_LUT_G[63] */
        imageioPackage.pDrvDip->writeReg(0x00001340, 0x00001080); /* 0x15023340: DIP_A_GGM_LUT_G[64] */
        imageioPackage.pDrvDip->writeReg(0x00001344, 0x00001084); /* 0x15023344: DIP_A_GGM_LUT_G[65] */
        imageioPackage.pDrvDip->writeReg(0x00001348, 0x00001088); /* 0x15023348: DIP_A_GGM_LUT_G[66] */
        imageioPackage.pDrvDip->writeReg(0x0000134C, 0x0000108C); /* 0x1502334C: DIP_A_GGM_LUT_G[67] */
        imageioPackage.pDrvDip->writeReg(0x00001350, 0x00001090); /* 0x15023350: DIP_A_GGM_LUT_G[68] */
        imageioPackage.pDrvDip->writeReg(0x00001354, 0x00001094); /* 0x15023354: DIP_A_GGM_LUT_G[69] */
        imageioPackage.pDrvDip->writeReg(0x00001358, 0x00001098); /* 0x15023358: DIP_A_GGM_LUT_G[70] */
        imageioPackage.pDrvDip->writeReg(0x0000135C, 0x0000109C); /* 0x1502335C: DIP_A_GGM_LUT_G[71] */
        imageioPackage.pDrvDip->writeReg(0x00001360, 0x000010A0); /* 0x15023360: DIP_A_GGM_LUT_G[72] */
        imageioPackage.pDrvDip->writeReg(0x00001364, 0x000010A4); /* 0x15023364: DIP_A_GGM_LUT_G[73] */
        imageioPackage.pDrvDip->writeReg(0x00001368, 0x000010A8); /* 0x15023368: DIP_A_GGM_LUT_G[74] */
        imageioPackage.pDrvDip->writeReg(0x0000136C, 0x000010AC); /* 0x1502336C: DIP_A_GGM_LUT_G[75] */
        imageioPackage.pDrvDip->writeReg(0x00001370, 0x000010B0); /* 0x15023370: DIP_A_GGM_LUT_G[76] */
        imageioPackage.pDrvDip->writeReg(0x00001374, 0x000010B4); /* 0x15023374: DIP_A_GGM_LUT_G[77] */
        imageioPackage.pDrvDip->writeReg(0x00001378, 0x000010B8); /* 0x15023378: DIP_A_GGM_LUT_G[78] */
        imageioPackage.pDrvDip->writeReg(0x0000137C, 0x000010BC); /* 0x1502337C: DIP_A_GGM_LUT_G[79] */
        imageioPackage.pDrvDip->writeReg(0x00001380, 0x000010C0); /* 0x15023380: DIP_A_GGM_LUT_G[80] */
        imageioPackage.pDrvDip->writeReg(0x00001384, 0x000010C4); /* 0x15023384: DIP_A_GGM_LUT_G[81] */
        imageioPackage.pDrvDip->writeReg(0x00001388, 0x000010C8); /* 0x15023388: DIP_A_GGM_LUT_G[82] */
        imageioPackage.pDrvDip->writeReg(0x0000138C, 0x000010CC); /* 0x1502338C: DIP_A_GGM_LUT_G[83] */
        imageioPackage.pDrvDip->writeReg(0x00001390, 0x000010D0); /* 0x15023390: DIP_A_GGM_LUT_G[84] */
        imageioPackage.pDrvDip->writeReg(0x00001394, 0x000010D4); /* 0x15023394: DIP_A_GGM_LUT_G[85] */
        imageioPackage.pDrvDip->writeReg(0x00001398, 0x000010D8); /* 0x15023398: DIP_A_GGM_LUT_G[86] */
        imageioPackage.pDrvDip->writeReg(0x0000139C, 0x000010DC); /* 0x1502339C: DIP_A_GGM_LUT_G[87] */
        imageioPackage.pDrvDip->writeReg(0x000013A0, 0x000010E0); /* 0x150233A0: DIP_A_GGM_LUT_G[88] */
        imageioPackage.pDrvDip->writeReg(0x000013A4, 0x000010E4); /* 0x150233A4: DIP_A_GGM_LUT_G[89] */
        imageioPackage.pDrvDip->writeReg(0x000013A8, 0x000010E8); /* 0x150233A8: DIP_A_GGM_LUT_G[90] */
        imageioPackage.pDrvDip->writeReg(0x000013AC, 0x000010EC); /* 0x150233AC: DIP_A_GGM_LUT_G[91] */
        imageioPackage.pDrvDip->writeReg(0x000013B0, 0x000010F0); /* 0x150233B0: DIP_A_GGM_LUT_G[92] */
        imageioPackage.pDrvDip->writeReg(0x000013B4, 0x000010F4); /* 0x150233B4: DIP_A_GGM_LUT_G[93] */
        imageioPackage.pDrvDip->writeReg(0x000013B8, 0x000010F8); /* 0x150233B8: DIP_A_GGM_LUT_G[94] */
        imageioPackage.pDrvDip->writeReg(0x000013BC, 0x000010FC); /* 0x150233BC: DIP_A_GGM_LUT_G[95] */
        imageioPackage.pDrvDip->writeReg(0x000013C0, 0x00002100); /* 0x150233C0: DIP_A_GGM_LUT_G[96] */
        imageioPackage.pDrvDip->writeReg(0x000013C4, 0x00002108); /* 0x150233C4: DIP_A_GGM_LUT_G[97] */
        imageioPackage.pDrvDip->writeReg(0x000013C8, 0x00002110); /* 0x150233C8: DIP_A_GGM_LUT_G[98] */
        imageioPackage.pDrvDip->writeReg(0x000013CC, 0x00002118); /* 0x150233CC: DIP_A_GGM_LUT_G[99] */
        imageioPackage.pDrvDip->writeReg(0x000013D0, 0x00002120); /* 0x150233D0: DIP_A_GGM_LUT_G[100] */
        imageioPackage.pDrvDip->writeReg(0x000013D4, 0x00002128); /* 0x150233D4: DIP_A_GGM_LUT_G[101] */
        imageioPackage.pDrvDip->writeReg(0x000013D8, 0x00002130); /* 0x150233D8: DIP_A_GGM_LUT_G[102] */
        imageioPackage.pDrvDip->writeReg(0x000013DC, 0x00002138); /* 0x150233DC: DIP_A_GGM_LUT_G[103] */
        imageioPackage.pDrvDip->writeReg(0x000013E0, 0x00002140); /* 0x150233E0: DIP_A_GGM_LUT_G[104] */
        imageioPackage.pDrvDip->writeReg(0x000013E4, 0x00002148); /* 0x150233E4: DIP_A_GGM_LUT_G[105] */
        imageioPackage.pDrvDip->writeReg(0x000013E8, 0x00002150); /* 0x150233E8: DIP_A_GGM_LUT_G[106] */
        imageioPackage.pDrvDip->writeReg(0x000013EC, 0x00002158); /* 0x150233EC: DIP_A_GGM_LUT_G[107] */
        imageioPackage.pDrvDip->writeReg(0x000013F0, 0x00002160); /* 0x150233F0: DIP_A_GGM_LUT_G[108] */
        imageioPackage.pDrvDip->writeReg(0x000013F4, 0x00002168); /* 0x150233F4: DIP_A_GGM_LUT_G[109] */
        imageioPackage.pDrvDip->writeReg(0x000013F8, 0x00002170); /* 0x150233F8: DIP_A_GGM_LUT_G[110] */
        imageioPackage.pDrvDip->writeReg(0x000013FC, 0x00002178); /* 0x150233FC: DIP_A_GGM_LUT_G[111] */
        imageioPackage.pDrvDip->writeReg(0x00001400, 0x00002180); /* 0x15023400: DIP_A_GGM_LUT_G[112] */
        imageioPackage.pDrvDip->writeReg(0x00001404, 0x00002188); /* 0x15023404: DIP_A_GGM_LUT_G[113] */
        imageioPackage.pDrvDip->writeReg(0x00001408, 0x00002190); /* 0x15023408: DIP_A_GGM_LUT_G[114] */
        imageioPackage.pDrvDip->writeReg(0x0000140C, 0x00002198); /* 0x1502340C: DIP_A_GGM_LUT_G[115] */
        imageioPackage.pDrvDip->writeReg(0x00001410, 0x000021A0); /* 0x15023410: DIP_A_GGM_LUT_G[116] */
        imageioPackage.pDrvDip->writeReg(0x00001414, 0x000021A8); /* 0x15023414: DIP_A_GGM_LUT_G[117] */
        imageioPackage.pDrvDip->writeReg(0x00001418, 0x000021B0); /* 0x15023418: DIP_A_GGM_LUT_G[118] */
        imageioPackage.pDrvDip->writeReg(0x0000141C, 0x000021B8); /* 0x1502341C: DIP_A_GGM_LUT_G[119] */
        imageioPackage.pDrvDip->writeReg(0x00001420, 0x000021C0); /* 0x15023420: DIP_A_GGM_LUT_G[120] */
        imageioPackage.pDrvDip->writeReg(0x00001424, 0x000021C8); /* 0x15023424: DIP_A_GGM_LUT_G[121] */
        imageioPackage.pDrvDip->writeReg(0x00001428, 0x000021D0); /* 0x15023428: DIP_A_GGM_LUT_G[122] */
        imageioPackage.pDrvDip->writeReg(0x0000142C, 0x000021D8); /* 0x1502342C: DIP_A_GGM_LUT_G[123] */
        imageioPackage.pDrvDip->writeReg(0x00001430, 0x000021E0); /* 0x15023430: DIP_A_GGM_LUT_G[124] */
        imageioPackage.pDrvDip->writeReg(0x00001434, 0x000021E8); /* 0x15023434: DIP_A_GGM_LUT_G[125] */
        imageioPackage.pDrvDip->writeReg(0x00001438, 0x000021F0); /* 0x15023438: DIP_A_GGM_LUT_G[126] */
        imageioPackage.pDrvDip->writeReg(0x0000143C, 0x000021F8); /* 0x1502343C: DIP_A_GGM_LUT_G[127] */
        imageioPackage.pDrvDip->writeReg(0x00001440, 0x00008200); /* 0x15023440: DIP_A_GGM_LUT_G[128] */
        imageioPackage.pDrvDip->writeReg(0x00001444, 0x00008220); /* 0x15023444: DIP_A_GGM_LUT_G[129] */
        imageioPackage.pDrvDip->writeReg(0x00001448, 0x00008240); /* 0x15023448: DIP_A_GGM_LUT_G[130] */
        imageioPackage.pDrvDip->writeReg(0x0000144C, 0x00008260); /* 0x1502344C: DIP_A_GGM_LUT_G[131] */
        imageioPackage.pDrvDip->writeReg(0x00001450, 0x00008280); /* 0x15023450: DIP_A_GGM_LUT_G[132] */
        imageioPackage.pDrvDip->writeReg(0x00001454, 0x000082A0); /* 0x15023454: DIP_A_GGM_LUT_G[133] */
        imageioPackage.pDrvDip->writeReg(0x00001458, 0x000082C0); /* 0x15023458: DIP_A_GGM_LUT_G[134] */
        imageioPackage.pDrvDip->writeReg(0x0000145C, 0x000082E0); /* 0x1502345C: DIP_A_GGM_LUT_G[135] */
        imageioPackage.pDrvDip->writeReg(0x00001460, 0x00008300); /* 0x15023460: DIP_A_GGM_LUT_G[136] */
        imageioPackage.pDrvDip->writeReg(0x00001464, 0x00008320); /* 0x15023464: DIP_A_GGM_LUT_G[137] */
        imageioPackage.pDrvDip->writeReg(0x00001468, 0x00008340); /* 0x15023468: DIP_A_GGM_LUT_G[138] */
        imageioPackage.pDrvDip->writeReg(0x0000146C, 0x00008360); /* 0x1502346C: DIP_A_GGM_LUT_G[139] */
        imageioPackage.pDrvDip->writeReg(0x00001470, 0x00008380); /* 0x15023470: DIP_A_GGM_LUT_G[140] */
        imageioPackage.pDrvDip->writeReg(0x00001474, 0x000083A0); /* 0x15023474: DIP_A_GGM_LUT_G[141] */
        imageioPackage.pDrvDip->writeReg(0x00001478, 0x000083C0); /* 0x15023478: DIP_A_GGM_LUT_G[142] */
        imageioPackage.pDrvDip->writeReg(0x0000147C, 0x00007FE0); /* 0x1502347C: DIP_A_GGM_LUT_G[143] */
        //imageioPackage.pDrvDip->writeReg(0x00001480, 0x00000000); /* 0x15023480: DIP_A_GGM_CTRL */
        imageioPackage.pDrvDip->writeReg(0x00001480, 0x000007FE); /* 0x15023480: DIP_A_GGM_CTRL */
        imageioPackage.pDrvDip->writeReg(0x00003000, 0x51FC32A2); /* 0x15025000: DIP_B_GGM_LUT_RB[0] */
        imageioPackage.pDrvDip->writeReg(0x00003004, 0x7CD35AAB); /* 0x15025004: DIP_B_GGM_LUT_RB[1] */
        imageioPackage.pDrvDip->writeReg(0x00003008, 0x832D3C37); /* 0x15025008: DIP_B_GGM_LUT_RB[2] */
        imageioPackage.pDrvDip->writeReg(0x0000300C, 0x19837F05); /* 0x1502500C: DIP_B_GGM_LUT_RB[3] */
        imageioPackage.pDrvDip->writeReg(0x00003010, 0xC5849CE7); /* 0x15025010: DIP_B_GGM_LUT_RB[4] */
        imageioPackage.pDrvDip->writeReg(0x00003014, 0xF302BF18); /* 0x15025014: DIP_B_GGM_LUT_RB[5] */
        imageioPackage.pDrvDip->writeReg(0x00003018, 0xAFB97024); /* 0x15025018: DIP_B_GGM_LUT_RB[6] */
        imageioPackage.pDrvDip->writeReg(0x0000301C, 0x4D0CC69E); /* 0x1502501C: DIP_B_GGM_LUT_RB[7] */
        imageioPackage.pDrvDip->writeReg(0x00003020, 0xC4F0D74B); /* 0x15025020: DIP_B_GGM_LUT_RB[8] */
        imageioPackage.pDrvDip->writeReg(0x00003024, 0x66A581F1); /* 0x15025024: DIP_B_GGM_LUT_RB[9] */
        imageioPackage.pDrvDip->writeReg(0x00003028, 0x4FF5E73F); /* 0x15025028: DIP_B_GGM_LUT_RB[10] */
        imageioPackage.pDrvDip->writeReg(0x0000302C, 0xC4718E8E); /* 0x1502502C: DIP_B_GGM_LUT_RB[11] */
        imageioPackage.pDrvDip->writeReg(0x00003030, 0x9C86DCB9); /* 0x15025030: DIP_B_GGM_LUT_RB[12] */
        imageioPackage.pDrvDip->writeReg(0x00003034, 0xBC3F0C2E); /* 0x15025034: DIP_B_GGM_LUT_RB[13] */
        imageioPackage.pDrvDip->writeReg(0x00003038, 0xA0204380); /* 0x15025038: DIP_B_GGM_LUT_RB[14] */
        imageioPackage.pDrvDip->writeReg(0x0000303C, 0x70314E74); /* 0x1502503C: DIP_B_GGM_LUT_RB[15] */
        imageioPackage.pDrvDip->writeReg(0x00003040, 0x5C52EDA8); /* 0x15025040: DIP_B_GGM_LUT_RB[16] */
        imageioPackage.pDrvDip->writeReg(0x00003044, 0x0C77F482); /* 0x15025044: DIP_B_GGM_LUT_RB[17] */
        imageioPackage.pDrvDip->writeReg(0x00003048, 0x2CE55E8C); /* 0x15025048: DIP_B_GGM_LUT_RB[18] */
        imageioPackage.pDrvDip->writeReg(0x0000304C, 0xC2B86EF5); /* 0x1502504C: DIP_B_GGM_LUT_RB[19] */
        imageioPackage.pDrvDip->writeReg(0x00003050, 0x118F80F7); /* 0x15025050: DIP_B_GGM_LUT_RB[20] */
        imageioPackage.pDrvDip->writeReg(0x00003054, 0xF430D1D9); /* 0x15025054: DIP_B_GGM_LUT_RB[21] */
        imageioPackage.pDrvDip->writeReg(0x00003058, 0x543E5522); /* 0x15025058: DIP_B_GGM_LUT_RB[22] */
        imageioPackage.pDrvDip->writeReg(0x0000305C, 0xF0F5A76C); /* 0x1502505C: DIP_B_GGM_LUT_RB[23] */
        imageioPackage.pDrvDip->writeReg(0x00003060, 0x446E7727); /* 0x15025060: DIP_B_GGM_LUT_RB[24] */
        imageioPackage.pDrvDip->writeReg(0x00003064, 0xE1A63E64); /* 0x15025064: DIP_B_GGM_LUT_RB[25] */
        imageioPackage.pDrvDip->writeReg(0x00003068, 0xEC939B44); /* 0x15025068: DIP_B_GGM_LUT_RB[26] */
        imageioPackage.pDrvDip->writeReg(0x0000306C, 0xC8ABC865); /* 0x1502506C: DIP_B_GGM_LUT_RB[27] */
        imageioPackage.pDrvDip->writeReg(0x00003070, 0x8BE4B9AD); /* 0x15025070: DIP_B_GGM_LUT_RB[28] */
        imageioPackage.pDrvDip->writeReg(0x00003074, 0x0CE97EDB); /* 0x15025074: DIP_B_GGM_LUT_RB[29] */
        imageioPackage.pDrvDip->writeReg(0x00003078, 0x38FD6F8D); /* 0x15025078: DIP_B_GGM_LUT_RB[30] */
        imageioPackage.pDrvDip->writeReg(0x0000307C, 0xA34FC705); /* 0x1502507C: DIP_B_GGM_LUT_RB[31] */
        imageioPackage.pDrvDip->writeReg(0x00003080, 0x875961C7); /* 0x15025080: DIP_B_GGM_LUT_RB[32] */
        imageioPackage.pDrvDip->writeReg(0x00003084, 0x1C4F550D); /* 0x15025084: DIP_B_GGM_LUT_RB[33] */
        imageioPackage.pDrvDip->writeReg(0x00003088, 0x7B40D1D2); /* 0x15025088: DIP_B_GGM_LUT_RB[34] */
        imageioPackage.pDrvDip->writeReg(0x0000308C, 0xB069AF63); /* 0x1502508C: DIP_B_GGM_LUT_RB[35] */
        imageioPackage.pDrvDip->writeReg(0x00003090, 0x8AD7D124); /* 0x15025090: DIP_B_GGM_LUT_RB[36] */
        imageioPackage.pDrvDip->writeReg(0x00003094, 0xF9EDA6D1); /* 0x15025094: DIP_B_GGM_LUT_RB[37] */
        imageioPackage.pDrvDip->writeReg(0x00003098, 0x5D907A88); /* 0x15025098: DIP_B_GGM_LUT_RB[38] */
        imageioPackage.pDrvDip->writeReg(0x0000309C, 0x2E233543); /* 0x1502509C: DIP_B_GGM_LUT_RB[39] */
        imageioPackage.pDrvDip->writeReg(0x000030A0, 0x80AD5315); /* 0x150250A0: DIP_B_GGM_LUT_RB[40] */
        imageioPackage.pDrvDip->writeReg(0x000030A4, 0xC3E272CB); /* 0x150250A4: DIP_B_GGM_LUT_RB[41] */
        imageioPackage.pDrvDip->writeReg(0x000030A8, 0x846AD653); /* 0x150250A8: DIP_B_GGM_LUT_RB[42] */
        imageioPackage.pDrvDip->writeReg(0x000030AC, 0x9A89C881); /* 0x150250AC: DIP_B_GGM_LUT_RB[43] */
        imageioPackage.pDrvDip->writeReg(0x000030B0, 0x5A90B9ED); /* 0x150250B0: DIP_B_GGM_LUT_RB[44] */
        imageioPackage.pDrvDip->writeReg(0x000030B4, 0x0BD2F910); /* 0x150250B4: DIP_B_GGM_LUT_RB[45] */
        imageioPackage.pDrvDip->writeReg(0x000030B8, 0xEFE451E9); /* 0x150250B8: DIP_B_GGM_LUT_RB[46] */
        imageioPackage.pDrvDip->writeReg(0x000030BC, 0x0E57DB60); /* 0x150250BC: DIP_B_GGM_LUT_RB[47] */
        imageioPackage.pDrvDip->writeReg(0x000030C0, 0x1444C642); /* 0x150250C0: DIP_B_GGM_LUT_RB[48] */
        imageioPackage.pDrvDip->writeReg(0x000030C4, 0x0E04AFD6); /* 0x150250C4: DIP_B_GGM_LUT_RB[49] */
        imageioPackage.pDrvDip->writeReg(0x000030C8, 0x2BD9F148); /* 0x150250C8: DIP_B_GGM_LUT_RB[50] */
        imageioPackage.pDrvDip->writeReg(0x000030CC, 0x9D1D1E6E); /* 0x150250CC: DIP_B_GGM_LUT_RB[51] */
        imageioPackage.pDrvDip->writeReg(0x000030D0, 0xEBEE3E03); /* 0x150250D0: DIP_B_GGM_LUT_RB[52] */
        imageioPackage.pDrvDip->writeReg(0x000030D4, 0x1EBB3E11); /* 0x150250D4: DIP_B_GGM_LUT_RB[53] */
        imageioPackage.pDrvDip->writeReg(0x000030D8, 0x2CC18D75); /* 0x150250D8: DIP_B_GGM_LUT_RB[54] */
        imageioPackage.pDrvDip->writeReg(0x000030DC, 0xEA162348); /* 0x150250DC: DIP_B_GGM_LUT_RB[55] */
        imageioPackage.pDrvDip->writeReg(0x000030E0, 0xE3E7EB69); /* 0x150250E0: DIP_B_GGM_LUT_RB[56] */
        imageioPackage.pDrvDip->writeReg(0x000030E4, 0x7ACFE8FD); /* 0x150250E4: DIP_B_GGM_LUT_RB[57] */
        imageioPackage.pDrvDip->writeReg(0x000030E8, 0xA4253C0A); /* 0x150250E8: DIP_B_GGM_LUT_RB[58] */
        imageioPackage.pDrvDip->writeReg(0x000030EC, 0x8B03FFA2); /* 0x150250EC: DIP_B_GGM_LUT_RB[59] */
        imageioPackage.pDrvDip->writeReg(0x000030F0, 0xE8994F52); /* 0x150250F0: DIP_B_GGM_LUT_RB[60] */
        imageioPackage.pDrvDip->writeReg(0x000030F4, 0xCF5DDB50); /* 0x150250F4: DIP_B_GGM_LUT_RB[61] */
        imageioPackage.pDrvDip->writeReg(0x000030F8, 0xA6BF21A2); /* 0x150250F8: DIP_B_GGM_LUT_RB[62] */
        imageioPackage.pDrvDip->writeReg(0x000030FC, 0xB98A101C); /* 0x150250FC: DIP_B_GGM_LUT_RB[63] */
        imageioPackage.pDrvDip->writeReg(0x00003100, 0x9BDA2515); /* 0x15025100: DIP_B_GGM_LUT_RB[64] */
        imageioPackage.pDrvDip->writeReg(0x00003104, 0xF9A256DF); /* 0x15025104: DIP_B_GGM_LUT_RB[65] */
        imageioPackage.pDrvDip->writeReg(0x00003108, 0xD84834D4); /* 0x15025108: DIP_B_GGM_LUT_RB[66] */
        imageioPackage.pDrvDip->writeReg(0x0000310C, 0x9FD42127); /* 0x1502510C: DIP_B_GGM_LUT_RB[67] */
        imageioPackage.pDrvDip->writeReg(0x00003110, 0x357C27D1); /* 0x15025110: DIP_B_GGM_LUT_RB[68] */
        imageioPackage.pDrvDip->writeReg(0x00003114, 0x053D1CD3); /* 0x15025114: DIP_B_GGM_LUT_RB[69] */
        imageioPackage.pDrvDip->writeReg(0x00003118, 0x758FA6EB); /* 0x15025118: DIP_B_GGM_LUT_RB[70] */
        imageioPackage.pDrvDip->writeReg(0x0000311C, 0xB4D85B4C); /* 0x1502511C: DIP_B_GGM_LUT_RB[71] */
        imageioPackage.pDrvDip->writeReg(0x00003120, 0x85F839D1); /* 0x15025120: DIP_B_GGM_LUT_RB[72] */
        imageioPackage.pDrvDip->writeReg(0x00003124, 0xFFE9F8ED); /* 0x15025124: DIP_B_GGM_LUT_RB[73] */
        imageioPackage.pDrvDip->writeReg(0x00003128, 0x45EF466C); /* 0x15025128: DIP_B_GGM_LUT_RB[74] */
        imageioPackage.pDrvDip->writeReg(0x0000312C, 0xF2875267); /* 0x1502512C: DIP_B_GGM_LUT_RB[75] */
        imageioPackage.pDrvDip->writeReg(0x00003130, 0x396A9866); /* 0x15025130: DIP_B_GGM_LUT_RB[76] */
        imageioPackage.pDrvDip->writeReg(0x00003134, 0xB3DE96C4); /* 0x15025134: DIP_B_GGM_LUT_RB[77] */
        imageioPackage.pDrvDip->writeReg(0x00003138, 0x1085FA77); /* 0x15025138: DIP_B_GGM_LUT_RB[78] */
        imageioPackage.pDrvDip->writeReg(0x0000313C, 0x97BBB97B); /* 0x1502513C: DIP_B_GGM_LUT_RB[79] */
        imageioPackage.pDrvDip->writeReg(0x00003140, 0xD9B2A1B9); /* 0x15025140: DIP_B_GGM_LUT_RB[80] */
        imageioPackage.pDrvDip->writeReg(0x00003144, 0x5FCFCE67); /* 0x15025144: DIP_B_GGM_LUT_RB[81] */
        imageioPackage.pDrvDip->writeReg(0x00003148, 0x969A3174); /* 0x15025148: DIP_B_GGM_LUT_RB[82] */
        imageioPackage.pDrvDip->writeReg(0x0000314C, 0xF94C1AF7); /* 0x1502514C: DIP_B_GGM_LUT_RB[83] */
        imageioPackage.pDrvDip->writeReg(0x00003150, 0x0A95B6C1); /* 0x15025150: DIP_B_GGM_LUT_RB[84] */
        imageioPackage.pDrvDip->writeReg(0x00003154, 0x0D245257); /* 0x15025154: DIP_B_GGM_LUT_RB[85] */
        imageioPackage.pDrvDip->writeReg(0x00003158, 0xF042416F); /* 0x15025158: DIP_B_GGM_LUT_RB[86] */
        imageioPackage.pDrvDip->writeReg(0x0000315C, 0x270D5666); /* 0x1502515C: DIP_B_GGM_LUT_RB[87] */
        imageioPackage.pDrvDip->writeReg(0x00003160, 0xD1DF52FC); /* 0x15025160: DIP_B_GGM_LUT_RB[88] */
        imageioPackage.pDrvDip->writeReg(0x00003164, 0xA93F054E); /* 0x15025164: DIP_B_GGM_LUT_RB[89] */
        imageioPackage.pDrvDip->writeReg(0x00003168, 0xDA58E517); /* 0x15025168: DIP_B_GGM_LUT_RB[90] */
        imageioPackage.pDrvDip->writeReg(0x0000316C, 0x827E0CCA); /* 0x1502516C: DIP_B_GGM_LUT_RB[91] */
        imageioPackage.pDrvDip->writeReg(0x00003170, 0x4E469D0B); /* 0x15025170: DIP_B_GGM_LUT_RB[92] */
        imageioPackage.pDrvDip->writeReg(0x00003174, 0xD9D8B48C); /* 0x15025174: DIP_B_GGM_LUT_RB[93] */
        imageioPackage.pDrvDip->writeReg(0x00003178, 0xFC7ACAF0); /* 0x15025178: DIP_B_GGM_LUT_RB[94] */
        imageioPackage.pDrvDip->writeReg(0x0000317C, 0x85EA04C7); /* 0x1502517C: DIP_B_GGM_LUT_RB[95] */
        imageioPackage.pDrvDip->writeReg(0x00003180, 0x8292AA9D); /* 0x15025180: DIP_B_GGM_LUT_RB[96] */
        imageioPackage.pDrvDip->writeReg(0x00003184, 0xA75FFC20); /* 0x15025184: DIP_B_GGM_LUT_RB[97] */
        imageioPackage.pDrvDip->writeReg(0x00003188, 0xA4CDA426); /* 0x15025188: DIP_B_GGM_LUT_RB[98] */
        imageioPackage.pDrvDip->writeReg(0x0000318C, 0xF4CF7855); /* 0x1502518C: DIP_B_GGM_LUT_RB[99] */
        imageioPackage.pDrvDip->writeReg(0x00003190, 0x1ABC3489); /* 0x15025190: DIP_B_GGM_LUT_RB[100] */
        imageioPackage.pDrvDip->writeReg(0x00003194, 0x1DD15E31); /* 0x15025194: DIP_B_GGM_LUT_RB[101] */
        imageioPackage.pDrvDip->writeReg(0x00003198, 0x52944250); /* 0x15025198: DIP_B_GGM_LUT_RB[102] */
        imageioPackage.pDrvDip->writeReg(0x0000319C, 0xBC53E69F); /* 0x1502519C: DIP_B_GGM_LUT_RB[103] */
        imageioPackage.pDrvDip->writeReg(0x000031A0, 0x40B1AF29); /* 0x150251A0: DIP_B_GGM_LUT_RB[104] */
        imageioPackage.pDrvDip->writeReg(0x000031A4, 0x91AEEBAE); /* 0x150251A4: DIP_B_GGM_LUT_RB[105] */
        imageioPackage.pDrvDip->writeReg(0x000031A8, 0xDFCC883F); /* 0x150251A8: DIP_B_GGM_LUT_RB[106] */
        imageioPackage.pDrvDip->writeReg(0x000031AC, 0xFE93F521); /* 0x150251AC: DIP_B_GGM_LUT_RB[107] */
        imageioPackage.pDrvDip->writeReg(0x000031B0, 0xC3FE399C); /* 0x150251B0: DIP_B_GGM_LUT_RB[108] */
        imageioPackage.pDrvDip->writeReg(0x000031B4, 0xB06CCBF5); /* 0x150251B4: DIP_B_GGM_LUT_RB[109] */
        imageioPackage.pDrvDip->writeReg(0x000031B8, 0xA1941704); /* 0x150251B8: DIP_B_GGM_LUT_RB[110] */
        imageioPackage.pDrvDip->writeReg(0x000031BC, 0xC8A9CE6A); /* 0x150251BC: DIP_B_GGM_LUT_RB[111] */
        imageioPackage.pDrvDip->writeReg(0x000031C0, 0xB466055C); /* 0x150251C0: DIP_B_GGM_LUT_RB[112] */
        imageioPackage.pDrvDip->writeReg(0x000031C4, 0xD2CF41C1); /* 0x150251C4: DIP_B_GGM_LUT_RB[113] */
        imageioPackage.pDrvDip->writeReg(0x000031C8, 0x2F900ED7); /* 0x150251C8: DIP_B_GGM_LUT_RB[114] */
        imageioPackage.pDrvDip->writeReg(0x000031CC, 0xD0A13E6D); /* 0x150251CC: DIP_B_GGM_LUT_RB[115] */
        imageioPackage.pDrvDip->writeReg(0x000031D0, 0x23EB776C); /* 0x150251D0: DIP_B_GGM_LUT_RB[116] */
        imageioPackage.pDrvDip->writeReg(0x000031D4, 0xF8329688); /* 0x150251D4: DIP_B_GGM_LUT_RB[117] */
        imageioPackage.pDrvDip->writeReg(0x000031D8, 0x04BF03BA); /* 0x150251D8: DIP_B_GGM_LUT_RB[118] */
        imageioPackage.pDrvDip->writeReg(0x000031DC, 0xFE0383A3); /* 0x150251DC: DIP_B_GGM_LUT_RB[119] */
        imageioPackage.pDrvDip->writeReg(0x000031E0, 0xB9F354D1); /* 0x150251E0: DIP_B_GGM_LUT_RB[120] */
        imageioPackage.pDrvDip->writeReg(0x000031E4, 0x1FC774F3); /* 0x150251E4: DIP_B_GGM_LUT_RB[121] */
        imageioPackage.pDrvDip->writeReg(0x000031E8, 0x8950DC74); /* 0x150251E8: DIP_B_GGM_LUT_RB[122] */
        imageioPackage.pDrvDip->writeReg(0x000031EC, 0x58C6BA69); /* 0x150251EC: DIP_B_GGM_LUT_RB[123] */
        imageioPackage.pDrvDip->writeReg(0x000031F0, 0x807E3B2D); /* 0x150251F0: DIP_B_GGM_LUT_RB[124] */
        imageioPackage.pDrvDip->writeReg(0x000031F4, 0xF2342D86); /* 0x150251F4: DIP_B_GGM_LUT_RB[125] */
        imageioPackage.pDrvDip->writeReg(0x000031F8, 0x809CBF51); /* 0x150251F8: DIP_B_GGM_LUT_RB[126] */
        imageioPackage.pDrvDip->writeReg(0x000031FC, 0x2FFC258F); /* 0x150251FC: DIP_B_GGM_LUT_RB[127] */
        imageioPackage.pDrvDip->writeReg(0x00003200, 0x9FE0EF0E); /* 0x15025200: DIP_B_GGM_LUT_RB[128] */
        imageioPackage.pDrvDip->writeReg(0x00003204, 0x05BE409B); /* 0x15025204: DIP_B_GGM_LUT_RB[129] */
        imageioPackage.pDrvDip->writeReg(0x00003208, 0x3FEDF830); /* 0x15025208: DIP_B_GGM_LUT_RB[130] */
        imageioPackage.pDrvDip->writeReg(0x0000320C, 0xD991AED8); /* 0x1502520C: DIP_B_GGM_LUT_RB[131] */
        imageioPackage.pDrvDip->writeReg(0x00003210, 0x95B77374); /* 0x15025210: DIP_B_GGM_LUT_RB[132] */
        imageioPackage.pDrvDip->writeReg(0x00003214, 0x92B3573D); /* 0x15025214: DIP_B_GGM_LUT_RB[133] */
        imageioPackage.pDrvDip->writeReg(0x00003218, 0x267E8F95); /* 0x15025218: DIP_B_GGM_LUT_RB[134] */
        imageioPackage.pDrvDip->writeReg(0x0000321C, 0x9F030BEC); /* 0x1502521C: DIP_B_GGM_LUT_RB[135] */
        imageioPackage.pDrvDip->writeReg(0x00003220, 0x1C0B9A54); /* 0x15025220: DIP_B_GGM_LUT_RB[136] */
        imageioPackage.pDrvDip->writeReg(0x00003224, 0x53454A3A); /* 0x15025224: DIP_B_GGM_LUT_RB[137] */
        imageioPackage.pDrvDip->writeReg(0x00003228, 0xC44FF7FB); /* 0x15025228: DIP_B_GGM_LUT_RB[138] */
        imageioPackage.pDrvDip->writeReg(0x0000322C, 0x8B920BAF); /* 0x1502522C: DIP_B_GGM_LUT_RB[139] */
        imageioPackage.pDrvDip->writeReg(0x00003230, 0xBB36387E); /* 0x15025230: DIP_B_GGM_LUT_RB[140] */
        imageioPackage.pDrvDip->writeReg(0x00003234, 0x19D78E97); /* 0x15025234: DIP_B_GGM_LUT_RB[141] */
        imageioPackage.pDrvDip->writeReg(0x00003238, 0x1B3BBF42); /* 0x15025238: DIP_B_GGM_LUT_RB[142] */
        imageioPackage.pDrvDip->writeReg(0x0000323C, 0x1086B7BD); /* 0x1502523C: DIP_B_GGM_LUT_RB[143] */
        imageioPackage.pDrvDip->writeReg(0x00003240, 0x0000AF3A); /* 0x15025240: DIP_B_GGM_LUT_G[0] */
        imageioPackage.pDrvDip->writeReg(0x00003244, 0x00004C1D); /* 0x15025244: DIP_B_GGM_LUT_G[1] */
        imageioPackage.pDrvDip->writeReg(0x00003248, 0x000061FD); /* 0x15025248: DIP_B_GGM_LUT_G[2] */
        imageioPackage.pDrvDip->writeReg(0x0000324C, 0x000088A7); /* 0x1502524C: DIP_B_GGM_LUT_G[3] */
        imageioPackage.pDrvDip->writeReg(0x00003250, 0x00002412); /* 0x15025250: DIP_B_GGM_LUT_G[4] */
        imageioPackage.pDrvDip->writeReg(0x00003254, 0x0000332D); /* 0x15025254: DIP_B_GGM_LUT_G[5] */
        imageioPackage.pDrvDip->writeReg(0x00003258, 0x0000F137); /* 0x15025258: DIP_B_GGM_LUT_G[6] */
        imageioPackage.pDrvDip->writeReg(0x0000325C, 0x00007A64); /* 0x1502525C: DIP_B_GGM_LUT_G[7] */
        imageioPackage.pDrvDip->writeReg(0x00003260, 0x000064ED); /* 0x15025260: DIP_B_GGM_LUT_G[8] */
        imageioPackage.pDrvDip->writeReg(0x00003264, 0x000083FA); /* 0x15025264: DIP_B_GGM_LUT_G[9] */
        imageioPackage.pDrvDip->writeReg(0x00003268, 0x0000C410); /* 0x15025268: DIP_B_GGM_LUT_G[10] */
        imageioPackage.pDrvDip->writeReg(0x0000326C, 0x000076FD); /* 0x1502526C: DIP_B_GGM_LUT_G[11] */
        imageioPackage.pDrvDip->writeReg(0x00003270, 0x0000310F); /* 0x15025270: DIP_B_GGM_LUT_G[12] */
        imageioPackage.pDrvDip->writeReg(0x00003274, 0x000076DC); /* 0x15025274: DIP_B_GGM_LUT_G[13] */
        imageioPackage.pDrvDip->writeReg(0x00003278, 0x0000B6B4); /* 0x15025278: DIP_B_GGM_LUT_G[14] */
        imageioPackage.pDrvDip->writeReg(0x0000327C, 0x00003CEF); /* 0x1502527C: DIP_B_GGM_LUT_G[15] */
        imageioPackage.pDrvDip->writeReg(0x00003280, 0x000077F2); /* 0x15025280: DIP_B_GGM_LUT_G[16] */
        imageioPackage.pDrvDip->writeReg(0x00003284, 0x0000902D); /* 0x15025284: DIP_B_GGM_LUT_G[17] */
        imageioPackage.pDrvDip->writeReg(0x00003288, 0x00009E30); /* 0x15025288: DIP_B_GGM_LUT_G[18] */
        imageioPackage.pDrvDip->writeReg(0x0000328C, 0x0000FA5C); /* 0x1502528C: DIP_B_GGM_LUT_G[19] */
        imageioPackage.pDrvDip->writeReg(0x00003290, 0x0000A3AB); /* 0x15025290: DIP_B_GGM_LUT_G[20] */
        imageioPackage.pDrvDip->writeReg(0x00003294, 0x0000F9E5); /* 0x15025294: DIP_B_GGM_LUT_G[21] */
        imageioPackage.pDrvDip->writeReg(0x00003298, 0x0000F6DA); /* 0x15025298: DIP_B_GGM_LUT_G[22] */
        imageioPackage.pDrvDip->writeReg(0x0000329C, 0x00002058); /* 0x1502529C: DIP_B_GGM_LUT_G[23] */
        imageioPackage.pDrvDip->writeReg(0x000032A0, 0x00001DB8); /* 0x150252A0: DIP_B_GGM_LUT_G[24] */
        imageioPackage.pDrvDip->writeReg(0x000032A4, 0x00003EE7); /* 0x150252A4: DIP_B_GGM_LUT_G[25] */
        imageioPackage.pDrvDip->writeReg(0x000032A8, 0x0000B8B1); /* 0x150252A8: DIP_B_GGM_LUT_G[26] */
        imageioPackage.pDrvDip->writeReg(0x000032AC, 0x000090F3); /* 0x150252AC: DIP_B_GGM_LUT_G[27] */
        imageioPackage.pDrvDip->writeReg(0x000032B0, 0x00001637); /* 0x150252B0: DIP_B_GGM_LUT_G[28] */
        imageioPackage.pDrvDip->writeReg(0x000032B4, 0x00007895); /* 0x150252B4: DIP_B_GGM_LUT_G[29] */
        imageioPackage.pDrvDip->writeReg(0x000032B8, 0x00003BF4); /* 0x150252B8: DIP_B_GGM_LUT_G[30] */
        imageioPackage.pDrvDip->writeReg(0x000032BC, 0x0000AF04); /* 0x150252BC: DIP_B_GGM_LUT_G[31] */
        imageioPackage.pDrvDip->writeReg(0x000032C0, 0x0000119A); /* 0x150252C0: DIP_B_GGM_LUT_G[32] */
        imageioPackage.pDrvDip->writeReg(0x000032C4, 0x0000A4C0); /* 0x150252C4: DIP_B_GGM_LUT_G[33] */
        imageioPackage.pDrvDip->writeReg(0x000032C8, 0x000000B9); /* 0x150252C8: DIP_B_GGM_LUT_G[34] */
        imageioPackage.pDrvDip->writeReg(0x000032CC, 0x0000531E); /* 0x150252CC: DIP_B_GGM_LUT_G[35] */
        imageioPackage.pDrvDip->writeReg(0x000032D0, 0x0000BEAC); /* 0x150252D0: DIP_B_GGM_LUT_G[36] */
        imageioPackage.pDrvDip->writeReg(0x000032D4, 0x00004E6B); /* 0x150252D4: DIP_B_GGM_LUT_G[37] */
        imageioPackage.pDrvDip->writeReg(0x000032D8, 0x00005BE5); /* 0x150252D8: DIP_B_GGM_LUT_G[38] */
        imageioPackage.pDrvDip->writeReg(0x000032DC, 0x000008F7); /* 0x150252DC: DIP_B_GGM_LUT_G[39] */
        imageioPackage.pDrvDip->writeReg(0x000032E0, 0x0000D97D); /* 0x150252E0: DIP_B_GGM_LUT_G[40] */
        imageioPackage.pDrvDip->writeReg(0x000032E4, 0x000069F5); /* 0x150252E4: DIP_B_GGM_LUT_G[41] */
        imageioPackage.pDrvDip->writeReg(0x000032E8, 0x00002048); /* 0x150252E8: DIP_B_GGM_LUT_G[42] */
        imageioPackage.pDrvDip->writeReg(0x000032EC, 0x000088A8); /* 0x150252EC: DIP_B_GGM_LUT_G[43] */
        imageioPackage.pDrvDip->writeReg(0x000032F0, 0x0000C246); /* 0x150252F0: DIP_B_GGM_LUT_G[44] */
        imageioPackage.pDrvDip->writeReg(0x000032F4, 0x0000EFE7); /* 0x150252F4: DIP_B_GGM_LUT_G[45] */
        imageioPackage.pDrvDip->writeReg(0x000032F8, 0x00002821); /* 0x150252F8: DIP_B_GGM_LUT_G[46] */
        imageioPackage.pDrvDip->writeReg(0x000032FC, 0x00005662); /* 0x150252FC: DIP_B_GGM_LUT_G[47] */
        imageioPackage.pDrvDip->writeReg(0x00003300, 0x0000EF56); /* 0x15025300: DIP_B_GGM_LUT_G[48] */
        imageioPackage.pDrvDip->writeReg(0x00003304, 0x00000C9A); /* 0x15025304: DIP_B_GGM_LUT_G[49] */
        imageioPackage.pDrvDip->writeReg(0x00003308, 0x0000C677); /* 0x15025308: DIP_B_GGM_LUT_G[50] */
        imageioPackage.pDrvDip->writeReg(0x0000330C, 0x0000D528); /* 0x1502530C: DIP_B_GGM_LUT_G[51] */
        imageioPackage.pDrvDip->writeReg(0x00003310, 0x0000A1AA); /* 0x15025310: DIP_B_GGM_LUT_G[52] */
        imageioPackage.pDrvDip->writeReg(0x00003314, 0x0000C734); /* 0x15025314: DIP_B_GGM_LUT_G[53] */
        imageioPackage.pDrvDip->writeReg(0x00003318, 0x0000E931); /* 0x15025318: DIP_B_GGM_LUT_G[54] */
        imageioPackage.pDrvDip->writeReg(0x0000331C, 0x00001E41); /* 0x1502531C: DIP_B_GGM_LUT_G[55] */
        imageioPackage.pDrvDip->writeReg(0x00003320, 0x0000FF7E); /* 0x15025320: DIP_B_GGM_LUT_G[56] */
        imageioPackage.pDrvDip->writeReg(0x00003324, 0x0000827C); /* 0x15025324: DIP_B_GGM_LUT_G[57] */
        imageioPackage.pDrvDip->writeReg(0x00003328, 0x0000AC78); /* 0x15025328: DIP_B_GGM_LUT_G[58] */
        imageioPackage.pDrvDip->writeReg(0x0000332C, 0x000070F7); /* 0x1502532C: DIP_B_GGM_LUT_G[59] */
        imageioPackage.pDrvDip->writeReg(0x00003330, 0x000054D4); /* 0x15025330: DIP_B_GGM_LUT_G[60] */
        imageioPackage.pDrvDip->writeReg(0x00003334, 0x0000950D); /* 0x15025334: DIP_B_GGM_LUT_G[61] */
        imageioPackage.pDrvDip->writeReg(0x00003338, 0x0000D624); /* 0x15025338: DIP_B_GGM_LUT_G[62] */
        imageioPackage.pDrvDip->writeReg(0x0000333C, 0x00002151); /* 0x1502533C: DIP_B_GGM_LUT_G[63] */
        imageioPackage.pDrvDip->writeReg(0x00003340, 0x00004241); /* 0x15025340: DIP_B_GGM_LUT_G[64] */
        imageioPackage.pDrvDip->writeReg(0x00003344, 0x00001A91); /* 0x15025344: DIP_B_GGM_LUT_G[65] */
        imageioPackage.pDrvDip->writeReg(0x00003348, 0x0000C2E7); /* 0x15025348: DIP_B_GGM_LUT_G[66] */
        imageioPackage.pDrvDip->writeReg(0x0000334C, 0x0000FBF6); /* 0x1502534C: DIP_B_GGM_LUT_G[67] */
        imageioPackage.pDrvDip->writeReg(0x00003350, 0x0000CDD3); /* 0x15025350: DIP_B_GGM_LUT_G[68] */
        imageioPackage.pDrvDip->writeReg(0x00003354, 0x00005C1F); /* 0x15025354: DIP_B_GGM_LUT_G[69] */
        imageioPackage.pDrvDip->writeReg(0x00003358, 0x00002A50); /* 0x15025358: DIP_B_GGM_LUT_G[70] */
        imageioPackage.pDrvDip->writeReg(0x0000335C, 0x0000ED09); /* 0x1502535C: DIP_B_GGM_LUT_G[71] */
        imageioPackage.pDrvDip->writeReg(0x00003360, 0x00006FA8); /* 0x15025360: DIP_B_GGM_LUT_G[72] */
        imageioPackage.pDrvDip->writeReg(0x00003364, 0x0000BBC4); /* 0x15025364: DIP_B_GGM_LUT_G[73] */
        imageioPackage.pDrvDip->writeReg(0x00003368, 0x00003E82); /* 0x15025368: DIP_B_GGM_LUT_G[74] */
        imageioPackage.pDrvDip->writeReg(0x0000336C, 0x0000BE3C); /* 0x1502536C: DIP_B_GGM_LUT_G[75] */
        imageioPackage.pDrvDip->writeReg(0x00003370, 0x0000756E); /* 0x15025370: DIP_B_GGM_LUT_G[76] */
        imageioPackage.pDrvDip->writeReg(0x00003374, 0x00009E14); /* 0x15025374: DIP_B_GGM_LUT_G[77] */
        imageioPackage.pDrvDip->writeReg(0x00003378, 0x0000EF5C); /* 0x15025378: DIP_B_GGM_LUT_G[78] */
        imageioPackage.pDrvDip->writeReg(0x0000337C, 0x0000B770); /* 0x1502537C: DIP_B_GGM_LUT_G[79] */
        imageioPackage.pDrvDip->writeReg(0x00003380, 0x000057C0); /* 0x15025380: DIP_B_GGM_LUT_G[80] */
        imageioPackage.pDrvDip->writeReg(0x00003384, 0x0000A47B); /* 0x15025384: DIP_B_GGM_LUT_G[81] */
        imageioPackage.pDrvDip->writeReg(0x00003388, 0x000041B0); /* 0x15025388: DIP_B_GGM_LUT_G[82] */
        imageioPackage.pDrvDip->writeReg(0x0000338C, 0x0000E787); /* 0x1502538C: DIP_B_GGM_LUT_G[83] */
        imageioPackage.pDrvDip->writeReg(0x00003390, 0x000067E3); /* 0x15025390: DIP_B_GGM_LUT_G[84] */
        imageioPackage.pDrvDip->writeReg(0x00003394, 0x00002BC6); /* 0x15025394: DIP_B_GGM_LUT_G[85] */
        imageioPackage.pDrvDip->writeReg(0x00003398, 0x0000BBD8); /* 0x15025398: DIP_B_GGM_LUT_G[86] */
        imageioPackage.pDrvDip->writeReg(0x0000339C, 0x0000057A); /* 0x1502539C: DIP_B_GGM_LUT_G[87] */
        imageioPackage.pDrvDip->writeReg(0x000033A0, 0x00003BFF); /* 0x150253A0: DIP_B_GGM_LUT_G[88] */
        imageioPackage.pDrvDip->writeReg(0x000033A4, 0x00000122); /* 0x150253A4: DIP_B_GGM_LUT_G[89] */
        imageioPackage.pDrvDip->writeReg(0x000033A8, 0x0000D958); /* 0x150253A8: DIP_B_GGM_LUT_G[90] */
        imageioPackage.pDrvDip->writeReg(0x000033AC, 0x000035A9); /* 0x150253AC: DIP_B_GGM_LUT_G[91] */
        imageioPackage.pDrvDip->writeReg(0x000033B0, 0x0000A94E); /* 0x150253B0: DIP_B_GGM_LUT_G[92] */
        imageioPackage.pDrvDip->writeReg(0x000033B4, 0x0000D3F6); /* 0x150253B4: DIP_B_GGM_LUT_G[93] */
        imageioPackage.pDrvDip->writeReg(0x000033B8, 0x00000D3F); /* 0x150253B8: DIP_B_GGM_LUT_G[94] */
        imageioPackage.pDrvDip->writeReg(0x000033BC, 0x00009276); /* 0x150253BC: DIP_B_GGM_LUT_G[95] */
        imageioPackage.pDrvDip->writeReg(0x000033C0, 0x0000E1DC); /* 0x150253C0: DIP_B_GGM_LUT_G[96] */
        imageioPackage.pDrvDip->writeReg(0x000033C4, 0x0000AFB4); /* 0x150253C4: DIP_B_GGM_LUT_G[97] */
        imageioPackage.pDrvDip->writeReg(0x000033C8, 0x0000F79F); /* 0x150253C8: DIP_B_GGM_LUT_G[98] */
        imageioPackage.pDrvDip->writeReg(0x000033CC, 0x00001FF1); /* 0x150253CC: DIP_B_GGM_LUT_G[99] */
        imageioPackage.pDrvDip->writeReg(0x000033D0, 0x00000A7B); /* 0x150253D0: DIP_B_GGM_LUT_G[100] */
        imageioPackage.pDrvDip->writeReg(0x000033D4, 0x0000BC4D); /* 0x150253D4: DIP_B_GGM_LUT_G[101] */
        imageioPackage.pDrvDip->writeReg(0x000033D8, 0x0000F204); /* 0x150253D8: DIP_B_GGM_LUT_G[102] */
        imageioPackage.pDrvDip->writeReg(0x000033DC, 0x00008334); /* 0x150253DC: DIP_B_GGM_LUT_G[103] */
        imageioPackage.pDrvDip->writeReg(0x000033E0, 0x0000A4A4); /* 0x150253E0: DIP_B_GGM_LUT_G[104] */
        imageioPackage.pDrvDip->writeReg(0x000033E4, 0x000001E0); /* 0x150253E4: DIP_B_GGM_LUT_G[105] */
        imageioPackage.pDrvDip->writeReg(0x000033E8, 0x00009C5D); /* 0x150253E8: DIP_B_GGM_LUT_G[106] */
        imageioPackage.pDrvDip->writeReg(0x000033EC, 0x00008D42); /* 0x150253EC: DIP_B_GGM_LUT_G[107] */
        imageioPackage.pDrvDip->writeReg(0x000033F0, 0x00006921); /* 0x150253F0: DIP_B_GGM_LUT_G[108] */
        imageioPackage.pDrvDip->writeReg(0x000033F4, 0x0000AD72); /* 0x150253F4: DIP_B_GGM_LUT_G[109] */
        imageioPackage.pDrvDip->writeReg(0x000033F8, 0x00006E43); /* 0x150253F8: DIP_B_GGM_LUT_G[110] */
        imageioPackage.pDrvDip->writeReg(0x000033FC, 0x0000D9C8); /* 0x150253FC: DIP_B_GGM_LUT_G[111] */
        imageioPackage.pDrvDip->writeReg(0x00003400, 0x00008FBE); /* 0x15025400: DIP_B_GGM_LUT_G[112] */
        imageioPackage.pDrvDip->writeReg(0x00003404, 0x00005E0B); /* 0x15025404: DIP_B_GGM_LUT_G[113] */
        imageioPackage.pDrvDip->writeReg(0x00003408, 0x0000CBB1); /* 0x15025408: DIP_B_GGM_LUT_G[114] */
        imageioPackage.pDrvDip->writeReg(0x0000340C, 0x0000C41C); /* 0x1502540C: DIP_B_GGM_LUT_G[115] */
        imageioPackage.pDrvDip->writeReg(0x00003410, 0x000080D3); /* 0x15025410: DIP_B_GGM_LUT_G[116] */
        imageioPackage.pDrvDip->writeReg(0x00003414, 0x0000F698); /* 0x15025414: DIP_B_GGM_LUT_G[117] */
        imageioPackage.pDrvDip->writeReg(0x00003418, 0x0000F16F); /* 0x15025418: DIP_B_GGM_LUT_G[118] */
        imageioPackage.pDrvDip->writeReg(0x0000341C, 0x00009D18); /* 0x1502541C: DIP_B_GGM_LUT_G[119] */
        imageioPackage.pDrvDip->writeReg(0x00003420, 0x00006923); /* 0x15025420: DIP_B_GGM_LUT_G[120] */
        imageioPackage.pDrvDip->writeReg(0x00003424, 0x000009FA); /* 0x15025424: DIP_B_GGM_LUT_G[121] */
        imageioPackage.pDrvDip->writeReg(0x00003428, 0x0000CBF8); /* 0x15025428: DIP_B_GGM_LUT_G[122] */
        imageioPackage.pDrvDip->writeReg(0x0000342C, 0x0000E856); /* 0x1502542C: DIP_B_GGM_LUT_G[123] */
        imageioPackage.pDrvDip->writeReg(0x00003430, 0x00005476); /* 0x15025430: DIP_B_GGM_LUT_G[124] */
        imageioPackage.pDrvDip->writeReg(0x00003434, 0x00002008); /* 0x15025434: DIP_B_GGM_LUT_G[125] */
        imageioPackage.pDrvDip->writeReg(0x00003438, 0x0000E70F); /* 0x15025438: DIP_B_GGM_LUT_G[126] */
        imageioPackage.pDrvDip->writeReg(0x0000343C, 0x0000DAFB); /* 0x1502543C: DIP_B_GGM_LUT_G[127] */
        imageioPackage.pDrvDip->writeReg(0x00003440, 0x00001F75); /* 0x15025440: DIP_B_GGM_LUT_G[128] */
        imageioPackage.pDrvDip->writeReg(0x00003444, 0x0000D91F); /* 0x15025444: DIP_B_GGM_LUT_G[129] */
        imageioPackage.pDrvDip->writeReg(0x00003448, 0x00004430); /* 0x15025448: DIP_B_GGM_LUT_G[130] */
        imageioPackage.pDrvDip->writeReg(0x0000344C, 0x0000375E); /* 0x1502544C: DIP_B_GGM_LUT_G[131] */
        imageioPackage.pDrvDip->writeReg(0x00003450, 0x000027CB); /* 0x15025450: DIP_B_GGM_LUT_G[132] */
        imageioPackage.pDrvDip->writeReg(0x00003454, 0x0000D6E6); /* 0x15025454: DIP_B_GGM_LUT_G[133] */
        imageioPackage.pDrvDip->writeReg(0x00003458, 0x0000BD2D); /* 0x15025458: DIP_B_GGM_LUT_G[134] */
        imageioPackage.pDrvDip->writeReg(0x0000345C, 0x00004148); /* 0x1502545C: DIP_B_GGM_LUT_G[135] */
        imageioPackage.pDrvDip->writeReg(0x00003460, 0x0000E03E); /* 0x15025460: DIP_B_GGM_LUT_G[136] */
        imageioPackage.pDrvDip->writeReg(0x00003464, 0x0000B386); /* 0x15025464: DIP_B_GGM_LUT_G[137] */
        imageioPackage.pDrvDip->writeReg(0x00003468, 0x00005405); /* 0x15025468: DIP_B_GGM_LUT_G[138] */
        imageioPackage.pDrvDip->writeReg(0x0000346C, 0x00007CF4); /* 0x1502546C: DIP_B_GGM_LUT_G[139] */
        imageioPackage.pDrvDip->writeReg(0x00003470, 0x0000810A); /* 0x15025470: DIP_B_GGM_LUT_G[140] */
        imageioPackage.pDrvDip->writeReg(0x00003474, 0x0000AF77); /* 0x15025474: DIP_B_GGM_LUT_G[141] */
        imageioPackage.pDrvDip->writeReg(0x00003478, 0x000032BE); /* 0x15025478: DIP_B_GGM_LUT_G[142] */
        imageioPackage.pDrvDip->writeReg(0x0000347C, 0x000019F6); /* 0x1502547C: DIP_B_GGM_LUT_G[143] */
        imageioPackage.pDrvDip->writeReg(0x00003480, 0x00000001); /* 0x15025480: DIP_B_GGM_CTRL */
    }
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
    DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_C02B_CON, 0x0f);
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
	if (imageioPackage.isApplyTuning == MTRUE){
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_CON,  imageioPackage.pTuningIspReg->DIP_X_MFB_CON.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON1,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON2,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON3,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON3.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON4,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON4.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_EDGE,  imageioPackage.pTuningIspReg->DIP_X_MFB_EDGE.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON5,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON5.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON6,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON6.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON7,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON7.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON8,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON8.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON9,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON9.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_LL_CON10,  imageioPackage.pTuningIspReg->DIP_X_MFB_LL_CON10.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON0,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON0.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON1,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON1.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON2,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON2.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON3,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON3.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON4,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON4.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON5,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON5.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON6,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON6.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON7,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON7.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON8,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON8.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON9,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON9.Raw);
        DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_MFB_MBD_CON10,  imageioPackage.pTuningIspReg->DIP_X_MFB_MBD_CON10.Raw);

	}

	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_CON,BLD_MODE,imageioPackage.mfb_cfg.bld_mode);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_LL_CON3,BLD_LL_OUT_XSIZE,imageioPackage.mfb_cfg.mfb_out.w);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_LL_CON3,BLD_LL_OUT_YSIZE,imageioPackage.mfb_cfg.mfb_out.h);
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_MFB_LL_CON3,BLD_LL_OUT_XOFST,imageioPackage.mfb_cfg.out_xofst);

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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_G2C_CONV_0A,size, (MUINT32*)imageioPackage.pTuningIspReg);

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
        //DIP_WRITE_REG(imageioPackage.pDrvDip, DIP_X_G2C_CFC_CON_2,  0x29485294);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2B_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2C_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2D_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2E_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2K_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2G_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2H_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
        DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SL2I_CEN,size, (MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}
MINT32
IspDrv_B::
setHlr2(DIP_ISP_PIPE &imageioPackage)
{
	if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_HLR2_1,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_HLR2_GAIN,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_RNR_CON1,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_HFG_CON_0,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_FLC_OFFST0,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_FLC2_OFFST0,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_ANR_TBL,size, (MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setNbc2(DIP_ISP_PIPE &imageioPackage)
{
//ANR2 and Bok : should choose one
	if (imageioPackage.isApplyTuning == MTRUE){
        MUINT32 addr_ANR2,size_ANR2;
        MUINT32 addr_CCR,size_CCR;
        MUINT32 addr_BOK,size_BOK;
        MUINT32 addr_ABF,size_ABF;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_ANR2,addr_ANR2,size_ANR2);
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_CCR,addr_CCR,size_CCR);
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_BOK,addr_BOK,size_BOK);
        imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_ABF,addr_ABF,size_ABF);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_ANR2_CON1,size_ANR2, (MUINT32*)imageioPackage.pTuningIspReg);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_CCR_CON,size_CCR, (MUINT32*)imageioPackage.pTuningIspReg);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_BOK_CON,size_BOK, (MUINT32*)imageioPackage.pTuningIspReg);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_ABF_CON1,size_ABF, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_NDG_RAN_0,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_NDG2_RAN_0,size, (MUINT32*)imageioPackage.pTuningIspReg);
	}
    return 0;
}

MINT32
IspDrv_B::
setSrz1(DIP_ISP_PIPE &imageioPackage)
{
    DIP_FUNC_DBG("setSrz1 ctrl(0x%x) in(%d/%d), crop(%d_0x%x/%d_0x%x), out(%d/%d)",imageioPackage.srz1Cfg.ctrl, imageioPackage.srz1Cfg.inout_size.in_w, imageioPackage.srz1Cfg.inout_size.in_h,\
        imageioPackage.srz1Cfg.crop.x, imageioPackage.srz1Cfg.crop.floatX, imageioPackage.srz1Cfg.crop.y, imageioPackage.srz1Cfg.crop.floatY,\
        imageioPackage.srz1Cfg.inout_size.out_w, imageioPackage.srz1Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_CONTROL,imageioPackage.srz1Cfg.ctrl);//.ctrl);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_IN_IMG,SRZ_IN_WD,imageioPackage.srz1Cfg.inout_size.in_w);//in_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_IN_IMG,SRZ_IN_HT,imageioPackage.srz1Cfg.inout_size.in_h);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_OUT_IMG,SRZ_OUT_WD,imageioPackage.srz1Cfg.inout_size.out_w);
    DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_SRZ1_OUT_IMG,SRZ_OUT_HT,imageioPackage.srz1Cfg.inout_size.out_h);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_SRZ1_HORI_STEP,imageioPackage.srz1Cfg.h_step);//.h_step);
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
		DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MIX3_CTRL_0,imageioPackage.pTuningIspReg->DIP_X_MIX3_CTRL_0.Raw);
		DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MIX3_CTRL_1,imageioPackage.pTuningIspReg->DIP_X_MIX3_CTRL_1.Raw);
		DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MIX3_SPARE,imageioPackage.pTuningIspReg->DIP_X_MIX3_SPARE.Raw);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_PCA_TBL,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_SEEE_CTRL,size, (MUINT32*)imageioPackage.pTuningIspReg);
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
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_CRSP_CTRL,CRSP_CROP_EN,imageioPackage.crspCfg.enCrsp);
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

        MUINT32 addr_NR3D,size_NR3D;
        MUINT32 addr_COLOR,size_COLOR;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_NR3D,addr_NR3D,size_NR3D);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_CAM_TNR_ENG_CON,size_NR3D, (MUINT32*)imageioPackage.pTuningIspReg);

        //if ((imageioPackage.pTuningIspReg->DIP_X_CAM_COLOR_START.Bits.COLOR_DISP_COLOR_START & 0x1) > 0)
        {
            imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_COLOR,addr_COLOR,size_COLOR);
            DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_CAM_COLOR_CFG_MAIN,size_COLOR, (MUINT32*)imageioPackage.pTuningIspReg);
        }

	}


	return 0;
}

MINT32
IspDrv_B::
setFe(DIP_ISP_PIPE &imageioPackage)
{
    DIP_FUNC_DBG("setG2c in(%d)",imageioPackage.isApplyTuning);
    if (imageioPackage.isApplyTuning == MTRUE)
    {
        if (imageioPackage.isApplyTuning == MTRUE){
		MUINT32 addr,size;
		imageioPackage.pDrvDip->getCQModuleInfo(DIP_A_FE,addr,size);
		DIP_WRITE_REGS(imageioPackage.pDrvDip, DIP_X_FE_CTRL1,size, (MUINT32*)imageioPackage.pTuningIspReg);
	}
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
	DIP_WRITE_BITS(imageioPackage.pDrvDip,DIP_X_C02_CON,C02_TPIPE_EDGE,0xF); //default 0xf, tpipemain would update it
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
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_FM_SIZE,imageioPackage.pTuningIspReg->DIP_X_FM_SIZE.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_FM_TH_CON0,imageioPackage.pTuningIspReg->DIP_X_FM_TH_CON0.Raw);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DMGI_CON,0x10000019);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DMGI_CON2,0x00160010);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DMGI_CON3,0x00000000);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DEPI_CON,0x20000020);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DEPI_CON2,0x001B000F);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_DEPI_CON3,0x001F000C);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MFBO_CON,0x80000040);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MFBO_CON2,0x000A0008);
    DIP_WRITE_REG(imageioPackage.pDrvDip,DIP_X_MFBO_CON3,0x00080008);

    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    IspDrvShellImp
/////////////////////////////////////////////////////////////////////////////*/
class IspDrvShellImp:public IspDrvShell
{
    public:
        IspDrvShellImp(){}

        static IspDrvShell* getInstance();
        virtual void        destroyInstance(void);
        virtual MBOOL       init(const char* userName="", MUINT32 secTag=0);
        virtual MBOOL       uninit(const char* userName="");
        virtual DipRingBuffer* getDipRingBufMgr();

    public://driver object operation
//        inline virtual IspDrv*      getPhyIspDrv(){return m_pPhyIspDrv_bak; }

    public://phy<->virt ISP switch
//        inline MBOOL       ispDrvSwitch2Virtual(MINT32 cq,MINT32 P1CqD,MINT32 burstQIdx,MINT32 P2DupCqIdx);
//        inline IspDrv*     ispDrvGetCQIspDrv(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx);

    public://commandQ operation
//        virtual MBOOL       cam_cq_cfg(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx,MINT32 isP2IspOnly=0);
//        virtual MBOOL       cqAddModule(E_ISP_DIP_CQ cq,MINT32 burstQIdx,MINT32 dupCqIdx, DIP_A_MODULE_ENUM moduleId);
//        virtual MBOOL       cqDelModule(E_ISP_DIP_CQ cq,MINT32 burstQIdx,MINT32 dupCqIdx, DIP_A_MODULE_ENUM moduleId);
//        virtual int         getCqModuleInfo(E_ISP_DIP_CQ cq,MINT32 burstQIdx,MINT32 dupCqIdx, DIP_A_MODULE_ENUM moduleId);
    protected:
        //volatile MINT32 mInitCount;
        static volatile MINT32 mInitCount;

public:
    //tpipe
//    IMEM_BUF_INFO  tpipeBufInfo[ISP_DVR_MAX_BURST_QUEUE_NUM][ISP_DRV_DIP_CQ_NUM][ISP_DRV_P2_CQ_DUPLICATION_NUM];
};

//
IspDrvShell* IspDrvShell::createInstance()
{
    return IspDrvShellImp::getInstance();
}
//
//
volatile MINT32 IspDrvShellImp::mInitCount = 0;

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


MBOOL
IspDrvShellImp::
init(const char* userName, MUINT32 secTag)
{
    MINT32 ret = MTRUE;
    MINT32 i, j, k, l;
    MUINTPTR tmpVaddr = 0, tmpPaddr = 0;
    MUINT32 offset;
    Mutex::Autolock lock(mLock);

    DIP_FUNC_INF("+, userName(%s), mInitCount(%d)", userName, this->mInitCount);
    m_p2disableSkipTpipe = ::property_get_int32("vendor.camera.disable_p2skiptpipe", 0);
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
    m_pIMemDrv = IMemDrv::createInstance();
    DIP_FUNC_DBG("[m_pIMemDrv]m_pIMemDrv(%lx)",(unsigned long)m_pIMemDrv);
    if ( NULL == m_pIMemDrv ) {
        DIP_FUNC_ERR("[Error]IMemDrv::createInstance fail.");
        ret = MFALSE;
        goto EXIT;
    }
    m_pIMemDrv->init();


    /*============================================
        ispDrvDip & ispDrvDipPhy driver
    =============================================*/
    for(i=DIP_A; i<DIP_MAX; i++) {
        MUINT32 moduleIdx;
        moduleIdx = i - DIP_START;
        // generate IspDrvDipPhy obj
        m_pDrvDipPhy = (IspDrvDipPhy*)IspDrvDipPhy::createInstance((ISP_HW_MODULE)i);
        m_pDrvDipPhy->init("IspDrvShellImp");
        //
        // generate IspDrvDip obj
        m_pDrvDip[moduleIdx] = IspDrvDip::createInstance((ISP_HW_MODULE)i);
        m_pDrvDip[moduleIdx]->init("IspDrvShellImp");
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
    m_pDipRingBuf = DipRingBuffer::createInstance(ISP_DIP_MODULE_IDX_DIP_A);
    if(m_pDipRingBuf)
    {
        ret=m_pDipRingBuf->init(secTag);
    }
    //Initial Debug Dump Buffer
    m_pDipRingBuf->getTpipeBuf(tmpVaddr, tmpPaddr);
    m_pDrvDipPhy->setMemInfo(ISP_MEMORY_INFO_TPIPE_CMD, (unsigned long)tmpPaddr,(unsigned int *)tmpVaddr,MAX_ISP_TILE_TDR_HEX_NO);
    m_pDipRingBuf->getCmdqBuf(tmpVaddr, tmpPaddr);
    m_pDrvDipPhy->setMemInfo(ISP_MEMORY_INFO_CMDQ_CMD, (unsigned long)tmpPaddr,(unsigned int *)tmpVaddr,MAX_DIP_CMDQ_BUFFER_SIZE);

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
    DIP_FUNC_INF("gFrameCQCfgInfoList size(%lu)",(unsigned long)gFrameCQCfgInfoList.size());


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
    for(i=DIP_A; i<DIP_MAX; i++) {
        MUINT32 moduleIdx;
        moduleIdx = i - DIP_START;
        // release IspDrvDipPhy obj
        m_pDrvDipPhy->uninit("IspDrvShellImp");
        m_pDrvDipPhy->destroyInstance();
        //
        // release IspDrvDip obj
        m_pDrvDip[moduleIdx]->uninit("IspDrvShellImp");
        m_pDrvDip[moduleIdx]->destroyInstance();
    }

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
    //
    for (list<_CQ_CONFIG_INFO_>::iterator it = gFrameCQCfgInfoList.begin(); it != gFrameCQCfgInfoList.end();)
    {
        it = gFrameCQCfgInfoList.erase(it);
        it++;
    }

    //
    DIP_FUNC_INF("crz unint start");
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
//
DipRingBuffer* IspDrvShellImp::getDipRingBufMgr()
{
    return this->m_pDipRingBuf;
}


/*/////////////////////////////////////////////////////////////////////////////
    IspFunctionDip_B
  /////////////////////////////////////////////////////////////////////////////*/
IspDrvShell*        IspFunctionDip_B::m_pIspDrvShell = NULL;
MBOOL IspFunctionDip_B::isDipOnly;
unsigned int IspFunctionDip_B::dipTh;
unsigned int IspFunctionDip_B::hwModule;
unsigned int IspFunctionDip_B::moduleIdx;
unsigned int IspFunctionDip_B::dupCqIdx;
unsigned int IspFunctionDip_B::burstQueIdx; // for burst queue number
unsigned int IspFunctionDip_B::RingBufIdx;
MBOOL IspFunctionDip_B::isApplyTuning;
dip_x_reg_t* IspFunctionDip_B::pTuningIspReg;  // p2 tuning data set by tuning provider
unsigned int IspFunctionDip_B::isSecureFra;
//
MUINT32
IspFunctionDip_B::
switchHwModuleToHwIdx(ISP_HW_MODULE hwModule)
{
    MUINT32 moduleIdx;
    //
    if(hwModule<DIP_MAX && hwModule>=DIP_A){
        moduleIdx = hwModule - DIP_START;
        DIP_FUNC_VRB("moduleIdx(%d),hwModule(%d)",moduleIdx,hwModule);
    } else {
        DIP_FUNC_ERR("[Error]hwModule(%d) out of the range",hwModule);
        return 0;
    }
    return moduleIdx;
}

//////
/*/////////////////////////////////////////////////////////////////////////////
    DMA
  /////////////////////////////////////////////////////////////////////////////*/
int DIP_DMA::config( void )
{
    MINT32 ret = MTRUE;
    IspDrvDip*   pDrvDip;
    MUINT32 ufd_en;

    DIP_FUNC_DBG("[DIP_DMA (%lu)] hwModule(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d)",\
        this->id(), this->hwModule,this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx);

    pDrvDip = m_pIspDrvShell->m_pDrvDip[this->moduleIdx];

    switch(this->id())
    {
        //in-dma
        case ENUM_DIP_DMA_TDRI:
            ///kk test need to add a function to get tile base address from isp_function
            DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_TDRI_BASE_ADDR,0x00);  //kk test
            DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_TDRI_OFST_ADDR,0x00);
            break;
        case ENUM_DIP_DMA_IMGI:
            //DIP_WRITE_REG(pDrvDip, DIP_X_IMGI_BASE_ADDR, (this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr));
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
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_UFDI_CON3,0x00200020); //default:0x00200020
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
        case ENUM_DIP_DMA_MFBO:
            DIP_WRITE_REG(pDrvDip,DIP_X_MFBO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            DIP_WRITE_REG(pDrvDip,DIP_X_MFBO_OFST_ADDR,0x00);
            DIP_WRITE_REG(pDrvDip,DIP_X_MFBO_XSIZE,this->dma_cfg.size.xsize - 1);
            DIP_WRITE_REG(pDrvDip,DIP_X_MFBO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            DIP_WRITE_BITS(pDrvDip, DIP_X_MFBO_STRIDE,STRIDE,this->dma_cfg.size.stride);
            DIP_WRITE_BITS(pDrvDip, DIP_X_MFBO_STRIDE,BUS_SIZE,this->dma_cfg.bus_size);
            DIP_WRITE_BITS(pDrvDip, DIP_X_MFBO_STRIDE,FORMAT,this->dma_cfg.format);
            DIP_WRITE_BITS(pDrvDip, DIP_X_MFBO_STRIDE,FORMAT_EN,this->dma_cfg.format_en);
            DIP_WRITE_BITS(pDrvDip, DIP_X_MFBO_STRIDE,BUS_SIZE_EN,this->dma_cfg.bus_size_en);
            //
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_MFBO_CON,0x80000040); //default:0x80000040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_MFBO_CON2,0x00400040); //default:0x00400040
            //DIP_WRITE_PHY_REG(m_pIspDrvShell->m_pDrvDipPhy,DIP_X_MFBO_CON3,0x00400040); //default:0x00400040
            //
            DIP_WRITE_REG(pDrvDip,DIP_X_MFBO_CROP,(this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
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
            //DIP_WRITE_PHY_REG(this->hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_IMG3BO_CON,0x80000020); //default:0x80000040
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
            DIP_FUNC_ERR("[Error]Not support this dip thread(%d)",this->dipTh);
            ret = MFALSE;
            goto EXIT;
            break;
    }
EXIT:
    return ret;
}

int DIP_DMA::write2CQ(void)
{
    MINT32 ret = MTRUE;
    MUINT32 cqThrModuleId = 0xff;
    MUINT32 cqThrModuleId2 = 0xff;
    //
    DIP_FUNC_DBG("[DIP_DMA]hwModule(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d)",\
        this->hwModule,this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx);

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
        case ENUM_DIP_DMA_MFBO:
            cqThrModuleId = DIP_A_MFBO;
            cqThrModuleId2 = DIP_A_MFBO_CROP;
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

    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(cqThrModuleId);
    if(cqThrModuleId2 != 0xff)
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(cqThrModuleId2);

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

    DIP_X_REG_CTL_YUV_EN *pYuvEn;
    DIP_X_REG_CTL_YUV2_EN *pYuv2En;
    DIP_X_REG_CTL_RGB_EN *pRgbEn;
    DIP_X_REG_CTL_DMA_EN *pDmaEn;
    DIP_X_REG_CTL_FMT_SEL *pFmtSel;
    DIP_X_REG_CTL_PATH_SEL *pPathSel;
    DIP_X_REG_CTL_MISC_SEL *pMiscSel;
    DIP_X_REG_CTL_TDR_CTL *pTdrCtl;
    dip_x_reg_t *tuningIspReg;
    dip_x_reg_t tmpTuningIspReg;

    ISP_TPIPE_CONFIG_STRUCT *pTdri = NULL;

    MUINT32 cqSecHdl, tpipeSecHdl;
    MUINT32 cqSecSize, tpipeSecSize;
    MUINT32 cqDescSize;
    //
    m_pDipWBuf->m_pMdpCfg->pDumpRegBuffer = this->pRegiAddr;
    pTdri = &m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo;
    pTdri->drvinfo.DesCqPa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getCQDescBufPhyAddr();
    pTdri->drvinfo.DesCqVa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getCQDescBufVirAddr();
    pTdri->drvinfo.pIspVirRegAddr_pa = m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegPhyAddr();
    pTdri->drvinfo.pIspVirRegAddr_va = m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegVirAddr();
    pTdri->drvinfo.VirtRegPa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegPhyAddr();
    pTdri->drvinfo.VirtRegVa = (unsigned long)m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->getIspVirRegVirAddr();
    pTdri->drvinfo.dupCqIdx = this->dupCqIdx;
    pTdri->drvinfo.burstCqIdx = this->burstQueIdx;
    pTdri->drvinfo.cqIdx = (E_ISP_DIP_CQ)this->dipTh;
    //Update IMGI Address
    pTdri->drvinfo.imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr + this->DMAImgi.dma_cfg.memBuf.ofst_addr;
#if 0
    if(function_Dip_DbgLogEnable_DEBUG==true)
        pDst->drvinfo.debugRegDump = 1;
    else
        pDst->drvinfo.debugRegDump = 0;
#else
    pTdri->drvinfo.debugRegDump = 0;
#endif

    pTdri->drvinfo.regCount = this->regCount;
    ::memcpy(pTdri->drvinfo.ReadAddrList, this->pReadAddrList, this->regCount*sizeof(int));

    // DIP settings for GCE cmd
    if (this->isSecureFra == 1)
    {
        pTdri->drvinfo.dip_ctl_yuv_en = this->isp_top_ctl.YUV_EN.Raw;
        pTdri->drvinfo.dip_ctl_yuv2_en = this->isp_top_ctl.YUV2_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb_en = this->isp_top_ctl.RGB_EN.Raw;
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
        pTdri->drvinfo.dip_mfbo_base_addr = this->DMAMfbo.dma_cfg.memBuf.base_pAddr + this->DMAMfbo.dma_cfg.memBuf.ofst_addr;
        pTdri->drvinfo.dip_mfbo_size[0] = this->DMAMfbo.dma_cfg.memBuf.size;
        pTdri->drvinfo.dip_mfbo_size[1] = pTdri->drvinfo.dip_mfbo_size[2] = 0;
        if (this->DMAImgi.dma_cfg.secureTag != 0)
        {
            pTdri->drvinfo.dip_imgi_base_addr = this->DMAImgi.dma_cfg.memBuf.base_pAddr;
            pTdri->drvinfo.dip_imgbi_base_addr = this->DMAImgi.dma_cfg.memBuf.size;
            pTdri->drvinfo.dip_imgbi_base_vaddr = 0;
            pTdri->drvinfo.dip_imgci_base_addr = this->DMAImgi.dma_cfg.memBuf.size + this->DMAImgbi.dma_cfg.memBuf.size;
            pTdri->drvinfo.dip_imgci_base_vaddr = 0;
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
        pTdri->drvinfo.dip_mfbo_secure_tag = this->DMAMfbo.dma_cfg.secureTag;
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
        m_pIspDrvShell->m_pDipRingBuf->getSecureBufHandle(cqSecHdl,tpipeSecHdl);
        m_pIspDrvShell->m_pDipRingBuf->getSecureBufSize(cqSecSize,tpipeSecSize);
        m_pIspDrvShell->m_pDipRingBuf->getCqDescBufSize(cqDescSize);
        pTdri->drvinfo.cqSecHdl = cqSecHdl;
        pTdri->drvinfo.tpipeTableSecHdl = tpipeSecHdl;
        pTdri->drvinfo.cqSecSize = cqSecSize;
        pTdri->drvinfo.tpipeTableSecSize = tpipeSecSize;
        pTdri->drvinfo.DesCqSize = cqDescSize;
    }
    else
    {
        pTdri->drvinfo.dip_ctl_yuv_en = this->isp_top_ctl.YUV_EN.Raw;
        pTdri->drvinfo.dip_ctl_yuv2_en = this->isp_top_ctl.YUV2_EN.Raw;
        pTdri->drvinfo.dip_ctl_rgb_en = this->isp_top_ctl.RGB_EN.Raw;
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
        pTdri->drvinfo.dip_mfbo_base_addr = this->DMAMfbo.dma_cfg.memBuf.base_pAddr + this->DMAMfbo.dma_cfg.memBuf.ofst_addr;
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
        pTdri->drvinfo.cqSecSize = 0;
        pTdri->drvinfo.tpipeTableSecSize = 0;
        pTdri->drvinfo.DesCqSize = 0;
        pTdri->drvinfo.dip_imgbi_base_vaddr = 0;
        pTdri->drvinfo.dip_imgci_base_vaddr	= 0;
        pTdri->drvinfo.dip_dmgi_base_vaddr = 0;
        pTdri->drvinfo.dip_depi_base_vaddr = 0;
        pTdri->drvinfo.dip_lcei_base_vaddr = 0;
    }

    DIP_FUNC_DBG("VirCqPa(0x%lx),dupCqIdx(%d)",pTdri->drvinfo.DesCqPa,pTdri->drvinfo.dupCqIdx);

    if (this->tdr_ctl&DIP_X_REG_CTL_TDR_RN)
    {
        //TPIPE Mode
        m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x0;
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_CTL,this->tdr_ctl);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TILE,this->tdr_tpipe);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TCM_EN,this->tdr_tcm_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TCM2_EN,this->tdr_tcm2_en);
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_TCM3_EN,this->tdr_tcm3_en);
	}
    else
    {
        //Frame Mode
        DIP_WRITE_REG(m_pIspDrvShell->m_pDrvDip[this->moduleIdx],DIP_X_CTL_TDR_CTL,0x0);
        m_pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x2;
        return ret;
    }

    //
    pYuvEn = (DIP_X_REG_CTL_YUV_EN*)&(this->isp_top_ctl.YUV_EN);
    pYuv2En = (DIP_X_REG_CTL_YUV2_EN*)&(this->isp_top_ctl.YUV2_EN);
    pRgbEn = (DIP_X_REG_CTL_RGB_EN*)&(this->isp_top_ctl.RGB_EN);
    pDmaEn = (DIP_X_REG_CTL_DMA_EN*)&(this->isp_top_ctl.DMA_EN);
    pFmtSel = (DIP_X_REG_CTL_FMT_SEL*)&(this->isp_top_ctl.FMT_SEL);
    pPathSel = (DIP_X_REG_CTL_PATH_SEL*)&(this->isp_top_ctl.PATH_SEL);
    pMiscSel = (DIP_X_REG_CTL_MISC_SEL*)&(this->isp_top_ctl.MISC_EN);
    pTdrCtl = (DIP_X_REG_CTL_TDR_CTL*)&(this->tdr_ctl);
    if(this->isApplyTuning==MTRUE) {
        tuningIspReg = this->pTuningIspReg;
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
    pTdri->top.ufdi_fmt = pFmtSel->Bits.UFDI_FMT;
    pTdri->top.vipi_fmt = pFmtSel->Bits.VIPI_FMT;
    pTdri->top.img3o_fmt = pFmtSel->Bits.IMG3O_FMT;
    pTdri->top.img2o_fmt = pFmtSel->Bits.IMG2O_FMT;
    pTdri->top.pak2_fmt = pMiscSel->Bits.PAK2_FMT;
    pTdri->top.imgi_en = pDmaEn->Bits.IMGI_EN;
    pTdri->top.imgbi_en = pDmaEn->Bits.IMGBI_EN;
    pTdri->top.imgci_en = pDmaEn->Bits.IMGCI_EN;
    pTdri->top.ufdi_en = pDmaEn->Bits.UFDI_EN;
    pTdri->top.ufo_imgi_en = pRgbEn->Bits.UFD_EN;
    pTdri->top.unp_en = pRgbEn->Bits.UNP_EN;
    pTdri->top.ufd_en = pRgbEn->Bits.UFD_EN;
    pTdri->top.ufd_crop_en = 0; // Paul will remove it later.
    pTdri->top.bnr_en = pRgbEn->Bits.BNR2_EN;
    pTdri->top.lsc2_en = pRgbEn->Bits.LSC2_EN;
    pTdri->top.rcp2_en = pRgbEn->Bits.RCP2_EN;
    pTdri->top.pak2_en = pRgbEn->Bits.PAK2_EN;
    pTdri->top.hlr2_en = pRgbEn->Bits.HLR2_EN;
    pTdri->top.c24_en = pYuvEn->Bits.C24_EN;
    pTdri->top.sl2_en = pRgbEn->Bits.SL2_EN;
    pTdri->top.udm_en = pRgbEn->Bits.UDM_EN;
    pTdri->top.vipi_en = pDmaEn->Bits.VIPI_EN;
    pTdri->top.vip2i_en = pDmaEn->Bits.VIP2I_EN;
    pTdri->top.vip3i_en = pDmaEn->Bits.VIP3I_EN;
    pTdri->top.mfb_en = pYuvEn->Bits.MFB_EN;
    pTdri->top.mfbw_en = pYuvEn->Bits.MFBW_EN;
    pTdri->top.mfbo_en = pDmaEn->Bits.MFBO_EN;
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
    pTdri->top.seee_en = pYuvEn->Bits.SEEE_EN;
    pTdri->top.lcei_en = pDmaEn->Bits.LCEI_EN;
    pTdri->top.lce_en = pRgbEn->Bits.LCE_EN;
    pTdri->top.mix3_en = pYuvEn->Bits.MIX3_EN;
    pTdri->top.crz_en = pYuvEn->Bits.CRZ_EN;
    pTdri->top.img2o_en = pDmaEn->Bits.IMG2O_EN;
    pTdri->top.img2bo_en = pDmaEn->Bits.IMG2BO_EN;
    pTdri->top.fe_en = pYuvEn->Bits.FE_EN;
    pTdri->top.feo_en = pDmaEn->Bits.FEO_EN;
    pTdri->top.c02_en = pYuvEn->Bits.C02_EN;
    pTdri->top.c02b_en = pYuvEn->Bits.C02B_EN;
    pTdri->top.nr3d_en = pYuvEn->Bits.NR3D_EN;
    pTdri->top.crsp_en = pYuvEn->Bits.CRSP_EN;
    pTdri->top.img3o_en = pDmaEn->Bits.IMG3O_EN;
    pTdri->top.img3bo_en = pDmaEn->Bits.IMG3BO_EN;
    pTdri->top.img3co_en = pDmaEn->Bits.IMG3CO_EN;
    pTdri->top.c24b_en = pYuvEn->Bits.C24B_EN;
    pTdri->top.mdp_crop_en = pYuvEn->Bits.MDPCROP_EN;
    pTdri->top.plnr1_en = pYuvEn->Bits.PLNR1_EN;
    pTdri->top.plnr2_en = pYuvEn->Bits.PLNR2_EN;
    pTdri->top.plnw1_en = pYuvEn->Bits.PLNW1_EN;
    pTdri->top.plnw2_en = pYuvEn->Bits.PLNW2_EN;
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
    pTdri->top.ggm_en = pRgbEn->Bits.GGM_EN;
    pTdri->top.nbcw_sel = pPathSel->Bits.NBCW_SEL;
    pTdri->top.nbc_sel = pPathSel->Bits.NBC_SEL;
    pTdri->top.rcp2_sel = pPathSel->Bits.RCP2_SEL;
    pTdri->top.feo_sel = pPathSel->Bits.FEO_SEL;
    pTdri->top.mfbo_sel = pMiscSel->Bits.MFBO_SEL;
    pTdri->top.rnr_en = pRgbEn->Bits.RNR_EN;
    pTdri->top.sl2g_en = pRgbEn->Bits.SL2G_EN;
    pTdri->top.sl2h_en = pRgbEn->Bits.SL2H_EN;
    pTdri->top.srz5_en = pYuv2En->Bits.SRZ5_EN;
    pTdri->top.hfg_en = pYuv2En->Bits.HFG_EN;
    pTdri->top.sl2i_en = pYuv2En->Bits.SL2I_EN;
    pTdri->top.sl2k_en = pRgbEn->Bits.SL2K_EN;
    pTdri->top.flc_en = pRgbEn->Bits.FLC_EN;
    pTdri->top.flc2_en = pRgbEn->Bits.FLC2_EN;
    pTdri->top.ndg_en = pYuv2En->Bits.NDG_EN;
    pTdri->top.ndg2_en = pYuv2En->Bits.NDG2_EN;

    pTdri->top.crsp_sel = pPathSel->Bits.CRSP_SEL;

    //
    pTdri->sw.log_en = 1;
    pTdri->sw.src_width = DMAImgi.dma_cfg.size.w;
    pTdri->sw.src_height = DMAImgi.dma_cfg.size.h;
    pTdri->sw.tpipe_irq_mode = TPIPE_IRQ_TPIPE;
    pTdri->sw.tpipe_width = MAX_TPIPE_WIDTH;
    pTdri->sw.tpipe_height = MAX_TPIPE_HEIGHT;
    pTdri->sw.tpipe_sel_mode = 0x1; /* Paul suggested setting 1 for this */
    //
    pTdri->imgi.imgi_v_flip_en = DMAImgi.dma_cfg.v_flip_en;
    pTdri->imgi.imgi_stride = DMAImgi.dma_cfg.size.stride;
    //
    pTdri->imgbi.imgbi_v_flip_en = DMAImgbi.dma_cfg.v_flip_en;
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
    pTdri->mfb.bld_deblock_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_LL_DB_EN;
    pTdri->mfb.bld_brz_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_LL_BRZ_EN;
    pTdri->mfb.bld_mbd_wt_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_MBD_WT_EN;
    //
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
    //
    pTdri->g2c.g2c_shade_en = tuningIspReg->DIP_X_G2C_SHADE_CON_1.Bits.G2C_SHADE_EN;
    pTdri->g2c.g2c_shade_xmid = tuningIspReg->DIP_X_G2C_SHADE_TAR.Bits.G2C_SHADE_XMID;
    pTdri->g2c.g2c_shade_ymid = tuningIspReg->DIP_X_G2C_SHADE_TAR.Bits.G2C_SHADE_YMID;
    pTdri->g2c.g2c_shade_var = tuningIspReg->DIP_X_G2C_SHADE_CON_1.Bits.G2C_SHADE_VAR;
    //
    pTdri->sl2b.sl2b_hrz_comp = tuningIspReg->DIP_X_SL2B_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2b.sl2b_vrz_comp = tuningIspReg->DIP_X_SL2B_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->nbc.anr_eny = tuningIspReg->DIP_X_ANR_CON1.Bits.ANR_ENY;
    pTdri->nbc.anr_enc = tuningIspReg->DIP_X_ANR_CON1.Bits.ANR_ENC;
    pTdri->nbc.anr_scale_mode = tuningIspReg->DIP_X_ANR_CON1.Bits.ANR_SCALE_MODE;
    //
    pTdri->nbc2.anr2_eny = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_ENY;
    pTdri->nbc2.anr2_enc = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_ENC;
    pTdri->nbc2.anr2_scale_mode = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_SCALE_MODE;
    pTdri->nbc2.anr2_mode = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_MODE;
    pTdri->nbc2.anr2_bok_mode = tuningIspReg->DIP_X_BOK_CON.Bits.BOK_MODE;
    pTdri->nbc2.anr2_bok_pf_en = tuningIspReg->DIP_X_BOK_CON.Bits.BOK_PF_EN;
    pTdri->nbc2.abf_en = tuningIspReg->DIP_X_ABF_CON1.Bits.ABF_EN;
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
    pTdri->sl2k.sl2k_hrz_comp = tuningIspReg->DIP_X_SL2K_RZ.Bits.SL2_HRZ_COMP;
    pTdri->sl2k.sl2k_vrz_comp = tuningIspReg->DIP_X_SL2K_RZ.Bits.SL2_VRZ_COMP;
    //
    pTdri->hfg.non_block_base = 0;   // need check with DE (It's cmodel parameter, don't care.)
    //
    pTdri->ndg.non_block_base = 0;
    pTdri->ndg2.non_block_base = 0;
    //
    pTdri->seee.se_edge = tuningIspReg->DIP_X_SEEE_OUT_EDGE_CTRL.Bits.SEEE_OUT_EDGE_SEL;

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
    pTdri->lce.lce_hlr_mode = tuningIspReg->DIP_X_HLR2_CFG.Bits.HLR_MODE;
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
        pTdri->srz4.srz_input_crop_width = srz4Cfg.inout_size.in_w;
        pTdri->srz4.srz_input_crop_height = srz4Cfg.inout_size.in_h;
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
    if(pTdri->top.srz5_en)
    {
        pTdri->srz5.srz_input_crop_width = srz5Cfg.inout_size.in_w;
        pTdri->srz5.srz_input_crop_height = srz5Cfg.inout_size.in_h;
        pTdri->srz5.srz_output_width = srz5Cfg.inout_size.out_w;
        pTdri->srz5.srz_output_height = srz5Cfg.inout_size.out_h;
        pTdri->srz5.srz_luma_horizontal_integer_offset = srz5Cfg.crop.x;
        pTdri->srz5.srz_luma_horizontal_subpixel_offset = srz5Cfg.crop.floatX;
        pTdri->srz5.srz_luma_vertical_integer_offset = srz5Cfg.crop.y;
        pTdri->srz5.srz_luma_vertical_subpixel_offset = srz5Cfg.crop.floatY;
        pTdri->srz5.srz_horizontal_coeff_step = srz5Cfg.h_step;
        pTdri->srz5.srz_vertical_coeff_step = srz5Cfg.v_step;
    }
    else
    {
        memset((void*)&pTdri->srz5, 0x00, sizeof(ISP_TPIPE_CONFIG_SRZ_STRUCT));
    }

    //
    pTdri->fe.fe_mode = tuningIspReg->DIP_X_FE_CTRL1.Bits.FE_MODE;
    DIP_FUNC_DBG("fe_mode(%d/%d)",pTdri->fe.fe_mode, tuningIspReg->DIP_X_FE_CTRL1.Bits.FE_MODE);
    //
    pTdri->feo.feo_stride = DMAFeo.dma_cfg.size.stride;
    //
    pTdri->nr3d.nr3d_on_en = tuningIspReg->DIP_X_NR3D_ON_CON.Bits.NR3D_ON_EN;
    pTdri->nr3d.nr3d_on_xoffset = tuningIspReg->DIP_X_NR3D_ON_OFF.Bits.NR3D_ON_OFST_X;
    pTdri->nr3d.nr3d_on_yoffset = tuningIspReg->DIP_X_NR3D_ON_OFF.Bits.NR3D_ON_OFST_Y;
    pTdri->nr3d.nr3d_on_width = tuningIspReg->DIP_X_NR3D_ON_SIZ.Bits.NR3D_ON_WD;
    pTdri->nr3d.nr3d_on_height = tuningIspReg->DIP_X_NR3D_ON_SIZ.Bits.NR3D_ON_HT;
    pTdri->nr3d.tnr_color_en = tuningIspReg->DIP_X_CAM_COLOR_START.Bits.COLOR_DISP_COLOR_START;
    pTdri->nr3d.tnr_en = tuningIspReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN;

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
    //
EXIT:
    return ret;

}
int DIP_ISP_PIPE::ConfigDipPipe(DipWorkingBuffer* pDipWBuf)
{
    int i;
    int             ret_val = ISP_ERROR_CODE_OK;
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

    DIP_FUNC_INF("moduleIdx(%d), pDipWBuf(0x%x), m_pIspDescript_phy(0x%x), m_pIspDescript_vir(0x%x), m_pIspVirRegAddr_pa(0x%x), m_pIspVirRegAddr_va(0x%x)",
        this->moduleIdx, pDipWBuf, pDipWBuf->m_pIspDescript_phy, pDipWBuf->m_pIspDescript_vir, pDipWBuf->m_pIspVirRegAddr_pa, pDipWBuf->m_pIspVirRegAddr_va);


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
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            DIP_FUNC_ERR("[ERROR] _config(%s) ",isplist[i]->name_Str());
            break;
        }

        if( ( ret_val = isplist[i]->write2CQ() ) < 0 )
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

int DIP_ISP_PIPE::config( void )
{
    MINT32 ret = MTRUE;
    MBOOL crz_result = MTRUE;
    MINT32 result = MTRUE;
    MUINT32 i;
    
    DIP_FUNC_DBG("[DIP_TOP_CTRL]hwModule(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d),isApplyTuning(%d)",\
        this->hwModule,this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx,this->isApplyTuning);

    DIP_FUNC_DBG("YUV_EN(0x%08x),YUV2_EN(0x%08x),RGB_EN(0x%08x),FMT_SEL(0x%08x),PATH_SEL(0x%08x)",\
        this->isp_top_ctl.YUV_EN.Raw,this->isp_top_ctl.YUV2_EN.Raw,this->isp_top_ctl.RGB_EN.Raw,\
        this->isp_top_ctl.FMT_SEL.Raw,this->isp_top_ctl.PATH_SEL.Raw);

    // TODO: hungwen modify
    this->pDrvDip = m_pIspDrvShell->m_pDrvDip[this->moduleIdx];
    //this->pDrvDip = m_pIspDrvShell->m_pDrvDip[this->moduleIdx][this->dipTh][this->burstQueIdx][this->dupCqIdx];
    this->pDrvDipPhy = this->m_pIspDrvShell->m_pDrvDipPhy;

    /********************************************************************************************************************/
    //moduleIdx = switchHwModuleToHwIdx((ISP_HW_MODULE)this->hwModule);
    // set registers to hardware register directly
    //when mfb dcm is enabled, the scenario realted mfb will happen seldom hang in last tile. 
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV_DCM_DIS, 0x00000001);//kk test (read clear or write one clear??) // Move to initial setting table
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_YUV2_DCM_DIS, 0x00000008);//kk test (read clear or write one clear??)
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_INT_EN, 0x0);//disable to save cpu usage // Move to initial setting table
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_CQ_INT_EN, 0x0);//disable to save cpu usage // Move to initial setting table
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_CQ_INT2_EN, 0x0);//disable to save cpu usage // Move to initial setting table
    //DIP_WRITE_PHY_REG(this->pDrvDipPhy,DIP_X_CTL_CQ_INT3_EN, 0x0);//disable to save cpu usage // Move to initial setting table
    // set registers to dip thread
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_DMA_EN, this->isp_top_ctl.DMA_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_YUV_EN, this->isp_top_ctl.YUV_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_YUV2_EN, this->isp_top_ctl.YUV2_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_RGB_EN, this->isp_top_ctl.RGB_EN.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_FMT_SEL, this->isp_top_ctl.FMT_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_PATH_SEL, this->isp_top_ctl.PATH_SEL.Raw);
    DIP_WRITE_REG(this->pDrvDip, DIP_X_CTL_MISC_SEL, this->isp_top_ctl.MISC_EN.Raw);

    //
    PushFrameCQforMDPStart();

    //DIP_RAW_PIPE Domain
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].eModule==eIspModuleRaw)&& \
                (gIspModuleMappingTableRbgEn[i].default_func!=NULL)) {
            gIspModuleMappingTableRbgEn[i].default_func((*this));
        }
    }

    //DIP_RGB_PIPE
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].eModule==eIspModuleRgb)&& \
                (gIspModuleMappingTableRbgEn[i].default_func!=NULL)) {
            gIspModuleMappingTableRbgEn[i].default_func((*this));
        }
    }

    //DIP_YUV_PIPE
    for(i=0;i<CHECK_CTL_YUV_EN_BITS;i++){
        if((gIspModuleMappingTableYuvEn[i].ctrlByte & this->isp_top_ctl.YUV_EN.Raw)&&\
                (gIspModuleMappingTableYuvEn[i].eModule==eIspModuleYuv)&& \
                (gIspModuleMappingTableYuvEn[i].default_func!=NULL)) {
            gIspModuleMappingTableYuvEn[i].default_func((*this));
        }
    }
    //
    for(i=0;i<CHECK_CTL_YUV2_EN_BITS;i++){
        if((gIspModuleMappingTableYuv2En[i].ctrlByte & this->isp_top_ctl.YUV2_EN.Raw)&&\
                (gIspModuleMappingTableYuv2En[i].eModule==eIspModuleYuv)&& \
                (gIspModuleMappingTableYuv2En[i].default_func!=NULL)) {
            gIspModuleMappingTableYuv2En[i].default_func((*this));
        }
    }

    //LCE and HLR
    if ((this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_LCE) || (this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_HLR2))
    {
        DIP_WRITE_REG(this->pDrvDip,DIP_X_HLR2_CFG,this->pTuningIspReg->DIP_X_HLR2_CFG.Raw);
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

int DIP_ISP_PIPE::PushFrameCQforMDPStart( void )
{
     _CQ_CONFIG_INFO_ cqcfginfo;
    cqcfginfo.hwModule=this->hwModule;
    cqcfginfo.moduleIdx=this->moduleIdx;
    cqcfginfo.dipTh=this->dipTh;
    cqcfginfo.dipCQ_dupIdx=this->dupCqIdx;
    cqcfginfo.dipCQ_burstIdx=this->burstQueIdx;
    cqcfginfo.isDipOnly=this->isDipOnly;
    cqcfginfo.dipCQ_ringIdx=this->RingBufIdx;
    cqcfginfo.pDipWBuf= this->m_pDipWBuf;
    pthread_mutex_lock(&mCQCfgInfoMutex);
    gFrameCQCfgInfoList.push_back(cqcfginfo);
    pthread_mutex_unlock(&mCQCfgInfoMutex);
    return MTRUE;
}

int DIP_ISP_PIPE::enable(void* pParam)
{
    MINT32 errSta;
    MINT32 ret = MTRUE;
    MUINT32 frameflag=0;
    DipRingBuffer* pDipRingBuffer=NULL;
    MUINT32 ref_ringbufidx=0;

    //
     _CQ_CONFIG_INFO_ cqcfginfo;
    list<_CQ_CONFIG_INFO_>::iterator it;
    pthread_mutex_lock(&mCQCfgInfoMutex);
    it = gFrameCQCfgInfoList.begin();
    cqcfginfo.hwModule=(*it).hwModule;
    cqcfginfo.moduleIdx=(*it).moduleIdx;
    cqcfginfo.dipTh=(*it).dipTh;
    cqcfginfo.dipCQ_dupIdx=(*it).dipCQ_dupIdx;
    cqcfginfo.dipCQ_burstIdx=(*it).dipCQ_burstIdx;
    cqcfginfo.dipCQ_ringIdx=(*it).dipCQ_ringIdx;
    cqcfginfo.isDipOnly=(*it).isDipOnly;
    cqcfginfo.pDipWBuf=(*it).pDipWBuf;
    gFrameCQCfgInfoList.pop_front();
    pthread_mutex_unlock(&mCQCfgInfoMutex);
    pParam;
    ref_ringbufidx=cqcfginfo.dipCQ_ringIdx;
    if (this->m_pIspDrvShell->m_p2disableSkipTpipe == 0)
    {
        pDipRingBuffer = this->m_pIspDrvShell->getDipRingBufMgr();
        if (pDipRingBuffer != NULL)
        {
            pDipRingBuffer->skipTpipeCalculatebyIdx((E_ISP_DIP_CQ)cqcfginfo.dipTh,frameflag,ref_ringbufidx,cqcfginfo.pDipWBuf);
            cqcfginfo.pDipWBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = frameflag;
            DIP_FUNC_INF("thiscq(%d), frameflag(0x%x), ring_index(%d), ref_ringbufidx(%d)",cqcfginfo.dipTh, frameflag, cqcfginfo.pDipWBuf->m_BufIdx, ref_ringbufidx);
        
        }
        ret=m_pIspDrvShell->m_pMdpMgr->updateDpStreamMap(frameflag,cqcfginfo.dipTh, cqcfginfo.dipCQ_ringIdx, ref_ringbufidx);
    }

    DIP_FUNC_DBG("[_enable]isDipOnly(%d), hwm(%d_%d_%d_%d_%d_%x_desphy:0x%x,desvir:0x%x,virispphy:0x%x,virispvir:0x%x)",cqcfginfo.isDipOnly, cqcfginfo.moduleIdx, cqcfginfo.moduleIdx,\
        cqcfginfo.dipTh,cqcfginfo.dipCQ_dupIdx,cqcfginfo.dipCQ_burstIdx, cqcfginfo.pDipWBuf, cqcfginfo.pDipWBuf->m_pIspDescript_phy, cqcfginfo.pDipWBuf->m_pIspDescript_vir,
        cqcfginfo.pDipWBuf->m_pIspVirRegAddr_pa, cqcfginfo.pDipWBuf->m_pIspVirRegAddr_va);

#if (DEBUG_ISP_ONLY == MFALSE)
        errSta = m_pIspDrvShell->m_pMdpMgr->startMdp((*cqcfginfo.pDipWBuf->m_pMdpCfg));
        if(MDPMGR_NO_ERROR != errSta){
            DIP_FUNC_ERR("[Error]pass2 TH(%d) m_pIspDrvShell->m_pMdpMgr->startMdp fail(%d)",cqcfginfo.dipTh,errSta);
            ret = -1;
            goto EXIT;
        }

#else
        ISP_REGISTER_USERKEY_STRUCT regIrq;
        regIrq.userName[0] = '\0';
        sprintf(regIrq.userName,"DIP_A");
        // register pass2 irq to get the irq key
        ret = m_pIspDrvShell->m_pDrvDipPhy->registerIrq(&regIrq);
        if ( MFALSE == ret ) {
            DIP_FUNC_ERR("[Error]registerIrq fail");
            ret = -1;
            goto EXIT;
        }
        this->irqUserKey = regIrq.userKey;

        switch (cqcfginfo.dipTh) {
            case ISP_DRV_DIP_CQ_THRE0:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR0_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE1:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR1_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE2:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR2_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE3:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR3_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE4:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR4_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE5:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR5_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE6:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR6_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE7:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR7_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE8:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR8_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE9:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR9_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE10:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR10_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE11:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR11_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE12:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR12_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE13:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR13_START,0x01);
                break;
            case ISP_DRV_DIP_CQ_THRE14:
                DIP_WRITE_PHY_BITS(cqcfginfo.hwModule, m_pIspDrvShell->m_pDrvDipPhy,DIP_X_CTL_START,CQ_THR14_START,0x01);
                break;
            default:
                DIP_FUNC_ERR("[Error]Not support this dip thread(%d)",cqcfginfo.dipTh);
                ret = -1;
                goto EXIT;
                break;
        }
#endif

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


int DIP_ISP_PIPE::write2CQ(void)
{
    MUINT32 i;

    DIP_FUNC_DBG("[DIP_TOP_CTRL]hwModule(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d)",\
        this->hwModule,this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx);

    //m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL); // Remove for change to GCE setting

    //DIP_RAW_PIPE Domain
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].eModule==eIspModuleRaw)&& \
                (gIspModuleMappingTableRbgEn[i].eCqThrFunc!=DIP_A_DUMMY)) {
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableRbgEn[i].eCqThrFunc);
        }
    }

    //DIP_RGB_PIPE
    for(i=0;i<CHECK_CTL_RAW_RGB_EN_BITS;i++){
        if((gIspModuleMappingTableRbgEn[i].ctrlByte & this->isp_top_ctl.RGB_EN.Raw)&&\
                (gIspModuleMappingTableRbgEn[i].eModule==eIspModuleRgb)&& \
                (gIspModuleMappingTableRbgEn[i].eCqThrFunc!=DIP_A_DUMMY)) {
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableRbgEn[i].eCqThrFunc);
        }
    }

    //DIP_YUV_PIPE
    for(i=0;i<CHECK_CTL_YUV_EN_BITS;i++){
        if((gIspModuleMappingTableYuvEn[i].ctrlByte & this->isp_top_ctl.YUV_EN.Raw)&&\
                (gIspModuleMappingTableYuvEn[i].eModule==eIspModuleYuv)&& \
                (gIspModuleMappingTableYuvEn[i].eCqThrFunc1!=DIP_A_DUMMY)) {
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuvEn[i].eCqThrFunc1);

            if(gIspModuleMappingTableYuvEn[i].eCqThrFunc2!=DIP_A_DUMMY)
                m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuvEn[i].eCqThrFunc2);

            if(gIspModuleMappingTableYuvEn[i].eCqThrFunc3!=DIP_A_DUMMY)
                m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuvEn[i].eCqThrFunc3);

            if(gIspModuleMappingTableYuvEn[i].eCqThrFunc4!=DIP_A_DUMMY)
                m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuvEn[i].eCqThrFunc4);
        }
    }

    for(i=0;i<CHECK_CTL_YUV2_EN_BITS;i++){
        if((gIspModuleMappingTableYuv2En[i].ctrlByte & this->isp_top_ctl.YUV2_EN.Raw)&&\
                (gIspModuleMappingTableYuv2En[i].eModule==eIspModuleYuv)&& \
                (gIspModuleMappingTableYuv2En[i].eCqThrFunc!=DIP_A_DUMMY)) {
            m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(gIspModuleMappingTableYuv2En[i].eCqThrFunc);
        }
    }
    //LCE and HLR
    if ((this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_LCE) || (this->isp_top_ctl.RGB_EN.Raw & DIP_X_REG_CTL_RGB_EN_HLR2))
    {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_HLR2);
    }

    //UFDI
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_SPECIAL_FUN);

    //CRZ
    if ( this->crzPipe.conf_cdrz ) {
        m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CRZ);
    }

    //TPIPE Ctrl
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_TDR);

    //FE Enable
    m_pIspDrvShell->m_pDrvDip[this->moduleIdx]->cqAddModule(DIP_A_CTL_TDR3);


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
    DIP_FUNC_DBG("[DIP_MDP_PIPE]hwModule(%d),moduleIdx(%d),hwTh(%d),burstQueIdx(%d),dupCqIdx(%d),dipCtlDmaEn(0x%x)",\
        this->hwModule,this->moduleIdx,this->dipTh,this->burstQueIdx,this->dupCqIdx,this->dipCtlDmaEn);

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
        //pMdpCfg->m_iDipMetEn = this->m_pIspDrvShell->m_iDipMetEn;
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
    //pmqos
    pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = this->framemode_total_in_w;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_out_w = this->framemode_total_out_w;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = this->framemode_h;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = this->total_data_size;

    // initialize
    for(MINT32 i = 0; i < ISP_OUTPORT_NUM; i++){
        pMdpCfg->dstPortCfg[i] = 0;
    }

    #if 0
    DIP_FUNC_DBG("DIP_MDP_PIPE,crop(x:%d,y:%d,w:%d,h:%d)", \
        this->src_crop.x, \
        this->src_crop.y, \
        this->src_crop.w, \
        this->src_crop.h);
    #endif

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

    DIP_FUNC_DBG("[imgi]p2cq(%d),cq(%d),dupCqIdx(%d),burstQueIdx(%d)",this->dipTh,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx,\
        pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx);

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
    if(this->pTuningIspReg != NULL) {
        pMdpCfg->isoValue = this->pTuningIspReg->DIP_X_ISO_ADAP.Bits.ISO_ADAPTIVE;
    }

    // set PQ parameter for MDP
    pMdpCfg->mdpWDMAPQParam = this->WDMAPQParam;
    pMdpCfg->mdpWROTPQParam = this->WROTPQParam;

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
dequeueMdpFrameEnd()
{
    if(MDPMGR_NO_ERROR != m_pIspDrvShell->m_pMdpMgr->dequeueFrameEnd((*m_pMdpMgrCfgData))){
        DIP_FUNC_ERR("dequeueMdpFrameEnd of DIP_BUF_CTRL is fail!!");
        return -1;
    }
    return 0;
}

#define DEQUE_DMA_MASK 0xfff12801

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
    IspDrvDip*   pDrvDip;
    MDPMGR_CFG_STRUCT mdpMgrCfgStructData;

    //get property for dumpHwBuf
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.dumpDipHwReg.enable", value, "0");
    int dumpDipHwRegEn=atoi(value);

    if (pDipWBuf==NULL){
        DIP_FUNC_ERR("deque pDipWBuf is NULL!!");
        return eIspRetStatus_Failed;
    }
    m_pMdpMgrCfgData = pDipWBuf->m_pMdpCfg;

    //pthread_mutex_lock(&mMdpCfgDataMutex);
    mdpMgrCfgDataImgxoEnP2 = m_pMdpMgrCfgData->imgxoEnP2;
    ispDmaEn = (m_pMdpMgrCfgData->ispDmaEn & DEQUE_DMA_MASK);
    //tpipeVa = (MUINTPTR)gpMdpMgrCfgData[this->m_BufmoduleIdx][this->m_BufdipTh][this->m_BufburstQueIdx][this->m_BufdupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa;
    //ispVirRegVa = (MUINTPTR)this->m_pIspDrvShell->m_pDrvDip[this->m_BufmoduleIdx][this->m_BufdipTh][this->m_BufburstQueIdx][this->m_BufdupCqIdx]->getIspVirRegVirAddr();
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
#define DIP_X_REG_CTL_DMA_EN_ADL                   (1L<<17)
#define DIP_X_REG_CTL_DMA_EN_WROTO                   (1L<<20)
#define DIP_X_REG_CTL_DMA_EN_WDMAO                   (1L<<21)
#define DIP_X_REG_CTL_DMA_EN_JPEGO                   (1L<<22)
#define DIP_X_REG_CTL_DMA_EN_VENCO                   (1L<<23)
*/

    for(int i = 23 ; i >= 0 ; i--) //DIP_X_REG_CTL_DMA_EN_VENCO
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
                        tpipe_main_platform(&m_pMdpMgrCfgData->ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspDipReg, NULL, NULL);
                    }
                    delete[] pWorkingBuffer;
                    pWorkingBuffer = NULL;
                } else {
                    DIP_FUNC_ERR("[Error]pWorkingBuffer(0x%lx),pIspReg(0x%lx)",(unsigned long)pWorkingBuffer,(unsigned long)pIspDipReg);
                }

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
                mdpRet = (MDPMGR_RETURN_TYPE)m_pIspDrvShell->m_pMdpMgr->dequeueBuf(mdpQueueIdx, (*m_pMdpMgrCfgData));
                //pthread_mutex_lock(&mMdpCfgDataMutex);
                //::memcpy(&mdpMgrCfgStructData, m_pMdpMgrCfgData, sizeof(MDPMGR_CFG_STRUCT));
                //pthread_mutex_unlock(&mMdpCfgDataMutex);
                //
                //mdpRet = (MDPMGR_RETURN_TYPE)m_pIspDrvShell->m_pMdpMgr->dequeueBuf(mdpQueueIdx, mdpMgrCfgStructData);
                DIP_FUNC_DBG("mdpRet(%d)",mdpRet);
                if(mdpRet == MDPMGR_NO_ERROR){
                    m_pMdpMgrCfgData->isWaitBuf = MTRUE;
                    ret = eIspRetStatus_Success;
                } else if(mdpRet == MDPMGR_VSS_NOT_READY){
                    ret = eIspRetStatus_VSS_NotReady;
                    break;
                } else {
            #if 0 //kk test, need to do later
                    MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
                    char* pWorkingBuffer =  NULL;
                    isp_reg_t *pIspReg;
                    IspDumpDbgLogP2Package p2DumpPackage;
                    ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
                    //
                    DIP_FUNC_INF("dequeBurstQueIdx(%d),p2Cq(%d),dequeDupCqIdx(%d),tpipeVa(0x%x),tpipePa(0x%x)",
                        this->dequeBurstQueIdx,p2Cq,this->dequeDupCqIdx,gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa,
                        gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa);
                    // dump CQ vir/des table
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((E_ISP_DIP_CQ)0,0, this->dequeDupCqIdx);
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpCQTable((E_ISP_DIP_CQ)this->dequeCq,this->burstQueIdx, this->dequeDupCqIdx);
                    // dump difference between tpipe and hardware registers
                    pIspReg = (isp_reg_t*)this->m_pIspDrvShell->m_pPhyIspDrv_bak->getCurHWRegValues();
                    pWorkingBuffer = new char[tpipeWorkingSize];
                    //
                    this->m_pIspDrvShell->gPass2Lock.lock();
                    if((pWorkingBuffer!=NULL)&&(pIspReg!=NULL)) {
                        tpipe_main_platform(&gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspReg);
                        delete[] pWorkingBuffer;
                        pWorkingBuffer = NULL;
                    } else {
                        DIP_FUNC_ERR("[Error]pWorkingBuffer(0x%08x),pIspReg(0x%08x)",pWorkingBuffer,pIspReg);
                    }
                    // dump hardware debug information
                    p2DumpPackage.tpipeTableVa = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa;
                    p2DumpPackage.tpipeTablePa = gpMdpMgrCfgData[this->dequeBurstQueIdx][p2Cq][this->dequeDupCqIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa;
                    this->m_pIspDrvShell->gPass2Lock.unlock();
                    //
                    this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpDBGLog(NULL,&p2DumpPackage);
                    //
            #endif
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
                            tpipe_main_platform(&m_pMdpMgrCfgData->ispTpipeCfgInfo, &tpipeDesc, pWorkingBuffer, tpipeWorkingSize, (void*)pIspDipReg, NULL, NULL);
                        }
                        delete[] pWorkingBuffer;
                        pWorkingBuffer = NULL;
                    } else {
                        DIP_FUNC_ERR("[Error]pWorkingBuffer(0x%lx),pIspReg(0x%lx)",(unsigned long)pWorkingBuffer,(unsigned long)pIspDipReg);
                    }
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

