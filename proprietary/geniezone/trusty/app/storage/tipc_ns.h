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

#include <stdbool.h>
#include <trusty_ipc.h>

typedef uint64_t ns_handle_t;
typedef uint64_t ns_off_t;

int ns_open_file(handle_t ipc_handle, const char *name,
                 ns_handle_t *handlep, bool create);
void ns_close_file(handle_t ipc_handle, ns_handle_t handle);
int ns_read_pos(handle_t ipc_handle, ns_handle_t handle,
                ns_off_t pos, void *data, int data_size);
int ns_write_pos(handle_t ipc_handle, ns_handle_t handle,
                 ns_off_t pos, const void *data, int data_size);
