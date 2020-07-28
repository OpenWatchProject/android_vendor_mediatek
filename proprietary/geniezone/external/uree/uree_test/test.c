#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log/log.h>
#include <tz_cross/trustzone.h>
#include <tz_cross/ta_test.h>
#include <uree/system.h>
#include <uree/mem.h>
#include <stdbool.h>
#include "unittest.h"
#include <time.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <pthread.h>

#include <sys/mman.h>

#include <ion.h>
#include <ion/ion.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#ifdef ION_NUM_HEAP_IDS
#undef ION_NUM_HEAP_IDS
#endif
#include <linux/mtk_ion.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef TRUSTONIC_TEE_ENABLED
#include <MobiCoreDriverApi.h>
#endif
#ifdef GP_TEE_ENABLED
#include <tee_client_api.h>
#endif

#include <tz_private/log.h>
#include <tz_cross/ta_fbc.h>
#include <tz_cross/ta_DL.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "uree_test"
#endif

#define TEST_STR_SIZE 512

#define TEE_ID 0
#if TEE_ID == 0
#define SRV_NAME(name)   "com.mediatek.geniezone." name
#else
#define SRV_NAME(name)   "nebula.com.mediatek.geniezone." name
#endif

INIT_UNITTESTS
static const char echo_srv_name[] = SRV_NAME("srv.echo");
static const char mem_srv_name[] =  SRV_NAME("srv.mem");
/*for dynamic loading*/
static const char DL_srv_name[] =  SRV_NAME("srv.DL");

static int check_gp_inout_mem(char *buffer)
{
	int i;

	for (i = 0; i < TEST_STR_SIZE; i++)
		if (i % 3) {
			if (buffer[i] != 'c')
				return 1;
		} else {
			if (buffer[i] != 'd')
				return 1;
		}
	return 0;
}

static void MTEEversionInUREE()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	char buf[TEST_STR_SIZE] = {0};

	param[0].mem.size = 256;
	param[0].mem.buffer = (void *)buf;

	TEST_BEGIN(" Get MTEE version! (show MTEE version in UREE)");

	ret = UREE_CreateSession(mem_srv_name, &session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create mem srv session");

	types = TZ_ParamTypes1(TZPT_MEM_OUTPUT);

	ret = UREE_TeeServiceCall(session, TZCMD_MTEE_VERSION, types, param);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "mem service call");

	DBG_LOG("MTEE version is: %s \n",(char *)param[0].mem.buffer);
	ALOGI("MTEE version is: %s \n",(char *)param[0].mem.buffer);

	ret = UREE_CloseSession(session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "close mem srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_Session()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session, session2;

	TEST_BEGIN("test_Session");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	/* connect to unknown server */
	ret = UREE_CreateSession("unknown.server", &session2);
	EXPECT_EQ_EXIT(TZ_RESULT_ERROR_COMMUNICATION, ret, "connect to unknown server");

	/* null checking */
	ret = UREE_CreateSession(echo_srv_name, NULL);
	EXPECT_EQ_EXIT(TZ_RESULT_ERROR_BAD_PARAMETERS, ret, "create session null checking");

	/* connect to the same server multiple times*/
	ret = UREE_CreateSession(echo_srv_name, &session2);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session 2");

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");
	ret = UREE_CloseSession(session2);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session 2");

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_ServiceCall()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	char buf1[TEST_STR_SIZE] = {0}, buf2[TEST_STR_SIZE] = {0};
	MTEEC_PARAM param[4];
	int i, tmp;
	uint32_t types;

	TEST_BEGIN("test_ServiceCall");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	for (i = 0; i < TEST_STR_SIZE; i++) buf2[i] = 'c';
	param[0].value.a = 0x1230;
	param[1].mem.buffer = (void *)buf1;
	param[1].mem.size = TEST_STR_SIZE;
	param[2].mem.buffer = (void *)buf2;
	param[2].mem.size = TEST_STR_SIZE;
	types = TZ_ParamTypes4(TZPT_VALUE_INPUT, TZPT_MEM_OUTPUT,
		TZPT_MEM_INOUT, TZPT_VALUE_OUTPUT);

	ret = UREE_TeeServiceCall(session, TZCMD_TEST_SYSCALL, types, param);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "echo service call");

	tmp = strcmp((char *)param[1].mem.buffer, "sample data 1!!");
	EXPECT_EQ(0, tmp, "echo service output buffer");

	tmp = check_gp_inout_mem(buf2);
	EXPECT_EQ(0, tmp, "echo service inout buffer");

	EXPECT_EQ(99, param[3].value.a, "echo service output value");

	ret = UREE_CloseSession(session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_MultiThread(uint32_t thread_num, uint32_t test_max_cnt)
{
	uint32_t test_cnt = 0;
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	char buf1[TEST_STR_SIZE] = {0}, buf2[TEST_STR_SIZE] = {0};
	MTEEC_PARAM param[4];
	int i;
	uint32_t types;

	TEST_BEGIN("test_MultiThread");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	for (i = 0; i < TEST_STR_SIZE; i++) buf2[i] = 'c';
	param[0].value.a = thread_num;
	param[0].value.b = 0;
	param[1].mem.buffer = (void *)buf1;
	param[1].mem.size = TEST_STR_SIZE;
	param[2].mem.buffer = (void *)buf2;
	param[2].mem.size = TEST_STR_SIZE;
	types = TZ_ParamTypes4(TZPT_VALUE_INPUT, TZPT_MEM_OUTPUT,
		TZPT_MEM_INOUT, TZPT_VALUE_OUTPUT);

	for (test_cnt = 0 ; test_cnt < test_max_cnt ; test_cnt++) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_MULTI_THREAD,
			types, param);
		DBG_LOG ("test_MultiThread() at %d test\n", test_cnt);
		EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "echo service call");
		if (param[3].value.a != 66) break;
	}

	EXPECT_EQ_EXIT(66, param[3].value.a, "echo service output value");

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_pThread(uint32_t thread_num, uint32_t test_max_cnt)
{
	uint32_t test_cnt = 0;
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	char buf1[TEST_STR_SIZE] = {0}, buf2[TEST_STR_SIZE] = {0};
	MTEEC_PARAM param[4];
	int i;
	uint32_t types;

	TEST_BEGIN("test_MultiThread");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");


	for (i = 0; i < TEST_STR_SIZE; i++)
		buf2[i] = 'c';

	param[0].value.a = thread_num;
	param[0].value.b = 0;
	param[1].mem.buffer = (void *) buf1;
	param[1].mem.size = TEST_STR_SIZE;
	param[2].mem.buffer = (void *) buf2;
	param[2].mem.size = TEST_STR_SIZE;
	types = TZ_ParamTypes4(TZPT_VALUE_INPUT, TZPT_MEM_OUTPUT,
		TZPT_MEM_INOUT, TZPT_VALUE_OUTPUT);

	for (test_cnt = 0 ; test_cnt < test_max_cnt ; test_cnt++) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_PTHREAD, types, param);
		DBG_LOG ("test_MultiThread() at %d test\n", test_cnt);
		EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "echo service call");
		if (param[3].value.a != 66) break;
	}

	EXPECT_EQ_EXIT(66, param[3].value.a, "echo service output value");

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static TZ_RESULT _alloc_ion_buf(int *ion_fd, int *ion_hd, int *map_fd,
	unsigned char **buf, int size)
{
	TZ_RESULT ret;
	unsigned int flags = ION_FLAG_CACHED;

	*ion_fd = ion_open();
	if (*ion_fd < 0) {
		DBG_LOG("ion_fd < 0. fail: ion_fd=%d\n", *ion_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	ret = ion_alloc(*ion_fd, size, PAGE_SIZE, ION_HEAP_MULTIMEDIA_MASK, flags, ion_hd);
	if (ret) {
		DBG_LOG("ion_alloc fail:ret=0x%x\n", ret);
		ion_close(*ion_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	ret = ion_map(*ion_fd, *ion_hd, size, PROT_READ | PROT_WRITE, MAP_SHARED, 0, buf, map_fd);
	if ((ret) || (buf == NULL)) {
		DBG_LOG("ion_map fail: ret = 0x%x\n", ret);
		ret = ion_free(*ion_fd, *ion_hd);
		ret = ion_close(*ion_fd);
		return TZ_RESULT_ERROR_GENERIC;
	} else {
		DBG_LOG("[%d]shmem [buf] alloc done.\n", __LINE__);
	}

	return TZ_RESULT_SUCCESS;
}

static TZ_RESULT _free_ion_buf(int ion_fd, int ion_hd, int map_fd, void *buf, int size)
{
	if (ion_fd > 0) {
		if (buf) {
			munmap(buf, size);
			close(map_fd);
		}
		if (ion_hd != 0)
			ion_free(ion_fd, ion_hd);

		ion_close(ion_fd);
	} else {
		DBG_LOG("ion_fd fail:0x%x\n", ion_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

static void test_ImageRecognition(char *img)
{
	FILE *img_fp;
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session, mem_session = 0;
	UREE_SHAREDMEM_PARAM shm_param;
	UREE_SHAREDMEM_HANDLE shm_handle;
	MTEEC_PARAM param[4];

	TEST_BEGIN("test_ImageRecognition");

	unsigned char *buf = NULL;
	uint32_t types;

	long shm_size;

	/*for shared memory buf*/
	int ion_fd = -1;
	ion_user_handle_t ion_hd = -1;
	int map_fd = -1;

	img_fp = fopen(img, "r");
	if (img_fp == NULL) {
		ERR_LOG("failed to open image file %s\n", img);
		goto img_recog_err;
	}

	fseek(img_fp, 0L, SEEK_END);
	shm_size = ftell(img_fp);
	fseek(img_fp, 0L, SEEK_SET);

	//buf = (char*) memalign(PAGE_SIZE, shm_size); //orig way, may be fail
	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd, &ion_hd, &map_fd, &buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto img_recog_err_buf;

	EXPECT_NEQ(buf, NULL, "shmem [buf] null!!!");
	if (buf == NULL) {
		ERR_LOG("failed to allocate buffer\n");
		goto img_recog_err_buf;
	}

	if (fread(buf, 1, shm_size, img_fp) != (size_t)shm_size) {
		ERR_LOG("failed to read image content\n");
		goto img_recog_err_buf;
	}

	ret = UREE_CreateSession(echo_srv_name, &session);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("failed to connect to echo server, ret=0x%x\n", ret);
		goto img_recog_err_session;
	}

	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("failed to connect to mem server, ret=0x%x\n", ret);
		goto img_recog_err_session;
    }

	shm_param.buffer = (void*)buf;
	shm_param.size = shm_size;
	shm_param.region_id = 0;
	ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("failed to register share memory, ret=0x%x\n", ret);
		goto img_recog_err_session;
	}

	param[0].value.a = shm_handle;
	param[1].value.b = shm_size;
	types = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT);

	ret = UREE_TeeServiceCall(session, TZCMD_TEST_SHM_IMAGE_RECOGNITION,
		types, param);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("MTEE service call failed, ret=0x%x\n", ret);

	ret = UREE_UnregisterSharedmem(mem_session, shm_handle);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("failed to unreigster share memory, ret=0x%x\n", ret);

img_recog_err_session:
	ret = UREE_CloseSession(session);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("failed to close echo session, ret=0x%x\n", ret);

	ret = UREE_CloseSession(mem_session);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("failed to close mem session, ret=0x%x\n", ret);

img_recog_err_buf:
	//free(buf);  //orig way
	ret = _free_ion_buf(ion_fd, ion_hd, map_fd, buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

	fclose(img_fp);

img_recog_err:
	TEST_END;
	REPORT_UNITTESTS;
}

static void test_SharedMem()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session, mem_session;
	UREE_SHAREDMEM_PARAM shm_param;
	UREE_SHAREDMEM_HANDLE shm_handle;
	MTEEC_PARAM param[4];
	unsigned char *buf = NULL;
	uint32_t types;
	int i, shm_size = 2*1024*1024, stat[2] = {0};

	/*for shared memory buf*/
	int ion_fd = -1;
	ion_user_handle_t ion_hd = -1;
	int map_fd = -1;

	TEST_BEGIN("test_SharedMem");

	//buf = (char *) memalign(PAGE_SIZE, shm_size);  //orig way, may be fail
	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd, &ion_hd, &map_fd, &buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	EXPECT_NEQ(buf, NULL, "shmem [buf] null!!!");
	if(buf == NULL) {
		DBG_LOG("[%d]shmem [buf] alloc fail. null.\n", __LINE__);
		goto err_state;
	}

	memset(buf, 'a', shm_size);

	for (i = 0; i < (shm_size); i++) {
		if (buf[i] == 'a') stat[0]++;
		else if (buf[i] == 'b')	stat[1]++;
	}
	DBG_LOG("init share mem: stat[0] = %d, stat[1] = %d \n", stat[0], stat[1]);
	EXPECT_EQ(shm_size, stat[0], "init share mem #a");
	if(stat[0] != shm_size)
		goto err_state;
	EXPECT_EQ(0, stat[1], "init share mem #b");
	if(stat[1] != 0)
		goto err_state;

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create echo srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create mem srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	shm_param.buffer = (void *) buf;
	shm_param.size = shm_size;
	shm_param.region_id = 0;
	ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "register share mem");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	param[0].value.a = shm_handle;
	param[1].value.b = shm_size;
	types = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT);

	ret = UREE_TeeServiceCall(session, TZCMD_SHARED_MEM_TEST, types, param);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "echo share mem call");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	for (i = 0; i < 2; i++)	stat[i] = 0;
	for (i = 0; i < (shm_size); i++) {
		if (buf[i] == 'a')
			stat[0]++;
		else if (buf[i] == 'b')
			stat[1]++;
	}
	DBG_LOG("after share mem test: stat[0] = %d, stat[1] = %d \n",
		stat[0], stat[1]);
	EXPECT_EQ(0, stat[0], "after share mem test #a");
	if(stat[0] != 0){
		goto err_state;
	}
	EXPECT_EQ(shm_size, stat[1], "after share mem test #b");
	if(stat[1] != shm_size)
		goto err_state;

	ret = UREE_UnregisterSharedmem(mem_session, shm_handle);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "un-register share mem");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_state;

