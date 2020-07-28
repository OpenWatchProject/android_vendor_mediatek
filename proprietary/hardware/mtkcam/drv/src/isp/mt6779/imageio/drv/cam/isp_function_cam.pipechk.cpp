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

#define LOG_TAG "ifunc_cam_ppc"

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
    if (func_cam_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_WRN("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_cam_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define CAM_READ_REG_INR(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readInnerReg(__offset,CAM_A);\
})


#define CAM_READ_BITS_INR(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readInnerReg(__offset,CAM_A);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

FP_SEN SENINF_DBG::m_fp_Sen = NULL;
Mutex PIPE_CHECK::m_lock[PHY_CAM];
E_PIPECHK_HW_PATH PIPE_CHECK::m_HWPath[PHY_CAM] = {E_PPC_MAX,E_PPC_MAX,E_PPC_MAX};

#define XXType_Main_check(path,type,twin_type,twin) {\
    type xx_chk(this->m_pDrv);\
    DYUV_PIPECHK yuv_chk(this->m_pDrv);\
    CAM_FUNC_WRN("start check with hw path:%d\n",path);\
    if(path == E_PPC_DL){\
        if(this->MAIN_CHECK() == MFALSE)\
            rst += 1;\
    }\
    else{\
        if(xx_chk.MAIN_CHECK() == MFALSE)\
            rst += 1;\
    }\
    yuv_chk.CHECK_START();/*YUV path check*/\
    if(twin.Bits.TWIN_EN == MTRUE){\
        ISP_DRV_CAM* ptr[PHY_CAM-1];\
        MUINT32 slave;\
        for(MUINT32 i=0;i<twin.Bits.SLAVE_CAM_NUM;i++){\
            if(i==0){\
                slave = twin.Bits.TWIN_MODULE;\
            }\
            else if(i==1){\
                CAM_FUNC_ERR("NO 2nd slave cam\n");\
                break;\
            }\
            ptr[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)slave,ISP_DRV_CQ_THRE0,0,"Check_Start");\
            if(ptr[i]){\
                twin_type xx_twin_chk(ptr[i]);\
                DYUV_PIPECHK twin_yuv_chk(ptr[i]);\
                if(xx_twin_chk.MAIN_CHECK(this->m_pDrv) == MFALSE)\
                    rst += 1;\
                else{\
                    CAM_FUNC_WRN("hw path:%d Twin pass\n",path);\
                }\
                twin_yuv_chk.CHECK_START();/*YUV path check*/\
                ptr[i]->destroyInstance();\
            }\
        }\
    }\
    if( rst == 0){\
        if(this->ENQUE_CHECK() == MFALSE){\
        }\
        else{\
            CAM_FUNC_ERR("hw path:%d find no err, plz look for previous err ,like enque flow err\n",path);\
        }\
    }\
}

MBOOL PIPE_CHECK::Check_Start(void)
{
    MUINT32 rst = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin_status;
    struct ISP_GET_CLK_INFO getClkInfo;

    if(this->m_pDrv == NULL){
        BASE_LOG_ERR("can't be NULL ptr, no pipechk\n");
        return MFALSE;
    }

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(this->m_hwModule >= PHY_CAM){
        CAM_FUNC_ERR("unsupported module, no pipehck\n");
        return MFALSE;
    }

    Mutex::Autolock lock(PIPE_CHECK::m_lock[this->m_hwModule]);

    if(this->m_HWPath[this->m_hwModule] == E_PPC_MAX){
        CAM_FUNC_ERR("need to register hw path 1st\n");
        return MFALSE;
    }
    else
        CAM_FUNC_WRN("check with hardware path:%d\n",this->m_HWPath[this->m_hwModule]);

    twin_status.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);

    CAM_FUNC_WRN("master cam:%d\n",this->m_hwModule);
    CAM_FUNC_WRN("current twin is %d\n",twin_status.Bits.TWIN_EN);
    for(MUINT32 i=0;i<twin_status.Bits.SLAVE_CAM_NUM;i++){
        if(i==0)
            CAM_FUNC_WRN("slave%d cam:%d\n",i,twin_status.Bits.TWIN_MODULE);
        else{
            rst += 1;
            CAM_FUNC_ERR("only 1 slave cam r supported\n");
            //over-write num for err check below
            twin_status.Bits.SLAVE_CAM_NUM = 1;
        }

        if( twin_status.Bits.MASTER_MODULE != this->m_hwModule){
            CAM_FUNC_ERR("can't start pipechk with slave cam:%d\n",this->m_hwModule);
            goto EXIT;
        }
    }


    //err status:
    memset((MUINT8*)&this->m_err_status,0 ,sizeof(struct ISP_RAW_INT_STATUS));
    this->m_pDrv->getDeviceInfo(_GET_INT_ERR,(MUINT8 *)&this->m_err_status);
    if( (this->m_err_status.ispIntErr + this->m_err_status.ispInt3Err + this->m_err_status.ispInt4Err + this->m_err_status.ispInt5Err) == 0){
        CAM_FUNC_WRN("deque fail with no INT err ???????????\n");
    }
    else{
        CAM_FUNC_ERR("accumulated err int_status:0x%x, int3_status:0x%x, int4_status:0x%x, int5_status:0x%x\n",\
            this->m_err_status.ispIntErr, this->m_err_status.ispInt3Err, this->m_err_status.ispInt4Err, this->m_err_status.ispInt5Err);
        rst += 1;
    }

    if ((this->m_pDrv->getDeviceInfo(_GET_CUR_ISP_CLOCK, (MUINT8*)&getClkInfo)) == MFALSE) {
        CAM_FUNC_ERR("Fail to Get ISP CLK\n");
        rst += 1;
    }
    else{
        CAM_FUNC_WRN("current ISP clk :%d,latest expected ISP clk :%d\n",getClkInfo.curClk, getClkInfo.targetClk);
    }

    //dump phy reg 1st, due to db_en will be disabled during check rdy/req
    CAM_FUNC_ERR("start dump master CAM register\n");
    this->m_pDrv->DumpReg(MTRUE);

    if(twin_status.Bits.TWIN_EN == 1){
        ISP_DRV_CAM* ptr[PHY_CAM-1];
        memset((void*)ptr, 0, sizeof(ptr));


        for(MUINT32 i=0;i<twin_status.Bits.SLAVE_CAM_NUM;i++){
            if(i==1){
                CAM_FUNC_ERR("2nd slave cam r not supported\n");
            }
            else{
                CAM_FUNC_ERR("start dump cam_%d register\n",twin_status.Bits.TWIN_MODULE);
                ptr[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)twin_status.Bits.TWIN_MODULE,ISP_DRV_CQ_THRE0,0,"TWIN_DUMP");
                if(ptr[i]){
                    ptr[i]->DumpReg(MTRUE);
                    ptr[i]->destroyInstance();
                }
            }
        }
    }

    switch(this->m_HWPath[this->m_hwModule])
    {
        case E_PPC_DL:
            XXType_Main_check(E_PPC_DL,PIPE_CHECK,TWIN_PIPECHK,twin_status);
            break;
        case E_PPC_DC:
            XXType_Main_check(E_PPC_DC,DC_PIPECHK,DC_TWIN_PIPECHK,twin_status);
            break;
        case E_PPC_RAWI:
            XXType_Main_check(E_PPC_RAWI,RAWI_PIPCHK,RAWI_TWIN_PIPCHK,twin_status);
            break;
        default:
            CAM_FUNC_ERR("unsupported hw path:%d\n",this->m_HWPath[this->m_hwModule]);
            return MFALSE;
            break;
    }

EXIT:

    if(rst)
        return MFALSE;
    else
        return MTRUE;
}

MBOOL PIPE_CHECK::Register_HW_PATH(E_PIPECHK_HW_PATH path)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    if(this->m_pDrv == NULL){
        BASE_LOG_ERR("can't be NULL ptr, no pipechk\n");
        return MFALSE;
    }

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(this->m_hwModule >= PHY_CAM){
        CAM_FUNC_ERR("unsupported module, no pipehck\n");
        return MFALSE;
    }

    this->m_HWPath[this->m_hwModule] = path;

    CAM_FUNC_WRN("registered HW path:%d\n",path);
    return MTRUE;
}

MBOOL PIPE_CHECK::SEPMRG_CHECK(void)
{
    MBOOL rst = MTRUE;
    #define MRG_MODE(str,en)   {\
        if(en.Bits.CAMCTL_MRG_##str##_EN){\
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),MRG_##str##_MRG_CTL,MRG_SIG_MODE1) != 1){\
                CAM_FUNC_ERR("MODE1 != 1\n");\
                rst = MFALSE;\
            }\
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),MRG_##str##_MRG_CTL,MRG_SIG_MODE2) != 0){\
                CAM_FUNC_ERR("MODE2 != 0\n");\
                rst = MFALSE;\
            }\
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),MRG_##str##_MRG_CTL,MRG_EDGE) != 0xF){\
                CAM_FUNC_ERR("EDGE should be 0xf\n");\
                rst = MFALSE;\
            }\
        }\
    }
    //check only twin_en != 1.
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN) == 0){
        if(this->m_hwModule != CAM_C){
            REG_CAMCTL_R1_CAMCTL_EN  en;
            REG_CAMCTL_R1_CAMCTL_EN2 en2;
            REG_CAMCTL_R1_CAMCTL_EN3 en3;
            REG_CAMCTL_R1_CAMCTL_EN4 en4;
            en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN);
            en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2);
            en3.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3);
            en4.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN4);
            MRG_MODE(R7,en);
            MRG_MODE(R5,en);
            MRG_MODE(R1,en);
            MRG_MODE(R8,en2);
            MRG_MODE(R2,en2);
            MRG_MODE(R3,en4);
            MRG_MODE(R4,en);
            /*MRG_MODE(R6);*/   //no this mrg
            MRG_MODE(R10,en);
            MRG_MODE(R2,en2);
            MRG_MODE(R11,en3);
            MRG_MODE(R12,en3);
            MRG_MODE(R13,en3);
            MRG_MODE(R14,en3);

            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),SEP_R1_SEP_CTL,SEP_EDGE) != 0xF){
                rst = MFALSE;
                CAM_FUNC_ERR("EDGE should be 0xf\n");
            }
        }
    }

    return rst;
}

RAWI_PIPCHK::RAWI_PIPCHK()
{
    BASE_LOG_ERR("obj can't be NULL\n");
    m_pDrv = NULL;
}

RAWI_PIPCHK::RAWI_PIPCHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL\n");
        m_pDrv = NULL;
    }
    else{
        m_pDrv = obj;
        m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    }
}

MBOOL RAWI_PIPCHK::_Front(MVOID* obj)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_SEL sel;
    REG_CAMCTL_R1_CAMCTL_SEL2 sel2;
    obj;


    sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL);
    sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);
    if( sel.Bits.CAMCTL_SEP_SEL != 0){
        CAM_FUNC_ERR("SEP_SEL error:\n");
        rst = MFALSE;
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("tg_en should be enabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_CAMSV_DC_MODE) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("cd mode should be disabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_STAG_MODE) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("stag mode should be enabled\n");
    }

    switch(sel.Bits.CAMCTL_RAW_SEL){
        case 2:
            if(this->RAWI_CHECK() == MFALSE)
                rst = MFALSE;

            if(this->UFDI_CHECK() == MFALSE)
                rst = MFALSE;

            break;
        default:
            CAM_FUNC_ERR("RAW_SEL error:%d\n",sel.Bits.CAMCTL_RAW_SEL);
            rst = MFALSE;
            break;
    }

    if(this->PDO_CHECK() == MTRUE){
        CAM_FUNC_WRN("pdo check pass!\n");
    }
    else
        rst = MFALSE;


    return rst;
}


MBOOL RAWI_PIPCHK::RAWI_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 in_w,in_h;

    this->InPutSize_TG(in_w,in_h,MTRUE);

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RAWI_R2_EN)!= 1){
        CAM_FUNC_ERR("rawi_en must be enabled\n");
        rst = MFALSE;
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAWI_R2)!= 1){
        CAM_FUNC_WRN("2pix mode is supported for best performance\n");
    }
    else if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAWI_R2)>= 2){
        rst = MFALSE;
        CAM_FUNC_ERR("no 4pix mode or higher\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_UNP_R2_EN) != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("unp err\n");
    }


    if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RAWI_R2_RAWI_XSIZE,RAWI_XSIZE) *
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RAWI_R2_RAWI_STRIDE,RAWI_STRIDE)) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("RAWI xsize|stride = 0\n");
    }


    if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RAWI_R2_RAWI_YSIZE,RAWI_YSIZE)+1) != in_h){
        CAM_FUNC_ERR("RAWI y-size mismatch(0x%x_0x%x)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RAWI_R2_RAWI_YSIZE,RAWI_YSIZE)+1,in_h);
        rst = MFALSE;
    }
    //on purpose?
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RAWI_R2_RAWI_YSIZE,RAWI_YSIZE) == 0){
        CAM_FUNC_WRN("RAWI ysize = 0?\n");
    }

    return rst;
}

MBOOL RAWI_PIPCHK::UFDI_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 in_w,in_h;

    this->InPutSize_TG(in_w,in_h,MTRUE);

    if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_UFD_R2_EN) ^ \
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFDI_R2_EN)) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("ufd err\n");
    }


    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFDI_R2_EN)){

        if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFO_RAWI_R2_EN) != 1){
            rst = MFALSE;
            CAM_FUNC_ERR("need to enable ufi mode\n");
        }

        /////////////////////////////////////
        if(CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), RAWI_R2_RAWI_CROP) != 0){
            CAM_FUNC_ERR("UFDI can't be enabled under RAWI crop!= 0(0x%x)\n",CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), RAWI_R2_RAWI_CROP));
            rst = MFALSE;
        }

        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RAWI_R2_RAWI_YSIZE,RAWI_YSIZE)+1) != in_h){
            CAM_FUNC_ERR("RAWI y-size mismatch(0x%x_0x%x)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RAWI_R2_RAWI_YSIZE,RAWI_YSIZE)+1,in_h);
            rst = MFALSE;
        }

        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAWI_UFO_SEL)) != 1){
            CAM_FUNC_ERR("UFDI opened but rawi_ufo_sel is %d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAWI_UFO_SEL));
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RAWI_R2_RAWI_STRIDE,RAWI_STRIDE)%16) != 0){
            CAM_FUNC_ERR("stride should be 16B align\n");
            rst = MFALSE;
        }
        if(( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RAWI_R2_RAWI_BASE_ADDR,RAWI_BASE_ADDR) + CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RAWI_R2_RAWI_OFST_ADDR,RAWI_OFST_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFDI_R2_UFDI_STRIDE,UFDI_STRIDE)%8) != 0){
            CAM_FUNC_ERR("stride should be 8B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFDI_R2_UFDI_YSIZE,UFDI_YSIZE)+1) != in_h){
             CAM_FUNC_ERR("UFDI y-size mismatch(0x%x_0x%x)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), UFDI_R2_UFDI_YSIZE,UFDI_YSIZE)+1,in_h);
            rst = MFALSE;
        }
        if((( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFDI_R2_UFDI_BASE_ADDR,UFDI_BASE_ADDR) + CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFDI_R2_UFDI_OFST_ADDR,UFDI_OFST_ADDR)) %16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
    }


    return rst;
}

RAWI_TWIN_PIPCHK::RAWI_TWIN_PIPCHK()
{
    BASE_LOG_ERR("obj can't be NULL\n");
    m_pDrv = NULL;
}

RAWI_TWIN_PIPCHK::RAWI_TWIN_PIPCHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL\n");
        m_pDrv = NULL;
    }
    else{
        m_pDrv = obj;
        m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    }
}

DC_PIPECHK::DC_PIPECHK()
{
    BASE_LOG_ERR("obj can't be NULL\n");
    m_pDrv = NULL;
}

DC_PIPECHK::DC_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL\n");
        m_pDrv = NULL;
    }
    else{
        m_pDrv = obj;
        m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    }
}

MBOOL DC_PIPECHK::_Front(MVOID* obj)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_SEL sel;
    REG_CAMCTL_R1_CAMCTL_SEL2 sel2;
    IspDrvCamsv* psv;
    REG_CAMSV_TG_VF_CON sv_tg;
    REG_CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2 _reg;
    obj;

    _reg.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2);
    if( _reg.Bits.CAMDMATOP1_DC_CAMSV_STAGER_EN_RAWI_R2 != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("RAWI stag addr err\n");
    }
    if( _reg.Bits.CAMDMATOP1_DC_RAWI_R2_BASE_ADDR_EN != 0){
        rst = MFALSE;
        CAM_FUNC_ERR("RAWI stag addr err\n");
    }

    if( _reg.Bits.CAMDMATOP1_DC_CAMSV_STAGER_DLNO_RAWI_R2 < 1){
        rst = MFALSE;
        CAM_FUNC_WRN("RAWI stag addr err\n");
    }
    else if( _reg.Bits.CAMDMATOP1_DC_CAMSV_STAGER_DLNO_RAWI_R2 < 10){
        CAM_FUNC_WRN("10 is recommended\n");
    }

    sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL);
    sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);
    if( sel.Bits.CAMCTL_SEP_SEL != 0){
        CAM_FUNC_ERR("SEP_SEL error:\n");
        rst = MFALSE;
    }

    if( sel2.Bits.CAMCTL_DCIF_MAIN_EN == 0 ){
        rst = MFALSE;
        CAM_FUNC_ERR("DCIF should be enabled\n");
    }
    if( sel2.Bits.CAMCTL_DCIF_MAIN_SEL != 0 ){
        rst = MFALSE;
        CAM_FUNC_ERR("DCIF sel should be 0\n");
    }

    psv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
    sv_tg.Raw = CAMSV_READ_REG(psv, CAMSV_TG_VF_CON);
    if(sv_tg.Bits.VFDATA_EN && sv_tg.Bits.VFDATA_EN_SEL1){
        if( sel2.Bits.CAMCTL_DCIF_SUB_EN == 0 ){
            rst = MFALSE;
            CAM_FUNC_ERR("DCIF should be enabled\n");
        }
        if( sel2.Bits.CAMCTL_DCIF_SUB_SEL != 1 ){
            rst = MFALSE;
            CAM_FUNC_ERR("DCIF sel should be 1\n");
        }
    }
    else{
        if( sel2.Bits.CAMCTL_DCIF_SUB_EN == 1 ){
            rst = MFALSE;
            CAM_FUNC_ERR("DCIF should be disabled\n");
        }
    }
    psv->destroyInstance();

    if( sel2.Bits.CAMCTL_DCIF_EN_SEL != 0 ){
        rst = MFALSE;
        CAM_FUNC_ERR("DCIF source should be CAM_A\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("tg_en should be disabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_CAMSV_DC_MODE) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("cd mode should be enabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_STAG_MODE) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("stag mode should be disable\n");
    }

    switch(sel.Bits.CAMCTL_RAW_SEL){
        case 2:
            if(this->RAWI_CHECK() == MFALSE)
                rst = MFALSE;

            if(this->UFDI_CHECK() == MFALSE)
                rst = MFALSE;
            else{
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFDI_R2_EN)){
                    REG_CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_UFDI_R2 _reg;

                    _reg.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_UFDI_R2);
                    if( _reg.Bits.CAMDMATOP1_DC_CAMSV_STAGER_EN_UFDI_R2 != 1){
                        rst = MFALSE;
                        CAM_FUNC_ERR("ufd stag addr err\n");
                    }
                    if( _reg.Bits.CAMDMATOP1_DC_UFDI_R2_BASE_ADDR_EN != 0){
                        rst = MFALSE;
                        CAM_FUNC_ERR("ufd stag addr err\n");
                    }

                    if( _reg.Bits.CAMDMATOP1_DC_CAMSV_STAGER_DLNO_UFDI_R2 < 1){
                        rst = MFALSE;
                        CAM_FUNC_WRN("ufd stag addr err\n");
                    }
                    else if( _reg.Bits.CAMDMATOP1_DC_CAMSV_STAGER_DLNO_UFDI_R2 < 10){
                        CAM_FUNC_WRN("10 is recommended\n");
                    }
                }
            }


            break;
        default:
            CAM_FUNC_ERR("RAW_SEL error:%d\n",sel.Bits.CAMCTL_RAW_SEL);
            rst = MFALSE;
            break;
    }

    if(this->PDO_CHECK() == MTRUE){
        CAM_FUNC_WRN("pdo check pass!\n");
    }
    else
        rst = MFALSE;


    return rst;
}


DC_TWIN_PIPECHK::DC_TWIN_PIPECHK()
{
    BASE_LOG_ERR("obj can't be NULL\n");
    m_pDrv = NULL;
}

DC_TWIN_PIPECHK::DC_TWIN_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL\n");
        m_pDrv = NULL;
    }
    else{
        m_pDrv = obj;
        m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    }
}

MBOOL DC_TWIN_PIPECHK::_Front(ISP_DRV_CAM* masterobj)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_SEL sel;
    REG_CAMCTL_R1_CAMCTL_SEL2 sel2;
    REG_CAMCTL_R1_CAMCTL_SEL2 main_sel2;

    sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL);
    sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);

    if( sel.Bits.CAMCTL_SEP_SEL != 1){
        CAM_FUNC_ERR("SEP_SEL error:\n");
        rst = MFALSE;
    }

    main_sel2.Raw = CAM_READ_REG_INR(masterobj->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);

    //for trig slave CQ
    if(sel2.Bits.CAMCTL_DCIF_MAIN_EN != main_sel2.Bits.CAMCTL_DCIF_MAIN_EN){
        CAM_FUNC_ERR("DCIF should be the same as master cam\n");
        rst = MFALSE;
    }
    if(sel2.Bits.CAMCTL_DCIF_SUB_EN != main_sel2.Bits.CAMCTL_DCIF_SUB_EN){
        CAM_FUNC_ERR("DCIF should be the same as master cam\n");
        rst = MFALSE;
    }
    if(sel2.Bits.CAMCTL_DCIF_MAIN_EN){
        if(sel2.Bits.CAMCTL_DCIF_MAIN_SEL != main_sel2.Bits.CAMCTL_DCIF_MAIN_SEL){
            CAM_FUNC_ERR("DCIF should be the same as master cam\n");
            rst = MFALSE;
        }
    }
    if(sel2.Bits.CAMCTL_DCIF_SUB_EN){
        if(sel2.Bits.CAMCTL_DCIF_MAIN_SEL != main_sel2.Bits.CAMCTL_DCIF_MAIN_SEL){
            CAM_FUNC_ERR("DCIF should be the same as master cam\n");
            rst = MFALSE;
        }
    }

    if( sel2.Bits.CAMCTL_DCIF_EN_SEL != 0 ){
        rst = MFALSE;
        CAM_FUNC_ERR("DCIF source should be from CAM_A\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("tg_en should be disabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_CAMSV_DC_MODE) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("cd mode should be enabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_STAG_MODE) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("stag mode should be disable\n");
    }

    return rst;
}

TWIN_PIPECHK::TWIN_PIPECHK()
{
    BASE_LOG_ERR("obj can't be NULL, start NE\n");
    m_pDrv = NULL;
}

TWIN_PIPECHK::TWIN_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL, start NE\n");
    }

    m_pDrv = obj;
    m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
}

