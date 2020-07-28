#define LOG_TAG "ifunc_dmax"

//open syscall
#include <mtkcam/def/PriorityDefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
#include <utils/StrongPointer.h>

#include <unistd.h> //for usleep

#include <cutils/properties.h>  // For property_get().

#include <ispio_pipe_ports.h>
#include "isp_function_cam.h"
//#include "dual_isp_config.h"
#include "Cam_Notify_datatype.h"
#include "tuning_drv.h"


#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


EXTERN_DBG_LOG_VARIABLE(func_cam);

// Clear previous define, use our own define.
#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST
#define CAM_FUNC_VRB(fmt, arg...)        do { if (func_cam_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do { if (func_cam_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_cam_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

//
/*/////////////////////////////////////////////////////////////////////////////
    DMAI_B
  /////////////////////////////////////////////////////////////////////////////*/
#define DMA_CON 1
DMAI_B::DMAI_B()
{
    m_pUniDrv = NULL;
    m_pDrv = NULL;
    m_updatecnt = 0;
}

MINT32 DMAI_B::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_config+ [%s]:cq(0x%x),page(0x%x),pa(0x%08X),ofst(0x%08X),size(%d,%d,%d,%d),pixel_byte(%d),crop(%d,%d)\n",this->name_Str(),cq,page,\
        this->dma_cfg.memBuf.base_pAddr, \
        this->dma_cfg.memBuf.ofst_addr,
        this->dma_cfg.size.w, \
        this->dma_cfg.size.h, \
        this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
        this->dma_cfg.pixel_byte, \
        this->dma_cfg.crop.x, \
        this->dma_cfg.crop.y);


    switch(this->id())
    {
        case CAM_DMA_CQ0I:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN))
                this->m_pDrv->setDeviceInfo(_SET_CQ_SW_PATCH,(MUINT8*)&this->dma_cfg.memBuf.base_pAddr);
            else
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ1I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ2I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR2_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ3I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR3_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ4I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR4_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ5I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR5_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ6I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR6_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ7I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR7_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ8I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR8_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ9I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR9_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ10I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ11I:
            //use CQ10 to load CQ11 inner & outter address.
            //because CPU can't program inner address when using device tree.
            CAM_WRITE_REG(this->m_pDrv,CAM_CQ_THR11_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            CAM_WRITE_REG(this->m_pDrv,CAM_CQ_THRE11_ADDR_INNER,this->dma_cfg.memBuf.base_pAddr);
           break;
        case CAM_DMA_BPCI:
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_STRIDE,this->dma_cfg.size.stride);

            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);


#if DMA_CON
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BPCI_CON,0x80000020);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BPCI_CON2,0x00000000);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BPCI_CON3,0x00000000);
#endif
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_STRIDE,this->dma_cfg.size.stride);//stride;

            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

#if DMA_CON
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_LSCI_CON,0x80000020);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_LSCI_CON2,0x00000000);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_LSCI_CON3,0x00000000);
#endif
            break;
        case UNI_DMA_RAWI:
            if(this->dma_cfg.crop.x + this->dma_cfg.crop.y + this->dma_cfg.crop.w + this->dma_cfg.crop.h){
                CAM_FUNC_ERR("%s :cropping is not supported.bypass cropping\n",this->name_Str());
            }
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RAWI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RAWI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);

            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RAWI_XSIZE,this->dma_cfg.size.xsize - 1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RAWI_YSIZE,this->dma_cfg.size.h - 1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RAWI_STRIDE,this->dma_cfg.size.stride);


#if DMA_CON
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_RAWI_CON,0x80a0a0a0);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_RAWI_CON2,0x00a0a000);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_RAWI_CON3,0x00a0a000);
#endif
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


    CAM_FUNC_DBG("-\n");
    return 0;
}



MINT32 DMAI_B::_enable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)pParam;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_enable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id()){
        case CAM_DMA_CQ0I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,1);
            break;
        case CAM_DMA_CQ1I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_CTL,CQ_THR1_EN,1);
            break;
        case CAM_DMA_CQ2I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR2_CTL,CQ_THR2_EN,1);
            break;
        case CAM_DMA_CQ3I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR3_CTL,CQ_THR3_EN,1);
            break;
        case CAM_DMA_CQ4I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR4_CTL,CQ_THR4_EN,1);
            break;
        case CAM_DMA_CQ5I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR5_CTL,CQ_THR5_EN,1);
            break;
        case CAM_DMA_CQ6I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR6_CTL,CQ_THR6_EN,1);
            break;
        case CAM_DMA_CQ7I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR7_CTL,CQ_THR7_EN,1);
            break;
        case CAM_DMA_CQ8I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR8_CTL,CQ_THR8_EN,1);
            break;
        case CAM_DMA_CQ9I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR9_CTL,CQ_THR9_EN,1);
            break;
        case CAM_DMA_CQ10I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_CTL,CQ_THR10_EN,1);
            break;
        case CAM_DMA_CQ11I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR11_CTL,CQ_THR11_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,I2C_CQ_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CQ_SEL,0);//always imm mode
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,BPCI_EN,1);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LSCI_EN,1);
            break;
        case UNI_DMA_RAWI:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,RAWI_A_EN,1);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            return -1;
            break;
    }
    return 0;
}


