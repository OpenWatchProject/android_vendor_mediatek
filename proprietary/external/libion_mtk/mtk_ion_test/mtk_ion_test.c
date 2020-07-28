/*
 *  mtk_ion_test.c
 *
 * Memory Allocator functions for ion
 *
 *   MediaTek Inc. (C) 2011. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "include/mtk_ion_test.h"

//#pragma GCC optimize ("O0")

/*param for map*/
int prot = PROT_READ | PROT_WRITE;
int map_flags = MAP_SHARED;
unsigned long cache_sync_size = 0;
unsigned long cache_sync_va = 0;


/*param for ion buffer*/
unsigned int bufsize = SZ_32M;
unsigned int heap_id_mask = 1 << 10; //default multi-media-heap
int align = 0;
int alloc_flags = 0;
int fd = 0;

/* param for multi-thread test
 * multi_thread_num: how many thread will run simultaneously
 * multi_thread_func_id: test function id.
 */
int multi_thread_num = 1;
int multi_thread_func_id = 0;
int multi_thread_delay = 0;
int ion_func_delay = 0;

int main(int argc, char* argv[])
{
    //index of struct option opts
    int i = 0;
    int c = 0;

    static struct option opts[] = {
            {"thread_count", required_argument, 0, 'c'},
            {"thread_func_id", required_argument, 0, 'i'},
            {"thread_delay_ms", required_argument, 0, 'd'},
            {"cache_sync_va", required_argument, 0, 'v'},
            {"cache_sync_size", required_argument, 0, 's'},
            {"ion_clean", no_argument, 0, 'p'},
            {"buffer_size", required_argument, 0, 'b'},
            {"heap_id_mask", required_argument, 0, 'm'},
            {"help", no_argument, 0, 'h'},
            {0,0,0,0},
    };

    while (1) {
        c = getopt_long(argc, argv, "c:i:d:v:s:pb:m:h", opts, &i);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            show_usage();
            break;

        /* buffer info */
        case 'm':
            heap_id_mask = 1 << (atol(optarg));
            break;
        case 'b':
            bufsize = atol(optarg);
            break;

        /* cache sync info */
        case 'v':
            cache_sync_va = atol(optarg);
            break;
        case 's':
            cache_sync_size = atol(optarg);
            break;

        /*multi-thread test cmd*/
        case 'c':
            multi_thread_num = atol(optarg);
            break;
        case 'i':
            multi_thread_func_id = atol(optarg);
            break;
        case 'd':
            multi_thread_delay = atol(optarg);
            break;

        case 'p':
            IONMSG("[%c] ion clean: start\n", c);
            //ion_clean;
            IONMSG("[%c] ion clean: end  \n", c);
            break;
        default:
            IONMSG("[%c] start\n", c);
            IONMSG("[%c] end  \n", c);
        }
    }

    IONMSG("len 0x%x, align 0x%x, map_flags %d, prot %d, heap_mask %d,"
           " alloc_flags %d\n", bufsize, align, map_flags, prot,
           heap_id_mask , alloc_flags);

    ion_multithread_test();

    return 0;
    //ion_mmap_test();
}


static
int ion_config_buffer_sample_code(int ion_fd, int handle, int module_id, int security, int coherent)
{
    struct ion_mm_data mm_data;

    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = module_id;
    mm_data.config_buffer_param.security = security;
    mm_data.config_buffer_param.coherent = coherent;
    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &mm_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Config Buffer failed!\n");
        return -1;
    }

    return 0;
}

static
int ion_get_phys_sample_code(int ion_fd, int handle)
{
    struct ion_sys_data sys_data;

    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Get Phys failed!\n");
        return -1;
    }
    IONMSG("child Physical address = 0x%lx, len = %ld\n",
           sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);

    return 0;
}

static
int ion_cache_sync_without_va_sample_code(int ion_fd, int handle)
{
    int ret = 0;
    struct ion_sys_data sys_data;

    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = handle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_CLEAN_BY_RANGE;
    IONMSG("Clean by range.\n");
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
    }

    IONMSG("Invalid by range.\n");
    sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
    }

    IONMSG("Flush by range.\n");
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
    }

    return ret;
}

