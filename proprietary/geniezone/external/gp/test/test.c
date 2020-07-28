#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log/log.h>
#include <stdbool.h>
#include "unittest.h"
#include "tee_client_api.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "gz_gp_test"
#endif

#define TEST_STR_SIZE 512

INIT_UNITTESTS
static const char test_srv_name[] = "com.mediatek.geniezone.test";

static int check_gp_inout_mem(char *buffer)
{
    int i;

    for (i = 0; i < TEST_STR_SIZE; i++)
    {
        if (i%3)
        {
            if (buffer[i] != 'c') return 1;
        }
        else
        {
            if (buffer[i] != 'd') return 1;
        }
    }
    return 0;
}

static void test_Session()
{
    TEEC_Result ret;
    TEEC_Context context;
    TEEC_Session session;

    TEST_BEGIN("test_Session");

    ret = TEEC_InitializeContext(NULL, &context);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "initial context");

    ret = TEEC_OpenSession(&context, &session, (TEEC_UUID *)test_srv_name, 0, NULL, NULL, NULL);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "open session to echo service");

    TEEC_CloseSession(&session);

    TEEC_FinalizeContext(&context);

    TEST_END;
}

static void test_ServiceCall()
{
    TEEC_Result ret;
    TEEC_Context context;
    TEEC_Session session;
    char buf1[TEST_STR_SIZE] = {0}, buf2[TEST_STR_SIZE] = {0};
    TEEC_Operation operation;
    int i, tmp;

    TEST_BEGIN("test_ServiceCall");

    ret = TEEC_InitializeContext(NULL, &context);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "initial context");

    ret = TEEC_OpenSession(&context, &session, (TEEC_UUID *)test_srv_name, 0, NULL, NULL, NULL);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "open session to echo service");

    for (i = 0; i < TEST_STR_SIZE; i++) buf2[i] = 'c';
    operation.params[0].value.a = 0x1230;
    operation.params[1].tmpref.buffer = (void *)buf1;
    operation.params[1].tmpref.size = TEST_STR_SIZE;
    operation.params[2].tmpref.buffer = (void *)buf2;
    operation.params[2].tmpref.size = TEST_STR_SIZE;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_MEMREF_TEMP_INOUT, TEEC_VALUE_OUTPUT);

    ret = TEEC_InvokeCommand(&session, 0x1234, &operation, NULL);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "echo invoke command");

    tmp = strcmp((char *)operation.params[1].tmpref.buffer, "sample data 1!!");
    EXPECT_EQ_EXIT(0, tmp, "echo service output buffer");

    tmp = check_gp_inout_mem(buf2);
    EXPECT_EQ_EXIT(0, tmp, "echo service inout buffer");

    EXPECT_EQ_EXIT(99, operation.params[3].value.a, "echo service output value");

    TEEC_CloseSession(&session);

    TEEC_FinalizeContext(&context);

    TEST_END;
}

static void test_SharedMem()
{
    TEEC_Result ret;
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation operation;
    TEEC_SharedMemory sharedMem;
    char *buf = NULL;
    int i, shm_size = 2*1024*1024, stat[2] = {0};

    TEST_BEGIN("test_SharedMem");

    buf = (char *)malloc(shm_size);

    for (i = 0; i < (shm_size-1); i++)
        buf[i] = 'a';
    buf[i] = '\0';

    for (i = 0; i < (shm_size); i++)
    {
        if (buf[i] == 'a') stat[0]++;
        else if (buf[i] == 'b')	stat[1]++;
    }
    ALOGI("init share mem: stat[0] = %d, stat[1] = %d \n", stat[0], stat[1]);
    EXPECT_EQ_EXIT(2097151, stat[0], "init share mem #a");
    EXPECT_EQ_EXIT(0, stat[1], "init share mem #b");

    ret = TEEC_InitializeContext(NULL, &context);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "initial context");

    ret = TEEC_OpenSession(&context, &session, (TEEC_UUID *)test_srv_name, 0, NULL, NULL, NULL);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "open session to echo service");

    sharedMem.buffer = (void *)buf;
    sharedMem.size = shm_size;
    ret = TEEC_RegisterSharedMemory(&context, &sharedMem);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "register share mem");

    operation.params[0].value.a = sharedMem.imp.gzSharedMem;
    operation.params[1].value.b = shm_size;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);

    ret = TEEC_InvokeCommand(&session, 0x5588, &operation, NULL);
    EXPECT_EQ_EXIT(TEEC_SUCCESS, ret, "echo share mem call");

    for (i = 0; i < 2; i++)	stat[i] = 0;
    for (i = 0; i < (shm_size); i++)
    {
        if (buf[i] == 'a') stat[0]++;
        else if (buf[i] == 'b')	stat[1]++;
    }
    ALOGI("after share mem test: stat[0] = %d, stat[1] = %d \n", stat[0], stat[1]);
    EXPECT_EQ_EXIT(0, stat[0], "after share mem test #a");
    EXPECT_EQ_EXIT(2097151, stat[1], "after share mem test #b");

    TEEC_ReleaseSharedMemory(&sharedMem);
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);

    free(buf);

    TEST_END;
}

int main(int argc, char *argv[])
{
    RESET_UNITTESTS;

    test_Session();
    test_ServiceCall();
    test_SharedMem();

    REPORT_UNITTESTS;
    return 0;
}
