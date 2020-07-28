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

#include <err.h>
#include <errno.h>
#include <compiler.h>
#include <stdint.h>
#include <string.h>
#include <trusty_ipc.h>

#include <interface/storage/storage.h>

#include "block_device_tipc.h"
#include "block_cache.h"
#include "client_tipc.h"
#include "ipc.h"
#include "tipc_ns.h"
#include "rpmb.h"

#ifdef APP_STORAGE_RPMB_BLOCK_SIZE
#define BLOCK_SIZE_RPMB (APP_STORAGE_RPMB_BLOCK_SIZE)
#else
#define BLOCK_SIZE_RPMB (512)
#endif
#ifdef APP_STORAGE_RPMB_BLOCK_COUNT
#define BLOCK_COUNT_RPMB (APP_STORAGE_RPMB_BLOCK_COUNT)
#else
#define BLOCK_COUNT_RPMB (0) /* Auto detect */
#endif
#ifdef APP_STORAGE_MAIN_BLOCK_SIZE
#define BLOCK_SIZE_MAIN  (APP_STORAGE_MAIN_BLOCK_SIZE)
#else
#define BLOCK_SIZE_MAIN (2048)
#endif
#ifdef APP_STORAGE_MAIN_BLOCK_COUNT
#define BLOCK_COUNT_MAIN (APP_STORAGE_MAIN_BLOCK_COUNT)
#else
#define BLOCK_COUNT_MAIN (0x10000000000 / BLOCK_SIZE_MAIN)
#endif

#ifdef MTK_TEE_GP_SUPPORT
/*
 * Co-exist RPMB partition with GP
 *
 * Please make sure GP do not overlap this region
 *
 * ex. RPMB 4MB, total block 16384
 *
 *      RPMB partition
 * +---------------------+ LBA: 0
 * |   GP RPMB region    |
 * |                     |
 * |      ....           |
 * +---------------------+ LBA: 16384 - (RPMB_GZ_PART_SIZE_KB / 256)
 * |   GZ RPMB region    |
 * |                     |
 * +---------------------+
 */
#define RPMB_GZ_PART_SIZE_KB 128
#endif

#define BLOCK_SIZE_RPMB_BLOCKS (BLOCK_SIZE_RPMB / RPMB_BUF_SIZE)

STATIC_ASSERT(BLOCK_SIZE_RPMB_BLOCKS == 1 || BLOCK_SIZE_RPMB_BLOCKS == 2);
STATIC_ASSERT(BLOCK_SIZE_RPMB_BLOCKS * RPMB_BUF_SIZE == BLOCK_SIZE_RPMB);

STATIC_ASSERT(BLOCK_COUNT_RPMB == 0 || BLOCK_COUNT_RPMB >= 8);

STATIC_ASSERT(BLOCK_SIZE_MAIN >= 256);
STATIC_ASSERT(BLOCK_COUNT_MAIN >= 8);
STATIC_ASSERT(BLOCK_SIZE_MAIN >= BLOCK_SIZE_RPMB);

#define SS_ERR(args...)  fprintf(stderr, "ss: " args)
#define SS_WARN(args...)  fprintf(stderr, "ss: " args)
#define SS_DBG_IO(args...)  do {} while(0)

static int rpmb_check(struct block_device_tipc *state, uint16_t block)
{
    int ret;
    uint8_t tmp[RPMB_BUF_SIZE];
    ret = rpmb_read(state->rpmb_state, tmp, block, 1);
    SS_DBG_IO("%s: check rpmb_block %d, ret %d\n",
              __func__, block, ret);
    return ret;
}

static uint32_t rpmb_search_size(struct block_device_tipc *state, uint16_t hint)
{
    int ret;
    uint32_t low = 0;
    uint16_t high = ~0;
    uint16_t curr = hint - 1;

    while (low <= high) {
        ret = rpmb_check(state, curr);
        switch (ret) {
        case 0:
            low = curr + 1;
            break;
        case -ENOENT:
            high = curr - 1;
            break;
        default:
            return 0;
        };
        if (ret || curr != hint) {
            curr = low + (high - low) / 2;
            hint = curr;
        } else {
            curr = curr + 1;
        }
    }
    assert ((uint32_t)high + 1 == low);
    state->rpmb_total_blocks = low;
    return low;
}

struct block_device_rpmb *dev_rpmb_to_state(struct block_device *dev)
{
    assert(dev);
    return containerof(dev, struct block_device_rpmb, dev);
}

static void block_device_tipc_rpmb_start_read(struct block_device *dev,
                                              data_block_t block)
{
    int ret;
    uint8_t tmp[BLOCK_SIZE_RPMB] = {0}; /* TODO: pass data in? */
    uint16_t rpmb_block;
    struct block_device_rpmb *dev_rpmb = dev_rpmb_to_state(dev);

    assert(block < dev->block_count);
    rpmb_block = block + dev_rpmb->base;

    ret = rpmb_read(dev_rpmb->state->rpmb_state, tmp,
                    rpmb_block * BLOCK_SIZE_RPMB_BLOCKS,
                    BLOCK_SIZE_RPMB_BLOCKS);

    SS_DBG_IO("%s: block %lld, base %d, rpmb_block %d, ret %d\n",
              __func__, block, dev_rpmb->base, rpmb_block, ret);

    block_cache_complete_read(dev, block, tmp, BLOCK_SIZE_RPMB, !!ret);
}

