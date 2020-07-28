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

#include <assert.h>
#include <reflist.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/rand.h>

#include "block_cache.h"
#include "block_cache_priv.h"
#include "crypt.h"
#include "debug.h"
#include "debug_stats.h"
#include "transaction.h"

static bool print_cache_lookup = false;
static bool print_cache_lookup_verbose = false;
static bool print_block_ops = false;
static bool print_block_load = false;
static bool print_block_store = false;
static bool print_block_move = false;
static bool print_block_decrypt_encrypt = false;
static bool print_clean_transaction = false;
static bool print_mac_update = false;
static bool print_cache_get_ref_block_count = true;

#define BLOCK_CACHE_GUARD_1 (0xdead0001dead0003)
#define BLOCK_CACHE_GUARD_2 (0xdead0005dead0007)

static struct list_node block_cache_lru = LIST_INITIAL_VALUE(block_cache_lru);
static struct block_cache_entry *block_cache_entries;

/**
 * block_cache_globalvar_init - Constructor to init global variables for block cache
 *
 * To support secure storage TA restart, the (static) global variables must be
 * initialized before used.
 */
static void __attribute__((constructor)) block_cache_globalvar_init(void)
{
    block_cache_lru.prev = &block_cache_lru;
    block_cache_lru.next = &block_cache_lru;
    block_cache_entries  = NULL;
}

/**
 * block_cache_queue_io_op - Helper function to start a read or write operation
 * @entry:      Cache entry.
 * @io_op:      BLOCK_CACHE_IO_OP_READ or BLOCK_CACHE_IO_OP_WRITE.
 *
 * Set io_op for cache entry and add it to the tail of the io_ops for the
 * block device that the cache entry belongs to.
 */
static void block_cache_queue_io_op(struct block_cache_entry *entry, int io_op)
{
    assert(io_op == BLOCK_CACHE_IO_OP_READ || io_op == BLOCK_CACHE_IO_OP_WRITE);
    assert(entry->io_op == BLOCK_CACHE_IO_OP_NONE);
    assert(entry->dev);
    assert(!list_in_list(&entry->io_op_node));

    entry->io_op = io_op;
    list_add_tail(&entry->dev->io_ops, &entry->io_op_node);
}

/**
 * block_cache_queue_read - Start a read operation
 * @entry:      Cache entry.
 */
static void block_cache_queue_read(struct block_cache_entry *entry)
{
    block_cache_queue_io_op(entry, BLOCK_CACHE_IO_OP_READ);
    stats_timer_start(STATS_CACHE_START_READ);
    entry->dev->start_read(entry->dev, entry->block);
    stats_timer_stop(STATS_CACHE_START_READ);
}

/**
 * block_cache_queue_write - Start a write operation
 * @entry:      Cache entry.
 */
static void block_cache_queue_write(struct block_cache_entry *entry,
                                    const void *encrypted_data)
{
    block_cache_queue_io_op(entry, BLOCK_CACHE_IO_OP_WRITE);
    stats_timer_start(STATS_CACHE_START_WRITE);
    entry->dev->start_write(entry->dev, entry->block,
                            encrypted_data, entry->block_size);
    stats_timer_stop(STATS_CACHE_START_WRITE);
}

/**
 * block_cache_complete_io - Wait for io operation on block device to complete
 * @dev:        Block device to wait for
 */
static void block_cache_complete_io(struct block_device *dev)
{
    while (!list_is_empty(&dev->io_ops)) {
        assert(dev->wait_for_io);
        dev->wait_for_io(dev);
    }
}

/**
 * block_cache_pop_io_op - Get cache entry for completed read or write operation
 * @dev:        Block device
 * @block:      Block number
 * @io_op:      BLOCK_CACHE_IO_OP_READ or BLOCK_CACHE_IO_OP_WRITE.
 *
 * Finds block cache entry that matches @dev and @block and remove it from
 * the io_ops queue of the block device.
 *
 * This is a helper function for block_cache_complete_read and
 * block_cache_complete_write.
 *
 * Return: Matching block cache entry.
 */
static struct block_cache_entry *block_cache_pop_io_op(struct block_device *dev,
                                                       data_block_t block,
                                                       uint io_op)
{
    struct block_cache_entry *entry;

    list_for_every_entry(&dev->io_ops, entry, struct block_cache_entry, io_op_node) {
        if (entry->block == block) {
            assert(entry->dev == dev);
            assert(entry->io_op == io_op);
            entry->io_op = BLOCK_CACHE_IO_OP_NONE;
            list_delete(&entry->io_op_node);
            return entry;
        }
        assert(false); /* Out of order completion not expected */
    }
    assert(false); /* No matching entry found */

    return NULL;
}

/**
 * block_cache_complete_read - Read complete callback from block device
 * @dev:        Block device
 * @block:      Block number
 * @data:       Pointer to encrypted data
 * @data_size:  Data size, must match block size of device.
 * @failed:     true if read operation failed, and data is not valid.
 *
 * Calculates mac and decrypts data into cache entry. Does not validate mac.
 */
