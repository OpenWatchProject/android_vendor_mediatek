/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "libPowerHal"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>  /* ioctl */
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include <utils/Log.h>

#include "common.h"

#include <string>

//#include <linux/disp_session.h>

int devfdDSC = -1;

using namespace std;

int compare(const void * arg1, const void * arg2)
{
  return ( *(int*)arg2 - *(int*)arg1 );
}

 /*   return value:
  *         0, error or read nothing
  *        !0, read counts
  */
static
int read_from_file(const char* path, char* buf, int size)
{
    if (!path) {
        return 0;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        ALOGE("Could not open '%s'\n", path);
        char *err_str = strerror(errno);
        ALOGE("error : %d, %s\n", errno, err_str);
        return 0;
    }

    int count = read(fd, buf, size);
    if (count > 0) {
        count = (count < size) ? count : size - 1;
        while (count > 0 && buf[count-1] == '\n') count--;
        buf[count] = '\0';
    } else {
        buf[0] = '\0';
    }

    close(fd);
    return count;
}

int get_int_value(const char * path)
{
    int size = 32;
    char buf[32] = {0};
    if(!read_from_file(path, buf, size))
        return 0;
    return atoi(buf);
}

int get_cpu_num(void)
{
    int size = 32, cpu_first, cpu_last;
    char buf[32] = {0};
    if(!read_from_file(PATH_CPUNUM_POSSIBLE, buf, size))
        return 1;
    sscanf(buf, "%d-%d", &cpu_first, &cpu_last);
    return (cpu_last+1);
}

void get_cputopo_cpu_info(int cluster_num, int *p_cpu_num, int *p_first_cpu) // find first cpu of each cluster
{
    FILE *ifp;
    char  buf[128] = {0}, *str;
    int  i = 0, mask, cluster = 0, count, index;

    if ((ifp = fopen(PATH_PERFMGR_TOPO_CLUSTER_CPU,"r")) == NULL) {
        if ((ifp = fopen(PATH_CPUTOPO_CLUSTER_CPU,"r")) == NULL)
            return ;
    }

    while(fgets(buf, 128, ifp) && cluster < cluster_num) {
        if (strlen(buf) < 3) // at least 3 characters, e.g., "a b"
            continue;

        str = strtok(buf, " ");
        //ALOGI("str : %s", str);
        str = strtok(NULL, " ");
        //ALOGI("str : %s", str);
        sscanf(str, "%x", &mask);
        //mask = atoi(str);
        //ALOGI("mask : %d, %x", mask, mask);

        count = 0;
        index = -1;
        for(i=0; mask>0; i++) {
            if (mask % 2 == 1) {
                if (index == -1)
                    index = i;
                count++;
            }
            mask /= 2;
        }
        p_cpu_num[cluster] = count;
        p_first_cpu[cluster] = index;

        cluster++;
    }

    fclose(ifp);
}


void get_task_comm(const char *path, char *comm)
{
    int size = 64;
    char buf[64] = {0};
    if(!read_from_file(path, buf, size))
        comm[0] = '\0';
    else
        set_str_cpy(comm, buf, size);
}

static
int write_to_file(const char* path, const char* buf, int size)
{
    if (!path) {
        ALOGE("null path to write");
        return 0;
    }

    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        ALOGE("Could not open '%s'\n", path);
        char *err_str = strerror(errno);
        ALOGE("error : %d, %s\n", errno, err_str);
        return 0;
    }

    int count = write(fd, buf, size);
    if (count != size) {
        ALOGE("write file (%s,%s) fail, count: %d\n", path, buf, count);
        char *err_str = strerror(errno);
        ALOGE("error : %d, %s\n", errno, err_str);
        close(fd);
        return 0;
    }

    close(fd);
    return count;
}

/*
 *  return
 *      0: fail
 *      count: number of byte written
 */
int set_value(const char * path, const int value_1, const int value_2)
{
    char buf[32] = {0};
    sprintf(buf, "%d %d", value_1, value_2);
    return write_to_file(path, buf, strlen(buf));
}

/*
 *  return
 *      0: fail
 *      count: number of byte written
 */
int set_value(const char * path, const int value)
{
    char buf[32] = {0};
    sprintf(buf, "%d", value);
    return write_to_file(path, buf, strlen(buf));
}

/*
 *  return
 *      0: fail
 *      count: number of byte written
 */
int set_value(const char * path, const char *str)
{
    return write_to_file(path, str, strlen(str));
}

/*
 *  return
 *      0: fail
 *      count: number of byte written
 */
int set_value(const char * path, const string *str)
{
    return write_to_file(path, str->c_str(), str->length());
}

void get_str_value(const char * path, char *str, int len)
{
    read_from_file(path, str, len);
}

void set_str_cpy(char * desc, const char *src, int desc_max_size)
{
    int len_sz = 0;

    len_sz = strlen(src);
    len_sz = (len_sz < desc_max_size) ? len_sz : (desc_max_size - 1);
    strncpy(desc, src, len_sz);
    desc[len_sz] = '\0';
}