MBOOL TWIN_PIPECHK::_Front(ISP_HW_MODULE master)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_SEL2 sel2;

    sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);

    if( sel2.Bits.CAMCTL_DCIF_MAIN_EN || sel2.Bits.CAMCTL_DCIF_SUB_EN ){
        rst = MFALSE;
        CAM_FUNC_ERR("DCIF should be disabled\n");
    }

    if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_SEP_SEL) != 1){
        CAM_FUNC_ERR("SEP_SEL error:\n");
        rst = MFALSE;
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("tg_en should not be enabled\n");
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_CAMSV_DC_MODE) == 1){
        rst = MFALSE;
        CAM_FUNC_ERR("cd mode should be disable\n");
    }

    if(this->m_HWPath[this->m_hwModule] == E_PPC_DL){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_STAG_MODE) == 1){
            rst = MFALSE;
            CAM_FUNC_ERR("stag mode should be disable\n");
        }
    }

    switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL)){
        case 3:
            {
                REG_CAMCTL_R1_CAMCTL_SEL2 sel2;
                sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);
                switch(master){
                    case CAM_A:
                        if(sel2.Bits.CAMCTL_RAW_TG_SEL != 0){
                            rst = MFALSE;
                            CAM_FUNC_ERR("timestamp src err\n");
                        }
                        break;
                    case CAM_B:
                        if(sel2.Bits.CAMCTL_RAW_TG_SEL != 1){
                            rst = MFALSE;
                            CAM_FUNC_ERR("timestamp src err\n");
                        }
                        break;
                    default:
                        rst = MFALSE;
                        CAM_FUNC_ERR("unsupported master:%d\n",master);
                        break;
                }
            }
            break;
        default:
            rst = MFALSE;
            CAM_FUNC_ERR("slave cam will have no timestamp\n");
            break;
    }

#if 0    //no tg , but have timestamp
    if( this->TG_CHECK() == MTRUE){
        CAM_FUNC_WRN("TG check pass!\n");
    }
    else{
        rst = MFALSE;
    }
#else
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_TIME_STAMP_CTL, TG_TG_TIME_STAMP_SOF_SEL) != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("source of SOF can't be itself \n");
    }
#endif
    return rst;
}

MBOOL TWIN_PIPECHK::_MAIN_CHECK(ISP_DRV_CAM* masterobj)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin;
    REG_CAMCTL_R1_CAMCTL_MISC misc_slave;
    REG_CAMCTL_R1_CAMCTL_MISC misc_master;


    twin.Raw = CAM_READ_REG_INR(masterobj->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);


    misc_master.Raw = CAM_READ_REG_INR(masterobj->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);


    if(CAM_READ_REG_INR(masterobj->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER) !=
        CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER) ){
        CAM_FUNC_ERR("update timing of master & slave's CQ is over p1 done\n");
        CAM_FUNC_ERR("master's:0x%x,slave's:0x%x\n",CAM_READ_REG_INR(masterobj->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER),\
            CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_CQ_COUNTER));
        rst = MFALSE;
    }

    misc_slave.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
    if(misc_slave.Bits.CAMCTL_DB_EN == 0 ){
        CAM_FUNC_ERR("need to enable db\n");
        rst = MFALSE;
    }
    else{
        if( (misc_slave.Bits.CAMCTL_DB_LOAD_HOLD) || (misc_slave.Bits.CAMCTL_DB_LOAD_HOLD_SUB) ){
            CAM_FUNC_ERR("why db is still holded(%d_%d)\n",misc_slave.Bits.CAMCTL_DB_LOAD_HOLD,misc_slave.Bits.CAMCTL_DB_LOAD_HOLD_SUB);
            rst = MFALSE;
        }
    }


    //cross-cam check
    if( (misc_master.Bits.CAMCTL_PASS1_DONE_SEL * misc_slave.Bits.CAMCTL_PASS1_DONE_SEL) != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("signal ctrl err\n");
    }
    switch(twin.Bits.MASTER_MODULE){
        case CAM_A:
            switch(this->m_hwModule){
                case CAM_B:
                    if(misc_slave.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x2 | 0x4 | 0x10 | 0x20) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal ctrl err\n");
                    }
                    if(misc_master.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x2 | 0x4 | 0x10 | 0x20) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal ctrl err\n");
                    }

                    break;
                case CAM_C:
                    rst = MFALSE;
                    CAM_FUNC_ERR("ac is not supported\n");
                    break;
                default:
                    CAM_FUNC_ERR("unsupported slave\n");
                    rst = MFALSE;
                    break;
            }
            break;
        case CAM_B:
            if(twin.Bits.SLAVE_CAM_NUM == 1){   //ac/ab
                switch(this->m_hwModule){
                    case CAM_A:
                        if(misc_slave.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x2 | 0x4 | 0x10 | 0x20) ){
                            rst = MFALSE;
                            CAM_FUNC_ERR("signal ctrl err\n");
                        }
                        if(misc_master.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x2 | 0x4 | 0x10 | 0x20) ){
                            rst = MFALSE;
                            CAM_FUNC_ERR("signal ctrl err\n");
                        }

                        break;
                    case CAM_C:
                        rst = MFALSE;
                        CAM_FUNC_ERR("bc is not supported\n");
                        break;
                    default:
                        CAM_FUNC_ERR("unsupported slave\n");
                        rst = MFALSE;
                        break;
                }
            }
            else{
                rst = MFALSE;
                CAM_FUNC_ERR("logic err\n");
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported master:%d\n",twin.Bits.MASTER_MODULE);
            rst = MFALSE;
            break;
    }

    //check slave module
    {
        if(this->ME_CHECK()){
            CAM_FUNC_WRN("no mutually exclusive function enabled\n");
        }
        else
            rst = MFALSE;

        switch(this->HW_Path()){
            case E_PPC_DL:
                {
                    MUINT32 _int = twin.Bits.MASTER_MODULE;
                    if (this->Front((MVOID*)&_int) == MFALSE)
                        rst = MFALSE;
                }
                break;
            case E_PPC_DC:
            case E_PPC_RAWI:
                if (this->Front((MVOID*)masterobj) == MFALSE)
                    rst = MFALSE;
                break;
            default:
                rst = MFALSE;
                CAM_FUNC_ERR("path err_%d\n",this->HW_Path());
            break;
        }
        if (this->Middle() == MFALSE)
            rst = MFALSE;

        if (this->End() == MFALSE)
            rst = MFALSE;

        this->RdyReq_Dump();
    }


    return rst;
}

MBOOL PIPE_CHECK::_Front(MVOID* obj)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_SEL sel;
    REG_CAMCTL_R1_CAMCTL_SEL2 sel2;
    obj;


    sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL);
    sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);
    if( sel.Bits.CAMCTL_SEP_SEL != 0){
        CAM_FUNC_ERR("SEP_SEL error:\n");
        rst = MFALSE;
    }

    switch(sel.Bits.CAMCTL_RAW_SEL){
        case 0:
        case 3:
            if( sel2.Bits.CAMCTL_DCIF_MAIN_EN || sel2.Bits.CAMCTL_DCIF_SUB_EN ){
                rst = MFALSE;
                CAM_FUNC_ERR("DCIF should be disabled\n");
            }

            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN) == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("tg_en should be enabled\n");
            }

            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_CAMSV_DC_MODE) == 1){
                rst = MFALSE;
                CAM_FUNC_ERR("cd mode should be disable\n");
            }

            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_STAG_MODE) == 1){
                rst = MFALSE;
                CAM_FUNC_ERR("stag mode should be disable\n");
            }

            if( this->TG_CHECK() == MTRUE){
                CAM_FUNC_WRN("TG check pass!\n");
            }
            else{
                rst = MFALSE;
            }
            break;
        default:
            CAM_FUNC_ERR("RAW_SEL error:%d\n",sel.Bits.CAMCTL_RAW_SEL);
            rst = MFALSE;
            break;
    }

    if(this->PDO_CHECK() == MTRUE){
        CAM_FUNC_WRN("pdo check pass!\n");
    }
    else
        rst = MFALSE;


    return rst;
}


MBOOL PIPE_CHECK::_Middle(void)
{
    MBOOL rst = MTRUE;

    if( this->OBC_CHECK() == MTRUE){
        CAM_FUNC_WRN("OBC check pass!\n");
    }
    else
        rst = MFALSE;

    if(this->BPC_CHECK() == MTRUE){
        CAM_FUNC_WRN("bnr check pass!\n");
    }
    else{
        rst = MFALSE;
    }

    if( this->LSC_CHECK() == MTRUE){
        CAM_FUNC_WRN("LSC/LSCI check pass!\n");
    }
    else{
        rst = MFALSE;
    }

    if( this->LTM_CHECK() == MTRUE) {
        CAM_FUNC_WRN("LTM check pass");
    }
    else{
        rst = MFALSE;
    }

    return rst;
}

MBOOL PIPE_CHECK::_End(void)
{
    MBOOL rst = MTRUE;

    if( this->RRZ_CHECK() == MTRUE){
        CAM_FUNC_WRN("RRZ check pass!\n");
    }
    else{
        rst = MFALSE;
    }

    if(this->UFEG_CHECK() == MTRUE){
        CAM_FUNC_WRN("UFEG check pass\n");
    }
    else{
        rst = MFALSE;
    }

    if(this->UFE_CHECK() == MTRUE){
        CAM_FUNC_WRN("UFE check pass\n");
    }
    else{
        rst = MFALSE;
    }

    if( this->AF_CHECK() == MTRUE){
        CAM_FUNC_WRN("master af/afo check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->AA_CHECK() == MTRUE){
        CAM_FUNC_WRN("AA/AAO check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->LCS_CHECK() == MTRUE){
        CAM_FUNC_WRN("LCS/LCSO check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->FLK_CHECK() == MTRUE){
        CAM_FUNC_WRN("FLK/FLKO check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->LMV_CHECK() == MTRUE){
        CAM_FUNC_WRN("LMV/LMVO check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->RSS_CHECK() == MTRUE){
        CAM_FUNC_WRN("RSS/RSSO check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->TSF_CHECK() == MTRUE){
        CAM_FUNC_WRN("TSFS/TSFSO check pass!\n");
    }
    else
        rst = MFALSE;

    if( this->LTMS_CHECK() == MTRUE){
        CAM_FUNC_WRN("LTMS/LTMSO check pass!\n");
    }
    else
        rst = MFALSE;

    //must be check last , due to off db_en
    if(this->DMAO_STATUS()){
        CAM_FUNC_WRN("find no dma err\n");
    }
    else
        rst = MFALSE;
    return rst;
}

MBOOL PIPE_CHECK::_MAIN_CHECK(ISP_DRV_CAM* masterobj)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_MISC misc;
    masterobj;

    misc.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
    if( misc.Bits.CAMCTL_DB_EN == 0 ){
        CAM_FUNC_ERR("need to enable db\n");
        rst = MFALSE;
    }
    else{
        if( (misc.Bits.CAMCTL_DB_LOAD_HOLD) || (misc.Bits.CAMCTL_DB_LOAD_HOLD_SUB) ){
            CAM_FUNC_ERR("why db is still holded(%d_%d)\n",misc.Bits.CAMCTL_DB_LOAD_HOLD,misc.Bits.CAMCTL_DB_LOAD_HOLD_SUB);
            rst = MFALSE;
        }
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN) == 0){
        switch(this->m_hwModule){
            case CAM_A:
                if(misc.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x2|0x10) ){
                    rst = MFALSE;
                    CAM_FUNC_ERR("signal err\n");
                }
                break;
            case CAM_B:
                if(misc.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x4|0x20) ){
                    rst = MFALSE;
                    CAM_FUNC_ERR("signal err\n");
                }
                break;
            case CAM_C:
                if(misc.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x8|0x40) ){
                    rst = MFALSE;
                    CAM_FUNC_ERR("signal err\n");
                }
                break;
            default:
                rst = MFALSE;
                CAM_FUNC_ERR("logic err\n");
                break;
        }

        rst = this->SEPMRG_CHECK();
    }
    else{//twin_en = 1 is in twin_path
    }

    //
    if( this->CrossPath_check() == MFALSE){
        rst = MFALSE;
    }

    //
    if(this->Pattern_CHECK()){
        CAM_FUNC_WRN("path check pass\n");
    }
    else
        rst = MFALSE;

    //
    if( this->Front() == MFALSE)
        rst = MFALSE;
    //
    if( this->Middle() == MFALSE)
        rst = MFALSE;

    //
    if( this->End() == MFALSE)
        rst = MFALSE;

    this->RdyReq_Dump();

    return rst;
}

MBOOL PIPE_CHECK::CrossPath_check(void)
{
    MBOOL rst = MTRUE;
    ISP_DRV_CAM* ptr[PHY_CAM];
    REG_CAMCTL_R1_CAMCTL_MISC misc;
    REG_CAMCTL_R1_CAMCTL_MISC misc_other;

    misc.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN) == 0){
        switch(this->m_hwModule){
            case CAM_A:
                if(misc.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x2|0x10) ){
                    rst = MFALSE;
                    CAM_FUNC_ERR("signal err\n");
                }

                ptr[CAM_B] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_B,ISP_DRV_CQ_THRE0,0,"CrossPath_check");
                ptr[CAM_C] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_C,ISP_DRV_CQ_THRE0,0,"CrossPath_check");

                if(ptr[CAM_B]){
                    ptr[CAM_B]->init("CrossPath_check");
                    misc_other.Raw = CAM_READ_REG_INR(ptr[CAM_B]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
                    if(misc_other.Bits.CAMCTL_LAST_PASS1_DONE_CTL & (0x2|0x10) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal err\n");
                    }

                    if( CAM_READ_BITS_INR(ptr[CAM_B]->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_SEP_SEL) == 1){
                        CAM_FUNC_ERR("SEP_SEL error:\n");
                        rst = MFALSE;
                    }
                    ptr[CAM_B]->uninit("CrossPath_check");
                    ptr[CAM_B]->destroyInstance();
                }

                if(ptr[CAM_C])
                {
                    ptr[CAM_C]->init("CrossPath_check");
                    misc_other.Raw = CAM_READ_REG_INR(ptr[CAM_C]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
                    if(misc_other.Bits.CAMCTL_LAST_PASS1_DONE_CTL & (0x2|0x10) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal err\n");
                    }
                    ptr[CAM_C]->uninit("CrossPath_check");
                    ptr[CAM_C]->destroyInstance();
                }


                break;
            case CAM_B:
                if(misc.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x4|0x20) ){
                    rst = MFALSE;
                    CAM_FUNC_ERR("signal err\n");
                }

                ptr[CAM_A] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_A,ISP_DRV_CQ_THRE0,0,"CrossPath_check");
                ptr[CAM_C] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_C,ISP_DRV_CQ_THRE0,0,"CrossPath_check");

                if(ptr[CAM_A]){
                    ptr[CAM_A]->init("CrossPath_check");
                    misc_other.Raw = CAM_READ_REG_INR(ptr[CAM_A]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
                    if(misc_other.Bits.CAMCTL_LAST_PASS1_DONE_CTL & (0x4|0x20) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal err\n");
                    }

                    if( CAM_READ_BITS_INR(ptr[CAM_A]->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_SEP_SEL) == 1){
                        CAM_FUNC_ERR("SEP_SEL error:\n");
                        rst = MFALSE;
                    }
                    ptr[CAM_A]->uninit("CrossPath_check");
                    ptr[CAM_A]->destroyInstance();
                }

                if(ptr[CAM_C])
                {
                    ptr[CAM_C]->init("CrossPath_check");
                    misc_other.Raw = CAM_READ_REG_INR(ptr[CAM_C]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
                    if(misc_other.Bits.CAMCTL_LAST_PASS1_DONE_CTL & (0x4|0x20) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal err\n");
                    }
                    ptr[CAM_C]->uninit("CrossPath_check");
                    ptr[CAM_C]->destroyInstance();
                }
                break;
            case CAM_C:
                if(misc.Bits.CAMCTL_LAST_PASS1_DONE_CTL != (0x8|0x40) ){
                    rst = MFALSE;
                    CAM_FUNC_ERR("signal err\n");
                }

                ptr[CAM_A] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_A,ISP_DRV_CQ_THRE0,0,"CrossPath_check");
                ptr[CAM_B] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_B,ISP_DRV_CQ_THRE0,0,"CrossPath_check");

                if(ptr[CAM_A]){
                    ptr[CAM_A]->init("CrossPath_check");
                    misc_other.Raw = CAM_READ_REG_INR(ptr[CAM_A]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
                    if(misc_other.Bits.CAMCTL_LAST_PASS1_DONE_CTL & (0x8|0x40) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal err\n");
                    }
                    ptr[CAM_A]->uninit("CrossPath_check");
                    ptr[CAM_A]->destroyInstance();
                }

                if(ptr[CAM_B])
                {
                    ptr[CAM_B]->init("CrossPath_check");
                    misc_other.Raw = CAM_READ_REG_INR(ptr[CAM_B]->getPhyObj(),CAMCTL_R1_CAMCTL_MISC);
                    if(misc_other.Bits.CAMCTL_LAST_PASS1_DONE_CTL & (0x8|0x40) ){
                        rst = MFALSE;
                        CAM_FUNC_ERR("signal err\n");
                    }
                    ptr[CAM_B]->uninit("CrossPath_check");
                    ptr[CAM_B]->destroyInstance();
                }
                break;
            default:
                rst = MFALSE;
                CAM_FUNC_ERR("logic err\n");
                break;
        }
    }
    else{
        //twin case is at twin_check
    }

    return rst;
}

MBOOL PIPE_CHECK::Pattern_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 pattern = eCAM_NORMAL;
    pattern = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DATA_PATTERN);

    CAM_FUNC_WRN("current image pattern: %d\n",pattern);
#if 0
    switch(pattern){
        case eCAM_NORMAL:
        case eCAM_NORMAL_PD:
            break;
        case eCAM_DUAL_PIX:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) == MFALSE){
                rst = MFALSE;
                CAM_FUNC_ERR("dbn should be enable under DPD\n");
            }
            break;
        case eCAM_4CELL:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN) == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("fbnd should be enable under 4cell_normal\n");
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

#else
    //Dual PD
    if(pattern == eCAM_DUAL_PIX){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) == MFALSE){
            rst = MFALSE;
            CAM_FUNC_ERR("dbn should be enable under DPD\n");
        }

        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_PDO_SEL) == 1){
            rst = MFALSE;
            CAM_FUNC_ERR("pdo_sel should be 0\n");
        }

        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN) == MTRUE){
            rst = MFALSE;
            CAM_FUNC_ERR("FBND should not be enable\n");
        }
    }
    else{
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) == MTRUE){
            rst = MFALSE;
            CAM_FUNC_ERR("dbn should be disable if pattern is not DPD\n");
        }
    }

    //4-cell
    if(pattern == eCAM_4CELL){
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN) == MFALSE) || \
            (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_FUS_R1_EN) == MFALSE)){
            rst = MFALSE;
            CAM_FUNC_ERR("fbnd/fus should be enable if pattern is 4cell\n");
        }
        //todo, 4cell is not rdy
    }
    else{
    }

    //todo, other pattern is not ready
#endif


    //
    return rst;
}


