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

#include "block_tree.h"

struct block_map {
    struct block_tree tree;
};

#define BLOCK_MAP_INITIAL_VALUE(block_map) { \
    .tree = BLOCK_TREE_INITIAL_VALUE(block_map.tree), \
}

struct block_map_path {
    struct block_tree_path path;
};

void block_map_init(const struct transaction *tr,
                    struct block_map *block_map,
                    const struct block_mac *root,
                    size_t block_size);

bool block_map_get(struct transaction *tr,
                   struct block_map *block_map,
                   data_block_t index,
                   struct block_mac *block_mac);

void block_map_set(struct transaction *tr, struct block_map *block_map,
                   data_block_t index, const struct block_mac *block_mac);

void block_map_put_dirty(struct transaction *tr, struct block_map *block_map,
                         data_block_t index, void *data, obj_ref_t *data_ref);

void block_map_truncate(struct transaction *tr,
                        struct block_map *block_map,
                        data_block_t index);

void block_map_free(struct transaction *tr, struct block_map *block_map);
