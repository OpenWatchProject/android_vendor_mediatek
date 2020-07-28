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
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <openssl/rand.h>

#include "../block_allocator.h"
#include "../block_cache.h"
#include "../block_map.h"
#include "../block_set.h"
#include "../debug_stats.h"
#include "../crypt.h"
#include "../file.h"
#include "../transaction.h"

#include <time.h>

long gettime(uint32_t clock_id, uint32_t flags, int64_t *time)
{
    int ret;
    struct timespec ts;
    assert(!clock_id);
    assert(!flags);

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(!ret);
    *time = ts.tv_sec * 1000000000LL + ts.tv_nsec;

    return 0;
}

#if 0 /* test tree order 3 */ /* not useful, b+tree for free set grows faster than the space that is added to it */
#define BLOCK_SIZE (64)
#define BLOCK_COUNT (256)
#elif 0 /* test tree order 4 */
#define BLOCK_SIZE (80)
#define BLOCK_COUNT (256)
#elif 0 /* test tree order 5 */
#define BLOCK_SIZE (96)
#define BLOCK_COUNT (256)
#elif 0 /* test tree order 6 */
#define BLOCK_SIZE (112)
#define BLOCK_COUNT (256)
#elif 0 /* test tree order 7 */
#define BLOCK_SIZE (128)
#define BLOCK_COUNT (256)
#elif 0 /* test tree order 8 */
#define BLOCK_SIZE (144)
#define BLOCK_COUNT (256)
#elif 0 /* test single rpmb block with 64-bit indexes */
#define BLOCK_SIZE (256)
#define BLOCK_COUNT (256)
#elif 1
#define BLOCK_SIZE (2048)
#define BLOCK_COUNT (256)
#elif 0 /* test single rpmb block with simulated 16-bit indexes, 128kb device */
#define BLOCK_SIZE (256 * 4)
#define BLOCK_COUNT (512)
#elif 0 /* test single rpmb block with simulated 16-bit indexes, 4MB device */
#define BLOCK_SIZE (256 * 4)
#define BLOCK_COUNT (16384)
#else
#define BLOCK_SIZE (256 * 4)
#define BLOCK_COUNT (0x10000)
#endif

struct block {
    char data[BLOCK_SIZE];
    char data_copy[BLOCK_SIZE];
    struct mac mac;
    bool loaded;
    bool dirty;
    bool dirty_ref;
    struct block *parent;
    struct block_mac *block_mac_in_parent;
    const char *used_by_str;
    data_block_t used_by_block;
};
static struct block blocks[BLOCK_COUNT];
static const struct key key;

static bool print_test_verbose = false;
static bool print_block_tree_test_verbose = false;

static void block_test_start_read(struct block_device *dev, data_block_t block)
{
    assert(dev->block_size <= BLOCK_SIZE);
    assert(block < countof(blocks));
    block_cache_complete_read(dev, block, blocks[block].data, dev->block_size, false);
}

static void block_test_start_write(struct block_device *dev, data_block_t block,
                                   const void *data, size_t data_size)
{
    assert(block < countof(blocks));
    assert(data_size <= sizeof(blocks[block].data));
    memcpy(blocks[block].data, data, data_size);
    block_cache_complete_write(dev, block, false);
}

#if FULL_ASSERT
static void block_clear_used_by(void)
{
    size_t block;
    for (block = 0; block < countof(blocks); block++) {
        blocks[block].used_by_str = NULL;
        blocks[block].used_by_block = 0;
    }
}

static void block_set_used_by(data_block_t block,
                              const char *used_by_str,
                              data_block_t used_by_block)
{
    assert(block < countof(blocks));

    if (!blocks[block].used_by_str) {
        blocks[block].used_by_str = used_by_str;
        blocks[block].used_by_block = used_by_block;
    }
    assert(blocks[block].used_by_str == used_by_str);
    assert(blocks[block].used_by_block == used_by_block);
}

static void mark_block_tree_in_use(struct transaction *tr,
                                   struct block_tree *block_tree,
                                   bool mark_data_used,
                                   const char *used_by_str,
                                   data_block_t used_by_block)
{
    struct block_tree_path path;
    int i;

    block_tree_walk(tr, block_tree, 0, true, &path);
    if (path.count) {
        /* mark root in use in case it is empty */
        block_set_used_by(block_mac_to_block(tr, &path.entry[0].block_mac),
                          used_by_str, used_by_block);
    }
    while (block_tree_path_get_key(&path)) {
        for (i = 0; i < path.count; i++) {
            block_set_used_by(block_mac_to_block(tr, &path.entry[i].block_mac),
                              used_by_str, used_by_block);
        }
        if (mark_data_used) {
            block_set_used_by(block_tree_path_get_data(&path),
                              used_by_str, used_by_block);
        }
        block_tree_path_next(&path);
    }
}

static void mark_files_in_use(struct transaction *tr)
{
    void file_block_map_init(struct transaction *tr,
                             struct block_map *block_map,
                             const struct block_mac *file);

    struct block_tree_path path;
    struct block_map block_map;

    block_tree_walk(tr, &tr->fs->files, 0, true, &path);
    while (block_tree_path_get_key(&path)) {
        struct block_mac block_mac = block_tree_path_get_data_block_mac(&path);
        file_block_map_init(tr, &block_map, &block_mac);
        mark_block_tree_in_use(tr, &block_map.tree, true, "file", block_mac_to_block(tr, &block_mac));
        block_tree_path_next(&path);
    }
}

static void check_fs_prepare(struct transaction *tr)
{
    data_block_t block;

    block_clear_used_by();

    for (block = tr->fs->dev->block_count; block < countof(blocks); block++) {
        block_set_used_by(block, "out-of-range", 0);
    }

    block_set_used_by(tr->fs->super_block[0], "superblock", 0);
    block_set_used_by(tr->fs->super_block[1], "superblock", 1);

    block = 1;
    while (true) {
        block = block_set_find_next_block(tr, &tr->fs->free, block, true);
        if (!block) {
            break;
        }
        block_set_used_by(block, "free", 0);
        block++;
    }

    mark_block_tree_in_use(tr, &tr->fs->free.block_tree, false, "free_tree_node", 0);

    mark_block_tree_in_use(tr, &tr->fs->files, true, "files", 0);
    mark_files_in_use(tr);

}

static bool check_fs_finish(struct transaction *tr)
{
    bool valid = true;
    data_block_t block;

    for (block = 0; block < countof(blocks); block++) {
        if (!blocks[block].used_by_str) {
            printf("block %lld, lost\n", block);
            valid = false;
        }
    }

    if (!valid) {
        printf("free:\n");
        block_set_print(tr, &tr->fs->free);
        files_print(tr);
    }

    return valid;
}

static bool check_fs_allocated(struct transaction *tr, data_block_t *allocated, size_t allocated_count)
{
    size_t i;

    check_fs_prepare(tr);

    for (i = 0; i < allocated_count; i++) {
        block_set_used_by(allocated[i], "allocated", 0);
    }

    return check_fs_finish(tr);
}

