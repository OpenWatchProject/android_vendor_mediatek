#ifndef __TWIN_MGR_H__
#define __TWIN_MGR_H__

#include <vector>
#include <cutils/atomic.h>

#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.h"
#include "twin_drv.h"



using namespace std;
using namespace NSImageio; // for ICamIoPipe
using namespace NSIspio;    // for ICamIoPipe




/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

enum ETwinMgr_PixMode
{
    eTwinMgr_1_Pix     = 0,
    eTwinMgr_2_Pix     = 1,
    eTwinMgr_4_Pix     = 2,
    eTwinMgr_unknown   = 3,
};



typedef struct
{
    // tg
    MBOOL bypass_tg;
    MUINT32 pix_mode_tg;
    MUINT32 tg_crop_w;

    // rrz
    MBOOL bypass_rrzo;
    MUINT32 max_rrz_out_w;

    MBOOL   offBin;
    MBOOL   bDbn;
}TWIN_MGR_PATH_CFG_IN_PARAM;

typedef struct
{
    // pixel mode of mux
    MUINT32 pix_mode_dmxi;
    MUINT32 pix_mode_dmxo;
    MUINT32 pix_mode_bmxo;
    MUINT32 pix_mode_rmxo;

    // frontal binning
    MBOOL dbn_en;
    MBOOL bin_en;

    // twin
    MBOOL isTwin;

}TWIN_MGR_PATH_CFG_OUT_PARAM;


typedef struct
{
    ISP_HW_MODULE hwModule;
    vector<ISP_HW_MODULE> twinHwModules;
}TWIN_HWMODULE_PAIR;


// Use to repsent how many hwModule used when enable twin
// In Everest, only eTwinMgr_TwinNum_2 is existed.
// But in Rocky, 4 hoMudle are existed so the possible combinations are up to eTwinMgr_TwinNum_4
enum ETwinMgr_TwinNum
{
    eTwinMgr_TwinNum_2,
    //eTwinMgr_TwinNum_3, // for Rocky
    //eTwinMgr_TwinNum_4, // for Rocky
    eTwinMgr_TwinNum_Max
};


#if 0
typedef struct
{
    MUINT32 SDBLK_XNUM;
    MUINT32 SDBLK_lWIDTH;
    MUINT32 RRZ_IN_CROP_HT;
    MUINT32 RRZ_HORI_INT_OFST;
    MUINT32 RRZ_HORI_INT_OFST_LAST;
    MUINT32 RRZ_HORI_SUB_OFST;
    MUINT32 RRZ_OUT_WD;
    MUINT32 AF_TAPS;
    MUINT32 AF_MARGIN;
    MUINT32 AF_OFFSET;
    MUINT32 AF_BLOCK_WD;
    MUINT32 RAWI_XSIZE;
}TWIN_DUAL_IN_CONFIG_SW_PARAM;
#endif

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class TwinMgr
{
public:
    TwinMgr();
    ~TwinMgr(){};

    static TwinMgr*         createInstance(ISP_HW_MODULE hwModule); // Get static instance of TwinMgr by hwModule: CAM_A/B/...
           void             destroyInstance();

    // Must configPath() before call TwinMgr init()
    // Config path according to input params, includes:
    // (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
    MBOOL                   configPath(TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam);


    ///
    /// Twin related control
    //
    MBOOL                   initTwin(DupCmdQMgr* pMainCmdQMgr, MUINT32 fps,MUINT32 subSample);
    MBOOL                   configTwin(void);
    MBOOL                   startTwin(void); // StartTwin: 1. init TwinDrv, 2.RunTwinDrv(), 3. twin cmdQ start
    MBOOL                   stopTwin(); // Uninit TwinDrv
    MBOOL                   updateTwin(); // Pass CQ baseaddr and related info to twin_drv and re-compute CQ data => EPIPECmd_SET_P1_UPDATE
    MBOOL                   uninitTwin();
    MBOOL                   getIsTwin(); // Get m_isTwin is TRUE or FALSE
    MBOOL                   runTwinDrv(); // Run twin drv

    //CQ ctrl under twin mode.
    MUINT32                 CQ_cfg(MINTPTR arg1, MINTPTR arg2);
    MUINT32                 CQ_Trig(void);

    list<ISP_HW_MODULE>*    getCurModule(void);

    //this lock is to avoid afo_en control rasing under afo disable & enqueuehwbuf
    void                    Twin_Lock(void);
    void                    Twin_UnLock(void);

private:
    MUINT32                 chkRRZOutSize(TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam);
    MBOOL                   cvtDupCmdQRegtoDualIspReg();
    MBOOL                   prepareDualInCfg();
    // 1. Update Twin's CQ descriptor according to Main's CQ descriptor
    // 2. Set CAMB's dmx_sel = 1
    MBOOL                   updateTwinCQandReg();

    typedef enum _E_FSM{
        op_unknown  = 0,
        op_cfg      = 1,
        op_startTwin     = 2,
        op_stopTwin   = 3,
    }E_FSM;

    // FSM check current op is corrected or not
    MBOOL                   FSM_CHECK(MUINT32 op);
    MBOOL                   FSM_UPDATE(MUINT32 op);

private:
    E_FSM m_FSM;

    ISP_HW_MODULE           m_hwModule;
    ISP_HW_MODULE           m_twinHwModule; // Twin hwmodules
    list<ISP_HW_MODULE>     m_occupiedModule;

    // tg
    MBOOL                   m_bypass_tg;
    MUINT32                 m_pix_mode_tg;
    MUINT32                 m_tg_crop_w;

    // rrz
    MBOOL                   m_bypass_rrzo;
    MUINT32                 m_max_rrz_out_w;
    MUINT32                 m_rrz_out_w;

    // pixel mode of mux
    MUINT32                 m_pix_mode_dmxi;
    MUINT32                 m_pix_mode_dmxo;
    MUINT32                 m_pix_mode_bmxo;
    MUINT32                 m_pix_mode_rmxo;

    // frontal binning
    MUINT32                   m_dbn_en;
    MUINT32                   m_bin_en;

    // twin
    MBOOL                           m_isTwin;
    TwinDrv*                        m_pTwinDrv;
    MUINT32                         m_hwModuleNum;
    DupCmdQMgr*                     m_pMainCmdQMgr; // CmdQMgr for main hwModule
    DupCmdQMgr*                     m_pTwinCmdQMgr[ISP_DRV_CAM_BASIC_CQ_NUM]; // CmdQMgr for twin hwModule
    ISP_DRV_CAM*                    m_pMainIspDrv;
    ISP_DRV_CAM*                    m_pTwinIspDrv;
    //TWIN_DUAL_INPUT_PARAM           m_dualInputParam;
    //TWIN_DUAL_IN_CONFIG_SW_PARAM    m_dualInConfigSwParam;

    CAM_TWIN_PIPE                   m_TwinCtrl;
    ST_CAM_TOP_CTRL                 m_TopCtrl;

    // counter
    volatile MINT32     mConfigCount;
    volatile MINT32     mInitTwinCount;

    // mutex
    mutable Mutex mTwinLock; // for protecting init, uninit twin related control
    mutable Mutex mUpdateTwinLock; // for protecting init, uninit twin related control
    mutable Mutex mConfigParamLock; // for protecting param config
    mutable Mutex mConfigTwinLock; // for protecting param config
    mutable Mutex mFSMLock;
};

#endif