err_state:
	//free(buf);  //orig way
	ret = _free_ion_buf(ion_fd, ion_hd, map_fd, buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_SecureMem()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE mem_session;
	UREE_SECUREMEM_HANDLE mem_handle[4];

	TEST_BEGIN("test Secure Memory");

	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create mem srv session");


	/**** Memory ****/
	DBG_LOG("Secure memory APIs...\n");

	ret = UREE_AllocSecuremem(mem_session, &mem_handle[0], 0, 128);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem 128");
	DBG_LOG("UREE_AllocSecuremem handle = %d.\n", mem_handle[0]);

	ret = UREE_AllocSecuremem(mem_session, &mem_handle[1], 0, 512);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem 512");
	DBG_LOG("UREE_AllocSecuremem handle = %d.\n", mem_handle[1]);

	ret = UREE_AllocSecuremem(mem_session, &mem_handle[2], 0, 1024);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem 1024");
	DBG_LOG("UREE_AllocSecuremem handle = %d.\n", mem_handle[2]);

	ret = UREE_AllocSecuremem(mem_session, &mem_handle[3], 0, 2048);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem 2048");
	DBG_LOG("UREE_AllocSecuremem handle = %d.\n", mem_handle[3]);

	ret = UREE_ReferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "reference secure mem 1");
	ret = UREE_ReferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "reference secure mem 2");

	ret = UREE_UnreferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unreference secure mem 1");
	ret = UREE_UnreferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unreference secure mem 2");
	ret = UREE_UnreferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unreference secure mem 3");

	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_SecureMem_WithTag()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE mem_session;
	UREE_SECUREMEM_HANDLE mem_handle[4];
	const char tag[] = "Client Name";
	const char *tag1 = "Client Name2";

	TEST_BEGIN("test Secure Memory With Tag");

	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create mem srv session");

	ret = UREE_AllocSecurememWithTag(mem_session, &mem_handle[0], 0, 128, tag);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem with tag 128");
	DBG_LOG("UREE_AllocSecurememWithTag handle = %d.\n", mem_handle[0]);

	ret = UREE_AllocSecurememWithTag(mem_session, &mem_handle[1], 0, 256, tag1);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem with tag 256");
	DBG_LOG("UREE_AllocSecurememWithTag handle = %d.\n", mem_handle[1]);

	ret = UREE_ZallocSecurememWithTag(mem_session, &mem_handle[2], 0, 512, tag);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "Zalloc secure mem with tag 512");
	DBG_LOG("UREE_ZallocSecurememWithTag handle = %d.\n", mem_handle[2]);

	ret = UREE_ZallocSecurememWithTag(mem_session, &mem_handle[3], 0, 1024, tag1);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "Zalloc secure mem with tag 1024");
	DBG_LOG("UREE_ZallocSecurememWithTag handle = %d.\n", mem_handle[3]);

	ret = UREE_ReferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "reference secure mem with tag 1");

	ret = UREE_UnreferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unreference secure mem 1");
	ret = UREE_UnreferenceSecuremem(mem_session, mem_handle[0]);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unreference secure mem 1");

	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

#define PMEM_ION_MASK ION_HEAP_MULTIMEDIA_TYPE_PROT_MASK

#ifdef TRUSTONIC_TEE_ENABLED
#define PMEM_DRIVER_UUID \
	{{ 0x08, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x19 }}
#define ION_FLAG_MM_HEAP_INIT_ZERO (1 << 16)

struct pmem_tci_params {
	int command;
	int ret;
	int params[8];
};

static const uint32_t g_device_id = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t g_drv_uuid = PMEM_DRIVER_UUID;
static mcSessionHandle_t pmem_drv_session;
static struct pmem_tci_params* pmem_drv_params = NULL;

static struct pmem_tci_params* TEE_Open(mcSessionHandle_t* pSessionHandle)
{
	struct pmem_tci_params* pTci = NULL;
	mcResult_t mcRet;

	do {
		if (pSessionHandle == NULL) {
			DBG_LOG("[ERROR] TEE_Open(): Invalid session handle!");
			return NULL;
		}

		memset(pSessionHandle, 0, sizeof(mcSessionHandle_t));

		mcRet = mcOpenDevice(g_device_id);
		if ((MC_DRV_OK != mcRet) && (MC_DRV_ERR_DEVICE_ALREADY_OPEN != mcRet)) {
			DBG_LOG("[ERROR] TEE_Open(): mcOpenDevice returned:");
			return NULL;
		}

		mcRet = mcMallocWsm(g_device_id, 0, sizeof(struct pmem_tci_params),
			(uint8_t**)&pTci, 0);
		if (MC_DRV_OK != mcRet) {
			mcCloseDevice(g_device_id);
			DBG_LOG("[ERROR] TEE_Open(): mcMallocWsm returned:");
			return NULL;
		}

		pSessionHandle->deviceId = g_device_id;
		mcRet = mcOpenSession(pSessionHandle, &g_drv_uuid, (uint8_t*)pTci,
			(uint32_t)sizeof(struct pmem_tci_params));
		if (MC_DRV_OK != mcRet) {
			mcFreeWsm(g_device_id, (uint8_t*)pmem_drv_params);
			mcCloseDevice(g_device_id);
			DBG_LOG("[ERROR] TEE_Open(): mcOpenSession returned:");
			return NULL;
		}
	} while (false);

	return pTci;
}

static void TEE_Close(mcSessionHandle_t* pSessionHandle)
{
	mcResult_t mcRet;

	do {
		if (pSessionHandle == NULL) {
			DBG_LOG("[ERROR] TEE_Close(): Invalid session handle!");
			break;
		}

		mcRet = mcCloseSession(pSessionHandle);
		if (MC_DRV_OK != mcRet) {
			DBG_LOG("[ERROR] TEE_Close(): mcCloseSession returned:");
			break;
		}

		mcRet = mcFreeWsm(g_device_id, (uint8_t*)pmem_drv_params);
		if (MC_DRV_OK != mcRet) {
			DBG_LOG("[ERROR] TEE_Close(): mcFreeWsm returned:");
			break;
		}

		mcRet = mcCloseDevice(g_device_id);
		if (MC_DRV_OK != mcRet) {
			DBG_LOG("[ERROR] TEE_Close(): mcCloseDevice returned:");
			break;
		}
	} while (false);
}

static int tee_open_pmem_tdriver()
{
	pmem_drv_params = TEE_Open(&pmem_drv_session);
	if (pmem_drv_params == NULL) {
		DBG_LOG("[ERROR] open pmem t-driver failed! \n");
		return -1;
	}

	return 0;
}

static int tee_close_pmem_tdriver()
{
	if (pmem_drv_params != NULL) {
		TEE_Close(&pmem_drv_session);
		pmem_drv_params = NULL;
	}
	return 0;
}
#endif

static int get_protected_handle(int ion_fd, ion_user_handle_t ion_handle,
	int* mem_handle)
{
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_GET_PHYS;
	sys_data.get_phys_param.handle = ion_handle;
	if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
		return -1;

	*mem_handle = sys_data.get_phys_param.phy_addr;
	return 0;
}

#ifdef GP_TEE_ENABLED
static void test_TeeShmIPC()
{
	int ion_fd, i, j;
	ion_user_handle_t ion_handle;
	UREE_SHAREDMEM_HANDLE gz_shm_handle;
	int ion_shm_handle;

	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	TZ_RESULT ret;

	/* from 1K to 16M*/
	uint32_t shm_size[15] = {0x00000400, 0x00000800, 0x00001000, 0x00002000,
		0x00004000, 0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00080000,
		0x00100000, 0x00200000, 0x00400000, 0x00800000, 0x01000000};
	uint32_t init_zero[2] = {0, ION_FLAG_MM_HEAP_INIT_ZERO};
	int check_zero;

	TEST_BEGIN("test_TeeShmIPC");

#ifdef TRUSTONIC_TEE_ENABLED
	/* load pmem t-driver */
	ret = tee_open_pmem_tdriver();
	EXPECT_EQ(0, ret, "tee_open_pmem_tdriver");
#endif

	/* create session to echo service */
	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	/* open ion fd */
	ion_fd = ion_open();
	DBG_LOG("open ion_fd = %d \n", ion_fd);

	for(j = 0; j < 2; j++) {
		for(i = 0; i<15; i++) {

			check_zero = init_zero[j] >> 16;

			/* ion alloc MTEE/TEE shared memory*/
			ret = ion_alloc(ion_fd, shm_size[i], 0, PMEM_ION_MASK,
				init_zero[j], &ion_handle);
			EXPECT_EQ(0, ret, "ion alloc shared memory");

			/* get ion_shm_handle and gz_shm_handle*/
			ret = get_protected_handle(ion_fd, ion_handle, &ion_shm_handle);
			EXPECT_EQ(0, ret, "get ion_shm_handle");
			DBG_LOG("ion_shm_handle = 0x%x \n", ion_shm_handle);

			ret = UREE_ION_TO_SHM_HANDLE(ion_shm_handle, &gz_shm_handle);
			EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "ion_shm_handle to gz_shm_handle");
			DBG_LOG("gz_shm_handle = %d \n", gz_shm_handle);

			/* call gz process shared memory*/
			param[0].value.a = gz_shm_handle;
			param[0].value.b = shm_size[i];
			param[1].value.a = ion_shm_handle;
			param[1].value.b = check_zero;
			types = TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT,
				TZPT_VALUE_OUTPUT);

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_SHM_IPC, types,
				param);
			EXPECT_EQ(TZ_RESULT_SUCCESS, ret,
				"echo TZCMD_TEST_SHM_IPC service call");
			EXPECT_EQ(0, param[2].value.a,
				"TZCMD_TEST_SHM_IPC function return");

			/* ion free MTEE/TEE shared memory */
			ret = ion_free(ion_fd, ion_handle);
			EXPECT_EQ(0, ret, "ion free shared memory");
		}
	}

	/* close ion fd */
	ret = ion_close(ion_fd);
	EXPECT_EQ(0, ret, "close ion fd");

	/* close session to echo service */
	ret = UREE_CloseSession(session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "close echo srv session");

#ifdef TRUSTONIC_TEE_ENABLED
	/* un-load pmem t-driver */
	ret = tee_close_pmem_tdriver();
	EXPECT_EQ(0, ret, "tee_close_pmem_tdriver");
#endif

	TEST_END;
	REPORT_UNITTESTS;
}
#endif

