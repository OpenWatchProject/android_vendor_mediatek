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

#define LOG_TAG "ifunc_cam"

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


#include <cutils/properties.h>  // For property_get().

#include "isp_function_cam.h"

#include "cam_capibility.h"


#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

EXTERN_DBG_LOG_VARIABLE(func_cam);

#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST
#define CAM_FUNC_VRB(fmt, arg...)        do { if (func_cam_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    MUINT32 module,cq,page;\
    this->m_pDrv->getCurObjInfo((ISP_HW_MODULE*)&module,(E_ISP_CAM_CQ*)&cq,&page);\
    if (func_cam_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,module, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    MUINT32 module,cq,page;\
    this->m_pDrv->getCurObjInfo((ISP_HW_MODULE*)&module,(E_ISP_CAM_CQ*)&cq,&page);\
    if (func_cam_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,module, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do { if (func_cam_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    MUINT32 module,cq,page;\
    this->m_pDrv->getCurObjInfo((ISP_HW_MODULE*)&module,(E_ISP_CAM_CQ*)&cq,&page);\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,module, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_cam_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


FP_SEN SENINF_DBG::m_fp_Sen = NULL;
Mutex PIPE_CHECK::m_lock[PHY_CAM];


MBOOL PIPE_CHECK::Check_Start(void)
{
    struct ISP_RAW_INT_STATUS err_status;
    MUINT32 rst = 0;
    ISP_HW_MODULE module;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    CAM_REG_CTL_TWIN_STATUS twin_status;
    struct ISP_GET_CLK_INFO getClkInfo;

    if(this->m_pDrv == NULL){
        BASE_LOG_ERR("can't be NULL ptr, no pipechk\n");
        return MFALSE;
    }

    this->m_pDrv->getCurObjInfo(&module,&cq,&page);
    if(module >= PHY_CAM){
        CAM_FUNC_ERR("unsupported module, no pipehck\n");
        return MFALSE;
    }

    Mutex::Autolock lock(PIPE_CHECK::m_lock[module]);

    twin_status.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS);
    CAM_FUNC_ERR("current twin is %d\n",twin_status.Bits.TWIN_EN);

    if( twin_status.Bits.MASTER_MODULE != module){
        MUINT32 tmp = 0;
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.camsys.dumpreg", value, "0");
        tmp = atoi(value);
        if(tmp){
            MUINT32 slave;
            MUINT32 TwinNum;
            ISP_DRV_CAM* ptr[PHY_CAM-1];
            memset((void*)ptr, 0, sizeof(ptr));
            TwinNum = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM);
            ptr[0] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,MASTER_MODULE),ISP_DRV_CQ_THRE0,0,"TWIN_DUMP");
            if(ptr[0]){
                ptr[0]->DumpReg(MTRUE);
                ptr[0]->destroyInstance();
            }
            for(MUINT32 i=0;i<TwinNum;i++){
                if(i==0){
                    slave = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE);
                    CAM_FUNC_ERR("1st slave cam:%d\n",slave);
                }
                else if(i==1){
                    slave = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TRIPLE_MODULE);
                    CAM_FUNC_ERR("2nd slave cam:%d\n",slave);
                }

                CAM_FUNC_ERR("twin is enabled, start dump cam_%d register\n",slave);
                ptr[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)slave,ISP_DRV_CQ_THRE0,0,"TWIN_DUMP");
                if(ptr[i]){
                    ptr[i]->DumpReg(MTRUE);
                    ptr[i]->destroyInstance();
                }
            }
        }

        CAM_FUNC_ERR("can't start pipechk with slave cam:%d\n",module);
        goto EXIT;
    }

    //err status:
    err_status.ispInt3Err = err_status.ispIntErr = 0;
    this->m_pDrv->getDeviceInfo(_GET_INT_ERR,(MUINT8 *)&err_status);
    if(err_status.ispIntErr == 0){
    }
    else{
        CAM_FUNC_ERR("accumulated err int_status:0x%x, int3_status:0x%x\n", err_status.ispIntErr, err_status.ispInt3Err);
        rst += 1;
    }

    if ((this->m_pDrv->getDeviceInfo(_GET_CUR_ISP_CLOCK, (MUINT8*)&getClkInfo)) == MFALSE) {
        CAM_FUNC_ERR("Fail to Get ISP CLK\n");
    }
    else{
        CAM_FUNC_ERR("current ISP clk :%d,latest expected ISP clk :%d\n",getClkInfo.curClk, getClkInfo.targetClk);
    }


    //master cam's check
    if( this->LSC_CHECK() == MTRUE){
        CAM_FUNC_INF("LSC/LSCI check pass!\n");
    }
    else{
        rst += 1;
    }

    if( this->RRZ_CHECK() == MTRUE){
        CAM_FUNC_INF("RRZ check pass!\n");
    }
    else{
        rst += 1;
    }

    if(this->XCPN_CHECK() == MTRUE){
        CAM_FUNC_INF("CPN/DCPN check pass\n");
    }
    else{
        rst += 1;
    }

    if( this->AF_CHECK(twin_status.Bits.TWIN_EN) == MTRUE){
        CAM_FUNC_INF("master af/afo check pass!\n");
    }
    else
        rst += 1;

    if(this->UFEG_CHECK() == MTRUE){
        CAM_FUNC_INF("UFEG check pass\n");
    }
    else{
        rst += 1;
    }

    if(this->UFE_CHECK() == MTRUE){
        CAM_FUNC_INF("UFE check pass\n");
    }
    else{
        rst += 1;
    }

    if(twin_status.Bits.TWIN_EN == MFALSE){
    }
    else{
        if(this->TWIN_CHECK() == MFALSE)
            rst += 1;
        else{
            CAM_FUNC_INF("Twin pass\n");
        }
    }

    //
    if(this->PATH_CHECK(twin_status.Bits.TWIN_EN)){
        CAM_FUNC_INF("path check pass\n");
    }
    else
        rst += 1;

    if(this->ME_CHECK()){
        CAM_FUNC_INF("no mutually exclusive function enabled\n");
    }
    else
        rst += 1;

    if(this->DMAO_STATUS(err_status.ispIntErr)){
        CAM_FUNC_INF("find no dma err\n");
    }
    else
        rst += 1;

    if( this->AA_CHECK() == MTRUE){
        CAM_FUNC_INF("AA/AAO check pass!\n");
    }
    else
        rst += 1;

    if( this->LCS_CHECK() == MTRUE){
        CAM_FUNC_INF("LCS/LCSO check pass!\n");
    }
    else
        rst += 1;


    if(this->PDO_CHECK() == MTRUE){
        CAM_FUNC_INF("pdo check pass!\n");
    }
    else
        rst += 1;

    if(this->BNR_CHECK() == MTRUE){
        CAM_FUNC_INF("bnr check pass!\n");
    }
    else
        rst += 1;


    if( this->PS_CHECK() == MTRUE){
        CAM_FUNC_INF("PS/PSO check pass!\n");
    }
    else
        rst += 1;

    if( this->FLK_CHECK() == MTRUE){
        CAM_FUNC_INF("FLK/FLKO check pass!\n");
    }
    else
        rst += 1;

    if( this->LMV_CHECK() == MTRUE){
        CAM_FUNC_INF("LMV/LMVO check pass!\n");
    }
    else
        rst += 1;

    if( this->RSS_CHECK() == MTRUE){
        CAM_FUNC_INF("RSS/RSSO check pass!\n");
    }
    else
        rst += 1;

    if( this->TG_CHECK(err_status.ispIntErr) == MTRUE){
        CAM_FUNC_INF("TG check pass!\n");
    }
    else
        rst += 1;

    if( rst == 0){
        if(this->ENQUE_CHECK() == MFALSE){
            //goto EXIT;
        }
        else{
            CAM_FUNC_ERR("find no err, plz look for previous err ,like enque flow err\n");
        }
    }


    //dump phy reg
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
        CAM_FUNC_ERR("start dump master CAM register\n");
        this->m_pDrv->DumpReg(MTRUE);
        if(twin_status.Bits.TWIN_EN == 1){
            MUINT32 slave;
            MUINT32 TwinNum;
            ISP_DRV_CAM* ptr[PHY_CAM-1];
            memset((void*)ptr, 0, sizeof(ptr));
            TwinNum = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM);
            for(MUINT32 i=0;i<TwinNum;i++){
                if(i==0){
                    slave = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE);
                    CAM_FUNC_ERR("1st slave cam:%d\n",slave);
                }
                else if(i==1){
                    slave = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TRIPLE_MODULE);
                    CAM_FUNC_ERR("2nd slave cam:%d\n",slave);
                }

                CAM_FUNC_ERR("twin is enabled, start dump cam_%d register\n",slave);
                ptr[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)slave,ISP_DRV_CQ_THRE0,0,"TWIN_DUMP");
                if(ptr[i]){
                    ptr[i]->DumpReg(MTRUE);
                    ptr[i]->destroyInstance();
                }
            }
        }
    }
    else{
        CAM_FUNC_ERR("viewfinder off,no reg dump.\n");
        rst = MFALSE;
    }

EXIT:

    if(rst)
        return MFALSE;
    else
        return MTRUE;
}

MBOOL PIPE_CHECK::PATH_CHECK(MBOOL bTwin)
{
    MBOOL rst = MTRUE;
    MUINT32 module;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    UniDrvImp* pUni =  (UniDrvImp*)UniDrvImp::createInstance(UNI_A);
    MUINT32 pattern = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DATA_PATTERN);
    this->m_pDrv->getCurObjInfo((ISP_HW_MODULE*)&module,&cq,&page);

    CAM_FUNC_WRN("current image pattern: %d\n",pattern);
    switch(pattern){
        case eCAM_NORMAL:
            break;
        case eCAM_DUAL_PIX:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN) == MFALSE){
                rst = MFALSE;
                CAM_FUNC_ERR("dbn should be enable under DPD\n");
            }
            break;
        case eCAM_4CELL:
            if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN) * \
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,VBN_EN)) == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("dbn & vbn should be enable under 4cell_normal\n");
            }
            break;
        case eCAM_MONO:
            break;
        case eCAM_IVHDR:
            break;
        case eCAM_ZVHDR:
            break;
        case eCAM_4CELL_IVHDR:
            break;
        case eCAM_4CELL_ZVHDR:
            break;
        case eCAM_YUV:
            break;
        default:
            rst = MFALSE;
            CAM_FUNC_ERR("unsupported pattern:%d\n",pattern);
            break;
    }

    switch(module){
        case CAM_A:
        case CAM_B:
            break;
        default:
            CAM_FUNC_ERR("unsupported CAM:%d\n",module);
            rst = MFALSE;
            break;
    }



    //uni check
    if(UNI_READ_BITS(pUni,CAM_UNI_TOP_MOD_EN,RLB_A_EN) == MFALSE){
        CAM_FUNC_ERR("RLB should be enabled\n");
        rst = MFALSE;
    }
    pUni->destroyInstance();


    //cq check
    if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_START) != 0){
        CAM_FUNC_ERR("CQ SMI request error\n");
        rst = MFALSE;
    }

    //dmx check
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,DMX_SEL) != 0){
        CAM_FUNC_ERR("master dmx_sel err\n");
        rst = MFALSE;
    }
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID) != module){
        CAM_FUNC_ERR("master dmx_id err(%d)\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID));
        rst = MFALSE;
    }

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,PDO_SEL) == 1){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN) == 0){
            rst = MFALSE;
            CAM_FUNC_ERR("is pattern Dual PD?\n");
        }
    }


    if(bTwin == MFALSE){//interactive function with other cam
        IspDrv* ptr[PHY_CAM-1];
        MUINT32 mod = PHY_CAM;

        for(MUINT32 i=0;i<(PHY_CAM-1);i++){
            ptr[i] = (IspDrv*)IspDrvImp::createInstance( (ISP_HW_MODULE)((module+i+1)%mod));

            if(ptr[i] != NULL){
                if( CAM_READ_BITS(ptr[i],CAM_CTL_FMT_SEL,DMX_ID) == 0){
                    if( CAM_READ_BITS(ptr[i],CAM_CTL_SEL,DMX_SEL) == 1){
                        CAM_FUNC_ERR("module:%d dmx_sel+dmx_id err\n",((module+i+1)%mod));
                        rst = MFALSE;
                    }
                }
            }
        }


        for(MUINT32 i=0;i<(PHY_CAM-1);i++){
            if(ptr[i])
                ptr[i]->destroyInstance();
        }
    }
    else{//twin case will be checked at TWIN_CHECK()
    }



    //
    return rst;
}


MBOOL PIPE_CHECK::ME_CHECK(void)
{
    MBOOL rst = MTRUE;
    CAM_REG_CTL_EN  en1;
    CAM_REG_CTL_EN2 en2;

    en1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN);
    en2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN2);


    if(en2.Bits.ADBS_EN || en1.Bits.DBS_EN){
        if(en2.Bits.ADBS_EN == en1.Bits.DBS_EN){
            CAM_FUNC_ERR("DBS/ADBS are mutually exclusive func(%d_%d)\n",en1.Bits.DBS_EN,en2.Bits.ADBS_EN);
            rst = MFALSE;
        }
    }

    if(en1.Bits.BIN_EN || en1.Bits.DBN_EN ){
        if(en1.Bits.BIN_EN == en1.Bits.DBN_EN){
            CAM_FUNC_ERR("BIN/DBN are mutually exclusive func(%d_%d)\n",en1.Bits.BIN_EN,en1.Bits.DBN_EN);
            rst = MFALSE;
        }
    }

    if((en1.Bits.DBN_EN == 1) && (en2.Bits.SCM_EN ==1)){
        CAM_FUNC_ERR("SCM+DBN -> pattern error\n");
        rst = MFALSE;
    }

    if((en1.Bits.BIN_EN == 1) && (en2.Bits.VBN_EN ==1)){
        CAM_FUNC_ERR("BIN+VBN -> pattern error\n");
        rst = MFALSE;
    }


    return rst;
}