static
int ion_cache_sync_va_sample_code(int ion_fd, int handle, void* va, unsigned int size)
{
    int ret = 0;
    struct ion_sys_data sys_data;

    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = handle;
    sys_data.cache_sync_param.va = va;
    sys_data.cache_sync_param.size = size;
    sys_data.cache_sync_param.sync_type = ION_CACHE_CLEAN_BY_RANGE;
    IONMSG("Clean by range.\n");
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
    }

    IONMSG("Invalid by range.\n");
    sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
    }
    IONMSG("Flush by range.\n");
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
    }

    return ret;
}

static
int ion_set_get_debug_info(int ion_fd, int handle, const char * str, 
    unsigned int value1, unsigned int value2, unsigned int value3, unsigned int value4)
{
    int ret = 0;

    struct ion_mm_data set_debug_info_mm_data;
    set_debug_info_mm_data.mm_cmd = ION_MM_SET_DEBUG_INFO;
    set_debug_info_mm_data.buf_debug_info_param.handle = handle;

    strncpy(set_debug_info_mm_data.buf_debug_info_param.dbg_name, str, strlen(str));
    set_debug_info_mm_data.buf_debug_info_param.value1 = value1;
    set_debug_info_mm_data.buf_debug_info_param.value2 = value2;
    set_debug_info_mm_data.buf_debug_info_param.value3 = value3;
    set_debug_info_mm_data.buf_debug_info_param.value4 = value4;

    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &set_debug_info_mm_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] set buffer debug info!\n");
        ret = -1;
    }

    struct ion_mm_data get_debug_info_mm_data;
    get_debug_info_mm_data.mm_cmd = ION_MM_GET_DEBUG_INFO;
    get_debug_info_mm_data.buf_debug_info_param.handle = handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &get_debug_info_mm_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] set buffer debug info!\n");
        ret = -1;
    }

    IONMSG( "IOCTL[ION_IOC_CUSTOM] get debug info:"
            "dbg_name = %s, value1 = %d, value2 = %d, value3 = %d, value4 = %d.\n",
            get_debug_info_mm_data.buf_debug_info_param.dbg_name,
            get_debug_info_mm_data.buf_debug_info_param.value1,
            get_debug_info_mm_data.buf_debug_info_param.value2,
            get_debug_info_mm_data.buf_debug_info_param.value3,
            get_debug_info_mm_data.buf_debug_info_param.value4);

    return ret;
}

int ion_custom_ioctl_test(void)
{
    unsigned int i;
    int ion_fd;
    ion_user_handle_t handle;
    int share_fd;
    volatile char* pBuf;
    pid_t pid;
    int ret;

    ion_fd = ion_open();

    ret = ion_alloc(ion_fd, bufsize, align, heap_id_mask, alloc_flags, &handle);
    if (ret) {
        IONMSG("%s alloc failed\n", __func__);
        goto exit3;
    }

    ret = ion_share(ion_fd, handle, &share_fd);
    if (ret) {
        IONMSG("%s share failed\n", __func__);
        goto exit3;
    }

    pBuf = ion_mmap(ion_fd, NULL, bufsize, prot, map_flags, share_fd, 0);
    if (!pBuf) {
        IONMSG("Cannot map ion buffer.\n");
        goto exit1;
    } else {
        IONMSG("ion_map: pBuf = 0x%s\n", pBuf);
    }

    // change buffer content
    for (i = 0; i < bufsize; i +=4 ) {
        *(volatile unsigned int*)(pBuf + i) = i;
    }

    //check buffer content
    for (i = 0; i < bufsize; i += 4) {
        if(*(volatile unsigned int*)(pBuf + i) != i) {
            IONMSG("ion_test: owner read error !!\n");
        }
    }

    //ion_processes_share_test_by_binder();

    IONMSG("share buffer to child!!\n");
    pid = fork();
    if (pid == 0) {
        //child
        ion_user_handle_t handle;
        ion_fd = open("/dev/ion", O_RDONLY);
        if (ion_fd < 0) {
            IONMSG("Cannot open ion device.\n");
            return 0;
        }

        ion_import(ion_fd, share_fd, &handle);
        pBuf = ion_mmap(ion_fd, NULL, bufsize, prot, map_flags, share_fd, 0);
        IONMSG("ion_test: map child pBuf = 0x%s\n", pBuf);
            if (!pBuf) {
            IONMSG("Cannot map ion buffer.\n");
            goto thread_exit;
        }

        for (i=0; i<bufsize; i+=4) {
            if(*(volatile unsigned int*)(pBuf+i) != i)
                IONMSG("ion_test: child read error 0x%x!=0x%x!!\n", *(volatile unsigned int*)(pBuf+i),i);
        }
        IONMSG("child verify done!\n");


        ion_config_buffer_sample_code(ion_fd, handle, 1, 0, 1);

        ion_get_phys_sample_code(ion_fd, handle);

thread_exit:
        ion_munmap(ion_fd, (void *)pBuf, bufsize);
        ion_share_close(ion_fd, share_fd);
        ret = ion_free(ion_fd, handle);

        if (ret) {
            IONMSG("%s failed,fd=%d\n", __func__, ion_fd);
            return 0;
        }
        close(ion_fd);

        IONMSG("ion_test: child exit\n");
        exit(0);
    }

    sleep(2);
    IONMSG("parent process goes...\n");

    ion_config_buffer_sample_code(ion_fd, handle, 1, 0, 1);
    ion_set_get_debug_info(ion_fd, handle, "mtk_ion_test_debug", 1, 2, 3, 4);

    ion_get_phys_sample_code(ion_fd, handle);

    /*old flow, maybe phase out someday, and sync without va will cause ke*/
    ion_cache_sync_without_va_sample_code(ion_fd, handle);

    //ion_cache_sync_va_sample_code(ion_fd, handle, (void *)va, size_t size);

exit1:
    ion_munmap(ion_fd, (void *)pBuf, bufsize);
//exit2:
    ion_share_close(ion_fd, share_fd);
    if (ion_free(ion_fd, handle)) {
        IONMSG("IOCTL[ION_IOC_FREE] failed!\n");
        return 0;
    }
exit3:
    ion_close(ion_fd);
    IONMSG("ION test done!\n");

    return 0;
}

