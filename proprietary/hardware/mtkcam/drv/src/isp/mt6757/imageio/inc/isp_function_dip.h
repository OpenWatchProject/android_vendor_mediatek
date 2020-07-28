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
#define CROP_TPIPE_PECISE_BIT   20
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
    ENUM_DIP_DMA_LCEI,   //15
    ENUM_DIP_DMA_VIPI,
    ENUM_DIP_DMA_VIP2I,
    ENUM_DIP_DMA_VIP3I,
    ENUM_DIP_DMA_DMGI,//new
    ENUM_DIP_DMA_DEPI,//new
    ENUM_DIP_DMA_MFBO,
    ENUM_DIP_DMA_IMG3O,
    ENUM_DIP_DMA_IMG3BO,
    ENUM_DIP_DMA_IMG3CO, //25
    ENUM_DIP_DMA_FEO,
    ENUM_DIP_DMA_IMG2O,  //35
    ENUM_DIP_DMA_IMG2BO,//new
    ENUM_DIP_DMA_WDMAO,
    ENUM_DIP_DMA_WROTO,
    ENUM_DIP_DMA_JPEGO,
    ENUM_DIP_DMA_VENC_STREAMO,
    ENUM_DIP_BUFFER,
    ENUM_DIP_TURNING,
    ENUM_DIP_CRZ,
    ENUM_DIP_FUNCTION_MAX_NUM
};

