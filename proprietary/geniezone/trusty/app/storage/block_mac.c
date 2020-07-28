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

#include <string.h>
#include <openssl/crypto.h> /* for CRYPTO_memcmp */

#include "block_device.h"
#include "block_mac.h"
#include "transaction.h"

static size_t block_mac_block_size(const struct transaction *tr)
{
    size_t size = tr->fs->block_num_size;
    assert(size);
    assert(size <= sizeof(data_block_t));
    return size;
}

static size_t block_mac_mac_size(const struct transaction *tr)
{
    size_t size = tr->fs->mac_size;
    assert(size);
    assert(size <= sizeof(struct mac));
    return size;
}

void block_mac_clear(const struct transaction *tr,
                     struct block_mac *dest)
{
    memset(dest, 0, block_mac_block_size(tr) + block_mac_mac_size(tr));
}

data_block_t block_mac_to_block(const struct transaction *tr,
                                const struct block_mac *block_mac)
{
    data_block_t block = 0;

    memcpy(&block, block_mac->data, block_mac_block_size(tr));

    return block;
}

const void *block_mac_to_mac(const struct transaction *tr,
                             const struct block_mac *block_mac)
{
    return block_mac->data + block_mac_block_size(tr);
}

void block_mac_set_block(const struct transaction *tr,
                         struct block_mac *block_mac,
                         data_block_t block)
{
    memcpy(block_mac->data, &block, block_mac_block_size(tr));
}

void block_mac_set_mac(const struct transaction *tr,
                       struct block_mac *block_mac,
                       const struct mac *mac)
{
    memcpy(block_mac->data + block_mac_block_size(tr), mac,
           block_mac_mac_size(tr));
}


bool block_mac_eq(const struct transaction *tr,
                  const struct block_mac *a,
                  const struct block_mac *b)
{
    return !CRYPTO_memcmp(a, b, block_mac_block_size(tr) + block_mac_mac_size(tr));
}

void block_mac_copy(const struct transaction *tr,
                    struct block_mac *dest,
                    const struct block_mac *src)
{
    memcpy(dest, src, block_mac_block_size(tr) + block_mac_mac_size(tr));
}
