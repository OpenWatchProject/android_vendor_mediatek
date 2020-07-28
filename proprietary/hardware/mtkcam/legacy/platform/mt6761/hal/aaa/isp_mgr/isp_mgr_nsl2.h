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
#ifndef _ISP_MGR_NSL2_H_
#define _ISP_MGR_NSL2_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NSL2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NSL2    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4F40
    MBOOL m_bNSL2AOnOff;
    MBOOL m_bNSL2BOnOff;
    MUINT32 m_NSL2width;
    MUINT32 m_NSL2height;
    
    enum
    {
        ERegInfo_CAM_NSL2_CEN, 
        ERegInfo_CAM_NSL2_RR_CON0,
        ERegInfo_CAM_NSL2_RR_CON1,
        ERegInfo_CAM_NSL2_GAIN, 
        ERegInfo_CAM_NSL2_RZ, 
        ERegInfo_CAM_NSL2_XOFF,
        ERegInfo_CAM_NSL2_YOFF, 
        ERegInfo_CAM_NSL2_SLP_CON0,
        ERegInfo_CAM_NSL2_SLP_CON1,
        ERegInfo_CAM_NSL2_SIZE,      
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_NSL2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(isEnableNSL2(eSensorDev) ? MTRUE : MFALSE)
        , m_u4StartAddr(REG_ADDR(CAM_NSL2_CEN))
        , m_bNSL2AOnOff(MTRUE)
        , m_bNSL2BOnOff(MTRUE)
        , m_NSL2width(0)
        , m_NSL2height(0)
    {
        INIT_REG_INFO_ADDR(CAM_NSL2_CEN); 
        INIT_REG_INFO_ADDR(CAM_NSL2_RR_CON0);
        INIT_REG_INFO_ADDR(CAM_NSL2_RR_CON1);
        INIT_REG_INFO_ADDR(CAM_NSL2_GAIN); 
        INIT_REG_INFO_ADDR(CAM_NSL2_RZ); 
        INIT_REG_INFO_ADDR(CAM_NSL2_XOFF);
        INIT_REG_INFO_ADDR(CAM_NSL2_YOFF); 
        INIT_REG_INFO_ADDR(CAM_NSL2_SLP_CON0);
        INIT_REG_INFO_ADDR(CAM_NSL2_SLP_CON1);
        INIT_REG_INFO_ADDR(CAM_NSL2_SIZE); 
    }

    virtual ~ISP_MGR_NSL2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    
    MBOOL getNSL2AOnOff()
    {
        return m_bNSL2AOnOff;
    }

    MBOOL getNSL2BOnOff()
    {
        return m_bNSL2BOnOff;
    }

    MVOID
    setNSL2Size(MUINT32 width, MUINT32 height)
    {
        m_NSL2width  = width;
        m_NSL2height = height;
    }


    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MVOID
    setNSL2AEnable(MBOOL bEnable)
    {
        m_bNSL2AOnOff = bEnable;
    }

    MVOID
    setNSL2BEnable(MBOOL bEnable)
    {
        m_bNSL2BOnOff = bEnable;
    }
    
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);


    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);

} ISP_MGR_NSL2_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_NSL2_DEV : public ISP_MGR_NSL2_T
{
public:
    static
    ISP_MGR_NSL2_T&
    getInstance()
    {
        static ISP_MGR_NSL2_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_NSL2_DEV()
        : ISP_MGR_NSL2_T(eSensorDev)
    {}

    virtual ~ISP_MGR_NSL2_DEV() {}

};



#endif

