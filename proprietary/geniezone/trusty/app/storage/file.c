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
#include "block_map.h"
#include "crypt.h"
#include "debug.h"
#include "file.h"
#include "transaction.h"

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BIT_MASK(x) (((x) >= sizeof(unsigned long long ) * 8) ? (0ULL-1) : ((1ULL << (x)) - 1))

#define FILE_ENTRY_MAGIC (0x0066797473757274) /* trustyf\0 */

/**
 * struct file_entry - On-disk file entry
 * @iv:         initial value used for encrypt/decrypt
 * @magic:      FILE_ENTRY_MAGIC.
 * @block_map:  Block and mac of file block map root node.
 * @size:       File size in bytes.
 * @reserved:   Reserved for future use. Write 0, read ignore.
 * @path:       File path and name.
 */
struct file_entry {
    struct iv iv;
    uint64_t magic;
    struct block_mac block_map;
    data_block_t size;
    uint64_t reserved;
    char path[FS_PATH_MAX];
};

static bool file_tree_lookup(struct block_mac *block_mac_out,
                             struct transaction *tr,
                             struct block_tree *tree,
                             struct block_tree_path *tree_path,
                             const char *file_path,
                             bool remove);

/**
 * path_hash - Convert a path string to hash value
 * @tr:         Transaction object.
 * @path:       String to calculate hash for.
 *
 * Return: non-zero hash value that fits in @tr->fs->block_num_size bytes
 * computed from @path
 */
static data_block_t path_hash(struct transaction *tr, const char *path)
{
    data_block_t hash = str_hash(path);

    hash &= BIT_MASK(tr->fs->block_num_size * 8);
    if (!hash) {
        hash++;  /* 0 key is not supported by block_tree */
    }

    return hash;
}

/**
 * file_block_map_init - Initialize in-memory block map state from file entry
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @file:       Block number and mac of file entry.
 *
 * Load file entry at @file and initialize @block_map with root node stored
 * there.
 */
void file_block_map_init(struct transaction *tr,
                         struct block_map *block_map,
                         const struct block_mac *file)
{
    const struct file_entry *file_entry_ro;
    obj_ref_t file_entry_ro_ref = OBJ_REF_INITIAL_VALUE(file_entry_ro_ref);

    if (!block_mac_valid(tr, file)) {
        goto err;
    }
    file_entry_ro = block_get(tr, file, NULL, &file_entry_ro_ref);
    if (!file_entry_ro) {
        goto err;
    }
    assert(file_entry_ro);
    block_map_init(tr, block_map, &file_entry_ro->block_map,
                   tr->fs->dev->block_size);
    block_put(file_entry_ro, &file_entry_ro_ref);

    return;

err:
    if (tr->failed) {
        pr_warn("can't read file entry at %lld, transaction failed\n",
                block_mac_to_block(tr, file));
    } else {
        pr_err("can't read file entry at %lld\n", block_mac_to_block(tr, file));
        transaction_fail(tr);
    }
}

/**
 * file_print - Print information about a file
 * @tr:         Transaction object.
 * @file:       File handle object.
 */
void file_print(struct transaction *tr, const struct file_handle *file)
{
    const struct file_entry *file_entry_ro;
    obj_ref_t file_entry_ro_ref = OBJ_REF_INITIAL_VALUE(file_entry_ro_ref);
    struct block_map block_map;

    file_block_map_init(tr, &block_map, &file->block_mac);
    file_entry_ro = block_get(tr, &file->block_mac, NULL, &file_entry_ro_ref);
    if (!file_entry_ro) {
        printf("can't read file entry at %lld\n",
               block_mac_to_block(tr, &file->block_mac));
        return;
    }
    assert(file_entry_ro);
    printf("file at %lld, path %s, size %lld, hash 0x%llx, block map (tree)\n",
           block_mac_to_block(tr, &file->block_mac), file_entry_ro->path,
           file_entry_ro->size, path_hash(tr, file_entry_ro->path));
    block_put(file_entry_ro, &file_entry_ro_ref);
    block_tree_print(tr, &block_map.tree);
}

/**
 * files_print - Print information about all committed files
 * @tr:         Transaction object.
 */
void files_print(struct transaction *tr)
{
    struct block_tree_path path;
    struct file_handle file;

    printf("%s: files:\n", __func__);
    block_tree_print(tr, &tr->fs->files);

    block_tree_walk(tr, &tr->fs->files, 0, true, &path);
    while (block_tree_path_get_key(&path)) {
        file.block_mac = block_tree_path_get_data_block_mac(&path);
        file_print(tr, &file);
        block_tree_path_next(&path);
    }
}

/**
 * file_block_map_update - Update file entry with block_map or size changes
 * @tr:         Transaction object.
 * @block_map:  Block map object.
 * @file:       File handle object.
 *
 */
