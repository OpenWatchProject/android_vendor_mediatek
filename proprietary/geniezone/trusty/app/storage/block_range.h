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

#include "block_device.h" /* for data_block_t */

/**
 * struct block_range - Struct describing a range of blocks
 * @start:      First block in range.
 * @end:        Last block in range plus one.
 */
struct block_range {
    data_block_t start;
    data_block_t end;
};
#define BLOCK_RANGE_INITIAL_VALUE(block_range) {0,0}

/**
 * block_range_empty - Check if block range is empty
 * @range:      Block range to check.
 *
 * Return: %true if @range is empty, %false otherwise.
 */
static inline bool block_range_empty(const struct block_range range)
{
    assert(range.end >= range.start);

    return range.start == range.end;
}

/*
 * block_in_range - Check if block is in range
 * @range:      Block range to check.
 * @block:      Block to check.
 *
 * Return: %true if @block is in @range, %false otherwise.
 */
static inline bool block_in_range(const struct block_range range,
                                  data_block_t block)
{
    assert(range.end >= range.start);

    return (block >= range.start && block < range.end);
}

/**
 * block_in_range - Check if two block ranges have any overlap
 * @a:          Block range to check.
 * @b:          Block range to check.
 *
 * Return: %true if @a and @b share any blocks, %false otherwise.
 */
static inline bool block_range_overlap(const struct block_range a,
                                       const struct block_range b)
{
    return block_in_range(a, b.start) || block_in_range(b, a.start);
}

/**
 * block_range_before - Check if a block range start before another block range
 * @a:          Block range to check.
 * @b:          Block range to check.
 *
 * Return: %true if @a starts at a lower block number than @b where the start
 * block number of an empty block range is considered infinite, %false
 * otherwise.
 */
static inline bool block_range_before(const struct block_range a,
                                      const struct block_range b)
{
    return !block_range_empty(a) && (block_range_empty(b) || a.start < b.start);
}

/**
 * block_range_is_sub_range - Check if a block range is a subset of another range
 * @range:      Block range to check.
 * @sub_range:  Block range to check.
 *
 * Return: %true if every block in @sub_range is also in @range, %false
 * otherwise. @sub_range is not allowed to be be empty.
 */
static inline bool block_range_is_sub_range(const struct block_range range,
                                            const struct block_range sub_range)
{
    assert(!block_range_empty(sub_range));
    return block_in_range(range, sub_range.start) &&
           block_in_range(range, sub_range.end - 1);
}

/**
 * block_range_eq - Check if two block ranges are identical
 * @a:          Block range to check.
 * @b:          Block range to check.
 *
 * Return: %true if @a and @b are identical, %false otherwise.
 */
static inline bool block_range_eq(const struct block_range a,
                                  const struct block_range b)
{
    assert(a.end >= a.start);
    assert(b.end >= b.start);

    return a.start == b.start && a.end == b.end;
}

/**
 * block_range_init_single - Initialize a block range containing a single block
 * @range:      Block range object to initialize.
 * @block:      Block that should be in @range.
 */
static inline void block_range_init_single(struct block_range *range,
                                           data_block_t block)
{
    range->start = block;
    range->end = block + 1;
}

/**
 * block_range_clear - Remove all blocks from a block range
 * @range:      Block range object to clear.
 */
static inline void block_range_clear(struct block_range *range)
{
    range->start = 0;
    range->end = 0;
}
