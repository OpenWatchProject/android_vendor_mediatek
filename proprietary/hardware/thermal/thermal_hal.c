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

#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/thermal.h>



#define TH_LOG_TAG "thermal_hal"
#define TH_DLOG(_priority_, _fmt_, args...)  /*LOG_PRI(_priority_, TH_LOG_TAG, _fmt_, ##args)*/
#define TH_LOG(_priority_, _fmt_, args...)  LOG_PRI(_priority_, TH_LOG_TAG, _fmt_, ##args)


#define CPU_LABEL               "CPU"
#define MAX_LENGTH              50

#define CPU_USAGE_FILE          "/proc/stat"
#define TEMPERATURE_DIR         "/proc/mtktz"
#define THERMAL_DIR             "mtkts"
#define CPU_ONLINE_FILE_FORMAT  "/sys/devices/system/cpu/cpu%d/online"
#define UNKNOWN_LABEL           "UNKNOWN"
#define LABEL_CPU               "CPU_T"
#define LABEL_GPU               "GPU_T"
#define LABEL_BAT               "BAT_T"
#define LABEL_SKIN              "SKIN_T"

#define TSCPU_PATH   "/proc/mtktz/mtktscpu"
#define TSBAT_PATH   "/proc/mtktz/mtktsbattery"
#define TSGPU_PATH   "/proc/mtktz/mtktscpu"  /*TODO, need GPU temp?*/
#define TSSKIN_PATH  "/proc/mtktz/mtktsAP"   /*on board sensor*/
#define CORENUM_PATH "/sys/devices/system/cpu/possible"

static const char *CPU_ALL_LABEL[] = {"CPU0", "CPU1", "CPU2", "CPU3", "CPU4", "CPU5", "CPU6", "CPU7", "CPU8", "CPU9"};

static ssize_t mtk_get_temperatures(thermal_module_t *module, temperature_t *list, size_t size) {

    FILE *file;
    float temp;
    size_t idx = 0;
    float temp_cpu = 0, temp_gpu = 0, temp_bat = 0, temp_tskin = 0;

    /* Read cpu/gpu/bat/tpcb temperatures from mtktz.
     * all temperatures are in Celsius.
     */

    /*TH_DLOG(ANDROID_LOG_INFO, "%s", __func__);*/


    /*get max CPU temperature*/
    file = fopen(TSCPU_PATH, "r");
    if (file == NULL) {
        ALOGW("thermal_hal: %s : failed to open file %s: %s", __func__, TSCPU_PATH, strerror(-errno));
    } else {
        if (fscanf(file, "%f", &temp) > 0) {
            temp_cpu = temp;

            if (list != NULL && idx < size) {
                list[idx] = (temperature_t) {
                .name = LABEL_CPU,
                .type = DEVICE_TEMPERATURE_CPU,
                .current_value = temp_cpu * 0.001,
                .throttling_threshold = 85,
                .shutdown_threshold = 117,
                .vr_throttling_threshold = 85,
                };
            }
            idx++;
        }
        else
            ALOGW("thermal_hal: %s: failed to fscanf %s: %s", __func__, TSCPU_PATH, strerror(-errno));

        fclose(file);

        TH_DLOG(ANDROID_LOG_INFO, "%s %s temp %f", __func__, TSCPU_PATH, temp);
    }



    /*get max GPU temperature*/
    file = fopen(TSGPU_PATH, "r");
    if (file == NULL) {
        ALOGW("thermal_hal: %s: failed to open directory %s: %s", __func__, TSGPU_PATH, strerror(-errno));
    } else {
        if (fscanf(file, "%f", &temp) > 0){
            temp_gpu = temp;

            if (list != NULL && idx < size) {
                list[idx] = (temperature_t) {
                .name = LABEL_GPU,
                .type = DEVICE_TEMPERATURE_GPU,
                .current_value = temp_gpu * 0.001,
                .throttling_threshold = 85,
                .shutdown_threshold = 117,
                .vr_throttling_threshold = 85,
                };
            }
            idx++;
        }
        else
            ALOGW("thermal_hal: %s: failed to fscanf %s: %s", __func__, TSGPU_PATH, strerror(-errno));

        fclose(file);


        TH_DLOG(ANDROID_LOG_INFO, "%s %s temp %f", __func__, TSGPU_PATH, temp);
    }


    /*get BATTERY temperature*/
    file = fopen(TSBAT_PATH, "r");
    if (file == NULL) {
        ALOGW("thermal_hal: %s: failed to open directory %s: %s", __func__, TSBAT_PATH, strerror(-errno));
    } else {
        if (fscanf(file, "%f", &temp) > 0) {
            temp_bat = temp;

            if (list != NULL && idx < size) {
                   list[idx] = (temperature_t) {
                   .name = LABEL_BAT,
                   .type = DEVICE_TEMPERATURE_BATTERY,
                   .current_value = temp_bat * 0.001,
                   .throttling_threshold = 50,
                   .shutdown_threshold = 60,
                   .vr_throttling_threshold = 50,
                   };
            }
            idx++;
        }
        else
            ALOGW("thermal_hal: %s: failed to fscanf %s: %s", __func__, TSBAT_PATH, strerror(-errno));

        fclose(file);

        TH_DLOG(ANDROID_LOG_INFO, "%s %s temp %f", __func__, TSBAT_PATH, temp);

    }



    /*get on board sensor temperature*/
    file = fopen(TSSKIN_PATH, "r");
    if (file == NULL) {
        ALOGW("thermal_hal: %s: failed to open directory %s: %s", __func__, TSSKIN_PATH, strerror(-errno));
    } else {
        if (fscanf(file, "%f", &temp) > 0){
            temp_tskin = temp;

            if (list != NULL && idx < size) {
                list[idx] = (temperature_t) {
                    .name = LABEL_SKIN,
                    .type = DEVICE_TEMPERATURE_SKIN,
                    .current_value = temp_tskin * 0.001,
                    .throttling_threshold = 50,
                    .shutdown_threshold = 90,
                    .vr_throttling_threshold = 50,
                };
            }
            idx++;
        }
        else
            ALOGW("thermal_hal: %s: failed to fscanf %s: %s", __func__, TSSKIN_PATH, strerror(-errno));


        fclose(file);

        TH_DLOG(ANDROID_LOG_INFO, "%s %s temp %f", __func__, TSSKIN_PATH, temp);

    }

    TH_DLOG(ANDROID_LOG_INFO, "%s: cpu = %f, gpu = %f, bat = %f, tskin = %f", __func__ , temp_cpu, temp_gpu, temp_bat, temp_tskin);

    return idx;
}


