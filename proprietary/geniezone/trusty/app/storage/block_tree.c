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
#include <compiler.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "block_allocator.h"
#include "block_cache.h"
#include "block_tree.h"
#include "crypt.h"
#include "debug.h"
#include "transaction.h"

#if BUILD_STORAGE_TEST
#include <math.h>
#endif

bool print_lookup;
static bool print_ops;
static bool print_node_changes;
static bool print_internal_changes;
static bool print_changes;
static bool print_changes_full_tree;

/* TODO: move to obj_ref lib */
static void obj_ref_transfer(obj_ref_t *dst, obj_ref_t *src)
{
    struct list_node *prev = src->ref_node.prev;
    list_delete(&src->ref_node);
    list_add_after(prev, &dst->ref_node);
}

/**
 * struct block_tree_node - On-disk B+ tree node header and payload start
 * @iv:         initial value used for encrypt/decrypt
 * @is_leaf:    0 for internal nodes, 1 for leaf nodes.
 * @data:       Key, child and data payload, size of entries is tree specific
 *              so accessor functions are needed to get to specific entries.
 *
 * On-disk B+ tree node format.
 *
 * If %is_leaf is 0, the node is an internal B+ tree node and @data contains n
 * keys and n + 1 child block-macs. If %is_leaf is 1, the node is a leaf node
 * and @data contains n keys and n data entries. For either node type
 * n <= block_tree_max_key_count. For root leaf nodes n >= 0, for root internal
 * nodes n >= 1, otherwise n >= block_tree_min_key_count. While updating the
 * tree, n can temporarily exceed these limits by 1 for a single node. There is
 * no room in this struct to store extra keys, so the extra key and child/data
 * is stored in the in-memory tree struct.
 *
 * The value of block_tree_max_key_count and block_tree_min_key_count depends
 * on the block, key, child and data sizes for the tree, and may not be the
 * same for leaf and internal nodes.
 *
 * Keys are stored starting at @data, child and data values are stored starting
 * at data + block_tree_max_key_count * key-size.
 *
 * TODO: store n in this struct? Currently a 0 key value us used to determine
 * how full a node is, which prevents storing 0 keys in the tree.
 */
struct block_tree_node {
    struct iv iv;
    uint64_t is_leaf;
    uint8_t data[0];
};

/**
 * block_tree_node_is_leaf - Check if node is a leaf or an internal node
 * @node_ro:    Node pointer.
 *
 * Return: %true if @node_ro is a leaf node, %false if @node_ro is an internal
 * node.
 */
static bool block_tree_node_is_leaf(const struct block_tree_node *node_ro)
{
    assert(node_ro);
    assert(node_ro->is_leaf <= 1);

    return node_ro->is_leaf;
}

/**
 * block_tree_set_sizes - Configure tree block and entry sizes
 * @tree:           Tree object.
 * @block_size:     Block size.
 * @key_size:       Key size. [1-8].
 * @child_size:     Child size. [@key_size-24].
 * @data_size:      Data size. [1-24].
 *
 * Store tree block and entry sizes and calculate key counts.
 */
static void block_tree_set_sizes(struct block_tree *tree,
                                 size_t block_size,
                                 size_t key_size,
                                 size_t child_size,
                                 size_t data_size)
{
    size_t payload_size = block_size - sizeof(struct block_tree_node);

    assert(payload_size < block_size);
    assert(key_size);
    assert(key_size <= sizeof(data_block_t));
    assert(child_size >= key_size);
    assert(child_size <= sizeof(struct block_mac));
    assert(data_size);
    assert(data_size <= sizeof(struct block_mac));

    tree->key_size = key_size;
    tree->child_data_size[0] = child_size;
    tree->child_data_size[1] = data_size;
    tree->key_count[0] = (payload_size - child_size) / (key_size + child_size);
    tree->key_count[1] = (payload_size) / (key_size + data_size); /* TODO: allow next pointer when mac is not needed? */
    tree->block_size = block_size;

    assert(tree->key_count[0] >= 2);
    assert(tree->key_count[1] >= 2);
}

/**
 * is_zero - Helper function to check that buffer only contain 0 bytes
 * @data:       Data pointer.
 * @size:       Number of bytes to check.
 *
 * Return: %true if @size is 0 or @data[0..@size - 1] is all 0, %false
 * otherwise.
 */
static bool is_zero(const void *data, size_t size)
{
    if (!size) {
        return true;
    }
    assert(size >= 1);
    return !*(char *)data && !memcmp(data, data + 1, size - 1);
}

/**
 * block_tree_max_key_count - Return max key count for leaf or internal nodes
 * @tree:       Tree object.
 * @leaf:       %true for leaf node, %false for internal node.
 *
 * Return: Maximum number of keys that fit in a leaf node or in and internal
 * node.
 */
static uint block_tree_max_key_count(const struct block_tree *tree, bool leaf)
{
    uint key_count = tree->key_count[leaf];
    assert(key_count);
    assert(key_count * tree->key_size < tree->block_size);

    return key_count;
}

/**
 * block_tree_node_max_key_count - Return max key count for specific node
 * @tree:       Tree object.
 * @node_ro:    Node pointer.
 *
 * Return: Maximum number of keys that fit @node_ro.
 */
static uint block_tree_node_max_key_count(const struct block_tree *tree,
                                          const struct block_tree_node *node_ro)
{
    return block_tree_max_key_count(tree, block_tree_node_is_leaf(node_ro));
}

/**
 * enum block_tree_shift_mode - Child selection for block_tree_node_shift
 * @SHIFT_LEAF_OR_LEFT_CHILD:   Required value for leaf nodes, selects left
 *                              child for internal nodes.
 * @SHIFT_RIGHT_CHILD:          Select right child.
 * @SHIFT_LEFT_CHILD:           Select left child (for src_index).
 * @SHIFT_BOTH:                 Select left child at start and right child at
 *                              end. Only valid when inserting data at the end
 *                              of a node.
 */
enum block_tree_shift_mode {
    SHIFT_LEAF_OR_LEFT_CHILD,
    SHIFT_RIGHT_CHILD = 1,
    SHIFT_LEFT_CHILD = 2,
    SHIFT_BOTH = SHIFT_RIGHT_CHILD | SHIFT_LEFT_CHILD,
};

/**
 * block_tree_node_shift - Helper function to insert or remove entries in a node
 * @tree:           Tree object.
 * @node_rw:        Node pointer.
 * @dest_index:     Destination index for existing entries to shift.
 * @src_index:      Source index for existing entries to shift.
 * @shift_mode:     Specifies which child to shift for internal nodes.
 * @new_key:        When shifting up (inserting), keys to insert.
 * @new_data:       When shifting up (inserting), data (or child) to insert.
 * @overflow_key:   When shifting up (inserting), location to store keys that
 *                  was shifted out of range. Can be %NULL if all those keys are
 *                  0.
 * @overflow_data:  When shifting up (inserting), location to store data (or
 *                  child) that was shifted out of range. Can be %NULL if that
 *                  data is all 0.
 */
static void block_tree_node_shift(const struct block_tree *tree,
                                  struct block_tree_node *node_rw,
                                  uint dest_index, uint src_index,
                                  enum block_tree_shift_mode shift_mode,
                                  const void *new_key,
                                  const void *new_data,
                                  void *overflow_key,
                                  void *overflow_data)
{
    bool is_leaf = block_tree_node_is_leaf(node_rw);
    uint max_count = tree->key_count[is_leaf];

    int i;
    void *base;
    size_t entry_size;

    const void *src;
    void *dest;
    size_t size;
    uint clear_index;

    assert(max_count);
    assert(dest_index <= max_count + !is_leaf + 1);

    for (i = 0; i < 2; i++) {
        if (i == 0) {
            /* key */
            base = node_rw->data;
            entry_size = tree->key_size;
        } else {
            /* data/child */
            base += tree->key_size * max_count;
            entry_size = tree->child_data_size[is_leaf];
            if (!is_leaf) {
                /* internal nodes have one more child than keys */
                max_count++;
            }
            if (shift_mode & SHIFT_RIGHT_CHILD) {
                assert(!is_leaf);
                if (!(shift_mode & SHIFT_LEFT_CHILD) && src_index != ~0U) {
                    src_index++;
                }
                dest_index++;
            }
        }

        if (src_index < dest_index) {
            /* Inserting, copy entries that will be overwritten to overflow_* */
            size = (dest_index - src_index) * entry_size;
            if (src_index == max_count) {
                src = i == 0 ? new_key : new_data;
            } else {
                src = base + max_count * entry_size - size;
            }
            dest = i == 0 ? overflow_key : overflow_data;
            if (dest) {
                if (print_node_changes) {
                    printf("%s: copy %p, index %d/%d, to overflow, %p, size %zd, is_zero %d\n",
                           __func__, src, max_count - (dest_index - src_index),
                           max_count, dest, size, is_zero(src, size));
                }
                memcpy(dest, src, size);
            } else {
                assert(is_zero(src, size));
            }
        }

        if (src_index < max_count) {
            /* Inserting or deleting, shift up or down */
            src = base + src_index * entry_size;
            dest = base + dest_index * entry_size;
            size = (max_count - MAX(src_index, dest_index)) * entry_size;
            if (print_node_changes) {
                printf("%s: move %p, index %d, to %p, index %d, size %zd, is_zero %d\n",
                       __func__, src, src_index, dest, dest_index, size, is_zero(src, size));
            }
            memmove(dest, src, size);
            if (src_index >= dest_index) {
                clear_index = max_count + dest_index - src_index;
            }
        } else {
            clear_index = dest_index;
        }

        if (src_index < dest_index) {
            /* Inserting, copy new entries passed in */
            assert(new_key);
            assert(new_data);
            src = i == 0 ? new_key : new_data;
            dest = base + src_index * entry_size;
            size = (dest_index - src_index) * entry_size;
            if (src_index == max_count) {
                /* NOP, data was copied to overflow_* above */
            } else {
                assert(src);
                if (print_node_changes) {
                    printf("%s: copy new data %p, to %p, index %d, size %zd, is_zero %d\n",
                           __func__, src, dest, src_index, size, is_zero(src, size));
                }
                memcpy(dest, src, size);
            }
        } else {
            /* Deleting, clear unused space at end */
            assert(dest_index <= max_count);
            dest = base + clear_index * entry_size;
            size = (max_count - clear_index) * entry_size;
            if (print_node_changes) {
                printf("%s: clear %p, index %d/%d, size %zd\n",
                       __func__, dest, clear_index, max_count, size);
            }
            memset(dest, 0, size);
        }
    }
}

/**
 * block_tree_node_merge_entries - Helper function to merge nodes
 * @tree:           Tree object.
 * @node_rw:        Destination node.
 * @src_node_ro:    Source node.
 * @dest_index:     Index to insert at in @node_rw.
 * @count:          Number of entries to copy from start of @src_node_ro.
 * @merge_key:      For internal nodes, key to insert between nodes.
 */
static void block_tree_node_merge_entries(const struct block_tree *tree,
                                          struct block_tree_node *node_rw,
                                          const struct block_tree_node *src_node_ro,
                                          uint dest_index, uint count,
                                          const void *merge_key)
{
    bool is_leaf = block_tree_node_is_leaf(node_rw);
    uint max_count = tree->key_count[is_leaf];
    void *dest_key;
    enum block_tree_shift_mode shift_mode = SHIFT_LEAF_OR_LEFT_CHILD;
    if (!is_leaf) {
        dest_key = node_rw->data + tree->key_size * dest_index;
        assert(is_zero(dest_key, tree->key_size));
        memcpy(dest_key, merge_key, tree->key_size);
        dest_index++;
        shift_mode = SHIFT_BOTH;
    }
    block_tree_node_shift(tree, node_rw, dest_index + count, dest_index,
                          shift_mode,
                          src_node_ro->data,
                          src_node_ro->data + tree->key_size * max_count,
                          NULL, NULL);
}

/**
 * block_tree_node_shift_down - Helper function to delete entries from node
 * @tree:           Tree object.
 * @node_rw:        Node.
 * @dest_index:     Destination index for existing entries to shift (or
 *                  first entry to remove).
 * @src_index:      Source index for existing entries to shift (or first entry
 *                  after @dest_index not to remove).
 * @shift_mode:     Specifies which child to shift for internal nodes.
 */
static void block_tree_node_shift_down(const struct block_tree *tree,
                                       struct block_tree_node *node_rw,
                                       uint dest_index, uint src_index,
                                       enum block_tree_shift_mode shift_mode)
{
    assert(dest_index < src_index);
    block_tree_node_shift(tree, node_rw, dest_index, src_index, shift_mode,
                          NULL, NULL, NULL, NULL);
}

/**
 * block_tree_node_shift_down - Helper function to delete entries from end of node
 * @tree:           Tree object.
 * @node_rw:        Node.
 * @start_index:    Index of first entry to remove. For internal nodes the
 *                  the right child is removed, so @start_index refers to the
 *                  key index, not the child index.
 */
static void block_tree_node_clear_end(const struct block_tree *tree,
                                      struct block_tree_node *node_rw,
                                      uint start_index)
{
    block_tree_node_shift_down(tree, node_rw, start_index, ~0,
                               block_tree_node_is_leaf(node_rw) ?
                               SHIFT_LEAF_OR_LEFT_CHILD : SHIFT_RIGHT_CHILD);
}

