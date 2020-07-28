#ifndef __ISP_FUNCTION_H__
#define __ISP_FUNCTION_H__
//
#include <vector>
#include <map>
#include <list>
using namespace std;
//
//#include <utils/Errors.h>
//#include <cutils/log.h>
//#include <fcntl.h>
//#include <sys/mman.h>
//#include <cutils/atomic.h>
//#include <cutils/pmem.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <unistd.h>
//
#include "isp_datatypes.h"
#include <isp_drv.h>

//
#define CAM_ISP_PIXEL_BYTE_FP               (3)
#define CAM_ISP_PIXEL_BYTE_MAX_TRUNCATION   ((1<<3)-1)





///////////////////////////////////////////////////////////////////////////////
/**
    paraent class for isp function
    frame-based asscess, should be accessed only once within 1 frame when streaming.
*/
class IspFunction_B
{
public:
    IspFunction_B();
    virtual ~IspFunction_B(){};


public:
    MINT32  config( void );
    MINT32  enable( void* pParam  );
    MINT32  disable( void* pParam = NULL );
    MINT32  write2CQ( void );
    MINT32  is_bypass( void ) {return this->m_bypass;}
    /**
        virtual function for flow debug log only
    */
    virtual MINT32  checkBusy( MUINTPTR param ){(void)param;return -1;}
    virtual MUINT32 id( void ){return 0;}
    virtual const char*     name_Str( void ) {     return  "IspFunction_B";  }   //Name string of module

protected:
    virtual MINT32     _config( void ) {return -1;}
    virtual MINT32     _enable( void* pParam  ) {(void)pParam;return -1;}
    virtual MINT32     _disable( void* pParam = NULL ) {(void)pParam;return -1;}
    virtual MINT32     _write2CQ( void ) {return -1;}

private:
    mutable Mutex       mLock;
protected:
            ISP_HW_MODULE       m_hwModule;
public:
            MINT32              m_bypass;
            IspDrvVir*          m_pIspDrv;              // obj ptr of current sw target Isp_Drv

	static Mutex   fbcPhyLock[CAM_MAX][_dmao_max_];
};

class DMA_B: public IspFunction_B
{
public:
    virtual ~DMA_B(){};
public:
    MUINT32 GetCropXUnitBussize(MUINT32 input, MUINT32 pixelBit, MUINT32 bussize)
    {
        switch(pixelBit){
            case 10:
            case 12:
                return ((input * pixelBit) / ((bussize+1)<<3));
                break;
            default:
                return input;
        }
    }
    MUINT32 GetCropXUnitPixel(MUINT32 input, MUINT32 pixelBit, MUINT32 bussize)
    {
        switch(pixelBit){
            case 10:
            case 12:
                return ((input * ((bussize+1)<<3)) / pixelBit);
                break;
            default:
                return input;
        }
    }

public:
    IspDMACfg   dma_cfg;

};

/**
    class for continuus data on mem.
*/
class _ST_BUF_INFO
{
public:
    MUINTPTR        pa_addr;
    MUINTPTR        va_addr;
    MUINT32         size;
    MUINT32         memID;
    MUINT32         bufSecu;
    MUINT32         bufCohe;
    MUINT32         magicIdx;   // new add in cannon_ep migrated from bianco //this term is used at deque when suspending //cannon_ep
};

/**
    data structure for isp4.0 hw IO
*/
typedef struct ST_BUF_INFO__{
    struct{
        _ST_BUF_INFO    mem_info;
        //
    }image;
    _ST_BUF_INFO    header;    //buf information for hw image header
}ST_BUF_INFO;

/**
    mata structure for image operation : deque /enque
*/
typedef struct _stISP_BUF_INFO
{
    struct{
        ST_BUF_INFO deque;
        vector<ST_BUF_INFO> enque;
    }u_op;
    MUINT32 bufidx;                     //for support replace indicated memory space without unique address
    //for replacing image
    MBOOL           bReplace;
    ST_BUF_INFO     Replace;

    _stISP_BUF_INFO(
        MBOOL               _bReplace = 0x0,
        MUINT32             _index = 0xffffffff
    )
    : bufidx(_index)
    , bReplace(_bReplace)
    {
        memset(&Replace, 0, sizeof(ST_BUF_INFO));
    }
}stISP_BUF_INFO;

/**
    list for deque/enque
*/
typedef list<stISP_BUF_INFO> ISP_BUF_INFO_L;

////////////////////////////////////////////////////////////////////////////////




#endif /*__ISP_FUNCTION_H__*/




