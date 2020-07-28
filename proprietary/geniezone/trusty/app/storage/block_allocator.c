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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "block_allocator.h"
#include "debug.h"
#include "transaction.h"

/*
 * BLOCK_ALLOCATOR_QUEUE_LEN:
 * Queue length large enough for a worst case tree update, e.g. update of a tree
 * where each entry needs to be copied then split.
 */
#define BLOCK_ALLOCATOR_QUEUE_LEN \
    (BLOCK_SET_MAX_DEPTH * 2 * BLOCK_SET_MAX_DEPTH * 2)

/**
 * struct block_allocator_queue_entry - pending block allocation set update
 * @block:      block number to free or allocate.
 * @tmp:        is_tmp value passed to block_allocate_etc or block_free_etc.
 * @free:       @block is free.
 * @removed:    queue entry was removed.
 */
struct block_allocator_queue_entry {
    data_block_t block;
    bool tmp;
    bool free;
    bool removed;
};

/**
 * struct block_allocator_queue - queue of pending block allocation set updates
 * @entry:      Ring buffer.
 * @head        Index in @entry of first (oldest) entry in queue.
 * @tail:       Index in @entry where next entry should be inserted.
 * @updating:   %true while updating sets, false otherwise.
 */
struct block_allocator_queue {
    struct block_allocator_queue_entry entry[BLOCK_ALLOCATOR_QUEUE_LEN];
    uint head;
    uint tail;
    bool updating;
};

/**
 * block_allocator_queue_empty - Check if block allocator queue is empty
 * @q:          Queue object.
 *
 * Return: %true if @q is empty, %false otherwise.
 */
static bool block_allocator_queue_empty(struct block_allocator_queue *q)
{
    assert(q->head < countof(q->entry));
    assert(q->tail < countof(q->entry));

    return q->head == q->tail;
}

/**
 * block_allocator_queue_count - Get number of entries in block allocator queue
 * @q:          Queue object.
 *
 * Return: number of etnries in @q.
 */
static uint block_allocator_queue_count(struct block_allocator_queue *q)
{
    return (q->tail + countof(q->entry) - q->head) % countof(q->entry);
}

/**
 * block_allocator_queue_find - Search block allocator queue
 * @q:          Queue object.
 * @block:      Block number to search for.
 *
 * Return: If block is in @q index in @q->entry of matching entry, -1 if @block
 * is not in @q.
 */
static int block_allocator_queue_find(struct block_allocator_queue *q,
                                      data_block_t block)
{
    uint i;

    assert(q->head < countof(q->entry));
    assert(q->tail < countof(q->entry));

    for (i = q->head; i != q->tail; i = (i + 1) % countof(q->entry)) {
        if (q->entry[i].removed) {
            continue;
        }
        if (block == q->entry[i].block) {
            return i;
        }
    }
    return -1;
}

/**
 * block_allocator_queue_add_dummy - Add a dummy entry to block allocator queue
 * @q:          Queue object.
 *
 * Add a dummy entry to @q to make it non-empty.
 */
static void block_allocator_queue_add_dummy(struct block_allocator_queue *q)
{
    assert(block_allocator_queue_empty(q));
    uint new_tail = (q->tail + 1) % countof(q->entry);
    q->entry[q->tail].removed = true;
    q->tail = new_tail;
    pr_write("index %d\n", q->tail);
}

/**
 * block_allocator_queue_add - Add a entry to block allocator queue
 * @q:          Queue object.
 * @block:      block number to free or allocate.
 * @is_tmp:     is_tmp value passed to block_allocate_etc or block_free_etc.
 * @is_free:    @block is free.
 */