static void block_device_tipc_rpmb_start_write(struct block_device *dev,
                                               data_block_t block,
                                               const void *data,
                                               size_t data_size)
{
    int ret;
    uint16_t rpmb_block;
    struct block_device_rpmb *dev_rpmb = dev_rpmb_to_state(dev);

    assert(data_size == BLOCK_SIZE_RPMB);
    assert(block < dev->block_count);

    rpmb_block = block + dev_rpmb->base;

    ret = rpmb_write(dev_rpmb->state->rpmb_state, data,
                     rpmb_block * BLOCK_SIZE_RPMB_BLOCKS,
                     BLOCK_SIZE_RPMB_BLOCKS, true);

    SS_DBG_IO("%s: block %lld, base %d, rpmb_block %d, ret %d\n",
              __func__, block, dev_rpmb->base, rpmb_block, ret);

    block_cache_complete_write(dev, block, !!ret);
}

static void block_device_tipc_rpmb_wait_for_io(struct block_device *dev)
{
    assert(0); /* TODO: use async read/write */
}


static struct block_device_tipc *dev_ns_to_state(struct block_device *dev)
{
    assert(dev);
    return containerof(dev, struct block_device_tipc, dev_ns);
}

static void block_device_tipc_ns_start_read(struct block_device *dev, data_block_t block)
{
    int ret;
    uint8_t tmp[BLOCK_SIZE_MAIN]; /* TODO: pass data in? */
    struct block_device_tipc *state = dev_ns_to_state(dev);

    ret = ns_read_pos(state->ipc_handle, state->ns_handle,
                      block * BLOCK_SIZE_MAIN, tmp, BLOCK_SIZE_MAIN);
    SS_DBG_IO("%s: block %lld, ret %d\n", __func__, block, ret);
    block_cache_complete_read(dev, block, tmp,
                              BLOCK_SIZE_MAIN, ret != BLOCK_SIZE_MAIN);
}

static void block_device_tipc_ns_start_write(struct block_device *dev,
                                             data_block_t block,
                                             const void *data,
                                             size_t data_size)
{
    int ret;
    struct block_device_tipc *state = dev_ns_to_state(dev);

    assert(data_size == BLOCK_SIZE_MAIN);

    ret = ns_write_pos(state->ipc_handle, state->ns_handle,
                       block * BLOCK_SIZE_MAIN, data, data_size);
    SS_DBG_IO("%s: block %lld, ret %d\n", __func__, block, ret);
    block_cache_complete_write(dev, block, ret != BLOCK_SIZE_MAIN);
}

static void block_device_tipc_ns_wait_for_io(struct block_device *dev)
{
    assert(0); /* TODO: use async read/write */
}

static void block_device_tipc_init_dev_rpmb(struct block_device_rpmb *dev_rpmb,
                                            struct block_device_tipc *state,
                                            uint16_t base,
                                            uint32_t block_count)
{
    dev_rpmb->dev.start_read = block_device_tipc_rpmb_start_read;
    dev_rpmb->dev.start_write = block_device_tipc_rpmb_start_write;
    dev_rpmb->dev.wait_for_io = block_device_tipc_rpmb_wait_for_io;
    dev_rpmb->dev.block_count = block_count;
    dev_rpmb->dev.block_size = BLOCK_SIZE_RPMB;
    dev_rpmb->dev.block_num_size = 2;
    dev_rpmb->dev.mac_size = 2;
    dev_rpmb->dev.tamper_detecting = true;
    list_initialize(&dev_rpmb->dev.io_ops);
    dev_rpmb->state = state;
    dev_rpmb->base = base;
}

