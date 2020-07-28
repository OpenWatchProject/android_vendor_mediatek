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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_CAM_MGR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_CAM_MGR_H_
//
#include <utils/Mutex.h>
#include <fcntl.h>
#include <sys/ioctl.h>

using namespace android;

#define LIKELY(x)                   __builtin_expect(!!(x), 1)
#define UNLIKELY(x)                 __builtin_expect(!!(x), 0)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/
class CamManager {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                              CamManager()
                                  : mLockMtx()
                                  , mLockFps()
                                  , mbRecord(false)
                                  , mbAvailable(true)
                                  , mbStereo(false)
                                  , mbReadSegCode(false)
                                  , mDeviceCnt(0)
                                  , mFrameRate0(0)
                                  , mFrameRate1(0)
                                  , mSegmentCode(0)
                                  , mEISSceType(EIS_SCE_TYPE_DONT_CARE)
                              {
                              }

    static CamManager*          getInstance();


    void                        incDevice();

    void                        decDevice();

    uint32_t                    getDeviceCount() const;

    void                        setFrameRate(uint32_t const id, uint32_t const frameRate);

    uint32_t                    getFrameRate(uint32_t const id) const;

    bool                        isMultiDevice() const;

    bool                        setThermalPolicy(char* policy, bool usage);

    /**
     *  Set recording mode hint.
     *  The default value is FALSE. This should be called after starting/stopping recod.
     *
     *  param
     *      TRUE if camera start record.
     *      FALSE if camera stop record.
     *
     *  return
     *      N/A
     *
     */
    void                        setRecordingHint(bool const isRecord);


    /**
     *  Set HW available hint.
     *  The default value is TRUE. This should be called after starting/stopping preview.
     *
     *  param
     *      TRUE if driver support app to open another camera.
     *      FALSE if driver does NOT support app to open another camera.
     *
     *  return
     *      N/A
     *
     */
    void                        setAvailableHint(bool const isAvailable);

    /**
     *  Set stereo mode hint.
     *  The default value is FALSE. This should be called after init/uninit stereo mode.
     *
     *  param
     *      TRUE if camera in stereo mode.
     *      FALSE if camera not in stereo mode.
     *
     *  return
     *      N/A
     *
     */
    void                        setStereoHint(bool const isStereo);

    /**
     *  Get permission to open camera or to start preview.
     *
     *  param
     *      N/A
     *
     *  return
     *      TRUE: app can operate on this camera.
     *      FALSE: app can NOT operate on this camera.
     *
     */
    bool                        getPermission() const;

public:
    typedef enum
    {
        EIS_SCE_TYPE_DONT_CARE,
        EIS_SCE_TYPE_NULL,
        EIS_SCE_TYPE_EIS,
    }EIS_SCE_TYPE;

    /**
     *  Get device data from EFUSE.
     *  This is only used in Jade Minus.
     *
     *  param
     *      Index of the entry of DevData
     *
     *  return
     *      DevData
     *
     */
	uint32_t                    readEFUSEDevData(uint32_t index);

    /**
    *  Get EIS scenario according to EFUSE, using segment code to distinguish ICs.
    *  This is only used in Jade Minus.
    *
    *  param
    *      Index of the entry of DevData
    *
    *  return
    *      DevData
    *
    */
    uint32_t                    getEISScenarioType();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    mutable Mutex                       mLockMtx;
    mutable Mutex                       mLockFps;
    bool                                mbRecord;
    bool                                mbAvailable;
    bool                                mbStereo;
    bool                                mbReadSegCode;
    uint32_t                            mDeviceCnt;
    uint32_t                            mFrameRate0;
    uint32_t                            mFrameRate1;
    uint32_t                            mSegmentCode;
    EIS_SCE_TYPE                        mEISSceType;
};
/******************************************************************************
 *
 ******************************************************************************/

};  // namespace Utils
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_CAM_MGR_H_

