 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "tlcsecmem.h"
#include "tlc_sec_mem.h"

#define LOG_TAG "TLC_SECMEM"
#include "tlsecmem_log.h"

#define NR_TEST_THREADS     (8)
#define NR_TEST_ROUNDS      (8)

#define CHK_RESULT(ret, expr) \
do { \
    unsigned int _result = (expr); \
    if ((ret) == _result) { \
        LOG_I("[TID%d] <%s> %s == %s", tid, "PASS", #ret, #expr); \
    } else { \
        LOG_E("[TID%d] <%s> %s != %s (0x%x), LINE: %d", tid, "FAIL", #ret, #expr, _result, __LINE__); \
    } \
} while(0)

#define DBG_HINFO(handle) \
do { \
    LOG_I("[TID%d][INFO] %s(0x%x)", tid, #handle, (unsigned int)handle); \
} while(0)

#define DBG_RINFO(handle, refcount) \
do { \
    LOG_I("[TID%d][INFO] %s(0x%x), ref(%d)", tid, #handle, (unsigned int)handle, (unsigned int)refcount); \
} while(0)

#define DBG_MINFO() \
do { \
    UREE_DumpSecurememInfo(); \
} while(0)

typedef struct {
    uint32_t tid;
} tc_param_t;

typedef int32_t (*tc_func_t)(uint32_t tid, tc_param_t *param);

typedef struct {
    uint32_t  tid;
    uint32_t  rounds;
    uint32_t  delay_ms;
    int32_t   result;
    tc_func_t func;
} tc_spec_t;

static pthread_t tc_thread[NR_TEST_THREADS];
static tc_spec_t tc_thread_spec[NR_TEST_THREADS];

static void *tc_thread_entry(void *data)
{
    int32_t  result;
    uint32_t i, tid, rounds;
    tc_spec_t *tc = (tc_spec_t*)data;
    tc_param_t param = {0};

    tid = tc->tid;
    rounds = tc->rounds;

    for (i = 0; i < rounds; i++) {
        result = tc->func(tid, &param);
        if (result) break;

        /* sleep a while */
        usleep(tc->delay_ms);
    }
    tc->result = result;

    pthread_exit(NULL);
}

static int32_t tc_func_00(uint32_t tid, tc_param_t *param)
{
    uint32_t refcount;
    mcResult_t ret = MC_DRV_OK;
    UREE_SECUREMEM_HANDLE handle;

    CHK_RESULT(MC_DRV_OK, tlcMemOpen());
    CHK_RESULT(MC_DRV_OK, UREE_AllocSecuremem(&handle, 0x100000, 0x100000));
    DBG_HINFO(handle);
    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, tlcMemOpen());
    CHK_RESULT(MC_DRV_OK, UREE_ReferenceSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);
    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, UREE_UnreferenceSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);
    DBG_MINFO();
    tlcMemClose();

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);
    DBG_MINFO();
    tlcMemClose();

    return 0;
}

static int32_t tc_func_01(uint32_t tid, tc_param_t *param)
{
    uint32_t refcount;
    mcResult_t ret = MC_DRV_OK;
    UREE_SECUREMEM_HANDLE handle1, handle2, handle3;

    CHK_RESULT(MC_DRV_OK, tlcMemOpen());

    CHK_RESULT(MC_DRV_OK, UREE_AllocSecuremem(&handle1, 0x1000, 0x1000));
    DBG_HINFO(handle1);
    CHK_RESULT(MC_DRV_OK, UREE_AllocSecuremem(&handle2, 0x100, 0x100));
    DBG_HINFO(handle2);
    CHK_RESULT(MC_DRV_OK, UREE_AllocSecuremem(&handle3, 0x10, 0x10));
    DBG_HINFO(handle3);

    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, UREE_ReferenceSecuremem(handle1, &refcount));
    DBG_RINFO(handle1, refcount);
    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, UREE_UnreferenceSecuremem(handle1, &refcount));
    DBG_RINFO(handle1, refcount);
    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecuremem(handle1, &refcount));
    DBG_RINFO(handle1, refcount);
    CHK_RESULT(MC_DRV_OK, UREE_FreeSecuremem(handle2, &refcount));
    DBG_RINFO(handle2, refcount);
    CHK_RESULT(MC_DRV_OK, UREE_FreeSecuremem(handle3, &refcount));
    DBG_RINFO(handle3, refcount);

    DBG_MINFO();

    tlcMemClose();

    return 0;
}