MINT32 DMAI_B::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_disable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id()){
        case CAM_DMA_CQ0I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,0);
            //reset update cnt when disable
            this->m_updatecnt = 0;
            break;
        case CAM_DMA_CQ1I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_CTL,CQ_THR1_EN,0);
            break;
        case CAM_DMA_CQ2I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR2_CTL,CQ_THR2_EN,0);
            break;
        case CAM_DMA_CQ3I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR3_CTL,CQ_THR3_EN,0);
            break;
        case CAM_DMA_CQ4I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR4_CTL,CQ_THR4_EN,0);
            break;
        case CAM_DMA_CQ5I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR5_CTL,CQ_THR5_EN,0);
            break;
        case CAM_DMA_CQ6I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR6_CTL,CQ_THR6_EN,0);
            break;
        case CAM_DMA_CQ7I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR7_CTL,CQ_THR7_EN,0);
            break;
        case CAM_DMA_CQ8I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR8_CTL,CQ_THR8_EN,0);
            break;
        case CAM_DMA_CQ9I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR9_CTL,CQ_THR9_EN,0);
            break;
        case CAM_DMA_CQ10I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_CTL,CQ_THR10_EN,0);
            break;
        case CAM_DMA_CQ11I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR11_CTL,CQ_THR11_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,I2C_CQ_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CQ_SEL,0);//always imm mode
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,BPCI_EN,0);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LSCI_EN,0);
            break;
        case UNI_DMA_RAWI:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,RAWI_A_EN,0);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            return -1;
            break;
    }
    return 0;
}



MINT32 DMAI_B::_write2CQ(void)
{
    MUINT32 cq_module_id = 0xFF;

    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_write2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id())
    {
        case CAM_DMA_BPCI:
            cq_module_id = CAM_DMA_BPCI_;
            break;
        case CAM_DMA_LSCI:
            cq_module_id = CAM_DMA_LSCI_;
            break;
        case UNI_DMA_RAWI:
            cq_module_id = UNI_DMA_RAWI_;
            break;
        default:
            CAM_FUNC_DBG("DMAI: NOT push to CQ ");
            break;
    }

    this->m_pDrv->cqAddModule(cq_module_id);

    //
    CAM_FUNC_DBG("-\n");
    return 0;
}


MINT32 DMA_CQ0::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ0::_write2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id())
    {
        case CAM_DMA_CQ0I:
            //when subsample function enable, using cq inner reg
            //otherwise, using outter reg
            if(this->m_bSubsample == MTRUE){
                this->m_pDrv->cqAddModule(CAM_CQ_THRE0_ADDR_CQONLY);
#if TWIN_SW_P1_DONE_WROK_AROUND
                this->m_pDrv->cqAddModule(CAM_CQ_THRE0_ADDR_);
#endif
            }else{
                this->m_pDrv->cqAddModule(CAM_CQ_THRE0_ADDR_);
            }

            break;
        default:
            CAM_FUNC_DBG("DMAI: NOT push to CQ ");
            break;
    }



    //
    CAM_FUNC_DBG("-\n");
    return 0;
}


MBOOL DMA_CQ0::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_INF("DMA_CQ0::setCQTriggerMode:+ mode = 0x%x\n",mode);

    if(mode == _immediate_){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
            CAM_FUNC_ERR("[%s]:can't use immediate mode under streaming\n",this->name_Str());
            return MFALSE;
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }
    else{
        //remove to cam_top_ctrl::_enable(), because this bit is a shared bit to all CQ
        //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}

MBOOL DMA_CQ0::setBaseAddr_byCQ(void)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //when subsample function enable, using cq inner reg
    //otherwise, using outter reg
    if(this->m_bSubsample == MTRUE){
        CAM_WRITE_REG(this->m_pDrv,CAM_CQ_THRE0_ADDR_INNER,this->dma_cfg.memBuf.base_pAddr);
#if TWIN_SW_P1_DONE_WROK_AROUND
        CAM_WRITE_REG(this->m_pDrv,CAM_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
#endif
    }
    else{
        CAM_WRITE_REG(this->m_pDrv,CAM_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
    }

    return MTRUE;
}


MBOOL DMA_CQ0::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ0::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR0_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR0_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ0::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ0::setLoadMode(E_LoadMode mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 tmp;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);


    this->m_pDrv->cqAddModule(CAM_CQ_EN_);
    tmp = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_EN);
    CAM_FUNC_INF("DMA_CQ0::setLoadMode:[%s]:page(0x%x),cur reg:0x%x\n",this->name_Str(),page,tmp);
    CAM_WRITE_REG(this->m_pDrv,CAM_CQ_EN,tmp);

    switch(mode){
        case _inner_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
            break;
        case _outer_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
            break;
        default:
            CAM_FUNC_ERR("unsupported mode:[%s]:cq(0x%x),page(0x%x),mode(0x%x)\n",this->name_Str(),cq,page,mode);
            return MFALSE;
            break;
    }
    return MTRUE;
}

