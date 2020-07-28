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

#ifndef _MTKCAM_HWNODE_P2_DUMP_PLUGIN_H_
#define _MTKCAM_HWNODE_P2_DUMP_PLUGIN_H_

#include "P2_Request.h"

namespace P2
{

class P2DumpPlugin : virtual public P2ImgPlugin
{
public:
    enum DUMP_IN {
        DUMP_IN_RRZO,
        DUMP_IN_IMGO,
        DUMP_IN_LCSO,
    };

    enum DUMP_OUT {
        DUMP_OUT_DISPLAY,
        DUMP_OUT_RECORD,
        DUMP_OUT_FD,
        DUMP_OUT_PREVIEWCB,
    };

public:
    P2DumpPlugin();
    virtual ~P2DumpPlugin();
    virtual MBOOL onPlugin(const P2Img *img);
    MBOOL isNddMode() const;
    MBOOL isDebugMode() const;
    #ifdef MTKCAM_CCT_TUNING_FOR_LOWDRAM
    P2DumpType determineP2DumpType(MINT32 frameNo);
    #endif
    MBOOL isFrameMode() const;
    P2DumpType needDumpFrame(MINT32 frameNo);
    MBOOL needDumpIn(DUMP_IN mask) const;
    MBOOL needDumpOut(DUMP_OUT mask) const;
    MBOOL needDump(const P2Img *img) const;
    MBOOL isFrameDump(MINT32 frameNo);

private:
    P2DumpType mMode = P2_DUMP_NONE;
    MINT32 mStart = 0;
    MUINT32 mCount = 0;
    MUINT32 mInMask = 0;
    MUINT32 mOutMask = 0;
    MINT32 mPreContinueDump = 0;

};

} // namespace

#endif // _MTKCAM_HWNODE_P2_DUMP_PLUGIN_H_