void ion_mmap_test(void)
{
    int ion_fd;
    int share_fd;
    volatile char* pBuf = NULL;
    int ret = 0;

    ion_fd = ion_open();
    if (ion_fd < 0) {
        IONMSG("Cannot open ion device.\n");
        return;
    }

    ret = ion_alloc_fd(ion_fd, bufsize, 0, ION_HEAP_MULTIMEDIA_MASK, 3, &share_fd);
    if (ret) {
        IONMSG("IOCTL[ion_alloc_fd] failed!\n");
        goto out;
    }

    //map size must less than buffer alloc size, or will cause exception
    pBuf = ion_mmap(ion_fd, NULL, bufsize, prot, map_flags, share_fd, 0);
    if (!pBuf) {
        IONMSG("Cannot map ion buffer.\n");
        goto out;
    } else {
        IONMSG("ion_map: pBuf = 0x%s\n", pBuf);
    }

    *pBuf = 0;
    IONMSG("va < (map_sz|buf_sz) pass\n");

    /* only can oprate mappped buffer, out of range will cause exception in uasan project */
    *(pBuf + bufsize * 2 - 10) = 1;
    IONMSG("buf_sz < va < map_sz pass\n");

out:
    ion_munmap(ion_fd, (void *)pBuf, bufsize);
    close(ion_fd);
    return;
}

/* ion cache sync va test
 * test ion cache sync speed
 * Notice:
 *    1. bufsize can be specified by cmd [-b bufsize, eg: -b 4096]
 *    2. Need record cache sync speed.
 * except result: No exception, and no DB dump
 */