static void run_internal_test(void *args)
{
	UREE_SESSION_HANDLE ut_session_handle;
	TZ_RESULT ret;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;

	TEST_BEGIN("run internal test");

	ret = UREE_CreateSession(echo_srv_name, &ut_session_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	paramTypes = TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT,
		TZPT_VALUE_OUTPUT);
	ret = UREE_TeeServiceCall(ut_session_handle, TZCMD_TEST_ALLUT,
		paramTypes, param);
	if (ret != TZ_RESULT_SUCCESS) {
		ERR_LOG("UREE_TeeServiceCall()[TZCMD_TEST_ALLUT] Fail. ret=0x%x\n",
		ret);
	}

	ret = UREE_CloseSession(ut_session_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void gz_abort_test()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("gz abort test");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");
	/**** Service call test ****/
	param[0].value.a = 0x1230;
	/* memory boundary case parameters */
	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	ret = UREE_TeeServiceCall(session, TZCMD_ABORT_TEST, types, param);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n",
		(uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void gz_rtc_test(char *mode, char *str)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	struct tm tm;
	char *pch;
	int arr[5];
	int i = 0;

	TEST_BEGIN("gz rtc test");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	DBG_LOG("\nmode = %s str = %s\n", mode, str);

	param[0].value.b = 0x72;

	if(strcmp(mode, "w") == 0) {
		if((strcmp(str, "default")) && (strptime(str, "%Y-%m-%d %H:%M:%S", &tm)
			!= NULL)) {
			DBG_LOG("tm year-mon-mday(wday) hour:min:sec = %d-%d-%d(%d) %d:%d:%d\n",
				tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_wday, tm.tm_hour,
				tm.tm_min, tm.tm_sec);
			param[1].value.a = tm.tm_year;
			param[1].value.b = (tm.tm_min |(tm.tm_hour << 8) |
				(tm.tm_mday << 16) | (tm.tm_mon << 24));
		} else {
			/*2018-2-28 23:59:00*/
			param[1].value.a = 118;
			param[1].value.b = 0x11c173b;
		}
		param[0].value.b = 0x77;
	} else {
		if(strcmp(str, "default")) {
			pch = strtok(str, ",");
			while (pch != NULL) {
				arr[i++] = atoi(pch);
				pch = strtok(NULL, ",");
			}
			param[1].value.a = arr[0];
			param[1].value.b = arr[1];
		} else {
			param[1].value.a = 1;
			param[1].value.b = 60;
		}
	}

	DBG_LOG("param[0].value.a/b = 0x%x/0x%x \n",param[0].value.a,
		param[0].value.b);
	DBG_LOG("param[1].value.a/b = %d/0x%x(%d) \n",param[1].value.a,
		param[1].value.b, param[1].value.b);

	/**** Service call test ****/
	param[0].value.a = 0x1230;
	/* memory boundary case parameters */
	types = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT);
	ret = UREE_TeeServiceCall(session, TZCMD_TEST_RTC, types, param);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n",
		(uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void gz_GetVersion_test()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("gz Get Version test");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	ret = UREE_TeeServiceCall(session, TZCMD_TEST_VERSION, types, param);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n", (uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void gz_GetCellInfo_test()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("gz Get Cell Info test");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	ret = UREE_TeeServiceCall(session, TZCMD_TEST_CELLINFO, types, param);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n",
		(uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void gz_internal_ipc_test()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("gz internel ipc test");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	ret = UREE_TeeServiceCall(session, TZCMD_TEST_INTERNAL_IPC, types, param);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n",
		(uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

#define FBC_SERV_NAME SRV_NAME("fbc.main")
#if 0
static void gz_fbc_test()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("gz fbc test");

	ret = UREE_CreateSession(FBC_SERV_NAME, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create FBC srv session");
	EXPECT_NEQ_EXIT(0, session, "create FBC srv session");

	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	param[0].value.a = 999;

	ret = UREE_TeeServiceCall(session, GZ_CMD_FBC_RUN1, types, param);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("service call 1 Error: handle 0x%x, ret %d\n",
		(uint32_t)session, ret);

	ret = UREE_TeeServiceCall(session, GZ_CMD_FBC_RUN2, types, param);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("service call 2 Error: handle 0x%x, ret %d\n",
		(uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close FBC srv session");

	TEST_END;
	REPORT_UNITTESTS;
}
#endif
static void gz_multithread_test()
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("gz multithread test");

	ret = UREE_CreateSession(FBC_SERV_NAME, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create FBC srv session");

	param[0].value.a = 3;
	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	ret = UREE_TeeServiceCall(session, GZ_CMD_FBC_FORK, types, param);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n",
				(uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close FBC srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void dma_test()
{
	UREE_SESSION_HANDLE ut_session_handle;
	TZ_RESULT ret;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;
	int stress = 0, repeat = 0, page_num = 0;

	TEST_BEGIN("dma test");

	//sscanf((char*)args, "%c %d %d %d", &c, &stress, &repeat, &page_num);

	ret = UREE_CreateSession(echo_srv_name, &ut_session_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	paramTypes = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT);
	param[0].value.a = (uint32_t)stress;
	param[1].value.a = (uint32_t)repeat;
	param[1].value.b = (uint32_t)page_num;
	ret = UREE_TeeServiceCall(ut_session_handle, TZCMD_DMA_TEST, paramTypes, param);
	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall() Fail. ret=0x%x\n", ret);

	ret = UREE_CloseSession(ut_session_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

static void test_SecureCarema()
{
	uint32_t shm_size = 3 * 4096;	/*alloc 3 pages*/
	uint32_t allocchm_size = shm_size;

	UREE_SESSION_HANDLE mem_session;

	UREE_SHAREDMEM_PARAM shm_param;
	UREE_SHAREDMEM_HANDLE shm_handle;

	int CA_ion_fd;
	ion_user_handle_t CA_ion_handle;
	int ION_Handle;

	/*for shared memory buf*/
	int ion_fd = -1;
	ion_user_handle_t ion_hd = -1;
	int map_fd = -1;

	unsigned char *buf = NULL;
	TZ_RESULT ret;
	int count = 0;
	int i;

	TEST_BEGIN("test_SecureCarema");

	/* 0. Create a memory space for the shared memory */
	//buf = (char *) memalign(PAGE_SIZE, shm_size); /*orig way, maybe fail.*/

	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd, &ion_hd, &map_fd, &buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto out;

	EXPECT_NEQ(buf, NULL, "shmem [buf] null!!!");
	if(buf == NULL) {
		DBG_LOG("[%d]shmem [buf] alloc fail. null.\n", __LINE__);
		goto out;
	}

	/* 0.1 [optional] init test data in REE's CA: buf value*/
	memset(buf, 'c', shm_size);
	for (i = 0; i < (int) shm_size; i++) {
		if (buf[i] == 'c')
			count++;
	}
	DBG_LOG("[before] buf(c) = %d\n", count);
	EXPECT_EQ(shm_size, count, "init share mem #c");

	/* 0.2 init: create session for mem access */
	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create mem_session");

	/* 1. Register a shared memory */
	shm_param.buffer = (void *)buf;
	shm_param.size = shm_size;
	ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "register share mem");

	/* 2. Alloc a memory from a secure protected shared mem (chmem)*/
	CA_ion_fd = ion_open();
	//DBG_LOG("open CA_ion_fd = %d \n", CA_ion_fd);

	ret = ion_alloc(CA_ion_fd, shm_size, 0, PMEM_ION_MASK,
		ION_FLAG_MM_HEAP_INIT_ZERO, &CA_ion_handle);
	EXPECT_EQ(0, ret, "ion_alloc");

	/*'ION_Handle' is used in M-TEE*/
	ret = get_protected_handle(CA_ion_fd, CA_ion_handle, &ION_Handle);
	EXPECT_EQ(0, ret, "get_protected_handle");

	/* 3. [optional] update test data in M-TEE's HA: update all data to 'e' */
	ret = UREE_ION_TEST_UPT_CHMDATA(ION_Handle, allocchm_size);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "UREE_ION_TEST_UPT_CHMDATA");

	/* 3. Copy chmem data to shmem */
	ret = UREE_ION_CP_Chm2Shm(mem_session, shm_handle, ION_Handle,
		allocchm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "UREE_ION_CP_Chm2Shm");
	if (ret != TZ_RESULT_SUCCESS) {
		if (ret == (int) TZ_RESULT_ERROR_NOT_SUPPORTED)
			DBG_LOG("UREE_ION_CP_Chm2Shm API is not supported. ret = 0x%x\n", ret);
		else
			DBG_LOG("COPY chm 2 shm fail. ret = 0x%x\n", ret);
		goto cp_test_out;
	}

	/* 4. [optional] verify shmem result in REE */
	count = 0;
	for (i = 0; i < (int) shm_size; i++) {
		if (buf[i] == 'e')
			count++;
	}
	DBG_LOG("[After] buf(e) = %d\n", count);

	/* 5. Write shmem data to a file */
	FILE *image_output = NULL;
	if ((image_output = fopen("/data/securecamera_out.raw","wb+")) == NULL) {
		printf("fopen error.\n");
	} else {
		printf("start to write file in /data/securecamera_out.raw\n");
		for (i = 0; i < (int) allocchm_size; i++)
			fputc(buf[i], image_output);

		fclose(image_output);
	}

cp_test_out:

	/* 6. free alloc chm memory and relase chunk memory */
	ret = ion_free(CA_ion_fd, CA_ion_handle);
	EXPECT_EQ(0, ret, "CA_ion_handle free shared memory");

	ret = ion_close(CA_ion_fd);
	EXPECT_EQ(0, ret, "close ion CA_ion_fd");

	/* 7. unreg. shared memory */
	ret = UREE_UnregisterSharedmem(mem_session, shm_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "un-register share mem");

	/* 8. close session */
	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem_session");

out:
	//free(buf);	/* free shmem. orig way */
	ret = _free_ion_buf(ion_fd, ion_hd, map_fd, buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

	TEST_END;
	REPORT_UNITTESTS;
}

#if 0 //#ifdef ENABLE_SYNC_TEST /*fixme!!!*/
static int sync_test(void)
{
	TZ_RESULT ret;
	KREE_SESSION_HANDLE sessionHandle, sessionHandle2;

	union MTEEC_PARAM param[4];
	uint32_t types;

	TEST_BEGIN("TA sync test");

	/* Connect to echo service */
	ret = KREE_CreateSession(echo_srv_name, &sessionHandle);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "create echo srv session");
	CHECK_GT_ZERO(sessionHandle, "check echo srv session value");

	/* Connect to sync-ut service */
	ret = KREE_CreateSession(APP_NAME2, &sessionHandle2);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "create sync-ut srv session");
	CHECK_GT_ZERO(sessionHandle, "check echo sync-ut session value");

	/* Request mutex handle from TA1 */
	types = TZ_ParamTypes2(TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT);
	ret = KREE_TeeServiceCall(sessionHandle, TZCMD_GET_MUTEX, types, param);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "get mutex handle from TA1");

	CHECK_GT_ZERO(param[0].value.a, "check mutex value");

	/* Send mutex handle to TA2 */
	types = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT);
	ret = KREE_TeeServiceCall(sessionHandle2, TZCMD_SEND_MUTEX, types, param);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "send mutex handle to TA2");

	/* start mutex test */
	ret = KREE_TeeServiceCall(sessionHandle, TZCMD_TEST_MUTEX, types, param);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "send start cmd to TA1");
	ret = KREE_TeeServiceCall(sessionHandle2, TZCMD_TEST_MUTEX, types, param);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "send start cmd to TA2");

	ret = KREE_CloseSession(sessionHandle);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "close echo srv session");
	ret = KREE_CloseSession(sessionHandle2);
	CHECK_EQ(TZ_RESULT_SUCCESS, ret, "close echo sync-ut session");

	TEST_END;
	return 0;
}
#endif

static void Lazy(char *server_name, uint32_t lazy_cmd)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	struct timeval tv1;
	struct timeval tv2;
	int time_cost = 0;

	TEST_BEGIN("Lazy ...... ");

	ret = UREE_CreateSession(server_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	types = TZ_ParamTypes1(TZPT_VALUE_INPUT);
	gettimeofday(&tv1, NULL);
	ret = UREE_TeeServiceCall(session, lazy_cmd, types, param);
	gettimeofday(&tv2, NULL);
	time_cost = (tv2.tv_sec - tv1.tv_sec) * 1000000 +
		(tv2.tv_usec - tv1.tv_usec);
	DBG_LOG("%ud cmd cost time: %dus\n", lazy_cmd, time_cost);

	if (ret != TZ_RESULT_SUCCESS)
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n", (uint32_t)session, ret);

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

void echo_function (int cmd)
{
	printf ("====> [echo] cmd = %d\n", cmd);
	Lazy((char *) echo_srv_name, cmd);
}

void lazy_function (char *server_name, int cmd)
{
	printf ("====> [lazy] cmd = %d\n", cmd);
	Lazy((char *) server_name, cmd);
}

#ifdef GP_TEE_ENABLED

#define TZCMD_TEST_SHARED_MEM_HAs_ALLOC  120  /*for geniezone.test*/
#define TZCMD_TEST_SHARED_MEM_HAs_QUERY  120  /*for geniezone.srv.echo*/
#define TZCMD_TEST_SHARED_MEM_HAs_COPY   121  /*for geniezone.test*/
#define TZCMD_TEST_SHARED_MEM_HAs_READ   121  /*for geniezone.srv.echo*/
#define TZCMD_TEST_SHARED_MEM_HAs_FREE   122  /*for geniezone.test*/
#define TZCMD_TEST_SHARED_MEM_HA_TA_SETUP 123 /*for geniezone.test*/

void SharedMem_HAs(uint32_t test_max_cnt)
{
	char *serverHA1_name = SRV_NAME("test");		/* do copy */
	char *serverHA2_name = SRV_NAME("srv.echo");	/* do algh */
	/* get mem for dump mem from secure mem to normal mem */
	char *serverHA3_name = SRV_NAME("srv.mem");
	TZ_RESULT ret;
	UREE_SESSION_HANDLE sessionHA1;
	UREE_SESSION_HANDLE sessionHA2;
	UREE_SESSION_HANDLE sessionHA3mem;
	MTEEC_PARAM paramHA1[4];
	MTEEC_PARAM paramHA2[4];
	UREE_SECUREMEM_HANDLE HA_HAs_sec_handle = 0;
	uint32_t types;
	uint32_t cnt;
	uint32_t *dump_buf_from_copy = NULL;
	uint32_t *dump_buf_from_read = NULL;

	int CA_ion_fd;
	ion_user_handle_t CA_ion_handle;
	int TA_HATA_ion_secshm_handle;
	UREE_SHAREDMEM_HANDLE HA_HATA_ion_secshm_handle;
	UREE_SHAREDMEM_HANDLE HA1_CAHA1_shm_handle;
	UREE_SHAREDMEM_HANDLE HA2_CAHA2_shm_handle;
	UREE_SHAREDMEM_PARAM CA_HA1_shm_info;
	UREE_SHAREDMEM_PARAM CA_HA2_shm_info;

	/*for shared memory buf*/
	int ion_fd_copy = -1;
	ion_user_handle_t ion_hd_copy = -1;
	int map_fd_copy = -1;

	int ion_fd_read = -1;
	ion_user_handle_t ion_hd_read = -1;
	int map_fd_read = -1;

	int shm_size_copy = 1024 * 128;
	int shm_size_read = 1024 * 128;

	printf ("====> %s measure %d\n", __func__, test_max_cnt);

	TEST_BEGIN("SharedMem_HAs");

	/************************************************************/
	/*create all of session*/
	ret = UREE_CreateSession(serverHA1_name, &sessionHA1);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "geniezone.test");

	ret = UREE_CreateSession(serverHA2_name, &sessionHA2);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "srv.echo");

	ret = UREE_CreateSession(serverHA3_name, &sessionHA3mem);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "srv.mem");
	/************************************************************/

	/************************************************************/
	/*create HA<-->TA memory*/
	CA_ion_fd = ion_open();
	DBG_LOG("open CA_ion_fd = %d \n", CA_ion_fd);

	/* ion alloc MTEE/TEE shared memory*/
	ret = ion_alloc(CA_ion_fd, (128 * 1024), 0, PMEM_ION_MASK,
		ION_FLAG_MM_HEAP_INIT_ZERO, &CA_ion_handle);
	EXPECT_EQ(0, ret, "ion alloc shared memory");

	/* get HA_HATA_ion_sec_shm_handle and TA_HATA_ion_secshm_handle*/
	ret = get_protected_handle(CA_ion_fd, CA_ion_handle,
		&TA_HATA_ion_secshm_handle);
	EXPECT_EQ(0, ret, "get TA_HATA_ion_secshm_handle");
	DBG_LOG("TA_HATA_ion_secshm_handle = 0x%x \n", TA_HATA_ion_secshm_handle);

	/*use TA memory handle to get HA memory handle*/
	ret = UREE_ION_TO_SHM_HANDLE(TA_HATA_ion_secshm_handle,
		&HA_HATA_ion_secshm_handle);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret,
		"TA_HATA_ion_secshm_handle to HA_HATA_ion_secshm_handle");
	/************************************************************/

	/************************************************************/
	/* create CA<-->HA memory */
	//dump_buf_from_copy = (uint32_t *)memalign(4096, 1024 * 128);  //orig way, maybe fail
	//dump_buf_from_read = (uint32_t *)memalign(4096, 1024 * 128);  //orig way, maybe fail

	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd_copy, &ion_hd_copy, &map_fd_copy, (unsigned char **)&dump_buf_from_copy, shm_size_copy);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto out1;

	EXPECT_NEQ(dump_buf_from_copy, NULL, "shmem [dump_buf_from_copy] null!!!");
	if(dump_buf_from_copy == NULL) {
		DBG_LOG("[%d]shmem [dump_buf_from_copy] alloc fail. null.\n", __LINE__);
		goto out1;
	}
	memset(dump_buf_from_copy, 0, shm_size_copy);

	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd_read, &ion_hd_read, &map_fd_read, (unsigned char **)&dump_buf_from_read, shm_size_read);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto out2;

	EXPECT_NEQ(dump_buf_from_read, NULL, "shmem [dump_buf_from_read] null!!!");
	if(dump_buf_from_read == NULL) {
		DBG_LOG("[%d]shmem [dump_buf_from_read alloc] fail. null.\n", __LINE__);
		goto out2;
	}
	memset(dump_buf_from_read, 0, shm_size_read);

	CA_HA1_shm_info.buffer = (void *)dump_buf_from_copy;
	CA_HA1_shm_info.size = 1024 * 128;
	CA_HA2_shm_info.buffer = (void *)dump_buf_from_read;
	CA_HA2_shm_info.size = 1024 * 128;
	/*use CA va to HA memory handle,              */
	/*in HA will use this handle to map va for use*/
	ret = UREE_RegisterSharedmem(sessionHA3mem, &HA1_CAHA1_shm_handle,
		&CA_HA1_shm_info);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "map dump_buf_from_copy");
	ret = UREE_RegisterSharedmem(sessionHA3mem, &HA2_CAHA2_shm_handle,
		&CA_HA2_shm_info);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "map dump_buf_from_read");
	printf("HA1_CAHA1_shm_handle=0x%x, HA2_CAHA2_shm_handle=0x%x\n",
		HA1_CAHA1_shm_handle, HA2_CAHA2_shm_handle);
	/************************************************************/

	types = TZ_ParamTypes4(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT,
		TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT);

	if (HA_HAs_sec_handle == 0) {
		/*create mem in HA1, return handle to CA               */
		/*CA pass handle to HA2, then HA2 use handle to map va */
		ret = UREE_TeeServiceCall(sessionHA1, TZCMD_TEST_SHARED_MEM_HAs_ALLOC,
			types, paramHA1);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t)sessionHA1, ret);
			return;
		}
		HA_HAs_sec_handle = paramHA1[2].value.a;
		//printf("alloc mem handle(0x%x)\n", HA_HAs_sec_handle);
		EXPECT_EQ_EXIT(0x66, paramHA1[3].value.a, "XX_01");

		paramHA2[0].value.a = HA_HAs_sec_handle;
		ret = UREE_TeeServiceCall(sessionHA2, TZCMD_TEST_SHARED_MEM_HAs_QUERY,
			types, paramHA2);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) sessionHA2, ret);
			return;
		}
		EXPECT_EQ_EXIT(0x66, paramHA1[3].value.a, "XX_02");
	}

	if (1) {
		uint32_t loop;
		const uint32_t loop_max = (128 * 1024) / 4;
		for ( cnt = 1; cnt <= test_max_cnt; cnt++) {
			/* setup HA<-->TA memory data, in secure camera case, skip it */
			paramHA1[0].value.a = cnt;
			paramHA1[0].value.b = HA_HATA_ion_secshm_handle;
			paramHA1[3].value.a = 0;
			ret = UREE_TeeServiceCall(sessionHA1, TZCMD_TEST_SHARED_MEM_HA_TA_SETUP,
				types, paramHA1);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) sessionHA1, ret);
				break;
			}
			EXPECT_EQ_EXIT(0x66, paramHA1[3].value.a, "XX_05");

			/* pass HA_HATA_ion_secshm_handle to get HA<-->TA va,                  */
			/* pass HA1_CAHA1_shm_handle to get CA<-->HA1 for dump memory to debug */
			paramHA1[0].value.a = HA_HATA_ion_secshm_handle;
			paramHA1[0].value.b = HA1_CAHA1_shm_handle;
			paramHA1[3].value.a = 0;

			ret = UREE_TeeServiceCall(sessionHA1, TZCMD_TEST_SHARED_MEM_HAs_COPY,
				types, paramHA1);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) sessionHA1, ret);
				break;
			}
			EXPECT_EQ_EXIT(0x66, paramHA1[3].value.a, "XX_05");

			/* compare dump buffer is equal */
			/* TZCMD_TEST_SHARED_MEM_HA_TA_SETUP setup data*/
			for (loop = 0 ; loop < loop_max ; loop++) {
				if (dump_buf_from_copy[loop] != cnt) {
					DBG_LOG("dump_buf_from_copy %u != %u %s\n",
						dump_buf_from_copy[loop], cnt, __func__);
					return;
				}
			}

			/* pass cnt, we can check TA<-->TA memory see the same memory          */
			/* pass HA2_CAHA2_shm_handle to get CA<-->HA2 for dump memory to debug */
			paramHA2[0].value.a = cnt;
			paramHA2[0].value.b = HA2_CAHA2_shm_handle;
			paramHA2[3].value.a = 0;
			ret = UREE_TeeServiceCall(sessionHA2, TZCMD_TEST_SHARED_MEM_HAs_READ,
				types, paramHA2);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) sessionHA2, ret);
				break;
			}
			EXPECT_EQ_EXIT(0x66, paramHA2[3].value.a, "XX_06");

			/* compare dump buffer is equal                 */
			/* TZCMD_TEST_SHARED_MEM_HA_TA_SETUP setup data */
			for (loop = 0 ; loop < loop_max ; loop++) {
				if (dump_buf_from_read[loop] != cnt) {
					DBG_LOG("dump_buf_from_read %u != %u %s\n",
						dump_buf_from_read[loop], cnt, __func__);
					return;
				}
			}

			printf("%s done %u, dump_buf_from_copy[0]=%u, dump_buf_from_read[0]=%u\n",
				__func__, cnt, dump_buf_from_copy[0], dump_buf_from_read[0]);
		}
	}

	if (HA_HAs_sec_handle != 0) {
		ret = UREE_TeeServiceCall(sessionHA1, TZCMD_TEST_SHARED_MEM_HAs_FREE,
			types, paramHA1);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) sessionHA1, ret);
			return;
		}
		EXPECT_EQ_EXIT(0x66, paramHA1[3].value.a, "XX_03");
	}

	ret = UREE_UnregisterSharedmem(sessionHA3mem, HA1_CAHA1_shm_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unmap dump_buf_from_copy");
	ret = UREE_UnregisterSharedmem(sessionHA3mem, HA2_CAHA2_shm_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unmap dump_buf_from_read");

out2:
	//free(dump_buf_from_copy);  //orig way
	//free(dump_buf_from_read);  //orig way
	ret = _free_ion_buf(ion_fd_read, ion_hd_read, map_fd_read, dump_buf_from_read, shm_size_read);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

out1:
	ret = _free_ion_buf(ion_fd_copy, ion_hd_copy, map_fd_copy, dump_buf_from_copy, shm_size_copy);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

	/* ion free MTEE/TEE shared memory */
	ret = ion_free(CA_ion_fd, CA_ion_handle);
	EXPECT_EQ(0, ret, "CA_ion_handle free shared memory");

	/* ion close */
	ret = ion_close(CA_ion_fd);
	EXPECT_EQ(0, ret, "close ion CA_ion_fd");

	ret = UREE_CloseSession(sessionHA3mem);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close srv.mem session");

	ret = UREE_CloseSession(sessionHA2);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close srv.echo session");

	ret = UREE_CloseSession(sessionHA1);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close geniezone.test session");

	TEST_END;
	REPORT_UNITTESTS;
}
#endif

#define TZCMD_TEST_OMP_BASIC       28
#define TZCMD_TEST_OMP_UT         101
#define TZCMD_TEST_OMP_102        102
#define TZCMD_TEST_OMP_103        103
#define TZCMD_TEST_OMP_104        104
#define TZCMD_TEST_OMP_DO_INIT    105
#define TZCMD_TEST_OMP_MULTI_CORE 106
#define TZCMD_TEST_OMP_ONE_CORE   107
#define TZCMD_TEST_OMP_RESULT     108
#define TZCMD_TEST_OMP_109        109

void multicore_test(char *server_name, uint32_t test_max_cnt, int test_case)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	uint32_t cnt;
	struct  timeval start_one_core, start_multi_core;
	struct  timeval end_one_core, end_multi_core;
	unsigned long diff_one_core, diff_multi_core;
	time_t timep;
	int loser_cnt = 0;

	printf ("====> %s measure %d\n", __func__, test_max_cnt);

	TEST_BEGIN("multicore_test");

	ret = UREE_CreateSession(server_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	types = TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT,
		TZPT_VALUE_OUTPUT);

	if (test_case == 1) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_MULTI_CORE,
				types, param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE,
				types, param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			} else if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("(loser=%d) multi_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s",
				loser_cnt,
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));

			ALOGI("multi_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s\n",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));
			if (diff_multi_core > diff_one_core) {
				loser_cnt++;
			}
		}
	}
	if (test_case == 10) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_MULTI_CORE,
				types, param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE,
				types, param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT,
				types, param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			} else if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("multi_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));

			ALOGI("multi_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s\n",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));
			if ( diff_multi_core > diff_one_core ) {
				loser_cnt++;
				if (loser_cnt >= 1) {
					printf("need stop for debug\n");
					break;
				}
			}
		}
	}
	if (test_case == 6) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, 110, types, param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE, types,
				param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			} else if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("(loser=%d) thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s",
				loser_cnt,
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));

			ALOGI("thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s\n",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));
			if (diff_multi_core > diff_one_core) {
				loser_cnt++;
			}
		}
	}
	if (test_case == 60) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, 110, types, param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE, types,
				param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			} else if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));

			ALOGI("thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s\n",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));
			if (diff_multi_core > diff_one_core) {
				loser_cnt++;
				if (loser_cnt >= 1) {
					printf("need stop for debug\n");
					break;
				}
			}
		}
	}
	if (test_case == 7) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		ret = UREE_TeeServiceCall(session, 115, types, param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		for (cnt =0; cnt < test_max_cnt; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, 116, types, param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE, types,
				param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			} else if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("(loser=%d) thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s",
				loser_cnt,
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));

			ALOGI("thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s\n",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));
			if (diff_multi_core > diff_one_core) {
				loser_cnt++;
			}
		}
		ret = UREE_TeeServiceCall(session, 117, types, param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
	}
	if (test_case == 70) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		ret = UREE_TeeServiceCall(session, 115, types, param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, 116, types, param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE, types,
				param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			} else if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("(loser=%d) thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s",
				loser_cnt,
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));

			ALOGI("thread_core(%6lu us) %s 1_core(%6lu us) at %6u test, compare result is %s at %s\n",
				diff_multi_core,
				diff_multi_core <= diff_one_core ? "win " : "lose",
				diff_one_core,
				cnt,
				param[2].value.a != 0xdead ? "same" : "different",
				asctime(gmtime(&timep)));
			if (diff_multi_core > diff_one_core) {
				loser_cnt++;
				if (loser_cnt >= 1) {
					printf("need stop for debug\n");
					break;
				}
			}
		}
		ret = UREE_TeeServiceCall(session, 117, types, param);
		if (ret != TZ_RESULT_SUCCESS) {
			printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
				(uint32_t) session, ret);
			return;
		}
	}
	if (test_case == 0) {
		ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
			param);
		if (ret != TZ_RESULT_SUCCESS) {
			ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n",
				(uint32_t) session, ret);
			return;
		}
		for (cnt=0 ; cnt < test_max_cnt ; cnt++) {
			gettimeofday(&start_multi_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_MULTI_CORE, types,
				param);
			gettimeofday(&end_multi_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}
			diff_multi_core = 1000000 *
				(end_multi_core.tv_sec - start_multi_core.tv_sec) +
				end_multi_core.tv_usec - start_multi_core.tv_usec;
			time (&timep);
			printf("multi_core(%6lu us) %3u test at %s\n", diff_multi_core, cnt,
				asctime(gmtime(&timep)));
		}
		for (cnt=0; cnt < test_max_cnt; cnt++)	{
			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE, types, param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}
			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			time (&timep);
			printf("  one_core(%6lu us) %3u test at %s\n", diff_one_core, cnt,
				asctime(gmtime(&timep)) );
		}
	}
	if (test_case == 2) {
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_UT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}
			if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}
			time (&timep);
			printf("UT done %6d at %s\n", cnt, asctime(gmtime(&timep)));
			ALOGI("UT done %6d at %s\n", cnt, asctime(gmtime(&timep)));
		}
	}
	if (test_case == 20) {
		for (cnt = 0; cnt < test_max_cnt; cnt++) {
			ret = UREE_TeeServiceCall(session, 111, types, param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}
			if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}
			time (&timep);
			printf("UT done %6d at %s", cnt, asctime(gmtime(&timep)));
			ALOGI("UT done %6d at %s\n", cnt, asctime(gmtime(&timep)));
		}
	}
	if (test_case == 21) {
		for (cnt=0; cnt < test_max_cnt; cnt++) {
			ret = UREE_TeeServiceCall(session, 115, types, param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				return;
			}
			ret = UREE_TeeServiceCall(session, 112, types, param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				break;
			}
			if (param[1].value.a != 0 || param[1].value.b != 0 ||
				param[2].value.a != 0 || param[2].value.b != 0) {
					printf("[1]0x%x(%u) 0x%x(%u)\n", param[1].value.a,
						param[1].value.a, param[1].value.b, param[1].value.b);
					printf("[2]0x%x(%u) 0x%x(%u)\n", param[2].value.a,
						param[2].value.a, param[2].value.b, param[2].value.b);
					break;
			}
			time (&timep);
			printf("UT done %6d at %s", cnt, asctime(gmtime(&timep)));
			ALOGI("UT done %6d at %s\n", cnt, asctime(gmtime(&timep)));
			ret = UREE_TeeServiceCall(session, 117, types, param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				return;
			}
		}
	}
	if (test_case == 3) {
		for (cnt=0; cnt < test_max_cnt; cnt++) {
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_DO_INIT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				return;
			}

			gettimeofday(&start_one_core, NULL);
			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_ONE_CORE, types,
				param);
			gettimeofday(&end_one_core, NULL);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				return;
			}

			ret = UREE_TeeServiceCall(session, TZCMD_TEST_OMP_RESULT, types,
				param);
			if (ret != TZ_RESULT_SUCCESS) {
				printf("%d Error: handle 0x%x, ret %d\n", __LINE__,
					(uint32_t) session, ret);
				return;
			}

			diff_one_core = 1000000 *
				(end_one_core.tv_sec - start_one_core.tv_sec) +
				end_one_core.tv_usec - start_one_core.tv_usec;
			time (&timep);
			printf("1_core(%6lu us) at %3u test, at %s\n",
				diff_one_core,
				cnt,
				asctime(gmtime(&timep)));
		}
	}
	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	TEST_END;
	REPORT_UNITTESTS;
}


