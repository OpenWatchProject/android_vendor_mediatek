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
 * @file flicker_hw_config.h
 * @brief FLK isp hardware config.
 */
#ifndef _ISP_FLICKER_CONFIG_H_
#define _ISP_FLICKER_CONFIG_H_

#include <ResultPoolBase.h>

using namespace NS3Av3;

enum
{
    ERegInfo_FLK_R1_FLK_CON,
    ERegInfo_FLK_R1_FLK_OFST,
    ERegInfo_FLK_R1_FLK_SIZE,
    ERegInfo_FLK_R1_FLK_NUM,
    ERegInfo_FLK_R1_FLK_SGG_GAIN,
    ERegInfo_FLK_R1_FLK_SGG_GMR1,
    ERegInfo_FLK_R1_FLK_SGG_GMR2,
    ERegInfo_FLK_R1_FLK_SGG_GMR3,
    ERegInfo_FLK_R1_FLK_SGG_GMR4,
    ERegInfo_FLK_R1_FLK_SGG_GMR5,
    ERegInfo_FLK_R1_FLK_SGG_GMR6,
    ERegInfo_FLK_R1_FLK_SGG_GMR7,
    ERegInfo_FLK_R1_FLK_ZHDR,
    ERegInfo_FLKO_R1_FLKO_XSIZE,
    ERegInfo_FLKO_R1_FLKO_YSIZE,
    ERegInfo_FLKO_R1_FLKO_STRIDE,

    EFLKRegInfo_NUM
};

typedef struct FLKISPREG_INFO
{
    MUINT32     addr; ///< address
    MUINT32     val;  ///< value
    FLKISPREG_INFO()
        :addr(0),val(0)
    {}
} FLKISPREG_INFO_T;

typedef FLKISPREG_INFO_T   FLKRegInfo_T;

typedef struct FLKResultConfig_t : public ResultPoolBase_T
{
    MBOOL           enableFLKHw;
    MINT32          configNum;
    FLKRegInfo_T    rFLKRegInfo[EFLKRegInfo_NUM];
    FLKResultConfig_t()
        : enableFLKHw(0)
        , configNum(0)
    {
        ::memset(rFLKRegInfo, 0, EFLKRegInfo_NUM*sizeof(FLKRegInfo_T));
    }

    MVOID clearMember()
    {
        FLKResultConfig_t();
    }

    // copy operator of FLKResultConfig_t
    FLKResultConfig_t& operator = (const FLKResultConfig_t& o)
    {
        ::memcpy(this, &o, sizeof(FLKResultConfig_t));
        return *this;
    }

    // move operator of FLKResultConfig_t
    FLKResultConfig_t& operator = (FLKResultConfig_t&& o) = default;

    virtual ~FLKResultConfig_t() = default;

    // Get Id
    virtual int getId() const { return E_FLK_CONFIGRESULTTOISP; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of FLKResultConfig_t, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of FLKResultConfig_t, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
} FLKResultConfig_T;

#endif // _ISP_FLICKER_CONFIG_H_