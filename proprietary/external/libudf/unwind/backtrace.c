/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "Corkscrew"

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unwind.h>
#include <stdio.h>

#include "backtrace-arch.h"
#include "backtrace-helper.h"
#include "ptrace-arch.h"
#include "libudf-unwind/map_info.h"
#include "libudf-unwind/ptrace.h"


typedef struct {
    backtrace_frame_t* backtrace;
    size_t ignore_depth;
    size_t max_depth;
    size_t ignored_frames;
    size_t returned_frames;
    memory_t memory;
} backtrace_state_t;

static _Unwind_Reason_Code unwind_backtrace_callback(struct _Unwind_Context* context, void* arg) 
{
    backtrace_state_t* state = (backtrace_state_t*)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        // TODO: Get information about the stack layout from the _Unwind_Context.
        //       This will require a new architecture-specific function to query
        //       the appropriate registers.  Current callers of unwind_backtrace
        //       don't need this information, so we won't bother collecting it just yet.
        add_backtrace_entry(rewind_pc_arch(&state->memory, pc), state->backtrace,
                state->ignore_depth, state->max_depth,
                &state->ignored_frames, &state->returned_frames);
    }
    return state->returned_frames < state->max_depth ? _URC_NO_REASON : _URC_END_OF_STACK;
}

ssize_t libudf_unwind_backtrace_gcc(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth) 
{
    map_info_t* milist = acquire_my_map_info_list();

    backtrace_state_t state;
    state.backtrace = backtrace;
    state.ignore_depth = ignore_depth;
    state.max_depth = max_depth;
    state.ignored_frames = 0;
    state.returned_frames = 0;
    init_memory(&state.memory, milist);

    _Unwind_Reason_Code rc = _Unwind_Backtrace(unwind_backtrace_callback, &state);

    release_my_map_info_list(milist);

    if (state.returned_frames) {
        return state.returned_frames;
    }
    return rc == _URC_END_OF_STACK ? 0 : -1;
}

extern ssize_t unwind_backtrace_signal_arch_selfnogcc(
        const map_info_t* map_info_list,
        backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth);

__attribute__((visibility("default")))
ssize_t libudf_unwind_backtrace(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth) 
{
    ssize_t frames = -1;
    map_info_t* milist = acquire_my_map_info_list();
    frames = unwind_backtrace_signal_arch_selfnogcc(milist, backtrace, ignore_depth, max_depth);
    release_my_map_info_list(milist);
    return frames;
}

ssize_t libudf_unwind_backtrace_ptrace(pid_t tid, const ptrace_context_t* context,
        backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth) {
    return unwind_backtrace_ptrace_arch(tid, context, backtrace, ignore_depth, max_depth);
}