MBOOL PIPE_CHECK::ME_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_EN2 en2;
    REG_CAMCTL_R1_CAMCTL_EN  en1;

    en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2);
    en1.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN);

    if( (en2.Bits.CAMCTL_BIN_R1_EN * en2.Bits.CAMCTL_FBND_R1_EN ) == 1){
        CAM_FUNC_ERR("BIN/FBND are mutually exclusive func(%d_%d)\n",en2.Bits.CAMCTL_BIN_R1_EN,en2.Bits.CAMCTL_FBND_R1_EN);
        rst = MFALSE;
    }

    if((en2.Bits.CAMCTL_BIN_R1_EN == 1) && (en2.Bits.CAMCTL_DBN_R1_EN ==1)){
        CAM_FUNC_ERR("PBN/BIN are mutually exclusive func(%d_%d)\n",en2.Bits.CAMCTL_BIN_R1_EN,en2.Bits.CAMCTL_DBN_R1_EN);
        rst = MFALSE;
    }

    if((en2.Bits.CAMCTL_FBND_R1_EN == 1) && (en2.Bits.CAMCTL_DBN_R1_EN ==1)){
        CAM_FUNC_ERR("PBN/FBND are mutually exclusive func(%d_%d)\n",en2.Bits.CAMCTL_FBND_R1_EN,en2.Bits.CAMCTL_DBN_R1_EN);
        rst = MFALSE;
    }

    if((en2.Bits.CAMCTL_ZFUS_R1_EN == 1) && (en1.Bits.CAMCTL_FUS_R1_EN ==1)){
        CAM_FUNC_ERR("FUS/ZFUS are mutually exclusive func(%d_%d)\n",en2.Bits.CAMCTL_FBND_R1_EN,en2.Bits.CAMCTL_DBN_R1_EN);
        rst = MFALSE;
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
    imgo_en = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN);
    rrzo_en = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN);
    ufeo_en = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN);
    lcso_en = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LCESO_R1_EN);
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
        imgo_fbc = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FBC_R1_FBC_IMGO_R1_CTL2,FBC_IMGO_R1_FBC_CNT);
        if(imgo_fbc == 0){
            CAM_FUNC_ERR("IMGO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(rrzo_en){
        rrzo_fbc = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FBC_R1_FBC_RRZO_R1_CTL2,FBC_RRZO_R1_FBC_CNT);
        if(rrzo_fbc == 0){
            CAM_FUNC_ERR("RRZO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(ufeo_en){
        ufeo_fbc = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FBC_R1_FBC_UFEO_R1_CTL2,FBC_UFEO_R1_FBC_CNT);
        if(ufeo_fbc == 0){
            CAM_FUNC_ERR("UFEO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(lcso_en){
        lcso_fbc = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FBC_R1_FBC_LCESO_R1_CTL2,FBC_LCESO_R1_FBC_CNT);
        if(lcso_fbc == 0){
            CAM_FUNC_ERR("LCSO have no enque record\n");
            rst = MFALSE;
        }
    }

    #if 0//cannon_ep reg
    if(eiso_en){
        eiso_fbc = UNI_READ_BITS(this->m_pUniDrv,FBC_R1_FBC_LMVO_R1_CTL2,FBC_LMVO_R1_FBC_CNT);
        if(eiso_fbc == 0){
            CAM_FUNC_ERR("EISO have no enque record\n");
            rst = MFALSE;
        }
    }

    if(rsso_en){
        rsso_fbc = UNI_READ_BITS(this->m_pUniDrv,FBC_R1_FBC_RSSO_R1_CTL2,FBC_RSSO_R1_FBC_CNT);
        if(rsso_fbc == 0){
            CAM_FUNC_ERR("RSSO have no enque record\n");
            rst = MFALSE;
        }
    }
    #endif

    return rst;
}


MBOOL PIPE_CHECK::DMAO_STATUS(void)
{
    #define str_leng    (400)
    MUINT32 dma_err[_cam_max_];
    char add[str_leng] = {"\0"};
    char str[32] = {"\0"};
    char* ptr = add;
    char const* dma_str[_cam_max_] = {"imgo","ltmso","rrzo","lcso","aao","flko","ufeo","afo","ufgo","rsso","lmvo",
        "yuvbo","tsfso","pdo","crzo","crzbo","yuvco","crzo_r2","crzbo_r2","rsso_r2","yuvo","rawi",
        "bpci","lsci","bpci_r2","pdi","ufdi_r2"};

    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_DMA_EN     dma_en;
    REG_CAMCTL_R1_CAMCTL_DMA2_EN    dma_en2;

    dma_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN);
    dma_en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA2_EN);

    //input
    if(this->m_err_status.ispInt3Err & LSC_R1_FIFO_UFLOW_EN_){
        rst = MFALSE;
        CAM_FUNC_ERR("LSCI under flow\n");
    }

    //output
    if(this->m_err_status.ispIntErr & DMA_ERR_EN_){//check cam's dma
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


        CAM_FUNC_ERR("[dmao status en1:0x%x en2:0x%x]%s\n",dma_en.Raw,dma_en2.Raw,add);

        //check fifo full
        {
            #define str_append(fmt) {\
                sprintf(str,fmt);\
                if(( str_leng - strlen(add) - 1) > strlen(str)){\
                    ptr += sprintf(ptr,"%s",str);\
                }\
                else{\
                    CAM_FUNC_ERR("string overflow\n");\
                }\
            }

            add[0] = '\0';
            if(this->m_err_status.ispInt4Err & RRZO_R1_ERR_EN_){
                str_append("rrzo-f,");
            }
            if(this->m_err_status.ispInt4Err & AFO_R1_ERR_EN_){
                str_append("afo-f,");
            }
            if(this->m_err_status.ispInt4Err & IMGO_R1_ERR_EN_){
                str_append("imgo-f,");
            }
            if(this->m_err_status.ispInt4Err & AAO_R1_ERR_EN_){
                str_append("aao-f,");
            }
            if(this->m_err_status.ispInt4Err & LCESO_R1_ERR_EN_){
                str_append("lcso-f,");
            }
            if(this->m_err_status.ispInt4Err & UFGO_R1_ERR_EN_){
                str_append("ufgo-f,");
            }
            if(this->m_err_status.ispInt4Err & UFEO_R1_ERR_EN_){
                str_append("ufeo-f,");
            }
            if(this->m_err_status.ispInt4Err & PDO_R1_ERR_EN_){
                str_append("pdo-f,");
            }
            if(this->m_err_status.ispInt4Err & FLKO_R1_ERR_EN_){
                str_append("flko-f,");
            }
            if(this->m_err_status.ispInt4Err & LMVO_R1_ERR_EN_){
                str_append("lmvo-f,");
            }
            if(this->m_err_status.ispInt4Err & RSSO_R1_ERR_EN_){
                str_append("rsso-f,");
            }
            if(this->m_err_status.ispInt4Err & LTMSO_R1_ERR_EN_){
                str_append("ltmso-f,");
            }
            if(this->m_err_status.ispInt4Err & RSSO_R2_ERR_EN_){
                str_append("rsso_r2-f,");
            }
            if(this->m_err_status.ispInt4Err & YUVO_R1_ERR_EN_){
                str_append("yuvo-f,");
            }
            if(this->m_err_status.ispInt4Err & YUVBO_R1_ERR_EN_){
                str_append("yuvbo-f,");
            }
            if(this->m_err_status.ispInt4Err & YUVCO_R1_ERR_EN_){
                str_append("yuvco-f,");
            }
            if(this->m_err_status.ispInt4Err & TSFSO_R1_ERR_EN_){
                str_append("tsfso-f,");
            }
            if(this->m_err_status.ispInt4Err & CRZO_R1_ERR_EN_){
                str_append("crzo-f,");
            }
            if(this->m_err_status.ispInt4Err & CRZBO_R1_ERR_EN_){
                str_append("crzbo-f,");
            }
            if(this->m_err_status.ispInt4Err & CRZO_R2_ERR_EN_){
                str_append("crzo_r2-f,");
            }
            if(this->m_err_status.ispInt4Err & CRZBO_R2_ERR_EN_){
                str_append("crzbo_r2-f,");
            }
            CAM_FUNC_ERR("[dmao fifo-F]%s\n",add);
        }


        //dma port debug info
        {
            MUINT32 checksum,data,linepix,linepix_r,base;
            MUINT32 smi_value,smi_base,fifo_value,fifo_base;
            MUINT32 bpci_r2dgn, cq_thread, cq_state, cq_chksum;
            MUINT32 bpci_r2dgn_base, cq_thread_base, cq_state_base, cq_chksum_base;
            //need to off db_en due to switch dbg_sel
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,0);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_SET,0xF0);

            for(MUINT32 i=0;i<_cam_max_;i++){
                switch(i){
                    case _imgo_:
                        base = 0xC0B;
                        smi_base = 0x403;
                        fifo_base = 0x20303;
                        break;
                    case _ltmso_:
                        base = 0x680B;
                        smi_base = 0x41C;
                        fifo_base = 0x2031C;
                        break;
                    case _rrzo_:
                        base = 0x100B;
                        smi_base = 0x404;
                        fifo_base = 0x20304;
                        break;
                    case _lcso_:
                        base = 0x1C0B;
                        smi_base = 0x407;
                        fifo_base = 0x20307;
                        break;
                    case _aao_:
                        base = 0x140B;
                        smi_base = 0x405;
                        fifo_base = 0x20305;
                        break;
                    case _flko_:
                        base = 0x380B;
                        smi_base = 0x410;
                        fifo_base = 0x20310;
                        break;
                    case _ufeo_:
                        base = 0x200B;
                        smi_base = 0x408;
                        fifo_base = 0x20308;
                        break;
                    case _afo_:
                        base = 0x180B;
                        smi_base = 0x406;
                        fifo_base = 0x20306;
                        break;
                    case _ufgo_:
                        base = 0x400B;
                        smi_base = 0x412;
                        fifo_base = 0x20312;
                        break;
                    case _rsso_:
                        base = 0x3C0B;
                        smi_base = 0x411;
                        fifo_base = 0x20311;
                        break;
                    case _lmvo_:
                        base = 0x340B;
                        smi_base = 0x40F;
                        fifo_base = 0x2030F;
                        break;
                    case _tsfso_:
                        base = 0x2C0B;
                        smi_base = 0x40D;
                        fifo_base = 0x2020D;
                        break;
                    case _pdo_:
                        base = 0x240B;
                        smi_base = 0x409;
                        fifo_base = 0x20309;
                        break;
                    case _crzo_:
                        base = 0x780B;
                        smi_base = 0x420;
                        fifo_base = 0x20320;
                        break;
                    case _crzbo_:
                        base = 0x7C0B;
                        smi_base = 0x421;
                        fifo_base = 0x20321;
                        break;
                    case _crzo_r2_:
                        base = 0x800B;
                        smi_base = 0x422;
                        fifo_base = 0x20322;
                        break;
                    case _crzbo_r2_:
                        base = 0x840B;
                        smi_base = 0x423;
                        fifo_base = 0x20323;
                        break;
                    case _rsso_r2_:
                        base = 0x740B;
                        smi_base = 0x41F;
                        fifo_base = 0x2031F;
                        break;
                    case _yuvo_:
                        base = 0x880B;
                        smi_base = 0x424;
                        fifo_base = 0x20324;
                        break;
                    case _yuvbo_:
                        base = 0x8C0B;
                        smi_base = 0x425;
                        fifo_base = 0x20325;
                        break;
                    case _yuvco_:
                        base = 0x900B;
                        smi_base = 0x426;
                        fifo_base = 0x20326;
                        break;
                    case _rawi_:
                        base = 0x80B;
                        smi_base = 0x102;
                        fifo_base = 0x20202;
                        break;
                    case _bpci_:
                        base = 0xB;
                        smi_base = 0x100;
                        fifo_base = 0x20200;
                        break;
                    case _lsci_:
                        base = 0x40B;
                        smi_base = 0x101;
                        fifo_base = 0x20201;
                        break;
                    case _bpci_r2_:
                        base = 0x500B;
                        smi_base = 0x116;
                        fifo_base = 0x20216;
                        break;
                    case _pdi_:
                        base = 0x300B;
                        smi_base = 0x10E;
                        fifo_base = 0x2020E;
                        break;
                    case _ufdi_r2:
                        base = 0x5C0B;
                        smi_base = 0x119;
                        fifo_base = 0x20219;
                        break;
                    default:
                        CAM_FUNC_ERR("unsupported port:%d\n",i);
                        break;
                }
                //
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(base));
                checksum = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(base + 0x100));
                linepix_r = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(base + 0x200));
                linepix = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(base + 0x300));
                data = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);

                //
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(smi_base));
                smi_value = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);

                //
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(fifo_base));
                fifo_value = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);

                CAM_FUNC_ERR("Port_%s: checksum:0x%x, linepix_r:0x%x, linepix:0x%x, data:0x%x, smi_value:0x%x, fifo_value:0x%x\n",\
                    dma_str[i],checksum,linepix_r,linepix,data,smi_value,fifo_value);
             }
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1);
             bpci_r2dgn_base = 0x316;
             CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DEBUG_SEL1,(bpci_r2dgn_base));
             bpci_r2dgn = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
             CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN,
                (CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN) & 0xEFFFFFFF));
             cq_thread_base = 0x1B;
             CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_SET,(cq_thread_base));
             cq_thread = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
             cq_state_base = 0x11B;
             CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_SET,(cq_state_base));
             cq_state = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
             CAM_FUNC_ERR("bpci_r2dgn:0x%x, cq thread:0x%x, state:0x%x\n", bpci_r2dgn, cq_thread, cq_state);
             for (MUINT32 i=0; i < 16; i++) {
                cq_chksum_base = 0x1B + (i<<8);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN,
                    (CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN) | 0x10000000));
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_SET,(cq_chksum_base));
                cq_chksum = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
                CAM_FUNC_ERR("cq chksum(%d):0x%x", i, cq_chksum);
             }
        }

        rst = MFALSE;
    }

    //check CQ
    if(CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START_ST)){
        CAM_FUNC_ERR("CQ can't read data from DRAM(0x%x)\n",CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_START_ST));
        rst = MFALSE;
    }

    return rst;
}

MBOOL PIPE_CHECK::UFE_CHECK(void)
{
    MBOOL rst = MTRUE;

    //
    if(( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_UFE_R1_EN) != \
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)) || \
        (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_UFE_R1_EN) != \
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFO_IMGO_R1_EN))){

        CAM_FUNC_ERR("ufe/ufeo/specail_ufe must be the same (%d_%d_%d)\n",\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_UFE_R1_EN),\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN),\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFO_IMGO_R1_EN));
        rst = MFALSE;
    }


    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
        REG_CAMCTL_R1_CAMCTL_SEL sel;
        MUINT32 in_w,in_h;

        sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL);

        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFO_IMGO_R1_EN) != 1){
            rst = MFALSE;
            CAM_FUNC_ERR("need to enable ufeo mode\n");
        }

        if (sel.Bits.CAMCTL_IMGO_SEL == 0 || sel.Bits.CAMCTL_IMGO_SEL == 2) {
            //imgo's xsize need to change back to normal-bayer if UFE_SEL want to be 0
            //for design convenience, just return err
            CAM_FUNC_ERR("UFE can't be enabled under pak path or pure raw path. Disable UFE/UFEO");
            rst = MFALSE;
        }

        if(CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), IMGO_R1_IMGO_CROP) != 0){
            CAM_FUNC_ERR("UFE can't be enabled under IMGO crop!= 0(0x%x)\n",CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), IMGO_R1_IMGO_CROP));
            rst = MFALSE;
        }

        switch(sel.Bits.CAMCTL_CRP_R3_SEL){
            case 0: //pure
            case 6:
                this->InPutSize_TG(in_w,in_h,MTRUE);
                break;
            case 1: //after all bin
            case 2:
            case 3:
            case 4:
            case 5:
                this->InPutSize_TG(in_w,in_h,MFALSE);
                break;
            case 7: //after bin
                this->InPutSize_TG(in_w,in_h,MTRUE);
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN)){
                    in_w = in_w>>1;
                    in_h = in_h>>1;
                }
                break;
            default:
                rst = MFALSE;
                CAM_FUNC_ERR("unsupported crop_r3_sel:%d\n",sel.Bits.CAMCTL_CRP_R3_SEL);
                break;
        }

        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), IMGO_R1_IMGO_YSIZE,IMGO_YSIZE)+1) != in_h){
            CAM_FUNC_ERR("UFE can't be enabled under IMGO vieritcal crop(0x%x_0x%x)\n",(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), IMGO_R1_IMGO_YSIZE,IMGO_YSIZE)+1),in_h);
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),IMGO_R1_IMGO_STRIDE,IMGO_STRIDE)%16) != 0){
            CAM_FUNC_ERR("stride should be 16B align\n");
            rst = MFALSE;
        }
        if((( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),IMGO_R1_IMGO_BASE_ADDR,IMGO_BASE_ADDR) + CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),IMGO_R1_IMGO_OFST_ADDR,IMGO_OFST_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),IMGO_R1_IMGO_FH_BASE_ADDR,IMGO_FH_BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_STRIDE,UFEO_STRIDE)%8) != 0){
            CAM_FUNC_ERR("stride should be 8B align\n");
            rst = MFALSE;
        }
        if(( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_XSIZE,UFEO_XSIZE)+1 )%8) != 0){
            CAM_FUNC_ERR("xsize should be 8B align\n");
            rst = MFALSE;
        }
        if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_YSIZE,UFEO_YSIZE)+1 ) != in_h){
            CAM_FUNC_ERR("ysize mismatch (%d_%d)\n",(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_YSIZE,UFEO_YSIZE)+1 ),in_h);
            rst = MFALSE;
        }
        if(((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_BASE_ADDR,UFEO_BASE_ADDR) + CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_OFST_ADDR,UFEO_OFST_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFEO_R1_UFEO_FH_BASE_ADDR,UFEO_FH_BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if (sel.Bits.CAMCTL_IMG_SEL == 1) { //pure raw path
            if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1) ) == 2){
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

    ufeg_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_UFEG_SEL);
    //
    if(( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_UFG_R1_EN) != \
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)) || \
        (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_UFG_R1_EN) != \
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFOG_RRZO_R1_EN))){

        CAM_FUNC_ERR("ufeg/ufgo/specail_ufg must be the same (%d_%d_%d)\n",\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_UFG_R1_EN),\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN),\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFOG_RRZO_R1_EN));
        rst = MFALSE;
    }


    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
        Header_RRZO rrzo_fh;
        MUINT32 size_v;
        size_v = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv->getPhyObj());
        size_v = (size_v>>16) & 0xffff;

        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,CAMDMATOP1_UFOG_RRZO_R1_EN) != 1){
            rst = MFALSE;
            CAM_FUNC_ERR("need to enable ufgo mode\n");
        }

        if(CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), RRZO_R1_RRZO_CROP) != 0){
            CAM_FUNC_ERR("UFEG can't be enabled under RRZO crop!= 0(0x%x)\n",CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), RRZO_R1_RRZO_CROP));
            rst = MFALSE;
        }

        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RRZO_R1_RRZO_YSIZE,RRZO_YSIZE)+1) != size_v){
            CAM_FUNC_ERR("UFEG can't be enabled under RRZO crop(0x%x)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RRZO_R1_RRZO_YSIZE,RRZO_YSIZE));
            rst = MFALSE;
        }

        if(ufeg_sel != 1){
            CAM_FUNC_ERR("UFEO opened but ufeg_sel is %d\n",ufeg_sel);
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RRZO_R1_RRZO_STRIDE,RRZO_STRIDE)%16) != 0){
            CAM_FUNC_ERR("stride should be 16B align\n");
            rst = MFALSE;
        }
        if(( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RRZO_R1_RRZO_BASE_ADDR,RRZO_BASE_ADDR) + CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RRZO_R1_RRZO_OFST_ADDR,RRZO_OFST_ADDR))%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RRZO_R1_RRZO_FH_BASE_ADDR,RRZO_FH_BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_STRIDE,UFGO_STRIDE)%8) != 0){
            CAM_FUNC_ERR("stride should be 8B align\n");
            rst = MFALSE;
        }
        if(( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_XSIZE,UFGO_XSIZE)+1 )%8) != 0){
            CAM_FUNC_ERR("xsize should be 8B align\n");
            rst = MFALSE;
        }
        if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_YSIZE,UFGO_YSIZE)+1 ) != size_v){
            CAM_FUNC_ERR("ysize mismatch (%d_%d)\n",(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_YSIZE,UFGO_YSIZE)+1 ),size_v);
            rst = MFALSE;
        }
        if((( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_BASE_ADDR,UFGO_BASE_ADDR) + CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_OFST_ADDR,UFGO_OFST_ADDR)) %16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),UFGO_R1_UFGO_FH_BASE_ADDR,UFGO_FH_BASE_ADDR)%16) != 0){
            CAM_FUNC_ERR("address should be 16B align\n");
            rst = MFALSE;
        }

        //
        if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1) ) == 2){
            CAM_FUNC_ERR("can't support 4-pix mode\n");
            rst = MFALSE;
        }
    }

    if(rst == MFALSE){
        CAM_FUNC_ERR("UFEG check fail:cur mux:0x%x\n",ufeg_sel);
    }
    return rst;
}



MBOOL PIPE_CHECK::AA_CHECK(void)
{
    MUINT32 aa_in_h,aa_in_v,aa_start_x,aa_start_y,_cnt=0;
    MUINT32 aa_isize_h,aa_isize_v;
    MUINT32 aa_win_h=0,aa_win_v=0;
    MUINT32 aa_win_size_h,aa_win_size_v;
    MUINT32 aa_xsize = 0,_tmp,_tmp1,_tmp2;
    MUINT32 qbn_pix=0;
    MBOOL   rst = MTRUE;
    REG_AA_R1_AA_AE_STAT_EN stt_en;
    REG_AA_R1_AA_AWB_WIN_SIZE win_size;
    REG_AA_R1_AA_AWB_WIN_PIT pit_size;
    //
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_AA_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AAO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable AAO without enable AA\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    pit_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_WIN_PIT);
    win_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_WIN_SIZE);
    stt_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_STAT_EN);
    if(stt_en.Bits.AA_AE_SE_STAT_EN != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("se must be enabled due to bitdepth is over 12bit\n");
    }

    //win number
    aa_win_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_WIN_NUM,AA_AWB_W_HNUM);
    aa_win_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_WIN_NUM,AA_AWB_W_VNUM);
    if((aa_win_h>AA_WIN_H) || (aa_win_v>AA_WIN_V)){
        CAM_FUNC_ERR("win num must be < (%d_%d),%d_%d\n",AA_WIN_H,AA_WIN_V,aa_win_h,aa_win_v);
        rst = MFALSE;
    }

    //aa start coordinate
    aa_start_x = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_WIN_ORG,AA_AWB_W_HORG);
    aa_start_y = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_WIN_ORG,AA_AWB_W_VORG);

    //aa input size , form amx output, do not just use AMX output size , it will be re-org by twin driver.
    this->InPutSize_TG(aa_in_h,aa_in_v);

    //if QBIN1 enable , hsize /2
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R1_EN)){
        //pix mode
        if((qbn_pix = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),QBIN_R1_QBIN_CTL,QBIN_ACC)))
            aa_in_h = aa_in_h >> qbn_pix;
    }
    else
        CAM_FUNC_WRN("disable QBN_R1?\n");

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
        char _str[32] = {'\0'};
        //pitch & window check
        _tmp = pit_size.Bits.AA_AWB_W_HPIT;
        snprintf(_str, sizeof(_str),"pit ");
        for(MUINT32 i=0;i<2;i++){
            //
            if(_tmp & 0x3){
                //won't crash
                CAM_FUNC_ERR("%s: size:0x%x[h-min is 4-alignment]!!,otherwise overexpcnt will be 0 (may not HW crash)\n",_str,_tmp);
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
            _tmp = win_size.Bits.AA_AWB_W_HSIZE;
            snprintf(_str, sizeof(_str),"win ");
        }
        //
        if( pit_size.Bits.AA_AWB_W_HPIT < win_size.Bits.AA_AWB_W_HSIZE){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",pit_size.Bits.AA_AWB_W_HPIT,win_size.Bits.AA_AWB_W_HSIZE);
            rst = MFALSE;
        }

        //image h_size check
        if(aa_start_x + aa_win_h * win_size.Bits.AA_AWB_W_HSIZE > aa_in_h){
            CAM_FUNC_ERR("image h_size check error , 0x%x_0x%x_0x%x_0x%x!!\n", aa_start_x, aa_win_h, win_size.Bits.AA_AWB_W_HSIZE, aa_in_h);
            rst = MFALSE;
        }
    }

    //v-win check
    if(aa_win_size_v < 2){
        CAM_FUNC_ERR("ideal v_win size:0x%x[v-min is 2] , underflow!!\n",aa_win_size_v);
        rst = MFALSE;
    }else{
        char _str[32] = {'\0'};
        //pitch & window check
        _tmp = pit_size.Bits.AA_AWB_W_VPIT;
        snprintf(_str, sizeof(_str),"pit ");
        for(MUINT32 i=0;i<2;i++){
            //
            if(_tmp & 0x1){
                //won't crash
                CAM_FUNC_ERR("%s: size:0x%x[v-min is  2-alignment]!!,otherwise overexpcnt will be 0 (may not HW crash)\n",_str,_tmp);
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
            _tmp = win_size.Bits.AA_AWB_W_VSIZE;
            snprintf(_str, sizeof(_str),"win ");
        }
        //
        if( pit_size.Bits.AA_AWB_W_VPIT < win_size.Bits.AA_AWB_W_VSIZE){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",pit_size.Bits.AA_AWB_W_VPIT,win_size.Bits.AA_AWB_W_VSIZE);
            rst = MFALSE;
        }

        //image v_size check
        if(aa_start_y + aa_win_v * win_size.Bits.AA_AWB_W_VSIZE > aa_in_v){
            CAM_FUNC_ERR("image v_size check error , 0x%x_0x%x_0x%x_0x%x!!\n", aa_start_y, aa_win_v, win_size.Bits.AA_AWB_W_VSIZE, aa_in_v);
            rst = MFALSE;
        }
    }

    //win pix number check
    if( (win_size.Bits.AA_AWB_W_HSIZE * win_size.Bits.AA_AWB_W_VSIZE) > 2047 ){
        CAM_FUNC_ERR("can't have over 2047pix in each window, data overflow\n");
        rst = MFALSE;
    }

    //pixel cnt check
    //lumi of accumulation will be affected if pix_cnt bias too much
    _tmp = (win_size.Bits.AA_AWB_W_HSIZE * win_size.Bits.AA_AWB_W_VSIZE) / 4;
    _tmp1 = ((1<<24) + (_tmp>>1)) / _tmp;
    _tmp2 = ((1<<24) + _tmp) / (_tmp<<1);

    _cnt = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_PIXEL_CNT0,AA_AWB_PIXEL_CNT0);
    if(_cnt != _tmp1){
        CAM_FUNC_ERR("awb pix cnt0 cur_tar:0x%x_0x%x, may have defect on stt data quality(may not HW crash)\n",_cnt,_tmp);
        rst = MFALSE;
    }
    _cnt = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_PIXEL_CNT1,AA_AWB_PIXEL_CNT1);
    if(_cnt != _tmp2){
        CAM_FUNC_ERR("awb pix cnt1 cur_tar:0x%x_0x%x, may have defect on stt data quality(may not HW crash)\n",_cnt,_tmp2);
        rst = MFALSE;
    }
    _cnt = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AWB_PIXEL_CNT2,AA_AWB_PIXEL_CNT2);
    if(_cnt != _tmp1){
        CAM_FUNC_ERR("awb pix cnt2 cur_tar:0x%x_0x%x, may have defect on stt data quality(may not HW crash)\n",_cnt,_tmp);
        rst = MFALSE;
    }


    aa_xsize += (aa_win_h*aa_win_v*AWB_WIN);//awb
    aa_xsize += (aa_win_h*aa_win_v*AE_WIN);// ae

    if(stt_en.Bits.AA_AE_SE_STAT_EN)
        aa_xsize += (aa_win_h*aa_win_v*AE_WIN);  //SE

    if(stt_en.Bits.AA_AE_OVERCNT_EN)
        aa_xsize += (aa_win_h*aa_win_v*AE_OE_WIN); // 1 byte each win

    //ae hist
    {
        MUINT32 BinNum[AE_HIST_TYPE][AE_HIST_NUM] = {0};
        REG_AA_R1_AA_AE_PIX_HST_CTL hst_ctrl;
        REG_AA_R1_AA_AE_PIX_HST_SET hst;
        REG_AA_R1_AA_AE_PIX_HST_SET_1 hst1;
        REG_AA_R1_AA_AE_PIX_SE_HST_SET se_hst;
        REG_AA_R1_AA_AE_PIX_SE_HST_SET_1 se_hst1;
        #define ROI_CHK(type,x,validy,validx) {\
            if(type == 0){\
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_HST##x##_YRNG,AA_AE_PIX_Y_HI_##x) \
                    > (aa_in_v/validy)){\
                    CAM_FUNC_ERR("ROI of hst_%d:Y over aainput size_%d\n",_cnt,(aa_in_v/validy));\
                    rst = MFALSE;\
                }\
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_HST##x##_XRNG,AA_AE_PIX_X_HI_##x) \
                    > (aa_in_h/validx)){\
                    CAM_FUNC_ERR("ROI of hst_%d:X over aainput size_%d\n",_cnt,(aa_in_h/validx));\
                    rst = MFALSE;\
                }\
            }\
            else{\
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_SE_HST##x##_YRNG,AA_AE_PIX_SE_Y_HI_##x) \
                    > (aa_in_v/validy)){\
                    CAM_FUNC_ERR("ROI of hst_%d:Y over aainput size_%d\n",_cnt,(aa_in_v/validy));\
                    rst = MFALSE;\
                }\
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_SE_HST##x##_XRNG,AA_AE_PIX_SE_X_HI_##x) \
                    > (aa_in_h/validx)){\
                    CAM_FUNC_ERR("ROI of hst_%d:X over aainput size_%d\n",_cnt,(aa_in_h/validx));\
                    rst = MFALSE;\
                }\
            }\
        }

        //HW issue.
        //Bin_mode_0 donimate all other bins' bin_number.
        //if Bin_mode_0 = 1, bin_mode_1 = 0 => bin_mode_1 will be also 256 bin number , but content is 128 bin's result. (content will have error)

        hst.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_HST_SET);
        hst1.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_HST_SET_1);
        se_hst.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_SE_HST_SET);
        se_hst1.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_SE_HST_SET_1);

        BinNum[0][0] = hst.Bits.AA_AE_PIX_BIN_MODE_0;
        BinNum[1][0] = se_hst.Bits.AA_AE_PIX_SE_BIN_MODE_0;

        for(MUINT32 i=1;i<AE_HIST_NUM;i++){
            BinNum[0][i] = BinNum[0][0];
            BinNum[1][i] = BinNum[1][0];
        }

        if( (BinNum[0][1] != hst.Bits.AA_AE_PIX_BIN_MODE_1) || \
            (BinNum[0][2] != hst.Bits.AA_AE_PIX_BIN_MODE_2) || \
            (BinNum[0][3] != hst.Bits.AA_AE_PIX_BIN_MODE_3) || \
            (BinNum[0][4] != hst1.Bits.AA_AE_PIX_BIN_MODE_4) || \
            (BinNum[0][5] != hst1.Bits.AA_AE_PIX_BIN_MODE_5))
        {
            rst = MFALSE;
            CAM_FUNC_ERR("bin_mode err,IQ issue\n");
        }
        if( (BinNum[1][1] != se_hst.Bits.AA_AE_PIX_SE_BIN_MODE_1) || \
            (BinNum[1][2] != se_hst.Bits.AA_AE_PIX_SE_BIN_MODE_2) || \
            (BinNum[1][3] != se_hst.Bits.AA_AE_PIX_SE_BIN_MODE_3) || \
            (BinNum[1][4] != se_hst1.Bits.AA_AE_PIX_SE_BIN_MODE_4) || \
            (BinNum[1][5] != se_hst1.Bits.AA_AE_PIX_SE_BIN_MODE_5))
        {
            rst = MFALSE;
            CAM_FUNC_ERR("se bin_mode err,IQ issue\n");
        }

        hst_ctrl.Raw = _tmp = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_PIX_HST_CTL);
        if( (hst_ctrl.Bits.AA_AE_PIX_HST0_EN != hst_ctrl.Bits.AA_AE_PIX_SE_HST0_EN) || \
            (hst_ctrl.Bits.AA_AE_PIX_HST1_EN != hst_ctrl.Bits.AA_AE_PIX_SE_HST1_EN) || \
            (hst_ctrl.Bits.AA_AE_PIX_HST2_EN != hst_ctrl.Bits.AA_AE_PIX_SE_HST2_EN) || \
            (hst_ctrl.Bits.AA_AE_PIX_HST3_EN != hst_ctrl.Bits.AA_AE_PIX_SE_HST3_EN) || \
            (hst_ctrl.Bits.AA_AE_PIX_HST4_EN != hst_ctrl.Bits.AA_AE_PIX_SE_HST4_EN) || \
            (hst_ctrl.Bits.AA_AE_PIX_HST5_EN != hst_ctrl.Bits.AA_AE_PIX_SE_HST5_EN)){
            rst = MFALSE;
            CAM_FUNC_ERR("SE hst must be the same as hst due to over 12-bitdepth,IQ issue\n");
        }
        for(_tmp2=0;_tmp2<AE_HIST_TYPE;_tmp2++){
            for(_cnt=0;_cnt<AE_HIST_NUM;_cnt++){
                if(_tmp & 0x1){
                    if(stt_en.Bits.AA_AE_HST_DMA_OUT_EN){
                        if(BinNum[_tmp2][_cnt] == 0)
                            aa_xsize += (AE_HIST_BINS*(AE_HIST_BIN/2));    // 128 bins
                        else
                            aa_xsize += (AE_HIST_BINS*AE_HIST_BIN);
                    }
                    //ROI chk
                    if(_cnt == 0){
                        ROI_CHK(_tmp2,0,2,4);
                    }
                    else if(_cnt == 1){
                        ROI_CHK(_tmp2,1,2,4);
                    }
                    else if(_cnt == 2){
                        ROI_CHK(_tmp2,2,2,4);
                    }
                    else if(_cnt == 3){
                        ROI_CHK(_tmp2,3,2,4);
                    }
                    else if(_cnt == 4){
                        ROI_CHK(_tmp2,4,2,4);
                    }
                    else if(_cnt == 5){
                        ROI_CHK(_tmp2,5,2,4);
                    }
                }
                _tmp=_tmp >> 1;
            }
            _tmp = _tmp >> 2;
        }
    }

    //
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AA_R1_AA_AE_CCU_HST_END_Y,AA_AE_CCU_HST_END_Y) > aa_win_v){
        rst = MFALSE;
        CAM_FUNC_ERR("CCU's end-Y should be <= %d\n",aa_win_v);
    }

    //
    aa_xsize = aa_xsize /8; //bits to byte
    if(aa_xsize != (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AAO_R1_AAO_XSIZE,AAO_XSIZE)+1)){
        CAM_FUNC_ERR("cur xsize :0x%x, valid xsize:0x%x\n",\
            (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AAO_R1_AAO_XSIZE,AAO_XSIZE)+1),\
            aa_xsize);
        rst = MFALSE;
    }

    if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AAO_R1_AAO_YSIZE,AAO_YSIZE) != 0){
        //aao is 1-d
        CAM_FUNC_ERR("current ysize:0x%x, user can't set 0 into drv\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AAO_R1_AAO_YSIZE,AAO_YSIZE));
        rst = MFALSE;
    }


EXIT:
    if(rst == MFALSE){
        CAM_FUNC_WRN("AE/AWB/AAO check error,cur window size:%d x %d\n",aa_win_h,aa_win_v);
    }
    return rst;
}

