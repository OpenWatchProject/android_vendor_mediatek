/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "ifunc_cam_dmax"

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
#include "Cam_Notify_datatype.h"
#include "tuning_drv.h"
#include "cam_capibility.h"


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

#define MAP_IFUNC_PIXMODE(src)({ \
    E_PIX _pixmode = _1_pix_; \
    switch(src){ \
        case ePixMode_1: _pixmode = _1_pix_; break; \
        case ePixMode_2: _pixmode = _2_pix_; break; \
        case ePixMode_4: _pixmode = _4_pix_; break; \
        default: CAM_FUNC_INF("unsupport pixel mode (%d)",src); break; \
    } \
    _pixmode; \
}) \

#define MAP_CAM_PIXMODE(src)({ \
    E_CamPixelMode _pixmode = ePixMode_1; \
    switch(src){ \
        case _1_pix_: _pixmode = ePixMode_1; break; \
        case _2_pix_: _pixmode = ePixMode_2; break; \
        case _4_pix_: _pixmode = ePixMode_4; break; \
        default: CAM_FUNC_INF("unsupport pixel mode (%d)",src); break; \
    } \
    _pixmode; \
}) \

//
/*/////////////////////////////////////////////////////////////////////////////
    DMAI_B
  /////////////////////////////////////////////////////////////////////////////*/

DMAI_B::DMAI_B()
{
    m_pUniDrv = NULL;
    m_pDrv = NULL;
    m_updatecnt = 0;
    //m_FIFO.clear();
}

MBOOL DMAI_B::BW_Meter(const Sig_Profile& prof,V_DMA_BW* pOut)
{
    DMA_BW fifo;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    if((this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv) == NULL){
        CAM_FUNC_ERR("drv obj can't be NULL\n");
        return MFALSE;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::BW_Meter:cq(0x%x),page(0x%x)\n",cq,page);
    //
#if 0
    if(this->m_FIFO.size() == 0){
        IspDrv* ptr;
        ptr = this->m_pDrv->getPhyObj();

        //lsci
        fifo.portID = _lsci_;
        fifo.ThruPut = CAM_READ_BITS(ptr,CAM_LSCI_CON,FIFO_SIZE);
        fifo.ThruPut = fifo.ThruPut<<4;
        this->m_FIFO.push_back(fifo);
        //bpci
        fifo.portID = _bpci_;
        fifo.ThruPut = CAM_READ_BITS(ptr,CAM_BPCI_CON,FIFO_SIZE);
        fifo.ThruPut = fifo.ThruPut<<4;
        this->m_FIFO.push_back(fifo);
        //pdi
        fifo.portID = _pdi_;
        fifo.ThruPut = CAM_READ_BITS(ptr,CAM_PDI_CON,FIFO_SIZE);
        fifo.ThruPut = fifo.ThruPut<<4;
        this->m_FIFO.push_back(fifo);
    }
#endif
    pOut->clear();
    if((prof.lineT == 0) || (prof.frameT == 0)){
        CAM_FUNC_ERR("lineT | frameT can't be 0\n");
        return MFALSE;
    }
    {
        CAM_REG_CTL_DMA_EN dma_en;
        dma_en.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_DMA_EN);

        /* bit 7 */
        pOut->push_back(BW_l(_bpci_,dma_en.Bits.BPCI_EN,CAM_BPCI_XSIZE,CAM_BPCI_YSIZE));
        /* bit 8 */
        pOut->push_back(BW_l(_lsci_,dma_en.Bits.LSCI_EN,CAM_LSCI_XSIZE,CAM_LSCI_YSIZE));
        /* bit 9 */
        //CACI, not used
        /* bit 11 */
        //LSC3I, not used
        /* bit 12 */
        pOut->push_back(BW_a(_pdi_,dma_en.Bits.PDI_EN,CAM_PDI_XSIZE,CAM_PDI_YSIZE));

    }

    return MTRUE;
}

MINT32 DMAI_B::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_config+ [%s]:cq(0x%x),page(0x%x),pa(0x%" PRIXPTR "),ofst(0x%08X),size(%ld,%ld,%ld,%ld),pixel_byte(%d),crop(%d,%d)\n",this->name_Str(),cq,page,\
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
        case CAM_DMA_CQ12I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR12_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_STRIDE,this->dma_cfg.size.stride);

            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,CAM_BPCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_STRIDE,this->dma_cfg.size.stride);//stride;

            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,CAM_LSCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

            break;
        case CAM_DMA_PDI:
            CAM_WRITE_REG(this->m_pDrv,CAM_PDI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_PDI_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,CAM_PDI_STRIDE,this->dma_cfg.size.stride);//stride;

            CAM_WRITE_REG(this->m_pDrv,CAM_PDI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,CAM_PDI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

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

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_enable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id()){
        case CAM_DMA_CQ0I:
            if(pParam == NULL){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,1);
            }
            else{

#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR0_CTL,CQ_THR0_EN,1);
#endif
            }
            break;
        case CAM_DMA_CQ1I:
            if(pParam == NULL){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_CTL,CQ_THR1_EN,1);
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR1_CTL,CQ_THR1_EN,1);
#endif
            }
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
            if(pParam == NULL){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_CTL,CQ_THR10_EN,1);
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR10_CTL,CQ_THR10_EN,1);
#endif
            }
            break;
        case CAM_DMA_CQ11I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR11_CTL,CQ_THR11_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,I2C_CQ_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CQ_SEL,0);//always imm mode
            break;
        case CAM_DMA_CQ12I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR12_CTL,CQ_THR12_EN,1);
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,BPCI_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,BPCI_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LSCI_EN,1);
             CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,LSCI_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_PDI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDI_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,PDI_LAST_ULTRA_EN ,1);
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

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_disable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id()){
        case CAM_DMA_CQ0I:
            if(pParam == NULL){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,0);
                //reset update cnt when disable
                this->m_updatecnt = 0;
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR0_CTL,CQ_THR0_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_START,CQ_THR0_START,0);
#endif
            }
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
        case CAM_DMA_CQ12I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR12_CTL,CQ_THR12_EN,0);
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,BPCI_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,BPCI_LAST_ULTRA_EN ,0);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LSCI_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,LSCI_LAST_ULTRA_EN ,0);
            break;
        case CAM_DMA_PDI:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDI_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,PDI_LAST_ULTRA_EN ,0);
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
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_write2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(this->id())
    {
        case CAM_DMA_BPCI:
            this->m_pDrv->cqAddModule(CAM_DMA_BPCI_);
            break;
        case CAM_DMA_LSCI:
            this->m_pDrv->cqAddModule(CAM_DMA_LSCI_);
            break;
        case CAM_DMA_PDI:
            this->m_pDrv->cqAddModule(CAM_DMA_PDI_);
            break;
        case UNI_DMA_RAWI:
            this->m_pDrv->cqAddModule(UNI_DMA_RAWI_);
            break;
        default:
            CAM_FUNC_DBG("DMAI: NOT push to CQ(%d)\n",this->id());
            break;
    }

    //
    CAM_FUNC_DBG("-\n");
    return 0;
}

