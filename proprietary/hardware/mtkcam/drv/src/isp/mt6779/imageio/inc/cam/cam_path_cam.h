#ifndef _CAM_PATH_CAM_H_
#define _CAM_PATH_CAM_H_

#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.h"

#include <Cam_Notify_datatype.h>
#include <mtkcam/drv/def/ICam_type.h>
#include <ispio_pipe_ports.h>

#include "sec_mgr.h"

using android::Mutex;

typedef enum {
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
    typedef enum {
        TG_INTERNAL = 0,
        TG_EXTERNAL
    } E_EVENT;

    UpdatePathParam(T_DLP *_pdlp, MBOOL _bin, MBOOL _bSetNotify) :
        pDlp(_pdlp), bBin(_bin), bDoneCollect(MFALSE), bForceConfigCB(MFALSE), bSetNotify(_bSetNotify), TgEvent(TG_INTERNAL){
            TgIdx = TG_A;}
    UpdatePathParam(T_DLP *_pdlp, MBOOL _bin, MBOOL _bSetNotify,
        MBOOL _bDone, MBOOL bForce, E_EVENT _tg_event, E_INPUT _tgidx, MSize _SrcSz) :
            pDlp(_pdlp), bBin(_bin),
            bDoneCollect(_bDone), bForceConfigCB(bForce), bSetNotify(_bSetNotify), TgEvent(_tg_event), TgIdx(_tgidx), SrcSize(_SrcSz){}

    T_DLP      *pDlp;
    MBOOL       bBin;
    MBOOL       bDoneCollect;
    MBOOL       bForceConfigCB;
    MBOOL       bSetNotify;

    //
    E_EVENT     TgEvent;
    E_INPUT     TgIdx;
    MSize       SrcSize;
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
        _TG_IN      = (1L<<0),
        _DRAM_IN    = (1L<<1)
    }E_PATH;
    MUINT32         m_Path;//E_PATH. tg+rawi = (_TG_IN|_DRAM_IN)

    MUINT32         m_fps;
    MUINT32         m_subSample;    //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    MBOOL           m_bOver4lane;
    MUINT32         m_tTimeClk;     // 10->1mhz,  20->2mhz
    MBOOL           m_bSvTwin;

    IspRect         m_src_size;
    E_CamPixelMode  m_src_pixmode;

    E_CamPattern    m_data_pat;
    ST_CAM_TOP_CTRL m_top_ctl;

    IspSize         m_Scaler;//rrz out

    typedef enum{
        _PURE_RAW       = 0,
        _NON_LSC_RAW    = 1,
        _PROC_RAW       = 2,
        _BTW_LSC_WB_RAW = 3,
    }E_NON_SCALE_RAW;
    E_NON_SCALE_RAW m_NonScale_RawType;

    IspRect         rrz_in_roi;
    MUINT32         rrz_rlb_offset;

    IspSize         m_ScalerYuvR1;
    IspRect         crzR1_in_roi;

    IspSize         m_ScalerYuvR2;
    IspRect         crzR2_in_roi;

    struct{
        IspDMACfg   dmao;
        MUINT32     FH_PA;
    }m_rawi;//for upper level to represent DRAM IN, might be rawi or ufdi
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
        MUINT32     pak_odr;
    }m_yuvo;
    struct{
        IspDMACfg   dmao;
        MUINT32     FH_PA;
        MUINT32     pak_odr;
    }m_crzor1;
    struct{
        IspDMACfg   dmao;
        MUINT32     FH_PA;
        MUINT32     pak_odr;
    }m_crzor2;

    //MINT32          bypass_tg; replaced by m_path due to input path is more the 2 kinds
    MINT32          bypass_ispDcifPipe;
    MINT32          bypass_ispRawPipe;
    MINT32          bypass_ispYuvPipe;
    MINT32          bypass_imgo;
    MINT32          bypass_rrzo;
    MINT32          bypass_yuvo;
    MINT32          bypass_crzor1;
    MINT32          bypass_crzor2;

    //
