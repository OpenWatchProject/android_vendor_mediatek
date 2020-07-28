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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_CAMERA_CORE_CAMSHOT_MFLLSHOT_MFLLDEBUGEXIF_H_
#define _MTK_CAMERA_CORE_CAMSHOT_MFLLSHOT_MFLLDEBUGEXIF_H_

#include <mtkcam/common.h>

#include <mtkcam/exif/IDbgInfoContainer.h>

class MfllDebugExif final
{
public:
    enum STAGE
    {
        STAGE_UNDEFINED = 0,
        STAGE_BFBLD,
        STAGE_SF,
        STAGE_BSS,
        STAGE_MEMC,
        STAGE_MFB,
        STAGE_AFBLD,
    };


//
// Methods (public)
//
public:
    inline void copyDbgInfo(const IDbgInfoContainer* dbg)
    {
        dbg->copyTo(m_dbgContainer);
    }

    inline IDbgInfoContainer* getDbgContainer() const
    {
        return m_dbgContainer;
    }

    // Directly set MF debug data.
    //  @param tag          Tag of DEBUG_MF_TAG_T.
    //  @param data         4 bytes data.
    //  @return             Set ok returns true.
    bool setData(MINT32 tag, MINT32 data);

    // Update MF debug info of the stage s by a debug info container.
    //  @param dbg          Debug info container, the memory block of
    //                      "IspExifDebugInfo_T" will be read.
    //  @param s            For each stage, there're the different registers
    //                      will be read.
    //  @return             Set ok returns true.
    bool setData(const IDbgInfoContainer* dbg, STAGE s);


//
// Attributes
//
private:
    IDbgInfoContainer*  m_dbgContainer;
    MVOID*              m_dbgInfo;


//
// Constructors / Destructor
//
public:
    MfllDebugExif() noexcept;
    ~MfllDebugExif();

}; // class MfllDebugExif
#endif // _MTK_CAMERA_CORE_CAMSHOT_MFLLSHOT_MFLLDEBUGEXIF_H_