MBOOL PIPE_CHECK::XCPN_CHECK(void)
{
    MBOOL rst = MTRUE;
    CAM_REG_CTL_EN  en1;
    CAM_REG_CTL_EN2 en2;
    MUINT32 in_w,in_h;


    en1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN);
    en2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN2);

    if(en2.Bits.CPN_EN != en2.Bits.DCPN_EN){
        CAM_FUNC_ERR("CPN/DCPN must be all on or all off(%d_%d)\n",en2.Bits.CPN_EN,en2.Bits.DCPN_EN);
        rst = MFALSE;
    }
    else{
        if( (en2.Bits.DCPN_EN == 1) && (en1.Bits.LSC_EN == 0)){
            CAM_FUNC_ERR("CPN/DCPN/LSC must be all on or all off(%d_%d_%d)\n",en2.Bits.CPN_EN,en2.Bits.DCPN_EN,en1.Bits.LSC_EN);
            rst = MFALSE;
        }
    }


    //get size
    this->InPutSize_DMX(in_w,in_h);

    if(en2.Bits.DCPN_EN){
        //dcpn
        if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_DCPN_IN_IMG_SIZE,DCPN_IN_IMG_W) != in_w){
            CAM_FUNC_ERR("DCPN Width err: %d_%d\n",in_w,CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_DCPN_IN_IMG_SIZE,DCPN_IN_IMG_W));
            rst = MFALSE;
        }
        if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_DCPN_IN_IMG_SIZE,DCPN_IN_IMG_H) != in_h){
            CAM_FUNC_ERR("DCPN Hight err: %d_%d\n",in_h,CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_DCPN_IN_IMG_SIZE,DCPN_IN_IMG_H));
            rst = MFALSE;
        }
    }

    if(en2.Bits.CPN_EN){
        //cpn
        if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CPN_IN_IMG_SIZE,CPN_IN_IMG_W) != in_w){
            CAM_FUNC_ERR("CPN Width err: %d_%d\n",in_w,CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CPN_IN_IMG_SIZE,CPN_IN_IMG_W));
            rst = MFALSE;
        }
        if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CPN_IN_IMG_SIZE,CPN_IN_IMG_H) != in_h){
            CAM_FUNC_ERR("CPN Hight err: %d_%d\n",in_h,CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CPN_IN_IMG_SIZE,CPN_IN_IMG_H));
            rst = MFALSE;
        }
    }


    return rst;
}

/**
    check whether this fail is caused by some dmao waiting for enque or not
    no rsso check
*/
MBOOL PIPE_CHECK::ENQUE_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 imgo_en,rrzo_en,eiso_en,lcso_en,rsso_en,ufeo_en;
    MUINT32 imgo_fbc,rrzo_fbc,eiso_fbc,lcso_fbc,rsso_fbc,ufeo_fbc;
    CAM_FUNC_ERR("########## TO_DO Fix Cannon_ep ##########\n");
    imgo_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,IMGO_EN);
    rrzo_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,RRZO_EN);
    ufeo_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFEO_EN);
    lcso_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LCSO_EN);
#if 0
    if(this->m_pUniDrv){
        /* cannon_ep reg *///eiso_en = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,EISO_A_EN);
        /* cannon_ep reg *///rsso_en = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,RSSO_A_EN);
    }
    else{
        eiso_en = rsso_en = 0;
    }
#endif
    if(imgo_en){
        imgo_fbc = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL2,FBC_CNT);
        if(imgo_fbc == 0){
            CAM_FUNC_ERR("IMGO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(rrzo_en){
        rrzo_fbc = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL2,FBC_CNT);
        if(rrzo_fbc == 0){
            CAM_FUNC_ERR("RRZO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(ufeo_en){
        ufeo_fbc = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL2,FBC_CNT);
        if(ufeo_fbc == 0){
            CAM_FUNC_ERR("UFEO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(lcso_en){
        lcso_fbc = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL2,FBC_CNT);
        if(lcso_fbc == 0){
            CAM_FUNC_ERR("LCSO have no enque record\n");
            rst = MFALSE;
        }
    }

    #if 0//cannon_ep reg
    if(eiso_en){
        eiso_fbc = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL2,FBC_CNT);
        if(eiso_fbc == 0){
            CAM_FUNC_ERR("EISO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(rsso_en){
        rsso_fbc = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL2,FBC_CNT);
        if(rsso_fbc == 0){
            CAM_FUNC_ERR("RSSO have no enque record\n");
            rst = MFALSE;
        }
    }
    #endif


    return rst;
}


MBOOL PIPE_CHECK::DMAO_STATUS(MUINT32 err_status)
{
    #define str_leng    (400)
    MUINT32 dma_err[_cam_max_];
    char add[str_leng] = {"\0"};
    char str[32] = {"\0"};
    char* ptr = add;
    char const* dma_str[_cam_max_] = {"_imgo_","_rrzo_","_ufeo_","_aao_","_afo_","_lcso_","_pdo_","_lmvo_","_flko_","_rsso_","_pso_","_ufgo_",
        "_bpci_","_lsci_","_rawi_","_pdi_"};
    MBOOL rst = MTRUE;
    UniDrvImp*      pUni =  (UniDrvImp*)UniDrvImp::createInstance(UNI_A);
    CAM_UNI_REG_TOP_DMA_INT_STATUS uni_dma;
    CAM_REG_CTL_RAW_INT3_STATUS int3_status;

    #define str_append(fmt) {\
        sprintf(str,fmt);\
        if(( str_leng - strlen(add) - 1) > strlen(str)){\
            ptr += sprintf(ptr,"%s",str);\
        }\
        else{\
            CAM_FUNC_ERR("string overflow\n");\
        }\
    }

    int3_status.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_RAW_INT3_STATUS);

    if(err_status & DMA_ERR_EN_){//check cam's dma
        this->m_pDrv->getDeviceInfo(_GET_DMA_ERR,(MUINT8 *)dma_err);

        for(MUINT32 i=0;i<_cam_max_;i++){
            if(dma_err[i] & 0xffff){
                sprintf(str,"%s ERR:0x%x ",dma_str[i],dma_err[i]);
                if(( str_leng - strlen(add) - 1) > strlen(str)){
                    ptr += sprintf(ptr,"%s",str);
                }
                else{
                    CAM_FUNC_ERR("string overflow\n");
                }
            }
        }


        CAM_FUNC_ERR("[dmao status:0x%x]%s\n",\
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN),add);

        //check fifo full
        add[0] = '\0';
        if(err_status & RRZO_ERR_EN_){
            str_append("rrzo-f,");
        }
        if(err_status & AFO_ERR_EN_){
            str_append("afo-f,");
        }
        if(err_status & IMGO_ERR_EN_){
            str_append("imgo-f,");
        }
        if(err_status & AAO_ERR_EN_){
            str_append("aao-f,");
        }
        if(err_status & PSO_ERR_EN_){
            str_append("pso-f,");
        }
        if(err_status & LCSO_ERR_EN_){
            str_append("lcso-f,");
        }
        if(err_status & UFGO_ERR_EN_){
            str_append("ufgo-f,");
        }
        if(err_status & UFEO_ERR_EN_){
            str_append("ufeo-f,");
        }
        if(err_status & PDO_ERR_EN_){
            str_append("pdo-f,");
        }
        //
        if(int3_status.Bits.FLKO_ERR_ST){
            str_append("flko-f,");
        }
        if(int3_status.Bits.LMVO_ERR_ST){
            str_append("lmvo-f,");
        }
        if(int3_status.Bits.RSSO_ERR_ST){
            str_append("rsso-f,");
        }

        CAM_FUNC_ERR("[dmao fifo-F]%s\n",add);
        rst = MFALSE;
    }

    for (MUINT32 _hw = 0; _hw < 2; _hw++) {
        for (MUINT32 twice = 0; twice < 2; twice++) {
            CAM_FUNC_ERR("cqvs: hw:0x%x loop:%d ##################################\n", _hw, twice);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DBG_SET,0x2100);
            CAM_FUNC_ERR("cqvs: dbg_set=0x%x dbg_port=0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DBG_SET),
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DBG_PORT));

            UNI_WRITE_BITS(pUni,CAM_UNI_TOP_DBG_SET,DEBUG_TOP_SEL,_hw);
            CAM_FUNC_ERR("cqvs: uni_top_dbg_set=0x%x\n", UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_SET));

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x0b);
            for (MUINT32 val = 0; val < 8; val++) {
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,val);
                CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                    CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));
            }

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0x01);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,0x02);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x00);
            CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0x00);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,0x01);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x00);
            CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0x00);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,0x03);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x00);
            CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0x01);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,0x02);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x01);
            CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0x00);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,0x01);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x01);
            CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,SUB_MODULE_SEL,0x00);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,R_W_DMA_TOP_SEL,0x03);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL,DMA_TOP_SEL,0x01);
            CAM_FUNC_ERR("cqvs: dma_dbg_sel=0x%x top_dbg_port=0x%x\n",
                CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_DEBUG_SEL), UNI_READ_REG(pUni,CAM_UNI_TOP_DBG_PORT));

        }
    }

    //check CQ
    if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_START)){
        CAM_FUNC_ERR("CQ can't read data from DRAM\n");
        rst = MFALSE;
    }

    //case for UNI
    uni_dma.Raw = UNI_READ_REG(pUni,CAM_UNI_TOP_DMA_INT_STATUS);
    if(uni_dma.Bits.DMA_ERR_INT_STATUS){
        CAM_FUNC_ERR("UNI have DMA err: ADL_DON:%d,ADL_ERR:%d\n",uni_dma.Bits.ADL_A_DON_INT_STATUS,uni_dma.Bits.ADL_A_ERR_INT_STATUS);
        rst = MFALSE;
    }



    pUni->destroyInstance();
    return rst;
}

MBOOL PIPE_CHECK::UFE_CHECK(void)
{
    MBOOL rst = MTRUE;


    //
    if(( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFE_EN) != \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFEO_EN)) || \
        (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFE_EN) != \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,UFO_IMGO_EN))){

        CAM_FUNC_ERR("ufe/ufeo/specail_ufe must be the same (%d_%d_%d)\n",\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFE_EN),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFEO_EN),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,UFO_IMGO_EN));
        rst = MFALSE;
    }


    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFEO_EN)){
        CAM_REG_CTL_SEL sel;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        sel.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SEL);
        //
        if (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_EN)) {
            if(sel.Bits.IMG_SEL == 2){
                CAM_FUNC_ERR("UFE can't be enabled under IMGO-Twin mode. Disable UFE/UFEO");
                rst = MFALSE;
            }
        }

        if (sel.Bits.UFE_SEL == 0 || sel.Bits.UFE_SEL == 2) {
            //imgo's xsize need to change back to normal-bayer if UFE_SEL want to be 0
            //for design convenience, just return err
            CAM_FUNC_ERR("UFE can't be enabled under pak path or pure raw path. Disable UFE/UFEO");
            rst = MFALSE;
        }

        if(CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_IMGO_CROP) != 0){
            CAM_FUNC_ERR("UFE can't be enabled under IMGO crop!= 0(0x%x)\n",CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_IMGO_CROP));
            rst = MFALSE;
        }

        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_IMGO_YSIZE,YSIZE)+1) != (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S)){
            CAM_FUNC_ERR("UFE can't be enabled under IMGO vieritcal crop(0x%x)\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_IMGO_YSIZE,YSIZE));
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_IMGO_STRIDE,STRIDE)%16) != 0){
            CAM_FUNC_ERR("stride should be 16B align\n");
            rst = MFALSE;
        }
        if((( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_IMGO_BASE_ADDR,BASE_ADDR) + CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_IMGO_OFST_ADDR,OFFSET_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_IMGO_FH_BASE_ADDR,BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        //
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFEO_STRIDE,STRIDE)%8) != 0){
            CAM_FUNC_ERR("stride should be 8B align\n");
            rst = MFALSE;
        }
        if(( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFEO_XSIZE,XSIZE)+1 )%8) != 0){
            CAM_FUNC_ERR("xsize should be 8B align\n");
            rst = MFALSE;
        }
        if(((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFEO_BASE_ADDR,BASE_ADDR) + CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFEO_OFST_ADDR,OFFSET_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFEO_FH_BASE_ADDR,BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if (sel.Bits.IMG_SEL == 1) { //pure raw path
            if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1) ) == 2){
                CAM_FUNC_ERR("can't support 4-pix mode\n");
                rst = MFALSE;
            }
        }
    }


    if(rst == MFALSE){
        CAM_FUNC_ERR("UFE check fail\n");
    }
    return rst;
}

MBOOL PIPE_CHECK::UFEG_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 ufeg_sel;

    ufeg_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,UFEG_SEL);
    //
    if(( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFEG_EN) != \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFGO_EN)) || \
        (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFEG_EN) != \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,UFOG_RRZO_EN))){

        CAM_FUNC_ERR("ufeg/ufgo/specail_ufg must be the same (%d_%d_%d)\n",\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFEG_EN),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFGO_EN),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,UFOG_RRZO_EN));
        rst = MFALSE;
    }



    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFGO_EN)){
        Header_RRZO rrzo_fh;
        MUINT32 size_v;
        size_v = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv->getPhyObj());
        size_v = (size_v>>16) & 0xffff;


        if(CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_RRZO_CROP) != 0){
            CAM_FUNC_ERR("UFEG can't be enabled under RRZO crop!= 0(0x%x)\n",CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_RRZO_CROP));
            rst = MFALSE;
        }

        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_RRZO_YSIZE,YSIZE)+1) != size_v){
            CAM_FUNC_ERR("UFEG can't be enabled under RRZO crop(0x%x)\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_RRZO_YSIZE,YSIZE));
            rst = MFALSE;
        }

        if(ufeg_sel != 1){
            CAM_FUNC_ERR("UFEO opened but ufeg_sel is %d\n",ufeg_sel);
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RRZO_STRIDE,STRIDE)%16) != 0){
            CAM_FUNC_ERR("stride should be 16B align\n");
            rst = MFALSE;
        }
        if(( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RRZO_BASE_ADDR,BASE_ADDR) + CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RRZO_OFST_ADDR,OFFSET_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RRZO_FH_BASE_ADDR,BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFGO_STRIDE,STRIDE)%8) != 0){
            CAM_FUNC_ERR("stride should be 8B align\n");
            rst = MFALSE;
        }
        if((( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFGO_BASE_ADDR,BASE_ADDR) + CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) %16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_UFGO_FH_BASE_ADDR,BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1) ) == 2){
            CAM_FUNC_ERR("can't support 4-pix mode\n");
            rst = MFALSE;
        }
    }


    if(rst == MFALSE){
        CAM_FUNC_ERR("UFEG check fail:cur mux:0x%x\n",ufeg_sel);
    }
    return rst;
}


