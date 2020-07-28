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
#include <ispio_pipe_ports.h>
#include "isp_function_cam.h"
#include "cam_capibility.h"
#include <string>
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


/**
    check whether this fail is caused by some dmao waiting for enque or not
    no rsso check
*/
MBOOL PIPE_CHECK::ENQUE_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 imgo_en,rrzo_en,eiso_en,lcso_en,rsso_en,ufeo_en;
    MUINT32 imgo_fbc,rrzo_fbc,eiso_fbc,lcso_fbc,rsso_fbc,ufeo_fbc;

    imgo_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,IMGO_EN);
    rrzo_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,RRZO_EN);
    ufeo_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFEO_EN);
    lcso_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LCSO_EN);
    if(this->m_pUniDrv){
        eiso_en = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,EISO_A_EN);
        rsso_en = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,RSSO_A_EN);
    }
    else{
        eiso_en = rsso_en = 0;
    }

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


    return rst;
}

#define _DMASTR(_str,dma_enum)({\
    char _tmp[16];\
    switch(dma_enum){\
    case _imgo_: _str.append("imgo"); break;\
    case _rrzo_: _str.append("rrzo"); break;\
    case _ufeo_: _str.append("ufeo"); break;\
    case _aao_:  _str.append("aao"); break;\
    case _afo_:  _str.append("afo"); break;\
    case _lcso_: _str.append("lcso"); break;\
    case _pdo_:  _str.append("pdo"); break;\
    case _eiso_: _str.append("eiso"); break;\
    case _flko_: _str.append("flko"); break;\
    case _rsso_: _str.append("rsso"); break;\
    case _pso_:  _str.append("pso"); break;\
    case _bpci_: _str.append("bpci"); break;\
    case _lsci_: _str.append("lsci"); break;\
    case _rawi_: _str.append("rawi"); break;\
    case _pdi_:  _str.append("pdi"); break;\
    default:\
    snprintf(_tmp, sizeof(_tmp), "unknown_%d", dma_enum);\
    _str.append(_tmp);\
    break;\
    }\
})

MBOOL PIPE_CHECK::FIFO_FULL_STATUS()
{
    string _str;
    MUINT32 _dbgsel = 0, _dbgval = 0;
    MBOOL ret = MTRUE;

    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_CTL_DBG_SET, 0xF0);

    for (MUINT32 dma=0; dma < (MUINT32)_cam_max_; dma++) {
        MUINT32 dbg_value = 0;

        switch(dma){
        case _imgo_: _dbgsel = 0x303; break;
        case _rrzo_: _dbgsel = 0x304; break;
        case _ufeo_: _dbgsel = 0x308; break;
        case _aao_:  _dbgsel = 0x305; break;
        case _afo_:  _dbgsel = 0x306; break;
        case _lcso_: _dbgsel = 0x307; break;
        case _pdo_:  _dbgsel = 0x309; break;
        case _eiso_: _dbgsel = 0x30F; break;
        case _flko_: _dbgsel = 0x310; break;
        case _rsso_: _dbgsel = 0x311; break;
        case _pso_:  _dbgsel = 0x20D; break;
        case _bpci_: _dbgsel = 0x200; break;
        case _lsci_: _dbgsel = 0x201; break;
        case _rawi_: _dbgsel = 0x202; break;
        case _pdi_:  _dbgsel = 0x20E; break;
        default:
            CAM_FUNC_ERR("unsupport dmao(%d)", dma);
            ret = MFALSE;
            break;
        }

        if (_dbgsel) {
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_DMA_DEBUG_SEL, _dbgsel);
            _dbgval = CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_CTL_DBG_PORT);

            if (_dbgval) {
                char _tmp[32];
                snprintf(_tmp, sizeof(_tmp), "_0x%x ", _dbgval);
                _DMASTR(_str,dma);
                _str.append(_tmp);
            }
        }
    }

    CAM_FUNC_INF("FIFO_FULL status: %s",_str.c_str());

    return ret;
}

MBOOL PIPE_CHECK::DMAO_STATUS(void)
{
    string _str;
    MUINT32 dma_err[_cam_max_];
    struct ISP_GET_CLK_INFO getClkInfo;

    this->m_pDrv->getDeviceInfo(_GET_DMA_ERR,(MUINT8 *)dma_err);

    for(MUINT32 i=0;i<_cam_max_;i++){
        if(dma_err[i] & 0xffff){
            char _tmp[32];
            snprintf(_tmp, sizeof(_tmp), "_0x%x ", dma_err[i]);
            _DMASTR(_str,i);
            _str.append(_tmp);
        }
    }

    if(this->m_pUniDrv){
        CAM_FUNC_ERR("[cam dmao:0x%x,uni dmao0x%x] ERR:%s\n",\
        CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN),\
        UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN),\
        _str.c_str());
    }
    else{
        CAM_FUNC_ERR("[cam dmao:0x%x] ERR:%s\n",\
        CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN),\
        _str.c_str());
    }
    if ((this->m_pDrv->getDeviceInfo(_GET_CUR_ISP_CLOCK, (MUINT8*)&getClkInfo)) == MFALSE) {
        CAM_FUNC_ERR("Error in _GET_CUR_ISP_CLOCK");
    } else if (getClkInfo.curClk < getClkInfo.targetClk)
        CAM_FUNC_ERR("Current clock(%d) is smaller than target clock(%d)", getClkInfo.curClk, getClkInfo.targetClk);

    return MTRUE;
}

MBOOL PIPE_CHECK::UFE_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 ufeg_sel;

    ufeg_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,UFEG_SEL);
    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,UFEG_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,UFEO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable UFEO without enable UFE\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }
    EXIT:
        if(rst == MFALSE){
            CAM_FUNC_ERR("UFE check fail:cur mux:0x%x\n",ufeg_sel);
        }
        return rst;

}

