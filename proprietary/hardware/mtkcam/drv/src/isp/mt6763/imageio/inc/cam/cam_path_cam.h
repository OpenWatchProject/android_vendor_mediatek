#ifndef _CAM_PATH_CAM_H_
#define _CAM_PATH_CAM_H_
#include "cam_path.h"
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.h"
#include "uni_mgr.h"

#include <Cam_Notify_datatype.h>



struct CamPathPass1Parameter;

class Cam_path_sel
{
public:
    Cam_path_sel(CamPathPass1Parameter* pObj);
    ~Cam_path_sel();
public:
    typedef enum{
        _normal_     = 0,
        _dual_pix_   = 1,
        _quad_code_  = 2, //normal 4 cell
    }E_DatPat;


public:
    //twin mode case is not take into consideration here yet
    //no UFE
    //no before LSC
    MBOOL Path_sel(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI,E_DatPat data_pat);

private:
    MBOOL Path_sel_RAW(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI,E_DatPat data_pat);
    MBOOL Path_sel_YUV(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI,E_DatPat data_pat);

private:
    CamPathPass1Parameter* m_this;
};

/**
*@brief  ISP pass1 path  parameter
*/
struct CamPathPass1Parameter
{
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
    CAM_TG_CTRL::E_PIX  m_src_pixmode;

    ST_CAM_TOP_CTRL m_top_ctl;
    Cam_path_sel::E_DatPat  m_data_pat;

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
    UniMgr*         m_pUniMgr;

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
        m_Path = _TG_IN;
        m_fps = 0;
        m_subSample = 0;
        m_bOver4lane = MFALSE;
        m_src_pixmode = (CAM_TG_CTRL::E_PIX)0;
        m_tTimeClk = 1;
        m_data_pat = Cam_path_sel::_normal_;
        m_NonScale_RawType = _PURE_RAW;
        rrz_rlb_offset = 0;
        m_RawIFmt = RAWI_FMT_RAW10;
        m_pUniMgr = 0;
        bypass_tg   = _bypass_tg;
        bypass_ispRawPipe = _bypass_RawPipe;
        bypass_ispYuvPipe = _bypass_YuvPipe;
        bypass_imgo = _bypass_imgo;
        bypass_rrzo = _bypass_rrzo;
        bypass_ufeo = _bypass_ufeo;
        m_top_ctl.bVBN = MFALSE;
    }
};

/**
*@brief  ISP pass1 path class
*/
class CamPathPass1:public CamPath_B
{
public:
    friend class Cam_path_sel;

    /**
      *@brief  Constructor
      */

    CamPathPass1() :
        m_pCmdQdrv(NULL),
        m_isp_function_count(0),
        m_p1NotifyTbl{  NSImageio::NSIspio::EPIPECmd_SET_SGG2_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_SET_EIS_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_SET_LCS_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_SET_RSS_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_SET_REGDUMP_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_MAX}
        {
            for(int i=0;i<p1Notify_node;i++)
                m_p1NotifyObj[i] = NULL;
            for(int i=0;i<CAM_FUNCTION_MAX_NUM;i++)
                m_isp_function_list[i] = NULL;
            m_TuningFlg = MFALSE;
        }
    virtual ~CamPathPass1(){}

protected:
    virtual IspFunction_B** isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
    virtual const char*     name_Str(){     return  "CamPathPass1";  }
    virtual MINT32          _stop( void* pParam );

public:
            MINT32  init(void);
            MINT32  uninit(void);
            MINT32  config( struct CamPathPass1Parameter* p_parameter );

            MINT32  setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L);
            MINT32  setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L);
            MINT32  setP1UfeoCfg(list<DMACfg>* pUfeo_cfg_L);
            MINT32  setP1TuneCfg(list<IspP1TuningCfg>* pTune_L);
            MINT32  setP1Notify(void);


            MINT32  enqueueBuf( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, MVOID* private_data = NULL,MBOOL bImdMode=0 );
            MINT32  dequeueBuf( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,MUINT32 const u4TimeoutMs=0, CAM_STATE_NOTIFY *pNotify = NULL);

            MBOOL   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj);


            typedef enum{
                _CAMPATH_SUSPEND_STOPHW = 0,
                _CAMPATH_SUSPEND_FLUSH = 1
            } E_CAMPATH_SUSPEND_OP;
            MBOOL   suspend(E_CAMPATH_SUSPEND_OP eOP);                      // suspend hw by semaphore
            MBOOL   resume(void);                       // resume hw post semaphore
            MBOOL   HW_recover(E_CAMPATH_STEP op);      // stop & reset hw, restart hw

            MBOOL   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset);
            //xmx will be runtime changed due to dynamic twin.(twin -> single -> twin)
            MBOOL   updateXMXO(T_XMXO* pxmx);
public:
    DupCmdQMgr*     m_pCmdQdrv;
    UniMgr                 m_UniMgr;

private:
    int             m_isp_function_count;
    IspFunction_B*  m_isp_function_list[CAM_FUNCTION_MAX_NUM];

    typedef enum{
        _SGG2_  = 0,
        _EIS_   = 1,
        _LCS_   = 2,
        _RSS_   = 3,
        _REG_DUMP_    = 4,
        //reserved one for m_p1NotifyTbl to keep last cmd "EPIPECmd_MAX"
        p1Notify_node = (_REG_DUMP_ + 2)
    }E_CB_MAP;
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
    BUF_CTRL_IMGO   m_Imgo_FBC;
    DMA_RRZO        m_Rrzo;
    BUF_CTRL_RRZO   m_Rrzo_FBC;
    DMA_UFEO        m_Ufeo;
    BUF_CTRL_UFEO   m_Ufeo_FBC;

    //
    DMA_LCSO        m_Lcso;
    BUF_CTRL_LCSO   m_Lcso_FBC;

    //mutex for subsample
    mutable Mutex   m_lock;//protect deque/enque behavior under subsample
};


#endif
