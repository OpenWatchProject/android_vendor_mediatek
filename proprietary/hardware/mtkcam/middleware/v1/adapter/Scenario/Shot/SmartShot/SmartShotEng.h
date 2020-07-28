/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_SMARTSHOTENG_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_SMARTSHOTENG_H_

#include "SmartShot.h"

namespace NSCamShot {
    struct CamShotNotifyInfo;
    struct CamShotDataInfo;
}

#include "../EngShot/EngParam.h"
#include "../EngShot/EngShot.h"


namespace android {
namespace NSShot {

class EngShotWrapper : public EngShot
{
public:
    EngShotWrapper(const char *pszShotName,
            uint32_t const u4ShotMode,
            int32_t const i4OpenId)
        : EngShot(pszShotName, u4ShotMode, i4OpenId)
    {
    };
    virtual ~EngShotWrapper()
    {
    };

public:
    virtual bool dumpEngRaw(
            uint32_t const u4RawImgSize,
            uint8_t const *puRawImgBuf,
            MINT const iBufferInFormat)
    {
        return EngShot::dumpEngRaw(u4RawImgSize, puRawImgBuf, iBufferInFormat);
    };
    virtual bool setEngParam(void const *pParam, size_t const size)
    {
        return EngShot::setEngParam(pParam, size);
    };

public:
    EngParam& engParam()
    {
        return mpEngParam;
    };
};

// ---------------------------------------------------------------------------

class SmartShotEng : virtual public android::NSShot::NSSmartShot::SmartShot
{
/* constructor & destructor */
public:
    SmartShotEng(
            const char *pszShotName,
            uint32_t const u4ShotMode,
            int32_t const i4OpenId
        );

    virtual ~SmartShotEng();


/* re-implementations of both MfllShot/EngShot*/
public:
    bool sendCommand(uint32_t const  cmd,
            MUINTPTR const  arg1,
            uint32_t const  arg2,
            uint32_t const  arg3 = 0
            ) override;

/* sub-methods invoked by sendCommand */
protected:
    using SmartShot::onCmd_capture;

    void onCmd_capture(const MUINTPTR arg1, const uint32_t arg2, const uint32_t arg3);


protected:
    void onCmd_capture_MFNR(const MUINTPTR arg1, const uint32_t arg2, const uint32_t arg3);


/* re-implementations of SCallbackListener */
protected:
    virtual MVOID   onMetaReceived(
                        MUINT32         const requestNo,
                        StreamId_T      const streamId,
                        MBOOL           const errorResult,
                        IMetadata       const result
                    ) override;

    virtual MVOID   onDataReceived(
                        MUINT32                     const requestNo,
                        StreamId_T                  const streamId,
                        MBOOL                       const errorResult,
                        android::sp<IImageBuffer>&  pBuffer
                    ) override;


/* SmartShotEng attributes */
protected:
    sp<EngShotWrapper>  m_engShot;
    int                 m_rawCount;
    EngParam            m_engParam;


};// class SmartShotEng
} // namespace NSShot
} // namespace android
#endif // _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_SmartShotEng_H_
