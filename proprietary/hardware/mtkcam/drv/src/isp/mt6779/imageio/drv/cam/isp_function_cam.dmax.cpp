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

#define SET_CQ0_BA_S(hw1) ({\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR, Addr[hw1]);\
    CAM_FUNC_DBG("hw(%d),addr(0x%x)\n",hw1,Addr[hw1]);\
    pDrv[hw1] = 0;\
    Addr[hw1] = 0;\
})

#define SET_CQ0_BA_D(hw1, hw2) ({\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL2, 0x100);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL, 0x80000000);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR, Addr[hw1]);\
    CAM_WRITE_REG(pDrv[hw2]->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR, Addr[hw2]);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL, 0x0);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL2, 0x0);\
    CAM_FUNC_DBG("hw(%d_%d),addr(0x%x_0x%x)\n",hw1,hw2,Addr[hw1],Addr[hw2]);\
    pDrv[hw1] = 0;\
    pDrv[hw2] = 0;\
    Addr[hw1] = 0;\
    Addr[hw2] = 0;\
})
#define SET_CQ0_BA_T(hw1, hw2, hw3) ({\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL2, 0x100);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL, 0x80000000);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR, Addr[hw1]);\
    CAM_WRITE_REG(pDrv[hw2]->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR, Addr[hw2]);\
    CAM_WRITE_REG(pDrv[hw3]->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR, Addr[hw3]);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL, 0x0);\
    CAM_WRITE_REG(pDrv[hw1]->getPhyObj(), CAMCTL_R1_CAMCTL_DONE_SEL2, 0x0);\
    CAM_FUNC_DBG("hw(%d_%d_%d),addr(0x%x_0x%x_0x%x)\n",hw1,hw2,hw3,Addr[hw1],Addr[hw2],Addr[hw3]);\
    pDrv[hw1] = 0;\
    pDrv[hw2] = 0;\
    pDrv[hw3] = 0;\
    Addr[hw1] = 0;\
    Addr[hw2] = 0;\
    Addr[hw3] = 0;\
})

//
/*/////////////////////////////////////////////////////////////////////////////
    DMAI_B
  /////////////////////////////////////////////////////////////////////////////*/

DMAI_B::DMAI_B()
{
    m_pDrv = NULL;
    m_updatecnt = 0;
}

MBOOL DMAI_B::BW_Meter(const Sig_Profile& prof,V_DMA_BW* pOut)
{
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
    pOut->clear();
    if((prof.lineT == 0) || (prof.frameT == 0)){
        CAM_FUNC_ERR("lineT | frameT can't be 0\n");
        return MFALSE;
    }
    {
        DMA_BW fifo;
        REG_CAMCTL_R1_CAMCTL_DMA_EN dma_en;
        MUINT32 v, lv, tmp;
        //
        dma_en.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN);
        v = CAM_READ_BITS(this->m_pDrv,LSC_R1_LSC_CTL3,LSC_SDBLK_HEIGHT);
        lv = CAM_READ_BITS(this->m_pDrv,LSC_R1_LSC_LBLOCK,LSC_SDBLK_lHEIGHT);
        tmp = (v > lv) ? lv : v;
        //
        fifo = BW_l(_lsci_,dma_en.Bits.CAMCTL_LSCI_R1_EN,LSCI_R1_LSCI_XSIZE,LSCI_R1_LSCI_YSIZE);
        fifo.ThruPut = fifo.ThruPut / (tmp) ? tmp : 1;
        pOut->push_back(fifo);
        pOut->push_back(BW_l(_bpci_,dma_en.Bits.CAMCTL_BPCI_R1_EN,BPCI_R1_BPCI_XSIZE,BPCI_R1_BPCI_YSIZE));
        pOut->push_back(BW_a(_pdi_,dma_en.Bits.CAMCTL_PDI_R1_EN,PDI_R1_PDI_XSIZE,PDI_R1_PDI_YSIZE));
        pOut->push_back(BW_f(_rawi_,dma_en.Bits.CAMCTL_RAWI_R2_EN,RAWI_R2_RAWI_XSIZE,RAWI_R2_RAWI_YSIZE));
        pOut->push_back(BW_f(_ufdi_r2,dma_en.Bits.CAMCTL_UFDI_R2_EN,UFDI_R2_UFDI_XSIZE,UFDI_R2_UFDI_YSIZE));
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
        case CAM_DMA_CQ0I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ1I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR1_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ2I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR2_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ3I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR3_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ4I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR4_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ5I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR5_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ6I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR6_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ7I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR7_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ8I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR8_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ9I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR9_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ10I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR10_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ11I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR11_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ12I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR12_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ13I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR13_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ14I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR14_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ15I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR15_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ16I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR16_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ17I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR17_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ18I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR18_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ19I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR19_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ20I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR20_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ21I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR21_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ22I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR22_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ23I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR23_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ24I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR24_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ25I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR25_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ26I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR26_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ27I:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR27_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_CQ28I:
            //use CQ10 to load CQ28 inner & outter address.
            //because CPU can't program inner address when using device tree.
            CAM_WRITE_REG(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR28_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
            CAM_WRITE_REG(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR28_BASEADDR_INNER,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_REG(this->m_pDrv,BPCI_R1_BPCI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,BPCI_R1_BPCI_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,BPCI_R1_BPCI_STRIDE,this->dma_cfg.size.stride);
            CAM_WRITE_REG(this->m_pDrv,BPCI_R1_BPCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,BPCI_R1_BPCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_REG(this->m_pDrv,LSCI_R1_LSCI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,LSCI_R1_LSCI_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,LSCI_R1_LSCI_STRIDE,this->dma_cfg.size.stride);//stride;
            CAM_WRITE_REG(this->m_pDrv,LSCI_R1_LSCI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,LSCI_R1_LSCI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

            break;
        case CAM_DMA_PDI:
            CAM_WRITE_REG(this->m_pDrv,PDI_R1_PDI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,PDI_R1_PDI_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,PDI_R1_PDI_STRIDE,this->dma_cfg.size.stride);//stride;
            CAM_WRITE_REG(this->m_pDrv,PDI_R1_PDI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            CAM_WRITE_REG(this->m_pDrv,PDI_R1_PDI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

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
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_EN,1);
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_EN,1);
#endif
            }
            break;
        case CAM_DMA_CQ1I:
            if(pParam == NULL){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR1_CTL,CAMCQ_CQ_THR1_EN,1);
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR1_CTL,CAMCQ_CQ_THR1_EN,1);
#endif
            }
            break;
        case CAM_DMA_CQ2I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR2_CTL,CAMCQ_CQ_THR2_EN,1);
            break;
        case CAM_DMA_CQ3I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR3_CTL,CAMCQ_CQ_THR3_EN,1);
            break;
        case CAM_DMA_CQ4I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR4_CTL,CAMCQ_CQ_THR4_EN,1);
            break;
        case CAM_DMA_CQ5I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR5_CTL,CAMCQ_CQ_THR5_EN,1);
            break;
        case CAM_DMA_CQ6I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR6_CTL,CAMCQ_CQ_THR6_EN,1);
            break;
        case CAM_DMA_CQ7I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR7_CTL,CAMCQ_CQ_THR7_EN,1);
            break;
        case CAM_DMA_CQ8I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR8_CTL,CAMCQ_CQ_THR8_EN,1);
            break;
        case CAM_DMA_CQ9I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR9_CTL,CAMCQ_CQ_THR9_EN,1);
            break;
        case CAM_DMA_CQ10I:
            if(pParam == NULL){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR10_CTL,CAMCQ_CQ_THR10_EN,1);
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR10_CTL,CAMCQ_CQ_THR10_EN,1);
#endif
            }
            break;
        case CAM_DMA_CQ11I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR11_CTL,CAMCQ_CQ_THR11_EN,1);
            break;
        case CAM_DMA_CQ12I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR12_CTL,CAMCQ_CQ_THR12_EN,1);
            break;
        case CAM_DMA_CQ13I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR13_CTL,CAMCQ_CQ_THR13_EN,1);
            break;
        case CAM_DMA_CQ14I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR14_CTL,CAMCQ_CQ_THR14_EN,1);
            break;
        case CAM_DMA_CQ15I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR15_CTL,CAMCQ_CQ_THR15_EN,1);
            break;
        case CAM_DMA_CQ16I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR16_CTL,CAMCQ_CQ_THR16_EN,1);
            break;
        case CAM_DMA_CQ17I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR17_CTL,CAMCQ_CQ_THR17_EN,1);
            break;
        case CAM_DMA_CQ18I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR18_CTL,CAMCQ_CQ_THR18_EN,1);
            break;
        case CAM_DMA_CQ19I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR19_CTL,CAMCQ_CQ_THR19_EN,1);
            break;
        case CAM_DMA_CQ20I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR20_CTL,CAMCQ_CQ_THR20_EN,1);
            break;
        case CAM_DMA_CQ21I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR21_CTL,CAMCQ_CQ_THR21_EN,1);
            break;
        case CAM_DMA_CQ22I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR22_CTL,CAMCQ_CQ_THR22_EN,1);
            break;
        case CAM_DMA_CQ23I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR23_CTL,CAMCQ_CQ_THR23_EN,1);
            break;
        case CAM_DMA_CQ24I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR24_CTL,CAMCQ_CQ_THR24_EN,1);
            break;
        case CAM_DMA_CQ25I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR25_CTL,CAMCQ_CQ_THR25_EN,1);
            break;
        case CAM_DMA_CQ26I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR26_CTL,CAMCQ_CQ_THR26_EN,1);
            break;
        case CAM_DMA_CQ27I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR27_CTL,CAMCQ_CQ_THR27_EN,1);
            break;
        case CAM_DMA_CQ28I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR28_CTL,CAMCQ_CQ_THR28_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_I2C_CQ_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_CQ_SEL,0);//always imm mode
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_BPCI_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LSCI_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_LSCI_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_PDI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDI_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_PDI_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_RAWI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RAWI_R2_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_RAWI_R2_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_UFDI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFDI_R2_EN,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_UFDI_R2_LAST_ULTRA_EN,1);
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
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_EN,0);
                //reset update cnt when disable
                this->m_updatecnt = 0;
            }
            else{
#if TWIN_CQ_SW_WORKAROUND
                CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR0_START,0);
