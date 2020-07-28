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
#include <compiler.h>
#include <err.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "array.h"
#include "block_allocator.h"
#include "block_set.h"
#include "debug.h"
#include "file.h"
#include "transaction.h"

bool print_merge_free;

/**
 * transaction_check_free
 * @tr:         Transaction object.
 * @set:        Free set to check.
 * @min_free    Number of block that must be in @set
 *
 * Return: %true is @set contains @min_free or more blocks, %false otherwise.
 */
static bool transaction_check_free(struct transaction *tr,
                                   struct block_set *set,
                                   data_block_t min_free)
{
    data_block_t next_block;
    struct block_range free_range;
    data_block_t count;

    next_block = 0;
    while (true) {
        free_range = block_set_find_next_range(tr, set, next_block);
        if (block_range_empty(free_range)) {
            return false;
        }
        count = free_range.end - free_range.start;
        if (count >= min_free) {
            return true;
        }
        min_free -= count;
        next_block = free_range.end;
    }
}

/**
 * transaction_merge_free_sets
 * @tr:         Transaction object.
 * @new_set:    Output set.
 * @set_i:      Initial set.
 * @set_d:      Set of blocks to delete.
 * @set_a:      Set of blocks to add.
 *
 * Helper function to update the free_set when committing a transaction.
 * @new_set = @set_i - @set_d - @new_set-blocks + @set_a + set_[ida]-blocks
 * The start state of @new_set must be a copy-on-write verison of @set_i.
 */
static void transaction_merge_free_sets(struct transaction *tr,
                                        struct block_set *new_set,
                                        struct block_set *set_i,
                                        struct block_set *set_d,
                                        struct block_set *set_a)
{
    data_block_t next_block;
    struct block_range delete_range = BLOCK_RANGE_INITIAL_VALUE(delete_range);
    struct block_range add_range = BLOCK_RANGE_INITIAL_VALUE(add_range);

    full_assert(block_set_check(tr, set_i));
    full_assert(block_set_check(tr, set_d));
    full_assert(block_set_check(tr, set_a));

    assert(!block_mac_valid(tr, &set_i->block_tree.root) ||
           transaction_block_need_copy(tr, block_mac_to_block(tr, &set_i->block_tree.root)));
    if (print_merge_free) {
        printf("%s\n", __func__);
    }

    /* TODO: don't walk the whole tree each time */
    next_block = 1;
    while (next_block != 0) {
        tr->min_free_block = next_block;
        delete_range = block_set_find_next_range(tr, set_d, next_block);
        add_range = block_set_find_next_range(tr, set_a, next_block);
        if (print_merge_free) {
            printf("%s: add %lld-%lld or delete %lld-%lld\n",
                   __func__, add_range.start, add_range.end - 1,
                   delete_range.start, delete_range.end - 1);
        }
        assert(!block_range_overlap(delete_range, add_range));
        if (block_range_before(delete_range, add_range)) {
            assert(delete_range.start >= next_block);
            tr->min_free_block = delete_range.end;
            block_allocator_suspend_set_updates(tr);
            block_set_remove_range(tr, new_set, delete_range);
            block_allocator_process_queue(tr);
            next_block = delete_range.end;
        } else if (!block_range_empty(add_range)) {
            assert(add_range.start >= next_block);
            tr->min_free_block = add_range.end;
            block_allocator_suspend_set_updates(tr);
            block_set_add_range(tr, new_set, add_range);
            block_allocator_process_queue(tr);
            next_block = add_range.end;
        } else {
            assert(block_range_empty(delete_range));
            assert(block_range_empty(add_range));
            next_block = 0;
        }
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }
    }
    full_assert(block_set_check(tr, new_set));
}

/**
 * transaction_block_need_copy - Check if block needs copy
 * @tr:         Transaction object.
 * @block:      Block number to check.
 *
 * Return: %true if block has not been allocated as a non-tmp block for @tr,
 * %false otherwise.
 */