static void block_allocator_queue_add(struct block_allocator_queue *q,
                                      data_block_t block,
                                      bool is_tmp,
                                      bool is_free)
{
    int ret;
    uint index;
    uint new_tail;

    ret = block_allocator_queue_find(q, block);
    if (ret >= 0) {
        index = ret;
        assert(index < countof(q->entry));
        assert(q->entry[index].tmp == is_tmp);
        assert(q->entry[index].free != is_free);

        q->entry[index].removed = true;
        if (index != q->head || !q->updating) {
            return;
        }
        pr_warn("block %lld, tmp %d, free %d, removed head during update\n",
                block, is_tmp, is_free);
    }

    new_tail = (q->tail + 1) % countof(q->entry);

    assert(q->head < countof(q->entry));
    assert(q->tail < countof(q->entry));
    assert(new_tail < countof(q->entry));
    assert(new_tail != q->head);

    q->entry[q->tail].block = block;
    assert(q->entry[q->tail].block == block);
    q->entry[q->tail].tmp = is_tmp;
    q->entry[q->tail].free = is_free;
    q->entry[q->tail].removed = false;
    q->tail = new_tail;
    pr_write("block %lld, tmp %d, free %d, index %d (rel %d)\n",
             block, is_tmp, is_free, q->tail, block_allocator_queue_count(q));
}

/**
 * block_allocator_queue_peek_head - Get head block allocator queue entry
 * @q:          Queue object.
 *
 * Return: entry at head of @q.
 */
static struct block_allocator_queue_entry
block_allocator_queue_peek_head(struct block_allocator_queue *q)
{
    assert(!block_allocator_queue_empty(q));

    return q->entry[q->head];
}

/**
 * block_allocator_queue_remove_head - Remove head block allocator queue entry
 * @q:          Queue object.
 * @entry:      Entry that should be at head of @q (used for validation only).
 */
static void
block_allocator_queue_remove_head(struct block_allocator_queue *q,
                                  struct block_allocator_queue_entry entry)
{
    assert(block_allocator_queue_peek_head(q).block == entry.block);

    pr_write("index %d, count %d\n", q->head, block_allocator_queue_count(q));
    q->head = (q->head + 1) % countof(q->entry);
}

/**
 * block_allocator_queue_find_free_block - Search allocator queue for free block
 * @q:          Queue object.
 * @block:      Block number to search for.
 *
 * Return: First block >= @block that is not in @q as an allocated block.
 */
static data_block_t
block_allocator_queue_find_free_block(struct block_allocator_queue *q,
                                      data_block_t block)
{
    int index;

    while (true) {
        index = block_allocator_queue_find(q, block);
        if (index < 0) {
            return block;
        }
        if (q->entry[index].free) {
            return block;
        }
        block++;
    }
}

static struct block_allocator_queue block_allocator_queue;

/**
 * find_free_block - Search for a free block
 * @tr:             Transaction object.
 * @min_block_in:   Block number to start search at.
 *
 * Return: Block number that is in commited free set and not already allocated
 * by any transaction.
 */
static data_block_t find_free_block(struct transaction *tr,
                                    data_block_t min_block_in)
{
    data_block_t block;
    data_block_t min_block = min_block_in;
    struct block_set *set;

    assert(list_in_list(&tr->node)); /* transaction must be active */

    pr_read("min_block %lld\n", min_block);

    block = min_block;
    do {
        block = block_set_find_next_block(tr, &tr->fs->free, block, true);
        if (tr->failed) {
            return 0;
        }
        if (block < min_block) {
            assert(!block);

            if (LOCAL_TRACE >= TRACE_LEVEL_READ) {
                if (min_block_in) {
                    block = find_free_block(tr, 0);
                }
                printf("%s: no space, min_block %lld, free block ignoring_min_block %lld\n",
                       __func__, min_block_in, block);

                printf("%s: free\n", __func__);
                block_set_print(tr, &tr->fs->free);
                list_for_every_entry(&tr->fs->allocated, set, struct block_set, node) {
                    printf("%s: allocated %p\n", __func__, set);
                    block_set_print(tr, set);
                }
                if (tr->new_free_set) {
                    printf("%s: new free\n", __func__);
                    block_set_print(tr, tr->new_free_set);
                }
            }

            return 0;
        }

        min_block = block;

        pr_read("check free block %lld\n", block);

        assert(!list_is_empty(&tr->fs->allocated));
        list_for_every_entry(&tr->fs->allocated, set, struct block_set, node) {
            block = block_set_find_next_block(tr, set, block, false);
            if (tr->failed) {
                return 0;
            }
            assert(block >= min_block);
        };
        block = block_allocator_queue_find_free_block(&block_allocator_queue, block);
    } while (block != min_block);

    pr_read("found free block %lld\n", block);

    return block;
}