MBOOL PIPE_CHECK::AA_CHECK(void)
{
    MUINT32 ae_hst_en,aa_in_h,aa_in_v,aa_start_x,aa_start_y,_cnt=0;
    MUINT32 aa_isize_h,aa_isize_v;
    MUINT32 aa_win_h=0,aa_win_v=0;
    MUINT32 aa_win_size_h,aa_win_size_v;
    MUINT32 aa_xsize = 0,_tmp;
    MUINT32 qbn_pix=0;
    MBOOL   rst = MTRUE;
    MBOOL hdr_st_en,ae_oe_en,tsf_st_en;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

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

    //ae additional func.

    hdr_st_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_STAT_EN,AE_HDR_STAT_EN);
    ae_oe_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_STAT_EN,AE_OVERCNT_EN);
    tsf_st_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AE_STAT_EN,AE_TSF_STAT_EN);

    //win number
    aa_win_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_NUM,AWB_W_HNUM);
    aa_win_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_NUM,AWB_W_VNUM);
    if((aa_win_h < 1) || (aa_win_h > 128) || (aa_win_v < 1) || (aa_win_v > 128)){
        CAM_FUNC_ERR("win number must be 1<=win_num<=128 (%d_%d)",aa_win_h,aa_win_v);
        rst = MFALSE;
        goto EXIT;
    }

    //aa start coordinate
    aa_start_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_ORG,AWB_W_HORG);
    aa_start_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_ORG,AWB_W_VORG);
    //aa input size , form bmx output
#if 0//bmxo should be the same as TG crop. do not just use BMX output size , it will be re-org by twin driver.
    aa_in_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_VSIZE,BMX_HT);
    aa_in_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_CROP,BMX_END_X) - \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_CROP,BMX_STR_X) + 1;
#else
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    aa_in_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    aa_in_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    aa_in_h = aa_in_h >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));

    aa_in_v = aa_in_v >> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);

#endif
    //if QBIN1 enable , hsize /2
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,QBIN1_EN)){
        //pix mode
        if((qbn_pix = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE,QBN_ACC)))
            aa_in_h = aa_in_h >> qbn_pix;
    }

    //aa statistic operation input size
    aa_isize_h = aa_in_h - aa_start_x;
    aa_isize_v = aa_in_v - aa_start_y;
    //aa win/pit size, win size <= pit size
    aa_win_size_h = (aa_isize_h / aa_win_h) /2 * 2;
    aa_win_size_v = (aa_isize_v / aa_win_v) /2 * 2;

    //h-win check
    if(aa_win_size_h < 4){
        CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",aa_win_size_h);
        rst = MFALSE;
    }
    else{

        _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_PIT,AWB_W_HPIT);
        if(_tmp < aa_win_size_h){

            CAM_FUNC_ERR("hpit size need >= ideal size, [0x%x_0x%x]\n",\
                _tmp,aa_win_size_h);
            rst = MFALSE;
        }
        else if(_tmp < 4){

            CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",_tmp);
            rst = MFALSE;
        }
        else if( (_tmp < CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE)) \
            || (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE) & 0x1)){

            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE),\
                _tmp);
            rst = MFALSE;
        }

    }
    //v-win check
    if(aa_win_size_v < 2){
        CAM_FUNC_ERR("ideal v_win size:0x%x[v-min is 2] , underflow!!\n",aa_win_size_v);
        rst = MFALSE;
    }else{

        _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_PIT,AWB_W_VPIT);
        if(_tmp < aa_win_size_v){

            CAM_FUNC_ERR("vpit size need >= ideal size, [0x%x_0x%x]\n",\
                _tmp,aa_win_size_v);
            rst = MFALSE;
        }
        else if(_tmp < 2){

            CAM_FUNC_ERR("ideal h_win size:0x%x[v-min is 2] , underflow!!\n",_tmp);
            rst = MFALSE;
        }
        else if( (_tmp < CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE)) \
            || (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE) & 0x1)){

            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE),\
                _tmp);
            rst = MFALSE;
        }

    }

    //pixel cnt check
    //lumi of accumulation will be affected if pix_cnt bias too much
    _tmp = (1<<24) / \
        (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_HSIZE) * \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_WIN_SIZE,AWB_W_VSIZE)) \
        *2 ;
    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_PIXEL_CNT0,AWB_PIXEL_CNT0);
    CAM_FUNC_INF("awb pix cnt0 cur_tar:0x%x_0x%x\n",_cnt,(_tmp<<1));

    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_PIXEL_CNT1,AWB_PIXEL_CNT1);
    CAM_FUNC_INF("awb pix cnt1 cur_tar:0x%x_0x%x\n",_cnt,_tmp);

    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AWB_PIXEL_CNT2,AWB_PIXEL_CNT2);
    CAM_FUNC_INF("awb pix cnt2 cur_tar:0x%x_0x%x\n",_cnt,(_tmp<<1));


    aa_xsize += (aa_win_h*aa_win_v*4);//awb , 4 bytes each win
    aa_xsize += (aa_win_h*aa_win_v);// ae , 1 byte each win
    //
    _cnt=0;
    if(hdr_st_en){
        aa_xsize += (aa_win_h*aa_win_v*4/8);    // 4 bits each win
    }
    if(ae_oe_en){
        aa_xsize += (aa_win_h*aa_win_v); // 1 byte each win
    }
    if(tsf_st_en){
        aa_xsize += (aa_win_h*aa_win_v*2); // 2 bytes each win
    }

    //ae hist
    _cnt = 4;//ae have 4 hist
    _tmp = ae_hst_en = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_AE_PIX_HST_CTL);
    while(_cnt--){
        if(_tmp & 0x1){
            aa_xsize += (192*2);
        }
        _tmp=_tmp >> 1;
    }

    if(aa_xsize != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_XSIZE,XSIZE)+1)){
        CAM_FUNC_ERR("cur xsize :0x%x, valid xsize:0x%x\n",\
            (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_XSIZE,XSIZE)+1),\
            aa_xsize);
        rst = MFALSE;
    }

    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_YSIZE,YSIZE) == 0xFFFF){
        CAM_FUNC_ERR("current ysize:0x%x, user can't set 0 into drv\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_YSIZE,YSIZE));
        rst = MFALSE;
    }

#if 0   //remove , aao only 1 dimension, so stride is useless
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_STRIDE,STRIDE) < aa_xsize){
        CAM_FUNC_ERR("cur stride:0x%x, valid stride:0x%x\n",\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AAO_STRIDE,STRIDE),\
            aa_xsize);
        rst = MFALSE;
    }