/**
 * block_tree_node_insert - Helper function to insert one entry in a node
 * @tree:           Tree object.
 * @node_rw:        Node.
 * @index:          Index to insert at.
 * @shift_mode:     Specifies which child to insert for internal nodes.
 * @new_key:        Key to insert.
 * @new_data:       Data or child to insert.
 * @overflow_key:   Location to store key that was shifted out of range. Can be
 *                  %NULL if that key is always 0.
 * @overflow_data:  Location to store data (or child) that was shifted out of
 *                  range. Can be %NULL if that data is all 0.
 */
static void block_tree_node_insert(const struct block_tree *tree,
                                   struct block_tree_node *node_rw,
                                   uint index,
                                   enum block_tree_shift_mode shift_mode,
                                   const void *new_key,
                                   const void *new_data,
                                   void *overflow_key,
                                   void *overflow_data)
{
    block_tree_node_shift(tree, node_rw, index + 1, index, shift_mode,
                          new_key, new_data, overflow_key, overflow_data);
}

/**
 * block_tree_node_get_key - Get key from node or in-memory pending update
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Node.
 * @index:          Index of key to get.
 *
 * Return: key at @index, or 0 if there is no key at @index. If @index is
 * equal to max key count, check for a matching entry in @tree->inserting.
 */
static data_block_t block_tree_node_get_key(const struct block_tree *tree,
                                            data_block_t node_block,
                                            const struct block_tree_node *node_ro,
                                            uint index)
{
    data_block_t key = 0;
    const void *keyp;
    const size_t key_count = block_tree_node_max_key_count(tree, node_ro);
    const size_t key_size = tree->key_size;

    assert(node_ro);

    if (index < key_count) {
        keyp = node_ro->data + index * tree->key_size;
        assert(sizeof(key) >= key_size);
        memcpy(&key, keyp, key_size);
    }
    if (!key && node_block == tree->inserting.block) {
        assert(index >= key_count);

        if (index <= key_count) {
            key = tree->inserting.key;
        }
    }
    return key;
}

/**
 * block_tree_node_set_key - Set key in node
 * @tree:           Tree object.
 * @node_rw:        Node.
 * @index:          Index of key to set.
 * @new_key:        Key value to write.
 */
static void block_tree_node_set_key(const struct block_tree *tree,
                                    struct block_tree_node *node_rw,
                                    uint index,
                                    data_block_t new_key)
{
    const size_t key_size = tree->key_size;
    const size_t key_count = block_tree_node_max_key_count(tree, node_rw);

    assert(node_rw);
    assert(index < key_count);
    assert(key_size <= sizeof(new_key));
    memcpy(node_rw->data + index * tree->key_size, &new_key, key_size); /* TODO: support big-endian host */
}

/**
 * block_tree_node_get_child_data - Get pointer to child or data
 * @tree:           Tree object.
 * @node_ro:        Node.
 * @index:          Index of child or data entry to get.
 *
 * Return: pointer to child or data entry at index @index in @node_ro.
 */
static const void *block_tree_node_get_child_data(const struct block_tree *tree,
                                                  const struct block_tree_node *node_ro,
                                                  uint index)
{
    bool is_leaf = block_tree_node_is_leaf(node_ro);
    const size_t max_key_count = tree->key_count[is_leaf];
    const size_t child_data_size = tree->child_data_size[is_leaf];
    const void *child_data;

    assert(index < max_key_count + !is_leaf);

    child_data = node_ro->data + tree->key_size * max_key_count + child_data_size * index;

    assert(child_data > (void *)node_ro->data);
    assert(child_data < (void *)node_ro + tree->block_size);

    return child_data;
}

/**
 * block_tree_node_get_child_data_rw - Get pointer to child or data
 * @tree:           Tree object.
 * @node_rw:        Node.
 * @index:          Index of child or data entry to get.
 *
 * Return: pointer to child or data entry at index @index in @node_rw.
 */
static void *block_tree_node_get_child_data_rw(const struct block_tree *tree,
                                               struct block_tree_node *node_rw,
                                               int index)
{
    return (void *)block_tree_node_get_child_data(tree, node_rw, index);
}

/**
 * block_tree_node_get_child - Get child from node or in-memory pending update
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Internal node.
 * @index:          Index of child to get.
 *
 * Return: child at @index, or 0 if there is no child at @index. If @index is
 * equal to max child count, check for a matching entry in @tree->inserting.
 */
static const struct block_mac *block_tree_node_get_child(const struct transaction *tr,
                                                         const struct block_tree *tree,
                                                         data_block_t node_block,
                                                         const struct block_tree_node *node_ro,
                                                         uint index)
{
    const struct block_mac *child = NULL;
    const size_t key_count = block_tree_node_max_key_count(tree, node_ro);

    assert(!block_tree_node_is_leaf(node_ro));

    if (index <= key_count) {
        child = block_tree_node_get_child_data(tree, node_ro, index);
        if (!block_mac_to_block(tr, child)) {
            child = NULL;
        }
    }

    if (!child && node_block == tree->inserting.block) {
        assert(index > key_count);

        if (index <= key_count + 1) {
            child = &tree->inserting.child;
        }
    }

    return child;
}

/**
 * block_tree_node_get_data - Get data from node or in-memory pending update
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Leaf node.
 * @index:          Index of data to get.
 *
 * Return: data at @index, or 0 if there is no data at @index. If @index is
 * equal to max key count, check for a matching entry in @tree->inserting.
 */
static struct block_mac block_tree_node_get_data(const struct transaction *tr,
                                                 const struct block_tree *tree,
                                                 data_block_t node_block,
                                                 const struct block_tree_node *node_ro,
                                                 uint index)
{
    const struct block_mac empty_block_mac = BLOCK_MAC_INITIAL_VALUE(empty_block_mac);
    const struct block_mac *datap = NULL;
    const size_t max_key_count = block_tree_node_max_key_count(tree, node_ro);

    assert(block_tree_node_is_leaf(node_ro));

    if (index < max_key_count) {
        datap = block_tree_node_get_child_data(tree, node_ro, index);
        if (!block_mac_to_block(tr, datap)) {
            datap = NULL;
        }
    }
    if (!datap && node_block == tree->inserting.block) {
        assert(index >= max_key_count);

        if (index <= max_key_count) {
            datap = &tree->inserting.data;
        }
    }
    if (datap) {
        return *datap;
    } else {
        return empty_block_mac;
    }
}


#define block_tree_node_for_each_child(tr, tree, block, node_ro, child, i) \
    for (i = 0; (child = block_tree_node_get_child(tr, tree, block, node_ro, i)); i++)

/**
 * block_tree_node_print_internal - Print internal node
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Node.
 */
static void block_tree_node_print_internal(const struct transaction *tr,
                                           const struct block_tree *tree,
                                           data_block_t node_block,
                                           const struct block_tree_node *node_ro)
{
    uint i;
    const struct block_mac *child;
    const size_t key_count = block_tree_node_max_key_count(tree, node_ro);

    assert(!block_tree_node_is_leaf(node_ro));

    for (i = 0; i <= key_count + 1; i++) {
        child = block_tree_node_get_child(tr, tree, node_block, node_ro, i);
        if (child) {
            printf(" %lld", block_mac_to_block(tr, child));
        } else if (i < key_count + 1) {
            printf(" .");
        }
        if (block_tree_node_get_key(tree, node_block, node_ro, i)) {
            if (i == key_count) {
                printf(" inserting");
            }
            printf(" [%lld-]", block_tree_node_get_key(tree, node_block, node_ro, i));
        }
    }
    assert(!block_tree_node_get_child(tr, tree, node_block, node_ro, i));
    printf("\n");
}

/**
 * block_tree_node_print_leaf - Print leaf node
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Node.
 */
static void block_tree_node_print_leaf(const struct transaction *tr,
                                       const struct block_tree *tree,
                                       data_block_t node_block,
                                       const struct block_tree_node *node_ro)
{
    uint i;
    data_block_t key;
    struct block_mac data;
    const size_t key_count = block_tree_node_max_key_count(tree, node_ro);

    assert(block_tree_node_is_leaf(node_ro));

    for (i = 0; i <= key_count; i++) {
        key = block_tree_node_get_key(tree, node_block, node_ro, i);
        data = block_tree_node_get_data(tr, tree, node_block, node_ro, i);
        if (key || block_mac_valid(tr, &data)) {
            if (i == key_count) {
                printf(" inserting");
            }
            printf(" [%lld: %lld]", key, block_mac_to_block(tr, &data));
        } else if (i < key_count){
            printf(" .");
        }
    }
    printf("\n");
}

/**
 * block_tree_node_print - Print node
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Node.
 */
static void block_tree_node_print(const struct transaction *tr,
                                  const struct block_tree *tree,
                                  data_block_t node_block,
                                  const struct block_tree_node *node_ro)
{
    printf("  %3lld:", node_block);
    if (node_ro->is_leaf == true) {
        block_tree_node_print_leaf(tr, tree, node_block, node_ro);
    } else if (!node_ro->is_leaf) {
        block_tree_node_print_internal(tr, tree, node_block, node_ro);
    } else {
        printf(" bad node header %llx\n", (long long)node_ro->is_leaf);
    }
}

/**
 * block_tree_print_sub_tree - Print tree or a branch of a tree
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @block_mac:      Root of tree or branch to print.
 */
static void block_tree_print_sub_tree(struct transaction *tr,
                                      const struct block_tree *tree,
                                      const struct block_mac *block_mac)
{
    int i;
    const struct block_tree_node *node_ro;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);
    const struct block_mac *child;

    if (!block_mac || !block_mac_valid(tr, block_mac)) {
        printf("empty\n");
        return;
    }

    node_ro = block_get(tr, block_mac, NULL, &node_ref);
    if (!node_ro) {
        printf("  %3lld: unreadable\n", block_mac_to_block(tr, block_mac));
        return;
    }
    block_tree_node_print(tr, tree, block_mac_to_block(tr, block_mac), node_ro);
    if (!node_ro->is_leaf) {
        block_tree_node_for_each_child(tr, tree, block_mac_to_block(tr, block_mac),
                                       node_ro, child, i) {
            block_tree_print_sub_tree(tr, tree, child);
        }
    }
    block_put(node_ro, &node_ref);
}

/**
 * block_tree_print - Print tree
 * @tr:             Transaction object.
 * @tree:           Tree object.
 */
void block_tree_print(struct transaction *tr, const struct block_tree *tree)
{
    block_tree_print_sub_tree(tr, tree, &tree->root);
}

/**
 * block_tree_node_check - Check node for errors
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @node_ro:        Node.
 * @node_block:     Block number where @node_ro data is stored.
 * @min_key:        Minimum allowed key value.
 * @max_key:        Maximum allowed key value.
 *
 * Return: %false is an error is detected, %true otherwise.
 */
static bool block_tree_node_check(const struct transaction *tr,
                                  const struct block_tree *tree,
                                  const struct block_tree_node *node_ro,
                                  data_block_t node_block,
                                  data_block_t min_key,
                                  data_block_t max_key)
{
    uint i;
    data_block_t key;
    data_block_t prev_key = 0;
    int empty_count;
    const void *child_data;
    size_t key_count = block_tree_node_max_key_count(tree, node_ro);
    bool is_leaf;

    if (node_ro->is_leaf && node_ro->is_leaf != true) {
        printf("%s: bad node header %llx\n", __func__, (long long)node_ro->is_leaf);
        goto err;
    }
    is_leaf = block_tree_node_is_leaf(node_ro);

    empty_count = 0;
    for (i = 0; i < key_count; i++) {
        key = block_tree_node_get_key(tree, node_block, node_ro, i);
        if (!key) {
            empty_count++;
        }
        if (empty_count) {
            if (key) {
                printf("%s: %lld: expected zero key at %d, found %lld\n",
                       __func__, node_block, i, key);
                goto err;
            }
            child_data = block_tree_node_get_child_data(tree, node_ro, i + !is_leaf);
            if (!is_zero(child_data, tree->child_data_size[is_leaf])) {
                printf("%s: %lld: expected zero data/right child value at %d\n",
                       __func__, node_block, i);
                goto err;
            }
            continue;
        }
        if (key < prev_key || key < min_key || key > max_key) {
            printf("%s: %lld: bad key at %d, %lld not in [%lld/%lld-%lld]\n",
                   __func__, node_block, i, key, min_key, prev_key, max_key);
            if (tr->failed && key >= prev_key) {
                printf("%s: transaction failed, ignore\n", __func__);
            }
            else {
                goto err;
            }
        }
        prev_key = key;
    }
    return true;

err:
    block_tree_node_print(tr, tree, node_block, node_ro);
    return false;
}

/**
 * block_tree_check_sub_tree - Check tree or a branch of a tree for errros
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @block_mac:      Root of tree or branch to check.
 * @is_root:        %true if @block_mac refers to the root of the tree, %false
 *                  otherwise.
 * @min_key:        Minimum allowed key value.
 * @max_key:        Maximum allowed key value.
 * @updating:       %true if @tree is currently updating and nodes below
 *                  min-full should be allowed, %false otherwise.
 *
 * Return: Depth of tree/branch, -1 if an error was detected or -2 if @block_mac
 * could not be read.
 *
 * TODO: Reduce overlap with and move more checks to block_tree_node_check.
 */