/**
 * block_allocate_etc - Allocate a block
 * @tr:         Transaction object.
 * @is_tmp:     %true if allocated block should be automatically freed when
 *              transaction completes, %false if allocated block should be added
 *              to free set when transaction completes.
 *
 * Find a free block and add queue a set update.
 *
 * Return: Allocated block number.
 */
data_block_t block_allocate_etc(struct transaction *tr, bool is_tmp)
{
    data_block_t block;
    data_block_t min_block;
    bool update_sets;

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");

        return 0;
    }
    assert(transaction_is_active(tr));

    /* TODO: group allocations by set */
    update_sets = block_allocator_queue_empty(&block_allocator_queue);
    if (update_sets) {
        tr->last_tmp_free_block = tr->fs->dev->block_count / 4 * 3;
        tr->last_free_block = 0;
    }
    min_block = is_tmp ? tr->last_tmp_free_block : tr->last_free_block;

    block = find_free_block(tr, min_block);
    if (!block) {
        block = find_free_block(tr, 0);
        if (!block) {
            if (!tr->failed) {
                pr_err("no space\n");
                transaction_fail(tr);
            }
            return 0;
        }
    }

    block_allocator_queue_add(&block_allocator_queue, block, is_tmp, false);
    full_assert (find_free_block(tr, block) != block);
    if (update_sets) {
        block_allocator_process_queue(tr);
    }

    full_assert (tr->failed || find_free_block(tr, block) != block);
    if (tr->failed) {
        return 0;
    }

    return block;
}

/**
 * block_allocator_add_allocated - Update block sets with new allocated block
 * @tr:         Transaction object.
 * @block:      Block that was allocated.
 * @is_tmp:     %true if allocated block should be automatically freed when
 *              transaction completes, %false if allocated block should be added
 *              to free set when transaction completes.
 *
 * Add @block to the allocated set selected by @is_tmp. If called while
 * completing the transaction update the new free set directly if needed.
 */
static void block_allocator_add_allocated(struct transaction *tr,
                                          data_block_t block,
                                          bool is_tmp)
{

    if (is_tmp) {
        pr_write("add %lld to tmp_allocated\n", block);

        block_set_add_block(tr, &tr->tmp_allocated, block);
        tr->last_tmp_free_block = block + 1;
    } else {
        pr_write("add %lld to allocated\n", block);

        block_set_add_block(tr, &tr->allocated, block);

        if (block < tr->min_free_block) {
            pr_write("remove %lld from new_free_set\n", block);

            assert(tr->new_free_set);
            block_set_remove_block(tr, tr->new_free_set, block);
            tr->last_free_block = block + 1;
        }
    }
}

/**
 * block_free_etc - Free a block
 * @tr:         Transaction object.
 * @block:      Block that should be freed.
 * @is_tmp:     Must match is_tmp value passed to block_allocate_etc (always
 *              %false if @block was not allocated by this transaction).
 */
void block_free_etc(struct transaction *tr, data_block_t block, bool is_tmp)
{
    bool update_sets = block_allocator_queue_empty(&block_allocator_queue);

    assert(block_is_clean(tr->fs->dev, block));

    block_allocator_queue_add(&block_allocator_queue, block, is_tmp, true);
    if (update_sets) {
        block_allocator_process_queue(tr);
    }
}