void block_cache_complete_read(struct block_device *dev,
                               data_block_t block,
                               const void *data,
                               size_t data_size,
                               bool failed)
{
    int ret;
    struct block_cache_entry *entry;

    assert(data_size <= sizeof(entry->data));
    assert(data_size == dev->block_size);

    entry = block_cache_pop_io_op(dev, block, BLOCK_CACHE_IO_OP_READ);
    assert(!entry->loaded);
    if (failed) {
        printf("%s: load block %lld failed\n",
               __func__, entry->block);
        return;
    }
    assert(!failed);
    entry->block_size = data_size;
    memcpy(entry->data, data, data_size); /* TODO: change decrypt function to take separate in/out buffers */

    stats_timer_start(STATS_FS_READ_BLOCK_CALC_MAC);
    ret = calculate_mac(entry->key, &entry->mac, entry->data, entry->block_size);
    stats_timer_stop(STATS_FS_READ_BLOCK_CALC_MAC);
    assert(!ret);
    entry->encrypted = true;

    /* TODO: check mac here instead of when getting data from the cache? */
    if (print_block_load) {
        printf("%s: load/decrypt block %lld complete\n",
               __func__, entry->block);
    }

    entry->loaded = true;
}

/**
 * block_cache_complete_write - Write complete callback from block device
 * @dev:        Block device
 * @block:      Block number
 * @failed:     true if write operation failed, and data is not on disc. If
 *              block device has tamper detection, e.g. rpmb, passing false here
 *              means that the secure side block device code has verified that
 *              the data was written to disk.
 */
void block_cache_complete_write(struct block_device *dev,
                                data_block_t block,
                                bool failed)
{
    struct block_cache_entry *entry;

    entry = block_cache_pop_io_op(dev, block, BLOCK_CACHE_IO_OP_WRITE);
    if (print_block_store) {
        printf("%s: write block %lld complete\n",
               __func__, entry->block);
    }
    assert(entry->dirty_tr);
    if (failed) {
        pr_err("write block %lld failed, fail transaction\n", entry->block);
        transaction_fail(entry->dirty_tr);
    }
    entry->dirty_tr = NULL;
}

/**
 * block_cache_entry_has_refs - Check if cache entry is referenced
 * @entry:      Cache entry
 *
 * Return: true if there are no references to @entry.
 */
static bool block_cache_entry_has_refs(struct block_cache_entry *entry)
{
    return !list_is_empty(&entry->obj.ref_list);
}

/**
 * block_cache_entry_has_one_ref - Check if cache entry is referenced once
 * @entry:      Cache entry
 *
 * Return: true if there is a single reference to @entry.
 */
static bool block_cache_entry_has_one_ref(struct block_cache_entry *entry)
{
    return list_length(&entry->obj.ref_list) == 1;
}

/**
 * block_cache_entry_decrypt - Decrypt cache entry
 * @entry:          Cache entry
 */
static void block_cache_entry_decrypt(struct block_cache_entry *entry)
{
    int ret;
    const struct iv *iv = NULL; /* TODO: support external iv */
    void *decrypt_data;
    size_t decrypt_size;

    assert(entry->loaded);
    assert(entry->encrypted);

    decrypt_data = entry->data;
    decrypt_size = entry->block_size;
    if (!iv) {
        iv = (void *)entry->data;
        assert(decrypt_size > sizeof(*iv));
        decrypt_data += sizeof(*iv);
        decrypt_size -= sizeof(*iv);
    }
    stats_timer_start(STATS_FS_READ_BLOCK_DECRYPT);
    ret = decrypt(entry->key, decrypt_data, decrypt_size, iv);
    stats_timer_stop(STATS_FS_READ_BLOCK_DECRYPT);
    assert(!ret);

    if (print_block_decrypt_encrypt) {
        printf("%s: decrypt block %lld complete\n",
               __func__, entry->block);
    }

    entry->encrypted = false;
}

/**
 * block_cache_entry_encrypt - Encrypt cache entry and update mac
 * @entry:          Cache entry
 */
static void block_cache_entry_encrypt(struct block_cache_entry *entry)
{
    int ret;
    void *encrypt_data;
    size_t encrypt_size;
    struct mac mac;
    struct iv *iv = NULL; /* TODO: support external iv */

    assert(entry->dirty);
    assert(!entry->encrypted);
    assert(!block_cache_entry_has_refs(entry));

    encrypt_data = entry->data;
    encrypt_size = entry->block_size;
    if (!iv) {
        iv = (void *)entry->data;
        assert(encrypt_size > sizeof(*iv));
        encrypt_data += sizeof(*iv);
        encrypt_size -= sizeof(*iv);
    }

    stats_timer_start(STATS_FS_WRITE_BLOCK_ENCRYPT);
    ret = encrypt(entry->key, encrypt_data, encrypt_size, iv);
    stats_timer_stop(STATS_FS_WRITE_BLOCK_ENCRYPT);
    assert(!ret);
    entry->encrypted = true;
    if (print_block_decrypt_encrypt) {
        printf("%s: encrypt block %lld complete\n",
               __func__, entry->block);
    }

    if (!entry->dirty_mac) {
        mac = entry->mac;
    }

    stats_timer_start(STATS_FS_WRITE_BLOCK_CALC_MAC);
    ret = calculate_mac(entry->key, &entry->mac,
                        entry->data, entry->block_size);
    stats_timer_stop(STATS_FS_WRITE_BLOCK_CALC_MAC);
    assert(!ret);

    if (!entry->dirty_mac) {
        assert(!CRYPTO_memcmp(&mac, &entry->mac, sizeof(mac)));
    }
    entry->dirty_mac = false;
    //assert(!entry->parent || entry->parent->ref_count);
    //assert(!entry->parent || entry->parent->dirty_ref);
}