static void file_block_map_update(struct transaction *tr,
                                  struct block_map *block_map,
                                  struct file_handle *file)
{
    bool found;
    data_block_t old_block;
    data_block_t file_block;
    data_block_t new_block;
    struct block_mac block_mac;
    const struct file_entry *file_entry_ro;
    struct file_entry *file_entry_rw = NULL;
    obj_ref_t file_entry_old_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);
    obj_ref_t file_entry_copy_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);
    obj_ref_t *file_entry_ref = &file_entry_old_ref;
    struct block_tree_path tree_path;

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        return;
    }

    assert(block_mac_valid(tr, &file->block_mac));

    file_entry_ro = block_get(tr, &file->block_mac, NULL, file_entry_ref);
    if (!file_entry_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        return;
    }
    assert(file_entry_ro);
    found = file_tree_lookup(&block_mac, tr, &tr->files_added, &tree_path,
                             file_entry_ro->path, false);
    if (!found) {
        found = file_tree_lookup(&block_mac, tr, &tr->fs->files, &tree_path,
                                 file_entry_ro->path, false);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err;
        }
        assert(found);
        old_block = block_mac_to_block(tr, &block_mac);
        file_block = block_mac_to_block(tr, &file->block_mac);
        if (transaction_block_need_copy(tr, file_block)) {
            if (tr->failed) {
                pr_warn("transaction failed, abort\n");
                goto err;
            }
            assert(block_map->tree.root_block_changed || file->size != file_entry_ro->size);
            assert(old_block == file_block);
            new_block = block_allocate(tr);
            if (tr->failed) {
                pr_warn("transaction failed, abort\n");
                goto err;
            }
            assert(new_block);
            block_mac_set_block(tr, &block_mac, new_block);

            pr_write("copy file block %lld -> %lld\n", file_block, new_block);

            /*
             * Use block_get_copy instead of block_move since tr->fs->files
             * is still used and not updated until file_transaction_complete.
             */
            file_entry_rw = block_get_copy(tr, file_entry_ro, new_block,
                                           false, &file_entry_copy_ref); // TODO: fix
            block_put(file_entry_ro, file_entry_ref);
            file_entry_ro = file_entry_rw;
            file_entry_ref = &file_entry_copy_ref;
            if (tr->failed) {
                pr_warn("transaction failed, abort\n");
                goto err;
            }
            block_tree_insert(tr, &tr->files_updated, file_block, new_block); /* TODO: insert mac */
            block_free(tr, file_block);
            file->block_mac = block_mac;
        }
        assert(!file_tree_lookup(NULL, tr, &tr->files_added, &tree_path,
                                 file_entry_ro->path, false));

        block_tree_walk(tr, &tr->files_updated, old_block, false, &tree_path); /* TODO: get path from insert operation */
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err;
        }
        assert(block_tree_path_get_key(&tree_path) == old_block);
        block_mac = block_tree_path_get_data_block_mac(&tree_path);
        assert(block_mac_to_block(tr, &block_mac) == block_mac_to_block(tr, &file->block_mac));
        //assert(block_mac_eq(tr, &block_mac, &file->block_mac)); /* TODO: enable after insert mac TODO */
    }
    if (!file_entry_rw) {
        file_entry_rw = block_dirty(tr, file_entry_ro, false);
    }

    pr_write("file at %lld: update block_map %lld -> %lld\n",
             block_mac_to_block(tr, &block_mac),
             block_mac_to_block(tr, &file_entry_rw->block_map),
             block_mac_to_block(tr, &block_map->tree.root));

    file_entry_rw->block_map = block_map->tree.root;
    file_entry_rw->size = file->size;
    block_tree_path_put_dirty(tr, &tree_path, tree_path.count,
                              file_entry_rw, file_entry_ref);
    file->block_mac = tree_path.entry[tree_path.count].block_mac; /* TODO: add better api */

    /*
     * Move to head of list so opening the same file twice in a transaction
     * commits the changes to the file_handle that was modified. Writing to
     * both handles is not currently supported.
     */
    list_delete(&file->node);
    list_add_head(&tr->open_files, &file->node);

    return;

err:
    if (file_entry_rw) {
        block_put_dirty_discard(file_entry_rw, file_entry_ref);
    } else {
        block_put(file_entry_ro, file_entry_ref);
    }
}

/**
 * get_file_block_size - Get file data block size
 * @fs:         File system state object.
 *
 * Return: Get size of data block returned by file_get_block and
 * file_get_block_write for @fs.
 */
size_t get_file_block_size(struct fs *fs)
{
    return fs->dev->block_size - sizeof(struct iv);
}

/**
 * file_get_block_etc - Helper function to get a file block for read or write
 * @tr:         Transaction object.
 * @file:       File handle object.
 * @file_block: File block number. 0 based.
 * @read:       %true if data should be read from disk, %false otherwise.
 * @write:      %true if returned data should be writeable, %false otherwise.
 * @ref:        Pointer to store reference in.
 *
 * Return: Const block data pointer, or NULL if no valid block is found at
 * @file_block or if @write is %true and a block could not be allocated.
 */