/* stress test, start */
#define stresstest_pThread     1
#define stresstest_TeeShmIPC   2
#define stresstest_secmem      3
#define stresstest_shdmem_case 4


void UREE_SecureMem_stress(uint32_t SecureMemSize, uint32_t numOfMemHandler)
{
	TZ_RESULT ret;
	int alignment = 0;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;
	UREE_SESSION_HANDLE echo_session, mem_session;
	UREE_SECUREMEM_HANDLE *sec_handle;


	if(numOfMemHandler < 1)
		numOfMemHandler = 1; /* default */

	printf("SecureMemSize = %d, numOfMemHandler = %d \n",SecureMemSize,
		numOfMemHandler);
	sec_handle = malloc(numOfMemHandler *sizeof(UREE_SECUREMEM_HANDLE));

	TEST_BEGIN("UREE Secure Memory Stress test ......");

	ret = UREE_CreateSession(echo_srv_name, &echo_session);
    EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create echo srv session");

	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create mem srv session");

	for(uint32_t i = 0; i < numOfMemHandler; i++) {
		ret = UREE_AllocSecuremem(mem_session, sec_handle+i, alignment,
			SecureMemSize);
		EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "alloc secure mem ");
		DBG_LOG("UREE_AllocSecuremem handle = %d.\n", *(sec_handle+i));

		param[0].value.a = *(sec_handle+i); /* secure memory handle */
		param[0].value.b = SecureMemSize;
		paramTypes = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT);
		UREE_TeeServiceCall(echo_session, TZCMD_MEM_STRESS_TEST, paramTypes,
			param);

		if (param[1].value.a != 0) {
			ERR_LOG("TZCMD_MEM_STRESS_TEST sec memory: memory handle = %d (alignment=%d, size=%d) [FAIL].\n"
				, mem_session, alignment, SecureMemSize);
			 /* fail */
		}

		ret = UREE_UnreferenceSecuremem(mem_session, *(sec_handle+i));
		EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "unreference secure mem ");
	}

	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem srv session");

	ret = UREE_CloseSession(echo_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close echo srv session");

	free(sec_handle);
	TEST_END;
	REPORT_UNITTESTS;
}