static int block_tree_check_sub_tree(struct transaction *tr,
                                     const struct block_tree *tree,
                                     const struct block_mac *block_mac,
                                     bool is_root,
                                     data_block_t min_key,
                                     data_block_t max_key,
                                     bool updating)
{
    const struct block_tree_node *node_ro;
    uint i;
    int last_child = 0;
    int empty_count;
    int depth;
    int sub_tree_depth;
    data_block_t child_min_key;
    data_block_t child_max_key;
    data_block_t key;
    int max_empty_count;
    size_t key_count;
    const void *child_data;
    struct block_mac child_block_mac;
    obj_ref_t ref = OBJ_REF_INITIAL_VALUE(ref);
    bool is_leaf;

    if (!block_mac || !block_mac_to_block(tr, block_mac))
        return 0;

    depth = 1;

    if (block_mac_to_block(tr, block_mac) >= tr->fs->dev->block_count) {
        printf("%s: %3lld: invalid\n",
               __func__, block_mac_to_block(tr, block_mac));
        return -1;
    }

    node_ro = block_get_no_tr_fail(tr, block_mac, NULL, &ref);
    if (!node_ro) {
        if (tr->failed) {
            /*
             * Failed transactions discards dirty cache entries so parts of the
             * tree may now be unreadable.
             */
            pr_warn("ignore unreadable block %lld, transaction failed\n",
                    block_mac_to_block(tr, block_mac));
            return -2;
        }
        pr_warn("%3lld: unreadable\n", block_mac_to_block(tr, block_mac));
        return -2;
    }

    if (!block_tree_node_check(tr, tree, node_ro, block_mac_to_block(tr, block_mac),
                               min_key, max_key)) {
        goto err;
    }

    if (node_ro->is_leaf && node_ro->is_leaf != true) {
        printf("%s: bad node header %llx\n", __func__, (long long)node_ro->is_leaf);
        goto err;
    }
    is_leaf = block_tree_node_is_leaf(node_ro);

    key_count = block_tree_node_max_key_count(tree, node_ro);
    max_empty_count = is_root ? (key_count /*- 1*/) : /* TODO: don't allow empty root */
                                ((key_count + 1) / 2 + updating);

    child_min_key = min_key;
    empty_count = 0;
    for (i = 0; i < key_count; i++) {
        key = block_tree_node_get_key(tree, block_mac_to_block(tr, block_mac),
                                      node_ro, i);
        if (!key) {
            empty_count++;
        }
        if (empty_count) {
            if (key) {
                printf("%s: %lld: expected zero key at %d, found %lld\n",
                       __func__, block_mac_to_block(tr, block_mac), i, key);
                goto err;
            }
            child_data = block_tree_node_get_child_data(tree, node_ro, i + !is_leaf);
            if (!is_zero(child_data, tree->child_data_size[is_leaf])) {
                printf("%s: %lld: expected zero data/right child value at %d\n",
                       __func__, block_mac_to_block(tr, block_mac), i);
                goto err;
            }
            continue;
        }
        if (i == 0 && min_key && is_leaf && key != min_key) {
            printf("%s: %lld: bad key at %d, %lld not start of [%lld-%lld]\n",
                   __func__, block_mac_to_block(tr, block_mac),
                   i, key, min_key, max_key);
            if (tr->failed) {
                printf("%s: transaction failed, ignore\n", __func__);
            } else if (!key) {
                printf("%s: ignore empty node error\n", __func__); // warn only for now
            } else {
                goto err;
            }
        }
        min_key = key;
        child_max_key = key;
        if (!is_leaf) {
            child_data = block_tree_node_get_child_data(tree, node_ro, i);
            block_mac_copy(tr, &child_block_mac, child_data);
            block_put(node_ro, &ref);
            sub_tree_depth = block_tree_check_sub_tree(tr, tree,
                                                       &child_block_mac, false,
                                                       child_min_key,
                                                       child_max_key,
                                                       updating);
            node_ro = block_get_no_tr_fail(tr, block_mac, NULL, &ref);
            if (!node_ro) {
                pr_warn("%3lld: unreadable\n",
                        block_mac_to_block(tr, block_mac));
                return -2;
            }
            if (sub_tree_depth == -1) {
                goto err;
            }
            if (sub_tree_depth == -2) {
                pr_warn("%lld: unreadable subtree at %d\n",
                        block_mac_to_block(tr, block_mac), i);
                if (depth == 1) {
                    depth = -2;
                }
            } else if (depth == 1 || depth == -2) {
                depth = sub_tree_depth + 1;
            } else if (sub_tree_depth != depth - 1) {
                printf("%s: %lld: bad subtree depth at %d, %d != %d\n",
                       __func__, block_mac_to_block(tr, block_mac),
                       i, sub_tree_depth, depth - 1);
                goto err;
            }
        }
        child_min_key = key;
        min_key = key;
        last_child = i + 1;
    }
    child_max_key = max_key;
    if (!is_leaf) {
        child_data = block_tree_node_get_child_data(tree, node_ro, last_child);
        block_mac_copy(tr, &child_block_mac, child_data);
        block_put(node_ro, &ref);
        sub_tree_depth = block_tree_check_sub_tree(tr, tree,
                                                   &child_block_mac, false,
                                                   child_min_key, child_max_key,
                                                   updating);
        node_ro = block_get_no_tr_fail(tr, block_mac, NULL, &ref);
        if (!node_ro) {
            pr_warn("%3lld: unreadable\n", block_mac_to_block(tr, block_mac));
            return -2;
        }
        if (sub_tree_depth == -1) {
            goto err;
        }
        if (sub_tree_depth == -2) {
            pr_warn("%lld: unreadable subtree at %d\n",
                    block_mac_to_block(tr, block_mac), last_child);
            if (depth == 1) {
                depth = -2;
            }
        } else if (depth == 1 || depth == -2) {
            depth = sub_tree_depth + 1;
        } else if (sub_tree_depth != depth - 1) {
            printf("%s: %lld: bad subtree depth at %d, %d != %d\n",
                   __func__, block_mac_to_block(tr, block_mac),
                   last_child, sub_tree_depth, depth - 1);
            goto err;
        }
    }
    if (empty_count > max_empty_count) {
        printf("%s: %lld: too many empty nodes, %d > %d\n", __func__,
               block_mac_to_block(tr, block_mac), empty_count, max_empty_count);
        if (tr->failed) {
            printf("%s: ignore error, transaction failed\n", __func__);
        } else {
            goto err;
        }
    }
    block_put(node_ro, &ref);

    return depth;

err:
    block_put(node_ro, &ref);
    return -1;
}

/**
 * block_tree_check - Check tree for errros
 * @tr:             Transaction object.
 * @tree:           Tree object.
 *
 * Return: %false if an error was detected, %true otherwise.
 */
bool block_tree_check(struct transaction *tr, const struct block_tree *tree)
{
    int ret;

    assert(tr->fs->dev->block_size >= tree->block_size);

    ret = block_tree_check_sub_tree(tr, tree, &tree->root, true,
                                    0, ~0ULL,
                                    tree->updating);
    if (ret < 0) {
        if (ret == -2) {
            pr_warn("block_tree not fully readable\n");
            if (!tr->failed) {
                transaction_fail(tr);
            }
            return true;
        }
        printf("%s: invalid block_tree:\n", __func__);
        block_tree_print(tr, tree);
        return false;
    }
    return true;
}

/**
 * block_tree_node_full - Check if node is full
 * @tree:           Tree object.
 * @node_ro:        Node.
 *
 * Return: %true is @node_ro is full, %false otherwise.
 */
static bool block_tree_node_full(const struct block_tree *tree,
                                 const struct block_tree_node *node_ro)
{
    const size_t key_count = block_tree_node_max_key_count(tree, node_ro);
    return !!block_tree_node_get_key(tree, ~0, node_ro, key_count - 1);
}

/**
 * block_tree_min_key_count - Return the minimum number of keys for a node
 * @tree:           Tree object.
 * @leaf:           %true for leaf nodes, %false for internal nodes.
 *
 * B+ tree minimum full rules for non-root nodes.
 *
 * min-children = (max-children + 1) / 2 - 1
 * max-keys = max-children - 1
 * max-children = max-keys + 1
 * min-keys = min-children - 1
 *          = (max-children + 1) / 2 - 1
 *          = (max-keys + 1 + 1) / 2 - 1
 *          = max-keys / 2
 *
 * Return: minimum number of keys required for non-root leaf or internal nodes.
 */
static uint block_tree_min_key_count(const struct block_tree *tree, bool leaf)
{
    return block_tree_max_key_count(tree, leaf) / 2;
}

/**
 * block_tree_node_min_full_index - Return index of last key in "min-full" node
 * @tree:           Tree object.
 * @node_ro:        Node.
 *
 * Return: index.
 */
static int block_tree_node_min_full_index(const struct block_tree *tree,
                                          const struct block_tree_node *node_ro)
{
    return block_tree_min_key_count(tree, block_tree_node_is_leaf(node_ro)) - 1;
}

/**
 * block_tree_above_min_full - Check if node has more than the minimum number of entries
 * @tree:           Tree object.
 * @node_ro:        Node.
 *
 * Return: %true if @node_ro has more than the minimim required entry count,
 * %false otherwise.
 */
bool block_tree_above_min_full(const struct block_tree *tree,
                               const struct block_tree_node *node_ro)
{
    int min_full_index = block_tree_node_min_full_index(tree, node_ro);
    return !!block_tree_node_get_key(tree, ~0, node_ro, min_full_index + 1);
}

/**
 * block_tree_above_min_full - Check if node has less than the minimum number of entries
 * @tree:           Tree object.
 * @node_ro:        Node.
 *
 * Return: %true if @node_ro has less than the minimim required entry count,
 * %false otherwise.
 */
bool block_tree_below_min_full(const struct block_tree *tree,
                               const struct block_tree_node *node_ro)
{
    int min_full_index = block_tree_node_min_full_index(tree, node_ro);
    return !block_tree_node_get_key(tree, ~0, node_ro, min_full_index);
}

/**
 * block_tree_node_need_copy - Check if node needs to be copied before write
 * @tr:         Transaction object.
 * @tree:       Tree object.
 * @block_mac:  Block number and mac of node to check.
 *
 * Return: %true if @tree is a copy_on_write tree and @block_mac needs to be
 * copied before the node can be written, %false otherwise.
 */
static bool block_tree_node_need_copy(struct transaction *tr,
                                      const struct block_tree *tree,
                                      const struct block_mac *block_mac)
{
    return tree->copy_on_write &&
           transaction_block_need_copy(tr, block_mac_to_block(tr, block_mac)) &&
           !tr->failed;
}

/**
 * block_tree_node_find_block - Helper function for block_tree_walk
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @node_block:     Block number where @node_ro data is stored.
 * @node_ro:        Node to search.
 * @key:            Key to search for.
 * @key_is_max:     If %true, and @key is not found in a leaf node, use largest
 *                  entry with a key less than @key. If %false, and @key is not
 *                  found in a leaf node, use smallest entry with a key greater
 *                  than @key. Ignored for internal nodes or when @key is found.
 * @child:          Output arg for child at returned index.
 * @prev_key:       Output arg. Unmodified if returned index is 0, set to key at
 *                  index - 1 otherwise.
 * @next_key:       Output arg. Unmodified if returned index does not contain
 *                  an entry, set to key at index otherwise.
 *
 * Return: index of closest matching entry.
 */
static int block_tree_node_find_block(const struct transaction *tr,
                                      const struct block_tree *tree,
                                      data_block_t node_block,
                                      const struct block_tree_node *node_ro,
                                      data_block_t key,
                                      bool key_is_max,
                                      const struct block_mac **child,
                                      data_block_t *prev_key,
                                      data_block_t *next_key)
{
    int i;
    data_block_t curr_key;
    int keys_count;
    bool is_leaf = block_tree_node_is_leaf(node_ro);

    keys_count = block_tree_node_max_key_count(tree, node_ro);

    /* TODO: better search? */
    for (i = 0; i < keys_count + 1; i++) {
        curr_key = block_tree_node_get_key(tree, node_block, node_ro, i);
        if (key <= (curr_key - !is_leaf) || !curr_key) {
            break;
        }
        curr_key = 0;
    }
    if (i == keys_count && curr_key) {
        assert(tree->inserting.block == node_block);
        if (print_ops) {
            printf("%s: using inserting block %lld, key %lld, child %lld, data %lld\n",
                   __func__, tree->inserting.block, tree->inserting.key,
                   block_mac_to_block(tr, &tree->inserting.child),
                   block_mac_to_block(tr, &tree->inserting.data));
        }
    }
    if (key_is_max && is_leaf && i > 0 && (!curr_key || curr_key > key)) {
        i--;
        curr_key = block_tree_node_get_key(tree, node_block, node_ro, i);
    }

    if (curr_key) {
        *next_key = curr_key;
    }
    if (i > 0) {
        *prev_key = block_tree_node_get_key(tree, node_block, node_ro, i - 1); /* TODO: move to loop */
    }

    if (is_leaf) {
        *child = NULL;
    } else {
        *child = block_tree_node_get_child(tr, tree, node_block, node_ro, i);
        assert(*child);
        assert(block_mac_valid(tr, *child));
    }
    if (print_lookup) {
        printf("%s: block %lld, key %lld return %d, child_block %lld, prev_key %lld, next_key %lld\n",
               __func__, node_block, key, i,
               *child ? block_mac_to_block(tr, *child) : 0, *prev_key, *next_key);
        block_tree_node_print(tr, tree, node_block, node_ro);
    }
    return i;
}