void ion_cache_sync_va_test(void) {
    int ret = 0;
    struct ion_sys_data sys_data;
    ion_user_handle_t handle;
    int share_fd = 0;
    int fd = 0, i = 0;
    long start = 0, end = 0;
    unsigned int *pBuf = NULL;

    fd = ion_open();
    if (fd < 0) {
        IONMSG("ion_open fail fail\n, ret = %d\n", fd);
        return;
    }

    ret = ion_alloc(fd, bufsize, align, heap_id_mask, alloc_flags, &handle);
    ret += ion_share(fd, handle, &share_fd);
    if (ret) {
        IONMSG("ion_alloc_fd fail\n, ret = %d\n", ret);
        return;
    }

    pBuf = ion_mmap(fd, NULL, bufsize, prot, map_flags, share_fd, 0);
    if (!pBuf) {
        IONMSG("Cannot map ion buffer.\n");
        return;
    } else {
        IONMSG("ion_map: pBuf = 0x%p\n", pBuf);
    }
    // change buffer content
    for (i = 0; i < (bufsize / 4); i++ ) {
        *(volatile unsigned int*)(pBuf + i) = i;
    }
    IONMSG("pBuf end addr + 1 = 0x%p\n", (pBuf + i));

    //check buffer content
    for (i = 0; i < (bufsize / 4); i ++) {
        if(*(volatile unsigned int*)(pBuf + i) != i) {
            IONMSG("ion_test: owner read error !!\n");
        }
    }

    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = handle;
    sys_data.cache_sync_param.va = cache_sync_va ? (void *)cache_sync_va: pBuf;
    sys_data.cache_sync_param.size = cache_sync_size ? cache_sync_size: bufsize;
    sys_data.cache_sync_param.sync_type = ION_CACHE_CLEAN_BY_RANGE;
    IONMSG("Clean by range.\n");

    start = ION_PRINT_TIME_NAME("start");
    if (ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
        return;
    }
    end = ION_PRINT_TIME_NAME("end");
    IONMSG("clean size: %ld[byte] time: %ld[ms]\n",
            cache_sync_size ? cache_sync_size: bufsize,
            (end - start) / 1000);
//>>>>>>>>>>>>>>>>
    IONMSG("Invalid by range.\n");
    sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
    start = ION_PRINT_TIME_NAME("start");
    if (ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data)) {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
        return;
    }
    end = ION_PRINT_TIME_NAME("end");
    IONMSG("clean size: %ld[byte] time: %ld[ms]\n",
            cache_sync_size ? cache_sync_size: bufsize,
            (end - start) / 1000);
//>>>>>>>>>>>>>>>>
    IONMSG("Flush by range.\n");
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
    start = ION_PRINT_TIME_NAME("start");
    if (ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data))
    {
        IONMSG("IOCTL[ION_IOC_CUSTOM] Cache sync failed!\n");
        ret = -1;
        return;
    }
    end = ION_PRINT_TIME_NAME("end");
    IONMSG("clean size: %ld[byte] time: %ld[ms]\n",
            cache_sync_size ? cache_sync_size: bufsize,
            (end - start) / 1000);
//>>>>>>>>>>>>>>>>
    ion_munmap(fd, (void *)pBuf, bufsize);
    ion_share_close(fd, share_fd);
    ion_free(fd, handle);
    ion_close(fd);
    return;
}

/* ion_alloc_fd multi-thread test */
void *ion_thread_1(void *arg) {
    int param = multi_thread_func_id;

    IONMSG("%s: cmd:%d start\n", __func__, param);
    switch (param) {
    case 1:
        ion_alloc_fd_test();
        break;
    case 2:
        ion_fd_leak_test();
        break;
    case 3:
        ion_custom_ioctl_test();
        break;
    case 4:
        ion_mmap_test();
        break;
    case 5:
        ion_cache_sync_va_test();
        break;
    case 6:
        ion_alloc_fd_test();
        break;
    case 7:
        ion_custom_ioctl_random_test();
        break;
    case 8:
        ion_alloc_test_for_ref_check();
        break;
    default:
        //ion_alloc_fd(fd, len, align, heap_mask, flags, handle_fd)
        break;
    }
    IONMSG("%s: cmd:%d end  \n", __func__, param);

    return 0;
}

#if 0
/* ion_alloc_test for measure alloc speed
 * Flow:
 * 1. alloc buffers, and free it.
 *    ==> test alloc time (alloc from system)when first time alloc buffer
 * 2. alloc buffers, and free it.
 *    ==> test alloc time (alloc from system)when first time alloc buffer
 */
int ion_alloc_test(void) {
}
#endif

/* ion_alloc_fd test for measure alloc speed
 * Notice: need to record alloc time
 *         bufsize can be specified by cmd[ -b size]
 *         alloc speed need measure two times
 *               1. alloc from system(after reboot, don't lunch any APP)
 *               2. alloc from ION(after running some APP, such as camera)
 * except result: no exception, and not trigger DB dump
 */
