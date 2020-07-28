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
* @file ResultPoolBase.h
* @brief Declarations of structure of module Manager
*/
#ifndef __RESULT_POOL_BASE_H__
#define __RESULT_POOL_BASE_H__
#include <type_traits>

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Modules need to add enum for new structure.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    //0-8 Enum for 3A HAL
    //For result convert to meta, which has 8 basic enum need to convert.
    E_HAL_RESULTTOMETA = 0,
    E_AE_RESULTTOMETA,
    E_AF_RESULTTOMETA,
    E_AWB_RESULTTOMETA,
    E_ISP_RESULTTOMETA,
    E_LSC_RESULTTOMETA,
    E_FLASH_RESULTTOMETA,
    E_FLK_RESULTTOMETA,
    E_NUM_RESULTTOMETA, // 8
    E_LCSO_RESULTTOMETA,

    //Other Enum for modules. Please define below.
    E_LSC_CONFIGRESULTTOISP,
    E_AF_CONFIGRESULTTOISP,
    E_AWB_CONFIGRESULTTOISP,
    E_FLK_CONFIGRESULTTOISP,
    E_AE_CONFIGRESULTTOISP,
    E_CCU_RESULTINFO4OVERWRITE,
    E_AWB_RESULTINFO4ISP,
    E_PDO_CONFIGRESULTTOISP,
    E_AE_RESULTINFO,
    E_NUM_RESULT_END // The end of enum
} E_PARTIAL_RESULT_OF_MODULE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Module base struct
//  For modules. Module struct need to inherit base struct
//  Modules need to implement operator(=). Be careful dynamic/static array.
//  Reference isp_af_config.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define RESULTPOOL_BASE_CHECK_TYPE 1
struct ResultPoolBase_T
{
    virtual MVOID clearMember(){}

    // Must define copy / move operator
    virtual ResultPoolBase_T& operator = (const ResultPoolBase_T& o) = default;
    virtual ResultPoolBase_T& operator = (ResultPoolBase_T&& o) = default;

    virtual ~ResultPoolBase_T() = default;

    // Get Id
    virtual int getId() const { return -1; }

protected:
    inline void _check_id(int id __attribute__((unused)))
    {
#if RESULTPOOL_BASE_CHECK_TYPE
        if (__builtin_expect(getId() != id, false)) {
            // invoke NE (null pointer dereference),
            // and set r0 to Bit[16:31]: id of lhs, Bit[0:15]: id of rhs for debug
            *(volatile uint32_t*)(0x0) = (getId() << 16) | id;
        }
#endif
    }
};

} //namespace NS3Av3

#endif  //__RESULT_POOL_BASE_H__
