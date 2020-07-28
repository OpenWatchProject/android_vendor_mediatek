#ifndef _CAM_PATH_CAM_H_
#define _CAM_PATH_CAM_H_

#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.h"
#include "uni_mgr.h"
#include "sec_mgr.h"

#include <Cam_Notify_datatype.h>
#include <mtkcam/drv/def/ICam_type.h>
#include <ispio_pipe_ports.h>


using android::Mutex;

typedef enum { // modify in cannon_ep migrated from bianco
    eCmd_path_stop,
    eCmd_path_restart,
} E_CAMPATH_STEP;

typedef enum{
    camstat_unknown  = 0,
    camstat_config,
    camstat_start,
    camstat_stop,
    camstat_suspend
} E_CAMPATH_STAT;


struct UpdatePathParam {
public:
    UpdatePathParam(T_XMXO *_pX, MBOOL _bin) :
        pXmx(_pX), bBin(_bin) {}

    T_XMXO      *pXmx;
    MBOOL       bBin;
};

/**
*@brief  ISP pass1 path  parameter
*/
struct CamPathPass1Parameter {
public:     //// fields.
    //scenario/sub_mode
    //start bit
    enum {
        CAM_START =0,
        CAM_CQ_START,
        CAM_START_NONE
    };

    typedef enum{
        _TG_IN      = 0,
        _DRAM_IN    = 1,
    }E_PATH;
    E_PATH          m_Path;

    MUINT32         m_fps;
    MUINT32         m_subSample;    //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    MBOOL           m_bOver4lane;
    MUINT32         m_tTimeClk;     // 10->1mhz,  20->2mhz

    IspRect         m_src_size;
    E_CamPixelMode  m_src_pixmode;

    E_CamPattern    m_data_pat;
    ST_CAM_TOP_CTRL m_top_ctl;

    IspSize         m_Scaler;

    typedef enum{
        _PURE_RAW       = 0,
        _NON_LSC_RAW    = 1,
        _PROC_RAW       = 2,
    }E_NON_SCALE_RAW;
    E_NON_SCALE_RAW m_NonScale_RawType;

    IspRect         rrz_in_roi;
    MUINT32         rrz_rlb_offset;

    struct{
        IspDMACfg   dmao;
        MUINT32     FH_PA;
    }m_imgo;
    struct{
        IspDMACfg   dmao;
        MUINT32     FH_PA;
    }m_rrzo;
    struct{
        IspDMACfg   dmao;
        MUINT32     FH_PA;
    }m_ufeo;//UFEO

    //for rawi
    E_UNI_TOP_FMT   m_RawIFmt;

    MINT32          bypass_tg;
    MINT32          bypass_ispRawPipe;
    MINT32          bypass_ispYuvPipe;
    MINT32          bypass_imgo;
    MINT32          bypass_rrzo;
    MINT32          bypass_ufeo;

    //
public:     //// constructors.
    CamPathPass1Parameter(
        int const _bypass_tg       = 0,
        int const _bypass_imgo     = 1,
        int const _bypass_rrzo     = 1,
        int const _bypass_ufeo     = 1,
        int const _bypass_RawPipe  = 0,
        int const _bypass_YuvPipe  = 1
    )
    {
        bypass_tg   = _bypass_tg;
        bypass_imgo = _bypass_imgo;
        bypass_rrzo = _bypass_rrzo;
        bypass_ufeo = _bypass_ufeo;
        bypass_ispRawPipe = _bypass_RawPipe;
        bypass_ispYuvPipe = _bypass_YuvPipe;
        m_subSample = 0;
        m_bOver4lane = MFALSE;
        m_tTimeClk = 1;

        m_Path = _TG_IN;
        m_fps = 1;
        m_src_pixmode = ePixMode_NONE;
        m_data_pat = eCAM_NORMAL;
        m_NonScale_RawType = _PURE_RAW;
        rrz_rlb_offset = 0;
        m_RawIFmt = RAWI_FMT_RAW10;
    }
};

