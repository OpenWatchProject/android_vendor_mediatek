/*
 * Copyright (C) 2016 The Android Open Source Project
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

#pragma once

#include <trace.h>

enum {
    TRACE_LEVEL_NONE,
    TRACE_LEVEL_ERROR,
    TRACE_LEVEL_WARNING,
    TRACE_LEVEL_INIT,
    TRACE_LEVEL_WRITE,
    TRACE_LEVEL_READ,
};

#define TRACEFF(f, str, x...) do { fprintf(f, "%s:%d: " str, __PRETTY_FUNCTION__, __LINE__, ## x); } while (0)
#undef LTRACEF_LEVEL
#define LTRACEF_LEVEL(level, x...) \
	do { \
		if (LOCAL_TRACE >= (level)) { \
			TRACEFF((LOCAL_TRACE_ERR >= level) ? stderr : stdout, x); \
		} \
	} while (0)

#ifndef LOCAL_TRACE
#define LOCAL_TRACE TRACE_LEVEL_WARNING
#endif
#ifndef LOCAL_TRACE_ERR
#define LOCAL_TRACE_ERR TRACE_LEVEL_ERROR
#endif

#define pr_err(x...) LTRACEF_LEVEL(TRACE_LEVEL_ERROR, x)
#define pr_warn(x...) LTRACEF_LEVEL(TRACE_LEVEL_WARNING, x)
#define pr_init(x...) LTRACEF_LEVEL(TRACE_LEVEL_INIT, x)
#define pr_write(x...) LTRACEF_LEVEL(TRACE_LEVEL_WRITE, x)
#define pr_read(x...) LTRACEF_LEVEL(TRACE_LEVEL_READ, x)