MBOOL DMA_CQ0::setDoneSubSample(ISP_DRV_CAM* pSrcObj,MBOOL bEn,MUINT32 nSub)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    #if (FBC_DRPGRP_SW_WORK_AROUND == 0)
    MUINT32 tmp;
    #endif

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);


    this->m_pDrv->cqAddModule(CAM_CTL_SW_DONE_SEL_);
    #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,IMGO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RRZO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,UFEO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,LCSO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,0);
    #else
    //signal control, can't run-time change
    //imgo
    tmp = CAM_READ_BITS(pSrcObj,CAM_FBC_IMGO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,IMGO_FIFO_FULL_DROP,tmp);

    //rrzo
    tmp = CAM_READ_BITS(pSrcObj,CAM_FBC_RRZO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RRZO_FIFO_FULL_DROP,tmp);

    //ufeo
    tmp = CAM_READ_BITS(pSrcObj,CAM_FBC_UFEO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,UFEO_FIFO_FULL_DROP,tmp);

    //lcso
    tmp = CAM_READ_BITS(pSrcObj,CAM_FBC_LCSO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,LCSO_FIFO_FULL_DROP,tmp);

    //eiso
    tmp = UNI_READ_BITS(pSrcObj,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,tmp);

    //rsso
    tmp = UNI_READ_BITS(pSrcObj,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,tmp);
    #endif


    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,bEn);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,nSub);

    CAM_FUNC_DBG("DMA_CQ0::setDoneSubSample:[%s]:page(0x%x),cur reg:0x%x\n",this->name_Str(),page,CAM_READ_REG(this->m_pDrv,CAM_CTL_SW_PASS1_DONE));

    return MTRUE;
}


#if CQ_SW_WORK_AROUND
void DMA_CQ0::DummyDescriptor(void)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->cqAddModule(CAM_DUMMY_);
}

#endif


MBOOL DMA_CQ0::SetCQupdateCnt(void)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //this is for user to have the capibitlity to judge the case which current CQ is already applied or not.
    this->m_pDrv->cqAddModule(CAM_CQ_COUNTER);

    this->m_updatecnt++;
    CAM_WRITE_REG(this->m_pDrv ,CAM_CTL_SPARE2,this->m_updatecnt);

    return MTRUE;
}

UINT32 DMA_CQ0::GetCQupdateCnt(MBOOL bPhy)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(bPhy)
        return CAM_READ_REG(this->m_pDrv->getPhyObj() ,CAM_CTL_SPARE2);
    else
        return CAM_READ_REG(this->m_pDrv ,CAM_CTL_SPARE2);
}


MBOOL DMA_CQ1::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ1::setCQTriggerMode:+ mode = 0x%x\n",mode);



    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_CTL,CQ_THR1_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}

MBOOL DMA_CQ1::setCQContent(MUINTPTR arg1,MUINTPTR arg2)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    list<NSImageio::NSIspio::ISPIO_REG_CFG>* ptr = (list<NSImageio::NSIspio::ISPIO_REG_CFG>*)arg2;
    list<NSImageio::NSIspio::ISPIO_REG_CFG>::iterator it;
    MUINT32 i;

    ISP_DRV_REG_IO_STRUCT* pReg = NULL;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ1::cfg:+ \n");

    //
    pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * ptr->size());
    if (NULL == pReg) {
        CAM_FUNC_ERR("DMA_CQ1::setCQContent: alloc buf fail\n");
        return MFALSE;
    }
    for(it = ptr->begin(),i=0;it != ptr->end();it++,i++){
        pReg[i].Addr = it->Addr;
        pReg[i].Data = it->Data;
    }
    this->m_pDrv->CQ_SetContent(pReg,ptr->size());
    free(pReg);

    return MTRUE;
}


MBOOL DMA_CQ1::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ1::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR1_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR1_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ1::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}