void UREE_ShareMem_stress(uint32_t ShareMemSize, uint32_t numOfMemHandler)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session, mem_session;
	UREE_SHAREDMEM_PARAM shm_param;
	UREE_SHAREDMEM_HANDLE shm_handle;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;
	unsigned char *buf = NULL;
	uint32_t i, stat[2] = {0};

	/*for shared memory buf*/
	int ion_fd = -1;
	ion_user_handle_t ion_hd = -1;
	int map_fd = -1;

	if(numOfMemHandler < 1)
		numOfMemHandler = 1; /* default */

	//DBG_LOG("ShareMemSize = %d, numOfMemHandler = %d \n",ShareMemSize,numOfMemHandler);

	TEST_BEGIN("UREE Share Memory Stress test ......");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create echo srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "create mem srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	//buf = (char *) memalign(PAGE_SIZE, ShareMemSize); //orig way, maybe fail

	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd, &ion_hd, &map_fd, &buf, ShareMemSize);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	EXPECT_NEQ(buf, NULL, "shmem [buf] null!!!");
	if(buf == NULL) {
		DBG_LOG("[%d]shmem [buf] alloc fail. null.\n", __LINE__);
		goto err_;
	}

	memset(buf, 'a', ShareMemSize);

    for (i = 0; i < (ShareMemSize); i++) {
		if (buf[i] == 'a')
			stat[0]++;
		else if (buf[i] == 'b')
			stat[1]++;
		//else DBG_LOG("buf[%d] = %x\n",i,(int)buf[i]);
	}

	//DBG_LOG("init share mem: stat[0] = %d, stat[1] = %d \n", stat[0], stat[1]);
	EXPECT_EQ(ShareMemSize, stat[0], "init share mem #a");
	if(stat[0] != ShareMemSize)
		goto err_;

    EXPECT_EQ(0, stat[1], "init share mem #b");
	if(stat[1] != 0)
		goto err_;

	shm_param.buffer = (void *)buf;
	shm_param.size = ShareMemSize;
	ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "register share mem");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	param[0].value.a = shm_handle;
	param[1].value.b = ShareMemSize;
	paramTypes = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT);

	ret = UREE_TeeServiceCall(session, TZCMD_SHARED_MEM_TEST, paramTypes,
		param);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "echo share mem call");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	for (i = 0; i < 2; i++)
		stat[i] = 0;
    for (i = 0; i < (ShareMemSize); i++) {
		if (buf[i] == 'a')
			stat[0]++;
		else if (buf[i] == 'b')
			stat[1]++;
	}
	// DBG_LOG("after share mem test: stat[0] = %d, stat[1] = %d \n", stat[0], stat[1]);
	EXPECT_EQ(0, stat[0], "after share mem test #a");
	if(stat[0] != 0) {
		ALOGE(" ERROR: the #a in buffer != 0 \n");
		goto err_;
	}
	EXPECT_EQ(ShareMemSize, stat[1], "after share mem test #b");
	if(stat[1] != ShareMemSize) {
		ALOGE(" ERROR: the #b in buffer != ShareMemSize-1 \n");
		goto err_;
	}

	ret = UREE_UnregisterSharedmem(mem_session, shm_handle);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "un-register share mem");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	ret = UREE_CloseSession(session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "close echo srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "close mem srv session");
	if(ret != TZ_RESULT_SUCCESS)
		goto err_;

err_:
	//free(buf);  //orig way
	ret = _free_ion_buf(ion_fd, ion_hd, map_fd, buf, ShareMemSize);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

	TEST_END;
	REPORT_UNITTESTS;
}

