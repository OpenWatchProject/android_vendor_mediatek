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
#ifndef _ISP_MGR_RMG_H_
#define _ISP_MGR_RMG_H_

#include <cutils/properties.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RMG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef class ISP_MGR_RMG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_RMG    MyType;
private:
    MINT32 debugDump;

     /* RMG Perframe Table */

    MINT32 tuningTable[5][2] =
    { // Gain,  Ratio
        { // se/le ratio 1
            256, 16
        },
        { // se/le ratio 2
            128, 32
        },
        { // se/le ratio 4
            64, 64
        },
        { // se/le ratio 8
            32, 128
        },
        { // se/le ratio 16
            16, 256
        },
    };
    enum
    {
        IND_GAIN = 0,
        IND_RATIO = 1,
    };
    enum
    {
        IND_RATIO_1 = 0,
        IND_RATIO_2 = 1,
        IND_RATIO_4 = 2,
        IND_RATIO_8 = 3,
        IND_RATIO_16 = 4,
    };

    MBOOL m_bEnable; // RMG enable or not
#if 0
    MFLOAT mAeExpRatio;
#endif

    /*IVHDR*/
    MBOOL m_bIHdrEnable; //
    MUINT32 mLeFirst;

    /*ZVHDR*/
    MBOOL m_bZHdrEnable;
    MUINT32 mZPattern;

    MUINT32 m_u4StartAddr; // For debug address

    // TG1
    enum
    {
        ERegInfo_CAM_RMG_HDR_CFG,
        ERegInfo_CAM_RMG_HDR_GAIN,
        ERegInfo_CAM_RMG_HDR_CFG2,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_RMG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , debugDump(0)
        , m_bEnable(MFALSE)
#if 0
        , mAeExpRatio(1.0)
#endif
        , m_bIHdrEnable(MFALSE)
        , mLeFirst(0)
        , m_bZHdrEnable(MFALSE)
        , mZPattern(0)
        , m_u4StartAddr(REG_ADDR_P1(CAM_RMG_HDR_CFG))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_RMG_HDR_CFG);
        INIT_REG_INFO_ADDR_P1(CAM_RMG_HDR_GAIN);
        INIT_REG_INFO_ADDR_P1(CAM_RMG_HDR_CFG2);
    }

    virtual ~ISP_MGR_RMG() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    /*==== Set debugDump ======*/
    MVOID
    setDebugDump(MINT32 debug)
    {
        debugDump = debug;
    }

    /*==== Set RMG enable ===== */
    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    /*==== Set IVHDR or ZVHDR enable ======*/
    MBOOL
    isIHdrEnable()
    {
        return m_bIHdrEnable;
    }

    MVOID
    setIHdrEnable(MBOOL bEnable)
    {
        m_bIHdrEnable = bEnable;
        m_bZHdrEnable = (!m_bIHdrEnable && bEnable);
    }

    MBOOL
    isZHdrEnable()
    {
        return m_bZHdrEnable;
    }

    MVOID
    setZHdrEnable(MBOOL bEnable)
    {
        m_bZHdrEnable = bEnable;
        m_bIHdrEnable = (!m_bZHdrEnable && bEnable);
    }

    /*==== Set IVHDR or ZVHDR parameters ======*/
    MVOID
    setLeFirst(MUINT32 leFirst)
    {
        mLeFirst = leFirst;
    }

    MVOID
    setZPattern(MUINT32 pattern)
    {
        mZPattern = pattern;
    }

#if 0
    /*==== Set AE ratio */
    MVOID setAeLeSeRatio(MINT32 aeRatio); // aeRatio = (LE/SE) * 100
#endif


   // MBOOL apply(EIspProfile_T eIspProfile, TuningMgr& rTuning);
    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_RMG_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RMG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef class ISP_MGR_RMG2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_RMG2    MyType;
private:

     /* RMG2 Perframe Table */

    MINT32 tuningTable[5][2] =
    { // Gain,  Ratio
        { // se/le ratio 1
            256, 16
        },
        { // se/le ratio 2
            128, 32
        },
        { // se/le ratio 4
            64, 64
        },
        { // se/le ratio 8
            32, 128
        },
        { // se/le ratio 16
            16, 256
        },
    };
    enum
    {
        IND_GAIN = 0,
        IND_RATIO = 1,
    };
    enum
    {
        IND_RATIO_1 = 0,
        IND_RATIO_2 = 1,
        IND_RATIO_4 = 2,
        IND_RATIO_8 = 3,
        IND_RATIO_16 = 4,
    };

    MBOOL m_bEnable; // RMG enable or not
    MINT32 debugDump;
#if 0
    MFLOAT mAeExpRatio;
#endif

    /*IVHDR*/
    MBOOL m_bIHdrEnable; //
    MUINT32 mLeFirst;

    /*ZVHDR*/
    MBOOL m_bZHdrEnable;
    MUINT32 mZPattern;

    MUINT32 m_u4StartAddr; // For debug address

    // TG1
    enum
    {
        ERegInfo_DIP_X_RMG2_HDR_CFG,
        ERegInfo_DIP_X_RMG2_HDR_GAIN,
        ERegInfo_DIP_X_RMG2_HDR_CFG2,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_RMG2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MFALSE)
        , debugDump(0)
#if 0
        , mAeExpRatio(1.0)
#endif
        , m_bIHdrEnable(MFALSE)
        , mLeFirst(0)
        , m_bZHdrEnable(MFALSE)
        , mZPattern(0)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_RMG2_HDR_CFG))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_RMG2_HDR_CFG);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMG2_HDR_GAIN);
        INIT_REG_INFO_ADDR_P2(DIP_X_RMG2_HDR_CFG2);

    }

    virtual ~ISP_MGR_RMG2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    //template <class ISP_xxx_T>
    //MyType& get(ISP_xxx_T & rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    /*==== Set debugDump ======*/
    MVOID
    setDebugDump(MINT32 debug)
    {
        debugDump = debug;
    }

    /*==== Set RMG enable ===== */
    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    /*==== Set IVHDR or ZVHDR enable ======*/
    MBOOL
    isIHdrEnable()
    {
        return m_bIHdrEnable;
    }

    MVOID
    setIHdrEnable(MBOOL bEnable)
    {
        m_bIHdrEnable = bEnable;
        m_bZHdrEnable = (!m_bIHdrEnable && bEnable);
    }

    MBOOL
    isZHdrEnable()
    {
        return m_bZHdrEnable;
    }

    MVOID
    setZHdrEnable(MBOOL bEnable)
    {
        m_bZHdrEnable = bEnable;
        m_bIHdrEnable = (!m_bZHdrEnable && bEnable);
    }

    /*==== Set IVHDR or ZVHDR parameters ======*/
    MVOID
    setLeFirst(MUINT32 leFirst)
    {
        mLeFirst = leFirst;
    }

    MVOID
    setZPattern(MUINT32 pattern)
    {
        mZPattern = pattern;
    }

#if 0
    /*==== Set AE ratio */
    MVOID setAeLeSeRatio(MINT32 aeRatio); // aeRatio = (LE/SE) * 100
#endif


   // MBOOL apply(EIspProfile_T eIspProfile, TuningMgr& rTuning);
    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_RMG2_T;


#endif


