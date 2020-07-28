#ifndef __ISP_FUNCTION_CAM_H__
#define __ISP_FUNCTION_CAM_H__

#include "isp_function.h"
#include "isp_function.timestamp.h"
#include "isp_drv_cam.h"
#include "uni_drv.h"
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include "ispio_stddef.h"

#include <Cam_Notify_datatype.h>

#include <Cam/buf_que_ctrl.h>
#include "cam_capibility.h"
#include <mtkcam/drv/def/ICam_type.h>
#include "cam_dupCmdQMgr.h"

//CQ sw wrok around for:
/**
a.after hw reset, cq under all modes,no matter what event trig/immediate or continuous, will be read at the very 1st p1_done after cq_en pulled high.
b.descriptor in cq can't be all nop cmd.
*/
#define CQ_SW_WORK_AROUND  1

/**
DMX_ID in CAM_A is 0,CAM_B is 1 at most of case , only 8lane-sensor is exclusive.
Because of HW reg initial value is 0, sw need to over write the pix ID of CAM_B even if CAM_B is not needed currently.
This operation can prevent Interrupt false alarm at CAM_B.
*/
#define DMX_ID_SW_WORK_AROUND 0

/**
    this solution is for CAM_B always have sw p1 done under twin mode.
    And this behavior will break the CQ link-list.
    Side effect of current work around solution:
        sw only have (n-1)/n * (time of p1_sof to sw_p1_don) to program whole enque.
*/
#define TWIN_SW_P1_DONE_WROK_AROUND 1

/**
    For main cam support dynamic-twin with slave cam suspend/resume flow, hw physical regs will be
    overwritten by main cam's twin part. When resume, all regs on CQ needs to be reload. Enable
    this flag to support all p1 internal setting load from cq for every frame.
    (Tuning setting in internal module not included in this flag, due to 3A is resposible for load
    full tuning setting along with enque request come with resume API)
*/
#define P1_STANDBY_DTWIN_SUPPORT    1

/**
    top dcm must be disabeld to avoid abnormally TG grab error
*/
#define TOP_DCM_SW_WORK_AROUND  1

/**
    SOF sig. of IMGO with IMG_SEL=TG will be 1T earlier than RRZO, which causes drop status check fail.
    Enque IMGO IMG_SEL=TG and RRZO when drop frame, then next frame IMGO_FBC drop_cnt and fbc_cnt will
    both increase, due to drop frame group is set for both IMGO & RRZO
*/
#define FBC_DRPGRP_SW_WORK_AROUND  0


/**
    need to close subsample done function under drop frame, otherwise, subsample hw counter will work abnormally.
*/
#define SUBSAMPLE_SW_WORK_AROUND    1

/**
    patch hw default value.  hw didn't assign a right default value
*/
#define UFE_HW_DEFAULT_WORK_AROUND  1
/**
    can't support arbitrary crop , plz reference to define:HW_TIMESTAMP_WORKAROUND.
    if output is cam_b only , there is no timestamp anymore.
*/
#if (HW_TIMESTAMP_WORKAROUND==1)
#define Arbitrary_Crop  (0)
#else
#define Arbitrary_Crop  (1)
#endif

/**
    [ISP 5.0 only]
    For the master cam that will be suspended,
    must disable it's BMX_EN & AMX_EN to avoid TG_overrun happend on another master cam.
*/
#define BMX_AMX_DTWIN_WROK_AROUND    (1)


/**
    Per-frame timeout
*/
#define MIN_GRPFRM_TIME_MS          (200)  //subsampled frame time minimum difference
#define MAX_RECENT_GRPFRM_TIME      (2)


/**
    Dynamic Bin callback control
*/
#define Dynamic_bin_Notify_Frame    (2)

/////////////////////////////////////////////////////////////////////////////
/*    ISP Function ID                                                      */
/////////////////////////////////////////////////////////////////////////////

/**
    individual sub-module existed in isp function.
*/
enum {
    CAM_TG  = 0,    //0
    CAM_TOP,
    CAM_RAW,
    CAM_RGB,
    CAM_YUV,
    CAM_DMA_CQ0I,   //5
    CAM_DMA_CQ1I,
    CAM_DMA_CQ2I,
    CAM_DMA_CQ3I,
    CAM_DMA_CQ4I,
    CAM_DMA_CQ5I,   //10
    CAM_DMA_CQ6I,
    CAM_DMA_CQ7I,
    CAM_DMA_CQ8I,
    CAM_DMA_CQ9I,
    CAM_DMA_CQ10I,  //15
    CAM_DMA_CQ11I,
    CAM_DMA_CQ12I,
    CAM_DMA_BPCI,
    CAM_DMA_LSCI,
    CAM_DMA_CACI,    //20
    CAM_DMA_PDI,
    CAM_DMA_IMGO,
    CAM_DMA_UFEO,
    CAM_DMA_RRZO,
    CAM_DMA_AFO,    //25
    CAM_DMA_AAO,
    CAM_DMA_PDO,
    CAM_DMA_LCSO,
    CAM_DMA_PSO,
    CAM_DMA_LMVO,
    CAM_DMA_FLKO,
    CAM_DMA_RSSO,
    CAM_DMA_UFGO,
    CAM_BUFFER_CTRL, //30
    CAM_TWIN,

    UNI_TOP,
    UNI_DMA_RAWI,
    CAM_FUNCTION_MAX_NUM,
};

typedef enum{
    IF_CAM_A,
    IF_CAM_B,
    IF_RAWI_CAM_A,
    IF_RAWI_CAM_B,
    IF_CAM_MAX,
}ENUM_IF_CAM;

/******************************************************************************
 * @enum E_CamIMGSel
 *
 * @IMG select path, please reference block diagram.
 *
 ******************************************************************************/
typedef enum {
    eIMGSel_0 = 0,
    eIMGSel_1,
    eIMGSel_2,
} E_CamIMGSel;

/******************************************************************************
 * @enum E_CamUFESel
 *
 * @UFE select path, please reference block diagram.
 *
 ******************************************************************************/
