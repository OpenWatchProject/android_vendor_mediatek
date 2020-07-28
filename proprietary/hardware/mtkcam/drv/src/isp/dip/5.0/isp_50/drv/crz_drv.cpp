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
#define LOG_TAG "CrzDrv"
//
#include <utils/Errors.h>
//#include <cutils/log.h>   //

#include "camera_sysram.h"  // For SYSRAM.
//#include <cutils/pmem.h>
//#include <cutils/memutil.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <sys/ioctl.h>
//#include "CameraProfile.h"  // For CPTLog*() CameraProfile APIS.
//
//#include "imageio_types.h"    // For type definitions.
#include <mtkcam/def/BuiltinTypes.h>    // For type definitions.
#include <dip_reg.h>    // For ISP register structures.
//#include <asm/arch/sync_write.h> // For dsb() in isp_drv.h.
#include "isp_drv_dip.h"
#include "isp_drv_dip_phy.h"
#include "crz_drv_imp.h"

#include <cutils/properties.h>  // For property_get().
//tpipe
#include <tpipe_config.h>


#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

//#undef   DBG_LOG_LEVEL                      // Decide Debug Log level for current file. Can only choose from 2~8.
//#define  DBG_LOG_LEVEL      2               // 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(crz_drv);
//EXTERN_DBG_LOG_VARIABLE(crz_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (crz_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (crz_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (crz_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (crz_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (crz_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (crz_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define VRZ_EXIST   0
#define RSP_EXIST   0
#define VRZO_EXIST  0
#define DISPO_HAS_ROTATE    0


class CrzDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    CrzDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
        if  (0==mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

	    //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};


#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 0   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 1   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   CrzDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   CrzDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT



#if 1 //_mt6593fpga_dvt_use_

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static MUINT32 CrzDrvRzUint[CRZ_DRV_ALGO_AMOUNT] =
{
    32768,
    1048576,
    1048576
};
#if 0
[1~32768] : Table 27
(32768:36409) : Table 20
[36409:40961) : Table 21
[40961:46812) : Table 22
[46812:54614) : Table 23
[46812:59579) : Table 24
[59579:65537) : Table 25
[65537~!U) : Table 26

#endif
static MUINT32 CrzDrvRzTable[CRZ_DRV_RZ_TABLE_AMOUNT] =
{
    36409,
    40961,
    46812,
    54614,
    59579,
    65537
};

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/











//-----------------------------------------------------------------------------
CrzDrvImp::CrzDrvImp()
    : mInitCount(0),
      mSysramUsageCount(0),
      m_pIspDrv(0x0),
      m_pPhyIspDrv(0x0),
      m_pPhyIspReg(0x0),
      mFdSysram(0),
      m_fgIsGdmaMode(0),
      m_fgVertFirst(MFALSE)
{
    MUINT32 i, j;
    GLOBAL_PROFILING_LOG_START(Event_CrzDrv);	// Profiling Start.

    //
    LOG_INF("");
    //
    crz_setting = {
        .cdrz_input_crop_width = 0,
        .cdrz_input_crop_height = 0,
        .cdrz_output_width = 0,
        .cdrz_output_height = 0,
        .cdrz_luma_horizontal_integer_offset = 0,
        .cdrz_luma_horizontal_subpixel_offset =0,
        .cdrz_luma_vertical_integer_offset = 0,
        .cdrz_luma_vertical_subpixel_offset = 0,
        .cdrz_horizontal_luma_algorithm = 0,
        .cdrz_vertical_luma_algorithm = 0,
        .cdrz_horizontal_coeff_step = 0,
        .cdrz_vertical_coeff_step = 0
    };
}


//-----------------------------------------------------------------------------
CrzDrvImp::~CrzDrvImp()
{
    LOG_INF("");
    GLOBAL_PROFILING_LOG_END(); 	// Profiling End.
}


//-----------------------------------------------------------------------------
CrzDrv* CrzDrv::CreateInstance()
{
    DBG_LOG_CONFIG(imageio, crz_drv);

    //LOG_INF("fgIsGdmaMode: %d.", fgIsGdmaMode);

    return CrzDrvImp::GetInstance();
}


//-----------------------------------------------------------------------------
CrzDrv* CrzDrvImp::GetInstance(bool fgIsGdmaMode)
{
    fgIsGdmaMode;
    static CrzDrvImp Singleton;
    Singleton.m_fgIsGdmaMode = MFALSE;

    return &Singleton;
}


//-----------------------------------------------------------------------------
void CrzDrvImp::DestroyInstance(void)
{
    LOG_INF("");
}

//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::Init(void)
{

    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_CrzDrv_Init);

    //
    Mutex::Autolock lock(mLock);    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    LOG_INF(" - E. mInitCount: %d. mSysramUsageCount: %d. ", mInitCount, mSysramUsageCount);


    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        LOCAL_PROFILING_LOG_PRINT("atomic_inc");
        goto lbEXIT;
    }

    // Increase CDR Drv count.
    android_atomic_inc(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_inc");

lbEXIT:
    LOG_INF(" - X. Result: %d. mInitCount: %d. mFdSysram: %d. mSysramUsageCount: %d.", Result, mInitCount, mFdSysram, mSysramUsageCount);

    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;

}


