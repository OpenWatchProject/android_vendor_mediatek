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
    struct DMA_BW{
        public:
            _isp_dma_enum_  portID;
            MUINT32         ThruPut;    //peak unit: MB/s
            MUINT32         Avg_ThruPut;//avg unit: MB/s
        public:
            DMA_BW()
            {
                portID = _cam_max_;
                ThruPut = 0;
                Avg_ThruPut = 0;
            }
    };
    typedef vector<DMA_BW>  V_DMA_BW;

    struct Sig_Profile{
        public:
            MUINT32 lineT;          //unit: 10ns
            MUINT32 lineblkT;       //unit: 10ns
            MUINT32 frameT;         //unit: 1us
            MUINT32 frameblkT;      //unit: 1us
        public:
            Sig_Profile(){
                lineT = lineblkT = frameT = frameblkT = 0;
            }
    };
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

            #define SVBW_l(port,en,regname,regname_1) ({\
                DMA_BW fifo;\
                if(en){\
                    MUINT32 div = (prof.lineT - prof.lineblkT);\
                    MUINT32 _value = (CAMSV_READ_REG(this->m_pDrv,regname)+1);\
                    fifo.ThruPut = (( _value * 95) + (div -1)) / div;\
                    fifo.Avg_ThruPut = (_value * (CAMSV_READ_REG(this->m_pDrv,regname_1)+1)) * 95 / prof.frameT / 100;\
                    CAM_FUNC_DBG("ysize:%d\n",CAMSV_READ_REG(this->m_pDrv,regname_1));\
                    CAM_FUNC_DBG("BW_l-port_%d:xsize:%d,resolut:%d,(%d_%d)\n",port,CAMSV_READ_REG(this->m_pDrv,regname),fifo.ThruPut,prof.lineT,prof.lineblkT);\
                }\
                fifo.portID = port;\
                fifo;})

            #define SVBW_a(port,en,regname,regname_1) ({\
                DMA_BW fifo;\
                if(en){\
                    MUINT32 div = ((prof.frameT - prof.frameblkT)* 100);\
                    fifo.Avg_ThruPut = fifo.ThruPut = ( ((CAMSV_READ_REG(this->m_pDrv,regname)+1) * (CAMSV_READ_REG(this->m_pDrv,regname_1)+1) * 95) + (div - 1))/ div;\
                    CAM_FUNC_DBG("ysize:%d\n",CAMSV_READ_REG(this->m_pDrv,regname_1));\
                    CAM_FUNC_DBG("BW_a-port_%d:xsize:%d,resolut:%d,(%d_%d)\n",port,CAMSV_READ_REG(this->m_pDrv,regname),fifo.ThruPut,prof.frameT,prof.frameblkT);\
                }\
                fifo.portID = port;\
                fifo;})

            #define BW_l(port,en,regname,regname_1) ({\
                DMA_BW fifo;\
                if(en){\
                    MUINT32 div = (prof.lineT - prof.lineblkT);\
                    MUINT32 _value = (CAM_READ_REG(this->m_pDrv,regname)+1);\
                    fifo.ThruPut = (( _value * 95) + (div -1)) / div;\
                    fifo.Avg_ThruPut = (_value * (CAM_READ_REG(this->m_pDrv,regname_1)+1)) * 95 / prof.frameT / 100;\
                    CAM_FUNC_DBG("ysize:%d\n",CAM_READ_REG(this->m_pDrv,regname_1));\
                    CAM_FUNC_DBG("BW_l-port_%d:xsize:%d,resolut:%d,%d(%d_%d)\n",port,CAM_READ_REG(this->m_pDrv,regname),fifo.ThruPut,fifo.Avg_ThruPut,prof.lineT,prof.lineblkT);\
                }\
                fifo.portID = port;\
                fifo;})

            #define BW_a(port,en,regname,regname_1) ({\
                DMA_BW fifo;\
                if(en){\
                    MUINT32 div = ((prof.frameT - prof.frameblkT)* 100);\
                    MUINT32 _size_1 = (0x0000ffff & CAM_READ_REG(this->m_pDrv, regname) + 1);\
                    MUINT32 _size_2 = (0x0000ffff & CAM_READ_REG(this->m_pDrv, regname_1) + 1);\
                    fifo.Avg_ThruPut = fifo.ThruPut = ( ( _size_1 * _size_2 * 95) + (div - 1))/ div;\
                    CAM_FUNC_DBG("ysize:%d\n",CAM_READ_REG(this->m_pDrv,regname_1));\
                    CAM_FUNC_DBG("BW_a-port_%d:xsize:%d,resolut:%d,(%d_%d)\n",port,CAM_READ_REG(this->m_pDrv,regname),fifo.ThruPut,prof.frameT,prof.frameblkT);\
                }\
                fifo.portID = port;\
                fifo;})

            #define BW_e(port,en,data_b) ({\
                DMA_BW fifo;\
                if(en){\
                    MUINT32 div = (prof.frameblkT * 100);\
                    fifo.ThruPut = (( data_b * 95) + (div - 1))/div ;\
                    fifo.Avg_ThruPut = data_b * 95 / prof.frameT / 100;\
                    CAM_FUNC_DBG("BW_e-port_%d:data byte:%d,resolut:%d,%d,(%d)\n",port,data_b,fifo.ThruPut,fifo.Avg_ThruPut,prof.frameblkT);\
                }\
                fifo.portID = port;\
                fifo;})

            #define BW_1D(port,en,regname,vertical,data_1D) ({\
                 DMA_BW fifo;\
                 if(en){\
                    MUINT32 div = (prof.lineT - prof.lineblkT);\
                    fifo.ThruPut = (( (((CAM_READ_REG(this->m_pDrv,regname)+1)-data_1D)/vertical) * 95) + (div - 1)) / div;\
                    fifo.Avg_ThruPut = (CAM_READ_REG(this->m_pDrv,regname)+1) * 95 / prof.frameT / 100;\
                    CAM_FUNC_DBG("ysize:%d\n",vertical);\
                    CAM_FUNC_DBG("BW_1D-port_%d:xsize:%d,1D:%d,resolut:%d,%d(%d_%d)\n",port,(CAM_READ_REG(this->m_pDrv,regname)+1),data_1D,fifo.ThruPut,fifo.Avg_ThruPut,prof.lineT,prof.lineblkT);\
                    fifo.ThruPut += (((data_1D*95)/prof.frameblkT)/100);\
                    CAM_FUNC_DBG("blk thruput:%d\n",(((data_1D*95)/prof.frameblkT)/100));\
                 }\
                 fifo.portID = port;\
                 fifo;})
    //
    //@In:
    //      lineT, lineblkT : unit : 10ns
    //      SenFIFO: unit: pix
    virtual MBOOL BW_Meter(const Sig_Profile& prof,V_DMA_BW* pOut) = 0;

public:
            IspDMACfg   dma_cfg;
protected:
            //V_DMA_BW    m_FIFO;
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
    MUINT32         magicIdx; //this term is used at deque when suspending //cannon_ep
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