typedef enum {
    eUFESel_0 = 0,
    eUFESel_1,
    eUFESel_2,
} E_CamUFESel;


///////////////////////////////////////////////////////////////////////////////
/**
    Class for TG module
*/
class CAM_TG_CTRL : public IspFunction_B
{
public:
    MBOOL                                             m_continuous;   //signle or continuous mode
    MUINT32                                           m_SubSample;      //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    IspRect                                           m_Crop;
    E_CamPixelMode                                    m_PixMode;
    E_CamPattern                                      m_Datapat;

public:
    CAM_TG_CTRL();

    virtual MUINT32     id( void )                    {   return CAM_TG;  }
    virtual const char* name_Str( void )              {   return "CAM_TG";}
    virtual MBOOL       checkBusy( MUINTPTR param ){param;return MFALSE;}

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam = NULL );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( void ){return 0;}
private:
            ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
};

///////////////////////////////////////////////////////////////////////////////
/**
    Class for CAM Top module
*/
class CAM_TOP_CTRL : public IspFunction_B
{
friend class CAM_TWIN_PIPE;
public:
    CAM_TOP_CTRL();

    virtual MUINT32     id( void )                    {   return CAM_TOP;  }
    virtual const char* name_Str( void )              {   return "CAM_TOP";}
    virtual MBOOL       checkBusy( MUINTPTR param );

            MBOOL       suspend(void){return MFALSE;}
            MBOOL       resume(void){return MFALSE;}
            MBOOL       HW_recover(MUINT32 step); //set 0:stop pipeline. set 1:restart pipeline
protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( void );


public:
    ST_CAM_TOP_CTRL     cam_top_ctl;

    ENUM_IF_CAM         CAM_Path;       //only used for TG_IN or DMAI

    MUINT32             SubSample;      //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample

private:
            MBOOL           m_bBusy;
            ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
#if DMX_ID_SW_WORK_AROUND
            MBOOL           m_bCfg;
#endif
            //for recover fbc cnt
            CAM_REG_FBC_IMGO_CTL2     fbc_IMGO;
            CAM_REG_FBC_RRZO_CTL2     fbc_RRZO;
            CAM_REG_FBC_UFEO_CTL2     fbc_UFEO;
            CAM_REG_FBC_UFGO_CTL2     fbc_UFGO;
            CAM_REG_FBC_AFO_CTL2      fbc_AFO;
            CAM_REG_FBC_AAO_CTL2      fbc_AAO;
            CAM_REG_FBC_LCSO_CTL2     fbc_LCSO;
            CAM_REG_FBC_PDO_CTL2      fbc_PDO;
            CAM_REG_FBC_PSO_CTL2      fbc_PSO;
            CAM_REG_FBC_FLKO_CTL2     fbc_FLKO;
            CAM_REG_FBC_LMVO_CTL2     fbc_LMVO;
            CAM_REG_FBC_RSSO_CTL2     fbc_RSSO;

            CAM_REG_CTL_SW_PASS1_DONE fbcSwP1DoneCon;

};


/////////////////////////////////////////////////////////////////////////////////
/**
    class for UNI
*/
class UNI_TOP_CTRL : public IspFunction_B
{
public:
    ST_UNI_TOP_CTRL uni_top_ctl;    /* cannon_ep reg */

    ENUM_IF_CAM     UNI_Source;
    IspRect         m_Crop;         //this struct is used if and only if RAWI

    UniDrvImp*      m_pUniDrv;
    IspDrvImp*      m_pCamDrv;      //phy obj
public:
    UNI_TOP_CTRL();

    virtual MUINT32     id( void )                    {   return UNI_TOP;  }
    virtual const char* name_Str( void )              {   return "UNI_TOP";}

            //special case for uni streaming stop.
            //incase of cmdq being switch during uni_stop.
            MINT32      ClrUniFunc(void);


            MBOOL       suspend(void);
            MBOOL       resume(void);
protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam = NULL );

private:
            ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
};

/**
*/
typedef struct
{
    E_CamPixelMode pix_mode_dmxi;
    E_CamPixelMode pix_mode_dmxo;
    E_CamPixelMode pix_mode_bmxo;
    E_CamPixelMode pix_mode_rmxo;
    E_CamPixelMode pix_mode_amxo;   //useless, cureent pipeline always 1pix mode
}T_XMXO;

/**
    class for DMX contrl
*/
class CAM_RAW_PIPE;
class CAM_XMX_CTRL
{
public:
    CAM_XMX_CTRL(){m_hwModule = CAM_MAX;m_pRaw_p = NULL;}
    virtual ~CAM_XMX_CTRL(){}
    MINT32 config(CAM_RAW_PIPE* _this);

private:
    virtual MINT32 _config(void){return -1;}

protected:
    CAM_RAW_PIPE*   m_pRaw_p;
    ISP_HW_MODULE   m_hwModule;
};

class CAM_DMX_CTRL:public CAM_XMX_CTRL
{
private:
    virtual MINT32 _config( void );
};

/**
    class for BMX contrl
*/
class CAM_BMX_CTRL:public CAM_XMX_CTRL
{
public:
    virtual MINT32 _config( void );

};

/**
    class for AMX contrl
*/
class CAM_AMX_CTRL:public CAM_XMX_CTRL
{
private:
    virtual MINT32 _config( void );

};

/**
    class for RMX contrl
*/
class CAM_RMX_CTRL:public CAM_XMX_CTRL
{
private:
    virtual MINT32 _config( void );

};

/**
    class for RCP contrl
*/
class CAM_RCP_CTRL:public CAM_XMX_CTRL
{
private:
    virtual MINT32 _config( void );

};


