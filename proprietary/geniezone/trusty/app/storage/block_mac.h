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

#include <assert.h>

#include "block_device.h"
#include "crypt.h"

/**
 * struct block_mac - Struct for storing packed block and mac values.
 * @data:       Packed block and mac values. Size of entries is fs specific.
 *
 * The size of this struct is large enough to store any supported block and mac
 * value, but data outside the fs specific block_num_size + mac_size range is
 * not touched. This means the full struct can be allocated where the fs
 * specific sizes are not yet known, and a block_num_size + mac_size size memory
 * block can be allocated where that size is known.
 */
struct block_mac {
    uint8_t data[sizeof(data_block_t) + sizeof(struct mac)];
};
#define BLOCK_MAC_INITIAL_VALUE(block_mac) { {0} }

struct transaction;

void block_mac_clear(const struct transaction *tr,
                     struct block_mac *dest);
data_block_t block_mac_to_block(const struct transaction *tr,
                                const struct block_mac *block_mac);
const void *block_mac_to_mac(const struct transaction *tr,
                             const struct block_mac *block_mac);
void block_mac_set_block(const struct transaction *tr,
                         struct block_mac *block_mac,
                         data_block_t block);
void block_mac_set_mac(const struct transaction *tr,
                       struct block_mac *block_mac,
                       const struct mac *mac);
bool block_mac_eq(const struct transaction *tr,
                  const struct block_mac *a,
                  const struct block_mac *b);
void block_mac_copy(const struct transaction *tr,
                    struct block_mac *dest,
                    const struct block_mac *src);

static inline bool block_mac_valid(const struct transaction *tr,
                                   const struct block_mac *block_mac)
{
    return block_mac_to_block(tr, block_mac);
}

static inline bool block_mac_same_block(const struct transaction *tr,
                                        const struct block_mac *a,
                                        const struct block_mac *b)
{
    return block_mac_to_block(tr, a) == block_mac_to_block(tr, b);
}

static inline void block_mac_copy_mac(const struct transaction *tr,
                                      struct block_mac *dest,
                                      const struct block_mac *src)
{
    assert(block_mac_same_block(tr, dest, src));
    block_mac_set_mac(tr, dest, block_mac_to_mac(tr, src));
}
