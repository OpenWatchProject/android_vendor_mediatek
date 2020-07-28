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
#ifndef _ISP_MGR_AWB_STAT_H_
#define _ISP_MGR_AWB_STAT_H_
/* Dynamic Bin */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AWB statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AWB_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AWB_STAT_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr;
    MINT32 m_i4SensorIndex;

    enum
    {
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
    };
    
    RegInfo_T   m_rIspRegInfo[ERegInfo_CAM_AWB_NUM];
    AWB_GAIN_T  m_rIspAWBPreGain1; // 1x = 512
    MUINT32 m_u4AAOmode;
    MUINT32 m_bDebugEnable;
    MUINT32 m_imgW;
    MUINT32 m_imgH;
    MUINT32 m_blkW;
    MUINT32 m_blkH;
protected:
    ISP_MGR_AWB_STAT_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_CAM_AWB_NUM, eSensorDev)      
        , m_i4SensorIndex(0)
    {
    // register info addr init
    #if 0
        INIT_REG_INFO_ADDR_P1(CAM_AWB_WIN_ORG);    // CAM+0x05B0
        INIT_REG_INFO_ADDR_P1(CAM_AWB_WIN_SIZE);   // CAM+0x05B4
        INIT_REG_INFO_ADDR_P1(CAM_AWB_WIN_PIT);    // CAM+0x05B8
        INIT_REG_INFO_ADDR_P1(CAM_AWB_WIN_NUM);    // CAM+0x05BC
        INIT_REG_INFO_ADDR_P1(CAM_AWB_GAIN1_0);    // CAM+0x05C0
        INIT_REG_INFO_ADDR_P1(CAM_AWB_GAIN1_1);    // CAM+0x05C4
        INIT_REG_INFO_ADDR_P1(CAM_AWB_LMT1_0);     // CAM+0x05C8
        INIT_REG_INFO_ADDR_P1(CAM_AWB_LMT1_1);     // CAM+0x05CC
        INIT_REG_INFO_ADDR_P1(CAM_AWB_LOW_THR);    // CAM+0x05D0
        INIT_REG_INFO_ADDR_P1(CAM_AWB_HI_THR);     // CAM+0x05D4
        INIT_REG_INFO_ADDR_P1(CAM_AWB_PIXEL_CNT0); // CAM+0x05D8
        INIT_REG_INFO_ADDR_P1(CAM_AWB_PIXEL_CNT1); // CAM+0x05DC
        INIT_REG_INFO_ADDR_P1(CAM_AWB_PIXEL_CNT2); // CAM+0x05E0
        INIT_REG_INFO_ADDR_P1(CAM_AWB_ERR_THR);    // CAM+0x05E4
        INIT_REG_INFO_ADDR_P1(CAM_AWB_ROT);        // CAM+0x05E8
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L0_X);       // CAM+0x05EC
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L0_Y);       // CAM+0x05F0
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L1_X);       // CAM+0x05F4
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L1_Y);       // CAM+0x05F8
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L2_X);       // CAM+0x05FC
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L2_Y);       // CAM+0x0600
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L3_X);       // CAM+0x0604
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L3_Y);       // CAM+0x0608
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L4_X);       // CAM+0x060C
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L4_Y);       // CAM+0x0610
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L5_X);       // CAM+0x0614
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L5_Y);       // CAM+0x0618
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L6_X);       // CAM+0x061C
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L6_Y);       // CAM+0x0620
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L7_X);       // CAM+0x0624
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L7_Y);       // CAM+0x0628
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L8_X);       // CAM+0x062C
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L8_Y);       // CAM+0x0630
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L9_X);       // CAM+0x0634
        INIT_REG_INFO_ADDR_P1(CAM_AWB_L9_Y);       // CAM+0x0638
        INIT_REG_INFO_ADDR_P1(CAM_AWB_SPARE);     
        INIT_REG_INFO_ADDR_P1(CAM_AWB_MOTION_THR);
        //RWB
        INIT_REG_INFO_ADDR_P1(CAM_AWB_RC_CNV_0);
        INIT_REG_INFO_ADDR_P1(CAM_AWB_RC_CNV_1);
        INIT_REG_INFO_ADDR_P1(CAM_AWB_RC_CNV_2);
        INIT_REG_INFO_ADDR_P1(CAM_AWB_RC_CNV_3);
        INIT_REG_INFO_ADDR_P1(CAM_AWB_RC_CNV_4);
        // PSO
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_WIN_ORG);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_WIN_SIZE);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_WIN_PIT);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_WIN_NUM);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_PIXEL_CNT0);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_PIXEL_CNT1);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_PIXEL_CNT2);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AWB_PIXEL_CNT3);
	#else
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_WIN_ORG);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_WIN_SIZE);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_WIN_PIT);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_WIN_NUM);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_GAIN1_0);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_GAIN1_1);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_VALID_DATA_WIDTH);	 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LMT1_0); 			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LMT1_1); 			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LMT1_2); 			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LOW_THR_0);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LOW_THR_1);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LOW_THR_2);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_HI_THR_0);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_HI_THR_1);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_HI_THR_2);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LIGHTSRC_LOW_THR_0);  
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LIGHTSRC_LOW_THR_1);  
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LIGHTSRC_HI_THR_0);	 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_LIGHTSRC_HI_THR_1);	 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_PIXEL_CNT0); 		 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_PIXEL_CNT1); 		 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_PIXEL_CNT2); 		 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_ERR_THR);			 
//	    INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE);
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_FORMAT_SHIFT);		 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_POSTGAIN_0); 		 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_POSTGAIN_1); 		 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_ROT);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L0_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L0_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L1_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L1_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L2_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L2_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L3_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L3_Y);							   
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L4_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L4_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L5_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L5_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L6_X);							 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L6_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L7_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L7_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L8_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L8_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L9_X);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_L9_Y);				 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_RC_CNV_0);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_RC_CNV_1);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_RC_CNV_2);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_RC_CNV_3);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AWB_RC_CNV_4);			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AE_GAIN2_0); 			 
        INIT_REG_INFO_ADDR_P1(AA_R1_AA_AE_GAIN2_1);

	#endif
        m_rIspAWBPreGain1.i4R = m_rIspAWBPreGain1.i4G = m_rIspAWBPreGain1.i4B = 512;
        m_u4AAOmode = 0;
        m_bDebugEnable = 0;
        m_imgW = 0;
        m_imgH = 0;
        m_blkW = 0;
        m_blkH = 0;
		m_u4StartAddr = 0;
    }

    virtual ~ISP_MGR_AWB_STAT_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