MBOOL PIPE_CHECK::QBN1_CHECK(void)
{
    MBOOL rst = MTRUE;
    CAM_REG_QBN1_MODE qbn1;

    qbn1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE);
    //
    if(qbn1.Bits.QBN_ACC != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM)){
        rst = MFALSE;
        CAM_FUNC_ERR("qbn1's acc mode error. (%d_%d)\n",qbn1.Bits.QBN_ACC,\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM));
    }

    //
    if( (qbn1.Bits.QBN_ACC == 0) && CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,QBIN1_EN) ){
        rst = MFALSE;
        CAM_FUNC_ERR("qbn1 supported no 1pix mode\n");
    }
    if(rst == MFALSE){
        CAM_FUNC_ERR("qbn1 check fail\n");
    }

    return rst;
}

MBOOL PIPE_CHECK::AA_CHECK(void)
{
    MUINT32 BinNum[AE_HIST_NUM] = {0};
    MUINT32 aa_in_h,aa_in_v,aa_start_x,aa_start_y,_cnt=0;
    MUINT32 aa_isize_h,aa_isize_v;
    MUINT32 aa_win_h=0,aa_win_v=0;
    MUINT32 aa_win_size_h,aa_win_size_v;
    MUINT32 aa_xsize = 0,_tmp,_tmp1,_tmp2;
    MUINT32 qbn_pix=0;
    MBOOL   rst = MTRUE;
    CAM_REG_AE_STAT_EN stt_en;

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,AA_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AAO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable AAO without enable AA\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    //
    rst = this->QBN1_CHECK();

    //ae additional func.
    stt_en.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AE_STAT_EN);

    //win number
    aa_win_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_NUM,AWB_W_HNUM);
    aa_win_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_NUM,AWB_W_VNUM);
    if((aa_win_h>AA_WIN_H) || (aa_win_v>AA_WIN_V)){
        CAM_FUNC_ERR("win num must be < (%d_%d),%d_%d\n",AA_WIN_H,AA_WIN_V,aa_win_h,aa_win_v);
        rst = MFALSE;
    }

    //aa start coordinate
    aa_start_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_ORG,AWB_W_HORG);
    aa_start_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_ORG,AWB_W_VORG);

    //aa input size , form amx output, do not just use AMX output size , it will be re-org by twin driver.
    this->InPutSize_TG(aa_in_h,aa_in_v);

    //if QBIN1 enable , hsize /2
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,QBIN1_EN)){
        //pix mode
        if((qbn_pix = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE,QBN_ACC)))
            aa_in_h = aa_in_h >> qbn_pix;
    }

    //org check
    if(aa_start_x > aa_in_h || aa_start_y > aa_in_v){
        CAM_FUNC_ERR("org over image size , h:0x%x_0x%x v:0x%x_0x%x!!\n", aa_start_x, aa_in_h, aa_start_y, aa_in_v);
        rst = MFALSE;
    }

    //aa statistic operation input size
    aa_isize_h = aa_in_h - aa_start_x;
    aa_isize_v = aa_in_v - aa_start_y;
    //aa win/pit size, win size <= pit size
    aa_win_size_h = (aa_isize_h / aa_win_h) /4 * 4;
    aa_win_size_v = (aa_isize_v / aa_win_v) /2 * 2;

    //h-win check
    if(aa_win_size_h < 4){
        CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",aa_win_size_h);
        rst = MFALSE;
    }
    else{
        MUINT32 pit_size,win_size;
        char _str[32];
        pit_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_PIT,AWB_W_HPIT);
        win_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE);
        //pitch & window check
        _tmp = pit_size;
        snprintf(_str, 31,"pit ");
        for(MUINT32 i=0;i<2;i++){
            //
            if(_tmp & 0x3){
                //won't crash
                CAM_FUNC_ERR("%s: size:0x%x[h-min is 4-alignment]!!,otherwise overexpcnt will be 0\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp < 4){
                CAM_FUNC_ERR("%s: size:0x%x[h-min is 4 ]!!\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp > aa_win_size_h){

                CAM_FUNC_ERR("%s size need <= ideal size, [0x%x_0x%x]\n",_str,_tmp,aa_win_size_h);
                rst = MFALSE;
            }
            //
            _tmp = win_size;
            snprintf(_str, 31,"win ");
        }
        //
        if( pit_size < win_size){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",pit_size,win_size);
            rst = MFALSE;
        }

        //image h_size check
        if(aa_start_x + aa_win_h * win_size > aa_in_h){
            CAM_FUNC_ERR("image h_size check error , 0x%x_0x%x_0x%x_0x%x!!\n", aa_start_x, aa_win_h, win_size, aa_in_h);
            rst = MFALSE;
        }
    }

    //v-win check
    if(aa_win_size_v < 2){
        CAM_FUNC_ERR("ideal v_win size:0x%x[v-min is 2] , underflow!!\n",aa_win_size_v);
        rst = MFALSE;
    }else{
        MUINT32 pit_size,win_size;
        char _str[32];
        pit_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_PIT,AWB_W_VPIT);
        win_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE);
        //pitch & window check
        _tmp = pit_size;
        snprintf(_str, 31,"pit ");
        for(MUINT32 i=0;i<2;i++){
            //
            if(_tmp & 0x1){
                //won't crash
                CAM_FUNC_ERR("%s: size:0x%x[v-min is  2-alignment]!!,otherwise overexpcnt will be 0\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp < 2){
                CAM_FUNC_ERR("%s: size:0x%x[v-min is 2 ]!!\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp > aa_win_size_v){

                CAM_FUNC_ERR("%s size need <= ideal size, [0x%x_0x%x]\n",_str,_tmp,aa_win_size_v);
                rst = MFALSE;
            }
            //
            _tmp = win_size;
            snprintf(_str, 31,"win ");
        }
        //
        if( pit_size < win_size){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",pit_size,win_size);
            rst = MFALSE;
        }

        //image v_size check
        if(aa_start_y + aa_win_v * win_size > aa_in_v){
            CAM_FUNC_ERR("image v_size check error , 0x%x_0x%x_0x%x_0x%x!!\n", aa_start_y, aa_win_v, win_size, aa_in_v);
            rst = MFALSE;
        }
    }

    //win pix number check
    if( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE) * \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE)) > 2047 ){
        CAM_FUNC_ERR("can't have over 2047pix in each window, data overflow\n");
        rst = MFALSE;
    }

    //pixel cnt check
    //lumi of accumulation will be affected if pix_cnt bias too much
    _tmp = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE) * \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE) ) / 4;
    _tmp1 = ((1<<24) + (_tmp>>1)) / _tmp;
    _tmp2 = ((1<<24) + _tmp) / (_tmp<<1);

    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_PIXEL_CNT0,AWB_PIXEL_CNT0);
    if(_cnt != _tmp1){
        CAM_FUNC_ERR("awb pix cnt0 cur_tar:0x%x_0x%x, may have defect on stt data quality(may not HW crash)\n",_cnt,_tmp);
        rst = MFALSE;
    }
    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_PIXEL_CNT1,AWB_PIXEL_CNT1);
    if(_cnt != _tmp2){
        CAM_FUNC_ERR("awb pix cnt1 cur_tar:0x%x_0x%x, may have defect on stt data quality(may not HW crash)\n",_cnt,_tmp2);
        rst = MFALSE;
    }
    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_PIXEL_CNT2,AWB_PIXEL_CNT2);
    if(_cnt != _tmp1){
        CAM_FUNC_ERR("awb pix cnt2 cur_tar:0x%x_0x%x, may have defect on stt data quality(may not HW crash)\n",_cnt,_tmp);
        rst = MFALSE;
    }


    aa_xsize += (aa_win_h*aa_win_v*AWB_WIN);//awb , 4 bytes each win
    aa_xsize += (aa_win_h*aa_win_v*AE_WIN);// ae , 1 byte each win
    //
    if(stt_en.Bits.AE_HDR_STAT_EN){
        aa_xsize += (aa_win_h*aa_win_v*HDR_WIN);    // 1 byte each win
    }
    if(stt_en.Bits.AE_OVERCNT_EN){
        aa_xsize += (aa_win_h*aa_win_v*AE_OE_WIN); // 1 byte each win
    }
    if(stt_en.Bits.AE_TSF_STAT_EN){
        aa_xsize += (aa_win_h*aa_win_v*TSF_WIN); // 4 bytes each win
    }
    if(stt_en.Bits.AE_LINEAR_STAT_EN){
        aa_xsize += (aa_win_h*aa_win_v*STAT_EN); //12 bytes each win
    }

    //ae hist
    {
        #define ROI_CHK(x,validy,validx) {\
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST##x##_YRNG,AE_PIX_Y_HI_##x) \
                > (aa_in_v/validy)){\
                CAM_FUNC_ERR("ROI of hst_%d:Y over aainput size_%d\n",_cnt,(aa_in_v/validy));\
                rst = MFALSE;\
            }\
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST##x##_XRNG,AE_PIX_X_HI_##x) \
                > (aa_in_h/validx)){\
                CAM_FUNC_ERR("ROI of hst_%d:X over aainput size_%d\n",_cnt,(aa_in_h/validx));\
                rst = MFALSE;\
            }\
        }

        //HW issue.
        //Bin_mode_0 donimate all other bins' bin_number.
        //if Bin_mode_0 = 1, bin_mode_1 = 0 => bin_mode_1 will be also 256 bin number , but content is 128 bin's result. (content will have error)
#if 1
        BinNum[0] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET,AE_PIX_BIN_MODE_0);
        for(_cnt=1;_cnt<AE_HIST_NUM;_cnt++)
            BinNum[_cnt] = BinNum[0];
#else
        BinNum[0] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET,AE_PIX_BIN_MODE_0);
        BinNum[1] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET,AE_PIX_BIN_MODE_1);
        BinNum[2] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET,AE_PIX_BIN_MODE_2);
        BinNum[3] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET,AE_PIX_BIN_MODE_3);
        BinNum[4] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET_1,AE_PIX_BIN_MODE_4);
        BinNum[5] = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_SET_1,AE_PIX_BIN_MODE_5);
#endif
        _tmp = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_CTL);
        for(_cnt=0;_cnt<AE_HIST_NUM;_cnt++){
            if(_tmp & 0x1){
                if(stt_en.Bits.AE_HST_DMA_OUT_EN){
                    if(BinNum[_cnt] == 0)
                        aa_xsize += (AE_HIST_BINS*(AE_HIST_BIN/2));    // 128 bins
                    else
                        aa_xsize += (AE_HIST_BINS*AE_HIST_BIN);
                }
                //ROI chk
                if(_cnt == 0){
                    ROI_CHK(0,2,4);
                }
                else if(_cnt == 1){
                    ROI_CHK(1,2,4);
                }
                else if(_cnt == 2){
                    ROI_CHK(2,2,4);
                }
                else if(_cnt == 3){
                    ROI_CHK(3,2,4);
                }
                else if(_cnt == 4){
                    ROI_CHK(4,2,4);
                }
                else if(_cnt == 5){
                    ROI_CHK(5,2,4);
                }
            }
            _tmp=_tmp >> 1;
        }

    }

    //awb motion chk
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RMM_EN) & \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMM_MC,RMM_MO_EN)){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_MOTION_THR,AWB_MOTION_MAP_EN) == 0){
            CAM_FUNC_ERR("awb stt data error\n");
            rst = MFALSE;
        }
    }

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_CCU_HST_END_Y,AE_CCU_HST_END_Y) > aa_win_v){
        rst = MFALSE;
        CAM_FUNC_ERR("CCU's end-Y should be <= %d\n",aa_win_v);
    }

    //
    aa_xsize = aa_xsize /8; //bits to byte
    if(aa_xsize != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_XSIZE,XSIZE)+1)){
        CAM_FUNC_ERR("cur xsize :0x%x, valid xsize:0x%x\n",\
            (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_XSIZE,XSIZE)+1),\
            aa_xsize);
        rst = MFALSE;
    }

    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_YSIZE,YSIZE) != 0){
        //aao is 1-d
        CAM_FUNC_ERR("current ysize:0x%x, user can't set 0 into drv\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_YSIZE,YSIZE));
        rst = MFALSE;
    }


EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("AE/AWB/AAO check error,cur window size:%d x %d\n",aa_win_h,aa_win_v);
    }

    return rst;
}

MBOOL PIPE_CHECK::RMB_CHECK(void)
{
    MBOOL   rst = MTRUE;
    MUINT32 pattern = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DATA_PATTERN);
    MUINT32 rmbn_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,RMBN_SEL);
    CAM_REG_RMB_MODE rmb;

    rmb.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RMB_MODE);

    //
    CAM_FUNC_WRN("current image pattern: %d\n",pattern);
    switch(pattern){
        case eCAM_NORMAL:
        case eCAM_DUAL_PIX:
        case eCAM_4CELL:
            if(rmb.Bits.SENSOR_TYPE != 0 ){
                rst = MFALSE;
                CAM_FUNC_ERR("rmb's sensor type should be 0(%d)\n",rmb.Bits.SENSOR_TYPE);
            }

            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,RMB_EN) == MFALSE){
                rst = MFALSE;
                CAM_FUNC_ERR("rmb should be enabled\n");
            }
            break;
        case eCAM_IVHDR:
        case eCAM_4CELL_IVHDR:
            switch(rmbn_sel){
                case 1:
                case 2:
                    if(rmb.Bits.SENSOR_TYPE != 1 ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("rmb's sensor type should be 1(%d)\n",rmb.Bits.SENSOR_TYPE);
                    }
                    break;
                case 0:
                case 3:
                    if(rmb.Bits.SENSOR_TYPE != 0 ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("rmb's sensor type should be 0(%d)\n",rmb.Bits.SENSOR_TYPE);
                    }
                    break;
                default:
                    rst = MFALSE;
                    CAM_FUNC_ERR("unsupported rmb_sel:%d\n",rmbn_sel);
                    break;
            }

            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,RMB_EN) == MFALSE){
                rst = MFALSE;
                CAM_FUNC_ERR("rmb should be enabled\n");
            }
            break;
        case eCAM_ZVHDR:
        case eCAM_4CELL_ZVHDR:
            switch(rmbn_sel){
                case 1:
                case 2:
                    if(rmb.Bits.SENSOR_TYPE != 2 ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("rmb's sensor type should be 2(%d)\n",rmb.Bits.SENSOR_TYPE);
                    }
                    break;
                case 0:
                case 3:
                    if(rmb.Bits.SENSOR_TYPE != 0 ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("rmb's sensor type should be 0(%d)\n",rmb.Bits.SENSOR_TYPE);
                    }
                    break;
                default:
                    rst = MFALSE;
                    CAM_FUNC_ERR("unsupported rmb_sel:%d\n",rmbn_sel);
                    break;
            }

            //
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,RMB_EN) == MFALSE){
                rst = MFALSE;
                CAM_FUNC_ERR("rmb should be enabled\n");
            }
            break;
        default://YUV & MONO
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,RMB_EN) == MTRUE){
                rst = MFALSE;
                CAM_FUNC_ERR("rmb should be disabled with current pattern\n");
            }
            break;
    }


    //
    if(rmb.Bits.ACC != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM)){
        rst = MFALSE;
        CAM_FUNC_ERR("rmb's acc mode error. (%d_%d)\n",rmb.Bits.ACC,\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM));
    }

    if(rst == MFALSE){
        CAM_FUNC_ERR("RMB check error,cur rmb_sel:%d\n",rmbn_sel);
    }

    return rst;
}