MBOOL PIPE_CHECK::TSF_CHECK(void)
{
    MBOOL   rst = MTRUE;
    MUINT32 in_h,in_v,_cnt=0;
    MUINT32 aa_isize_h,aa_isize_v;
    MUINT32 aa_win_size_h,aa_win_size_v;
    MUINT32 aa_xsize = 0,_tmp;

    REG_TSFS_R1_TSFS_NUM    win_num;
    REG_TSFS_R1_TSFS_ORG    start_pix;
    REG_TSFS_R1_TSFS_SIZE   win_size;
    REG_TSFS_R1_TSFS_PIT    pit_size;
    win_num.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_NUM);
    start_pix.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_ORG);
    win_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_SIZE);
    pit_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_PIT);
    //
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_TSFS_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_TSFSO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable TSFSO without enable TSFS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    //aa win number chk
    if( (win_num.Bits.TSFS_W_HNUM < 16) || (win_num.Bits.TSFS_W_HNUM > 128)){
        rst = MFALSE;
        CAM_FUNC_ERR("win_h number must be 16 ~ 128\n");
    }
    if( (win_num.Bits.TSFS_W_VNUM < 16) || (win_num.Bits.TSFS_W_VNUM > 128)){
        rst = MFALSE;
        CAM_FUNC_ERR("win_v number must be 16 ~ 128\n");
    }
    //aa input size , from bmx output, but do not read frm bmx directly, will be re-set by twin drv
    this->InPutSize_TG(in_h,in_v);

    //
    if (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R2_EN)) {
        //pix mode
        in_h = in_h >> CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),QBIN_R2_QBIN_CTL,QBIN_ACC);
    }
    else{
        CAM_FUNC_WRN("disable qbn_r2 ?\n");
    }

    // statistic operation input size
    aa_isize_h = in_h - start_pix.Bits.TSFS_W_HORG;
    aa_isize_v = in_v - start_pix.Bits.TSFS_W_VORG;
    // win/pit size, win size <= pit size
    aa_win_size_h = (aa_isize_h / win_num.Bits.TSFS_W_HNUM) /2 * 2;
    aa_win_size_v = (aa_isize_v / win_num.Bits.TSFS_W_VNUM) /2 * 2;


    //win pix number check
    if( (win_size.Bits.TSFS_W_HSIZE * win_size.Bits.TSFS_W_VSIZE) > 102920 ){
        CAM_FUNC_ERR("can't have over 102920pix in each window, data overflow\n");
        rst = MFALSE;
    }

    //h-win check
    if(aa_win_size_h < 4){
        CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",aa_win_size_h);
        rst = MFALSE;
    }
    else{
        char _str[32]={'\0'};

        //pitch & window check
        _tmp = pit_size.Bits.TSFS_W_HPIT;
        snprintf(_str, sizeof(_str),"pit ");
        for(MUINT32 i=0;i<2;i++){
            if((i==1) && (_tmp & 0x1) ){
                //win size must be even
                CAM_FUNC_ERR("%s: size:0x%x[h-min is 4 & 2-alignment]!!\n",_str,_tmp);
                rst = MFALSE;
            }

            if(_tmp < 4){
                CAM_FUNC_ERR("%s: size:0x%x[h-min is 4]!!\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp > aa_win_size_h){

                CAM_FUNC_ERR("%s size need <= ideal size, [0x%x_0x%x]\n",_str,_tmp,aa_win_size_h);
                rst = MFALSE;
            }
            //
            _tmp = win_size.Bits.TSFS_W_HSIZE;
            snprintf(_str, sizeof(_str),"win ");
        }
        //
        if( pit_size.Bits.TSFS_W_HPIT < win_size.Bits.TSFS_W_HSIZE){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",\
                pit_size.Bits.TSFS_W_HPIT,win_size.Bits.TSFS_W_HSIZE);
            rst = MFALSE;
        }
    }


    //v-win check
    if(aa_win_size_v < 2){
        CAM_FUNC_ERR("ideal v_win size:0x%x[v-min is 2] , underflow!!\n",aa_win_size_v);
        rst = MFALSE;
    }else{
        char _str[32] = {'\0'};

        //pitch & window check
        _tmp = pit_size.Bits.TSFS_W_VPIT;
        snprintf(_str, sizeof(_str),"pit ");
        for(MUINT32 i=0;i<2;i++){
            if((i==1) && (_tmp & 0x1) ){
                //win size must be even
                CAM_FUNC_ERR("%s: size:0x%x[v-min is 2 & 2-alignment]!!\n",_str,_tmp);
                rst = MFALSE;
            }
            //
            if(_tmp < 2){
                CAM_FUNC_ERR("%s: size:0x%x[v-min is 2]!!\n",_str,_tmp);
                rst = MFALSE;
            }
            else if(_tmp > aa_win_size_v){

                CAM_FUNC_ERR("%s size need <= ideal size, [0x%x_0x%x]\n",_str,_tmp,aa_win_size_v);
                rst = MFALSE;
            }

            //
            _tmp = win_size.Bits.TSFS_W_VSIZE;
            snprintf(_str, sizeof(_str),"win ");
        }
        //
        if( pit_size.Bits.TSFS_W_VPIT < win_size.Bits.TSFS_W_VSIZE){
            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",\
                pit_size.Bits.TSFS_W_VPIT,win_size.Bits.TSFS_W_VSIZE);
            rst = MFALSE;
        }

    }

    //pixel cnt check
    //lumi of accumulation will be affected if pix_cnt bias too much
    {
        MUINT32 div = 4;

        _tmp = (win_size.Bits.TSFS_W_HSIZE * win_size.Bits.TSFS_W_VSIZE / div);
        _tmp = ((1<<24) + (_tmp>>1))/ _tmp;

        _cnt = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_PC0,TSFS_PIXEL_CNT0);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("tsfs pix cnt0 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }

        _cnt = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_PC2,TSFS_PIXEL_CNT2);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("tsfs pix cnt2 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }

        _tmp = _tmp>>1;
        _cnt = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFS_R1_TSFS_PC1,TSFS_PIXEL_CNT1);
        if(_cnt != _tmp){
            CAM_FUNC_ERR("tsfs pix cnt1 cur_tar:0x%x_0x%x,(quality loss,may not HW crash)\n",_cnt,_tmp);
            rst = MFALSE;
        }
    }



    //dram size
    aa_xsize = (win_num.Bits.TSFS_W_HNUM*12);  //12 byte per window,R/G/B 4B each


    if(aa_xsize != (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFSO_R1_TSFSO_XSIZE,TSFSO_XSIZE)+1)){
        CAM_FUNC_ERR("cur xsize :0x%x, valid xsize:0x%x\n",\
            (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFSO_R1_TSFSO_XSIZE,TSFSO_XSIZE)+1),\
            aa_xsize);
        rst = MFALSE;
    }

    if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFSO_R1_TSFSO_YSIZE,TSFSO_YSIZE)+1) != \
        win_num.Bits.TSFS_W_VNUM){
        CAM_FUNC_ERR("current ysize:0x%x, valid size:0x%x\n",\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TSFSO_R1_TSFSO_YSIZE,TSFSO_YSIZE),win_num.Bits.TSFS_W_VNUM);
        rst = MFALSE;
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("tsfs check error\n!!\n");
    }

    return rst;
}


MBOOL PIPE_CHECK::FLK_CHECK(void)
{
    #define flk_bype_perpix (6)
    MBOOL rst = MTRUE;
    MUINT32 size_h,size_v,in_size_h,in_size_v;
    MUINT32 x_size;
    MUINT32 flk_sel;
    MUINT32 win_num_h,win_num_v;
    MUINT32 ofst_x,ofst_y;
    MUINT32 qbn_pix=0;
    MUINT32 tmp;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_FLK_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_FLKO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable FLKO without enable FLK\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    else{
        capibility CamInfo(this->m_hwModule);
        tCAM_rst ret;

        CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_FUNC,\
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),ret);
        if(ret.bSupportedModule.bFlk == MFALSE){
            CAM_FUNC_ERR("current cam_%d have no flk\n",this->m_hwModule);
            rst = MFALSE;
            goto EXIT;
        }
    }

    flk_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_FLK_SEL);

    switch (flk_sel ) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            this->InPutSize_TG(in_size_h,in_size_v);
            if ((qbn_pix = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),QBIN_R3_QBIN_CTL,QBIN_ACC))!= 0) {
                in_size_h = in_size_h >> qbn_pix;
            }
            break;
        case 5:
            this->InPutSize_TG(in_size_h,in_size_v,MTRUE);
            in_size_h = in_size_h >> 1; //YUV
            rst = MFALSE;
            CAM_FUNC_ERR("can't support YUV format with FLK due to TG issue\n");
            break;
        default:
            CAM_FUNC_ERR("flk_sel:0x%x err\n",flk_sel);
            rst = MFALSE;
            goto EXIT;
            break;
    }

    //start
    ofst_x = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLK_R1_FLK_OFST,FLK_OFST_X);
    ofst_y = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLK_R1_FLK_OFST,FLK_OFST_Y);
    //win num
    win_num_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLK_R1_FLK_NUM,FLK_NUM_X);
    win_num_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLK_R1_FLK_NUM,FLK_NUM_Y);
    //win size
    size_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLK_R1_FLK_SIZE,FLK_SIZE_X);
    size_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLK_R1_FLK_SIZE,FLK_SIZE_Y);
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
    x_size = (win_num_h*win_num_v*size_v * flk_bype_perpix);
    if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_XSIZE,FLKO_XSIZE) + 1) != x_size){
        rst = MFALSE;
        CAM_FUNC_ERR("xsize mismatch[0x%x_0x%x]\n",(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_XSIZE,FLKO_XSIZE) + 1),\
            x_size);
    }
    //ysize
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_YSIZE,FLKO_YSIZE) != 0){
        rst = MFALSE;
        CAM_FUNC_ERR("Ysize need  to be 0[0x%x]\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_YSIZE,FLKO_YSIZE));
    }
#if 0   //hw : no need to asign stride due to FLKO is 1-D dma. and 16bit for stride.(18bit for xsize)
    //stride
    if((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_XSIZE,FLKO_XSIZE) + 1) > CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_STRIDE,FLKO_STRIDE)){
        rst = MFALSE;
        CAM_FUNC_ERR("stride > xsize+1[0x%x_0x%x]\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_STRIDE,FLKO_STRIDE),\
            (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),FLKO_R1_FLKO_XSIZE,FLKO_XSIZE) + 1));
    }
#endif
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

    lcs_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_LCES_SEL);
    //
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_LCES_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LCESO_R1_EN) == 1){

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
        pixMode = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),QBIN_R1_QBIN_CTL, QBIN_ACC);
        break;
    case 0:
        this->InPutSize_TG(in_h,in_v,MTRUE);
        pixMode = (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
                        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1)) + 1;
        break;
    default:
        CAM_FUNC_ERR("lcs_sel error(%d)\n",lcs_sel);
        rst = MFALSE;
        goto EXIT;
        break;
    }
    in_h = in_h >> pixMode;


    win_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_OUT_SIZE, LCES_OUT_WD );
    win_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_OUT_SIZE, LCES_OUT_HT );

    if(( win_h & 0x1 ) || (win_v & 0x1)){
        CAM_FUNC_ERR("lcs output h/v should be even[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    if((win_h < 8) || (win_v<8) || (win_h>510) || (win_v>510)){
        CAM_FUNC_ERR("lcs output size : h:8~510, v:8~510[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    crop_x = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_START, LCES_START_J );
    crop_y = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_START, LCES_START_I );
    crop_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_IN_SIZE, LCES_IN_WD );
    crop_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_IN_SIZE, LCES_IN_HT );

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

    scaling_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_LRZRX, LCES_LRZR_X );
    scaling_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_LRZRY, LCES_LRZR_Y );
    if(scaling_h != ((win_h - 1)* 1048576 / (crop_h>>1) )){
        CAM_FUNC_ERR("lcs h-scaling factor error [0x%x_0x%x_0x%x]\n",scaling_h,win_h,crop_h);
        rst = MFALSE;
    }
    if(scaling_v != ((win_v - 1)* 1048576 / (crop_v>>1) )){
        CAM_FUNC_ERR("lcs v-scaling factor error [0x%x_0x%x_0x%x]\n",scaling_v,win_v,crop_v);
        rst = MFALSE;
    }

    if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCESO_R1_LCESO_XSIZE, LCESO_XSIZE ) != (win_h*2 -1) ){
        CAM_FUNC_ERR("LCS xsize error:0x%x_0x%x\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCESO_R1_LCESO_XSIZE, LCESO_XSIZE ),(win_h*2 -1));
        rst = MFALSE;
    }
    if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCESO_R1_LCESO_YSIZE, LCESO_YSIZE ) != (win_v -1) ){
        CAM_FUNC_ERR("LCS ysize error:0x%x_0x%x\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCESO_R1_LCESO_YSIZE, LCESO_YSIZE ),(win_v -1));
        rst = MFALSE;
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), LCES_R1_LCES_AH, LCES_HIST_OUT_EN ) == 1){
        CAM_FUNC_ERR("hist is not supported(no hw)\n");
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


    hds_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_HDS_SEL);

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_LMV_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LMVO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable LMVO without enable LMV\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    else{
        capibility CamInfo(this->m_hwModule);
        tCAM_rst ret;

        CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_FUNC,\
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),ret);
        if(ret.bSupportedModule.bLMV == MFALSE){
            CAM_FUNC_ERR("current cam_%d have no lmv\n",this->m_hwModule);
            rst = MFALSE;
            goto EXIT;
        }
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_GSE_R1_EN) == 0){
        CAM_FUNC_ERR("GSE must be enabled\n");
        rst = MFALSE;
    }
    //max window number
    win_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_NUM_HWIN);
    win_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_NUM_VWIN);
    if( (win_h > 4) || (win_v > 8)){
        CAM_FUNC_ERR("eis win size max:4*8[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    //ofst,ofst have min constraint
    ofst_x = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_MB_OFFSET,LMV_RP_HOFST);
    ofst_y = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_MB_OFFSET,LMV_RP_VOFST);
    if((ofst_x < 17) || (ofst_y < 17)){
        CAM_FUNC_ERR("eis ofset must > 16[0x%x_0x%x]\n",ofst_x,ofst_y);
    }
    //floating ofset, current frame ofst from previous frame, normally set 0.
    //this->m_pIspDrv->writeReg(0x355c, ((FL_ofst_x<<16)|FL_ofst_y));

    switch(hds_sel){
        case 0:
            this->InPutSize_TG(in_size_h,in_size_v,MTRUE);

            in_size_h = in_size_h>>1;// YUV fmt
            rst = MFALSE;
            CAM_FUNC_ERR("can't support YUV format with LMV due to TG issue\n");
            break;
        case 1:
            tmp = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv->getPhyObj());

            in_size_h = tmp & 0xffff;
            in_size_v = (tmp>>16) & 0xffff;

            in_size_h = in_size_h>>CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),HDS_R1_HDS_HDS_MODE,HDS_DS_MODE);
            break;
        default:
            in_size_h = in_size_v = 0;
            CAM_FUNC_ERR("hdr_sel error(%d)\n",hds_sel);
            rst = MFALSE;
            goto EXIT;
            break;
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_WIDTH) != in_size_h){
        CAM_FUNC_ERR("eis input-h err[0x%x_0x%x]\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_WIDTH),in_size_h);
        rst = MFALSE;
    }
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_HEIGHT) != in_size_v){
        CAM_FUNC_ERR("eis input-v err[0x%x_0x%x]\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_HEIGHT),in_size_v);
        rst = MFALSE;
    }

    //EIS step size
    step_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_NUM_HRP);
    step_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_NUM_VRP);
    if((step_h > 16) || (step_v > 8)){
        CAM_FUNC_ERR("eis step size max:16*8[0x%x_0x%x]\n",step_h,step_v);
        rst = MFALSE;
    }

    //EIS window size
    win_size_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_MB_INTERVAL,LMV_WIN_HSIZE);
    win_size_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_MB_INTERVAL,LMV_WIN_VSIZE);
    if(win_size_h < ((step_h + 1) * 16 + 2)){
        CAM_FUNC_ERR("eis h win size too small[0x%x_0x%x]\n",win_size_h,((step_h + 1) * 16 + 2));
        rst = MFALSE;
    }
    if(win_size_v < ((step_v + 1) * 16 + 2)){
        CAM_FUNC_ERR("eis v win size too small[0x%x_0x%x]\n",win_size_v,((step_v + 1) * 16 + 2));
        rst = MFALSE;
    }

    //input check
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_WIDTH) != in_size_h){
        CAM_FUNC_ERR("input width err:%d_%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_WIDTH),in_size_h);
        in_size_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_WIDTH);
        rst = MFALSE;
    }
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_HEIGHT) != in_size_v){
        CAM_FUNC_ERR("input height err:%d_%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_HEIGHT),in_size_v);
        in_size_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,LMV_HEIGHT);
        rst = MFALSE;
    }
    op_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_OP_HORI);
    op_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_OP_VERT);
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
    if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_PREP_ME_CTRL1,LMV_SUBG_EN) == 1){
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
    MUINT32 in_size_h,in_size_v;
    MUINT32 pixMode = 0,hds_sel;
    Header_RRZO rrzo_fh;
    MUINT32 tmp=0;
    MUINT32 win_out_w=0,win_out_h=0, win_in_w=0, win_in_h=0;
    MBOOL rst = MTRUE;
    MUINT32 CoeffStep_H,CoeffStep_V;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_RSS_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RSSO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable RSSO without enable RSS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    else{
        capibility CamInfo(this->m_hwModule);
        tCAM_rst ret;

        CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_FUNC,\
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),ret);
        if(ret.bSupportedModule.bRSS == MFALSE){
            CAM_FUNC_ERR("current cam_%d have no rss\n",this->m_hwModule);
            rst = MFALSE;
            goto EXIT;
        }
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_GSE_R1_EN) == 0){
        CAM_FUNC_ERR("GSE must be enabled\n");
        rst = MFALSE;
    }

    hds_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_HDS_SEL);
    switch(hds_sel){
        case 0:
            this->InPutSize_TG(in_size_h,in_size_v,MTRUE);

            in_size_h = in_size_h>>1;//for YUV fmt
            rst = MFALSE;
            CAM_FUNC_ERR("can't support YUV format with rss due to TG issue\n");
            break;
        case 1:
            tmp = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv->getPhyObj());

            in_size_h = tmp & 0xffff;
            in_size_v = (tmp>>16) & 0xffff;

            in_size_h = in_size_h>>CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),HDS_R1_HDS_HDS_MODE,HDS_DS_MODE);
            break;
        default:
            in_size_h = in_size_v = 0;
            CAM_FUNC_ERR("hdr_sel error(%d)\n",hds_sel);
            rst = MFALSE;
            goto EXIT;
            break;
    }

    win_in_w = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RSS_R1_RSS_IN_IMG, RSS_IN_WD);
    win_in_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RSS_R1_RSS_IN_IMG, RSS_IN_HT);
    if((win_in_w != in_size_h) || (win_in_h != in_size_v)){
        CAM_FUNC_ERR("input size mismatch:%d_%d(%d_%d,%d)\n",win_in_w,win_in_h,in_size_h,in_size_v,hds_sel);
        rst = MFALSE;
    }

    //out range is limited
    win_out_w = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RSS_R1_RSS_OUT_IMG, RSS_OUT_WD);
    win_out_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(), RSS_R1_RSS_OUT_IMG, RSS_OUT_HT);

    if( (win_out_w > 2) && (win_out_h > 2) ){

        REG_RSS_R1_RSS_HORI_STEP hori_step;
        REG_RSS_R1_RSS_VERT_STEP vert_step;
        CoeffStep_H = (MUINT32)(((win_out_w-1)*1048576 + (win_in_w-1) - 1) / (win_in_w - 1));
        CoeffStep_V = (MUINT32)(((win_out_h-1)*1048576 + (win_in_h-1) - 1) / (win_in_h - 1));
        hori_step.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), RSS_R1_RSS_HORI_STEP);
        vert_step.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(), RSS_R1_RSS_VERT_STEP);

        if(hori_step.Bits.RSS_HORI_STEP != CoeffStep_H ){
            rst = MFALSE;
            CAM_FUNC_ERR("hor step r different(%d_%d)\n",hori_step.Bits.RSS_HORI_STEP,CoeffStep_H);
        }
        if(vert_step.Bits.RSS_VERT_STEP != CoeffStep_V){
            rst = MFALSE;
            CAM_FUNC_ERR("ver step r different(%d_%d)\n",vert_step.Bits.RSS_VERT_STEP,CoeffStep_V);
        }
    }
    else{
        rst = MFALSE;
        CAM_FUNC_ERR("out size err(%d_%d)\n",win_out_w,win_out_h);
    }

    if((win_out_h < 22) || (win_out_h > 511) ){
        rst = MFALSE;
        CAM_FUNC_ERR("vertical size must be within 22~511\n");
    }

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
    if(( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_LSC_R1_EN) ^ \
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LSCI_R1_EN) ) == 1){
        CAM_FUNC_ERR("LSC/LSCI must be enable/disable togather(%d_%d)\n",\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_LSC_R1_EN),\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LSCI_R1_EN));
        rst = MFALSE;
        goto EXIT;
    }
    else if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_LSC_R1_EN) == 0){
        goto EXIT;
    }


    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN) == MTRUE){
        //twin drv will have adjusted configuration on LSC due to overlap processing
        CAM_FUNC_WRN("bypass lsc check\n");
        goto EXIT;
    }

    //
    this->InPutSize_SEP(in_size_x,in_size_y);

    nWin_h = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_CTL2,LSC_SDBLK_XNUM);
    nWin_v = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_CTL3,LSC_SDBLK_YNUM);
    win_size_x = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_CTL2,LSC_SDBLK_WIDTH);
    win_size_y = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_CTL3,LSC_SDBLK_HEIGHT);
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
    win_lsize_x = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_LBLOCK,LSC_SDBLK_lWIDTH);
    win_lsize_y = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_LBLOCK,LSC_SDBLK_lHEIGHT);
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

    xsize = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSCI_R1_LSCI_XSIZE,LSCI_XSIZE);
    extend = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSC_R1_LSC_CTL1,LSC_EXTEND_COEF_MODE);
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
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSCI_R1_LSCI_YSIZE,LSCI_YSIZE) != ((nWin_v+1) - 1) ){
        CAM_FUNC_ERR("lsci ysize error[0x%x_0x%x]\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LSCI_R1_LSCI_YSIZE,LSCI_YSIZE) + 1,(nWin_v+1));
        rst = MFALSE;
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("LSC check fail\n");
    }

    return rst;
}