//isp top register range
#define DIP_ISP_TOP_BASE             (ISP_BASE_HW + 0x22000)
#define DIP_ISP_TOP_REG_RANGE        (0x00)  //kk test
//ISP_RAW
#define DIP_ISP_RAW_BASE             (DIP_ISP_TOP_BASE + 0x0)
#define DIP_ISP_RAW_REG_RANGE        DIP_ISP_TOP_REG_RANGE
//ISP_RGB
#define DIP_ISP_RGB_BASE             (DIP_ISP_TOP_BASE + 0x0)
#define DIP_ISP_RGB_REG_RANGE        DIP_ISP_TOP_REG_RANGE
//ISP_YUV
#define DIP_ISP_YUV_BASE             (DIP_ISP_TOP_BASE + 0x0)
#define DIP_ISP_YUV_REG_RANGE        DIP_ISP_TOP_REG_RANGE
//ISP_MDP
#define DIP_ISP_MDP_BASE             (DIP_ISP_TOP_BASE + 0x0)
#define DIP_ISP_MDP_REG_RANGE        DIP_ISP_TOP_REG_RANGE
//ISP_CRZ
#define DIP_ISP_CRZ_BASE             (DIP_ISP_TOP_BASE + 0x0)
#define DIP_ISP_CRZ_REG_RANGE        0x1000
//ISP_DMA_IMGI
#define DIP_DMA_IMGI_BASE            (DIP_ISP_TOP_BASE + 0x3230)
#define DIP_DMA_IMGI_REG_RANGE       0x20
//ISP_DMA_IMGBI
#define DIP_DMA_IMGBI_BASE            (DIP_ISP_TOP_BASE + 0x3230)
#define DIP_DMA_IMGBI_REG_RANGE       0x20
//ISP_DMA_IMGCI
#define DIP_DMA_IMGCI_BASE            (DIP_ISP_TOP_BASE + 0x3230)
#define DIP_DMA_IMGCI_REG_RANGE       0x20
//ISP_DMA_DMGI
#define DIP_DMA_DMGI_BASE            (DIP_ISP_TOP_BASE + 0x3288)
#define DIP_DMA_DMGI_REG_RANGE       0x1C
//ISP_DMA_DEPI
#define DIP_DMA_DEPI_BASE            (DIP_ISP_TOP_BASE + 0x3288)
#define DIP_DMA_DEPI_REG_RANGE       0x1C
//ISP_DMA_UFDI
#define DIP_DMA_UFDI_BASE            (DIP_ISP_TOP_BASE + 0x3288)
#define DIP_DMA_UFDI_REG_RANGE       0x1C
//ISP_DMA_LCEI
#define DIP_DMA_LCEI_BASE            (DIP_ISP_TOP_BASE + 0x32A4)
#define DIP_DMA_LCEI_REG_RANGE       0x1C
//ISP_DMA_VIPI
#define DIP_DMA_VIPI_BASE            (DIP_ISP_TOP_BASE + 0x32C0)
#define DIP_DMA_VIPI_REG_RANGE       0x20
//ISP_DMA_VIP2I
#define DIP_DMA_VIP2I_BASE            (DIP_ISP_TOP_BASE + 0x32E0)
#define DIP_DMA_VIP2I_REG_RANGE       0x20
//ISP_DMA_IMGO
#define DIP_DMA_IMGO_BASE            (DIP_ISP_TOP_BASE + 0x3300)
#define DIP_DMA_IMGO_REG_RANGE       0x20
//ISP_DMA_RRZO
#define DIP_DMA_RRZO_BASE            (DIP_ISP_TOP_BASE + 0x3320)
#define DIP_DMA_RRZO_REG_RANGE       0x20
//ISP_DMA_RRZO_D
#define DIP_DMA_RRZO_D_BASE            (DIP_ISP_TOP_BASE + 0x2F0)
#define DIP_DMA_RRZO_D_REG_RANGE       0x20
//ISP_DMA_LCSO
#define DIP_DMA_LCSO_BASE            (DIP_ISP_TOP_BASE + 0x3340)
#define DIP_DMA_LCSO_REG_RANGE       0x1C
//ISP_DMA_EISO
#define DIP_DMA_EISO_BASE            (DIP_ISP_TOP_BASE + 0x335C)
#define DIP_DMA_EISO_REG_RANGE       0x8
//ISP_DMA_AFO
#define DIP_DMA_AFO_BASE             (DIP_ISP_TOP_BASE + 0x3364)
#define DIP_DMA_AFO_REG_RANGE        0x8
//ISP_DMA_ESFKO
#define DIP_DMA_ESFKO_BASE            (DIP_ISP_TOP_BASE + 0x336C)
#define DIP_DMA_ESFKO_REG_RANGE       0x1C
//ISP_DMA_AAO
#define DIP_DMA_AAO_BASE            (DIP_ISP_TOP_BASE + 0x3388)
#define DIP_DMA_AAO_REG_RANGE       0x1C
//ISP_DMA_VIP3I
#define DIP_DMA_VIP3I_BASE            (DIP_ISP_TOP_BASE + 0x33A4)
#define DIP_DMA_VIP3I_REG_RANGE       0x20
//ISP_DMA_UFEO
#define DIP_DMA_UFEO_BASE            (DIP_ISP_TOP_BASE + 0x33C4)
#define DIP_DMA_UFEO_REG_RANGE       0x1C
//ISP_DMA_MFBO
#define DIP_DMA_MFBO_BASE            (DIP_ISP_TOP_BASE + 0x33E0)
#define DIP_DMA_MFBO_REG_RANGE       0x20
//ISP_DMA_IMG3BO
#define DIP_DMA_IMG3BO_BASE            (DIP_ISP_TOP_BASE + 0x3400)
#define DIP_DMA_IMG3BO_REG_RANGE       0x20
//ISP_DMA_IMG3CO
#define DIP_DMA_IMG3CO_BASE            (DIP_ISP_TOP_BASE + 0x3420)
#define DIP_DMA_IMG3CO_REG_RANGE       0x1C
//ISP_DMA_IMG2O
#define DIP_DMA_IMG2O_BASE            (DIP_ISP_TOP_BASE + 0x3440)
#define DIP_DMA_IMG2O_REG_RANGE       0x20
//ISP_DMA_IMG2BO
#define DIP_DMA_IMG2BO_BASE            (DIP_ISP_TOP_BASE + 0x3440)
#define DIP_DMA_IMG2BO_REG_RANGE       0x20
//ISP_DMA_IMG3O
#define DIP_DMA_IMG3O_BASE            (DIP_ISP_TOP_BASE + 0x3460)
#define DIP_DMA_IMG3O_REG_RANGE       0x20
//ISP_DMA_FEO
#define DIP_DMA_FEO_BASE            (DIP_ISP_TOP_BASE + 0x3480)
#define DIP_DMA_FEO_REG_RANGE       0x1C
//ISP_DMA_CQ0I
#define DIP_DMA_CQI_BASE            (DIP_ISP_TOP_BASE + 0x3210)
#define DIP_DMA_CQI_REG_RANGE       0x08
//ISP_DMA_TDRI
#define DIP_DMA_TDRI_BASE            (DIP_ISP_TOP_BASE + 0x3204)
#define DIP_DMA_TDRI_REG_RANGE       0xC




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
    ISP_TPIPE_CONFIG_FE_STRUCT fe;
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


 class IspImageioPackage
 {
 public:
     MBOOL isApplyTuning;
     MUINT32 yuv_en;
     MUINT32 yuv2_en;
     MUINT32 rgb_en;
     MUINT32 enFgMode;
     IspDrvDipPhy* pDrvDipPhy;
     IspDrvDip* pDrvDip;
     dip_x_reg_t *pTuningIspReg;
     MUINT32 hwModule;
     CrspCfg crspCfg;
     C02Cfg	c02Cfg;
     C02Cfg	c02bCfg;
     SrzCfg srz1Cfg;
     SrzCfg srz2Cfg;
     SrzCfg srz3Cfg;
     SrzCfg srz4Cfg;
     MfbCfg mfbCfg;
     //Nr3DCfg nr3dCfg;
     LceCfg lceCfg;
     CrzCfg crzCfg;
     //DrzCfg drzCfg;

     IspImageioPackage()
     : enFgMode(0x0),isApplyTuning(0),yuv_en(0),yuv2_en(0),rgb_en(0),pDrvDipPhy(NULL),pDrvDip(NULL),pTuningIspReg(NULL),hwModule(0)
     {}
 };


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
    MINT32 (*default_func)(IspImageioPackage &imageioPackage); // run individual function
 };

 class IspTopModuleMapping3LayerTable{
 public:
    MINT64 ctrlByte;  // isp register byte
    EIspModule eModule;  // module
    DIP_A_MODULE_ENUM eCqThrFunc1;
    DIP_A_MODULE_ENUM eCqThrFunc2;
    DIP_A_MODULE_ENUM eCqThrFunc3;
    MINT32 (*default_func)(IspImageioPackage &imageioPackage); // run individual function
 };

