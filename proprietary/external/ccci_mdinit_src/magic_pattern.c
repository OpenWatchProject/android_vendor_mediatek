/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <android/log.h>
#include "ccci_common.h"

#define DEV_IOC_MAGIC               'd'
#define READ_DEV_DATA               _IOR(DEV_IOC_MAGIC,  1, unsigned int)
#define OPEN_DEVINFO_NODE_FAIL        0x1001
#define READ_DEVINFO_DATA_FAIL        0x1002
#define ID_INDEX                    12        //HRID


static int byte_reverse(unsigned char *id_ptr){
    int i = 0;
    int tmp = 0;

    for (i=0 ; i<=2;i+=2){
        tmp = *(id_ptr+i);
        *(id_ptr+i) = *(id_ptr+i+1);
        *(id_ptr+i+1) = tmp;
    }
    return 0;
}

static int cross_id_switch(unsigned char *id_1_ptr, unsigned char *id_2_ptr){
    unsigned char tmp = 0;

    tmp = *(id_1_ptr+1);
    *(id_1_ptr+1) = *(id_2_ptr);
    *(id_2_ptr) = tmp;

    tmp = *(id_1_ptr+2);
    *(id_1_ptr+2) = *(id_2_ptr+3);
    *(id_2_ptr+3) = tmp;

    return 0;
}

static int dump_id_data(unsigned int id_1, unsigned int id_2){

    /* -----------------------------------     */
    /* Dump for debug                        */
    /* -----------------------------------     */
    CCCI_LOGD("id_1:0x%x\n", id_1);
    CCCI_LOGD("id_2:0x%x\n", id_2);

    return 0;
}

int compute_random_pattern(unsigned int * p_val){

    int fd = 0;
    int ret = 0;
    int i = 0;
    unsigned char tmp;
    unsigned int id_1 = ID_INDEX;
    unsigned int id_2 = ID_INDEX + 1;
    unsigned char* id_1_ptr = (unsigned char *)&id_1;
    unsigned char* id_2_ptr = (unsigned char *)&id_2;

    /* =================================== */
    /* open devinfo driver                 */
    /* =================================== */
    fd = open("/dev/devmap", O_RDONLY, 0);
    if (fd < 0)
    {
        ret = OPEN_DEVINFO_NODE_FAIL;
        goto _fail;
    }

    /* -----------------------------------     */
    /* Read ID data                           */
    /* -----------------------------------     */
    if ((ret = ioctl(fd, READ_DEV_DATA, &id_1)) != 0)
    {
        CCCI_LOGE("get id_1 fail:%d\n", ret);
        ret = READ_DEVINFO_DATA_FAIL;
        goto _fail;
    }

    if ((ret = ioctl(fd, READ_DEV_DATA, &id_2)) != 0)
    {
        CCCI_LOGE("get id_2 fail:%d\n", ret);
        ret = READ_DEVINFO_DATA_FAIL;
        goto _fail;
    }

    dump_id_data(id_1, id_2);

    /* -----------------------------------     */
    /* Byte reverse                            */
    /* -----------------------------------     */
    byte_reverse(id_1_ptr);
    byte_reverse(id_2_ptr);
    dump_id_data(id_1, id_2);

    /* -----------------------------------     */
    /* Cross ID Switch                       */
    /* -----------------------------------     */
    cross_id_switch(id_1_ptr, id_2_ptr);
    dump_id_data(id_1, id_2);

    /* -----------------------------------     */
    /* xor ids                                   */
    /* -----------------------------------     */
    *p_val = id_1 = id_1 ^ id_2;
    CCCI_LOGD("random pattern:0x%x", id_1);

_end:
    close(fd);
    return 0;

_fail:
    CCCI_LOGE("failure occured!! ret:%d\n", ret);
    if (fd >= 0)
        close(fd);

    return -1;
}