uintptr_t    DMA_CQ0::Addr[PHY_CAM] = {0,};
ISP_DRV_CAM* DMA_CQ0::pDrv[PHY_CAM] = {0,};
unsigned int twinAddr[3] = {0, 0, 0};

MBOOL DMA_CQ0::Init(void)
{
    memset((void*)this->Addr, 0, sizeof(this->Addr));
    memset((void*)this->pDrv, 0, sizeof(this->pDrv));

    return 0;
}

MINT32 DMA_CQ0::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ0::_config+ [%s]:cq(0x%x),page(0x%x),pa(0x%" PRIXPTR "),ofst(0x%08X),size(%ld,%ld,%ld,%ld),pixel_byte(%d),crop(%d,%d)\n",this->name_Str(),cq,page,\
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
            {
                capibility CamInfo;
                MUINT32 cq0_data[CAM_MAX][2];
                if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN)){
                        multiRAWConfig.master_module = CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,MASTER_MODULE);
                        multiRAWConfig.slave_cam_num = CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM);
                        multiRAWConfig.twin_module = CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE);

                        pDrv[this->m_hwModule] = this->m_pDrv;
                        Addr[this->m_hwModule] = this->dma_cfg.memBuf.base_pAddr;

                        switch (multiRAWConfig.slave_cam_num) {
                        case 1: /* 2 RAWS to 1 tg for static twin mode */
                            if (multiRAWConfig.master_module == CAM_A &&
                                multiRAWConfig.twin_module == CAM_C) {
                                if ((Addr[CAM_A] != 0) &&
                                    (Addr[CAM_C] != 0)) {

                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_CD_DONE_SEL, 0x10000);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x1);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.twin_module]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_C]);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_A]);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x0);
                                    Addr[CAM_A] = Addr[CAM_C] = 0;
                                    pDrv[CAM_A] = pDrv[CAM_C] = 0;
                                }
                            } else if (multiRAWConfig.master_module == CAM_B &&
                                    multiRAWConfig.twin_module == CAM_C) {
                                if ((Addr[CAM_B] != 0) &&
                                    (Addr[CAM_C] != 0)) {

                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_CD_DONE_SEL, 0x10000);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x1);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.twin_module]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_C]);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_B]);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x0);
                                    Addr[CAM_B] = Addr[CAM_C] = 0;
                                    pDrv[CAM_B] = pDrv[CAM_C] = 0;
                                }
                            } else {
                                if ((Addr[CAM_A] != 0) &&
                                    (Addr[CAM_B] != 0)) {

                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_CD_DONE_SEL, 0x10000);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x1);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.twin_module]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_B]);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_A]);
                                    CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x0);
                                    Addr[CAM_A] = Addr[CAM_B] = 0;
                                    pDrv[CAM_A] = pDrv[CAM_B] = 0;
                                }
                            }
                        break;
                        case 2: /* 3 RAWS to 1 tg for static twin mode */
                            if ((Addr[CAM_A] != 0) && (Addr[CAM_B] != 0) && (Addr[CAM_C] != 0)) {
                                CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_CD_DONE_SEL, 0x10000);
                                CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x1);
                                CAM_WRITE_REG(pDrv[CAM_C]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_C]);
                                CAM_WRITE_REG(pDrv[CAM_B]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_B]);
                                CAM_WRITE_REG(pDrv[CAM_A]->getPhyObj(), CAM_CQ_THR0_BASEADDR, Addr[CAM_A]);
                                CAM_WRITE_REG(pDrv[multiRAWConfig.master_module]->getPhyObj(), CAM_CTL_UNI_DONE_SEL, 0x0);
                                Addr[CAM_A] = Addr[CAM_B] = Addr[CAM_C] = 0;
                                pDrv[CAM_A] = pDrv[CAM_B] = pDrv[CAM_C] = 0;
                            }
                        break;
                        };

                        //this->m_pDrv->setDeviceInfo(_SET_CQ_SW_PATCH,(MUINT8*)&multiRAWConfig);
                    }
                    else{
                        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
                    }
                }
                else
                    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
                /* + It's for Sylvia_WAM_CQ_ERR */
                cq0_data[CAM_A][0] = cq0_data[CAM_B][0] = cq0_data[CAM_C][0] = 0;
                cq0_data[CAM_A][1] = cq0_data[CAM_B][1] = cq0_data[CAM_C][1] = 0;

                if (CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN)) {
                    twinAddr[this->m_hwModule] = (unsigned int)this->dma_cfg.memBuf.base_pAddr;
                    if (CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM) == 1) {

                        if ((twinAddr[CAM_A] != 0) && (twinAddr[CAM_C] != 0)) {
                            cq0_data[CAM_A][0] =  ISP_CAM_A_IDX;
                            cq0_data[CAM_C][0] =  ISP_CAM_C_IDX;
                            cq0_data[CAM_A][1] = twinAddr[CAM_A];
                            cq0_data[CAM_C][1] = twinAddr[CAM_C];

                            this->m_pDrv->setDeviceInfo(_NOTE_CQTHR0_BASE,(MUINT8 *)&cq0_data);
                            //CAM_FUNC_INF("cq data(%d,0x%x)(%d,0x%x)", cq0_data[CAM_A][0], cq0_data[CAM_A][1],
                            //    cq0_data[CAM_C][0], cq0_data[CAM_C][1]);
                            twinAddr[CAM_A] = twinAddr[CAM_C] = 0;
                        }
                        if ((twinAddr[CAM_C] != 0) && (twinAddr[CAM_B] != 0)) {
                            cq0_data[CAM_C][0] =  ISP_CAM_C_IDX;
                            cq0_data[CAM_B][0] =  ISP_CAM_B_IDX;
                            cq0_data[CAM_C][1] = twinAddr[CAM_C];
                            cq0_data[CAM_B][1] = twinAddr[CAM_B];

                            this->m_pDrv->setDeviceInfo(_NOTE_CQTHR0_BASE,(MUINT8 *)&cq0_data);
                            //CAM_FUNC_INF("cq data(%d,0x%x)(%d,0x%x)", cq0_data[CAM_C][0], cq0_data[CAM_C][1],
                            //    cq0_data[CAM_B][0], cq0_data[CAM_B][1]);
                            twinAddr[CAM_C] = twinAddr[CAM_B] = 0;
                        }
                    } else if (CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM) == 2) {
                        if ((twinAddr[CAM_A] != 0) && (twinAddr[CAM_B] != 0) && (twinAddr[CAM_C] != 0)) {
                            cq0_data[CAM_A][0] =  ISP_CAM_A_IDX;
                            cq0_data[CAM_B][0] =  ISP_CAM_B_IDX;
                            cq0_data[CAM_C][0] =  ISP_CAM_C_IDX;
                            cq0_data[CAM_A][1] = twinAddr[CAM_A];
                            cq0_data[CAM_B][1] = twinAddr[CAM_B];
                            cq0_data[CAM_C][1] = twinAddr[CAM_C];
                            this->m_pDrv->setDeviceInfo(_NOTE_CQTHR0_BASE,(MUINT8 *)&cq0_data);
                            //CAM_FUNC_INF("cq data(%d,0x%x)(%d,0x%x)(%d,0x%x)", cq0_data[CAM_A][0], cq0_data[CAM_A][1],
                            //    cq0_data[CAM_B][0], cq0_data[CAM_B][1],cq0_data[CAM_C][0], cq0_data[CAM_C][1]);
                            twinAddr[CAM_A] = twinAddr[CAM_B] = twinAddr[CAM_C] = 0;
                        }
                    }
                } else {
                    if (this->m_hwModule == CAM_A)
                        cq0_data[this->m_hwModule][0] =  ISP_CAM_A_IDX;
                    else if (this->m_hwModule == CAM_B)
                        cq0_data[this->m_hwModule][0] =  ISP_CAM_B_IDX;
                    else if (this->m_hwModule == CAM_C)
                        cq0_data[this->m_hwModule][0] =  ISP_CAM_C_IDX;

                    cq0_data[this->m_hwModule][1] =  (unsigned int)this->dma_cfg.memBuf.base_pAddr;

                    this->m_pDrv->setDeviceInfo(_NOTE_CQTHR0_BASE,(MUINT8 *)&cq0_data);
                    //CAM_FUNC_INF("cq data(%d,0x%x)", cq0_data[this->m_hwModule][0], cq0_data[this->m_hwModule][1]);

                }
                /* - It's for Sylvia_WAM_CQ_ERR */
            }
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


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


    //when subsample function enable, using cq inner reg
    //otherwise, using outter reg
    if(this->m_bSubsample == MTRUE){
        this->m_pDrv->cqAddModule(CAM_CQ_THRE0_ADDR_CQONLY);
#if TWIN_SW_P1_DONE_WROK_AROUND
        this->m_pDrv->cqAddModule(CAM_CQ_THRE0_ADDR_);
#endif
    }
    else{
        this->m_pDrv->cqAddModule(CAM_CQ_THRE0_ADDR_);
    }


    //
    CAM_FUNC_DBG("-\n");
    return 0;
}