/**
    class for RCP3 contrl
*/
class CAM_RCP3_CTRL:public CAM_XMX_CTRL
{
private:
    typedef enum _rcp3_sel{
        E_rcp3_0 = 0,
        E_rcp3_1,
        E_rcp3_2,
        E_rcp3_3,
    }E_RCP3_SEL;
    virtual MINT32 _config( void );

};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM when input data type is Bayer, change raw pipe setting under streaming
*/
class CAM_RAW_PIPE:public IspFunction_B
{
public:
    friend class CAM_XMX_CTRL;
    friend class CAM_DMX_CTRL;
    friend class CAM_BMX_CTRL;
    friend class CAM_RMX_CTRL;
    friend class CAM_AMX_CTRL;
    friend class CAM_RCP_CTRL;
    friend class CAM_RCP3_CTRL;

public:
    typedef enum{
        E_FromTG    = 0,
        E_BeforLSC  = 1,
        E_AFTERLSC  = 2,
    }E_RawType;
    E_RawType           m_RawType;

    ENUM_IF_CAM         m_Source;
    P1_TUNING_NOTIFY*   m_pP1Tuning;
    T_XMXO              m_xmxo;
    MBOOL               m_bBin;
    MBOOL               m_bRRz;                 //for 3-raw case, once if 3-raw + h-ratio zoomin,  rrz at main-cam will be disabled by twin drv.
                                                //use this member to keep scenario-level's rrz info
    E_CamPattern        m_DataPat;

    CAM_RAW_PIPE();
    virtual ~CAM_RAW_PIPE();

public:
    virtual MUINT32     id( void )                    {   return CAM_RAW;  }
    virtual const char* name_Str( void )              {   return "CAM_RAW";}

            MBOOL   setLMV(void);                       //because of LMV have the requirement of real time sync with rrz. so, need additional process
            MBOOL   setLCS(void);                       //because of LCS of sync requirement when LCS croping r needed.
            MBOOL   setSGG2(void);                  //sgg2 have 2 user:eis/rss, so using CB method the same as EIS/LCS. avoid Racing condition
            MBOOL   setRSS(void);
            MBOOL   setIMG_SEL(void);
            MBOOL   setRRZ(void);                      //for dynamical bin switch & per-frame notify MW
            MBOOL   setAWB(void);                      //for awb setting apply timing be earlier 1VD
            MBOOL   setTuning(void);                  //for dynamical bin switch
            MBOOL   dumpCQReg(void);

            inline void CBTimeChk(char *str, MBOOL bStartT);

            MUINT32 getCurPixMode(_isp_dma_enum_ dmao);
            MBOOL   setNotifyDone(void);
            MINT32  configNotify(void); //only for ISP config stage using, after RAW_PIPE::_config
            MBOOL   dynamicPak(MUINT32 hwImgoFmt); // perframe control pak/unpak Imgo format
protected:
    virtual MINT32 _config( void );             //for initial only, work with _config() in CAM_TOP_CTRL
    virtual MINT32 _write2CQ( void );

    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}

protected:
    ISP_DRV_CAM*    m_pDrv;

    CAM_REG_TG_SEN_GRAB_PXL m_TG_W;
    CAM_REG_TG_SEN_GRAB_LIN m_TG_H;
    BIN_INPUT_INFO          m_input;
    CQ_DUMP_INFO            m_dumpInput;//for RegDumpCB use

    CAM_DMX_CTRL m_Dmx;
    CAM_BMX_CTRL m_Bmx;
    CAM_RMX_CTRL m_Rmx;
    CAM_AMX_CTRL m_Amx;
    CAM_RCP_CTRL m_Rcp;
    CAM_RCP3_CTRL m_Rcp3;

    MVOID  *pIspReg;
    MUINT32 m_NotifyFrame;
    MBOOL   m_CurBin;
    MUINT32 m_CurQbn1Acc;//acc of qbin1 & rmb (pixel mode)
};


/////////////////////////////////////////////////////////////////////////////////////
class CAM_DPD_CTRL: public CAM_RAW_PIPE
{
protected:
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( void );

    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}


public:
    typedef enum{
        _level_1    = 0,
        _level_2    = 1,
        _level_max  = 2,
    }E_Density;
    E_Density       m_Density;
};

/////////////////////////////////////////////////////////////////////////////////////
class CAM_SPS_CTRL: public CAM_RAW_PIPE //source of PS
{
public:
    CAM_SPS_CTRL(){m_PSType = 0;}

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( void ){return 0;}

    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}


public:
    MUINT32         m_PSType;
};


/////////////////////////////////////////////////////////////////////////////////////
/**
    spare register of Frame Header mapping function
*/
#define E_HEADER_MAX    (16)
class HEADER_PARSER
{
protected:
    virtual ~HEADER_PARSER(){}
public:

    virtual void Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value) = 0;
    virtual void Header_Deque(MUINTPTR FH_VA,void* pOut) = 0;

public:
            ISP_HW_MODULE   m_hwModule;
};

/**
    spare register of Frame Header mapping function for imgo
*/
class Header_IMGO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_RAW_TYPE          = 3,    //spare4,
        E_PIX_ID            = 4,    //spare5,
        E_FMT               = 5,    //spare6,
        E_CROP_START        = 6,    //spare7.   [31:16]:start_y, [15:0]: start_x
        E_CROP_SIZE         = 7,    //spare8.   [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9.
        E_IMG_PA            = 9,
        E_SRC_SIZE          = 10,   //spare11 [31]:dbn_on/off [30:16]: size_h, [15:0]: size_w
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality. spare12 is shared.
        E_IS_UFE_FMT        = 13,   //spare12 record UFE fmt or not per-frame will be changed.
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_IMGO()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for rrzo
*/
class Header_RRZO: public HEADER_PARSER
{
public:

    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PIX_ID            = 3,    //spare4,
        E_FMT               = 4,    //spare5,
        E_RRZ_CRP_START     = 5,    //spare6,   [31:16]:start_y, [15:0]: start_x
        E_RRZ_CRP_SIZE      = 6,    //sapre7,   [31:16]:size_h, [15:0]: size_w
        E_RRZ_DST_SIZE      = 7,    //spare8,   [31:16]:dst_h,  [15:0]: dst_w
        E_ENQUE_CNT         = 8,    //spare9
        E_IMG_PA            = 9,
        E_IS_UFG_FMT        = 10, //spare11 record UFG fmt or not per-frame will be changed.
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
    static Mutex   m_tagAddrLock;
public:
    Header_RRZO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);    //for pipe chk use
     static MUINT32 GetTagAddr(MUINT32 Tag);
};