/**
 * block_cache_entry_clean - Write dirty cache entry to disc
 * @entry:          Cache entry
 *
 * Does not wait for write to complete.
 */
static void block_cache_entry_clean(struct block_cache_entry *entry)
{
    if (!entry->dirty) {
        return;
    }

    if (print_block_store) {
        printf("%s: encrypt block %lld\n", __func__, entry->block);
    }

    assert(entry->block_size <= sizeof(entry->data));
    if (!entry->encrypted) {
        block_cache_entry_encrypt(entry);
    }
    assert(entry->encrypted);
    /* TODO: release ref to parent */

    block_cache_queue_write(entry, entry->data);
    entry->dirty = false;
}

/**
 * block_cache_entry_score - Get a keep score
 * @entry:      Block cache entry to check
 * @index:      Number of available entries before @entry in lru.
 *
 * Return: A score value indicating in what order entries that are close in the
 * lru should be replaced.
 */
static uint block_cache_entry_score(struct block_cache_entry *entry, uint index)
{
    if (!entry->dev) {
        return ~0;
    }
    return (entry->dirty ? (entry->dirty_tmp ? 1 : 2) : 4) * index;
}

/**
 * block_cache_lookup - Get cache entry for a specific block
 * @fs:         File system state object, or %NULL is @allocate is %false.
 * @dev:        Block device object.
 * @block:      Block number
 * @allocate:   If true, assign an unused entry to the specified @dev,@block
 *              if no matching entry is found.
 *
 * Return: cache entry matching @dev and @block. If no matching entry is found,
 * and @allocate is true, pick an unused entry and update it to match. If no
 * entry can be used, return NULL.
 */

static struct block_cache_entry *block_cache_lookup(struct fs *fs,
                                                    struct block_device *dev,
                                                    data_block_t block,
                                                    bool allocate)
{
    struct block_cache_entry *entry;
    struct block_cache_entry *unused_entry = NULL;
    uint unused_entry_score = 0;
    uint score;
    uint available = 0;
    uint in_use = 0;

    assert(dev);
    assert(fs || !allocate);

    stats_timer_start(STATS_CACHE_LOOKUP);
    list_for_every_entry(&block_cache_lru, entry, struct block_cache_entry, lru_node) {
        assert(entry->guard1 == BLOCK_CACHE_GUARD_1);
        assert(entry->guard2 == BLOCK_CACHE_GUARD_2);
        if (entry->dev == dev && entry->block == block) {
            if (print_cache_lookup) {
                printf("%s: block %lld, found cache entry %zd, loaded %d, dirty %d\n",
                       __func__, block, entry - block_cache_entries,
                       entry->loaded, entry->dirty);
            }
            stats_timer_start(STATS_CACHE_LOOKUP_FOUND);
            stats_timer_stop(STATS_CACHE_LOOKUP_FOUND);
            goto done;
        }
        if (!block_cache_entry_has_refs(entry)) {
            score = block_cache_entry_score(entry, available);
            available++;
            if (score >= unused_entry_score) {
                unused_entry = entry;
                unused_entry_score = score;
            }
            if (print_cache_lookup_verbose) {
                printf("%s: block %lld, cache entry %zd available last used for %lld\n",
                       __func__, block, entry - block_cache_entries, entry->block);
            }
        } else {
            if (print_cache_lookup_verbose) {
                printf("%s: block %lld, cache entry %zd in use for %lld\n",
                       __func__, block, entry - block_cache_entries, entry->block);
            }
            in_use++;
        }
    }
    entry = unused_entry;

    if (!entry || !allocate) {
        if (print_cache_lookup) {
            printf("%s: block %lld, no available entries, %u in use, allocate %d\n",
                   __func__, block, in_use, allocate);
        }
        entry = NULL;
        goto done;
    }

    if (print_cache_lookup) {
        printf("%s: block %lld, use cache entry %zd, dirty %d, %u available, %u in_use\n",
               __func__, block, entry - block_cache_entries,
               entry->dirty, available, in_use);
    }

    assert(!entry->dirty_ref);

    if (entry->dirty) {
        stats_timer_start(STATS_CACHE_LOOKUP_CLEAN);
        block_cache_entry_clean(entry);
        block_cache_complete_io(entry->dev);
        stats_timer_stop(STATS_CACHE_LOOKUP_CLEAN);
    }
    assert(!entry->dirty);
    assert(!entry->dirty_mac);
    assert(!entry->dirty_tr);

    entry->dev = dev;
    entry->block = block;
    assert(dev->block_size <= sizeof(entry->data));
    entry->block_size = dev->block_size;
    entry->key = fs->key;
    entry->loaded = false;
    entry->encrypted = false;

done:
    stats_timer_stop(STATS_CACHE_LOOKUP);

    return entry;
}

/**
 * block_cache_load_entry - Get cache entry for a specific block
 * @entry:      Block cache entry to load.
 * @mac:        Optional mac.
 * @mac_size:   Size of @mac.
 *
 * Return: false if entry could not be loaded or if non-NULL @mac does not match
 * the computed mac. true if entry was loaded and @mac is NULL.
 */