MBOOL DMA_CQ4::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ4::setCQTriggerMode:+ mode = 0x%x\n",mode);

    if(mode == _immediate_){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
            CAM_FUNC_ERR("[%s]:can't use immediate mode under streaming\n",this->name_Str());
            return MFALSE;
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }
    else{
        //remove to cam_top_ctrl::_enable(), because this bit is a shared bit to all CQ
        //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR4_CTL,CQ_THR4_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}


MBOOL DMA_CQ4::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ4::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR4_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR4_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR4_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ4::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ5::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ5::setCQTriggerMode:+ mode = 0x%x\n",mode);

    if(mode == _immediate_){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
            CAM_FUNC_ERR("[%s]:can't use immediate mode under streaming\n",this->name_Str());
            return MFALSE;
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }
    else{
        //remove to cam_top_ctrl::_enable(), because this bit is a shared bit to all CQ
        //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR5_CTL,CQ_THR5_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}

MBOOL DMA_CQ5::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ5::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR5_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR5_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR5_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ5::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ7::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ7::setCQTriggerMode:+ mode = 0x%x\n",mode);

    if(mode == _immediate_){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
            CAM_FUNC_ERR("[%s]:can't use immediate mode under streaming\n",this->name_Str());
            return MFALSE;
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }
    else{
        //remove to cam_top_ctrl::_enable(), because this bit is a shared bit to all CQ
        //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR7_CTL,CQ_THR7_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}

MBOOL DMA_CQ7::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ7::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR7_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR7_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR7_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ7::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ8::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ8::setCQTriggerMode:+ mode = 0x%x\n",mode);

    if(mode == _immediate_){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
            CAM_FUNC_ERR("[%s]:can't use immediate mode under streaming\n",this->name_Str());
            return MFALSE;
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }
    else{
        //remove to cam_top_ctrl::_enable(), because this bit is a shared bit to all CQ
        //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR8_CTL,CQ_THR8_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}

MBOOL DMA_CQ8::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ8::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR8_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR8_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR8_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ8::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}

MINT32 DMA_CQ10::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ10::_write2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id())
    {
        case CAM_DMA_CQ10I:
            this->m_pDrv->cqAddModule(CAM_CQ_THRE11_ADDR_CQONLY, ISP_DRV_CQ_THRE11);
            this->m_pDrv->cqAddModule(CAM_CQ_THRE11_ADDR_);
            break;
        default:
            CAM_FUNC_DBG("DMAI: NOT push to CQ ");
            break;
    }



    //
    CAM_FUNC_DBG("-\n");
    return 0;
}


MBOOL DMA_CQ10::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ10::setCQTriggerMode:+ mode = 0x%x\n",mode);

#if 0   //remove this if statement is because of CQ11.
    if(mode == _immediate_){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
            CAM_FUNC_ERR("[%s]:can't use immediate mode under streaming\n",this->name_Str());
            return MFALSE;
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }
    else{
        //remove to cam_top_ctrl::_enable(), because this bit is a shared bit to all CQ
        //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }
#endif
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_CTL,CQ_THR10_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}

MBOOL DMA_CQ10::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ10::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR10_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR10_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            break;
            CAM_FUNC_ERR("DMA_CQ10::TrigCQ: RDMA fail\n");
            return MFALSE;
        }
    }
    return MTRUE;
}


MBOOL DMA_CQ11::setCQTriggerMode(E_MODE mode)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ11::setCQTriggerMode:+ mode = 0x%x\n",mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR11_CTL,CQ_THR11_MODE,mode);

    //
    CAM_FUNC_DBG("-\n");

    return MTRUE;
}


MBOOL DMA_CQ11::TrigCQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 curSOF;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ11::TrigCQ:+ \n");

    //CQ11's trig source under tiwn mode is still from TG_A.
    if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE))
    {
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN)){
            return MTRUE;
        }
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&curSOF);
    if(this->m_prvSOF != 0xFFFF){
        MUINT32 diff = (curSOF > this->m_prvSOF)?(curSOF - this->m_prvSOF):(this->m_prvSOF - curSOF);
        MUINT32 _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD)+1;

        if(diff < (2*_tmp)){
            CAM_FUNC_ERR("CQ11 can't be trigged again when CQ11 still running %d_%d\n", this->m_prvSOF, curSOF);
            return MFALSE;
        }
    }
    this->m_prvSOF = curSOF;

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_I2C_CQ_TRIG,TG_I2C_CQ_TRIG,1);
    return MTRUE;
}

MBOOL DMA_CQ11::setCQContent(MUINTPTR arg1,MUINTPTR arg2)
{
    MUINT32 _tmp;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    list<NSImageio::NSIspio::ISPIO_REG_CFG>* ptr = (list<NSImageio::NSIspio::ISPIO_REG_CFG>*)arg2;
    list<NSImageio::NSIspio::ISPIO_REG_CFG>::iterator it;
    MUINT32 i;

    ISP_DRV_REG_IO_STRUCT* pReg = NULL;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ11::cfg:+ timing:0x%x\n",arg1);

    _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD);
    //cfg number
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_CQ_NUM,TG_CQ_NUM,_tmp);
    //cfg timing
    if(arg1 == 0){
        CAM_FUNC_ERR("CQI loading at idx:0 is not supported\n");
        return MFALSE;
    }
    arg1 = (1<<(arg1-1));
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_CQ_TIMING,TG_I2C_CQ_TIM,arg1);

    //
    if(ptr != NULL){
        pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * ptr->size());
        if (NULL == pReg) {
            CAM_FUNC_ERR("DMA_CQ11::setCQContent: alloc buf fail\n");
            return MFALSE;
        }
        for(it = ptr->begin(),i=0;it != ptr->end();it++,i++){
            pReg[i].Addr = it->Addr;
            pReg[i].Data = it->Data;
        }
        this->m_pDrv->CQ_SetContent(pReg,ptr->size());
        free(pReg);
    }
    return MTRUE;
}

