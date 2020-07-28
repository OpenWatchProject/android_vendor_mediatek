#ifndef __ISP_FUNCTION_DIP_H__
#define __ISP_FUNCTION_DIP_H__
//
#include <vector>
#include <map>
#include <list>
using namespace std;
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
//
#include <mtkcam/drv/def/ispio_port_index.h>
#include "isp_datatypes.h"
#include "camera_isp.h" //For Isp function ID
#include "crz_drv.h"
#include <isp_drv.h>
#include <imem_drv.h>
#include <ispio_pipe_scenario.h>    // For enum EDrvScenario.
#include "isp_drv_dip.h"
#include "isp_drv_dip_phy.h"
#include "mdp_mgr.h"



/*/////////////////////////////////////////////////////////////////////////////
    ISP Function ID
  /////////////////////////////////////////////////////////////////////////////*/
// for crop function
#define CROP_FLOAT_PECISE_BIT   31    // precise 31 bit
#define CROP_TPIPE_PECISE_BIT   15
#define CROP_CDP_HARDWARE_BIT   8


//start bit
enum {
    DIP_ISP_PASS2_START,
    DIP_ISP_PASS2B_START,
    DIP_ISP_PASS2C_START,
    DIP_ISP_NONE_START,
};

//
enum {
    ENUM_DIP_TOP = 0,    //0
    ENUM_DIP_RAW,
    ENUM_DIP_RGB,
    ENUM_DIP_YUV,
    ENUM_DIP_MDP,
    ENUM_DIP_CDP,        //5
    ENUM_DIP_TDRI,
    ENUM_DIP_DMA_CQI,
    ENUM_DIP_DMA_TDRI,
    ENUM_DIP_DMA_IMGI,
    ENUM_DIP_DMA_IMGBI,//new
    ENUM_DIP_DMA_IMGCI,//new
    ENUM_DIP_DMA_UFDI,
    ENUM_DIP_DMA_LCEI,   
    ENUM_DIP_DMA_VIPI,
    ENUM_DIP_DMA_VIP2I, //15
    ENUM_DIP_DMA_VIP3I,
    ENUM_DIP_DMA_DMGI,//new
    ENUM_DIP_DMA_DEPI,//new
    ENUM_DIP_DMA_MFBO,
    ENUM_DIP_DMA_IMG3O, //20
    ENUM_DIP_DMA_IMG3BO,
    ENUM_DIP_DMA_IMG3CO, 
    ENUM_DIP_DMA_FEO,
    ENUM_DIP_DMA_IMG2O,  
    ENUM_DIP_DMA_IMG2BO,//new, 25
    ENUM_DIP_DMA_WDMAO,
    ENUM_DIP_DMA_WROTO,
    ENUM_DIP_DMA_JPEGO,
    ENUM_DIP_DMA_VENC_STREAMO,
    ENUM_DIP_BUFFER,
    ENUM_DIP_TURNING,
    ENUM_DIP_CRZ,
    ENUM_DIP_FUNCTION_MAX_NUM
};

/*/////////////////////////////////////////////////////////////////////////////
    DMA sharing table
  /////////////////////////////////////////////////////////////////////////////*/
 //->//
#define ISP_SUB_MODE_RAW        0
#define ISP_SUB_MODE_YUV        1
#define ISP_SUB_MODE_RGB        2
#define ISP_SUB_MODE_JPG        3
#define ISP_SUB_MODE_MAX        4



#if 1  //remove it later
//cam_fmt_sel
//->//
#define DIP_SCENARIO_IC         0
#define DIP_SCENARIO_VR         1
#define DIP_SCENARIO_ZSD        2
#define DIP_SCENARIO_VEC        4
#define DIP_SCENARIO_RESERVE01  5
#define DIP_SCENARIO_N3D_IC     6
#define DIP_SCENARIO_N3D_VR     7
#define DIP_SCENARIO_MAX        8

#define DIP_SUB_MODE_MFB        5

#endif


//->//
#define CAM_FMT_SEL_TG_FMT_RAW8   0
#define CAM_FMT_SEL_TG_FMT_RAW10  1
#define CAM_FMT_SEL_TG_FMT_RAW12  2
#define CAM_FMT_SEL_TG_FMT_YUV422 3
#define CAM_FMT_SEL_TG_FMT_RGB565 5
#define CAM_FMT_SEL_TG_FMT_JPEG   7
//->//
#define CAM_FMT_SEL_YUV420_2P    0
#define CAM_FMT_SEL_YUV420_3P    1
#define CAM_FMT_SEL_YUV422_1P    2
#define CAM_FMT_SEL_YUV422_2P    3
#define CAM_FMT_SEL_YUV422_3P    4
#define CAM_FMT_SEL_Y_ONLY       12

#define CAM_FMT_SEL_RGB565       0
#define CAM_FMT_SEL_RGB888       1
#define CAM_FMT_SEL_XRGB8888     2
#define CAM_FMT_SEL_RGB101010    3
#define CAM_FMT_SEL_RGB121212    4
#define CAM_FMT_SEL_BAYER8       0
#define CAM_FMT_SEL_BAYER10      1
#define CAM_FMT_SEL_BAYER12      2
#define CAM_FMT_SEL_BAYER14      3

//->//
#define CAM_FMT_SEL_TG_SW_UYVY      0
#define CAM_FMT_SEL_TG_SW_YUYV      1
#define CAM_FMT_SEL_TG_SW_VYUY      2
#define CAM_FMT_SEL_TG_SW_YVYU      3
#define CAM_FMT_SEL_TG_SW_RGB       0
#define CAM_FMT_SEL_TG_SW_BGR       2
//camsv_fmt
#define CAMSV_FMT_SEL_IMGO_OTHER    0
#define CAMSV_FMT_SEL_IMGO_YUV      1
#define CAMSV_FMT_SEL_IMGO_JPG      2