int ion_alloc_fd_test(void) {
    int handle_fd[20], ret, alloc_num = 10;
    int fd = 0;
    int i = 0;
    int loop_times = 0; //second time is to check time from ion pool alloc
    long start_time, end_time;
    bool timestamp_f = 0;

    for (loop_times = 0; loop_times < 4; loop_times++) {
        fd = ion_open();
        usleep(10000);

        if (loop_times == 0)
            IONMSG("!!First Time alloc,"
                "To measure alloc speed from system"
                "Please make Sure test it after reboot!!!!\n");
        else
            IONMSG("%d times alloc, to test ion alloc speed\n", loop_times);

        ION_PRINT_TIME_NAME("start");
        for (i = 0; i < alloc_num; i++) {
            start_time = ION_TIME(timestamp_f);
            ret = ion_alloc_fd(fd, bufsize, align, heap_id_mask, alloc_flags, &handle_fd[i]);
            if (ret) {
                IONMSG("--- %s %d alloc_fd error:%s\n", __func__, __LINE__, strerror(ret));
                return ret;
            }
            end_time = ION_TIME(timestamp_f);
            IONMSG("number:%d share_fd:%d, buf_sz:0x%x, spend:%ld us\n",
                        i, handle_fd[i], bufsize, end_time - start_time);
        }
        ION_PRINT_TIME_NAME("alloc end");
        usleep(10000);

        for (i = 0; i < alloc_num; i++ ) {
            IONMSG("%s fd[%d], share_fd[%d]\n", __func__, fd, handle_fd[i]);
            ret = ion_close(handle_fd[i]);
            if (ret < 0) {
                IONMSG("%s =%d= close failed: %s share_fd=%d\n",
                        __func__, i, strerror(ret), handle_fd[i]);
    //            aee_system_warning()
            }
        }
        ion_close(fd);
        IONMSG("%s free done! fd = %d\n", __func__, fd);
    }
    return ret;
}

/* test function with multi-thread
 */
void ion_multithread_test(void) {
    pthread_t thread1;
    int ret;
    int i = 0;
    char thread_name[31];


    fd = ion_open();
    if (fd < 0) {
        IONMSG("open /dev/ion failed: %s", strerror(errno));
        return;
    }

    for (i = 0; i < multi_thread_num; i++) {
        //update thread name
        snprintf(thread_name, 30, "ion-thread_test-%d", i + 1);

        //create thread to do something
        ret = pthread_create(&thread1, NULL, ion_thread_1, thread_name);
        if (ret != 0) {
            IONMSG("Thread Create fail\n");
            continue;
        }

        //do detach to unblock main thread
        ret = pthread_detach(thread1);

        //ms delay for next thread;
        usleep(multi_thread_delay * 1000);
    }

    //keap main thread always running
    while (1) {
        usleep(10000000);
        IONMSG("%s running\n", __func__);
    }

}

void show_usage(void){

    IONMSG("Usage:\n");
    IONMSG("-i, func id\n");
    IONMSG("---\t0: Default value, no function will run\n");
    IONMSG("---\t1: int ion_alloc_fd_test(void);\n");
    IONMSG("---\t2: int ion_fd_leak_test(void);\n");
    IONMSG("---\t3: int ion_custom_ioctl_test(void);\n");
    IONMSG("---\t4: void ion_mmap_test(void);\n");
    IONMSG("---\t5: void ion_cache_sync_va_test(void);\n");
    IONMSG("---\t6: int ion_alloc_fd_test(void);\n");
    IONMSG("---\t7: void ion_custom_ioctl_random_test(void);\n");
    IONMSG("---\t8: \n");
    IONMSG("---\t\n");
    IONMSG("\n\n");

}

static void *ion_random_data(int len) {
    int *p;
    int i;

    IONMSG("data:  ");
    p = malloc(sizeof(int) * len);
    for (i = 0; i < len; i++) {
        *(p + i) = ion_get_random_32();
        IONMSG("%x", *(p + i));
    }
    IONMSG("\n\n");

    return (void *)p;
}


void *ion_custom_random_data(int fd, unsigned int cmd) {

    void *data = NULL;

    IONMSG("%s: fd:%d, cmd: %d\n", __func__, fd, cmd);
    switch(cmd) {
        case ION_CMD_SYSTEM:
            data = ion_random_data(sizeof(struct ion_sys_data));
            //ioctl data print
            break;
        case ION_CMD_MULTIMEDIA:
            data = ion_random_data(sizeof(struct ion_mm_data));
            break;
        default:
            IONMSG("error cmd 0x%x\n", cmd);
            data = NULL;
            break;
    }

    return data;
}

/* ion_custom_ioctl fuzzer test
 * except result: no except, not trigger DB dump
 */
