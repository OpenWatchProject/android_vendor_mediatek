#ifndef __TWIN_MGR_H__
#define __TWIN_MGR_H__

#include <vector>
#include <cutils/atomic.h>

#include "isp_function_cam.h"
#include "ispio_stddef.h"
#include <Cam_Notify_datatype.h>
#include "cam_dupCmdQMgr.h"
#include "cam_path_cam.h"

using namespace std;
using namespace NSImageio; // for ICamIoPipe
using namespace NSIspio;    // for ICamIoPipe


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
typedef struct {
    MUINT32             tg_crop_w;
    MUINT32             tg_crop_h;

    MBOOL               bypass_rrzo;
    MBOOL               bypass_imgo;

    MBOOL               bDbn;
    MBOOL               bBin;
    MBOOL               isTwin;

    vector<ISP_HW_MODULE> slave_mod;

    P1_TUNING_NOTIFY*   pTwinCB;
} TWIN_MGR_PATH_CFG_IN_PARAM;

typedef struct {
    // pixel mode of mux
    T_XMXO  xmxo;

    // frontal binning
    MBOOL   dbn_en;
    MBOOL   bin_en;

    // twin
    MBOOL   isTwin;

    MUINT32 clk_level;      //clk level needed after twin's thru-put calculaton
} TWIN_MGR_PATH_CFG_OUT_PARAM;


typedef struct {
    ISP_HW_MODULE hwModule;
    vector<ISP_HW_MODULE> twinHwModules;
} TWIN_HWMODULE_PAIR;

typedef enum {// new add in cannon_ep migrated from bianco
    TWIN_CMD_GET_CUR_TWINMODULE,    //return current frame's slave cam which is used by twinmgr. arg1 type:list<ISP_HW_MODULE>*
    TWIN_CMD_GET_CUR_SLAVE_CQMGR,   //return current slave cam's cqmgr. arg1 type: E_ISP_CAM_CQ. arg2 type : list<ISP_HW_MODULE>*. arg3 type : vector<DupCmdQMgr*>*
    TWIN_CMD_GET_SLAVE_HEADERSIZE,  //return slave cam's headersize of each dmao. arg1 type: _isp_dma_enum_.  arg2 type:MUINT32*
    TWIN_CMD_GET_TWIN_STATE,        //return current state of twinmgr : arg1 type:E_TWIN_STATUS*
    TWIN_CMD_GET_TWIN_CROPINFO,     //return all twinpath's crop infor. arg1 type:  vector<vector<ISP_HW_MODULE>>*. arg2 type: vector<vector<STImgCrop>>*  (1st vector for serveral kinds of twin's path, 2nd vector for slave cam's info)
    TWIN_CMD_GET_TWIN_REG,          //reutrn the master cam's twinstatus register of all twinpaths',arg1 for CAM_A,arg2 for CAM_B, arg3 for CAM_C. arg1/2/3 type:MUINT32*
} E_TWIN_CMD;

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 note:
    1. support separate DMAO at imgo/rrzo.
        a. statistic is not supported , because of enque must be worked with runtiwindrv()
    2. support dynamic Twin ctrl
 **************************************************************************/
class TwinMgr : public CamPathPass1 {

public:
    typedef enum {
        E_NOTWIN,
        E_INIT,     //only init,not start yet.
        E_START,
        E_SUSPEND,
    } E_TWIN_STATUS;

    typedef enum {
        E_SUS_START,
        E_SUS_READY,
    } E_SUSPEND_OP;

                                    TwinMgr() {};
    virtual                         ~TwinMgr() {};

    static TwinMgr*                 createInstance(ISP_HW_MODULE hwModule); // Get static instance of TwinMgr by hwModule: CAM_A/B/...
    virtual void                    destroyInstance() = 0;

    static char*                    ModuleName(ISP_HW_MODULE hwModule);

    // from cam_path_pass1 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MINT32                  init(DupCmdQMgr* pMainCmdQMgr) = 0;
    virtual MINT32                  uninit(void) = 0;
    virtual MINT32                  start(void* pParam) = 0;
    virtual MINT32                  stop(void* pParam) = 0;