/**
 * block_tree_walk - Walk tree
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @key:            Key to search for.
 * @key_is_max:     If %true, and @key is not found in a leaf node, use largest
 *                  entry with a key less than @key. If %false, and @key is not
 *                  found in a leaf node, use smallest entry with a key greater
 *                  than @key. Ignored for internal nodes or when @key is found.
 * @path:           Tree path object to initalize.
 *
 * Walk tree to find path to @key or the insert point for @key.
 *
 * Note that if @key is not found, @path may point to an empty insert slot. A
 * caller that is looking for the closest match to @key, must therefore call
 * block_tree_path_next to advance @path to a valid entry if this case is hit.
 * TODO: Add a helper function or argument to avoid this.
 */
void block_tree_walk(struct transaction *tr,
                     struct block_tree *tree,
                     data_block_t key,
                     bool key_is_max,
                     struct block_tree_path *path)
{
    const struct block_tree_node *node_ro;
    const struct block_tree_node *parent_node_ro;
    obj_ref_t ref[2] = {
        OBJ_REF_INITIAL_VALUE(ref[0]),
        OBJ_REF_INITIAL_VALUE(ref[1]),
    };
    int ref_index = 0;
    const struct block_mac *child;
    data_block_t prev_key = 0;
    data_block_t next_key = 0;
    int child_index;
    const struct block_mac *block_mac;

    if (print_lookup) {
        printf("%s: tree root block %lld, key %lld, key_is_max %d\n",
               __func__, block_mac_to_block(tr, &tree->root), key, key_is_max);
    }

    assert(tr);
    assert(tree);
    assert(tree->block_size <= tr->fs->dev->block_size);

    path->count = 0;
    memset(&path->data, 0, sizeof(path->data));
    path->tr = tr;
    path->tree = tree;
    path->tree_update_count = tree->update_count;

    block_mac = &tree->root;
    parent_node_ro = NULL;

    while(block_mac && block_mac_valid(tr, block_mac)) {
        assert(path->count < countof(path->entry));

        node_ro = block_get(tr, block_mac, NULL, &ref[ref_index]);
        if (!node_ro) {
            assert(tr->failed);
            pr_warn("transaction failed, abort\n");
            path->count = 0;
            goto err;
        }
        assert(node_ro);

        path->entry[path->count].block_mac = *block_mac;

        child_index = block_tree_node_find_block(tr, path->tree,
                                                 block_mac_to_block(tr, block_mac),
                                                 node_ro,
                                                 key, key_is_max, &child,
                                                 &prev_key, &next_key);
        if (path->count) {
            assert(!path->entry[path->count - 1].next_key || next_key);
            assert(!path->entry[path->count - 1].next_key || next_key <= path->entry[path->count - 1].next_key);
            assert(!path->entry[path->count - 1].prev_key || prev_key);
            assert(!path->entry[path->count - 1].prev_key || prev_key >= path->entry[path->count - 1].prev_key);
        }
        path->entry[path->count].index = child_index;
        path->entry[path->count].prev_key = prev_key;
        path->entry[path->count].next_key = next_key;
        if (!child) {
            assert(block_tree_node_is_leaf(node_ro));
            path->data = block_tree_node_get_data(tr, tree, block_mac_to_block(tr, block_mac), node_ro, child_index);
            assert(!key_is_max || block_mac_valid(path->tr, &path->data) || path->count == 0);
        }

        block_mac = child;
        if (parent_node_ro) {
            block_put(parent_node_ro, &ref[!ref_index]);
        }
        parent_node_ro = node_ro;
        ref_index = !ref_index;

        if (print_lookup) {
            printf("%s: path[%d] = %lld, index %d, prev_key %lld, next_key %lld\n",
                   __func__, path->count,
                   block_mac_to_block(path->tr, &path->entry[path->count].block_mac),
                   path->entry[path->count].index,
                   path->entry[path->count].prev_key,
                   path->entry[path->count].next_key);
            if (!block_mac) {
                printf("%s: path.data.block = %lld\n",
                       __func__, block_mac_to_block(path->tr, &path->data));
            }
        }
        path->count++;
    }
err:
    if (parent_node_ro) {
        block_put(parent_node_ro, &ref[!ref_index]);
    }
}

/**
 * block_tree_path_next - Walk tree to get to next entry
 * @path:           Tree path object.
 */
void block_tree_path_next(struct block_tree_path *path)
{
    const struct block_tree_node *node_ro;
    const struct block_tree_node *parent_node_ro;
    obj_ref_t ref[2] = {
        OBJ_REF_INITIAL_VALUE(ref[0]),
        OBJ_REF_INITIAL_VALUE(ref[1]),
    };
    bool ref_index = 0;
    uint index;
    uint depth;
    const struct block_mac *block_mac;
    data_block_t prev_key;
    data_block_t next_key;
    struct block_mac next_data;
    const struct block_mac *next_child;
    data_block_t parent_next_key;

    assert(path->tree_update_count == path->tree->update_count);
    assert(path->count);

    depth = path->count - 1;
    assert(path->entry[depth].next_key);

    if (print_lookup) {
        printf("%s: tree root block %lld\n",
               __func__, block_mac_to_block(path->tr, &path->tree->root));
    }

    block_mac = &path->entry[depth].block_mac;
    index = path->entry[depth].index;
    parent_next_key = depth > 0 ? path->entry[depth - 1].next_key : 0;

    node_ro = block_get(path->tr, block_mac, NULL, &ref[ref_index]);
    if (!node_ro) {
        assert(path->tr->failed);
        pr_warn("transaction failed, abort\n");
        goto err_no_refs;
    }
    assert(node_ro);
    assert(block_tree_node_is_leaf(node_ro));
    prev_key = block_tree_node_get_key(path->tree, block_mac_to_block(path->tr, block_mac), node_ro, index);
    index++;
    next_key = block_tree_node_get_key(path->tree, block_mac_to_block(path->tr, block_mac), node_ro, index);
    next_data = block_tree_node_get_data(path->tr, path->tree, block_mac_to_block(path->tr, block_mac), node_ro, index);
    block_put(node_ro, &ref[ref_index]);

    assert(path->entry[depth].next_key == prev_key || !prev_key);

    if (next_key || !parent_next_key) {
        assert(!next_key || next_key >= prev_key);
        path->entry[depth].index = index;
        path->entry[depth].prev_key = prev_key;
        path->entry[depth].next_key = next_key;
        path->data = next_data;
        if (print_lookup) {
            printf("%s: path[%d] = %lld, index %d, prev_key %lld, next_key %lld\n",
                   __func__, depth,
                   block_mac_to_block(path->tr, &path->entry[depth].block_mac),
                   path->entry[depth].index,
                   path->entry[depth].prev_key,
                   path->entry[depth].next_key);
            printf("%s: path.data.block = %lld\n",
                   __func__, block_mac_to_block(path->tr, &path->data));
        }
        return;
    }

    assert(depth > 0);

    while (depth > 0) {
        depth--;
        if (!path->entry[depth].next_key) {
            continue;
        }
        block_mac = &path->entry[depth].block_mac;
        index = path->entry[depth].index;

        node_ro = block_get(path->tr, block_mac, NULL, &ref[ref_index]);
        if (!node_ro) {
            assert(path->tr->failed);
            pr_warn("transaction failed, abort\n");
            goto err_no_refs;
        }
        assert(node_ro);
        assert(!block_tree_node_is_leaf(node_ro));

        parent_next_key = depth > 0 ? path->entry[depth - 1].next_key : 0;

        prev_key = block_tree_node_get_key(path->tree, block_mac_to_block(path->tr, block_mac), node_ro, index);
        index++;
        next_key = block_tree_node_get_key(path->tree, block_mac_to_block(path->tr, block_mac), node_ro, index);
        next_child = block_tree_node_get_child(path->tr, path->tree, block_mac_to_block(path->tr, block_mac), node_ro, index);
        if (next_child) {
            parent_node_ro = node_ro;
            ref_index = !ref_index;
            assert(prev_key && prev_key == path->entry[depth].next_key);
            path->entry[depth].index = index;
            path->entry[depth].prev_key = prev_key;
            path->entry[depth].next_key = next_key ?: parent_next_key;
            if (print_lookup) {
                printf("%s: path[%d] = %lld, index %d, prev_key %lld, next_key %lld\n",
                       __func__, depth,
                       block_mac_to_block(path->tr, &path->entry[depth].block_mac),
                       path->entry[depth].index,
                       path->entry[depth].prev_key,
                       path->entry[depth].next_key);
            }
            break;
        }
        block_put(node_ro, &ref[ref_index]);
    }
    assert(next_child);
    while (++depth < path->count - 1) {
        node_ro = block_get(path->tr, next_child, NULL, &ref[ref_index]);
        if (!node_ro) {
            assert(path->tr->failed);
            pr_warn("transaction failed, abort\n");
            goto err_has_parent_ref;
        }
        assert(node_ro);
        assert(!block_tree_node_is_leaf(node_ro));
        path->entry[depth].block_mac = *next_child;
        block_put(parent_node_ro, &ref[!ref_index]);
        path->entry[depth].index = 0;
        path->entry[depth].prev_key = prev_key;
        path->entry[depth].next_key = block_tree_node_get_key(path->tree, ~0, node_ro, 0);
        if (print_lookup) {
            printf("%s: path[%d] = %lld, index %d, prev_key %lld, next_key %lld\n",
                   __func__, depth,
                   block_mac_to_block(path->tr, &path->entry[depth].block_mac),
                   path->entry[depth].index,
                   path->entry[depth].prev_key,
                   path->entry[depth].next_key);
        }
        next_child = block_tree_node_get_child(path->tr, path->tree, ~0, node_ro, 0);
        parent_node_ro = node_ro;
        ref_index = !ref_index;
        assert(path->entry[depth].next_key);
        assert(next_child);
    }

    assert(next_child);
    node_ro = block_get(path->tr, next_child, NULL, &ref[ref_index]);
    if (!node_ro) {
        assert(path->tr->failed);
        pr_warn("transaction failed, abort\n");
        goto err_has_parent_ref;
    }
    assert(node_ro);
    assert(block_tree_node_is_leaf(node_ro));
    path->entry[depth].block_mac = *next_child;
    block_put(parent_node_ro, &ref[!ref_index]);
    path->entry[depth].index = 0;
    path->entry[depth].prev_key = prev_key;
    path->entry[depth].next_key = block_tree_node_get_key(path->tree, ~0, node_ro, 0);
    path->data = block_tree_node_get_data(path->tr, path->tree, ~0, node_ro, 0);
    block_put(node_ro, &ref[ref_index]);
    assert(path->entry[depth].next_key);
    if (print_lookup) {
        printf("%s: path[%d] = %lld, index %d, prev_key %lld, next_key %lld\n",
               __func__, depth,
               block_mac_to_block(path->tr, &path->entry[depth].block_mac),
               path->entry[depth].index,
               path->entry[depth].prev_key,
               path->entry[depth].next_key);
        printf("%s: path.data.block = %lld\n",
               __func__, block_mac_to_block(path->tr, &path->data));
    }
    return;

err_has_parent_ref:
    block_put(parent_node_ro, &ref[!ref_index]);
err_no_refs:
    assert(path->tr->failed);
    path->count = 0;
}

/**
 * block_tree_block_dirty - Get writable node
 * @tr:         Transaction object.
 * @path:       Tree path object.
 * @path_index: Path depth that @node_ro was found at.
 * @node_ro:    Source node
 *
 * Copy @node_ro and update @path if needed.
 *
 * Return: writable pointer to @node_ro.
 */
static struct block_tree_node *block_tree_block_dirty(struct transaction *tr,
                                                      struct block_tree_path *path,
                                                      int path_index,
                                                      const struct block_tree_node *node_ro)
{
    data_block_t new_block;
    struct block_mac *block_mac;

    block_mac = &path->entry[path_index].block_mac;

    assert(path_index || block_mac_same_block(tr, block_mac, &path->tree->root));

    if (!block_tree_node_need_copy(tr, path->tree, block_mac)) {
        if (tr->failed) {
            return NULL;
        }
        return block_dirty(tr, node_ro, !path->tree->allow_copy_on_write);
    }
    assert(path->tree->allow_copy_on_write);
    new_block = block_allocate_etc(tr, !path->tree->allow_copy_on_write);
    if (print_internal_changes) {
        printf("%s: copy block %lld to %lld\n",
               __func__, block_mac_to_block(tr, block_mac), new_block);
    }
    if (!new_block) {
        return NULL;
    }
    assert(new_block != block_mac_to_block(tr, block_mac));
    assert(!tr->failed);
    block_free(tr, block_mac_to_block(tr, block_mac));
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        return NULL;
    }
    block_mac_set_block(tr, block_mac, new_block);
    if (!path_index) {
        block_mac_set_block(tr, &path->tree->root, new_block);
        path->tree->root_block_changed = true;
    }
    return block_move(tr, node_ro, new_block, !path->tree->allow_copy_on_write);
}

