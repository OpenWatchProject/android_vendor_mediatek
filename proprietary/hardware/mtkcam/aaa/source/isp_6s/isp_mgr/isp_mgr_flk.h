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
#ifndef _ISP_MGR_FLK_H_
#define _ISP_MGR_FLK_H_

#include <aaa/flicker_param.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FLK statistics
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_FLK_REG_INFO_ADDR_P1_MULTI(index,reg, name)\
    REG_INFO_ADDR_MULTI(index,name##reg) = REG_ADDR_P1(name##reg)

#define INIT_FLKO_ADDR(reg) INIT_FLK_REG_INFO_ADDR_P1_MULTI(EFLK_R1 ,reg, FLKO_R1_);

#define INIT_FLK_ADDR(reg) INIT_FLK_REG_INFO_ADDR_P1_MULTI(EFLK_R1 ,reg, FLK_R1_);

#define REG_FLK_R1_INFO_VALUE(reg) REG_INFO_VALUE_MULTI(EFLK_R1, reg)


typedef class ISP_MGR_FLK_CONFIG : public ISP_MGR_BASE_T
{
private:
    enum
    {
        EFLK_R1,
        ESubModule_NUM
    };
    mutable std::mutex m_Lock;
    RegInfo_T m_rIspRegInfo[ESubModule_NUM][EFLKRegInfo_NUM];
	int isEnableFLK;

protected:
    ISP_MGR_FLK_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, EFLKRegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for (MUINT32 i = 0; i < ESubModule_NUM; i++)
        {
            ::memset(m_rIspRegInfo[i],    0, sizeof(RegInfo_T)*EFLKRegInfo_NUM);
        }

        INIT_FLK_ADDR(FLK_CON);
        INIT_FLK_ADDR(FLK_OFST);
        INIT_FLK_ADDR(FLK_SIZE);
        INIT_FLK_ADDR(FLK_NUM);
        INIT_FLK_ADDR(FLK_SGG_GAIN);
        INIT_FLK_ADDR(FLK_SGG_GMR1);
        INIT_FLK_ADDR(FLK_SGG_GMR2);
        INIT_FLK_ADDR(FLK_SGG_GMR3);
        INIT_FLK_ADDR(FLK_SGG_GMR4);
        INIT_FLK_ADDR(FLK_SGG_GMR5);
        INIT_FLK_ADDR(FLK_SGG_GMR6);
        INIT_FLK_ADDR(FLK_SGG_GMR7);
        INIT_FLK_ADDR(FLK_ZHDR);

        INIT_FLKO_ADDR(FLKO_XSIZE);
        INIT_FLKO_ADDR(FLKO_YSIZE);
        INIT_FLKO_ADDR(FLKO_STRIDE);

		isEnableFLK = MTRUE;
    }

    virtual ~ISP_MGR_FLK_CONFIG() {}

public: ////
    static ISP_MGR_FLK_CONFIG& getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces
    MVOID configReg(FLKResultConfig_T *pResultConfig);
    MBOOL apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex = 0);

}ISP_MGR_FLK_CONFIG_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_FLK_CONFIG_DEV : public ISP_MGR_FLK_CONFIG_T
{
public:
    static
    ISP_MGR_FLK_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_FLK_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_FLK_CONFIG_DEV()
        : ISP_MGR_FLK_CONFIG_T(eSensorDev)
    {}

    virtual ~ISP_MGR_FLK_CONFIG_DEV() {}

};

#endif