static bool check_fs(struct transaction *tr)
{
    return check_fs_allocated(tr, NULL, 0);
}
#endif

static void empty_test(struct transaction *tr)
{
    struct block_range range;

    range.start = 4;
    range.end = tr->fs->dev->block_count;
    assert(block_set_range_in_set(tr, &tr->fs->free, range));

    if (print_test_verbose) {
        printf("%s: initial free state:\n", __func__);
        block_set_print(tr, &tr->fs->free);
    }
}

typedef uint16_t (*keyfunc_t)(uint index, uint rindex, uint maxindex);
static uint16_t inc_inc_key(uint index, uint rindex, uint maxindex)
{
    return rindex ?: index;
}

static uint16_t inc_dec_key(uint index, uint rindex, uint maxindex)
{
    return index;
}

static uint16_t dec_inc_key(uint index, uint rindex, uint maxindex)
{
    return maxindex + 1 - index;
}

static uint16_t dec_dec_key(uint index, uint rindex, uint maxindex)
{
    return maxindex + 1 - (rindex ?: index);
}

static uint16_t same_key(uint index, uint rindex, uint maxindex)
{
    return 1;
}

static uint16_t rand_key(uint index, uint rindex, uint maxindex)
{
    uint16_t key;

    RAND_bytes((uint8_t *)&key, sizeof(key));

    return key ?: 1; /* 0 key is not currently supported */
}

keyfunc_t keyfuncs[] = {
    inc_inc_key,
    inc_dec_key,
    dec_inc_key,
    dec_dec_key,
    same_key,
    rand_key,
};

static void block_tree_test_etc(struct transaction *tr,
                                uint order,
                                uint count,
                                uint commit_interval,
                                keyfunc_t keyfunc)
{
    uint i;
    uint ri;
    uint16_t key;
    uint16_t tmpkey;
    uint commit_count = 0;
    struct block_tree tree = BLOCK_TREE_INITIAL_VALUE(tree);
    struct block_tree_path path;
    const size_t key_size = sizeof(key);
    const size_t header_size = sizeof(struct iv) + 8;
    const size_t child_size = sizeof(struct block_mac);
    size_t block_size = header_size + key_size * (order - 1) + child_size * order;

    if (block_size > tr->fs->dev->block_size) {
        printf("block tree order %d does not fit in block. block size %zd > %zd, skip test\n",
               order, block_size, tr->fs->dev->block_size);
        return;
    }

    block_tree_init(&tree, block_size, key_size, child_size, child_size);
    if (commit_interval) {
        tree.copy_on_write = true;
        tree.allow_copy_on_write = true;
    }

    assert(tree.key_count[0] == order - 1);
    assert(tree.key_count[1] == order - 1);

    for (i = 1; i <= count; key++, i++) {
        key = keyfunc(i, 0, count);
        if (print_block_tree_test_verbose) {
            printf("block tree order %d, insert %d:\n", order, key);
        }
        block_tree_insert(tr, &tree, key, i);
        if (tr->failed) {
            return;
        }
        if (commit_interval && ++commit_count == commit_interval) {
            commit_count = 0;
            transaction_complete(tr);
            assert(!tr->failed);
            transaction_activate(tr);
        }
        if (print_block_tree_test_verbose) {
            printf("block tree order %d after %d inserts, last %d:\n", order, i, key);
            block_tree_print(tr, &tree);
        }
    }
    for (ri = 1, i--; i >= 1; i--, ri++) {
        tmpkey = keyfunc(i, ri, count);
        assert(tmpkey);
        block_tree_walk(tr, &tree, tmpkey, false, &path);
        key = block_tree_path_get_key(&path);
        if (!key) {
            key = path.entry[path.count - 1].prev_key;
            assert(key);
        }
        if (key != tmpkey) {
            block_tree_walk(tr, &tree, key, false, &path);
        }
        assert(key = block_tree_path_get_key(&path));
        assert(block_tree_path_get_data(&path));
        if (print_block_tree_test_verbose) {
            printf("block tree order %d, remove %d (%d):\n", order, key, tmpkey);
        }
        block_tree_remove(tr, &tree, key, block_tree_path_get_data(&path));
        if (tr->failed) {
            return;
        }
        if (commit_interval && ++commit_count == commit_interval) {
            commit_count = 0;
            transaction_complete(tr);
            assert(!tr->failed);
            transaction_activate(tr);
        }
        if (print_block_tree_test_verbose) {
            printf("block tree order %d removed %d:\n", order, key);
            block_tree_print(tr, &tree);
        }
    }

    if (commit_interval) {
        block_discard_dirty_by_block(tr->fs->dev, block_mac_to_block(tr, &tree.root));
        block_free(tr, block_mac_to_block(tr, &tree.root));
    }
}

static void block_tree_keyfuncs_test(struct transaction *tr,
                                     uint order, uint count)
{
    uint commit_interval;
    uint i;

    for (commit_interval = 0; commit_interval < 2; commit_interval++) {
        for (i = 0; i < countof(keyfuncs); i++) {
            block_tree_test_etc(tr, order, count, commit_interval, keyfuncs[i]);
        }
    }
}

static void block_tree_test(struct transaction *tr)
{
    uint order;

    block_tree_keyfuncs_test(tr, 6, 5); /* test leaf node only */
    block_tree_keyfuncs_test(tr, 6, 10);

    for (order = 3; order <= 5; order++) {
        block_tree_keyfuncs_test(tr, order, order - 1);
        block_tree_keyfuncs_test(tr, order, order);
        block_tree_keyfuncs_test(tr, order, order * 2);
        block_tree_keyfuncs_test(tr, order, order * order);
        block_tree_keyfuncs_test(tr, order, order * order * order);
    }
}

static void block_set_test(struct transaction *tr)
{
    struct block_set sets[3];
    int si, i;

    for (si = 0; si < countof(sets); si++) {
        block_set_init(tr->fs, &sets[si]);
    }

    for (i = 0; i < 10; i++) {
       for (si = 0; si < countof(sets); si++) {
           block_set_add_block(tr, &sets[si], 2 + i * 3 + si);
       }
    }
    for (si = 0; si < countof(sets); si++) {
       assert(!block_set_overlap(tr, &sets[si], &sets[(si + 1) % countof(sets)]));
    }
    for (si = 1; si < countof(sets); si++) {
       block_set_add_block(tr, &sets[0], 2 + 5 * 3 + si);
    }
    for (si = 1; si < countof(sets); si++) {
       assert(block_set_overlap(tr, &sets[si], &sets[0]));
       assert(block_set_overlap(tr, &sets[0], &sets[si]));
       assert(si < 2 || !block_set_overlap(tr, &sets[si], &sets[si - 1]));
    }
}