#define CAMSV_FMT_SEL_TG_SW_UYVY    0
#define CAMSV_FMT_SEL_TG_SW_YUYV    1
#define CAMSV_FMT_SEL_TG_SW_VYUY    2
#define CAMSV_FMT_SEL_TG_SW_YVYU    3
#define CAMSV_FMT_SEL_TG_SW_RGB     0
#define CAMSV_FMT_SEL_TG_SW_BGR     2

#define CAMSV_FMT_SEL_TG_FMT_RAW8   0
#define CAMSV_FMT_SEL_TG_FMT_RAW10  1
#define CAMSV_FMT_SEL_TG_FMT_RAW12  2
#define CAMSV_FMT_SEL_TG_FMT_YUV422 3
#define CAMSV_FMT_SEL_TG_FMT_RAW14  4
#define CAMSV_FMT_SEL_TG_FMT_JPEG   7

#define CAMSV_PAK_MODE_FMT_RAW14 0
#define CAMSV_PAK_MODE_FMT_RAW12 1
#define CAMSV_PAK_MODE_FMT_RAW10 2
#define CAMSV_PAK_MODE_FMT_RAW8  3

////////////////////////////////////
//imgi
#define DIP_IMGI_FMT_YUV422_1P    0
#define DIP_IMGI_FMT_YUV422_2P    1
#define DIP_IMGI_FMT_YUV422_3P    2
#define DIP_IMGI_FMT_YUV420_2P    3
#define DIP_IMGI_FMT_YUV420_3P    4
#define DIP_IMGI_FMT_Y_ONLY    5
#define DIP_IMGI_FMT_MFB_BLEND_MODE    6
#define DIP_IMGI_FMT_MFB_MIX_MODE    7
#define DIP_IMGI_FMT_BAYER8    8
#define DIP_IMGI_FMT_BAYER10    9
#define DIP_IMGI_FMT_BAYER12    10
#define DIP_IMGI_FMT_BAYER14    11
#define DIP_IMGI_FMT_BAYER8_2BYTEs    12
#define DIP_IMGI_FMT_BAYER10_2BYTEs    13
#define DIP_IMGI_FMT_BAYER12_2BYTEs    14
#define DIP_IMGI_FMT_BAYER14_2BYTEs    15
#define DIP_IMGI_FMT_BAYER10_MIPI    16
#define DIP_IMGI_FMT_RGB565    24
#define DIP_IMGI_FMT_RGB888    25
#define DIP_IMGI_FMT_XRGB8888    26
#define DIP_IMGI_FMT_RGB101010    27
#define DIP_IMGI_FMT_RGB121212    28

//vipi_fmt
#define DIP_VIPI_FMT_YUV422_1P                  0
#define DIP_VIPI_FMT_YUV422_2P                  1
#define DIP_VIPI_FMT_YUV422_3P                  2
#define DIP_VIPI_FMT_YUV420_2P                  3
#define DIP_VIPI_FMT_YUV420_3P                  4
#define DIP_VIPI_FMT_Y_ONLY                  5
#define DIP_VIPI_FMT_RGB565                  24
#define DIP_VIPI_FMT_RGB888                  25
#define DIP_VIPI_FMT_XRGB8888                  26
#define DIP_VIPI_FMT_RGB101010                  27
#define DIP_VIPI_FMT_RGB121212                  28

//ufdi fmt
#define DIP_UFDI_FMT_UFO_LENGTH     0
#define DIP_UFDI_FMT_WEIGHTING      1

//dmgi fmt
#define DIP_DMGI_FMT_DRZ     0
#define DIP_DMGI_FMT_FM      1

//depi fmt
#define DIP_DEPI_FMT_DRZ     0
#define DIP_DEPI_FMT_FM      1

//img3o_fmt
#define DIP_IMG3O_FMT_YUV422_1P    0
#define DIP_IMG3O_FMT_YUV422_2P    1
#define DIP_IMG3O_FMT_YUV422_3P    2
#define DIP_IMG3O_FMT_YUV420_2P    3
#define DIP_IMG3O_FMT_YUV420_3P    4

//img3o_fmt
#define DIP_IMG2O_FMT_YUV422_1P    0
#define DIP_IMG2O_FMT_YUV422_2P    1

//pixel id
#define DIP_PIX_ID_B    0
#define DIP_PIX_ID_Gb   1
#define DIP_PIX_ID_Gr   2
#define DIP_PIX_ID_R    3

//fg_mode
#define DIP_FG_MODE_DISABLE  0
#define DIP_FG_MODE_ENABLE   1


/*/////////////////////////////////////////////////////////////////////////////
    TODO: temp from tg_common
  /////////////////////////////////////////////////////////////////////////////*/
//
#define CAM_ISP_SETTING_DONT_CARE 0
//CDP
#define CAM_CRZ_6_TAP  0
#define CAM_CRZ_N_TAP  1
#define CAM_CRZ_4N_TAP 2
//
#define CAM_MODE_FRAME  0
#define CAM_MODE_TPIPE  1
//
#define _FMT_YUV420_2P_ 0
#define _FMT_YUV420_3P_ 1
#define _FMT_YUV422_1P_ 2
#define _FMT_YUV422_2P_ 3
#define _FMT_YUV422_3P_ 4

#define _FMT_SEQ_422_UYVY_  0
#define _FMT_SEQ_422_VYUY_  1
#define _FMT_SEQ_422_YUYV_  2
#define _FMT_SEQ_422_YVYU_  3
#define _FMT_SEQ_420_UV_    0
#define _FMT_SEQ_420_VU_    1