static void UREE_stress_loop(char *server_name, int test_cmd,
	MTEEC_PARAM *param)
{
	uint32_t SecureMemSize, numOfMemHandler, ShareMemSize;

	switch (test_cmd) {

	case stresstest_pThread:
		test_pThread(3, 6);
		break;
#ifdef GP_TEE_ENABLED
	case stresstest_TeeShmIPC:
		test_TeeShmIPC();
		break;
#endif
	case stresstest_secmem:
		SecureMemSize = param[0].value.a;
		numOfMemHandler = param[1].value.b;
		UREE_SecureMem_stress(SecureMemSize,numOfMemHandler);
		break;
	case stresstest_shdmem_case:
		ShareMemSize = param[0].value.a;
		numOfMemHandler = param[1].value.b;
		UREE_ShareMem_stress(ShareMemSize,numOfMemHandler);
		break;
	default:
		lazy_function(server_name, test_cmd);
	}
}

struct gz_stress_setup {
	char *str;
	char *server_name;
	uint32_t cmd;
	uint32_t runs;
	MTEEC_PARAM param[4];
};

#define stress_test_runs 10
#define max_stress_test_param 4
static const struct gz_stress_setup UREE_stress_param[] = {
	[0] = { .str = " RTC test! (default parameter)",
			.server_name = SRV_NAME("srv.echo"),
			.cmd = TZCMD_TEST_RTC,
			.runs = 1 },
	[1] = { .str = " Get MTEE version! ",
			.server_name = SRV_NAME("srv.echo"),
			.cmd = TZCMD_TEST_VERSION,
			.runs = 10 },

	[2] = { .str = " Get cell info! ",
			.server_name = SRV_NAME("srv.echo"),
			.cmd = TZCMD_TEST_CELLINFO,
			.runs = 10 },
/*
	[3] = { .str = " SW Timer !!",
			.server_name = SRV_NAME("srv.echo"),
			.cmd = 23,
			.runs = 10 },
*/
	[4] = { .str = " C++ test! ",
			.server_name = SRV_NAME("srv.echo"),
			.cmd = 24,
			.runs = 10 },

/////////////TA: gz-test ///////////////////////////////////////////
/*
	[5] = { .str = " Openmp test! ",
			.server_name = SRV_NAME("test"),
			.cmd = 28,
			.runs = 10 },
*/

	[6] = { .str = " RTC test! (default parameter)",
			.server_name = SRV_NAME("test"),
			.cmd = TZCMD_TEST_RTC,
			.runs = 1 },
	[7] = { .str = " Get MTEE version! ",
			.server_name = SRV_NAME("test"),
			.cmd = TZCMD_TEST_VERSION,
			.runs = 10 },

	[8] = { .str = " Get cell info! ",
			.server_name = SRV_NAME("test"),
			.cmd = TZCMD_TEST_CELLINFO,
			.runs = 10 },
/*
	[9] = { .str = " SW Timer !!",
			.server_name = SRV_NAME("test"),
			.cmd = 23,
			.runs = 10 },

	*/
	[10] = {.str = " C++ test! ",
			.server_name = SRV_NAME("test"),
			.cmd = 24,
			.runs = 10 },

	//////////////////////////////////////////////////////////

/*	[11] = {.str = " pThread test! ",
			.cmd = stresstest_pThread,
			.runs = 10 },
*/

#ifdef GP_TEE_ENABLED
	[12] = {.str = " TeeShmIPC test! ",
			.cmd = stresstest_TeeShmIPC,
			.runs = 10 },
#endif

	[13] = {.str = "secure mem: 128KB+1 mem_handlers",
			.cmd = (uint32_t) stresstest_secmem,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 131072, .value.b = 0xffffffff },
						[1] = { .value.b = 1,    .value.a = 0xffffffff }
			} },

	[14] = {.str = "secure mem: 256KB+1 mem_handlers",
			.cmd = (uint32_t) stresstest_secmem,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 262144, .value.b = 0xffffffff },
						[1] = { .value.b = 1,    .value.a = 0xffffffff }
			} },

	[15] = {.str = "secure mem: 1KB+5 mem_handlers",
			.cmd = (uint32_t) stresstest_secmem,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 1024, .value.b = 0xffffffff},
						[1] = { .value.b = 5,    .value.a = 0xffffffff }
			} },

	[16] = {.str = "secure mem: 1KB+10 mem_handlers",
			.cmd = (uint32_t) stresstest_secmem,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 1024, .value.b = 0xffffffff},
						[1] = { .value.b = 10,   .value.a = 0xffffffff }
			} },

	[17] = {.str = "secure mem: 1KB+20 mem_handlers",
			.cmd = (uint32_t) stresstest_secmem,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 1024, .value.b = 0xffffffff},
						[1] = { .value.b = 20,   .value.a = 0xffffffff }
			} },

	[18] = {.str = "shared mem case 3: 1 handler+size 40KB",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 40960, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },

	[19] = {.str = "shared mem case 3: 1 handler+size 1M",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 1048576, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },

	[20] = {.str = "shared mem case 3: 1 handler+size 2M",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 2097152, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },
	[21] = {.str = "shared mem case 3: 1 handler+size 5M",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 10,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 5242880, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },
	[22] = {.str = "shared mem case 3: 1 handler+size 4M",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 1,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 4194304, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },
	[23] = { .str = " vfp tests ",
			.server_name = SRV_NAME("test"),
			.cmd = TZCMD_TEST_VFP,
			.runs = 1
			},
	[24] = {.str = "shared mem case 3: 1 handler+size 12M",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 1,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 12582912, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },
	[25] = {.str = "shared mem case 3: 1 handler+size 32M",
			.cmd = (uint32_t) stresstest_shdmem_case,
			.runs = 1,
			.param = { [2 ... 3].value.a = 0xffffffff, [2 ... 3].value.b = 0xffffffff,
						[0] = { .value.a = 33554432, .value.b = 0xffffffff},
						[1] = { .value.b = 1,     .value.a = 0xffffffff}
			} },
	{ .str = "EOF" }
};

void init_test_param(MTEEC_PARAM *param)
{
	param[0].value.a = 0;	/*size*/
	param[0].value.b = 0;	/*isRandom*/

	param[1].value.a = 1;	/*numOfSession*/
	param[1].value.b = 1;	/*numOfHandler*/

	param[2].value.a = 1;	/*enableUnregMem*/
	param[2].value.b = 0;	/*numOfPA*/

	param[3].value.a = 1;	/*enableRegMem*/
	param[3].value.b = 0;	/*undefined*/

	/*
	 * DBG_LOG("==> [0].a=%d, [0].b=%d, [1].a=%d, [1].b=%d, [2].a=%d, [2].b=%d\n"
	 *	, param[0].value.a, param[0].value.b, param[1].value.a, param[1].value.b
	 *	, param[2].value.a, param[2].value.b);
	 */
}

static void UREE_stress_test(int stress_rate)
{
	int i = 0;
	int run_times = 0;
	MTEEC_PARAM param[4];
	printf("stress rate is : %d\n",stress_rate);

	while (1) {
		if (UREE_stress_param[i].str == NULL) {
			i++;
			continue;
		}

		if (!strcmp(UREE_stress_param[i].str, "EOF"))
			break;

		DBG_LOG("[%d]===============>[stress test] %s\n", i,
			UREE_stress_param[i].str);

		if (UREE_stress_param[i].runs == 0)
			run_times = (int) stress_test_runs*stress_rate;
		else
			run_times = UREE_stress_param[i].runs * stress_rate;

		init_test_param(param);
		for (int j = 0; j < (int) max_stress_test_param; j++) {
			if (UREE_stress_param[i].param[j].value.a != 0xffffffff)
				param[j].value.a = UREE_stress_param[i].param[j].value.a;

			if (UREE_stress_param[i].param[j].value.b != 0xffffffff)
				param[j].value.b = UREE_stress_param[i].param[j].value.b;
		}

		for(int z = 0; z < run_times; z++)
			UREE_stress_loop(UREE_stress_param[i].server_name,
			UREE_stress_param[i].cmd, param);

		i++;
	}
}

static void tmp_stress_test( int index, int rate)
{
	printf("rate= %d, index = %d\n", rate, index);

	int run_times = 0;
	MTEEC_PARAM param[4];

	DBG_LOG("=>[TEMPORARY stress test] %s [FOR DEBUG]\n",
		UREE_stress_param[index].str);

	if (UREE_stress_param[index].runs == 0)
		run_times = (int) stress_test_runs*rate;
	else
		run_times = UREE_stress_param[index].runs * rate;

	init_test_param(param);
	for (int j = 0; j < (int) max_stress_test_param; j++) {
		if (UREE_stress_param[index].param[j].value.a != 0xffffffff)
			param[j].value.a = UREE_stress_param[index].param[j].value.a;

		if (UREE_stress_param[index].param[j].value.b != 0xffffffff)
			param[j].value.b = UREE_stress_param[index].param[j].value.b;
	}

	for(int z = 0; z < run_times; z++)
		UREE_stress_loop(UREE_stress_param[index].server_name,
		UREE_stress_param[index].cmd, param);

}
////////////////stress test, end//////////////////////////////////////////////////////////////////////

/* Dynamic loading: CA sample code*/
int read_elf_file(char *elf_file, char **out_buf)
{
	int fd;
	struct stat st;
	int file_len;
	int read_len;

	fd = open(elf_file, O_RDONLY);
	if (fd == -1) {
		ERR_LOG("file %s not found!\n", elf_file);
		return -1;
	}

	if (fstat(fd, &st)) {
		ERR_LOG("fstat failed!\n");
		return -1;
	}

	file_len = st.st_size;
	*out_buf = malloc(file_len);
	if (*out_buf == NULL) {
		ERR_LOG("malloc failed! size=%d\n", file_len);
		return -1;
	}
	//memset(*out_buf, 0, file_len);

	read_len = read(fd, *out_buf, file_len);
	if (read_len != file_len) {
		printf("read_len %d != file_len %d\n", read_len, file_len);
		return -1;
	}

	read_len = file_len;
	close(fd);
	return read_len;
}

typedef struct uuid
{
	uint32_t time_low;
	uint16_t time_mid;
	uint16_t time_hi_and_version;
	uint8_t clock_seq_and_node[8];
} uuid_t;

static uuid_t gz_test_uuid = { 0xd2c50855, 0x6f2b, 0x57ee, { 0x2d, 0xb6, 0x7c, 0x49, 0x94, 0x4f, 0x30, 0xa0 }};