MBOOL PIPE_CHECK::AF_CHECK(void)
{
    #define AF_MAX_IN     (5504)
    #define AF_MAX_CROP   (4096)
    MBOOL rst = MTRUE;
    MUINT32 tmp=0;
    MUINT32 af_sel;
    MUINT32 h_size;
    MUINT32 v_size;

    REG_AF_R1_AF_BLK_PROT blk_2;
    REG_AF_R1_AF_BLK_1 blk_1;
    REG_AF_R1_AF_BLK_0 blk_0;
    MUINT32 xsize,ysize;
    REG_AF_R1_AF_CON af_con;
    REG_AF_R1_AF_CON2 af_con2;
    REG_AF_R1_AF_VLD af_vld;
    MUINT32 afo_xsize, afo_ysize;


    af_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_AF_SEL);

    blk_0.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_BLK_0);
    blk_1.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_BLK_1);

    blk_2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_BLK_PROT);

    //
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_AF_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable AFO without enable AF\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    //
    af_con.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_CON);
    af_con2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_CON2);

    //AFO and AF relaterd module enable check
    tmp  = (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN)? 0:1);
    tmp += (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R1_EN)? 0:1);
    if(tmp){
        CAM_FUNC_ERR("AF is enabled, MUST enable AFO/AFO_FBC/CRP:0x%x_0x%x_0x%x\n",
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN),
            CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1),
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R1_EN));
        rst = MFALSE;
    }

    if((af_con.Bits.AF_H_GONLY & af_con2.Bits.AF_DS_EN) != 0){
        CAM_FUNC_ERR("h_gonly & ds_en r mutually exclusive function\n");
        rst = MFALSE;
    }

    //AF image wd
    switch(af_sel){
        case 0:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN) == MTRUE){
                CAM_FUNC_WRN("af_sel = 0 under twin mode,is IQ ok?\n");
            }
            this->InPutSize_TG(h_size,v_size,MTRUE);
            h_size = h_size>>((CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
                        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
            break;
        case 1:
            {
                REG_CRP_R1_CRP_X_POS rcp_w;
                REG_CRP_R1_CRP_Y_POS rcp_h;
                rcp_w.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRP_R1_CRP_X_POS);
                rcp_h.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRP_R1_CRP_Y_POS);

                h_size = rcp_w.Bits.CRP_XEND - rcp_w.Bits.CRP_XSTART + 1;
                v_size = rcp_h.Bits.CRP_YEND - rcp_h.Bits.CRP_YSTART + 1;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported af_sel:0x%x\n",af_sel);
            return MFALSE;
            break;
    }
    if(h_size != CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_SIZE,AF_IMAGE_WD)){
        CAM_FUNC_ERR("AF input size mismatch:0x%x_0x%x\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_SIZE,AF_IMAGE_WD),h_size);
        rst = MFALSE;
    }
    else{
        if(h_size > AF_MAX_IN){
            CAM_FUNC_ERR("AF input width should be <= %d\n",AF_MAX_IN);
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
    af_vld.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_VLD);
    if((af_vld.Bits.AF_VLD_XSTART&0x1)||(af_vld.Bits.AF_VLD_YSTART&0x1)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF vld start must be even:0x%x_0x%x\n",af_vld.Bits.AF_VLD_XSTART,af_vld.Bits.AF_VLD_YSTART);
    }
    else{
        if((af_vld.Bits.AF_VLD_XSTART + blk_0.Bits.AF_BLK_XSIZE*blk_1.Bits.AF_BLK_XNUM) > AF_MAX_IN){
            rst = MFALSE;
            CAM_FUNC_ERR("af h window out of range:0x%x_0x%x_0x%x_0x%x\n",af_vld.Bits.AF_VLD_XSTART,blk_0.Bits.AF_BLK_XSIZE, blk_1.Bits.AF_BLK_XNUM,AF_MAX_IN);
        }
        if((blk_0.Bits.AF_BLK_XSIZE*blk_1.Bits.AF_BLK_XNUM) > AF_MAX_CROP){
            CAM_FUNC_ERR("af h window out of range:0x%x_0x%x_0x%x\n",blk_0.Bits.AF_BLK_XSIZE, blk_1.Bits.AF_BLK_XNUM,AF_MAX_CROP);
        }
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
            tmp = 0x1F;
        }
        else if((af_con.Bits.AF_V_AVG_LVL == 3) && (af_con.Bits.AF_V_GONLY  == 0)){
            tmp= 0xF;
        }
        else if((af_con.Bits.AF_V_AVG_LVL == 2) && (af_con.Bits.AF_V_GONLY  == 1)){
            tmp= 0xF;
        }
        else{
            tmp= 0x7;
            if(af_con.Bits.AF_V_AVG_LVL < 2){
                CAM_FUNC_ERR("AF_V_AVG_LVL can only be 2 or 3\n");
                rst = MFALSE;
            }
        }

        if(blk_0.Bits.AF_BLK_XSIZE<8){
            CAM_FUNC_ERR("af min h win size::8, cur:0x%x   [0x%x_0x%x]\n",blk_0.Bits.AF_BLK_XSIZE,af_con.Bits.AF_V_AVG_LVL,af_con.Bits.AF_V_GONLY);
            rst = MFALSE;
        }
        else if(blk_0.Bits.AF_BLK_XSIZE < blk_2.Bits.AF_PROT_BLK_XSIZE){
            CAM_FUNC_ERR("x blk size can't be < protected size(%d_%d)\n",blk_0.Bits.AF_BLK_XSIZE,blk_2.Bits.AF_PROT_BLK_XSIZE);
            rst = MFALSE;
        }
    }

    if(blk_0.Bits.AF_BLK_XSIZE & tmp){
        CAM_FUNC_ERR("af blk x-size:%d should have %d-alignment. [0x%x_0x%x]\n",blk_0.Bits.AF_BLK_XSIZE,(tmp+1),af_con.Bits.AF_V_AVG_LVL,af_con.Bits.AF_V_GONLY);
        rst = MFALSE;
    }


    if(blk_0.Bits.AF_BLK_YSIZE > 128){
        rst = MFALSE;
        CAM_FUNC_ERR("af max v win size:128 cur:0x%x\n",blk_0.Bits.AF_BLK_YSIZE);
    }
    else{//min constraint
        if(blk_0.Bits.AF_BLK_YSIZE<1){
            CAM_FUNC_ERR("af min v win size:1, cur:0x%x\n",blk_0.Bits.AF_BLK_YSIZE);
            rst = MFALSE;
        }
        else if(blk_0.Bits.AF_BLK_YSIZE < blk_2.Bits.AF_PROT_BLK_YSIZE){
            CAM_FUNC_ERR("y blk size can't be < protected size(%d_%d)\n",blk_0.Bits.AF_BLK_YSIZE,blk_2.Bits.AF_PROT_BLK_YSIZE);
            rst = MFALSE;
        }
    }

    //check max afo size, 128*128*AFO_NORMAL_SZ
    afo_xsize = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AFO_R1_AFO_XSIZE,AFO_XSIZE);
    afo_ysize = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AFO_R1_AFO_YSIZE,AFO_YSIZE);
    //xsize/ysize
    xsize = blk_1.Bits.AF_BLK_XNUM*AFO_NORMAL_SZ;
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AFO_R1_AFO_XSIZE,AFO_XSIZE) != (xsize -1 )){
        CAM_FUNC_ERR("afo xsize mismatch:0x%x_0x%x\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AFO_R1_AFO_XSIZE,AFO_XSIZE),(xsize -1 ));
        rst = MFALSE;
    }
    ysize = blk_1.Bits.AF_BLK_YNUM;
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AFO_R1_AFO_YSIZE,AFO_YSIZE) != (ysize -1 )){
        CAM_FUNC_ERR("afo ysize mismatch:0x%x_0x%x\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AFO_R1_AFO_YSIZE,AFO_YSIZE),(ysize -1 ));
        rst = MFALSE;
    }


    //lookup table need to maintian incremental
    {
        #define _bitf_h(idx,x) AF_H_TH_##idx##_D##x
        #define _reg_h(idx,x) AF_R1_AF_LUT_H##idx##_##x
        #define _bitf_v(x) AF_V_TH_D##x
        #define _reg_v(x) AF_R1_AF_LUT_V_##x

        #define bitf_h(idx,x) _bitf_h(idx,x)
        #define reg_h(idx,x) _reg_h(idx,x)
        #define bitf_v(x) _bitf_v(x)
        #define reg_v(x) _reg_v(x)

        #define hor_lut(k,j,i) {\
            if( (((i+1)*CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_h(k,j),bitf_h(k,i+1))) - \
                ((i)*CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_h(k,j),bitf_h(k,i))))  < 0 ){  \
                CAM_FUNC_ERR("hor lut talbe:%d should be incremental(%d)\n",k,i); \
            }\
        }

        #define ver_lut(j,i) {\
            if( (((i+1)*CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_v(j),bitf_v(i+1))) - \
                ((i)*CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_v(j),bitf_v(i))))  < 0 ){  \
                CAM_FUNC_ERR("ver lut table should be oncremental(%d)\n",i);    \
            }\
        }

        #define en_chk(idx) ({\
            MBOOL chk = MFALSE;\
            if(af_con2.Bits.AF_H_FV##idx##_EN == 1)\
                chk = MTRUE;\
            chk;\
        })

        #define reg_mode(z) AF_H_TH_##z##_LUT_MODE
        #define reg_h_lut(z) AF_R1_AF_LUT_H##z##_0

#if 0
        //horizontal
        for(MUINT32 z=0;z<3;z++){
            if(en_chk(z)){
                for(MUINT32 y=1;y<3;y++){
                    for(MUINT32 w=1;w<4;w++){
                        hor_lut(z,y, (w + 4*(y-1)) );
                    }
                }
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_h_lut(z),reg_mode(z))){
                    for(MUINT32 y=3;y<5;y++){
                        for(MUINT32 w=1;w<4;w++){
                            hor_lut(z,y, (w + 4*(y-1)) );
                        }
                    }
                }
            }
        }
#else
        #define layer_2_h(l2) {\
                    hor_lut(l2,1,1);\
                    hor_lut(l2,1,2);\
                    hor_lut(l2,1,3);\
                    hor_lut(l2,2,5);\
                    hor_lut(l2,2,6);\
                    hor_lut(l2,2,7);\
                    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_h_lut(l2),reg_mode(l2))){\
                        hor_lut(l2,3,9);\
                        hor_lut(l2,3,10);\
                        hor_lut(l2,3,11);\
                        hor_lut(l2,4,13);\
                        hor_lut(l2,4,14);\
                        hor_lut(l2,4,15);\
                    }\
                }

        if(en_chk(0)){
            layer_2_h(0);
        }
        if(en_chk(1)){
            layer_2_h(1);
        }
        if(en_chk(2)){
            layer_2_h(2);
        }

#endif
#if 0
        //vertical
        if(af_con2.Bits.AF_V_FV0_ABS){
            for(MUINT32 y=1;y<3;y++){
                for(MUINT32 w=1;w<4;w++){
                    ver_lut(y, (w + 4*(y-1)) );
                }
            }
            if(this->m_pDrv->getPhyObj(),reg_v(0),AF_V_TH_LUT_MODE){
                for(MUINT32 y=3;y<5;y++){
                    for(MUINT32 w=1;w<4;w++){
                        ver_lut(y, (w + 4*(y-1)) );
                    }
                }
            }
        }
#else
        if(af_con2.Bits.AF_V_FV0_ABS){
            ver_lut(1,1);
            ver_lut(1,2);
            ver_lut(1,3);
            ver_lut(2,5);
            ver_lut(2,6);
            ver_lut(2,7);
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg_v(0),AF_V_TH_LUT_MODE)){
                ver_lut(3,9);
                ver_lut(3,10);
                ver_lut(3,11);
                ver_lut(4,13);
                ver_lut(4,14);
                ver_lut(4,15);
            }
        }
#endif
    }

    {
        #define _bitf(idx,x) AF_SGG##idx##_GMR_##x
        #define _reg(idx,x) AF_R1_AF_SGG##idx##_##x

        #define bitf(idx,x) _bitf(idx,x)
        #define reg(idx,x) _reg(idx,x)

        #define SGG(idx,z,i) {\
            if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg(idx,z),bitf(idx,i+1)) - \
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),reg(idx,z),bitf(idx,i)))  < 0 ){  \
                CAM_FUNC_ERR("SGG%d:%d should be incremental(%d)\n",idx,i); \
            }\
        }

        if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_SGG1_1,AF_SGG1_GMR_2) -
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_SGG1_0,AF_SGG1_GMR_1))  < 0 ){
            CAM_FUNC_ERR("SGG1:1/2 should be incremental\n");
        }
        if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_SGG5_1,AF_SGG5_GMR_2) -
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),AF_R1_AF_SGG5_0,AF_SGG5_GMR_1))  < 0 ){
            CAM_FUNC_ERR("SGG5:1/2 should be incremental\n");
        }
        //SGG should be incremental
#if 0
        for(MUINT32 z=1;z<=5;z++){
            SGG(1,z,2*z);
            SGG(5,z,2*z);
        }
#else
        #define layer(idx){\
            SGG(idx,1,2);\
            SGG(idx,2,4);\
            SGG(idx,3,6);\
            SGG(idx,4,8);\
            SGG(idx,5,10);\
        }

        layer(1);
        layer(5);
#endif
    }
EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("af check fail:cur mux:0x%x\n",af_sel);
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
    MUINT32 in_h,in_v;

    this->InPutSize_TG(in_h,in_v,MTRUE);

    pdo_sel = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_PDO_SEL);

    switch(pdo_sel){
        case 1:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_PDE_R1_EN) ^
               CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDI_R1_EN))
            {
                CAM_FUNC_ERR("pde & pdi must enable/disable at the same time(%d_%d)\n",
                    CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_PDE_R1_EN),
                    CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDI_R1_EN));
                rst = MFALSE;
            }
            else if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDI_R1_EN)){
                //can't use stride, pdi is 1-D
                if(this->PD_TABLE_CHECK(this->m_PDI_VA,CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDI_R1_PDI_XSIZE,PDI_XSIZE)+1) == MFALSE){
                    rst = MFALSE;
                    CAM_FUNC_ERR("PDI's check MAY BE fail due to table\n");
                }
            }
            break;
        case 0:
            {
                capibility CamInfo(this->m_hwModule);
                tCAM_rst ret;

                REG_CRP_R8_CRP_X_POS crp_x;
                REG_CRP_R8_CRP_Y_POS crp_y;
                REG_TG_R1_TG_SEN_MODE sen_mode;
                MUINT32 pixmode = 0;

                CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_FUNC,\
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),ret);
                if(ret.bSupportedModule.bPDE == MFALSE){
                    CAM_FUNC_ERR("current cam_%d have no pde\n",this->m_hwModule);
                    rst = MFALSE;
                    goto EXIT;
                }

                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN)){
                    MUINT32 input_v;
                    sen_mode.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE);
                    pixmode = sen_mode.Bits.TG_DBL_DATA_BUS + sen_mode.Bits.TG_DBL_DATA_BUS1;

                    if(pixmode == 0){
                        rst = MFALSE;
                        CAM_FUNC_ERR("PBN can't support 1-pix mode\n");
                    }

                    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R8_EN)){
                        crp_x.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRP_R8_CRP_X_POS);
                        crp_y.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRP_R8_CRP_Y_POS);
                        if( (in_h >> pixmode) < (crp_x.Bits.CRP_XEND - crp_x.Bits.CRP_XSTART + 1) ){
                            rst = MFALSE;
                            CAM_FUNC_ERR("crop size > tg size (%d_%d)\n",(in_h >> pixmode),(crp_x.Bits.CRP_XEND - crp_x.Bits.CRP_XSTART + 1));
                        }
                        if(in_v < (crp_y.Bits.CRP_YEND - crp_y.Bits.CRP_YSTART + 1)){
                            rst = MFALSE;
                            CAM_FUNC_ERR("crop size > tg size (%d_%d)\n",in_v,(crp_y.Bits.CRP_YEND - crp_y.Bits.CRP_YSTART + 1));
                        }

                        //
                        input_v = (crp_y.Bits.CRP_YEND - crp_y.Bits.CRP_YSTART + 1);
                    }
                    else
                        input_v = in_v;

                    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PBN_R1_PBN_PBN_VSIZE,PBN_PBN_VSIZE) != input_v){
                        rst = MFALSE;
                        CAM_FUNC_ERR("PBN vsize err(%d_%d)\n",input_v,\
                            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PBN_R1_PBN_PBN_VSIZE,PBN_PBN_VSIZE));
                    }
                }
            }
            break;
        default:
            rst = MFALSE;
            CAM_FUNC_ERR("unsupported pdo path:%d\n",pdo_sel);
            break;
    }
    //pdo
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN) == 1){
        if(pdo_sel == 0) {//
            MUINT32 pdo_stride, pdo_ysize;
            pdo_stride = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_STRIDE, PDO_STRIDE);
            pdo_ysize = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_YSIZE, PDO_YSIZE)+1;

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
            //if sel = 0 => running at UFO mode. need to have 16B alignment
            //BA alignment check
            MUINT32 _cnt = 0;
            MUINT32 xsize = in_h*PDO_PIX_B;
            tmp = (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_BASE_ADDR,PDO_BASE_ADDR) + \
                    CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_OFST_ADDR,PDO_OFST_ADDR));
            do{
                if((tmp % 16 ) != 0){
                    CAM_FUNC_ERR("pdo/fh_pdo base addr + offset addr must be 16-alignment(0x%x)\n",tmp);
                    rst = MFALSE;
                }

                if(_cnt++ > 0)
                    break;

                tmp = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_FH_BASE_ADDR,PDO_FH_BASE_ADDR);
            }while(1);

            //stride/xsize alignment check
            tmp = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_STRIDE,PDO_STRIDE);
            if((tmp % 16 ) != 0){
                CAM_FUNC_ERR("pdo stride must be 16-alignment(0x%x),suggest:0x%x\n",tmp,(xsize+16)/16*16);
                rst = MFALSE;
            }

#if 0   //under UF MODE , stride need to be 16-alignment, won't be xsize+1
            if(tmp != (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_XSIZE,PDO_XSIZE)+1) ){
                CAM_FUNC_ERR(" pdo stride must be = xsize+1 \n");
                rst = MFALSE;
            }