static bool block_cache_load_entry(struct block_cache_entry *entry,
                                   const void *mac, size_t mac_size)
{
    if (!entry->loaded) {
        assert(!block_cache_entry_has_refs(entry));
        if (print_block_load) {
            printf("%s: request load block %lld\n", __func__, entry->block);
        }
        block_cache_queue_read(entry);
        block_cache_complete_io(entry->dev);
    }
    if (!entry->loaded) {
        printf("%s: failed to load block %lld\n", __func__, entry->block);
        return false;
    }
    if (mac) {
        if (CRYPTO_memcmp(&entry->mac, mac, mac_size)) {
            printf("%s: block %lld, mac mismatch, %p\n",
                   __func__, entry->block, mac);
            return false;
        }
    }
    if (entry->encrypted) {
        block_cache_entry_decrypt(entry);
    }
    assert(!entry->encrypted);

    return true;
}

/**
 * block_cache_get - Get cache entry for a specific block and add a reference
 * @fs:         File system state object.
 * @dev:        Block device object.
 * @block:      Block number.
 * @load:       If true, load data if needed.
 * @mac:        Optional mac. Unused if @load is false.
 * @mac_size:   Size of @mac.
 * @ref:        Pointer to store reference in.
 *
 * Find cache entry, optionally load then add a reference to it.
 *
 * Return: cache entry matching dev in @tr and @block. Can return NULL if @load
 * is true and entry could not be loaded or does not match provided mac.
 */
static struct block_cache_entry *block_cache_get(struct fs *fs,
                                                 struct block_device *dev,
                                                 data_block_t block,
                                                 bool load,
                                                 const void *mac,
                                                 size_t mac_size,
                                                 obj_ref_t *ref)
{
    bool loaded;
    struct block_cache_entry *entry;

    assert(dev);

    if (block >= dev->block_count) {
        printf("%s: bad block num %lld >= %lld\n",
               __func__, block, dev->block_count);
        return NULL;
    }
    assert(block < dev->block_count);

    entry = block_cache_lookup(fs, dev, block, true);
    assert(entry);

    if (load) {
        loaded = block_cache_load_entry(entry, mac, mac_size);
        if (!loaded) {
            return NULL;
        }
    }

    assert(!entry->dirty_ref);
    obj_add_ref(&entry->obj, ref);
    if (print_block_ops) {
        printf("%s: block %lld, cache entry %zd, loaded %d, dirty %d\n",
               __func__, block, entry - block_cache_entries,
               entry->loaded, entry->dirty);
    }
    return entry;
}

/**
 * block_cache_get_data - Call block_cache_get and return data pointer
 * @fs:         File system state object.
 * @dev:        Block device object.
 * @block:      Block number.
 * @load:       If true, load data if needed.
 * @mac:        Optional mac. Unused if @load is false.
 * @mac_size:   Size of @mac.
 * @ref:        Pointer to store reference in.
 *
 * Return: block data pointer, or NULL if block_cache_get returned NULL.
 */
static void *block_cache_get_data(struct fs *fs,
                                  struct block_device *dev,
                                  data_block_t block,
                                  bool load,
                                  const void *mac,
                                  size_t mac_size,
                                  obj_ref_t *ref)
{
    struct block_cache_entry *entry;
    entry = block_cache_get(fs, dev, block, load, mac, mac_size, ref);
    if (!entry) {
        return NULL;
    }
    return entry->data;
}

/**
 * data_to_block_cache_entry - Get cache entry from data pointer
 * @data:       Pointer to data member of cache entry.
 *
 * Return: cache entry matching @data.
 */
static struct block_cache_entry *data_to_block_cache_entry(const void *data)
{
    struct block_cache_entry *entry;

    assert(data);
    entry = containerof(data, struct block_cache_entry, data);
    assert(entry >= block_cache_entries);
    assert(entry < &block_cache_entries[BLOCK_CACHE_SIZE]);
    assert(((uintptr_t)entry - (uintptr_t)entry) % sizeof(entry[0]) == 0);
    return entry;
}

/**
 * data_to_block_cache_entry_or_null - Get cache entry or NULL from data pointer
 * @data:       Pointer to data member of cache entry or NULL.
 *
 * Return: cache entry matching @data, or NULL is data is NULL.
 */
static struct block_cache_entry *data_to_block_cache_entry_or_null(const void *data)
{
    return data ? data_to_block_cache_entry(data) : NULL;
}

/**
 * block_cache_entry_destroy - Callback function for obj_del_ref
 * @obj:        Pointer to obj member of cache entry.
 *
 * Callback called by reference tracking code when the last reference to a
 * cache entry has been released. Since this is a cache, and not a normal heap
 * allocated object, the cache entry is not destroyed here. It is instead left
 * in a state where block_cache_lookup can reuse it.
 */
static void block_cache_entry_destroy(obj_t *obj)
{
    struct block_cache_entry *entry = containerof(obj, struct block_cache_entry, obj);

    list_delete(&entry->lru_node);
    list_add_head(&block_cache_lru, &entry->lru_node);

    if (entry->dirty_mac) {
        block_cache_entry_encrypt(entry);
    }
}

/**
 * block_cache_init - Allocate and initialize block cache
 */