MBOOL PIPE_CHECK::PS_CHECK(void)
{
    MBOOL   rst = MTRUE;
    MUINT32 aa_in_h,aa_in_v,_cnt=0;
    MUINT32 aa_isize_h,aa_isize_v;
    MUINT32 aa_win_size_h,aa_win_size_v;
    MUINT32 aa_xsize = 0,_tmp, V_align;
    CAM_REG_PS_DATA_TYPE ps_type;
    CAM_REG_PS_AWB_WIN_NUM win_num;
    CAM_REG_PS_AWB_WIN_ORG aa_start;
    CAM_REG_PS_HST_CFG hst_cfg;
    MUINT32 pattern = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DATA_PATTERN);
    MUINT32 rmbn_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,RMBN_SEL);

    ps_type.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_DATA_TYPE);
    win_num.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_NUM);
    aa_start.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_ORG);
    hst_cfg.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_HST_CFG);
    //
    rst = this->RMB_CHECK();
    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PS_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PSO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable PSO without enable PS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,PIX_BUS_BMXO)!=0){
        rst = MFALSE;
        CAM_FUNC_ERR("PS must be enabled with BMXO = 1pix mode (%d)\n",\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,PIX_BUS_BMXO));
    }


    //
    if(ps_type.Bits.SENSOR_TYPE != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMB_MODE,SENSOR_TYPE)){
        rst = MFALSE;
        CAM_FUNC_ERR("ps's sensor type should be the same with rmbs' (%d_%d)\n",\
            ps_type.Bits.SENSOR_TYPE,CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMB_MODE,SENSOR_TYPE));
    }

    //
    switch(pattern){
        case eCAM_IVHDR:
        case eCAM_4CELL_IVHDR:
            if(ps_type.Bits.SENSOR_TYPE == 1 ){
                if(ps_type.Bits.G_LE_FIRST != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMG_HDR_CFG,RMG_IHDR_LE_FIRST)){
                    rst = MFALSE;
                    CAM_FUNC_ERR("ps's le_1st should be the same with rmgs' (%d_%d)\n",\
                        ps_type.Bits.G_LE_FIRST,CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RMG_HDR_CFG,RMG_IHDR_LE_FIRST));
                }
            }
            V_align = 4;
            break;
        case eCAM_ZVHDR:
        case eCAM_4CELL_ZVHDR:
            if(ps_type.Bits.SENSOR_TYPE == 2 ){
                CAM_REG_RMG_HDR_CFG hdr_cfg;
                hdr_cfg.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RMG_HDR_CFG);

                //
                if(ps_type.Bits.G_LE_FIRST != hdr_cfg.Bits.RMG_ZHDR_GLE){
                    rst = MFALSE;
                    CAM_FUNC_ERR("ps's le_1st should be the same with rmgs' (%d_%d)\n",\
                        ps_type.Bits.G_LE_FIRST,hdr_cfg.Bits.RMG_ZHDR_GLE);
                }
                //
                if(ps_type.Bits.R_LE_FIRST != hdr_cfg.Bits.RMG_ZHDR_RLE){
                    rst = MFALSE;
                    CAM_FUNC_ERR("ps's le_1st should be the same with rmgs' (%d_%d)\n",\
                        ps_type.Bits.G_LE_FIRST,hdr_cfg.Bits.RMG_ZHDR_RLE);
                }
                //
                if(ps_type.Bits.B_LE_FIRST != hdr_cfg.Bits.RMG_ZHDR_BLE){
                    rst = MFALSE;
                    CAM_FUNC_ERR("ps's le_1st should be the same with rmgs' (%d_%d)\n",\
                        ps_type.Bits.G_LE_FIRST,hdr_cfg.Bits.RMG_ZHDR_BLE);
                }
            }
            V_align = 4;
            break;
        default:
            V_align = 2;
            break;
    }

    //start pix alignment chk
    if(aa_start.Bits.AWB_W_HORG & 0x3){
        rst = MFALSE;
        CAM_FUNC_ERR("start_x need to be 4-alignment\n");
    }

    if(aa_start.Bits.AWB_W_VORG & (V_align-1)){
        rst = MFALSE;
        CAM_FUNC_ERR("start_y need to be %d-alignment\n",V_align);
    }
    //aa win number chk
    if( (win_num.Bits.AWB_W_HNUM < 32) || (win_num.Bits.AWB_W_HNUM > AA_WIN_H)){
        rst = MFALSE;
        CAM_FUNC_ERR("win_h number must be 32 ~ 128\n");
    }
    if( (win_num.Bits.AWB_W_VNUM < 32) || (win_num.Bits.AWB_W_VNUM > AA_WIN_V)){
        rst = MFALSE;
        CAM_FUNC_ERR("win_v number must be 32 ~ 128\n");
    }
    //aa input size , from bmx output, but do not read frm bmx directly, will be re-set by twin drv
    this->InPutSize_TG(aa_in_h,aa_in_v);

    //
    if (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,RMB_EN)) {
        //pix mode
        aa_in_h = aa_in_h >> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMB_MODE,ACC);
    }

    //aa statistic operation input size
    aa_isize_h = aa_in_h - aa_start.Bits.AWB_W_HORG;
    aa_isize_v = aa_in_v - aa_start.Bits.AWB_W_VORG;
    //aa win/pit size, win size <= pit size
    aa_win_size_h = (aa_isize_h / win_num.Bits.AWB_W_HNUM) /2 * 2;
    aa_win_size_v = (aa_isize_v / win_num.Bits.AWB_W_VNUM) /2 * 2;

    //win pix number check
    if( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE) * \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE)) > 2047 ){
        CAM_FUNC_ERR("can't have over 2047pix in each window, data overflow\n");
        rst = MFALSE;
    }

    //h-win check
    if(aa_win_size_h < 4){
        CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",aa_win_size_h);
        rst = MFALSE;
    }
    else{
        MUINT32 pit_size,win_size;
        char _str[32];
        pit_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_PIT,AWB_W_HPIT);
        win_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE);
        //pitch & window check
        _tmp = pit_size;
        snprintf(_str, 31,"pit ");
        for(MUINT32 i=0;i<2;i++){
            //
            if((_tmp < 4)|| (_tmp & 0x3)){

                CAM_FUNC_ERR("%s: size:0x%x[h-min is 4 & 4-alignment]!!\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp > aa_win_size_h){

                CAM_FUNC_ERR("%s size need <= ideal size, [0x%x_0x%x]\n",_str,_tmp,aa_win_size_h);
                rst = MFALSE;
            }
            //
            _tmp = win_size;
            snprintf(_str, 31,"win ");
        }
        //
        if( pit_size < win_size){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",pit_size,win_size);
            rst = MFALSE;
        }
    }


    //v-win check
    if(aa_win_size_v < V_align){
        CAM_FUNC_ERR("ideal v_win size:0x%x[v-min is %d] , underflow!!\n",aa_win_size_v,V_align);
        rst = MFALSE;
    }else{
        MUINT32 pit_size,win_size;
        char _str[32];
        pit_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_PIT,AWB_W_VPIT);
        win_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE);

        //pitch & window check
        _tmp = pit_size;
        snprintf(_str, 31,"pit ");
        for(MUINT32 i=0;i<2;i++){
            //
            if(_tmp > aa_win_size_v){

                CAM_FUNC_ERR("%s size need <= ideal size, [0x%x_0x%x]\n",_str,_tmp,aa_win_size_v);
                rst = MFALSE;
            }
            else if((_tmp < V_align)|| (_tmp & (V_align-1))){

                CAM_FUNC_ERR("%s: size:0x%x[v-min is %d & %d-alignment]!!\n",_str,_tmp,V_align,V_align);
                rst = MFALSE;
            }
            //
            _tmp = win_size;
            snprintf(_str, 31,"win ");
        }
        //
        if( pit_size < win_size){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",pit_size,win_size);
            rst = MFALSE;
        }

    }

    //pixel cnt check
    //lumi of accumulation will be affected if pix_cnt bias too much
    {
        MUINT32 div;
        switch(pattern){
            case eCAM_IVHDR:
            case eCAM_4CELL_IVHDR:
            case eCAM_ZVHDR:
            case eCAM_4CELL_ZVHDR:
                div = 8; // 8 for R/B,  4 for Gr/Gb
                break;
            default:
                div = 4;
                break;
        }
        _tmp = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE) / div);
        _tmp = ((1<<24) + (_tmp>>1))/ _tmp;

        _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT0,AWB_PIXEL_CNT0);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("awb pix cnt0 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }

        _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT3,AWB_PIXEL_CNT3);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("awb pix cnt3 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }

        switch(pattern){
            case eCAM_ZVHDR:
            case eCAM_4CELL_ZVHDR:
                _tmp = _tmp*2; // 8 for R/B,  4 for Gr/Gb
                break;
            default:// do nothing
                break;
        }
        _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT1,AWB_PIXEL_CNT1);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("awb pix cnt1 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }

        _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT2,AWB_PIXEL_CNT2);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("awb pix cnt2 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }
    }

    //ROI check
    {
        CAM_REG_PS_HST_ROI_X roi_x;
        CAM_REG_PS_HST_ROI_Y roi_y;
        roi_x.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_HST_ROI_X);
        roi_y.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_HST_ROI_Y);
        if(roi_x.Bits.PS_X_HI >= roi_x.Bits.PS_X_LOW){
            if(roi_x.Bits.PS_X_HI > (aa_in_h/4)){
                CAM_FUNC_ERR("ROI over aainput size_%d\n",(aa_in_h/4));
                rst = MFALSE;
            }
        }
        else{
            rst = MFALSE;
            CAM_FUNC_ERR("ROI must be high >= low\n");
        }

        if(roi_y.Bits.PS_Y_HI >= roi_y.Bits.PS_Y_LOW){
            if(roi_y.Bits.PS_Y_HI > (aa_in_v/V_align)){
                CAM_FUNC_ERR("ROI over aainput size_%d\n",(aa_in_v/V_align));
                rst = MFALSE;
            }
        }
        else{
            rst = MFALSE;
            CAM_FUNC_ERR("ROI must be high >= low\n");
        }
    }

    //coef chk, this is warning , hw won't be crash , but have quality lost
    {
        CAM_REG_PS_AE_YCOEF0 coef0;
        CAM_REG_PS_AE_YCOEF1 coef1;
        coef0.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_AE_YCOEF0);
        coef1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_PS_AE_YCOEF1);

        if( (coef0.Bits.AE_YCOEF_R + coef0.Bits.AE_YCOEF_GR + coef1.Bits.AE_YCOEF_GB + coef1.Bits.AE_YCOEF_B) != 16384 ){
            CAM_FUNC_ERR("warning, quality lost\n");
        }
    }

    //dram size
    aa_xsize += (win_num.Bits.AWB_W_HNUM*win_num.Bits.AWB_W_VNUM*(PS_TSF_WIN+AWB_WIN));  //awb , 4 channel each win, 2bytes each channel
    aa_xsize += (win_num.Bits.AWB_W_HNUM*win_num.Bits.AWB_W_VNUM*AE_WIN_MAX);    // ae , 2 byte each win

    switch(pattern){
        case eCAM_IVHDR:
        case eCAM_4CELL_IVHDR:
        case eCAM_ZVHDR:
        case eCAM_4CELL_ZVHDR:
            switch(ps_type.Bits.EXP_MODE){
                case 0:
                case 1:
                    if(hst_cfg.Bits.PS_HST_EN){
                        aa_xsize += 256 * BIN_PER_BIT; //3bytes each bin, total 256 bin
                    }
                    break;
                case 2:
                    if(hst_cfg.Bits.PS_HST_EN){
                        aa_xsize += 256 * BIN_PER_BIT; //3bytes each bin, total 256 bin
                    }

                    aa_xsize *= 2; // le + se output
                    break;
            }
            break;
        default:
            if(hst_cfg.Bits.PS_HST_EN){
                aa_xsize += HIST_BIN_MAX * BIN_PER_BIT; //3bytes each bin, total 1024 bin
            }
            break;
    }

    aa_xsize = aa_xsize/8;
    if(aa_xsize != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_XSIZE,XSIZE)+1)){
        CAM_FUNC_ERR("cur xsize :0x%x, valid xsize:0x%x\n",\
            (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_XSIZE,XSIZE)+1),\
            aa_xsize);
        rst = MFALSE;
    }

    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_YSIZE,YSIZE) != 0){
        //if set 0 into drv, 0  -1 will be underflow
        //ps is 1-d output
        CAM_FUNC_ERR("current ysize:0x%x, user can't set 0 into drv\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_YSIZE,YSIZE));
        rst = MFALSE;
    }



    //
    if(hst_cfg.Bits.PS_HST_SEP_G == 1){
        //hw always tie 0
        CAM_FUNC_ERR("hw don't have seperate G\n");
    }
    if(hst_cfg.Bits.PS_CCU_HST_END_Y > win_num.Bits.AWB_W_VNUM){
        rst = MFALSE;
        CAM_FUNC_ERR("CCU end Y should be < %d\n",win_num.Bits.AWB_W_VNUM);
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("PS check error\n!!\n");
    }

    return rst;
}