#endif
        }
    }


EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("pd check fail:cur mux:0x%x\n",pdo_sel);
    }
    return rst;
}

MBOOL PIPE_CHECK::OBC_CHECK(void)
{
    MBOOL rst = MTRUE;

    switch(this->m_hwModule){
        case CAM_C:
            break;
        default:
#if 0   //RTL hardcode , always 0xF
            if( 0 == CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN)){
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),OBC_R1_OBC_CTL,OBC_EDGE) != 0xF){
                    rst = MFALSE;
                    CAM_FUNC_ERR("edge should be fixed at 0xF\n");
                }
            }
#endif
            break;
    }
    return rst;
}

MBOOL PIPE_CHECK::BPC_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_PDC_CON,BPC_PDC_EN) == 1){//case for phase detection
        MUINT32 tmp = 0;
        //
        tmp = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_PDC_CON,BPC_PDC_EN) * \
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_CON,BPC_BPC_LUT_EN) * \
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN);
        if(tmp == 0){
            rst = 0;
            CAM_FUNC_ERR("PDC fail (pdc_en/lut_en/CAMCTL_BPCI_R1_EN:%d_%d_%d)\n",\
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_PDC_CON,BPC_PDC_EN),\
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_CON,BPC_BPC_LUT_EN),\
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN));
        }

        if ( 0 == CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN)){
            MUINT32 in_h = 0,in_v = 0;
            REG_BPC_R1_BPC_BPC_TBLI1 tbl1;
            REG_BPC_R1_BPC_BPC_TBLI2 tbl2;
            tbl1.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_TBLI1);
            tbl2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_TBLI2);

            //
            this->InPutSize_SEP(in_h,in_h);
            if((in_h-1) != (tbl1.Bits.BPC_XOFFSET + tbl2.Bits.BPC_XSIZE)) {
                CAM_FUNC_ERR("bnr bpc_xize error:0x%x_0x%x",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_TBLI2,BPC_XSIZE),in_h-1);
                rst = MFALSE;
            }
            if((in_v-1) != (tbl1.Bits.BPC_YOFFSET + tbl2.Bits.BPC_YSIZE)) {
                CAM_FUNC_ERR("bnr bpc_yize error:0x%x_0x%x",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_TBLI2,BPC_YSIZE),in_v-1);
                rst = MFALSE;
            }


            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_CON,BPC_BNR_EDGE) != 0xF){
                rst = MFALSE;
                CAM_FUNC_ERR("edge must be 0xF\n");
            }
        }
        else {
            /*twin case, twin driver will take care size setting, we just check size should > 0*/
            if(0 == CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_TBLI2,BPC_XSIZE)){
                CAM_FUNC_ERR("bnr bpc_xize must > 0");
                rst = MFALSE;
            }
            if(0 == CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_TBLI2,BPC_YSIZE)){
                CAM_FUNC_ERR("bnr bpc_yize must > 0");
                rst = MFALSE;
            }
        }
    }
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_CON,BPC_BPC_EN)){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),BPC_R1_BPC_BPC_CON,BPC_BPC_LUT_EN) ^ \
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN) ){
            rst = MFALSE;
            CAM_FUNC_ERR("can't user bpc look-up table without input table\n");
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
        MUINT32 length = 0;
        MUINT32 cnum = 0,pnum = 0;
        REG_PDE_R1_PDE_TBLI1 oft;
        MUINT32 img_h,img_v;
        this->InPutSize_TG(img_h,img_v);

        oft.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),PDE_R1_PDE_TBLI1);
        for(MUINT32 i=0; i<tbl_stride; )
        {
            //pd line
            if( ((*ptr)&SPECIAL_TOKEN)==SPECIAL_TOKEN){
                ysize++;
                //ofset
                n_2bytejump = 1;
                i += (n_2bytejump*2);
                ptr += n_2bytejump;
                length = oft.Bits.PDE_XOFFSET + *ptr;
                if(length > img_h){
                    CAM_FUNC_ERR("out of range(%d_%d)\n",length , img_h);
                    rst = MFALSE;
                }
                //cnum
                n_2bytejump = 1;
                i += (n_2bytejump*2);
                ptr += n_2bytejump;
                cnum = *ptr;
                //pnum
                n_2bytejump = 1;
                i += (n_2bytejump*2);
                ptr += n_2bytejump;
                pnum = (*ptr) + 1;
                //line end, should accumulate length + pnum based on cnum.
                CAM_FUNC_WRN("valid line end have no pipeline check\n");
                //jump number of pnum
                n_2bytejump = pnum + 1;//+ 1 for next special token
                i += (n_2bytejump*2);
                ptr += n_2bytejump;
            }
            else{
                i+=2;
                ptr = ptr + 1;
            }
        }

    #undef SPECIAL_TOKEN
        if( (ysize+oft.Bits.PDE_YOFFSET) > img_v){
            CAM_FUNC_ERR("out of range(%d_%d)\n",(ysize+oft.Bits.PDE_YOFFSET) , img_v);
            rst = MFALSE;
        }

        if(ysize != (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_YSIZE,PDO_YSIZE) + 1)){
            ptr = (unsigned short int*)va;
            CAM_FUNC_ERR("PD table error:0x%x_0x%x\n",ysize,(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),PDO_R1_PDO_YSIZE,PDO_YSIZE) + 1));
            CAM_FUNC_ERR("table content : 1st pdc line : 0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x\n",
                ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);
            rst = MFALSE;
        }
    }

    return rst;
}

MBOOL PIPE_CHECK::TG_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
    MUINT32 TG_IN_W,TG_IN_V;
    MUINT32 TG_IN_W_R,TG_IN_V_R;
    MUINT32 DATA_CNT_R;
    MUINT32 tmp, needReset = 0;
    MBOOL bTgRdy;
    SENINF_DBG seninf_dbg;
    REG_CAMCTL_R1_CAMCTL_INT_STATUS irqStatCheck;
    MUINT32 bInnerDump = MTRUE;
    REG_TG_R1_TG_SEN_MODE sen_mode;
    MUINT32 pixmode = 0;
    REG_TG_R1_TG_TIME_STAMP_CTL ctrl;
    REG_TG_R1_TG_VF_CON vf_con;
    ISP_DRV_CAM* ptr = NULL;
    MBOOL bDestroy = MTRUE;

    switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL)){
        case RAW_SEL_0:
            ptr = this->m_pDrv;
            bDestroy = MFALSE;
            break;
        case RAW_SEL_3:
            {
                switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAW_TG_SEL)){
                    case FROM_TG_A:
                        if(this->m_hwModule == CAM_A){
                            rst = MFALSE;
                            CAM_FUNC_ERR("CAM_A with RAW_SEL_3 + RAW_TG_SEL_A ??\n");
                        }
                        else{
                            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_A,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                            if(ptr){
                                ptr->init(__FUNCTION__);
                            }
                            else{
                                rst = MFALSE;
                                CAM_FUNC_ERR("no TG check due to create drv obj:CAM_B fail\n");
                                return MFALSE;
                            }
                        }
                        break;
                    case FROM_TG_B:
                        if(this->m_hwModule == CAM_B){
                            rst = MFALSE;
                            CAM_FUNC_ERR("CAM_B with RAW_SEL_3 + RAW_TG_SEL_A ??\n");
                        }
                        else{
                            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_B,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                            if(ptr){
                                ptr->init(__FUNCTION__);
                            }
                            else{
                                rst = MFALSE;
                                CAM_FUNC_ERR("no TG check due to create drv obj:CAM_B fail\n");
                                return MFALSE;
                            }
                        }
                        break;
                    case FROM_TG_C:
                        if(this->m_hwModule == CAM_C){
                            rst = MFALSE;
                            CAM_FUNC_ERR("CAM_C with RAW_SEL_3 + RAW_TG_SEL_A ??\n");
                        }
                        else{
                            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_C,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                            if(ptr){
                                ptr->init(__FUNCTION__);
                            }
                            else{
                                rst = MFALSE;
                                CAM_FUNC_ERR("no TG check due to create drv obj:CAM_C fail\n");
                                return MFALSE;
                            }
                        }
                        break;
                    default:
                        rst = MFALSE;
                        CAM_FUNC_ERR("unsupported raw_tg_sel:%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAW_TG_SEL));
                        break;
                }
            }
            break;
        default:
            rst = MFALSE;
            CAM_FUNC_ERR("unsupported raw_sel:%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL));
            break;
    }

    if(ptr == NULL) {
        goto EXIT;
    }
    ctrl.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_TIME_STAMP_CTL);
    vf_con.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_VF_CON);



    if(vf_con.Bits.TG_VFDATA_EN == 0){
        CAM_FUNC_ERR("#############################");
        CAM_FUNC_ERR("viewfinder is not opened yet\n");
        CAM_FUNC_ERR("user should not wait signal:WDMA_Done/P1_Done/SOF before viewfinder enable\n");
        CAM_FUNC_ERR("#############################");
        rst = MTRUE;
        goto EXIT;
    }
    else{
        irqStatCheck.Raw = this->m_err_status.ispIntErr;
        if (irqStatCheck.Bits.CAMCTL_TG_GBERR_ST) {
            needReset = 1;
        }

        //hw issue. TG can't run with YUV format. plz use bayer8 to run YUV sensor.
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_TG_FMT) == SV_TG_FMT_YUV422){
            rst = MFALSE;
            CAM_FUNC_ERR("TG support no YUV format due to hw issue\n");
        }

        //mux check
        if( (vf_con.Bits.TG_VFDATA_EN_MUX_0_SEL + vf_con.Bits.TG_VFDATA_EN_MUX_1_SEL)!= 0 ){
            rst = MFALSE;
            CAM_FUNC_ERR("mux_0 & mux_1 should be 0\n");
        }

        if(ctrl.Bits.TG_TG_TIME_STAMP_SOF_SEL != 1){
            CAM_FUNC_WRN("source of SOF alwasy from raw_sel is more easier for all hw path\n \n");
        }

        //under twin case, sgg_sel won't be 0 , so , don't need to take into consideration at twin case
        TG_W.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);

        sen_mode.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_SEN_MODE);

        pixmode = sen_mode.Bits.TG_DBL_DATA_BUS + sen_mode.Bits.TG_DBL_DATA_BUS1;
        switch(pixmode){
            case 0: // 1pix
                break;
            case 1: // 2pix
                if(TG_W.Bits.TG_PXL_S & 0x1){
                    CAM_FUNC_ERR("TG's cooordinate must be 2-alignment at 2 pix mode\n");
                    rst = MFALSE;
                }
                if(TG_W.Bits.TG_PXL_E & 0x1){
                    CAM_FUNC_ERR("TG's cooordinate must be 2-alignment at 2 pix mode\n");
                    rst = MFALSE;
                }
                break;
            case 2: // 4pix
                if(TG_W.Bits.TG_PXL_S & 0x3){
                    CAM_FUNC_ERR("TG's cooordinate must be 4-alignment at 4 pix mode\n");
                    rst = MFALSE;
                }
                if(TG_W.Bits.TG_PXL_E & 0x3){
                    CAM_FUNC_ERR("TG's cooordinate must be 4-alignment at 4 pix mode\n");
                    rst = MFALSE;
                }
                break;
            default:
                break;
        }

        TG_IN_W = CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_FRMSIZE_ST,TG_PXL_CNT);
        TG_IN_V = CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_FRMSIZE_ST,TG_LINE_CNT);


        TG_IN_W_R = CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_FRMSIZE_ST_R,TG_PXL_CNT_R);
        TG_IN_V_R = CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_FRMSIZE_ST_R,TG_LINE_CNT_R);

        DATA_CNT_R = CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_DAT_NO_R,TG_DAT_NO_R);

        if(TG_IN_W < (TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S)){
            bInnerDump = MFALSE;
            CAM_FUNC_ERR("current seninf horizontal data is small than grab window_w:%d_%d\n",TG_IN_W,(TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S));
            rst = MFALSE;
        }
        if(TG_IN_V < (TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S)){
            bInnerDump = MFALSE;
            CAM_FUNC_ERR("current seninf vertical data is small than grab window_v:%d_%d\n",TG_IN_V,(TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S));
            rst = MFALSE;
        }
        if(bInnerDump == MTRUE){
            if(irqStatCheck.Bits.CAMCTL_TG_GBERR_ST){//this case meas tg_grab is happened during streaming. (not happned af the beginning of streaming)
                #define LOOP (10)
                char str[256] = {'\0'};
                char tmp[32] = {'\0'};
                for(MUINT32 i=0;i<LOOP;i++){
                    snprintf(tmp,sizeof(tmp),"0x%x,",CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_FRMSIZE_ST_R));
                    strncat(str,tmp,sizeof(tmp));
                    usleep(500);
                }
                CAM_FUNC_ERR("Frm_R size:%s\n",str);
            }
        }


        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN)) {
            if(((TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S) % 16 ) != 0){
                CAM_FUNC_ERR("tg grab width must be 16-alignment(0x%x) when pbn_en=1\n",\
                    (TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S));
                rst = MFALSE;
            }
        }

        tmp = CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_INTER_ST,TG_TG_CAM_CS);
        switch(tmp){
            case 2: //
            case 16:
                usleep(1000);
                if(TG_IN_W_R != TG_IN_W){
                    if(DATA_CNT_R == CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_DAT_NO_R,TG_DAT_NO_R)){
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
    }

    //timestamp check
    if(CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_SEN_MODE,TG_TIME_STP_EN)){
        if(ctrl.Bits.TG_TG_TIME_STAMP_SEL == 0){
            rst = MFALSE;
            CAM_FUNC_ERR("running with Local timer???????\n");

            if(ctrl.Bits.TG_TG_TIME_STAMP_SOF_SEL == 1){
                CAM_FUNC_ERR("local timer is not supported when SOF is from raw_sel\n");
            }

            if(CAM_READ_BITS_INR(ptr->getPhyObj(),TG_R1_TG_TIME_STAMP_CNT,TG_TIME_STAMP_CNT) == 0){
                CAM_FUNC_ERR("timestamp will be 0\n");
            }
        }
    }
    else{
        rst = MFALSE;
        CAM_FUNC_ERR("no timestamp\n");
    }



    if(rst == MFALSE){
        CAM_FUNC_ERR("TG check fail\n");
    }

EXIT:

    if(bDestroy && ptr){
        ptr->uninit(__FUNCTION__);
        ptr->destroyInstance();
    }
    if(seninf_dbg.m_fp_Sen != NULL){
        CAM_FUNC_ERR("start dump seninf info\n");
        seninf_dbg.m_fp_Sen((MUINT32)this->m_hwModule, needReset);
    }

    return rst;

}

MBOOL PIPE_CHECK::RRZ_CHECK(void)
{
    MBOOL           rst = MTRUE;
    MUINT32         rrz_in[2];
    MUINT32         rrz_crop[4];
    capibility CamInfo;
    tCAM_rst ret;
    MUINT32 rlb_oft;
    MUINT32 phy_in_w,phy_in_h;
    REG_RRZ_R1_RRZ_CTL ctrl;
    MUINT32 pattern = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_DATA_PATTERN);

    if( CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_RRZ_R1_EN) ^
        CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN)){
        rst = MFALSE;
        //rrz enable without rrzo is not supported by drv currently.
        CAM_FUNC_ERR("rrz/rrzo must be enabled at the same time(%d_%d)\n",\
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_RRZ_R1_EN)\
            ,CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN));
        goto EXIT;
    }
    else if (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_RRZ_R1_EN) == 0){
        CAM_FUNC_WRN("rrz is not enabled.\n");
        goto EXIT;
    }

    //support no scale-up
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN4,CAMCTL_RLB_R1_EN) != 1){
        REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin;
        twin.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);
        if( (twin.Bits.TWIN_EN == 0) || ((this->m_hwModule == twin.Bits.MASTER_MODULE) && (twin.Bits.TWIN_EN == 1)) ){
            CAM_FUNC_ERR("can't enable rrz without RLB\n");
            rst = MFALSE;
        }
    }

    rrz_in[0] = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_IN_IMG);
    rrz_in[1] = rrz_in[0]>>16;
    rrz_in[0] = rrz_in[0] & 0xffff;
    this->InPutSize_SEP(phy_in_w,phy_in_h);

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

    rrz_crop[0] = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_HORI_INT_OFST);
    rrz_crop[1] = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_VERT_INT_OFST);

    rrz_crop[2] = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_OUT_IMG);
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
    rlb_oft = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_RLB_AOFST,RRZ_RLB_AOFST);
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

    ctrl.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_CTL);
    if( (ctrl.Bits.RRZ_HORI_EN & ctrl.Bits.RRZ_VERT_EN) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("hrz/ver scaler must be enabled\n");
    }

    switch(pattern){
        case eCAM_MONO:
            if(ctrl.Bits.RRZ_MONO == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("pattern mismatch\n");
            }
            break;
        default:
            if(ctrl.Bits.RRZ_MONO == 1){
                rst = MFALSE;
                CAM_FUNC_ERR("pattern mismatch\n");
            }
            break;
    }

    //RRZO
    {
        MUINT32 _tmp = 0;
        _tmp = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_RRZO_FG_MODE) +
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PAKG_R1_FG_IN) +
            CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PAKG_R1_FG_OUT);

        if((_tmp != 0) && (_tmp != 3)){

            CAM_FUNC_ERR("FG pattern err(%d_%d_%d),IQ defect or hw hang if FG_OUT & RRZO xsize mismatch\n",
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_RRZO_FG_MODE),
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PAKG_R1_FG_IN),
                CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PAKG_R1_FG_OUT));

            rst = MFALSE;
        }
    }

EXIT:
    return rst;
}

MBOOL PIPE_CHECK::InPutSize_TG(MUINT32& in_w,MUINT32& in_h,MBOOL pure)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_EN2 en2;
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
    ISP_DRV_CAM* ptr = NULL;
    MBOOL bDestroy = MTRUE;

    if(this->m_pDrv == NULL){
        in_w = in_h = 0;
        BASE_LOG_ERR("can't be NULL ptr, get no input size\n");
        return MFALSE;
    }

    en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2);

    switch(this->HW_Path()) {
        case E_PPC_DL:
            switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL)){
                case RAW_SEL_0:
                    ptr = this->m_pDrv;
                    bDestroy = MFALSE;
                    break;
                case RAW_SEL_3:
                    {
                        switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAW_TG_SEL)){
                            case FROM_TG_A:
                                ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_A,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                                if(ptr){
                                    ptr->init(__FUNCTION__);
                                }
                                else{
                                    ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_A,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                                    if(ptr){
                                        ptr->init(__FUNCTION__);
                                    }
                                }
                                break;
                            case FROM_TG_B:
                                if(this->m_hwModule == CAM_B){
                                    rst = MFALSE;
                                    CAM_FUNC_ERR("CAM_B with RAW_SEL_3 + RAW_TG_SEL_A ??\n");
                                }
                                else{
                                    ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_B,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                                    if(ptr){
                                        ptr->init(__FUNCTION__);
                                    }
                                }
                                break;
                            case FROM_TG_C:
                                if(this->m_hwModule == CAM_C){
                                    rst = MFALSE;
                                    CAM_FUNC_ERR("CAM_C with RAW_SEL_3 + RAW_TG_SEL_A ??\n");
                                }
                                else{
                                    ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( CAM_C,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
                                    if(ptr){
                                        ptr->init(__FUNCTION__);
                                    }
                                }
                                break;
                            default:
                                rst = MFALSE;
                                CAM_FUNC_ERR("unsupported raw_tg_sel:%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAW_TG_SEL));
                                break;
                        }
                    }
                    break;
                default:
                    rst = MFALSE;
                    CAM_FUNC_ERR("unsupported raw_sel:%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL));
                    break;
            }
            if(ptr){
                TG_W.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);

                if(bDestroy){
                    ptr->uninit(__FUNCTION__);
                    ptr->destroyInstance();
                }
            }
            break;
        case E_PPC_DC:
            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
            break;
        case E_PPC_RAWI:
            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
            break;
        default:
            CAM_FUNC_ERR("HW path:%d is not supported\n",this->HW_Path());
            return MFALSE;
            break;
    }

    in_w = TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S;
    in_h = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;

    if(pure == MFALSE){
        if(en2.Bits.CAMCTL_DBN_R1_EN){
                in_w = in_w>>1;
        }
        if(en2.Bits.CAMCTL_FBND_R1_EN){
            in_w = in_w>>1;
            in_h = in_h>>1;
        }
        else if(en2.Bits.CAMCTL_BIN_R1_EN){
            in_w = in_w>>1;
            in_h = in_h>>1;
        }

        CAM_FUNC_WRN("input size from tg after bin/fbnd/dbn:%d_%d(0x%x)\n",in_w,in_h,en2.Raw);
    }
    else
        CAM_FUNC_WRN("input size from tg without bin:%d_%d\n",in_w,in_h);

    return rst;
}

MBOOL PIPE_CHECK::InPutSize_SEP(MUINT32& in_w,MUINT32& in_h)
{
    MBOOL rst = MTRUE;
    REG_SEP_R1_SEP_CROP    sep_w;
    REG_SEP_R1_SEP_VSIZE   sep_h;
    REG_CAMCTL_R1_CAMCTL_EN2 en2;
    if(this->m_pDrv == NULL){
        in_w = in_h = 0;
        BASE_LOG_ERR("can't be NULL ptr, get no input size\n");
        return MFALSE;
    }
    en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2);
    sep_w.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),SEP_R1_SEP_CROP);
    sep_h.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),SEP_R1_SEP_VSIZE);

    in_w = sep_w.Bits.SEP_END_X - sep_w.Bits.SEP_STR_X + 1;
    in_h = sep_h.Bits.SEP_HT;

    if(en2.Bits.CAMCTL_FBND_R1_EN){
        in_w = in_w>>1;
        in_h = in_h>>1;
    }
    CAM_FUNC_WRN("input size from sep after fbnd:%d_%d\n",in_w,in_h);
    return rst;
}


MBOOL PIPE_CHECK::RdyReq_Dump(void)
{
    #define grp (6)
    REG_CAMCTL_R1_CAMCTL_DBG_SET set;
    MUINT32 req[grp],rdy[grp];
    set.Raw = 0;
    set.Bits.CAMCTL_SNAPSHOT_SEL = 0x80;

    for(MUINT32 i=1;i<=grp;i++) {
        //
        set.Bits.CAMCTL_DEBUG_SEL = i;
        set.Bits.CAMCTL_DEBUG_TOP_SEL = 0;
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_SET,set.Raw);
        req[i-1] = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
        //
        set.Bits.CAMCTL_DEBUG_TOP_SEL = 1;
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_SET,set.Raw);
        rdy[i-1] = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DBG_PORT);
    }

    CAM_FUNC_ERR("[req1:0x%8x,rdy1:0x%8x],[req2:0x%8x,rdy2:0x%8x],[req3:0x%8x,rdy3:0x%8x]\n",\
        req[0],rdy[0],req[1],rdy[1],req[2],rdy[2]);
    CAM_FUNC_ERR("[req4:0x%8x,rdy4:0x%8x],[req5:0x%8x,rdy5:0x%8x],[req6:0x%8x,rdy6:0x%8x]\n",\
        req[3],rdy[3],req[4],rdy[4],req[5],rdy[5]);
    return MTRUE;
}