static void block_tree_allocate_all_test(struct transaction *tr)
{
    uint i;

    for (i = 0; i < countof(keyfuncs); i++) {
        assert(!tr->failed);
        block_tree_test_etc(tr, 3, ~0, 0, keyfuncs[i]);
        assert(tr->failed);
        transaction_complete(tr);
        transaction_activate(tr);
    }
}
static void block_map_test(struct transaction *tr)
{
    uint i;
    struct block_mac block_mac = BLOCK_MAC_INITIAL_VALUE(block_mac);
    struct block_map block_map = BLOCK_MAP_INITIAL_VALUE(block_map);

    block_map_init(tr, &block_map, &block_mac, 128);

    for (i = 1; i <= 100; i++) {
        block_mac_set_block(tr, &block_mac, block_allocate(tr));
        block_map_set(tr, &block_map, i, &block_mac);
    }
    for (; i >= 2; i /= 2) {
        block_map_truncate(tr, &block_map, i);
        assert(!block_map_get(tr, &block_map, i, &block_mac));
        assert(block_map_get(tr, &block_map, i - 1, &block_mac));
    }
    block_map_free(tr, &block_map);
}

static void free_frag_etc_test(struct transaction *tr, int start, int end, int inc)
{
    int i;

    for (i = start; i < end; i += inc) {
        block_free(tr, i);
        assert(!tr->failed);
    }
}

static void allocate_2_transactions_test_etc(struct transaction *tr,
                                             data_block_t blocks1[],
                                             size_t blocks1_count,
                                             data_block_t blocks2[],
                                             size_t blocks2_count)
{
    int i;
    struct transaction tr1;
    struct transaction tr2;
    size_t blocks_max_count = MAX(blocks1_count, blocks2_count);

    transaction_init(&tr1, tr->fs, true);
    transaction_init(&tr2, tr->fs, true);


    for (i = 0; i < blocks_max_count; i++) {
        if (i < blocks1_count) {
            blocks1[i] = block_allocate(&tr1);
        }
        if (i < blocks2_count) {
            blocks2[i] = block_allocate(&tr2);
        }
    }
    assert(!tr1.failed);
    assert(!tr2.failed);

    transaction_complete(&tr1);

    assert(!tr1.failed);
    assert(!tr2.failed);

    for (i = 0; i < blocks1_count; i++) {
        assert(!block_set_block_in_set(tr, &tr->fs->free, blocks1[i]));
    }
    for (i = 0; i < blocks2_count; i++) {
        assert(block_set_block_in_set(tr, &tr->fs->free, blocks2[i]));
    }

    transaction_complete(&tr2);

    for (i = 0; i < blocks1_count; i++) {
        assert(!block_set_block_in_set(tr, &tr->fs->free, blocks1[i]));
    }
    for (i = 0; i < blocks2_count; i++) {
        assert(!block_set_block_in_set(tr, &tr->fs->free, blocks2[i]));
    }

    assert(!block_cache_debug_get_ref_block_count());
#if FULL_ASSERT
    check_fs_prepare(tr);
    for (i = 0; i < blocks1_count; i++) {
        block_set_used_by(blocks1[i], "allocated", 0);
    }
    for (i = 0; i < blocks2_count; i++) {
        block_set_used_by(blocks2[i], "allocated2", 0);
    }
    assert(check_fs_finish(tr));
#endif

    transaction_free(&tr1);
    transaction_free(&tr2);
}

static void free_test_etc(struct transaction *tr,
                          data_block_t blocks1[],
                          size_t blocks1_count,
                          data_block_t blocks2[],
                          size_t blocks2_count)
{
    int i;
    struct transaction tr1;
    struct transaction tr2;
    size_t blocks_max_count = MAX(blocks1_count, blocks2_count);

    transaction_init(&tr1, tr->fs, true);
    transaction_init(&tr2, tr->fs, true);

    for (i = 0; i < blocks_max_count; i++) {
        if (i < blocks1_count) {
            block_free(&tr1, blocks1[i]);
            if (print_test_verbose) {
                printf("tr1.freed after free %lld:\n", blocks1[i]);
                block_set_print(&tr1, &tr1.freed);
            }
        }
        if (i < blocks2_count) {
            block_free(&tr2, blocks1[i]);
            if (print_test_verbose) {
                printf("tr2.freed after free %lld:\n", blocks2[i]);
                block_set_print(&tr2, &tr2.freed);
            }
        }
    }
    assert(!tr1.failed);
    assert(!tr2.failed);

    transaction_complete(&tr1);

    assert(!tr1.failed);

    for (i = 0; i < blocks1_count; i++) {
        assert(block_set_block_in_set(tr, &tr->fs->free, blocks1[i]));
    }

    if (blocks1 == blocks2) {
        /* free conflict test */
        assert(tr2.failed);
    }
    transaction_complete(&tr2);

    if (blocks1 != blocks2) {
        assert(!tr2.failed);
        for (i = 0; i < blocks2_count; i++) {
            assert(block_set_block_in_set(tr, &tr->fs->free, blocks2[i]));
        }
    }

    assert(!block_cache_debug_get_ref_block_count());

    transaction_free(&tr1);
    transaction_free(&tr2);
}

enum {
    test_free_start = BLOCK_SIZE > 112 ? 20 : 200,
    test_free_split = test_free_start + (BLOCK_SIZE > 112 ? 60 : 20),
    test_free_end = BLOCK_SIZE > 96 ? BLOCK_COUNT : BLOCK_SIZE > 80 ? BLOCK_COUNT - 8 : BLOCK_COUNT - 16,
    test_free_increment = BLOCK_SIZE > 64 ? 2 : 20,

    allocated_size = BLOCK_SIZE > 128 ? 40 : 10,
    allocated2_size = BLOCK_SIZE > 64 ? allocated_size : 4,
};
static data_block_t allocated[allocated_size];
static data_block_t allocated2[allocated2_size];

static void allocate_frag_test(struct transaction *tr)
{
    int i;
    struct block_range range;

    range.start = test_free_start;
    range.end = test_free_end;
    block_set_add_range(tr, &tr->allocated, range);
    assert(!block_cache_debug_get_ref_block_count());

    for (i = test_free_start; i + 1 < test_free_end; i += test_free_increment) {
        if (print_test_verbose) {
            printf("%s: remove block %d\n", __func__, i);
        }
        range.start = i + 1;
        range.end = i + test_free_increment;
        if (range.end > test_free_end) {
            range.end = test_free_end;
        }
        block_set_remove_range(tr, &tr->allocated, range);
        assert(!block_cache_debug_get_ref_block_count());
        assert(!tr->failed);
    }

    if (print_test_verbose) {
        printf("%s: tr.tmp_allocated:\n", __func__);
        block_set_print(tr, &tr->tmp_allocated);
        printf("%s: tr.allocated:\n", __func__);
        block_set_print(tr, &tr->allocated);
    }

    assert(!tr->failed);
    assert(!block_cache_debug_get_ref_block_count());
    transaction_complete(tr);
    assert(!tr->failed);
    transaction_activate(tr);
    if (print_test_verbose) {
        printf("%s: free state after transaction complete:\n", __func__);
        block_set_print(tr, &tr->fs->free);
    }
    assert(!block_cache_debug_get_ref_block_count());
#if FULL_ASSERT
    check_fs_prepare(tr);
    for (i = test_free_start; i < test_free_end; i += test_free_increment) {
        block_set_used_by(i, "test_free_fragmentation", 0);
    }
    assert(check_fs_finish(tr));
#endif
}

