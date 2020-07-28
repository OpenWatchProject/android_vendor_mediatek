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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_DEBUGTIMER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_DEBUGTIMER_H_

/******************************************************************************
 *
 ******************************************************************************/
#include <time.h>
#include <queue>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/
#define ADD_DEBUGTIMER(name)                         \
    DebugTimer mTimer_##name;                        \
    MVOID start##name()                         \
    {                                           \
      mTimer_##name.start();                    \
    }                                           \
    MVOID stop##name()                          \
    {                                           \
      return mTimer_##name.stop();              \
    }                                           \
    MUINT32 getElapsed##name() const            \
    {                                           \
      return mTimer_##name.getElapsed();        \
    }

class DebugTimer
{
public:
    DebugTimer()
    {
    }

    MVOID start()
    {
        clock_gettime(CLOCK_MONOTONIC, &mStart);
    }

    MVOID stop()
    {
        clock_gettime(CLOCK_MONOTONIC, &mStop);
    }

    static MUINT32 diff(const timespec &from, const timespec &to)
    {
        MUINT32 diff = 0;
        if( to.tv_sec || to.tv_nsec ||
            from.tv_sec || from.tv_nsec )
        {
            diff = ((to.tv_sec - from.tv_sec) * 1000) +
                   ((to.tv_nsec - from.tv_nsec) / 1000000);
        }
        return diff;
    }

    static MUINT32 diffU(const timespec &from, const timespec &to)
    {
        MUINT32 diff = 0;
        if( to.tv_sec || to.tv_nsec ||
            from.tv_sec || from.tv_nsec )
        {
            diff = ((to.tv_sec - from.tv_sec) * 1000000) +
                   ((to.tv_nsec - from.tv_nsec) / 1000);
        }
        return diff;
    }

    // millisecond
    MUINT32 getElapsed() const
    {
        return diff(mStart, mStop);
    }

    // usecond
    MUINT32 getElapsedU() const
    {
        return diffU(mStart, mStop);
    }

protected:
    timespec mStart = timespec();
    timespec mStop = timespec();

private:
    DebugTimer& operator=(const DebugTimer&);
};

/******************************************************************************
 *
 ******************************************************************************/
class FPSCounter
{
public:
    FPSCounter() {}
    ~FPSCounter() {}

    MVOID update()
    {
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        mMarks.push(now);
        if( mMarks.size() > 30 )
        {
            mMarks.pop();
        }
    }

    double getFPS() const
    {
        double fps = 0;
        unsigned size = mMarks.size();
        if( size > 1 )
        {
            MUINT32 time = DebugTimer::diff(mMarks.front(), mMarks.back());
            if( time > 0 )
            {
                fps = 1000.0 * (size-1) / time;
            }
        }
        return fps;
    }

private:
    std::queue<timespec> mMarks;
};

/******************************************************************************
*
*******************************************************************************/
};  // namespace Utils
};  // namespace NSCam
#endif  //  _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_DEBUGTIMER_H_

