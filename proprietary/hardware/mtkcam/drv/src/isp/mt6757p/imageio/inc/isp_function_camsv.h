#ifndef __ISP_FUNCTION_CAMSV_H__  // JINN +
#define __ISP_FUNCTION_CAMSV_H__

#include "isp_function.h"
#include "isp_drv_camsv.h"
#include "ispio_stddef.h"

#include <deque>


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
    E_PIX_MODE          m_PixMode;
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

    MUINT32             SubSample;      //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    E_PIX_MODE          m_PixMode; // Seninf pixel mode

public:
    IspDrvCamsv*    m_pDrv;     //for casting type , coding convenience only

private:
            MBOOL           m_bBusy;
#if DMX_ID_SW_WORK_AROUND
            MBOOL           m_bCfg;
#endif

};
#endif


#if 0
/////////////////////////////////////////////////////////////////////////////////////
/**
    spare register of Frame Header mapping function
*/
#define E_HEADER_MAX    (16)
class CAMSV_HEADER_PARSER
{
protected:
    virtual ~CAMSV_HEADER_PARSER(){}
public:

    virtual void Header_Enque(MUINT32 Tag,IspDrvCamsv* DrvPtr,MUINT32 value) = 0;
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
        E_TimeStamp         = 0,    //spare1
        E_Magic             = 1,    //spare2 ,
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_RAW_TYPE          = 3,    //spare4,
        E_PIX_ID            = 4,    //spare5,
        E_FMT               = 5,    //spare6,
        E_CROP_START        = 6,    //spare7.   [31:16]:start_y, [15:0]: start_x
        E_CROP_SIZE         = 7,    //spare8.   [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9.
        E_IMG_PA            = 9,
        E_MAX               = 10
    };
public:
    Header_CAMSV_IMGO()
    {
        m_hwModule = CAMSV_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,IspDrvCamsv* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};
#endif


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
    E_PIX_MODE          m_PixMode;
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
    virtual MBOOL waitBufReady( void);
    /**
        push empty buffer into hw
    */
    MBOOL enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);

    /**
        retrieve available buffer on dram
    */
    MBOOL dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );


private:
    mutable Mutex   m_bufctrl_lock;//protect deque/enque behavior

    #define MAX_DEPTH (32)

protected:
    /**
        container
    */
    template<typename typ>
    class QueueMgr {
        /////
        protected:
            std::deque<typ>         m_v_pQueue;
            //remove this mutex, using buffer cotrl mutex to protect whole deque/enque behavior.
            //mutable Mutex                   mQueueLock;
        /////
        public:
            QueueMgr()
                : m_v_pQueue()
            {}

            virtual ~QueueMgr(){};
            ////
                    void    init();
                    void    uninit();
            //FIFO cmd
            typedef enum{
                eCmd_getsize = 0,
                eCmd_push = 1,
                eCmd_pop = 2,
                eCmd_front = 3,
                eCmd_end = 4,
                eCmd_at = 5,
                eCmd_pop_back = 6
            }QueueMgr_cmd;
                    MBOOL   sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2);
        };

        QueueMgr<ST_BUF_INFO>   m_Queue;
        QueueMgr<MUINTPTR>  m_Queue_deque_ptr;
#if 0
    class BUF_CNT{
        protected:
            mutable Mutex   mLock;
        private:
            MUINT32         m_buf_cnt;
        public:
            BUF_CNT()
                :m_buf_cnt(0)
            {}
            virtual ~BUF_CNT(){}

            void setCnt(void);
            void clrCnt(void);
            void getCnt(void);
        };

        BUF_CNT     m_BufCnt;
#endif

protected:
    virtual MINT32  _config( void );
    virtual MINT32  _enable( void* pParam  ) ;
    virtual MINT32  _disable( void* pParam ) ;
    //virtual MINT32  _write2CQ( void ) ;

            MBOOL   PipeCheck(void);

    virtual void    FBC_STATUS(){}
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr){fh_va;buf_info;header_data;pStr;}
            MBOOL   TimeStamp_mapping(MUINT32* pSec, MUINT32* pUsec);

public:
    static MUINT32  m_fps;          //fps here is sw operation frequency, m_fps = sensor fps / signal subsample.

    IspDrvCamsv*    m_pDrv;

protected:
    MUINT32         m_buf_cnt;
    MINT32          m_tail_idx;

    MUINT32         m_prvClk;
    MUINT32         m_nOverFlow;
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
    //virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr);
};

#endif // __ISP_FUNCTION_CAMSV_H__ // JINN -

