/*
 * Copyright 2014 The Android Open Source Project
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

#include <stdio.h>

enum {
	DEBUG_LVL,    // Messages used only for debugging
	INFO_LVL,     // Informational messages; something is unusual but not wrong
	WARNING_LVL,  // There's an indication of trouble, but it may be okay.
	ERROR_LVL,    // A problem has occurred, but processing can continue
	SEVERE_LVL,   // A severe problem has occurred; likely indicates a defect.
};

#ifndef LOG_TAG
#define LOG_TAG "trusty_logger"
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO_LVL
#endif

#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED(text)     "\x1b[31;1m" text ANSI_COLOR_RESET
#define ANSI_COLOR_GREEN(text)   "\x1b[32;1m" text ANSI_COLOR_RESET
#define ANSI_COLOR_YELLOW(text)  "\x1b[33;1m" text ANSI_COLOR_RESET
#define ANSI_COLOR_BLUE(text)    "\x1b[34;1m" text ANSI_COLOR_RESET
#define ANSI_COLOR_MAGENTA(text) "\x1b[35;1m" text ANSI_COLOR_RESET

#define LOG(level, level_text, fmt, ...) \
	if (level >= LOG_LEVEL) \
		fprintf(stderr, "[%s] " "\x1b[33m" "%s:%s:%d: " "\x1b[0m" fmt "\n", level_text, LOG_TAG, __func__, __LINE__,  ## __VA_ARGS__)

#define LOG_DEBUG(fmt, ...)  LOG(DEBUG_LVL,   "debug",  fmt, ## __VA_ARGS__)
#define LOG_INFO(fmt, ...)   LOG(INFO_LVL,    ANSI_COLOR_BLUE("info"),   fmt, ## __VA_ARGS__)
#define LOG_WARN(fmt, ...)   LOG(WARNING_LVL, ANSI_COLOR_YELLOW("warn"),   fmt, ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...)  LOG(ERROR_LVL,   ANSI_COLOR_RED("error"),  fmt, ## __VA_ARGS__)
#define LOG_SEVERE(fmt, ...) LOG(SEVERE_LVL,  ANSI_COLOR_MAGENTA("severe"), fmt, ## __VA_ARGS__)

