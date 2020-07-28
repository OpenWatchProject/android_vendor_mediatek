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

#include "block_mac.h"
#include "block_set.h"
#include "block_tree.h"
#include "fs.h"

/**
 * struct transaction - Transaction state
 * @node:                   List node used to link transaction in fs.
 * @fs:                     Pointer to file system state.
 * @open_files:             List of open files.
 * @failed:                 %true if transaction ran out of disk space, or
 *                          collided with another transaction, %false if no
 *                          error has occured since transaction_activate.
 * @complete:               Transaction has been written to disk.
 * @min_free_block:         Used when completing a transaction to track how much
 *                          of the free set has been updated.
 * @last_free_block:        Similar to @last_tmp_free_block, used when
 *                          completing a transaction.
 * @last_tmp_free_block:    Used by allocator to prevent ping-pong between
 *                          allocating and freeing the same block when
 *                          re-entering the same set.
 * @new_free_set:           New free set used while completing a transaction.
 * @tmp_allocated:          Blocks used while transaction is pending.
 * @allocated:              Blocks allocated by transaction.
 * @freed:                  Blocks freed by transaction.
 * @files_added:            Files added by transaction.
 * @files_updated:          Files modified by transaction.
 * @files_removed:          Files removed by transaction.
 */
struct transaction {
    struct list_node node;
    struct fs *fs;
    struct list_node open_files;
    bool failed;
    bool complete;
    data_block_t min_free_block;
    data_block_t last_free_block;
    data_block_t last_tmp_free_block;
    struct block_set *new_free_set;
    struct block_set tmp_allocated;
    struct block_set allocated;
    struct block_set freed;

    struct block_tree files_added;
    struct block_tree files_updated;
    struct block_tree files_removed;
};

void transaction_init(struct transaction *tr,
                      struct fs *fs,
                      bool activate);
void transaction_free(struct transaction *tr);
void transaction_activate(struct transaction *tr);
void transaction_fail(struct transaction *tr);
void transaction_complete(struct transaction *tr);

static inline bool transaction_is_active(struct transaction *tr) {
    return list_in_list(&tr->allocated.node);
};

bool transaction_block_need_copy(struct transaction *tr, data_block_t block);