//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::Uninit(void)
{

    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_CrzDrv_Uninit);

    //
    Mutex::Autolock lock(mLock);

    LOG_INF(" - E. mInitCount: %d.", mInitCount);

    if(mInitCount <= 0)
    {
        // No more users
        goto EXIT;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_dec");

    if(mInitCount > 0)    // If there are still users, exit.
    {
        goto EXIT;
    }



EXIT:
    LOG_INF(" - X. Result: %d. mInitCount: %d. mFdSysram: %d. mSysramUsageCount: %d.", Result, mInitCount, mFdSysram, mSysramUsageCount);

    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;

}




MBOOL CrzDrvImp::SetIspDrv(DipDrv* pIspDrv)
{
    LOG_DBG("- E. pIspDrv: 0x%lx.", (unsigned long)pIspDrv);

    if (mInitCount <= 0)
    {
        LOG_ERR("No CRZ Drv. Please init one first!");
        return MFALSE;
    }

    if (pIspDrv == NULL)
    {
        LOG_ERR("pIspDrv is NULL.");
        return MFALSE;
    }

    m_pIspDrv = pIspDrv;

	return MTRUE;
}



///////////////////////////////////////////////////////////////////////////
/// @brief Check if CDR Drv is ready.
///
/// 1. Check CRZ Drv instance count. If count <= 0, it means no CRZ Drv.
/// 2. Check if m_pIspDrv is ready.
///
/// @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
///////////////////////////////////////////////////////////////////////////
MBOOL CrzDrvImp::CheckReady(void)
{
    // 1. Check CRZ Drv instance count. If count <= 0, it means no CRZ Drv.
    if (mInitCount <= 0)
    {
        LOG_ERR("No more CRZ Drv user. Please init one first!");
        return MFALSE;
    }

    // 2. Check if m_pIspDrv is ready.
    if (m_pIspDrv == NULL)
    {
        LOG_ERR("m_pIspDrv is NULL.");
        return MFALSE;
    }

    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::CalAlgoAndCStep(
    CRZ_DRV_MODE_ENUM       eFrameOrTpipeOrVrmrg,
    CRZ_DRV_RZ_ENUM         eRzName,
    MUINT32                 SizeIn_H,
    MUINT32                 SizeIn_V,
    MUINT32                 u4CroppedSize_H,
    MUINT32                 u4CroppedSize_V,
    MUINT32                 SizeOut_H,
    MUINT32                 SizeOut_V,
    CRZ_DRV_ALGO_ENUM       *pAlgo_H,
    CRZ_DRV_ALGO_ENUM       *pAlgo_V,
    MUINT32                 *pTable_H,
    MUINT32                 *pTable_V,
    MUINT32                 *pCoeffStep_H,
    MUINT32                 *pCoeffStep_V
)
{
    MUINT32 Mm1_H, Mm1_V, Nm1_H, Nm1_V;
    MUINT32 u4SupportingWidth_4tap  = 0;
    MUINT32 u4SupportingWidth_4Ntap = 0;
    MUINT32 u4SupportingWidth_Ntap  = 0;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 u4SizeToBeChecked_H = 0;
    MUINT32 u4SizeToBeChecked_V = 0;

    //
    LOG_DBG("- E. Mode: %d. eRzName: %d. In/Crop/Out: (%d, %d)/(%d, %d)/(%d, %d).", eFrameOrTpipeOrVrmrg, eRzName, SizeIn_H, SizeIn_V, u4CroppedSize_H, u4CroppedSize_V, SizeOut_H, SizeOut_V);

#if 0
    // Check if CRZ Drv is ready.
    if (!CheckReady())
    {
        Result = MFALSE;
        LOG_ERR("[Error]CheckReady fail");
        goto lbExit;
    }
#endif

    // Calculate Mm1 = u4CroppedSize - 1.
    Mm1_H = u4CroppedSize_H - 1;
    Mm1_V = u4CroppedSize_V - 1;
    // Calculate Nm1 = SizeOut - 1.
    Nm1_H = SizeOut_H - 1;
    Nm1_V = SizeOut_V - 1;

    // Decide Supporting Width.
    switch (eFrameOrTpipeOrVrmrg)
    {
        case CRZ_DRV_MODE_TPIPE:
        {
            switch (eRzName)
            {
                case CRZ_DRV_RZ_CRZ:
                u4SupportingWidth_4tap  = CRZ_DRV_SUPPORT_WIDTH_TPIPE_CRZ_4_TAP;
                u4SupportingWidth_4Ntap = CRZ_DRV_SUPPORT_WIDTH_TPIPE_CRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CRZ_DRV_SUPPORT_WIDTH_TPIPE_CRZ_N_TAP;
                break;

                default:
                    LOG_ERR("Not support eRzName. eRzName: %d.", eRzName);
                    Result = MFALSE;
                    goto lbExit;
            }
        }
        break;

        default:
            LOG_ERR("Not support CRZ_DRV_MODE_VRMRG. CRZ_DRV_MODE_FRAME: %d.", eFrameOrTpipeOrVrmrg);
            Result = MFALSE;
            goto lbExit;
    }

    //     Calculate Algo/CoeffStep/Table.
    //horizontal first when resizer to small, vertial first when resizer to big according the joesphi lai's suppestions.
    m_fgVertFirst = MFALSE;
	
    if ((u4CroppedSize_H  * CRZ_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_H &&  
        (SizeOut_H * CRZ_DRV_RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_H )
// 4-tap    
//	(u4CroppedSize_H  * CRZ_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_H && 	   // 32x >= (Ratio = SizeOut/u4CroppedSize).
//	(SizeOut_H * CRZ_DRV_RZ_4_TAP_RATIO_MIN) >	u4CroppedSize_H 		   // (Ratio = u4CroppedSize/SizeIn) > 1/2.
    {
        if (((u4CroppedSize_H  * CRZ_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_H) &&
             ((u4CroppedSize_H  * CRZ_DRV_RZ_4_TAP_RATIO_MAX) <= SizeOut_H))
        {
            m_fgVertFirst = MTRUE;
        }
		
        //LOG_INF("Using 4_TAP: Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4tap);

        //     Decide Algorithm.
        *pAlgo_H = CRZ_DRV_ALGO_6_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Mm1_H * CrzDrvRzUint[*pAlgo_H] + (Nm1_H >> 1)) / Nm1_H);
        //     Find Table.
        
        if ((*pCoeffStep_H) <= 32768)
        {
            (*pTable_H) = 27;
        }
        else
        {
            for ((*pTable_H) = CRZ_DRV_RZ_TABLE_OFFSET; (*pTable_H) < (CRZ_DRV_RZ_TABLE_OFFSET+CRZ_DRV_RZ_TABLE_AMOUNT); (*pTable_H)++)
            {
                if ((*pCoeffStep_H) <= CrzDrvRzTable[(*pTable_H) - CRZ_DRV_RZ_TABLE_OFFSET])
                {
                    //(*pTable_H) = (*pTable_H) + CRZ_DRV_RZ_TABLE_OFFSET;
                    break;
                }
            }
			// When Table exceed CRZ_DRV_RZ_TABLE_AMOUNT, use last table.
			if ((*pTable_H) >= (CRZ_DRV_RZ_TABLE_OFFSET+CRZ_DRV_RZ_TABLE_AMOUNT))
			{
				(*pTable_H) = 26;
			}
        }

    }
    else if ((SizeOut_H * CRZ_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_H && 
		(SizeOut_H * CRZ_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_H )
    {
// 4N-tap    
//	(SizeOut_H * CRZ_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_H &&    // (Ratio = SizeOut/u4CroppedSize) <= 1/2.	 //Vent@20120627: Joseph Lai suggests that when ratio is 1/2, accumulation should be used (i.e. 4n-tap).
//	(SizeOut_H * CRZ_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_H	   // (Ratio = SizeOut/u4CroppedSize) >= 1/64.

        #if 0
        //     Decide Algorithm.
        *pAlgo_H = CRZ_DRV_ALGO_4N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Nm1_H * CrzDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
        //     Find Table.
        (*pTable_H) = CRZ_DRV_RZ_4N_TAP_TABLE;
        #else
        //     Decide Algorithm.
        *pAlgo_H = CRZ_DRV_ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Nm1_H * CrzDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
        //     Find Table.
        (*pTable_H) = CRZ_DRV_RZ_N_TAP_TABLE;
        #endif

    }
    else if ((SizeOut_H * CRZ_DRV_RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_H && 
		(SizeOut_H * CRZ_DRV_RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_H) 
    {
// N-tap    
//    (SizeOut_H * CRZ_DRV_RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_H &&     // (Ratio = SizeOut/SizeIn) < 1/64.
//    (SizeOut_H * CRZ_DRV_RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_H)       // (Ratio = SizeOut/SizeIn) >= 1/256.
    
        //     Decide Algorithm.
        *pAlgo_H = CRZ_DRV_ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Nm1_H * CrzDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
        //     Find Table.
        (*pTable_H) = CRZ_DRV_RZ_N_TAP_TABLE;
    }
    else    // Ratio out of range.
    {
        LOG_ERR("[Error]Not support ratio. u4CroppedSize_H: %d, SizeOut_H: %d.", u4CroppedSize_H, SizeOut_H);
        Result = MFALSE;
        goto lbExit;
    }


    // Calculate vertical part.
    //     Calculate Algo/CoeffStep/Table.
    if ( ((u4CroppedSize_V  * CRZ_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_V && 
            (SizeOut_V * CRZ_DRV_RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_V ))
    {
// 4-tap
//(u4CroppedSize_V  * CRZ_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_V &&        // 32x >= (Ratio = SizeOut/u4CroppedSize).
//(SizeOut_V * CRZ_DRV_RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_V            // (Ratio = u4CroppedSize/SizeIn) > 1/2.
        //     Decide Algorithm.
        *pAlgo_V = CRZ_DRV_ALGO_6_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Mm1_V * CrzDrvRzUint[*pAlgo_V] + (Nm1_V >> 1)) / Nm1_V);
        //     Find Table.
        if ((*pCoeffStep_V) <= 32768)
        {
            (*pTable_V) = 27;
        }
        else
        {
            for ((*pTable_V) = CRZ_DRV_RZ_TABLE_OFFSET; (*pTable_V) < (CRZ_DRV_RZ_TABLE_OFFSET+CRZ_DRV_RZ_TABLE_AMOUNT); (*pTable_V)++)
            {
                if ((*pCoeffStep_V) <= CrzDrvRzTable[(*pTable_V) - CRZ_DRV_RZ_TABLE_OFFSET])
                {
                    //(*pTable_H) = (*pTable_H) + CRZ_DRV_RZ_TABLE_OFFSET;
                    break;
                }
            }
			
			// When Table exceed CRZ_DRV_RZ_TABLE_AMOUNT, use last table.
			if ((*pTable_V) >= (CRZ_DRV_RZ_TABLE_OFFSET+CRZ_DRV_RZ_TABLE_AMOUNT))
			{
				(*pTable_V) = 26;
			}

        }
    }
    else if ((SizeOut_V * CRZ_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_V &&   
        (SizeOut_V * CRZ_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_V )
    {
// 4N-tap
//(SizeOut_V * CRZ_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_V &&    // (Ratio = SizeOut/u4CroppedSize) <= 1/2.    //Vent@20120627: Joseph Lai suggests that when ratio is 1/2, accumulation should be used (i.e. 4n-tap).
//(SizeOut_V * CRZ_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_V       // (Ratio = SizeOut/u4CroppedSize) >= 1/64.
        #if 0
        //     Decide Algorithm.
        *pAlgo_V = CRZ_DRV_ALGO_4N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Nm1_V * CrzDrvRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
        //     Find Table.
        (*pTable_V) = CRZ_DRV_RZ_4N_TAP_TABLE;
        #else
        //     Decide Algorithm.
        *pAlgo_V = CRZ_DRV_ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Nm1_V * CrzDrvRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
        //     Find Table.
        (*pTable_V) = CRZ_DRV_RZ_N_TAP_TABLE;
        #endif

    }
    else if ((SizeOut_V * CRZ_DRV_RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_V && 
        (SizeOut_V * CRZ_DRV_RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_V) 
    {
// N-tap
//(SizeOut_V * CRZ_DRV_RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_V &&     // (Ratio = SizeOut/SizeIn) < 1/64.
//(SizeOut_V * CRZ_DRV_RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_V)       // (Ratio = SizeOut/SizeIn) >= 1/256.
        //     Decide Algorithm.
        *pAlgo_V = CRZ_DRV_ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Nm1_V * CrzDrvRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
        //     Find Table.
        (*pTable_V) = CRZ_DRV_RZ_N_TAP_TABLE;
    }
    else    // Ratio out of range.
    {
        LOG_ERR("[Error]Not support ratio. u4CroppedSize_V: %d, SizeOut_V: %d.", u4CroppedSize_V, SizeOut_V);
        Result = MFALSE;
        goto lbExit;
    }

lbExit:

    //
    LOG_DBG("- X. Result: %d. Algo(%d, %d) Table(%d, %d) CoeffStep(%d, %d).", Result, *pAlgo_H, *pAlgo_V, *pTable_H, *pTable_V, *pCoeffStep_H, *pCoeffStep_V);
    //
    return Result;

}


//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::CalOffset(
    CRZ_DRV_ALGO_ENUM   Algo,
    MBOOL               IsWidth,
    MUINT32             CoeffStep,
    MFLOAT              Offset,
    MUINT32*            pLumaInt,
    MUINT32*            pLumaSub,
    MUINT32*            pChromaInt,
    MUINT32*            pChromaSub
)
{
    MUINT32 OffsetInt, OffsetSub;
    //
    LOG_DBG("- E. Algo: %d, IsWidth: %d, CoeffStep: %d, Offset: %f.", Algo, IsWidth, CoeffStep, Offset);

    // Check if CRZ Drv is ready.
    if (!CheckReady())
    {
        return MFALSE;
    }

    //
    OffsetInt = floor(Offset);
    OffsetSub = CrzDrvRzUint[Algo] * (Offset - floor(Offset));

    // Calculate pChromaInt/pChromaSub according current algorithm.
    if (Algo == CRZ_DRV_ALGO_6_TAP)
    {
        *pLumaInt = OffsetInt;
        *pLumaSub = OffsetSub;

        //
        if (IsWidth)
        {
            *pChromaInt = floor(Offset / (2.0)); // Because format is YUV422, so the width of chroma is half of Y.
            *pChromaSub = CrzDrvRzUint[Algo] * 2 * (Offset / (2.0) - floor(Offset / (2.0)));
        }
        else
        {
            *pChromaInt = (*pLumaInt);
            *pChromaSub = (*pLumaSub);
        }
    }
    else
    {
        *pLumaInt = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / CrzDrvRzUint[Algo]) / CrzDrvRzUint[Algo];
        *pLumaSub = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / CrzDrvRzUint[Algo]) % CrzDrvRzUint[Algo];
        *pChromaInt = (*pLumaInt);
        *pChromaSub = (*pLumaSub);
    }

    //
    LOG_DBG("- X. LumaInt/Sub(%d, %d), ChromaInt/Sub(%d, %d).", *pLumaInt, *pLumaSub, *pChromaInt, *pChromaSub);
    //
    return MTRUE;
}

//-------------------------------------------------------------------------
MBOOL CrzDrvImp::Reset(void)
{
    LOG_DBG("");

    if (!CheckReady())
    {
        return MFALSE;
    }

	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CTL_YUV_EN, CRZ_EN,MFALSE);
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::ResetDefault(void)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }    
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_CONTROL, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_IN_IMG, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_OUT_IMG, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_HORI_STEP, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_VERT_STEP, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_LUMA_HORI_INT_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_LUMA_HORI_SUB_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_LUMA_VERT_INT_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_LUMA_VERT_SUB_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_CHRO_HORI_INT_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_CHRO_HORI_SUB_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_CHRO_VERT_INT_OFST, 0x00000000);
	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_CHRO_VERT_SUB_OFST, 0x00000000);
//	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_DER_1, 0x00000000);
//	DIP_WRITE_REG(m_pIspDrv,DIP_X_CRZ_DER_2, 0x00000000);
    //
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::DumpReg(void)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    LOG_DBG("CONTROL                            = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_CONTROL));
    LOG_DBG("INPUT_IMAGE                        = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_IN_IMG));
    LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_OUT_IMG));
    LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_HORI_STEP));
    LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_VERT_STEP));
    LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_LUMA_HORI_INT_OFST));
    LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_LUMA_HORI_SUB_OFST));
    LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_LUMA_VERT_INT_OFST));
    LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_LUMA_VERT_SUB_OFST));
    LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_CHRO_HORI_INT_OFST));
    LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_CHRO_HORI_SUB_OFST));
    LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_CHRO_VERT_INT_OFST));
    LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",DIP_READ_REG(m_pIspDrv,DIP_X_CRZ_CHRO_VERT_SUB_OFST));
