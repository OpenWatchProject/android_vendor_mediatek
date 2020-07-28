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

#include <stdlib.h>

#define array_copy(dest, src, count) \
    do { \
        STATIC_ASSERT(sizeof((dest)[0]) == sizeof((src)[0])); \
        memcpy((dest), (src), sizeof((dest)[0]) * (count)); \
    } while(false)

#define array_shift(array, dest, src) \
    do { \
        assert(src <= countof((array))); \
        assert(dest <= countof((array))); \
        memmove((array) + (dest), (array) + (src), \
                sizeof((array)[0]) * (countof((array)) - MAX((src), (dest)))); \
    } while(false)

#define array_insert(array, index, value) \
    do { \
        array_shift((array), (index) + 1, (index)); \
        (array)[(index)] = (value); \
    } while(false)

#define array_insert_overflow(array, index, value, overflow_value) \
    do { \
        if (index == countof((array))) { \
            *overflow_value = value; \
        } else { \
            *overflow_value = (array)[countof((array)) - 1]; \
            array_insert((array), (index), (value)); \
        } \
    } while(false)

#define array_clear_end(array, start) \
    do { \
        memset((array) + (start), 0, \
               sizeof((array)[0]) * (countof((array)) - (start))); \
    } while(false)

#define array_shift_down(array, dest, src) \
    do { \
        assert(src > dest); \
        assert(src <= countof((array))); \
        array_shift((array), (dest), (src)); \
        array_clear_end((array), countof((array)) - ((src) - (dest))); \
    } while(false)