MBOOL PIPE_CHECK::FLK_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 size_h,size_v,in_size_h,in_size_v;
    MUINT32 x_size;
    MUINT32 flk1_sel;
    MUINT32 win_num_h,win_num_v;
    MUINT32 ofst_x,ofst_y;
    MUINT32 qbn_pix=0;
    MUINT32 tmp;

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,FLK_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,FLKO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable FLKO without enable FLK\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    flk1_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,FLK1_SEL);

    switch (flk1_sel ) {
    case 0:
        this->InPutSize_TG(in_size_h,in_size_v);
        if ((qbn_pix = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE,QBN_ACC))!= 0) {
            in_size_h = in_size_h >> qbn_pix;
        }
        break;
    case 1:
        this->InPutSize_TG(in_size_h,in_size_v,MTRUE);
        in_size_h = in_size_h >> ( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);   //+1 for YUV fmt
        break;
    default:
        CAM_FUNC_ERR("flk1_sel:0x%x err\n",flk1_sel);
        rst = MFALSE;
        goto EXIT;
        break;
    }

    //start
    ofst_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLK_A_OFST,FLK_OFST_X);
    ofst_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLK_A_OFST,FLK_OFST_Y);
    //win num
    win_num_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLK_A_NUM,FLK_NUM_X);
    win_num_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLK_A_NUM,FLK_NUM_Y);
    //win size
    size_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLK_A_SIZE,FLK_SIZE_X);
    size_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLK_A_SIZE,FLK_SIZE_Y);
    if((size_h & 0x1) || (size_v & 0x1)){
        rst = MFALSE;
        CAM_FUNC_ERR("flk win size can't be odd\n");
    }
    if((ofst_x + size_h * win_num_h) > in_size_h){
        rst = MFALSE;
        CAM_FUNC_ERR("H-direction out of range[0x%x_0x%x]\n",(ofst_x + size_h * win_num_h),in_size_h);
    }
    if((ofst_y + size_v * win_num_v) > in_size_v){
        rst = MFALSE;
        CAM_FUNC_ERR("V-direction out of range[0x%x_0x%x]\n",(ofst_y + size_v * win_num_v),in_size_v);
    }

    //xsize
    x_size = (win_num_h*win_num_v*size_v * 2);
    if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_XSIZE,XSIZE) + 1) != x_size){
        rst = MFALSE;
        CAM_FUNC_ERR("xsize mismatch[0x%x_0x%x]\n",(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_XSIZE,XSIZE) + 1),\
            x_size);
    }
    //ysize
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_YSIZE,YSIZE) != 0){
        rst = MFALSE;
        CAM_FUNC_ERR("Ysize need  to be 0[0x%x]\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_YSIZE,YSIZE));
    }
    //stride
    if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_XSIZE,XSIZE) + 1) > CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_STRIDE,RSV)){
        rst = MFALSE;
        CAM_FUNC_ERR("stride > xsize+1[0x%x_0x%x]\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_STRIDE,RSV),\
            (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FLKO_XSIZE,XSIZE) + 1));
    }

#if 0
    P1_MSG("flk1_sel:0x%x,flk2_sel:0x%x,in_size:0x%x_0x%x,win num:0x%x_0x%x,win size:0x%x_0x%x,xsize:0x%x\n",
        flk1_sel,((this->m_pIspDrv->readReg(0x3018)&300) >> 8),
        in_size_h,in_size_v,
        win_num_h,win_num_v,
        size_h,size_v,x_size);
#endif

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("FLK check fail\n");
    }

    return rst;
}



MBOOL PIPE_CHECK::LCS_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 lcs_sel;
    MUINT32 win_h,win_v;
    MUINT32 in_h,in_v;
    MUINT32 crop_x,crop_y,crop_h,crop_v;
    MUINT32 scaling_h,scaling_v;
    MUINT32 pixMode;
    MUINT32 tmp=0;

    lcs_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,LCS_SEL);
    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,LCS_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LCSO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable LCSO without enable LCS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    switch (lcs_sel) {
    case 1:
        this->InPutSize_TG(in_h,in_v);
        pixMode = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC);
        break;
    case 0:
        this->InPutSize_TG(in_h,in_v,MTRUE);
        pixMode = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1;
        break;
    default:
        CAM_FUNC_ERR("lcs_sel error(%d)\n",lcs_sel);
        rst = MFALSE;
        goto EXIT;
        break;
    }
    in_h = in_h >> pixMode;


    win_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_CON, LCS25_OUT_WD );
    win_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_CON, LCS25_OUT_HT );

    if(( win_h & 0x1 ) || (win_v & 0x1)){
        CAM_FUNC_ERR("lcs output h/v should be even[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    if((win_h < 8) || (win_v<8) || (win_h>384) || (win_v>384)){
        CAM_FUNC_ERR("lcs output size : h:8~384, v:8~384[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    crop_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_ST, LCS25_START_J );
    crop_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_ST, LCS25_START_I );
    crop_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_AWS, LCS25_IN_WD );
    crop_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_AWS, LCS25_IN_HT );

    if( (crop_x + crop_h) != in_h){
        CAM_FUNC_ERR("lcs crop_x + crop_h must be equal to in_h [0x%x_0x%x_0x%x]\n",crop_x,crop_h,in_h);
        rst = MFALSE;
    }
    if( (crop_y + crop_v) != in_v){
        CAM_FUNC_ERR("lcs crop_y + crop_v must be equal to in_v [0x%x_0x%x_0x%x]\n",crop_y,crop_v,in_v);
        rst = MFALSE;
    }

    if( win_h*2 >= crop_h){
        CAM_FUNC_ERR("lcs out_h must <= in_h[0x%x_0x%x]\n",(win_h*2),crop_h);
        rst = MFALSE;
    }
    if(win_v*2 >= crop_v){
        CAM_FUNC_ERR("lcs out_v must < in_v[0x%x_0x%x]\n",(win_v*2),crop_v);
        rst = MFALSE;
    }

    scaling_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_LRZR_1, LCS25_LRZR_X );
    scaling_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCS25_LRZR_2, LCS25_LRZR_Y );
    if(scaling_h != ((win_h - 1)* 1048576 / (crop_h>>1) )){
        CAM_FUNC_ERR("lcs h-scaling factor error [0x%x_0x%x_0x%x]\n",scaling_h,win_h,crop_h);
        rst = MFALSE;
    }
    if(scaling_v != ((win_v - 1)* 1048576 / (crop_v>>1) )){
        CAM_FUNC_ERR("lcs v-scaling factor error [0x%x_0x%x_0x%x]\n",scaling_v,win_v,crop_v);
        rst = MFALSE;
    }

    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCSO_XSIZE, XSIZE ) != (win_h*2 -1) ){
        CAM_FUNC_ERR("LCS xsize error:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCSO_XSIZE, XSIZE ),(win_h*2 -1));
        rst = MFALSE;
    }
    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCSO_YSIZE, YSIZE ) != (win_v -1) ){
        CAM_FUNC_ERR("LCS ysize error:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_LCSO_YSIZE, YSIZE ),(win_v -1));
        rst = MFALSE;
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("LCS check fail:cur mux:0x%x\n",lcs_sel);
    }
    return rst;
}

MBOOL PIPE_CHECK::LMV_CHECK(void)
{
    MUINT32 win_h = 4,win_v = 8;
    MUINT32 ofst_x=0,ofst_y=0;
    MUINT32 in_size_h,in_size_v;
    MUINT32 pixMode = 0,hds_sel;
    Header_RRZO rrzo_fh;
    MUINT32 tmp=0;
    MBOOL rst = MTRUE;
    MUINT32 step_h = 16,step_v = 8,win_size_h,win_size_v,op_h=1,op_v=1;


    hds_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,HDS1_SEL);

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,LMV_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LMVO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable LMVO without enable LMV\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    //max window number
    win_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_NUM_HWIN);
    win_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_NUM_VWIN);
    if( (win_h > 4) || (win_v > 8)){
        CAM_FUNC_ERR("eis win size max:4*8[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    //ofst,ofst have min constraint
    ofst_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_MB_OFFSET,EIS_RP_HOFST);
    ofst_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_MB_OFFSET,EIS_RP_VOFST);
    if((ofst_x < 17) || (ofst_y < 17)){
        CAM_FUNC_ERR("eis ofset must > 16[0x%x_0x%x]\n",ofst_x,ofst_y);
    }
    //floating ofset, current frame ofst from previous frame, normally set 0.
    //this->m_pIspDrv->writeReg(0x355c, ((FL_ofst_x<<16)|FL_ofst_y));

    switch(hds_sel){
        case 0:
            this->InPutSize_TG(in_size_h,in_size_v,MTRUE);

            in_size_h = in_size_h>>((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
            break;
        case 1:
            tmp = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv->getPhyObj());

            in_size_h = tmp & 0xffff;
            in_size_v = (tmp>>16) & 0xffff;

            in_size_h = in_size_h>>CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_HDS_MODE,HDS_DS);
            break;
        default:
            in_size_h = in_size_v = 0;
            CAM_FUNC_ERR("hdr_sel error(%d)\n",hds_sel);
            rst = MFALSE;
            goto EXIT;
            break;
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,WIDTH) != in_size_h){
        CAM_FUNC_ERR("eis input-h err[0x%x_0x%x]\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,WIDTH),in_size_h);
        rst = MFALSE;
    }
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,HEIGHT) != in_size_v){
        CAM_FUNC_ERR("eis input-v err[0x%x_0x%x]\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,HEIGHT),in_size_v);
        rst = MFALSE;
    }

    //EIS step size
    step_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_NUM_HRP);
    step_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_NUM_VRP);
    if((step_h > 16) || (step_v > 8)){
        CAM_FUNC_ERR("eis step size max:16*8[0x%x_0x%x]\n",step_h,step_v);
        rst = MFALSE;
    }

    //EIS window size
    win_size_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_MB_INTERVAL,EIS_WIN_HSIZE);
    win_size_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_MB_INTERVAL,EIS_WIN_VSIZE);
    if(win_size_h < ((step_h + 1) * 16 + 2)){
        CAM_FUNC_ERR("eis h win size too small[0x%x_0x%x]\n",win_size_h,((step_h + 1) * 16 + 2));
        rst = MFALSE;
    }
    if(win_size_v < ((step_v + 1) * 16 + 2)){
        CAM_FUNC_ERR("eis v win size too small[0x%x_0x%x]\n",win_size_v,((step_v + 1) * 16 + 2));
        rst = MFALSE;
    }

    //input check
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,WIDTH) != in_size_h){
        CAM_FUNC_ERR("input width err:%d_%d\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,WIDTH),in_size_h);
        in_size_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,WIDTH);
        rst = MFALSE;
    }
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,HEIGHT) != in_size_v){
        CAM_FUNC_ERR("input height err:%d_%d\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,HEIGHT),in_size_v);
        in_size_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_IMAGE_CTRL,HEIGHT);
        rst = MFALSE;
    }
    op_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_OP_HORI);
    op_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_OP_VERT);
    /* EIS hsize error won't cause pipeline fail, ISP still has P1 done & EISO done,
       but EISO content might has quality issue. */
    if( (in_size_h/op_h)  < (win_size_h*(win_h-1) + 16*(step_h+1) + (ofst_x-16))){
        CAM_FUNC_ERR("h size error:%d_%d_%d_%d_%d_%d, but won't cause pipeline fail\n",in_size_h,op_h,step_h,ofst_x,win_size_h,win_h);
        rst = MFALSE;
    }
    /* EIS vsize error will cause pipeline fail */
    if( (in_size_v/op_v)  < (win_size_v*(win_v-1) + 16*(step_v+1) + (ofst_y-16))){
        CAM_FUNC_ERR("v size error:%d_%d_%d_%d_%d_%d\n",in_size_v,op_v,step_v,ofst_y,win_size_v,win_v);
        rst = MFALSE;
    }

    //eis subg is always off
    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_EIS_PREP_ME_CTRL1,EIS_SUBG_EN) == 1){
        CAM_FUNC_ERR("EIS SubG is always off at isp4.0\n");
        rst = MFALSE;
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("LMV check fail:cur mux:0x%x\n",hds_sel);
    }

    return rst;
}

MBOOL PIPE_CHECK::RSS_CHECK(void)
{
    MUINT32 win_out_w=0,win_out_h=0, win_in_w=0, win_in_h=0;
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,RSS_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,RSSO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable RSSO without enable RSS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    win_in_w = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_RSS_A_IN_IMG, RSS_IN_WD);
    win_in_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_RSS_A_IN_IMG, RSS_IN_HT);


    //out range is limited
    win_out_w = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_RSS_A_OUT_IMG, RSS_OUT_WD);
    win_out_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_RSS_A_OUT_IMG, RSS_OUT_HT);

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("RSS check fail\n");
    }

    return rst;
}