#endif
            }
            break;
        case CAM_DMA_CQ1I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR1_CTL,CAMCQ_CQ_THR1_EN,0);
            break;
        case CAM_DMA_CQ2I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR2_CTL,CAMCQ_CQ_THR2_EN,0);
            break;
        case CAM_DMA_CQ3I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR3_CTL,CAMCQ_CQ_THR3_EN,0);
            break;
        case CAM_DMA_CQ4I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR4_CTL,CAMCQ_CQ_THR4_EN,0);
            break;
        case CAM_DMA_CQ5I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR5_CTL,CAMCQ_CQ_THR5_EN,0);
            break;
        case CAM_DMA_CQ6I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR6_CTL,CAMCQ_CQ_THR6_EN,0);
            break;
        case CAM_DMA_CQ7I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR7_CTL,CAMCQ_CQ_THR7_EN,0);
            break;
        case CAM_DMA_CQ8I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR8_CTL,CAMCQ_CQ_THR8_EN,0);
            break;
        case CAM_DMA_CQ9I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR9_CTL,CAMCQ_CQ_THR9_EN,0);
            break;
        case CAM_DMA_CQ10I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR10_CTL,CAMCQ_CQ_THR10_EN,0);
            break;
        case CAM_DMA_CQ11I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR11_CTL,CAMCQ_CQ_THR11_EN,0);
            break;
        case CAM_DMA_CQ12I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR12_CTL,CAMCQ_CQ_THR12_EN,0);
            break;
        case CAM_DMA_CQ13I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR13_CTL,CAMCQ_CQ_THR13_EN,0);
            break;
        case CAM_DMA_CQ14I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR14_CTL,CAMCQ_CQ_THR14_EN,0);
            break;
        case CAM_DMA_CQ15I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR15_CTL,CAMCQ_CQ_THR15_EN,0);
            break;
        case CAM_DMA_CQ16I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR16_CTL,CAMCQ_CQ_THR16_EN,0);
            break;
        case CAM_DMA_CQ17I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR17_CTL,CAMCQ_CQ_THR17_EN,0);
            break;
        case CAM_DMA_CQ18I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR18_CTL,CAMCQ_CQ_THR18_EN,0);
            break;
        case CAM_DMA_CQ19I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR19_CTL,CAMCQ_CQ_THR19_EN,0);
            break;
        case CAM_DMA_CQ20I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR20_CTL,CAMCQ_CQ_THR20_EN,0);
            break;
        case CAM_DMA_CQ21I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR21_CTL,CAMCQ_CQ_THR21_EN,0);
            break;
        case CAM_DMA_CQ22I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR22_CTL,CAMCQ_CQ_THR22_EN,0);
            break;
        case CAM_DMA_CQ23I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR23_CTL,CAMCQ_CQ_THR23_EN,0);
            break;
        case CAM_DMA_CQ24I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR24_CTL,CAMCQ_CQ_THR24_EN,0);
            break;
        case CAM_DMA_CQ25I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR25_CTL,CAMCQ_CQ_THR25_EN,0);
            break;
        case CAM_DMA_CQ26I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR26_CTL,CAMCQ_CQ_THR26_EN,0);
            break;
        case CAM_DMA_CQ27I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR27_CTL,CAMCQ_CQ_THR27_EN,0);
            break;
        case CAM_DMA_CQ28I:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR28_CTL,CAMCQ_CQ_THR28_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_I2C_CQ_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_CQ_SEL,0);//always imm mode
            break;
        case CAM_DMA_BPCI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_BPCI_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_LSCI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LSCI_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_LSCI_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_PDI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDI_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_PDI_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_RAWI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RAWI_R2_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_RAWI_R2_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_UFDI:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFDI_R2_EN,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_UFDI_R2_LAST_ULTRA_EN,0);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            return -1;
            break;
    }
    return 0;
}



MINT32 DMAI_B::_write2CQ(MUINT32 burstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMAI_B::_write2CQ+ [%s]:cq(0x%x),page(0x%x),subsample(0x%x)\n",this->name_Str(),cq,page,burstIndex);

    if(burstIndex > 0){
        return 0;
    }

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
        case CAM_DMA_RAWI:
            this->m_pDrv->cqAddModule(CAM_DMA_RAWI_);
            break;
        case CAM_DMA_UFDI:
            this->m_pDrv->cqAddModule(CAM_DMA_UFDI_R2_);
            break;
        default:
            CAM_FUNC_DBG("DMAI: NOT push to CQ(%d)\n",this->id());
            break;
    }

    //
    CAM_FUNC_DBG("-\n");
    return 0;
}

MINT32 DMA_RAWI::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_RAWI::_config+ [%s]:cq(0x%x),page(0x%x),size(%ld,%ld,%ld,%ld),pixel_byte(%d),crop(%d,%d)\n",this->name_Str(),cq,page,\
        this->dma_cfg.size.w, \
        this->dma_cfg.size.h, \
        this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
        this->dma_cfg.pixel_byte, \
        this->dma_cfg.crop.x, \
        this->dma_cfg.crop.y);

    switch(this->id())
    {
        case CAM_DMA_RAWI:
            if(((this->dma_cfg.crop.x + this->dma_cfg.crop.y) != 0) || (this->dma_cfg.size.w!=this->dma_cfg.crop.w) || (this->dma_cfg.size.h!=this->dma_cfg.crop.h) ){
                CAM_FUNC_ERR("%s :cropping is not supported.bypass cropping(%d_%d_%d_%d)\n",this->name_Str(),\
                    this->dma_cfg.crop.x,this->dma_cfg.crop.y,this->dma_cfg.crop.w,this->dma_cfg.crop.h);
            }

            CAM_WRITE_REG(this->m_pDrv,RAWI_R2_RAWI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,RAWI_R2_RAWI_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,RAWI_R2_RAWI_STRIDE,this->dma_cfg.size.stride);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


    CAM_FUNC_DBG("-\n");
    return 0;
}

