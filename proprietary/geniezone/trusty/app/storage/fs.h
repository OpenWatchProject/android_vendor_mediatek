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

#if BUILD_STORAGE_TEST
#define FULL_ASSERT 1
#else
#define FULL_ASSERT 0
#endif
#if FULL_ASSERT
#define full_assert assert
#else
#define full_assert(x) \
    do { } while(0)
#endif

#include "block_mac.h"
#include "block_set.h"
#include "block_tree.h"

/**
 * struct fs - File system state
 * @dev:                            Main block device.
 * @transactions:                   Transaction list.
 * @allocated:                      List of block sets containing blocks
 *                                  allocated by active transactions.
 * @free:                           Block set of free blocks.
 * @files:                          B+ tree of all files.
 * @super_dev:                      Block device used to store super blocks.
 * @key:                            Key to use for encrypt, decrypt and mac.
 * @super_block:                    Block numbers in @super_dev to store
 *                                  super-block in.
 * @super_block_version:            Last read or written super block version.
 * @written_super_block_version:    Last written super block version.
 * @min_block_num:                  First block number that can store non
 *                                  super blocks.
 * @block_num_size:                 Number of bytes used to store block numbers.
 * @mac_size:                       Number of bytes used to store mac values.
 *                                  Must be 16 if @dev is not tamper_detecting.
 * @reserved_count:                 Number of free blocks reserved for active
 *                                  transactions.
 */

struct fs {
    struct block_device *dev;
    struct list_node transactions;
    struct list_node allocated;
    struct block_set free;
    struct block_tree files;
    struct block_device *super_dev;
    const struct key *key;
    data_block_t super_block[2];
    uint super_block_version;
    uint written_super_block_version;
    data_block_t min_block_num;
    size_t block_num_size;
    size_t mac_size;
    data_block_t reserved_count;
};

bool update_super_block(struct transaction *tr,
                        const struct block_mac *free,
                        const struct block_mac *files);

int fs_init(struct fs *fs,
            const struct key *key,
            struct block_device *dev,
            struct block_device *super_dev,
            bool clear);