void block_cache_init(void)
{
    int i;
    obj_ref_t ref;
    struct block_cache_entry *entry;

    assert(!block_cache_entries);

    entry = malloc(sizeof(block_cache_entries[0]) * BLOCK_CACHE_SIZE);
    assert(entry);
    full_assert(memset(entry, 1, sizeof(block_cache_entries[0]) * BLOCK_CACHE_SIZE));
    block_cache_entries = entry;

    for (i = 0; i < BLOCK_CACHE_SIZE; i++, entry++) {
        entry->guard1 = BLOCK_CACHE_GUARD_1;
        entry->guard2 = BLOCK_CACHE_GUARD_2;
        entry->dev = NULL;
        entry->block = ~0;
        entry->dirty = false;
        entry->dirty_ref = false;
        entry->dirty_mac = false;
        entry->dirty_tr = NULL;
        entry->io_op = BLOCK_CACHE_IO_OP_NONE;
        obj_init(&entry->obj, &ref);
        list_clear_node(&entry->io_op_node);
        list_add_head(&block_cache_lru, &entry->lru_node);
        obj_del_ref(&entry->obj, &ref, block_cache_entry_destroy);
    }
}

/**
 * block_cache_clean_transaction - Clean blocks modified by transaction
 * @tr:         Transaction
 */
void block_cache_clean_transaction(struct transaction *tr)
{
    struct block_cache_entry *entry;
    struct block_device *dev = NULL;

    stats_timer_start(STATS_CACHE_CLEAN_TRANSACTION);

    list_for_every_entry(&block_cache_lru, entry, struct block_cache_entry, lru_node) {
        assert(entry->guard1 == BLOCK_CACHE_GUARD_1);
        assert(entry->guard2 == BLOCK_CACHE_GUARD_2);
        if (entry->dirty_tr != tr) {
            continue;
        }

        assert(entry->dirty);

        assert(!entry->dirty_ref);

        if (entry->dirty_tmp) {
            continue;
        }

        if (!dev) {
            dev = entry->dev;
            assert(dev == tr->fs->dev || dev == tr->fs->super_dev);
        }

        assert(entry->dev == dev);

        if (print_clean_transaction) {
            printf("%s: tr %p, block %lld\n", __func__, tr, entry->block);
        }

        assert(!block_cache_entry_has_refs(entry));
        stats_timer_start(STATS_CACHE_CLEAN_TRANSACTION_ENT_CLN);
        block_cache_entry_clean(entry);
        stats_timer_stop(STATS_CACHE_CLEAN_TRANSACTION_ENT_CLN);
        assert(!entry->dirty);
        assert(!entry->dirty_tr);
    }

    if (dev) {
        stats_timer_start(STATS_CACHE_CLEAN_TRANSACTION_WAIT_IO);
        block_cache_complete_io(dev);
        stats_timer_stop(STATS_CACHE_CLEAN_TRANSACTION_WAIT_IO);
    }
    stats_timer_stop(STATS_CACHE_CLEAN_TRANSACTION);
}

/**
 * block_cache_discard_transaction - Discard blocks modified by transaction
 * @tr:             Transaction
 * @discard_all:    If true, discard all dirty blocks modified by @tr. If false,
 *                  discard tmp dirty blocks modified by @tr.
 *
 * If @discard_all is %false, only tmp blocks should be dirty. @discard_all
 * therefore only affects errors checks.
 */
void block_cache_discard_transaction(struct transaction *tr, bool discard_all)
{
    struct block_cache_entry *entry;
    struct block_device *dev = NULL;

    list_for_every_entry(&block_cache_lru, entry, struct block_cache_entry, lru_node) {
        assert(entry->guard1 == BLOCK_CACHE_GUARD_1);
        assert(entry->guard2 == BLOCK_CACHE_GUARD_2);
        if (entry->dirty_tr != tr) {
            continue;
        }

        if (!dev) {
            dev = entry->dev;
            assert(dev == tr->fs->dev || dev == tr->fs->super_dev);
        }

        assert(entry->dev == dev);
        assert(entry->dirty);

        if (print_clean_transaction) {
            printf("%s: tr %p, block %lld, tmp %d\n",
                   __func__, tr, entry->block, entry->dirty_tmp);
        }

        if (block_cache_entry_has_refs(entry)) {
            pr_warn("tr %p, block %lld has ref (dirty_ref %d)\n",
                    tr, entry->block, entry->dirty_ref);
        } else {
            assert(!entry->dirty_ref);
        }
        if (!discard_all) {
            assert(!block_cache_entry_has_refs(entry));
            assert(entry->dirty_tmp);
        }
        entry->dirty = false;
        entry->dirty_tr = NULL;
        entry->loaded = false;
        assert(!entry->dirty);
        assert(!entry->dirty_tr);
    }
}

/**
 * block_get_no_read - Get block data without read
 * @tr:         Transaction to get device from
 * @block:      Block number
 * @ref:        Pointer to store reference in.
 *
 * Return: Const block data pointer.
 *
 * This is only useful if followed by block_dirty.
 */
const void *block_get_no_read(struct transaction *tr, data_block_t block, obj_ref_t *ref)
{
    assert(tr);
    assert(tr->fs);

    return block_cache_get_data(tr->fs, tr->fs->dev,
                                block, false, NULL, 0, ref);
}

/**
 * block_get_super - Get super block data without checking mac
 * @fs:         File system state object.
 * @block:      Block number.
 * @ref:        Pointer to store reference in.
 *
 * Return: Const block data pointer.
 *
 * Should only be used if block device performs tamper detection.
 */