void DL(char *elf_file, char *HA)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE DL_session, mem_session;
	UREE_SHAREDMEM_PARAM shm_param;
	UREE_SHAREDMEM_HANDLE shm_handle;
	MTEEC_PARAM param[4];
	uint32_t types, unload_types;
	char *elf_buf = NULL;
	unsigned char *shm_buf = NULL;
	int shm_size;
	int read_len;

	/*for shared memory buf*/
	int ion_fd = -1;
	ion_user_handle_t ion_hd = -1;
	int map_fd = -1;


	/**********for gz-test********************/
	int CA_ion_fd;

	/*for TA.elf*/
	ion_user_handle_t CA_ion_handle;
	int ion_shm_handle;
	UREE_SHAREDMEM_HANDLE gz_shm_handle;

	/*for TA stack**/
	int TA_stack_size ;
	ion_user_handle_t ion_handle_S;
	int ion_shm_handle_S;
	UREE_SHAREDMEM_HANDLE gz_chmhandle_S;

	/*for TA heap**/
	int TA_heap_size ;
	ion_user_handle_t ion_handle_H;
	int ion_shm_handle_H;
	UREE_SHAREDMEM_HANDLE gz_chmhandle_H;

	MTEEC_PARAM param_SH[4];

	/*for unload ha*/
	MTEEC_PARAM unload_param[4];
	char buf[TEST_STR_SIZE] = {0};
	/*******************************************/
	TEST_BEGIN("test_loader");

	ALOGI("[%s] Dynamic loading, start ......\n",__func__);

	if (elf_file == NULL) {
		ERR_LOG("elf_file == NULL\n");
		return;
	}
	DBG_LOG("elf_file = %s\n", elf_file);

	read_len = read_elf_file(elf_file, &elf_buf);
	shm_size = (read_len + (0x200000-1)) & ~(0x200000-1);
	//shm_buf = (char *) memalign(PAGE_SIZE,shm_size);  //orig way, maybe fail

	/*malloc shared mem: buf by ion API*/
	ret = _alloc_ion_buf(&ion_fd, &ion_hd, &map_fd, &shm_buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_alloc_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		goto out;

	EXPECT_NEQ(shm_buf, NULL, "shmem [buf] null!!!");
	if(shm_buf == NULL) {
		DBG_LOG("[%d]shmem shm_buf alloc fail. null.\n", __LINE__);
		goto out;
	}

	memset(shm_buf, 0, shm_size);
	memcpy(shm_buf, elf_buf, read_len);
	DBG_LOG("read_len = %d shm_size=%d\n", read_len, shm_size);
	if (elf_buf == NULL)
		return;


	ret = UREE_CreateSession(mem_srv_name, &mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create mem srv session");

	ret = UREE_CreateSession(DL_srv_name, &DL_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create DL srv session");

	/***chunk memory (for TA elf) start*****************************/
	/* create HA<-->TA memory*/
	CA_ion_fd = ion_open();
	DBG_LOG("open CA_ion_fd = %d \n", CA_ion_fd);

	/* ion alloc MTEE/TEE shared memory*/
	ret = ion_alloc(CA_ion_fd, shm_size, 0, PMEM_ION_MASK,
		ION_FLAG_MM_HEAP_INIT_ZERO, &CA_ion_handle);
	EXPECT_EQ_EXIT(0, ret, "ion alloc shared memory");

	/* get HA_HATA_ion_sec_shm_handle and TA_HATA_ion_secshm_handle*/
	ret = get_protected_handle(CA_ion_fd, CA_ion_handle, &ion_shm_handle);
	EXPECT_EQ_EXIT(0, ret, "get ion_shm_handle");
	DBG_LOG("ion_shm_handle = 0x%x \n", ion_shm_handle);

	/* use TA memory handle to get HA memory handle*/
	ret = UREE_ION_TO_SHM_HANDLE(ion_shm_handle, &gz_shm_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "ion_shm_handle to gz_shm_handle");
	/***chunk memory related? end*******************************/

	shm_param.buffer = (void *) shm_buf;
	shm_param.size = shm_size;
	ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "register share mem");

	param[0].value.a = shm_handle;
	param[0].value.b = gz_shm_handle;
	param[1].value.a = read_len;
	types = TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT);
	ret = UREE_TeeServiceCall(DL_session, TZCMD_DYNAMIC_LOAD_CREATE_TA, types,
		param);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "UREE_TeeServiceCall");

	DBG_LOG("After UREE_TeeServiceCall, stack size = %d, heap size = %d \n",
		param[2].value.a, param[2].value.b);
	TA_heap_size = param[2].value.b;
	TA_stack_size = param[2].value.a;

	ret = UREE_UnregisterSharedmem(mem_session, shm_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "un-register share mem");

	/***chunk mem for loadee heap & stack !!!!!!!!!!!***********************************/
	ret = ion_alloc(CA_ion_fd, TA_stack_size, 0, PMEM_ION_MASK,
		ION_FLAG_MM_HEAP_INIT_ZERO, &ion_handle_S);
	EXPECT_EQ_EXIT(0, ret, "ion alloc shared memory");

	ret = get_protected_handle(CA_ion_fd, ion_handle_S, &ion_shm_handle_S);
	EXPECT_EQ_EXIT(0, ret, "get ion_shm_handle_S");

	ret = UREE_ION_TO_SHM_HANDLE(ion_shm_handle_S, &gz_chmhandle_S);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "ion_shm_handle_S to gz_chmhandle_S");

	/***for TA heap***/
	ret = ion_alloc(CA_ion_fd, TA_heap_size, 0, PMEM_ION_MASK,
		ION_FLAG_MM_HEAP_INIT_ZERO, &ion_handle_H);
	EXPECT_EQ_EXIT(0, ret, "ion alloc shared memory");

	ret = get_protected_handle(CA_ion_fd, ion_handle_H, &ion_shm_handle_H);
	EXPECT_EQ_EXIT(0, ret, "get ion_shm_handle_H");

	ret = UREE_ION_TO_SHM_HANDLE(ion_shm_handle_H, &gz_chmhandle_H);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "ion_shm_handle_H to gz_chmhandle_H");
	/***chunk mem for loadee heap & stack !!!!!!!!!!!END!!!***********************************/

	param_SH[0].value.a = gz_chmhandle_S;
	param_SH[0].value.b = gz_chmhandle_H;
	param_SH[1].value.a = gz_shm_handle; /* for free*/

	ret = UREE_TeeServiceCall(DL_session, TZCMD_DYNAMIC_LOAD_START_TA, types, param_SH);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "UREE_TeeServiceCall");

	lazy_function(HA, TZCMD_TEST_VERSION);
	/* for dynamic load TA stress test, not necessary ! start ...*/
	DBG_LOG("Press enter to exit HA \n");
	fflush(stdout);
	getchar();
	/* for dynamic load TA stress test, not necessary ! end ...*/
	lazy_function(HA, TZCMD_TEST_EXIT);

	/*unload HA*/
	memcpy(buf, &gz_test_uuid, sizeof(uuid_t));
	unload_param[0].mem.size = 256;
	unload_param[0].mem.buffer = (void *)buf;

	unload_types = TZ_ParamTypes1(TZPT_MEM_INPUT);
	ret = UREE_TeeServiceCall(DL_session, TZCMD_DYNAMIC_LOAD_UNLOAD_TA, unload_types, unload_param);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "UREE_TeeServiceCall");

	/* ion free MTEE/TEE shared memory*/
	ret = ion_free(CA_ion_fd, ion_handle_H);
	EXPECT_EQ(0, ret, "ion_handle_H free shared memory");
	ret = ion_free(CA_ion_fd, ion_handle_S);
	EXPECT_EQ(0, ret, "ion_handle_S free shared memory");
	ret = ion_free(CA_ion_fd, CA_ion_handle);
	EXPECT_EQ(0, ret, "CA_ion_handle free shared memory");

	/* ion close*/
	ret = ion_close(CA_ion_fd);
	EXPECT_EQ(0, ret, "close ion CA_ion_fd");

	ret = UREE_CloseSession(mem_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem srv session");
	ret = UREE_CloseSession(DL_session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close DL srv session");

	if (elf_buf != NULL)
		free(elf_buf);
out:
	//if (shm_buf != NULL)
	//	free(shm_buf);

	ret = _free_ion_buf(ion_fd, ion_hd, map_fd, shm_buf, shm_size);
	EXPECT_EQ(TZ_RESULT_SUCCESS, ret, "_free_ion_buf");
	if(ret != TZ_RESULT_SUCCESS)
		DBG_LOG("[%d]_free_ion_buf fail.\n", __LINE__);

	TEST_END;
	REPORT_UNITTESTS;
}

void test_chunkmem(void)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	int chm_size = 2 * 1024 * 1024;
	int CA_ion_fd;
	ion_user_handle_t CA_ion_handle;
	int ion_shm_handle;
	UREE_SHAREDMEM_HANDLE gz_shm_handle;

	TEST_BEGIN("chunkmem_CA_sample_code");

	ret = UREE_CreateSession(echo_srv_name, &session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "create mem srv session");

	/***Alloc chunk memory start*****************************/
	/*create HA<-->TA memory*/
	CA_ion_fd = ion_open();
	DBG_LOG("open CA_ion_fd = %d \n", CA_ion_fd);

	/* ion alloc MTEE/TEE shared memory*/
	ret = ion_alloc(CA_ion_fd, chm_size, 0, PMEM_ION_MASK,
		ION_FLAG_MM_HEAP_INIT_ZERO, &CA_ion_handle);
	EXPECT_EQ_EXIT(0, ret, "ion alloc shared memory");

	/* get HA_HATA_ion_sec_shm_handle and TA_HATA_ion_secshm_handle*/
	ret = get_protected_handle(CA_ion_fd, CA_ion_handle, &ion_shm_handle);
	EXPECT_EQ_EXIT(0, ret, "get ion_shm_handle");
	DBG_LOG("ion_shm_handle = 0x%x \n", ion_shm_handle);

	/* use TA memory handle to get HA memory handle*/
	ret = UREE_ION_TO_SHM_HANDLE(ion_shm_handle, &gz_shm_handle);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "ion_shm_handle to gz_shm_handle");
	/***Alloc chunk memory done*****************************/

	param[0].value.a = gz_shm_handle;
	param[0].value.b = chm_size;
	types = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT);
	ret = UREE_TeeServiceCall(session, TZCMD_TEST_CHMEM_QUERY_PA, types, param);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "UREE_TeeServiceCall");


	ret = ion_free(CA_ion_fd, CA_ion_handle);
	EXPECT_EQ(0, ret, "CA_ion_handle free shared memory");

	/* ion close*/
	ret = ion_close(CA_ion_fd);
	EXPECT_EQ(0, ret, "close ion CA_ion_fd");

	ret = UREE_CloseSession(session);
	EXPECT_EQ_EXIT(TZ_RESULT_SUCCESS, ret, "close mem srv session");

	TEST_END;
	REPORT_UNITTESTS;
}

enum gz_log_case {
	GZLOG_DEFAULT = 0,
	GZLOG_ONELINE,
	GZLOG_ZERO,
	GZLOG_HALF,
	GZLOG_ONEBYTE,
	GZLOG_NULL,
	GZLOG_END
};

int gz_log_rw(int logfd, int outfd, unsigned int size, enum gz_log_case mode)
{
	unsigned int rchars = 0, wchars = 0, line = 64;
	char *tbuf;

	if (mode == GZLOG_NULL) {
		printf("null pointer test\n");
		return read(logfd, NULL, 0);
	}

	tbuf = calloc(1, size + 1);
	if (!tbuf)
		return 0;

	switch (mode) {
	case GZLOG_ONELINE:
		size = (size < 256) ? size : 256;
		printf("one line  sz:%4d,", size);
		break;
	case GZLOG_ZERO:
		size = 0;
		printf("zero size sz:%4d,", size);
		break;
	case GZLOG_HALF:
		size /= 2;
		printf("half size sz:%4d,", size);
		break;
	case GZLOG_ONEBYTE:
		size = 1;
		printf("one byte  sz:%4d,", size);
		break;
	default:
		printf("normal    sz:%4d,", size);
		break;
	}

	rchars = read(logfd, tbuf, size);
	printf("read:%4d", rchars);

	if (rchars > 0) {
		tbuf[rchars] = '\0';

		if (outfd < 0)
			wchars = 0;
		else
			wchars = write(outfd, tbuf, rchars);

		if (wchars < 0 || wchars != rchars)
			printf("write file error, read %d, write %d not eq\n",
				rchars, wchars);

		line = (line < size) ? line : size;
		tbuf[line] = '\0';
		printf("::%s", tbuf);
	}
	printf("\n");

	free(tbuf);
	return rchars;
}

void test_gz_log(int max_round)
{
	int logfd, outfd, ret = 0;
	unsigned int max_size = 1 << 13, minor_round = 10, i, rchars = 0;
	struct pollfd pfd;

	max_round = (max_round > 0) ? max_round : 0;
	DBG_LOG("gz_log stress test start with round %d\n", max_round);


	outfd = open("/data/gz_log_output.log", O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR);
	if (outfd < 0)
		printf("[%s] Open out file failed\n", __func__);

	while ((max_round--) > 0) {
		TEST_BEGIN("gz_log-stress-test");

		logfd = open("/proc/gz_log", O_RDONLY);
		if (logfd < 0) {
			EXPECT_LE(0, logfd, "Open gz_log failed");
			TEST_END;
			break;
		}

		pfd.fd = logfd;
		pfd.events = POLLIN;
		srand(time(0));

		for (i = 0; i < minor_round; i++) {
			ret = poll(&pfd, 1, -1);

			if (ret < 0) {
				EXPECT_LE(0, ret, "gz_log poll error");
				close(logfd);
				TEST_END;
				break;
			}

			if (pfd.revents & POLLIN) {
				unsigned int size = rand() % max_size + 1;
				unsigned int mode = rand() % GZLOG_END;
				rchars = gz_log_rw(logfd, outfd, size, mode);
				if (mode == GZLOG_NULL) {
					/* EINVAL */
					EXPECT_EQ(22, errno,
						  "NULL test error");
				} else {
					EXPECT_LE(0, rchars,
						  "gz_log read error,");
 					if (rchars < 0) {
						close(logfd);
						TEST_END;
 					}
				}

			} else
				printf("no events\n");
		}

		close(logfd);
		TEST_END;
	}

	REPORT_UNITTESTS;
	close(outfd);
}

int run_cmd_11335577(UREE_SESSION_HANDLE session)
{
	char buf[256] = {0};
	MTEEC_PARAM param[1];
	uint32_t types;
	param[0].mem.size = 256;
	param[0].mem.buffer = (void *)buf;
	types = TZ_ParamTypes1(TZPT_MEM_OUTPUT);

	return UREE_TeeServiceCall(session, 0x11335577, types, param);
}

