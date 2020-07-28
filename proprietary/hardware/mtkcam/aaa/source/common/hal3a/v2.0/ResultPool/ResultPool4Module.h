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
* @file ResultPool4Module.h
* @brief Declarations of structure of module Manager
*/
#ifndef __RESULT_POOL_4_MODULE_H__
#define __RESULT_POOL_4_MODULE_H__

#include <ResultPoolBase.h>

// TODO : This need to remove
//#include <awb_param.h>

#include <tuning/ae_flow_custom.h>
#include <tuning/awb_flow_custom.h>


namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Modules Info to modules structure
//  For modules. Not to update to "Result to Meta structure".
//  The struct format is xxxResultInfo_T.
//  Modules need to update all structure. Don't update one member of structure.
//  If modules want to update one member, need to define new structure which has one member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=============================CCUResult=============================//
struct CCUResultInfo_T : ResultPoolBase_T
{
    MUINT32 u4Rto;
    MUINT32 u4P1DGNGain;

    CCUResultInfo_T()
        : u4Rto(0)
        , u4P1DGNGain(0)
    {
    }

    MVOID clearMember()
    {
        CCUResultInfo_T();
    }

    // copy operator
    virtual CCUResultInfo_T& operator = (const CCUResultInfo_T& ) = default;

    // move operator
    virtual CCUResultInfo_T& operator = (CCUResultInfo_T&& ) = default;

    // destructor
    virtual ~CCUResultInfo_T() = default;

    // Get Id
    virtual int getId() const { return E_CCU_RESULTINFO4OVERWRITE; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of CCUResultInfo_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of CCUResultInfo_T, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};


//=============================AWBInfoResult=============================//
struct AWBResultInfo_T : ResultPoolBase_T
{
    AWB_ISP_INFO_T AWBInfo4ISP;

    AWBResultInfo_T()
    {
        ::memset(&AWBInfo4ISP, 0, sizeof(AWBInfo4ISP));
        AWBInfo4ISP.rPregain1.i4R = 512;
        AWBInfo4ISP.rPregain1.i4G = 512;
        AWBInfo4ISP.rPregain1.i4B = 512;
    }

    MVOID clearMember()
    {
        AWBResultInfo_T();
    }

    // copy operator of AWBResultInfo_T
    AWBResultInfo_T& operator = (const AWBResultInfo_T& o)
    {
        ::memcpy(&(this->AWBInfo4ISP), &(o.AWBInfo4ISP), sizeof(AWB_ISP_INFO_T));
        return *this;
    }

    // move operator of AWBResultInfo_T
    virtual AWBResultInfo_T& operator = (AWBResultInfo_T&& ) = default;

    // destructor
    virtual ~AWBResultInfo_T() = default;

    // Get Id
    virtual int getId() const { return E_AWB_RESULTINFO4ISP; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of AWBResultInfo_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of AWBResultInfo_T, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};

//=============================AEInfoResult=============================//
struct AEResultInfo_T : ResultPoolBase_T
{
    AE_PERFRAME_INFO_T AEPerframeInfo;

    AEResultInfo_T()
    {
        ::memset(&AEPerframeInfo, 0, sizeof(AE_PERFRAME_INFO_T));
    }

    MVOID clearMember()
    {
        AEResultInfo_T();
    }

    // copy operator of AEResultInfo_T
    AEResultInfo_T& operator = (const AEResultInfo_T& o)
    {
        ::memcpy(&(this->AEPerframeInfo), &(o.AEPerframeInfo), sizeof(AE_PERFRAME_INFO_T));
        return *this;
    }

    // move operator of AEResultInfo_T
    virtual AEResultInfo_T& operator = (AEResultInfo_T&& ) = default;

    // destructor
    virtual ~AEResultInfo_T() = default;

    // Get Id
    virtual int getId() const { return E_AE_RESULTINFO; }

    //----------------------------------------------------------
    // re-implement of ResultPoolBase_T
    //----------------------------------------------------------
    // override copy operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T& o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of AEResultInfo_T, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // override move operator of ResultPoolBase_T
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T && o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of AEResultInfo_T, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));

        return *this;
    }
};


} //namespace NS3Av3

#endif  //__RESULT_POOL_4_MODULE_H__