static const void *file_get_block_etc(struct transaction *tr,
                                      struct file_handle *file,
                                      data_block_t file_block,
                                      bool read,
                                      bool write,
                                      obj_ref_t *ref)
{
    bool found = false;
    const void *data = NULL;
    struct block_map block_map;
    struct block_mac block_mac;
    data_block_t old_disk_block;
    data_block_t new_block;
    bool dirty = false;

    if (tr->failed) {
        pr_warn("transaction failed, ignore\n");
        goto err;
    }

    file_block_map_init(tr, &block_map, &file->block_mac);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }

    file->used_by_tr = true;

    found = block_map_get(tr, &block_map, file_block, &block_mac);
    if (found) {
        if (read) {
            data = block_get(tr, &block_mac, NULL, ref); /* TODO: pass iv? */
        } else {
            data = block_get_no_read(tr, block_mac_to_block(tr, &block_mac), ref);
        }
        /* TODO: handle mac mismatch better */
    }

    old_disk_block = found ? block_mac_to_block(tr, &block_mac) : 0;
    if (write && (!found || transaction_block_need_copy(tr, old_disk_block))) {
        new_block = block_allocate(tr);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err;
        }
        assert(new_block);
        block_mac_set_block(tr, &block_mac, new_block);

        if (found) {
            data = block_move(tr, data, new_block, false);
            dirty = true;
            assert(!tr->failed);
            block_free(tr, old_disk_block);
        } else {
            if (read) {
                assert(write);
                data = block_get_cleared(tr, new_block, false, ref);
                dirty = true;
            } else {
                data = block_get_no_read(tr, new_block, ref);
            }
        }
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err;
        }
        /* TODO: get new mac */
        assert(!tr->failed);
        block_map_set(tr, &block_map, file_block, &block_mac);
        file_block_map_update(tr, &block_map, file);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err;
        }
    }
    if (write && !dirty) {
        data = block_dirty(tr, data, false);
    }
    if (!data) {
        return NULL;
    }
    return data + sizeof(struct iv);

err:
    if (dirty) {
        block_put_dirty_discard((void *)data, ref);
    } else if (data) {
        block_put(data, ref);
    }
    return NULL;
}

/**
 * file_get_block - Get a file block for read
 * @tr:         Transaction object.
 * @file:       File handle object.
 * @file_block: File block number. 0 based.
 * @ref:        Pointer to store reference in.
 *
 * Return: Const block data pointer, or NULL if no valid block is found at
 * @file_block.
 */
const void *file_get_block(struct transaction *tr, struct file_handle *file,
                           data_block_t file_block, obj_ref_t *ref)
{
    return file_get_block_etc(tr, file, file_block, true, false, ref);
}

/**
 * file_get_block_write - Helper function to get a file block for write
 * @tr:         Transaction object.
 * @file:       File handle object.
 * @file_block: File block number. 0 based.
 * @read:       %true if data should be read from disk, %false otherwise.
 * @ref:        Pointer to store reference in.
 *
 * Return: Block data pointer, or NULL if  a block could not be allocated.
 */
void *file_get_block_write(struct transaction *tr, struct file_handle *file,
                           data_block_t file_block, bool read, obj_ref_t *ref)
{
     return (void *)file_get_block_etc(tr, file, file_block, read, true, ref);
}

/**
 * file_block_put - Release reference to a block returned by file_get_block
 * @data:       File block data pointer
 * @data_ref:   Reference pointer to release
 */
void file_block_put(const void *data, obj_ref_t *data_ref)
{
    block_put(data - sizeof(struct iv), data_ref);
}

/**
 * file_block_put_dirty - Release reference to a dirty file block
 * @tr:         Transaction
 * @file:       File handle object.
 * @file_block: File block number. 0 based.
 * @data:       File block data pointer
 * @data_ref:   Reference pointer to release
 *
 * Release reference to a file block previously returned by
 * file_get_block_write, and update mac value in block_mac.
 */
void file_block_put_dirty(struct transaction *tr,
                          struct file_handle *file, data_block_t file_block,
                          void *data, obj_ref_t *data_ref)
{
    struct block_map block_map;

    file_block_map_init(tr, &block_map, &file->block_mac);

    block_map_put_dirty(tr, &block_map, file_block,
                        data - sizeof(struct iv), data_ref);

    file_block_map_update(tr, &block_map, file);
}

/**
 * file_get_size - Get file size
 * @tr:         Transaction
 * @file:       File handle object.
 * @size:       Pointer to return size in.
 *
 * Return: %true if @size was set, %false if @size was not set because @file is
 * invalid or @tr has failed.
 */
bool file_get_size(struct transaction *tr,
                   struct file_handle *file,
                   data_block_t *size)
{
    if (tr->failed) {
        pr_warn("transaction failed, ignore\n");
        return false;
    }

    if (!block_mac_valid(tr, &file->block_mac)) {
        pr_warn("invalid file handle\n");
        return false;
    }

    file->used_by_tr = true;
    *size = file->size;

    return true;
}

/**
 * file_set_size - Set file size
 * @tr:         Transaction
 * @file:       File handle object.
 * @size:       New file size.
 *
 * Set file size and free blocks that are not longer needed. Does not clear any
 * partial block data.
 */
void file_set_size(struct transaction *tr,
                   struct file_handle *file,
                   data_block_t size)
{
    data_block_t file_block;
    struct block_map block_map;
    size_t file_block_size = get_file_block_size(tr->fs);

    if (tr->failed) {
        pr_warn("transaction failed, ignore\n");
        return;
    }

    file_block_map_init(tr, &block_map, &file->block_mac);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        return;
    }
    if (size == file->size) {
        return;
    }
    if (size < file->size) {
        file_block = DIV_ROUND_UP(size, file_block_size);
        block_map_truncate(tr, &block_map, file_block);
    }
    file->size = size;
    file_block_map_update(tr, &block_map, file);
}