void get_ppm_cpu_freq_info(int cluster_index, int *p_max_freq, int *p_count, int **pp_table) // max freq, freq level counts, freq table
{
    char file[128] = {0}, *str, buf[256] = {0};
    int *tbl = NULL, count=0, i=0;

    sprintf(file, "/proc/ppm/dump_cluster_%d_dvfs_table", cluster_index);
    if(!read_from_file(file, buf, sizeof(buf)))
        return;

    str = strtok(buf, " ");
    while(str) {
        count++;
        str = strtok(NULL, " ");
    }

    *p_count = count;
    if(count <= 0) return;

    /* create table */
    *pp_table = (int*)malloc(sizeof(int)*count);
    tbl = (int*)malloc(sizeof(int)*count);

    if(*pp_table == NULL || tbl == NULL) {
        if(tbl != NULL)
            free(tbl);
        return;
    }

    for(i=0; i<count; i++)
        tbl[i] = 0;

    if(!read_from_file(file, buf, sizeof(buf)))
        return;
    str = strtok(buf, " ");
    i = 0;
    while(str && i<count) {
        tbl[i] = atoi(str);
        i++;
        str = strtok(NULL, " ");
    }

    for(i=0; i<count; i++)
        (*pp_table)[i] = tbl[count-1-i];
    *p_max_freq = tbl[0];
    free(tbl);
}


void get_cpu_freq_info(int cpu_index, int *p_max_freq, int *p_count, int **pp_table) // max freq, freq level counts, freq table
{
    char file[128] = {0}, *str, buf[256] = {0};
    int *tbl = NULL, count=0, i=0;

#if 0
    sprintf(file, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpu_index);
    while((*p_max_freq = get_int_value(file)) == 0) {
        usleep(4000);
        if(timeout++ > 50) {
            ALOGI("get_cpu_freq_info:%d, timeout!!!", cpu_index);
            break;
        }
    }
#endif

    sprintf(file, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cpu_index);
    if(!read_from_file(file, buf, sizeof(buf)))
        return;

    str = strtok(buf, " ");
    while(str) {
        count++;
        str = strtok(NULL, " ");
    }

    *p_count = count;
    if(count <= 0) return;

    /* create table */
    *pp_table = (int*)malloc(sizeof(int)*count);
    tbl = (int*)malloc(sizeof(int)*count);

    if(*pp_table == NULL || tbl == NULL) {
        if(tbl != NULL)
            free(tbl);
        return;
    }

    if(!read_from_file(file, buf, sizeof(buf)))
        return;
    str = strtok(buf, " ");
    while(str) {
        tbl[i] = atoi(str);
        i++;
        str = strtok(NULL, " ");
    }

    qsort(tbl, count, sizeof(int), compare);
    for(i=0; i<count; i++)
        (*pp_table)[i] = tbl[count-1-i];

    /* workaround */
    /*if(*p_max_freq == 0)*/
    if((*p_max_freq) != ((*pp_table)[count - 1]))
        *p_max_freq = (*pp_table)[count - 1];

    free(tbl);
}

void get_gpu_freq_level_count(int *p_count)
{
    *p_count = get_int_value(PATH_GPUFREQ_COUNT);
    //ALOGI("get_gpu_freq_level_count:%d", *p_count);
}

void set_gpu_freq_level(int level)
{
    char buf[32];
    sprintf(buf, "%d", level);
    write_to_file(PATH_GPUFREQ_BASE, buf, strlen(buf));
}

void set_gpu_freq_level_max(int level)
{
    char buf[32];
    sprintf(buf, "%d", level);
    write_to_file(PATH_GPUFREQ_MAX, buf, strlen(buf));
}

void set_vcore_level(int level)
{
    char buf[32];
    sprintf(buf, "POWER_MODE %d", level);
    write_to_file(PATH_VCORE, buf, strlen(buf));
}

#if 0 // not support now
static int check_display_ctl_valid(void)
{
    int fd = -1;

    //ALOGI("check_display_ctl_valid");
    if (devfdDSC >= 0) {
        return 0;
    } else if (devfdDSC == -1) {
        devfdDSC = open(DISP_DEV_ID, O_RDONLY, 0);
        // file not exits
        if (devfdDSC < 0 && errno == ENOENT)
            devfdDSC = -2;
        // file exist, but can't open
        if (devfdDSC == -1) {
            ALOGD("Can't open %s: %s", DISP_DEV_ID, strerror(errno));
            return -1;
        }
    // file not exist
    } else if (devfdDSC == -2) {
        //ALOGD("Can't open %s: %s", DISP_DEV_ID, strerror(errno));
        return -2;
    }
    return 0;
}

int set_disp_ctl(int enable)
{
    //struct disp_scenario_config_t cfg;

    if (check_display_ctl_valid() != 0)
        return -1;
/*
    cfg.session_id = MAKE_DISP_SESSION(DISP_SESSION_PRIMARY, 0);

    if (enable)
        cfg.scenario = DISP_SCENARIO_FORCE_DC;
    else
        cfg.scenario = DISP_SCENARIO_NORMAL;

    ioctl(devfdDSC, DISP_IOCTL_SET_SCENARIO, &cfg);
*/
    return 0;
}
#endif