MBOOL PIPE_CHECK::LSC_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 in_size_x,in_size_y;
    MUINT32 win_size_x,win_size_y,win_lsize_x,win_lsize_y,xsize;
    MUINT32 tmp=0;
    MUINT32 nWin_h,nWin_v, extend;

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,LSC_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LSCI_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable LSCI without enable LSC\n");
            rst = MFALSE;
            goto EXIT;
        }
        else{
            if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,CPN_EN) + \
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,DCPN_EN)) != 0)
            {
                CAM_FUNC_ERR("LSC must be enabled when CPN/DPCN is enabled\n");
                rst = MFALSE;
            }
            goto EXIT;
        }
    }
    else{
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LSCI_EN) == 0){
            CAM_FUNC_ERR("DO NOT enable LSC without enable LSCI\n");
            rst = MFALSE;
            goto EXIT;
        }
    }

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN) == MTRUE){
        //twin drv will have adjusted configuration on LSC due to overlap processing
        CAM_FUNC_INF("bypass lsc check\n");
        goto EXIT;
    }
    //
    this->InPutSize_DMX(in_size_x,in_size_y);

    nWin_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL2,LSC_SDBLK_XNUM);
    nWin_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL3,LSC_SDBLK_YNUM);
    win_size_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL2,LSC_SDBLK_WIDTH);
    win_size_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL3,LSC_SDBLK_HEIGHT);
    if(win_size_x != (in_size_x / (2*(nWin_h+1)))){
        CAM_FUNC_ERR("lsc win size error[0x%x_0x%x]\n",win_size_x,(in_size_x / (2*(nWin_h+1))));
        rst = MFALSE;
    }
    if(win_size_y != (in_size_y / (2*(nWin_v+1)))){
        CAM_FUNC_ERR("lsc win size error[0x%x_0x%x]\n",win_size_y,(in_size_y / (2*(nWin_v+1))));
        rst = MFALSE;
    }

    if(rst == MFALSE){
        CAM_FUNC_ERR("current lsc win:[%d_%d]\n",nWin_h,nWin_v);
    }
    win_lsize_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_LBLOCK,LSC_SDBLK_lWIDTH);
    win_lsize_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_LBLOCK,LSC_SDBLK_lHEIGHT);
    if(win_lsize_x != ((in_size_x/2) - (nWin_h*win_size_x))){
        if((in_size_x/2) < (nWin_h*win_size_x))
            CAM_FUNC_ERR("LSC cfg size > TG size.[%d_%d]\n",(nWin_h*win_size_x),(in_size_x/2));
        CAM_FUNC_ERR("lsc last win size error[0x%x_0x%x]\n",win_lsize_x,((in_size_x/2) - (nWin_h*win_size_x)));
        rst = MFALSE;
    }
    if(win_lsize_y != ((in_size_y/2) - (nWin_v*win_size_y))){
        if((in_size_y/2) < (nWin_v*win_size_y))
            CAM_FUNC_ERR("LSC cfg size > TG size.[%d_%d]\n",(nWin_h*win_size_x),(in_size_x/2));
        CAM_FUNC_ERR("lsc last win size error[0x%x_0x%x]\n",win_lsize_y,((in_size_y/2) - (nWin_v*win_size_y)));
        rst = MFALSE;
    }

    xsize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSCI_XSIZE,XSIZE);
    extend = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL1,LSC_EXTEND_COEF_MODE);
    if(extend == 1) {//lsci table is 192bits
        if((xsize+1) != ((nWin_h+1) * 4 * 192 / 8)){
            CAM_FUNC_ERR("lsci xsize error[0x%x_0x%x]ext[%d]\n",(xsize+1),((nWin_h+1) * 4 * 192 / 8),extend);
            rst = MFALSE;
        }
    }
    else {//lsci table is 128 bits
        if((xsize+1) != ((nWin_h+1) * 4 * 128 / 8)){
            CAM_FUNC_ERR("lsci xsize error[0x%x_0x%x]ext[%d]\n",(xsize+1),((nWin_h+1) * 4 * 128 / 8),extend);
            rst = MFALSE;
        }
    }
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSCI_YSIZE,YSIZE) != ((nWin_v+1) - 1) ){
        CAM_FUNC_ERR("lsci ysize error[0x%x_0x%x]\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSCI_YSIZE,YSIZE) + 1,(nWin_v+1));
        rst = MFALSE;
    }


EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("LSC check fail\n");
    }

    return rst;
}


MBOOL PIPE_CHECK::AF_CHECK(MBOOL bTwin)
{
    MBOOL rst = MTRUE;
    MUINT32 tmp=0;
    MUINT32 sgg_sel;
    MUINT32 h_size;
    MUINT32 v_size;
    CAM_REG_AF_BLK_2 blk_2;
    CAM_REG_AF_BLK_1 blk_1;
    CAM_REG_AF_BLK_0 blk_0;
    MUINT32 xsize,ysize;
    CAM_REG_AF_CON af_con;
    CAM_REG_AF_CON2 af_con2;
    CAM_REG_AF_VLD af_vld;
    MUINT32 af_blk_sz;
    MUINT32 afo_xsize, afo_ysize;
    CAM_REG_AF_TH_2 af_th2;

    sgg_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,SGG_SEL);

    blk_0.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_BLK_0);
    blk_1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_BLK_1);
    blk_2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_BLK_2);

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,AF_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AFO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable AFO without enable AF\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    //
    af_con.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_CON);
    af_con2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_CON2);

    //AFO and AF relaterd module enable check
    tmp  = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AFO_EN)? 0:1);
    tmp += (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RCP_EN)? 0:1);
    tmp += (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,SGG1_EN)? 0:1);
    tmp += (af_con.Bits.AF_EXT_STAT_EN ? 0:1);
    if(tmp){
        CAM_FUNC_ERR("AF is enabled, MUST enable AFO/RCP/SGG1/AF_EXT_STAT_EN:0x%x_0x%x_0x%x\n",
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AFO_EN),
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RCP_EN),
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,SGG1_EN));
        rst = MFALSE;
    }

    if((af_con.Bits.AF_H_GONLY & af_con2.Bits.AF_DS_EN) != 0){
        CAM_FUNC_ERR("h_gonly & ds_en r mutually exclusive function\n");
        rst = MFALSE;
    }


    //AF image wd
    switch(sgg_sel){
        case 0:
            {
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;

                if(bTwin == MTRUE){
                    CAM_FUNC_WRN("sgg_sel = 0 under twin mode,is IQ ok?\n");
                }
                TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

                h_size = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
                v_size = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

                h_size = h_size>>((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
            }
            break;
        case 1:
            {
                CAM_REG_RCP_CROP_CON1 rcp_w;
                CAM_REG_RCP_CROP_CON2 rcp_h;
                rcp_w.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RCP_CROP_CON1);
                rcp_h.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RCP_CROP_CON2);

                h_size = rcp_w.Bits.RCP_END_X - rcp_w.Bits.RCP_STR_X + 1;
                v_size = rcp_h.Bits.RCP_END_Y - rcp_h.Bits.RCP_STR_Y + 1;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported sgg_sel:0x%x\n",sgg_sel);
            return MFALSE;
            break;
    }
    if(h_size != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_SIZE,AF_IMAGE_WD)){
        CAM_FUNC_ERR("AF input size mismatch:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_SIZE,AF_IMAGE_WD),h_size);
        rst = MFALSE;
    }
    else{
        if(h_size > 5376){
            CAM_FUNC_ERR("AF input width should be <= 5376\n");
            rst = MFALSE;
        }
        if(h_size & 0x1){
            CAM_FUNC_ERR("AF input width should be multiple of 2\n");
            rst = MFALSE;
        }
        if((af_con.Bits.AF_H_GONLY == 1) && (h_size < 48)){
            CAM_FUNC_ERR("AF input min width=48 if h_gonly = 1\n");
            rst = MFALSE;
        }
        if((af_con2.Bits.AF_DS_EN == 1) && (h_size < 56)){
            CAM_FUNC_ERR("AF input min width=56 if ds_en = 1\n");
            rst = MFALSE;
        }
    }



    //ofset
    af_vld.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_VLD);
    if((af_vld.Bits.AF_VLD_XSTART&0x1)||(af_vld.Bits.AF_VLD_YSTART&0x1)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF vld start must be even:0x%x_0x%x\n",af_vld.Bits.AF_VLD_XSTART,af_vld.Bits.AF_VLD_YSTART);
    }
    else{
        if((af_vld.Bits.AF_VLD_XSTART + blk_0.Bits.AF_BLK_XSIZE*blk_1.Bits.AF_BLK_XNUM) > h_size){
            rst = MFALSE;
            CAM_FUNC_ERR("af h window out of range:0x%x_0x%x\n",(af_vld.Bits.AF_VLD_XSTART + blk_0.Bits.AF_BLK_XSIZE*blk_1.Bits.AF_BLK_XNUM),h_size);
        }
        if((af_vld.Bits.AF_VLD_YSTART + blk_0.Bits.AF_BLK_YSIZE*blk_1.Bits.AF_BLK_YNUM) > v_size){
            rst = MFALSE;
            CAM_FUNC_ERR("af v window out of range:0x%x_0x%x\n",(af_vld.Bits.AF_VLD_YSTART + blk_0.Bits.AF_BLK_YSIZE*blk_1.Bits.AF_BLK_YNUM),v_size);
        }
    }

    //window num
    if((blk_1.Bits.AF_BLK_XNUM==0) || (blk_1.Bits.AF_BLK_XNUM>AF_WIN_H)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF xwin num :0x%x[1~128]\n",blk_1.Bits.AF_BLK_XNUM);
    }
    if((blk_1.Bits.AF_BLK_YNUM==0) || (blk_1.Bits.AF_BLK_YNUM>AF_WIN_V)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF ywin num :0x%x[1~128]\n",blk_1.Bits.AF_BLK_YNUM);
    }

    //win size
    //max
    if(blk_0.Bits.AF_BLK_XSIZE > 128){
        rst = MFALSE;
        CAM_FUNC_ERR("af max h win size:128 cur:0x%x\n",blk_0.Bits.AF_BLK_XSIZE);
    }
    else{//min constraint
        if((af_con.Bits.AF_V_AVG_LVL == 3) && (af_con.Bits.AF_V_GONLY == 1)){
            tmp = 32;
        }
        else if((af_con.Bits.AF_V_AVG_LVL == 3) && (af_con.Bits.AF_V_GONLY  == 0)){
            tmp= 16;
        }
        else if((af_con.Bits.AF_V_AVG_LVL == 2) && (af_con.Bits.AF_V_GONLY  == 1)){
            tmp= 16;
        }
        else{
            tmp= 8;
            if(af_con.Bits.AF_V_AVG_LVL < 2){
                CAM_FUNC_ERR("AF_V_AVG_LVL can only be 2 or 3\n");
                rst = MFALSE;
            }
        }
        if(blk_0.Bits.AF_BLK_XSIZE<tmp){
            CAM_FUNC_ERR("af min h win size::0x%x cur:0x%x   [0x%x_0x%x]\n",tmp,blk_0.Bits.AF_BLK_XSIZE,af_con.Bits.AF_V_AVG_LVL,af_con.Bits.AF_V_GONLY);
            rst = MFALSE;
        }
        else if(blk_0.Bits.AF_BLK_XSIZE < blk_2.Bits.AF_PROT_BLK_XSIZE){
            CAM_FUNC_ERR("x blk size can't be < protected size(%d_%d)\n",blk_0.Bits.AF_BLK_XSIZE,blk_2.Bits.AF_PROT_BLK_XSIZE);
            rst = MFALSE;
        }
    }

    if(af_con.Bits.AF_V_GONLY == 1){
        if(blk_0.Bits.AF_BLK_XSIZE & 0x3){
            CAM_FUNC_ERR("af min h win size must 4 alighment:0x%x\n",blk_0.Bits.AF_BLK_XSIZE);
            rst = MFALSE;
        }
    }
    else{
        if(blk_0.Bits.AF_BLK_XSIZE & 0x1){
            CAM_FUNC_ERR("af min h win size must 2 alighment:0x%x\n",blk_0.Bits.AF_BLK_XSIZE);
            rst = MFALSE;
        }
    }

    if(blk_0.Bits.AF_BLK_YSIZE > 128){
        rst = MFALSE;
        CAM_FUNC_ERR("af max v win size:128 cur:0x%x\n",blk_0.Bits.AF_BLK_YSIZE);
    }
    else{//min constraint
        if(blk_0.Bits.AF_BLK_XSIZE<1){
            CAM_FUNC_ERR("af min v win size:1, cur:0x%x\n",blk_0.Bits.AF_BLK_XSIZE);
            rst = MFALSE;
        }

        if (af_con.Bits.AF_EXT_STAT_EN == 1){
            if(blk_0.Bits.AF_BLK_XSIZE < 8){
                CAM_FUNC_ERR("AF_EXT_STAT_EN=1, af min h win size::8 cur:0x%x\n",blk_0.Bits.AF_BLK_XSIZE);
                rst = MFALSE;
            }
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,SGG5_EN) == 0) {
                CAM_FUNC_ERR("AF_EXT_STAT_EN=1, MUST enable sgg5 & disable AF_H_GONLY:0x%x_0x%x\n",
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,SGG5_EN),
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_CON,AF_H_GONLY));
                rst = MFALSE;
            }
        } else {
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,SGG5_EN) == 1) {
                CAM_FUNC_ERR("AF_EXT_STAT_EN=0, sgg5 must be disabled\n");
                rst = MFALSE;
            }
        }

        if(blk_0.Bits.AF_BLK_YSIZE < blk_2.Bits.AF_PROT_BLK_YSIZE){
            CAM_FUNC_ERR("y blk size can't be < protected size(%d_%d)\n",blk_0.Bits.AF_BLK_YSIZE,blk_2.Bits.AF_PROT_BLK_YSIZE);
            rst = MFALSE;
        }
    }

    //check max afo size, 128*128*af_blk_sz
    afo_xsize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_XSIZE,XSIZE);
    afo_ysize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_YSIZE,YSIZE);
    af_blk_sz = ((af_con.Bits.AF_EXT_STAT_EN == MTRUE)?AFO_EXT_STAT_SZ:(AFO_EXT_STAT_SZ/2));
    af_blk_sz += AFO_NORMAL_SZ;
    //xsize/ysize
    xsize = blk_1.Bits.AF_BLK_XNUM*af_blk_sz;
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_XSIZE,XSIZE) != (xsize -1 )){
        CAM_FUNC_ERR("afo xsize mismatch:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_BLK_0,AF_BLK_XSIZE),(xsize -1 ));
        rst = MFALSE;
    }
    ysize = blk_1.Bits.AF_BLK_YNUM;
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_YSIZE,YSIZE) != (ysize -1 )){
        CAM_FUNC_ERR("afo ysize mismatch:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_YSIZE,YSIZE),(ysize -1 ));
        rst = MFALSE;
    }


    //AF_TH
    af_th2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AF_TH_2);
    if((af_th2.Bits.AF_HFLT2_SAT_TH0 > af_th2.Bits.AF_HFLT2_SAT_TH1) || \
        (af_th2.Bits.AF_HFLT2_SAT_TH1 > af_th2.Bits.AF_HFLT2_SAT_TH2) || \
        (af_th2.Bits.AF_HFLT2_SAT_TH2 > af_th2.Bits.AF_HFLT2_SAT_TH3)){
        CAM_FUNC_ERR("af sat th, MUST th3 >= th2 >= th1 >= th0:0x%x_0x%x_0x%x_0x%x\n",
            af_th2.Bits.AF_HFLT2_SAT_TH3,af_th2.Bits.AF_HFLT2_SAT_TH2,af_th2.Bits.AF_HFLT2_SAT_TH1,af_th2.Bits.AF_HFLT2_SAT_TH0);
        rst = MFALSE;
    }

    //lookup table need to maintian incremental
    {
        #define _bitf_h(idx,x) AF_H_TH_##idx##_D##x
        #define _reg_h(idx,x) CAM_AF_LUT_H##idx##_##x
        #define _bitf_v(x) AF_V_TH_D##x
        #define _reg_v(x) CAM_AF_LUT_V_##x

        #define bitf_h(idx,x) _bitf_h(idx,x)
        #define reg_h(idx,x) _reg_h(idx,x)
        #define bitf_v(x) _bitf_v(x)
        #define reg_v(x) _reg_v(x)

        #define hor_lut(k,j,i) {\
            if( (((i+1)*CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_h(k,j),bitf_h(k,i+1))) - \
                ((i)*CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_h(k,j),bitf_h(k,i))))  < 0 ){  \
                CAM_FUNC_ERR("hor lut talbe:%d should be incremental(%d)\n",k,i); \
                rst = MFALSE;   \
            }\
        }

        #define ver_lut(j,i) {\
            if( (((i+1)*CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_v(j),bitf_v(i+1))) - \
                ((i)*CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_v(j),bitf_v(i))))  < 0 ){  \
                CAM_FUNC_ERR("ver lut table should be oncremental(%d)\n",i);    \
                rst = MFALSE;   \
            }\
        }

        //hor lut table
        //1st
        hor_lut(0,1,1);
        hor_lut(0,1,2);
        hor_lut(0,1,3);
        hor_lut(0,1,4);
        hor_lut(0,2,5);
        hor_lut(0,2,6);
        hor_lut(0,2,7);
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_h(0,0),AF_H_TH_0_LUT_MODE) == 0){
            //8 step
        }
        else{
            //16 step
            hor_lut(0,2,8);
            hor_lut(0,3,9);
            hor_lut(0,3,10);
            hor_lut(0,3,11);
            hor_lut(0,3,12);
            hor_lut(0,4,13);
            hor_lut(0,4,14);
            hor_lut(0,4,15);
        }

        //2nd
        hor_lut(1,1,1);
        hor_lut(1,1,2);
        hor_lut(1,1,3);
        hor_lut(1,1,4);
        hor_lut(1,2,5);
        hor_lut(1,2,6);
        hor_lut(1,2,7);
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_h(1,0),AF_H_TH_1_LUT_MODE) == 0){
            //8 step
        }
        else{
            //16 step
            hor_lut(1,2,8);
            hor_lut(1,3,9);
            hor_lut(1,3,10);
            hor_lut(1,3,11);
            hor_lut(1,3,12);
            hor_lut(1,4,13);
            hor_lut(1,4,14);
            hor_lut(1,4,15);
        }
        //ver lut table
        ver_lut(1,1);
        ver_lut(1,2);
        ver_lut(1,3);
        ver_lut(1,4);
        ver_lut(2,5);
        ver_lut(2,6);
        ver_lut(2,7);
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),reg_v(0),AF_V_TH_LUT_MODE) == 0){
            //8 step
        }
        else{
            //16 step
            ver_lut(2,8);
            ver_lut(3,9);
            ver_lut(3,10);
            ver_lut(3,11);
            ver_lut(3,12);
            ver_lut(4,13);
            ver_lut(4,14);
            ver_lut(4,15);
        }

    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("af check fail:cur mux:0x%x\n",sgg_sel);
    }

    return rst;

}