/**
 * file_tree_lookup - Helper function to search for a file in a specific tree
 * @block_mac_out:  Block-mac object to return block number and mac in.
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @tree_path:      Tree path object.
 * @file_path:      File path string.
 * @remove:         %true if found entry should be removed, %false otherwise.
 *
 * Return: %true if @file_path was found in @tree, %false otherwise.
 */
static bool file_tree_lookup(struct block_mac *block_mac_out,
                             struct transaction *tr,
                             struct block_tree *tree,
                             struct block_tree_path *tree_path,
                             const char *file_path,
                             bool remove)
{
    struct block_mac block_mac;
    data_block_t hash = path_hash(tr, file_path);
    const struct file_entry *file_entry;
    obj_ref_t file_entry_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);
    bool found = false;

    assert(strlen(file_path) < sizeof(file_entry->path));
    assert(sizeof(*file_entry) <= tr->fs->dev->block_size);

    block_tree_walk(tr, tree, hash - 1, false, tree_path);
    while (block_tree_path_get_key(tree_path) && block_tree_path_get_key(tree_path) < hash) {
        block_tree_path_next(tree_path);
    }
    while (block_tree_path_get_key(tree_path) == hash) {
        block_mac = block_tree_path_get_data_block_mac(tree_path);
        if (!block_mac_to_block(tr, &block_mac)) {
            if (LOCAL_TRACE >= TRACE_LEVEL_WARNING) {
                pr_warn("got 0 block pointer for hash %lld while looking for %s, hash 0x%llx\n",
                        block_tree_path_get_key(tree_path), file_path, hash);
                block_tree_print(tr, tree);
            }
            block_tree_path_next(tree_path);
            block_mac = block_tree_path_get_data_block_mac(tree_path);
            pr_warn("next %lld, hash 0x%llx\n",
                    block_mac_to_block(tr, &block_mac),
                    block_tree_path_get_key(tree_path));
        }
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return false;
        }
        assert(block_mac_to_block(tr, &block_mac));
        file_entry = block_get(tr, &block_mac, NULL, &file_entry_ref);
        if (!file_entry) {
            assert(tr->failed);
            pr_warn("transaction failed, abort\n");
            return false;
        }
        assert(file_entry);
        assert(file_entry->magic == FILE_ENTRY_MAGIC);
        found = !strcmp(file_path, file_entry->path);

        pr_read("block %lld, hash 0x%llx match, found %d\n",
                block_mac_to_block(tr, &block_mac), hash, found);

        block_put(file_entry, &file_entry_ref);
        if (found) {
            if (remove) {
                //block_tree_remove_path(tr, tree_path); /* TODO: remove by path */
                block_tree_remove(tr, tree, hash, block_mac_to_block(tr, &block_mac)); /* TODO: pass mac */
            }
            *block_mac_out = block_mac;
            return true;
        }
        block_tree_path_next(tree_path);
    }
    if (LOCAL_TRACE >= TRACE_LEVEL_READ) {
        pr_read("%s: hash 0x%llx does not match 0x%llx\n",
                __func__, hash, block_tree_path_get_key(tree_path));
        block_tree_print(tr, tree);
    }
    return false;
}

/**
 * file_create - Helper function to create a new file
 * @block_mac_out:  Block-mac object to return block number and mac in.
 * @tr:             Transaction object.
 * @path:           File path string.
 *
 * Create a new file and insert it into &tr->files_added. Caller must make
 * a file matching @path does not already exist.
 *
 * Return: %true if file was created, %false otherwise.
 */
static bool file_create(struct block_mac *block_mac_out,
                        struct transaction *tr,
                        const char *path)
{
    data_block_t block;
    struct file_entry *file_entry;
    obj_ref_t file_entry_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);
    data_block_t hash;

    hash = path_hash(tr, path);

    block = block_allocate(tr);

    pr_write("create file, %s, hash 0x%llx, at block %lld\n",
             path, hash, block);

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }

    assert(block);
    block_mac_set_block(tr, block_mac_out, block);

    file_entry = block_get_cleared(tr, block, false, &file_entry_ref);
    assert(file_entry);
    file_entry->magic = FILE_ENTRY_MAGIC;
    strcpy(file_entry->path, path);
    block_put_dirty(tr, file_entry, &file_entry_ref, block_mac_out, NULL);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    block_tree_insert_block_mac(tr, &tr->files_added, hash, *block_mac_out);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    return true;

err:
    return false;
}

/**
 * file_is_removed - Helper function to check if transaction deletes file
 * @tr:         Transaction object.
 * @block:      Block number for file.
 *
 * Return: %true if file is removed in @tr, %false otherwise.
 */
static bool file_is_removed(struct transaction *tr, data_block_t block)
{
    struct block_tree_path tree_path;

    block_tree_walk(tr, &tr->files_removed, block, false, &tree_path);

    return block_tree_path_get_key(&tree_path) == block;
}

/**
 * file_lookup_not_removed - Helper function to search for a file
 * @block_mac_out:              Block-mac object to return block number and mac
 *                              in for the current state of the file.
 * @committed_block_mac_out:    Block-mac object to return block number and mac
 *                              in for the committed state of the file.
 * @tr:                         Transaction object.
 * @tree_path:                  Tree path object.
 * @file_path:                  File path string.
 *
 * Helper function to search for a file that existed before @tr was activated.
 *
 * Return: %true if @file_path was found in @tr->fs->files but not found in
 * @tr->files_removed, %false otherwise.
 */
