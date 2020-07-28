#ifndef __ISP_FUNCTION_CAMSV_H__  // JINN +
#define __ISP_FUNCTION_CAMSV_H__

#include "isp_function.h"
#include "isp_function.timestamp.h"
#include "isp_drv_camsv.h"
#include "ispio_stddef.h"

#include <deque>

#include <Cam/buf_que_ctrl.h>

/*/////////////////////////////////////////////////////////////////////////////
    ISP Function ID
  /////////////////////////////////////////////////////////////////////////////*/

/**
    individual sub-module existed in isp function.
*/
enum {
    CAMSV_TG  = 0,    //0
    CAMSV_TOP,
    CAMSV_DMA_IMGO,
    CAMSV_BUFFER_CTRL,
    CAMSV_FUNCTION_MAX_NUM,
};

/**
    Per-frame timeout
*/
#undef MIN_GRPFRM_TIME_MS
#undef MAX_RECENT_GRPFRM_TIME
#define MIN_GRPFRM_TIME_MS          (200)  //subsampled frame time minimum difference
#define MAX_RECENT_GRPFRM_TIME      (2)


///////////////////////////////////////////////////////////////////////////////
/**
    Class for CAMSV TG module
*/
class CAMSV_TG_CTRL : public IspFunction_B
{
public:
    MBOOL               m_continuous;   //signle or continuous mode
    MUINT32             m_SubSample;      //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    IspRect             m_Crop;
    E_CamPixelMode      m_PixMode;
public:
    CAMSV_TG_CTRL();

    virtual MUINT32     id( void )                    {   return CAMSV_TG;  }
    virtual const char* name_Str( void )              {   return "CAMSV_TG";}
    virtual MUINTPTR    reg_Base_Addr_Phy( void )     {   return 0;  } // no use
    virtual MUINTPTR    reg_Range( void )             {   return 0;  } // no use
    virtual MBOOL       checkBusy( MUINTPTR param ){param;return MFALSE;}

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam );
    //virtual MINT32 _write2CQ( void ){return 0;}
public:
    IspDrvCamsv*    m_pDrv;     //for casting type , coding convenience only
};


#if 1
///////////////////////////////////////////////////////////////////////////////
/**
    Class for CAMSV Top module
*/
class CAMSV_TOP_CTRL : public IspFunction_B
{
public:
    CAMSV_TOP_CTRL();

    virtual MUINT32     id( void )                    {   return CAMSV_TOP;  }
    virtual const char* name_Str( void )              {   return "CAMSV_TOP";}
    virtual MUINTPTR    reg_Base_Addr_Phy( void )     {   return 0;  } // no use
    virtual MUINTPTR    reg_Range( void )             {   return 0;  } // no use
    virtual MBOOL       checkBusy( MUINTPTR param );

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam );
    //virtual MINT32 _write2CQ( void );


public:
    ST_CAMSV_TOP_CTRL     camsv_top_ctl;

    MUINT32               SubSample;      //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    E_CamPixelMode        m_PixMode; // Seninf pixel mode

public:
    IspDrvCamsv*    m_pDrv;     //for casting type , coding convenience only

private:
            MBOOL           m_bBusy;
#if DMX_ID_SW_WORK_AROUND
            MBOOL           m_bCfg;
#endif

};
#endif


/////////////////////////////////////////////////////////////////////////////////////
/**
    spare register of Frame Header mapping function
*/
#undef E_HEADER_MAX
#define E_HEADER_MAX    (16)
class CAMSV_HEADER_PARSER
{
protected:
    virtual ~CAMSV_HEADER_PARSER(){}
public:

    //virtual void Header_Enque(MUINT32 Tag,IspDrvCamsv* DrvPtr,MUINT32 value) = 0;
    virtual void Header_Deque(MUINTPTR FH_VA,void* pOut) = 0;

public:
            ISP_HW_MODULE   m_hwModule;
};

/**
    spare register of Frame Header mapping function for imgo
*/
class Header_CAMSV_IMGO: public CAMSV_HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_TimeStamp_MSB     = 12,    //spare13
        E_MAX
    };
public:
    Header_CAMSV_IMGO()
    {
        m_hwModule = CAMSV_MAX;
    }

    //virtual void    Header_Enque(MUINT32 Tag,IspDrvCamsv* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


///////////////////////////////////////////////////////////////////////////////
/**
    Class for DMAO modules
*/
class DMAO_CAMSV_B : public DMA_B
{
public:
    DMAO_CAMSV_B();

    //
    MUINT32 BusSizeCal(void);
    MINT32  setBaseAddr(void);
    MBOOL   Init(void);
protected:
    virtual MINT32 _config( void ); //config all dma but baseaddress
    virtual MINT32 _enable( void* pParam );
    virtual MINT32 _disable( void* pParam );

public:
    IspDrvCamsv*    m_pDrv;     //for casting type , coding convenience only
    MUINT32         Header_Addr;
    MUINT32         m_fps;
    E_CamPixelMode  m_PixMode;
    CAMSV_REG_FMT_SEL fmt_sel;
};