#endif
EXIT:
    if(rst == MFALSE){
#if 0
        CAM_FUNC_ERR("hst_en:0x%x,qbn_pix=0x%x,aa_in=0x%x_0x%x,aa start:0x%x_0x%x,aa_op_in:0x%x_0x%x,aa_win_#:0x%x_0x%x,aa_win_size:0x%x_0x%x,aa_size=0x%x,add-func:%d_%d_%d\n",
        ae_hst_en,\
        qbn_pix,\
        aa_in_h,aa_in_v,\
        aa_start_x,aa_start_y,\
        aa_isize_h,aa_isize_v,\
        aa_win_h,aa_win_v,\
        aa_win_size_h,aa_win_size_v,\
        aa_xsize,\
        tsf_st_en,ae_oe_en,hdr_st_en);
#else
        CAM_FUNC_ERR("AE/AWB/AAO check error,cur window size:%d x %d\n!!\n",aa_win_h,aa_win_v);
#endif
    }

    return rst;
}

MBOOL PIPE_CHECK::PS_CHECK(void)
{
    MUINT32 aa_in_h,aa_in_v,aa_start_x,aa_start_y,_cnt=0;
    MUINT32 aa_isize_h,aa_isize_v;
    MUINT32 aa_win_h=0,aa_win_v=0;
    MUINT32 aa_win_size_h,aa_win_size_v;
    MUINT32 aa_xsize = 0,_tmp;
    MUINT32 qbn_pix=0;
    MBOOL   rst = MTRUE;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PS_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PSO_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable PSO without enable PS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    //win number
    aa_win_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_NUM,AWB_W_HNUM);
    aa_win_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_NUM,AWB_W_VNUM);

    //aa start coordinate
    aa_start_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_ORG,AWB_W_HORG);
    aa_start_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_ORG,AWB_W_VORG);
    //aa input size , form bmx output
#if 0//bmxo should be the same as TG crop. do not just use BMX output size , it will be re-org by twin driver.
    aa_in_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_VSIZE,BMX_HT);
    aa_in_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_CROP,BMX_END_X) - \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_CROP,BMX_STR_X) + 1;
#else
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    aa_in_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    aa_in_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    aa_in_h = aa_in_h >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));

    aa_in_v = aa_in_v >> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);

#endif
    //if QBIN4 enable , hsize /2
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,QBIN4_EN)){
        //pix mode
        if((qbn_pix = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN4_MODE,QBN_ACC)))
            aa_in_h = aa_in_h >> qbn_pix;
    }

    //aa statistic operation input size
    aa_isize_h = aa_in_h - aa_start_x;
    aa_isize_v = aa_in_v - aa_start_y;
    //aa win/pit size, win size <= pit size
    aa_win_size_h = (aa_isize_h / aa_win_h) /2 * 2;
    aa_win_size_v = (aa_isize_v / aa_win_v) /2 * 2;

    //h-win check
    if(aa_win_size_h < 4){
        CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",aa_win_size_h);
        rst = MFALSE;
    }
    else{

        _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_PIT,AWB_W_HPIT);
        if(_tmp < aa_win_size_h){

            CAM_FUNC_ERR("hpit size need >= ideal size, [0x%x_0x%x]\n",\
                _tmp,aa_win_size_h);
            rst = MFALSE;
        }
        else if(_tmp < 4){

            CAM_FUNC_ERR("ideal h_win size:0x%x[h-min is 4] , underflow!!\n",_tmp);
            rst = MFALSE;
        }
        else if( (_tmp < CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE)) \
            || (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE) & 0x1)){

            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE),\
                _tmp);
            rst = MFALSE;
        }

    }
    //v-win check
    if(aa_win_size_v < 2){
        CAM_FUNC_ERR("ideal v_win size:0x%x[v-min is 2] , underflow!!\n",aa_win_size_v);
        rst = MFALSE;
    }else{

        _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_PIT,AWB_W_VPIT);
        if(_tmp < aa_win_size_v){

            CAM_FUNC_ERR("vpit size need >= ideal size, [0x%x_0x%x]\n",\
                _tmp,aa_win_size_v);
            rst = MFALSE;
        }
        else if(_tmp < 2){

            CAM_FUNC_ERR("ideal h_win size:0x%x[v-min is 2] , underflow!!\n",_tmp);
            rst = MFALSE;
        }
        else if( (_tmp < CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE)) \
            || (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE) & 0x1)){

            CAM_FUNC_ERR("win size need <= pit size, [0x%x_0x%x]\n",\
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE),\
                _tmp);
            rst = MFALSE;
        }

    }

    //pixel cnt check
    //lumi of accumulation will be affected if pix_cnt bias too much
    _tmp = (1<<24) / \
        (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_HSIZE) * \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_WIN_SIZE,AWB_W_VSIZE)) \
        *2 ;
    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT0,AWB_PIXEL_CNT0);
    CAM_FUNC_INF("awb pix cnt0 cur_tar:0x%x_0x%x\n",_cnt,(_tmp<<1));

    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT1,AWB_PIXEL_CNT1);
    CAM_FUNC_INF("awb pix cnt1 cur_tar:0x%x_0x%x\n",_cnt,_tmp);

    _cnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PS_AWB_PIXEL_CNT2,AWB_PIXEL_CNT2);
    CAM_FUNC_INF("awb pix cnt2 cur_tar:0x%x_0x%x\n",_cnt,(_tmp<<1));


    aa_xsize += (aa_win_h*aa_win_v*4);  //awb , 4 bytes each win
    aa_xsize += (aa_win_h*aa_win_v);    // ae , 1 byte each win
    aa_xsize += (aa_win_h*aa_win_v*4/8);//hdr , 4 bits each win
    aa_xsize += (aa_win_h*aa_win_v*4);  //tsf , 4 bytes each win (R,Gr,B,Gb)

    if(aa_xsize != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_XSIZE,XSIZE)+1)){
        CAM_FUNC_ERR("cur xsize :0x%x, valid xsize:0x%x\n",\
            (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_XSIZE,XSIZE)+1),\
            aa_xsize);
        rst = MFALSE;
    }

    if( CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_YSIZE,YSIZE) == 0xFFFF){
        CAM_FUNC_ERR("current ysize:0x%x, user can't set 0 into drv\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PSO_YSIZE,YSIZE));
        rst = MFALSE;
    }

EXIT:
    if(rst == MFALSE){
#if 0
        CAM_FUNC_ERR("qbn_pix=0x%x,ps_in=0x%x_0x%x,ps start:0x%x_0x%x,ps_op_in:0x%x_0x%x,ps_win_#:0x%x_0x%x,ps_win_size:0x%x_0x%x,ps_size=0x%x\n",
        qbn_pix,\
        aa_in_h,aa_in_v,\
        aa_start_x,aa_start_y,\
        aa_isize_h,aa_isize_v,\
        aa_win_h,aa_win_v,\
        aa_win_size_h,aa_win_size_v,\
        aa_xsize);
#else
        CAM_FUNC_ERR("PS_AE/PS_AWB/PSO check error,cur window size:%d x %d\n!!\n",aa_win_h,aa_win_v);
#endif
    }

    return rst;
}


