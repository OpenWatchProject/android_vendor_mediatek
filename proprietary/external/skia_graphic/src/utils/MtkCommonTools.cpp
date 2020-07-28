/*
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Created on: 2017-11-30
 * Author: Christ Sun
 */

#include "MtkCommonTools.h"
#include <string>
#include <string.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "skia", __VA_ARGS__)

extern "C" {
    #include <stdio.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <stdlib.h>
}

#include "cutils/properties.h"
using std::string;

bool isBootCompleted() {
    char prop_value[PROPERTY_VALUE_MAX];
    char boot_completed[] = "1";
    if (property_get("sys.boot_completed", prop_value, NULL) != 0) {
        if (strncmp(prop_value, boot_completed, 1) == 0) {
#if defined(__MTK_DUMP_DRAW_BITMAP__) || defined(__MTK_DUMP_DRAW_BITMAP_RECT__)
            LOGD("boot is completed!");
#endif
            return true;
        } else {
            LOGD("boot is not completed!");
            return false;
        }

    } else {
        LOGD("property_get return 0");
        return false;
    }
}


namespace MtkCommonTools {
AutoSystrace::AutoSystrace(const char * const traceName) {
    needAutoEnd = true;
    ATRACE_BEGIN(traceName);
}
void AutoSystrace::start() {}
void AutoSystrace::end() {
    needAutoEnd = false;
    ATRACE_END();
}
AutoSystrace::~AutoSystrace() {
    if (needAutoEnd)
        ATRACE_END();
}

AutoTimer::AutoTimer(const char * const funcName)
    : str(funcName), start(), end() {
    gettimeofday(&start,NULL);
}
AutoTimer::~AutoTimer() {
    gettimeofday(&end,NULL);
    unsigned long diff = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    LOGD("%s consumed %ld us\n", str, diff);
}
}