bool transaction_block_need_copy(struct transaction *tr, data_block_t block)
{
    assert(block);
    assert(!block_set_block_in_set(tr, &tr->tmp_allocated, block));
    assert(!block_allocator_allocation_queued(tr, block, true));

    return !block_set_block_in_set(tr, &tr->allocated, block) &&
           !block_allocator_allocation_queued(tr, block, false);
}

/**
 * transaction_delete_active - Remove transaction from active lists (internal)
 * @tr:         Transaction object.
 */
static void transaction_delete_active(struct transaction *tr)
{
    assert(list_in_list(&tr->allocated.node));
    assert(list_in_list(&tr->tmp_allocated.node));
    list_delete(&tr->allocated.node);
    list_delete(&tr->tmp_allocated.node);
}

/**
 * transaction_fail - Fail transaction
 * @tr:         Transaction object.
 *
 * Marks transaction as failed, removes it from active list, discards dirty
 * cache entries and restore open files to last committed state.
 */
void transaction_fail(struct transaction *tr)
{
    assert(!tr->failed);

    tr->failed = true;

    if (tr->complete) {
        return;
    }

    block_cache_discard_transaction(tr, true);
    transaction_delete_active(tr);
    file_transaction_failed(tr);
}

/**
 * transaction_free - Free transaction
 * @tr:         Transaction object.
 *
 * Prepare @tr for free. @tr must not be active and all open files must already
 * be closed.
 */
void transaction_free(struct transaction *tr)
{
    assert(!transaction_is_active(tr));
    assert(list_is_empty(&tr->open_files));
    assert(list_in_list(&tr->node));
    list_delete(&tr->node);
}

/**
 * check_free_tree - Check tree of free set (internal)
 * @tr:         Transaction object.
 * @free:       Set object.
 *
 * Check that the blocks used by the tree for a free set are not in the same
 * set.
 */
static void check_free_tree(struct transaction *tr, struct block_set *free)
{
    uint i;
    struct block_tree_path path;

    block_tree_walk(tr, &free->block_tree, 0, true, &path);
    while (block_tree_path_get_key(&path)) {
        for (i = 0; i < path.count; i++) {
            assert(!block_set_block_in_set(tr, free,
                                           block_mac_to_block(tr, &path.entry[i].block_mac)));
        }
        block_tree_path_next(&path);
    }
}

/**
 * transaction_complete - Complete transaction
 * @tr:         Transaction object.
 */