MUINTPTR PIPE_CHECK::m_BPCI_VA = 0;
MUINTPTR PIPE_CHECK::m_PDI_VA = 0;
MBOOL PIPE_CHECK::PDO_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 tmp=0;
    MUINT32 pdo_sel;
    CAM_DPD_CTRL::E_Density density = CAM_DPD_CTRL::_level_max;

    pdo_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,PDO_SEL);

    switch(pdo_sel){
        case 0:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PDE_EN) ^
               CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDI_EN))
            {
                CAM_FUNC_ERR("pde & pdi must enable/disable at the same time(%d_%d)\n",
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PDE_EN),
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDI_EN));
                rst = MFALSE;
            }
            break;
        case 1:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,PBN_EN,1)){
                if( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PSB_EN) + CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PCP_EN)) > 0){
                    CAM_FUNC_ERR("should be density with level_1\n");
                    rst = MFALSE;
                }
                else
                    density = CAM_DPD_CTRL::_level_1;
            }
            else{
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PSB_EN) ^ CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN2,PCP_EN)){
                    CAM_FUNC_ERR("should be density with level_2\n");
                    rst = MFALSE;
                }
                else
                    density = CAM_DPD_CTRL::_level_2;
            }
            break;
        default:
            rst = MFALSE;
            CAM_FUNC_ERR("unsupported pdo path:%d\n",pdo_sel);
            break;
    }
    //pdo
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDO_EN) == 1){
        MUINT32 in_h,in_v;
        this->InPutSize_TG(in_h,in_v,MTRUE);

        if(pdo_sel == 1) {//
            MUINT32 pdo_stride, pdo_ysize;
            if(density == CAM_DPD_CTRL::_level_1){
                pdo_stride = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_STRIDE, STRIDE);
                pdo_ysize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_YSIZE, YSIZE)+1;

                tmp = (in_h >> 4) << 2;/* TG_W/16*2*2 ,output 1pix everesy 2*16, 2B per Pix*/
                if(pdo_stride != tmp) {
                    CAM_FUNC_ERR("pdo stride error : tg_w(%d)stride(%d_%d)\n",in_h, tmp, pdo_stride);
                    rst = MFALSE;
                }
                tmp = in_v >> 3;/* TG_H/8 */
                if(pdo_ysize != tmp) {
                    CAM_FUNC_ERR("pdo ysize error : tg_h(%d)ysize(%d_%d)\n",in_v, tmp, pdo_ysize);
                    rst = MFALSE;
                }
            }
            else{
                rst = MFALSE;
                CAM_FUNC_ERR("currently, only level_1's density is supported\n");
            }
        }
        else{
            //if sel = 0 => running at UFO mode. need to have 16B alignment
            //BA alignment check
            MUINT32 _cnt = 0;
            MUINT32 xsize = in_h*PDO_PIX_B;
            tmp = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_BASE_ADDR,BASE_ADDR) + \
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_OFST_ADDR,OFFSET_ADDR));
            do{
                if((tmp % 16 ) != 0){
                    CAM_FUNC_ERR("pdo/fh_pdo base addr + offset addr must be 16-alignment(0x%x)\n",tmp);
                    rst = MFALSE;
                }

                if(_cnt++ > 0)
                    break;

                tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_FH_BASE_ADDR,BASE_ADDR);
            }while(1);

            //stride/xsize alignment check
            tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_STRIDE,STRIDE);
            if((tmp % 16 ) != 0){
                CAM_FUNC_ERR("pdo stride must be 16-alignment(0x%x),suggest:0x%x\n",tmp,(xsize+16)/16*16);
                rst = MFALSE;
            }

#if 0   //under UF MODE , stride need to be 16-alignment, won't be xsize+1
            if(tmp != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_XSIZE,XSIZE)+1) ){
                CAM_FUNC_ERR(" pdo stride must be = xsize+1 \n");
                rst = MFALSE;
            }
#endif
        }
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDI_EN)){
        //can't use stride, pdi is 1-D
        if(this->PD_TABLE_CHECK(this->m_PDI_VA,CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDI_XSIZE,XSIZE)+1) == MFALSE){
            rst = MFALSE;
            CAM_FUNC_ERR("PDI's check MAY BE fail due to table\n");
        }
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("pd check fail:cur mux:0x%x\n",pdo_sel);
    }
    return rst;
}

MBOOL PIPE_CHECK::BNR_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN) == 1){//case for phase detection
        MUINT32 tmp = 0;
        //
        tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN);
        if(tmp == 0){
            rst = 0;
            CAM_FUNC_ERR("PDC fail (pdc_en/lut_en/bpci_en:%d_%d_%d)\n",\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN),\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN),\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN));
        }

        //
        if ( 0 == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN)){
            MUINT32 in_h,in_v;
            //
            this->InPutSize_TG(in_h,in_v);
            if((in_h-1) != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_XSIZE)) {
                CAM_FUNC_ERR("bnr bpc_xize error:0x%x_0x%x",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_XSIZE),in_h-1);
                rst = MFALSE;
            }
            if((in_v-1) != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_YSIZE)) {
                CAM_FUNC_ERR("bnr bpc_yize error:0x%x_0x%x",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_YSIZE),in_v-1);
                rst = MFALSE;
            }
        }
        else {
            /*twin case, twin driver will take care size setting, we just check size should > 0*/
            if(0 == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_XSIZE)){
                CAM_FUNC_ERR("bnr bpc_xize must > 0");
                rst = MFALSE;
            }
            if(0 == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_YSIZE)){
                CAM_FUNC_ERR("bnr bpc_yize must > 0");
                rst = MFALSE;
            }
        }

    }
    else{//case for bad pix
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_EN)){
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN) ^ \
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN) ){
                rst = MFALSE;
                CAM_FUNC_ERR("can't user bpc look-up table without input table\n");
            }
        }
    }


    if(rst == MFALSE){
        CAM_FUNC_ERR("BNR check fail\n");
    }
    return rst;
}

MBOOL PIPE_CHECK::PD_TABLE_CHECK(MUINTPTR va,MUINT32 tbl_stride)
{
    MBOOL rst = MTRUE;
    if(va == 0 ){
        CAM_FUNC_ERR("PD table's va is NULL, can't check table's content\n");
        rst = MFALSE;
    }
    else{
    #define SPECIAL_TOKEN 0xc000
        unsigned short int* ptr = (unsigned short int*)va;
        MUINT32 n_2bytejump = 0;
        MUINT32 ysize = 0;

        for(MUINT32 i=0; i<tbl_stride; )
        {
            //pd line
            if( ((*ptr)&SPECIAL_TOKEN)==SPECIAL_TOKEN){
                ysize++;
                //jump ofset & cnum
                n_2bytejump = 3;
                i += (n_2bytejump*2);
                ptr += n_2bytejump;
                //jump number of pnum
                n_2bytejump = (*ptr) + 1 + 1;//+ 1 for next special token , +1 for pnum indexing is start from 0
                i += (n_2bytejump*2);
                ptr += n_2bytejump;
            }
            else{
                i+=2;
                ptr = ptr + 1;
            }
        }
    #undef SPECIAL_TOKEN
        if(ysize != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_YSIZE,YSIZE) + 1)){
            ptr = (unsigned short int*)va;
            CAM_FUNC_ERR("PD table error:0x%x_0x%x\n",ysize,(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_YSIZE,YSIZE) + 1));
            CAM_FUNC_ERR("table content : 1st pdc line : 0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x\n",
                ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);
            rst = MFALSE;
        }

    }

    return rst;
}

MBOOL PIPE_CHECK::TG_CHECK(MUINT32 intErrStatus)
{
    MBOOL rst = MTRUE;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    MUINT32 TG_IN_W,TG_IN_V;
    MUINT32 TG_IN_W_R,TG_IN_V_R;
    MUINT32 DATA_CNT_R;
    MUINT32 tmp, needReset = 0;
    MBOOL bTgRdy;
    SENINF_DBG seninf_dbg;
    CAM_REG_CTL_RAW_INT_STATUS irqStatCheck;
    MUINT32 bInnerDump = MTRUE;
    CAM_REG_TG_SEN_MODE sen_mode;
    MUINT32 pixmode = 0;
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
        CAM_FUNC_ERR("viewfinder is not opened yet,flase alarm\n");
        rst = MTRUE;
        goto EXIT;
    }

    irqStatCheck.Raw = intErrStatus;

    if (irqStatCheck.Bits.TG_GBERR_ST) {
        needReset = 1;
    }

    //under twin case, sgg_sel won't be 0 , so , don't need to take into consideration at twin case
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    sen_mode.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE);

    pixmode = sen_mode.Bits.DBL_DATA_BUS + sen_mode.Bits.DBL_DATA_BUS1;
    switch(pixmode){
        case 0: // 1pix
            break;
        case 1: // 2pix
            if(TG_W.Bits.PXL_S & 0x1){
                CAM_FUNC_ERR("TG's cooordinate must be 2-alignment at 2 pix mode\n");
                rst = MFALSE;
            }
            if(TG_W.Bits.PXL_E & 0x1){
                CAM_FUNC_ERR("TG's cooordinate must be 2-alignment at 2 pix mode\n");
                rst = MFALSE;
            }
            break;
        case 2: // 4pix
            if(TG_W.Bits.PXL_S & 0x3){
                CAM_FUNC_ERR("TG's cooordinate must be 4-alignment at 4 pix mode\n");
                rst = MFALSE;
            }
            if(TG_W.Bits.PXL_E & 0x3){
                CAM_FUNC_ERR("TG's cooordinate must be 4-alignment at 4 pix mode\n");
                rst = MFALSE;
            }
            break;
        default:
            break;
    }

    TG_IN_W = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST,PXL_CNT);
    TG_IN_V = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST,LINE_CNT);


    TG_IN_W_R = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST_R,PXL_CNT);
    TG_IN_V_R = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST_R,LINE_CNT);

    DATA_CNT_R = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_DAT_NO_R,DAT_NO);

    if(TG_IN_W < (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S)){
        bInnerDump = MFALSE;
        CAM_FUNC_ERR("current seninf horizontal data is small than grab window_w:%d_%d\n",TG_IN_W,(TG_W.Bits.PXL_E - TG_W.Bits.PXL_S));
        rst = MFALSE;
    }
    if(TG_IN_V < (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S)){
        bInnerDump = MFALSE;
        CAM_FUNC_ERR("current seninf vertical data is small than grab window_v:%d_%d\n",TG_IN_V,(TG_H.Bits.LIN_E - TG_H.Bits.LIN_S));
        rst = MFALSE;
    }
    if(bInnerDump == MTRUE){
        if(irqStatCheck.Bits.TG_GBERR_ST){//this case meas tg_grab is happened during streaming. (not happned af the beginning of streaming)
            #define LOOP (10)
            char str[256] = {'\0'};
            char tmp[32] = {'\0'};
            for(MUINT32 i=0;i<LOOP;i++){
                snprintf(tmp,sizeof(tmp),"0x%x,",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST_R));
                strncat(str,tmp,sizeof(tmp));
                usleep(500);
            }
            CAM_FUNC_ERR("Frm_R size:%s\n",str);
        }
    }


    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,PBN_EN)) {
        if(((TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) % 16 ) != 0){
            CAM_FUNC_ERR("tg grab width must be 16-alignment(0x%x) when pbn_en=1\n",\
                (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S));
            rst = MFALSE;
        }
    }

    tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_INTER_ST,TG_CAM_CS);
    switch(tmp){
        case 2: //
        case 16:
            usleep(1000);
            if(TG_IN_W_R != TG_IN_W){
                if(DATA_CNT_R == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_DAT_NO_R,DAT_NO)){
                    CAM_FUNC_ERR("seninf have no input data for over 1ms when TG is under exposure,cur data:0x%x\n",DATA_CNT_R);
                    rst = MFALSE;
                }
            }
            break;
        default:
            CAM_FUNC_ERR("TG is in idle status:0x%x\n",tmp);
            rst = MTRUE;
            break;

    }
    bTgRdy = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_RAW_RDY_STATUS,TG_RDY_STATUS);
    if(bTgRdy == MFALSE){
        CAM_FUNC_INF("current TG is overrun\n");
        rst = MFALSE;
    }


    if(rst == MFALSE){
        CAM_FUNC_ERR("TG checkl fail\n");
    }

    if(seninf_dbg.m_fp_Sen != NULL){
        E_ISP_CAM_CQ cq;
        MUINT32 page;
        ISP_HW_MODULE module;

        CAM_FUNC_ERR("start dump seninf info\n");

        this->m_pDrv->getCurObjInfo(&module,&cq,&page);
        seninf_dbg.m_fp_Sen((MUINT32)module, needReset);
    }