MINT32 DMA_CQ0::IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("IO2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(op){
        case _cq_add_:
            this->m_pDrv->cqAddModule(CAM_CQ0_EN_, slaveCam);
            this->m_pDrv->cqAddModule(CAM_CQ0_TRIG_, slaveCam);
            this->m_pDrv->cqAddModule(TWIN_CQ0_BASEADDRESS, slaveCam);
            break;
        case _cq_delete_:
            this->m_pDrv->cqDelModule(CAM_CQ0_EN_);
            this->m_pDrv->cqDelModule(CAM_CQ0_TRIG_);
            this->m_pDrv->cqDelModule(TWIN_CQ0_BASEADDRESS);
            break;
        default:
            break;
    }

    return 0;
}


MBOOL DMA_CQ0::setCQTriggerMode(E_MODE mode,void* ptr)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    if(ptr != NULL){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR0_CTL,CQ_THR0_MODE,mode);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_MODE,mode);
    }

    return MTRUE;
}

MBOOL DMA_CQ0::setBaseAddr_byCQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ0::setBaseAddr_byCQ:+ addr = 0x%" PRIXPTR "\n",this->dma_cfg.memBuf.base_pAddr);

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


MBOOL DMA_CQ0::setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("%s::setSlaveAddr_byCQ:+ addr = 0x%" PRIXPTR "\n", this->name_Str(), this->dma_cfg.memBuf.base_pAddr);

    this->m_pDrv->cqAddModule(TWIN_CQ0_BASEADDRESS, slaveCam);
    CAM_WRITE_REG(this->m_pDrv,TWIN_CQ_THRE0_ADDR,this->dma_cfg.memBuf.base_pAddr);

    return MTRUE;
}

MBOOL DMA_CQ0::TrigCQ(MBOOL bWait)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(bWait == MTRUE){
        CAM_FUNC_DBG("%s::TrigCQ:+ current PA:0x%x\n", this->name_Str(), CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_BASEADDR));

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR0_START,1);
        usleep(300);
        while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR0_START) == 1){
            //wait cq loading done
            usleep(300);
            if(loop++ > 10){
                CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
                break;
            }
        }
    }
    else{
#if TWIN_CQ_SW_WORKAROUND
        //this virtual register is mapping to slave cam's pa register.
        //mapping method is in isp_drv_cam
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_START,CQ_THR0_START,1);
#endif
    }
    return MTRUE;
}

MBOOL DMA_CQ0::setLoadMode(E_LoadMode mode, MBOOL bPhysical)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 tmp;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if (bPhysical) {
        switch(mode){
        case _inner_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
            break;
        case _outer_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
            break;
        default:
            CAM_FUNC_ERR("unsupported mode:[%s]:cq(0x%x),page(0x%x),mode(0x%x)\n",this->name_Str(),cq,page,mode);
            return MFALSE;
        }
    }
    else {
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
    tmp = CAM_READ_BITS(pSrcObj,CAM_FBC_LMVO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,LMVO_FIFO_FULL_DROP,tmp);

    //rsso
    tmp = CAM_READ_BITS(pSrcObj,CAM_FBC_RSSO_CTL1,FBC_EN);
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

MBOOL DMA_CQ1::setCQTriggerMode(E_MODE mode,void* ptr)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    if(ptr != NULL){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR1_CTL,CQ_THR1_MODE,mode);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_CTL,CQ_THR1_MODE,mode);
    }

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
    arg1;

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


MBOOL DMA_CQ1::TrigCQ(MBOOL bWait)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(bWait == MTRUE){
        CAM_FUNC_DBG("%s::TrigCQ:+ current PA:0x%x\n", this->name_Str(), CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR1_BASEADDR));

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR1_START,1);
        usleep(300);
        while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR1_START) == 1){
            //wait cq loading done
            usleep(300);
            if(loop++ > 10){
                CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
                break;
            }
        }
    }
    else{
#if TWIN_CQ_SW_WORKAROUND
        //this virtual register is mapping to slave cam's pa register.
        //mapping method is in isp_drv_cam
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_START,CQ_THR1_START,1);
#endif
    }

    return MTRUE;
}

MBOOL DMA_CQ1::setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("%s::setSlaveAddr_byCQ:+ addr = 0x%" PRIXPTR "\n", this->name_Str(), this->dma_cfg.memBuf.base_pAddr);

    this->m_pDrv->cqAddModule(TWIN_CQ1_BASEADDRESS, slaveCam);
    CAM_WRITE_REG(this->m_pDrv,TWIN_CQ_THRE1_ADDR,this->dma_cfg.memBuf.base_pAddr);

    return MTRUE;
}

MINT32 DMA_CQ1::IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("IO2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(op){
        case _cq_add_:
            this->m_pDrv->cqAddModule(CAM_CQ1_EN_, slaveCam);
            this->m_pDrv->cqAddModule(CAM_CQ1_TRIG_, slaveCam);
            this->m_pDrv->cqAddModule(TWIN_CQ1_BASEADDRESS, slaveCam);
            break;
        case _cq_delete_:
            this->m_pDrv->cqDelModule(CAM_CQ1_EN_);
            this->m_pDrv->cqDelModule(CAM_CQ1_TRIG_);
            this->m_pDrv->cqDelModule(TWIN_CQ1_BASEADDRESS);
            break;
        default:
            break;
    }

    //
    CAM_FUNC_DBG("-\n");
    return 0;
}