/**
 * block_tree_block_get_write - Get writable node fro path
 * @tr:         Transaction object.
 * @path:       Tree path object.
 * @path_index: Path depth to get node from.
 * @ref:        Pointer to store reference in.
 *
 * Get node from @path and @path_index, copying it and updating @path if
 * needed.
 *
 * Return: writable pointer to node.
 */
static struct block_tree_node *block_tree_block_get_write(struct transaction *tr,
                                                          struct block_tree_path *path,
                                                          int path_index,
                                                          obj_ref_t *ref)
{
    const struct block_tree_node *node_ro;
    struct block_tree_node *node_rw;

    node_ro = block_get(tr, &path->entry[path_index].block_mac, NULL, ref);
    if (!node_ro) {
        return NULL;
    }

    node_rw = block_tree_block_dirty(tr, path, path_index, node_ro);
    if (!node_rw) {
        block_put(node_ro, ref);
    }
    return node_rw;
}

/**
 * block_tree_path_put_dirty - Release dirty node and update tree
 * @tr:         Transaction object.
 * @path:       Tree path object.
 * @path_index: Path depth to get node from.
 * @data:       Block data pointed to by leaf node when @path_index is
 *              @path->count. Tree node otherwise.
 * @data_ref:   Reference to @data that will be released.
 *
 * Release dirty external block or tree node and update tree. Update mac values
 * and copy blocks as needed until root of tree is reached.
 */
void block_tree_path_put_dirty(struct transaction *tr,
                               struct block_tree_path *path,
                               int path_index,
                               void *data,
                               obj_ref_t *data_ref)
{
    uint index;
    struct block_mac *block_mac;
    struct block_tree_node *parent_node_rw;
    bool parent_is_leaf;
    obj_ref_t parent_node_ref = OBJ_REF_INITIAL_VALUE(parent_node_ref);
    obj_ref_t tmp_ref = OBJ_REF_INITIAL_VALUE(tmp_ref);

    if (path_index == (int)path->count) {
        assert(path_index < (int)countof(path->entry));
        path->entry[path_index].block_mac = path->data; // copy data
    }

    while (--path_index >= 0) {
        parent_node_rw = block_tree_block_get_write(tr, path, path_index, &parent_node_ref);
        if (tr->failed) {
            assert(!parent_node_rw);
            block_put_dirty_discard(data, data_ref);
            pr_warn("transaction failed, abort\n");
            return;
        }
        assert(parent_node_rw);
        parent_is_leaf = block_tree_node_is_leaf(parent_node_rw);
        index = path->entry[path_index].index;
        assert(path_index == (int)path->count - 1 || !parent_is_leaf);
        assert(path->tree->child_data_size[parent_is_leaf] <= sizeof(*block_mac));
        assert(path->tree->child_data_size[parent_is_leaf] >=
               tr->fs->block_num_size + tr->fs->mac_size);
        block_mac = block_tree_node_get_child_data_rw(path->tree, parent_node_rw, index);

        /* check that block was copied if needed */
        assert(!block_tree_node_need_copy(tr, path->tree, block_mac) ||
               !block_mac_same_block(tr, block_mac, &path->entry[path_index + 1].block_mac));

        /* check that block was not copied when not needed */
        assert(block_tree_node_need_copy(tr, path->tree, block_mac) ||
               block_mac_same_block(tr, block_mac, &path->entry[path_index + 1].block_mac) ||
               tr->failed);

        if (!block_mac_same_block(tr, block_mac, &path->entry[path_index + 1].block_mac)) {
            if (print_internal_changes) {
                printf("%s: update copied block %lld to %lld\n",
                       __func__, block_mac_to_block(tr, block_mac),
                       block_mac_to_block(tr, &path->entry[path_index + 1].block_mac));
            }
            block_mac_set_block(tr, block_mac, block_mac_to_block(tr, &path->entry[path_index + 1].block_mac));
        }

        if (tr->failed) {
            block_put_dirty_discard(data, data_ref);
            block_put_dirty_discard(parent_node_rw, &parent_node_ref);
            pr_warn("transaction failed, abort\n");
            return;
        }

        assert(block_mac_eq(tr, block_mac, &path->entry[path_index + 1].block_mac));
        block_put_dirty(tr, data, data_ref, block_mac, parent_node_rw);
        assert(!block_mac_eq(tr, block_mac, &path->entry[path_index + 1].block_mac) || tr->fs->mac_size < 16);
        block_mac_copy_mac(tr, &path->entry[path_index + 1].block_mac, block_mac);
        assert(block_mac_eq(tr, block_mac, &path->entry[path_index + 1].block_mac));
        data = parent_node_rw;
        data_ref = &tmp_ref;
        obj_ref_transfer(data_ref, &parent_node_ref);
    }
    block_mac = &path->tree->root;
    assert(block_mac_same_block(tr, block_mac, &path->entry[0].block_mac));
    assert(block_mac_eq(tr, block_mac, &path->entry[0].block_mac));
    block_put_dirty(tr, data, data_ref, block_mac, NULL);
    assert(!block_mac_eq(tr, block_mac, &path->entry[0].block_mac) || tr->fs->mac_size < 16);
    block_mac_copy_mac(tr, &path->entry[0].block_mac, block_mac);
    assert(block_mac_eq(tr, block_mac, &path->entry[0].block_mac));
}

/**
 * block_tree_update_key - Update key in internal parent node
 * @tr:         Transaction object.
 * @path:       Tree path object.
 * @path_index: Path depth start search at.
 * @new_key:    New key value.
 *
 * Search path until a left parent key is found (non-zero index) then set it
 * to @new_key.
 *
 * TODO: merge with mac update?
 */
void block_tree_update_key(struct transaction *tr,
                           struct block_tree_path *path,
                           int path_index,
                           data_block_t new_key)
{
    const struct block_mac *block_mac;
    uint index;
    struct block_tree_node *node_rw;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);

    assert(new_key);

    while (path_index >= 0) {
        assert(path_index < (int)countof(path->entry));
        block_mac = &path->entry[path_index].block_mac;
        index = path->entry[path_index].index;
        if (index == 0) {
            path_index--;
            continue;
        }
        node_rw = block_tree_block_get_write(tr, path, path_index, &node_ref);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            return;
        }
        assert(node_rw);
        if (print_internal_changes) {
            printf("%s: block %lld, index %d, [%lld-] -> [%lld-]\n",
                   __func__, block_mac_to_block(tr, block_mac), index,
                   block_tree_node_get_key(path->tree, ~0, node_rw, index - 1),
                   new_key);
            block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac), node_rw);
        }
        assert(!block_tree_node_is_leaf(node_rw));
        assert(index == 1 || new_key >= block_tree_node_get_key(path->tree, ~0, node_rw, index - 2));
        assert(index == block_tree_node_max_key_count(path->tree, node_rw) ||
               new_key <= block_tree_node_get_key(path->tree, ~0, node_rw, index) ||
               !block_tree_node_get_key(path->tree, ~0, node_rw, index));
        assert(path->entry[path_index].prev_key == block_tree_node_get_key(path->tree, ~0, node_rw, index - 1));
        block_tree_node_set_key(path->tree, node_rw, index - 1, new_key);
        assert(block_tree_node_check(tr, path->tree, node_rw, block_mac_to_block(tr, block_mac), 0, ~0));
        path->entry[path_index].prev_key = new_key;
        if (print_internal_changes) {
            block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac), node_rw);
        }
        block_tree_path_put_dirty(tr, path, path_index, node_rw, &node_ref);
        return;
    }
    if (print_internal_changes) {
        printf("%s: root reached, no update needed (new key %lld)\n",
               __func__, new_key);
    }
}

/**
 * block_tree_node_get_key_count - Get number of keys currently stored in node
 * @tree:           Tree object.
 * @node_ro:        Node.
 */
static int block_tree_node_get_key_count(const struct block_tree *tree,
                                         const struct block_tree_node *node_ro)
{
    uint i;
    uint max_key_count = block_tree_node_max_key_count(tree, node_ro);

    for (i = 0; i < max_key_count; i++) {
        if (!block_tree_node_get_key(tree, ~0, node_ro, i)) {
            break;
        }
    }

    return i;
}

/**
 * block_tree_node_leaf_remove_entry - Remove entry from a leaf node
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @node_block_mac: Block number and mac for @node_rw.
 * @node_rw:        Node.
 * @index:          Index of entry to remove.
 */
static void block_tree_node_leaf_remove_entry(struct transaction *tr,
                                              const struct block_tree *tree,
                                              const struct block_mac *node_block_mac,
                                              struct block_tree_node *node_rw,
                                              uint index)
{
    uint max_key_count = block_tree_node_max_key_count(tree, node_rw);

    if (print_internal_changes) {
        printf("%s: index %d:", __func__, index);
        block_tree_node_print(tr, tree, block_mac_to_block(tr, node_block_mac), node_rw);
    }
    assert(index < max_key_count);
    assert(block_tree_node_is_leaf(node_rw));
    block_tree_node_shift_down(tree, node_rw, index, index + 1,
                               SHIFT_LEAF_OR_LEFT_CHILD);
    if (print_internal_changes) {
        printf("%s: done:   ", __func__);
        block_tree_node_print(tr, tree, block_mac_to_block(tr, node_block_mac), node_rw);
    }
}

/**
 * block_tree_node_split - Split a full node and add entry.
 * @tr:             Transaction object.
 * @path:           Tree path object.
 * @append_key:     Key to add.
 * @append_child:   Child to add if @path points to an internal node.
 * @append_data:    Data to add if @path points to a leaf node.
 *
 * Allocate a new node and move half of the entries to it. If the old node was
 * the root node, allocate a new root node. Add new node to parent.
 */