static bool file_lookup_not_removed(struct block_mac *block_mac_out,
                                    struct block_mac *committed_block_mac_out,
                                    struct transaction *tr,
                                    struct block_tree_path *tree_path,
                                    const char *file_path)
{
    bool found;
    struct block_mac block_mac;

    found = file_tree_lookup(&block_mac, tr, &tr->fs->files, tree_path,
                             file_path, false);
    if (!found || file_is_removed(tr, block_mac_to_block(tr, &block_mac))) {
        if (found) {
            pr_read("file %s, %lld in removed\n",
                    file_path, block_mac_to_block(tr, &block_mac));
        } else {
            pr_read("file %s not in tr->fs->files\n", file_path);
        }
        return false;
    }
    *committed_block_mac_out = block_mac;

    block_tree_walk(tr, &tr->files_updated, block_mac_to_block(tr, &block_mac),
                    false, tree_path);
    if (block_tree_path_get_key(tree_path) == block_mac_to_block(tr, &block_mac)) {
        pr_read("file %s, %lld, already updated in this transaction, use new copy %lld\n",
                file_path, block_mac_to_block(tr, &block_mac),
                block_tree_path_get_data(tree_path));
        block_mac = block_tree_path_get_data_block_mac(tree_path);
    }
    pr_read("file %s, %lld, found in tr->fs->files\n",
            file_path, block_mac_to_block(tr, &block_mac));

    /*
     * TODO: mark file (or blocks) in_use so other writers to the same file
     * will cause this transaction to fail. Currently conflicts are only
     * detected after each transaction writes to the file.
     */

    *block_mac_out = block_mac;
    return true;
}

/**
 * file_find_open - Check if transaction has file open and return it
 * @tr:         Transaction object.
 * @block_mac:  Current block and mac value for open file.
 *
 * Return: file handle of open file if file is open in @tr, NULL otherwise.
 */
static struct file_handle *file_find_open(struct transaction *tr,
                                          const struct block_mac *block_mac)
{
    struct file_handle *file;

    list_for_every_entry(&tr->open_files, file, struct file_handle, node) {
        if (block_mac_same_block(tr, &file->block_mac, block_mac)) {
            return file;
        }
    }

    return NULL;
}

/**
 * file_open - Open file
 * @tr:         Transaction object.
 * @path:       Path to find or create file at.
 * @file:       File handle object.
 * @create:     FILE_OPEN_NO_CREATE, FILE_OPEN_CREATE or
 *              FILE_OPEN_CREATE_EXCLUSIVE.
 *
 * Return: %true if file was opened, %false if file could not be opeened.
 */
bool file_open(struct transaction *tr, const char *path,
               struct file_handle *file, enum file_create_mode create)
{
    bool found;
    struct block_mac block_mac;
    struct block_mac committed_block_mac = BLOCK_MAC_INITIAL_VALUE(committed_block_mac);
    struct block_tree_path tree_path;
    const struct file_entry *file_entry_ro;
    obj_ref_t file_entry_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);

    assert(tr->fs);

    found = file_tree_lookup(&block_mac, tr, &tr->files_added, &tree_path,
                             path, false);
    if (found) {
        goto found;
    }

    found = file_lookup_not_removed(&block_mac, &committed_block_mac,
                                    tr, &tree_path, path);
    if (found) {
        goto found;
    }

    if (create != FILE_OPEN_NO_CREATE) {
        found = file_create(&block_mac, tr, path);
    }
    if (found) {
        goto created;
    }
    return false;

found:
    if (create == FILE_OPEN_CREATE_EXCLUSIVE) {
        return false;
    }
    if (file_find_open(tr, &block_mac)) {
        pr_warn("%s already open\n", path);
        return false;
    }
created:
    file_entry_ro = block_get(tr, &block_mac, NULL, &file_entry_ref);
    if (!file_entry_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        return false;
    }
    assert(file_entry_ro);
    list_add_head(&tr->open_files, &file->node);
    file->to_commit_block_mac = committed_block_mac;
    file->committed_block_mac = committed_block_mac;
    file->block_mac = block_mac;
    file->size = file_entry_ro->size;
    file->used_by_tr = false;
    block_put(file_entry_ro, &file_entry_ref);

    return true;
}

/**
 * file_close - Close file
 * @file:       File handle object.
 *
 * Must be called before freeing @file after a successful file_open call.
 */
void file_close(struct file_handle *file)
{
    list_delete(&file->node);
}

/**
 * file_delete - Delete file
 * @tr:         Transaction object.
 * @path:       Path to find file at.
 *
 * Return: %true if file was found, %false otherwise.
 */
