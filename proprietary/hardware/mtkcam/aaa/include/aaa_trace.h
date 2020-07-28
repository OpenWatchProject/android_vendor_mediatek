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

#ifndef _AAA_TRACE_H_
#define _AAA_TRACE_H_

#include <cutils/properties.h>

extern int AAA_TRACE_LEVEL;    // declared in Hal3AAdapter.cpp

/*#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}*/

//static MINT32 AAA_TRACE_LEVEL;    //http://stackoverflow.com/questions/92546/variable-declarations-in-header-files-static-or-not
//static void setTraceLevel( MINT32 level) { AAA_TRACE_LEVEL = level; }


//GET_PROP("vendor.debug.hal3av3.systrace", 0, AAA_TRACE_LEVEL);

//ISP 4.X
// D : default, G : global, L : local
#define AAA_TRACE_D(fmt, arg...)    do{ CAM_TRACE_FMT_BEGIN(fmt, ##arg); } while(0)
#define AAA_TRACE_G(VAR)    do{ if(AAA_TRACE_LEVEL == 1) CAM_TRACE_FMT_BEGIN("G_"#VAR); } while(0)
#define AAA_TRACE_NG(VAR)   do{ if(AAA_TRACE_LEVEL == 1) CAM_TRACE_FMT_BEGIN("NG_"#VAR); } while(0)    // Negative Global
#define AAA_TRACE_L(VAR)    do{ if(AAA_TRACE_LEVEL == 2) CAM_TRACE_FMT_BEGIN("L_"#VAR); } while(0)
#define AAA_TRACE_NL(VAR)   do{ if(AAA_TRACE_LEVEL == 2) CAM_TRACE_FMT_BEGIN("NL_"#VAR); } while(0)        // Negative Local
#define AAA_TRACE_S(VAR)    do{ if(AAA_TRACE_LEVEL == 3) CAM_TRACE_FMT_BEGIN("S_"#VAR); } while(0)        // subtle
#define AAA_TRACE_NS(VAR)   do{ if(AAA_TRACE_LEVEL == 3) CAM_TRACE_FMT_BEGIN("NS_"#VAR); } while(0)    // outside subtle

#define AAA_TRACE_END_D     do{ CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_G     do{ if(AAA_TRACE_LEVEL == 1) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_NG    do{ if(AAA_TRACE_LEVEL == 1) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_L     do{ if(AAA_TRACE_LEVEL == 2) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_NL    do{ if(AAA_TRACE_LEVEL == 2) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_S     do{ if(AAA_TRACE_LEVEL == 3) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_NS    do{ if(AAA_TRACE_LEVEL == 3) CAM_TRACE_FMT_END(); } while(0)

// ISP 5.0
// New method
#define AAA_TRACE_D(fmt, arg...)    do{ CAM_TRACE_FMT_BEGIN(fmt, ##arg); } while(0)
#define AAA_TRACE_HAL(VAR)          do{ if( ((AAA_TRACE_LEVEL>>0)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("HAL_"#VAR); } while(0)
#define AAA_TRACE_MGR(VAR)          do{ if( ((AAA_TRACE_LEVEL>>1)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("MGR_"#VAR); } while(0)
#define AAA_TRACE_ISP(VAR)          do{ if( ((AAA_TRACE_LEVEL>>2)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("ISP_"#VAR); } while(0)
#define AAA_TRACE_LSC(VAR)          do{ if( ((AAA_TRACE_LEVEL>>3)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("LSC_"#VAR); } while(0)
#define AAA_TRACE_ALG(VAR)          do{ if( ((AAA_TRACE_LEVEL>>4)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("ALG_"#VAR); } while(0)
#define AAA_TRACE_DRV(VAR)          do{ if( ((AAA_TRACE_LEVEL>>5)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("DRV_"#VAR); } while(0)
#define AAA_TRACE_MW(VAR)           do{ if( ((AAA_TRACE_LEVEL>>6)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("MW_"#VAR);  } while(0)
#define AAA_TRACE_CCU(VAR)           do{ if( ((AAA_TRACE_LEVEL>>7)&0x1) == 1 ) CAM_TRACE_FMT_BEGIN("CCU_"#VAR);  } while(0)


#define AAA_TRACE_END_D             do{ CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_HAL           do{ if( ((AAA_TRACE_LEVEL>>0)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_MGR           do{ if( ((AAA_TRACE_LEVEL>>1)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_ISP           do{ if( ((AAA_TRACE_LEVEL>>2)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_LSC           do{ if( ((AAA_TRACE_LEVEL>>3)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_ALG           do{ if( ((AAA_TRACE_LEVEL>>4)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_DRV           do{ if( ((AAA_TRACE_LEVEL>>5)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_MW            do{ if( ((AAA_TRACE_LEVEL>>6)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)
#define AAA_TRACE_END_CCU            do{ if( ((AAA_TRACE_LEVEL>>7)&0x1) == 1 ) CAM_TRACE_FMT_END(); } while(0)




/*
MVOID aaaTraceDefault(fmt, ##arg)
{
    if(arg[0])
        CAM_TRACE_FMT_BEGIN(fmt);
    else
        CAM_TRACE_FMT_END();

}

MVOID aaaTraceGlobal(fmt, ##arg)
{

}

MVOID aaaTraceLocal(fmt, ##arg)
{

}*/





#endif //_AAA_TRACE_H_