public:     //// constructors.
    CamPathPass1Parameter(
        int const _bypass_Dcif     = 1,
        int const _bypass_imgo     = 1,
        int const _bypass_rrzo     = 1,
        int const _bypass_yuvo     = 1,
        int const _bypass_crzor1   = 1,
        int const _bypass_crzor2   = 1,
        int const _bypass_RawPipe  = 0,
        int const _bypass_YuvPipe  = 1
    )
    {
        bypass_ispDcifPipe = _bypass_Dcif;
        bypass_imgo = _bypass_imgo;
        bypass_rrzo = _bypass_rrzo;
        bypass_yuvo = _bypass_yuvo;
        bypass_crzor1 = _bypass_crzor1;
        bypass_crzor2 = _bypass_crzor2;
        bypass_ispRawPipe = _bypass_RawPipe;
        bypass_ispYuvPipe = _bypass_YuvPipe;
        m_subSample = 0;
        m_bOver4lane = MFALSE;
        m_tTimeClk = 1;
        m_bSvTwin = MFALSE;

        m_Path = _TG_IN;
        m_fps = 1;
        m_src_pixmode = ePixMode_NONE;
        m_data_pat = eCAM_NORMAL;
        m_NonScale_RawType = _PURE_RAW;
        rrz_rlb_offset = 0;

        memset((void*)&m_top_ctl, 0, sizeof(ST_CAM_TOP_CTRL));
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
    //que for sw enque record (burst)
    //statistic is not supported,because statistic's enque control timing is not the same with main image
    QueueMgr<ISP_BUF_INFO_L>    m_enque_IMGO;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_RRZO;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_YUVO;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_CRZOr1;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_CRZOr2;


    //keep enque's buffer condition(plane number) for deque
    QueueMgr<stISP_BUF_INFO>    m_enqRecImgo;
    QueueMgr<stISP_BUF_INFO>    m_enqRecRrzo;
    QueueMgr<stISP_BUF_INFO>    m_enqRecYuvo;
    QueueMgr<stISP_BUF_INFO>    m_enqRecCrzoR1;
    QueueMgr<stISP_BUF_INFO>    m_enqRecCrzoR2;

    CamPathPass1*               m_pCamPathImp;

    //MAIN hwmodule's dma&fbc
    BUF_CTRL_IMGO   m_Imgo_FBC;
    BUF_CTRL_RRZO   m_Rrzo_FBC;

    BUF_CTRL_UFEO   m_Ufeo_FBC;
    BUF_CTRL_UFGO   m_Ufgo_FBC;

    BUF_CTRL_YUVO   m_Yuvo_FBC;
    BUF_CTRL_YUVBO  m_Yuvbo_FBC;
    BUF_CTRL_YUVCO  m_Yuvco_FBC;

    BUF_CTRL_CRZO_R1    m_CrzoR1_FBC;
    BUF_CTRL_CRZBO_R1   m_CrzboR1_FBC;
    BUF_CTRL_CRZO_R2    m_CrzoR2_FBC;
    BUF_CTRL_CRZBO_R2   m_CrzboR2_FBC;

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
            MINT32  setP1YuvoCfg(list<DMACfg>* pYuvo_cfg_L);
            MINT32  setP1CrzoR1Cfg(list<IspCrzrCfg>* pCrzo_cfg_L);
            MINT32  setP1CrzoR2Cfg(list<IspCrzrCfg>* pCrzo_cfg_L);
            MINT32  setP1RawiCfg(list<DMACfg>* pRawi_cfg_L);
            MINT32  setP1TuneCfg(list<IspP1TuningCfg>* pTune_L);
            MINT32  setP1Notify(void);
            MINT32  setCQStartNotify(void);
            E_CAMPATH_STAT  getCamPathState(void);

            MINT32  enqueueBuf( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo);
            MINT32  dequeueBuf( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo, CAM_STATE_NOTIFY *pNotify = NULL);

            MBOOL   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj,MBOOL bInitFBC = MTRUE);
            MBOOL   P1Notify_Global_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj);

            typedef enum {
                _CAMPATH_SUSPEND_STOPHW = 0,
                _CAMPATH_SUSPEND_FLUSH = 1
            } E_CAMPATH_SUSPEND_OP;
            MBOOL   suspend(E_CAMPATH_SUSPEND_OP eOP);  // suspend hw by semaphore
            MBOOL   resume(void);                       // resume hw post semaphore
            MBOOL   recoverSingle(E_CAMPATH_STEP op);  // stop & reset hw, restart hw

            MBOOL   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset);

            //dlp will be runtime changed due to dynamic twin.(twin -> single -> twin)
            MBOOL   updatePath(UpdatePathParam pathParm);
            MBOOL   DoneCollector(MBOOL bDoneCollect, E_INPUT TgIdx);