enum EIspControlBufferCmd
{
    eIspControlBufferCmd_Alloc  = 0,
    eIspControlBufferCmd_Free,
    eIspControlBufferCmd_Num
};


//
/**/
class IspFunctionDip_B;

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
        //Raw
        static MINT32 setUnp(IspImageioPackage &imageioPackage);
        static MINT32 setUfd(IspImageioPackage &imageioPackage);
        static MINT32 setRcp2(IspImageioPackage &imageioPackage);
        static MINT32 setDbs2(IspImageioPackage &imageioPackage);
        static MINT32 setOb2(IspImageioPackage &imageioPackage);
        static MINT32 setBnr2(IspImageioPackage &imageioPackage);
        static MINT32 setRmg2(IspImageioPackage &imageioPackage);
        static MINT32 setRmm2(IspImageioPackage &imageioPackage);
        static MINT32 setLsc2(IspImageioPackage &imageioPackage);
        static MINT32 setSl2(IspImageioPackage &imageioPackage);
        static MINT32 setSl2g(IspImageioPackage &imageioPackage);
        static MINT32 setSl2h(IspImageioPackage &imageioPackage);
        static MINT32 setPgn(IspImageioPackage &imageioPackage);
        static MINT32 setRnr(IspImageioPackage &imageioPackage);
        //Rgb
        static MINT32 setUdm(IspImageioPackage &imageioPackage);
        static MINT32 setG2g(IspImageioPackage &imageioPackage);
        static MINT32 setLce(IspImageioPackage &imageioPackage);
        static MINT32 setGgm(IspImageioPackage &imageioPackage);
        //Yuv
        static MINT32 setC24(IspImageioPackage &imageioPackage);
        static MINT32 setC02b(IspImageioPackage &imageioPackage);
        static MINT32 setC02(IspImageioPackage &imageioPackage);
        static MINT32 setMfb(IspImageioPackage &imageioPackage);
        static MINT32 setG2c(IspImageioPackage &imageioPackage);
        static MINT32 setC42(IspImageioPackage &imageioPackage);
        static MINT32 setSl2b(IspImageioPackage &imageioPackage);
        static MINT32 setSl2c(IspImageioPackage &imageioPackage);
        static MINT32 setSl2d(IspImageioPackage &imageioPackage);
        static MINT32 setSl2e(IspImageioPackage &imageioPackage);
        static MINT32 setSl2i(IspImageioPackage &imageioPackage);
        static MINT32 setMfbw(IspImageioPackage &imageioPackage);
        static MINT32 setNbc(IspImageioPackage &imageioPackage);
        static MINT32 setNbc2(IspImageioPackage &imageioPackage);
        static MINT32 setSrz1(IspImageioPackage &imageioPackage);
        static MINT32 setSrz2(IspImageioPackage &imageioPackage);
        static MINT32 setSrz3(IspImageioPackage &imageioPackage);
        static MINT32 setSrz4(IspImageioPackage &imageioPackage);
        static MINT32 setMix1(IspImageioPackage &imageioPackage);
        static MINT32 setMix2(IspImageioPackage &imageioPackage);
        static MINT32 setMix3(IspImageioPackage &imageioPackage);
        static MINT32 setPca(IspImageioPackage &imageioPackage);
        static MINT32 setSeee(IspImageioPackage &imageioPackage);
        static MINT32 setCrsp(IspImageioPackage &imageioPackage);
        static MINT32 setNr3d(IspImageioPackage &imageioPackage);
        static MINT32 setFe(IspImageioPackage &imageioPackage);
        static MINT32 setC24b(IspImageioPackage &imageioPackage);
        static MINT32 setFm(IspImageioPackage &imageioPackage);
        static MINT32 setHfg(IspImageioPackage &imageioPackage);

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
    //isp top camera base address
    static unsigned long    m_Isp_Top_Reg_Base_Addr;   //maybe virtual address
    //function base address
    unsigned long           m_Reg_Base_Addr;          //maybe virtual address
    mutable Mutex           mLock;
