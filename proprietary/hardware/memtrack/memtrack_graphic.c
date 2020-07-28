/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#include <hardware/memtrack.h>

#include "memtrack_mtk.h"

#define LOG_TAG "memtrack_graphic"
#include <cutils/log.h>

int graphic_memtrack_get_memory(pid_t pid, enum memtrack_type type,
                             struct memtrack_record *records,
                             size_t *num_records)
{
        FILE *fp;
        char line[1024];
        size_t unaccounted_size = 0;

	ALOGV("%s pid=%d, type=%d, num_records=%d\n", __func__, pid, type, *num_records);
	
        assert(type == MEMTRACK_TYPE_GRAPHICS);

        if (*num_records == 0) {
                *num_records = 1;
                return 0;
        }

        fp = fopen("/sys/kernel/debug/ion/clients/clients_summary", "r");
        if (fp == NULL) {
                ALOGE("%s error to open /sys/kernel/debug/ion/clients/clients_summary\n", __func__);
                return -errno;
        }

        while (1) {
                int handle_pid;
                unsigned int size;
                int ret;
                int count = 0;

                if (fgets(line, sizeof(line), fp) == NULL)
                    break;

                ret = sscanf(line, "%*s %d %u\n", &handle_pid, &size);
                if (ret == 2 && pid == handle_pid) {
                        unaccounted_size += size;
                        ALOGV("%s match %d: %s : %d %d %u %d\n", __func__, ret, line,
                              pid, handle_pid, size, (unsigned int)unaccounted_size);
               }
        }
        fclose(fp);

        if(*num_records > 0) {
                records[0].size_in_bytes = unaccounted_size;
                records[0].flags = MEMTRACK_FLAG_SMAPS_UNACCOUNTED|
                                        MEMTRACK_FLAG_SHARED |
                                        MEMTRACK_FLAG_SYSTEM |
                                        MEMTRACK_FLAG_NONSECURE;
        }

        *num_records = 1;

        return 0;
}

int graphic_memtrack_init()
{
        return 0;
}
