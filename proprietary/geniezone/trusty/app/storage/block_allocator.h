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

#include <list.h>
#include <stdbool.h>

#include "block_cache.h"

struct transaction;

data_block_t block_allocate_etc(struct transaction *tr, bool is_tmp);
void block_free_etc(struct transaction *tr, data_block_t block, bool is_tmp);
bool block_allocator_allocation_queued(struct transaction *tr,
                                       data_block_t block,
                                       bool is_tmp);
void block_allocator_suspend_set_updates(struct transaction *tr);
void block_allocator_process_queue(struct transaction *tr);

static inline data_block_t block_allocate(struct transaction *tr)
{
    return block_allocate_etc(tr, false);
}

static inline void block_free(struct transaction *tr, data_block_t block)
{
    block_free_etc(tr, block, false);
}

static inline data_block_t block_allocate_tmp(struct transaction *tr)
{
    return block_allocate_etc(tr, true);
}

static inline void block_free_tmp(struct transaction *tr, data_block_t block)
{
    block_free_etc(tr, block, true);
}
