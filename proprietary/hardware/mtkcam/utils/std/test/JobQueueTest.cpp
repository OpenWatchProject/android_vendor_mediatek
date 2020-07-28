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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include <mtkcam/utils/std/JobQueue.h>

#include <thread>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>

using std::cout;
using std::endl;

#define MSLEEP(x) std::this_thread::sleep_for( std::chrono::milliseconds(x) )

static int my_task(int val)
{
    MSLEEP(50);
    cout << "running task, val=" << val << endl;
    return val;
}

static int my_task_priority(int val, int p)
{
    MSLEEP(50);
    cout << "running task, priority=" << p << " , val=" << val << endl;
    return val;
}

void test_noraml_flow()
{ // {{{
    NSCam::JobQueue<int()> q("UT");
    int c = 0;

    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }
} // }}}


void test_handle_get()
{ // {{{
    NSCam::JobQueue<int()> q("UT");
    std::deque< decltype(q)::JobHandle > handles;
    //
    int c = 0;

    for (int i = 0; i < 10; i++) {
        auto h = q.addJob( std::bind(my_task, c++) );
        handles.emplace_back(std::move(h));
    }

    for (const auto& itr : handles) {
        if (itr.valid())
            cout << "get task result=" << itr.get() << endl; // wait finished and get result
        else
            cout << "get task which is invalid" << endl;
    }
} // }}}


void test_remove_flow()
{ // {{{
    NSCam::JobQueue<int()> q("UT");
    std::deque< decltype(q)::JobHandle > handles;
    //
    int c = 0;

    for (int i = 0; i < 10; i++) {
        auto h = q.addJob( std::bind(my_task, c++) );
        handles.emplace_back(std::move(h));
    }

    // remove item 8
    handles[8].abandonSharedState(); // MUST invoke abandonSharedState first
    auto bRemoved = q.removeJob(handles[8].getId());

    cout << "remove task(8)" << endl;
    cout << "Removed? " << (bRemoved ? "Yes" : "No") << endl;

    for (const auto& itr : handles) {
        if (itr.valid())
            cout << "get task result=" << itr.get() << endl;
        else
            cout << "get task which is invalid" << endl;
    }
} // }}}


void test_flush_pendding()
{ // {{{
    NSCam::JobQueue<int()> q("UT");
    int c = 0;

    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }

    // wait 100 ms and flush all pending jobs
    MSLEEP(100);

    q.flush(); // flush all pending jobs

    // keep adding new, JobQueue is still alive
    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }
} // }}}


void test_request_exit()
{ // {{{
    NSCam::JobQueue<int()> q("UT");
    int c = 0;

    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }

    q.requestExit();

    // add more job! it shouldn't be added due to it's exiting
    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }
} // }}}


void test_aggress_exit()
{ //{{{
    NSCam::JobQueue<int()> q("UT");
    int c = 0;

    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }

    // wait 100 ms and aggressExit
    MSLEEP(100);

    // request to exit
    q.aggressExit();

    // add more job! it shouldn't be added due to it's exiting
    for (int i = 0; i < 10; i++) {
        q.addJob( std::bind(my_task, c++) );
    }
} //}}}


void test_priority_queue()
{ // {{{
    std::srand(std::time(0)); // use current time as seed for random generator
    const int testCnt = 20;
    //
    NSCam::JobQueue<int(), NSCam::JQPriority_Greater> q("UT");
    // add a delay job
    q.addJob([](){ MSLEEP(500); return 0; });

    // put main thread to sleep
    MSLEEP(100);

    // ramdon add
    for (int i = 0; i < testCnt; i++) {
        int r = std::rand() % testCnt;
        cout << "priority =" << r << endl;
        q.addJob( std::bind(my_task_priority, i, r), r);
    }
} // }}}


void test_priority_fifo_queue()
{ // {{{
    std::srand(std::time(0)); // use current time as seed for random generator
    const int testCnt = 20;
    //
    NSCam::JobQueue<int(), NSCam::JQPriority_GreaterFIFO> q("UT");
    // add a delay job
    q.addJob([](){ MSLEEP(500); return 0; });

    // put main thread to sleep
    MSLEEP(100);

    // ramdon add
    for (int i = 0; i < testCnt; i++) {
        int r = std::rand() % testCnt;
        cout << "priority =" << r << endl;
        q.addJob( std::bind(my_task_priority, i, r), r);
    }
} // }}}


int main(int argc, char** argv)
{
    int testCnt = 0;
    int testCase = 0;

    if (argc > 1) {
        testCase = ::atoi(argv[1]);
    }

lbStart:
    cout << "==================================================================" << endl;
    cout << "Test Count: " << testCnt << endl;
    cout << "==================================================================" << endl;

    cout << "---test_noraml_flow---" << endl;
    test_noraml_flow();
    cout << "---test_noraml_flow---" << endl;

    cout << endl;

    cout << "---test_handle_get----" << endl;
    test_handle_get();
    cout << "---test_handle_get----" << endl;

    cout << endl;

    cout << "---test_remove_flow---" << endl;
    test_remove_flow();
    cout << "---test_remove_flow---" << endl;

    cout << endl;

    cout << "---test_flush_pendding---" << endl;
    test_flush_pendding();
    cout << "---test_flush_pendding---" << endl;

    cout << endl;

    cout << "---test_request_exit--" << endl;
    test_request_exit();
    cout << "---test_request_exit--" << endl;

    cout << endl;

    cout << "---test_aggress_exit--" << endl;
    test_aggress_exit();
    cout << "---test_aggress_exit--" << endl;

    cout << endl;

    cout << "---test_priority_queue--" << endl;
    test_priority_queue();
    cout << "---test_priority_queue--" << endl;

    cout << endl;

    cout << "---test_priority_fifo_queue---" << endl;
    test_priority_fifo_queue();
    cout << "---test_priority_fifo_queue---" << endl;

    if (++testCnt < testCase)
        goto lbStart;

    return 0;
}