//
#define DIP_ISP_CQ_NONE (0xFFFF)
#define DIP_ISP_CQ0     ISP_DRV_DIP_CQ_THRE0
#define DIP_ISP_CQ1     ISP_DRV_DIP_CQ_THRE1
#define DIP_ISP_CQ2     ISP_DRV_DIP_CQ_THRE2
#define DIP_ISP_CQ3     ISP_DRV_DIP_CQ_THRE3
#define DIP_ISP_CQ4     ISP_DRV_DIP_CQ_THRE4
#define DIP_ISP_CQ5     ISP_DRV_DIP_CQ_THRE5
#define DIP_ISP_CQ6     ISP_DRV_DIP_CQ_THRE6
#define DIP_ISP_CQ7     ISP_DRV_DIP_CQ_THRE7
#define DIP_ISP_CQ8     ISP_DRV_DIP_CQ_THRE8
#define DIP_ISP_CQ9     ISP_DRV_DIP_CQ_THRE9
#define DIP_ISP_CQ10    ISP_DRV_DIP_CQ_THRE10
#define DIP_ISP_CQ11    ISP_DRV_DIP_CQ_THRE11
#define DIP_ISP_CQ12    ISP_DRV_DIP_CQ_THRE12
#define DIP_ISP_CQ13    ISP_DRV_DIP_CQ_THRE13
#define DIP_ISP_CQ14    ISP_DRV_DIP_CQ_THRE14
#define DIP_ISP_CQ15    ISP_DRV_DIP_CQ_THRE15
#define DIP_ISP_CQ16    ISP_DRV_DIP_CQ_THRE16
#define DIP_ISP_CQ17    ISP_DRV_DIP_CQ_THRE17
#define DIP_ISP_CQ18    ISP_DRV_DIP_CQ_THRE18


//
#define CAM_CQ_SINGLE_IMMEDIATE_TRIGGER CQ_SINGLE_IMMEDIATE_TRIGGER
#define CAM_CQ_SINGLE_EVENT_TRIGGER     CQ_SINGLE_EVENT_TRIGGER
#define CAM_CQ_CONTINUOUS_EVENT_TRIGGER CQ_CONTINUOUS_EVENT_TRIGGER
//
#define CAM_CQ_TRIG_BY_START        CQ_TRIG_BY_START
#define CAM_CQ_TRIG_BY_PASS2_DONE   CQ_TRIG_BY_PASS2_DONE
#define CAM_CQ_TRIG_BY_IMGO_DONE    CQ_TRIG_BY_IMGO_DONE
#define CAM_CQ_TRIG_BY_RRZO_DONE    CQ_TRIG_BY_RRZO_DONE
#define CAM_CQ_TRIG_BY_NONE         (-1)

//
#define CAM_ISP_PIXEL_BYTE_FP               (3)
#define CAM_ISP_PIXEL_BYTE_MAX_TRUNCATION   ((1<<3)-1)
//isp line buffer
#define CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL (2304)
//
#define CAM_CDP_PRZ_CONN_TO_DISPO 0
#define CAM_CDP_PRZ_CONN_TO_VIDO  1
//interrupt timeout time
#define CAM_INT_WAIT_TIMEOUT_MS 3000
#define CAM_INT_PASS2_WAIT_TIMEOUT_MS 3000

enum EIspUpdateTgtDev
{
    eIspUpdateTgtDev_Main  = 1,
    eIspUpdateTgtDev_Sub   = 2,
    eIspUpdateTgtDev_Main2 = 3,
};

enum EIspRetStatus
{
    eIspRetStatus_VSS_NotReady  = 1,
    eIspRetStatus_Success       = 0,
    eIspRetStatus_Failed        = -1,
};