MINT32 DMA_RAWI::setBaseAddr( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_RAWI::_config+ [%s]:cq(0x%x),page(0x%x),pa(0x%" PRIXPTR "),ofst(0x%08X)\n",this->name_Str(),cq,page,\
        this->dma_cfg.memBuf.base_pAddr, \
        this->dma_cfg.memBuf.ofst_addr);

    switch(this->id())
    {
        case CAM_DMA_RAWI:
            CAM_WRITE_REG(this->m_pDrv,RAWI_R2_RAWI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            CAM_WRITE_REG(this->m_pDrv,RAWI_R2_RAWI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


    CAM_FUNC_DBG("-\n");
    return 0;
}

MINT32 DMA_UFDI::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_UFDI::_config+ [%s]:cq(0x%x),page(0x%x),size(%ld,%ld,%ld,%ld),pixel_byte(%d),crop(%d,%d)\n",this->name_Str(),cq,page,\
        this->dma_cfg.size.w, \
        this->dma_cfg.size.h, \
        this->dma_cfg.size.stride, this->dma_cfg.size.xsize, \
        this->dma_cfg.pixel_byte, \
        this->dma_cfg.crop.x, \
        this->dma_cfg.crop.y);

    switch(this->id())
    {
        case CAM_DMA_UFDI:
            if(this->dma_cfg.crop.x + this->dma_cfg.crop.y + this->dma_cfg.crop.w + this->dma_cfg.crop.h){
                CAM_FUNC_ERR("%s :cropping is not supported.bypass cropping\n",this->name_Str());
            }
            CAM_WRITE_REG(this->m_pDrv,UFDI_R2_UFDI_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,UFDI_R2_UFDI_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,UFDI_R2_UFDI_STRIDE,this->dma_cfg.size.stride);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


    CAM_FUNC_DBG("-\n");
    return 0;
}

MINT32 DMA_UFDI::setBaseAddr( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_UFDI::_config+ [%s]:cq(0x%x),page(0x%x),pa(0x%" PRIXPTR "),ofst(0x%08X)\n",this->name_Str(),cq,page,\
        this->dma_cfg.memBuf.base_pAddr, \
        this->dma_cfg.memBuf.ofst_addr);

    switch(this->id())
    {
        case CAM_DMA_UFDI:
            CAM_WRITE_REG(this->m_pDrv,UFDI_R2_UFDI_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
            CAM_WRITE_REG(this->m_pDrv,UFDI_R2_UFDI_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


    CAM_FUNC_DBG("-\n");
    return 0;
}

uintptr_t    DMA_CQ0::Addr[PHY_CAM] = {0,0,0};
ISP_DRV_CAM* DMA_CQ0::pDrv[PHY_CAM] = {0,0,0};
unsigned int twinAddr[3] = {0, 0, 0};

MBOOL DMA_CQ0::Init(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    pDrv[this->m_hwModule] = 0;
    Addr[this->m_hwModule] = 0;

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
                    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin_status;
                    twin_status.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS);

                    pDrv[this->m_hwModule] = this->m_pDrv;
                    Addr[this->m_hwModule] = this->dma_cfg.memBuf.base_pAddr;

                    if(twin_status.Bits.TWIN_EN){
                        switch (twin_status.Bits.SLAVE_CAM_NUM) {
                        case 1: /* 2 RAWS to 1 tg for static twin mode */
                            if (twin_status.Bits.MASTER_MODULE == CAM_A &&
                                twin_status.Bits.TWIN_MODULE == CAM_B) {
                                if ((Addr[CAM_A] != 0) &&
                                    (Addr[CAM_B] != 0)) {
                                    //if last p1 done refers to another cam, this means w+t cam switch.
                                    //in this case, need to make sure that the setting updated in the same frame
                                    //and so on, update cq base address at the same time
                                    MUINT32 P1DoneRef = CAM_READ_BITS_INR(pDrv[CAM_A]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL);
                                    if(P1DoneRef & eRefCamC){
                                        if(Addr[CAM_C] != 0){
                                            SET_CQ0_BA_T(CAM_A, CAM_B, CAM_C);
                                        }
                                    }
                                    else{
                                        SET_CQ0_BA_D(CAM_A, CAM_B);
                                    }
                                }
                            }
                            else {
                                CAM_FUNC_ERR("un-support twin case, master_slave:%d_%d", twin_status.Bits.MASTER_MODULE, twin_status.Bits.TWIN_MODULE);
                            }
                            break;
                        default:
                            CAM_FUNC_ERR("un-support twin case, slave_cam_num:%d", twin_status.Bits.SLAVE_CAM_NUM);
                            break;
                        }
                    }
                    else{
                        if(this->m_hwModule == CAM_A){
                            MUINT32 P1DoneRef = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL);
                            if(P1DoneRef & eRefCamC){
                                if ((Addr[CAM_A] != 0) &&
                                    (Addr[CAM_C] != 0)) {
                                    SET_CQ0_BA_D(CAM_A, CAM_C);
                                }
                            }
                            else if(P1DoneRef & eRefCamB){
                                if ((Addr[CAM_A] != 0) &&
                                    (Addr[CAM_B] != 0)) {
                                    SET_CQ0_BA_D(CAM_A, CAM_B);
                                }
                            }
                            else{
                                SET_CQ0_BA_S(CAM_A);
                            }
                        }
                        else if(this->m_hwModule == CAM_B){
                            MUINT32 P1DoneRef = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL);
                            if(P1DoneRef & eRefCamA){
                                if ((Addr[CAM_A] != 0) &&
                                    (Addr[CAM_B] != 0)) {
                                    SET_CQ0_BA_D(CAM_B, CAM_A);
                                }
                            }
                            else if(P1DoneRef & eRefCamC){
                                if ((Addr[CAM_B] != 0) &&
                                    (Addr[CAM_C] != 0)) {
                                    SET_CQ0_BA_D(CAM_B, CAM_C);
                                }
                            }
                            else{
                                SET_CQ0_BA_S(CAM_B);
                            }
                        }
                        else if(this->m_hwModule == CAM_C){
                            MUINT32 P1DoneRef = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL);
                            if(P1DoneRef & eRefCamA){
                                if(pDrv[CAM_A]){
                                    twin_status.Raw = CAM_READ_REG(pDrv[CAM_A],CAMCTL_R1_CAMCTL_TWIN_STATUS);
                                    if(twin_status.Bits.TWIN_EN){
                                        switch (twin_status.Bits.SLAVE_CAM_NUM) {
                                        case 1: /* 2 RAWS to 1 tg for static twin mode */
                                            if (twin_status.Bits.MASTER_MODULE == CAM_A &&
                                                twin_status.Bits.TWIN_MODULE == CAM_B) {
                                                if ((Addr[CAM_A] != 0) &&
                                                    (Addr[CAM_B] != 0) &&
                                                    (Addr[CAM_C] != 0)) {
                                                    SET_CQ0_BA_T(CAM_C, CAM_A, CAM_B);
                                                }
                                            }
                                            else {
                                                CAM_FUNC_ERR("un-support twin case, master_slave:%d_%d", twin_status.Bits.MASTER_MODULE, twin_status.Bits.TWIN_MODULE);
                                            }
                                            break;
                                        default:
                                            CAM_FUNC_ERR("un-support twin case, slave_cam_num:%d", twin_status.Bits.SLAVE_CAM_NUM);
                                            break;
                                        }
                                    }
                                    else{
                                        if ((Addr[CAM_A] != 0) &&
                                            (Addr[CAM_C] != 0)) {
                                            SET_CQ0_BA_D(CAM_C, CAM_A);
                                        }
                                    }
                                }
                            }
                            else if(P1DoneRef & eRefCamB){
                                if ((Addr[CAM_B] != 0) &&
                                    (Addr[CAM_C] != 0)) {
                                    SET_CQ0_BA_D(CAM_C, CAM_B);
                                }
                            }
                            else{
                                SET_CQ0_BA_S(CAM_C);
                            }
                        }
                        else{
                            CAM_FUNC_ERR("un-support hwModule:%d", this->m_hwModule);
                        }
                    }
                }
                else{
                    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
                }
                /* + It's for Lafi_WAM_CQ_ERR */
                cq0_data[CAM_A][0] = cq0_data[CAM_B][0] = cq0_data[CAM_C][0] = 0;
                cq0_data[CAM_A][1] = cq0_data[CAM_B][1] = cq0_data[CAM_C][1] = 0;

                if (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN)) {
                    twinAddr[this->m_hwModule] = (unsigned int)this->dma_cfg.memBuf.base_pAddr;
                    if ((twinAddr[CAM_A] != 0) && (twinAddr[CAM_B] != 0)) {
                        cq0_data[CAM_A][0] =  ISP_CAM_A_IDX;
                        cq0_data[CAM_B][0] =  ISP_CAM_B_IDX;
                        cq0_data[CAM_A][1] = twinAddr[CAM_A];
                        cq0_data[CAM_B][1] = twinAddr[CAM_B];

                        this->m_pDrv->setDeviceInfo(_NOTE_CQTHR0_BASE,(MUINT8 *)&cq0_data);
                        //CAM_FUNC_INF("cq data(%d,0x%x)(%d,0x%x)", cq0_data[CAM_A][0], cq0_data[CAM_A][1],
                        //    cq0_data[CAM_B][0], cq0_data[CAM_B][1]);
                        twinAddr[CAM_A] = twinAddr[CAM_B] = 0;
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
                /* - It's for Lafi_WAM_CQ_ERR */
            }
            break;
        default:
            CAM_FUNC_ERR("un-support DMAI:%s\n",this->name_Str());
            break;
    }


    CAM_FUNC_DBG("-\n");
    return 0;
}


MINT32 DMA_CQ0::_write2CQ(MUINT32 burstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ0::_write2CQ+ [%s]:cq(0x%x),page(0x%x),burstIndex(0x%x)\n",this->name_Str(),cq,page,burstIndex);

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
        CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_MODE,mode);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_MODE,mode);
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
        CAM_WRITE_REG(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR_INNER,this->dma_cfg.memBuf.base_pAddr);
#if TWIN_SW_P1_DONE_WROK_AROUND
        CAM_WRITE_REG(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
#endif
    }
    else{
        CAM_WRITE_REG(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,this->dma_cfg.memBuf.base_pAddr);
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
    CAM_WRITE_REG(this->m_pDrv,TWIN_CQ_THR0_ADDR,this->dma_cfg.memBuf.base_pAddr);

    return MTRUE;
}

MBOOL DMA_CQ0::setDBLoadHold(MBOOL bHold, MBOOL bPhy)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if (bHold) {
        if (bPhy) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_LOAD_HOLD,1);
        }
        else {
            this->m_pDrv->cqAddModule(CAM_CTL_MISC_);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_LOAD_HOLD,1);
        }
    }
    else {
        if (bPhy) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_LOAD_HOLD,0);
        }
        else {
            this->m_pDrv->cqAddModule(CAM_CTL_MISC_);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_LOAD_HOLD,0);
        }
    }

    return MTRUE;
}

MBOOL DMA_CQ0::updateDB_byCQ(MBOOL byCQ)
{
    if (byCQ) {
        this->m_pDrv->cqApbModule(CAM_CTL_MISC_);
    }
    else {
        this->m_pDrv->cqNopModule(CAM_CTL_MISC_);
    }

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
        CAM_FUNC_DBG("%s::TrigCQ:+ current PA:0x%x\n", this->name_Str(), CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR));

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR0_START,1);
        usleep(300);
        while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR0_START) == 1){
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
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR0_START,1);
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
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,0);
            break;
        case _outer_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,1);
            break;
        default:
            CAM_FUNC_ERR("unsupported mode:[%s]:cq(0x%x),page(0x%x),mode(0x%x)\n",this->name_Str(),cq,page,mode);
            return MFALSE;
        }
    }
    else {
        this->m_pDrv->cqAddModule(CAM_CQ_EN_);
        tmp = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN);
        CAM_FUNC_INF("DMA_CQ0::setLoadMode:[%s]:page(0x%x),cur reg:0x%x\n",this->name_Str(),page,tmp);
        CAM_WRITE_REG(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_EN,tmp);

        switch(mode){
            case _inner_:
                CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,0);
                break;
            case _outer_:
                CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,1);
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
    MUINT32 tmp;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);


    this->m_pDrv->cqAddModule(CAM_CTL_SW_DONE_SEL_);

    //signal control, can't run-time change
    //imgo
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_IMGO_R1_FIFO_FULL_DROP,tmp);

    //rrzo
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_RRZO_R1_FIFO_FULL_DROP,tmp);

    //yuvo
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_YUVO_R1_FIFO_FULL_DROP,tmp);

    //crzo_r1
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_CRZO_R1_FIFO_FULL_DROP,tmp);

    //crzo_r2
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_CRZO_R2_FIFO_FULL_DROP,tmp);

    //ufeo
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_UFEO_R1_FIFO_FULL_DROP,tmp);

    //lcso
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_LCESO_R1_FIFO_FULL_DROP,tmp);

    //eiso
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_LMVO_R1_FIFO_FULL_DROP,tmp);

    //rsso
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_RSSO_R1_FIFO_FULL_DROP,tmp);

    //ufgo
    tmp = CAM_READ_BITS(pSrcObj,FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_UFGO_R1_FIFO_FULL_DROP,tmp);

    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,bEn);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD,nSub);

    CAM_FUNC_DBG("DMA_CQ0::setDoneSubSample:[%s]:page(0x%x),cur reg:0x%x\n",this->name_Str(),page,CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE));

    return MTRUE;
}