MINT32 DMA_CQ11::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ11::_disable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR11_CTL,CQ_THR11_EN,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,I2C_CQ_EN,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CQ_SEL,0);//always imm mode

    this->m_prvSOF = 0xFFFF;
    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    DMAO_B
/////////////////////////////////////////////////////////////////////////////*/
DMAO_B::DMAO_B()
{
    m_pUniDrv = NULL;
    Header_Addr = 0x0;
    m_pDrv = NULL;
    m_fps = 0;
}

MBOOL DMAO_B::Init(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_INF("DMAO_B::Init+[%s]:cq(0x%x),page(0x%x)\n", \
        this->name_Str(), \
        cq,page);

    switch(this->id())
    {
        case UNI_DMA_FLKO:
        case CAM_DMA_AFO:
        case CAM_DMA_AAO:
        case CAM_DMA_PDO:
            //clr BUF LIST
            CQ_RingBuf_ST ctrl;
            ctrl.ctrl = BUF_CTRL_CLEAR;
            this->m_pDrv->cqRingBuf(&ctrl);
            break;
        default:
            CAM_FUNC_ERR("this dmao:%s is not supported\n",this->name_Str());
            return MFALSE;
            break;
    }

    return MTRUE;
}

MINT32 DMAO_B::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 dbn_en, bin_en;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    dbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);
    bin_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);

    CAM_FUNC_DBG("DMAO_B::_config+[%s]:port(%d),cq(0x%x),page(0x%x),pa(0x%08X_0x%08X),ofst(0x%X)\n", \
        this->name_Str(), this->id(),\
        cq,page,\
        this->dma_cfg.memBuf.base_pAddr, this->Header_Addr,\
        this->dma_cfg.memBuf.ofst_addr);


    switch(this->id())
    {
        case UNI_DMA_EISO:
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_EISO_XSIZE,0xFF);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_EISO_YSIZE,0);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_EISO_STRIDE,0xFF + 1);
#if DMA_CON
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_EISO_CON,0x80000020);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_EISO_CON2,0x00100010);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_EISO_CON3,0x00100010);
#endif
            break;
        case UNI_DMA_FLKO:
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FLKO_XSIZE,this->dma_cfg.size.xsize - 1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FLKO_YSIZE,this->dma_cfg.size.h - 1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FLKO_STRIDE,this->dma_cfg.size.stride);
#if DMA_CON
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FLKO_CON,0x80000020);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FLKO_CON2,0x00100010);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FLKO_CON3,0x00100010);
#endif
            break;
        case UNI_DMA_RSSO:
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RSSO_A_XSIZE,this->dma_cfg.size.xsize - 1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RSSO_A_YSIZE,this->dma_cfg.size.h - 1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RSSO_A_STRIDE,this->dma_cfg.size.stride);
#if DMA_CON
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_RSSO_A_CON,0x80000040);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_RSSO_A_CON2,0x00200020);
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_RSSO_A_CON3,0x00200020);
#endif
            break;
        case CAM_DMA_AFO:
            {
                Header_AFO fh_afo;

                fh_afo.Header_Enque(Header_AFO::E_STRIDE,this->m_pDrv,this->dma_cfg.size.stride);

                CAM_WRITE_REG(this->m_pDrv,CAM_AFO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_AFO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_AFO_STRIDE,this->dma_cfg.size.stride);

#if DMA_CON
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_AFO_CON,0x80000080);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_AFO_CON2,0x00400040);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_AFO_CON3,0x00400040);
#endif
            }
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_REG(this->m_pDrv,CAM_AAO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_AAO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_AAO_STRIDE,this->dma_cfg.size.stride);

#if DMA_CON
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_AAO_CON,0x80000040);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_AAO_CON2,0x00200020);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_AAO_CON3,0x00200020);
#endif

            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_STRIDE,this->dma_cfg.size.stride);


#if DMA_CON
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_LCSO_CON,0x80000020);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_LCSO_CON2,0x00100010);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_LCSO_CON3,0x00100010);
#endif
            break;
        case CAM_DMA_IMGO:
            {
            #define ROUNDING 0  //current cropping method is round-down , so no need to check wheather the cropping is out of range or not
                MUINT32 new_crop_x = 0;
                Header_IMGO fh_imgo;
#if ROUNDING
                MUINT32 in_w=0,new_crop_x = 0;
                //
                //bus size only needed at dmao crop. because of bus_size_en is always 0.
                //but cropping start need the information of current hw bus size.
                //for getting max cropping range
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,DMX_SEL) == 2){//rawi path
                    if( (in_w = (UNI_READ_REG(this->m_pDrv,CAM_UNI_RAWI_XSIZE) + 1)) == 0){
                        CAM_FUNC_ERR("Can't configure IMGO before RAWI\n");
                        return -1;
                    }
                    in_w = in_w * 8 / this->dma_cfg.pixel_byte;
                }
                else{ // TG in
                    in_w = (this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                    in_w = (in_w>>16) - in_w;
                }
#endif

// IMGO always output full-size whether dbn/bin is enable or not
// So, mark this part
#if 0
                // If DBN=1, imgo setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
                // dbn: x-dir => /2, y-dir => same; bin: x-dir => /2, y-dir => /2
                this->dma_cfg.size.w >>= (dbn_en + bin_en);
                this->dma_cfg.size.h >>= (bin_en);
                this->dma_cfg.size.xsize >>= (dbn_en + bin_en);
                this->dma_cfg.crop.x >>= (dbn_en + bin_en);
                this->dma_cfg.crop.y >>= (bin_en);
                this->dma_cfg.crop.floatX>>= (dbn_en + bin_en);
                this->dma_cfg.crop.floatY >>= (bin_en);
                this->dma_cfg.crop.w >>= (dbn_en + bin_en);
                this->dma_cfg.crop.h >>= (bin_en);

                /// TODO: ISP_QUERY_SIZE??
#endif

                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_STRIDE,this->dma_cfg.size.stride);
                //
                this->dma_cfg.bus_size = BusSizeCal();
                new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));
                //reverse for FH below
                new_crop_x = GetCropXUnitPixel(new_crop_x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);