// TPIPE
class TdriDrvCfg
{
public:
    ISP_TPIPE_CONFIG_TOP_STRUCT top;
    ISP_TPIPE_CONFIG_SW_STRUCT sw;
    ISP_TPIPE_CONFIG_IMGI_STRUCT imgi;
    ISP_TPIPE_CONFIG_IMGBI_STRUCT imgbi;
    ISP_TPIPE_CONFIG_IMGCI_STRUCT imgci;
    ISP_TPIPE_CONFIG_UFDI_STRUCT ufdi;
    ISP_TPIPE_CONFIG_BNR_STRUCT bnr;
    ISP_TPIPE_CONFIG_RMG_STRUCT rmg;
    ISP_TPIPE_CONFIG_LSC_STRUCT lsc2;
    ISP_TPIPE_CONFIG_SL2_STRUCT sl2;
    ISP_TPIPE_CONFIG_UDM_STRUCT udm;
    ISP_TPIPE_CONFIG_VIPI_STRUCT vipi;
    ISP_TPIPE_CONFIG_VIP2I_STRUCT vip2i;
    ISP_TPIPE_CONFIG_VIP3I_STRUCT vip3i;
    ISP_TPIPE_CONFIG_MFB_STRUCT mfb;
    ISP_TPIPE_CONFIG_MFBO_STRUCT mfbo;
    ISP_TPIPE_CONFIG_G2C_STRUCT g2c;
    ISP_TPIPE_CONFIG_SL2B_STRUCT sl2b;
    ISP_TPIPE_CONFIG_NBC_STRUCT nbc;
    ISP_TPIPE_CONFIG_NBC2_STRUCT nbc2;
    ISP_TPIPE_CONFIG_SL2C_STRUCT sl2c;
    ISP_TPIPE_CONFIG_SL2D_STRUCT sl2d;
    ISP_TPIPE_CONFIG_SL2E_STRUCT sl2e;
    ISP_TPIPE_CONFIG_SL2G_STRUCT sl2g;
    ISP_TPIPE_CONFIG_SL2H_STRUCT sl2h;
    ISP_TPIPE_CONFIG_SL2I_STRUCT sl2i;
    ISP_TPIPE_CONFIG_SL2I_STRUCT sl2k;
    ISP_TPIPE_CONFIG_HFG_STRUCT hfg;
    ISP_TPIPE_CONFIG_SEEE_STRUCT seee;
    ISP_TPIPE_CONFIG_LCEI_STRUCT lcei;
    ISP_TPIPE_CONFIG_LCE_STRUCT lce;
    ISP_TPIPE_CONFIG_CDRZ_STRUCT cdrz;
    ISP_TPIPE_CONFIG_IMG2O_STRUCT img2o;
    ISP_TPIPE_CONFIG_IMG2BO_STRUCT img2bo;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz1;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz2;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz3;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz4;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz5;
    //ISP_TPIPE_CONFIG_FE_STRUCT fe;
    ISP_TPIPE_CONFIG_FEO_STRUCT feo;
    ISP_TPIPE_CONFIG_NR3D_STRUCT nr3d;
	ISP_TPIPE_CONFIG_CRSP_STRUCT crsp;
    ISP_TPIPE_CONFIG_IMG3O_STRUCT img3o;
    ISP_TPIPE_CONFIG_IMG3BO_STRUCT img3bo;
    ISP_TPIPE_CONFIG_IMG3CO_STRUCT img3co;
    ISP_TPIPE_CONFIG_DMGI_STRUCT dmgi;
    ISP_TPIPE_CONFIG_DEPI_STRUCT depi;
    ISP_TPIPE_CONFIG_PCA_STRUCT pca;
};

 class CrspCfg
 {
 public:
     MUINT32 enCrsp;
     IspSize out;
 };
 class C02Cfg
 {
 public:
     IspSize in;
 };

 class LceCfg
 {
 public:
     IspSize in;
 };

 class CrzCfg
 {
 public:
	 MFLOAT CrzRatio;
	 MUINT32 crz_crop_initial_x;
	 MUINT32 crz_crop_initail_y;
     IspSize in;
 };

 class DrzCfg
 {
 public:
     IspSize in;
 };


 class MfbCfg
 {
 public:
     MUINT32 bld_mode;
     IspSize mfb_out;        //actually the same with mfbo
     MUINT32 out_xofst;
     MUINT32 bld_ll_db_en;   //set default from imageio, would update via tuning
     MUINT32 bld_ll_brz_en;  //set default from imageio, would update via tuning
 };

 class SrzSizeCfg
 {
  public:
      MUINT32 in_w;
      MUINT32 in_h;
      MUINT32 out_w;
      MUINT32 out_h;
     SrzSizeCfg()
         : in_w(0x0)
         , in_h(0x0)
         , out_w(0x0)
         , out_h(0x0)
     {
     }
 };

 class SrzCfg
 {
 public:
 	 MUINT32 ctrl;
     SrzSizeCfg inout_size;
     IspRect crop;
     MUINT32 h_step;
     MUINT32 v_step;

     SrzCfg()
    : ctrl(0x0)
    , h_step(0x0)
    , v_step(0x0)
     {}
 };

 class Nr3DCfg
 {
 public:
     MUINT32 fbcntOff_xoff;
     MUINT32 fbcntOff_yoff;
     MUINT32 fbcntSiz_xsiz;
     MUINT32 fbcntSiz_ysiz;
     MUINT32 fbCount_cnt;
     MUINT32 ctrl_lmtOutCntTh;
     MUINT32 ctrl_onEn;
     MUINT32 onOff_onOfStX;
     MUINT32 onOff_onOfStY;
     MUINT32 onSiz_onWd;
     MUINT32 onSiz_onHt;
     MUINT32 gain_weighting; // divided by 256
     MUINT32 vipi_offst;            //in byte
     MUINT32 vipi_readW;        //in pixel
     MUINT32 vipi_readH;        //in pixel

    Nr3DCfg()
        : fbcntOff_xoff(0x0)
        , fbcntOff_yoff(0x0)
        , fbcntSiz_xsiz(0x0)
        , fbcntSiz_ysiz(0x0)
        , fbCount_cnt(0x0)
        , ctrl_lmtOutCntTh(0x0)
        , ctrl_onEn(0x0)
        , onOff_onOfStX(0x0)
        , onOff_onOfStY(0x0)
        , onSiz_onWd(0x0)
        , onSiz_onHt(0x0)
        , gain_weighting(0x100)
        , vipi_offst(0x0)
        , vipi_readW(0x0)
        , vipi_readH(0x0)
    {
    }
 };


 /******************************************************************************
  *
  * @struct _CQ_CONFIG_INFO_
  * @brief config info for dip frame
  * @details
  *
  ******************************************************************************/
 struct _CQ_CONFIG_INFO_
 {
  public:
        MINT32 hwModule;
        MINT32 moduleIdx;
        MINT32 dipTh;
        MINT32 dipCQ_dupIdx;
        MINT32 dipCQ_burstIdx;
        MBOOL isDipOnly;

     _CQ_CONFIG_INFO_()
         : hwModule(0)
         , moduleIdx(0)
         , dipTh(0)
         , dipCQ_dupIdx(0)
         , dipCQ_burstIdx(0)
         , isDipOnly(MFALSE)
     {}
 };


//
/**/
class IspFunctionDip_B;
class DIP_ISP_PIPE;


