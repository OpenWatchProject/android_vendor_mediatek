#ifndef __ISP_PATH_H__
#define __ISP_PATH_H__
#include "ispio_stddef.h"
#include "isp_function.h"   //for class : isp_function_b
#include "camera_isp.h"     //for wait struct
#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"

using android::Mutex;


#define CONFIG_FOR_SYSTRACE     0   // default:0
#if CONFIG_FOR_SYSTRACE
#define ATRACE_TAG ATRACE_TAG_CAMERA
#define ISP_TRACE_CALL() ATRACE_CALL()
#else
#define ISP_TRACE_CALL()
#endif // CONFIG_FOR_SYSTRACE


typedef enum{
    eCmd_path_stop,
    eCmd_path_restart,
}E_CAMPATH_STEP;

/*/////////////////////////////////////////////////////////////////////////////
    IspPath_B
  /////////////////////////////////////////////////////////////////////////////*/
class CamPath_B
{
public:
    CamPath_B();
    virtual ~CamPath_B(){};

public:
    virtual void    destroyInstance(void){}

    MINT32  start( void* pParam );
    MINT32  stop( void* pParam );


    virtual MINT32  registerIrq( ISP_WAIT_IRQ_ST* pIrq ){pIrq;return -1;}
    virtual MINT32  waitIrq( ISP_WAIT_IRQ_ST* pIrq ){pIrq;return -1;}
    virtual MINT32  signalIrq( ISP_WAIT_IRQ_ST* pIrq ){pIrq;return -1;}

    virtual MUINT32 readReg( MUINT32 offset ){offset;return -1;}


protected:
    virtual IspFunction_B** isp_function_list(void) {return NULL;}
    virtual MINT32          isp_function_count(void){return -1;}

    //write CQ is included
    virtual MINT32  _config( void* pParam );
    virtual MINT32  _start( void* pParam );
    virtual MINT32  _stop( void* pParam );

    virtual const char* name_Str(){     return  "IspPath";  }

public:
    ISP_HW_MODULE   m_hwModule;
protected:

    typedef enum{
        op_unknown  = 0,
        //op_cfg      = 1,
        op_start    = 2,
        op_stop     = 3,
        op_suspend
    }E_OP;
    E_OP            m_FSM;
};






#endif