void ion_custom_ioctl_random_test(void) {
    int fd = 0;
    int cmd = 0;
    void * pdata = NULL;
    unsigned int test_times = 0;
    int ret = -1;

    fd = ion_open();
    if (fd < 0) {
        IONMSG("--- ion_open failed, %d\n", fd);
        return;
    } else
        IONMSG("+++ ion dev fd: %d\n", fd);

    while (1) {
        cmd = RAND_32() % 10; // cmd range[0:9]
        pdata = ion_custom_random_data(fd, cmd);

        if (!pdata)
            continue;

        ret = ion_custom_ioctl(fd, cmd, pdata);

        if (ret)
            //failed
            IONMSG("--- test cmd: %d, ret %d, %s\n",
                cmd, ret, strerror(errno));
        else
            //pass
            IONMSG("+++ test cmd: %d, ret %d, %s\n",
                cmd, ret, strerror(errno));
        if(test_times++ == 0xfffffff) {
            IONMSG("test done!\n");
            break;
        }

        free(pdata);
    }

}


/* func: ion_fd_leak_test
 * Notice: only eng version can test fd leak
 * test fd debug patch: platform/system/core/+/2556238
 * except result: trigger fd leak db
 */
void ion_fd_leak_test(void) {
    int fd = 0;
    int ret = 0;
    int handle_fd = 0;

    fd = ion_open();
    if (fd < 0)
        return ;

    // fd leak test don't support change buffer size
    while(1) {
        ret = ion_alloc_fd(fd, 4096, align, heap_id_mask, alloc_flags, &handle_fd);
        if (ret) {
            IONMSG("--- ion_alloc_fd failed with code %d: %s", ret, strerror(errno));
            break;
        } else {
            IONMSG("+++ dev fd:%d, share_fd: %d\n", fd, handle_fd);
        }
    }
    getchar();
    // dump all share fd
    close(fd);
    // check share_fd here, if all share fd is closed after close device fd
    // interms of theory, share fd will not be closed after device fd(client),
    //    only after process be killed, share fd will be closed by system.
    getchar();
}

/* sample alloc and share UT code
 * Goal: check buffer freerence change after ion api
 * notice: in usleep time[usleep(5000000)]
 *         need to dump /d/ion/ion_mm_heap to check buffer reference
 * except result: manually check buffer reference
 */
void ion_alloc_test_for_ref_check(void) {
    int fd = 0;
    int ret = 0;
    int share_fd = 0;
    ion_user_handle_t handle = 0;

    fd = ion_open();
    IONMSG("[%d]please check buf-reference!!!\n", __LINE__);
    usleep(5000000);

    ret = ion_alloc(fd, bufsize, align, heap_id_mask, alloc_flags, &handle);
    ret += ion_set_get_debug_info(fd, handle, "Guangming_T", 0x11, 0x22, 0x33, 0x44);
    if (ret) {
        IONMSG("--- ion_alloc failed with code %d: %s", ret, strerror(errno));
        goto out2;
    } else {
        IONMSG("+++ [ion_alloc]dev fd:%d, handle: %d\n", fd, handle);
    }
    IONMSG("[%d]please check buf-reference!!!\n", __LINE__);
    usleep(5000000);

    ret = ion_share(fd, handle, &share_fd);
    if (ret) {
        IONMSG("--- ion_share failed with code %d: %s", ret, strerror(errno));
        goto out1;
    } else {
        IONMSG("+++ [ion_share]dev fd:%d, share_fd: %d\n", fd, share_fd);
    }
    IONMSG("[%d]please check buf-reference!!!\n", __LINE__);
    usleep(5000000);

out1:
    ret = ion_share_close(fd, share_fd);
    if (ret) {
        IONMSG("--- ion_share failed with code %d: %s", ret, strerror(errno));
        goto out2;
    } else {
        IONMSG("+++ [ion_share_close]dev fd:%d, share_fd: %d\n", fd, share_fd);
    }
    IONMSG("[%d]please check buf-reference!!!\n", __LINE__);
    usleep(5000000);

out2:
    ret = ion_free(fd, handle);
    if (ret) {
        IONMSG("--- ion_free failed with code %d: %s", ret, strerror(errno));
        goto out3;
    } else {
        IONMSG("+++ [ion_free]dev fd:%d, handle: %d\n", fd, handle);
    }
    IONMSG("[%d]please check buf-reference!!!\n", __LINE__);
    usleep(5000000);

out3:
    close(fd);
    IONMSG("[%d]please check buf-reference!!!\n", __LINE__);
    usleep(5000000);

}