protected://isp driver operation
    //static IspDrvDipPhy*    m_pDrvDipPhy;
    //static IspDrvDip*****   m_pDrvDip;


public://isp driver operation
    int                     bypass;
    static IspDrvShell*     m_pIspDrvShell;
public:
    IspFunctionDip_B(): m_Reg_Base_Addr(0),bypass(0){};
    virtual ~IspFunctionDip_B(){};
public:
    virtual int checkBusy( unsigned long* param ) = 0;
    virtual unsigned long   id( void ) = 0;
    virtual const char*     name_Str( void ) = 0;   //Name string of module
    virtual unsigned long   reg_Base_Addr_Phy( void ) = 0;      //Phy reg base address, usually a fixed value
    virtual unsigned long   reg_Range( void ) = 0;
    virtual int             is_bypass( void ) {return bypass;}

    unsigned long           dec_Id( void );         //Decimal ID number
    static unsigned long    isp_Top_Reg_Base_Addr( void )    {   return m_Isp_Top_Reg_Base_Addr;   }
    static unsigned long    isp_Top_Reg_Base_Addr_Phy( void ) {   return DIP_ISP_TOP_BASE;  }//Phy reg base address, usually a fixed value
    unsigned long           reg_Base_Addr( void )           {   return m_Reg_Base_Addr;   }
    static void             remap_Top_Reg_Base_Addr( unsigned long new_addr )      {   m_Isp_Top_Reg_Base_Addr = new_addr;}
    void                    remap_Reg_Base_Addr( unsigned long new_addr )          {   m_Reg_Base_Addr = new_addr;         }
    //
    MUINT32                 switchHwModuleToHwIdx(ISP_HW_MODULE hwModule);

public: //isp driver operationr
    inline int              waitIrq( ISP_WAIT_IRQ_ST* pWaitIrq ){return (int)m_pIspDrvShell->m_pDrvDipPhy->waitIrq( pWaitIrq );}
    mutable Mutex           queHwLock;

public:
    int             config( void );
    int             enable( void* pParam);
    int             disable( void* pParam);
    int             write2CQ( void );
    int             setZoom( void );
protected:
    virtual int     _config( void ) = 0;
    virtual int     _enable( void* pParam ) = 0;
    virtual int     _disable( void* pParam ) = 0;
    virtual int     _write2CQ( void ) {return 0;}
    virtual int     _setZoom( void ) {return 0;}
};

/*/////////////////////////////////////////////////////////////////////////////
    DMAI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMAI_B:public IspFunctionDip_B
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
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    MBOOL isApplyTuning;
    MBOOL isDipOnly;
    int dip_cq_thr_ctl;
//    int isP2IspOnly;

private:

public: /*ctor/dtor*/
    DIP_DMAI_B():dupCqIdx(0),burstQueIdx(0),dipTh(ISP_DRV_DIP_CQ_THRE0),hwModule(DIP_A),moduleIdx(0),isApplyTuning(MTRUE),isDipOnly(MFALSE),dip_cq_thr_ctl(0)
        {dma_cfg.pixel_byte = 1;};
public: /*[IspFunctionDip_B]*/
    virtual int checkBusy( unsigned long* param ){return 0;}
private:/*[IspFunctionDip_B]*/
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void );
public: /*[DIP_DMAI_B]*/


private: /*[DIP_DMAI_B]*/


};

