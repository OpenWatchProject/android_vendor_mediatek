#ifndef _CAM_PATH_CAM_H_
#define _CAM_PATH_CAM_H_
#include "cam_path.h"
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.h"
#include "uni_mgr.h"

#include <Cam_Notify_datatype.h>



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
        _UNKNOWN_IN = 2,
    }E_PATH;
    E_PATH          m_Path;

    MUINT32         m_fps;
    MUINT32         m_subSample;    //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    MBOOL           m_bOver4lane;
    MUINT32         m_tTimeClk;     // 1->1mhz,  2->2mhz

    IspRect         m_src_size;
    CAM_TG_CTRL::E_PIX  m_src_pixmode;

    ST_CAM_TOP_CTRL m_top_ctl;

    IspSize         m_Scaler;

    typedef enum{
        _PURE_RAW       = 0,
        _NON_LSC_RAW    = 1,
        _PROC_RAW       = 2,
        _UNKNOWN_RAW    = 3,
    }E_NON_SCALE_RAW;
    E_NON_SCALE_RAW m_NonScale_RawType;

    IspRect         rrz_in_roi;

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
        bypass_tg   = _bypass_tg;
        bypass_imgo = _bypass_imgo;
        bypass_rrzo = _bypass_rrzo;
        bypass_ufeo = _bypass_ufeo;
        bypass_ispRawPipe = _bypass_RawPipe;
        bypass_ispYuvPipe = _bypass_YuvPipe;
        m_subSample = 0;
        m_pUniMgr = NULL;
        m_bOver4lane = MFALSE;
        m_tTimeClk = 1;
        m_Path = _UNKNOWN_IN;
        m_fps = 0;
        m_src_pixmode = CAM_TG_CTRL::_unknown_pix_;
        m_NonScale_RawType = _UNKNOWN_RAW;
        m_RawIFmt = RAWI_FMT_UNKNOWN;
    }
};

/**
*@brief  ISP pass1 path class
*/
class CamPathPass1:public CamPath_B
{
public:

    /**
      *@brief  Constructor
      */

    CamPathPass1() :
        m_pCmdQdrv(NULL),
        m_isp_function_count(0),
        m_p1NotifyTbl{  NSImageio::NSIspio::EPIPECmd_SET_SGG2_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_SET_EIS_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_SET_LCS_CBFP,\
                        NSImageio::NSIspio::EPIPECmd_MAX,\
                        NSImageio::NSIspio::EPIPECmd_MAX}
        {
            for(int j=0;j<CAM_FUNCTION_MAX_NUM;j++)
                m_isp_function_list[j] = NULL;
            for(int i=0;i<p1Notify_node;i++)
                m_p1NotifyObj[i] = NULL;
            m_TuningFlg = MFALSE;

            for(MUINT32 i = 0 ; i < CAM_FUNCTION_MAX_NUM ; i++)
                m_isp_function_list[i] = NULL;
        }
    virtual ~CamPathPass1(){}

protected:
    virtual IspFunction_B** isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
    virtual const char*     name_Str(){     return  "CamPathPass1";  }
    virtual MINT32          _stop( void* pParam );

public:
            MINT32  config( struct CamPathPass1Parameter* p_parameter );

            MINT32  setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L);
            MINT32  setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L);
			MINT32  setP1UfeoCfg(list<DMACfg>* pUfeo_cfg_L);
            MINT32  setP1TuneCfg(list<IspP1TuningCfg>* pTune_L);
            MINT32  setP1Notify(void);


            MINT32  enqueueBuf( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, MVOID* private_data = NULL,MBOOL bImdMode=0 );
            MINT32  dequeueBuf( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,MUINT32 const u4TimeoutMs=0, CAM_STATE_NOTIFY *pNotify = NULL);

            MBOOL   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj);

            MBOOL   suspend(void);
            MBOOL   resume(void);
            MBOOL   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset);
public:
    DupCmdQMgr*     m_pCmdQdrv;


private:
    int             m_isp_function_count;
    IspFunction_B*  m_isp_function_list[CAM_FUNCTION_MAX_NUM];

    typedef enum{
        _SGG2_  = 0,
        _EIS_   = 1,
        _LCS_   = 2,
        p1Notify_node = 5
    }E_CB_MAP;
    P1_TUNING_NOTIFY*   m_p1NotifyObj[p1Notify_node];
    const MUINT32       m_p1NotifyTbl[p1Notify_node];

    UniMgr          m_UniMgr;

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