MBOOL DMA_CQ0::disableAllDma(ISP_DRV_CAM* pSrcObj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MBOOL bEnDCIF;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);



    // if check physical is DCIF, set dummyPage all dma = 0
    bEnDCIF = (MBOOL)CAM_READ_BITS(pSrcObj, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN);
    if (bEnDCIF) {
        this->m_pDrv->cqAddModule(CAM_DMA_EN_);
        CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN, 0);
    }

    CAM_FUNC_DBG("DMA_CQ0::setDoneSubSample:[%s]:page(0x%x),cur reg:0x%x\n",this->name_Str(),page,CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE));

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
    CAM_WRITE_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER,this->m_updatecnt);

    return MTRUE;
}

UINT32 DMA_CQ0::GetCQupdateCnt(MBOOL bPhy)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(bPhy)
        return CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER);
    else
        return CAM_READ_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER);
}

MBOOL DMA_CQ1::setCQTriggerMode(E_MODE mode,void* ptr)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    if(ptr != NULL){
        CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR1_CTL,CAMCQ_CQ_THR1_MODE,mode);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR1_CTL,CAMCQ_CQ_THR1_MODE,mode);
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
        CAM_FUNC_DBG("%s::TrigCQ:+ current PA:0x%x\n", this->name_Str(), CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR1_BASEADDR));

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR1_START,1);
        usleep(300);
        while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR1_START) == 1){
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
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR1_START,1);
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
    CAM_WRITE_REG(this->m_pDrv,TWIN_CQ_THR1_ADDR,this->dma_cfg.memBuf.base_pAddr);

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

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR4_CTL,CAMCQ_CQ_THR4_MODE,mode);

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
    CAM_FUNC_DBG("DMA_CQ4::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR4_BASEADDR));
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR4_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR4_START) == 1){
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

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR5_CTL,CAMCQ_CQ_THR5_MODE,mode);

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
    CAM_FUNC_DBG("DMA_CQ5::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR5_BASEADDR));
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR5_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR5_START) == 1){
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

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR7_CTL,CAMCQ_CQ_THR7_MODE,mode);

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
    CAM_FUNC_DBG("DMA_CQ7::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR7_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR7_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR7_START) == 1){
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

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR8_CTL,CAMCQ_CQ_THR8_MODE,mode);

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

    CAM_FUNC_DBG("DMA_CQ8::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR8_BASEADDR));

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR8_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR8_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
        }
    }

    return MTRUE;
}

MINT32 DMA_CQ10::_write2CQ(MUINT32 burstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ10::_write2CQ+ [%s]:cq(0x%x),page(0x%x),burstIndex(0x%x)\n",this->name_Str(),cq,page,burstIndex);

    if(burstIndex > 0){
        return 0;
    }

    switch(this->id())
    {
        case CAM_DMA_CQ10I:
            this->m_pDrv->cqAddModule(CAM_CQ_THRE28_ADDR_CQONLY, ISP_DRV_CQ_THRE28);
            this->m_pDrv->cqAddModule(CAM_CQ_THRE28_ADDR_);
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
        CAM_WRITE_BITS(this->m_pDrv,CAMCQ_R1_CAMCQ_CQ_THR10_CTL,CAMCQ_CQ_THR10_MODE,mode);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR10_CTL,CAMCQ_CQ_THR10_MODE,mode);
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
        CAM_FUNC_DBG("%s::TrigCQ:+ current PA:0x%x\n", this->name_Str(), CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR10_BASEADDR));

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR10_START,1);
        usleep(300);
        while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR10_START) == 1){
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
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR10_START,1);
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
    CAM_WRITE_REG(this->m_pDrv,TWIN_CQ_THR10_ADDR,this->dma_cfg.memBuf.base_pAddr);

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

MBOOL DMA_CQ12::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR12_CTL,CAMCQ_CQ_THR12_MODE,mode);

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
    CAM_FUNC_DBG("DMA_CQ12::TrigCQ:+ current PA:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR12_BASEADDR));
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR12_START,1);
    usleep(300);
    while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START,CAMCTL_CQ_THR12_START) == 1){
        //wait cq loading done
        usleep(300);
        if(loop++ > 10){
            CAM_FUNC_ERR("%s::TrigCQ: RDMA fail\n", this->name_Str());
            break;
        }
    }
    return MTRUE;
}

MBOOL DMA_CQ28::setCQTriggerMode(E_MODE mode)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    CAM_FUNC_DBG("%s::setCQTriggerMode:+ mode = 0x%x\n", this->name_Str(), mode);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR28_CTL,CAMCQ_CQ_THR28_MODE,mode);

    return MTRUE;
}


MBOOL DMA_CQ28::TrigCQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 curSOF;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ28::TrigCQ:+ \n");

    //CQ28's trig source under tiwn mode is still from TG_A.
    if((this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)))
    {
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN)){
            return MTRUE;
        }
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&curSOF);
    if(this->m_prvSOF != 0xFFFF){
        MUINT32 diff = (curSOF > this->m_prvSOF)?(curSOF - this->m_prvSOF):(this->m_prvSOF - curSOF);
        MUINT32 _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD)+1;

        if(diff < (2*_tmp)){
            CAM_FUNC_ERR("CQ28 can't be trigged again when CQ28 still running %d_%d\n", this->m_prvSOF, curSOF);
            return MFALSE;
        }
    }
    this->m_prvSOF = curSOF;

    //laifte todo:this function is already phased out by hw
    //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_I2C_CQ_TRIG,TG_TG_I2C_CQ_TRIG,1);

    return MTRUE;
}

