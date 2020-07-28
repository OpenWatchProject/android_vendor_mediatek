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
/**
 * @file ae_mgr_configt.h
 * @brief AE isp hardware config.
 */
#ifndef _AE_MGR_CONFIG_H_
#define _AE_MGR_CONFIG_H_
#include <ResultPoolBase.h>

using namespace NS3Av3;
enum
{
	//ERegInfo_CAM_AE_GAIN2_0,
	//ERegInfo_CAM_AE_GAIN2_1,
	ERegInfo_AA_R1_AA_AE_LMT2_0       ,
    ERegInfo_AA_R1_AA_AE_LMT2_1       ,
    ERegInfo_AA_R1_AA_AE_RC_CNV_0     ,
    ERegInfo_AA_R1_AA_AE_RC_CNV_1     ,
    ERegInfo_AA_R1_AA_AE_RC_CNV_2     ,
    ERegInfo_AA_R1_AA_AE_RC_CNV_3     ,
    ERegInfo_AA_R1_AA_AE_RC_CNV_4     ,
    ERegInfo_AA_R1_AA_AE_YGAMMA_0     ,
    ERegInfo_AA_R1_AA_AE_YGAMMA_1     ,
    ERegInfo_AA_R1_AA_AE_OVER_EXPO_CFG,
    ERegInfo_AA_R1_AA_AE_PIX_HST_CTL  ,
    ERegInfo_AA_R1_AA_AE_PIX_HST_SET  ,
    ERegInfo_AA_R1_AA_AE_PIX_HST_SET_1,
	ERegInfo_AA_R1_AA_AE_PIX_HST0_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST0_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST1_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST1_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST2_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST2_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST3_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST3_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST4_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST4_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST5_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_HST5_XRNG,
  ERegInfo_AA_R1_AA_AE_PIX_SE_HST_SET  ,
    ERegInfo_AA_R1_AA_AE_PIX_SE_HST_SET_1,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST0_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST0_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST1_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST1_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST2_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST2_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST3_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST3_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST4_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST4_XRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST5_YRNG,
	ERegInfo_AA_R1_AA_AE_PIX_SE_HST5_XRNG,
	ERegInfo_AA_R1_AA_AE_STAT_EN,
	ERegInfo_AAO_R1_AAO_XSIZE,
	ERegInfo_AAO_R1_AAO_YSIZE,
	ERegInfo_AA_R1_AA_AE_YCOEF,
	ERegInfo_AA_R1_AA_AE_MODE,
	ERegInfo_AA_R1_AA_AE_BIT_CTL,
	ERegInfo_AA_R1_AA_AE_HDR_CFG,
	// [PSO]
	//ERegInfo_CAM_PS_AE_YCOEF0,
	//ERegInfo_CAM_PS_AE_YCOEF1,
	//ERegInfo_CAM_PS_DATA_TYPE,
	//ERegInfo_CAM_PS_HST_CFG,
	//ERegInfo_CAM_PS_HST_ROI_X,
	//ERegInfo_CAM_PS_HST_ROI_Y,
	//ERegInfo_CAM_PSO_XSIZE,
	//ERegInfo_CAM_PSO_YSIZE,
	EAERegInfo_NUM
};

typedef struct AEISPREG_INFO
{
    MUINT32     addr; ///< address
    MUINT32     val;  ///< value
    AEISPREG_INFO()
        :addr(0),val(0)
    {}
} AEISPREG_INFO_T;

typedef AEISPREG_INFO_T   AERegInfo_T;

typedef struct AEResultConfig_t : public ResultPoolBase_T
{
    AERegInfo_T rAERegInfo[EAERegInfo_NUM];

    AEResultConfig_t()
    {
        ::memset(rAERegInfo, 0, EAERegInfo_NUM*sizeof(AERegInfo_T));
    }

    MVOID clearMember()
    {
        AEResultConfig_t();
    }

    // copy operator of AEResultConfig_t
    AEResultConfig_t& operator = (const AEResultConfig_t& o)
    {
        ::memcpy(this->rAERegInfo, o.rAERegInfo, EAERegInfo_NUM*sizeof(AERegInfo_T));
        return *this;
    }

    // move operator of AEResultConfig_t
    AEResultConfig_t& operator = (AEResultConfig_t&& o) = default;

    virtual ~AEResultConfig_t() = default;

    // Get Id
    virtual int getId() const { return E_AE_CONFIGRESULTTOISP; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of AFConfigResult, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of AFConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }

} AEResultConfig_T;

#endif // _AE_CONFIG_H_
