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

#include <list.h>
#include <stdint.h>

typedef unsigned long long data_block_t;

/**
 * struct block_device - Block device functions and state
 * @start_read:         Function to start a read operation from block device.
 * @start_write:        Function to start a write operation to block device.
 * @wait_for_io:        Function to wait for read or write operations to
 *                      complete. If @start_read and @start_write always call
 *                      block_cache_complete_read and block_cache_complete_write
 *                      this can be %NULL.
 * @block_count:        Number of blocks in block device.
 * @block_size:         Number of bytes per block.
 * @block_num_size:     Number of bytes used to store block numbers.
 * @mac_size:           Number of bytes used to store mac values.
 *                      Must be 16 if @tamper_detecting is %false.
 * @tamper_detecting:   %true if block device detects tampering of read and
 *                      write opeartions. %false otherwise. If %true, when a
 *                      write operation is reported as successfully completed
 *                      it should not be possible for non-secure code to modify
 *                      the stored data.
 * @io_ops:             List os active read or write operations. Once
 *                      initialized, this list is managed by block cache, not by
 *                      the block device.
 */
struct block_device {
    void (*start_read)(struct block_device *dev, data_block_t block);
    void (*start_write)(struct block_device *dev, data_block_t block,
                        const void *data, size_t data_size);
    void (*wait_for_io)(struct block_device *dev);

    data_block_t block_count;
    size_t block_size;
    size_t block_num_size;
    size_t mac_size;
    bool tamper_detecting;

    struct list_node io_ops;
};