MBOOL PIPE_CHECK::LTM_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 x_pos;
    REG_SEP_R1_SEP_CROP sep_crp;
    REG_LTM_R1_LTM_BLK_NUM blk_num;
    REG_LTM_R1_LTM_BLK_SZ blk_sz;
    REG_LTM_R1_LTM_TILE_NUM tile_num;
    REG_LTM_R1_LTM_TILE_CNTX tile_cntx;
    REG_LTM_R1_LTM_TILE_CNTY tile_cnty;
    REG_LTM_R1_LTM_TILE_SIZE tile_size;
    REG_LTM_R1_LTM_ATPG atpg;
    MUINT32 tmp_h,tmp_v,tmp,sep_width,sep_height;

    this->InPutSize_SEP(sep_width,sep_height);

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_LTM_R1_EN) == 0){
        CAM_FUNC_WRN("bypass LTM\n");
        return rst;
    }

    //
    sep_crp.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),SEP_R1_SEP_CROP);
    x_pos = sep_crp.Bits.SEP_STR_X;
    //
    blk_num.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_BLK_NUM);
    blk_sz.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_BLK_SZ);
    tile_num.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_TILE_NUM);
    tile_cntx.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_TILE_CNTX);
    tile_cnty.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_TILE_CNTY);

    if(((tile_num.Bits.LTM_TILE_BLK_X_NUM_END - tile_num.Bits.LTM_TILE_BLK_X_NUM_START) * blk_sz.Bits.LTM_BLK_WIDTH + \
        (tile_cntx.Bits.LTM_TILE_BLK_X_CNT_END - tile_cntx.Bits.LTM_TILE_BLK_X_CNT_START + 1)) != sep_width){
        rst =MFALSE;
        CAM_FUNC_ERR("ltm-width setting err %d_%d_%d_%d,  size:%d\n",\
            (tile_num.Bits.LTM_TILE_BLK_X_NUM_END - tile_num.Bits.LTM_TILE_BLK_X_NUM_START),blk_sz.Bits.LTM_BLK_WIDTH,\
            tile_cntx.Bits.LTM_TILE_BLK_X_CNT_END,tile_cntx.Bits.LTM_TILE_BLK_X_CNT_START,sep_width);
    }

    if(((tile_num.Bits.LTM_TILE_BLK_Y_NUM_END - tile_num.Bits.LTM_TILE_BLK_Y_NUM_START) * blk_sz.Bits.LTM_BLK_HEIGHT + \
        (tile_cnty.Bits.LTM_TILE_BLK_Y_CNT_END - tile_cnty.Bits.LTM_TILE_BLK_Y_CNT_START + 1)) != sep_height){
        rst =MFALSE;
        CAM_FUNC_ERR("ltm-height setting err %d_%d_%d_%d,  size:%d\n",\
            (tile_num.Bits.LTM_TILE_BLK_Y_NUM_END - tile_num.Bits.LTM_TILE_BLK_Y_NUM_START),blk_sz.Bits.LTM_BLK_HEIGHT,\
            tile_cnty.Bits.LTM_TILE_BLK_Y_CNT_END,tile_cnty.Bits.LTM_TILE_BLK_Y_CNT_START,sep_height);
    }

    //
    tile_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_TILE_SIZE);
    if(tile_size.Bits.LTM_TILE_HSIZE != sep_width){
        rst = MFALSE;
        CAM_FUNC_ERR("tile width error %d_%d\n",tile_size.Bits.LTM_TILE_HSIZE,sep_width);
    }
    if(tile_size.Bits.LTM_TILE_VSIZE != sep_height){
        rst = MFALSE;
        CAM_FUNC_ERR("tile height error %d_%d\n",tile_size.Bits.LTM_TILE_VSIZE,sep_height);
    }

    atpg.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_ATPG);
    if( (atpg.Bits.LTM_ltm_atpg_ob != 0) || (atpg.Bits.LTM_ltm_atpg_ct != 0) ){
        rst = MFALSE;
        CAM_FUNC_ERR("ATPG need to be 0\n");
    }
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),LTM_R1_LTM_TILE_EDGE,LTM_TILE_EDGE) != 0xF){
        rst = MFALSE;
        CAM_FUNC_ERR("tile edge err\n");
    }

    return rst;
}
MBOOL PIPE_CHECK::LTMS_CHECK(void)
{
    #define win_x_pix (20)
    #define win_y_full_frm  (1)
    #define pix_byte    (4)

    MBOOL rst = MTRUE;
    REG_LTMS_R1_LTMS_CTRL       _ctl;
    REG_LTMS_R1_LTMS_BLK_NUM    _num;
    REG_LTMS_R1_LTMS_BLK_SZ     _sz;
    REG_LTMS_R1_LTMS_IN_SIZE    _in_size;
    REG_LTMS_R1_LTMS_OUT_SIZE   _out_size;
    REG_LTMS_R1_LTMS_SRAM_CFG   _sram_cfg;
    REG_LTMS_R1_LTMS_ATPG       _atpg;
    REG_LTMSO_R1_LTMSO_YSIZE    _ysize;
    REG_LTMSO_R1_LTMSO_XSIZE    _xsize;
    MUINT32 img_w,img_v,qbn, blk_w,blk_v,last_blk_w;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_LTMS_R1_EN) == 0){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LTMSO_R1_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable LTMSO without enable LTMS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else {
            CAM_FUNC_WRN("bypass LTMS\n");
            goto EXIT;
       }
    }
    else{
        REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin;
        twin.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);
        if(twin.Bits.TWIN_EN == 1){
            if(this->m_hwModule == twin.Bits.TWIN_MODULE){
                rst = MFALSE;
                CAM_FUNC_ERR("can't enable LTMS at slave cam\n");
            }
        }
    }

    this->InPutSize_TG(img_w,img_v);
    if ((qbn = CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),QBIN_R4_QBIN_CTL,QBIN_ACC))!= 0) {
        img_w = img_w >> qbn;
    }
    //
    _in_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_IN_SIZE);
    if(_in_size.Bits.LTMS_IN_HSIZE != img_w){
        rst = MFALSE;
        CAM_FUNC_ERR("input size-w mismatch:%d_%d\n",img_w,_in_size.Bits.LTMS_IN_HSIZE);
    }
    if(_in_size.Bits.LTMS_IN_VSIZE != img_v){
        rst = MFALSE;
        CAM_FUNC_ERR("input size-v mismatch:%d_%d\n",img_w,_in_size.Bits.LTMS_IN_HSIZE);
    }
    //
    _ctl.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_CTRL);
    _out_size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_OUT_SIZE);
    switch(_ctl.Bits.LTMS_RESIZE_MODE){
        case 0:
            img_w = img_w >> 1;
            img_v = img_v >> 1;
            break;
        case 1:
            img_v = img_v >> 1;
            break;
        case 2:
            img_v = img_v >> 2;
            break;
        default:
            CAM_FUNC_ERR("resize mode err %d\n",_ctl.Bits.LTMS_RESIZE_MODE);
            rst = MFALSE;
            break;
    }
    if(_out_size.Bits.LTMS_OUT_HSIZE != img_w){
        rst = MFALSE;
        CAM_FUNC_ERR("output size-w mismatch:%d_%d\n",img_w,_out_size.Bits.LTMS_OUT_HSIZE);
    }
    if(_out_size.Bits.LTMS_OUT_VSIZE != img_v){
        rst = MFALSE;
        CAM_FUNC_ERR("output size-v mismatch:%d_%d\n",img_w,_out_size.Bits.LTMS_OUT_VSIZE);
    }
    //
    _num.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_BLK_NUM);
    _sz.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_BLK_SZ);
    if( (_num.Bits.LTMS_BLK_X_NUM < 4) || (_num.Bits.LTMS_BLK_X_NUM > 12) ){
        rst = MFALSE;
        CAM_FUNC_ERR("4<= xnum <= 12 (may not crash , but output data will err)\n");
    }
    if( (_num.Bits.LTMS_BLK_Y_NUM < 3) || (_num.Bits.LTMS_BLK_Y_NUM > 9) ){
        rst = MFALSE;
        CAM_FUNC_ERR("3<= ynum <= 9 (may not crash , but output data will err)\n");
    }

    blk_w = (img_w + (_num.Bits.LTMS_BLK_X_NUM-1)) / _num.Bits.LTMS_BLK_X_NUM;
    blk_v = (img_v + (_num.Bits.LTMS_BLK_Y_NUM-1)) / _num.Bits.LTMS_BLK_Y_NUM;
    if(_sz.Bits.LTMS_BLK_WIDTH != blk_w) {
        rst = MFALSE;
        CAM_FUNC_ERR("blk_sz mismatch:%d_%d\n",_sz.Bits.LTMS_BLK_WIDTH,blk_w);
    }
    if(_sz.Bits.LTMS_BLK_WIDTH < 52 ){
        rst = MFALSE;
        CAM_FUNC_ERR("blk_sz_%d too small\n",_sz.Bits.LTMS_BLK_WIDTH);
    }
    last_blk_w = img_w - ((_num.Bits.LTMS_BLK_X_NUM-1) * _sz.Bits.LTMS_BLK_WIDTH );
    if(last_blk_w < 52 ){
        rst = MFALSE;
        CAM_FUNC_ERR("last blk_sz_%d too small\n",last_blk_w);
    }

    if(_sz.Bits.LTMS_BLK_HEIGHT != blk_v) {
        rst = MFALSE;
        CAM_FUNC_ERR("blk_sz mismatch:%d_%d\n",_sz.Bits.LTMS_BLK_HEIGHT,blk_v);
    }

    //
    _sram_cfg.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_SRAM_CFG);
    _atpg.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMS_R1_LTMS_ATPG);
    if((_sram_cfg.Bits.LTMS_SRAM_RREQ_EN + _sram_cfg.Bits.LTMS_SRAM_WREQ_EN) != 2){
        rst = MFALSE;
        CAM_FUNC_ERR("r/w req must be enabled\n");
    }
    if( (_atpg.Bits.LTMS_ltms_atpg_ob != 0) || (_atpg.Bits.LTMS_ltms_atpg_ct != 0) ){
        rst = MFALSE;
        CAM_FUNC_ERR("ATPG need to be 0\n");
    }

    //
    _xsize.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMSO_R1_LTMSO_XSIZE);
    _ysize.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),LTMSO_R1_LTMSO_YSIZE);
    if( (_xsize.Bits.LTMSO_XSIZE+1) != (_num.Bits.LTMS_BLK_X_NUM * win_x_pix * pix_byte) ){
        rst = MFALSE;
        CAM_FUNC_ERR("xsize err:0x%x_0x%x\n",(_xsize.Bits.LTMSO_XSIZE+1),(_num.Bits.LTMS_BLK_X_NUM * win_x_pix * pix_byte));
    }
    if( (_ysize.Bits.LTMSO_YSIZE+1) != (_num.Bits.LTMS_BLK_Y_NUM + win_y_full_frm) ){
        rst = MFALSE;
        CAM_FUNC_ERR("Ysize err:0x%x_0x%x\n",(_ysize.Bits.LTMSO_YSIZE+1),(_num.Bits.LTMS_BLK_Y_NUM + win_y_full_frm));
    }

EXIT:
    return rst;
}


DYUV_PIPECHK::DYUV_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL, start NE\n");
    }

    m_pDrv = obj;
    m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
}

#define DYUV_PIPELINE(obj) {\
    if(obj.DM_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("DM is pass");\
    if(obj.CCM_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("CCM is pass");\
    if(obj.GGM_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("GGM is pass");\
    if(obj.G2C_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("G2C is pass");\
    if(obj.CRP_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("CRP is pass");\
    if(obj.CRSP_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("CRSP is pass");\
    if(obj.CRZ_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("CRZ is pass");\
    if(obj.C42_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("C42 is pass");\
    if(obj.YNRS_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("YNRS is pass");\
    if(obj.BS_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("BS is pass");\
    if(obj.DMAO_CHECK() == MFALSE)\
        rst = MFALSE;\
    else\
        CAM_FUNC_WRN("dmao is pass");\
}

MBOOL DYUV_PIPECHK::InPutSize(E_YUV_STATE yuvo)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_SEL2 sel2;
    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin;
    twin.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);

    sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2);

    switch(sel2.Bits.CAMCTL_DM_R1_SEL){
        case 0:
            {
                REG_SEP_R1_SEP_CROP crp;
                if(yuvo == E_BYPASS){
                    CAM_FUNC_WRN("power issue, more power consumption\n");
                }
                crp.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),SEP_R1_SEP_CROP);
                this->m_InPut.w = crp.Bits.SEP_END_X - crp.Bits.SEP_STR_X + 1;
                this->m_InPut.h = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),SEP_R1_SEP_VSIZE);

                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN)){
                    this->m_InPut.w = this->m_InPut.w>>1;
                    this->m_InPut.h = this->m_InPut.h>>1;
                }
            }
            break;
        case 1:
            {
                REG_RRZ_R1_RRZ_OUT_IMG rrz;
                if(yuvo != E_BYPASS){
                    CAM_FUNC_ERR("DM_SEL can't be 1 when direct YUVO output\n");
                    return MFALSE;
                }
                rrz.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),RRZ_R1_RRZ_OUT_IMG);
                this->m_InPut.w = rrz.Bits.RRZ_OUT_WD;
                this->m_InPut.h = rrz.Bits.RRZ_OUT_HT;
            }
            break;
    }

    if(twin.Bits.TWIN_EN){
        REG_MRG_R13_MRG_CTL mrg_r13;
        mrg_r13.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),MRG_R13_MRG_CTL);
        if( (mrg_r13.Bits.MRG_SIG_MODE1 == 0) && (mrg_r13.Bits.MRG_SIG_MODE2 == 0)){
            ISP_DRV_CAM* ptr = NULL;
            REG_MRG_R14_MRG_CROP mrg_r14;
            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance((ISP_HW_MODULE)twin.Bits.TWIN_MODULE,ISP_DRV_CQ_THRE0,0,__FUNCTION__);
            if(ptr){
                ptr->init(__FUNCTION__);
            }
            else{
                CAM_FUNC_ERR("NULL ptr\n");
                return MFALSE;
            }
            mrg_r14.Raw = CAM_READ_REG_INR(ptr->getPhyObj(),MRG_R14_MRG_CROP);
            this->m_InPut.w += (mrg_r14.Bits.MRG_END_X - mrg_r14.Bits.MRG_STR_X + 1);
            CAM_FUNC_WRN("add mrg_r14 width\n");

            ptr->uninit(__FUNCTION__);
            ptr->destroyInstance();
        }
    }

    CAM_FUNC_WRN("dma_%s: input size:%d_%d\n",this->DMA_ID(),this->m_InPut.w,this->m_InPut.h);
    return rst;
}

MBOOL DYUV_PIPECHK::CHECK_START(void)
{
    MBOOL rst = MTRUE;
    YUVO_PIPECHK yuvo(this->m_pDrv);
    CRZO_R1_PIPECHK crzo(this->m_pDrv);
    CRZO_R2_PIPECHK crzo_r2(this->m_pDrv);
    E_YUV_STATE s_yuvo,s_crzo,s_crzo_r2;

    s_yuvo = yuvo.ENABLE_CHECK();
    s_crzo = crzo.ENABLE_CHECK();
    s_crzo_r2 = crzo_r2.ENABLE_CHECK();

    if((s_yuvo == E_BYPASS) && (s_crzo == E_BYPASS) && (s_crzo_r2 == E_BYPASS)){
        CAM_FUNC_WRN("direct YUV is bypass\n");
        return MTRUE;
    }
    else if((s_yuvo == E_FAIL) || (s_crzo == E_FAIL) || (s_crzo_r2 == E_FAIL)){
        rst = MFALSE;
    }

    //
    if(this->m_hwModule == CAM_C){
        if((s_yuvo != E_BYPASS) && (s_crzo != E_BYPASS) && (s_crzo_r2 != E_BYPASS)){
            CAM_FUNC_ERR("CAM_C have no direct YUV hw path\n");
            return MFALSE;
        }
        else
            return MTRUE;
    }

    //input size
    if(this->InPutSize(s_yuvo) == MFALSE){
        return MFALSE;
    }
    else{
        yuvo.m_InPut = crzo.m_InPut = crzo_r2.m_InPut = this->m_InPut;
    }
#if 0
    //if twin mode , only fmt chk, no size chk
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN)){

    }
    else{
        //YUVO
        if(s_yuvo != E_BYPASS){
            DYUV_PIPELINE(yuvo);
        }
        //CRZO
        if(s_crzo != E_BYPASS){
            DYUV_PIPELINE(crzo);
        }
        //CRZO_R2
        if(s_crzo_r2 != E_BYPASS){
            DYUV_PIPELINE(crzo_r2);
        }
    }
#else
    //YUVO
    if(s_yuvo == E_PASS){
        CAM_FUNC_WRN("start YUVO check\n");
        DYUV_PIPELINE(yuvo);
    }
    //CRZO
    if(s_crzo == E_PASS){
        CAM_FUNC_WRN("start CRZO check\n");
        DYUV_PIPELINE(crzo);
    }
    //CRZO_R2
    if(s_crzo_r2 == E_PASS){
        CAM_FUNC_WRN("start CRZO_R2 check\n");
        DYUV_PIPELINE(crzo_r2);
    }
#endif
    return rst;
}

MBOOL DYUV_PIPECHK::DM_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_DM_R1_EN) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("DM is disabled , IQ issue\n");
        goto EXIT;
    }

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),DM_R1_DM_TILE_EDGE,DM_TILE_EDGE) != 0xF){
        rst = MFALSE;
        CAM_FUNC_ERR("tile edge err\n");
    }


    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),DM_R1_DM_SL_CTL,DM_SL_EN)){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_SLK_R1_EN) == 0){
            rst = MFALSE;
            CAM_FUNC_ERR("slk_r1 must be enabeld when dm_sl_en=1\n");
        }
        else{
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_DM_R1_SEL)== 0){
                CAM_FUNC_WRN("slk_en with DM_SEL_0 ???(IQ issue)\n");
            }
            this->SLK_CHECK(1);
        }
    }

    if( (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),DM_R1_DM_DSB,DM_FL_MODE)) != \
        (CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_DM_R1_SEL))){
        rst = MFALSE;
        CAM_FUNC_ERR("FL_MODE = 1 if DM_SEL =1 , FL_MODE = 0 if DM_SLE = 0\n");
    }

EXIT:
    return rst;
}

MBOOL DYUV_PIPECHK::YNRS_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_YNRS_R1_EN)){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),YNRS_R1_YNRS_CON1,YNRS_SL2_LINK)){
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_SLK_R2_EN) == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("slk_r2 must be enabeld when YNRS_SL2_LINK=1\n");
            }
            else{
                if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,CAMCTL_DM_R1_SEL)== 0){
                    CAM_FUNC_WRN("slk_en with DM_SEL_0 ???(IQ issue)\n");
                }
                this->SLK_CHECK(2);
            }
        }
    }

    return rst;
}


MBOOL DYUV_PIPECHK::SLK_CHECK(MUINT32 idx)
{
    #define SLK_reg(name) {\
        REG_SLK_##name##_SLK_SIZE size;\
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),SLK_##name##_SLK_XOFF,SLK_X_OFST) != 0){\
            CAM_FUNC_WRN("ofset must be 0, (IQ issue)\n");\
        }\
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),SLK_##name##_SLK_YOFF,SLK_Y_OFST) != 0){\
            CAM_FUNC_WRN("ofset must be 0, (IQ issue)\n");\
        }\
        size.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),SLK_##name##_SLK_SIZE);\
        if(size.Bits.SLK_TPIPE_WD != this->m_InPut.w){\
            CAM_FUNC_WRN("width mismatch, (IQ issue)\n");\
        }\
        if(size.Bits.SLK_TPIPE_HT != this->m_InPut.h){\
            CAM_FUNC_WRN("height mismatch, (IQ issue)\n");\
        }\
    }

    MBOOL rst = MTRUE;

    switch(idx){
        case 1: SLK_reg(R1);
            break;
        case 2: SLK_reg(R2);
            break;
        default:
            rst = MFALSE;
            CAM_FUNC_ERR(" no this SLK:%d\n",idx);
            break;
    }
    return rst;
}

MBOOL DYUV_PIPECHK::G2C_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_G2C_R1_EN) != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("RGB->YUV need to be enabled, IQ error\n");
    }
    return rst;
}

MBOOL DYUV_PIPECHK::C42_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_C42_R1_C42_CON con;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_C42_R1_EN) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("c42 must be enabled\n");
    }

    con.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),C42_R1_C42_CON);

    if((con.Bits.C42_FILT_DIS != 0x1) || (con.Bits.C42_TDR_EDGE != 0xF)){
        rst = MFALSE;
        CAM_FUNC_ERR("c42 error\n");
    }

    return rst;
}

MBOOL DYUV_PIPECHK::CCM_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_CCM_R1_EN) == 1){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CCM_R1_CCM_CTRL,CCM_IN_FMT) != 0){
            rst = MFALSE;
            CAM_FUNC_ERR("support only RGB format, IQ error\n");
        }
    }

    return rst;
}

MBOOL DYUV_PIPECHK::GGM_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,CAMCTL_GGM_R1_EN) == 1){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),GGM_R1_GGM_CTRL,GGM_LNR) != 0){
            CAM_FUNC_WRN("GGM with linear gamma?????\n");
        }
    }

    return rst;
}

YUVO_PIPECHK::YUVO_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL, start NE\n");
    }

    m_pDrv = obj;
    m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
}

MBOOL YUVO_PIPECHK::CRP_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CRP_R4_CRP_X_POS x;
    REG_CRP_R4_CRP_Y_POS y;
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_CRP_R4_EN)){
        x.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRP_R4_CRP_X_POS);
        y.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRP_R4_CRP_Y_POS);

        if(this->m_InPut.w != (x.Bits.CRP_XEND - x.Bits.CRP_XSTART + 1)){
            rst =MFALSE;
            CAM_FUNC_ERR("size mismatch %d_%d\n",this->m_InPut.w,(x.Bits.CRP_XEND - x.Bits.CRP_XSTART + 1));
        }

        if(this->m_InPut.h != (y.Bits.CRP_YEND - y.Bits.CRP_YSTART + 1)){
            rst =MFALSE;
            CAM_FUNC_ERR("size mismatch %d_%d\n",this->m_InPut.h,(y.Bits.CRP_YEND - y.Bits.CRP_YSTART + 1));
        }
    }

    return rst;
}

MBOOL YUVO_PIPECHK::CRSP_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CRSP_R1_CRSP_CTRL ctrl;
    REG_CRSP_R1_CRSP_OUT_IMG out_img;
    REG_CRSP_R1_CRSP_STEP_OFST ofst;
    REG_CRSP_R1_CRSP_CROP_X crop_x;
    REG_CRSP_R1_CRSP_CROP_Y crop_y;

    switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_YUVO_R1_FMT)){
        case YUVO_FMT_YUV422_3P:
        case YUVO_FMT_YUV422_2P:
        case YUVO_FMT_YUV422_1P:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_CRSP_R1_EN) == 1){
                rst = MFALSE;
                CAM_FUNC_ERR("crsp should not be enabled with 422fmt\n");
            }
            else
                return MTRUE;
            break;
        case YUVO_FMT_YUV420_2P:
        case YUVO_FMT_YUV420_3P:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_CRSP_R1_EN) == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("crsp should be enabled with 420fmt\n");
            }
            break;
        default:
            CAM_FUNC_ERR("fmt err_%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_YUVO_R1_FMT));
            break;
    }

    ctrl.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R1_CRSP_CTRL);
    out_img.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R1_CRSP_OUT_IMG);
    ofst.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R1_CRSP_STEP_OFST);
    crop_x.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R1_CRSP_CROP_X);
    crop_y.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R1_CRSP_CROP_Y);

    if((ctrl.Bits.CRSP_HORI_EN != 0) || (ctrl.Bits.CRSP_VERT_EN != 1) || (ctrl.Bits.CRSP_CROP_EN != 1)){
        rst = MFALSE;
        CAM_FUNC_ERR("crsp func mismatch(0x%x), may be crash\n",ctrl.Raw);
    }

    if((ofst.Bits.CRSP_STEP_X != 4) || (ofst.Bits.CRSP_STEP_Y != 4) || (ofst.Bits.CRSP_OFST_X != 0) || (ofst.Bits.CRSP_OFST_Y != 1)){
        rst = MFALSE;
        CAM_FUNC_ERR("crsp ofst mismatch(0x%x), may be crash\n",ofst.Raw);
    }

    if(this->m_InPut.w != out_img.Bits.CRSP_WD){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch_(%d_%d)\n",this->m_InPut.w,out_img.Bits.CRSP_WD);
    }

    if(this->m_InPut.h != out_img.Bits.CRSP_HT){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch_(%d_%d)\n",this->m_InPut.h,out_img.Bits.CRSP_HT);
    }

    if( (crop_x.Bits.CRSP_CROP_XSTART != 0) || (crop_y.Bits.CRSP_CROP_YSTART != 0) ){
        rst = MFALSE;
        CAM_FUNC_ERR("crop mismatch_(%d_%d)\n",crop_x.Bits.CRSP_CROP_XSTART,crop_y.Bits.CRSP_CROP_YSTART);
    }

    if( (crop_x.Bits.CRSP_CROP_XEND != (this->m_InPut.w-1)) || (crop_y.Bits.CRSP_CROP_YEND != (this->m_InPut.h -1 )) ){
        rst = MFALSE;
        CAM_FUNC_ERR("crop mismatch_(%d_%d),(%d_%d)\n",crop_x.Bits.CRSP_CROP_XEND,crop_y.Bits.CRSP_CROP_YEND,(this->m_InPut.w-1),(this->m_InPut.h -1 ));
    }

    return rst;
}