static void allocate_free_same_test(struct transaction *tr)
{
    int i;
    printf("%s: start allocate then free same test\n", __func__);
    for (i = 0; i < countof(allocated); i++) {
        allocated[i] = block_allocate(tr);
        assert(!tr->failed);
    }
    if (print_test_verbose) {
        printf("%s: tr.tmp_allocated:\n", __func__);
        block_set_print(tr, &tr->tmp_allocated);
        printf("%s: tr.allocated:\n", __func__);
        block_set_print(tr, &tr->allocated);
    }

    for (i = 0; i < countof(allocated); i++) {
        block_free(tr, allocated[i]);
    }
    assert(!tr->failed);
    transaction_complete(tr);
    assert(!tr->failed);
    transaction_activate(tr);
    assert(block_set_check(tr, &tr->fs->free));
    assert(!block_cache_debug_get_ref_block_count());
#if FULL_ASSERT
    check_fs_prepare(tr);
    for (i = test_free_start; i < test_free_end; i += test_free_increment) {
        block_set_used_by(i, "test_free_fragmentation", 0);
    }
    assert(check_fs_finish(tr));
#endif
    printf("%s: start allocate then free same test, done\n", __func__);
}

static void allocate_free_other_test(struct transaction *tr)
{
    int i;

    printf("%s: start allocate then free some other test\n", __func__);
    for (i = 0; i < countof(allocated); i++) {
        allocated[i] = block_allocate(tr);
        if (print_test_verbose) {
            printf("tr.tmp_allocated after allocate %d, %lld:\n", i, allocated[i]);
            block_set_print(tr, &tr->tmp_allocated);
            printf("tr.allocated after allocate %d, %lld:\n", i, allocated[i]);
            block_set_print(tr, &tr->allocated);
        }
        assert(!tr->failed);
    }
    for (i = test_free_start; i < test_free_split; i += test_free_increment) {
        block_free(tr, i);
        if (print_test_verbose) {
            printf("tr.freed after free %d:\n", i);
            block_set_print(tr, &tr->freed);
        }
        assert(!tr->failed);
    }
    if (print_test_verbose) {
        printf("fs.super->free:\n");
        block_set_print(tr, &tr->fs->free);
    }
    assert(!tr->failed);
    transaction_complete(tr);
    assert(!tr->failed);
    transaction_activate(tr);
    assert(block_set_check(tr, &tr->fs->free));

    for (i = 0; i < countof(allocated); i++) {
        assert(!block_set_block_in_set(tr, &tr->fs->free, allocated[i]));
    }
    for (i = test_free_start; i < test_free_split; i += test_free_increment) {
        assert(block_set_block_in_set(tr, &tr->fs->free, i));
    }
    assert(!block_cache_debug_get_ref_block_count());
#if FULL_ASSERT
    check_fs_prepare(tr);
    for (i = test_free_split; i < test_free_end; i += test_free_increment) {
        block_set_used_by(i, "test_free_fragmentation", 0);
    }
    for (i = 0; i < countof(allocated); i++) {
        block_set_used_by(allocated[i], "allocated", 0);
    }
    assert(check_fs_finish(tr));
#endif
    printf("%s: start allocate then free some other test, done\n", __func__);
}

static void free_frag_rem_test(struct transaction *tr)
{
    int i;
    printf("%s: start free rem test\n", __func__);
    free_frag_etc_test(tr, test_free_split, test_free_end, test_free_increment);
    transaction_complete(tr);
    assert(!tr->failed);
    transaction_activate(tr);
    assert(block_set_check(tr, &tr->fs->free));
    for (i = test_free_split; i < test_free_end; i += test_free_increment) {
        assert(block_set_block_in_set(tr, &tr->fs->free, i));
    }
    assert(!block_cache_debug_get_ref_block_count());
    full_assert(check_fs_allocated(tr, allocated, countof(allocated)));
    printf("%s: start free rem test, done\n", __func__);
}

static void free_test(struct transaction *tr)
{
    int i;

    free_test_etc(tr, allocated, countof(allocated), NULL, 0);

    i = 0;
    do {
        //i = block_set_find_next_block(&fs.free, i, false); // TODO: use this version, currently does not work since ranges aer not merged accross nodes
        while (block_set_find_next_block(tr, &tr->fs->free, i, true) == i) {
            i++;
        }
        int free = block_set_find_next_block(tr, &tr->fs->free, i, true);
        printf("not free: [%d-%d]\n", i, free - 1);
        i = free;
    } while (i);
}

static void allocate_2_transactions_test(struct transaction *tr)
{
    printf("%s: start allocate 2 transactions test\n", __func__);
    allocate_2_transactions_test_etc(tr, allocated, countof(allocated),
                                     allocated2, countof(allocated2));
    printf("%s: allocate 2 transactions test done\n", __func__);
}

static void free_2_transactions_same_test(struct transaction *tr)
{
    printf("%s: start free 2 transactions same test\n", __func__);
    free_test_etc(tr, allocated, countof(allocated),
                                      allocated, countof(allocated));
    full_assert(check_fs_allocated(tr, allocated2, countof(allocated2)));
    printf("%s: free 2 transactions same test done\n", __func__);
}

static void free_2_transactions_same_test_2(struct transaction *tr)
{
    printf("%s: start free 2 transactions same test 2\n", __func__);
    free_test_etc(tr, allocated2, countof(allocated2),
                                      allocated2, countof(allocated2));
    full_assert(check_fs(tr));
    printf("%s: free 2 transactions same test 2 done\n", __func__);
}

static void allocate_all_test(struct transaction *tr)
{
    while (block_allocate(tr)) {
        assert(!tr->failed);
    }
    assert(tr->failed);
    transaction_complete(tr);
    transaction_activate(tr);
}

static void open_test_file_etc(struct transaction *tr,
                               struct file_handle *file,
                               const char *path,
                               enum file_create_mode create,
                               bool expect_failure)
{
    bool found;
    found = file_open(tr, path, file, create);
    if (print_test_verbose) {
        printf("%s: lookup file %s, create %d, got %lld:\n", __func__,
               path, create, block_mac_to_block(tr, &file->block_mac));
    }

    assert(found == !expect_failure);
    assert(!found || block_mac_valid(tr, &file->block_mac));
}

static void open_test_file(struct transaction *tr,
                           struct file_handle *file,
                           const char *path,
                           enum file_create_mode create)
{
    open_test_file_etc(tr, file, path, create, false);
}

