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
 * @file awb_mgr_config.h
 * @brief AWB isp hardware config.
 */
#ifndef _AWB_MGR_CONFIG_H_
#define _AWB_MGR_CONFIG_H_
#include <ResultPoolBase.h>

using namespace NS3Av3;
enum
{
#if 0
	ERegInfo_CAM_AWB_WIN_ORG,
	ERegInfo_CAM_AWB_WIN_SIZE,
	ERegInfo_CAM_AWB_WIN_PIT,
	ERegInfo_CAM_AWB_WIN_NUM,
	ERegInfo_CAM_AWB_GAIN1_0,
	ERegInfo_CAM_AWB_GAIN1_1,
	ERegInfo_CAM_AWB_LMT1_0,
	ERegInfo_CAM_AWB_LMT1_1,
	ERegInfo_CAM_AWB_LOW_THR,
	ERegInfo_CAM_AWB_HI_THR,
	ERegInfo_CAM_AWB_PIXEL_CNT0,
	ERegInfo_CAM_AWB_PIXEL_CNT1,
	ERegInfo_CAM_AWB_PIXEL_CNT2,
	ERegInfo_CAM_AWB_ERR_THR,
	ERegInfo_CAM_AWB_ROT,
	ERegInfo_CAM_AWB_L0_X,
	ERegInfo_CAM_AWB_L0_Y,
	ERegInfo_CAM_AWB_L1_X,
	ERegInfo_CAM_AWB_L1_Y,
	ERegInfo_CAM_AWB_L2_X,
	ERegInfo_CAM_AWB_L2_Y,
	ERegInfo_CAM_AWB_L3_X,
	ERegInfo_CAM_AWB_L3_Y,
	ERegInfo_CAM_AWB_L4_X,
	ERegInfo_CAM_AWB_L4_Y,
	ERegInfo_CAM_AWB_L5_X,
	ERegInfo_CAM_AWB_L5_Y,
	ERegInfo_CAM_AWB_L6_X,
	ERegInfo_CAM_AWB_L6_Y,
	ERegInfo_CAM_AWB_L7_X,
	ERegInfo_CAM_AWB_L7_Y,
	ERegInfo_CAM_AWB_L8_X,
	ERegInfo_CAM_AWB_L8_Y,
	ERegInfo_CAM_AWB_L9_X,
	ERegInfo_CAM_AWB_L9_Y,
	ERegInfo_CAM_AWB_SPARE,
	ERegInfo_CAM_AWB_MOTION_THR,
	// RWB
	ERegInfo_CAM_AWB_RC_CNV_0,
	ERegInfo_CAM_AWB_RC_CNV_1,
	ERegInfo_CAM_AWB_RC_CNV_2,
	ERegInfo_CAM_AWB_RC_CNV_3,
	ERegInfo_CAM_AWB_RC_CNV_4,
	//AE pregain2
	ERegInfo_CAM_AE_GAIN2_0,
	ERegInfo_CAM_AE_GAIN2_1,
	// PSO
	ERegInfo_CAM_PS_AWB_WIN_ORG,
	ERegInfo_CAM_PS_AWB_WIN_SIZE,
	ERegInfo_CAM_PS_AWB_WIN_PIT,
	ERegInfo_CAM_PS_AWB_WIN_NUM,
	ERegInfo_CAM_PS_AWB_PIXEL_CNT0,
	ERegInfo_CAM_PS_AWB_PIXEL_CNT1,
	ERegInfo_CAM_PS_AWB_PIXEL_CNT2,
	ERegInfo_CAM_PS_AWB_PIXEL_CNT3,
	ERegInfo_CAM_AWB_NUM
#else //ISP6.0
    ERegInfo_AA_R1_AA_AWB_WIN_ORG,             
    ERegInfo_AA_R1_AA_AWB_WIN_SIZE,            
    ERegInfo_AA_R1_AA_AWB_WIN_PIT,             
    ERegInfo_AA_R1_AA_AWB_WIN_NUM,             
    ERegInfo_AA_R1_AA_AWB_GAIN1_0,             
    ERegInfo_AA_R1_AA_AWB_GAIN1_1,             
    ERegInfo_AA_R1_AA_AWB_VALID_DATA_WIDTH,    
    ERegInfo_AA_R1_AA_AWB_LMT1_0,              
    ERegInfo_AA_R1_AA_AWB_LMT1_1,              
    ERegInfo_AA_R1_AA_AWB_LMT1_2,              
    ERegInfo_AA_R1_AA_AWB_LOW_THR_0,           
    ERegInfo_AA_R1_AA_AWB_LOW_THR_1,           
    ERegInfo_AA_R1_AA_AWB_LOW_THR_2,           
    ERegInfo_AA_R1_AA_AWB_HI_THR_0,            
    ERegInfo_AA_R1_AA_AWB_HI_THR_1,            
    ERegInfo_AA_R1_AA_AWB_HI_THR_2,            
    ERegInfo_AA_R1_AA_AWB_LIGHTSRC_LOW_THR_0,  
    ERegInfo_AA_R1_AA_AWB_LIGHTSRC_LOW_THR_1,  
    ERegInfo_AA_R1_AA_AWB_LIGHTSRC_HI_THR_0,   
    ERegInfo_AA_R1_AA_AWB_LIGHTSRC_HI_THR_1,   
    ERegInfo_AA_R1_AA_AWB_PIXEL_CNT0,          
    ERegInfo_AA_R1_AA_AWB_PIXEL_CNT1,          
    ERegInfo_AA_R1_AA_AWB_PIXEL_CNT2,          
    ERegInfo_AA_R1_AA_AWB_ERR_THR,             
    ERegInfo_AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE,
    ERegInfo_AA_R1_AA_AWB_FORMAT_SHIFT,        
    ERegInfo_AA_R1_AA_AWB_POSTGAIN_0,          
    ERegInfo_AA_R1_AA_AWB_POSTGAIN_1,          
    ERegInfo_AA_R1_AA_AWB_ROT,                 
    ERegInfo_AA_R1_AA_AWB_L0_X,                
    ERegInfo_AA_R1_AA_AWB_L0_Y,                
    ERegInfo_AA_R1_AA_AWB_L1_X,                
    ERegInfo_AA_R1_AA_AWB_L1_Y,                
    ERegInfo_AA_R1_AA_AWB_L2_X,                
    ERegInfo_AA_R1_AA_AWB_L2_Y,                
    ERegInfo_AA_R1_AA_AWB_L3_X,                
    ERegInfo_AA_R1_AA_AWB_L3_Y,                
    ERegInfo_AA_R1_AA_AWB_L4_X,                
    ERegInfo_AA_R1_AA_AWB_L4_Y,                
    ERegInfo_AA_R1_AA_AWB_L5_X,                
    ERegInfo_AA_R1_AA_AWB_L5_Y,                
    ERegInfo_AA_R1_AA_AWB_L6_X,                
    ERegInfo_AA_R1_AA_AWB_L6_Y,                
    ERegInfo_AA_R1_AA_AWB_L7_X,                
    ERegInfo_AA_R1_AA_AWB_L7_Y,                
    ERegInfo_AA_R1_AA_AWB_L8_X,                
    ERegInfo_AA_R1_AA_AWB_L8_Y,                
    ERegInfo_AA_R1_AA_AWB_L9_X,                
    ERegInfo_AA_R1_AA_AWB_L9_Y,                
    ERegInfo_AA_R1_AA_AWB_RC_CNV_0,            
    ERegInfo_AA_R1_AA_AWB_RC_CNV_1,            
    ERegInfo_AA_R1_AA_AWB_RC_CNV_2,            
    ERegInfo_AA_R1_AA_AWB_RC_CNV_3,            
    ERegInfo_AA_R1_AA_AWB_RC_CNV_4,            
    ERegInfo_AA_R1_AA_AWB_SPARE,               
    ERegInfo_AA_R1_AA_AE_GAIN2_0,              
    ERegInfo_AA_R1_AA_AE_GAIN2_1,              
    ERegInfo_CAM_AWB_NUM

#endif
};