bool file_delete(struct transaction *tr, const char *path)
{
    bool found;
    struct block_mac block_mac = {0};
    struct block_mac old_block_mac = {0};
    bool in_files = false;
    const struct file_entry *file_entry;
    obj_ref_t file_entry_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);
    struct block_map block_map = BLOCK_MAP_INITIAL_VALUE(block_map);
    struct block_tree_path tree_path;
    struct file_handle *open_file;

    found = file_tree_lookup(&block_mac, tr, &tr->files_added, &tree_path,
                             path, true);
    if (!found) {
        pr_read("file %s not in tr->files_added\n", path);
        found = file_lookup_not_removed(&block_mac, &old_block_mac,
                                        tr, &tree_path, path);
        if (!found) {
            pr_warn("file %s not found\n", path);
            return false;
        }
        in_files = true;
    }

    pr_write("delete file, %s, at block %lld\n",
             path, block_mac_to_block(tr, &block_mac));

    file_entry = block_get(tr, &block_mac, NULL, &file_entry_ref);
    if (!file_entry) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        return false;
    }
    assert(file_entry);
    assert(!strcmp(file_entry->path, path));
    block_map_init(tr, &block_map, &file_entry->block_map,
                   tr->fs->dev->block_size);
    if (!in_files || !block_mac_same_block(tr, &block_mac, &old_block_mac)) {
        block_discard_dirty(file_entry);
    }
    block_put(file_entry, &file_entry_ref);
    if (in_files) {
        if (!block_mac_same_block(tr, &block_mac, &old_block_mac)) {
            block_tree_remove(tr, &tr->files_updated,
                              block_mac_to_block(tr, &old_block_mac),
                              block_mac_to_block(tr, &block_mac)); /* TODO: pass mac */
            if (tr->failed) {
                pr_warn("transaction failed, abort\n");
                return false;
            }
        }
        block_tree_insert_block_mac(tr,&tr->files_removed,
                                    block_mac_to_block(tr, &old_block_mac),
                                    old_block_mac);
    }
    block_free(tr, block_mac_to_block(tr, &block_mac));
    block_map_free(tr, &block_map);

    open_file = file_find_open(tr, &block_mac);
    if (open_file) {
        struct block_mac clear = BLOCK_MAC_INITIAL_VALUE(clear);
        open_file->block_mac = clear;
    }
    return true;
}

/**
 * file_for_each_open - Call function for every open file in every transaction
 * @tr:         Transaction object.
 * @func:       Function to call.
 */
static void file_for_each_open(struct transaction *tr,
                               void (*func)(struct transaction *tr,
                                            struct transaction *file_tr,
                                            struct file_handle *file))
{
    struct transaction *tmp_tr;
    struct transaction *other_tr;
    struct file_handle *file;

    list_for_every_entry_safe(&tr->fs->transactions, other_tr, tmp_tr, struct transaction, node) {
        list_for_every_entry(&other_tr->open_files, file, struct file_handle, node) {
            func(tr, other_tr, file);
        }
    }
}

/**
 * file_restore_to_commit - Restore to_commit state to commited state
 * @tr:         Current transaction.
 * @file_tr:    Transaction that @file was found in.
 * @file:       File handle object.
 */
static void file_restore_to_commit(struct transaction *tr,
                                   struct transaction *file_tr,
                                   struct file_handle *file)
{
    struct block_mac *src = &file->committed_block_mac;
    struct block_mac *dest = &file->to_commit_block_mac;
    if (block_mac_same_block(tr, src, dest)) {
        assert(block_mac_eq(tr, src, dest));
        return;
    }
    pr_write("file handle %p, abort block %lld/%lld -> %lld, size %lld -> %lld\n",
             file,
             block_mac_to_block(tr, &file->committed_block_mac),
             block_mac_to_block(tr, &file->block_mac),
             block_mac_to_block(tr, &file->to_commit_block_mac),
             file->size, file->to_commit_size);
    block_mac_copy(tr, dest, src);
}

/**
 * file_apply_to_commit - Apply to_commit state
 * @tr:         Current transaction.
 * @file_tr:    Transaction that @file was found in.
 * @file:       File handle object.
 *
 * Copies to_commit to commited and current state and fails conflicting
 * transactions.
 */
static void file_apply_to_commit(struct transaction *tr,
                                 struct transaction *file_tr,
                                 struct file_handle *file)
{
    struct block_mac *src = &file->to_commit_block_mac;
    struct block_mac *dest = &file->committed_block_mac;

    if (tr == file_tr) {
        file->used_by_tr = false;
    }

    if (block_mac_same_block(tr, src, dest)) {
        assert(block_mac_eq(tr, src, dest));
        pr_write("file handle %p, unchanged file at %lld\n",
                 file, block_mac_to_block(tr, &file->committed_block_mac));
        return;
    }

    if (file_tr != tr) {
        if (file->used_by_tr) {
            pr_warn("file handle %p, conflict %lld != %lld || used_by_tr %d\n",
                    file,
                    block_mac_to_block(tr, &file->committed_block_mac),
                    block_mac_to_block(tr, &file->block_mac),
                    file->used_by_tr);
            assert(!file_tr->failed);
            transaction_fail(file_tr);
        }
        assert(block_mac_same_block(tr, dest, &file->block_mac));
    }

    pr_write("file handle %p, apply block %lld/%lld -> %lld, size %lld -> %lld\n",
             file,
             block_mac_to_block(tr, &file->committed_block_mac),
             block_mac_to_block(tr, &file->block_mac),
             block_mac_to_block(tr, &file->to_commit_block_mac),
             file->size, file->to_commit_size);

