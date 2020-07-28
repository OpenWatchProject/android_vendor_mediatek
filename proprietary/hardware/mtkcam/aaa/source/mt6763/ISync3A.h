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
    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL init(MINT32 i4Policy, MINT32 i4Master, MINT32 i4Slave, const char* strName) = 0;

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL uninit() = 0;

    /**
     * Functions for 3A sync control
     */
    virtual MINT32 sync(MINT32 i4Sensor, MINT32 i4Param, MVOID* pSttBuf, MINT32 i4FrameNum = 0) = 0;

    /**
     * Functions for AF sync control
     * update lens position to Main2 lens
     */
    virtual MINT32 syncAF(MINT32 i4Sensor, MBOOL initLens = 0) = 0;
    virtual MVOID enableSync(MBOOL fgOnOff) = 0;

    virtual MBOOL isSyncEnable() const = 0;

    virtual MINT32 getFrameCount() const = 0;

    virtual MBOOL setDebugInfo(void* prDbgInfo) const = 0;

    virtual MBOOL getRemappingAeSetting(void* pAeSetting) const = 0; // Only use for ISP 3.0, slave need to remapping AE setting

    /**
     * Functions for AE sync control
     */
    virtual MBOOL isAeStable() const = 0;

    virtual MINT32 getAeSchedule() const = 0;

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

    virtual MVOID enableSyncSupport(E_SYNC3A_SUPPORT iSupport) = 0;

    virtual MVOID disableSyncSupport(E_SYNC3A_SUPPORT iSupport) = 0;

    virtual MINT32 getSyncSupport() const = 0;

    virtual MBOOL isAFSyncFinish() = 0;

    virtual MBOOL is2ASyncFinish() = 0;

    virtual MBOOL isAESyncStable() = 0;

    virtual MVOID sync2ASetting(MINT32 i4Master, MINT32 i4Slave) = 0;

    virtual MBOOL isPerframeAE() = 0;

protected:

    virtual ~ISync3A(){}
};

/******************************************************************************
 *  ISync3A Interface.
 ******************************************************************************/
class ISync3AMgr
{
public:
    enum
    {
        E_SYNC3AMGR_PRVIEW  = 0,
        E_SYNC3AMGR_CAPTURE = 1
    };

    enum E_SYNC3AMGR_CAPMODE
    {
        E_SYNC3AMGR_CAPMODE_3D = 0,
        E_SYNC3AMGR_CAPMODE_2D = 1
    };

    enum E_SYNC3AMGR_AF_STATE
    {
        E_SYNC3AMGR_AF_STATE_IDLE = 0,
        E_SYNC3AMGR_AF_STATE_BEGIN = 1,
        E_SYNC3AMGR_AF_STATE_SCANNING = 2,
    };

    /**
     * get singleton.
     */
    static ISync3AMgr* getInstance();

    virtual ISync3A* getSync3A(MINT32 i4Id=E_SYNC3AMGR_PRVIEW) const = 0;

    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL init(MINT32 i4Policy, MINT32 i4MasterIdx, MINT32 i4SlaveIdx) = 0;

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL uninit() = 0;

    virtual MVOID DevCount(MBOOL bEnable, MINT32 i4SensorDev) = 0;

    virtual MBOOL isInit() const = 0;

    virtual MBOOL isActive() const = 0;

    virtual MINT32 getMasterDev() const = 0;

    virtual MINT32 getSlaveDev() const = 0;

    virtual MVOID setAFState(MINT32 i4AfState) = 0;

    virtual MINT32 getAFState() const = 0;

    virtual MVOID setStereoParams(Stereo_Param_T const &rNewParam) = 0;

    virtual Stereo_Param_T getStereoParams() const = 0;

    virtual MINT32 getFrmSyncOpt() const = 0;

    virtual MINT32 getAWBMasterDev() const = 0;

    virtual MINT32 getAWBSlaveDev() const = 0;

    // only for 97
    virtual MVOID setAFSyncMode(MINT32 i4AfSyncMode) = 0;

    virtual MVOID enable() = 0;

    virtual MVOID disable() = 0;

    virtual MVOID updateInitParams() = 0;

    virtual MVOID setManualControl(MBOOL bEnable) = 0;

    virtual MBOOL isManualControl() = 0;

protected:
    virtual ~ISync3AMgr(){}
};

}


/**
 * @brief The definition of the maker of ISync3AMgr instance.
 */
typedef NS3Av3::ISync3AMgr* (*Sync3AMgr_FACTORY_T)();
#define MAKE_Sync3AMgr(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_SYNC_3A_MGR, Sync3AMgr_FACTORY_T, __VA_ARGS__)


#endif /* _AAA_N3D_SYNC_MGR_H_ */