MBOOL DMA_CQ28::setCQContent(MUINTPTR arg1,MUINTPTR arg2)
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
    CAM_FUNC_DBG("DMA_CQ28::cfg:+ timing:0x%" PRIXPTR "\n",arg1);

    _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD);
    //cfg number
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_CQ_NUM,TG_TG_CQ_NUM,_tmp);
    //cfg timing
    if(arg1 == 0){
        CAM_FUNC_ERR("CQI loading at idx:0 is not supported\n");
        return MFALSE;
    }
    arg1 = (1<<(arg1-1));
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_CQ_TIMING,TG_TG_I2C_CQ_TIM,arg1);

    //
    if(ptr != NULL){
        pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * ptr->size());
        if (NULL == pReg) {
            CAM_FUNC_ERR("DMA_CQ28::setCQContent: alloc buf fail\n");
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

MINT32 DMA_CQ28::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("DMA_CQ28::_disable+ [%s]:cq(0x%x),page(0x%x)\n",this->name_Str(),cq,page);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR28_CTL,CAMCQ_CQ_THR28_EN,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_I2C_CQ_EN,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_CQ_SEL,0);//always imm mode

    this->m_prvSOF = 0xFFFF;
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    DMAO_B
/////////////////////////////////////////////////////////////////////////////*/
DMAO_B::DMAO_B()
{
    Header_Addr = 0x0;
    m_pDrv = NULL;
    m_fps = 0;
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

    pOut->clear();
    if((prof.lineT == 0) || (prof.frameT == 0)){
        CAM_FUNC_ERR("lineT | frameT can't be 0\n");
        return MFALSE;
    }
    {
        REG_CAMCTL_R1_CAMCTL_DMA_EN dma_en;
        REG_CAMCTL_R1_CAMCTL_DMA2_EN dma_en2;
        dma_en.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN);
        dma_en2.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN);

        //
        pOut->push_back(BW_l(_imgo_,dma_en.Bits.CAMCTL_IMGO_R1_EN,IMGO_R1_IMGO_XSIZE,IMGO_R1_IMGO_YSIZE));
        pOut->push_back(BW_a(_rrzo_,dma_en.Bits.CAMCTL_RRZO_R1_EN,RRZO_R1_RRZO_XSIZE,RRZO_R1_RRZO_YSIZE));
        pOut->push_back(BW_a(_ufeo_,dma_en.Bits.CAMCTL_UFEO_R1_EN,UFEO_R1_UFEO_XSIZE,UFEO_R1_UFEO_YSIZE));
        pOut->push_back(BW_a(_ufgo_,dma_en.Bits.CAMCTL_UFGO_R1_EN,UFGO_R1_UFGO_XSIZE,UFGO_R1_UFGO_YSIZE));
        pOut->push_back(BW_e(_lmvo_,dma_en.Bits.CAMCTL_LMVO_R1_EN,CAM_READ_REG(this->m_pDrv,LMVO_R1_LMVO_XSIZE)));
        pOut->push_back(BW_a(_rsso_,dma_en.Bits.CAMCTL_RSSO_R1_EN,RSSO_R1_RSSO_XSIZE,RSSO_R1_RSSO_YSIZE));
        pOut->push_back(BW_a(_lcso_,dma_en.Bits.CAMCTL_LCESO_R1_EN,LCESO_R1_LCESO_XSIZE,LCESO_R1_LCESO_YSIZE));
        pOut->push_back(BW_l(_yuvo_,dma_en2.Bits.CAMCTL_YUVO_R1_EN,YUVO_R1_YUVO_XSIZE,YUVO_R1_YUVO_YSIZE));
        pOut->push_back(BW_l(_yuvbo_,dma_en.Bits.CAMCTL_YUVBO_R1_EN,YUVBO_R1_YUVBO_XSIZE,YUVBO_R1_YUVBO_YSIZE));
        pOut->push_back(BW_l(_yuvco_,dma_en.Bits.CAMCTL_YUVCO_R1_EN,YUVCO_R1_YUVCO_XSIZE,YUVCO_R1_YUVCO_YSIZE));
        pOut->push_back(BW_a(_crzo_,dma_en.Bits.CAMCTL_CRZO_R1_EN,CRZO_R1_CRZO_XSIZE,CRZO_R1_CRZO_YSIZE));
        pOut->push_back(BW_a(_crzbo_,dma_en.Bits.CAMCTL_CRZBO_R1_EN,CRZBO_R1_CRZBO_XSIZE,CRZBO_R1_CRZBO_YSIZE));
        pOut->push_back(BW_a(_crzo_r2_,dma_en2.Bits.CAMCTL_CRZO_R2_EN,CRZO_R2_CRZO_XSIZE,CRZO_R2_CRZO_YSIZE));
        pOut->push_back(BW_a(_crzbo_r2_,dma_en2.Bits.CAMCTL_CRZBO_R2_EN,CRZBO_R2_CRZBO_XSIZE,CRZBO_R2_CRZBO_YSIZE));
        pOut->push_back(BW_a(_afo_,dma_en.Bits.CAMCTL_AFO_R1_EN,AFO_R1_AFO_XSIZE,AFO_R1_AFO_YSIZE));
        pOut->push_back(BW_a(_pdo_,dma_en.Bits.CAMCTL_PDO_R1_EN,PDO_R1_PDO_XSIZE,PDO_R1_PDO_YSIZE));
        if(dma_en.Bits.CAMCTL_IMGO_R1_EN){
            //true bw request is to search maximum bw between vsync-nonblanking and vsync-blanking, not just sum the bw of non-blanking & blanking.
            //use imgo is due to bw of imgo is always > bw of aao+pso at vsync-blanking at 99.9%.
            pOut->push_back(BW_1D(_aao_,dma_en.Bits.CAMCTL_AAO_R1_EN,AAO_R1_AAO_XSIZE,0));
        }
        else{
            pOut->push_back(BW_1D(_aao_,dma_en.Bits.CAMCTL_AAO_R1_EN,AAO_R1_AAO_XSIZE,AE_LE_BIN_SIZE(this->m_pDrv)+AE_SE_BIN_SIZE(this->m_pDrv)));
        }
        pOut->push_back(BW_1D(_flko_,dma_en.Bits.CAMCTL_FLKO_R1_EN,FLKO_R1_FLKO_XSIZE,0));
        pOut->push_back(BW_1D(_tsfso_,dma_en.Bits.CAMCTL_TSFSO_R1_EN,TSFSO_R1_TSFSO_XSIZE,0));
    }

    return MTRUE;
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
        case CAM_DMA_TSFSO:
            //clr BUF LIST
            CQ_RingBuf_ST ctrl;
            ctrl.dma_PA = 0;
            ctrl.dma_FH_PA = 0;
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
            CAM_WRITE_REG(this->m_pDrv,LMVO_R1_LMVO_XSIZE,0xFF);
            CAM_WRITE_REG(this->m_pDrv,LMVO_R1_LMVO_YSIZE,0);
            CAM_WRITE_REG(this->m_pDrv,LMVO_R1_LMVO_STRIDE,0xFF + 1);

            break;
        case CAM_DMA_FLKO:
            CAM_WRITE_REG(this->m_pDrv,FLKO_R1_FLKO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,FLKO_R1_FLKO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,FLKO_R1_FLKO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_RSSO:
            {
                Header_RSSO fh_rsso;
                fh_rsso.Header_Enque(Header_RSSO::E_DST_SIZE, this->m_pDrv, ((this->dma_cfg.size.h << 16) | this->dma_cfg.size.xsize));
                CAM_WRITE_REG(this->m_pDrv,RSSO_R1_RSSO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,RSSO_R1_RSSO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,RSSO_R1_RSSO_STRIDE,this->dma_cfg.size.stride);
            }
            break;
        case CAM_DMA_AFO:
            {
                Header_AFO fh_afo;

                fh_afo.Header_Enque(Header_AFO::E_STRIDE,this->m_pDrv,this->dma_cfg.size.stride);

                CAM_WRITE_REG(this->m_pDrv,AFO_R1_AFO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,AFO_R1_AFO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,AFO_R1_AFO_STRIDE,this->dma_cfg.size.stride);
            }
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_REG(this->m_pDrv,AAO_R1_AAO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,AAO_R1_AAO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,AAO_R1_AAO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_TSFSO:
            CAM_WRITE_REG(this->m_pDrv,TSFSO_R1_TSFSO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,TSFSO_R1_TSFSO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,TSFSO_R1_TSFSO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_REG(this->m_pDrv,LCESO_R1_LCESO_XSIZE,this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,LCESO_R1_LCESO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,LCESO_R1_LCESO_STRIDE,this->dma_cfg.size.stride);

            break;
        case CAM_DMA_IMGO:
            {
                MUINT32 new_crop_x = 0;
                Header_IMGO fh_imgo;
                E_PIX pixMode = _1_pix_;
                capibility CamInfo;
                tCAM_rst rst;

                if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
                {
                    if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                        break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                    }
                }

                CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_STRIDE,this->dma_cfg.size.stride);

                switch(this->dma_cfg.lIspColorfmt){
                    case NSCam::eImgFmt_UFO_BAYER8:
                    case NSCam::eImgFmt_UFO_BAYER10:
                    case NSCam::eImgFmt_UFO_BAYER12:
                        //imgo can't do crop when compressed
                        CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_CROP,0);
                        break;
                    default:
                        //
                        this->dma_cfg.bus_size = BusSizeCal(pixMode);

                        if (CamInfo.GetCapibility(
                             NSImageio::NSIspio::EPortIndex_IMGO,
                             NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                             NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->dma_cfg.lIspColorfmt,
                                                                                this->dma_cfg.crop.x, MAP_CAM_PIXMODE(pixMode,CAM_FUNC_INF)), rst, E_CAM_UNKNOWNN) == MFALSE) {
                            CAM_FUNC_ERR("cropping size is not fit hw constraint(%d_%d)\n",this->dma_cfg.crop.x,rst.crop_x);
                            return -1;
                        }
                        new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);
                        CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));
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
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
            {
                if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                    break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                }
            }

            CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_YSIZE,this->dma_cfg.size.h - 1);//ySize;
            CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_STRIDE,this->dma_cfg.size.stride);//stride;

            switch(this->dma_cfg.lIspColorfmt){
                case NSCam::eImgFmt_UFO_FG_BAYER8:
                case NSCam::eImgFmt_UFO_FG_BAYER10:
                case NSCam::eImgFmt_UFO_FG_BAYER12:
                    //imgo can't do crop when compressed
                    CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_CROP,0);
                    break;
                default:
                    //  We use RRZ to corp, thus almost the RRZO crop width & height is 0
                    //CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_CROP, (this->dma_cfg.crop.y << 16) | GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size));
                    if(this->dma_cfg.crop.y || this->dma_cfg.crop.x)
                        CAM_FUNC_ERR("rrzo support no cropping , replace dmao cropping start by 0\n");
                    break;
            }
            break;
        case CAM_DMA_UFEO:
            {
                Header_UFEO fh_ufeo;

                fh_ufeo.Header_Enque(Header_UFEO::E_UFEO_XSIZE,this->m_pDrv,this->dma_cfg.size.xsize);  //AU size don't need to -1

                CAM_WRITE_REG(this->m_pDrv,UFEO_R1_UFEO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,UFEO_R1_UFEO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,UFEO_R1_UFEO_STRIDE,this->dma_cfg.size.stride);
                //CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_UFE_CON,0x00000001);  //If set this, UFG will not compress, means that compress ratio = 100%
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_UFGO fh_ufgo;

                fh_ufgo.Header_Enque(Header_UFGO::E_UFGO_XSIZE,this->m_pDrv,this->dma_cfg.size.xsize ); //AU size don't need to -1

                CAM_WRITE_REG(this->m_pDrv,UFGO_R1_UFGO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,UFGO_R1_UFGO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,UFGO_R1_UFGO_STRIDE,this->dma_cfg.size.stride);
            }
            break;
        case CAM_DMA_PDO:
            {
                Header_PDO fh_pdo;

                fh_pdo.Header_Enque(Header_PDO::E_STRIDE,this->m_pDrv,this->dma_cfg.size.stride);

                CAM_WRITE_REG(this->m_pDrv,PDO_R1_PDO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,PDO_R1_PDO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,PDO_R1_PDO_STRIDE,this->dma_cfg.size.stride);
                if((this->dma_cfg.size.stride % 16) != 0){
                    CAM_FUNC_ERR("PDO stride must be 16-alignment\n");
                }
            }
            break;
        case CAM_DMA_YUVO:
            {
                MUINT32 new_crop_x = 0;
                Header_YUVO fh_yuvo;
                E_PIX pixMode = _1_pix_;
                capibility CamInfo;
                tCAM_rst rst;

                if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
                {
                    if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                        break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                    }
                }

                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_STRIDE,this->dma_cfg.size.stride);

                // only could be one pixel mode, and so as bus_size is 1
                pixMode = _1_pix_;
                this->dma_cfg.bus_size = 1;

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_YUVO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->dma_cfg.lIspColorfmt,
                                                                    this->dma_cfg.crop.x, MAP_CAM_PIXMODE(pixMode,CAM_FUNC_INF)), rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("cropping size is not fit hw constraint(%d_%d)\n",this->dma_cfg.crop.x,rst.crop_x);
                    return -1;
                }
                new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x,this->dma_cfg.pixel_byte,this->dma_cfg.bus_size);
                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));
                //reverse for FH below
                new_crop_x = GetCropXUnitPixel(new_crop_x,this->dma_cfg.pixel_byte,this->dma_cfg.bus_size);

                //frame header enque
                fh_yuvo.Header_Enque(Header_YUVO::E_CROP_START,this->m_pDrv,((this->dma_cfg.crop.y << 16) | new_crop_x));
                fh_yuvo.Header_Enque(Header_YUVO::E_CROP_SIZE,this->m_pDrv,((this->dma_cfg.crop.h << 16) | this->dma_cfg.crop.w));
            }
            break;
        case CAM_DMA_YUVBO:
            {
                MUINT32 new_crop_x = 0;
                Header_YUVBO fh_yuvbo;
                E_PIX pixMode = _1_pix_;
                capibility CamInfo;
                tCAM_rst rst;

                if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
                {
                    if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                        break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                    }
                }

                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_STRIDE,this->dma_cfg.size.stride);

                // only could be one pixel mode, and so as bus_size is 1
                pixMode = _1_pix_;
                this->dma_cfg.bus_size = 1;

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_YUVBO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->dma_cfg.lIspColorfmt,
                                                                    this->dma_cfg.crop.x, MAP_CAM_PIXMODE(pixMode,CAM_FUNC_INF)), rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("cropping size is not fit hw constraint(%d_%d)\n",this->dma_cfg.crop.x,rst.crop_x);
                    return -1;
                }
                new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x,this->dma_cfg.pixel_byte,this->dma_cfg.bus_size);
                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));
                //reverse for FH below
                new_crop_x = GetCropXUnitPixel(new_crop_x,this->dma_cfg.pixel_byte,this->dma_cfg.bus_size);

                //frame header enque
                fh_yuvbo.Header_Enque(Header_YUVBO::E_CROP_START,this->m_pDrv,((this->dma_cfg.crop.y << 16) | new_crop_x));
                fh_yuvbo.Header_Enque(Header_YUVBO::E_CROP_SIZE,this->m_pDrv,((this->dma_cfg.crop.h << 16) | this->dma_cfg.crop.w));
            }
            break;
        case CAM_DMA_YUVCO:
            {
                MUINT32 new_crop_x = 0;
                Header_YUVCO fh_yuvco;
                E_PIX pixMode = _1_pix_;
                capibility CamInfo;
                tCAM_rst rst;

                if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
                {
                    if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                        break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                    }
                }

                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_XSIZE,this->dma_cfg.size.xsize - 1);
                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_YSIZE,this->dma_cfg.size.h - 1);
                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_STRIDE,this->dma_cfg.size.stride);

                // only could be one pixel mode, and so as bus_size is 1
                pixMode = _1_pix_;
                this->dma_cfg.bus_size = 1;

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_YUVCO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->dma_cfg.lIspColorfmt,
                                                                    this->dma_cfg.crop.x, MAP_CAM_PIXMODE(pixMode,CAM_FUNC_INF)), rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("cropping size is not fit hw constraint(%d_%d)\n",this->dma_cfg.crop.x,rst.crop_x);
                    return -1;
                }
                new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x,this->dma_cfg.pixel_byte,this->dma_cfg.bus_size);
                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));
                //reverse for FH below
                new_crop_x = GetCropXUnitPixel(new_crop_x,this->dma_cfg.pixel_byte,this->dma_cfg.bus_size);

                //frame header enque
                fh_yuvco.Header_Enque(Header_YUVCO::E_CROP_START,this->m_pDrv,((this->dma_cfg.crop.y << 16) | new_crop_x));
                fh_yuvco.Header_Enque(Header_YUVCO::E_CROP_SIZE,this->m_pDrv,((this->dma_cfg.crop.h << 16) | this->dma_cfg.crop.w));
            }
            break;
        case CAM_DMA_CRZO_R1:
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
            {
                if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                    break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                }
            }

            CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_STRIDE,this->dma_cfg.size.stride);

            //crzo_r1 support no crop
            CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_CROP,0);
            break;
        case CAM_DMA_CRZBO_R1:
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
            {
                if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                    break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                }
            }

            CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_STRIDE,this->dma_cfg.size.stride);

            //crzbo_r1 support no crop
            CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_CROP,0);
            break;
        case CAM_DMA_CRZO_R2:
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
            {
                if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                    break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                }
            }

            CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_STRIDE,this->dma_cfg.size.stride);

            //crzo_r2 support no crop
            CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_CROP,0);
            break;
        case CAM_DMA_CRZBO_R2:
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN))
            {
                if(this->m_hwModule == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE)) {
                    break;//twin's cofig is via twin_drv. add this if is for preventing meaningless error log,twin_mgr have no these info.
                }
            }

            CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_XSIZE, this->dma_cfg.size.xsize - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_YSIZE,this->dma_cfg.size.h - 1);
            CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_STRIDE,this->dma_cfg.size.stride);

            //crzbo_r2 support no crop
            CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_CROP,0);
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
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LMVO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_LMVO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_LMVO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_LMVO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_FLKO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_FLKO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_FLKO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_FLKO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_FLKO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_RSSO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RSSO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_RSSO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_RSSO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_RSSO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_AFO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_AFO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_AFO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_AFO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AAO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_AAO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_AAO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_AAO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_TSFSO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_TSFSO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_TSFSO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_TSFSO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_TSFSO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LCESO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_LCESO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_LCESO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_LCESO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_IMGO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_IMGO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_IMGO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_IMGO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_RRZO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_RRZO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_RRZO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_UFEO:
            {
                Header_IMGO fh_imgo;
                fh_imgo.Header_Enque(Header_IMGO::E_IS_UFE_FMT,this->m_pDrv,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN3,CAMCTL_UFE_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_PAK_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_IMGO_SEL,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_UFEO_R1_DONE_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFO_IMGO_R1_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_UFEO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_UFEO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IS_UFG_FMT,this->m_pDrv,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_UFG_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_PAKG_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_UFEG_SEL,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_UFGO_R1_DONE_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFOG_RRZO_R1_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_UFGO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_UFGO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_PDO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_PDO_R1_DONE_EN,1);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_PDO_R1,1);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_PDO_R1_LAST_ULTRA_EN,1);
            break;
        case CAM_DMA_YUVO:
            {
                MUINT32 enYUVPlane;
                Header_YUVO fh_yuvo;
                enYUVPlane = fh_yuvo.GetRegInfo(Header_YUVO::E_PLANE_EN,this->m_pDrv);
                fh_yuvo.Header_Enque(Header_YUVO::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x1));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_YUVO_R1_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_YUVO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_YUVO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_YUVBO:
            {
                MUINT32 enYUVPlane;
                Header_YUVO fh_yuvo;
                enYUVPlane = fh_yuvo.GetRegInfo(Header_YUVO::E_PLANE_EN,this->m_pDrv);
                fh_yuvo.Header_Enque(Header_YUVO::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x2));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_YUVBO_R1_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_YUVBO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_YUVBO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_YUVCO:
            {
                MUINT32 enYUVPlane;
                Header_YUVO fh_yuvo;
                enYUVPlane = fh_yuvo.GetRegInfo(Header_YUVO::E_PLANE_EN,this->m_pDrv);
                fh_yuvo.Header_Enque(Header_YUVO::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x4));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_YUVCO_R1_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_YUVCO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_YUVCO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_CRZO_R1:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R1 fh_crzo_r1;
                enYUVPlane = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x1));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZO_R1_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_CRZBO_R1:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R1 fh_crzo_r1;
                enYUVPlane = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x2));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZBO_R1_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZBO_R1,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZBO_R1_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_CRZO_R2:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R2 fh_crzo_r2;
                enYUVPlane = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x1));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZO_R2_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZO_R2,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZO_R2_LAST_ULTRA_EN,1);
            }
            break;
        case CAM_DMA_CRZBO_R2:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R2 fh_crzo_r2;
                enYUVPlane = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv,(enYUVPlane|0x2));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZBO_R2_DONE_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZBO_R2,1);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZBO_R2_LAST_ULTRA_EN,1);
            }
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
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LMVO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_LMVO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_LMVO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_LMVO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_FLKO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_FLKO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_FLKO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_FLKO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_FLKO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_RSSO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RSSO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_RSSO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_RSSO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_RSSO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_AFO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_AFO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_AFO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_AFO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_AAO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AAO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_AAO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_AAO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_AAO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_TSFSO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_TSFSO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_TSFSO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_TSFSO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_TSFSO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_LCSO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LCESO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_LCESO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_LCESO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_LCESO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_IMGO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_IMGO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_IMGO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_IMGO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_RRZO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_RRZO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_RRZO_R1,0);


            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_RRZO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_UFEO:
            {
                Header_IMGO fh_imgo;
                fh_imgo.Header_Enque(Header_IMGO::E_IS_UFE_FMT,this->m_pDrv,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN3,CAMCTL_UFE_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_PAK_R1_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_UFEO_R1_DONE_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFO_IMGO_R1_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_UFEO_R1,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_IMGO_SEL,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_UFEO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IS_UFG_FMT,this->m_pDrv,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_UFG_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_PAKG_R1_EN,1);

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_UFGO_R1_DONE_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFOG_RRZO_R1_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_UFGO_R1,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_UFEG_SEL,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_UFGO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_PDO:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_PDO_R1_DONE_EN,0);

            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_PDO_R1,0);

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_PDO_R1_LAST_ULTRA_EN,0);
            break;
        case CAM_DMA_YUVO:
            {
                MUINT32 enYUVPlane;
                Header_YUVO fh_yuvo;
                enYUVPlane = fh_yuvo.GetRegInfo(Header_YUVO::E_PLANE_EN,this->m_pDrv);
                fh_yuvo.Header_Enque(Header_YUVO::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFE));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_YUVO_R1_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_YUVO_R1,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_YUVO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_YUVBO:
            {
                MUINT32 enYUVPlane;
                Header_YUVO fh_yuvo;
                enYUVPlane = fh_yuvo.GetRegInfo(Header_YUVO::E_PLANE_EN,this->m_pDrv);
                fh_yuvo.Header_Enque(Header_YUVO::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFD));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_YUVBO_R1_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_YUVBO_R1,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_YUVBO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_YUVCO:
            {
                MUINT32 enYUVPlane;
                Header_YUVO fh_yuvo;
                enYUVPlane = fh_yuvo.GetRegInfo(Header_YUVO::E_PLANE_EN,this->m_pDrv);
                fh_yuvo.Header_Enque(Header_YUVO::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFB));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_YUVCO_R1_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_YUVCO_R1,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_YUVCO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_CRZO_R1:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R1 fh_crzo_r1;
                enYUVPlane = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFE));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZO_R1_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZO_R1,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_CRZBO_R1:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R1 fh_crzo_r1;
                enYUVPlane = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFD));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZBO_R1_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZBO_R1,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZBO_R1_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_CRZO_R2:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R2 fh_crzo_r2;
                enYUVPlane = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFE));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZO_R2_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZO_R2,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZO_R2_LAST_ULTRA_EN,0);
            }
            break;
        case CAM_DMA_CRZBO_R2:
            {
                MUINT32 enYUVPlane;
                Header_CRZO_R2 fh_crzo_r2;
                enYUVPlane = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv);
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_PLANE_EN,this->m_pDrv,(enYUVPlane&0xFFFFFFFD));

                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_CRZBO_R2_DONE_EN,0);

                CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_CRZBO_R2,0);

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_LAST_ULTRA_EN1,CAMDMATOP1_CRZBO_R2_LAST_ULTRA_EN,0);
            }
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
MINT32 DMAO_B::_write2CQ(MUINT32 burstIndex)
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
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_LMVO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_LMVO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_LMVO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LMVO_SPARE_PA);
            }
            break;
        case CAM_DMA_FLKO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_FLKO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_FLKO_SPARE_CQ0_);
            }
            break;
        case CAM_DMA_RSSO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_RSSO_A_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_RSSO_A_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_RSSO_A_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RSSO_A_SPARE_PA);
            }
            break;
        case CAM_DMA_IMGO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_X_);
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_Y_);
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_STRIDE_);
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_CROP_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_CROP);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_PA);
                this->m_pDrv->cqAddModule(CAM_ISP_PAK_);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA);
                //this->m_pDrv->cqAddModule(CAM_DMA_IMGO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_IMGO_Y_);
                //this->m_pDrv->cqAddModule(CAM_DMA_IMGO_CROP_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_);
                //this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_CROP);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_IMGO_SPARE_PA);
            }
            break;
        case CAM_DMA_RRZO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_X_);
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_Y_);
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_STRIDE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_CROP);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA);
                //this->m_pDrv->cqAddModule(CAM_DMA_RRZO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_RRZO_Y_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_);
                //this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_CROP);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_RRZO_SPARE_PA);
            }
            break;
        case CAM_DMA_UFEO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_UFEO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA);
                //this->m_pDrv->cqAddModule(CAM_DMA_UFEO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFEO_SPARE_PA);
            }
            break;
        case CAM_DMA_UFGO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_UFGO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_UFGO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_UFGO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_UFGO_BA);
                //this->m_pDrv->cqAddModule(CAM_DMA_UFGO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_UFGO_SPARE_PA);
            }
            break;
        case CAM_DMA_AFO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_AFO_);
                this->m_pDrv->cqAddModule(CAM_DMA_AFO_BA_OFST);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_AFO_SPARE_CQ0_);
            }
            break;
        case CAM_DMA_AAO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_AAO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_AAO_SPARE_CQ0_);
            }
            break;
        case CAM_DMA_TSFSO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_TSFSO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_TSFSO_SPARE_CQ0_);
            }
            break;
        case CAM_DMA_LCSO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_LCSO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_LCSO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_LCSO_BA);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_LCSO_SPARE_PA);
            }
            break;
        case CAM_DMA_PDO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_PDO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_PDO_SPARE_CQ0_);
            }
            break;
        case CAM_DMA_YUVO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_YUVO_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVO_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVO_CROP_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_YUVO_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_YUVO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVO_SPARE_PA);
            }
            break;
        case CAM_DMA_YUVBO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_YUVBO_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVBO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVBO_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVBO_CROP_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVBO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVBO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVBO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_YUVBO_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_YUVBO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVBO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVBO_SPARE_PA);
            }
            break;
        case CAM_DMA_YUVCO:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_YUVCO_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVCO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVCO_);
                this->m_pDrv->cqAddModule(CAM_DMA_YUVCO_CROP_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVCO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVCO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVCO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_YUVCO_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_YUVCO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVCO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_YUVCO_SPARE_PA);
            }
            break;
        case CAM_DMA_CRZO_R1:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_CRZO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_SPARE_PA);
            }
            break;
        case CAM_DMA_CRZBO_R1:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_SPARE_PA);
            }
            break;
        case CAM_DMA_CRZO_R2:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_R2_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_R2_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_R2_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_R2_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_R2_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_R2_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_CRZO_R2_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_CRZO_R2_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_R2_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZO_R2_SPARE_PA);
            }
            break;
        case CAM_DMA_CRZBO_R2:
            if(burstIndex == 0){
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_R2_BA_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_R2_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_R2_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_R2_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_R2_SPARE_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_R2_SPARE_PA);
            }
            else{
                this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_R2_BA_);
                //this->m_pDrv->cqAddModule(CAM_DMA_CRZBO_R2_BA_OFST_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_R2_);
                this->m_pDrv->cqAddModule(CAM_DMA_FH_CRZBO_R2_SPARE_PA);
            }
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

                CAM_WRITE_REG(this->m_pDrv,LMVO_R1_LMVO_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,LMVO_R1_LMVO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,LMVO_R1_LMVO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_FLKO:
            {
                Header_FLKO fh_flko;
                CQ_RingBuf_ST bufctrl;
                if (CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_SUB_RATIO)) {
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
                CAM_WRITE_REG(this->m_pDrv,RSSO_R1_RSSO_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,RSSO_R1_RSSO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,RSSO_R1_RSSO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_AFO:
            {
                Header_AFO  fh_afo;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_SUB_RATIO))
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
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_SUB_RATIO))
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
        case CAM_DMA_TSFSO:
            {
                Header_TSFSO  fh_tsfso;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_SUB_RATIO))
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
                fh_tsfso.Header_Enque(Header_TSFSO::E_IMG_PA,(ISP_DRV_CAM*)bufctrl.pDma_fh_spare,this->dma_cfg.memBuf.base_pAddr);
            }
            break;
        case CAM_DMA_LCSO:
            {
                Header_LCSO fh_lcso;
                fh_lcso.Header_Enque(Header_LCSO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,LCESO_R1_LCESO_OFST_ADDR,0x0);
                CAM_WRITE_REG(this->m_pDrv,LCESO_R1_LCESO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,LCESO_R1_LCESO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_IMGO:
            {
                Header_IMGO fh_imgo;
                fh_imgo.Header_Enque(Header_IMGO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                Header_UFEO fh_ufeo;
                fh_ufeo.Header_Enque(Header_UFEO::E_IMGO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,IMGO_R1_IMGO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_RRZO:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.Header_Enque(Header_RRZO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                Header_UFGO fh_ufgo;
                fh_ufgo.Header_Enque(Header_UFGO::E_RRZO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,RRZO_R1_RRZO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_UFEO:
            {
                Header_UFEO fh_ufeo;
                fh_ufeo.Header_Enque(Header_UFEO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                fh_ufeo.Header_Enque(Header_UFEO::E_UFEO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,UFEO_R1_UFEO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,UFEO_R1_UFEO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,UFEO_R1_UFEO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_UFGO:
            {
                Header_UFGO fh_ufgo;
                fh_ufgo.Header_Enque(Header_UFGO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);
                fh_ufgo.Header_Enque(Header_UFGO::E_UFGO_OFFST_ADDR,this->m_pDrv,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,UFGO_R1_UFGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);
                CAM_WRITE_REG(this->m_pDrv,UFGO_R1_UFGO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);

                CAM_WRITE_REG(this->m_pDrv,UFGO_R1_UFGO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        case CAM_DMA_PDO:
            {
                Header_PDO  fh_pdo;
                CQ_RingBuf_ST bufctrl;
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_SUB_RATIO))
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
        case CAM_DMA_YUVO:
            {
                Header_YUVO fh_yuvo;
                fh_yuvo.Header_Enque(Header_YUVO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,YUVO_R1_YUVO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_YUVBO:
            {
                Header_YUVBO fh_yuvbo;
                fh_yuvbo.Header_Enque(Header_YUVBO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,YUVBO_R1_YUVBO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_YUVCO:
            {
                Header_YUVCO fh_yuvco;
                fh_yuvco.Header_Enque(Header_YUVCO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,YUVCO_R1_YUVCO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_CRZO_R1:
            {
                Header_CRZO_R1 fh_crzo_r1;
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZO_R1_CRZO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_CRZBO_R1:
            {
                Header_CRZBO_R1 fh_crzbo_r1;
                fh_crzbo_r1.Header_Enque(Header_CRZBO_R1::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZBO_R1_CRZBO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_CRZO_R2:
            {
                Header_CRZO_R2 fh_crzo_r2;
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZO_R2_CRZO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
            }
            break;
        case CAM_DMA_CRZBO_R2:
            {
                Header_CRZBO_R2 fh_crzbo_r2;
                fh_crzbo_r2.Header_Enque(Header_CRZBO_R2::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_OFST_ADDR,this->dma_cfg.memBuf.ofst_addr);
                CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                CAM_WRITE_REG(this->m_pDrv,CRZBO_R2_CRZBO_FH_BASE_ADDR,this->Header_Addr);//because header BA will be reference offeset addr directly by HW
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
    MBOOL bDbn, bBin;
    REG_CAMCTL_R1_CAMCTL_SEL ctl_sel;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;
    REG_CAMCTL_R1_CAMCTL_FMT2_SEL fmt2_sel;
    REG_CAMCTL_R1_CAMCTL_EN2 ctl_en2;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    ctl_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL);
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL);
    fmt2_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL);
    ctl_en2.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2);

    switch(fmt_sel.Bits.CAMCTL_TG_FMT){
        case 3: //YUV
        case 7: //JPG
            pixMode = (E_PIX)(fmt_sel.Bits.CAMCTL_PIX_BUS_TGO);
            if(pixMode == _4_pix_)
                return 3;
            else
                return 1;
            break;
        default: //BAYER
            switch(ctl_sel.Bits.CAMCTL_IMGO_SEL){
                case eUFESel_1: //UFE
                    CAM_FUNC_ERR("hw have no this path\n");
                    break;
                case eUFESel_2: //TG
                    pixMode = (E_PIX)(fmt_sel.Bits.CAMCTL_PIX_BUS_TGO);
                    switch(pixMode){
                        case _2_pix_: return 3; break;
                        case _4_pix_: return 7; break;
                        default: return 1; break;
                    }
                    break;
                default: //PAK
                    switch(ctl_sel.Bits.CAMCTL_IMG_SEL){
                        case eIMGSel_1:
                            pixMode = (E_PIX)(fmt_sel.Bits.CAMCTL_PIX_BUS_TGO);
                            switch(pixMode){
                                case _2_pix_: return 3; break;
                                case _4_pix_: return 7; break;
                                default: return 1; break;
                            }
                            break;
                        case eIMGSel_2:
                        case eIMGSel_0:
                            if(ctl_sel.Bits.CAMCTL_CRP_R3_SEL == 0){
                                pixMode = (E_PIX)(fmt2_sel.Bits.CAMCTL_PIX_BUS_RAW_SELO);
                            }
                            else if(ctl_sel.Bits.CAMCTL_CRP_R3_SEL == 6){
                                bDbn = ctl_en2.Bits.CAMCTL_DBN_R1_EN;
                                pixMode = (E_PIX)(fmt2_sel.Bits.CAMCTL_PIX_BUS_RAW_SELO >> bDbn);
                            }
                            else if(ctl_sel.Bits.CAMCTL_CRP_R3_SEL == 7){
                                bDbn = ctl_en2.Bits.CAMCTL_DBN_R1_EN;
                                bBin = ctl_en2.Bits.CAMCTL_BIN_R1_EN;
                                pixMode = (E_PIX)(fmt2_sel.Bits.CAMCTL_PIX_BUS_RAW_SELO >> (bDbn + bBin));
                            }
                            else{
                                pixMode = _1_pix_;
                            }
                            switch(pixMode){
                                case _2_pix_: return 3; break;
                                case _4_pix_: return 7; break;
                                default: return 1; break;
                            }
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

    if( CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN) == 1){
        REG_CAMCTL_R1_CAMCTL_SEL ctl_sel;
        ctl_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL);

        if (ctl_sel.Bits.CAMCTL_IMG_SEL == 1) { //pure raw path
            if (CAM_READ_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_MODE, TG_DBL_DATA_BUS) == 1 &&
                CAM_READ_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_MODE, TG_DBL_DATA_BUS1) == 1) {
                    CAM_FUNC_WRN("UFE can't be enabled under 4 pixel mode. Disable UFE/UFEO");
                    enable = MFALSE;
                    goto EXIT;
                }
        }

        if (ctl_sel.Bits.CAMCTL_IMGO_SEL == 0 || ctl_sel.Bits.CAMCTL_IMGO_SEL == 2) {
            //imgo's xsize need to change back to normal-bayer if CAMCTL_IMGO_SEL want to be 0
            //for design convenience, just return err
            CAM_FUNC_WRN("UFE can't be enabled under pak path or pure raw path. Disable UFE/UFEO");
            enable = MFALSE;
            goto EXIT;
        }

        if(CAM_READ_REG(this->m_pDrv, IMGO_R1_IMGO_CROP) != 0){
            CAM_FUNC_WRN("UFE can't be enabled under IMGO crop!= 0(0x%x)\n",CAM_READ_REG(this->m_pDrv, IMGO_R1_IMGO_CROP));

            enable = MFALSE;
            goto EXIT;
        }

        if((CAM_READ_BITS(this->m_pDrv, IMGO_R1_IMGO_YSIZE,IMGO_YSIZE)+1) != m_img_size.h){
            CAM_FUNC_WRN("UFE can't be enabled under IMGO vieritcal crop(0x%x_0x%x)\n",CAM_READ_BITS(this->m_pDrv, IMGO_R1_IMGO_YSIZE,IMGO_YSIZE),m_img_size.h);
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
                capibility CamInfo;
                DMA_IMGO Imgo;
                tCAM_rst rst;
                REG_TG_R1_TG_SEN_GRAB_PXL TG_W;

                E_PIX pixMode = _1_pix_;

                switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_IMGO_R1_FMT)) {
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
                        CAM_FUNC_ERR("Unsupport fmt:%d", CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_IMGO_R1_FMT));
                        return;
                }

                if (ctl_sel.Bits.CAMCTL_IMGO_SEL == 2 || (ctl_sel.Bits.CAMCTL_IMGO_SEL != 2 && ctl_sel.Bits.CAMCTL_IMG_SEL == 1)) {//pure raw path
                    pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_TGO));
                    Imgo.dma_cfg.size.w = m_img_size.w;

                } else {
                    MBOOL bDbn, bBin, bFbnd;
                    MUINT32 crpr3_sel;
                    bDbn = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_DBN_R1_EN);
                    bBin = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
                    bFbnd = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_FBND_R1_EN);

                    if (ctl_sel.Bits.CAMCTL_IMG_SEL == 2) { //From MRGr3
                        crpr3_sel = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL);
                        pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_MRG_R3O));

                        if (crpr3_sel == 0) {
                            Imgo.dma_cfg.size.w = m_img_size.w;
                        } else if (crpr3_sel == 6) {
                            Imgo.dma_cfg.size.w = m_img_size.w;
                        } else if (crpr3_sel == 7) {
                            Imgo.dma_cfg.size.w = m_img_size.w;
                        } else {
                            Imgo.dma_cfg.size.w = m_img_size.w >> (bDbn + bBin + bFbnd);
                        }
                    } else if (ctl_sel.Bits.CAMCTL_IMG_SEL == 0) { //From CRPr3
                        crpr3_sel = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL);

                        if (crpr3_sel == 0) {
                            pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO));
                        } else if (crpr3_sel == 6) {
                            pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO) >> bDbn);
                        } else if (crpr3_sel == 7) {
                            pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO) >> (bDbn + bBin));
                        } else {
                            pixMode = (E_PIX)(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_SEPO) >> bFbnd);
                        }

                        Imgo.dma_cfg.size.w = (CAM_READ_BITS(this->m_pDrv,CRP_R3_CRP_X_POS,CRP_XEND) -
                                               CAM_READ_BITS(this->m_pDrv,CRP_R3_CRP_X_POS,CRP_XSTART) + 1) * (pixMode + 1); //Because CRPr3 control use cycle counts not pixel counts.
                    }
                }

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_IMGO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)Imgo.dma_cfg.lIspColorfmt, Imgo.dma_cfg.size.w, (E_CamPixelMode)(pixMode+1)),
                    rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("img x_byte size err(0x%x)\n", Imgo.dma_cfg.size.w);
                }
                CAM_WRITE_REG(this->m_pDrv, IMGO_R1_IMGO_XSIZE, rst.xsize_byte[0]-1);
            }
        }
    }
}