EXIT:

    return rst;

}

MBOOL PIPE_CHECK::TWIN_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 TwinNum;
    ISP_DRV_CAM* ptr[PHY_CAM-1];
    UniDrvImp*      pUni =  (UniDrvImp*)UniDrvImp::createInstance(UNI_A);
    CAM_UNI_REG_TOP_CTL top;
    MUINT32 master;
    MUINT32 slave;

    memset((void*)ptr, 0, sizeof(ptr));
    master = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,MASTER_MODULE);

    TwinNum = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM);
    CAM_FUNC_ERR("twinnum:%d,master:%d\n",TwinNum,master);

    for(MUINT32 i=0;i<TwinNum;i++){
        if(i==0){
            slave = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE);
            CAM_FUNC_ERR("1st slave cam:%d\n",slave);
        }
        else if(i==1){
            slave = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TRIPLE_MODULE);
            CAM_FUNC_ERR("2nd slave cam:%d\n",slave);
        }

        ptr[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)slave,ISP_DRV_CQ_THRE0,0,"TWIN_CHK");
        if(ptr[i]){
            //check cq counter
            if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SPARE1) !=
                CAM_READ_REG(ptr[i]->getPhyObj(),CAM_CTL_SPARE1) ){
                CAM_FUNC_ERR("update timing of master & slave's CQ is over p1 done\n");
                CAM_FUNC_ERR("master's:0x%x,slave's:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SPARE1),\
                    CAM_READ_REG(ptr[i]->getPhyObj(),CAM_CTL_SPARE1));
                rst = MFALSE;
            }

            //cq check
            if(CAM_READ_REG(ptr[i]->getPhyObj(),CAM_CTL_START) != 0){
                CAM_FUNC_ERR("CQ SMI request error\n");
                rst = MFALSE;
            }

            switch(master){
                case CAM_A:
                    if( CAM_READ_BITS(ptr[i]->getPhyObj(),CAM_CTL_SEL,DMX_SEL) != 1){
                        CAM_FUNC_ERR("DMX_SEL error:\n");
                        rst = MFALSE;
                    }

                    if( CAM_READ_BITS(ptr[i]->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID) != 0){
                        CAM_FUNC_ERR("DMX_ID error:\n");
                        rst = MFALSE;
                    }

                    if(TwinNum == 1){   //ac/ab
                        top.Raw = UNI_READ_REG(pUni,CAM_UNI_TOP_CTL);
                        switch(slave){
                            case CAM_B:
                                if(top.Bits.RAW_A_XMX_IN_SEL != 1){
                                    CAM_FUNC_ERR("xmx_in_sel error\n");
                                    rst = MFALSE;
                                }
                                break;
                            case CAM_C:
                                if( (top.Bits.RAW_A_XMX_IN_SEL + top.Bits.RAW_C_XMX_IN_SEL) != 0){
                                    CAM_FUNC_ERR("xmx_in_sel error\n");
                                    rst = MFALSE;
                                }
                                break;
                            default:
                                CAM_FUNC_ERR("unsupported slave:%d\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE));
                                rst = MFALSE;
                                break;
                        }
                    }
                    if(TwinNum == 2){ //masterA, 3raw
                        top.Raw = UNI_READ_REG(pUni,CAM_UNI_TOP_CTL);
                        if((top.Bits.RAW_A_XMX_IN_SEL * top.Bits.RAW_B_XMX_IN_SEL * top.Bits.RAW_C_XMX_IN_SEL) != 1){
                            CAM_FUNC_ERR("xmx_in_sel error\n");
                            rst = MFALSE;
                        }
                    }
                    break;
                case CAM_B:
                    if( CAM_READ_BITS(ptr[i]->getPhyObj(),CAM_CTL_SEL,DMX_SEL) != 1){
                        CAM_FUNC_ERR("DMX_SEL error:\n");
                        rst = MFALSE;
                    }

                    if( CAM_READ_BITS(ptr[i]->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID) != 1){
                        CAM_FUNC_ERR("DMX_ID error:\n");
                        rst = MFALSE;
                    }

                    if(TwinNum == 1){   //bc
                        top.Raw = UNI_READ_REG(pUni,CAM_UNI_TOP_CTL);
                        switch(slave){
                            case CAM_C:
                                if( (top.Bits.RAW_B_XMX_IN_SEL * top.Bits.RAW_C_XMX_IN_SEL) != 1){
                                    CAM_FUNC_ERR("xmx_in_sel error\n");
                                    rst = MFALSE;
                                }
                                break;
                            default:
                                CAM_FUNC_ERR("unsupported slave:%d\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE));
                                rst = MFALSE;
                                break;
                        }
                    }
                    if(TwinNum == 2){   //bca
                        if( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_MODULE) == CAM_C) &&
                            (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TRIPLE_MODULE) == CAM_A) ){


                            top.Raw = UNI_READ_REG(pUni,CAM_UNI_TOP_CTL);
                            if((top.Bits.RAW_A_XMX_IN_SEL * top.Bits.RAW_B_XMX_IN_SEL * top.Bits.RAW_C_XMX_IN_SEL) != 1){
                                CAM_FUNC_ERR("xmx_in_sel error\n");
                                rst = MFALSE;
                            }
                        }
                        else{
                            CAM_FUNC_ERR("unsupported triple path\n");
                            rst = MFALSE;
                        }
                    }
                    break;
                default:
                    CAM_FUNC_ERR("unsupported master:%d\n",master);
                    rst = MFALSE;
                    break;
            }

            //check slave module
            {
                PIPE_CHECK twin_check;
                twin_check.m_pDrv = ptr[i];


                if( twin_check.LSC_CHECK() == MTRUE){
                    CAM_FUNC_INF("slave:%d LSC/LSCI check pass!\n",slave);
                }
                else{
                    rst += 1;
                    CAM_FUNC_ERR("slave:%d LSC/LSCI check fail\n",slave);
                }

                if( twin_check.RRZ_CHECK() == MTRUE){
                    CAM_FUNC_INF("slave:%d RRZ check pass!\n",slave);
                }
                else{
                    rst += 1;
                    CAM_FUNC_ERR("slave:%d RRZ check fail\n",slave);
                }

                if(twin_check.XCPN_CHECK() == MTRUE){
                    CAM_FUNC_INF("slave:%d CPN/DCPN check pass\n",slave);
                }
                else{
                    rst += 1;
                    CAM_FUNC_ERR("slave:%d CPN/DCPN check fail\n",slave);
                }

                if( twin_check.AF_CHECK(MTRUE) == MTRUE){
                    CAM_FUNC_INF("slave:%d af/afo check pass!\n",slave);
                }
                else{
                    rst += 1;
                    CAM_FUNC_ERR("slave:%d af/afo check fail\n",slave);
                }

                if(twin_check.UFEG_CHECK() == MTRUE){
                    CAM_FUNC_INF("slave:%d UFEG check pass\n",slave);
                }
                else{
                    rst += 1;
                    CAM_FUNC_ERR("slave:%d UFEG check fail\n",slave);
                }

                if(twin_check.UFE_CHECK() == MTRUE){
                    CAM_FUNC_INF("slave:%d UFE check pass\n",slave);
                }
                else{
                    rst += 1;
                    CAM_FUNC_ERR("slave:%d UFE check fail\n",slave);
                }
            }
        }
    }


    for(MUINT32 i=0;i<TwinNum;i++){
        if(ptr[i]){
            ptr[i]->destroyInstance();
        }
    }
    pUni->destroyInstance();
    return rst;
}

MBOOL PIPE_CHECK::RRZ_CHECK(void)
{
    MBOOL           rst = MTRUE;
    MUINT32         rrz_in[2];
    MUINT32         rrz_crop[4];
    UniDrvImp*      pUni;
    capibility CamInfo;
    tCAM_rst ret;
    MUINT32 rlb_oft;
    MUINT32 phy_in_w,phy_in_h;

    //support no scale-up
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RRZ_EN) != 1){
        CAM_FUNC_ERR("rrz is disabled, is it corrected??\n");
        goto EXIT;
    }
    else{
        pUni =  (UniDrvImp*)UniDrvImp::createInstance(UNI_A);

        if(UNI_READ_BITS(pUni,CAM_UNI_TOP_MOD_EN,RLB_A_EN) != 1){
            CAM_FUNC_ERR("can't enable rrz without RLB\n");
            rst = MFALSE;
            goto EXIT;
        }
        pUni->destroyInstance();
    }

    rrz_in[0] = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RRZ_IN_IMG);
    rrz_in[1] = rrz_in[0]>>16;
    rrz_in[0] = rrz_in[0] & 0xffff;
    this->InPutSize_DMX(phy_in_w,phy_in_h);

    if(rrz_in[0] > phy_in_w){
        CAM_FUNC_ERR("rrz_hor input setting > input size(%d_%d)\n",rrz_in[0],phy_in_w);
        rst = MFALSE;
        goto EXIT;
    }
    if(rrz_in[1] > phy_in_h){
        CAM_FUNC_ERR("rrz_ver input setting > input size(%d_%d)\n",rrz_in[1],phy_in_h);
        rst = MFALSE;
        goto EXIT;
    }

    rrz_crop[0] = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RRZ_HORI_INT_OFST);
    rrz_crop[1] = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RRZ_VERT_INT_OFST);

    rrz_crop[2] = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RRZ_OUT_IMG);
    rrz_crop[3] = rrz_crop[2]>>16;
    rrz_crop[2] = rrz_crop[2] & 0xffff;

    if((rrz_crop[0] + rrz_crop[2]) > rrz_in[0]){
        CAM_FUNC_ERR("scaler support no hor-scaling-up(in:%d, crop:%d_%d)\n",rrz_in[0],rrz_crop[0],rrz_crop[2]);
        rst = MFALSE;
    }

    if((rrz_crop[1] + rrz_crop[3]) > rrz_in[1]){
        CAM_FUNC_ERR("scaler support no ver-scaling-up(in:%d, crop:%d_%d)\n",rrz_in[1],rrz_crop[1],rrz_crop[3]);
        rst = MFALSE;
    }


    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,\
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),ret,E_CAM_BS_Alignment);
    rlb_oft = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RRZ_RLB_AOFST,RRZ_RLB_AOFST);
    if((rlb_oft % ret.bs_info.bs_alignment) != 0){
        CAM_FUNC_ERR("RLB_OFFSET must be %d alignment:%d\n",ret.bs_info.bs_alignment,rlb_oft);
        rst = MFALSE;
    }

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,\
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),ret,E_CAM_BS_Max_size);

    if((rrz_crop[2] + rlb_oft) > ret.bs_info.bs_max_size){
        CAM_FUNC_ERR("rrz_hor over sram size:%d_%d_%d\n",rrz_crop[2],rlb_oft,ret.bs_info.bs_max_size);
        rst = MFALSE;
    }

EXIT:
    return rst;
}

MBOOL PIPE_CHECK::InPutSize_TG(MUINT32& in_w,MUINT32& in_h,MBOOL pure)
{
    MBOOL rst = MTRUE;
    CAM_REG_CTL_EN  en1;
    CAM_REG_CTL_EN2 en2;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;


    if(this->m_pDrv == NULL){
        in_w = in_h = 0;
        BASE_LOG_ERR("can't be NULL ptr, get no input size\n");
        return MFALSE;
    }

    en1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN);
    en2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN2);


    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    in_w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    in_h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    if(pure == MFALSE){
        in_w = in_w >> (en1.Bits.BIN_EN + en1.Bits.DBN_EN);
        in_h = in_h >> (en1.Bits.BIN_EN + en2.Bits.VBN_EN);
        CAM_FUNC_WRN("input size from tg after bin:%d_%d\n",in_w,in_h);
    }
    else
        CAM_FUNC_WRN("input size from tg without bin:%d_%d\n",in_w,in_h);

    return rst;
}

MBOOL PIPE_CHECK::InPutSize_DMX(MUINT32& in_w,MUINT32& in_h)
{
    MBOOL rst = MTRUE;
    CAM_REG_CTL_EN  en1;
    CAM_REG_CTL_EN2 en2;
    CAM_REG_DMX_CROP    dmx_w;
    CAM_REG_DMX_VSIZE   dmx_h;

    if(this->m_pDrv == NULL){
        in_w = in_h = 0;
        BASE_LOG_ERR("can't be NULL ptr, get no input size\n");
        return MFALSE;
    }

    en1.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN);
    en2.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN2);

    dmx_w.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMX_CROP);
    dmx_h.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMX_VSIZE);

    in_w = dmx_w.Bits.DMX_END_X - dmx_w.Bits.DMX_STR_X + 1;
    in_h = dmx_h.Bits.DMX_HT;

    in_w = in_w >> (en1.Bits.BIN_EN + en1.Bits.DBN_EN);
    in_h = in_h >> (en1.Bits.BIN_EN + en2.Bits.VBN_EN);

    CAM_FUNC_WRN("input size from dmx after bin:%d_%d\n",in_w,in_h);
    return rst;
}

