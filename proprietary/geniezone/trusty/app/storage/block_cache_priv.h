/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <reflist.h>
#include <stdbool.h>
#include <stdint.h>

#include "block_device.h"
#include "crypt.h"

#ifdef APP_STORAGE_BLOCK_CACHE_SIZE
#define BLOCK_CACHE_SIZE (APP_STORAGE_BLOCK_CACHE_SIZE)
#else
#define BLOCK_CACHE_SIZE (64)
#endif
#ifdef APP_STORAGE_MAIN_BLOCK_SIZE
#define MAX_BLOCK_SIZE  (APP_STORAGE_MAIN_BLOCK_SIZE)
#else
#define MAX_BLOCK_SIZE (2048)
#endif

/**
 * struct block_cache_entry - block cache entry
 * @guard1:                 Set to BLOCK_CACHE_GUARD_1 to detect out of bound
 *                          writes to data.
 * @data:                   Decrypted block data.
 * @guard2:                 Set to BLOCK_CACHE_GUARD_2 to detect out of bound
 *                          writes to data.
 * @key:                    Key to use for encrypt, decrypt and calculate_mac.
 * @dev:                    Device that block was read from and will be written
 *                          to.
 * @block:                  Block number in dev.
 * @block_size:             Size of block, but match dev->block_size.
 * @mac:                    Last calculated mac of encrypted block data.
 * @loaded:                 Data contains valid data.
 * @dirty:                  Data is not on disk and must be written back or
 *                          discarded before cache entry can be reused.
 * @dirty_ref:              Data is currently being modified. Only a single
 *                          reference should be allowed.
 * @dirty_mac:              Data has been modified. Mac needs to be updated
 *                          after encrypting block.
 * @dirty_tmp:              Data can be discarded by
 *                          block_cache_discard_transaction.
 * @dirty_tr:               Transaction that modified block.
 * @obj:                    Reference tracking struct.
 * @lru_node:               List node for tracking least recently used cache
 *                          entries.
 * @io_op_node:             List node for tracking active read and write
 *                          operations.
 * @io_op:                  Currently active io operation.
 */
struct block_cache_entry {
    uint64_t guard1;
    uint8_t data[MAX_BLOCK_SIZE];
    uint64_t guard2;

    const struct key *key;
    struct block_device *dev;
    data_block_t block;
    size_t block_size;
    struct mac mac;
    bool loaded;
    bool encrypted;
    bool dirty;
    bool dirty_ref;
    bool dirty_mac;
    bool dirty_tmp;
    struct transaction *dirty_tr;

    obj_t obj;
    struct list_node lru_node;
    struct list_node io_op_node;
    enum {
        BLOCK_CACHE_IO_OP_NONE,
        BLOCK_CACHE_IO_OP_READ,
        BLOCK_CACHE_IO_OP_WRITE,
    } io_op;
};

#define BLOCK_CACHE_SIZE_BYTES (sizeof(struct block_cache_entry[BLOCK_CACHE_SIZE]))