typedef struct AWBISPREG_INFO
{
    MUINT32     addr; ///< address
    MUINT32     val;  ///< value
    AWBISPREG_INFO()
        :addr(0),val(0)
    {}
} AWBISPREG_INFO_T;

typedef AWBISPREG_INFO_T   AWBRegInfo_T;

typedef struct AWBResultConfig_t : public ResultPoolBase_T
{
    AWBRegInfo_T rAWBRegInfo[ERegInfo_CAM_AWB_NUM];

    AWBResultConfig_t()
    {
        ::memset(rAWBRegInfo, 0, ERegInfo_CAM_AWB_NUM*sizeof(AWBRegInfo_T));
    }

    MVOID clearMember()
    {
        AWBResultConfig_t();
    }

    // copy operator of AWBResultConfig_t
    AWBResultConfig_t& operator = (const AWBResultConfig_t& o)
    {
        ::memcpy(this->rAWBRegInfo, o.rAWBRegInfo, ERegInfo_CAM_AWB_NUM*sizeof(AWBRegInfo_T));
        return *this;
    }

    // move operator of AEResultConfig_t
    AWBResultConfig_t& operator = (AWBResultConfig_t&& o) = default;

    virtual ~AWBResultConfig_t() = default;

    // Get Id
    virtual int getId() const { return E_AWB_CONFIGRESULTTOISP; }

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

} AWBResultConfig_T;

#endif // _AWB_CONFIG_H_