static void file_allocate_all_test(struct transaction *master_tr,
                                   uint tr_count,
                                   int success_count,
                                   int step_size,
                                   const char *path,
                                   enum file_create_mode create)
{
    int i;
    int j;
    int done;
    int count;
    struct file_handle file[tr_count];
    data_block_t file_size[tr_count];
    struct transaction tr[tr_count];
    int written_count[tr_count];
    size_t file_block_size = master_tr->fs->dev->block_size - sizeof(struct iv);
    void *block_data_rw;
    obj_ref_t ref = OBJ_REF_INITIAL_VALUE(ref);

    for (i = 0; i < tr_count; i++) {
        transaction_init(&tr[i], master_tr->fs, true);
    }

    for (count = INT_MAX, done = 0; count > 0; count -= step_size) {
        for (i = 0; i < tr_count; i++) {
            open_test_file(&tr[i], &file[i], path, create);
            file_size[i] = file[i].size;
            written_count[i] = 0;
        }

        for (j = 0, done = 0; done != tr_count && j < count; j++) {
            for (i = 0, done = 0; i < tr_count; i++) {
                if (tr[i].failed) {
                    done++;
                    assert(j);
                    continue;
                }
                block_data_rw = file_get_block_write(&tr[i], &file[i], j, true, &ref);
                if (!block_data_rw) {
                    done++;
                    continue;
                }
                assert(!tr[i].failed);
                file_block_put_dirty(&tr[i], &file[i], j, block_data_rw, &ref);
                written_count[i] = j + 1;
            }
        }
        for (i = 0, done = 0; i < tr_count; i++) {
            file_set_size(&tr[i], &file[i], written_count[i] * file_block_size);
            if (count == INT_MAX) {
                assert(tr[i].failed);
            }
            transaction_complete(&tr[i]);
            if (!tr[i].failed) {
                assert(file[i].size == written_count[i] * file_block_size);
                done++;
            } else if (!done) {
                assert(file_size[i] == file[i].size);
            }
            file_close(&file[i]);
            transaction_activate(&tr[i]);
        }
        if (count == INT_MAX) {
            count = j;
        }
        if (success_count && done) {
            success_count--;
        }
        if (!success_count) {
            break;
        }
        if (done) {
            file_delete(&tr[0], path);
            transaction_complete(&tr[0]);
            assert(!tr[0].failed);
            transaction_activate(&tr[0]);
        }
    }
    assert(!success_count);
    for (i = 0; i < tr_count; i++) {
        transaction_complete(&tr[i]);
        transaction_free(&tr[i]);
    }
}

static void file_create_all_test(struct transaction *tr)
{
    int i;
    char path[4+8+1];
    struct file_handle file;

    bool found;
    for (i = 0;; i++) {
        snprintf(path, sizeof(path), "test%08x", i);
        found = file_open(tr, path, &file, FILE_OPEN_CREATE_EXCLUSIVE);
        if (!found) {
            break;
        }
        file_close(&file);
        assert(!tr->failed);
    }

    assert(tr->failed);
    transaction_complete(tr);
    transaction_activate(tr);
}

/* run tests on already open file */
static void file_test_open(struct transaction *tr,
                           struct file_handle *file,
                           int allocate,
                           int read,
                           int free,
                           int id)
{
    int i;
    int *block_data_rw;
    obj_ref_t ref = OBJ_REF_INITIAL_VALUE(ref);
    const int *block_data_ro;
    size_t file_block_size = tr->fs->dev->block_size - sizeof(struct iv);

    if (allocate) {
        for (i = 0; i < allocate; i++) {
            block_data_rw = file_get_block_write(tr, file, i, true, &ref);
            if (print_test_verbose) {
                printf("%s: allocate file block %d, %lld:\n",
                       __func__, i, data_to_block_num(block_data_rw));
            }
            assert(block_data_rw);
            /* TODO: store iv in file block map */
            block_data_rw = (void *)block_data_rw + sizeof(struct iv);
            //block_data_rw = block_get_cleared(block)+ sizeof(struct iv);
            block_data_rw[0] = i;
            block_data_rw[1] = ~i;
            block_data_rw[2] = id;
            block_data_rw[3] = ~id;
            file_block_put_dirty(tr, file, i,
                                 (void *)block_data_rw - sizeof(struct iv), &ref);
        }
        if (file->size < i * file_block_size) {
            file_set_size(tr, file, i * file_block_size);
        }
        assert(file->size >= i * file_block_size);
        if (print_test_verbose) {
            printf("%s: allocated %d file blocks\n", __func__, i);
            file_print(tr, file);
        }
    }

    if (read) {
        for (i = 0;; i++) {
            block_data_ro = file_get_block(tr, file, i, &ref);
            if (!block_data_ro) {
                break;
            }
            if (print_test_verbose) {
                printf("%s: found file block %d, %lld:\n",
                       __func__, i, data_to_block_num(block_data_ro));
            }
            block_data_ro = (void *)block_data_ro + sizeof(struct iv);
            assert(block_data_ro[0] == i);
            assert(block_data_ro[1] == ~i);
            assert(block_data_ro[2] == id);
            assert(block_data_ro[3] == ~id);
            file_block_put((void *)block_data_ro - sizeof(struct iv), &ref);
        }
        assert(i == read);
        assert(file->size >= i * file_block_size);
    }

    if (free) {
        file_set_size(tr, file, 0);
        for (i = 0; i < free; i++) {
            block_data_ro = file_get_block(tr, file, i, &ref);
            if (block_data_ro) {
                file_block_put(block_data_ro, &ref);
                printf("%s: file block %d, %lld not deleted\n",
                       __func__, i, data_to_block_num(block_data_ro));
                break;
            }
        }
        if (print_test_verbose) {
            printf("%s: deleted %d file blocks\n", __func__, i);
            file_print(tr, file);
        }
        assert(i == free);
    }
}

static void file_test(struct transaction *tr,
                      const char *path,
                      enum file_create_mode create,
                      int allocate,
                      int read,
                      int free,
                      bool delete,
                      int id)
{
    bool deleted;
    struct file_handle file;

    open_test_file(tr, &file, path, create);
    file_test_open(tr, &file, allocate, read, free, id);

    if (delete) {
        if (print_test_verbose) {
            printf("%s: delete file %s, at %lld:\n",
                   __func__, path, block_mac_to_block(tr, &file.block_mac));
        }
        deleted = file_delete(tr, path);
        assert(deleted);
    }

    file_close(&file);
}

