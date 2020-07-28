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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#include <mtkcam3/feature/utils/p2/P2IOClassfier.h>
#define ILOG_MODULE_TAG P2IOClassfier
#include <mtkcam3/feature/utils/log/ILogHeader.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace NSCam {
namespace Feature {
namespace P2Util {

P2IOClassfier::P2IOClassfier(const std::vector<P2IO> &list)
    : mCollection(list)
{
    const size_t s = mCollection.size();
    for(MUINT32 i = 0; i < s ; i++)
    {
        if(mCollection[i].mTransform > 0)
        {
            mRotateQ.push(i);
        }
        else
        {
            mNonRotateQ.push(i);
        }
    }
}

P2IOClassfier::~P2IOClassfier()
{
}

MBOOL P2IOClassfier::hasIO() const
{
    return hasRotateIO() || hasNonRotateIO();
}

MBOOL P2IOClassfier::hasRotateIO() const
{
    return !mRotateQ.empty();
}

MBOOL P2IOClassfier::hasNonRotateIO() const
{
    return !mNonRotateQ.empty();
}

size_t P2IOClassfier::totalSize() const
{
    return (rotateSize() + nonRotateSize());
}

size_t P2IOClassfier::rotateSize() const
{
    return mRotateQ.size();
}

size_t P2IOClassfier::nonRotateSize() const
{
    return mNonRotateQ.size();
}

P2IO P2IOClassfier::popRotate()
{
    if(hasRotateIO())
    {
        MUINT32 index = mRotateQ.front();
        mRotateQ.pop();
        return mCollection.at(index);
    }
    else
    {
        MY_LOGE("Rotate Queue is empty! pop undefined P2IO !!");
        return P2IO();
    }
}

P2IO P2IOClassfier::popNonRotate()
{
    if(hasNonRotateIO())
    {
        MUINT32 index = mNonRotateQ.front();
        mNonRotateQ.pop();
        return mCollection.at(index);
    }
    else
    {
        MY_LOGE("Non Rotate Queue is empty! pop undefined P2IO !!");
        return P2IO();
    }
}

MVOID P2IOClassfier::popAll(std::vector<P2IO> &outs)
{
    outs.clear();
    outs.reserve(mRotateQ.size() + mNonRotateQ.size());
    while(hasRotateIO())
    {
        outs.emplace_back(popRotate());
    }
    while(hasNonRotateIO())
    {
        outs.emplace_back(popNonRotate());
    }
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