MBOOL PIPE_CHECK::FLK_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 size_h,size_v,in_size_h,in_size_v;
    MUINT32 x_size;
    MUINT32 flk1_sel, flk2_sel=0;
    MUINT32 win_num_h,win_num_v;
    MUINT32 ofst_x,ofst_y;
    MUINT32 qbn_pix=0;
    MUINT32 tmp;
    ISP_HW_MODULE m_hwModule = CAM_MAX;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,FLK_A_EN) == 0){
        if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,FLKO_A_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable FLKO without enable FLK\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    flk2_sel = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_TOP_PATH_SEL, FLK2_A_SEL);
    this->m_pDrv->getCurObjInfo(&m_hwModule,&cq,&page);

    if(flk2_sel != m_hwModule) {
        CAM_FUNC_INF("FLK2_SEL(0x%x) is not linked to CAM(0x%x)", flk2_sel, m_hwModule);
        goto EXIT;
    }

    flk1_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,FLK1_SEL);
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    in_size_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    in_size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    in_size_h = in_size_h >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));
    in_size_v = in_size_v >> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);

    switch (flk1_sel ) {
    case 0:
        if ((qbn_pix = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE,QBN_ACC))!= 0) {
            in_size_h = in_size_h >> qbn_pix;
        }
        break;
    case 1:
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
    ofst_x = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLK_A_OFST,FLK_OFST_X);
    ofst_y = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLK_A_OFST,FLK_OFST_Y);
    //win num
    win_num_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLK_A_NUM,FLK_NUM_X);
    win_num_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLK_A_NUM,FLK_NUM_Y);
    //win size
    size_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLK_A_SIZE,FLK_SIZE_X);
    size_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLK_A_SIZE,FLK_SIZE_Y);
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
    if((UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_XSIZE,XSIZE) + 1) != x_size){
        rst = MFALSE;
        CAM_FUNC_ERR("xsize mismatch[0x%x_0x%x]\n",(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_XSIZE,XSIZE) + 1),\
            x_size);
    }
    //ysize
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_YSIZE,YSIZE) != 0){
        rst = MFALSE;
        CAM_FUNC_ERR("Ysize need  to be 0[0x%x]\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_YSIZE,YSIZE));
    }
    //stride
    if((UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_XSIZE,XSIZE) + 1) > UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_STRIDE,STRIDE)){
        rst = MFALSE;
        CAM_FUNC_ERR("stride > xsize+1[0x%x_0x%x]\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_STRIDE,STRIDE),\
            (UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FLKO_XSIZE,XSIZE) + 1));
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
        CAM_FUNC_ERR("FLK check fail:cur mux:0x%x\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_PATH_SEL,FLK2_A_SEL));
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
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

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
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    in_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    in_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    /**
        note: if TG_h > 1728*4 under twin mode. QBIN1 will have sacling ratio:4x
    */
    switch (lcs_sel) {
    case 1:
        in_h = in_h>> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));
        in_v = in_v>> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);

        pixMode = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC);
        break;
    case 0:
        pixMode = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1;
        if (pixMode != 0) {
            rst = MFALSE;
            CAM_FUNC_ERR("lcs support no 2/4 pix mode under YUV format\n");
            goto EXIT;
        }
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

    if (win_h*4 > crop_h) {
        CAM_FUNC_ERR("win_h*4(%d) must <= crop_h(%d)", win_h*4, crop_h);
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
    if(scaling_h != ((win_h - 1)* 1048576 / ((crop_h>>1)) )){
        CAM_FUNC_ERR("lcs h-scaling factor error [0x%x_0x%x_0x%x]\n",scaling_h,win_h,crop_h);
        rst = MFALSE;
    }
    if(scaling_v != ((win_v - 1)* 1048576 / ((crop_v>>1)) )){
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

MBOOL PIPE_CHECK::EIS_CHECK(void)
{
    MUINT32 win_h = 4,win_v = 8;
    MUINT32 ofst_x=0,ofst_y=0;
    MUINT32 in_size_h,in_size_v;
    MUINT32 pixMode = 0,hds_sel,bFG = MFALSE, hds2_sel=0;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    Header_RRZO rrzo_fh;
    MUINT32 tmp=0;
    MBOOL rst = MTRUE;
    MUINT32 step_h = 16,step_v = 8,win_size_h,win_size_v,op_h=1,op_v=1;
    TG_W.Raw = TG_H.Raw = 0;
    ISP_HW_MODULE m_hwModule = CAM_MAX;
    E_ISP_CAM_CQ cq;
    MUINT32 page;


    hds2_sel = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_TOP_PATH_SEL, HDS2_A_SEL);
    hds_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,HDS1_SEL);

    this->m_pDrv->getCurObjInfo(&m_hwModule,&cq,&page);
    if(hds2_sel != m_hwModule) {
        CAM_FUNC_INF("HDS2_SEL(0x%x) is not linked to CAM(0x%x)", hds2_sel, m_hwModule);
        goto EXIT;
    }
    //
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,EIS_A_EN) == 0){
        if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,EISO_A_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable EISO without enable EIS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    //max window number
    win_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_NUM_HWIN);
    win_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_NUM_VWIN);
    if( (win_h > 4) || (win_v > 8)){
        CAM_FUNC_ERR("eis win size max:4*8[0x%x_0x%x]\n",win_h,win_v);
        rst = MFALSE;
    }

    //ofst,ofst have min constraint
    ofst_x = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_MB_OFFSET,EIS_RP_HOFST);
    ofst_y = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_MB_OFFSET,EIS_RP_VOFST);
    if((ofst_x < 17) || (ofst_y < 17)){
        CAM_FUNC_ERR("eis ofset must > 16[0x%x_0x%x]\n",ofst_x,ofst_y);
    }
    //floating ofset, current frame ofst from previous frame, normally set 0.
    //this->m_pIspDrv->writeReg(0x355c, ((FL_ofst_x<<16)|FL_ofst_y));

    switch(hds_sel){
        case 0:
            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

            in_size_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            in_size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

            in_size_h = in_size_h>>((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
            break;
        case 1:
            tmp = rrzo_fh.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv->getPhyObj());

            in_size_h = tmp & 0xffff;
            in_size_v = (tmp>>16) & 0xffff;

            in_size_h = in_size_h>>UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_HDS_A_MODE,HDS_DS);
            break;
        default:
            in_size_h = in_size_v = 0;
            CAM_FUNC_ERR("hdr_sel error(%d)\n",hds_sel);
            rst = MFALSE;
            goto EXIT;
            break;
    }

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,WIDTH) != in_size_h){
        CAM_FUNC_ERR("eis input-h err[0x%x_0x%x]\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,WIDTH),in_size_h);
        rst = MFALSE;
    }
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,HEIGHT) != in_size_v){
        CAM_FUNC_ERR("eis input-v err[0x%x_0x%x]\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,HEIGHT),in_size_v);
        rst = MFALSE;
    }

    //EIS step size
    step_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_NUM_HRP);
    step_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_NUM_VRP);
    if((step_h > 16) || (step_v > 8)){
        CAM_FUNC_ERR("eis step size max:16*8[0x%x_0x%x]\n",step_h,step_v);
        rst = MFALSE;
    }

    //EIS window size
    win_size_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_MB_INTERVAL,EIS_WIN_HSIZE);
    win_size_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_MB_INTERVAL,EIS_WIN_VSIZE);
    if(win_size_h < ((step_h + 1) * 16)){
        CAM_FUNC_ERR("eis h win size too small[0x%x_0x%x]\n",win_size_h,((step_h + 1) * 16));
        rst = MFALSE;
    }
    if(win_size_v < ((step_v + 1) * 16)){
        CAM_FUNC_ERR("eis v win size too small[0x%x_0x%x]\n",win_size_v,((step_v + 1) * 16));
        rst = MFALSE;
    }

    //input check
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,WIDTH) != in_size_h){
        CAM_FUNC_ERR("input width err:%d_%d\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,WIDTH),in_size_h);
        in_size_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,WIDTH);
        rst = MFALSE;
    }
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,HEIGHT) != in_size_v){
        CAM_FUNC_ERR("input height err:%d_%d\n",UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,HEIGHT),in_size_v);
        in_size_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_IMAGE_CTRL,HEIGHT);
        rst = MFALSE;
    }
    op_h = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_OP_HORI);
    op_v = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_OP_VERT);
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
    if( UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,EIS_SUBG_EN) == 1){
        CAM_FUNC_ERR("EIS SubG is always off at isp4.0\n");
        rst = MFALSE;
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("EIS check fail:cur mux:0x%x\n",hds_sel);
    }

    return rst;

}