// FBC+ DMA will have dependency due to UF mode.( FBC/DMA have no dependency under non-uf  mode)
class Path_BufCtrl
{
protected:
    virtual         ~Path_BufCtrl() {};
public:
    typedef struct{
        MUINT32 dmachannel;
        MBOOL   bOFF;
    }T_UF;
    virtual MBOOL   PBC_config(void* pPathMgr, vector<DupCmdQMgr*>* pSlave,list<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput) = 0;
    virtual MBOOL   PBC_Start(MVOID) = 0;
    virtual MBOOL   PBC_Stop(MVOID) = 0;

    virtual MBOOL   enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, vector<ISP_HW_MODULE>* pTwinVMod) = 0;
    virtual MINT32  enque_pop(MVOID) = 0;
    virtual MBOOL   enque_UF_patch(vector<T_UF>* pUF) = 0;

    virtual MINT32  deque( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify) = 0;


protected:
    typedef enum _E_FSM{
        op_unknown  = 0,
        op_cfg      = 1,
        op_startTwin,
        op_runtwin,
    }E_FSM;
            MBOOL   FSM_UPDATE(E_FSM op);

protected:
    mutable Mutex   mFSMLock;
    list<MUINT32>   m_OpenedChannel;
            MUINT32 m_FSM;


    vector<MBOOL>               m_bUF_imgo;     //for UF's constraint check result,vector for Burst number
    vector<MBOOL>               m_bUF_rrzo;     //for UF's constraint check result
};

class CamPathPass1;
class CamPath_BufCtrl : public Path_BufCtrl
{
public:
            CamPath_BufCtrl();
            ~CamPath_BufCtrl() {};

    virtual MBOOL   PBC_config(void* pPathMgr, vector<DupCmdQMgr*>* pSlave,list<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput);
    virtual MBOOL   PBC_Start(MVOID);
    virtual MBOOL   PBC_Stop(MVOID);

    virtual MBOOL   enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, vector<ISP_HW_MODULE>* pTwinVMod);
    virtual MINT32  enque_pop(MVOID);
    virtual MBOOL   enque_UF_patch(vector<T_UF>* pUF);

    virtual MINT32  deque( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify);

private:
    MINT32  enqueHW(MUINT32 const dmaChannel);

public:

private:
    //que for sw enque record
    //statistic is not supported,because statistic's enque control timing is not the same with main image
    QueueMgr<ISP_BUF_INFO_L>    m_enque_IMGO;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_RRZO;

    QueueMgr<stISP_BUF_INFO>    m_enqRecImgo;   //keep enque's buffer condition(plane number) for deque
    QueueMgr<stISP_BUF_INFO>    m_enqRecRrzo;   //keep enque's buffer condition(plane number) for deque

    CamPathPass1*               m_pCamPathImp;

    //MAIN hwmodule's dma&fbc
    BUF_CTRL_IMGO   m_Imgo_FBC;
    BUF_CTRL_RRZO   m_Rrzo_FBC;

    BUF_CTRL_UFEO   m_Ufeo_FBC;
    BUF_CTRL_UFGO   m_Ufgo_FBC;

    //mutex for subsample
    mutable Mutex   m_lock; //protect deque/enque behavior under subsample

    ISP_HW_MODULE   m_hwModule; //for dbg log
};

/**
*@brief  ISP pass1 path class
*/
class Cam_path_sel;
class CamPathPass1 {
public:
    friend class Cam_path_sel;
    friend class CamPath_BufCtrl;

    CamPathPass1();
    virtual ~CamPathPass1(){}

protected:
    virtual IspFunction_B** isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
    virtual const char*     name_Str() { return  "CamPathPass1"; }

public:
            MINT32  init(DupCmdQMgr* pMainCmdQMgr);
            MINT32  uninit(void);
            MINT32  config( struct CamPathPass1Parameter* p_parameter );
            MINT32  start(void* pParam);
            MINT32  stop(void* pParam);