#if 0
    MVOID setIspAEGain(MINT32 i4IspAEGain)
    {
        m_i4IspAEGain = i4IspAEGain;
    }

    MVOID getIspAEGain(MUINT32 *u4IspAEGain)
    {
        *u4IspAEGain = m_i4IspAEGain;
    }
#endif

public: //    Interfaces
    //MBOOL config(AWB_STAT_CONFIG_T& rAWBStatConfig, MBOOL bHBIN2Enable, int imgW, int imgH, int blkW, int blkH);
    MBOOL apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
    //MBOOL SetNonBayer(MUINT32 rCscCCM[9]);
	MVOID configReg(MVOID *pIspRegInfo);

	//MBOOL setIspAWBPreGain1(AWB_GAIN_T& rIspAWBGain);

    MVOID getIspAWBPreGain1(AWB_GAIN_T* u4IspAWBGain)
    {
        *u4IspAWBGain = m_rIspAWBPreGain1;
    }
    MBOOL SetAAOMode(MUINT32 u4AAOmode);
    //MBOOL reconfig( MVOID *pDBinInfo, MVOID *pOutRegCfg);
    //MBOOL Cbreconfig(AWB_STAT_CONFIG_T& rAWBStatConfig, AWB_OUTPUT_T& m_rAWBOutput, MUINT32 m_i4FlareGain, MUINT32 m_i4FlareOffset, MVOID *pDBinInfo, MVOID *pOutRegCfg);
} ISP_MGR_AWB_STAT_CONFIG_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_AWB_STAT_CONFIG_DEV : public ISP_MGR_AWB_STAT_CONFIG_T
{
public:
    static
    ISP_MGR_AWB_STAT_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AWB_STAT_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AWB_STAT_CONFIG_DEV()
        : ISP_MGR_AWB_STAT_CONFIG_T(eSensorDev)
    {}

    virtual ~ISP_MGR_AWB_STAT_CONFIG_DEV() {}

};

#endif