MBOOL PIPE_CHECK::RSS_CHECK(void)
{
    MUINT32 win_out_w=0,win_out_h=0, win_in_w=0, win_in_h=0;
    MBOOL rst = MTRUE;
    MUINT32 hds2_sel=0;
    ISP_HW_MODULE m_hwModule = CAM_MAX;
    E_ISP_CAM_CQ cq;
    MUINT32 page;


    hds2_sel = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_TOP_PATH_SEL, HDS2_A_SEL);
    this->m_pDrv->getCurObjInfo(&m_hwModule,&cq,&page);

    if(hds2_sel != m_hwModule) {
        CAM_FUNC_INF("HDS2_SEL(0x%x) is not linked to CAM(0x%x)", hds2_sel, m_hwModule);
        goto EXIT;
    }

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,RSS_A_EN) == 0){
        if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,RSSO_A_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable RSSO without enable RSS\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    win_in_w = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_RSS_A_IN_IMG, RSS_IN_WD);
    win_in_h = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_RSS_A_IN_IMG, RSS_IN_HT);
    if((win_in_w < 288) || (win_in_h < 511)) {
        CAM_FUNC_ERR("rss win_in_size need to be bigger than w:288 h:511. Now w:%d, h:%d\n", win_in_w, win_in_h);
        rst = MFALSE;
    }

    //out range is limited
    win_out_w = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_RSS_A_OUT_IMG, RSS_OUT_WD);
    win_out_h = UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_RSS_A_OUT_IMG, RSS_OUT_HT);
    if( (win_out_w > 288) || (win_out_w < 6 ) || (win_out_h < 22) || (win_out_h > 511)){
        CAM_FUNC_ERR("rss out win range is width[6~288], height[22~511]\n");
        rst = MFALSE;
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
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

    //
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,LSC_EN) == 0){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,LSCI_EN) == 1){

            CAM_FUNC_ERR("DO NOT enable LSCI without enable LSC\n");
            rst = MFALSE;
            goto EXIT;
        }
        else
            goto EXIT;
    }

    nWin_h = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL2,LSC_SDBLK_XNUM);
    nWin_v = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_LSC_CTL3,LSC_SDBLK_YNUM);

    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    in_size_x = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    in_size_y = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
    in_size_x = in_size_x >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));
    in_size_y = in_size_y >> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);


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