const void *block_get_super(struct fs *fs,
                            data_block_t block,
                            obj_ref_t *ref)
{
    assert(fs);
    assert(fs->super_dev);
    assert(fs->super_dev->tamper_detecting);

    return block_cache_get_data(fs, fs->super_dev, block, true, NULL, 0, ref);
}

/**
 * block_get_no_tr_fail - Get block data
 * @tr:         Transaction to get device from
 * @block_mac:  Block number and mac
 * @iv:         Initial vector used to decrypt block, or NULL. If NULL, the
 *              start of the loaded block data is used as the iv.
 *              Only NULL is currently supported.
 * @ref:        Pointer to store reference in.
 *
 * Return: Const block data pointer, or NULL if mac of loaded data does not mac
 * in @block_mac or a read error was reported by the block device when loading
 * the data.
 */
const void *block_get_no_tr_fail(struct transaction *tr,
                                 const struct block_mac *block_mac,
                                 const struct iv *iv,
                                 obj_ref_t *ref)
{
    data_block_t block;

    assert(tr);
    assert(tr->fs);
    assert(block_mac);

    block = block_mac_to_block(tr, block_mac);
    assert(block);

    return block_cache_get_data(tr->fs, tr->fs->dev, block,
                                true, block_mac_to_mac(tr, block_mac),
                                tr->fs->mac_size, ref);
}

/**
 * block_get - Get block data
 * @tr:         Transaction to get device from
 * @block_mac:  Block number and mac
 * @iv:         Initial vector used to decrypt block, or NULL. If NULL, the
 *              start of the loaded block data is used as the iv.
 *              Only NULL is currently supported.
 * @ref:        Pointer to store reference in.
 *
 * Return: Const block data pointer, or NULL if the transaction has failed. A
 * transaction failure is triggered if mac of loaded data does not mac in
 * @block_mac or a read error was reported by the block device when loading the
 * data.
 */
const void *block_get(struct transaction *tr,
                      const struct block_mac *block_mac,
                      const struct iv *iv,
                      obj_ref_t *ref)
{
    const void *data;

    assert(tr);

    if (tr->failed) {
        pr_warn("transaction already failed\n");
        return NULL;
    }

    data = block_get_no_tr_fail(tr, block_mac, iv, ref);
    if (!data && !tr->failed) {
        pr_warn("transaction failed\n");
        transaction_fail(tr);
    }
    return data;
}


/**
 * block_dirty - Mark cache entry dirty and return non-const block data pointer.
 * @tr:         Transaction
 * @data:       Const block data pointer
 * @is_tmp:     If true, data is only needed until @tr is commited.
 *
 * Return: Non-const block data pointer.
 */
void *block_dirty(struct transaction *tr, const void *data, bool is_tmp)
{
    struct block_cache_entry *entry = data_to_block_cache_entry(data);

    assert(tr);
    assert(list_in_list(&tr->node)); /* transaction must be active */
    assert(!entry->dirty_tr || entry->dirty_tr == tr);
    assert(!entry->dirty_ref);

    if (!entry->loaded || entry->encrypted) {
        if (print_block_ops) {
            printf("%s: skip decrypt block %lld\n", __func__, entry->block);
        }
        entry->loaded = true;
        entry->encrypted = false;
    }
    assert(block_cache_entry_has_one_ref(entry));
    assert(!entry->encrypted);
    entry->dirty = true;
    entry->dirty_ref = true;
    entry->dirty_tmp = is_tmp;
    entry->dirty_tr = tr;
    return (void *)data;
}

/**
 * block_is_clean - Check if block is clean
 * @dev:        Block device
 * @block:      Block number
 *
 * Return: %true if there is no matching dirty cache entry, %false if the cache
 * contains a dirty block matching @dev and @block.
 */
bool block_is_clean(struct block_device *dev, data_block_t block)
{
    struct block_cache_entry *entry;

    entry = block_cache_lookup(NULL, dev, block, false);
    return !entry || !entry->dirty;
}

/**
 * block_discard_dirty - Discard dirty cache data.
 * @data:       Block data pointer
 */
void block_discard_dirty(const void *data)
{
    struct block_cache_entry *entry = data_to_block_cache_entry(data);

    if (entry->dirty) {
        assert(entry->dev);
        entry->loaded = false;
        entry->dev = NULL;
        entry->block = ~0;
        entry->dirty = false;
        entry->dirty_tr = NULL;
    }
}

/**
 * block_discard_dirty_by_block - Discard cache entry if dirty.
 * @dev:        Block device
 * @block:      Block number
 */
void block_discard_dirty_by_block(struct block_device *dev, data_block_t block)
{
    struct block_cache_entry *entry;

    entry = block_cache_lookup(NULL, dev, block, false);
    if (!entry) {
        return;
    }
    assert(!entry->dirty_ref);
    assert(!block_cache_entry_has_refs(entry));
    if (!entry->dirty) {
        return;
    }
    block_discard_dirty(entry->data);
}

/**
 * block_put_dirty - Release reference to dirty block.
 * @tr:             Transaction
 * @data:           Block data pointer
 * @data_ref:       Reference pointer to release
 * @block_mac:      block_mac pointer to update after encryting block
 * @block_mac_ref:  Block data pointer that @block_mac belongs to, or NULL if
 *                  @block_mac points to a memory only location.
 *
 * Helper function to for block_put_dirty, block_put_dirty_no_mac and
 * block_put_dirty_discard.
 */