private:
            MINT32  _config(MUINT32 BurstIdx);
            MBOOL   dynamicPakFmtMapping(NSImageio::NSIspio::ImgInfo::EImgFmt_t imgFmt, CAM_RAW_PIPE::E_RawType mRawType,
                                         MUINT32* pHwImgoFmt);
            MBOOL   dynamicUNPFmtMapping(NSImageio::NSIspio::ImgInfo::EImgFmt_t imgFmt,
                                         MUINT32* pHwRawiFmt);
            MBOOL   dynamicDngFmtMapping(NSImageio::NSIspio::ImgInfo::EImgFmt_t imgFmt,
                                         MUINT32* pHwYuvoFmt,Pak_Func* pYuvPak);
            MBOOL   updateP1NotifyObj(ISP_HW_MODULE hwModule);

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
        _GSE_,
        _LMV_,
        _LCS_,
        _RSS_,
        _CRZR1_,
        _CRZR2_,
        _3A_,       //NOTE: _3A must be called before _TUNING
        _TUNING_,
        _AWB_,
        _REG_DUMP_, //NOTE: _REG_DUMP must be the last callback to ensure dump lastest cq content
        p1Notify_node,
        _CQSTART_ = p1Notify_node,
        //reserved one for m_p1NotifyTbl to keep last cmd "EPIPECmd_MAX"
        CB_node = (_CQSTART_ + 2)
    } E_CB_MAP;
    P1_TUNING_NOTIFY*   m_p1NotifyObj[CB_node];
    const MUINT32       m_p1NotifyTbl[CB_node];
    static P1_TUNING_NOTIFY* g_p1NotifyObj[PHY_CAM][CamPathPass1::CB_node];

    static CAM_TG_CTRL  g_TgCtrl[PHY_CAM];
    CAM_TG_CTRL*    m_TgCtrl;

    IspRect         m_srcImgSize;
    CAM_DCIF_CTRL   m_DcifCtrl;
    CAM_TOP_CTRL    m_TopCtrl;
    CAM_RAW_PIPE    m_RawCtrl;
    CAM_YUV_PIPE    m_YuvCtrl;

    CAM_RRZ_CTRL    m_Rrz;

    CAM_CRZ_CTRL*   m_CrzR1;
    CAM_CRZ_CTRL*   m_CrzR2;

    static MUINT32  g_SenDev[PHY_CAM];
    CAM_TUNING_CTRL m_Tuning;
    MBOOL           m_TuningFlg;

    CAM_MAGIC_CTRL  m_Magic;

    CAM_DPD_CTRL    m_Dpd_ctl;

    DMA_RAWI        m_Rawi;
    DMA_UFDI        m_Ufdi;
    BUF_CTRL_RAWI   m_Rawi_FBC;
    BUF_CTRL_UFDI   m_Ufdi_FBC;
    list<MBOOL>     m_vUFI;

    DMA_IMGO        m_Imgo;
    DMA_UFEO        m_Ufeo;//hide from user
    DMA_RRZO        m_Rrzo;
    DMA_UFGO        m_Ufgo;//hide from user

    DMA_YUVO        m_Yuvo;
    DMA_YUVBO       m_Yuvbo;//hide from user
    DMA_YUVCO       m_Yuvco;//hide from user

    DMA_CRZO_R1     m_CrzoR1;
    DMA_CRZBO_R1    m_CrzboR1;//hide from user
    DMA_CRZO_R2     m_CrzoR2;
    DMA_CRZBO_R2    m_CrzboR2;//hide from user

    DMA_LCSO        m_Lcso;
    BUF_CTRL_LCSO   m_Lcso_FBC;
    DMA_EISO        m_Eiso;
    BUF_CTRL_LMVO   m_Eiso_FBC;
    DMA_RSSO        m_Rsso;
    BUF_CTRL_RSSO   m_Rsso_FBC;

    static IspDMACfg g_ImgoDmaCfg[PHY_CAM];
    static IspDMACfg g_RrzoDmaCfg[PHY_CAM];
    static IspDMACfg g_YuvoDmaCfg[PHY_CAM];
    static IspDMACfg g_YuvboDmaCfg[PHY_CAM];
    static IspDMACfg g_YuvcoDmaCfg[PHY_CAM];
    static IspDMACfg g_CrzoR1DmaCfg[PHY_CAM];
    static IspDMACfg g_CrzboR1DmaCfg[PHY_CAM];
    static IspDMACfg g_CrzoR2DmaCfg[PHY_CAM];
    static IspDMACfg g_CrzboR2DmaCfg[PHY_CAM];

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
    MBOOL Path_sel(MUINT32 mPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat);

private:
    MBOOL Path_sel_RAW(MUINT32 mPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat);
    MBOOL Path_sel_YUV(MUINT32 mPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat);

private:
    CamPathPass1Parameter* m_this;
    ISP_HW_MODULE   m_hwModule;//for log
};
#endif