            MINT32  setP1Update(void);
            MINT32  setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L);
            MINT32  setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L);
            MINT32  setP1TuneCfg(list<IspP1TuningCfg>* pTune_L);
            MINT32  setP1Notify(void);

            E_CAMPATH_STAT  getCamPathState(void);

            MINT32  enqueueBuf( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo);
            MINT32  dequeueBuf( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo, CAM_STATE_NOTIFY *pNotify = NULL);

            MBOOL   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj);

            typedef enum {
                _CAMPATH_SUSPEND_STOPHW = 0,
                _CAMPATH_SUSPEND_FLUSH = 1
            } E_CAMPATH_SUSPEND_OP;
            MBOOL   suspend(E_CAMPATH_SUSPEND_OP eOP);  // suspend hw by semaphore
            MBOOL   resume(void);                       // resume hw post semaphore
            MBOOL   recoverSingle(E_CAMPATH_STEP op);  // stop & reset hw, restart hw

            MBOOL   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset);

            //xmx will be runtime changed due to dynamic twin.(twin -> single -> twin)
            MBOOL   updatePath(UpdatePathParam pathParm);

private:
            MINT32  _config(void *);
            MBOOL   dynamicPakFmtMapping(NSImageio::NSIspio::ImgInfo::EImgFmt_t imgFmt,
                                         MUINT32* pHwImgoFmt);
protected:
    ISP_HW_MODULE   m_hwModule;
    DupCmdQMgr*     m_pCmdQdrv;
    CamPath_BufCtrl m_PBC;  //this is for UF mode

    typedef enum{
        op_unknown  = 0,
        op_cfg      = 1,
        op_start    = 2,
        op_stop     = 3,
        op_suspend
    } E_OP;

private:
    E_OP            m_FSM;

    int             m_isp_function_count;
    IspFunction_B*  m_isp_function_list[CAM_FUNCTION_MAX_NUM];

    typedef enum {
        _RRZ_ = 0, //NOTE: _RRZ must be the first callback
        _SGG2_,
        _LMV_,
        _LCS_,
        _RSS_,
        _TUNING_,
        _AWB_,
        _REG_DUMP_, //NOTE: _REG_DUMP must be the last callback to ensure dump lastest cq content
        //reserved one for m_p1NotifyTbl to keep last cmd "EPIPECmd_MAX"
        p1Notify_node = (_REG_DUMP_ + 2)
    } E_CB_MAP;
    P1_TUNING_NOTIFY*   m_p1NotifyObj[p1Notify_node];
    const MUINT32       m_p1NotifyTbl[p1Notify_node];

    CAM_TG_CTRL     m_TgCtrl;
    CAM_TOP_CTRL    m_TopCtrl;
    CAM_RAW_PIPE    m_RawCtrl;
    CAM_YUV_PIPE    m_YuvCtrl;

    CAM_RRZ_CTRL    m_Rrz;

    CAM_TUNING_CTRL m_Tuning;
    MBOOL           m_TuningFlg;
    CAM_MAGIC_CTRL  m_Magic;

    DMA_IMGO        m_Imgo;
    DMA_UFEO        m_Ufeo;
    DMA_RRZO        m_Rrzo;
    DMA_UFGO        m_Ufgo;

    DMA_LCSO        m_Lcso;
    BUF_CTRL_LCSO   m_Lcso_FBC;
    DMA_EISO        m_Eiso;
    BUF_CTRL_LMVO   m_Eiso_FBC;
    DMA_RSSO        m_Rsso;
    BUF_CTRL_RSSO   m_Rsso_FBC;

    //mutex for subsample
    mutable Mutex   m_lock;//protect deque/enque behavior under subsample
};


class Cam_path_sel {
public:
    Cam_path_sel(CamPathPass1Parameter* pObj);
    ~Cam_path_sel();

    //twin mode case is not take into consideration here yet
    //no UFE
    //no before LSC
    MBOOL Path_sel(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat);

private:
    MBOOL Path_sel_RAW(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat);
    MBOOL Path_sel_YUV(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat);

private:
    CamPathPass1Parameter* m_this;
    ISP_HW_MODULE   m_hwModule;//for log
};
#endif