static void block_tree_node_split(struct transaction *tr,
                                  struct block_tree_path *path,
                                  data_block_t append_key,
                                  const struct block_mac *append_child,
                                  const struct block_mac *append_data)
{
    struct block_tree_node *parent_node_rw;
    obj_ref_t parent_node_ref = OBJ_REF_INITIAL_VALUE(parent_node_ref);
    struct block_tree_node *node_left_rw;
    obj_ref_t node_left_ref = OBJ_REF_INITIAL_VALUE(node_left_ref);
    struct block_tree_node *node_right_rw;
    obj_ref_t node_right_ref = OBJ_REF_INITIAL_VALUE(node_right_ref);
    bool is_leaf;
    struct block_mac *left_block_mac;
    struct block_mac *right_block_mac;
    data_block_t left_block_num;
    struct block_mac right;
    const struct block_mac *block_mac;
    const struct block_mac *parent_block_mac;
    uint parent_index;
    int split_index;
    data_block_t parent_key;
    data_block_t overflow_key = 0;
    struct block_mac overflow_child;
    size_t max_key_count;

    assert(path->count > 0);
    assert(path->tree);

    block_mac = &path->entry[path->count - 1].block_mac;
    path->count--;

    if (print_internal_changes) {
        printf("%s: tree root %lld, block %lld\n",
               __func__, block_mac_to_block(tr, &path->tree->root),
               block_mac_to_block(tr, block_mac));
    }

    assert(append_key);
    assert(!path->tree->inserting.block); /* we should only insert one node at a time */
    path->tree->inserting.block = block_mac_to_block(tr, block_mac);
    path->tree->inserting.key = append_key;
    if (append_data) {
        assert(!append_child);
        path->tree->inserting.data = *append_data;
    }
    if (append_child) {
        assert(!append_data);
        path->tree->inserting.child = *append_child;
    }

    assert(!path->tree->copy_on_write || path->tree->allow_copy_on_write);

    block_mac_set_block(tr, &right,
                        block_allocate_etc(tr, !path->tree->allow_copy_on_write));
    if (!path->count) {
        /* use old block at the new root */
        left_block_num = block_allocate_etc(tr, !path->tree->allow_copy_on_write);
    } else {
        left_block_num = block_mac_to_block(tr, block_mac);
    }
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        return;
    }
    assert(block_mac);
    assert(block_mac_valid(tr, block_mac));
    assert(path->tree->inserting.block = block_mac_to_block(tr, block_mac));
    assert(path->tree->inserting.key == append_key);
    assert(block_mac_to_block(tr, &path->tree->inserting.data) ==
           (append_data ? block_mac_to_block(tr, append_data) : 0));
    assert(block_mac_to_block(tr, &path->tree->inserting.child) ==
           (append_child ? block_mac_to_block(tr, append_child) : 0));
    memset(&path->tree->inserting, 0, sizeof(path->tree->inserting));

    if (print_internal_changes) {
        printf("%s: %lld -> %lld %lld\n",
               __func__, block_mac_to_block(tr, block_mac), left_block_num,
               block_mac_to_block(tr, &right));
    }

    node_left_rw = block_tree_block_get_write(tr, path, path->count, &node_left_ref);
    if (!node_left_rw) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        return;
    }
    assert(node_left_rw);
    is_leaf = block_tree_node_is_leaf(node_left_rw);
    if (!path->count) {
        assert(left_block_num != block_mac_to_block(tr, block_mac));
        parent_node_rw = node_left_rw;
        obj_ref_transfer(&parent_node_ref, &node_left_ref);
        node_left_rw = block_get_copy(tr, node_left_rw, left_block_num,
                                      !path->tree->allow_copy_on_write,
                                      &node_left_ref); /* TODO: use allocated node for root instead since we need to update the mac anyway */
        assert(node_left_rw);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err_copy_parent;
        }
        //parent_node_rw = block_get_cleared(path->tree->root.block);
        memset(parent_node_rw, 0, path->tree->block_size); /* TODO: use block_get_cleared */
        parent_index = 0;
        left_block_mac = block_tree_node_get_child_data_rw(path->tree, parent_node_rw, parent_index);
        block_mac_set_block(tr, left_block_mac, left_block_num);
        parent_block_mac = block_mac;
    } else {
        assert(left_block_num == block_mac_to_block(tr, block_mac));
        parent_block_mac = &path->entry[path->count - 1].block_mac;
        parent_index = path->entry[path->count - 1].index;
        parent_node_rw = block_tree_block_get_write(tr, path, path->count - 1,
                                                    &parent_node_ref);
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err_get_parent;
        }
        assert(parent_node_rw);
        assert(!block_tree_node_is_leaf(parent_node_rw));
        left_block_mac = block_tree_node_get_child_data_rw(path->tree, parent_node_rw, parent_index);
    }
    assert(block_mac_to_block(tr, left_block_mac) == left_block_num);
    assert(!tr->failed);
    node_right_rw = block_get_copy(tr, node_left_rw,
                                   block_mac_to_block(tr, &right),
                                   !path->tree->allow_copy_on_write,
                                   &node_right_ref);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err_copy_right;
    }
    assert(block_tree_node_is_leaf(node_left_rw) == is_leaf);
    assert(block_tree_node_is_leaf(node_right_rw) == is_leaf);
    assert(is_leaf || (append_key && append_child));
    assert(!is_leaf || (append_key && append_data));
    assert(block_tree_node_full(path->tree, node_left_rw));
    assert(!tr->failed);

    max_key_count = block_tree_node_max_key_count(path->tree, node_left_rw);
    split_index = (max_key_count + 1) / 2;

    if (print_internal_changes) {
        printf("%s: insert split_index %d", __func__, split_index);
        block_tree_node_print(tr, path->tree, left_block_num, node_left_rw);
        if (is_leaf) {
            printf("%s: append key, data: [%lld: %lld]\n",
                   __func__, append_key, block_mac_to_block(tr, append_data));
        } else {
            printf("%s: append key, child: [%lld-] %lld\n",
                   __func__, append_key, block_mac_to_block(tr, append_child));
        }
    }

    /* Update left node */
    block_tree_node_clear_end(path->tree, node_left_rw, split_index);
    if (print_internal_changes) {
        printf("%s: left  %3lld", __func__, left_block_num);
        block_tree_node_print(tr, path->tree, left_block_num, node_left_rw);
    }

    /*
     * Update right node. For internal nodes the key at split_index moves to
     * the parent, for leaf nodes it gets copied
     */
    parent_key = block_tree_node_get_key(path->tree, ~0, node_right_rw, split_index);
    block_tree_node_shift_down(path->tree, node_right_rw, 0,
                               split_index + !is_leaf,
                               SHIFT_LEAF_OR_LEFT_CHILD);
    assert(max_key_count == block_tree_node_max_key_count(path->tree, node_right_rw));
    block_tree_node_insert(path->tree, node_right_rw,
                           max_key_count - split_index - !is_leaf,
                           is_leaf ?
                           SHIFT_LEAF_OR_LEFT_CHILD : SHIFT_RIGHT_CHILD,
                           &append_key,
                           is_leaf ? append_data : append_child,
                           NULL, NULL);
    if (print_internal_changes) {
        printf("%s: right %3lld", __func__, block_mac_to_block(tr, &right));
        block_tree_node_print(tr, path->tree, block_mac_to_block(tr, &right),
                              node_right_rw);
    }

    /* Insert right node in parent node */
    assert(!block_tree_node_is_leaf(parent_node_rw));

    if (print_internal_changes) {
        printf("%s: insert [%lld-] %lld @%d:", __func__,
               parent_key, block_mac_to_block(tr, &right), parent_index);
        block_tree_node_print(tr, path->tree,
                              block_mac_to_block(tr, parent_block_mac),
                              parent_node_rw);
    }

    assert(parent_index == block_tree_node_max_key_count(path->tree, parent_node_rw) ||
           !block_tree_node_get_key(path->tree, ~0, parent_node_rw, parent_index) ||
           parent_key < block_tree_node_get_key(path->tree, ~0, parent_node_rw, parent_index));
    assert(parent_index == 0 || block_tree_node_get_key(path->tree, ~0, parent_node_rw, parent_index - 1) <= parent_key);
    block_tree_node_insert(path->tree, parent_node_rw, parent_index, SHIFT_RIGHT_CHILD,
                           &parent_key, &right, &overflow_key, &overflow_child);
    assert(!overflow_key == !block_mac_valid(tr, &overflow_child));
    /*
     * If overflow_key is set it is not safe to re-enter the tree at this point
     * as overflow_child is not reachable. It becomes reachable again when
     * saved in tree->inserting.child at the top of this function.
     */

    if (parent_index < block_tree_node_max_key_count(path->tree, parent_node_rw)) {
        right_block_mac = block_tree_node_get_child_data_rw(path->tree, parent_node_rw, parent_index + 1);
    } else {
        assert(block_mac_valid(tr, &overflow_child));
        right_block_mac = &overflow_child;
    }

    if (print_internal_changes) {
        printf("%s: insert [%lld-] %lld @%d done:", __func__,
               parent_key, block_mac_to_block(tr, &right), parent_index);
        block_tree_node_print(tr, path->tree,
                              block_mac_to_block(tr, parent_block_mac),
                              parent_node_rw);
    }

    if (!path->count && print_internal_changes) {
        printf("%s root", __func__);
        block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac),
                              parent_node_rw);
    }

    block_put_dirty(tr, node_left_rw, &node_left_ref, left_block_mac, parent_node_rw);
    block_put_dirty(tr, node_right_rw, &node_right_ref, right_block_mac, parent_node_rw);
    block_tree_path_put_dirty(tr, path, path->count - 1,
                              parent_node_rw, &parent_node_ref);

    if (overflow_key) {
        assert(path->count); /* new root node should not overflow */
        if (print_internal_changes) {
            printf("%s: overflow [%lld-] %lld\n",
                   __func__, overflow_key, block_mac_to_block(tr, &overflow_child));
        }
        block_tree_node_split(tr, path, overflow_key, &overflow_child, 0); /* TODO: use a loop instead */
    }
    return;

err_copy_right:
    block_put_dirty_discard(node_right_rw, &node_right_ref);
err_copy_parent:
    block_put_dirty_discard(parent_node_rw, &parent_node_ref);
err_get_parent:
    block_put_dirty_discard(node_left_rw, &node_left_ref);
}

/**
 * block_tree_sibling_index - Get the index of a sibling node
 * @index:      Index of current node.
 *
 * Use left sibling when possible, otherwise use right sibling. Other siblings
 * could be used, but this simple rule avoids selecting an empty sibling if
 * a non-empty sibling exists.
 */
static int block_tree_sibling_index(int index)
{
    return !index ? 1 : index - 1;
}

/**
 * block_tree_get_sibling_block - Get block and mac for sibling block
 * @tr:             Transaction object.
 * @path:           Tree path object.
 *
 * Return: block_mac of sibling block selected by block_tree_sibling_index
 */
static struct block_mac block_tree_get_sibling_block(struct transaction *tr,
                                                     struct block_tree_path *path)
{
    struct block_mac block_mac;
    const struct block_mac *parent;
    const struct block_mac *block_mac_ptr;
    int parent_index;
    const struct block_tree_node *node_ro;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);

    assert(path->tree);
    assert(path->count > 1);

    parent = &path->entry[path->count - 2].block_mac;
    parent_index = path->entry[path->count - 2].index;

    node_ro = block_get(tr, parent, NULL, &node_ref);
    if (!node_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    parent_index = block_tree_sibling_index(parent_index);
    block_mac_ptr = block_tree_node_get_child_data(path->tree, node_ro, parent_index);
    block_mac = *block_mac_ptr; /* TODO: support variable size block_mac */
    assert(block_mac_valid(tr, &block_mac));
    block_put(node_ro, &node_ref);
err:
    return block_mac;
}

/**
 * block_tree_path_set_sibling_block - Set path to point to sibling block_mac
 * @path:           Tree path object.
 * @block_mac:      Block-mac of sibling block to swap with path entry
 * @left:           %true if @block_mac is the left sibling on entry (right on
 *                  return) %false otherwise.
 *
 * Update @path to point to the left (if @left is %true) or right sibling node
 * @block_mac, and return the block_mac of the old node that @path pointed to in
 * @block_mac.
 *
 * This is used to toggle the path between two sibling nodes without re-reading
 * the parent node. It does not preserve the prev_key or next_key value that is
 * not shared beween the siblings.
 */
static void block_tree_path_set_sibling_block(struct block_tree_path *path,
                                              struct block_mac *block_mac,
                                              bool left)
{
    int parent_index;
    struct block_mac tmp;

    assert(path->count > 1);
    parent_index = path->entry[path->count - 2].index;
    //assert(left || parent_index <= 1); // assert only valid on initial call
    assert(!left || parent_index > 0);

    if (left) {
        parent_index--;
    } else {
        parent_index++;
    }

    if (print_internal_changes) {
        printf("%s: path[%d].index: %d -> %d\n",
               __func__, path->count - 2,
               path->entry[path->count - 2].index, parent_index);
        printf("%s: path[%d].block_mac.block: %lld -> %lld\n",
               __func__, path->count - 1,
               block_mac_to_block(path->tr, &path->entry[path->count - 1].block_mac),
               block_mac_to_block(path->tr, block_mac));
    }

    path->entry[path->count - 2].index = parent_index;
    if (left) {
        path->entry[path->count - 2].next_key = path->entry[path->count - 2].prev_key;
        path->entry[path->count - 2].prev_key = 0; //unknown
    } else {
        path->entry[path->count - 2].prev_key = path->entry[path->count - 2].next_key;
        path->entry[path->count - 2].next_key = 0; //unknown
    }

    tmp = *block_mac;
    *block_mac = path->entry[path->count - 1].block_mac;
    path->entry[path->count - 1].block_mac = tmp;

}

static void block_tree_node_merge(struct transaction *tr,
                                  struct block_tree_path *path);

/**
 * block_tree_remove_internal - Remove key and right child from internal node
 * @tr:             Transaction object.
 * @path:           Tree path object.
 *
 * Removes an entry from an internal node, and check the resulting node follows
 * B+ tree rules. If a root node would have no remaining entries, delete it. If
 * a non-root node has too few remaining entries, call block_tree_node_merge
 * which will either borrow an entry from, or merge with a sibling node.
 *
 * TODO: merge with block_tree_node_merge and avoid recursion?
 */
static void block_tree_remove_internal(struct transaction *tr,
                                       struct block_tree_path *path)
{
    const struct block_tree_node *node_ro;
    struct block_tree_node *node_rw;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);
    int index;
    bool need_merge;
    const struct block_mac *block_mac;

    assert(path->count);

    block_mac = &path->entry[path->count - 1].block_mac;
    index = path->entry[path->count - 1].index;
    node_ro = block_get(tr, block_mac, NULL, &node_ref);
    if (!node_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        return;
    }
    assert(!block_tree_node_is_leaf(node_ro));
    assert(index > 0);

    if (print_internal_changes) {
        printf("%s: @%d:", __func__, index);
        block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac), node_ro);
    }

    if (path->count == 1 && !block_tree_node_get_key(path->tree, ~0, node_ro, 1)) {
        assert(index == 1); /* block_tree_node_merge always removes right node */
        const struct block_mac *new_root = block_tree_node_get_child_data(path->tree, node_ro, 0);
        if (print_internal_changes) {
            printf("%s: root emptied, new root %lld\n",
                   __func__, block_mac_to_block(tr, new_root));
        }
        assert(block_mac_valid(tr, new_root));
        path->tree->root = *new_root;
        block_discard_dirty(node_ro);
        block_put(node_ro, &node_ref);
        assert(!path->tree->copy_on_write || path->tree->allow_copy_on_write);
        block_free_etc(tr, block_mac_to_block(tr, block_mac),
                       !path->tree->allow_copy_on_write);
        return;
    }

    node_rw = block_tree_block_dirty(tr, path, path->count - 1, node_ro);
    if (tr->failed) {
        block_put(node_ro, &node_ref);
        pr_warn("transaction failed, abort\n");
        return;
    }

    block_tree_node_shift_down(path->tree, node_rw, index - 1, index,
                               SHIFT_RIGHT_CHILD);

    if (print_internal_changes) {
        printf("%s: @%d done:", __func__, index);
        block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac), node_rw);
    }
    need_merge = path->count > 1 && block_tree_below_min_full(path->tree, node_rw);
    block_tree_path_put_dirty(tr, path, path->count - 1, node_rw, &node_ref);

    if (need_merge) {
        block_tree_node_merge(tr, path);
    }
}