    block_mac_copy(tr, dest, src);
    if (tr == file_tr) {
        assert(block_mac_eq(tr, &file->block_mac, src));
        assert(file->size == file->to_commit_size);
    } else {
        block_mac_copy(tr, &file->block_mac, src);
        file->size = file->to_commit_size;
    }
}

/**
 * file_transaction_complete_failed - Restore open files state
 * @tr:                     Transaction object.
 *
 * Revert open file changes done by file_transaction_complete.
 */
void file_transaction_complete_failed(struct transaction *tr)
{
    file_for_each_open(tr, file_restore_to_commit);
}

/**
 * file_update_block_mac_tr - Update open files with committed changes
 * @tr:                 Current transaction object.
 * @other_tr:           Transaction object to find files in.
 * @old_block_mac:      Block and mac of committed file.
 * @old_block_no_mac:   %true if @old_block_mac->max is invalid.
 * @new_block_mac:      New block and mac.
 * @new_size:           New size.
 *
 * Prepare update of open files referring to the file at @old_block_mac.
 */
static void file_update_block_mac_tr(struct transaction *tr,
                                     struct transaction *other_tr,
                                     const struct block_mac *old_block_mac,
                                     bool old_block_no_mac,
                                     const struct block_mac *new_block_mac,
                                     data_block_t new_size)
{
    struct file_handle *file;

    assert(block_mac_valid(tr, old_block_mac) || other_tr == tr);
    list_for_every_entry(&other_tr->open_files, file, struct file_handle, node) {
        if (!block_mac_same_block(tr, &file->committed_block_mac, old_block_mac) ||
            (!block_mac_valid(tr, &file->committed_block_mac) &&
             !block_mac_same_block(tr, &file->block_mac, new_block_mac))) {
            pr_write("file handle %p, unrelated %lld != %lld\n",
                     file, block_mac_to_block(tr, &file->committed_block_mac),
                     block_mac_to_block(tr, new_block_mac));
            continue; /*unrelated file */
        }
        assert(old_block_no_mac ||
               block_mac_eq(tr, &file->committed_block_mac, old_block_mac));

        pr_write("file handle %p, stage block %lld/%lld -> %lld, size %lld -> %lld\n",
                 file,
                 block_mac_to_block(tr, &file->committed_block_mac),
                 block_mac_to_block(tr, &file->block_mac),
                 block_mac_to_block(tr, new_block_mac),
                 file->size, new_size);

        block_mac_copy(tr, &file->to_commit_block_mac, new_block_mac);
        file->to_commit_size = new_size;
    }
}

/**
 * file_update_block_mac_all - Update open files with committed changes
 * @tr:                 Transaction object.
 * @old_block_mac:      Block and mac of committed file.
 * @old_block_no_mac:   %true if @old_block_mac->max is invalid.
 * @new_block_mac:      New block and mac.
 * @new_size:           New size.
 *
 * Update other open files referring to the same file as @src_file with the
 * size, block and mac from @src_file.
 */
static void file_update_block_mac_all(struct transaction *tr,
                                      const struct block_mac *old_block_mac,
                                      bool old_block_no_mac,
                                      const struct block_mac *new_block_mac,
                                      data_block_t new_size)
{
    struct transaction *tmp_tr;
    struct transaction *other_tr;

    list_for_every_entry_safe(&tr->fs->transactions, other_tr, tmp_tr,
                              struct transaction, node) {
        file_update_block_mac_tr(tr, other_tr, old_block_mac, old_block_no_mac,
                                 new_block_mac, new_size);
    }
}

/**
 * file_transaction_complete - Update files
 * @tr:                     Transaction object.
 * @new_files_block_mac:    Object to return block and mac of updated file tree
 *
 * Create a copy of @tr->fs->files and apply all file updates from @tr to it.
 * Called by transaction_complete, which will update the super-block and
 * @tr->fs->files if the transaction succeeds.
 */
void file_transaction_complete(struct transaction *tr,
                               struct block_mac *new_files_block_mac)
{
    bool found;
    const struct file_entry *file_entry_ro;
    obj_ref_t file_entry_ref = OBJ_REF_INITIAL_VALUE(file_entry_ref);
    struct block_tree_path tree_path;
    struct block_tree_path tmp_tree_path;
    struct block_mac old_file;
    struct block_mac file;
    struct block_tree new_files;
    data_block_t hash;
    const struct block_mac clear_block_mac = BLOCK_MAC_INITIAL_VALUE(clear);

    block_tree_copy(&new_files, &tr->fs->files);

    block_tree_walk(tr, &tr->files_updated, 0, true, &tree_path);
    while (true) {
        file = block_tree_path_get_data_block_mac(&tree_path);
        if (!block_mac_to_block(tr, &file)) {
            break;
        }
        file_entry_ro = block_get(tr, &file, NULL, &file_entry_ref);
        if (!file_entry_ro) {
            assert(tr->failed);
            pr_warn("transaction failed, abort\n");
            return;
        }
        assert(file_entry_ro);
        block_mac_set_block(tr, &old_file, block_tree_path_get_key(&tree_path));

        pr_write("update file at %lld -> %lld, %s\n",
                 block_mac_to_block(tr, &old_file),
                 block_mac_to_block(tr, &file), file_entry_ro->path);

        file_update_block_mac_all(tr, &old_file, true,
                                  &file, file_entry_ro->size);

        hash = path_hash(tr, file_entry_ro->path);

        block_put(file_entry_ro, &file_entry_ref);

        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }

