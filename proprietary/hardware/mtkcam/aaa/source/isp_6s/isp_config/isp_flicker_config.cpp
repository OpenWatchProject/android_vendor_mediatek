/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_flicker_config"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <flicker_hal.h>
#include <isp_flicker_config.h>
#include <drv/isp_reg.h>
#include <flicker_platform_adapter.h>

using namespace NS3Av3;

#define REG_FLK_INFO(REG, VALUE) \
    { \
        pFLKRegInfo[ERegInfo_##REG].addr = ((MUINT32)offsetof(cam_reg_t, REG)); \
        pFLKRegInfo[ERegInfo_##REG].val  = VALUE; \
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT FlickerPlatformAdapter::FLKConfig(FLKWinCFG_T *a_sInputFLKInfo, MVOID *p_sFLKResultConfig)
{
    FLKRegInfo_T *pFLKRegInfo = ((FLKResultConfig_T *)p_sFLKResultConfig)->rFLKRegInfo;

    REG_FLK_R1_FLK_CON reg_con;
    reg_con.Raw = 0;
    reg_con.Bits.FLK_INPUT_BIT_SEL = a_sInputFLKInfo->m_u4INPUT_BIT_SEL;

    REG_FLK_R1_FLK_OFST reg_ofst;
    reg_ofst.Raw = 0;
    reg_ofst.Bits.FLK_OFST_X = a_sInputFLKInfo->m_u4OffsetX;
    reg_ofst.Bits.FLK_OFST_Y = a_sInputFLKInfo->m_u4OffsetY;

    REG_FLK_R1_FLK_SIZE reg_size;
    reg_size.Raw = 0;
    reg_size.Bits.FLK_SIZE_X = a_sInputFLKInfo->m_u4SizeX;
    reg_size.Bits.FLK_SIZE_Y = a_sInputFLKInfo->m_u4SizeY;

    REG_FLK_R1_FLK_NUM reg_num;
    reg_num.Raw = 0;
    reg_num.Bits.FLK_NUM_X = a_sInputFLKInfo->m_u4NumX;
    reg_num.Bits.FLK_NUM_Y = a_sInputFLKInfo->m_u4NumY;

    REG_FLK_R1_FLK_SGG_GAIN reg_pgn;
    reg_pgn.Raw = 0;
    reg_pgn.Bits.FLK_SGG_GAIN = a_sInputFLKInfo->m_u4SGG3_PGN;

    REG_FLK_R1_FLK_SGG_GMR1 reg_gmr1;
    reg_gmr1.Raw = 0;
    reg_gmr1.Bits.FLK_SGG_GMR1 = a_sInputFLKInfo->m_u4SGG3_GMR1;

    REG_FLK_R1_FLK_SGG_GMR2 reg_gmr2;
    reg_gmr2.Raw = 0;
    reg_gmr2.Bits.FLK_SGG_GMR2 = a_sInputFLKInfo->m_u4SGG3_GMR2;

    REG_FLK_R1_FLK_SGG_GMR3 reg_gmr3;
    reg_gmr3.Raw = 0;
    reg_gmr3.Bits.FLK_SGG_GMR3 = a_sInputFLKInfo->m_u4SGG3_GMR3;

    REG_FLK_R1_FLK_SGG_GMR4 reg_gmr4;
    reg_gmr4.Raw = 0;
    reg_gmr4.Bits.FLK_SGG_GMR4 = a_sInputFLKInfo->m_u4SGG3_GMR4;

    REG_FLK_R1_FLK_SGG_GMR5 reg_gmr5;
    reg_gmr5.Raw = 0;
    reg_gmr5.Bits.FLK_SGG_GMR5 = a_sInputFLKInfo->m_u4SGG3_GMR5;

    REG_FLK_R1_FLK_SGG_GMR6 reg_gmr6;
    reg_gmr6.Raw = 0;
    reg_gmr6.Bits.FLK_SGG_GMR6 = a_sInputFLKInfo->m_u4SGG3_GMR6;

    REG_FLK_R1_FLK_SGG_GMR7 reg_gmr7;
    reg_gmr7.Raw = 0;
    reg_gmr7.Bits.FLK_SGG_GMR7 = a_sInputFLKInfo->m_u4SGG3_GMR7;

    REG_FLK_R1_FLK_ZHDR reg_zhdr;
    reg_zhdr.Raw = 0;
    reg_zhdr.Bits.FLK_ZHDR_NOISE_VAL = a_sInputFLKInfo->m_u4ZHDR_NOISE_VAL;
    reg_zhdr.Bits.FLK_SGG_OUT_MAX_VAL = a_sInputFLKInfo->m_u4SGG_OUT_MAX_VAL;

    REG_FLKO_R1_FLKO_XSIZE reg_xsize;
    reg_xsize.Raw = 0;
    reg_xsize.Bits.FLKO_XSIZE = a_sInputFLKInfo->m_u4DmaSize;

    REG_FLKO_R1_FLKO_YSIZE reg_ysize;
    reg_ysize.Raw = 0;
    reg_ysize.Bits.FLKO_YSIZE = 0x0001;

    REG_FLKO_R1_FLKO_STRIDE reg_stride;
    reg_stride.Raw = 0;
    reg_stride.Bits.FLKO_STRIDE = a_sInputFLKInfo->m_u4DmaSize;


    REG_FLK_INFO(FLK_R1_FLK_CON, (MUINT32)reg_con.Raw);
    REG_FLK_INFO(FLK_R1_FLK_OFST, (MUINT32)reg_ofst.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SIZE, (MUINT32)reg_size.Raw);
    REG_FLK_INFO(FLK_R1_FLK_NUM, (MUINT32)reg_num.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GAIN, (MUINT32)reg_pgn.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR1, (MUINT32)reg_gmr1.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR2, (MUINT32)reg_gmr2.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR3, (MUINT32)reg_gmr3.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR4, (MUINT32)reg_gmr4.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR5, (MUINT32)reg_gmr5.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR6, (MUINT32)reg_gmr6.Raw);
    REG_FLK_INFO(FLK_R1_FLK_SGG_GMR7, (MUINT32)reg_gmr7.Raw);
    REG_FLK_INFO(FLK_R1_FLK_ZHDR, (MUINT32)reg_zhdr.Raw);
    REG_FLK_INFO(FLKO_R1_FLKO_XSIZE, (MUINT32)reg_xsize.Raw);
    REG_FLK_INFO(FLKO_R1_FLKO_YSIZE, (MUINT32)reg_ysize.Raw);
    REG_FLK_INFO(FLKO_R1_FLKO_STRIDE, (MUINT32)reg_stride.Raw);


    ((FLKResultConfig_T *)p_sFLKResultConfig)->enableFLKHw = 1;
    ((FLKResultConfig_T *)p_sFLKResultConfig)->configNum = EFLKRegInfo_NUM;

    return MTRUE;
}