/*/////////////////////////////////////////////////////////////////////////////
    DMAO
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMAO_B:public IspFunctionDip_B
{
public:/*[member veriable]*/
    /*
        ->image ouput(imgo,img2o,)
                IspMemBuffer.base_pAddr           ->base_pAddr
                IspMemBuffer.ofst_addr            ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZE
                IspSize.stride                          ->stride
                IspRect.x                               ->crop xoffset
                IspRect.y                               ->crop yoffset
        ->others(lcso,ESFKO(FLKO),AAO,NR3O,...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspMemBuffer.ofst_addr            ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZE
                IspSize.stride                          ->stride
        ->others(EISO,AFO...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspSize.w                              -> XSIZE
        */

    IspDMACfg   dma_cfg;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;

private:

public:
    DIP_DMAO_B():dupCqIdx(0),burstQueIdx(0),dipTh(ISP_DRV_DIP_CQ_THRE0),hwModule(DIP_A),moduleIdx(0)
        {dma_cfg.pixel_byte = 1;};

public: /*[IspFunctionDip_B]*/
    virtual int checkBusy( unsigned long* param ){return 0;}

private:/*[IspFunctionDip_B]*/
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void );

public: /*[DIP_DMAO_B]*/


private: /*[DIP_DMAO_B]*/


};

/*/////////////////////////////////////////////////////////////////////////////
    DIP_TUNNING_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_TUNNING_CTRL:public IspFunctionDip_B
{
public:
    int CQ;

public:
    DIP_TUNNING_CTRL():
        CQ(DIP_ISP_CQ_NONE){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_TURNING;  }
    virtual const char*   name_Str( void )              {   return "DIP_TURNING";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return (unsigned long)NULL;  }
    virtual unsigned long reg_Range( void )             {   return (unsigned long)NULL;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){ return 0;}
    virtual int _write2CQ( void ){ return 0;}
    virtual int _setZoom( void ){ return 0;}
};



/*/////////////////////////////////////////////////////////////////////////////
    DIP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_TOP_CTRL : public IspFunctionDip_B
{
public:
    //
    struct ST_DIP_TOP_CTRL  isp_top_ctl;
    //
    MBOOL isDipOnly;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    MBOOL isApplyTuning;
    //
    MINT32 irqUserKey; // get the user key when call function "registerIrq" for each dip, and user use this user key to call function "waitirq"
    //

public:
    DIP_TOP_CTRL():
		isDipOnly(MFALSE),
		dupCqIdx(0),
		burstQueIdx(0),
		dipTh(0),
		hwModule(DIP_A),
		moduleIdx(0),
		isApplyTuning(MFALSE),
		irqUserKey(0)
		{};

    virtual unsigned long id( void )                    {   return ENUM_DIP_TOP;  }
    virtual const char*   name_Str( void )              {   return "DIP_TOP";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return DIP_ISP_TOP_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_ISP_TOP_REG_RANGE;  }
    //virtual int is_bypass( void )                       {   return 0;       }
    virtual int checkBusy( unsigned long* param ){return 0;}

protected:
    virtual int _config( void );
    virtual int _enable( void* pParam );
    virtual int _disable( void* pParam );
    virtual int _write2CQ( void );
};