/**
    spare register of Frame Header mapping function for ufeo
*/
class Header_UFEO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3buf_ctrl dbg log
        E_IMG_PA            = 3,
        E_ENQUE_CNT         = 4,
        E_FMT               = 5,    //spare6,
        E_UFEO_OFFST_ADDR   = 6,  //spare7
        E_UFEO_XSIZE        = 7,            //spare8
        E_IMGO_OFFST_ADDR   = 8, //spare9
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_UFEO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for ufeo
*/
class Header_UFGO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3buf_ctrl dbg log
        E_IMG_PA            = 3,
        E_ENQUE_CNT         = 4,
        E_FMT               = 5,    //spare6,
        E_UFGO_OFFST_ADDR   = 6, //spare7
        E_UFGO_XSIZE        = 7,            //spare8
        E_RRZO_OFFST_ADDR   = 8, //spare9
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_UFGO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for AAO
*/
class Header_AAO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_IQ_LEVEL          = 2,  //spare3 record high or low image quality
        E_ENQUE_SOF         = 9,    //spare10. SOF need to be first enum.
        E_IMG_PA            = 10,
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_AAO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for AAO
*/
class Header_PSO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic              = 4,    //spare5 buf_ctrl dbg log
        E_IQ_LEVEL          = 5,  //spare6 record high or low image quality
        E_ENQUE_SOF         = 8,    //spare9
        E_IMG_PA            = 9, //spare10
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_PSO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


/**
    spare register of Frame Header mapping function for AFO
*/
class Header_AFO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_STRIDE            = 2,    //spare3,
        E_IQ_LEVEL          = 3,  //spare4, record high or low image quality
        /* FH correctness guarantee to first 4 spare registers only.
         * FOLLOWING SPARE REGS COULD BE OVERWRITTEN BY THE OTHER CAM'S
         * FH OUTPUT UNDER TAF WITH TWIN CASE. */
        E_ENQUE_SOF         = 9,    //spare10. SOF need to be first enum.
        E_IMG_PA            = 10,
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_AFO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


/**
    spare register of Frame Header mapping function for LCSO
*/
class Header_LCSO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 , buf_ctrl dbg log
        E_ENQUE_SOF         = 2,    //spare3
        E_IMG_PA            = 3,
        E_ENQUE_CNT         = 4,
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_LCSO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for PDO
*/
class Header_PDO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_STRIDE            = 2,    //spare3,
        E_PMX_A_CROP        = 3,    //spare4,  horizontal cropping start/end.[31~16:end , 15~0:start]
        E_PMX_B_CROP        = 4,    //spare5,  horizontal cropping start/end.[31~16:end , 15~0:start]
        E_BMX_A_CROP        = 5,    //spare6,  horizontal cropping start/end.[31~16:end , 15~0:start]
        E_BMX_B_CROP        = 6,    //spare7,  horizontal cropping start/end.[31~16:end , 15~0:start]
        E_IMG_PA_OFSET      = 7,
        E_IQ_LEVEL           = 8,  //spare9 record high or low image quality
        E_ENQUE_SOF         = 9,    //spare10. SOF need to be first enum.
        E_IMG_PA            = 10,    //spare11
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_PDO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for EISO
*/
class Header_LMVO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_IMG_PA            = 3,
        E_ENQUE_CNT         = 4,
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_LMVO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


/**
    spare register of Frame Header mapping function for FLKO
*/
class Header_FLKO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_IQ_LEVEL          = 2,  //spare3, record high or low image quality
        E_ENQUE_SOF         = 8,    //spare9 buf_ctrl dbg log
        E_IMG_PA            = 9,
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_FLKO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for RSSO
*/
class Header_RSSO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_IMG_PA            = 3,
        E_ENQUE_CNT         = 4,
        E_DST_SIZE          = 5, //[31:16]:dst_h,  [15:0]: dst_w
        E_IQ_LEVEL          = 11,  //spare12 record high or low image quality
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_RSSO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};



/**
    class for RRZ contrl
*/
class CAM_RRZ_CTRL: public CAM_RAW_PIPE
{
public:
    IspRect rrz_in_roi;
    IspSize rrz_out_size;
    MUINT32 rrz_rlb_offset;


    class RRZ_PQ{
    public:
        RRZ_PQ():
            PRF_BLD(0),
            PRF(0),
            BLD_SL(16),
            CR_MODE(1),
            TH_MD(80),
            TH_HI(40),
            TH_LO(80),
            TH_MD2(0)
        {};
    public:
        MUINT8  PRF_BLD;
        MUINT8  PRF;
        MUINT8  BLD_SL;
        MUINT8  CR_MODE;
        MUINT8  TH_MD;
        MUINT8  TH_HI;
        MUINT8  TH_LO;
        MUINT8  TH_MD2;
    };

public:
    CAM_RRZ_CTRL(){rrz_rlb_offset = 0;};
public:
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( void ){return 0;}
            MINT32 setDefault(void);

private:
            MUINT32 calCoefStep(MUINT32 in,MUINT32 crop,MUINT32 crop_ofst,MUINT32 crop_sub_ofst,MUINT32 out);
            MUINT32 getCoefTbl(MUINT32 resize_in,MUINT32 resize_out);
            MUINT32 getCoefNNIRTbl(MUINT32 tableSel);
};

//////////////////////////////////////////////////////////////////////////////////
/**
    class for CAM under twin control
*/
#define         _SLAVE_CAM_MAX_ (2)
class CAM_TWIN_PIPE:public CAM_RAW_PIPE
{
public:
    CAM_TWIN_PIPE();
public:
    virtual MBOOL       checkBusy( MUINTPTR param ){(void)param;return 0;}
    virtual MUINT32     id( void )                    {   return CAM_TWIN;  }
    virtual const char* name_Str( void )              {   return "CAM_TWIN";}

    typedef enum {
        E_ENABLE_TWIN = 0,
        E_BEFORE_TWIN,
        E_AFTER_TWIN,
    } E_TWIN_UPDATE_OP;