//// for tuning function
typedef enum
{
    eIspModuleNull= 0x00000000,
    eIspModuleRaw = 0x00000001,
    eIspModuleRgb = 0x00000002,
    eIspModuleYuv = 0x00000004,
}EIspModule;


 class IspTopModuleMappingTable{
 public:
    MUINT32 ctrlByte;  // isp register byte
    EIspModule eModule;  // module
    DIP_A_MODULE_ENUM eCqThrFunc;
    MINT32 (*default_func)(DIP_ISP_PIPE &imageioPackage); // run individual function
 };

 class IspTopModuleMapping3LayerTable{
 public:
    MUINT32 ctrlByte;  // isp register byte
    EIspModule eModule;  // module
    DIP_A_MODULE_ENUM eCqThrFunc1;
    DIP_A_MODULE_ENUM eCqThrFunc2;
    DIP_A_MODULE_ENUM eCqThrFunc3;
    DIP_A_MODULE_ENUM eCqThrFunc4;
    MINT32 (*default_func)(DIP_ISP_PIPE &imageioPackage); // run individual function
 };

enum EIspControlBufferCmd
{
    eIspControlBufferCmd_Alloc  = 0,
    eIspControlBufferCmd_Free,
    eIspControlBufferCmd_Num
};




/*/////////////////////////////////////////////////////////////////////////////
    Isp driver object
/////////////////////////////////////////////////////////////////////////////*/
class IspDrv_B
{    public:
        IspDrv_B(){}
        virtual ~IspDrv_B(){}
    public://
        static IspDrvDipPhy*    m_pDrvDipPhy;
        static IspDrvDip*       m_pDrvDip[ISP_DIP_MODULE_IDX_MAX][ISP_DRV_DIP_CQ_NUM][MAX_BURST_QUE_NUM][MAX_DUP_CQ_NUM];
        static IMemDrv*         m_pIMemDrv ; /* IMEM */
        static CrzDrv*          m_pDipCrzDrv; /* CRZ Driver */
        static MdpMgr*          m_pMdpMgr; /* MdpMgr */
    public:

        //
        friend class DIP_ISP_PIPE;

