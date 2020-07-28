/*
 * Copyright (C) 2018-2019 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GPUD_INCLUDE_GPUD_GPUD_TRACE_H_
#define GPUD_INCLUDE_GPUD_GPUD_TRACE_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <cutils/trace.h>

#define ATRACE_TAG ATRACE_TAG_ALWAYS

// ATRACE_NAME traces the beginning and end of the current scope.
// To trace the correct start and end times this macro should be declared first in the scope body.
#define GPUD_ATRACE_NAME(name) gpudScopedTrace ___gpudtracer(ATRACE_TAG, name)

// ATRACE_CALL is an ATRACE_NAME that uses the current function name.
#define GPUD_ATRACE_CALL() GPUD_ATRACE_NAME(__FUNCTION__)

#define GPUD_ATRACE_BUFFER(fmt, ...)                    \
    if (atrace_is_tag_enabled(ATRACE_TAG)) {            \
        char buf[256];                                  \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        gpudScopedTrace tracer(ATRACE_TAG, buf);        \
    }

__BEGIN_DECLS

class gpudScopedTrace {
 public:
    inline gpudScopedTrace(uint64_t tag, const char *name) : mTag(tag) {
        atrace_begin(mTag, name);
    }
    inline ~gpudScopedTrace() {
        atrace_end(mTag);
    }
 private:
    uint64_t mTag;
};

__END_DECLS

#endif  // GPUD_INCLUDE_GPUD_GPUD_TRACE_H_