    typedef struct{
        ISP_HW_MODULE   module;
        _isp_dma_enum_  channel;
        MUINT32         offsetAdr;  // this is for patch header address
        MUINT32         uf_offsetAdr;
        MBOOL           bFrameEnd;
    }T_TWIN_DMA;
    typedef list<T_TWIN_DMA> L_T_TWIN_DMA; //this list is for multi-cam
            MINT32 get_RunTwinRst(MUINT32 dmachannel,vector<L_T_TWIN_DMA>& channel,MBOOL bDeque = MFALSE);
            MINT32 clr_RunTwinRst(MUINT32 dmachannel);

            MBOOL  HW_recover(MUINT32 step); //set 0:stop pipeline. set 1:restart pipeline
            MINT32 updateTwinInfo(MBOOL bEn, E_TWIN_UPDATE_OP op);    //update twin's info perframe

            MINT32 suspend( void ); //pause via CQ
            MINT32 resume( void );  //check slave cam which is really configured with setting of resume.Due to slave cam's page may be different at execution suspend() & resumse()

     static MBOOL  GetTwinRegAddr(ISP_HW_MODULE module,MUINT32* pAddr);

private:
    MINT32 update_TwinEn(MBOOL bEn);
    MINT32 update_beforeRunTwin(void);
    MVOID  update_before_twindrv_request(void); //this is for twin drv's SW request
    MINT32 update_afterRunTwin(void);
    MINT32 update_afterRunTwin_Dma(void);
    MINT32 update_afterRunTwin_FH(MUINT32 slave_num);
    MUINT32 update_afterRunTwin_rlb(MUINT32 rlb_oft_prv,MUINT32 rrz_wd_prv,MUINT32 rrz_wd);
    MINT32 update_cq(void);
    MINT32 update_cq_suspend(void);//for suspend


    MINT32 slaveNumCheck(MUINT32 &slave_num);
    ISP_HW_MODULE slaveModuleMap(MUINT32 slave_num, ISP_HW_MODULE MhwModule, ISP_HW_MODULE ShwModule);

protected:
    virtual MINT32 _config( void );             //for initial only, work with _config() in CAM_TOP_CTRL, via CQ
    virtual MINT32 _enable( void* pParam  );    //via CQ
    virtual MINT32 _disable(void* pParam = NULL );  //write to phy reg directly
    virtual MINT32 _write2CQ( void );

public:
    //CAM_TOP_CTRL    m_TopCtrl;

    //ENUM_IF_TWIN    m_Twin_type;
    //input info for ifunc_twin
    vector<ISP_DRV_CAM*>    m_pTwinIspDrv_v;
    MUINT32         m_subsample;

    //output info for twin drv
    MUINT32         m_rrz_out_wd;
    IspRect         m_rrz_roi;
    IspSize         m_rrz_in;

    MUINT32         m_af_vld_xstart;
    MUINT32         m_af_blk_xsize;
    MUINT32         m_af_x_win_num;

    MUINT32         m_lsc_lwidth;
    MUINT32         m_lsc_win_num_x;

    P1_TUNING_NOTIFY*       m_pTwinCB;
private:
    vector<L_T_TWIN_DMA>    m_dma_imgo_L; //this list is for multi-frame,subsample is included
    vector<L_T_TWIN_DMA>    m_dma_rrzo_L; //this list is for multi-frame,subsample is included

    MUINT32         m_rlb_oft;

    mutable Mutex   m_muList;   //protect lsit's push/pop

    //for recover fbc cnt
    CAM_REG_FBC_IMGO_CTL2       fbc_IMGO;
    CAM_REG_FBC_RRZO_CTL2       fbc_RRZO;
    CAM_REG_FBC_UFEO_CTL2       fbc_UFEO;
    CAM_REG_FBC_UFGO_CTL2       fbc_UFGO;
    CAM_REG_FBC_AFO_CTL2        fbc_AFO;
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM  when input data type is YUV
*/
class CAM_YUV_PIPE: public IspFunction_B
{
public:
    P1_TUNING_NOTIFY*   m_pP1Tuning;
public:
    CAM_YUV_PIPE()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_pP1Tuning = NULL;
    };

public:

    virtual MUINT32     id( void )                    {   return CAM_YUV;  }
    virtual const char* name_Str( void )              {   return "CAM_YUV";}

            MBOOL   setLMV(void);                       //because of LMV have the requirement of real time sync with rrz. so, need additional process
            MBOOL   setLCS(void);                       //because of LCS of sync requirement when LCS croping r needed.
            MBOOL   setSGG2(void);                  //sgg2 have 2 user:eis/rss, so using CB method the same as EIS/LCS. avoid Racing condition
            MBOOL   setRSS(void);
            MUINT32 getCurPixMode(_isp_dma_enum_ dmao=_imgo_);

protected:
    virtual MINT32 _config( void );             //for initial only, work with _config() in CAM_TOP_CTRL
    virtual MINT32 _write2CQ( void );

    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}

protected:
    ISP_DRV_CAM*    m_pDrv;

};

///////////////////////////////////////////////////////////////////////////////
/**
    class for magic number update
*/
class CAM_MAGIC_CTRL: public CAM_RAW_PIPE, public CAM_YUV_PIPE
{
public:
    MUINT32 m_nMagic;
public:
    IspDrvVir*          m_pIspDrv;              // obj for new frame
    ISP_HW_MODULE       m_hwModule;
public:
    CAM_MAGIC_CTRL():
        m_nMagic(0x0fffffff)
    {
        m_pIspDrv = NULL;
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_hwModule = CAM_MAX;
    };
public://_config defined as public member is to avoid abstract data type
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( void );

private:
    ISP_DRV_CAM*    m_pDrv;
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM tuning
*/
class CAM_TUNING_CTRL: public CAM_RAW_PIPE, public CAM_YUV_PIPE
{
public:
    CAM_TUNING_CTRL():
        m_Magic(NULL),
        m_SenDev(0xFFFFFFFF),
        m_pUniDrv(NULL),
        m_pTuningDrv(NULL),
        m_BQNum(0),
        m_hwModule (CAM_MAX),
        m_pIspDrv(NULL),
        m_pDrv(NULL)