MBOOL PIPE_CHECK::AF_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 tmp=0;
    MUINT32 win_num_x,win_num_y;
    MUINT32 sgg_sel;
    MUINT32 h_size,win_h_size;
    MUINT32 v_size,win_v_size;
    MUINT32 xsize,ysize;
    MUINT32 af_v_avg_lvl,af_v_gonly;
    MUINT32 start_x=0,start_y=0;
    MUINT32 af_ext_stat_en, af_blk_sz;
    MUINT32 afo_xsize, afo_ysize;
    MUINT32 af_sat_th0, af_sat_th1,af_sat_th2,af_sat_th3;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    //under twin case, sgg_sel won't be 0 , so , don't need to take into consideration at twin case
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
    sgg_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,SGG_SEL);
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
    if(TG_W.Bits.PXL_E - TG_W.Bits.PXL_S < 32) {
        CAM_FUNC_ERR("tg width < 32, can't enable AF:0x%x\n",(TG_W.Bits.PXL_E - TG_W.Bits.PXL_S));
        rst = MFALSE;
    }

    //AFO and AF relaterd module enable check
    tmp  = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AFO_EN)? 0:1);
    tmp += (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RCP_EN)? 0:1);
    tmp += (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,SGG1_EN)? 0:1);
    if(tmp){
        CAM_FUNC_ERR("AF is enabled, MUST enable AFO/RCP/SGG1:0x%x_0x%x_0x%x\n",
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AFO_EN),
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RCP_EN),
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,SGG1_EN));
        rst = MFALSE;
    }

    //
    af_v_avg_lvl = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_CON,AF_V_AVG_LVL);
    af_v_gonly = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_CON,AF_V_GONLY);

    //AF image wd
    switch(sgg_sel){
        case 0:
            h_size = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            v_size = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

            h_size = h_size>>((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
            break;
        case 1:
            h_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RCP_CROP_CON1,RCP_END_X) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RCP_CROP_CON1,RCP_STR_X) + 1;
            v_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RCP_CROP_CON2,RCP_END_Y) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RCP_CROP_CON2,RCP_STR_Y) + 1;
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


    //ofset
    start_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_VLD,AF_VLD_XSTART);
    start_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_VLD,AF_VLD_YSTART);
    if((start_x&0x1)||(start_y&0x1)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF vld start must be even:0x%x_0x%x\n",start_x,start_y);
    }

    //window num
    win_num_x = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_BLK_1,AF_BLK_XNUM);
    win_num_y = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_BLK_1,AF_BLK_YNUM);
    if((win_num_x==0) || (win_num_x>128)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF xwin num :0x%x[1~128]\n",win_num_x);
    }
    if((win_num_y==0) || (win_num_y>128)){
        rst = MFALSE;
        CAM_FUNC_ERR("AF ywin num :0x%x[1~128]\n",win_num_y);
    }

    //win size
    win_h_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_BLK_0,AF_BLK_XSIZE);
    win_v_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_BLK_0,AF_BLK_YSIZE);
    //max
    if(win_h_size > 254){
        rst = MFALSE;
        CAM_FUNC_ERR("af max h win size:254 cur:0x%x\n",win_h_size);
    }
    //min constraint
    if((af_v_avg_lvl == 3) && (af_v_gonly == 1)){
        tmp = 32;
    }
    else if((af_v_avg_lvl == 3) && (af_v_gonly == 0)){
        tmp= 16;
    }
    else if((af_v_avg_lvl == 2) && (af_v_gonly == 1)){
        tmp= 16;
    }
    else{
        tmp= 8;
    }
    if(win_h_size<tmp){
        CAM_FUNC_ERR("af min h win size::0x%x cur:0x%x   [0x%x_0x%x]\n",tmp,win_h_size,af_v_avg_lvl,af_v_gonly);
        rst = MFALSE;
    }

    if(af_v_gonly == 1){
        if(win_h_size & 0x3){
            CAM_FUNC_ERR("af min h win size must 4 alighment:0x%x\n",win_h_size);
            rst = MFALSE;
        }
    }
    else{
        if(win_h_size & 0x1){
            CAM_FUNC_ERR("af min h win size must 2 alighment:0x%x\n",win_h_size);
            rst = MFALSE;
        }
    }

    if(win_v_size > 255){
        rst = MFALSE;
        CAM_FUNC_ERR("af max v win size:255 cur:0x%x\n",win_v_size);
    }
    //min constraint
    if(win_h_size<1){
        CAM_FUNC_ERR("af min v win size:1, cur:0x%x\n",win_h_size);
        rst = MFALSE;
    }

    af_ext_stat_en = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_CON,AF_EXT_STAT_EN);
    af_blk_sz = ((af_ext_stat_en == MTRUE)?32:16);
    if (af_ext_stat_en == 1){
        if(win_h_size < 8){
            CAM_FUNC_ERR("AF_EXT_STAT_EN=1, af min h win size::8 cur:0x%x\n",win_h_size);
            rst = MFALSE;
        }
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,SGG5_EN) == 0 ||
           CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_CON,AF_H_GONLY) != 0) {
            CAM_FUNC_ERR("AF_EXT_STAT_EN=1, MUST enable sgg5 & disable AF_H_GONLY:0x%x_0x%x\n",
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,SGG5_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_CON,AF_H_GONLY));
            rst = MFALSE;
        }
    } else {
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,SGG5_EN) == 1) {
            CAM_FUNC_ERR("AF_EXT_STAT_EN=0, sgg5 must be disabled:0x%x\n", CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,SGG5_EN));
            rst = MFALSE;
        }
    }

    //check max afo size, 128*128*af_blk_sz
    afo_xsize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_XSIZE,XSIZE);
    afo_ysize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_YSIZE,YSIZE);
    if(afo_xsize * afo_ysize > 128*128*af_blk_sz){
        rst = MFALSE;
        CAM_FUNC_ERR("afo max size out of range:0x%x_0x%x\n",afo_xsize * afo_ysize, 128*128*af_blk_sz);
    }

    //xsize/ysize
    xsize = win_num_x*af_blk_sz;
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_XSIZE,XSIZE) != (xsize -1 )){
        CAM_FUNC_ERR("afo xsize mismatch:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_BLK_0,AF_BLK_XSIZE),(xsize -1 ));
        rst = MFALSE;
    }
    ysize = win_num_y;
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_YSIZE,YSIZE) != (ysize -1 )){
        CAM_FUNC_ERR("afo ysize mismatch:0x%x_0x%x\n",CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AFO_YSIZE,YSIZE),(ysize -1 ));
        rst = MFALSE;
    }

    if((start_x + win_h_size*win_num_x) > h_size){
        rst = MFALSE;
        CAM_FUNC_ERR("af h window out of range:0x%x_0x%x\n",(start_x + win_h_size*win_num_x),h_size);
    }
    if((start_y + win_v_size*win_num_y) > v_size){
        rst = MFALSE;
        CAM_FUNC_ERR("af v window out of range:0x%x_0x%x\n",(start_y + win_v_size*win_num_y),v_size);
    }

    //AF_TH
    af_sat_th0 = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_TH_2,AF_HFLT2_SAT_TH0);
    af_sat_th1 = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_TH_2,AF_HFLT2_SAT_TH1);
    af_sat_th2 = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_TH_2,AF_HFLT2_SAT_TH2);
    af_sat_th3 = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AF_TH_2,AF_HFLT2_SAT_TH3);
    if((af_sat_th0 > af_sat_th1) || (af_sat_th1 > af_sat_th2) || (af_sat_th2 > af_sat_th3)){
        CAM_FUNC_ERR("af sat th, MUST th3 >= th2 >= th1 >= th0:0x%x_0x%x_0x%x_0x%x\n",
            af_sat_th3,af_sat_th2,af_sat_th1,af_sat_th0);
        rst = MFALSE;
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
    MUINT32 pmx_sel;
    MUINT32 *pbpc_lut = NULL;
    MUINT32 in_h,in_v;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

    pmx_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,PMX_SEL);


    //pdo
    if( (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDO_EN) == 1) | \
        (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN) == 1)){
        tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_OUT) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN);
        if(tmp == 0 && pmx_sel == PMX_SEL_0){
            CAM_FUNC_ERR("do not enable pdo/pdc_en without enable pdc_en/pdc_out/bpc_lut_en/bpci"
                "_(%d_%d)(%d_%d_%d_%d)\n",
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDO_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_OUT),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN));
            rst = MFALSE;
        }
        //
        tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN) * \
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN);
        if(tmp == 0 && pmx_sel == PMX_SEL_2){
            CAM_FUNC_ERR("do not enable pdo/pdc_en without enable pdc_en/bpc_lut_en/bpci"
                "_(%d_%d)(%d_%d_%d)\n",
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDO_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN));
            rst = MFALSE;
        }
        //mux check
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,PMX_SEL) == PMX_SEL_0){
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN)){
                CAM_FUNC_ERR("warning:bin_en should not be enabled, it may caused crash if bpci table is not corrected\n");
            }
        }
        else if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SEL) & PMX_SEL_1){
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,PBN_EN) == 0){
                CAM_FUNC_ERR("pbn_en should be enabled when pmx_sel is 1\n");
                rst = MFALSE;
            }
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN) == 0){
                CAM_FUNC_ERR("dbn_en should be enabled when pmx_sel is 1\n");
                rst = MFALSE;
            }
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PBN_TYPE,PBN_TYPE) == 1 &&
               CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PBN_LST,PBN_LST) > 1) {

                CAM_FUNC_ERR("pbn start line should be 0~1 when pbn_type is 1\n");
                return MFALSE;
            }
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,PDC_EN)){
               CAM_FUNC_ERR("BNR_PDC shouldn't enable when pmx_sel is 1\n");
               return MFALSE;
            }
        }
        else {
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDE_EN) == 0){
                CAM_FUNC_ERR("pde_en should be enabled when pmx_sel is 2\n");
                rst = MFALSE;
            }
        }
    }

    if((CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_EN) * \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,BPC_LUT_EN)) == 1){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,BPCI_EN) == 0){
            CAM_FUNC_ERR("bpci should be enabled when bpc_en & bpc_lut r enabled\n");
            rst = MFALSE;
        }
    }


    //alignment check
    tmp = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_BASE_ADDR,BASE_ADDR) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_OFST_ADDR,OFFSET_ADDR));

    if((tmp % 16 ) != 0){
        CAM_FUNC_ERR("pdo base addr + offset addr must be 16-alignment(0x%x_0x%x)\n",\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_BASE_ADDR,BASE_ADDR),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_OFST_ADDR,OFFSET_ADDR));
        rst = MFALSE;
    }


    if(this->m_BPCI_VA != 0 && pmx_sel != 1){//not dual pd
        #define SPECIAL_TOKEN 0xc000
        unsigned short int* ptr = (unsigned short int*)this->m_BPCI_VA;
        MUINT32 n_2bytejump = 0;
        MUINT32 ysize = 0;
        MUINT32 table_size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BPCI_STRIDE,STRIDE);
        for(MUINT32 i=0; i<table_size; )
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
            else//bp
                i+=2;
        }
        #undef SPECIAL_TOKEN

        if(ysize != (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_YSIZE,YSIZE) + 1)){
            ptr = (unsigned short int*)this->m_BPCI_VA;
            CAM_FUNC_ERR("BPCI table error:0x%x_0x%x\n",ysize,(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_YSIZE,YSIZE) + 1));
            CAM_FUNC_ERR("table content : 1st pdc line : 0x%4x,0x%4x,0x%4x,0x%4x,0x%4x,0x%4x,0x%4x,0x%4x\n",
                ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);

            rst = MFALSE;
        }

        if ( 0 == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN)){
            /* single case */
            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

            in_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            in_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

            in_h = in_h>> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));
            in_v = in_v>> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);

            if((in_h-1) != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_XSIZE)) {
                CAM_FUNC_ERR("bnr bpc_xize error:0x%x_0x%x", \
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_XSIZE),in_h-1);
            }
            if((in_v-1) != CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_YSIZE)) {
                CAM_FUNC_ERR("bnr bpc_yize error:0x%x_0x%x", \
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_YSIZE),in_v-1);
            }
        }
        else {
            /*twin case, twin driver will take care size setting, we just check size should > 0*/
            if(0 == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_XSIZE))
                CAM_FUNC_ERR("bnr bpc_xize must > 0");
            if(0 == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_TBLI2,BPC_YSIZE))
                CAM_FUNC_ERR("bnr bpc_yize must > 0");
        }
    }
    if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SEL) & PMX_SEL_1) {
        MUINT32 pdo_stride, pdo_ysize;
        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

        pdo_stride = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_STRIDE, STRIDE);
        pdo_ysize = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PDO_YSIZE, YSIZE)+1;

        in_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        in_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

        tmp = (in_h >> 4) << 2;/* TG_W/16*2*2 */
        if(pdo_stride != tmp) {
            CAM_FUNC_ERR("pdo stride error : tg_w(%d)stride(%d_%d)\n",in_h, tmp, pdo_stride);
            rst = MFALSE;
        }
        tmp = in_v >> 3;/* TG_W/8 */
        if(pdo_ysize != tmp) {
            CAM_FUNC_ERR("pdo ysize error : tg_h(%d)ysize(%d_%d)\n",in_v, tmp, pdo_ysize);
            rst = MFALSE;
        }

    }

    if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SEL) & PMX_SEL_2) {
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDE_EN) ^
           CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDI_EN))
        {
            CAM_FUNC_ERR("pde & pdi must enable/disable at the same time(%d_%d)\n",
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDE_EN),
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,PDI_EN));
            rst = MFALSE;
        }
    }