///////////////////////////////////////////////////////////////////////////////
/**
    class for IMGO
*/
class DMA_CAMSV_IMGO : public DMAO_CAMSV_B
{
public:
    DMA_CAMSV_IMGO(){};

public:
    virtual MUINT32 id( void )                {    return     CAMSV_DMA_IMGO;  }
    virtual const char*   name_Str( void )          {    return     "DMA_CAMSV_IMGO";}
    virtual MUINTPTR reg_Base_Addr_Phy( void )  {    return     0;  } // no use
    virtual MUINTPTR reg_Range( void )         {    return     0;  } // no use
};

///////////////////////////////////////////////////////////////////////////////
class CAMSV_PIPE_CHECK
{
    public:
        CAMSV_PIPE_CHECK(void)
            :m_pDrv(NULL),
            m_hwModule(CAMSV_0)
            {}
        ~CAMSV_PIPE_CHECK(void){}

        MBOOL   DMAO_STATUS(void);

        MBOOL   ENQUE_CHECK(void);

        MBOOL   TG_CHECK(MUINT32 intErrStatus);
    public:
        IspDrvCamsv*    m_pDrv;
        ISP_HW_MODULE   m_hwModule;
};


/**
    class for CAMSV DAMO ctrl, support only deque/enque 1 image at 1 time.
*/
class CAMSV_BUF_CTRL : public IspFunction_B
{
public:
    CAMSV_BUF_CTRL(void);
    ~CAMSV_BUF_CTRL(void){};

    virtual MUINT32 id( void )                    {   return 0xffffffff;  }
    virtual const char*   name_Str( void )              {   return "CAMSV_BUFFER_CTRL";}
    virtual MUINTPTR reg_Base_Addr_Phy( void )     {   return (MUINT32)NULL;  }
    virtual MUINTPTR reg_Range( void )             {   return (MUINT32)NULL;  }


    /**
        check if any ready image on dram.
    */
    typedef enum{
        eCmd_Fail = 0,          //fail
        eCmd_Pass = 1,          //ok
        eCmd_Stop_Pass,         //this state is for stopped already
        eCmd_Suspending_Pass,   //this state is for starting suspending
    }E_BC_STATUS;
    E_BC_STATUS waitBufReady( void);
    /**
        push empty buffer into hw
    */
    MBOOL       enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);

    /**
        retrieve available buffer on dram
    */
    E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );
    MUINT32 estimateTimeout(MUINT32 subSample);
    MBOOL updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset, ISP_HW_MODULE camModule);

    MBOOL suspend(void);
    MBOOL resume(void);

protected:
    virtual MINT32  _config( void );
    virtual MINT32  _enable( void* pParam  ) ;
    virtual MINT32  _disable( void* pParam ) ;
    //virtual MINT32  _write2CQ( void ) ;

            MBOOL   PipeCheck(void);

    virtual void    FBC_STATUS(){}
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr){fh_va=(MUINTPTR)NULL;buf_info;header_data=NULL;pStr=NULL;}

public:
    static MUINT32  m_fps;          //fps here is sw operation frequency, m_fps = sensor fps / signal subsample.
    static MUINT32  m_recentFrmTimeMs[CAMSV_MAX][MAX_RECENT_GRPFRM_TIME]; //upadted through normalpipe enque, for dynamic change deque timeout

    IspDrvCamsv*    m_pDrv;
    CAM_TIMESTAMP*  m_pTimeStamp;

protected:

    MUINT32                 m_buf_cnt;

private:
    class C_FSM
    {
        public:
        C_FSM(){m_State = E_NORMAL;}
        public:
            typedef enum{
                E_NORMAL,
                E_SUSPEND,
                E_RESUME,
            }E_STATE;

            MBOOL   SetFSM(E_STATE state,MBOOL lock);
            E_STATE GetFSM(void);
        private:
                    E_STATE         m_State;
            mutable Mutex           m_lock;

    };

    C_FSM                   m_FSM;
    mutable Mutex           m_bufctrl_lock;//protect deque/enque behavior
    QueueMgr<ST_BUF_INFO>   m_Queue;

    #undef MAX_DEPTH
    #define MAX_DEPTH   (32)
};

class BUF_CTRL_CAMSV_IMGO : public CAMSV_BUF_CTRL
{
public:
    BUF_CTRL_CAMSV_IMGO()
    {
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_CAMSV_IMGO";}
    virtual MUINT32     id(void)            {   return _camsv_imgo_;  }

protected:
    virtual void    FBC_STATUS();
};

#endif // __ISP_FUNCTION_CAMSV_H__ // JINN -

