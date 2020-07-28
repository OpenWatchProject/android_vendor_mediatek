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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _ISP_PDO_CONFIG_H_
#define _ISP_PDO_CONFIG_H_

//
#include <ResultPoolBase.h>

//
using namespace NS3Av3;

//
/***************************************************
 * PDO stride should be 16 Byte align
 ***************************************************/
#define _PDO_STRIDE_ALIGN_(W) ({\
    MUINT32 _align = 0;\
    if(((W) % 16) != 0){\
        _align = ((W + 15)/16 * 16);\
    }\
    else{\
        _align = W;\
    }\
    _align;\
})
/***************************************************
 *
 ***************************************************/
typedef union pdo_en_ctrl_t
{
    struct
    {
        unsigned char  pdo_en :  1;
        unsigned char  pde_en :  1;
        unsigned char  pbn_en :  1;
        unsigned char  rsv    : 29;
    } Bits;
    MUINT32 Raw;
} PDO_EN_CTRL_T;

typedef struct isp_pdo_cfg_t : public ResultPoolBase_T
{
    //
    MINT32 sensor_idx;
    //
    PDO_EN_CTRL_T pdo_ctl;
    MUINT32       pdo_xsz;
    MUINT32       pdo_ysz;
    MUINT32       bit_depth;
    // for shield pd
    MUINT32  pdi_tbl_xsz;
    MUINT32  pdi_tbl_ysz;
    MUINTPTR pdi_tbl_pa;
    MUINTPTR pdi_tbl_va;
    MINT32   pdi_tbl_memID;
    //for dualpd
    MUINT8   pbn_type; /* [0]1x4, [1]4x4 */
    MUINT8   pbn_start_line; /* Can be [0], [1], [2], [3] */
    MUINT8   pbn_separate_mode;

    isp_pdo_cfg_t() :
        ResultPoolBase_T(),
        sensor_idx(0),
        pdo_xsz(0),
        pdo_ysz(0),
        bit_depth(0),
        pdi_tbl_xsz(0),
        pdi_tbl_ysz(0),
        pdi_tbl_pa(0),
        pdi_tbl_va(0),
        pdi_tbl_memID(0),
        pbn_type(0),
        pbn_start_line(0),
        pbn_separate_mode(0)
    {
        pdo_ctl.Raw = 0;
    }

    isp_pdo_cfg_t(MINT32 _sensor_idx) :
        ResultPoolBase_T(),
        sensor_idx(_sensor_idx),
        pdo_xsz(0),
        pdo_ysz(0),
        bit_depth(0),
        pdi_tbl_xsz(0),
        pdi_tbl_ysz(0),
        pdi_tbl_pa(0),
        pdi_tbl_va(0),
        pdi_tbl_memID(0),
        pbn_type(0),
        pbn_start_line(0),
        pbn_separate_mode(0)
    {
        pdo_ctl.Raw = 0;
    }


    virtual ~isp_pdo_cfg_t() = default;


    // copy operator
    isp_pdo_cfg_t& operator =(const isp_pdo_cfg_t &in)
    {
        sensor_idx        = in.sensor_idx       ;
        pdo_ctl.Raw       = in.pdo_ctl.Raw      ;
        pdo_xsz           = in.pdo_xsz          ;
        pdo_ysz           = in.pdo_ysz          ;
        bit_depth         = in.bit_depth        ;
        pdi_tbl_xsz       = in.pdi_tbl_xsz      ;
        pdi_tbl_ysz       = in.pdi_tbl_ysz      ;
        pdi_tbl_pa        = in.pdi_tbl_pa       ;
        pdi_tbl_va        = in.pdi_tbl_va       ;
        pdi_tbl_memID     = in.pdi_tbl_memID    ;
        pbn_type          = in.pbn_type         ;
        pbn_start_line    = in.pbn_start_line   ;
        pbn_separate_mode = in.pbn_separate_mode;
        return (*this);
    }

    // move operator
    isp_pdo_cfg_t& operator = (isp_pdo_cfg_t&& o) = default;


    //
    MVOID clearMember()
    {
        sensor_idx        = 0;
        pdo_ctl.Raw       = 0;
        pdo_xsz           = 0;
        pdo_ysz           = 0;
        bit_depth         = 0;
        pdi_tbl_xsz       = 0;
        pdi_tbl_ysz       = 0;
        pdi_tbl_pa        = 0;
        pdi_tbl_va        = 0;
        pdi_tbl_memID     = 0;
        pbn_type          = 0;
        pbn_start_line    = 0;
        pbn_separate_mode = 0;
    }

    // Get Id
    virtual int getId() const
    {
        return E_PDO_CONFIGRESULTTOISP;
    }


    //----------------------------------------------------------
    // override operator of ResultPoolBase_T
    //----------------------------------------------------------
    // copy operator
    virtual ResultPoolBase_T& operator =(const ResultPoolBase_T &o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of PDOConfigResult, it happens UB
        *this = *static_cast<const MY_TYPE*>(&o);
        return *this;
    }

    // move operator
    virtual ResultPoolBase_T& operator=(ResultPoolBase_T &&o) override
    {
        _check_id(o.getId());
        typedef std::remove_pointer<decltype(this)>::type MY_TYPE;

        // be careful, if o is not the type of PDOConfigResult, it happens UB
        *this = std::move(*static_cast<const MY_TYPE*>(&o));
        return *this;
    }

} ISP_PDO_CFG_T;
#endif // _ISP_PDO_CONFIG_H_
