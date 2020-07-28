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

#ifndef LOCAL_TRACE
#define LOCAL_TRACE TRACE_LEVEL_INIT
#endif
#ifndef LOCAL_TRACE_ERR
#define LOCAL_TRACE_ERR TRACE_LEVEL_INIT
#endif

#include "array.h"
#include "block_allocator.h"
#include "block_set.h"
#include "debug.h"
#include "file.h"
#include "transaction.h"

#define SUPER_BLOCK_MAGIC (0x0073797473757274) /* trustys */
#define SUPER_BLOCK_FLAGS_VERSION_MASK (0x3)
#define SUPER_BLOCK_FLAGS_BLOCK_INDEX_MASK (0x1)
#define SUPER_BLOCK_FS_VERSION (0)

/**
 * struct super_block - On-disk root block for file system state
 * @iv:             Initial value used for encrypt/decrypt.
 * @magic:          SUPER_BLOCK_MAGIC.
 * @flags:          Version in bottom two bits, other bits are reserved.
 * @fs_version:     Required file system version. If greater than
 *                  %SUPER_BLOCK_FS_VERSION, do not mount or overwrite
 *                  filesystem.
 * @block_size:     Block size of file system.
 * @block_num_size: Number of bytes used to store block numbers.
 * @mac_size:       number of bytes used to store mac values.
 * @res1:           Reserved for future use. Write 0, read ignore.
 * @res2:           Reserved for future use. Write 0, read ignore.
 * @block_count:    Size of file system.
 * @free:           Block and mac of free set root node.
 * @free_count:     Currently unused.
 * @files:          Block and mac of files tree root node.
 * @res3:           Reserved for future use. Write 0, read ignore.
 * @flags2:         Copy of @flags. Allows storing the super-block in a device
 *                  that does not support an atomic write of the entire
 *                  super-block.
 *
 * Block numbers and macs in @free and @files are packed as indicated by
 * @block_num_size and @mac_size, but unlike other on-disk data, the size of the
 * whole field is always the full 24 bytes needed for a 8 byte block number and
 * 16 byte mac This allows the @flags2 to be validated before knowing
 * @block_num_size and @mac_size.
 */
struct super_block {
    struct iv iv;
    uint64_t magic;
    uint32_t flags;
    uint32_t fs_version;
    uint32_t block_size;
    uint8_t block_num_size;
    uint8_t mac_size;
    uint8_t res1;
    uint8_t res2;
    data_block_t block_count;
    struct block_mac free;
    data_block_t free_count;
    struct block_mac files;
    uint32_t res3[5];
    uint32_t flags2;
};
STATIC_ASSERT(offsetof(struct super_block, flags2) == 124);
STATIC_ASSERT(sizeof(struct super_block) <= 128);
STATIC_ASSERT(sizeof(struct super_block) >= 128);

/**
 * update_super_block - Generate and write superblock
 * @tr:         Transaction object.
 * @free:       New free root.
 * @files:      New files root.
 *
 * Return: %true if super block was updated (in cache), %false if transaction
 * failed before super block was updated.
 */
bool update_super_block(struct transaction *tr,
                        const struct block_mac *free,
                        const struct block_mac *files)
{
    struct super_block *super_rw;
    obj_ref_t super_ref = OBJ_REF_INITIAL_VALUE(super_ref);
    uint ver;
    uint index;
    uint32_t block_size = tr->fs->super_dev->block_size;

    assert(block_size >= sizeof(struct super_block));

    ver = (tr->fs->super_block_version + 1) & SUPER_BLOCK_FLAGS_VERSION_MASK;
    index = ver & SUPER_BLOCK_FLAGS_BLOCK_INDEX_MASK;

    pr_write("write super block %lld, ver %d\n",
             tr->fs->super_block[index], ver);

    super_rw = block_get_cleared_super(tr, tr->fs->super_block[index],
                                       &super_ref);
    if (tr->failed) {
        block_put_dirty_discard(super_rw, &super_ref);
        return false;
    }
    super_rw->magic = SUPER_BLOCK_MAGIC;
    super_rw->flags = ver;
    super_rw->fs_version = SUPER_BLOCK_FS_VERSION; /* TODO: keep existing fs version when possible */
    super_rw->block_size = tr->fs->dev->block_size;
    super_rw->block_num_size = tr->fs->block_num_size;
    super_rw->mac_size = tr->fs->mac_size;
    super_rw->block_count = tr->fs->dev->block_count;
    super_rw->free = *free;
    super_rw->free_count = 0; /* TODO: remove or update */
    super_rw->files = *files;
    super_rw->flags2 = ver;
    tr->fs->written_super_block_version = ver;

    block_put_dirty_no_mac(super_rw, &super_ref);

    return true;
}