MBOOL DMA_CQ4::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR4_CTL,CQ_THR4_MODE,mode);

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
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ5::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR5_CTL,CQ_THR5_MODE,mode);

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
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ7::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR7_CTL,CQ_THR7_MODE,mode);

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
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ8::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR8_CTL,CQ_THR8_MODE,mode);

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
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
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

    return 0;
}


MBOOL DMA_CQ10::setCQTriggerMode(E_MODE mode,void* ptr)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    if(ptr != NULL){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CQ_THR10_CTL,CQ_THR10_MODE,mode);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_CTL,CQ_THR10_MODE,mode);
    }

    return MTRUE;
}

MBOOL DMA_CQ10::setCQContent(MUINTPTR arg1,MUINTPTR arg2)
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
    CAM_FUNC_WRN("DMA_CQ10::cfg:+ \n");

    //
    pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * ptr->size());
    if (NULL == pReg) {
        CAM_FUNC_ERR("DMA_CQ10::setCQContent: alloc buf fail\n");
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


MBOOL DMA_CQ10::TrigCQ(MBOOL bWait)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(bWait == MTRUE){
        CAM_FUNC_DBG("%s::TrigCQ:+ current PA:0x%x\n", this->name_Str(), CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR10_BASEADDR));

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR10_START,1);
        usleep(300);
        while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR10_START) == 1){
            //wait cq loading done
            usleep(300);
            if(loop++ > 10){
                CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
                break;
            }
        }
    }
    else{
#if TWIN_CQ_SW_WORKAROUND
        //this virtual register is mapping to slave cam's pa register.
        //mapping method is in isp_drv_cam
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_START,CQ_THR10_START,1);
#endif
    }

    return MTRUE;
}

MBOOL DMA_CQ10::setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("%s::setSlaveAddr_byCQ:+ addr = 0x%" PRIXPTR "\n", this->name_Str(), this->dma_cfg.memBuf.base_pAddr);

    this->m_pDrv->cqAddModule(TWIN_CQ10_BASEADDRESS, slaveCam);
    CAM_WRITE_REG(this->m_pDrv,TWIN_CQ_THRE10_ADDR,this->dma_cfg.memBuf.base_pAddr);

    return MTRUE;
}

MINT32 DMA_CQ10::IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("IO2CQ+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    switch(op){
        case _cq_add_:
            this->m_pDrv->cqAddModule(CAM_CQ10_EN_, slaveCam);
            this->m_pDrv->cqAddModule(CAM_CQ10_TRIG_, slaveCam);
            this->m_pDrv->cqAddModule(TWIN_CQ10_BASEADDRESS, slaveCam);
            break;
        case _cq_delete_:
            this->m_pDrv->cqDelModule(CAM_CQ10_EN_);
            this->m_pDrv->cqDelModule(CAM_CQ10_TRIG_);
            this->m_pDrv->cqDelModule(TWIN_CQ10_BASEADDRESS);
            break;
        default:
            break;

    }

    //
    CAM_FUNC_DBG("-\n");
    return 0;
}


MBOOL DMA_CQ11::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR11_CTL,CQ_THR11_MODE,mode);

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
    if((this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE)) ||
        (this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TRIPLE_MODULE)))
    {
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN)){
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
    CAM_FUNC_DBG("DMA_CQ11::cfg:+ timing:0x%" PRIXPTR "\n",arg1);

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


MBOOL DMA_CQ12::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_THR12_CTL,CQ_THR12_MODE,mode);

    return MTRUE;
}

MBOOL DMA_CQ12::TrigCQ(void)
{
    MUINT32 loop=0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ12::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR12_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR12_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_START,CQ_THR12_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
        }
    }
    return MTRUE;
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
    //m_FIFO.clear();
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
        case CAM_DMA_FLKO:
        case CAM_DMA_AFO:
        case CAM_DMA_AAO:
        case CAM_DMA_PDO:
        case CAM_DMA_PSO:
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

MBOOL DMAO_B::BW_Meter(const Sig_Profile& prof,V_DMA_BW* pOut)
{
    DMA_BW fifo;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    if((this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv) == NULL){
        CAM_FUNC_ERR("drv obj can't be NULL\n");
        return MFALSE;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAO_B::BW_Meter:cq(0x%x),page(0x%x)\n",cq,page);
#if 0
    //
    if(this->m_FIFO.size() == 0){
        IspDrv* ptr;
        #define FIFO_a(port,regname) {\
            fifo.portID = port;\
            fifo.ThruPut = CAM_READ_BITS(ptr,regname,FIFO_SIZE);\
            fifo.ThruPut = fifo.ThruPut<<4;\
            this->m_FIFO.push_back(fifo);}

        ptr = this->m_pDrv->getPhyObj();

        //
        FIFO_a(_imgo_,CAM_IMGO_CON);
        FIFO_a(_rrzo_,CAM_RRZO_CON);
        FIFO_a(_ufeo_,CAM_UFEO_CON);
        FIFO_a(_aao_,CAM_AAO_CON);
        FIFO_a(_afo_,CAM_AFO_CON);
        FIFO_a(_lcso_,CAM_LCSO_CON);
        FIFO_a(_pdo_,CAM_PDO_CON);
        FIFO_a(_lmvo_,CAM_LMVO_CON);
        FIFO_a(_flko_,CAM_FLKO_CON);
        FIFO_a(_rsso_,CAM_RSSO_CON);
        FIFO_a(_pso_,CAM_PSO_CON);
        FIFO_a(_ufgo_,CAM_UFGO_CON);

        #undef FIFO_a
    }
#endif
    pOut->clear();
    if((prof.lineT == 0) || (prof.frameT == 0)){
        CAM_FUNC_ERR("lineT | frameT can't be 0\n");
        return MFALSE;
    }
    {
        CAM_REG_CTL_DMA_EN dma_en;
        dma_en.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_DMA_EN);
        /* bit 0 */
        pOut->push_back(BW_l(_imgo_,dma_en.Bits.IMGO_EN,CAM_IMGO_XSIZE,CAM_IMGO_YSIZE));
        /* bit 1 */
        pOut->push_back(BW_a(_ufeo_,dma_en.Bits.UFEO_EN,CAM_UFEO_XSIZE,CAM_UFEO_YSIZE));
        /* bit 2 */
        pOut->push_back(BW_a(_rrzo_,dma_en.Bits.RRZO_EN,CAM_RRZO_XSIZE,CAM_RRZO_YSIZE));
        /* bit 3 */
        pOut->push_back(BW_a(_afo_,dma_en.Bits.AFO_EN,CAM_AFO_XSIZE,CAM_AFO_YSIZE));
        /* bit 4 */
        pOut->push_back(BW_a(_lcso_,dma_en.Bits.LCSO_EN,CAM_LCSO_XSIZE,CAM_LCSO_YSIZE));
        /* bit 5 */
        pOut->push_back(BW_1D(_aao_,dma_en.Bits.AAO_EN,CAM_AAO_XSIZE,CAM_READ_BITS(this->m_pDrv,CAM_AWB_WIN_NUM,AWB_W_VNUM),AE_BIN_SIZE(this->m_pDrv)));
        /* bit 6 */
        pOut->push_back(BW_1D(_pso_,dma_en.Bits.PSO_EN,CAM_PSO_XSIZE,CAM_READ_BITS(this->m_pDrv,CAM_PS_AWB_WIN_NUM,AWB_W_VNUM),PS_BIN_SIZE(this->m_pDrv)));
        /* bit 10 */
        pOut->push_back(BW_a(_pdo_,dma_en.Bits.PDO_EN,CAM_PDO_XSIZE,CAM_PDO_YSIZE));
        /* bit 13 */
        pOut->push_back(BW_1D(_flko_,dma_en.Bits.FLKO_EN,CAM_FLKO_XSIZE,(CAM_READ_BITS(this->m_pDrv,CAM_FLK_A_NUM,FLK_NUM_Y)*CAM_READ_BITS(this->m_pDrv,CAM_FLK_A_SIZE,FLK_SIZE_Y)),0));
        /* bit 14 */
        pOut->push_back(BW_e(_lmvo_,dma_en.Bits.LMVO_EN,CAM_READ_REG(this->m_pDrv,CAM_LMVO_XSIZE)));
        /* bit 15 */
        pOut->push_back(BW_a(_rsso_,dma_en.Bits.RSSO_EN,CAM_RSSO_A_XSIZE,CAM_RSSO_A_YSIZE));
        /* bit 16 */
        pOut->push_back(BW_a(_ufgo_,dma_en.Bits.UFGO_EN,CAM_UFGO_XSIZE,CAM_UFGO_YSIZE));
    }

    return MTRUE;
}