EXIT:
    if(rst == MFALSE){
        CAM_FUNC_ERR("pd check fail:cur mux:0x%x\n",pmx_sel);
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

    TG_IN_W = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST,PXL_CNT);
    TG_IN_V = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST,LINE_CNT);


    TG_IN_W_R = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST_R,PXL_CNT);
    TG_IN_V_R = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_FRMSIZE_ST_R,LINE_CNT);

    DATA_CNT_R = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_DAT_NO_R,DAT_NO);

    if(TG_IN_W < (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S)){
        CAM_FUNC_ERR("seninf horizontal data is small than grab window_w:%d_%d\n",TG_IN_W,(TG_W.Bits.PXL_E - TG_W.Bits.PXL_S));
        rst = MFALSE;
    }
    if(TG_IN_V < (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S)){
        CAM_FUNC_ERR("seninf vertical data is small than grab window_v:%d_%d\n",TG_IN_V,(TG_H.Bits.LIN_E - TG_H.Bits.LIN_S));
        rst = MFALSE;
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
        case 16: {
            MUINT32 count = 10000;
            while(count-->0) {;}
                if(TG_IN_W_R != TG_IN_W){
                    if(DATA_CNT_R == CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_DAT_NO_R,DAT_NO)){
                        CAM_FUNC_ERR("seninf have no input data for 10000 times loop when TG is under exposure,cur data:0x%x\n",DATA_CNT_R);
                        rst = MFALSE;
                    }
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

MBOOL PIPE_CHECK::TWIN_CHECK(ISP_DRV_CAM* pTwin)
{
    MBOOL rst = MTRUE;

    if(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SPARE1) !=
        CAM_READ_REG(pTwin->getPhyObj(),CAM_CTL_SPARE1) ){
        CAM_FUNC_ERR("update timing of CAM_A & CAM_B 's CQ is over h1 p1 done\n");
        CAM_FUNC_ERR("CAM_A's:0x%x,CAM_B's:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SPARE1),\
            CAM_READ_REG(pTwin->getPhyObj(),CAM_CTL_SPARE1));
        rst = MFALSE;
    }

    return rst;
}

MBOOL PIPE_CHECK::RRZ_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 rrz_in[2];
    MUINT32 rrz_crop[4];

    //support no scale-up
    rrz_in[0] = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_RRZ_IN_IMG);
    rrz_in[1] = rrz_in[0]>>16;
    rrz_in[0] = rrz_in[0] & 0xffff;

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

    return rst;
}