static void file_test_split_tr(struct transaction *tr,
                               const char *path,
                               enum file_create_mode create,
                               int open_count,
                               int allocate_file_index,
                               int allocate_index,
                               int allocate,
                               int read,
                               int free,
                               bool delete,
                               int id)
{
    bool deleted;
    int i;
    struct file_handle file[open_count];

    assert(allocate_file_index <= allocate_index);
    assert(allocate_index < open_count);

    for (i = 0; i < open_count; i++) {
        open_test_file(tr, &file[i], path, (i == 0) ? create : FILE_OPEN_NO_CREATE);
        assert(file[i].size == 0 || i > allocate_index);
        if (i == allocate_index) {
            file_test_open(tr, &file[allocate_file_index], allocate, 0, 0, id);
        }
    }

    if (create == FILE_OPEN_NO_CREATE) {
        transaction_fail(tr);
        transaction_activate(tr);
        for (i = 0; i < open_count; i++) {
            assert(file[i].size == 0);
        }

        file_test_open(tr, &file[allocate_file_index], allocate, 0, 0, id);
    }

    assert(!tr->failed);
    transaction_complete(tr);
    assert(!tr->failed);
    transaction_activate(tr);

    for (i = 0; i < open_count; i++) {
        assert(file[i].size != 0);
    }

    for (i = 1; i < open_count; i++) {
        file_test_open(tr, &file[i], 0, read, 0, id);
        assert(!tr->failed);
        transaction_complete(tr);
        assert(!tr->failed);
        transaction_activate(tr);
    }

    file_test_open(tr, &file[0], 0, read, free, id);

    if (delete) {
        if (print_test_verbose) {
            printf("%s: delete file %s, at %lld:\n",
                   __func__, path, block_mac_to_block(tr, &file[i].block_mac));
        }
        deleted = file_delete(tr, path);
        assert(deleted);
    }

    for (i = 0; i < open_count; i++) {
        file_close(&file[i]);
    }
}

static void file_read_after_delete_test(struct transaction *tr)
{
    const char *path = "test1s";
    struct file_handle file;
    struct file_handle file2;
    obj_ref_t ref = OBJ_REF_INITIAL_VALUE(ref);
    const void *block_data_ro;
    void *block_data_rw;
    struct transaction tr2;

    transaction_init(&tr2, tr->fs, true);

    /* create test file */
    open_test_file(tr, &file, path, FILE_OPEN_CREATE_EXCLUSIVE);
    block_data_rw = file_get_block_write(tr, &file, 0, false, &ref);
    file_block_put_dirty(tr, &file, 0, block_data_rw, &ref);
    transaction_complete(tr);
    assert(!tr->failed);

    /* open in second transaction */
    open_test_file(&tr2, &file2, path, FILE_OPEN_NO_CREATE);

    /* delete and try read in same transaction */
    transaction_activate(tr);
    file_delete(tr, path);
    block_data_ro = file_get_block(tr, &file, 0, &ref);
    assert(!block_data_ro);
    assert(tr->failed);

    /* read in second transaction */
    block_data_ro = file_get_block(&tr2, &file2, 0, &ref);
    assert(block_data_ro);
    file_block_put(block_data_ro, &ref);

    /* read file then delete file */
    transaction_activate(tr);
    block_data_ro = file_get_block(tr, &file, 0, &ref);
    assert(block_data_ro);
    file_block_put(block_data_ro, &ref);

    file_delete(tr, path);
    transaction_complete(tr);
    assert(!tr->failed);

    /* try to read in both transactions */
    transaction_activate(tr);
    block_data_ro = file_get_block(tr, &file, 0, &ref);
    assert(!block_data_ro);
    assert(tr->failed);

    block_data_ro = file_get_block(&tr2, &file2, 0, &ref);
    assert(!block_data_ro);
    assert(tr2.failed);

    file_close(&file);
    file_close(&file2);

    transaction_activate(tr);
    transaction_free(&tr2);
}

static const int file_test_block_count = BLOCK_SIZE > 64 ? 40 : 10;
static const int file_test_many_file_count = BLOCK_SIZE > 80 ? 40 : 10;

static void file_create1_small_test(struct transaction *tr)
{
    file_test(tr, "test1s", FILE_OPEN_CREATE_EXCLUSIVE, 0, 0, 0, false, 1);
}

static void file_write1_small_test(struct transaction *tr)
{
    file_test(tr, "test1s", FILE_OPEN_NO_CREATE, 1, 0, 0, false, 1);
}

static void file_delete1_small_test(struct transaction *tr)
{
    file_test(tr, "test1s", FILE_OPEN_NO_CREATE, 0, 1, 1, true, 1);
}

static void file_create_write_delete1_small_test(struct transaction *tr)
{
    file_test(tr, "test1s", FILE_OPEN_CREATE_EXCLUSIVE, 1, 1, 1, true, 1);
}

static void file_splittr1_small_test(struct transaction *tr)
{
    file_test_split_tr(tr, "test1s", FILE_OPEN_NO_CREATE, 1, 0, 0, 1, 1, 0, false, 1);
}

static void file_splittr1o4_small_test(struct transaction *tr)
{
#if 0
    /*
     * Disabled test: Current file code does not allow having the same
     * file open more than once per transaction.
     */
    file_test_split_tr(tr, "test1s", FILE_OPEN_NO_CREATE, 4, 1, 2, 1, 1, 0, false, 1);
#else
    struct file_handle file[2];
    open_test_file_etc(tr, &file[0], "test1s", FILE_OPEN_NO_CREATE, false);
    open_test_file_etc(tr, &file[1], "test1s", FILE_OPEN_NO_CREATE, true);
    file_close(&file[0]);
    file_splittr1_small_test(tr);
#endif
}

static void file_splittr1c_small_test(struct transaction *tr)
{
    file_test_split_tr(tr, "test1s", FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, 1, 1, 1, true, 1);
}

static void file_splittr1o4c_small_test(struct transaction *tr)
{
#if 0
    /*
     * Disabled test: Current file code does not allow having the same
     * file open more than once per transaction.
     */
    file_test_split_tr(tr, "test1s", FILE_OPEN_CREATE_EXCLUSIVE, 4, 1, 2, 1, 1, 1, true, 1);
#endif
}

static void file_splittr1o4cl_small_test(struct transaction *tr)
{
#if 0
    /*
     * Disabled test: Current file code does not allow having the same
     * file open more than once per transaction.
     */
    file_test_split_tr(tr, "test1s", FILE_OPEN_CREATE_EXCLUSIVE, 4, 2, 3, 1, 1, 1, true, 1);
#endif
}

static void file_create1_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_CREATE_EXCLUSIVE, 0, 0, 0, false, 1);
}

static void file_write1h_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, file_test_block_count / 2, 0, 0, false, 1);
}

static void file_write1_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, file_test_block_count, 0, 0, false, 1);
}

static void file_delete1_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, 0, file_test_block_count, file_test_block_count, true, 1);
}

static void file_delete_create_write1_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, 0, file_test_block_count, file_test_block_count, true, 1);
    file_test(tr, "test1", FILE_OPEN_CREATE_EXCLUSIVE, file_test_block_count, 0, 0, false, 1);
}

static void file_delete1_no_free_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, 0, 0, 0, true, 1);
}

