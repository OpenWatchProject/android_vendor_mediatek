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

/**
* @file ResultPool4LSCConfig.h
* @brief Declarations of structure of module Manager
*/
#ifndef __RESULT_POOL_4_LSCCONFIG_H__
#define __RESULT_POOL_4_LSCCONFIG_H__

#include <ResultPoolBase.h>
#include <lsc/ILscBuf.h>

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Modules config structure
//  For modules. Not to update to "Result to Meta structure".
//  The struct format is xxxConfigResult_T.
//  Modules need to update all structure. Don't update one member of structure.
//  If modules want to update one member, need to define new structure which has one member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct LSCConfigResult_T : public ResultPoolBase_T
{
public:
    NSIspTuning::ILscBuf *rResultBuf;
    MUINT32 ratio;

    //ILscMgr::SL2_CFG_T cfg_result;
    int i4CenterX;
    int i4CenterY;
    int i4R0;
    int i4R1;
    int i4R2;
    int i4Gain0;
    int i4Gain1;
    int i4Gain2;
    int i4Gain3;
    int i4Gain4;
    int i4SetZero;

    MUINT32 u4RawWd;
    MUINT32 u4RawHt;
    MUINT32 u4TsfsH;
    MUINT32 u4TsfsV;

    int eIdx_Shading_CCT;

    int fgOnOff;

    LSCConfigResult_T()
        : rResultBuf(nullptr)
        , ratio(0)
        , i4CenterX(0)
        , i4CenterY(0)
        , i4R0(0)
        , i4R1(0)
        , i4R2(0)
        , i4Gain0(0)
        , i4Gain1(64)
        , i4Gain2(128)
        , i4Gain3(192)
        , i4Gain4(255)
        , i4SetZero(0)
        , u4RawWd(0)
        , u4RawHt(0)
        , u4TsfsH(0)
        , u4TsfsV(0)
        , eIdx_Shading_CCT(0)
        , fgOnOff(1)
    {}

    MVOID clearMember()
    {
        LSCConfigResult_T();
    }

    // copy operator of LSCConfigResult_T
    LSCConfigResult_T& operator = (const LSCConfigResult_T& o) = default;

    // move operator of LSCConfigResult_T
    LSCConfigResult_T& operator = (LSCConfigResult_T&& o) = default;

    virtual ~LSCConfigResult_T()
    {
        if (rResultBuf)
            rResultBuf = nullptr;
    }

    // Get Id
    virtual int getId() const { return E_LSC_CONFIGRESULTTOISP; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of LSCConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

} //namespace NS3Av3

#endif  //__RESULT_POOL_4_LSCCONFIG_H__
