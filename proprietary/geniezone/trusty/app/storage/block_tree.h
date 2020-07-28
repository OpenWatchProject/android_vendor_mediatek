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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "block_cache.h"
#include "block_mac.h"

extern bool print_lookup; /* TODO: remove */

struct transaction;

#define BLOCK_TREE_MAX_DEPTH (9)

/**
 * struct block_tree - In-memory state of block backed B+ tree
 * @block_size:             Block size
 * @key_size:               Number of bytes used per key. 0 < @key_size <= 8.
 * @child_data_size:        Child or data size. The value at index 0 is the
 *                          number of bytes used for each child block_mac in
 *                          internal nodes, and the value at index 1 is number
 *                          of bytes stored in each data entry in leaf nodes.
 *                          0 < @child_data_size[n] <= sizeof(struct block_mac)
 *                          == 24.
 * @key_count:              Array with number of keys per node. The value at
 *                          index 0 applies to internal nodes and the value at
 *                          index 1 applied to leaf nodes.
 * @root:                   Block number and mac value for root block in tree.
 * @inserting:              Data currently beeing added to full node in the
 *                          tree. Allows the tree to be read while it is
 *                          updating.
 * @inserting.block:        Block number of node that is updating.
 * @inserting.key:          Key of entry that should be added.
 * @inserting.child:        Child that should be added to an internal node.
 * @inserting.data:         Data that should be added to a leaf node.
 * @update_count:           Update counter used to check that the three has not
 *                          been modified after a block_tree_path was created.
 * @root_block_changed:     %true if root block was allocated or copied after
 *                          this tree struct was initialized. %false otherwise.
 * @updating:               Used to relax some debug check while the tree is
 *                          updating, and to detect reentrant updates.
 * @copy_on_write:          %true if tree is persistent, %false if tree should
 *                          be discarded when completing transaction. Affects
 *                          which set block are allocated from, and if
 *                          copy-on-write opearation should be enabled.
 * @allow_copy_on_write:    %false if @allow_copy_on_write is %false or if
 *                          tree is read-only, %true otherwise.
 */
struct block_tree {
    size_t block_size;
    size_t key_size;
    size_t child_data_size[2]; /* 0: internal/child, 1: leaf/data */
    size_t key_count[2]; /* 0: internal, 1: leaf */
    struct block_mac root;
    struct {
        data_block_t block;
        data_block_t key;
        struct block_mac child;
        struct block_mac data;
    } inserting;
    int update_count;
    bool root_block_changed;
    bool updating;
    bool copy_on_write;
    bool allow_copy_on_write;
};

#define BLOCK_TREE_INITIAL_VALUE(block_tree) { 0 }


/**
 * struct block_tree_path_entry - block tree path entry
 * @block_mac:              Block number and mac of tree node
 * @index:                  Child or data index
 * @prev_key:               Key at @index - 1, or left key in parent when
 *                          @index == 0.
 * @next_key:               Key at @index. Or, right key in parent if key at
 *                          @index is not valid (0 or out of range).
 */
struct block_tree_path_entry {
    struct block_mac block_mac;
    uint index;
    data_block_t prev_key;
    data_block_t next_key;
};

/**
 * struct block_tree_path - block tree
 * @entry:              Array of block tree path entries.
 * @count:              Number of entries in @entry.
 * @data:               Data found in leaf node at @entry[@count-1].index.
 * @tr:                 Transaction object.
 * @tree:               Tree object.
 * @tree_update_count:  @tree.update_count at time of walk.
 */
struct block_tree_path {
    struct block_tree_path_entry entry[BLOCK_TREE_MAX_DEPTH];
    uint count;
    struct block_mac data;
    struct transaction *tr;
    struct block_tree *tree;
    int tree_update_count;
};

void block_tree_print(struct transaction *tr, const struct block_tree *tree);
bool block_tree_check(struct transaction *tr, const struct block_tree *tree);

void block_tree_walk(struct transaction *state,
                     struct block_tree *tree,
                     data_block_t key,
                     bool key_is_max,
                     struct block_tree_path *path);

void block_tree_path_next(struct block_tree_path *path);

static inline data_block_t block_tree_path_get_key(struct block_tree_path *path) {
    return (path->count > 0) ? path->entry[path->count - 1].next_key : 0;
}

static inline data_block_t block_tree_path_get_data(struct block_tree_path *path) {
    return block_mac_to_block(path->tr, &path->data);
}

static inline struct block_mac block_tree_path_get_data_block_mac(struct block_tree_path *path) {
    return path->data;
}

void block_tree_path_put_dirty(struct transaction *tr,
                               struct block_tree_path *path,
                               int path_index,
                               void *data,
                               obj_ref_t *data_ref);

void block_tree_insert(struct transaction *state, struct block_tree *tree,
                       data_block_t key, data_block_t data);

void block_tree_insert_block_mac(struct transaction *state,
                                 struct block_tree *tree,
                                 data_block_t key, struct block_mac data);

void block_tree_update(struct transaction *state, struct block_tree *tree,
                       data_block_t old_key, data_block_t old_data,
                       data_block_t new_key, data_block_t new_data);

void block_tree_update_block_mac(struct transaction *state, struct block_tree *tree,
                       data_block_t old_key, struct block_mac old_data,
                       data_block_t new_key, struct block_mac new_data);

void block_tree_remove(struct transaction *state, struct block_tree *tree,
                       data_block_t key, data_block_t data);

void block_tree_init(struct block_tree *tree,
                     size_t block_size,
                     size_t key_size,
                     size_t child_size,
                     size_t data_size);

void block_tree_copy(struct block_tree *dst, const struct block_tree *src);

#if BUILD_STORAGE_TEST
void block_tree_check_config(struct block_device *dev);
void block_tree_check_config_done(void);
#endif