static void file_create2_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_CREATE_EXCLUSIVE, file_test_block_count, 0, 0, false, 2);
    file_test(tr, "test2", FILE_OPEN_CREATE_EXCLUSIVE, file_test_block_count, 0, 0, false, 3);
}

static void file_delete2_test(struct transaction *tr)
{
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, 0, file_test_block_count, false, false, 2);
    file_test(tr, "test2", FILE_OPEN_NO_CREATE, 0, file_test_block_count, false, false, 3);
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, 0, file_test_block_count, file_test_block_count, file_test_block_count, 2);
    file_test(tr, "test2", FILE_OPEN_NO_CREATE, 0, file_test_block_count, file_test_block_count, file_test_block_count, 3);
}

static void file_create2_read_after_commit_test(struct transaction *tr)
{
    int i;
    struct file_handle file[2];

    open_test_file(tr, &file[0], "test1", FILE_OPEN_CREATE_EXCLUSIVE);
    open_test_file(tr, &file[1], "test2", FILE_OPEN_CREATE_EXCLUSIVE);

    for (i = 0; i < 2; i++) {
        file_test_open(tr, &file[i], file_test_block_count, 0, 0, 2 + i);
    }
    transaction_complete(tr);
    assert(!tr->failed);
    transaction_activate(tr);
    for (i = 0; i < 2; i++) {
        file_test_open(tr, &file[i], 0, file_test_block_count, 0, 2 + i);
        file_close(&file[i]);
    }
}


static void file_create3_conflict_test(struct transaction *tr)
{
    struct transaction tr1;
    struct transaction tr2;
    struct transaction tr3;

    transaction_init(&tr1, tr->fs, true);
    transaction_init(&tr2, tr->fs, true);
    transaction_init(&tr3, tr->fs, true);

    file_test(&tr1, "test1", FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, false, 4);
    file_test(&tr2, "test1", FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, false, 5);
    file_test(&tr3, "test2", FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, false, 6);

    assert(!tr1.failed);
    assert(!tr2.failed);
    assert(!tr3.failed);
    transaction_complete(&tr1);
    transaction_complete(&tr2);
    transaction_complete(&tr3);
    assert(!tr1.failed);
    assert(tr2.failed);
    assert(!tr3.failed);
    file_test(tr, "test1", FILE_OPEN_NO_CREATE, 0, 1, 1, true, 4);
    file_test(tr, "test2", FILE_OPEN_NO_CREATE, 0, 1, 1, true, 6);

    transaction_free(&tr1);
    transaction_free(&tr2);
    transaction_free(&tr3);
}

static void file_create_delete_2_transaction_test(struct transaction *tr)
{
    struct transaction tr1;
    struct transaction tr2;

    transaction_init(&tr1, tr->fs, true);
    transaction_init(&tr2, tr->fs, true);

    file_test(&tr1, "test1", FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, false, 7);
    file_test(&tr2, "test2", FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, false, 8);

    assert(!tr1.failed);
    assert(!tr2.failed);
    transaction_complete(&tr1);
    transaction_complete(&tr2);
    assert(!tr1.failed);
    assert(!tr2.failed);

    transaction_activate(&tr1);
    transaction_activate(&tr2);
    file_test(&tr1, "test1", FILE_OPEN_NO_CREATE, 0, 1, 1, true, 7);
    file_test(&tr2, "test2", FILE_OPEN_NO_CREATE, 0, 1, 1, true, 8);

    assert(!tr1.failed);
    assert(!tr2.failed);
    transaction_complete(&tr1);
    transaction_complete(&tr2);
    assert(!tr1.failed);
    assert(!tr2.failed);

    transaction_free(&tr1);
    transaction_free(&tr2);
}

static void file_create_many_test(struct transaction *tr)
{
    char path[10];
    int i;
    for (i = 0; i < file_test_many_file_count; i++) {
        snprintf(path, sizeof(path), "test%d", i);
        file_test(tr, path, FILE_OPEN_CREATE_EXCLUSIVE, 1, 0, 0, false, 7 + i);
    }
}

static void file_delete_many_test(struct transaction *tr)
{
    char path[10];
    int i;

    files_print(tr);

    for (i = 0; i < file_test_many_file_count; i++) {
        snprintf(path, sizeof(path), "test%d", i);
        file_test(tr, path, false, 0, 1, 1, true, 7 + i);
    }
}

static void file_allocate_all1_test(struct transaction *tr)
{
    file_allocate_all_test(tr, 1, 0, 1, "test1", FILE_OPEN_CREATE);
}

static void file_allocate_all_2tr_1_test(struct transaction *tr)
{
    file_allocate_all_test(tr, 2, 0, 1, "test1", FILE_OPEN_CREATE);
}

static void file_allocate_all_8tr_1_test(struct transaction *tr)
{
    file_allocate_all_test(tr, 8, 0, 1, "test1", FILE_OPEN_CREATE);
}

static void file_allocate_all_complete1_test(struct transaction *tr)
{
    file_allocate_all_test(tr, 1, 1, 1, "test1", FILE_OPEN_CREATE);
}

static void file_allocate_all_complete_multi1_test(struct transaction *tr)
{
    file_allocate_all_test(tr, 2, 8, 10, "test1", FILE_OPEN_CREATE);
}

static void file_allocate_leave_10_test(struct transaction *tr)
{
    file_allocate_all_test(tr, 1, 1, 10, "test1", FILE_OPEN_CREATE);
}

static void future_fs_version_test(struct transaction *tr)
{
    obj_ref_t super_ref = OBJ_REF_INITIAL_VALUE(super_ref);
    struct fs *fs = tr->fs;
    const struct key *key = fs->key;
    struct block_device *dev = fs->dev;
    struct block_device *super_dev = fs->super_dev;
    const void *super_ro;
    uint32_t *super_rw;
    data_block_t block;
    int ret;

    transaction_complete(tr);

    block = tr->fs->super_block[fs->super_block_version & 1];
    super_ro = block_get_super(fs, block, &super_ref);
    assert(super_ro);
    super_rw = block_dirty(tr, super_ro, false);
    assert(super_rw);
    super_rw[7]++;
    block_put_dirty_no_mac(super_rw, &super_ref);
    block_cache_clean_transaction(tr);

    transaction_free(tr);

    ret = fs_init(fs, key, dev, super_dev, false);
    assert(ret == -1);

    ret = fs_init(fs, key, dev, super_dev, true);
    assert(ret == -1);

    fs->dev = dev;
    fs->super_dev = super_dev;
    transaction_init(tr, fs, false);
    super_ro = block_get_super(fs, block, &super_ref);
    assert(super_ro);
    super_rw = block_dirty(tr, super_ro, false);
    assert(super_rw);
    super_rw[7]--;
    block_put_dirty_no_mac(super_rw, &super_ref);
    block_cache_clean_transaction(tr);
    transaction_free(tr);

    ret = fs_init(fs, key, dev, super_dev, false);
    assert(ret == 0);

    transaction_init(tr, fs, true);
}