/**
 * super_block_valid - Check if superblock is valid
 * @dev:        Block device that supoer block was read from.
 * @super:      Super block data.
 *
 * Return: %true if @super is valid for @dev, %false otherwise.
 */
static bool super_block_valid(const struct block_device *dev,
                              const struct super_block *super)
{
    if (super->magic != SUPER_BLOCK_MAGIC) {
        pr_init("bad magic, 0x%llx\n", (unsigned long long)super->magic);
        return false;
    }
    if (super->flags != super->flags2) {
        pr_warn("flags, 0x%x, does not match flags2, 0x%x\n",
               super->flags, super->flags2);
        return false;
    }
    if (super->fs_version > SUPER_BLOCK_FS_VERSION) {
        pr_warn("super block is from the future: 0x%x\n", super->fs_version);
        return true;
    }
    if (super->flags & ~SUPER_BLOCK_FLAGS_VERSION_MASK) {
        pr_warn("unknown flags set, 0x%x\n", super->flags);
        return false;
    }
    if (super->block_size != dev->block_size) {
        pr_warn("bad block size 0x%x, expected 0x%zx\n",
                super->block_size, dev->block_size);
        return false;
    }
    if (super->block_num_size < dev->block_num_size ||
        super->block_num_size > sizeof(data_block_t)) {
        pr_warn("invalid block_num_size %d not in [%zd, %zd]\n",
                super->block_num_size,
                dev->block_num_size, sizeof(data_block_t));
        return false;
    }
    if (super->mac_size < dev->mac_size ||
        super->mac_size > sizeof(struct mac)) {
        pr_warn("invalid mac_size %d not in [%zd, %zd]\n",
                super->mac_size, dev->mac_size, sizeof(struct mac));
        return false;
    }
    if (!dev->tamper_detecting && super->mac_size != sizeof(struct mac)) {
        pr_warn("invalid mac_size %d != %zd\n",
                super->mac_size, sizeof(data_block_t));
        return false;
    }
    if (super->block_count > dev->block_count) {
        pr_warn("bad block count 0x%llx, expected <= 0x%llx\n",
                super->block_count, dev->block_count);
        return false;
    }
    return true;
}

/**
 * use_new_super - Check if new superblock is valid and more recent than old
 * @dev:                Block device that super block was read from.
 * @new_super:          New super block data.
 * @new_super_index:    Index that @new_super was read from.
 * @old_super:          Old super block data, or %NULL.
 *
 * Return: %true if @new_super is valid for @dev, and more recent than
 * @old_super (or @old_super is %NULL), %false otherwise.
 */
static bool use_new_super(const struct block_device *dev,
                          const struct super_block *new_super,
                          uint new_super_index,
                          const struct super_block *old_super)
{
    uint dv;
    if (!super_block_valid(dev, new_super)) {
        return false;
    }
    if ((new_super->flags & SUPER_BLOCK_FLAGS_BLOCK_INDEX_MASK) != new_super_index) {
        pr_warn("block index, 0x%x, does not match flags, 0x%x\n",
                new_super_index, new_super->flags);
        return false;
    }
    if (!old_super) {
        return true;
    }
    dv = (new_super->flags - old_super->flags) & SUPER_BLOCK_FLAGS_VERSION_MASK;
    pr_read("version delta, %d (new flags 0x%x, old flags 0x%x)\n",
            dv, new_super->flags, old_super->flags);
    if (dv == 1) {
        return true;
    }
    if (dv == 3) {
        return false;
    }
    pr_warn("bad version delta, %d (new flags 0x%x, old flags 0x%x)\n",
            dv, new_super->flags, old_super->flags);
    return false;
}

/* TODO: move to obj_ref lib */
static void obj_ref_transfer(obj_ref_t *dst, obj_ref_t *src)
{
    struct list_node *prev = src->ref_node.prev;
    list_delete(&src->ref_node);
    list_add_after(prev, &dst->ref_node);
}

/**
 * fs_init_empty - Initialize free set for empty file system
 * @fs:         File system state object.
 */
static void fs_init_empty(struct fs *fs)
{
    struct block_range range = {
        .start = fs->min_block_num,
        .end = fs->dev->block_count,
    };
    block_set_add_initial_range(&fs->free, range);
}

/**
 * fs_init_from_super - Initialize file system from super block
 * @fs:         File system state object.
 * @super:      Superblock data, or %NULL.
 * @clear:      If %true, clear fs state if allowed by super block state.
 *
 * Return: 0 if super block was usable, -1 if not.
 */