/*/////////////////////////////////////////////////////////////////////////////
    DIP_RAW_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_RAW_PIPE:public IspFunctionDip_B
{
public:
    unsigned int rgb_en;
    unsigned int dupCqIdx;  //current working Que
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
    MBOOL isApplyTuning;
    MUINT32 enFgMode; //for udm
 public:
    DIP_RAW_PIPE():
        rgb_en(0),
        dupCqIdx(0),
        burstQueIdx(0),
        dipTh(ISP_DRV_DIP_CQ_THRE0),
        hwModule(DIP_A),
        moduleIdx(0),
        pTuningIspReg(NULL),
        isApplyTuning(MFALSE),
        enFgMode(0x0)
        {};
public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_RAW;  }
    virtual const char*   name_Str( void )              {   return "DIP_RAW";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return DIP_ISP_RAW_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_ISP_RAW_REG_RANGE;  }
    //
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void );
private:

};

/*/////////////////////////////////////////////////////////////////////////////
    DIP_RGB_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_RGB_PIPE:public IspFunctionDip_B
{
public:
    unsigned int rgb_en;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    MBOOL isApplyTuning;
    LceCfg lceCfg;
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
public:
    DIP_RGB_PIPE():
		rgb_en(0),
		dupCqIdx(0),
		burstQueIdx(0),
		dipTh(ISP_DRV_DIP_CQ_THRE0),
		hwModule(DIP_A), 
		moduleIdx(0),
		isApplyTuning(MFALSE),
		pTuningIspReg(NULL)
    {};
    virtual ~DIP_RGB_PIPE(){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_RGB;  }
    virtual const char*   name_Str( void )              {   return "DIP_RGB";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return DIP_ISP_RGB_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_ISP_RGB_REG_RANGE;  }
    //
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void );
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_YUV
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_YUV_PIPE:public IspFunctionDip_B
{
public:
    //unsigned int en_p2;//
    unsigned int yuv_en;
    unsigned int yuv2_en;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    //int drvScenario;
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    MBOOL isApplyTuning;
    MFLOAT CrzRatio;
    C02Cfg  c02Cfg;
    C02Cfg  c02bCfg;
    MfbCfg mfb_cfg;
    SrzCfg srz1Cfg;
    SrzCfg srz2Cfg;
    SrzCfg srz3Cfg;
    SrzCfg srz4Cfg;
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
public:
    DIP_YUV_PIPE():
		yuv_en(0),
		yuv2_en(0),
		dupCqIdx(0),
		burstQueIdx(0),
		dipTh(ISP_DRV_DIP_CQ_THRE0),
		hwModule(DIP_A), 
		moduleIdx(0),
		isApplyTuning(MFALSE),
		CrzRatio(0),
		pTuningIspReg(NULL)
    {};
    ~DIP_YUV_PIPE(){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_YUV;  }
    virtual const char*   name_Str( void )              {   return "DIP_YUV";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return DIP_ISP_YUV_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_ISP_YUV_REG_RANGE;  }
    //

protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void );
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
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    MBOOL lastframe;  //last frame in enque package or not
    unsigned int lastframe_dupCqIdx;
    unsigned int lastframe_burstQueIdx; // for burst queue number
    unsigned int lastframe_dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
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
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
    MVOID       *WDMAPQParam;
    MVOID       *WROTPQParam;

public:
    DIP_MDP_PIPE():
        mdp_imgxo_p2_en(0),
        yuv2_en(0),
        dupCqIdx(0),
        burstQueIdx(0),
        dipTh(ISP_DRV_DIP_CQ_THRE0), 
        lastframe(MFALSE),
        lastframe_dupCqIdx(0),
        lastframe_burstQueIdx(0),
        lastframe_dipTh(ISP_DRV_DIP_CQ_THRE0),
        hwModule(DIP_A),
        moduleIdx(0),
        dip_dma_en(0),
        drvScenario(-1),
        isWaitBuf(MTRUE),
        dipCtlDmaEn(0),
        p2MdpSrcFmt(eImgFmt_YV12),
        p2MdpSrcW(0),
        p2MdpSrcH(0),
        p2MdpSrcYStride(0),
        p2MdpSrcUVStride(0),
        p2MdpSrcSize(0),
        p2MdpSrcCSize(0),
        p2MdpSrcVSize(0),
        p2MdpSrcPlaneNum(0),
        pTuningIspReg(NULL),
        WDMAPQParam(NULL),
        WROTPQParam(NULL)
        {};

    ~DIP_MDP_PIPE(){};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_MDP;  }
    virtual const char*   name_Str( void )              {   return "DIP_MDP";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return DIP_ISP_MDP_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_ISP_MDP_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void ){return MTRUE;}
public:
    MBOOL startVencLink( MINT32 fps, MINT32 wd,MINT32 ht);
    MBOOL stopVencLink();
};