static int tc_basic(void)
{
    uint32_t tid = 0;
    uint32_t refcount;
    mcResult_t ret = MC_DRV_OK;
    UREE_SECUREMEM_HANDLE handle, handle1, handle2, handle3, handle4;

    CHK_RESULT(MC_DRV_OK, tlcMemOpen());
    CHK_RESULT(MC_DRV_OK, tlcMemOpen());

    CHK_RESULT(MC_DRV_OK, UREE_AllocSecuremem(&handle, 0x100000, 0x100000));
    DBG_HINFO(handle);

    CHK_RESULT(MC_DRV_OK, UREE_ReferenceSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_UnreferenceSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);

    /* 0x0000010b = E_TLAPI_INVALID_PARAMETER */
    CHK_RESULT(0x0000010b, UREE_UnreferenceSecuremem(handle, &refcount));
    DBG_RINFO(handle, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_AllocSecurememTBL(&handle1, 0x2000, 0x100000));
    DBG_HINFO(handle1);

    CHK_RESULT(MC_DRV_OK, UREE_AllocSecurememTBL(&handle2, 4, 1024));
    DBG_HINFO(handle2);
    
    CHK_RESULT(MC_DRV_OK, UREE_AllocSecurememTBL(&handle3, 4, 2048));
    DBG_HINFO(handle3);

    CHK_RESULT(MC_DRV_OK, UREE_AllocSecurememTBL(&handle4, 4, 4096));
    DBG_HINFO(handle4);
    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecurememTBL(handle3, &refcount));
    DBG_RINFO(handle3, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecurememTBL(handle2, &refcount));
    DBG_RINFO(handle2, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecurememTBL(handle1, &refcount));
    DBG_RINFO(handle1, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_AllocSecurememTBL(&handle1, 4, 1024));
    DBG_HINFO(handle1);
    DBG_MINFO();

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecurememTBL(handle4, &refcount));
    DBG_RINFO(handle4, refcount);

    CHK_RESULT(MC_DRV_OK, UREE_FreeSecurememTBL(handle1, &refcount));
    DBG_RINFO(handle1, refcount);
    DBG_MINFO();

exit:   
    tlcMemClose();
    tlcMemClose();
    
    return ret;
}

static int tc_multithread(void)
{
    uint32_t i;
    uint32_t result = 0;

    memset(tc_thread, 0, sizeof(tc_thread));
    memset(tc_thread_spec, 0, sizeof(tc_thread_spec));

    for (i = 0; i < NR_TEST_THREADS; i++) {
        tc_thread_spec[i].tid = i;
        tc_thread_spec[i].rounds = NR_TEST_ROUNDS;
        tc_thread_spec[i].delay_ms = (i + 1) * 100;
        tc_thread_spec[i].func = (i % 2) ? tc_func_00 : tc_func_01;
        result = pthread_create(&tc_thread[i], NULL, tc_thread_entry, (void *)&tc_thread_spec[i]);
        LOG_I("[INFO] Create thread%d ret=%d!!", i, result);
    }

    /* sleep a while */    
    usleep(1000000);
    
    for (i = 0; i < NR_TEST_THREADS; i++) {
        LOG_I("[INFO] Wait for thread%d terminated!!", i);
        pthread_join(tc_thread[i], NULL);
    }
    return 0;
}

static int tc_misc(void)
{
    uint32_t tid = 0, i;
    uint32_t refcount;

    for (i = 0; i < 32; i++) {
        CHK_RESULT(MC_DRV_OK, tlcMemOpen());
        usleep(1 * 1000 * 1000);
    }
    for (i = 0; i < 32; i++) {
        tlcMemClose();
        usleep(1 * 1000 * 1000);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int ret;

    LOG_I("[INFO] <START>");

    /* basic unit test */
    ret = tc_basic();

    /* multi thread test */
    ret = tc_multithread();

    /* misc test */
    //ret = tc_misc();

    LOG_I("[INFO] <END>");

    return ret;
}

