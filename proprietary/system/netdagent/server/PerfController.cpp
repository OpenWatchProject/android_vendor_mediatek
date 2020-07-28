/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include "PerfController.h"
#include "NetdConstants.h"
#include <cutils/properties.h>
#include "PerfServiceNative.h"
#include <android-base/file.h>
#define LIB_FULL_NAME "libperfservicenative.so"
#define LOG_TAG "PerfController"
#include "log/log.h"

using android::base::WriteStringToFile;

int PerfController::tether_perfHandle = -1;
int PerfController::lowpower_perfHandle = -1;
user_reg_scn PerfController::perfUserRegScn = NULL;
user_reg_scn_config PerfController::perfUserRegScnConfig = NULL;
user_unreg_scn PerfController::perfUserUnregScn = NULL;
user_enable PerfController::perfUserScnEnable = NULL;
user_disable PerfController::perfUserScnDisable = NULL;
user_reset_all PerfController::perfUserScnResetAll = NULL;
user_disable_all PerfController::perfUserScnDisableAll = NULL;
dump_all PerfController::perfDumpAll = NULL;
set_favor_pid PerfController::perfSetFavorPid = NULL;
notify_user_status PerfController::perfNotifyUserStatus = NULL;

int readFile(const char *path, char *buf, int len)
{
    int fd = open(path, O_RDONLY);
    int size;

    if (fd < 0) {
        ALOGE("Failed to open %s: %s", path, strerror(errno));
        return -1;
    }

    size = read(fd, buf, len);
    if (size < 0) {
        ALOGE("Failed to read %s: %s", path, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return size;
}

PerfController::PerfController() {

}

PerfController::~PerfController() {

}

int PerfController::is_eng(void)
{
    char value[PROPERTY_VALUE_MAX] = {0};

    if(property_get("ro.build.type", value, NULL) <= 0)
        return 0;

    ALOGI("%s type is loaded", value);
    if (0 == strcmp(value, "eng"))
        return 1;
    else
        return 0;
}

int PerfController::is_testsim(void)
{
    char value_a[PROPERTY_VALUE_MAX] = {0};
    char value_b[PROPERTY_VALUE_MAX] = {0};
    if((property_get("vendor.gsm.sim.ril.testsim", value_a, NULL) <= 0)
        && (property_get("vendor.gsm.sim.ril.testsim.2", value_b, NULL) <= 0)) {
        //ALOGI("no sim card is checked");
        return 0;
    }

    if((0 == strcmp(value_a, "1")) || (0 == strcmp(value_b, "1"))) {
        //ALOGI("testsim checked");
        return 1;
    } else {
        //ALOGI("no testsim checked");
        return 0;
    }
}

int PerfController::is_op01(void)
{
    char value[PROPERTY_VALUE_MAX] = {0};
    if(property_get("persist.vendor.operator.optr", value, NULL) <= 0) {
        ALOGI("no flavor");
        return 0;
    }
    if (0 == strcmp(value, "OP01")) {
        //ALOGI("OP01 flavor");
        return 1;
    } else {
        //ALOGI("not OP01 flavor");
        return 0;
    }
}

int PerfController::load_PerfService(void)
{
    void *handle, *func;

    handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
    func = dlsym(handle, "PerfServiceNative_userRegScn");
    perfUserRegScn = reinterpret_cast<user_reg_scn>(func);

    if (perfUserRegScn == NULL) {
        ALOGE("perfUserRegScn error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_userRegScnConfig");
    perfUserRegScnConfig = reinterpret_cast<user_reg_scn_config>(func);

    if (perfUserRegScnConfig == NULL) {
        ALOGE("perfUserRegScnConfig error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_userUnregScn");
    perfUserUnregScn = reinterpret_cast<user_unreg_scn>(func);

    if (perfUserUnregScn == NULL) {
        ALOGE("perfUserUnregScn error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_userEnable");
    perfUserScnEnable = reinterpret_cast<user_enable>(func);

    if (perfUserScnEnable == NULL) {
        ALOGE("perfUserScnEnable error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_userDisable");
    perfUserScnDisable = reinterpret_cast<user_disable>(func);

    if (perfUserScnDisable == NULL) {
        ALOGE("perfUserScnDisable error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_userResetAll");
    perfUserScnResetAll = reinterpret_cast<user_reset_all>(func);

    if (perfUserScnResetAll == NULL) {
        ALOGE("perfUserScnResetAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_userDisableAll");
    perfUserScnDisableAll = reinterpret_cast<user_disable_all>(func);

    if (perfUserScnDisableAll == NULL) {
        ALOGE("perfUserScnDisableAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_dumpAll");
    perfDumpAll = reinterpret_cast<dump_all>(func);

    if (perfDumpAll == NULL) {
        ALOGE("perfDumpAll error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    func = dlsym(handle, "PerfServiceNative_notifyUserStatus");
    perfNotifyUserStatus= reinterpret_cast<notify_user_status>(func);

    if (perfNotifyUserStatus == NULL) {
        ALOGE("perfNotifyUserStatus error: %s", dlerror());
        dlclose(handle);
        return -1;
    }

    return 0;
}

int PerfController::set_ack_reduction(const char *ack_setting)
{
    const char fname[] = "/proc/sys/net/ipv4/tcp_ack_number";
    if(!WriteStringToFile(ack_setting, fname)) {
        ALOGI("set ack reduction failed");
        return -1;
    }
    return 0;
}

int PerfController::restore_ack_reduction()
{
    const char fname[] = "/proc/sys/net/ipv4/tcp_ack_number";
    const char *ack_restore = "1"; //data:ack = (1+1):1
    if(!WriteStringToFile(ack_restore, fname)) {
        ALOGI("restore ack reduction failed");
        return -1;
    }
    return 0;
}

int PerfController::set_rps(const char* iface, const char* rps)
{
   char *fname;

   if(strncmp(iface, "ccmni", 5) && strncmp(iface, "rndis", 5)) {
       ALOGI("set_rps: wrong interface");
       return -1;
   }

   asprintf(&fname, "/sys/class/net/%s/queues/rx-0/rps_cpus", iface);
   if(!WriteStringToFile(rps, fname))
   {
     ALOGI("set_rps: fail to set rps ");
     free(fname);
     return -1 ;
   }
   free(fname);
   return 0 ;
}

int PerfController::recover_rps(const char* iface)
{
    const char *rps_prop_default_name = "vendor.net.perf.rps.default";
    char rps_prop_default_value[PROPERTY_VALUE_MAX] = {0};
    property_get(rps_prop_default_name, rps_prop_default_value, NULL);
    return set_rps(iface, rps_prop_default_value);
}

int PerfController::parse_perf_prop(char *split, char *prop_value, int parse_value[][2])
{
    char *tok = NULL;
    unsigned char i;

    tok = strtok(prop_value, split);
    for(i=0; tok != NULL; i++) {
        parse_value[i][0] = atoi(tok);
        tok = strtok(NULL, split);
        parse_value[i][1] = atoi(tok);
#ifdef PERF_DEBUG
        ALOGI("parse_value[%d][0]:%d, parse_value[%d][1]:%d", i, parse_value[i][0], i, parse_value[i][1]);
#endif
        tok = strtok(NULL, split);
    }
#ifdef PERF_DEBUG
    ALOGI("cluster %d", i);
#endif
    return i;

}

#define MAX_CLUSTER 10
int PerfController::enable_perf_rps(const char* intIface, const char* extIface)
{
    const char *rps_prop_name  = "vendor.net.perf.tether.rps";
    const char *core_prop_name = "vendor.net.perf.tether.cpu.core";
    const char *freq_prop_name = "vendor.net.perf.tether.cpu.freq";
    char rps_prop_value[PROPERTY_VALUE_MAX]  = {0};
    char core_prop_value[PROPERTY_VALUE_MAX] = {0};
    char freq_prop_value[PROPERTY_VALUE_MAX] = {0};
    char split[8];

    int i, cluster;
    int core[MAX_CLUSTER][2];
    int freq[MAX_CLUSTER][2];

    if(is_eng() == 1)
        return 0;

    if(strncmp(intIface, "rndis", 5) != 0)
        return 0;

    if(tether_perfHandle != -1)
        return 0;

    if(lowpower_perfHandle != -1) {
        ALOGI("tether mode is on, low power mode ready to exit");
        exit_little_cpu();
    }

    memset(core, -1, sizeof(core));
    memset(freq, -1, sizeof(freq));

    property_get(rps_prop_name, rps_prop_value, NULL);
    property_get(core_prop_name, core_prop_value, NULL);
    property_get(freq_prop_name, freq_prop_value, NULL);

#ifdef PERF_DEBUG
    ALOGI("rps_prop_value,%s",  rps_prop_value);
    ALOGI("core_prop_value,%s", core_prop_value);
    ALOGI("freq_prop_value,%s", freq_prop_value);
#endif

    strcpy(split, ",");
    cluster = parse_perf_prop(split, core_prop_value, core);
    cluster = parse_perf_prop(split, freq_prop_value, freq);

    set_rps(intIface, rps_prop_value);
    set_rps(extIface, rps_prop_value);

    /*config perfService*/

    tether_perfHandle = perfUserRegScn();
    if(tether_perfHandle < 0) {
        ALOGI("perfServie register fail");
        return -1;
    }

    perfUserRegScnConfig(tether_perfHandle, CMD_SET_SCREEN_OFF_STATE, SCREEN_OFF_ENABLE, 0, 0, 0);
    for(i=0; i < cluster; i++) {
        perfUserRegScnConfig(tether_perfHandle, CMD_SET_CLUSTER_CPU_CORE_MIN, i, core[i][0], 0, 0);
        perfUserRegScnConfig(tether_perfHandle, CMD_SET_CLUSTER_CPU_CORE_MAX, i, core[i][1], 0, 0);
        perfUserRegScnConfig(tether_perfHandle, CMD_SET_CLUSTER_CPU_FREQ_MIN, i, freq[i][0], 0, 0);
        perfUserRegScnConfig(tether_perfHandle, CMD_SET_CLUSTER_CPU_FREQ_MAX, i, freq[i][1], 0, 0);
    }
    perfUserScnEnable(tether_perfHandle);
    ALOGI("tether perfservice and rps enable");
    return 0;
}


int PerfController::disable_perf_rps(const char* intIface, const char* extIface)
{
    if(is_eng() == 1)
        return 0;
    if(strncmp(intIface, "rndis", 5) != 0)
        return 0;
    if(tether_perfHandle == -1)
        return 0;
    //rndis rps will be automatically cleared, so rps disable do not need
    recover_rps(extIface);
    perfUserScnDisable(tether_perfHandle);
    tether_perfHandle = -1;
    ALOGI("tether perfservice and rps disable");
    return 0;
}

int PerfController::enter_little_cpu() {

    const char *internal_core_prop_name = "vendor.net.perf.internal.cpu.core";
    const char *internal_freq_prop_name = "vendor.net.perf.internal.cpu.freq";
    char internal_core_prop_value[PROPERTY_VALUE_MAX] = {0};
    char internal_freq_prop_value[PROPERTY_VALUE_MAX] = {0};
    int i, cluster;
    int internal_core[MAX_CLUSTER][2];
    int internal_freq[MAX_CLUSTER][2];
    char split[8];

    if((lowpower_perfHandle != -1) || (tether_perfHandle != -1))
        return 0;

    memset(internal_core, -1, sizeof(internal_core));
    memset(internal_freq, -1, sizeof(internal_freq));

    property_get(internal_core_prop_name, internal_core_prop_value, NULL);
    property_get(internal_freq_prop_name, internal_freq_prop_value, NULL);

#ifdef PERF_DEBUG
    ALOGI("internal_core_prop_value,%s", internal_core_prop_value);
    ALOGI("internal_freq_prop_value,%s", internal_freq_prop_value);
#endif

    strcpy(split, ",");
    cluster = parse_perf_prop(split, internal_core_prop_value, internal_core);
    cluster = parse_perf_prop(split, internal_freq_prop_value, internal_freq);

    lowpower_perfHandle = perfUserRegScn();
    if(lowpower_perfHandle < 0) {
        ALOGI("perfServie register fail");
        return -1;
    }

    perfUserRegScnConfig(lowpower_perfHandle, CMD_SET_SCREEN_OFF_STATE, SCREEN_OFF_ENABLE, 0, 0, 0);
    for(i=0; i < cluster; i++) {
        perfUserRegScnConfig(lowpower_perfHandle, CMD_SET_CLUSTER_CPU_CORE_MIN, i, internal_core[i][0], 0, 0);
        perfUserRegScnConfig(lowpower_perfHandle, CMD_SET_CLUSTER_CPU_CORE_MAX, i, internal_core[i][1], 0, 0);
        perfUserRegScnConfig(lowpower_perfHandle, CMD_SET_CLUSTER_CPU_FREQ_MIN, i, internal_freq[i][0], 0, 0);
        perfUserRegScnConfig(lowpower_perfHandle, CMD_SET_CLUSTER_CPU_FREQ_MAX, i, internal_freq[i][1], 0, 0);
    }
    perfUserScnEnable(lowpower_perfHandle);
    ALOGI("enter little cpu mode");
    return 0;
}

int PerfController::exit_little_cpu()
{
    if(lowpower_perfHandle == -1)
        return 0;
    perfUserScnDisable(lowpower_perfHandle);
    lowpower_perfHandle = -1;
    ALOGI("exit little cpu mode");
    return 0;
}


int PerfController::get_load() {
    const char *file  = "proc/cpuinfo";
    char chip_platform[128]  = {0};
    enum load{NONE, JADE, EVEREST, OLYMPUS}type = NONE;
    FILE *fp = fopen(file, "r");
    if(fp == NULL) {
        ALOGE("open file failed");
        return type;
    }

    while(!feof(fp))
        fgets(chip_platform, sizeof(chip_platform), fp);
    fclose(fp);
    if(strstr(chip_platform, "Hardware") == NULL) {
        ALOGI("get hardware info failed");
        return type;
    }
    ALOGI("chip_platform %s", chip_platform);
    if(strstr(chip_platform, "MT6755") != NULL)
        type = JADE;
    if(strstr(chip_platform, "MT6797") != NULL)
        type = EVEREST;
    if(strstr(chip_platform, "MT6799") != NULL)
        type = EVEREST;
    if(strstr(chip_platform, "MT6757") != NULL)
        type = OLYMPUS;

    return type;
}


void PerfController::dump_cpuinfo(int type)
{
    enum load{NONE, JADE, EVEREST, OLYMPUS};
    switch(type) {
        case JADE:dump_cpuinfo_jade(); break;
        case EVEREST:dump_cpuinfo_everest();break;
        case OLYMPUS:dump_cpuinfo_olympus();break;
        default:ALOGI("cpu info of the load is not supported");
    }
}

void PerfController::dump_cpuinfo_jade()
{
    const char *file_cpu = "/sys/devices/system/cpu/online";
    const char *file_freq_ll = "/proc/cpufreq/MT_CPU_DVFS_LITTLE/cpufreq_freq";
    const char *file_freq_l = "/proc/cpufreq/MT_CPU_DVFS_BIG/cpufreq_freq";
    char cpu_value[64];
    char freq_ll_value[64];
    char freq_l_value[64];
    int cpu_len, freq_ll_len, freq_l_len;

    if((cpu_len = readFile(file_cpu, cpu_value, sizeof(cpu_value))) > 0)
        cpu_value[cpu_len-1] = '\0';

    if((freq_ll_len = readFile(file_freq_ll, freq_ll_value, sizeof(freq_ll_value))) > 0)
        freq_ll_value[freq_ll_len-1] = '\0';

    if((freq_l_len = readFile(file_freq_l, freq_l_value, sizeof(freq_l_value))) > 0)
        freq_l_value[freq_l_len-1] = '\0';

    ALOGI("cpu_core %s, cpu_freq_ll %s, cpu_freq_l %s", cpu_value, freq_ll_value, freq_l_value);

}

void PerfController::dump_cpuinfo_everest()
{
    const char *file_cpu = "/sys/devices/system/cpu/online";
    const char *file_freq_ll = "/proc/cpufreq/MT_CPU_DVFS_LL/cpufreq_freq";
    const char *file_freq_l = "/proc/cpufreq/MT_CPU_DVFS_L/cpufreq_freq";
    const char *file_freq_b = "/proc/cpufreq/MT_CPU_DVFS_B/cpufreq_freq";
    char cpu_value[64];
    char freq_ll_value[64];
    char freq_l_value[64];
    char freq_b_value[64];
    int cpu_len, freq_ll_len, freq_l_len, freq_b_len;

    if((cpu_len = readFile(file_cpu, cpu_value, sizeof(cpu_value))) > 0)
        cpu_value[cpu_len-1] = '\0';

    if((freq_ll_len = readFile(file_freq_ll, freq_ll_value, sizeof(freq_ll_value))) > 0)
        freq_ll_value[freq_ll_len-1] = '\0';

    if((freq_l_len = readFile(file_freq_l, freq_l_value, sizeof(freq_l_value))) > 0)
        freq_l_value[freq_l_len-1] = '\0';

    if((freq_b_len = readFile(file_freq_b, freq_b_value, sizeof(freq_b_value))) > 0)
        freq_b_value[freq_b_len-1] = '\0';

    ALOGI("cpu_core %s, cpu_freq_ll %s, cpu_freq_l %s, cpu_freq_b %s", cpu_value, freq_ll_value, freq_l_value, freq_b_value);
}

void PerfController::dump_cpuinfo_olympus()
{
    const char *file_cpu = "/sys/devices/system/cpu/online";
    const char *file_freq_ll = "/proc/cpufreq/MT_CPU_DVFS_LL/cpufreq_freq";
    const char *file_freq_l = "/proc/cpufreq/MT_CPU_DVFS_L/cpufreq_freq";
    char cpu_value[64];
    char freq_ll_value[64];
    char freq_l_value[64];
    int cpu_len, freq_ll_len, freq_l_len;

    if((cpu_len = readFile(file_cpu, cpu_value, sizeof(cpu_value))) > 0)
        cpu_value[cpu_len-1] = '\0';

    if((freq_ll_len = readFile(file_freq_ll, freq_ll_value, sizeof(freq_ll_value))) > 0)
        freq_ll_value[freq_ll_len-1] = '\0';

    if((freq_l_len = readFile(file_freq_l, freq_l_value, sizeof(freq_l_value))) > 0)
        freq_l_value[freq_l_len-1] = '\0';

    ALOGI("cpu_core %s, cpu_freq_ll %s, cpu_freq_l %s", cpu_value, freq_ll_value, freq_l_value);
}