static void block_put_dirty_etc(struct transaction *tr,
                                void *data, obj_ref_t *data_ref,
                                struct block_mac *block_mac,
                                void *block_mac_ref)
{
    int ret;
    struct block_cache_entry *entry = data_to_block_cache_entry(data);
    struct block_cache_entry *parent = data_to_block_cache_entry_or_null(block_mac_ref);
    struct iv *iv = (void *)entry->data; /* TODO: support external iv */

    if (tr) {
        assert(block_mac);
        assert(entry->loaded);
        assert(!entry->encrypted);
        assert(entry->dirty);
        assert(entry->dirty_ref);
    } else {
        assert(!block_mac);
    }
    assert(entry->guard1 == BLOCK_CACHE_GUARD_1);
    assert(entry->guard2 == BLOCK_CACHE_GUARD_2);

    entry->dirty_ref = false;
    if (entry->dirty) {
        entry->dirty_mac = true;
        ret = generate_iv(iv);
        assert(!ret);
    } else {
        pr_warn("block %lld, not dirty\n", entry->block);
        assert(entry->dirty_tr == NULL);
        assert(!tr);
    }

    block_put(data, data_ref);
    assert(entry->encrypted || !tr); /* TODO: fix clients to support lazy write */
    assert(!entry->dirty_mac);
    if (block_mac) {
        assert(block_mac_to_block(tr, block_mac) == entry->block);
        block_mac_set_mac(tr, block_mac, &entry->mac);
    }
    if (print_mac_update) {
        printf("%s: block %lld, update parent mac, %p, block %lld\n",
               __func__, entry->block, block_mac,
               parent ? parent->block : 0);
    }
}

/**
 * block_put_dirty - Release reference to dirty block.
 * @tr:             Transaction
 * @data:           Block data pointer
 * @data_ref:       Reference pointer to release
 * @block_mac:      block_mac pointer to update after encryting block
 * @block_mac_ref:  Block data pointer that @block_mac belongs to, or NULL if
 *                  @block_mac points to a memory only location.
 */
void block_put_dirty(struct transaction *tr, void *data, obj_ref_t *data_ref,
                     struct block_mac *block_mac, void *block_mac_ref)
{
    assert(tr);
    assert(block_mac);
    block_put_dirty_etc(tr, data, data_ref, block_mac, block_mac_ref);
}

/**
 * block_put_dirty_no_mac - Release reference to dirty super block.
 * @data:           Block data pointer
 * @data_ref:       Reference pointer to release
 *
 * Similar to block_put_dirty except no transaction or block_mac is needed.
 */
void block_put_dirty_no_mac(void *data, obj_ref_t *data_ref)
{
    struct block_cache_entry *entry = data_to_block_cache_entry(data);

    assert(entry->dev);
    assert(entry->dev->tamper_detecting);
    block_put_dirty_etc(NULL, data, data_ref, NULL, NULL);
}

/**
 * block_put_dirty_discard - Release reference to dirty block.
 * @data:           Block data pointer
 * @data_ref:       Reference pointer to release
 *
 * Similar to block_put_dirty except data can be discarded.
 */
void block_put_dirty_discard(void *data, obj_ref_t *data_ref)
{
    block_discard_dirty(data);
    block_put_dirty_etc(NULL, data, data_ref, NULL, NULL);
}

/**
 * block_get_write_no_read - Get block data without read for write
 * @tr:         Transaction
 * @block:      Block number
 * @is_tmp:     If true, data is only needed until @tr is commited.
 * @ref:        Pointer to store reference in.
 *
 * Same as block_get_no_read followed by block_dirty.
 *
 * Return: Block data pointer.
 */
void *block_get_write_no_read(struct transaction *tr,
                              data_block_t block,
                              bool is_tmp,
                              obj_ref_t *ref)
{
    const void *data_ro = block_get_no_read(tr, block, ref);
    return block_dirty(tr, data_ro, is_tmp);
}

/**
 * block_get_write - Get block data for write
 * @tr:         Transaction
 * @block_mac:  Block number and mac
 * @iv:         Initial vector used to decrypt block, or NULL. If NULL, the
 *              start of the loaded block data is used as the iv.
 *              Only NULL is currently supported.
 * @is_tmp:     If true, data is only needed until @tr is commited.
 * @ref:        Pointer to store reference in.
 *
 * Same as block_get followed by block_dirty.
 *
 * Return: Block data pointer.
 */
void *block_get_write(struct transaction *tr,
                      const struct block_mac *block_mac,
                      const struct iv *iv,
                      bool is_tmp,
                      obj_ref_t *ref)
{
    const void *data_ro = block_get(tr, block_mac, iv, ref);
    if (!data_ro) {
        return NULL;
    }
    return block_dirty(tr, data_ro, is_tmp);
}

/**
 * block_get_cleared - Get block cleared data for write
 * @tr:         Transaction
 * @block:      Block number
 * @is_tmp:     If true, data is only needed until @tr is commited.
 * @ref:        Pointer to store reference in.
 *
 * Return: Block data pointer.
 */