        //Raw
        static MINT32 setUnp(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setUfd(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setRcp2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setDbs2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setOb2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setBnr2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setRmg2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setRmm2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setLsc2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2g(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2h(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setPgn(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setRnr(DIP_ISP_PIPE &imageioPackage);
        //Rgb
        static MINT32 setHlr2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setUdm(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setG2g(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setLce(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setGgm(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setFlc(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setFlc2(DIP_ISP_PIPE &imageioPackage);
        //Yuv
        static MINT32 setC24(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setC02b(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setC02(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setMfb(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setG2c(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setC42(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2b(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2c(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2d(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2e(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2i(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSl2k(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setMfbw(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setNbc(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setNbc2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setNdg(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setNdg2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSrz1(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSrz2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSrz3(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSrz4(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSrz5(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setMix1(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setMix2(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setMix3(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setPca(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setSeee(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setCrsp(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setNr3d(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setFe(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setC24b(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setFm(DIP_ISP_PIPE &imageioPackage);
        static MINT32 setHfg(DIP_ISP_PIPE &imageioPackage);

};

/*/////////////////////////////////////////////////////////////////////////////
    Isp driver shell
/////////////////////////////////////////////////////////////////////////////*/
class IspDrvShell:virtual public IspDrv_B
{
    protected:
        IspDrvShell(){}
        virtual             ~IspDrvShell() {};
    public:
        static IspDrvShell* createInstance();
        virtual void        destroyInstance(void) = 0;
        virtual MBOOL       init(const char* userName="") = 0;
        virtual MBOOL       uninit(const char* userName="") = 0;
        //
        friend class IspFunctionDip_B;

    public://commandQ operation
        mutable Mutex   mLock;

};


/*/////////////////////////////////////////////////////////////////////////////
    IspFunctionDip_B
  /////////////////////////////////////////////////////////////////////////////*/
class IspFunctionDip_B
{
private:
    mutable Mutex           mLock;
protected://isp driver operation
    //static IspDrvDipPhy*    m_pDrvDipPhy;
    //static IspDrvDip*****   m_pDrvDip;

public://isp driver operation

    static MBOOL isDipOnly;
    static unsigned int dipTh;
    static unsigned int hwModule;
    static unsigned int moduleIdx;
    static unsigned int dupCqIdx;
    static unsigned int burstQueIdx; // for burst queue number
    static MBOOL isApplyTuning;
    static dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider

    int                     bypass;
    static IspDrvShell*     m_pIspDrvShell;
public:
    IspFunctionDip_B(): bypass(0){};
    virtual ~IspFunctionDip_B(){};
public:
    virtual unsigned long   id( void ) = 0;
    virtual const char*     name_Str( void ) = 0;   //Name string of module
    virtual int             is_bypass( void ) {return bypass;}
    MUINT32                 switchHwModuleToHwIdx(ISP_HW_MODULE hwModule);

public: //isp driver operationr
    inline int              waitIrq( ISP_WAIT_IRQ_ST* pWaitIrq ){return (int)m_pIspDrvShell->m_pDrvDipPhy->waitIrq( pWaitIrq );}
    mutable Mutex           queHwLock;

public:
    virtual int     config( void ) = 0;
    virtual int     enable( void* pParam ) = 0;
    virtual int     disable( void* pParam ) = 0;
    virtual int     write2CQ( void ) = 0 ;

};

/*/////////////////////////////////////////////////////////////////////////////
    DMA
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA:public IspFunctionDip_B
{
public:/*[member veriable]*/
    /*
        ->image input(imgi,imgci,nr3i,flki,lcei,vipi,vip2i)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspMemBuffer.ofst_addr           ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZW
                IspSize.stride                          ->stride
        ->others(TDR,...)
                IspMemBuffer.base_pAddr         ->base_pAddr
                IspMemBuffer.ofst_addr          ->ofst_addr
                IspSize.w                              -> XSIZE
        ->others(CQ,...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspSize.w                              -> XSIZE
        */

    IspDMACfg   dma_cfg;
        int dma_ID;

private:

public: /*ctor/dtor*/
        DIP_DMA(const int& dmaID):dma_ID(-1)
            {dma_cfg.pixel_byte = 1; dma_ID = dmaID;};

public:/*[IspFunctionDip_B]*/
    virtual int config( void );
    virtual int enable( void* pParam ){pParam; return 0;}
    virtual int disable( void* pParam ){pParam; return 0;}
    virtual int write2CQ( void );
    virtual unsigned long id( void ) {return this->dma_ID;}
    virtual const char*   name_Str( void )
    {   //ONLY FOR DEBUG USAGE
        switch(this->dma_ID)
        {
            case ENUM_DIP_DMA_CQI:
                return     "DIP_DMA_CQI";
            case ENUM_DIP_DMA_TDRI:
                return     "DIP_DMA_TDRI";
            case ENUM_DIP_DMA_IMGI:
                return     "DIP_DMA_IMGI";
            case ENUM_DIP_DMA_IMGBI:
                return     "DIP_DMA_IMGBI";
            case ENUM_DIP_DMA_IMGCI:
                return     "DIP_DMA_IMGCI";
            case ENUM_DIP_DMA_UFDI:
                return     "DIP_DMA_UFDI";
            case ENUM_DIP_DMA_LCEI:
                return     "DIP_DMA_LCEI";
            case ENUM_DIP_DMA_VIPI:
                return     "DIP_DMA_VIPI";
            case ENUM_DIP_DMA_VIP2I:
                return     "DIP_DMA_VIP2I";
            case ENUM_DIP_DMA_VIP3I:
                return     "DIP_DMA_VIP3I";
            case ENUM_DIP_DMA_DMGI:
                return     "DIP_DMA_DMGI";
            case ENUM_DIP_DMA_DEPI:
                return     "DIP_DMA_DEPI";
            case ENUM_DIP_DMA_MFBO:
                return     "DIP_DMA_MFBO";
            case ENUM_DIP_DMA_IMG3O:
                return     "DIP_DMA_IMG3O";
            case ENUM_DIP_DMA_IMG3BO:
                return     "DIP_DMA_IMG3BO";
            case ENUM_DIP_DMA_IMG3CO:
                return     "DIP_DMA_IMG3CO";
            case ENUM_DIP_DMA_FEO:
                return     "DIP_DMA_FEO";
            case ENUM_DIP_DMA_IMG2O:
                return     "DIP_DMA_IMG2O";
            case ENUM_DIP_DMA_IMG2BO:
                return     "DIP_DMA_IMG2BO";
            default:
                return     "WRONG......";
        }
    }
    public: /*[DIP_DMA]*/

    private: /*[DIP_DMA]*/

};

class Crz_Cfg
{
public:
    int conf_cdrz;
    IspSize crz_in;
    IspSize crz_out;
    IspRect crz_crop;
    MUINT32 hAlgo,vAlgo;
    MUINT32 hTable, vTable;
    MUINT32 hCoeffStep,vCoeffStep;
    int tpipeMode;

    Crz_Cfg():conf_cdrz(0),hAlgo(0),vAlgo(0),hTable(0),vTable(0),hCoeffStep(0),vCoeffStep(0),tpipeMode(CRZ_DRV_MODE_TPIPE){};
};


/*/////////////////////////////////////////////////////////////////////////////
    DIP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_ISP_PIPE : public IspFunctionDip_B
{
public:
    //
    struct ST_DIP_TOP_CTRL  isp_top_ctl;

    MINT32 irqUserKey; // get the user key when call function "registerIrq" for each dip, and user use this user key to call function "waitirq"

    //common
    int             m_isp_function_count;
    IspFunctionDip_B*   m_isp_function_list[ENUM_DIP_FUNCTION_MAX_NUM];

    DIP_DMA      DMAImgi{ENUM_DIP_DMA_IMGI};
    DIP_DMA      DMAImgbi{ENUM_DIP_DMA_IMGBI};
    DIP_DMA      DMAImgci{ENUM_DIP_DMA_IMGCI};
    DIP_DMA      DMAVipi{ENUM_DIP_DMA_VIPI};
    DIP_DMA      DMAVip2i{ENUM_DIP_DMA_VIP2I};
    DIP_DMA      DMAVip3i{ENUM_DIP_DMA_VIP3I};
    DIP_DMA      DMADepi{ENUM_DIP_DMA_DEPI};
    DIP_DMA      DMADmgi{ENUM_DIP_DMA_DMGI};
    DIP_DMA      DMAUfdi{ENUM_DIP_DMA_UFDI};
    DIP_DMA      DMALcei{ENUM_DIP_DMA_LCEI};
    DIP_DMA      DMAImg2o{ENUM_DIP_DMA_IMG2O};
    DIP_DMA      DMAImg2bo{ENUM_DIP_DMA_IMG2BO};
    DIP_DMA      DMAImg3o{ENUM_DIP_DMA_IMG3O};
    DIP_DMA      DMAImg3bo{ENUM_DIP_DMA_IMG3BO};
    DIP_DMA      DMAImg3co{ENUM_DIP_DMA_IMG3CO};
    DIP_DMA      DMAMfbo{ENUM_DIP_DMA_MFBO};
    DIP_DMA      DMAFeo{ENUM_DIP_DMA_FEO};

    CrspCfg crspCfg;
    MfbCfg mfb_cfg;
    SrzCfg srz1Cfg;
    SrzCfg srz2Cfg;
    SrzCfg srz3Cfg;
    SrzCfg srz4Cfg;
    SrzCfg srz5Cfg;

    Crz_Cfg crzPipe;
    //
    IspDrvDipPhy* pDrvDipPhy;
    IspDrvDip* pDrvDip;
    int dip_cq_thr_ctl;
    int tdr_ctl;
    int tdr_tpipe;
    int tdr_tcm_en;
    int tdr_tcm2_en;

    MUINTPTR pRegiAddr;
    unsigned int regCount;
    unsigned int* pReadAddrList;


public:
    DIP_ISP_PIPE():irqUserKey(0),m_isp_function_count(0),pDrvDipPhy(NULL),pDrvDip(NULL),dip_cq_thr_ctl(0),tdr_ctl(0),tdr_tpipe(0),
        tdr_tcm_en(0),tdr_tcm2_en(0),pRegiAddr(0x0),regCount(0),pReadAddrList(NULL){

        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&(*this);
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImgi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImgbi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImgci;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAUfdi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMALcei;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAVipi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAVip2i;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAVip3i;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMADepi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMADmgi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAMfbo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg2o;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg2bo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg3o;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg3bo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg3co;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAFeo;

    };

    virtual unsigned long id( void )                    {   return ENUM_DIP_TOP;  }
    virtual const char*   name_Str( void )              {   return "DIP_TOP";}
    int configTpipeData( void );
    int ConfigDipPipe( void );

public:
    virtual int config( void );
    virtual int enable( void* pParam );
    virtual int disable( void* pParam );
    virtual int write2CQ( void );
    
    virtual MBOOL   CalAlgoAndCStep(
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
        MUINT32                 *pCoeffStep_V);


protected:
    IspFunctionDip_B**  isp_function_list()  {   return m_isp_function_list; }
    int             isp_function_count() {return m_isp_function_count; }
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_MDP
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_MDP_PIPE:public IspFunctionDip_B
{
public:
    //IspRect src_crop;
    MdpRotDMACfg wroto_out;
    MdpRotDMACfg wdmao_out;
    //MdpRotDMACfg jpego_out;
    MdpRotDMACfg imgxo_out;
    MdpRotDMACfg venco_out;
    int mdp_imgxo_p2_en;
    unsigned int yuv2_en;

    MBOOL lastframe;  //last frame in enque package or not
    unsigned int lastframe_dupCqIdx;
    unsigned int lastframe_burstQueIdx; // for burst queue number
    unsigned int lastframe_dipTh;
    unsigned int dip_dma_en;
    int drvScenario;
    MBOOL isWaitBuf;
    int dipCtlDmaEn;
    //
    EImageFormat p2MdpSrcFmt;
    unsigned int p2MdpSrcW;
    unsigned int p2MdpSrcH;
    unsigned int p2MdpSrcYStride;
    unsigned int p2MdpSrcUVStride;
    unsigned int p2MdpSrcSize;
    unsigned int p2MdpSrcCSize;
    unsigned int p2MdpSrcVSize;
    unsigned int p2MdpSrcPlaneNum;
    IspDMACfg   imgi_dma_cfg;
    //MVOID       *DCParam;
    //MVOID       *CZParam; // For clear zoom
    //MVOID       *VSDOFPQParam;
    MVOID       *WDMAPQParam;
    MVOID       *WROTPQParam;
public:
    DIP_MDP_PIPE():mdp_imgxo_p2_en(0),yuv2_en(0),lastframe(MFALSE),lastframe_dupCqIdx(0),lastframe_burstQueIdx(0),lastframe_dipTh(ISP_DRV_DIP_CQ_THRE0),
        dip_dma_en(0),drvScenario(-1),isWaitBuf(MTRUE),dipCtlDmaEn(0),p2MdpSrcFmt(eImgFmt_YV12),p2MdpSrcW(0),p2MdpSrcH(0),p2MdpSrcYStride(0),p2MdpSrcUVStride(0),p2MdpSrcSize(0),p2MdpSrcCSize(0),
        p2MdpSrcVSize(0),p2MdpSrcPlaneNum(0),WDMAPQParam(NULL),WROTPQParam(NULL){};

    ~DIP_MDP_PIPE(){};

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_MDP;  }
    virtual const char*   name_Str( void )              {   return "DIP_MDP";}

    int configMDP( void );

public:
    virtual int config( void ){return 0;}
    virtual int enable( void* pParam ){pParam; return 0;}
    virtual int disable( void* pParam ){pParam; return 0;}
    virtual int write2CQ( void ){return 0;}

public:
    MBOOL startVencLink( MINT32 fps, MINT32 wd,MINT32 ht);
    MBOOL stopVencLink();
};



typedef enum
{
    ISP_BUF_EMPTY = 0,
    ISP_BUF_FILLED,
}ISP_BUF_STATUS;
//
#if 0
typedef enum
{
    ISP_BUF_TYPE_PMEM    = BUF_TYPE_PMEM,
    ISP_BUF_TYPE_STD_M4U = BUF_TYPE_STD_M4U,
    ISP_BUF_TYPE_ION     = BUF_TYPE_ION,
}ISP_BUF_TYPE;
#endif
//


//
struct stISP_BUF_INFO
{
    ISP_BUF_STATUS          status;
    MUINTPTR                base_vAddr;
    MUINTPTR                base_pAddr;
    MUINT32                 size;
    MUINT32                 memID;
    MINT32                  bufSecu;
    MINT32                  bufCohe;
    MUINT64                 timeStampS;
    MUINT32                 timeStampUs;
    //
    MUINT32                 img_w;
    MUINT32                 img_h;
    MUINT32                 img_stride;
    MUINT32                 img_fmt;
    MUINT32                 img_pxl_id;
    MUINT32                 img_wbn;
    MUINT32                 img_ob;
    MUINT32                 img_lsc;
    MUINT32                 img_rpg;
    MUINT32                 m_num_0;
    MUINT32                 m_num_1;
    MUINT32                 frm_cnt;
    MUINT32                 raw_type;
    MUINT32                 dma;
    //
    IspRect                 dma_crop;   //dmao cropping
    MVOID                   *header_info; //point to self
    MUINT32                 header_size;
    MINT32                  mBufIdx; // used for replace buffer

    //
    MUINT32                 jpg_size;
    MVOID                   *private_info;
    struct stISP_BUF_INFO   *next;
    //
    stISP_BUF_INFO(
        ISP_BUF_STATUS          _status = ISP_BUF_EMPTY,
        MUINTPTR                 _base_vAddr = 0,
        MUINTPTR                 _base_pAddr = 0,
        MUINT32                 _size = 0,
        MUINT32                 _memID = -1,
        MINT32                  _bufSecu = 0,
        MINT32                  _bufCohe = 0,
        MUINT64                 _timeStampS = 0,
        MUINT32                 _timeStampUs = 0,
        MUINT32                 _img_w = 0,
        MUINT32                 _img_h = 0,
        /*MUINT32                 _img_fmt = 0,*/
        /*MUINT32                 _img_hbin1 = 0,*/
        /*MUINT32                 _img_ob = 0,*/
        /*MUINT32                 _img_lsc = 0,*/
        /*MUINT32                 _img_rpg = 0,*/
        MUINT32                 jpg_size = 0,
        MVOID                   *_private_info = NULL,
        MUINT32                 dma = _imgo_,
        struct stISP_BUF_INFO   *_next = 0)
    : status(_status)
    , base_vAddr(_base_vAddr)
    , base_pAddr(_base_pAddr)
    , size(_size)
    , memID(_memID)
    , bufSecu(_bufSecu)
    , bufCohe(_bufCohe)
    , timeStampS(_timeStampS)
    , timeStampUs(_timeStampUs)
    , img_w(_img_w)
    , img_h(_img_h)
    , img_stride(0)
    , img_fmt(0)
    , img_pxl_id(0)
    , img_wbn(0)
    , img_ob(0)
    , img_lsc(0)
    , img_rpg(0)
    , m_num_0(0)
    , m_num_1(0)
    , frm_cnt(0)
    , raw_type(0)
    , dma(dma)
    , header_info(NULL)
    , header_size(0)
    , mBufIdx(0)
    , jpg_size(jpg_size)
    , private_info(_private_info)
    , next(_next)
    {}
};


/*/////////////////////////////////////////////////////////////////////////////
    DIP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
//
typedef list<stISP_BUF_INFO> ISP_BUF_INFO_L;
//
typedef struct  _isp_buf_list_{
    MUINT32         filledCnt;            //  fill count
    ISP_BUF_INFO_L  bufInfoList;
}stISP_BUF_LIST;
//
typedef struct  _isp_filled_buf_list_{
    ISP_BUF_INFO_L  *pBufList;
}stISP_FILLED_BUF_LIST;


#define ISP_BUF_INFO_NODE_MAX_CNT 32
typedef struct  _isp_buf_header_node_{
    stISP_BUF_INFO  mInfo; //put this in the begin of the struct to reduce isp_container_of overhead
    MBOOL           mOccupied;
}stISP_BUF_HEADER_NODE;

typedef struct  _isp_buf_header_list_{
    mutable Mutex          mLLock;
    MUINT32                mIdx;
    stISP_BUF_HEADER_NODE  mN[ISP_BUF_INFO_NODE_MAX_CNT];
}stISP_BUF_HEADER_LIST;

//
class DIP_BUF_CTRL : public IspFunctionDip_B
{
//friend class IspDrvShellImp;
friend class DIP_TOP_CTRL;

private:

    static stISP_BUF_LIST m_p2HwBufL[ISP_DRV_DIP_CQ_NUM][MAX_DUP_CQ_NUM][_rt_dma_max_];

public:
    int m_BufdrvScenario;
    int m_BufdipTh;  // for P2 deque use
    unsigned int m_BufdupCqIdx;
    unsigned int m_BufburstQueIdx; // for burst queue number
    unsigned int m_BufmoduleIdx;
public:
    DIP_BUF_CTRL():m_BufdrvScenario(-1),m_BufdipTh(ISP_DRV_DIP_CQ_THRE0),m_BufdupCqIdx(0),m_BufburstQueIdx(0),m_BufmoduleIdx(0)
        {};
    virtual unsigned long id( void )                    {   return ENUM_DIP_BUFFER;  }
    virtual const char*   name_Str( void )              {   return "DIP_BUFFER";}
    //virtual int init( MUINT32 dmaChannel );
    //virtual MBOOL waitBufReady( MUINT32 dmaChannel );
    /*
        * dequeueMdpFrameEnd
        *       set mdp deque frame end for each frame end
        */
    virtual MINT32 dequeueMdpFrameEnd( MINT32 drvScenario, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx);
    /*
        * dequeueHwBuf
        *       set buffer FILLED type and
        *       move filled Hw buffer to sw the end of list.
        */
    virtual EIspRetStatus dequeueHwBuf( MUINT32 dmaChannel);

    virtual EIspRetStatus dequeHwBuf( MBOOL& isVencContained);
private:

protected:
    virtual int     config( void ) {return 0;}
    virtual int     enable( void* pParam ) {pParam; return 0;}
    virtual int     disable( void* pParam ) {pParam; return 0;}
    virtual int     write2CQ( void ) {return 0;}

};

////////////////////////////////////////////////////////////////////////////////


#endif /*__ISP_FUNCTION_DIP_H__*/