/*/////////////////////////////////////////////////////////////////////////////
    ISP_CDP
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_CRZ_PIPE:public IspFunctionDip_B
{
public:
    int cdrz_filter;
    int crz_walgo;
    int crz_halgo;
    IspSize crz_in;
    IspSize crz_out;
    IspRect crz_crop;
    MdpRotDMACfg vido_out;
    MdpRotDMACfg dispo_out;
    int tpipeMode;
    int disp_vid_sel;
    int conf_cdrz;
    int conf_rotDMA;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    MBOOL isApplyTuning;
	MFLOAT CrzRatio;

public:
    DIP_CRZ_PIPE():
        cdrz_filter(0),
        crz_walgo(0),
        crz_halgo(0),
        tpipeMode(CRZ_DRV_MODE_TPIPE),
        disp_vid_sel(0),
        conf_cdrz(0),
        conf_rotDMA(0),
        dupCqIdx(0),
        burstQueIdx(0),
        dipTh(ISP_DRV_DIP_CQ_THRE0),
        hwModule(DIP_A),
        moduleIdx(0),
        isApplyTuning(MFALSE),
        CrzRatio(0)
        {};
public:
    virtual int checkBusy( unsigned long* param ){return 0;}

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

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_CRZ;  }
    virtual const char*   name_Str( void )              {   return "DIP_CRZ";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return DIP_ISP_CRZ_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_ISP_CRZ_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){return 0;}
    virtual int _write2CQ( void );
    virtual int _setZoom( void );
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_TDRI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_TDRI_PIPE:public IspFunctionDip_B
{
public:
    TdriDrvCfg tdri;
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int dipTh;
    unsigned int hwModule;
    unsigned int moduleIdx;
    int enTdri;
//    int tcm_en;
//    int tdr_en;
    int tdr_ctl;
    int tdr_tpipe;
    int tdr_tcm_en;
    int tdr_tcm2_en;
//    int drvScenario;
    MUINT32 lceInputW;
    MUINT32 lceInputH;
    MBOOL isApplyTuning;

    MUINTPTR pRegiAddr;
    unsigned int regCount;
    unsigned int* pReadAddrList;
public:
    DIP_TDRI_PIPE():
		dupCqIdx(0),
		burstQueIdx(0),
		dipTh(ISP_DRV_DIP_CQ_THRE0),
		hwModule(DIP_A),
		moduleIdx(0),
		enTdri(0),
		isApplyTuning(MFALSE),
		tdr_ctl(0),
		tdr_tpipe(0),
		tdr_tcm_en(0),
		tdr_tcm2_en(0),
		lceInputW(0),
		lceInputH(0),
		pReadAddrList(NULL)
		{};

public:
    virtual int checkBusy( unsigned long* param ){return 0;}
//    virtual MBOOL runTpipeDbgLog( void );
    virtual MBOOL configTdriSetting( TdriDrvCfg* pSrcImageio);


public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_TDRI;  }
    virtual const char*   name_Str( void )              {   return "DIP_TDRI";}
    virtual unsigned long reg_Base_Addr_Phy( void )      {   return DIP_DMA_TDRI_BASE;  }
    virtual unsigned long reg_Range( void )             {   return DIP_DMA_TDRI_REG_RANGE;  }
protected:
    virtual int _config( void );
    virtual int _enable( void* pParam ){return 0;}
    virtual int _disable( void* pParam ){ return 0;}
    virtual int _write2CQ( void );
    virtual int _setZoom( void ){ return 0;}
};



/*/////////////////////////////////////////////////////////////////////////////
    DIP_DMA_IMGI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMGI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_IMGI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMGI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMGI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMGI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMGI_REG_RANGE;     }
protected:

};

/*/////////////////////////////////////////////////////////////////////////////
    DIP_DMA_IMGBI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMGBI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_IMGBI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMGBI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMGBI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMGBI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMGBI_REG_RANGE;     }
protected:

};


/*/////////////////////////////////////////////////////////////////////////////
    DIP_DMA_IMGCI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMGCI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_IMGCI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMGCI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMGCI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMGCI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMGCI_REG_RANGE;     }
protected:

};

/*/////////////////////////////////////////////////////////////////////////////
    DIP_DMA_DMGI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_DMGI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_DMGI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_DMGI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_DMGI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_DMGI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_DMGI_REG_RANGE;     }
protected:

};

/*/////////////////////////////////////////////////////////////////////////////
    DIP_DMA_DEPI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_DEPI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_DEPI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_DEPI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_DEPI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_DEPI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_DEPI_REG_RANGE;     }
protected:

};