static int fs_init_from_super(struct fs *fs,
                              const struct super_block *super,
                              bool clear)
{
    size_t block_mac_size;

    if (super && super->fs_version > SUPER_BLOCK_FS_VERSION) {
        pr_err("ERROR: super block is from the future 0x%x\n",
               super->fs_version);
        return -1;
    }
    if (clear) {
        super = NULL;
    }
    if(super) {
        fs->block_num_size = super->block_num_size;
        fs->mac_size = super->mac_size;
    } else {
        fs->block_num_size = fs->dev->block_num_size;
        fs->mac_size = fs->dev->mac_size;
    }
    block_mac_size = fs->block_num_size + fs->mac_size;
    block_set_init(fs, &fs->free);
    fs->free.block_tree.copy_on_write = true;
    block_tree_init(&fs->files, fs->dev->block_size,
                    fs->block_num_size,
                    block_mac_size, block_mac_size);
    fs->files.copy_on_write = true;
    fs->files.allow_copy_on_write = true;

    /* Reserve 1/4 for tmp blocks plus half of the remaining space */
    fs->reserved_count = fs->dev->block_count / 8 * 5;

    if (super) {
        fs->free.block_tree.root = super->free;
        fs->files.root = super->files;
        fs->super_block_version = super->flags &
                                     SUPER_BLOCK_FLAGS_VERSION_MASK;
        pr_init("loaded super block version %d\n", fs->super_block_version);
    } else {
        if (clear) {
            pr_init("clear requested, create empty\n");
        } else {
            pr_init("no valid super-block found, create empty\n");
        }
        fs_init_empty(fs);
    }
    assert(fs->block_num_size >= fs->dev->block_num_size);
    assert(fs->block_num_size <= sizeof(data_block_t));
    assert(fs->mac_size >= fs->dev->mac_size);
    assert(fs->mac_size <= sizeof(struct mac));
    assert(fs->mac_size == sizeof(struct mac) || fs->dev->tamper_detecting);

    return 0;
}

/**
 * load_super_block - Find and load superblock and initialize file system state
 * @fs:         File system state object.
 * @clear:      If %true, clear fs state if allowed by super block state.
 *
 * Return: 0 if super block was readable and not from a future file system
 * version (regardless of its other content), -1 if not.
 */
static int load_super_block(struct fs *fs, bool clear)
{
    uint i;
    int ret;
    const struct super_block *new_super;
    obj_ref_t new_super_ref = OBJ_REF_INITIAL_VALUE(new_super_ref);
    const struct super_block *old_super = NULL;
    obj_ref_t old_super_ref = OBJ_REF_INITIAL_VALUE(old_super_ref);

    assert(fs->super_dev->block_size >= sizeof(struct super_block));

    for (i = 0; i < countof(fs->super_block); i++) {
        new_super = block_get_super(fs, fs->super_block[i],
                                    &new_super_ref);
        if (!new_super) {
            pr_err("failed to read super-block\n");
            ret = -1; // -EIO ? ERR_IO?;
            goto err;
        }
        if (use_new_super(fs->dev, new_super, i, old_super)) {
            if (old_super) {
                block_put(old_super, &old_super_ref);
            }
            old_super = new_super;
            obj_ref_transfer(&old_super_ref, &new_super_ref);
        } else {
            block_put(new_super, &new_super_ref);
        }
    }

    ret = fs_init_from_super(fs, old_super, clear);
err:
    if (old_super) {
        block_put(old_super, &old_super_ref);
    }
    return ret;
}

/**
 * fs_init - Initialize file system state
 * @fs:         File system state object.
 * @key:        Key pointer. Must not be freed while @fs is in use.
 * @dev:        Main block device.
 * @super_dev:  Block device for super block.
 * @clear:      If %true, clear fs state if allowed by super block state.
 */
int fs_init(struct fs *fs,
            const struct key *key,
            struct block_device *dev,
            struct block_device *super_dev,
            bool clear)
{
    int ret;

    if (super_dev->block_size < sizeof(struct super_block)) {
        pr_err("unsupported block size for super_dev, %zd < %zd\n",
               super_dev->block_size, sizeof(struct super_block));
        return -1; //ERR_NOT_VALID?
    }

    if (super_dev->block_count < 2) {
        pr_err("unsupported block count for super_dev, %lld\n",
               super_dev->block_count);
        return -1; //ERR_NOT_VALID?
    }

    fs->key = key;
    fs->dev = dev;
    fs->super_dev = super_dev;
    list_initialize(&fs->transactions);
    list_initialize(&fs->allocated);

    if (dev == super_dev) {
        fs->min_block_num = 2;
    } else {
        fs->min_block_num = 1; /* TODO: use 0 when btree code allows it */;
    }
    fs->super_block[0] = 0;
    fs->super_block[1] = 1;
    ret = load_super_block(fs, clear);
    if (ret) {
        fs->dev = NULL;
        fs->super_dev = NULL;
        return ret;
    }

    return 0;
}