MBOOL PIPE_CHECK::XMX_CHECK(void)
{
    MBOOL ret = MTRUE;
    capibility CamInfo;
    tCAM_rst rst;
    MUINT32 _size;


    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN)){
        //this 2 module have qbn before xmx,  so need to have extra check because of
        // 1: under single path ,  sram size is don't-care
        // 2: under twin path , size before xmx need to have < xmx sram size.

        //bmx
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BMX_EN)){
            CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_BMX);
            _size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_CROP,BMX_END_X) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_BMX_CROP,BMX_STR_X) + 1;
            if(_size > rst.xmx.line_bmx){
                CAM_FUNC_ERR("BMX over linebuffer : %d_%d\n",_size,rst.xmx.line_bmx);
                ret = MFALSE;
            }
        }

        //amx
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,AMX_EN)){
            CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_AMX);
            _size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AMX_CROP,AMX_END_X) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_AMX_CROP,AMX_STR_X) + 1;
            if(_size > rst.xmx.line_amx){
                CAM_FUNC_ERR("AMX over linebuffer : %d_%d\n",_size,rst.xmx.line_amx);
                ret = MFALSE;
            }
        }
    }



    //pmx
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,PMX_EN)){
        CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_PMX);
        _size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PMX_CROP,PMX_END_X) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_PMX_CROP,PMX_STR_X) + 1;
        if(_size > rst.xmx.line_pmx){
            CAM_FUNC_ERR("PMX over linebuffer : %d_%d\n",_size,rst.xmx.line_pmx);
            ret = MFALSE;
        }
    }


    //rmx
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,RMX_EN)){
        CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_RMX);
        _size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMX_CROP,RMX_END_X) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_RMX_CROP,RMX_STR_X) + 1;
        if(_size > rst.xmx.line_rmx){
            CAM_FUNC_ERR("RMX over linebuffer : %d_%d\n",_size,rst.xmx.line_rmx);
            ret = MFALSE;
        }
    }


    //dmx
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DMX_EN)){
        CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_DMX);
        _size = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_DMX_CROP,DMX_END_X) - CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_DMX_CROP,DMX_STR_X) + 1;
        if(_size > rst.xmx.line_dmx){
            CAM_FUNC_ERR("DMX over linebuffer : %d_%d\n",_size,rst.xmx.line_dmx);
            ret = MFALSE;
        }
    }

    return ret;
}