    {};
public://_config defined as public member is to avoid abstract data type
    virtual MINT32  _config( void );
    virtual MINT32  _enable( void* pParam );
    virtual MINT32  _disable( void* pParam = NULL );
            MINT32  _init(void);
            MINT32  _uninit(void);
            MINT32  update(MBOOL UpdateFLK);
            MINT32  update(void);
            MINT32  update_end(void);

public:
            MUINT32* m_Magic;
            MUINT32 m_SenDev;
            MUINT32 m_BQNum;
            IspDrvVir*          m_pIspDrv;              // obj for new frame
            UniDrvImp*          m_pUniDrv;              // uni is linked to current path is not NULL
            ISP_HW_MODULE       m_hwModule;
private:
    ISP_DRV_CAM*    m_pDrv;
    void*           m_pTuningDrv;
};




///////////////////////////////////////////////////////////////////////////////
/**
    Class for DMAI modules
*/
class DMAI_B:public DMA_B
{
public:
    DMAI_B();

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( void );
    virtual MBOOL   Init(void) {return MFALSE;};

protected:
    ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
    MUINT32         m_updatecnt;//only workable in CQ0
public:
    UniDrvImp*      m_pUniDrv;
};

///////////////////////////////////////////////////////////////////////////////
/**
    Class for DMAO modules
*/
class DMAO_B:public DMA_B
{
public:
    DMAO_B();

    //
            MUINT32 BusSizeCal(E_PIX& pixMode);
            MINT32  setBaseAddr(void);
            MBOOL   Init(void);             //for stt dmao
protected:
    virtual MINT32 _config( void ); //config all dma but baseaddress
    virtual MINT32 _enable( void* pParam );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( void );

protected:
    ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
public:
    UniDrvImp*      m_pUniDrv;
    MUINT32         Header_Addr;
    MUINT32         m_fps;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for BPCI
*/
class DMA_BPCI:public DMAI_B
{
public:
    DMA_BPCI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_BPCI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_BPCI";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for PDI
*/
class DMA_PDI:public DMAI_B
{
public:
    DMA_PDI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_PDI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_PDI";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for LSCI
*/
class DMA_LSCI:public DMAI_B
{
public:
    DMA_LSCI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_LSCI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_LSCI";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CACI
*/
class DMA_CACI:public DMAI_B
{
public:
    DMA_CACI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CACI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CACI";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ0I
*/
class DMA_CQ0:public DMAI_B
{
public:
    DMA_CQ0(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_bSubsample = MFALSE;
        m_updatecnt = 0;
        m_bBurstQ = 0;
        memset(&multiRAWConfig, 0, sizeof(multiRAWConfig));
    };
    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
    typedef enum{
        _inner_,
        _outer_,
    }E_LoadMode;
    typedef enum{
        _cq_add_,
        _cq_delete_,
    }E_Write2CQ;

            MBOOL   setCQTriggerMode(E_MODE mode,void* ptr = NULL);
            MBOOL   TrigCQ(MBOOL bWait = MTRUE);
            MBOOL   setBaseAddr_byCQ(void);     //for cq link-list
            MBOOL   setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam);    //master cam to program slave cam's baseaddress
            MBOOL   setLoadMode(E_LoadMode mode, MBOOL bPhysical);
            MBOOL   setDoneSubSample(ISP_DRV_CAM* pSrcObj,MBOOL bEn,MUINT32 nSub=0); //for high speed case
            MBOOL   SetCQupdateCnt(void);
            UINT32  GetCQupdateCnt(MBOOL bPhy);
            MINT32  IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam);

protected:
    virtual MINT32      _config( void ); //config all dma but baseaddress
    virtual MINT32      _write2CQ( void );

public:
    virtual MBOOL       Init(void);
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ0I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ0I";}

#if CQ_SW_WORK_AROUND
            void    DummyDescriptor(void);
#endif

public:
            MBOOL   m_bSubsample;
            MUINT32 m_bBurstQ;
            //for config
            struct ISP_MULTI_RAW_CONFIG multiRAWConfig;
            static uintptr_t Addr[PHY_CAM];
            static ISP_DRV_CAM* pDrv[PHY_CAM];
            static uintptr_t Addr_dummy[PHY_CAM];
            static ISP_DRV_CAM* pDrv_dummy[PHY_CAM];
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ1I
*/
class DMA_CQ1:public DMAI_B
{
public:
    DMA_CQ1(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_bSubsample = MFALSE;
    };
    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        //_continuouse_
    }E_MODE;
    typedef enum{
        _cq_add_,
        _cq_delete_,
    }E_Write2CQ;
            MBOOL   setCQTriggerMode(E_MODE mode,void* ptr = NULL);
            MBOOL   TrigCQ(MBOOL bWait = MTRUE);

