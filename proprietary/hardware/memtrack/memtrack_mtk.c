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

#include <hardware/memtrack.h>

#include "memtrack_mtk.h"

static int memtrack_device_open(const hw_module_t* module, const char* name,
          hw_device_t** device);

int mtk_memtrack_init(const struct memtrack_module *module)
{
  if (module != NULL) {
    memtrack_dbg("[memtrack] module: 0x%x", module);
  }

  gl_memtrack_init();
  graphic_memtrack_init();
  return 0;
}

int mtk_memtrack_get_memory(const struct memtrack_module *module,
        pid_t pid,
        int type,
        struct memtrack_record *records,
        size_t *num_records)
{
  if (module != NULL) {
                memtrack_dbg("[memtrack] module: 0x%x", module);
        }

  if (type == MEMTRACK_TYPE_GL) {
      return gl_memtrack_get_memory(pid, type, records, num_records);
  } else if(type == MEMTRACK_TYPE_GRAPHICS) {
      return graphic_memtrack_get_memory(pid, type, records, num_records);
  }

  return -EINVAL;
}

int memtrack_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
  if (module != NULL) {
                memtrack_dbg("[memtrack] module: 0x%x", module);
        }

  if (name != NULL) {
    memtrack_dbg("[memtrack] name: %s", name);
  }

  if (module) {
    memtrack_module_t **outModule = (memtrack_module_t **) device;
    *outModule = (memtrack_module_t *)module;
    return 0;
  } else {
    return -EINVAL;
  }
}


static struct hw_module_methods_t memtrack_module_methods = {
  .open = memtrack_device_open
};

struct memtrack_module HAL_MODULE_INFO_SYM = {
  .common = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = MEMTRACK_MODULE_API_VERSION_0_1,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = MEMTRACK_HARDWARE_MODULE_ID,
    .name = "MediaTek Memory Tracker HAL",
    .author = "The Android Open Source Project",
    .methods = &memtrack_module_methods,
  },

  .init = mtk_memtrack_init,
  .getMemory = mtk_memtrack_get_memory,
};