MINT32 DMAO_B::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("DMAO_B::_config+[%s]:port(%d),cq(0x%x),page(0x%x),pa(0x%" PRIXPTR "_0x%08X),ofst(0x%X),w/h(%d_%d)\n", \
        this->name_Str(), this->id(),\
        cq,page,\
        this->dma_cfg.memBuf.base_pAddr, this->Header_Addr,\
        this->dma_cfg.memBuf.ofst_addr,this->dma_cfg.size.xsize,\
        this->dma_cfg.size.h);


    switch(this->id())
    {
        case CAM_DMA_LMVO:
            CAM_WRITE_REG(this->m_pDrv,CAM_LMVO_XSIZE,0xFF);
            CAM_WRITE_REG(this->m_pDrv,CAM_LMVO_YSIZE,0);
            CAM_WRITE_REG(this->m_pDrv,CAM_LMVO_STRIDE,0xFF + 1);

            break;
        case CAM_DMA_FLKO:
            CAM_WRITE_REG(this->m_pDrv,CAM_FLKO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_FLKO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_FLKO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_RSSO:
            {
                Header_RSSO fh_rsso;
                fh_rsso.Header_Enque(Header_RSSO::E_DST_SIZE, this->m_pDrv, ((this->dma_cfg.size.h << 16) | this->dma_cfg.size.xsize));
                CAM_WRITE_REG(this->m_pDrv,CAM_RSSO_A_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_RSSO_A_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_RSSO_A_STRIDE,this->dma_cfg.size.stride);
            }
            break;
        case CAM_DMA_AFO:
            {
                Header_AFO fh_afo;

                fh_afo.Header_Enque(Header_AFO::E_STRIDE,this->m_pDrv,this->dma_cfg.size.stride);

                CAM_WRITE_REG(this->m_pDrv,CAM_AFO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_AFO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_AFO_STRIDE,this->dma_cfg.size.stride);
            }
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_REG(this->m_pDrv,CAM_AAO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_AAO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_AAO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_PSO:
            CAM_WRITE_REG(this->m_pDrv,CAM_PSO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_PSO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_PSO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_LCSO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_IMGO:
            {
                MUINT32 new_crop_x = 0;
                Header_IMGO fh_imgo;
                E_PIX pixMode;
                capibility CamInfo;
                tCAM_rst rst;

                if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN))
                {
                    if((this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE)) ||
                        (this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TRIPLE_MODULE))) {
                        break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                    }
                }

                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_STRIDE,this->dma_cfg.size.stride);

                switch(this->dma_cfg.lIspColorfmt){
                    case NSCam::eImgFmt_UFO_BAYER8:
                    case NSCam::eImgFmt_UFO_BAYER10:
                    case NSCam::eImgFmt_UFO_BAYER12:
                        //imgo can't do crop when compressed
                        CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_CROP,0);
                        break;
                    default:
                        //
                        this->dma_cfg.bus_size = BusSizeCal(pixMode);

                        if (CamInfo.GetCapibility(
                             NSImageio::NSIspio::EPortIndex_IMGO,
                             NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                             NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->dma_cfg.lIspColorfmt,
                                                                                this->dma_cfg.crop.x, MAP_CAM_PIXMODE(pixMode)), rst, E_CAM_UNKNOWNN) == MFALSE) {
                            CAM_FUNC_ERR("cropping size is not fit hw constraint(%d_%d)\n",this->dma_cfg.crop.x,rst.crop_x);
                            return -1;
                        }
                        new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);
                        CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));
                        //reverse for FH below
                        new_crop_x = GetCropXUnitPixel(new_crop_x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);


                        //
                        fh_imgo.Header_Enque(Header_IMGO::E_CROP_START,this->m_pDrv,((this->dma_cfg.crop.y << 16) | new_crop_x));
                        fh_imgo.Header_Enque(Header_IMGO::E_CROP_SIZE,this->m_pDrv,((this->dma_cfg.crop.h << 16) | this->dma_cfg.crop.w));
                        break;
                }
            }
            break;
        case CAM_DMA_RRZO:

            if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN))
            {
                if((this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE)) ||
                    (this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TRIPLE_MODULE))) {
                    break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                }
            }

            CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_STRIDE,this->dma_cfg.size.stride);//stride;

            switch(this->dma_cfg.lIspColorfmt){
                case NSCam::eImgFmt_UFO_FG_BAYER8:
                case NSCam::eImgFmt_UFO_FG_BAYER10:
                case NSCam::eImgFmt_UFO_FG_BAYER12:
                    //imgo can't do crop when compressed
                    CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_CROP,0);
                    break;
                default:
                    //  We use RRZ to corp, thus almost the RRZO crop width & height is 0
                    //CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_CROP, (this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size));
                    if(this->dma_cfg.crop.y || this->dma_cfg.crop.x)
                        CAM_FUNC_ERR("rrzo support no cropping , replace dmao cropping start by 0\n");
                    break;
            }


            break;
        case CAM_DMA_UFEO:
            {
                Header_UFEO fh_ufeo;

                fh_ufeo.Header_Enque(Header_UFEO::E_UFEO_XSIZE,this->m_pDrv,this->dma_cfg.size.xsize);  //AU size don't need to -1

                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_STRIDE,this->dma_cfg.size.stride);
                //CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_UFE_CON,0x00000001);  //If set this, UFG will not compress, means that compress ratio = 100%
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_UFGO fh_ufgo;

                fh_ufgo.Header_Enque(Header_UFGO::E_UFGO_XSIZE,this->m_pDrv,this->dma_cfg.size.xsize ); //AU size don't need to -1

                CAM_WRITE_REG(this->m_pDrv,CAM_UFGO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFGO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFGO_STRIDE,this->dma_cfg.size.stride);
            }
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
            }
            break;
        default:
            break;
    }

    CAM_FUNC_DBG("DMAO_B::_config-[%s]:size(%ld,%ld,%ld,%ld), pixel_byte(%d),cropW(%d,%d,%ld,%ld),format(0x%x),fps(0x%x),cropX_inbussize(%d)", \
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
    GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size));

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
        case CAM_DMA_LMVO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LMVO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,LMVO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_LMVO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,LMVO_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_FLKO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,FLKO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,FLKO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_FLKO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,FLKO_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_RSSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RSSO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,RSSO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RSSO_A,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,RSSO_A_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_AFO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AFO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,AFO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AFO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,AFO_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AAO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,AAO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AAO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,AAO_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_PSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PSO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,PSO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_PSO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,PSO_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LCSO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,LCSO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_LCSO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,LCSO_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_IMGO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,IMGO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_IMGO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,IMGO_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,RRZO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RRZO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,RRZO_LAST_ULTRA_EN ,1);
            break;
        case CAM_DMA_UFEO:
            {
                Header_IMGO fh_imgo;
                fh_imgo.Header_Enque(Header_IMGO::E_IS_UFE_FMT,this->m_pDrv,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,UFE_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PAK_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL,UFE_SEL,1);

                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,UFEO_DONE_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_SPECIAL_FUN_EN,UFO_IMGO_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_UFEO,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,UFEO_LAST_ULTRA_EN ,1);
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IS_UFG_FMT,this->m_pDrv,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,UFEG_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PAKG_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL,UFEG_SEL,1);

                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFGO_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,UFGO_DONE_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_SPECIAL_FUN_EN,UFOG_RRZO_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_UFGO,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,UFGO_LAST_ULTRA_EN ,1);
            }
            break;
        case CAM_DMA_PDO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,PDO_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_PDO,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,PDO_LAST_ULTRA_EN ,1);
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
        case CAM_DMA_LMVO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LMVO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,LMVO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_LMVO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,LMVO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_FLKO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,FLKO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,FLKO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_FLKO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,FLKO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_RSSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RSSO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,RSSO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RSSO_A,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,RSSO_A_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_AFO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AFO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,AFO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AFO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,AFO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AAO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,AAO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_AAO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,AAO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_PSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PSO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,PSO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_PSO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,PSO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,LCSO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,LCSO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_LCSO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,LCSO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_IMGO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,IMGO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_IMGO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,IMGO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,RRZO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_RRZO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,RRZO_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_UFEO:
            {
                Header_IMGO fh_imgo;
                fh_imgo.Header_Enque(Header_IMGO::E_IS_UFE_FMT,this->m_pDrv,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,UFE_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PAK_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,UFEO_DONE_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_SPECIAL_FUN_EN,UFO_IMGO_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_UFEO,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL,UFE_SEL,0);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,UFEO_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IS_UFG_FMT,this->m_pDrv,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,UFEG_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PAKG_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFGO_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,UFGO_DONE_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_SPECIAL_FUN_EN,UFOG_RRZO_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_UFGO,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL,UFEG_SEL,0);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,UFGO_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_PDO:
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_RAW_INT2_EN,PDO_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_PDO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_LAST_ULTRA_EN,PDO_LAST_ULTRA_EN,0);
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
        case CAM_DMA_LMVO:
            this->m_pDrv->cqAddModule(CAM_DMA_LMVO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_LMVO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_SPARE_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_SPARE_2);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_EN_);
            break;
        case CAM_DMA_FLKO:
            this->m_pDrv->cqAddModule(CAM_DMA_FLKO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_FLKO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_EN_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_FLKO_SPARE_CQ0_);
            break;
        case CAM_DMA_RSSO:
            this->m_pDrv->cqAddModule(CAM_DMA_RSSO_A_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_RSSO_A_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_SPARE_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_SPARE_2);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_SPARE_3);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_EN_);
            break;
        case CAM_DMA_IMGO:
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA_OFST);
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_);
            this->m_pDrv->cqAddModule(CAM_DMA_IMGO_CROP_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
            break;
        case CAM_DMA_RRZO:
            this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA_OFST);
            this->m_pDrv->cqAddModule(CAM_DMA_RRZO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_RRZO_CROP_);   rrzo crop is not supported, using rrz crop
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);
            break;
        case CAM_DMA_UFEO:
            this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA_OFST);
            this->m_pDrv->cqAddModule(CAM_DMA_UFEO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_SPARE_);
            break;
        case CAM_DMA_UFGO:
            this->m_pDrv->cqAddModule(CAM_DMA_UFGO_BA);
            this->m_pDrv->cqAddModule(CAM_DMA_UFGO_BA_OFST);
            this->m_pDrv->cqAddModule(CAM_DMA_UFGO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_SPARE_);
            break;
        case CAM_DMA_AFO:
            this->m_pDrv->cqAddModule(CAM_DMA_AFO_);
            this->m_pDrv->cqAddModule(CAM_DMA_AFO_BA_OFST);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_AFO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_AFO_SPARE_CQ0_);
            break;
        case CAM_DMA_AAO:
            this->m_pDrv->cqAddModule(CAM_DMA_AAO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_AAO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_AAO_SPARE_CQ0_);
            break;
        case CAM_DMA_PSO:
            this->m_pDrv->cqAddModule(CAM_DMA_PSO_);
            //this->m_pDrv->cqAddModule(CAM_DMA_FH_PSO_);
            this->m_pDrv->cqAddModule(CAM_DMA_FH_PSO_SPARE_CQ0_);
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
            this->m_pDrv->cqAddModule(CAM_DMA_FH_PDO_SPARE_CQ0_);
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

    CAM_FUNC_DBG("DMAO_B::setBaseAddr+[%s]:cq(0x%x),page(0x%x),pa(0x%" PRIXPTR "_0x%08X),ofst(0x%X)", \
        this->name_Str(), \
        cq,page,\
        this->dma_cfg.memBuf.base_pAddr, this->Header_Addr,\
        this->dma_cfg.memBuf.ofst_addr);


    switch(this->id())
    {
        case CAM_DMA_LMVO:
            {
                Header_LMVO fh_lmvo;
                fh_lmvo.Header_Enque(Header_LMVO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_LMVO_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,CAM_LMVO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_LMVO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_FLKO:
            {
                Header_FLKO fh_flko;
                CQ_RingBuf_ST bufctrl;
                if (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_FLKO_CTL1,SUB_RATIO)) {
                    bufctrl.ctrl = BUF_CTRL_ENQUE_HIGH_SPEED;
                }else
                    bufctrl.ctrl = BUF_CTRL_ENQUE;
                bufctrl.dma_PA = this->dma_cfg.memBuf.base_pAddr;
                bufctrl.dma_FH_PA = this->Header_Addr;
                //
                if(this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE){
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                if (this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE) {
                    CAM_FUNC_ERR("dma:0x%x: set fh spare ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                fh_flko.Header_Enque(Header_FLKO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        case CAM_DMA_RSSO:
            {
                Header_RSSO fh_rsso;
                fh_rsso.Header_Enque(Header_RSSO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_RSSO_A_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,CAM_RSSO_A_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_RSSO_A_FH_BASE_ADDR,this->Header_Addr);
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
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                if (this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE) {
                    CAM_FUNC_ERR("dma:0x%x: set fh spare ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
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
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                if (this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE) {
                    CAM_FUNC_ERR("dma:0x%x: set fh spare ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                fh_aao.Header_Enque(Header_AAO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        case CAM_DMA_PSO:
            {
                Header_PSO  fh_pso;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,SUB_RATIO))
                    bufctrl.ctrl = BUF_CTRL_ENQUE_HIGH_SPEED;
                else
                    bufctrl.ctrl = BUF_CTRL_ENQUE;
                bufctrl.dma_PA = this->dma_cfg.memBuf.base_pAddr;
                bufctrl.dma_FH_PA = this->Header_Addr;
                //
                if(this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE){
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                if (this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE) {
                    CAM_FUNC_ERR("dma:0x%x: set fh spare ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                fh_pso.Header_Enque(Header_PSO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
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

                Header_UFEO fh_ufeo;
                fh_ufeo.Header_Enque(Header_UFEO::E_IMGO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_IMGO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_RRZO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                Header_UFGO fh_ufgo;
                fh_ufgo.Header_Enque(Header_UFGO::E_RRZO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CAM_RRZO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_UFEO:
            {
                Header_UFEO fh_ufeo;
                fh_ufeo.Header_Enque(Header_UFEO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                fh_ufeo.Header_Enque(Header_UFEO::E_UFEO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,CAM_UFEO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_UFGO fh_ufgo;
                fh_ufgo.Header_Enque(Header_UFGO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                fh_ufgo.Header_Enque(Header_UFGO::E_UFGO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,CAM_UFGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,CAM_UFGO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,CAM_UFGO_FH_BASE_ADDR,this->Header_Addr);
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
                    CAM_FUNC_ERR("dma:0x%x: enque ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                bufctrl.ctrl = SET_FH_SPARE;
                if (this->m_pDrv->cqRingBuf(&bufctrl) == MFALSE) {
                    CAM_FUNC_ERR("dma:0x%x: set fh spare ringbuf fail.[addr:0x%" PRIXPTR "]\n",this->id(),this->dma_cfg.memBuf.base_pAddr);
                    return MFALSE;
                }
                fh_pdo.Header_Enque(Header_PDO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        default:
            break;
    }

    //CAM_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}


MUINT32 DMAO_B::BusSizeCal(E_PIX &pixMode)
{
    CAM_REG_CTL_SEL ctl_sel;
    CAM_REG_CTL_FMT_SEL fmt_sel;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    ctl_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL);
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_FMT_SEL);
    pixMode = (E_PIX)fmt_sel.Bits.PIX_BUS_DMXI;

    switch(fmt_sel.Bits.IMGO_FMT){
        case 3://yuv
        case 7://jpg
            if(pixMode == _4_pix_)
                return 3;
            else
                return 1;
            break;
        default://bayer
            switch(ctl_sel.Bits.UFE_SEL){
                case eUFESel_1: //UFE
                    CAM_FUNC_ERR("hw have no this path\n");
                    break;
                case eUFESel_2: //TG
                    switch(pixMode){
                        case _2_pix_: return 3; break;
                        case _4_pix_: return 7; break;
                        default: return 1; break;
                    }
                    break;
                default:    //PAK
                    switch(ctl_sel.Bits.IMG_SEL){
                        case eIMGSel_0:
                            CAM_FUNC_ERR("hw have no this path\n");
                            break;
                        case eIMGSel_1:
                            switch(pixMode){
                                case _2_pix_: return 3; break;
                                case _4_pix_: return 7; break;
                                default: return 1; break;
                            }
                            break;
                        case eIMGSel_2:
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


///////////////////////////////////////////////////////////////
//                    Bayer   Encoder                                                                        //
///////////////////////////////////////////////////////////////
MVOID DMA_UFEO::check(MBOOL& bUF_OFF)
{
    MBOOL enable = MTRUE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    bUF_OFF = MFALSE;

    if( CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN) == 1){
        CAM_REG_CTL_SEL sel;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL);
        //
        if (CAM_READ_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_EN)) {
            if(sel.Bits.IMG_SEL == 2){
                CAM_FUNC_WRN("UFE can't be enabled under IMGO-Twin mode. Disable UFE/UFEO");
                enable = MFALSE;
                goto EXIT;
            }
        }

        if (sel.Bits.IMG_SEL == 1) { //pure raw path
            if (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_TG_SEN_MODE, DBL_DATA_BUS) == 1 &&
                CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_TG_SEN_MODE, DBL_DATA_BUS1) == 1) {
                    CAM_FUNC_WRN("UFE can't be enabled under 4 pixel mode. Disable UFE/UFEO");
                    enable = MFALSE;
                    goto EXIT;
                }
        }

        if (sel.Bits.UFE_SEL == 0 || sel.Bits.UFE_SEL == 2) {
            //imgo's xsize need to change back to normal-bayer if UFE_SEL want to be 0
            //for design convenience, just return err
            CAM_FUNC_WRN("UFE can't be enabled under pak path or pure raw path. Disable UFE/UFEO");
            enable = MFALSE;
            goto EXIT;
        }

        if(CAM_READ_REG(this->m_pDrv, CAM_IMGO_CROP) != 0){
            CAM_FUNC_WRN("UFE can't be enabled under IMGO crop!= 0(0x%x)\n",CAM_READ_REG(this->m_pDrv, CAM_IMGO_CROP));

            enable = MFALSE;
            goto EXIT;
        }

        if((CAM_READ_BITS(this->m_pDrv, CAM_IMGO_YSIZE,YSIZE)+1) != (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S)){
            CAM_FUNC_WRN("UFE can't be enabled under IMGO vieritcal crop(0x%x)\n",CAM_READ_BITS(this->m_pDrv, CAM_IMGO_YSIZE,YSIZE));
            enable = MFALSE;
            goto EXIT;
        }
    EXIT:
        if (enable == MFALSE) {
            DMA_UFEO ufeo;
            BUF_CTRL_UFEO fbc_ufeo;
            bUF_OFF = MTRUE;
            //
            ufeo.m_pIspDrv = this->m_pIspDrv;
            ufeo.disable();
            //
            fbc_ufeo.m_pIspDrv = this->m_pIspDrv;
            fbc_ufeo.disable();


            //Calculate xsize for imgo due to img FMT is changed
            {
                capibility      CamInfo;
                DMA_IMGO Imgo;
                tCAM_rst    rst;
                CAM_REG_TG_SEN_GRAB_PXL TG_W;

                E_PIX pixMode = _1_pix_;
                TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_TG_SEN_GRAB_PXL);

                switch(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,IMGO_FMT)) {
                    case IMGO_FMT_RAW8:
                    case IMGO_FMT_RAW8_2B:
                        Imgo.dma_cfg.lIspColorfmt = eImgFmt_BAYER8;
                        break;
                    case IMGO_FMT_RAW10:
                    case IMGO_FMT_RAW10_2B:
                    case IMGO_FMT_RAW10_MIPI:
                        Imgo.dma_cfg.lIspColorfmt = eImgFmt_BAYER10;
                        break;
                    case IMGO_FMT_RAW12:
                    case IMGO_FMT_RAW12_2B:
                        Imgo.dma_cfg.lIspColorfmt = eImgFmt_BAYER12;
                        break;
                    case IMGO_FMT_RAW14:
                    case IMGO_FMT_RAW14_2B:
                        Imgo.dma_cfg.lIspColorfmt = eImgFmt_BAYER14;
                        break;
                    default:
                        CAM_FUNC_ERR("Unsupport fmt:%d", CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,IMGO_FMT));
                        return;
                }


                if (sel.Bits.UFE_SEL == 2 || (sel.Bits.UFE_SEL != 2 && sel.Bits.IMG_SEL == 1)) {//pure raw path
                    pixMode = (E_PIX)CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_DMXI);
                    Imgo.dma_cfg.size.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;

                } else {
                    MBOOL bDbn, bin;
                    bDbn = CAM_READ_BITS(this->m_pDrv, CAM_CTL_EN, DBN_EN);
                    bin = CAM_READ_BITS(this->m_pDrv, CAM_CTL_EN, BIN_EN);

                    if (sel.Bits.IMG_SEL == 0) { //From AMX
                        pixMode = (E_PIX)CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_AMXO);
                        Imgo.dma_cfg.size.w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> (bDbn + bin);

                    } else if (sel.Bits.IMG_SEL == 2) { //From RCP3

                        if (CAM_READ_BITS(this->m_pDrv, CAM_CTL_SEL, RCP3_SEL) == 2) { //Pure raw after DMX path
                            pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_DMXO) >> bDbn);
                        } else {
                            pixMode = _1_pix_;
                        }

                        Imgo.dma_cfg.size.w = (CAM_READ_BITS(this->m_pDrv,CAM_RCP3_CROP_CON1,RCP_END_X) -
                                               CAM_READ_BITS(this->m_pDrv,CAM_RCP3_CROP_CON1,RCP_STR_X)+ 1) * (pixMode + 1); //Because RCP3 control use cycle counts not pixel counts.
                    }
                }

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_IMGO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)Imgo.dma_cfg.lIspColorfmt, Imgo.dma_cfg.size.w, (E_CamPixelMode)(pixMode+1)),
                    rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("img x_byte size err(0x%x)\n", Imgo.dma_cfg.size.w);
                }
                CAM_WRITE_REG(this->m_pDrv, CAM_IMGO_XSIZE, rst.xsize_byte[0]-1);
            }
        }
    }
}

MVOID DMA_UFEO::update_AU(void)
{
    Header_UFEO fh_ufeo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    fh_ufeo.Header_Enque(Header_UFEO::E_UFEO_XSIZE,this->m_pDrv,CAM_READ_BITS(this->m_pDrv,CAM_UFEO_XSIZE,XSIZE)+1);
}

MVOID DMA_UFGO::check(MBOOL& bUF_OFF)
{
    MBOOL enable = MTRUE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    bUF_OFF = MFALSE;

    if( CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFGO_EN) == 1){
        Header_RRZO rrzo_fh;
        MUINT32 size_v;
        size_v = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        size_v = (size_v>>16) & 0xffff;
        //only have twin's constraint. maintain in twinmgr
        if (CAM_READ_BITS(this->m_pDrv, CAM_CTL_SEL, UFEG_SEL) == 0) {
            //imgo's xsize need to change back to normal-bayer if UFE_SEL want to be 0
            //for design convenience, just return err
            CAM_FUNC_WRN("UFEG can't be enabled under pak path. Disable UFEG/UFGO");
            enable = MFALSE;
            goto EXIT;
        }

        if(CAM_READ_BITS(this->m_pDrv, CAM_RRZ_OUT_IMG, RRZ_OUT_WD) < 512 ){
            CAM_FUNC_WRN("UFEG can't be enabled when rrz output width < 512(%d)\n",CAM_READ_BITS(this->m_pDrv, CAM_RRZ_OUT_IMG, RRZ_OUT_WD));
            enable = MFALSE;
            goto EXIT;
        }

        if(CAM_READ_REG(this->m_pDrv, CAM_RRZO_CROP) != 0){
            CAM_FUNC_WRN("UFEG can't be enabled under RRZO crop!= 0(0x%x)\n",CAM_READ_REG(this->m_pDrv, CAM_RRZO_CROP));
            enable = MFALSE;
            goto EXIT;
        }

        if((CAM_READ_BITS(this->m_pDrv, CAM_RRZO_YSIZE,YSIZE)+1) != size_v){
            CAM_FUNC_WRN("UFEG can't be enabled under RRZO crop(0x%x)\n",CAM_READ_BITS(this->m_pDrv, CAM_RRZO_YSIZE,YSIZE));
            enable = MFALSE;
            goto EXIT;
        }
EXIT:
        if (enable == MFALSE) {
            DMA_UFGO ufgo;
            BUF_CTRL_UFGO fbc_ufgo;
            bUF_OFF = MTRUE;
            //
            ufgo.m_pIspDrv = this->m_pIspDrv;
            ufgo.disable();
            //
            fbc_ufgo.m_pIspDrv = this->m_pIspDrv;
            fbc_ufgo.disable();

            {
                capibility      CamInfo;
                DMA_RRZO Rrzo;
                tCAM_rst    rst;
                E_CamPixelMode pixMode = ePixMode_1; //always be fixed in 1-pix mode.

                Rrzo.dma_cfg.size.w = CAM_READ_BITS(this->m_pDrv, CAM_RRZ_OUT_IMG, RRZ_OUT_WD);

                switch(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,RRZO_FMT)) {
                    case RRZO_FMT_RAW8:
                        Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER8;
                        break;
                    case RRZO_FMT_RAW10:
                        Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER10;
                        break;
                    case RRZO_FMT_RAW12:
                        Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER12;
                        break;
                    case RRZO_FMT_RAW14:
                        Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER14;
                        break;
                    default:
                        CAM_FUNC_ERR("Unsupport fmt:%d", CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,RRZO_FMT));
                        return;
                }

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_RRZO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)Rrzo.dma_cfg.lIspColorfmt, Rrzo.dma_cfg.size.w, pixMode),
                    rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("rrz x_byte size err(0x%x)\n", Rrzo.dma_cfg.size.w);
                }

                CAM_WRITE_REG(this->m_pDrv, CAM_RRZO_XSIZE, rst.xsize_byte[0]-1);
            }
        }
    }
}

MVOID DMA_UFGO::update_AU(void)
{
    Header_UFGO fh_ufgo;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    fh_ufgo.Header_Enque(Header_UFGO::E_UFGO_XSIZE,this->m_pDrv,CAM_READ_BITS(this->m_pDrv,CAM_UFGO_XSIZE,XSIZE)+1);
}