/**
 * block_allocator_add_free - Update block sets with new free block
 * @tr:         Transaction object.
 * @block:      Block that should be freed.
 * @is_tmp:     Must match is_tmp value passed to block_allocate_etc (always
 *              %false if @block was not allocated by this transaction).
 *
 * If @block was allocated in this transaction, remove it from the allocated set
 * (selected by @is_tmp). Otherwise add it to the set of blocks to remove when
 * transaction completes. If called while completing the transaction update the
 * new free set directly if needed.
 */
static void block_allocator_add_free(struct transaction *tr,
                                     data_block_t block,
                                     bool is_tmp)
{
    assert(block_is_clean(tr->fs->dev, block));
    if (is_tmp) {
        assert(!block_set_block_in_set(tr, &tr->allocated, block));
        assert(!block_set_block_in_set(tr, &tr->freed, block));

        pr_write("remove %lld from tmp_allocated\n", block);

        block_set_remove_block(tr, &tr->tmp_allocated, block);

        return;
    }

    assert(!block_set_block_in_set(tr, &tr->tmp_allocated, block));
    if (block_set_block_in_set(tr, &tr->allocated, block)) {
        pr_write("remove %lld from allocated\n", block);

        block_set_remove_block(tr, &tr->allocated, block);

        if (block < tr->min_free_block) {
            pr_write("add %lld to new_free_root\n", block);

            assert(tr->new_free_set);
            block_set_add_block(tr, tr->new_free_set, block);
        }
    } else {
        if (block < tr->min_free_block) {
            pr_write("add %lld to new_free_root\n", block);

            assert(tr->new_free_set);
            block_set_add_block(tr, tr->new_free_set, block);
        } else {
            pr_write("add %lld to freed\n", block);

            block_set_add_block(tr, &tr->freed, block);
        }
    }
}

/**
 * block_allocator_suspend_set_updates - Prevent set updates
 * @tr:         Transaction object.
 */
void block_allocator_suspend_set_updates(struct transaction *tr)
{
    block_allocator_queue_add_dummy(&block_allocator_queue);
}

/**
 * block_allocator_allocation_queued - Check for queued block allocation
 * @tr:         Transaction object.
 * @block:      Block to serach for.
 * @is_tmp:     is_tmp value passed to block_allocate_etc.
 *
 * Return: %true if a block matching @block, @is_tmp and !free is in the
 * block allocator queue, %false otherwise.
 */
bool block_allocator_allocation_queued(struct transaction *tr,
                                       data_block_t block,
                                       bool is_tmp)
{
    int index;
    index = block_allocator_queue_find(&block_allocator_queue, block);
    return index >= 0 &&
           block_allocator_queue.entry[index].tmp == is_tmp &&
           !block_allocator_queue.entry[index].free;
}

/**
 * block_allocator_process_queue - Process all block allocator queue entries
 * @tr:         Transaction object.
 */
void block_allocator_process_queue(struct transaction *tr)
{
    struct block_allocator_queue_entry entry;
    int loop_limit = BLOCK_SET_MAX_DEPTH * BLOCK_SET_MAX_DEPTH * BLOCK_SET_MAX_DEPTH + tr->fs->dev->block_count;

    assert(!block_allocator_queue.updating);

    block_allocator_queue.updating = true;
    while (!block_allocator_queue_empty(&block_allocator_queue)) {
        assert(loop_limit-- > 0);
        entry = block_allocator_queue_peek_head(&block_allocator_queue);
        pr_write("block %lld, tmp %d, free %d, removed %d\n",
                 (data_block_t)entry.block, entry.tmp,
                 entry.free, entry.removed);
        if (entry.removed) {
        } else if (entry.free) {
            block_allocator_add_free(tr, entry.block, entry.tmp);
        } else {
            block_allocator_add_allocated(tr, entry.block, entry.tmp);
        }
        block_allocator_queue_remove_head(&block_allocator_queue, entry);
    }
    block_allocator_queue.updating = false;
}
