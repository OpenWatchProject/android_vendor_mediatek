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

#ifndef _AAA_N3D_SYNC_MGR_H_
#define _AAA_N3D_SYNC_MGR_H_

#include <mtkcam/utils/module/module.h>
#include <mtkcam/aaa/aaa_hal_common.h>


namespace NS3Av3
{
/******************************************************************************
 *  ISync3A Interface.
 ******************************************************************************/
class ISync3A
{
public:
    enum
    {
        E_SYNC3A_DO_AE        = (1<<0),
        E_SYNC3A_DO_AE_PRECAP = (1<<1),
        E_SYNC3A_DO_AWB       = (1<<2),
        E_SYNC3A_BYP_AE       = (1<<3),
        E_SYNC3A_DO_HW_SYNC   = (1<<4),
        E_SYNC3A_DO_SW_SYNC   = (1<<5)
    };

    struct Sync3A_Remapping_AeSetting_T
    {
        MUINT32 u4Eposuretime;   //!<: Exposure time in us
        MUINT32 u4AfeGain;       //!<: sensor gain
        MUINT32 u4IspGain;       //!<: raw gain

        Sync3A_Remapping_AeSetting_T()
            : u4Eposuretime(0)
            , u4AfeGain(0)
            , u4IspGain(0)
        {}
    };

    enum E_SYNC3A_SUPPORT
    {
        E_SYNC3A_SUPPORT_AE        = (1<<0),
        E_SYNC3A_SUPPORT_AWB       = (1<<1),
        E_SYNC3A_SUPPORT_AF        = (1<<2)
    };

    enum ESync3ACtrl_T
    {
        ESync3ACtrl_SetIsStart = 0,
        ESync3ACtrl_GetIsStart = 1,
    };

    /**
     * get singleton.
     */
    static ISync3A* getInstance();

    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL init(MINT32 i4Master, MINT32 i4Slave, const char* strName = "Preview") = 0;

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL uninit() = 0;

    /**
     * Functions for 3A sync control by 3A HAL
     */
    virtual MINT32 sync(MINT32 i4Sensor, MINT32 i4Param, MVOID* pSttBuf, MINT32 i4FrameNum = 0) = 0;

    /**
     * Functions for AF sync control
     * update lens position to Main2 lens
     */
    virtual MINT32 syncAF(MINT32 i4Sensor, MBOOL initLens = 0) = 0;

    /**
     * Functions for enable 3A sync machine
     */
    virtual MVOID enableSync(MBOOL fgOnOff) = 0;

    /**
     * Functions for is enable 3A sync or not
     */
    virtual MBOOL isSyncEnable() const = 0;

    /**
     * Functions for Algo getting debug info
     */
    virtual MBOOL setDebugInfo(void* prDbgInfo) const = 0;

    /**
     * Functions for AE is stable with master and slave
     */
    virtual MBOOL isAeStable() const = 0;

    /**
     * Functions for get AE schedule
     */
    virtual MINT32 getAeSchedule() const = 0;

    /**
     * Functions for enable sync support for modules
     */
    virtual MVOID enableSyncSupport(E_SYNC3A_SUPPORT iSupport) = 0;

    /**
     * Functions for disable sync support for modules
     */
    virtual MVOID disableSyncSupport(E_SYNC3A_SUPPORT iSupport) = 0;

    /**
     * Functions for get sync support info
     */
    virtual MINT32 getSyncSupport() const = 0;

    /**
     * Functions for is AF Sync Finish or not with master and slave
     */
    virtual MBOOL isAFSyncFinish() = 0;

    /**
     * Functions for is AWB Sync Finish or not
     */
    virtual MBOOL is2ASyncFinish() = 0;

    /**
     * Functions for is AE stable or not with master and slave
     */
    virtual MBOOL isAESyncStable() = 0;

    /**
     * Functions for sync AE setting to slave from master
     */
    virtual MVOID sync2ASetting(MINT32 i4Master, MINT32 i4Slave) = 0;

    /**
     * Functions for device count for 3A HAL
     */
    virtual MVOID DevCount(MBOOL bEnable, MINT32 i4SensorDev) = 0;

    /**
     * Functions for query is init or not
     */
    virtual MBOOL isInit() const = 0;

    /**
     * Functions for query is active or not
     */
    virtual MBOOL isActive() const = 0;

    /**
     * Functions for query Master Dev
     */
    virtual MINT32 getMasterDev() const = 0;

    /**
     * Functions for query Slave Dev
     */
    virtual MINT32 getSlaveDev() const = 0;

    /**
     * Functions for set AF state
     */
    virtual MVOID setAFState(MINT32 i4AfState) = 0;

    /**
     * Functions for update stereo parameter by control
     */
    virtual MVOID setStereoParams(Stereo_Param_T const &rNewParam) = 0;

    /**
     * Functions for query frame sync option
     */
    virtual MINT32 getFrmSyncOpt() const = 0;

    /**
     * Functions for query AWB Master Dev
     */
    virtual MINT32 getAWBMasterDev() const = 0;

    /**
     * Functions for query AWB Slave Dev
     */
    virtual MINT32 getAWBSlaveDev() const = 0;

    virtual MVOID  setSwitchCamCount(MBOOL bEnable) = 0;

    virtual MINT32 getSwitchCamCount() const = 0;

    virtual MINT32 sendSync3ACtrl(ESync3ACtrl_T eSync3ACtrl, MINTPTR iArg1, MINTPTR iArg2) = 0;

protected:

    virtual ~ISync3A(){}
};

}

/**
 * @brief The definition of the maker of ISync3AMgr instance.
 */
typedef NS3Av3::ISync3A* (*ISync3A_FACTORY_T)();
#define MAKE_Sync3A(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_SYNC_3A, Sync3A_FACTORY_T, __VA_ARGS__)


#endif /* _AAA_N3D_SYNC_MGR_H_ */