/**
 * block_tree_node_merge - Merge node or borrow entry from sibling block
 * @tr:             Transaction object.
 * @path:           Tree path object.
 *
 * If sibling block has more entries than it needs, migrate an entry from it,
 * otherwise merge the two nodes and free the block that stored the right node.
 */
static void block_tree_node_merge(struct transaction *tr,
                                  struct block_tree_path *path)
{
    const struct block_tree_node *node_ro;
    const struct block_tree_node *merge_node_ro;
    struct block_tree_node *node_rw;
    struct block_tree_node *merge_node_rw;
    obj_ref_t node_ref1 = OBJ_REF_INITIAL_VALUE(node_ref1);
    obj_ref_t node_ref2 = OBJ_REF_INITIAL_VALUE(node_ref2);
    obj_ref_t *node_ref = &node_ref1;
    obj_ref_t *merge_node_ref = &node_ref2;
    const struct block_mac *block_mac;
    struct block_mac merge_block;
    uint src_index;
    uint dest_index;
    data_block_t key;
    data_block_t parent_key;
    int index;
    bool node_is_left;
    bool is_leaf;

    assert(path->count > 1);
    assert(path->tree);

    block_mac = &path->entry[path->count - 1].block_mac;
    node_is_left = !path->entry[path->count - 2].index;
    merge_block = block_tree_get_sibling_block(tr, path);

    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        return;
    }

    node_ro = block_get(tr, block_mac, NULL, node_ref);
    if (!node_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        return;
    }
    assert(node_ro);
    is_leaf = block_tree_node_is_leaf(node_ro);

    merge_node_ro = block_get(tr, &merge_block, NULL, merge_node_ref);
    if (!merge_node_ro) {
        assert(tr->failed);
        block_put(node_ro, node_ref);
        pr_warn("transaction failed, abort\n");
        return;
    }
    assert(merge_node_ro);
    assert(is_leaf == block_tree_node_is_leaf(merge_node_ro));

    if (print_internal_changes) {
        printf("%s: node_is_left %d\n", __func__, node_is_left);
        block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac), node_ro);
        block_tree_node_print(tr, path->tree, block_mac_to_block(tr, &merge_block), merge_node_ro);
    }

    assert(block_tree_below_min_full(path->tree, node_ro));
    assert(!block_tree_below_min_full(path->tree, merge_node_ro));
    if (block_tree_above_min_full(path->tree, merge_node_ro)) {
        /* migrate entries instead */
        block_tree_path_set_sibling_block(path, &merge_block, !node_is_left);
        assert(!tr->failed);
        merge_node_rw = block_tree_block_dirty(tr, path, path->count - 1, merge_node_ro);
        block_tree_path_set_sibling_block(path, &merge_block, node_is_left);
        if (!merge_node_rw) {
             assert(tr->failed);
             block_put(node_ro, node_ref);
             block_put(merge_node_ro, merge_node_ref);
             pr_warn("transaction failed, abort\n");
             return;
        }
        assert(!block_tree_node_need_copy(tr, path->tree, &merge_block));

        node_rw = block_dirty(tr, node_ro, !path->tree->copy_on_write);
        assert(!block_tree_node_need_copy(tr, path->tree, block_mac));

        if (node_is_left) {
            src_index = 0;
            dest_index = block_tree_node_min_full_index(path->tree, node_rw);
        } else {
            src_index = block_tree_node_get_key_count(path->tree, merge_node_rw) - 1;
            dest_index = 0;
        }

        key = block_tree_node_get_key(path->tree, ~0, merge_node_rw, src_index);
        if (node_is_left && is_leaf) {
            parent_key = block_tree_node_get_key(path->tree, ~0, merge_node_rw, 1);
        } else {
            parent_key = key;
        }
        if (!is_leaf) {
            if (node_is_left) {
                key = path->entry[path->count - 2].next_key;
            } else {
                key = path->entry[path->count - 2].prev_key;
            }
        }

        block_tree_node_insert(path->tree, node_rw, dest_index,
                               (node_is_left && !is_leaf) ?
                               SHIFT_RIGHT_CHILD : SHIFT_LEAF_OR_LEFT_CHILD,
                               &key,
                               block_tree_node_get_child_data(path->tree, merge_node_rw,
                                                              src_index +
                                                              (!node_is_left && !is_leaf)),
                               NULL, NULL);

        block_tree_node_shift_down(path->tree, merge_node_rw, src_index, src_index + 1,
                                   (node_is_left || is_leaf) ?
                                   SHIFT_LEAF_OR_LEFT_CHILD : SHIFT_RIGHT_CHILD);

        if (node_is_left) {
            if (dest_index == 0 && is_leaf) {
                data_block_t key0;
                key0 = block_tree_node_get_key(path->tree, ~0, node_rw, 0);
                assert(key0);
                block_tree_update_key(tr, path, path->count - 2, key0);
            }
            block_tree_path_set_sibling_block(path, &merge_block, !node_is_left);
        }
        block_tree_update_key(tr, path, path->count - 2, parent_key);
        if (node_is_left) {
            block_tree_path_set_sibling_block(path, &merge_block, node_is_left);
        }
        if (print_internal_changes) {
            printf("%s: %lld %lld done\n",
                   __func__, block_mac_to_block(tr, block_mac),
                   block_mac_to_block(tr, &merge_block));
            block_tree_node_print(tr, path->tree, block_mac_to_block(tr, block_mac), node_rw);
            block_tree_node_print(tr, path->tree, block_mac_to_block(tr, &merge_block), merge_node_rw);
        }
        block_tree_path_put_dirty(tr, path, path->count - 1,
                                  node_rw, node_ref);
        block_tree_path_set_sibling_block(path, &merge_block, !node_is_left);
        block_tree_path_put_dirty(tr, path, path->count - 1,
                                  merge_node_rw, merge_node_ref);
        block_tree_path_set_sibling_block(path, &merge_block, node_is_left);
    } else {
        const struct block_mac *left;
        const struct block_mac *right;
        data_block_t *merge_key;
        if (!node_is_left) {
            const struct block_tree_node *tmp_node;
            obj_ref_t *tmp_ref;
            tmp_node = node_ro;
            node_ro = merge_node_ro;
            merge_node_ro = tmp_node;
            tmp_ref = node_ref;
            node_ref = merge_node_ref;
            merge_node_ref = tmp_ref;
            block_tree_path_set_sibling_block(path, &merge_block, true);
        }
        left = block_mac;
        right = &merge_block;
        node_rw = block_tree_block_dirty(tr, path, path->count - 1, node_ro);
        if (!node_rw) {
             assert(tr->failed);
             block_put(node_ro, node_ref);
             block_put(merge_node_ro, merge_node_ref);
             pr_warn("transaction failed, abort\n");
             return;
        }
        assert(!block_tree_node_need_copy(tr, path->tree, left));

        index = block_tree_node_get_key_count(path->tree, node_rw);
        if (is_leaf) {
            merge_key = NULL;
        } else {
            merge_key = &path->entry[path->count - 2].next_key;
            assert(*merge_key);
        }
        block_tree_node_merge_entries(path->tree, node_rw, merge_node_ro, index,
                                      block_tree_node_get_key_count(path->tree, merge_node_ro),
                                      merge_key);
        assert(block_tree_node_check(tr, path->tree, node_rw, block_mac_to_block(tr, left), 0, ~0));

        if (is_leaf
            && !block_tree_node_min_full_index(path->tree, node_rw)
            && !index) {
            data_block_t key0;
            key0 = block_tree_node_get_key(path->tree, ~0, node_rw, 0);
            assert(key0);
            if (print_internal_changes) {
                printf("hack, special case for order <= 4 tree\n");
            }
            block_tree_update_key(tr, path, path->count - 2, key0);
        }

        if (print_internal_changes) {
            printf("%s: merge done, free %lld\n",
                   __func__, block_mac_to_block(tr, right));
            block_tree_node_print(tr, path->tree, block_mac_to_block(tr, left), node_rw);
        }

        block_tree_path_put_dirty(tr, path, path->count - 1,
                                  node_rw, node_ref);
        block_discard_dirty(merge_node_ro);
        block_put(merge_node_ro, merge_node_ref);
        block_tree_path_set_sibling_block(path, &merge_block, false);
        path->count--;
        block_tree_remove_internal(tr, path);
        block_free_etc(tr, block_mac_to_block(tr, block_mac),
                       !path->tree->allow_copy_on_write);
    }
}

/**
 * block_tree_insert_block_mac - Insert an entry into a B+ tree
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @key:            Key of new entry.
 * @data:           Data of new entry.
 *
 * Find a valid insert point for @key and insert @key and @data there. If this
 * node becomes overfull, split it.
 *
 * TODO: use a void * for data and merge with block_tree_insert.
 * TODO: Allow insert by path
 */
void block_tree_insert_block_mac(struct transaction *tr, struct block_tree *tree,
                                 data_block_t key, struct block_mac data)
{
    const struct block_tree_node *node_ro;
    struct block_tree_node *node_rw;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);
    const struct block_mac *block_mac;
    data_block_t overflow_key = 0;
    struct block_mac overflow_data;
    int index;
    struct block_tree_path path;

    assert(!tr->failed);
    assert(!tree->updating);
    assert(key);
    assert(block_mac_valid(tr, &data));

    tree->updating = true;

    if (!block_mac_valid(tr, &tree->root)) {
        assert(!tree->copy_on_write || tree->allow_copy_on_write);
        block_mac_set_block(tr, &tree->root, block_allocate_etc(tr, !tree->allow_copy_on_write));
        if (tr->failed) {
            pr_warn("transaction failed, abort\n");
            goto err;
        }
        if (print_internal_changes) {
            printf("%s: new root block %lld\n",
                   __func__, block_mac_to_block(tr, &tree->root));
        }
        assert(block_mac_valid(tr, &tree->root));
        node_rw = block_get_cleared(tr, block_mac_to_block(tr, &tree->root),
                                    !tree->allow_copy_on_write, &node_ref);
        node_rw->is_leaf = true;
        block_put_dirty(tr, node_rw, &node_ref, &tree->root, NULL);
        tree->root_block_changed = true;
    }

    block_tree_walk(tr, tree, key, false, &path);
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }

    assert(path.count > 0);

    block_mac = &path.entry[path.count - 1].block_mac;
    index = path.entry[path.count - 1].index;

    node_ro = block_get(tr, block_mac, NULL, &node_ref);
    if (!node_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        goto err;
    }

    if (print_changes) {
        printf("%s: key %lld, data.block %lld, index %d\n",
               __func__, key, block_mac_to_block(tr, &data), index);
        assert(node_ro);
        block_tree_node_print(tr, tree, block_mac_to_block(tr, block_mac), node_ro);
    }

    assert(node_ro);
    assert(block_tree_node_is_leaf(node_ro));
    assert(index || !path.entry[path.count - 1].prev_key ||
           block_tree_node_get_key(tree, ~0, node_ro, index) == key);

    node_rw = block_tree_block_dirty(tr, &path, path.count - 1, node_ro);
    if (!node_rw) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        block_put(node_ro, &node_ref);
        goto err;
    }

    block_tree_node_insert(tree, node_rw, index, SHIFT_LEAF_OR_LEFT_CHILD,
                           &key, &data, &overflow_key, &overflow_data);

    block_tree_path_put_dirty(tr, &path, path.count - 1, node_rw, &node_ref);

    if (overflow_key) {
        assert(block_mac_valid(tr, &overflow_data));
        block_tree_node_split(tr, &path, overflow_key, NULL, &overflow_data);
    }

    if (print_changes_full_tree) {
        printf("%s: key %lld, data.block %lld, done:\n",
               __func__, key, block_mac_to_block(tr, &data));
        block_tree_print(tr, tree);
    }

err:
    tree->update_count++;
    tree->updating = false;
    full_assert(block_tree_check(tr, tree));
}

/**
 * block_tree_insert - Insert a data_block_t type entry into a B+ tree
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @key:            Key of new entry.
 * @data_block:     Data of new entry.
 */
void block_tree_insert(struct transaction *tr, struct block_tree *tree,
                       data_block_t key, data_block_t data_block)
{
    struct block_mac data = BLOCK_MAC_INITIAL_VALUE(data);

    block_mac_set_block(tr, &data, data_block);

    block_tree_insert_block_mac(tr, tree, key, data);
}

/**
 * block_tree_remove - Remove an entry from a B+ tree
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @key:            Key of entry to remove.
 * @data:           Data of entry to remove.
 *
 * Find an entry in the tree where the key matches @key, and the start of data
 * matches @data, and remove it from the tree. Call block_tree_node_merge if
 * the resulting node does follow B+ tree rules.
 *
 * TODO: Remove by by path instead
 */