    virtual MINT32                  config(struct CamPathPass1Parameter* p_parameter) = 0;
    virtual MINT32                  setP1Update(void) = 0;
    virtual MINT32                  setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L) = 0;
    virtual MINT32                  setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L) = 0;
    virtual MINT32                  setP1TuneCfg(list<IspP1TuningCfg>* pTune_L) = 0;
    virtual MINT32                  setP1Notify(void) = 0;
    virtual MINT32                  enqueueBuf(MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo) = 0;
    virtual MINT32                  dequeueBuf(MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo, CAM_STATE_NOTIFY *pNotify = NULL) = 0;
    virtual MBOOL                   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj) = 0;
    virtual MBOOL                   HW_recover(E_CAMPATH_STEP op) = 0;
    virtual MBOOL                   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset) = 0;
    virtual MBOOL                   updatePath(UpdatePathParam pathParm) = 0;
    // from cam_path_pass1 ---------------------------------------------------------

    // Must configPath() before call TwinMgr init()
    // Config path according to input params, includes:
    // (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
    // per-frame ctrl: support dynamic twin, can be ionvoked each frame.
    virtual MBOOL                   pathControl(TWIN_MGR_PATH_CFG_IN_PARAM &cfgInParam) = 0;

    /// Twin 1-time related control
    virtual MBOOL                   initTwin(DupCmdQMgr* pMainCmdQMgr,const NSImageio::NSIspio::PortID* pInput,MUINT32 subSample,list<MUINT32>* pChannle) = 0;
    virtual MBOOL                   uninitTwin() = 0;

    virtual MBOOL                   startTwin(MBOOL bStreaming = MTRUE) = 0; // StartTwin: 1. init TwinDrv, 2.RunTwinDrv(), 3. twin cmdQ start
    virtual MBOOL                   stopTwin() = 0; // Uninit TwinDrv

    virtual MBOOL                   suspend(void) = 0;
    virtual MBOOL                   resume(void) = 0;

    virtual MBOOL                   recoverTwin(E_CAMPATH_STEP step) = 0; // new add in cannon_ep migrated from bianco   //stop & reset master & salve cam / restart master & slave cam
    //per-frame control
    virtual MBOOL                   suspendTwin(E_SUSPEND_OP op) = 0;
    virtual MBOOL                   resumeTwin(void) = 0;
    virtual MBOOL                   updateTwin() = 0; // Pass CQ baseaddr and related info to twin_drv and re-compute CQ data => EPIPECmd_SET_P1_UPDATE
    virtual MBOOL                   getIsTwin(MUINT32 dmaChannel,MBOOL bDeque = MFALSE) = 0; // Get m_isTwin is TRUE or FALSE,bDeque can only be true if function is involked at deque()
    virtual MBOOL                   popTwinRst(MUINT32 dmaChannel) = 0; // for dynamic twin, need to pop result after deque . this result is pushed perframe
    virtual MBOOL                   getIsTwin(MBOOL bDeque = MFALSE) = 0; // this is for camio only, bDeque can only be true if function is involked at deque()
    virtual MBOOL                   getIsInitTwin() = 0;  // new add in cannon_ep migrated from bianco
    virtual MBOOL                   getIsStartTwin() = 0; // new add in cannon_ep migrated from bianco
    virtual MBOOL                   runTwinDrv() = 0; // Run twin drv
        //twin's dmao ctrl
    virtual MINT32                  enqueTwin( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo) = 0;//enque buffer at twin mode
        /* return value:
             1: already stopped
             0: sucessed
            -1: fail */
    virtual MINT32                  dequeTwin( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify) = 0;//deque buffer at twin mode

    //CQ ctrl under twin mode.
    virtual MUINT32                 CQ_cfg(MINTPTR arg1, MINTPTR arg2) = 0;
    virtual MUINT32                 CQ_Trig(void) = 0;

    //get twinmgr's information
    virtual list<ISP_HW_MODULE>     getCurModule(void) = 0;//this is an dynamic result after configPath()
    virtual DupCmdQMgr*             getTwinCmdQ(ISP_HW_MODULE module,E_ISP_CAM_CQ cq) = 0;
    virtual MUINT32                 getTwinHeaderSize(MUINT32 dmaChannel) = 0;
    virtual E_TWIN_STATUS           getTwinStatuts(void) = 0;
    virtual MBOOL                   sendCommand(E_TWIN_CMD cmd,MINTPTR arg1,MINTPTR arg2,MINTPTR arg3) = 0; // new add in cannon_ep migrated from bianco
};

#endif