void transaction_complete(struct transaction *tr)
{
    struct block_mac new_files;
    struct transaction *tmp_tr;
    struct transaction *other_tr;
    struct block_set new_free_set = BLOCK_SET_INITIAL_VALUE(new_free_set);
    bool super_block_updated;

    assert(tr->fs);
    assert(!tr->complete);

    //printf("%s: %lld\n", __func__, tr->version);

    block_set_copy(tr, &new_free_set, &tr->fs->free);

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err_transaction_failed;
    }

    assert(transaction_is_active(tr));

    file_transaction_complete(tr, &new_files);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err_transaction_failed;
    }

    tr->new_free_set = &new_free_set;
    transaction_merge_free_sets(tr, &new_free_set, &tr->fs->free, &tr->allocated, &tr->freed);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err_transaction_failed;
    }

    if (!transaction_check_free(tr, &new_free_set, tr->fs->reserved_count)) {
        if (!tr->failed) {
            transaction_fail(tr);
        }
        pr_warn("transaction would leave fs too full, abort\n");
        goto err_transaction_failed;
    }

    if (0) {
        printf("%s: old free:\n", __func__);
        block_set_print(tr, &tr->fs->free);
        printf("%s: tmp_allocated:\n", __func__);
        block_set_print(tr, &tr->tmp_allocated);
        printf("%s: allocated:\n", __func__);
        block_set_print(tr, &tr->allocated);
        printf("%s: freed:\n", __func__);
        block_set_print(tr, &tr->freed);
        printf("%s: new free:\n", __func__);
        block_set_print(tr, &new_free_set);
    }

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err_transaction_failed;
    }

    block_cache_clean_transaction(tr);

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err_transaction_failed;
    }

    assert(block_range_empty(new_free_set.initial_range));
    check_free_tree(tr, &new_free_set);

    super_block_updated = update_super_block(tr, &new_free_set.block_tree.root,
                                             &new_files);
    if (!super_block_updated) {
        assert(tr->failed);
        pr_warn("failed to update super block, abort\n");
        goto err_transaction_failed;
    }
    block_cache_clean_transaction(tr);

    /*
     * If an error was detected writing the super block, it is not safe to
     * continue as we do not know if the write completed.
     */
    assert(!tr->failed);

    tr->fs->free.block_tree.root = new_free_set.block_tree.root;
    block_range_clear(&tr->fs->free.initial_range); /* clear for initial file-system state */
    tr->fs->files.root = new_files;
    tr->fs->super_block_version = tr->fs->written_super_block_version;

    transaction_delete_active(tr);
    tr->complete = true;

    file_transaction_success(tr);
    assert(!tr->failed);

    check_free_tree(tr, &tr->fs->free);

    list_for_every_entry_safe(&tr->fs->transactions, other_tr, tmp_tr, struct transaction, node) {
        if (tr->failed) {
            break;
        }
        if (!transaction_is_active(other_tr)) {
            continue;
        }
        if (block_set_overlap(tr, &tr->freed, &other_tr->freed)) {
            pr_warn("tr %p, fail conflicting transaction: %p\n", tr, other_tr);
            transaction_fail(other_tr);
        }
    }
    if (tr->failed) {
        pr_warn("tr %p, transaction failed while failing conflicting transactions\n",
                tr);
        tr->failed = false;
        list_for_every_entry_safe(&tr->fs->transactions, other_tr, tmp_tr, struct transaction, node) {
            if (!transaction_is_active(other_tr)) {
                continue;
            }
            pr_warn("tr %p, fail possibly conflicting transaction: %p\n",
                    tr, other_tr);
            transaction_fail(other_tr);
        }
    }
    assert(!tr->failed);
    block_cache_discard_transaction(tr, false);

err_transaction_failed:
    if (tr->failed) {
        file_transaction_complete_failed(tr);
    }
    assert(!block_cache_debug_get_ref_block_count());
}

/**
 * transaction_activate - Activate transaction
 * @tr:         Transaction object.
 */
void transaction_activate(struct transaction *tr)
{
    size_t block_num_size;
    size_t block_mac_size;

    assert(tr->fs);
    assert(!transaction_is_active(tr));

    block_num_size = tr->fs->block_num_size;
    block_mac_size = block_num_size + tr->fs->mac_size;

    tr->failed = false;
    tr->complete = false;
    tr->min_free_block = 0;
    tr->last_free_block = 0;
    tr->last_tmp_free_block = 0;
    tr->new_free_set = NULL;

    block_set_init(tr->fs, &tr->tmp_allocated);
    block_set_init(tr->fs, &tr->allocated);
    block_set_init(tr->fs, &tr->freed);

    block_tree_init(&tr->files_added, tr->fs->dev->block_size,
                    block_num_size, block_mac_size, block_mac_size);
    block_tree_init(&tr->files_updated, tr->fs->dev->block_size,
                    block_num_size, block_mac_size, block_mac_size);
    block_tree_init(&tr->files_removed, tr->fs->dev->block_size,
                    block_num_size, block_mac_size, block_mac_size);

    list_add_tail(&tr->fs->allocated, &tr->allocated.node);
    list_add_tail(&tr->fs->allocated, &tr->tmp_allocated.node);
}

/**
 * transaction_init - Initialize new transaction object
 * @tr:         Transaction object.
 * @fs:         File system state object.
 * @activate:   Activate transaction
 */
void transaction_init(struct transaction *tr,
                      struct fs *fs,
                      bool activate)
{
    assert(fs);
    assert(fs->dev);

    memset(tr, 0, sizeof(*tr));
    tr->fs = fs;

    list_initialize(&tr->open_files);
    list_add_tail(&fs->transactions, &tr->node);

    if (activate) {
        transaction_activate(tr);
    }
}