void block_tree_remove(struct transaction *tr, struct block_tree *tree,
                       data_block_t key, data_block_t data)
{
    struct block_tree_path path;
    const struct block_tree_node *node_ro;
    struct block_tree_node *node_rw;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);
    const struct block_mac *block_mac;
    int index;
    bool need_merge = false;
    data_block_t new_parent_key;

    assert(!tr->failed);
    assert(!tree->updating);
    assert(block_mac_valid(tr, &tree->root));
    full_assert(block_tree_check(tr, tree));
    assert(key);
    assert(data);

    tree->updating = true;

    block_tree_walk(tr, tree, key, false, &path); /* TODO: make writeable */
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    assert(path.count > 0);

    if (block_mac_to_block(tr, &path.data) != data) {
        block_tree_walk(tr, tree, key - 1, true, &path); /* TODO: make path writeable after finding maching entry */
        while (block_mac_to_block(tr, &path.data) != data || block_tree_path_get_key(&path) != key) {
            assert(block_tree_path_get_key(&path));
            block_tree_path_next(&path);
        }
    }

    block_mac = &path.entry[path.count - 1].block_mac;
    index = path.entry[path.count - 1].index;

    node_ro = block_get(tr, block_mac, NULL, &node_ref);
    if (!node_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    assert(block_tree_node_is_leaf(node_ro));
    assert(block_tree_node_get_key(tree, ~0, node_ro, index) == key);
    assert(!memcmp(block_tree_node_get_child_data(tree, node_ro, index), &data,
           MIN(sizeof(data), tr->fs->block_num_size)));

    if (print_changes) {
        printf("%s: key %lld, data %lld, index %d\n",
               __func__, key, data, index);
        block_tree_node_print(tr, tree, block_mac_to_block(tr, block_mac), node_ro);
    }

    node_rw = block_tree_block_dirty(tr, &path, path.count - 1, node_ro);
    if (tr->failed) {
        block_put(node_ro, &node_ref);
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    assert(node_rw);
    block_tree_node_leaf_remove_entry(tr, tree, block_mac, node_rw, index);
    if (path.count > 1) {
        if (!index) {
            new_parent_key = block_tree_node_get_key(tree, ~0, node_rw, 0);
            assert(new_parent_key || !block_tree_node_min_full_index(tree, node_rw));
            if (new_parent_key) {
                block_tree_update_key(tr, &path, path.count - 2, new_parent_key);
            }
        }
        need_merge = block_tree_below_min_full(tree, node_rw);
    }
    block_tree_path_put_dirty(tr, &path, path.count - 1, node_rw, &node_ref);
    if (need_merge) {
        block_tree_node_merge(tr, &path);
    }

    if (print_changes_full_tree) {
        printf("%s: key %lld, data %lld, done:\n", __func__, key, data);
        block_tree_print(tr, tree);
    }

err:
    tree->update_count++;
    tree->updating = false;
    full_assert(block_tree_check(tr, tree));
}

/**
 * block_tree_update_block_mac - Update key or data of an existing B+ tree entry
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @old_key:        Key of entry to update.
 * @old_data:       Data of entry to update.
 * @new_key:        New key of entry.
 * @new_data:       New data of entry.
 *
 * Find an entry in the tree where the key matches @key, and the start of data
 * matches @data, and update it's key or data. When updating key, the new key
 * must not cause the entry to move in the tree.
 *
 * TODO: Update by path instead
 */
void block_tree_update_block_mac(struct transaction *tr, struct block_tree *tree,
                                 data_block_t old_key, struct block_mac old_data,
                                 data_block_t new_key, struct block_mac new_data)
{
    struct block_tree_path path;
    const struct block_tree_node *node_ro;
    struct block_tree_node *node_rw;
    obj_ref_t node_ref = OBJ_REF_INITIAL_VALUE(node_ref);
    const struct block_mac *block_mac;
    data_block_t prev_key;
    data_block_t next_key;
    uint index;
    size_t max_key_count;

    assert(!tr->failed);
    assert(!tree->updating);
    assert(block_mac_valid(tr, &tree->root));
    full_assert(block_tree_check(tr, tree));
    assert(old_key);
    assert(block_mac_valid(tr, &old_data));
    assert(new_key);
    assert(block_mac_valid(tr, &new_data));
    assert(old_key == new_key || block_mac_same_block(tr, &old_data, &new_data));
    assert(old_key != new_key || !block_mac_same_block(tr, &old_data, &new_data));

    tree->updating = true;

    block_tree_walk(tr, tree, old_key - 1, true, &path);
    prev_key = block_tree_path_get_key(&path);
    if (prev_key == old_key && block_mac_same_block(tr, &path.data, &old_data)) { /* modify leftmost entry in tree */
        prev_key = 0;
    }

    block_tree_walk(tr, tree, old_key, false, &path); /* TODO: make writeable */
    if (tr->failed) {
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    assert(path.count > 0);

    if (!block_mac_same_block(tr, &path.data, &old_data)) {
        block_tree_walk(tr, tree, old_key - 1, true, &path); /* TODO: make path writeable after finding maching entry */
        while (!block_mac_same_block(tr, &path.data, &old_data) ||
               block_tree_path_get_key(&path) != old_key) {
            assert(block_tree_path_get_key(&path));
            block_tree_path_next(&path);
        }
    }

    block_mac = &path.entry[path.count - 1].block_mac;
    index = path.entry[path.count - 1].index;

    node_ro = block_get(tr, block_mac, NULL, &node_ref);
    if (!node_ro) {
        assert(tr->failed);
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    max_key_count = block_tree_node_max_key_count(tree, node_ro);
    assert(block_tree_node_is_leaf(node_ro));
    assert(block_tree_node_get_key(tree, ~0, node_ro, index) == old_key);
    assert(block_mac_same_block(tr, block_tree_node_get_child_data(tree, node_ro, index), &old_data));

    if (print_changes) {
        printf("%s: key %lld->%lld, data %lld->%lld, index %d\n",
               __func__, old_key, new_key, block_mac_to_block(tr, &old_data),
               block_mac_to_block(tr, &new_data), index);
        block_tree_node_print(tr, tree, block_mac_to_block(tr, block_mac), node_ro);
    }

    next_key = (index + 1 < max_key_count) ?
               block_tree_node_get_key(tree, ~0, node_ro, index + 1) : 0;
    if (path.count > 1 && !next_key) {
        next_key = path.entry[path.count - 2].next_key;
    }

    node_rw = block_tree_block_dirty(tr, &path, path.count - 1, node_ro);
    if (tr->failed) {
        block_put(node_ro, &node_ref);
        pr_warn("transaction failed, abort\n");
        goto err;
    }
    assert(node_rw);

    if (old_key == new_key) {
        assert(tree->child_data_size[1] <= sizeof(new_data));
        memcpy(block_tree_node_get_child_data_rw(tree, node_rw, index),
               &new_data, tree->child_data_size[1]);
    } else if (new_key >= prev_key &&
               (new_key <= next_key || !next_key)) {
        block_tree_node_set_key(tree, node_rw, index, new_key);
        if (!index) {
            path.count--;
            assert(new_key);
            assert(new_key == block_tree_node_get_key(tree, ~0, node_rw, index));
            block_tree_update_key(tr, &path, path.count - 1, new_key);
            path.count++;
        }
    } else {
        assert(0); /* moving entries with block_tree_update is not supported */
    }
    block_tree_path_put_dirty(tr, &path, path.count - 1, node_rw, &node_ref);

    if (print_changes_full_tree) {
        printf("%s: key %lld->%lld, data %lld->%lld, done:\n",
               __func__, old_key, new_key, block_mac_to_block(tr, &old_data),
               block_mac_to_block(tr, &new_data));
        block_tree_print(tr, tree);
    }

err:
    tree->update_count++;
    tree->updating = false;
    full_assert(block_tree_check(tr, tree));
}

/**
 * block_tree_update_block_mac - Update key or data of an existing B+ tree entry
 * @tr:             Transaction object.
 * @tree:           Tree object.
 * @old_key:        Key of entry to update.
 * @old_data_block: Data of entry to update.
 * @new_key:        New key of entry.
 * @new_data_block: New data of entry.
 *
 * Same as block_tree_update_block_mac but data of type data_block_t.
 *
 * TODO: Merge with block_tree_update_block_mac
 */
void block_tree_update(struct transaction *tr, struct block_tree *tree,
                       data_block_t old_key, data_block_t old_data_block,
                       data_block_t new_key, data_block_t new_data_block)
{
    struct block_mac old_data = BLOCK_MAC_INITIAL_VALUE(old_data);
    struct block_mac new_data = BLOCK_MAC_INITIAL_VALUE(new_data);

    block_mac_set_block(tr, &old_data, old_data_block);
    block_mac_set_block(tr, &new_data, new_data_block);

    block_tree_update_block_mac(tr, tree, old_key, old_data, new_key, new_data);
}

/**
 * block_tree_init - Initialize in-memory state of block backed B+ tree
 * @tree:           Tree object.
 * @block_size:     Block size.
 * @key_size:       Key size. [1-8].
 * @child_size:     Child size. [@key_size-24].
 * @data_size:      Data size. [1-24].
 *
 * TODO: set copy-on-write flags.
 */
void block_tree_init(struct block_tree *tree,
                     size_t block_size,
                     size_t key_size,
                     size_t child_size,
                     size_t data_size)
{
    memset(tree, 0, sizeof(*tree));
    block_tree_set_sizes(tree, block_size, key_size, child_size, data_size);
}

/**
 * block_tree_init - Initialize in-memory state of copy-on-write B+ tree
 * @dst:        New tree object.
 * @src:        Source tree object.
 *
 * Initialize @dst to point to the same tree as @src, but set flag to enable
 * copy-on-write so all nodes that need changes get copied to new blocks before
 * those changes are made.
 */
void block_tree_copy(struct block_tree *dst, const struct block_tree *src)
{
    assert(src->copy_on_write);
    *dst = *src;
    dst->allow_copy_on_write = true;
}


#if BUILD_STORAGE_TEST
static double log_n(double n, double x)
{
    return log(x) / log(n);
}

static bool block_tree_max_depth_needed;

void block_tree_check_config(struct block_device *dev)
{
    struct block_tree tree[1];
    int node_internal_min_key_count;
    int node_internal_max_key_count;
    int node_min_child_count;
    int node_max_child_count;
    int node_leaf_min_key_count;
    int node_leaf_max_key_count;
    double tree_min_entry_count;
    double tree_max_entry_count;
    int depth;
    int max_entries_needed = (dev->block_count + 1) / 2;
    int depth_needed;

    block_tree_set_sizes(tree, dev->block_size, sizeof(data_block_t),
                         sizeof(struct block_mac), sizeof(struct block_mac));

    node_internal_min_key_count = block_tree_min_key_count(tree, false);
    node_internal_max_key_count = block_tree_max_key_count(tree, false);
    node_min_child_count = node_internal_min_key_count + 1;
    node_max_child_count = node_internal_max_key_count + 1;
    node_leaf_min_key_count = block_tree_min_key_count(tree, true);
    node_leaf_max_key_count = block_tree_max_key_count(tree, true);

    printf("block_tree config\n");
    printf("internal min key count: %6d\n", node_internal_min_key_count);
    printf("internal max key count: %6d\n", node_internal_max_key_count);
    printf("min child count:        %6d\n", node_min_child_count);
    printf("max child count:        %6d\n", node_max_child_count);
    printf("leaf min key count:     %6d\n", node_leaf_min_key_count);
    printf("leaf max key count:     %6d\n", node_leaf_max_key_count);
    printf("block size:             %6zd\n", dev->block_size);
    printf("block count:            %6lld\n", dev->block_count);
    printf("max entries needed:     %6d\n", max_entries_needed);
    printf("max depth:              %6d\n", BLOCK_TREE_MAX_DEPTH);
    for (depth = 1; depth <= BLOCK_TREE_MAX_DEPTH; depth++) {
        if (depth == 1) {
            tree_min_entry_count = 0;
            tree_max_entry_count = node_leaf_max_key_count;
        } else if (depth == 2) {
            tree_min_entry_count = node_leaf_min_key_count * 2;
        } else {
            tree_min_entry_count *= node_min_child_count;
        }
        tree_max_entry_count *= node_max_child_count;
        printf("depth %2d: entry count range: %20.15g - %20.15g (alt. %15.10g - %15.10g)\n",
               depth, tree_min_entry_count, tree_max_entry_count,
               2.0 * pow(ceil(node_max_child_count/2.0), depth - 1),
               pow(node_max_child_count, depth) - pow(node_max_child_count, depth - 1));
    }
    depth_needed = ceil(log_n(ceil(node_max_child_count / 2.0), ceil(max_entries_needed / 2.0))) + 1;
    printf("est. tree depth needed for %u entries: %d\n", max_entries_needed, depth_needed);
    printf("est. tree depth needed for %llu entries: %g\n", ((data_block_t)~0 / dev->block_size),
           ceil(log_n(ceil(node_max_child_count / 2.0), ceil(((data_block_t)~0 / dev->block_size) / 2.0)) + 1.0));
    printf("est. tree depth needed for %llu entries: %g\n", ((data_block_t)~0),
           ceil(log_n(ceil(node_max_child_count / 2.0), ceil(((data_block_t)~0) / 2.0)) + 1.0));
    assert(tree_min_entry_count >= max_entries_needed);
    if (tree_min_entry_count / node_min_child_count < max_entries_needed) {
        block_tree_max_depth_needed = true;
    }
}

void block_tree_check_config_done(void)
{
    assert(block_tree_max_depth_needed);
}

#endif
