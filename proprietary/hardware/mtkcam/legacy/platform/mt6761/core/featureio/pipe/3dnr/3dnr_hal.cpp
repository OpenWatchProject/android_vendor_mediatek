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

#define LOG_TAG "3dnr_hal"
//
#include <mtkcam/Log.h>
#include "3dnr_hal.h"
#include <camera_custom_3dnr.h>
#include <sys/resource.h>
#include <cutils/properties.h>  // For property_get().
#include <mtkcam/imageio/ispio_stddef.h> 	//for EPIPECmd_GET_NR3D_GAIN (remove after)
#include <mtkcam/ispio_sw_scenario.h>

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;

using namespace NSImageio;
using namespace NSImageio::NSIspio;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//
#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif


#define NR3D_FORCE_GMV_ZERO     0
#define NR3D_NO_HW_POWER_OFF    0



/*******************************************************************************
*
********************************************************************************/
static hal3dnrBase *pHal3dnr = NULL;
static MINT32 clientCnt = 0;

/*******************************************************************************
*
********************************************************************************/
hal3dnrBase*
Hal3dnr::
getInstance()
{
    clientCnt++;
    MY_LOGD("clientCnt:%d \n", clientCnt);

    if (pHal3dnr == NULL) {
        pHal3dnr = new Hal3dnr();
    }
    return pHal3dnr;
}

/*******************************************************************************
*
********************************************************************************/
Hal3dnr::Hal3dnr()
{
    mpIFeatureStream = NULL;
    mMaxInputImageSize = MSize(0,0);
    mPrevFrameWidth = 0;
    mPrevFrameHeight = 0;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;
    m3dnrGainZeroCount = 0;
    m3dnrErrorStatus = 0;
    m3dnrStateMachine = NR3D_STATE_STOP;
    mpNr3dParam = NULL;
}