/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_UFDI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_UFDI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_UFDI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_UFDI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_UFDI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_UFDI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_UFDI_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_LCEI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_LCEI:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_LCEI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_LCEI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_LCEI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_LCEI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_LCEI_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_VIPI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_VIPI:public DIP_DMAI_B
{
public:
public:
    DIP_DMA_VIPI(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_VIPI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_VIPI";}
    virtual unsigned long reg_Base_Addr_Phy( void ) {    return     DIP_DMA_VIPI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_VIPI_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_VIP2I
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_VIP2I:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_VIP2I(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_VIP2I;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_VIP2I";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_VIP2I_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_VIP2I_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_VIP3I
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_VIP3I:public DIP_DMAI_B
{
public:

public:
    DIP_DMA_VIP3I(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_VIP3I;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_VIP3I";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return    DIP_DMA_VIP3I_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_VIP3I_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_TDRI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_TDRI:public DIP_DMAI_B
{
public:
public:
    DIP_DMA_TDRI()
        {};
public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_TDRI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_TDRI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_TDRI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_TDRI_REG_RANGE;     }
protected:
    //virtual int _config( void );
    //virtual int _write2CQ( int cq );
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_CQI
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_CQ:public DIP_DMAI_B
{
public:
public:
    DIP_DMA_CQ(){};
public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_CQI;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_CQI";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_CQI_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_CQI_REG_RANGE;     }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG2O
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMG2O:public DIP_DMAO_B // for pass2 img2o
{
public:

public:
    DIP_DMA_IMG2O(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMG2O;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMG2O";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMG2O_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMG2O_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG2BO
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMG2BO:public DIP_DMAO_B
{
public:

public:
    DIP_DMA_IMG2BO(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMG2BO;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMG2BO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMG2BO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMG2BO_REG_RANGE;     }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG3O
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMG3O:public DIP_DMAO_B
{
public:

public:
    DIP_DMA_IMG3O(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMG3O;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMG3O";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMG3O_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMG3O_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG3BO
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMG3BO:public DIP_DMAO_B
{
public:

public:
    DIP_DMA_IMG3BO(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMG3BO;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMG3BO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMG3BO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMG3BO_REG_RANGE;     }
protected:
};
/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_IMG3CO
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_IMG3CO:public DIP_DMAO_B
{
public:

public:
    DIP_DMA_IMG3CO(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_IMG3CO;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_IMG3CO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_IMG3CO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_IMG3CO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_FEO
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_FEO:public DIP_DMAO_B
{
public:

public:
    DIP_DMA_FEO(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_FEO;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_FEO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_FEO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_FEO_REG_RANGE;     }
protected:
};

/*/////////////////////////////////////////////////////////////////////////////
    ISP_DMA_MFBO
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA_MFBO:public DIP_DMAO_B
{
public:

public:
    DIP_DMA_MFBO(){};

public:/*[IspFunctionDip_B]*/
    virtual unsigned long id( void )                {    return     ENUM_DIP_DMA_MFBO;  }
    virtual const char*   name_Str( void )          {    return     "DIP_DMA_MFBO";}
    virtual unsigned long reg_Base_Addr_Phy( void )  {    return     DIP_DMA_MFBO_BASE;     }
    virtual unsigned long reg_Range( void )         {    return     DIP_DMA_MFBO_REG_RANGE;     }
protected:
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
        MUINT32                 _img_fmt = 0,
        MUINT32                 _img_hbin1 = 0,
        MUINT32                 _img_ob = 0,
        MUINT32                 _img_lsc = 0,
        MUINT32                 _img_rpg = 0,
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
    int drvScenario;
    int dipTh;  // for P2 deque use
    unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int moduleIdx;
public:
    DIP_BUF_CTRL():drvScenario(-1),dipTh(ISP_DRV_DIP_CQ_THRE0),dupCqIdx(0),burstQueIdx(0),moduleIdx(0)
        {};
    virtual unsigned long id( void )                    {   return ENUM_DIP_BUFFER;  }
    virtual const char*   name_Str( void )              {   return "DIP_BUFFER";}
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return (unsigned long)NULL;  }
    virtual unsigned long reg_Range( void )             {   return (unsigned long)NULL;  }
    virtual int checkBusy( unsigned long* param ) {return 0;}
    //virtual int init( MUINT32 dmaChannel );
    //virtual MBOOL waitBufReady( MUINT32 dmaChannel );
    /*
        * enqueueBuf
        *       append new buffer to the end of hwBuf list
        */
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo,MINT32 enqueCq =0,MINT32 dupCqIdx =0,MBOOL bImdMode = 0 );
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data,MBOOL bImdMode);
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
    /*
        * freeSinglePhyBuf
        *       free single physical buffer
        */
    virtual MUINT32 freeSinglePhyBuf(
        stISP_BUF_INFO bufInfo);
    /*
        * freeAllPhyBuf
        *       free all physical buffer
        */
    virtual MUINT32 freeAllPhyBuf( void );

private:
    int getDmaBufIdx( MUINT32 dmaChannel );
     //MINT32 returnDeqBufInfo( MINT32 rt_dma, stISP_BUF_INFO* bufInfo);

protected:
    virtual int     _config( void ) {return 0;}
    virtual int     _enable( void* pParam ) {return 0;}
    virtual int     _disable( void* pParam ) {return 0;}
    virtual int     _write2CQ( void ) {return 0;}
    virtual int     _setZoom( void ) {return 0;}

};

////////////////////////////////////////////////////////////////////////////////


#endif /*__ISP_FUNCTION_DIP_H__*/