static void *thread1_entry(void *ptr)
{
	int ret;
	UREE_SESSION_HANDLE my_session;
	UREE_SESSION_HANDLE main_session = *(UREE_SESSION_HANDLE *)ptr;
	uint64_t show=0;
	time_t timep;

	DBG_LOG("session=%u %s for main_session\n", main_session, __func__);
	if (main_session==0xFFFFFFFF) {
		ret = UREE_CreateSession(SRV_NAME("test"), &my_session);
		DBG_LOG("session=%u %s for my_session\n", my_session, __func__);
	} else {
		my_session = main_session;
	}

	while (1) {
		ret = run_cmd_11335577(my_session);
		if (ret) {
			ERR_LOG("%s exit(%d)\n", __func__, ret);
			break;
		}
		if ((show++&0xffff)==0) {
			time (&timep);
			DBG_LOG("%s mem_type show=%"PRIu64" at %s", __func__, show, ctime(&timep));
		}
	}

	if (main_session==0xFFFFFFFF) {
		UREE_CloseSession(my_session);
	}
	pthread_exit(NULL);
}

uint32_t cnt[4] = {0x0, 0x100, 0x10000, 0x1000000};

int run_cmd_22446688(UREE_SESSION_HANDLE session)
{
    TZ_RESULT ret;
    MTEEC_PARAM param[4];
    uint32_t types;
    //uint32_t vpu_struct[2] ={ 0 };

    param[0].value.a = cnt[0];
    param[0].value.b = 0;
    param[1].value.a = cnt[1];
    param[1].value.b = 0;
    param[2].value.a = cnt[2];
    param[2].value.b = 0;
    param[3].value.a = cnt[3];
    param[3].value.b = 0;

    types = TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT,
                    TZPT_VALUE_INOUT, TZPT_VALUE_INOUT);

    ret = UREE_TeeServiceCall(session, 0x22446688, types, param);
    if (ret)
    {
        ERR_LOG("UREE_TeeServiceCall Error: ret %d\n", ret);
		return ret;
    }
	else {
        if ( (param[0].value.b != param[0].value.a) ||
			 (param[1].value.b != param[1].value.a) ||
			 (param[2].value.b != param[2].value.a) ||
			 (param[3].value.b != param[3].value.a) )
        {
			ERR_LOG("Get incorrect 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				param[0].value.a, param[0].value.b, param[1].value.a, param[1].value.b,
				param[2].value.a, param[2].value.b, param[3].value.a, param[3].value.b);
			ret = -1;
        }
	}
	cnt[0]++;
	cnt[1]++;
	cnt[2]++;
	cnt[3]++;
	return ret;
}

static void *thread2_entry(void *ptr)
{
	int ret;
	UREE_SESSION_HANDLE my_session;
	UREE_SESSION_HANDLE main_session = *(UREE_SESSION_HANDLE *)ptr;
	uint64_t show=0;
	time_t timep;

	DBG_LOG("session=%u %s for main_session\n", main_session, __func__);
	if (main_session==0xFFFFFFFF) {
		ret = UREE_CreateSession(SRV_NAME("test"), &my_session);
		DBG_LOG("session=%u %s for my_session\n", my_session, __func__);
	} else {
		my_session = main_session;
	}

	while (1) {
		ret = run_cmd_22446688(my_session);
		if (ret) {
			ERR_LOG("%s exit(%d)\n", __func__, ret);
			break;
		}
		if ((show++&0xffff)==0) {
			time (&timep);
			DBG_LOG("%s value_type show=%"PRIu64" at %s", __func__, show, ctime(&timep));
		}
	}

	if (main_session==0xFFFFFFFF) {
		UREE_CloseSession(my_session);
	}
	pthread_exit(NULL);
}

void again_test(bool one_session) {
	UREE_SESSION_HANDLE main_session = 0xFFFFFFFF;
	pthread_t thread1_id, thread2_id;
	int ret;

	if (one_session) {
		ret = UREE_CreateSession(SRV_NAME("test"), &main_session);
		DBG_LOG("session=%u %s for main_session\n", main_session, __func__);
	}

	pthread_create(&thread1_id, NULL, &thread1_entry, &main_session);
	pthread_create(&thread2_id, NULL, &thread2_entry, &main_session);
	pthread_setname_np(thread1_id, "myut1");
	pthread_setname_np(thread2_id, "myut2");

	pthread_join(thread1_id, NULL);
	pthread_join(thread2_id, NULL);
	if (one_session) {
		UREE_CloseSession(main_session);
	}
	DBG_LOG("test exit\n");
}

int main(int argc, char *argv[])
{
	RESET_UNITTESTS;
	uint32_t test_max_cnt;
	uint32_t thread_num;
	char mode[5], buf[50];
	int stress_rate, tmp_rate, tmp_index;

	char *c;
	int i = 1;

	if ( strcmp(argv[1], "again_test") == 0 ) {
		bool one_session = true;
		if ((argc >= 3) && (strcmp(argv[2], "private_session") == 0))
			one_session = false;
		again_test(one_session);
		return 0;
	}

	if ((argc == 3) && (strcmp(argv[1], "echo") == 0)) {
		int cmd = strtol(argv[2], NULL, 10);
		echo_function (cmd);
		return 0;
	} else if (strcmp(argv[1], "echo") == 0) {
		printf ("Wrong command!!!! ex: %s %s [cmd] (cmd: copmmand in integer)\n",
			argv[0], argv[1]);
		return -1;
	}

	if ((argc == 4) && (strcmp(argv[1], "lazy") == 0)) {
		char *server_name = argv[2];
		int cmd = strtol(argv[3], NULL, 10);
		lazy_function (server_name, cmd);
		return 0;
	} else if (strcmp(argv[1], "lazy") == 0) {
		printf ("Wrong command!!!! ex: %s %s [server_name] [cmd] (server_name: server name in string, cmd: copmmand in integer)\n",
			argv[0], argv[1]);
		return -1;
	}

	if (strcmp(argv[1], "multicore") == 0) {
		char *server_name = SRV_NAME("test");
		int test_case = 0;
		// multicore how to, X is test count
		// multicore 1 X	omp multi-core/single-core UT
		// multicore 10 X	omp multi-core/single-core UT, but when measure score loser is multi-core, stop test
		// multicore 6 X	fork thread multi-core/single-core UT
		// multicore 60 X	fork thread multi-core/single-core UT, but when measure score loser is multi-core, stop test
		// multicore 7 X	use semaphore to trigger fork thread multi-core/single-core UT
		// multicore 70 X	use semaphore to trigger fork thread multi-core/single-core UT, but when measure score loser is multi-core, stop test
		// multicore 2 X	omp integrate test
		// multicore 20 X	fork thread integrate test
		// multicore 21 X	use semaphore to trigger fork thread integrate test
		if (argv[2] != NULL) {
			test_case = strtol(argv[2], NULL, 10);
		}
		test_max_cnt = 10;
		if (argv[3] != NULL) {
			test_max_cnt = strtol(argv[3], NULL, 10);
		}
		multicore_test(server_name, test_max_cnt, test_case);
		return 0;
	}

	if (argc == 3 && (strcmp(argv[1], "imgrecog")) == 0) {
		ALOGI("test image recognition by feeding images directly\n");
		test_ImageRecognition(argv[2]);
		return 0;
	}

#ifdef GP_TEE_ENABLED
	if (strcmp(argv[1], "SharedMem_HAs") == 0) {
		test_max_cnt = 10;
		if (argv[2] != NULL) {
			test_max_cnt = strtol(argv[2], NULL, 10);
		}
		SharedMem_HAs(test_max_cnt);
		return 0;
	}

	if ( strcmp(argv[1], "gDL") == 0 ) {
		DL("/data/gz-test.elf", SRV_NAME("test"));
		return 0;
	}

	if ((argc == 4) && strcmp(argv[1], "DL") == 0) {
		DL(argv[2], argv[3]);
		return 0;
	}
#endif

	while(argv[i]) {
		c = argv[i++];
		printf("Test Case %s:\n", c);
		switch(*c) {
		case '0':
			DBG_LOG ("=> in test.c: test_Session() is running. \n");
			test_Session();
			break;
		case '1':
			DBG_LOG ("=> in test.c: test_ServiceCall() is running. \n");
			test_ServiceCall();
			break;
		case '2':
			DBG_LOG ("=> in test.c: test_SharedMem() is running. \n");
			test_SharedMem();
			break;
		case '3':
			DBG_LOG ("=> in test.c: test_SecureMem() is running. \n");
			test_SecureMem();
			break;
		case '4':
			DBG_LOG ("=> in test.c: test_SecureMem_WithTag() is running.\n");
			test_SecureMem_WithTag();
			break;
		case '5':
			DBG_LOG ("=> in test.c: run_internal_test() is running. \n");
			run_internal_test(NULL);
			break;
		case '6':
			DBG_LOG ("=> in test.c: dma_test() is running. \n");
			dma_test();
			break;
		case '7':
			DBG_LOG ("=> in test.c: ??? is running. \n");
			break;
		case '8':
			DBG_LOG ("=> in test.c: gz_abort_test() is running. \n");
			gz_abort_test();
			break;
		case '9':
			DBG_LOG ("=> in test.c: gz_RTC_test() is running. \n");
			if (argv[i])
				strcpy(mode, argv[i++]);
			else
				strcpy(mode, "r");
			if (argv[i])
				strcpy(buf, argv[i++]);
			else
				strcpy(buf, "default");
			gz_rtc_test(mode, buf);
			break;
		case 'a':
			DBG_LOG ("=> in test.c: gz_internel_ipc_test() is running. \n");
			gz_internal_ipc_test();
			break;
		case 'b':
			DBG_LOG ("=> in test.c: gz_GetVersion_test() is running. \n");
			gz_GetVersion_test();
			break;
		case 'c':
			DBG_LOG ("=> in test.c: gz_GetCellInfo_test() is running. \n");
			gz_GetCellInfo_test();
			break;
#ifdef GP_TEE_ENABLED
		case 'd':
			DBG_LOG ("=> in test.c: test_TeeShmIPC() is running. \n");
			test_TeeShmIPC();
			break;
#endif
		case 'm':
			if (argv[i])
				thread_num = strtol(argv[i++], NULL, 10);
			else
				thread_num = 2;
			if (argv[i])
				test_max_cnt = strtol(argv[i++], NULL, 10);
			else
				test_max_cnt = 1;
			ALOGE("=> in test.c: test_MultiThread() is running. thread_num=%d, test_max_cnt=%d\n",
				thread_num, test_max_cnt);
			DBG_LOG ("=> in test.c: test_MultiThread() is running. thread_num=%d, test_max_cnt=%d\n",
				thread_num, test_max_cnt);
			test_MultiThread(thread_num, test_max_cnt);
			break;
		case 's': /*s for stress*/
			printf("How stress do you want????\n");
			printf("Stress rate: ");
			scanf("%d",&stress_rate);
			printf("\n");
			DBG_LOG ("=> in test.c: stress_test() is running. \n");
			UREE_stress_test(stress_rate);
			break;
		case 'g':
			DBG_LOG ("=> in test.c: test_SecureCarema() is running. \n");
			test_SecureCarema();
			break;
		case 'p':
			if (argv[i])
				thread_num = strtol(argv[i++], NULL, 10);
			else
				thread_num = 2;
			if (argv[i])
				test_max_cnt = strtol(argv[i++], NULL, 10);
			else
				test_max_cnt = 1;
			ALOGE("=> in test.c: test_pThread() is running. thread_num=%d, test_max_cnt=%d\n",
				thread_num, test_max_cnt);
			DBG_LOG ("=> in test.c: test_pThread() is running. thread_num=%d, test_max_cnt=%d\n",
				thread_num, test_max_cnt);
			test_pThread(thread_num, test_max_cnt);
			break;
		case 't':/*t for temp. For temp stress test*/
			if (argv[i])
				tmp_index = strtol(argv[i++], NULL, 10);
			else
				tmp_index = 0;
			if (argv[i])
				tmp_rate = strtol(argv[i++], NULL, 10);
			else
				tmp_rate = 100;
			DBG_LOG ("=> in test.c: UREE_stress_test is running.\n");
			tmp_stress_test(tmp_index, tmp_rate);
			break;
		case 'M': /*for 'M'TEE version*/
			DBG_LOG("show MTEE version in UREE & dump threads\n");
			ALOGI("show MTEE version in UREE & dump threads\n");
			MTEEversionInUREE();
			break;
		case 'Q':
			DBG_LOG(" CA sample code: query chunk mem PA \n");
			test_chunkmem();
			break;
		case 'L':
			DBG_LOG("====> gz_log stress test\n");
			if (argv[i])
				test_gz_log(strtol(argv[i++], NULL, 10));
			else
				test_gz_log(5000);
			break;
		default:
			DBG_LOG("ERROR: Unknown test case\n");
			break;
		}
	}

	return 0;
}
