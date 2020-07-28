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

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <linux/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define LOG_TAG "ThroughputMonitor"
#define DBG 1

#include "log/log.h"

#include "ThroughputMonitor.h"
#include "PerfController.h"
#include <cutils/properties.h>

#ifdef DENALI
  #define ACK_REDUCTION "9"
  #define THREASHOLD 5<<17
#else
  #define THREASHOLD 35<<18
#endif

ThroughputMonitor::ThroughputMonitor() {
    mThread = 0;
    mRunning = 0;
    mDumpRunning = 0;
}

ThroughputMonitor::~ThroughputMonitor() {
    mThread = 0;
    mRunning = 0;
    mDumpRunning = 0;
}

int ThroughputMonitor::dumpOn() {
    if(mDumpRunning == 1){
        ALOGI("ThroughputDump is already running");
        return 0;
    }

    ALOGI("ThroughputDump trys to start!");
    mDumpRunning = 1;
    return 0;
}

void ThroughputMonitor::dumpOff() {
  if(mDumpRunning == 0) {
        ALOGI("ThroughputDump has already stop");
        return;
    }

    ALOGI("ThroughputDump trys to stop!");
    mDumpRunning = 0;
    return;
}

int ThroughputMonitor::start() {
    if(mThread == 0){
        pthread_create(&mThread, NULL, ThroughputMonitor::threadStart, this);
    }else{
        ALOGW("ThroughputMonitor is already running");
    }

    return 0;
}

void ThroughputMonitor::stop() {
  if(mThread == 0) {
        ALOGI("ThroughputMonitor has already stop");
        return;
    }

    ALOGI("ThroughputMonitor trys to stop!");
    if(PerfController::get_lowpower_perfhandle() != -1) {
        PerfController::exit_little_cpu();
        PerfController::restore_ack_reduction();
    }
    if(mDumpRunning == 1)
        dumpOff();
    mRunning = 0;
    mThread = 0;
    return;
}

void* ThroughputMonitor::threadStart(void* obj) {
    ThroughputMonitor* monitor = reinterpret_cast<ThroughputMonitor*>(obj);

    monitor->run();
    //delete monitor;

    pthread_exit(NULL);
    ALOGI("ThroughputMonitor thread exit!");
    return NULL;
}

unsigned long long ThroughputMonitor::readCount(char const* filename) {
    char buf[80];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        if (errno != ENOENT) ALOGE("Can't open %s: %s", filename, strerror(errno));
        return -1;
    }

    int len = read(fd, buf, sizeof(buf) - 1);
    if (len < 0) {
        ALOGE("Can't read %s: %s", filename, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    buf[len] = '\0';
    return strtoull(buf, 0, 10);
}

void ThroughputMonitor::run() {
    char filename[80];
    int index = 0;
    unsigned long long current = 0;
    unsigned long long last = 0;
    unsigned long long lowpower_threshold = THREASHOLD;// k*2^17 bytes <-> k Mbps
    const char *lowpower_threshold_name = "vendor.net.perf.internal.threshold";
	const char* iface_list[] = {
		"ccmni0",
		"ccmni1",
		"ccmni2",
		0
	};
    int type = PerfController::get_load();
#ifdef ACK_REDUCTION
    const char *ack_setting = ACK_REDUCTION; //data:ack = (3+1):1
#endif

    char lowpower_threshold_value[PROPERTY_VALUE_MAX]  = {0};
    if (property_get(lowpower_threshold_name, lowpower_threshold_value, NULL) > 0)
       lowpower_threshold = strtoull(lowpower_threshold_value, 0, 10);

    mRunning = 1;
    ALOGI("ThroughputMonitor is running, thread id = %d, threashold = %llu!", gettid(), lowpower_threshold);
    while (mRunning){
        if(PerfController::is_testsim() || (PerfController::get_tether_perfhandle() != -1)) {
            while (iface_list[index] != 0) {
                snprintf(filename, sizeof(filename), "/sys/class/net/%s/statistics/rx_bytes", iface_list[index]);
                unsigned long long number = readCount(filename);
                if (number != 0) {
                    current += number;
                }
            index++;
            }
            if(last == 0) last = current;
            //ALOGI("current rx %llu, last rx %llu", current, last);
            if(mDumpRunning) { //dump throughput, cpu core and frequency for debug
                ALOGI("throughput: %llu bps", (current-last)*8L);
                PerfController::dump_cpuinfo(type);
            }
        }

        if(!PerfController::is_testsim()) {
            //ALOGI("testsim is not checked, thoughput monitor suspend");
            if(PerfController::get_lowpower_perfhandle() != -1) {
                PerfController::exit_little_cpu();
#ifdef ACK_REDUCTION
                PerfController::restore_ack_reduction();
#endif
            }

            sleep(5);
            goto reset;
        }
        if(current >= last) {
            if((current - last) > lowpower_threshold) {
                if((PerfController::get_lowpower_perfhandle() == -1) && (PerfController::get_tether_perfhandle() == -1)) {
                    PerfController::enter_little_cpu();
#ifdef ACK_REDUCTION
                    PerfController::set_ack_reduction(ack_setting);
#endif

                    ALOGI("throughput reaches %llu bps and exceeds limitation", (current-last)*8L);
                }
            } else {
                if(PerfController::get_lowpower_perfhandle() != -1) {
                    PerfController::exit_little_cpu();
#ifdef ACK_REDUCTION
                    PerfController::restore_ack_reduction();
#endif
                    ALOGI("throughput reaches %llu bps and belows limitation", (current-last)*8L);
                }
            }
        }//if current < last, it means sys/class/net/[interface]/statistics/rx_bytes overflow
        usleep(1000000);
    reset:
        last = current;
        current = 0;
        index = 0;
    }
}