#if ROUNDING
                if( (new_crop_x + this->dma_cfg.size.w) > in_w){
                    CAM_FUNC_ERR("cropping start + cropping size is out of range:0x%x_0x%x\n",\
                        (new_crop_x + this->dma_cfg.size.w),\
                        in_w);
                    return -1;
                }
#endif

#if DMA_CON
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_IMGO_CON,0x80000180); // ultra-
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_IMGO_CON2,0x00600060);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_IMGO_CON3,0x00600060);
#endif

                //
                fh_imgo.Header_Enque(Header_IMGO::E_CROP_START,this->m_pDrv,((this->dma_cfg.crop.y << 16) | new_crop_x));
                fh_imgo.Header_Enque(Header_IMGO::E_CROP_SIZE,this->m_pDrv,((this->dma_cfg.crop.h << 16) | this->dma_cfg.crop.w));

            }
            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_STRIDE,this->dma_cfg.size.stride);//stride;

#if DMA_CON
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_RRZO_CON,0x80000180); // ultra-
            //CAM_WRITE_REG(this->m_pIspDrvShell->m_pPhyIspDrv_bak,CAM_IMGI_SLOW_DOWN,0x0c000000); // ultra-
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_RRZO_CON2,0x00200020);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_RRZO_CON3,0x00200020);
#endif
            //  We use RRZ to corp, thus almost the RRZO crop width & height is 0
            //CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_CROP, (this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size));
            if(this->dma_cfg.crop.y || this->dma_cfg.crop.x)
                CAM_FUNC_ERR("rrzo support no cropping , replace dmao cropping start by 0\n");


            break;
        case CAM_DMA_UFEO:
            CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_STRIDE,this->dma_cfg.size.xsize);
            //CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_UFE_CON,0x00000001);  //If set this, UFG will not compress, means that compress ratio = 100%
#if DMA_CON
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_UFEO_CON,0x80000020);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_UFEO_CON2,0x00100010);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_UFEO_CON3,0x00100010);
#endif

            break;
        case CAM_DMA_PDO:
            {
                Header_PDO fh_pdo;

                fh_pdo.Header_Enque(Header_PDO::E_STRIDE,this->m_pDrv,this->dma_cfg.size.stride);

                CAM_WRITE_REG(this->m_pDrv,CAM_PDO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_PDO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_PDO_STRIDE,this->dma_cfg.size.stride);
                if((this->dma_cfg.size.stride % 16) != 0){
                    CAM_FUNC_ERR("PDO stride must be 16-alignment\n");
                }

#if DMA_CON
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_PDO_CON,0x80000040);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_PDO_CON2,0x00200020);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_PDO_CON3,0x00200020);
#endif
            }
            break;
        default:
            break;
    }

    CAM_FUNC_DBG("DMAO_B::_config-[%s]:size(%d,%d,%d,%d), pixel_byte(%d),cropW(%d,%d,%d,%d),format(0x%x),fps(0x%x),cropX_inbussize(%d), dbn/bin(%d/%d)", \
    this->name_Str(), \
    this->dma_cfg.size.w, \
    this->dma_cfg.size.h, \
    this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
    this->dma_cfg.pixel_byte, \
    this->dma_cfg.crop.x, \
    this->dma_cfg.crop.y,\
    this->dma_cfg.crop.w,\
    this->dma_cfg.crop.h,\
    this->dma_cfg.format,\
    this->m_fps,\
    GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size),\
    dbn_en, bin_en);

    //CAM_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}