static ssize_t get_cpu_usages(thermal_module_t *module, cpu_usage_t *list) {
    int vals;
    ssize_t read;
    uint64_t user, nice, system, idle, active, total;
    char *line = NULL;
    size_t len = 0;
    size_t size = 0;

    FILE *file = NULL;
    unsigned int i = 0;

    unsigned int max_core_num, cpu_num, cpu_array;
    FILE *core_num_file = NULL;



    /*======get device max core num=======*/
    core_num_file = fopen(CORENUM_PATH, "r");
    if (core_num_file == NULL) {
        ALOGW("thermal_hal: %s: failed to open:CORENUM_PATH %s", __func__, strerror(errno));
        return -errno;
    }

    if (fscanf(core_num_file, "%*d-%d", &max_core_num) != 1) {
    	ALOGW("thermal_hal: %s: unable to parse CORENUM_PATH", __func__);
        fclose(core_num_file);
    	return -1;
    }
    fclose(core_num_file);


    cpu_array = sizeof(CPU_ALL_LABEL) / sizeof(CPU_ALL_LABEL[0]);

    if (((max_core_num + 1) > cpu_array) || ((max_core_num + 1) <= 0)) {
        ALOGW("thermal_hal: %s: max_core_num = %d, cpu_array = %d", __func__, max_core_num, cpu_array);
        return -1;
    }

    max_core_num += 1;

    TH_DLOG(ANDROID_LOG_INFO, "%s: max_core_num=%d", __func__, max_core_num);
    /*======get device max core num=======*/


    if (list == NULL){
        return max_core_num;
    }

    file = fopen(CPU_USAGE_FILE, "r");
    if (file == NULL) {
        ALOGW("thermal_hal: %s: failed to open: CPU_USAGE_FILE: %s", __func__, strerror(errno));
        return -errno;
    }



    /*set initial default value*/
    for (i = 0; i < max_core_num; i++) {
       list[i] = (cpu_usage_t) {
       .name = CPU_ALL_LABEL[i],
       .active = 0,
       .total = 0,
       .is_online = 0
       };
    }

    while ((read = getline(&line, &len, file)) != -1) {
        // Skip non "cpu[0-9]" lines.
        if (strnlen(line, read) < 4 || strncmp(line, "cpu", 3) != 0 || !isdigit(line[3])) {
            free(line);
            line = NULL;
            len = 0;
            continue;
        }


        vals = sscanf(line, "cpu%d %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64, &cpu_num, &user,
                        &nice, &system, &idle);

        free(line);
        line = NULL;
        len = 0;


        if (vals != 5) {
            ALOGW("thermal_hal: %s: failed to read CPU information from file: %s", __func__, strerror(errno));
            fclose(file);
            return errno ? -errno : -EIO;
        }

        active = user + nice + system;
        total = active + idle;

        if (cpu_num < max_core_num) {
            list[cpu_num] = (cpu_usage_t) {
            .name = CPU_ALL_LABEL[cpu_num],
            .active = active,
            .total = total,
            .is_online = 1 /*cpu online*/
            };
        } else {
            ALOGW("thermal_hal: %s: cpu_num %d > max_core_num %d", __func__, cpu_num, max_core_num);
            fclose(file);
            return errno ? -errno : -EIO;
        }

        size++;
    }

    TH_DLOG(ANDROID_LOG_INFO, "%s end loop, size %d, cpu_num = %d, max_core_num = %d", __func__, size, cpu_num, max_core_num);

    fclose(file);

    return max_core_num;

}

static ssize_t get_cooling_devices(thermal_module_t *module, cooling_device_t *list, size_t size) {
    return 0;
}

static struct hw_module_methods_t mtk_thermal_module_methods = {
    .open = NULL,
};

thermal_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = THERMAL_HARDWARE_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = THERMAL_HARDWARE_MODULE_ID,
        .name = "MTK Thermal HAL",
        .author = "The Android Open Source Project",
        .methods = &mtk_thermal_module_methods,
    },

    .getTemperatures = mtk_get_temperatures,
    .getCpuUsages = get_cpu_usages,
    .getCoolingDevices = get_cooling_devices,
};