int block_device_tipc_init(struct block_device_tipc *state,
                           handle_t ipc_handle,
                           const struct key *fs_key,
                           const struct rpmb_key *rpmb_key)
{
    int ret;
    bool new_ns_fs;
    uint8_t dummy;
    uint32_t rpmb_block_count;
#ifdef RPMB_GZ_PART_SIZE_KB
    uint32_t rpmb_part_base; /* the start base of trusty */
#endif
    uint32_t rpmb_part1_block_count = 2;
    uint16_t rpmb_part1_base = 1; /* TODO: change to 0 and overwrite old fs */
    uint16_t rpmb_part2_base = rpmb_part1_base + rpmb_part1_block_count;

    state->ipc_handle = ipc_handle;

    /* init rpmb */
    ret = rpmb_init(&state->rpmb_state, &state->ipc_handle, rpmb_key);
    if (ret < 0) {
        SS_ERR("%s: rpmb_init failed (%d)\n", __func__, ret);
        goto err_rpmb_init;
    }

    if (BLOCK_COUNT_RPMB) {
        rpmb_block_count = BLOCK_COUNT_RPMB;
        ret = rpmb_check(state, rpmb_block_count * BLOCK_SIZE_RPMB_BLOCKS - 1);
        if (ret) {
            SS_ERR("%s: bad static rpmb size, %d\n", __func__, rpmb_block_count);
            goto err_bad_rpmb_size;
        }
    } else {
        rpmb_block_count = rpmb_search_size(state, 0); /* TODO: get hint from ns */
        rpmb_block_count /= BLOCK_SIZE_RPMB_BLOCKS;
    }

#ifdef RPMB_GZ_PART_SIZE_KB
    rpmb_part_base = rpmb_block_count - (RPMB_GZ_PART_SIZE_KB * 1024 / BLOCK_SIZE_RPMB);
    /* adjust the base address */
    rpmb_part2_base += rpmb_part_base;
#endif

    if (rpmb_block_count < rpmb_part2_base) {
        ret = -1;
        SS_ERR("%s: bad rpmb size, %d\n", __func__, rpmb_block_count);
        goto err_bad_rpmb_size;
    }

    block_device_tipc_init_dev_rpmb(&state->dev_rpmb, state, rpmb_part2_base,
                                    rpmb_block_count - rpmb_part2_base);

    /* TODO: allow non-rpmb based tamper proof storage */
    ret = fs_init(&state->tr_state_rpmb, fs_key,
                  &state->dev_rpmb.dev, &state->dev_rpmb.dev, false);
    if (ret < 0) {
        goto err_init_tr_state_rpmb;
    }

    state->fs_rpmb.tr_state = &state->tr_state_rpmb;

    ret = client_create_port(&state->fs_rpmb.client_ctx,
                             STORAGE_CLIENT_TP_PORT);
    if (ret < 0) {
        goto err_fs_rpmb_create_port;
    }

    state->fs_rpmb_boot.tr_state = &state->tr_state_rpmb;

    ret = client_create_port(&state->fs_rpmb_boot.client_ctx,
                             STORAGE_CLIENT_TDEA_PORT);
    if (ret < 0) {
        goto err_fs_rpmb_boot_create_port;
    }

    state->dev_ns.start_read = block_device_tipc_ns_start_read;
    state->dev_ns.start_write = block_device_tipc_ns_start_write;
    state->dev_ns.wait_for_io = block_device_tipc_ns_wait_for_io;
    state->dev_ns.block_count = BLOCK_COUNT_MAIN;
    state->dev_ns.block_size = BLOCK_SIZE_MAIN;
    state->dev_ns.block_num_size = sizeof(data_block_t);
    state->dev_ns.mac_size = sizeof(struct mac);
    list_initialize(&state->dev_ns.io_ops);

    ret = ns_open_file(state->ipc_handle, "0", &state->ns_handle, true);
    if (ret < 0) {
        /* RPMB fs only */
        state->dev_ns.block_count = 0;
        return 0;
    }

    /* Request empty file system if file is empty */
    ret = ns_read_pos(state->ipc_handle, state->ns_handle, 0,
                      &dummy, sizeof(dummy));
    new_ns_fs = ret < (int)sizeof(dummy);

    state->fs_ns.tr_state = &state->tr_state_ns;

#ifdef RPMB_GZ_PART_SIZE_KB
    /* adjust the base address */
    rpmb_part1_base += rpmb_part_base;
#endif

    block_device_tipc_init_dev_rpmb(&state->dev_ns_rpmb, state,
                                    rpmb_part1_base, rpmb_part1_block_count);

    ret = fs_init(&state->tr_state_ns, fs_key,
                  &state->dev_ns, &state->dev_ns_rpmb.dev, new_ns_fs);
    if (ret < 0) {
        goto err_init_fs_ns_tr_state;
    }

    ret = client_create_port(&state->fs_ns.client_ctx, STORAGE_CLIENT_TD_PORT);
    if (ret < 0) {
        goto err_fs_ns_create_port;
    }

    return 0;

err_fs_ns_create_port:
    /* undo fs_init */
err_init_fs_ns_tr_state:
    ns_close_file(state->ipc_handle, state->ns_handle);
    ipc_port_destroy(&state->fs_rpmb_boot.client_ctx);
err_fs_rpmb_boot_create_port:
    ipc_port_destroy(&state->fs_rpmb.client_ctx);
err_fs_rpmb_create_port:
    /* undo fs_init */
err_init_tr_state_rpmb:
err_bad_rpmb_size:
    rpmb_uninit(state->rpmb_state);
err_rpmb_init:
    return ret;
}

void block_device_tipc_uninit(struct block_device_tipc *state)
{
    if (state->dev_ns.block_count) {
        ipc_port_destroy(&state->fs_ns.client_ctx);
        /* undo fs_init */
        ns_close_file(state->ipc_handle, state->ns_handle);
    }
    ipc_port_destroy(&state->fs_rpmb_boot.client_ctx);
    ipc_port_destroy(&state->fs_rpmb.client_ctx);
    /* undo fs_init */
    rpmb_uninit(state->rpmb_state);
}
