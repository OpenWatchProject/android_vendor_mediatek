/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_EVENTLOG_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_EVENTLOG_H_
//
#include <mtkcam/utils/std/RingBuffer.h>
#include <mtkcam/utils/std/LogTool.h>
//
#include <mutex>
#include <string>
//
#include <utils/Printer.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
/**
 *
 *  +---------+---------+---------+----------++----------+
 *  | The first buffer (to keep the oldest logs)         |
 *  |                                                    |
 *  +---------+---------+---------+----------++----------+
 *
 *  +---------+---------+---------+----------++----------+
 *  | The second buffer (to keep the latest logs)        |
 *  |                                                    |
 *  +---------+---------+---------+----------++----------+
 *
 * The first buffer: a linear buffer to keep the oldest logs
 *  - When it is full, new logs will start to be written to the second buffer.
 *
 * The second buffer: a ring buffer to keep the latest logs
 *  - When it is full, the oldest logs in this buffer will be overwritten with
 *    the latest logs.
 *
 */
class EventLog
{
protected:  ////    Definitions.

    struct  Item
    {
        struct timespec     timestamp;
        std::string         event;
    };

    typedef NSCam::Utils::RingBuffer<Item>  LogBufferT;

protected:  ////    Data Members.
    static const size_t kDefaultOldestBufferCapacity = 0;
    static const size_t kDefaultLatestBufferCapacity = 25;

    mutable std::mutex      mLogLock;
    LogBufferT              mLogBuffer1;
    LogBufferT              mLogBuffer2;
    NSCam::Utils::LogTool*  mLogTool = nullptr;

public:     ////    Operations.
                    EventLog(
                        const size_t nLatestBufferCapacity = kDefaultLatestBufferCapacity,
                        const size_t nOldestBufferCapacity = kDefaultOldestBufferCapacity
                    )
                        : mLogLock()
                        , mLogBuffer1(nOldestBufferCapacity)
                        , mLogBuffer2(nLatestBufferCapacity)
                        , mLogTool(NSCam::Utils::LogTool::get())
                    {
                    }

                    template <class T>
    auto            add(const T& event)
                    {
                        Item item;
                        item.event = event;
                        mLogTool->getCurrentLogTime(&item.timestamp);

                        std::lock_guard<std::mutex> _l(mLogLock);

                        if (mLogBuffer1.size() < mLogBuffer1.capacity()) {
                            mLogBuffer1.push_back(std::move(item));
                        }
                        else {
                            mLogBuffer2.push_back(std::move(item));
                        }
                    }

    auto            print(android::Printer& printer) const
                    {
                        std::lock_guard<std::mutex> _l(mLogLock);

                        for (const auto& item : mLogBuffer1) {
                            printer.printFormatLine("  %s : %s",
                                mLogTool->convertToFormattedLogTime(&item.timestamp).c_str(),
                                item.event.c_str());
                        }

                        if ( mLogBuffer1.empty() && mLogBuffer2.empty() ) {
                            printer.printLine("  [no events yet]");
                        }
                        else if ( ! mLogBuffer1.empty() && ! mLogBuffer2.empty() ) {
                            printer.printLine("  ...");
                            if (mLogBuffer2.size() == mLogBuffer2.capacity()) {
                                printer.printLine("  ...");
                            }
                        }

                        for (const auto& item : mLogBuffer2) {
                            printer.printFormatLine("  %s : %s",
                                mLogTool->convertToFormattedLogTime(&item.timestamp).c_str(),
                                item.event.c_str());
                        }
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
class EventLogPrinter
    : public android::Printer
{
protected:  ////    Data Members.
    static const size_t kDefaultOldestBufferCapacity = 0;
    static const size_t kDefaultLatestBufferCapacity = 25;
    EventLog        mEventLog;

public:     ////    Operations.
                    EventLogPrinter(
                        const size_t nLatestBufferCapacity = kDefaultLatestBufferCapacity,
                        const size_t nOldestBufferCapacity = kDefaultOldestBufferCapacity
                    )
                        : mEventLog(nLatestBufferCapacity, nOldestBufferCapacity)
                    {
                    }

    auto            print(android::Printer& printer) const
                    {
                        mEventLog.print(printer);
                    }

public:     ////    Printer Interface.
    virtual void    printLine(const char* string)
                    {
                        mEventLog.add(string);
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_EVENTLOG_H_

