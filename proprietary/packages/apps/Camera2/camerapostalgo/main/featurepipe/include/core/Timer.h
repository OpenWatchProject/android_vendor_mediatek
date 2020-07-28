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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_TIMER_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_TIMER_H_

#include <core/common.h>

#include <queue>
#include <time.h>
#include <utils/RefBase.h>

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

#define ADD_TIMER(name)                         \
    Timer mTimer_##name;                        \
    MVOID start##name()                         \
    {                                           \
      mTimer_##name.start();                    \
    }                                           \
    MVOID stop##name(bool keepRunning=true)     \
    {                                           \
      return mTimer_##name.stop(keepRunning);   \
    }                                           \
    MVOID resume##name()                        \
    {                                           \
      return mTimer_##name.resume();            \
    }                                           \
    MUINT32 getElapsed##name() const            \
    {                                           \
      return mTimer_##name.getElapsed();        \
    }

#define ADD_TIMER_LIST(name, count)             \
    Timer mTimer_##name[count];                 \
    MVOID start##name(int index)                \
    {                                           \
      mTimer_##name[index].start();             \
    }                                           \
    MVOID stop##name(int index, bool keep=true) \
    {                                           \
      return mTimer_##name[index].stop(keep);   \
    }                                           \
    MVOID resume##name(int index)               \
    {                                           \
      return mTimer_##name[index].resume();     \
    }                                           \
    MUINT32 getElapsed##name(int index) const   \
    {                                           \
      return mTimer_##name[index].getElapsed(); \
    }

class Timer
{
public:
  enum Flag { STOP_RUNNING = 0, KEEP_RUNNING = 1 };

public:
  Timer(bool start=false);
  Timer(const timespec &start);
  MVOID start();
  MVOID resume();
  MVOID stop(bool keepRunning=KEEP_RUNNING);

  // millisecond
  MUINT32 getElapsed() const;
  MUINT32 getNow() const;
  static timespec getTimeSpec();
  static MUINT32 diff(const timespec &from, const timespec &to);

protected:
  timespec mStart;
  timespec mStop;
  bool mKeepRunning;
  MUINT32 mCumulative;

private:
  Timer& operator=(const Timer&);
};

class FPSCounter : public virtual android::RefBase
{
public:
  FPSCounter();
  ~FPSCounter();
  MVOID update(const timespec &mark);
  double getFPS() const;

private:
  std::queue<timespec> mMarks;
};

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com


#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_TIMER_H_