MINT32 DMAO_B::_enable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)pParam;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAO_B::_enable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);


    switch(this->id())
    {
        case UNI_DMA_EISO:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,EISO_A_EN,1);

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_EISO,1);

            break;
        case UNI_DMA_FLKO:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,FLKO_A_EN,1);

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_FLKO,1);

            break;
        case UNI_DMA_RSSO:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,RSSO_A_EN,1);

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RSSO_A,1);

            break;
        case CAM_DMA_AFO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AFO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AFO,1);
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AAO_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AAO,1);

            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LCSO_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_LCSO,1);

            break;
        case CAM_DMA_IMGO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_IMGO,1);

            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RRZO,1);

            break;
        case CAM_DMA_UFEO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_SPECIAL_FUN_EN,UFOG_RRZO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_UFEO,1);

            break;
        case CAM_DMA_PDO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_PDO,1);

            break;
        default:
            CAM_FUNC_ERR("un-support DMAO:%s\n",this->name_Str());
            return -1;
            break;
    }
    return 0;
}

MINT32 DMAO_B::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAO_B::_disable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id())
    {
        case UNI_DMA_EISO:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,EISO_A_EN,0);

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_EISO,0);

            break;
        case UNI_DMA_FLKO:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,FLKO_A_EN,0);

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_FLKO,0);

            break;
        case UNI_DMA_RSSO:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_DMA_EN,RSSO_A_EN,0);

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RSSO_A,0);

            break;
        case CAM_DMA_AFO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AFO_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AFO,0);

            break;
        case CAM_DMA_AAO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AAO_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AAO,0);

            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LCSO_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_LCSO,0);

            break;
        case CAM_DMA_IMGO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_IMGO,0);

            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RRZO,0);

            break;
        case CAM_DMA_UFEO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_SPECIAL_FUN_EN,UFOG_RRZO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_UFEO,0);

            break;
        case CAM_DMA_PDO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_PDO,0);

            break;
        default:
            CAM_FUNC_ERR("un-support DMAO:%s\n",this->name_Str());
            return -1;
            break;
    }
    return 0;
}

/**
ba descirptor is removed into addDescritpor() because of BA descriptor & other descriptor using different CQ
*/
MINT32 DMAO_B::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("DMAO_B::_write2CQ+[%s]:cq(0x%x),page(0x%x)\n", \
        this->name_Str(), \
        cq,page);

     switch(this->id())
     {
        case UNI_DMA_EISO:
            this->m_pDrv->cqAddModule(UNI_DMA_EISO_BA);
            this->m_pDrv->cqAddModule(UNI_DMA_EISO_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_EISO_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_EISO_SPARE_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_EN_);
            break;
        case UNI_DMA_FLKO:
            this->m_pDrv->cqAddModule(UNI_DMA_FLKO_);
            //this->m_pDrv->cqAddModule(UNI_DMA_FH_FLKO_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_EN_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_FLKO_SPARE_MAGIC_);
            break;
        case UNI_DMA_RSSO:
            this->m_pDrv->cqAddModule(UNI_DMA_RSSO_A_BA);
            this->m_pDrv->cqAddModule(UNI_DMA_RSSO_A_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_RSSO_A_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_RSSO_A_SPARE_);
            this->m_pDrv->cqAddModule(UNI_DMA_FH_EN_);
            break;
        case CAM_DMA_IMGO:
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_);
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_CROP_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
            break;
        case CAM_DMA_RRZO:
            this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_RRZO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_RRZO_CROP_);   rrzo crop is not supported, using rrz crop
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);
            break;
        case CAM_DMA_UFEO:
            this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_UFEO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_SPARE_);
            break;
        case CAM_DMA_AFO:
            this->m_pDrv->cqAddModule(CAM_DMA_AFO_);
            this->m_pDrv->cqAddModule(CAM_DMA_AFO_BA_OFST);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_AFO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_AFO_SPARE_MAGIC_);
            break;
        case CAM_DMA_AAO:
            this->m_pDrv->cqAddModule(CAM_DMA_AAO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_AAO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_AAO_SPARE_MAGIC_);
            break;
        case CAM_DMA_LCSO:
            this->m_pDrv->cqAddModule(CAM_DMA_LCSO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_LCSO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_SPARE_);
            break;
        case CAM_DMA_PDO:
            this->m_pDrv->cqAddModule(CAM_DMA_PDO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_PDO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_PDO_SPARE_MAGIC_);
            break;
        default:
            CAM_FUNC_ERR(" NOT push to CQ ");
            break;
    }

    return 0;
}


