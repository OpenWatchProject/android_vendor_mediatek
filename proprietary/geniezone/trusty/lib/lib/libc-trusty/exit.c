/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <compiler.h>
#include <trusty_std.h>

#include "atexit.h"

/* HACK: needed temporarily until the syscalls file gets the
 * __NO_RETURN annotation */
__NO_RETURN long exit_group (void);

__NO_RETURN void exit(int status)
{
	__cxa_finalize();
	exit_group();
}

__NO_RETURN void mtee_exit(int status)
{
    exit_group();
}

