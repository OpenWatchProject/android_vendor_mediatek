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

#define LOG_TAG "ifunc_cam_uni"

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




UNI_TOP_CTRL::UNI_TOP_CTRL()
{
    UNI_Source = IF_CAM_MAX;
    m_pUniDrv= NULL;
    m_pDrv = NULL;
    m_pCamDrv = NULL;
}

MINT32 UNI_TOP_CTRL::_config( void )
{
    MUINT32 _loglevel;
    MUINT32 _tmp;

    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_CAM_A:
            this->m_hwModule = CAM_A;
            break;
        case IF_RAWI_CAM_B:
        case IF_CAM_B:
            this->m_hwModule = CAM_B;
            break;
        default:
            CAM_FUNC_ERR("unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }
    if(this->m_pCamDrv == NULL){
        CAM_FUNC_ERR("can't be NULL\n");
        return -1;
    }
    CAM_FUNC_INF("UNI_TOP_CTRL::_config +,uni_module(0x%x),en(0x%08x),fmtSel(0x%08x)\n", \
            this->m_pUniDrv->getCurObjInfo(), \
            this->uni_top_ctl.FUNC_EN.Raw, \
            this->uni_top_ctl.FMT_SEL.Raw);

    /********************************************************************************************************************/
    //most uni setting r configured thru CQ
    /********************************************************************************************************************/

    //func en
    //UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_MOD_EN,this->uni_top_ctl.FUNC_EN.Raw);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,this->uni_top_ctl.FUNC_EN.Raw);


    //default always enabe for rrz sram share needs
    UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,RLB_A_EN,1);

    //fmt sel
    //UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_FMT_SEL,this->uni_top_ctl.FMT_SEL.Raw);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_FMT_SEL,this->uni_top_ctl.FMT_SEL.Raw);

    //inte en
    UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_DMA_INT_EN,DMA_ERR_INT_EN,1);

    //db_en
    UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);


    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            //input information, borrow tg crop to keep informaiton
            CAM_WRITE_REG(this->m_pCamDrv,CAM_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
            CAM_WRITE_REG(this->m_pCamDrv,CAM_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

            CAM_WRITE_BITS(this->m_pCamDrv,CAM_CTL_EN,TG_EN,0);
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            break;
        default:
            CAM_FUNC_ERR("unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }

    //
    CAM_FUNC_DBG("-");
    return 0;
}


MINT32 UNI_TOP_CTRL::_enable( void* pParam  )
{
    (void)pParam;
     CAM_FUNC_DBG("UNI_TOP_CTRL::_enable: + \n");


    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_RAWI_TRIG,TOP_RAWI_TRIG,1);
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            CAM_FUNC_INF("UNI link with CAM(0x%x)\n",this->UNI_Source);
            break;
        default:
            CAM_FUNC_ERR("UNI unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }
    //
    CAM_FUNC_DBG("-");
    return 0;
}

MINT32 UNI_TOP_CTRL::ClrUniFunc(void)
{
    CAM_FUNC_DBG("UNI_TOP_CTRL::ClrUniFunc: +\n");

    /* clear all bits except RLB_A_EN */
    //UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
    UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
    UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,ADL_A_EN,0);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,0);
    return 0;
}

MINT32 UNI_TOP_CTRL::_disable( void* pParam )
{
    MINT32 ret = 0;
    ISP_WAIT_IRQ_ST irq;
    (void)pParam;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;

    irq.St_type = SIGNAL_INT;
    irq.Timeout = 0x1000;
    irq.UserKey = 0x0;

    //dbg log
    CAM_FUNC_DBG("UNI_TOP_CTRL::_disable: + \n");

    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            if(CAM_READ_BITS(this->m_pCamDrv,CAM_TG_VF_CON,VFDATA_EN))
            {
                MUINT32 _cnt = 0;
                //wait sof for timing-align
                irq.Status = SOF_INT_ST;
                if(this->m_pCamDrv->waitIrq(&irq) == MFALSE)
                    CAM_FUNC_ERR("UNI waitIrq fail\n");
                //off db load to force register setting to be flushed into inner immediately
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,0);
                /* clear all bits except RLB_A_EN */
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,ADL_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);
            }
            else
            {
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,0);
                /* clear all bits except RLB_A_EN */
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,ADL_A_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);
            }
            break;
        default:
            CAM_FUNC_ERR("UNI unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }

    //reset
    CAM_FUNC_DBG("UNI reset\n");
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x2);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x1);
#if 0
    while(UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL) != 0x222){
        CAM_FUNC_DBG("UNI reseting...\n");
    }
#else
    usleep(10);
#endif
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x4);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0);
EXIT:
    CAM_FUNC_DBG("-\n");

    return ret;
}
