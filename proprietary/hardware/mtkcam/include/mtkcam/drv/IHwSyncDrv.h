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

#ifndef _HWSYNC_DRV_H_
#define _HWSYNC_DRV_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/module/module.h>

typedef enum{
    E_SMI_LOW = 0,  //dvfs at low
    E_SMI_LEVEL_1 = 1,
    E_SMI_LEVEL_2 = 2,
    E_SMI_HIGH =3,  // dvfs at high
}E_SMI_LEVEL;


/******************************************************************************
 *
 * @enum HW_SYNC_CMD_ENUM
 * @brief cmd enum for sendCommand.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_CMD_SET_MODE        = 0x01,         //sensor mode(by resolution and fps)
    HW_SYNC_CMD_DISABLE_SYNC    = 0x02,         //stop hwsync syncing precedure
    HW_SYNC_CMD_ENABLE_SYNC     = 0x03,         //start hwsync syncing precedure
    HW_SYNC_CMD_GET_FRAME_TIME  = 0x04,         // AE set AE cycle period
    HW_SYNC_CMD_SET_PARA        = 0x05,         //set hwsync init para, need to send before hwsync enable
    HW_SYNC_CMD_UNDATE_CLOCK    = 0x06,         //For Legacy HW Sync: update clock base (control by stereo adapter)
    HW_SYNC_CMD_SYNC_TYPE       = 0x07,         //get hwsync sync type
}HW_SYNC_CMD_ENUM;


/******************************************************************************
 *
 * @enum HW_SYNC_USER_ENUM
 * @brief the user enum for using hwsync driver.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_USER_HALSENSOR      = 0x00,
    HW_SYNC_USER_AE             = 0x01,
    HW_SYNC_USER_STEREO_ADAPTER = 0x02, //For Legacy HW Sync
}HW_SYNC_USER_ENUM;

typedef enum{
    HW_SYNC_BLOCKING_EXE        = 0x0,
    HW_SYNC_NON_BLOCKING_EXE    = 0x1,
}ENUM_HW_SYNC_EXE;

/******************************************************************************
 *
 * @enum HW_SYNC_CLOCK_STAGE
 * @brief For Legacy HW Sync, the clock stage user could adjust
 * @details
 *
 ******************************************************************************/
typedef enum{
        HW_SYNC_CLOCK_STAGE_0   = 0x00,
        HW_SYNC_CLOCK_STAGE_1   = 0x01,
}HW_SYNC_CLOCK_STAGE_ENUM;

/******************************************************************************
 *
 * @enum HW_SYNC_TYPE
 * @brief sync type enum
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_TYPE_VER_1      = 0x01,
    HW_SYNC_TYPE_VER_2      = 0x02,
    HW_SYNC_TYPE_SENSOR     = 0x03,
    HW_SYNC_TYPE_CCU        = 0x04,
    HW_SYNC_TYPE_ASG        = 0x05,
}HW_SYNC_TYPE_ENUM;

/******************************************************************************
 *
 * @class HWSyncDrv BaseClass
 * @brief Driver to sync VDs.
 * @details
 *
 ******************************************************************************/
class HWSyncDrv
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    HWSyncDrv(){};
    virtual ~HWSyncDrv() {};
//
public:
    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static HWSyncDrv*   createInstance();
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void        destroyInstance(void) = 0;
    virtual MBOOL       init(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx, MINT32 aeCyclePeriod = -1) = 0;
    virtual MBOOL       uninit(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx) = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
        cmd:HW_SYNC_CMD_SET_PARA
            @in
            arg1: sensor's devID
            arg2: dvfs level
            arg3: sensor's shutter speed
        cmd:HW_SYNC_CMD_ENABLE_SYNC
            @in
            arg2: sensor's devID
            arg3: HW_SYNC_BLOCKING_EXE->blocked enable flow.  HW_SYNC_NON_BLOCKING_EXE->non-blocked enable flow.
        cmd:HW_SYNC_CMD_DISABLE_SYNC
            @in
            arg1: 0->blocked disable flow.  1->non-blocked disable flow.
            arg2: sebsir's devID
        cmd:HW_SYNC_CMD_GET_FRAME_TIME
            @in
            arg1: current sensor's devID
            arg2: 2 sensors' shutter speed , order is decided by arg1.
            @out
            arg3: 2 sensors' frame length, order is decided by arg1.
        cmd:HW_SYNC_CMD_SET_MODE
            @in:
            arg1: sensor's devID
            arg2: sensor's scenarioID
            arg3: sensor's fps
        cmd:HW_SYNC_CMD_UNDATE_CLOCK
            @in:
            arg1: sensor's devID
            arg2: sensor's scenarioID
            arg3: sensor's fps
            arg4: isp clock
        cmd:HW_SYNC_CMD_UNDATE_CLOCK
            @out:
            arg2: current hwsync type
    */
    virtual MINT32      sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 arg1,MUINTPTR arg2, MUINTPTR arg3=0x0, MUINTPTR arg4=0x0)=0;
};


/**
 * @brief The definition of the maker of HWSyncDrv instance.
 */
typedef HWSyncDrv* (*HWSyncDrv_FACTORY_T)();
#define MAKE_HWSyncDrv(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_DRV_HW_SYNC_DRV, HWSyncDrv_FACTORY_T, __VA_ARGS__)


//----------------------------------------------------------------------------
#endif  // _HWSYNC_DRV_H_