DYUV_PIPECHK::E_YUV_STATE YUVO_PIPECHK::ENABLE_CHECK(void)
{
    E_YUV_STATE state = E_PASS;
    REG_CAMCTL_R1_CAMCTL_DMA_EN dma_en;
    REG_CAMCTL_R1_CAMCTL_DMA2_EN dma_en2;
    REG_CAMCTL_R1_CAMCTL_FMT2_SEL fmt_sel2;

    dma_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN);
    dma_en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA2_EN);
    fmt_sel2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT2_SEL);

    //YUVO
    if(dma_en2.Bits.CAMCTL_YUVO_R1_EN){
        if((dma_en.Bits.CAMCTL_YUVBO_R1_EN == 0) && (dma_en.Bits.CAMCTL_YUVCO_R1_EN == 1)){
            state = E_FAIL;
            CAM_FUNC_ERR("can't enable YUVCO without BO\n");
        }
        else if((dma_en.Bits.CAMCTL_YUVBO_R1_EN == 1) && (dma_en.Bits.CAMCTL_YUVCO_R1_EN == 0)){
            switch(fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT){
                case YUVO_FMT_YUV422_2P:
                case YUVO_FMT_YUV420_2P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT);
                    state = E_FAIL;
                    break;
            }
        }
        else if((dma_en.Bits.CAMCTL_YUVBO_R1_EN == 1) && (dma_en.Bits.CAMCTL_YUVCO_R1_EN == 1)){
            switch(fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT){
                case YUVO_FMT_YUV422_3P:
                case YUVO_FMT_YUV420_3P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT);
                    state = E_FAIL;
                    break;
            }
        }
        else {
            switch(fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT){
                case YUVO_FMT_YUV422_1P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT);
                    state = E_FAIL;
                    break;
            }
        }
    }
    else{
        if(dma_en.Bits.CAMCTL_YUVCO_R1_EN | dma_en.Bits.CAMCTL_YUVBO_R1_EN){
            state = E_FAIL;
            CAM_FUNC_ERR("can't enable YUVBO/CO without VO\n");
        }
        else
            state = E_BYPASS;
    }

    return state;
}

#define DMA_reg(name,name2,idx,height) {\
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),name##_##name2##_##name##_STRIDE,name##_STRIDE) < result.stride_byte[idx]){\
        rst = MFALSE;\
        CAM_FUNC_ERR("stride too small(%d_%d)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),name##_##name2##_##name##_STRIDE,name##_STRIDE),result.stride_byte[idx]);\
    }\
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),name##_##name2##_##name##_XSIZE,name##_XSIZE) != (result.stride_byte[idx]-1)){\
        rst = MFALSE;\
        CAM_FUNC_ERR("xsize err(%d_%d)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),name##_##name2##_##name##_XSIZE,name##_XSIZE),result.stride_byte[idx]);\
    }\
    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),name##_##name2##_##name##_YSIZE,name##_YSIZE) != (height-1)){\
        rst = MFALSE;\
        CAM_FUNC_ERR("ysize err(%d_%d)\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),name##_##name2##_##name##_XSIZE,name##_XSIZE),height);\
    }\
}

#define PAK_reg(name,imgfmt) {\
    REG_PAK_##name##_PAK_CONT pak;\
    REG_CAMCTL_R1_CAMCTL_EN3 en3;\
    pak.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),PAK_##name##_PAK_CONT);\
    en3.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3);\
    if(en3.Bits.CAMCTL_PAK_##name##_EN != 1){\
        rst = MFALSE;\
        CAM_FUNC_ERR("pak must be enable\n");\
    }\
    if(pak.Bits.PAK_UV_SIGN != 0){\
        rst =MFALSE;\
        CAM_FUNC_ERR("UV sign must be 0(IQ issue)\n");\
    }\
    switch(imgfmt){\
        case NSCam::eImgFmt_YUY2:\
        case NSCam::eImgFmt_YVYU:\
        case NSCam::eImgFmt_UYVY:\
        case NSCam::eImgFmt_VYUY:\
        case NSCam::eImgFmt_NV16:\
        case NSCam::eImgFmt_NV61:\
        case NSCam::eImgFmt_NV21:\
        case NSCam::eImgFmt_NV12:\
        case NSCam::eImgFmt_I422:\
        case NSCam::eImgFmt_I420:\
            if((pak.Bits.PAK_YUV_BIT!= 0) || (pak.Bits.PAK_YUV_DNG!= 0)){\
                rst = MFALSE;\
                CAM_FUNC_ERR("bit err(0x%x)",pak.Raw);\
            }\
        break;\
        case NSCam::eImgFmt_MTK_YUYV_Y210:\
        case NSCam::eImgFmt_MTK_YVYU_Y210:\
        case NSCam::eImgFmt_MTK_UYVY_Y210:\
        case NSCam::eImgFmt_MTK_VYUY_Y210:\
        case NSCam::eImgFmt_MTK_YUV_P210:\
        case NSCam::eImgFmt_MTK_YVU_P210:\
        case NSCam::eImgFmt_MTK_YUV_P010:\
        case NSCam::eImgFmt_MTK_YVU_P010:\
        case NSCam::eImgFmt_MTK_YUV_P210_3PLANE:\
        case NSCam::eImgFmt_MTK_YUV_P010_3PLANE:\
            if((pak.Bits.PAK_YUV_BIT!= 1) || (pak.Bits.PAK_YUV_DNG!= 0)){\
                rst = MFALSE;\
                CAM_FUNC_ERR("bit err(0x%x)",pak.Raw);\
            }\
            break;\
        case NSCam::eImgFmt_YUYV_Y210:\
        case NSCam::eImgFmt_YVYU_Y210:\
        case NSCam::eImgFmt_UYVY_Y210:\
        case NSCam::eImgFmt_VYUY_Y210:\
        case NSCam::eImgFmt_YUV_P210:\
        case NSCam::eImgFmt_YVU_P210:\
        case NSCam::eImgFmt_YUV_P010:\
        case NSCam::eImgFmt_YVU_P010:\
        case NSCam::eImgFmt_YUV_P210_3PLANE:\
        case NSCam::eImgFmt_YUV_P010_3PLANE:\
            if((pak.Bits.PAK_YUV_BIT!= 1) || (pak.Bits.PAK_YUV_DNG!= 1)){\
                rst = MFALSE;\
                CAM_FUNC_ERR("bit err(0x%x)",pak.Raw);\
            }\
            break;\
    }\
}

MBOOL YUVO_PIPECHK::DMAO_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_DMA_EN dma_en;
    capibility cap(this->m_hwModule);
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo inputInfo;
    tCAM_rst result;

    dma_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN);

    inputInfo.width = this->m_InPut.w;
    inputInfo.pixelMode = ePixMode_1;   //always 1 pix mode only
    inputInfo.format = (NSCam::EImageFormat)CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),YUVO_R1_YUVO_FH_SPARE_5);//(NSCam::EImageFormat)header_yuv.GetRegInfo(Header_YUVO::E_SW_FMT,this->m_pDrv);

    cap.GetCapibility(NSImageio::NSIspio::EPortIndex_YUVO,\
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,\
        inputInfo,result,E_CAM_UNKNOWNN);

    CAM_FUNC_WRN("%s imageformat:0x%x\n",this->DMA_ID(),inputInfo.format);

    //YUVO
    DMA_reg(YUVO,R1,0,this->m_InPut.h);
    PAK_reg(R3,inputInfo.format);
    //YUVBO
    if(dma_en.Bits.CAMCTL_YUVBO_R1_EN){
        DMA_reg(YUVBO,R1,1,this->m_InPut.h);
        PAK_reg(R4,inputInfo.format);
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_PLNW_R1_EN)!= 1){
            rst = MFALSE;
            CAM_FUNC_ERR("plnw must be enabled\n");
        }
    }
    //YUVCO
    if(dma_en.Bits.CAMCTL_YUVCO_R1_EN){
        DMA_reg(YUVCO,R1,2,this->m_InPut.h);
        PAK_reg(R5,inputInfo.format);
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_PLNW_R1_EN)!= 1){
            rst = MFALSE;
            CAM_FUNC_ERR("plnw must be enabled\n");
        }
    }

    return rst;
}

MBOOL CRZO_R1_PIPECHK::G2C_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_G2C_R2_EN) != 1){
        rst = MFALSE;
        CAM_FUNC_ERR("RGB->YUV need to be enabled, IQ error\n");
    }
    return rst;
}

CRZO_R1_PIPECHK::CRZO_R1_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL, start NE\n");
    }

    m_pDrv = obj;
    m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
}

DYUV_PIPECHK::E_YUV_STATE CRZO_R1_PIPECHK::ENABLE_CHECK(void)
{
    E_YUV_STATE state = E_PASS;
    REG_CAMCTL_R1_CAMCTL_DMA_EN dma_en;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;

    dma_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN);
    fmt_sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL);

    //CRZO_R1
    if(dma_en.Bits.CAMCTL_CRZO_R1_EN){
        if(dma_en.Bits.CAMCTL_CRZBO_R1_EN == 1){
            switch(fmt_sel.Bits.CAMCTL_CRZO_R1_FMT){
                case CRZO_FMT_YUV422_2P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel.Bits.CAMCTL_CRZO_R1_FMT);
                    state = E_FAIL;
                    break;
            }
        }
        else {
            switch(fmt_sel.Bits.CAMCTL_CRZO_R1_FMT){
                case CRZO_FMT_YUV422_1P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel.Bits.CAMCTL_CRZO_R1_FMT);
                    state = E_FAIL;
                    break;
            }
        }
    }
    else{
        if(dma_en.Bits.CAMCTL_CRZBO_R1_EN){
            state = E_FAIL;
            CAM_FUNC_ERR("can't enable CRZBO_R1 without CRZO_R1\n");
        }
        else
            state = E_BYPASS;
    }

    return state;
}

MBOOL CRZO_R1_PIPECHK::CRZ_CHECK(void)
{
    MBOOL rst = MTRUE;

    REG_CRZ_R1_CRZ_IN_IMG in;
    REG_CRZ_R1_CRZ_OUT_IMG out;

    in.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R1_CRZ_IN_IMG);
    out.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R1_CRZ_OUT_IMG);

    if(in.Bits.CRZ_IN_WD != this->m_InPut.w){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch(%d_%d)\n",in.Bits.CRZ_IN_WD,this->m_InPut.w);
    }

    if(in.Bits.CRZ_IN_HT != this->m_InPut.h){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch(%d_%d)\n",in.Bits.CRZ_IN_HT,this->m_InPut.h);
    }

    if(out.Bits.CRZ_OUT_WD > in.Bits.CRZ_IN_WD){
        rst = MFALSE;
        CAM_FUNC_ERR("support no scaling up\n");
    }

    if(out.Bits.CRZ_OUT_HT > in.Bits.CRZ_IN_HT){
        rst = MFALSE;
        CAM_FUNC_ERR("support no scaling up\n");
    }

    //max size is checked by per-frame setting

    return rst;
}


MBOOL CRZO_R1_PIPECHK::GGM_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_GGM_R2_EN) == 1){
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),GGM_R2_GGM_CTRL,GGM_LNR) != 0){
            CAM_FUNC_WRN("GGM with linear gamma?????\n");
        }
    }

    return rst;
}

MBOOL CRZO_R1_PIPECHK::DMAO_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_DMA_EN dma_en;
    capibility cap(this->m_hwModule);
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo inputInfo;
    tCAM_rst result;
    REG_CRZ_R1_CRZ_OUT_IMG out;

    out.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R1_CRZ_OUT_IMG);
    dma_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN);

    if((out.Bits.CRZ_OUT_WD & 0x1) || (out.Bits.CRZ_OUT_HT & 0x1) ){
        rst = MFALSE;
        CAM_FUNC_ERR("cant have odd output size(%d_%d)\n",out.Bits.CRZ_OUT_WD,out.Bits.CRZ_OUT_HT);
    }

    inputInfo.width = out.Bits.CRZ_OUT_WD;
    inputInfo.pixelMode = ePixMode_1;   //always 1 pix mode only
    inputInfo.format = (NSCam::EImageFormat)CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZO_R1_CRZO_FH_SPARE_4);//(NSCam::EImageFormat)header_crz.GetRegInfo(Header_CRZO_R1::E_SW_FMT,this->m_pDrv);


    cap.GetCapibility(NSImageio::NSIspio::EPortIndex_CRZO,\
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,\
        inputInfo,result,E_CAM_UNKNOWNN);

    CAM_FUNC_WRN("%s imageformat:0x%x\n",this->DMA_ID(),inputInfo.format);

    //CRZO
    DMA_reg(CRZO,R1,0,out.Bits.CRZ_OUT_HT);
    //CRZBO
    if(dma_en.Bits.CAMCTL_CRZBO_R1_EN){
        DMA_reg(CRZBO,R1,1,out.Bits.CRZ_OUT_HT);
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_PLNW_R2_EN)!= 1){
            rst = MFALSE;
            CAM_FUNC_ERR("plnw must be enabled\n");
        }
    }

    return rst;
}

MBOOL CRZO_R1_PIPECHK::BS_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_BS_R1_EN) == 0){
        rst =MFALSE;
        CAM_FUNC_ERR("IQ issue , alway get black image\n");
    }

    return rst;
}

CRZO_R2_PIPECHK::CRZO_R2_PIPECHK(ISP_DRV_CAM* obj)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    if(obj == NULL){
        BASE_LOG_ERR("obj can't be NULL, start NE\n");
    }

    m_pDrv = obj;
    m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
}

DYUV_PIPECHK::E_YUV_STATE CRZO_R2_PIPECHK::ENABLE_CHECK(void)
{
    E_YUV_STATE state = E_PASS;
    REG_CAMCTL_R1_CAMCTL_DMA2_EN dma_en2;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;

    dma_en2.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA2_EN);
    fmt_sel.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL);


    //CRZO_R2
    if(dma_en2.Bits.CAMCTL_CRZO_R2_EN){
        if(dma_en2.Bits.CAMCTL_CRZBO_R2_EN == 1){
            switch(fmt_sel.Bits.CAMCTL_CRZO_R2_FMT){
                case CRZO_R2_FMT_YUV422_2P:
                case CRZO_R2_FMT_YUV420_2P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel.Bits.CAMCTL_CRZO_R2_FMT);
                    state = E_FAIL;
                    break;
            }
        }
        else {
            switch(fmt_sel.Bits.CAMCTL_CRZO_R2_FMT){
                case CRZO_R2_FMT_YUV422_1P:
                    break;
                default:
                    CAM_FUNC_ERR("fmt err_%d\n",fmt_sel.Bits.CAMCTL_CRZO_R2_FMT);
                    state = E_FAIL;
                    break;
            }
        }
    }
    else{
        if(dma_en2.Bits.CAMCTL_CRZBO_R2_EN){
            state = E_FAIL;
            CAM_FUNC_ERR("can't enable CRZBO_R2 without CRZO_R2\n");
        }
        else
            state = E_BYPASS;
    }

    return state;
}

MBOOL CRZO_R2_PIPECHK::CRZ_CHECK(void)
{
    MBOOL rst = MTRUE;

    REG_CRZ_R2_CRZ_IN_IMG in;
    REG_CRZ_R2_CRZ_OUT_IMG out;

    in.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R2_CRZ_IN_IMG);
    out.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R2_CRZ_OUT_IMG);

    if(in.Bits.CRZ_IN_WD != this->m_InPut.w){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch(%d_%d)\n",in.Bits.CRZ_IN_WD,this->m_InPut.w);
    }

    if(in.Bits.CRZ_IN_HT != this->m_InPut.h){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch(%d_%d)\n",in.Bits.CRZ_IN_HT,this->m_InPut.h);
    }

    if(out.Bits.CRZ_OUT_WD > in.Bits.CRZ_IN_WD){
        rst = MFALSE;
        CAM_FUNC_ERR("support no scaling up\n");
    }

    if(out.Bits.CRZ_OUT_HT > in.Bits.CRZ_IN_HT){
        rst = MFALSE;
        CAM_FUNC_ERR("support no scaling up\n");
    }

    //max size is checked by per-frame setting

    return rst;
}

MBOOL CRZO_R2_PIPECHK::CRSP_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CRSP_R2_CRSP_CTRL ctrl;
    REG_CRSP_R2_CRSP_OUT_IMG out_img;
    REG_CRSP_R2_CRSP_STEP_OFST ofst;
    REG_CRSP_R2_CRSP_CROP_X crop_x;
    REG_CRSP_R2_CRSP_CROP_Y crop_y;
    REG_CRZ_R2_CRZ_OUT_IMG out;

    switch(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_CRZO_R2_FMT)){
        case CRZO_R2_FMT_YUV422_2P:
        case CRZO_R2_FMT_YUV422_1P:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_CRSP_R2_EN) == 1){
                rst = MFALSE;
                CAM_FUNC_ERR("crsp should not be enabled with 422fmt\n");
            }
            else
                return MTRUE;
            break;
        case CRZO_R2_FMT_YUV420_2P:
            if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_CRSP_R2_EN) == 0){
                rst = MFALSE;
                CAM_FUNC_ERR("crsp should be enabled with 420fmt\n");
            }
            break;
        default:
            CAM_FUNC_ERR("fmt err_%d\n",CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_CRZO_R2_FMT));
            break;
    }

    ctrl.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R2_CRSP_CTRL);
    out_img.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R2_CRSP_OUT_IMG);
    ofst.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R2_CRSP_STEP_OFST);
    crop_x.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R2_CRSP_CROP_X);
    crop_y.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRSP_R2_CRSP_CROP_Y);
    out.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R2_CRZ_OUT_IMG);

    if((ctrl.Bits.CRSP_HORI_EN != 0) || (ctrl.Bits.CRSP_VERT_EN != 1) || (ctrl.Bits.CRSP_CROP_EN != 1)){
        rst = MFALSE;
        CAM_FUNC_ERR("crsp func mismatch(0x%x), may be crash\n",ctrl.Raw);
    }

    if((ofst.Bits.CRSP_STEP_X != 4) || (ofst.Bits.CRSP_STEP_Y != 4) || (ofst.Bits.CRSP_OFST_X != 0) || (ofst.Bits.CRSP_OFST_Y != 1)){
        rst = MFALSE;
        CAM_FUNC_ERR("crsp ofst mismatch(0x%x), may be crash\n",ofst.Raw);
    }

    if(out.Bits.CRZ_OUT_WD != out_img.Bits.CRSP_WD){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch_(%d_%d)\n",out.Bits.CRZ_OUT_WD,out_img.Bits.CRSP_WD);
    }

    if(out.Bits.CRZ_OUT_HT != out_img.Bits.CRSP_HT){
        rst = MFALSE;
        CAM_FUNC_ERR("size mismatch_(%d_%d)\n",out.Bits.CRZ_OUT_HT,out_img.Bits.CRSP_HT);
    }

    if( (crop_x.Bits.CRSP_CROP_XSTART != 0) || (crop_y.Bits.CRSP_CROP_YSTART != 0) ){
        rst = MFALSE;
        CAM_FUNC_ERR("crop mismatch_(%d_%d)\n",crop_x.Bits.CRSP_CROP_XSTART,crop_y.Bits.CRSP_CROP_YSTART);
    }

    if( (crop_x.Bits.CRSP_CROP_XEND != (out.Bits.CRZ_OUT_WD-1)) || (crop_y.Bits.CRSP_CROP_YEND != (out.Bits.CRZ_OUT_HT -1 )) ){
        rst = MFALSE;
        CAM_FUNC_ERR("crop mismatch_(%d_%d),(%d_%d)\n",crop_x.Bits.CRSP_CROP_XEND,crop_y.Bits.CRSP_CROP_YEND,(out.Bits.CRZ_OUT_WD-1),(out.Bits.CRZ_OUT_HT -1 ));
    }

    return rst;
}


MBOOL CRZO_R2_PIPECHK::C42_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_C42_R2_C42_CON con;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_C42_R2_EN) == 0){
        rst = MFALSE;
        CAM_FUNC_ERR("c42 must be enabled\n");
    }

    con.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),C42_R2_C42_CON);

    if((con.Bits.C42_FILT_DIS != 0x1) || (con.Bits.C42_TDR_EDGE != 0xF)){
        rst = MFALSE;
        CAM_FUNC_ERR("c42 error\n");
    }

    return rst;
}

MBOOL CRZO_R2_PIPECHK::DMAO_CHECK(void)
{
    MBOOL rst = MTRUE;
    REG_CAMCTL_R1_CAMCTL_DMA2_EN dma_en;
    capibility cap(this->m_hwModule);
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo inputInfo;
    tCAM_rst result;
    REG_CRZ_R2_CRZ_OUT_IMG out;

    out.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZ_R2_CRZ_OUT_IMG);
    dma_en.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA2_EN);

    if((out.Bits.CRZ_OUT_WD & 0x1) || (out.Bits.CRZ_OUT_HT & 0x1) ){
        rst = MFALSE;
        CAM_FUNC_ERR("cant have odd output size(%d_%d)\n",out.Bits.CRZ_OUT_WD,out.Bits.CRZ_OUT_HT);
    }

    inputInfo.width = out.Bits.CRZ_OUT_WD;
    inputInfo.pixelMode = ePixMode_1;   //always 1 pix mode only
    inputInfo.format = (NSCam::EImageFormat)CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CRZO_R2_CRZO_FH_SPARE_4);//(NSCam::EImageFormat)header_crz.GetRegInfo(Header_CRZO_R2::E_SW_FMT,this->m_pDrv);


    cap.GetCapibility(NSImageio::NSIspio::EPortIndex_CRZO_R2,\
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,\
        inputInfo,result,E_CAM_UNKNOWNN);

    CAM_FUNC_WRN("%s imageformat:0x%x\n",this->DMA_ID(),inputInfo.format);

    //CRZO
    DMA_reg(CRZO,R2,0,out.Bits.CRZ_OUT_HT);
    //CRZBO
    if(dma_en.Bits.CAMCTL_CRZBO_R2_EN){
        DMA_reg(CRZBO,R2,1,out.Bits.CRZ_OUT_HT);
        if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,CAMCTL_PLNW_R3_EN)!= 1){
            rst = MFALSE;
            CAM_FUNC_ERR("plnw must be enabled\n");
        }
    }

    return rst;
}

MBOOL CRZO_R2_PIPECHK::BS_CHECK(void)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_BITS_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,CAMCTL_BS_R2_EN) == 0){
        rst =MFALSE;
        CAM_FUNC_ERR("IQ issue , alway get black image\n");
    }

    return rst;
}