            MBOOL   setCQContent(MUINTPTR arg1,MUINTPTR arg2);
            MBOOL   setBaseAddr_byCQ(void);     //for cq link-list
            MBOOL   setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam);    //master cam to program slave cam's baseaddress
            MINT32  IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam);
public:
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ1I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ1I";}

public:
            MBOOL   m_bSubsample;


};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ2I
*/
class DMA_CQ2:public DMAI_B
{
public:
    DMA_CQ2(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ2I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ2I";}
protected:
    virtual MINT32 _config( void ){return 1;}
    virtual MINT32 _enable( void* pParam ){(void)pParam;return 1;}
    virtual MINT32 _disable( void* pParam = NULL ){(void)pParam;return 1;}
    virtual MINT32 _write2CQ( void ){return 1;}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ3I
*/
class DMA_CQ3:public DMAI_B
{
public:
    DMA_CQ3(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ3I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ3I";}
protected:
    virtual MINT32 _config( void ){return 1;}
    virtual MINT32 _enable( void* pParam ){(void)pParam;return 1;}
    virtual MINT32 _disable( void* pParam = NULL ){(void)pParam;return 1;}
    virtual MINT32 _write2CQ( void ){return 1;}

};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ4I
*/
class DMA_CQ4:public DMAI_B
{
public:
    DMA_CQ4(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ4I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ4I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ5I
*/
class DMA_CQ5:public DMAI_B
{
public:
    DMA_CQ5(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ5I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ5I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ6I
*/
class DMA_CQ6:public DMAI_B
{
public:
    DMA_CQ6(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ6I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ6I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ7I
*/
class DMA_CQ7:public DMAI_B
{
public:
    DMA_CQ7(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ7I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ7I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ8I
*/
class DMA_CQ8:public DMAI_B
{
public:
    DMA_CQ8(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ8I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ8I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ9I
*/
class DMA_CQ9:public DMAI_B
{
public:
    DMA_CQ9(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ9I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ9I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ10I
    CQ10 is dedicated for CQ11, to loading CQ11 inner BA only under high-speed mode
    because of device tree, CPU can't program inner BA directly.

    but in D-Twin ON, CQ10 at slave cam will running in immediate mode, just like CQ1 at slave cam under D-Twin ON.
*/
class DMA_CQ10:public DMAI_B
{
public:
    DMA_CQ10(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _immediate_ = 1,
    }E_MODE;
    typedef enum{
        _cq_add_,
        _cq_delete_,
    }E_Write2CQ;
            MBOOL   setCQTriggerMode(E_MODE mode,void* ptr = NULL);
            MBOOL   setCQContent(MUINTPTR arg1,MUINTPTR arg2);
            MBOOL   TrigCQ(MBOOL bWait = MTRUE);
            MBOOL   setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam);    //master cam to program slave cam's baseaddress
            MINT32  IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam);
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ10I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ10I";}

protected:
    virtual MINT32  _write2CQ( void );
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ11I
    CQ11 support only event_trig.  this is hw constraint.
*/
class DMA_CQ11:public DMAI_B
{
public:
    DMA_CQ11(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_prvSOF = 0xFFFF;
    };
    typedef enum{
        _event_trig = 0,
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

            MBOOL   setCQContent(MUINTPTR arg1,MUINTPTR arg2);

protected:
    virtual MINT32 _write2CQ( void ){return 1;};
    virtual MINT32 _disable( void* pParam = NULL );

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ11I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ11I";}

private:
            MUINT32 m_prvSOF;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ12I
*/
class DMA_CQ12:public DMAI_B
{
public:
    DMA_CQ12(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ12I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ12I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for IMGO
*/
class DMA_IMGO:public DMAO_B
{
public:
    DMA_IMGO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_IMGO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_IMGO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for RRZO
*/
class DMA_RRZO:public DMAO_B
{
public:
    DMA_RRZO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_RRZO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_RRZO";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for LCSO
*/
class DMA_LCSO:public DMAO_B
{
public:
    DMA_LCSO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_LCSO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_LCSO";}
};



///////////////////////////////////////////////////////////////////////////////
/**
    class for AAO
*/
class DMA_AAO:public DMAO_B
{
public:
    DMA_AAO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_AAO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_AAO";}
};



///////////////////////////////////////////////////////////////////////////////
/**
    class for AFO
*/
class DMA_AFO:public DMAO_B
{
public:
    DMA_AFO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_AFO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_AFO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for PSO
*/
class DMA_PSO:public DMAO_B
{
public:
    DMA_PSO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_PSO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_PSO";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for UFEO
*/
class DMA_UFEO:public DMAO_B
{
public:
    DMA_UFEO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_UFEO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_UFEO";}
    MVOID check(MBOOL& bUF_OFF);
    MVOID update_AU(void);
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for UFGO
*/
class DMA_UFGO:public DMAO_B
{
public:
    DMA_UFGO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_UFGO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_UFGO";}
    MVOID check(MBOOL& bUF_OFF);
    MVOID update_AU(void);
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for PDO
*/
class DMA_PDO:public DMAO_B
{
public:
    DMA_PDO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_PDO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_PDO";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for EISO
*/
class DMA_EISO:public DMAO_B
{
public:
    DMA_EISO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_LMVO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_LMVO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for FLKO
*/
class DMA_FLKO:public DMAO_B
{
public:
    DMA_FLKO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_FLKO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_FLKO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for RSSO
*/
class DMA_RSSO:public DMAO_B
{
public:
    DMA_RSSO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_RSSO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_RSSO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for RAWI
*/
class DMA_RAWI:public DMAI_B
{
public:
    DMA_RAWI(){};

public:
    virtual MUINT32 id( void )                {    return     UNI_DMA_RAWI;  }
    virtual const char*   name_Str( void )          {    return     "UNI_DMA_RAWI";}
};



///////////////////////////////////////////////////////////////////////////////
class PIPE_CHECK
{
    public:
        PIPE_CHECK(void)
            :m_pDrv(NULL)
            {}
        ~PIPE_CHECK(void){}

        MBOOL   Check_Start(void);
    private:
        MBOOL   DMAO_STATUS(MUINT32 err_status);
        MBOOL   UFE_CHECK(void);
        MBOOL   UFEG_CHECK(void);
        MBOOL   AA_CHECK(void);
        MBOOL   QBN1_CHECK(void);
        MBOOL   FLK_CHECK(void);
        MBOOL   LCS_CHECK(void);
        MBOOL   LMV_CHECK(void);
        MBOOL   RSS_CHECK(void);
        MBOOL   LSC_CHECK(void);
        MBOOL   AF_CHECK(MBOOL bTwin);
        MBOOL   PDO_CHECK(void);
        MBOOL   BNR_CHECK(void);
        MBOOL   TG_CHECK(MUINT32 intErrStatus);
        MBOOL   PS_CHECK(void);
        MBOOL   RMB_CHECK(void);

        MBOOL   ENQUE_CHECK(void);
        MBOOL   TWIN_CHECK(void);
        MBOOL   RRZ_CHECK(void);
        MBOOL   ME_CHECK(void); //mutully exclusive check
        MBOOL   PATH_CHECK(MBOOL bTwin);
        MBOOL   D_CPN_CHECK(void);
        MBOOL   XCPN_CHECK(void);

        MBOOL   PD_TABLE_CHECK(MUINTPTR va,MUINT32 tbl_stride);


        MBOOL   InPutSize_DMX(MUINT32& in_w,MUINT32& in_h);
        MBOOL   InPutSize_TG(MUINT32& in_w,MUINT32& in_h,MBOOL pure = MFALSE);
    public:


        ISP_DRV_CAM*    m_pDrv;
        static MUINTPTR m_BPCI_VA;
        static MUINTPTR m_PDI_VA;
    private:
        static Mutex    m_lock[PHY_CAM];
};


///////////////////////////////////////////////////////////////////////////////
typedef enum{
    eCmd_Fail = 0,          //fail
    eCmd_Pass = 1,          //ok
    eCmd_Stop_Pass,         //this state is for stopped already
    eCmd_Suspending_Pass,   //this state is for starting suspending
}E_BC_STATUS;

/**
    class for CAM DAMO ctrl, support only deque/enque 1 image at 1 time.
*/
class CAM_BUF_CTRL : public IspFunction_B
{
public:
    CAM_BUF_CTRL(void);
    ~CAM_BUF_CTRL(void){};

    virtual MUINT32 id( void )                    {   return 0xffffffff;  }
    virtual const char*   name_Str( void )              {   return "CAM_BUFFER_CTRL";}

    /**
        check if any ready image on dram.
    */
    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);
    /**
        push empty buffer into hw
    */
            E_BC_STATUS enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);

    /**
        retrieve available buffer on dram
    */
    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

    mutable Mutex   m_bufctrl_lock;//protect deque/enque behavior

private:

    #define MAX_DEPTH (64)

protected:
        QueueMgr<ST_BUF_INFO>   m_Queue;
        QueueMgr<MUINTPTR>  m_Queue_deque_ptr;

protected:
    virtual MINT32  _config( void );                // this is "scenario-level" function.
    virtual MINT32  _enable( void* pParam  ) ;      // this is "frame-leve" fucntion, but statistic-ports r "partial scneraio-level"
    virtual MINT32  _disable( void* pParam = NULL ) ;// this is "frame-leve" fucntion, but statistic-ports r "partial scneraio-level"
    virtual MINT32  _write2CQ( void ) ;

            MBOOL   PipeCheck(void);

    virtual void    FBC_STATUS(IspDrv* ptr){(void)ptr;}
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr,ISP_HW_MODULE masterModule)
                                                        {(void)fh_va;(void)buf_info;(void)header_data;(void)pStr;masterModule;}
            MUINT32 estimateTimeout(MUINT32 subSample);
public:
    static  MBOOL   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset, ISP_HW_MODULE camModule);

    typedef enum {
        eCamState_runnable = 0,     //resume flow is ready(no wait)
        eCamState_stop,             //already stopped
        eCamState_suspending,       //starting suspend flow
        eCamState_suspend,          //already suspened, for dynamic twin.
        eCamState_max
    } E_CAM_STATE;
    typedef enum {
        eCamDmaType_main = 0,
        eCamDmaType_stt,
        eCamDmaType_max
    } E_CAM_DMA_TYPE;
    //virtual cam is not supported!(due to only phy cam is supported at waitbufready())
    typedef enum{
        eSus_HWOFF  = 0x01,
        eSus_HW_SW_STATE  = 0x02,
        eSus_SIGNAL = 0x04,
        eSus_ALL    = 0x07,
    }E_SUSPEND_OP;
    static  MBOOL   suspend(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam, E_SUSPEND_OP op = eSus_ALL);
    static  MBOOL   resume(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam);
    static  MBOOL   updateState(E_CAM_STATE camState, ISP_HW_MODULE camModule, E_CAM_DMA_TYPE dmaType);

public:
    static MUINT32          m_fps[CAM_MAX]; //fps here is sw operation frequency, m_fps = sensor fps / signal subsample.
    static MUINT32          m_recentFrmTimeMs[CAM_MAX][MAX_RECENT_GRPFRM_TIME]; //upadted through normalpipe enque, for dynamic change deque timeout
    static E_CAM_STATE      m_CamState[CAM_MAX];
    static E_CAM_STATE      m_SttState[CAM_MAX];

    CAM_TIMESTAMP*          m_pTimeStamp;


protected:
    ISP_DRV_CAM*    m_pDrv;

    MUINT32         m_buf_cnt;
};

class BUF_CTRL_IMGO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_IMGO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_IMGO";}
    virtual MUINT32     id(void)            {   return _imgo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);


};

class BUF_CTRL_RRZO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_RRZO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_RRZO";}
    virtual MUINT32     id(void)            {   return _rrzo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);


};

class BUF_CTRL_UFEO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_UFEO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_UFEO";}
    virtual MUINT32     id(void)            {   return _ufeo_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_UFGO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_UFGO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_UFGO";}
    virtual MUINT32     id(void)            {   return _ufgo_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_LCSO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_LCSO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_LCSO";}
    virtual MUINT32     id(void)            {   return _lcso_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_LMVO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_LMVO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_LMVO";}
    virtual MUINT32     id(void)            {   return _lmvo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_RSSO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_RSSO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_RSSO";}
    virtual MUINT32     id(void)            {   return _rsso_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};


class BUF_CTRL_AAO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_AAO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_AAO";}
    virtual MUINT32     id(void)            {   return _aao_;  }

    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);

};

class BUF_CTRL_AFO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_AFO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_AFO";}
    virtual MUINT32     id(void)            {   return _afo_;  }

    using CAM_BUF_CTRL::waitBufReady;
    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
private:

};

class BUF_CTRL_FLKO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_FLKO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_FLKO";}
    virtual MUINT32     id(void)            {   return _flko_;  }

    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);

};

class BUF_CTRL_PDO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_PDO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_PDO";}
    virtual MUINT32     id(void)            {   return _pdo_;  }

    virtual E_BC_STATUS waitBufReady( CAM_STATE_NOTIFY *pNotify = NULL);
protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_PSO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_PSO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_PSO";}
    virtual MUINT32     id(void)            {   return _pso_;  }

    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);

};

#endif