#if 0
static void file_allocate_leave_10_test2(struct transaction *tr)
{
    int i;
    for (i = 1; i < 10; i++) {
        file_allocate_all_test(tr, 1, 0, 1, "test1", FILE_OPEN_NO_CREATE);
        file_allocate_all_test(tr, 1, 1, i, "test2", FILE_OPEN_CREATE);
        file_delete(tr, "test2");
        transaction_complete(tr);
        assert(!tr->failed);
        transaction_activate(tr);
    }
}
#endif

#define TEST(a, ...) {.name = #a, .func = (a), ##__VA_ARGS__}
struct {
    const char *name;
    bool no_free_check;
    void (*func)(struct transaction *tr);
} tests[]= {
    TEST(empty_test),
    TEST(empty_test),
    TEST(block_tree_test),
    TEST(block_set_test),
    TEST(block_map_test),
    TEST(allocate_frag_test, .no_free_check = true),
    TEST(allocate_free_same_test, .no_free_check = true),
    TEST(allocate_free_other_test, .no_free_check = true),
    TEST(free_frag_rem_test, .no_free_check = true),
    TEST(free_test),
    TEST(allocate_2_transactions_test, .no_free_check = true),
    TEST(free_2_transactions_same_test, .no_free_check = true),
    TEST(free_2_transactions_same_test_2),
    TEST(allocate_all_test),
    TEST(block_tree_allocate_all_test),
    TEST(file_create1_small_test),
    TEST(file_write1_small_test),
    TEST(file_delete1_small_test),
    TEST(file_read_after_delete_test),
    TEST(file_create1_small_test),
    TEST(file_splittr1_small_test),
    TEST(file_delete1_small_test),
    TEST(file_create1_small_test),
    TEST(file_splittr1o4_small_test),
    TEST(file_delete1_small_test),
    TEST(file_create_write_delete1_small_test),
    TEST(file_splittr1c_small_test),
    TEST(file_splittr1o4c_small_test),
    TEST(file_splittr1o4cl_small_test),
    TEST(file_create1_test),
    TEST(file_write1h_test),
    TEST(file_write1_test),
    TEST(file_delete1_test),
    TEST(file_create2_test),
    TEST(file_delete2_test),
    TEST(file_create2_read_after_commit_test),
    TEST(file_delete2_test),
    TEST(file_create3_conflict_test),
    TEST(file_create_delete_2_transaction_test),
    TEST(file_create_many_test),
    TEST(file_create1_small_test),
    TEST(file_write1_small_test),
    TEST(file_write1_small_test),
    TEST(file_delete1_small_test),
    TEST(file_delete_many_test),
    TEST(file_create1_test),
    TEST(file_allocate_all1_test),
    TEST(file_write1_test),
    TEST(file_delete1_no_free_test),
    TEST(file_allocate_all1_test),
    TEST(file_create_all_test),
    TEST(file_create1_test),
    TEST(file_write1_test),
    TEST(file_delete_create_write1_test),
    TEST(file_write1_test),
    TEST(file_delete1_test),
    TEST(file_allocate_all_2tr_1_test),
    TEST(file_allocate_all_8tr_1_test),
    TEST(file_create1_test),
    TEST(file_allocate_all_complete1_test),
    TEST(file_delete1_no_free_test),
    TEST(file_allocate_all_complete1_test),
    TEST(file_delete1_no_free_test),
    TEST(file_create1_test),
    TEST(file_allocate_all_complete_multi1_test),
    TEST(file_delete1_no_free_test),
    TEST(file_allocate_all_complete_multi1_test),
    TEST(file_delete1_no_free_test),
    TEST(file_create1_test),
    TEST(file_allocate_leave_10_test),
    TEST(file_create1_small_test),
    TEST(file_write1_small_test),
    TEST(file_delete1_small_test),
    TEST(file_create1_small_test),
    TEST(file_splittr1_small_test),
    TEST(file_delete1_small_test),
    TEST(file_create1_small_test),
    TEST(file_splittr1o4_small_test),
    TEST(file_delete1_small_test),
    TEST(file_allocate_all1_test),
//    TEST(file_write1_test),
//    TEST(file_allocate_leave_10_test2),
    TEST(file_delete1_no_free_test),
    TEST(future_fs_version_test),
};

int main(int argc, const char *argv[])
{
    //struct block_set_node *node;
    struct block_device dev = {
        .start_read = block_test_start_read,
        .start_write = block_test_start_write,
        .block_count = BLOCK_COUNT,
        .block_size = 256,
        .block_num_size = 8,
        .mac_size = 16,
        .tamper_detecting = true,
        .io_ops = LIST_INITIAL_VALUE(dev.io_ops),
    };
    struct block_device dev256 = {
        .start_read = block_test_start_read,
        .start_write = block_test_start_write,
        .block_count = 0x10000,
        .block_size = 256,
        .block_num_size = 2,
        .mac_size = 2,
        .tamper_detecting = true,
        .io_ops = LIST_INITIAL_VALUE(dev256.io_ops),
    };
    struct fs fs = {
        .dev = &dev,
        .transactions = LIST_INITIAL_VALUE(fs.transactions),
        .allocated = LIST_INITIAL_VALUE(fs.allocated),
        .files = {
            .copy_on_write = true,
            //.allow_copy_on_write = true,
        },
    };
    struct transaction tr = {};
    int i;
    bool test_remount = true;

    if (argc > 1) {
        print_lookup = true;
    }

    assert(test_free_start < test_free_split);
    assert(test_free_split < test_free_end);

    stats_timer_reset();

    block_tree_check_config(&dev);
    block_tree_check_config(&dev256);
    block_tree_check_config_done();
    block_cache_init();

    fs_init(&fs, &key, &dev, &dev, true);
    fs.reserved_count = 18; /* HACK: override default reserved space */
    transaction_init(&tr, &fs, false);

    for (i = 0; i < countof(tests); i++) {
        transaction_activate(&tr);
        printf("%s: start test: %s\n", __func__, tests[i].name);
        tests[i].func(&tr);
        transaction_complete(&tr);
        assert(!block_cache_debug_get_ref_block_count());
        if (!tests[i].no_free_check) {
            full_assert(check_fs(&tr));
        }
        if (0) { // per test stats
            stats_timer_print();
            stats_timer_reset();
        }
        printf("%s: test done: %s\n", __func__, tests[i].name);
        assert(!tr.failed);
        if (test_remount) {
            transaction_free(&tr);
            fs_init(&fs, &key, &dev, &dev, false);
            fs.reserved_count = 18; /* HACK: override default reserved space */
            transaction_init(&tr, &fs, false);
        }
    }
    full_assert(check_fs(&tr));
    files_print(&tr);
    block_set_print(&tr, &tr.fs->free);
    stats_timer_print();
    transaction_free(&tr);

    printf("%s: done\n", __func__);

    return 0;
}