//    LOG_DBG("DERING_1                           = 0x%08X",DIP_READ_REG( m_pIspDrv,DIP_X_CRZ_DER_1));
//    LOG_DBG("DERING_2                           = 0x%08X",DIP_READ_REG( m_pIspDrv,DIP_X_CRZ_DER_2));
    //
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::CDRZ_Config(
    CRZ_DRV_MODE_ENUM           eFrameOrTileOrVrmrg,
    CRZ_DRV_IMG_SIZE_STRUCT     SizeIn,
    CRZ_DRV_IMG_SIZE_STRUCT     SizeOut,
    CRZ_DRV_IMG_CROP_STRUCT     Crop)
{
    CRZ_DRV_ALGO_ENUM Algo_H, Algo_V;
    MUINT32 Table_H, Table_V;
    MUINT32 CoeffStep_H, CoeffStep_V;
    MUINT32 LumaInt_H,LumaInt_V;
    MUINT32 LumaSub_H,LumaSub_V;
    MUINT32 ChromaInt_H,ChromaInt_V;
    MUINT32 ChromaSub_H,ChromaSub_V;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    
    LOG_DBG("In(%d, %d). Crop(%d, %d). Out(%d, %d). CropStart(%f, %f).",SizeIn.Width, SizeIn.Height, Crop.Width.Size, Crop.Height.Size, SizeOut.Width, SizeOut.Height, Crop.Width.Start, Crop.Height.Start);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
	
    //Width
    if(Crop.Width.Size == 0)
    {
        Crop.Width.Start = 0;
        Crop.Width.Size = SizeIn.Width;
    }
    else if( Crop.Width.Start < 0 || Crop.Width.Size > SizeIn.Width ||
        (Crop.Width.Start+Crop.Width.Size) > SizeIn.Width)
    {
        LOG_ERR("Error crop. InWidth: %d, Crop.Width::Start: %f, Size: %d.", SizeIn.Width, Crop.Width.Start, Crop.Width.Size);
        return MFALSE;
    }
	
    //Height
    if(Crop.Height.Size == 0)
    {
        Crop.Height.Start = 0;
        Crop.Height.Size = SizeIn.Height;
    }
    else if( Crop.Height.Start < 0 || Crop.Height.Size > SizeIn.Height ||
        (Crop.Height.Start+Crop.Height.Size) > SizeIn.Height)
    {
        LOG_ERR("Error crop. InHeight: %d. Crop.Height::Start: %f, Size: %d.",SizeIn.Height, Crop.Height.Start, Crop.Height.Size);
        return MFALSE;
    }

    Result = CalAlgoAndCStep(
                eFrameOrTileOrVrmrg,
                CRZ_DRV_RZ_CRZ,
                SizeIn.Width,
                SizeIn.Height,
                Crop.Width.Size,
                Crop.Height.Size,
                SizeOut.Width,
                SizeOut.Height,
                &Algo_H,
                &Algo_V,
                &Table_H,
                &Table_V,
                &CoeffStep_H,
                &CoeffStep_V
             );

    if (!Result)
    {
        LOG_ERR("[Error]CalAlgoAndCStep fail.");
        return MFALSE;
    }

    if (!CalOffset(Algo_H, MTRUE, CoeffStep_H, Crop.Width.Start, &LumaInt_H, &LumaSub_H, &ChromaInt_H, &ChromaSub_H))
    {
        LOG_ERR("CalOffset H fail.");
        return MFALSE;
    }

    if (!CalOffset(Algo_V, MFALSE, CoeffStep_V, Crop.Height.Start, &LumaInt_V, &LumaSub_V, &ChromaInt_V, &ChromaSub_V))
    {
        LOG_ERR("CalOffset V fail.");
        return MFALSE;
    }

    // check value
    if((Table_H > CRZ_DRV_MASK_TABLE_SELECT)||(Table_V > CRZ_DRV_MASK_TABLE_SELECT))
    {
        LOG_ERR("Table(%d) is out of %d",Table_H,CRZ_DRV_MASK_TABLE_SELECT);
        LOG_ERR("Table(%d) is out of %d",Table_V,CRZ_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }
	
    if((SizeIn.Width > CRZ_DRV_MASK_IMAGE_SIZE)||(SizeOut.Width > CRZ_DRV_MASK_IMAGE_SIZE)
	   ||(SizeIn.Height > CRZ_DRV_MASK_IMAGE_SIZE)||(SizeOut.Height > CRZ_DRV_MASK_IMAGE_SIZE))
    {
        LOG_ERR("SizeIn Width(%d) is out of %d",SizeIn.Width,CRZ_DRV_MASK_IMAGE_SIZE);
        LOG_ERR("SizeOut Width(%d) is out of %d",SizeOut.Width,CRZ_DRV_MASK_IMAGE_SIZE);
        LOG_ERR("SizeIn Height(%d) is out of %d",SizeIn.Height,CRZ_DRV_MASK_IMAGE_SIZE);
        LOG_ERR("SizeOut Height(%d) is out of %d",SizeOut.Height,CRZ_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
	
    if((CoeffStep_H > CRZ_DRV_MASK_COEFF_STEP)||(CoeffStep_V > CRZ_DRV_MASK_COEFF_STEP))
    {
        LOG_ERR("CoeffStep H(%d) is out of %d",CoeffStep_H,CRZ_DRV_MASK_COEFF_STEP);
        LOG_ERR("CoeffStep V(%d) is out of %d",CoeffStep_V,CRZ_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }
	
    if((LumaInt_H > CRZ_DRV_MASK_INT_OFFSET || ChromaInt_H > CRZ_DRV_MASK_INT_OFFSET)
		||(LumaSub_H > CRZ_DRV_MASK_SUB_OFFSET || ChromaSub_H > CRZ_DRV_MASK_SUB_OFFSET)
		||(LumaInt_V > CRZ_DRV_MASK_INT_OFFSET || ChromaInt_V > CRZ_DRV_MASK_INT_OFFSET)
		||(LumaSub_V > CRZ_DRV_MASK_SUB_OFFSET || ChromaSub_V > CRZ_DRV_MASK_SUB_OFFSET))
    {
        LOG_ERR("LumaInt H(%d) or ChromaInt H(%d) is out of %d",LumaInt_H,ChromaInt_H,CRZ_DRV_MASK_INT_OFFSET);
        LOG_ERR("LumaSub H(%d) or ChromaSub H(%d) is out of %d",LumaSub_H,ChromaSub_H,CRZ_DRV_MASK_SUB_OFFSET);
        LOG_ERR("LumaInt V(%d) or ChromaInt V(%d) is out of %d",LumaInt_V,ChromaInt_V,CRZ_DRV_MASK_INT_OFFSET);
        LOG_ERR("LumaSub V(%d) or ChromaSub V(%d) is out of %d",LumaSub_V,ChromaSub_V,CRZ_DRV_MASK_SUB_OFFSET);

		return MFALSE;
    }

	//HW Setting
	//Width
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_HORI_EN, MTRUE);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_HORI_ALGO, Algo_H);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_TRNC_HORI, CRZ_DRV_DEFAULT_TRUNC_BIT);	
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_HORI_TBL_SEL, Table_H);	
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_IN_IMG, CRZ_IN_WD, SizeIn.Width);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_OUT_IMG, CRZ_OUT_WD, SizeOut.Width);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_HORI_STEP, CRZ_HORI_STEP, CoeffStep_H);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_LUMA_HORI_INT_OFST,CRZ_LUMA_HORI_INT_OFST, LumaInt_H);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_LUMA_HORI_SUB_OFST,CRZ_LUMA_HORI_SUB_OFST, LumaSub_H);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CHRO_HORI_INT_OFST,CRZ_CHRO_HORI_INT_OFST, ChromaInt_H);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CHRO_HORI_SUB_OFST,CRZ_CHRO_HORI_SUB_OFST, ChromaSub_H);
	
    //Height
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_VERT_EN, MTRUE);	
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_VERT_FIRST, m_fgVertFirst);	
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_VERT_ALGO, Algo_V);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_TRNC_VERT, CRZ_DRV_DEFAULT_TRUNC_BIT);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CONTROL, CRZ_VERT_TBL_SEL, Table_V);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_IN_IMG, CRZ_IN_HT,SizeIn.Height);	
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_OUT_IMG, CRZ_OUT_HT, SizeOut.Height);
    DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_VERT_STEP, CRZ_VERT_STEP, CoeffStep_V);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_LUMA_VERT_INT_OFST,CRZ_LUMA_VERT_INT_OFST, LumaInt_V);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_LUMA_VERT_SUB_OFST,CRZ_LUMA_VERT_SUB_OFST, LumaSub_V);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CHRO_VERT_INT_OFST,CRZ_CHRO_VERT_INT_OFST, ChromaInt_V);
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CRZ_CHRO_VERT_SUB_OFST,CRZ_CHRO_VERT_SUB_OFST, ChromaSub_V);
	
	
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CTL_YUV_EN, CRZ_EN,MTRUE);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CrzDrvImp::CDRZ_Unconfig(void)
{
    LOG_DBG("");
    if(!CheckReady())
    {
        return MFALSE;
    }
	DIP_WRITE_BITS(m_pIspDrv,DIP_X_CTL_YUV_EN, CRZ_EN,MFALSE);
    return MTRUE;
}



#endif

//-----------------------------------------------------------------------------

