/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
typedef unsigned long addr_t; /* used by list.h */

#include <compiler.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "trusty_ipc.h"
#include "trusty_uuid.h"
#define nanosleep trusty_nanosleep
#include "trusty_syscalls.h"
#undef nanosleep
#define nanosleep undefined_nanosleep