MVOID DMA_UFEO::update_AU(void)
{
    Header_UFEO fh_ufeo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    fh_ufeo.Header_Enque(Header_UFEO::E_UFEO_XSIZE,this->m_pDrv,CAM_READ_BITS(this->m_pDrv,UFEO_R1_UFEO_XSIZE,UFEO_XSIZE)+1);
}

MVOID DMA_UFGO::check(MBOOL& bUF_OFF)
{
    MBOOL enable = MTRUE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    bUF_OFF = MFALSE;

    if( CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN) == 1){
        Header_RRZO rrzo_fh;
        MUINT32 size_v;
        size_v = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        size_v = (size_v>>16) & 0xffff;
        //only have twin's constraint. maintain in twinmgr
        if (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_UFEG_SEL) == 0) {
            //imgo's xsize need to change back to normal-bayer if CAMCTL_IMGO_SEL want to be 0
            //for design convenience, just return err
            CAM_FUNC_WRN("UFEG can't be enabled under pak path. Disable UFEG/UFGO");
            enable = MFALSE;
            goto EXIT;
        }

        if(CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD) < 512 ){
            CAM_FUNC_WRN("UFEG can't be enabled when rrz output width < 512(%d)\n",CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD));
            enable = MFALSE;
            goto EXIT;
        }

        if(CAM_READ_REG(this->m_pDrv, RRZO_R1_RRZO_CROP) != 0){
            CAM_FUNC_WRN("UFEG can't be enabled under RRZO crop!= 0(0x%x)\n",CAM_READ_REG(this->m_pDrv, RRZO_R1_RRZO_CROP));
            enable = MFALSE;
            goto EXIT;
        }

        if((CAM_READ_BITS(this->m_pDrv, RRZO_R1_RRZO_YSIZE,RRZO_YSIZE)+1) != size_v){
            CAM_FUNC_WRN("UFEG can't be enabled under RRZO crop(0x%x)\n",CAM_READ_BITS(this->m_pDrv, RRZO_R1_RRZO_YSIZE,RRZO_YSIZE));
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
                capibility CamInfo;
                DMA_RRZO Rrzo;
                tCAM_rst rst;
                E_CamPixelMode pixMode = ePixMode_1; //always be fixed in 1-pix mode.

                Rrzo.dma_cfg.size.w = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD);

                switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_RRZO_R1_FMT)) {
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
                        CAM_FUNC_ERR("Unsupport fmt:%d", CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_RRZO_R1_FMT));
                        return;
                }

                if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_RRZO,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)Rrzo.dma_cfg.lIspColorfmt, Rrzo.dma_cfg.size.w, pixMode),
                    rst, E_CAM_UNKNOWNN) == MFALSE) {
                    CAM_FUNC_ERR("rrz x_byte size err(0x%x)\n", Rrzo.dma_cfg.size.w);
                }

                CAM_WRITE_REG(this->m_pDrv, RRZO_R1_RRZO_XSIZE, rst.xsize_byte[0]-1);
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
    fh_ufgo.Header_Enque(Header_UFGO::E_UFGO_XSIZE,this->m_pDrv,CAM_READ_BITS(this->m_pDrv,UFGO_R1_UFGO_XSIZE,UFGO_XSIZE)+1);
}

