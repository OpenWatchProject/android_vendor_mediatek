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

#include <assert.h>

#include "block_allocator.h"
#include "block_map.h"
#include "block_tree.h"
#include "debug.h"
#include "transaction.h"

static bool print_block_map;

/**
 * block_map_init - Initialize in-memory block map structute
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @root:       Block mac of root tree.
 * @block_size: Block size of device.
 */

void block_map_init(const struct transaction *tr,
                    struct block_map *block_map,
                    const struct block_mac *root,
                    size_t block_size)
{
    size_t block_num_size = tr->fs->block_num_size;
    size_t block_mac_size = block_num_size + tr->fs->mac_size;
    memset(block_map, 0, sizeof(*block_map));
    block_tree_init(&block_map->tree, block_size,
                    block_num_size, block_mac_size, block_mac_size);
    block_map->tree.copy_on_write = 1;
    block_map->tree.allow_copy_on_write = 1;
    block_map->tree.root = *root;
}

/**
 * block_map_get - Lookup a block
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @index:      Index of block to get.
 * @block_mac:  Pointer to return block_mac in.
 *
 * Return: %true if a block_mac exists at @index, %false if not. When returning
 * %true, @block_mac will be filled in. Otherwise, @block_mac is not touched.
 */
bool block_map_get(struct transaction *tr,
                   struct block_map *block_map,
                   data_block_t index,
                   struct block_mac *block_mac)
{
    struct block_tree_path path;

    index++; /* 0 is not a valid block tree key */

    block_tree_walk(tr, &block_map->tree, index, false, &path);
    if (block_tree_path_get_key(&path) != index) {
        if (print_block_map) {
            printf("%s: %lld not found (next key %lld)\n",
                   __func__, index, block_tree_path_get_key(&path));
        }
        return false;
    }
    *block_mac = block_tree_path_get_data_block_mac(&path);

    return true;
}

/**
 * block_map_set - Store a block_mac
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @index:      Index of block to set.
 * @block_mac:  block_mac to store, or %NULL to remove the block_mac at @index.
 */
void block_map_set(struct transaction *tr, struct block_map *block_map,
                   data_block_t index, const struct block_mac *block_mac)
{
    struct block_tree_path path;

    index++; /* 0 is not a valid block tree key */

    if (tr->failed) {
        pr_warn("transaction failed, ignore\n");
        return;
    }

    block_tree_walk(tr, &block_map->tree, index, false, &path);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        return;
    }
    if (block_tree_path_get_key(&path) == index) {
        if (print_block_map) {
            printf("%s: block_map at %lld: remove existing entry at %lld\n",
                   __func__, block_mac_to_block(tr, &block_map->tree.root), index);
        }
        block_tree_remove(tr, &block_map->tree, index, block_tree_path_get_data(&path));
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }
    }
    if (block_mac && block_mac_valid(tr, block_mac)) {
        if (print_block_map) {
            printf("%s: block_map at %lld: [%lld] = %lld\n",
                   __func__, block_mac_to_block(tr, &block_map->tree.root),
                   index, block_mac_to_block(tr, block_mac));
        }
        block_tree_insert(tr, &block_map->tree, index, block_mac_to_block(tr, block_mac));
        /* TODO: insert mac */
    }
}

/**
 * block_map_put_dirty - Release a block stored in a block_map, and update mac
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @index:      Index of block to update.
 * @data:       block cache entry.
 * @data_ref:   reference to @data.
 */
void block_map_put_dirty(struct transaction *tr, struct block_map *block_map,
                         data_block_t index, void *data, obj_ref_t *data_ref)
{
    struct block_tree_path path;

    index++; /* 0 is not a valid block tree key */

    block_tree_walk(tr, &block_map->tree, index, false, &path);
    if (tr->failed) {
        pr_warn("transaction failed\n");
        block_put_dirty_discard(data, data_ref);
        return;
    }

    if (print_block_map) {
        printf("%s: %lld (found key %lld)\n",
               __func__, index, block_tree_path_get_key(&path));
    }

    assert(block_tree_path_get_key(&path) == index);
    block_tree_path_put_dirty(tr, &path, path.count, data, data_ref);
}

/**
 * block_map_truncate - Free blocks
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @index:      Index to start freeing at.
 *
 * Remove and free all blocks starting at @index.
 */
void block_map_truncate(struct transaction *tr,
                        struct block_map *block_map,
                        data_block_t index)
{
    struct block_tree_path path;
    data_block_t key;
    data_block_t data;
    data_block_t curr_index;

    curr_index = index + 1; /* 0 is not a valid block tree key */

    while (true) {
        block_tree_walk(tr, &block_map->tree, curr_index, false, &path);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }
        key = block_tree_path_get_key(&path);
        if (!key) {
            break;
        }
        assert(key >= curr_index);
        data = block_tree_path_get_data(&path);
        if (!data) {
            /* block_tree_walk returned an empty insert spot for curr_index */
            assert(key != curr_index);
            curr_index = key;
            continue;
        }
        assert(data);
        block_tree_remove(tr, &block_map->tree, key, data);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }
        block_discard_dirty_by_block(tr->fs->dev, data);
        block_free(tr, data);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }
    }

    /* Only a root leaf node should remain if truncating to 0 */
    assert(index || path.count == 1);
}

/**
 * block_map_free - Free blocks
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 *
 * Free block_map and all blocks stored in it.
 */
void block_map_free(struct transaction *tr, struct block_map *block_map)
{
    data_block_t root_block;

    if (!block_mac_valid(tr, &block_map->tree.root)) {
        return;
    }
    if (print_block_map) {
        printf("%s: root %lld\n",
               __func__, block_mac_to_block(tr, &block_map->tree.root));
        block_tree_print(tr, &block_map->tree);
    }

    block_map_truncate(tr, block_map, 0);
    if (tr->failed) {
        pr_warn("transaction failed\n");
        return;
    }

    root_block = block_mac_to_block(tr, &block_map->tree.root);

    block_discard_dirty_by_block(tr->fs->dev, root_block);
    block_free(tr, root_block);
}
