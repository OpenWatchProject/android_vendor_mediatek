/*
 * Copyright (C) 2015-2016 The Android Open Source Project
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <list.h>

#include "block_cache.h"
#include "block_range.h"
#include "block_tree.h"

extern bool print_lookup; /* TODO: remove */

#define BLOCK_SET_MAX_DEPTH (BLOCK_TREE_MAX_DEPTH)

/**
 * struct block_set - In-memory state of B+ tree backed block set
 * @node:               List node used to link transaction allocated sets in fs.
 * @block_tree:         Block tree state.
 * @inserting_range:    Block range added to set but not yet in @block_tree.
 * @updating:           %true while updating the set, %false otherwise. If a
 *                      set insert or remove operation is re-entered while
 *                      @updating is %true, the update is only applied to
 *                      @inserting_range/@removing_range.
 *
 * In-memory state of B+ tree backed block set. Blocks that are part of the
 * block-set are tracked as ranges. A range is stored in @block_tree with the
 * start values as the key and the end value as the data. Unless an update is
 * in progress, all ranges in @block_tree are non-overlapping, and non-adjacent.
 * For instance, a block set containing only block 1 and 3 will be stored as two
 * ranges in @block_tree. Adding block 2 to this block-set will cause those two
 * ranges to be merged into a single range covering block 1 through 3.
 */
struct block_set {
    struct list_node node;
    struct block_tree block_tree;
    struct block_range initial_range;
    bool updating;
};

#define BLOCK_SET_INITIAL_VALUE(block_set) { \
    .node = LIST_INITIAL_CLEARED_VALUE, \
    .block_tree = BLOCK_TREE_INITIAL_VALUE(block_set.block_tree), \
    .initial_range = {0}, \
    .updating = 0, \
}

struct transaction;
struct fs;

void block_set_print(struct transaction *tr, struct block_set *set);
bool block_set_check(struct transaction *tr, struct block_set *set);

bool block_set_block_in_set(struct transaction *tr,
                            struct block_set *set,
                            data_block_t block);
bool block_set_range_in_set(struct transaction *tr,
                            struct block_set *set,
                            struct block_range range);
bool block_set_range_not_in_set(struct transaction *tr,
                                struct block_set *set,
                                struct block_range range);

data_block_t block_set_find_next_block(struct transaction *tr,
                                       struct block_set *set,
                                       data_block_t min_block,
                                       bool in_set);

struct block_range block_set_find_next_range(struct transaction *tr,
                                             struct block_set *set,
                                             data_block_t min_block);

bool block_set_overlap(struct transaction *tr,
                       struct block_set *set_a,
                       struct block_set *set_b);

void block_set_add_range(struct transaction *tr,
                         struct block_set *set,
                         struct block_range range);

void block_set_add_block(struct transaction *tr,
                         struct block_set *set,
                         data_block_t block);

void block_set_remove_range(struct transaction *tr,
                            struct block_set *set,
                            struct block_range range);

void block_set_remove_block(struct transaction *tr,
                            struct block_set *set,
                            data_block_t block);

void block_set_init(struct fs *fs, struct block_set *set);

void block_set_add_initial_range(struct block_set *set,
                                 struct block_range range);

void block_set_copy(struct transaction *tr,
                    struct block_set *dest,
                    const struct block_set *src);