MINT32 DMAO_B::setBaseAddr(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("DMAO_B::setBaseAddr+[%s]:cq(0x%x),page(0x%x),pa(0x%08X_0x%08X),ofst(0x%X)", \
        this->name_Str(), \
        cq,page,\
        this->dma_cfg.memBuf.base_pAddr, this->Header_Addr,\
        this->dma_cfg.memBuf.ofst_addr);
    if(this->dma_cfg.memBuf.ofst_addr != 0){
        CAM_FUNC_ERR("support no damo offsetaddress,bypass offset setting\n");
    }

    switch(this->id())
    {
        case UNI_DMA_EISO:
            {
                Header_EISO fh_eiso;
                fh_eiso.Header_Enque(Header_EISO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                UNI_WRITE_REG(this->m_pDrv,CAM_UNI_EISO_OFST_ADDR,0x0);
                UNI_WRITE_REG(this->m_pDrv,CAM_UNI_EISO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                UNI_WRITE_REG(this->m_pDrv,CAM_UNI_EISO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case UNI_DMA_FLKO:
            {
                Header_FLKO fh_flko;
                CQ_RingBuf_ST bufctrl;
                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,SUB_RATIO))
                    bufctrl.ctrl = BUF_CTRL_ENQUE_HIGH_SPEED;
                else
                    bufctrl.ctrl = BUF_CTRL_ENQUE;
                bufctrl.dma_PA = this->dma_cfg.memBuf.base_pAddr;
                bufctrl.dma_FH_PA = this->Header_Addr;
                //
                if(this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE){
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%x]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                this->m_pDrv->cqRingBuf(&bufctrl);
                fh_flko.Header_Enque(Header_FLKO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        case UNI_DMA_RSSO:
            {
                Header_RSSO fh_rsso;
                fh_rsso.Header_Enque(Header_RSSO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RSSO_A_OFST_ADDR,0x0);
                UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RSSO_A_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                UNI_WRITE_REG(this->m_pDrv,CAM_UNI_RSSO_A_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_AFO:
            {
                Header_AFO  fh_afo;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,SUB_RATIO))
                    bufctrl.ctrl = BUF_CTRL_ENQUE_HIGH_SPEED;
                else
                    bufctrl.ctrl = BUF_CTRL_ENQUE;
                bufctrl.dma_PA = this->dma_cfg.memBuf.base_pAddr;
                bufctrl.dma_FH_PA = this->Header_Addr;
                //
                if(this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE){
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%x]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                this->m_pDrv->cqRingBuf(&bufctrl);
                fh_afo.Header_Enque(Header_AFO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        case CAM_DMA_AAO:
            {
                Header_AAO  fh_aao;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,SUB_RATIO))
                    bufctrl.ctrl = BUF_CTRL_ENQUE_HIGH_SPEED;
                else
                    bufctrl.ctrl = BUF_CTRL_ENQUE;
                bufctrl.dma_PA = this->dma_cfg.memBuf.base_pAddr;
                bufctrl.dma_FH_PA = this->Header_Addr;
                //
                if(this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE){
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%x]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                this->m_pDrv->cqRingBuf(&bufctrl);
                fh_aao.Header_Enque(Header_AAO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        case CAM_DMA_LCSO:
            {
                Header_LCSO fh_lcso;
                fh_lcso.Header_Enque(Header_LCSO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_IMGO:
            {
                Header_IMGO fh_imgo;
                fh_imgo.Header_Enque(Header_IMGO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_RRZO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_OFST_ADDR,0x0);

                CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_UFEO:
            {
                Header_UFEO fh_ufeo;
                fh_ufeo.Header_Enque(Header_UFEO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_OFST_ADDR,0x0);

                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_PDO:
            {
                Header_PDO  fh_pdo;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,SUB_RATIO))
                    bufctrl.ctrl = BUF_CTRL_ENQUE_HIGH_SPEED;
                else
                    bufctrl.ctrl = BUF_CTRL_ENQUE;
                bufctrl.dma_PA = this->dma_cfg.memBuf.base_pAddr;
                bufctrl.dma_FH_PA = this->Header_Addr;
                //
                if(this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE){
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%x]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                this->m_pDrv->cqRingBuf(&bufctrl);
                fh_pdo.Header_Enque(Header_PDO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        default:
            break;
    }

    //CAM_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}


MUINT32 DMAO_B::BusSizeCal(void)
{
    CAM_REG_CTL_SEL ctl_sel;
    CAM_REG_CTL_FMT_SEL fmt_sel;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    ctl_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL);
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_FMT_SEL);
    switch(fmt_sel.Bits.IMGO_FMT){
        case 3://yuv
        case 7://jpg
            if(fmt_sel.Bits.PIX_BUS_DMXI == 2)
                return 3;
            else
                return 1;
            break;
        default://bayer
            switch(ctl_sel.Bits.UFE_SEL){
                case 1: //UFE
                    return 7;
                    break;
                case 2: //TG
                    switch(fmt_sel.Bits.PIX_BUS_DMXI){
                        case 1:     return 3; break;
                        case 2:     return 7; break;
                        default:    return 1; break;
                    }
                    break;
                default:    //PAK
                    switch(ctl_sel.Bits.IMG_SEL){
                        case 0:
                            switch(fmt_sel.Bits.PIX_BUS_BMXO){
                                case 1:     return 3; break;
                                case 2:     return 7; break;
                                default:    return 1; break;
                            }
                            break;
                        case 1:
                            switch(fmt_sel.Bits.PIX_BUS_DMXI){
                                case 1:     return 3; break;
                                case 2:     return 7; break;
                                default:    return 1; break;
                            }
                            break;
                        case 2:
                            return 1;
                            break;
                    }
                    break;
            }
            break;
    }
    CAM_FUNC_ERR("BusSizeCal fail\n");
    return 0;
}