        block_tree_update_block_mac(tr, &new_files,
                                    hash, old_file,
                                    hash, file);

        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }

        assert(block_mac_to_block(tr, &old_file) != block_mac_to_block(tr, &file));
        block_tree_path_next(&tree_path);
    }

    block_tree_walk(tr, &tr->files_removed, 0, true, &tree_path);
    while (true) {
        file = block_tree_path_get_data_block_mac(&tree_path);
        if (!block_mac_to_block(tr, &file)) {
            break;
        }
        file_entry_ro = block_get(tr, &file, NULL, &file_entry_ref);
        if (!file_entry_ro) {
            assert(tr->failed);
            pr_warn("transaction failed, abort\n");
            return;
        }
        assert(file_entry_ro);

        pr_write("delete file at %lld, %s\n",
                 block_mac_to_block(tr, &file), file_entry_ro->path);

        file_update_block_mac_all(tr, &file, false, &clear_block_mac, 0);

        found = file_tree_lookup(&old_file, tr, &new_files,
                                 &tmp_tree_path, file_entry_ro->path, true);
        block_put(file_entry_ro, &file_entry_ref);

        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }

        assert(found);
        assert(block_mac_to_block(tr, &old_file) == block_mac_to_block(tr, &file));
        block_tree_path_next(&tree_path);
    }
    block_tree_walk(tr, &tr->files_added, 0, true, &tree_path);
    while (true) {
        file = block_tree_path_get_data_block_mac(&tree_path);
        if (!block_mac_valid(tr, &file)) {
            break;
        }
        file_entry_ro = block_get(tr, &file, NULL, &file_entry_ref);
        if (!file_entry_ro) {
            assert(tr->failed);
            pr_warn("transaction failed, abort\n");
            return;
        }
        assert(file_entry_ro);
        pr_write("add file at %lld, %s\n",
                 block_mac_to_block(tr, &file), file_entry_ro->path);

        if (file_tree_lookup(&old_file, tr, &new_files, &tmp_tree_path,
                             file_entry_ro->path, false)) {
            block_put(file_entry_ro, &file_entry_ref);
            pr_err("add file at %lld, %s, failed, conflicts with %lld\n",
                   block_mac_to_block(tr, &file),
                   file_entry_ro->path, block_mac_to_block(tr, &old_file));
            transaction_fail(tr);
            return;
        }

        file_update_block_mac_tr(tr, tr, &clear_block_mac, false,
                                 &file, file_entry_ro->size);

        hash = path_hash(tr, file_entry_ro->path);
        block_put(file_entry_ro, &file_entry_ref);

        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }

        block_tree_insert_block_mac(tr, &new_files, hash, file);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }

        block_tree_path_next(&tree_path);
    }
    *new_files_block_mac = new_files.root;
}

/**
 * transaction_changed_file - Check if file was modified by current transaction
 * @tr:         Transaction object.
 * @file:       File handle.
 *
 * Return: %true if @file was modified by @tr, %false otherwise.
 */
static bool transaction_changed_file(struct transaction *tr,
                                     struct file_handle *file)
{
    return !block_mac_same_block(tr, &file->committed_block_mac, &file->block_mac);
}

/**
 * file_transaction_success - Update all file handles modified by transaction
 * @tr:         Transaction object.
 */
void file_transaction_success(struct transaction *tr)
{
    file_for_each_open(tr, file_apply_to_commit);
}

/**
 * file_read_size - Read file entry to get file size
 * @tr:         Transaction object.
 * @block_mac:  Block and mac of file entry to read.
 * @sz:         Pointer to store file size in.
 *
 * Return: %true if @sz was set, %false otherwise.
 */
static bool file_read_size(struct transaction *tr,
                           const struct block_mac *block_mac,
                           data_block_t *sz)
{
    const struct file_entry *file_entry_ro;
    obj_ref_t ref = OBJ_REF_INITIAL_VALUE(ref);

    if (!block_mac_valid(tr, block_mac)) {
         *sz = 0;
         return true;
    }

    file_entry_ro = block_get_no_tr_fail(tr, block_mac, NULL, &ref);
    if (!file_entry_ro) {
        return false;
    }
    *sz = file_entry_ro->size;
    block_put(file_entry_ro, &ref);
    return true;
}

/**
 * file_transaction_failed - Restore file handles after failed transaction
 * @tr:         Transaction object.
 *
 * Revert all file handles modified by @tr to the state of the files before @tr
 * was activeted. File handles for files crreated by @tr become invalid.
 */
void file_transaction_failed(struct transaction *tr)
{
    struct file_handle *file;
    bool success;

    list_for_every_entry(&tr->open_files, file, struct file_handle, node) {
        file->used_by_tr = false;
        if (transaction_changed_file(tr, file)) {
            file->block_mac = file->committed_block_mac;
            success = file_read_size(tr, &file->block_mac, &file->size);
            if (!success) {
                pr_warn("failed to read block %lld, clear file handle\n",
                        block_mac_to_block(tr, &file->block_mac));
                block_mac_clear(tr, &file->block_mac);
                block_mac_clear(tr, &file->committed_block_mac);
                file->size = 0;
            }
        }
    }
}