/*******************************************************************************
*
********************************************************************************/
Hal3dnr::~Hal3dnr()
{

}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3dnr::destroyInstance()
{
    MY_LOGD("destroyInstance \n");
    if (clientCnt == 0 && pHal3dnr) {
        delete pHal3dnr;
        pHal3dnr = NULL;
    }
    clientCnt--;

    MY_LOGD("pHal3dnr=%p, clientCnt:%d \n", pHal3dnr, clientCnt);

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::init()
{
	FUNC_START;
	mpNr3dParam = new NR3D;

    mPrevFrameWidth         = 0;
    mPrevFrameHeight        = 0;
    m3dnrGainZeroCount      = 0;
    m3dnrErrorStatus        = NR3D_ERROR_NONE;
    m3dnrStateMachine       = NR3D_STATE_PREPARING;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;

	FUNC_END;
	return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::uninit()
{
	FUNC_START;
	delete mpNr3dParam; // Return allocated memory.

    mPrevFrameWidth         = 0;
    mPrevFrameHeight        = 0;
    m3dnrGainZeroCount      = 0;
    m3dnrErrorStatus        = NR3D_ERROR_NONE;
    m3dnrStateMachine       = NR3D_STATE_PREPARING;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;

	FUNC_END;
	return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::prepare(MUINT32 frameNo, MINT32 iso)
{
    FUNC_START;

    char aInputValue[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32  i4TempInputValue = 0;
    MUINT32 u4Img3oOffset_X  = 0;
    MUINT32 u4Img3oOffset_Y  = 0;
    MUINT32 eisFeBlock      = 0;
    MUINT32 u4Img3oWidth    = 0;
    MUINT32 u4Img3oHeight   = 0;
    MUINT32 u4VipiWidth     = 0;
    MUINT32 u4VipiHeight    = 0;
    MUINT32 u4VipiStartAddrOffset = 0;
    MSize imgSize, TempImgSize;

    /************************************************************************/
    /*                          Preprocessing                               */
    /************************************************************************/

    //*****************STEP 1
    //////////////////////////////////////////////////////////////////////////
    // 3DNR State Machine operation                                         //
    //////////////////////////////////////////////////////////////////////////
    if (m3dnrStateMachine == NR3D_STATE_PREPARING) // Last frame is NR3D_STATE_PREPARING.
    {
        m3dnrStateMachine = NR3D_STATE_WORKING;   // NR3D, IMG3O, VIPI all enabled.
    }
    MY_LOGD("STEP 1,2: m3dnrStateMachine=%d, frame:%d", m3dnrStateMachine, frameNo);
    //*****************STEP 2
    // Reset m3dnrErrorStatus.
    m3dnrErrorStatus = NR3D_ERROR_NONE;

    //*****************STEP 3
    //////////////////////////////////////////////////////////////////////////
    //  3DNR HW module on/off according Nr3dGain                            //
    //////////////////////////////////////////////////////////////////////////
    MUINT32 u43dnrHwPowerOffThreshold   = get_3dnr_hw_power_off_threshold();
    MUINT32 u43dnrHwPowerReopenDelay    = get_3dnr_hw_power_reopen_delay();
    MINT32 i4Nr3dGain = 0;
    MY_LOGD("u43dnrHwPowerOffThreshold=%d, u43dnrHwPowerReopenDelay=%d, m3dnrGainZeroCount=%d frame:%d",
        u43dnrHwPowerOffThreshold, u43dnrHwPowerReopenDelay, m3dnrGainZeroCount, frameNo);

    //mpIFeatureStream->sendCommand(EPIPECmd_GET_NR3D_GAIN, frameNo, (MINTPTR)&i4Nr3dGain);
    if (iso > NR3D_OFF_ISO_THRESHOLD)
    {
        if (m3dnrGainZeroCount > 0)       // limit at 0.
            m3dnrGainZeroCount--;
    }
    else    // i4Nr3dGain0 is 0.
    {
        m3dnrGainZeroCount++;
        if (m3dnrGainZeroCount > u43dnrHwPowerOffThreshold)       // Cap at u43dnrHwPowerOffThreshold.
            m3dnrGainZeroCount = u43dnrHwPowerOffThreshold;
    }

    #if NR3D_NO_HW_POWER_OFF
    m3dnrGainZeroCount = 0; // For m3dnrGainZeroCount to be 0, so it won't change m3dnrStateMachine.
    #endif  // NR3D_NO_HW_POWER_OFF

    if (m3dnrGainZeroCount >= u43dnrHwPowerOffThreshold)
    {
        m3dnrStateMachine = NR3D_STATE_STOP;
    }
    else if (m3dnrGainZeroCount > (u43dnrHwPowerOffThreshold - u43dnrHwPowerReopenDelay))
    {
        // StateMachine stays the same.
    }
    else    // (u43dnrHwPowerOffThreshold - u43dnrHwPowerReopenDelay) > m3dnrGainZeroCount > 0
    {
        if (m3dnrStateMachine == NR3D_STATE_STOP)
        {
            m3dnrStateMachine = NR3D_STATE_PREPARING;
            m3dnrGainZeroCount = 0; // Reset m3dnrGainZeroCount.
        }
    }
    MY_LOGD("STEP 3: StateMachine=%d, i4Nr3dGain=%d, Zero gain count=%d, Poweroff threshold=%d, frame:%d",
        m3dnrStateMachine, i4Nr3dGain, m3dnrGainZeroCount, u43dnrHwPowerOffThreshold, frameNo);

    FUNC_END;
    return MTRUE;
}


MBOOL
Hal3dnr::setGMV(MUINT32 frameNo, MINT32 gmvX, MINT32 gmvY, MINT32 cmvX_Int, MINT32 cmvY_Int)
{
    FUNC_START;
    char aInputValue[PROPERTY_VALUE_MAX] = {'\0'};
    //*****************STEP 4
    //////////////////////////////////////////////////////////////////////////
    //  3DNR GMV Calculation                                                //
    //////////////////////////////////////////////////////////////////////////
    MINT32 i4TempNmvXFromQueue = 0, i4TempNmvYFromQueue = 0;
    MINT32 i4TempX = 0, i4TempY = 0;


    i4TempNmvXFromQueue = (-(gmvX) / 256);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_w must be even, so make mNmvX even. Discussed with James Liang.
    i4TempNmvYFromQueue = (-(gmvY) / 256);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_h must be even when image format is 420, so make mNmvX even. Discussed with TC & Christopher.

    property_get("vendor.camera.3dnr.forcegmv.enable", aInputValue, "0");
    if (aInputValue[0] == '1')
    {
        property_get("vendor.camera.3dnr.forcegmv.x", aInputValue, "0");
        i4TempNmvXFromQueue = atoi(aInputValue);

        property_get("vendor.camera.3dnr.forcegmv.y", aInputValue, "0");
        i4TempNmvYFromQueue = atoi(aInputValue);

        LOG_DBG("Force GMV X/Y (%d, %d)", i4TempNmvXFromQueue, i4TempNmvYFromQueue);
    }

    #if (NR3D_FORCE_GMV_ZERO)   // Force GMV to be 0.
    mNmvX = 0;
    mNmvY = 0;
    #else   // Normal flow.
    property_get("vendor.camera.3dnr.usecmv.enable", aInputValue, "1");
    if (aInputValue[0] == '1')   // For EIS 1.2 (use CMV). gmv_crp (t) = gmv(t) - ( cmv(t) - cmv(t-1) )
    {
        // Use GMV and CMV
        mCmvX = cmvX_Int;    // Curr frame CMV X. Make it even.
        mCmvY = cmvY_Int;    // Curr frame CMV Y. Make it even.
        mNmvX = (i4TempNmvXFromQueue - (mCmvX - mPrevCmvX)) & ~1;    // Make it even.
        mNmvY = (i4TempNmvYFromQueue - (mCmvY - mPrevCmvY)) & ~1;    // Make it even.
        i4TempX = mCmvX - mPrevCmvX;
        i4TempY = mCmvY - mPrevCmvY;

        mPrevCmvX = mCmvX;                   // Recore last frame CMV X.
        mPrevCmvY = mCmvY;                   // Recore last frame CMV Y.

        // Use GMV only. (david 2015.07.03)
        //mNmvX = i4TempNmvXFromQueue & ~1;    // Make it even.
        //mNmvY = i4TempNmvYFromQueue & ~1;    // Make it even.
    }
    else   // For EIS 2.0 (use GMV)
    {
        // Use GMV only.
        mNmvX = i4TempNmvXFromQueue & ~1;    // Make it even.
        mNmvY = i4TempNmvYFromQueue & ~1;    // Make it even.

        mCmvX = 0;
        mCmvY = 0;
        mPrevCmvX = 0;                   // Recore last frame CMV X.
        mPrevCmvY = 0;                   // Recore last frame CMV Y.
    }
    #endif  // NR3D_FORCE_GMV_ZERO


    if ((abs(mNmvX) >= 28) || (abs(mNmvY) >= 28))
    {
        m3dnrErrorStatus |= NR3D_ERROR_GMV_TOO_LARGE;
        if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
    }

    MY_LOGD("STEP 4: gmv cal,ST=%d, gmv(x,y)=(%5d,%5d),CmvX/Y(%5d,%5d),NmvX/Y(%5d,%5d), (cmv diff %5d,%5d), frame:%d",
        m3dnrStateMachine, gmvX, gmvY, cmvX_Int, cmvY_Int, mNmvX, mNmvY, i4TempX, i4TempY, frameNo);

    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::checkIMG3OSize(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH)
{
    FUNC_START;

    //*****************STEP 5
    //////////////////////////////////////////////////////////////////////////
    //  Calculate target width/height to set IMG3O                          //
    //////////////////////////////////////////////////////////////////////////
    /* Calculate P2A output width and height */

    //david: in hal3 img3o size is the same to imgi in

    //*****************STEP 6
    // W/H of buffer (i.e. Current frame size) is determined, so check previous vs. current frame size for 3DNR.
    if ( (mPrevFrameWidth  != imgiW) || (mPrevFrameHeight != imgiH) )
    {
        m3dnrErrorStatus |= NR3D_ERROR_FRAME_SIZE_CHANGED;
        // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
        if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;
    }

    MY_LOGD("STEP 5: ,PrevFrameW/H(%d,%d),imgiW/H(%d,%d), frame:%d",
        mPrevFrameWidth, mPrevFrameHeight, imgiW, imgiH, frameNo);

    //*****************STEP 7
    //resize to real output size
    //david 3dnr only can skip this step
    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::setVipiParams(MBOOL isVIPIIn, MUINT32 vipiW, MUINT32 vipiH, MINT imgFormat, size_t stride)
{
    FUNC_START;
    char aInputValue[PROPERTY_VALUE_MAX] = {'\0'};

    if (isVIPIIn)
    {
        // Config VIPI for 3DNR previous frame input.
        MUINT32 u4VipiOffset_X = 0;
        MUINT32 u4VipiOffset_Y = 0;
        MUINT32 u4PixelToBytes = 0;
        MSize imgSize, tempImgSize;

        // Calculate VIPI Start Address = nmv_x + nmv_y * vipi_stride. Unit: bytes.
        //     Calculate Offset X/Y according to NMV X/Y.
        u4VipiOffset_X = (mNmvX >= 0) ? (mNmvX): (0);
        u4VipiOffset_Y = (mNmvY >= 0) ? (mNmvY): (0);
        //     Calculate u4PixelToBytes.
        if (imgFormat == eImgFmt_YUY2) u4PixelToBytes = 2;
        if (imgFormat == eImgFmt_YV12) u4PixelToBytes = 1;
        //     Calculate VIPI start addr offset.
        // u4VipiStartAddrOffset = u4VipiOffset_Y * minput.mBuffer->getBufStridesInBytes(0) + u4VipiOffset_X * u4PixelToBytes;
        mpNr3dParam->vipi_offst = u4VipiOffset_Y * stride + u4VipiOffset_X * u4PixelToBytes;    //in byte
        LOG_DBG("vipi offset=%d,(xy=%d,%d), stride=%d, u4PixelToBytes=%d",mpNr3dParam->vipi_offst, u4VipiOffset_X, u4VipiOffset_Y, stride, u4PixelToBytes);

        //     Calculate VIPI valid region w/h.
        tempImgSize.w = vipiW - abs(mNmvX);  // valid region w
        tempImgSize.h = vipiH - abs(mNmvY);  // valid region h
        imgSize.w = tempImgSize.w & ~1;  // valid region w
        imgSize.h = tempImgSize.h & ~1;  // valid region h

        mpNr3dParam->vipi_readW = imgSize.w;    //in pixel
        mpNr3dParam->vipi_readH = imgSize.h;    //in pixel

        //david for test, force set stateMachine to Wokring
        //m3dnrStateMachine = NR3D_STATE_WORKING;

        // Check whether current frame size is equal to last frame size.
        property_get("vendor.camera.3dnr.forceskip", aInputValue, "0");
        if (aInputValue[0] == '1')
        {
            m3dnrErrorStatus |= NR3D_ERROR_FORCE_SKIP;
            if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
        }

        if (m3dnrStateMachine == NR3D_STATE_WORKING)
        {
            //enqueParams.mvIn.push_back(minput);    // Only push_back VIPI when N3RD state machine is NR3D_STATE_WORKING.
            //LOG_DBG("push_back VIPI buffer.");

            LOG_DBG("[P2A sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn)",
                    m3dnrStateMachine,
                    m3dnrErrorStatus
            );
            mpNr3dParam->onSiz_onWd = imgSize.w  & ~1;   // Must be even.
            mpNr3dParam->onSiz_onHt = imgSize.h & ~1;
        }
        else    // Not NR3D_STATE_WORKING.
        {
            LOG_DBG("[P2A not sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn)",
                    m3dnrStateMachine,
                    m3dnrErrorStatus
            );
            mpNr3dParam->onSiz_onWd = 0;
            mpNr3dParam->onSiz_onHt = 0;
            return MFALSE;
        }
    }
    else
    {
        LOG_DBG("[P2A not sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn). m3dnrPrvFrmQueue is empty", m3dnrStateMachine, m3dnrErrorStatus); // m3dnrPrvFrmQueue is empty => maybe first run.
        mpNr3dParam->onSiz_onWd = 0;
        mpNr3dParam->onSiz_onHt = 0;
        if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;
        return MFALSE;
    }

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3D* &pNr3dParam)
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //update mpNr3dParam info
    if (m3dnrStateMachine == NR3D_STATE_WORKING)    // Only set NR3D register when N3RD state machine is NR3D_STATE_WORKING.
    {
        mpNr3dParam->ctrl_onEn = 1;
        //david modified, u4Img3oOffset_X can skip because tile driver doesn't have limitation about crop region.
        mpNr3dParam->onOff_onOfStX = ((mNmvX >= 0) ? (0) : (-mNmvX));   // Must be even.
        mpNr3dParam->onOff_onOfStY = ((mNmvY >= 0) ? (0) : (-mNmvY));
    }
    else
    {
        mpNr3dParam->ctrl_onEn      = 1;
        mpNr3dParam->onOff_onOfStX  = 0;   // Must be even.
        mpNr3dParam->onOff_onOfStY  = 0;
        mpNr3dParam->onSiz_onWd     = 0;   // Must be even.
        mpNr3dParam->onSiz_onHt     = 0;
        mpNr3dParam->vipi_offst     = 0;    //in byte
        mpNr3dParam->vipi_readW     = 0;    //in pixel
        mpNr3dParam->vipi_readH     = 0;    //in pixel
        ret = MFALSE;
    }
    LOG_DBG("3dnrSM2(%d S0P1W2),ES(0x%02X FsSzDfLrIn),NmvX/Y(%d, %d),onOfX/Y(%d, %d).onW/H(%d, %d).VipiOff/W/H(%d, %d, %d).MaxIsoInc(%d)",
        m3dnrStateMachine,
        m3dnrErrorStatus,
        mNmvX, mNmvY,
        mpNr3dParam->onOff_onOfStX, mpNr3dParam->onOff_onOfStY,
        mpNr3dParam->onSiz_onWd, mpNr3dParam->onSiz_onHt,
        mpNr3dParam->vipi_offst, mpNr3dParam->vipi_readW, mpNr3dParam->vipi_readH,
        get_3dnr_max_iso_increase_percentage()
    );

    pNr3dParam = mpNr3dParam;
    // Recordng mPrevFrameWidth/mPrevFrameHeight for next frame.
    mPrevFrameWidth  = imgiW;
    mPrevFrameHeight = imgiH;
    FUNC_END;
    return ret;
}

MBOOL
Hal3dnr::checkStateMachine(NR3D_STATE_ENUM status)
{
    return (status == m3dnrStateMachine);
}