void *block_get_cleared(struct transaction *tr,
                        data_block_t block,
                        bool is_tmp,
                        obj_ref_t *ref)
{
    void *data = block_get_write_no_read(tr, block, is_tmp, ref);
    memset(data, 0, MAX_BLOCK_SIZE);
    return data;
}

/**
 * block_get_cleared_super - Get block with cleared data for write on super_dev
 * @tr:         Transaction
 * @block:      Block number
 * @ref:        Pointer to store reference in.
 *
 * Return: Block data pointer.
 */
void *block_get_cleared_super(struct transaction *tr,
                              data_block_t block,
                              obj_ref_t *ref)
{
    void *data_rw;
    const void *data_ro =  block_cache_get_data(tr->fs, tr->fs->super_dev,
                                                block, false, NULL, 0, ref);
    data_rw = block_dirty(tr, data_ro, false);
    assert(tr->fs->super_dev->block_size <= MAX_BLOCK_SIZE);
    memset(data_rw, 0, tr->fs->super_dev->block_size);
    return data_rw;
}

/**
 * block_get_copy - Get block for write with data copied from another block.
 * @tr:         Transaction
 * @data:       Block data pointer
 * @block:      New block number
 * @is_tmp:     If true, data is only needed until @tr is commited.
 * @new_ref:    Pointer to store reference to new block in.
 *
 * Return: Block data pointer.
 */
void *block_get_copy(struct transaction *tr,
                     const void *data,
                     data_block_t block,
                     bool is_tmp,
                     obj_ref_t *new_ref)
{
    void *dst_data;
    struct block_cache_entry *src_entry = data_to_block_cache_entry(data);

    assert(block);
    assert(block < tr->fs->dev->block_count);

    dst_data = block_get_write_no_read(tr, block, is_tmp, new_ref);
    memcpy(dst_data, data, src_entry->block_size);
    return dst_data;
}

/**
 * block_move - Get block for write and move to new location
 * @tr:         Transaction
 * @data:       Block data pointer
 * @block:      New block number
 * @is_tmp:     If true, data is only needed until @tr is commited.
 *
 * Change block number of cache entry mark new block dirty. Useful for
 * copy-on-write.
 *
 * Return: Non-const block data pointer.
 */
void *block_move(struct transaction *tr,
                 const void *data,
                 data_block_t block,
                 bool is_tmp)
{
    struct block_cache_entry *dest_entry;
    struct block_cache_entry *entry = data_to_block_cache_entry(data);

    assert(block_cache_entry_has_one_ref(entry));
    assert(!entry->dirty);
    assert(entry->dev == tr->fs->dev);

    if (print_block_move) {
        printf("%s: move cache entry %zd, from block %lld to %lld\n",
               __func__, entry - block_cache_entries, entry->block, block);
    }

    dest_entry = block_cache_lookup(NULL, tr->fs->dev, block, false);
    if (dest_entry) {
        assert(!block_cache_entry_has_refs(dest_entry));
        assert(!dest_entry->dirty_ref);
        assert(!dest_entry->dirty_tr || dest_entry->dirty_tr == tr);
        assert(!list_in_list(&dest_entry->io_op_node));
        assert(dest_entry->block == block);
        if (print_block_move) {
            printf("%s: clear old cache entry for block %lld, %zd\n",
                   __func__, block, dest_entry - block_cache_entries);
        }
        dest_entry->loaded = false;
        dest_entry->dev = NULL;
        dest_entry->block = ~0;
        dest_entry->dirty = false;
        dest_entry->dirty_tr = NULL;
    }

    entry->block = block;
    return block_dirty(tr, data, is_tmp);
}

/**
 * block_put - Release reference to block.
 * @data:           Block data pointer
 * @data_ref:       Reference pointer to release
 */
void block_put(const void *data, obj_ref_t *ref)
{
    struct block_cache_entry *entry = data_to_block_cache_entry(data);

    if (print_block_ops) {
        printf("%s: block %lld, cache entry %zd, loaded %d, dirty %d\n",
               __func__, entry->block, entry - block_cache_entries,
               entry->loaded, entry->dirty);
    }

    assert(!entry->dirty_ref);

    obj_del_ref(&entry->obj, ref, block_cache_entry_destroy);
}

/**
 * data_to_block_num - Get block number from block data pointer
 * @data:       Block data pointer
 *
 * Only used for debug code.
 *
 * Return: block number.
 */
data_block_t data_to_block_num(const void *data)
{
    struct block_cache_entry *entry = data_to_block_cache_entry(data);

    return entry->block;
}

/**
 * block_cache_debug_get_ref_block_count - Get number of blocks that have references
 *
 * Only used for debug code.
 *
 * Return: number of blocks in cache that have references.
 */
uint block_cache_debug_get_ref_block_count(void)
{
    uint count = 0;
    struct block_cache_entry *entry;

    list_for_every_entry(&block_cache_lru, entry, struct block_cache_entry, lru_node) {
        assert(entry->guard1 == BLOCK_CACHE_GUARD_1);
        assert(entry->guard2 == BLOCK_CACHE_GUARD_2);
        if (block_cache_entry_has_refs(entry)) {
            if (print_cache_get_ref_block_count) {
                printf("%s: cache entry %zd in use for %lld, dev %p\n",
                       __func__, entry - block_cache_entries,
                       entry->block, entry->dev);
            }
            count++;
        }
    }
    return count;
}
