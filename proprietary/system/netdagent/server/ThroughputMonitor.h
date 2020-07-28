/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _THROUGHPUTMONITOR_H__
#define _THROUGHPUTMONITOR_H__

#include <pthread.h>

class ThroughputMonitor {
public:
        ThroughputMonitor();
        ~ThroughputMonitor();

        static void* threadStart(void* monitor);
        int start();
        void stop();
        int dumpOn();
        void dumpOff();
private:
       void run();
       unsigned long long readCount(char const* filename);
       int mRunning, mDumpRunning;
       pthread_t mThread;
    };

#endif
