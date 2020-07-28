/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GP_SHARED_MEM_H__
#define __GP_SHARED_MEM_H__

#include <algorithm>
#include <mutex>
#include <vector>

#include <sys/mman.h>

#include "tee_client_api.h"

namespace trustonic {

// Keep internal track of allocated shared memories
class TEECAllocatedSharedMemories {
    std::mutex shared_mems_mutex_;
    std::vector<TEEC_SharedMemory> shared_mems_;
public:
    bool allocate(TEEC_SharedMemory* shared_mem) {
        // Allocate memory (use size 1 if 0 given)
        size_t size = shared_mem->size;
        if (!size) {
            size++;
        }

        shared_mem->buffer = ::mmap(0, size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (shared_mem->buffer == MAP_FAILED) {
            shared_mem->buffer = nullptr;
            return false;
        }

        std::lock_guard<std::mutex> lock(shared_mems_mutex_);
        shared_mems_.push_back(*shared_mem);
        LOG_D("allocated shared mem, size %zu", shared_mems_.size());
        return true;
    }

    bool free(TEEC_SharedMemory* shared_mem) {
        std::lock_guard<std::mutex> lock(shared_mems_mutex_);
        auto it = std::find_if(shared_mems_.begin(), shared_mems_.end(),
        [shared_mem](const TEEC_SharedMemory& other) {
            return shared_mem->buffer == other.buffer;
        });
        if (it == shared_mems_.end()) {
            LOG_D("shared mem not found");
            return false;
        }

        // Free allocated memory (use size 1 if 0 given)
        size_t size = shared_mem->size;
        if (!size) {
            size++;
        }

        ::munmap(shared_mem->buffer, size);
        shared_mem->buffer = nullptr;
        shared_mem->size = 0;
        shared_mems_.erase(it);
        LOG_D("freed shared mem, size %zu", shared_mems_.size());
        return true;
    }
};

}

#endif // __GP_SHARED_MEM_H__
