/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "MobiCoreDriverApi.h"
#include "tlcrpmb.h"

#define LOG_TAG "TLC_RPMB_TEST"
#include "log.h"

#define LOG_I printf
#define LOG_E printf

#define PRINT_RUN_RPMB_OPEN(uid)  printf("Begin run tlcRpmbOpen(), uid: 0x%x\n\n", uid)
#define PRINT_RUN_RPMB_READ(uid)  printf("Begin run tlcRpmbRead(), uid: 0x%x\n\n", uid)
#define PRINT_RUN_RPMB_WRITE(uid) printf("Begin run tlcRpmbWrite(), uid: 0x%x\n\n", uid)
#define PRINT_RUN_RPMB_GET_SIZE(uid)  printf("Begin run tlcRpmbGetSize(), uid: 0x%x\n\n", uid)

#define PRINT_FUNC_CALL_RET(funcName, ret) printf("Call %s(), ret:%d\n", funcName, ret)
#define FUNC_OPEN_STR "tlcRpmbOpen"
#define FUNC_READ_STR "tlcRpmbRead"
#define FUNC_WRITE_STR "tlcRpmbWrite"
#define FUNC_GET_SIZE_STR "tlcRpmbGetSize"

#define TEST_PASS_RET_ZERO(ret)  \
        do { \
            if (ret == 0) { \
                printf("============Test case PASS!!============\n"); \
            }\
            else { \
                printf("-------------Test case FAIL!!------------\n"); \
                printf("######RPMB test finished, Some test case fail.#######, line: %d\n\n", __LINE__); \
                caClose(); \
                exit(-1); \
            } \
        }while(0)

#define TEST_PASS_RET_NON_ZERO(ret) \
        do { \
            if (ret != 0) { \
                printf("============Test case PASS!!============\n"); \
            }\
            else { \
                LOG_I("------------Test case FAIL!!------------\n"); \
                printf("#######RPMB test finished, Some test cases fail.#######, line:%d\n\n", __LINE__); \
                caClose(); \
                exit(-1); \
            } \
        }while(0)

#define MAX_RPMB_USER_NUMBER 4

typedef enum {
    RPMB_USER_ID_BASE = 0,
    RPMB_USER_ID_0 = RPMB_USER_ID_BASE,
    RPMB_USER_ID_1,
    RPMB_USER_ID_2,
    RPMB_USER_ID_3,
    RPMB_USER_ID_END = RPMB_USER_ID_BASE + MAX_RPMB_USER_NUMBER,
} rpmb_uid_t;

#define convertUserId(uid) (uid - RPMB_USER_ID_BASE)



static uint32_t userRpmbSize = 0x400000;

int read_data_from_file(uint8_t *buf, uint32_t size)
{
	FILE*   pStream;
	long    filesize;
	uint8_t* content = NULL;
    const char *pPath = "/system/framework/framework.jar";
	/*
	 * The stat function is not used (not available in WinCE).
	 */

	/* Open the file */
	pStream = fopen(pPath, "rb");
	if (pStream == NULL)
	{
		LOG_E("Error: Cannot open file: %s.\n\n", pPath);
		return 0;
	}

	if (fseek(pStream, 0L, SEEK_END) != 0)
	{
		LOG_E("Error: Cannot read file: %s.\n\n", pPath);
		goto error;
	}

	filesize = ftell(pStream);
	if (filesize < 0)
	{
		LOG_E("Error: Cannot get the file size: %s.\n", pPath);
		goto error;
	}

	if (filesize == 0)
	{
		LOG_E("Error: Empty file: %s.\n", pPath);
		goto error;
	}

	if (filesize < size)
	{
		LOG_E("Error: file size too short: %s.\n", pPath);
		goto error;
	}

	/* Set the file pointer at the beginning of the file */
	if (fseek(pStream, 0L, SEEK_SET) != 0)
	{
		LOG_E("Error: Cannot read file: %s.\n\n", pPath);
		goto error;
	}

	/* Read data from the file into the buffer */
	if (fread(buf, (size_t)size, 1, pStream) != 1)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n\n", pPath);
		goto error;
	}
    fclose(pStream);
    return 0;

error:
	fclose(pStream);
	return -1;
}
/*
int test_case_open(void)
{
    int ret;

    printf("############### Open RPMB Session Test ###############\n");
    LOG_I("############### Open RPMB Session Test ###############");
    PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_0);
    ret = caOpen(RPMB_USER_ID_0);
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR,ret);
    TEST_PASS_RET_ZERO(ret);


    PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_0);
    ret = tlcRpmbOpen(RPMB_USER_ID_0);
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR,ret);
    TEST_PASS_RET_ZERO(ret);

    PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_1);
    ret = tlcRpmbOpen(RPMB_USER_ID_1);
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR, ret);
    TEST_PASS_RET_NON_ZERO(ret);

    PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_2);
    ret = tlcRpmbOpen(RPMB_USER_ID_2);
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR, ret);
    TEST_PASS_RET_NON_ZERO(ret);

    PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_3);
    ret = tlcRpmbOpen(RPMB_USER_ID_3);
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR, ret);
    TEST_PASS_RET_NON_ZERO(ret);

    PRINT_RUN_RPMB_OPEN(0xABCD);
    ret = tlcRpmbOpen((rpmb_uid_t)0xABCD);
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR, ret);
    TEST_PASS_RET_NON_ZERO(ret);

    //fork a child process to open session with same uid and different uid.
    pid_t pid = fork();
    if(pid < 0) {
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    TEST_PASS_RET_ZERO(ret);

    if (0 == pid)
	{
		LOG_I("+++++++ Child process is forked to execute another rpmb_test ++++++++");
		execl("/system/bin/rpmb_test\n", "rpmb_test\n", "1\n", (char *) NULL);
		LOG_E("Some error occur: %s\n", strerror(errno));
		ret = -1;
        TEST_PASS_RET_ZERO(ret);
	}

    if (0 < pid)
	{
	    int status;
	    //wait child process to exit.
		waitpid(pid, &status, WUNTRACED);
		LOG_I("+++++++ Child process exited. +++++++");
		if (WIFEXITED(status))
		{
			ret = WEXITSTATUS(status);
            TEST_PASS_RET_ZERO(ret);
		}
	}

    return 0;
}
*/

int test_case_write_read_1(rpmb_uid_t uid)
{
#define TEST_BUF_SIZE   65535   //64KB
    int i;
    int j;
    uint32_t offset_tab[] = {0, 128, 256, 1024, 19457, 32761, 0xFFFFFFFF};
    uint32_t data_len_tab[] = {0, 112, 683, 9603, 19201, 32768};
    //uint32_t offset_tab[1] = {0xFFFFFFFF};
    //uint32_t data_len_tab[1] = {9603};
    int offset_tab_entries = sizeof(offset_tab)/sizeof(offset_tab[0]);
    int datalen_tab_entries = sizeof(data_len_tab)/sizeof(data_len_tab[0]);
    uint8_t *write_buf = NULL;
    uint8_t *read_buf = NULL;
    uint32_t tranSize;
    uint32_t result;
    int ret;

    //LOG_I("############### Write/Read/Compare Test1, user:0x%x ###############\n", convertUserId(uid));
    printf("############### Write/Read/Compare Test1, user:0x%x ###############\n\n", convertUserId(uid));

    //alloc read/write buffer.
    write_buf = (uint8_t *)malloc(TEST_BUF_SIZE);
    if (write_buf == NULL)
    {
        ret = -1;
        TEST_PASS_RET_ZERO(ret);        
    }

    read_buf = (uint8_t *)malloc(TEST_BUF_SIZE);
    if (read_buf == NULL)
    {
        ret = -1;
        TEST_PASS_RET_ZERO(ret);        
    }    

    //======= fill write buffer with test file data =======
    ret = read_data_from_file(write_buf, TEST_BUF_SIZE);
    PRINT_FUNC_CALL_RET("read_data_from_file\n",ret);
    TEST_PASS_RET_ZERO(ret);
    
    for (i=0; i < offset_tab_entries; i++)
    {
        //we found that the last half-block of the whole rpmb can not be written with its half-block address
        //but write the last full-block (last half-block address -1) is allowed.
        if (offset_tab[i] == 0xFFFFFFFF)
        {
            datalen_tab_entries = 1;
            data_len_tab[0] = 599;
            offset_tab[i] = userRpmbSize - 512;
        }
        
        for (j=0; j < datalen_tab_entries; j++)
        {
            LOG_I("Write Test1, Offset: %u, Size: %u\n", offset_tab[i], data_len_tab[j]);
            tranSize = data_len_tab[j];
            PRINT_RUN_RPMB_WRITE(uid);
            ret = (int)caRpmbWriteByOffset(write_buf, tranSize, offset_tab[i], uid, &result);
            PRINT_FUNC_CALL_RET(FUNC_WRITE_STR, ret);
            if (tranSize==0 || offset_tab[i] >= userRpmbSize)
            {
                TEST_PASS_RET_NON_ZERO(ret);
            }
            else
            {
                TEST_PASS_RET_ZERO(ret);

                if (data_len_tab[j]+offset_tab[i] > userRpmbSize)
                {
                    ret = (tranSize == (userRpmbSize - offset_tab[i])) ? 0 : -1;
                    TEST_PASS_RET_ZERO(ret);
                }
            }

            LOG_I("Read Test1, Offset: %u, Size: %u\n", offset_tab[i], data_len_tab[j]);           
            tranSize = data_len_tab[j];
            memset(read_buf, 0, TEST_BUF_SIZE);
            PRINT_RUN_RPMB_READ(uid);
            ret = (int)caRpmbReadByOffset(read_buf, tranSize, offset_tab[i], uid, &result);
            PRINT_FUNC_CALL_RET(FUNC_READ_STR, ret);
            if (tranSize==0 || offset_tab[i] >= userRpmbSize)
            {
                TEST_PASS_RET_NON_ZERO(ret);
            }
            else
            {
                TEST_PASS_RET_ZERO(ret);

                if (data_len_tab[j]+offset_tab[i] > userRpmbSize)
                {
                    ret = (tranSize == (userRpmbSize - offset_tab[i])) ? 0 : -1;
                    TEST_PASS_RET_ZERO(ret);
                }
            }

            if (ret == 0)
            {
                //LOG_I("Data Compare Test1, Offset: %u, Size: %u\n", offset_tab[i], tranSize);
                printf("Data Compare Test1, Offset: %u, Size: %u\n\n", offset_tab[i], tranSize);
                ret = memcmp(write_buf, read_buf, tranSize);
#if 1                
                uint32_t *write_buf_ptr = (uint32_t *)(write_buf);
                uint32_t *read_buf_ptr = (uint32_t *)(read_buf);
                LOG_I("Write_buf(0x%x), 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x\n", offset_tab[i],
                      write_buf_ptr[0], write_buf_ptr[1], write_buf_ptr[2], write_buf_ptr[3],
                      write_buf_ptr[4], write_buf_ptr[5], write_buf_ptr[6], write_buf_ptr[7]);
                LOG_I("Read_buf(0x%x), 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x, 0x%.8x\n", offset_tab[i],
                      read_buf_ptr[0], read_buf_ptr[1], read_buf_ptr[2], read_buf_ptr[3],
                      read_buf_ptr[4], read_buf_ptr[5], read_buf_ptr[6], read_buf_ptr[7]);
#endif
                TEST_PASS_RET_ZERO(ret);
            }
        }
    }

    free(write_buf);
    free(read_buf);
    return 0;
}

int test_case_write_read_2(rpmb_uid_t uid)
{
    uint32_t offset = 0;
    uint32_t size = 123;
    uint32_t total_size = userRpmbSize;
    uint8_t *write_buf = NULL;
    uint8_t *read_buf = NULL;    
    uint32_t result;
    int ret = 0;

    //LOG_I("############### Write/Read/Compare Test2, user:0x%x ###############\n", convertUserId(uid));
    printf("############### Write/Read/Compare Test2, user:0x%x ###############\n\n", convertUserId(uid));

    //alloc read/write buffer.
    write_buf = (uint8_t *)malloc(userRpmbSize);
    if (write_buf == NULL)
    {
        ret = -1;
        TEST_PASS_RET_ZERO(ret);        
    }

    read_buf = (uint8_t *)malloc(userRpmbSize);
    if (read_buf == NULL)
    {
        ret = -1;
        TEST_PASS_RET_ZERO(ret);        
    }

    //======= fill write buffer with test file data =======
    ret = read_data_from_file(write_buf, userRpmbSize);
    PRINT_FUNC_CALL_RET("read_data_from_file\n",ret);
    TEST_PASS_RET_ZERO(ret);

    printf("Write/Read Test2, Total size: %u\n\n", total_size);
    while (offset < total_size)
    {
        LOG_I("Write Test2, Offset: %d, Size: %d\n", offset, size);
        printf("Write Test2, Offset: %d, Size: %d\n\n", offset, size);
        ret = (int)caRpmbWriteByOffset(write_buf+offset, size, offset, uid, &result);
        PRINT_FUNC_CALL_RET(FUNC_WRITE_STR, ret);        
        TEST_PASS_RET_ZERO(ret);
        
        offset += size;
        size *= 3;
        if ((offset + size) > total_size)
        {
            size = total_size - offset;
        }
    }

    offset = 0;
    size = 99;
    while (offset < total_size)
    {
        LOG_I("Read Test2, Offset: %d, Size: %d\n", offset, size);
        printf("Read Test2, Offset: %d, Size: %d\n\n", offset, size);        
        ret = (int)caRpmbReadByOffset(read_buf+offset, size, offset, uid, &result);
        PRINT_FUNC_CALL_RET(FUNC_READ_STR, ret);        
        TEST_PASS_RET_ZERO(ret);
        
        offset += size;
        size *= 5;
        if ((offset + size) > total_size)
        {
            size = total_size - offset;
        }
    }
    
    LOG_I("Data Comare Test2, Total Size: %u\n", total_size);
    printf("Data Comare Test2, Total Size: %u\n\n", total_size);    
    ret = memcmp(write_buf, read_buf, total_size);
    TEST_PASS_RET_ZERO(ret);    

    return 0;
}

//==================================================================//
int rpmb_write_read_test(rpmb_uid_t uid)
{
    int ret;

		
    //open rpmb session for user X
    PRINT_RUN_RPMB_OPEN(uid);
    ret = caOpen();
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR,ret);
    TEST_PASS_RET_ZERO(ret);

    //read rpmb size for user X
    //PRINT_RUN_RPMB_GET_SIZE(uid);
    //ret = tlcRpmbGetSize(uid, &userRpmbSize);
    //PRINT_FUNC_CALL_RET(FUNC_GET_SIZE_STR, ret);
    //TEST_PASS_RET_ZERO(ret);

    //write/read test for user 0
    //test_case_write_read_1(uid);
    test_case_write_read_2(uid);

    caClose();
    return ret;
}

//==========================================================//
int rpmb_performance_test(void)
{
    uint8_t *write_buf = NULL;
    uint8_t *read_buf = NULL;
    uint32_t size = 0;
    int ret = 0;
    uint32_t data_len_tab[] = {256, 512, 1024, 4096, 16384/*16KB*/, 65536/*64KB*/, 
                               262144/*256KB*/, 524288/*512KB*/, 1048576/*1MB*/, 2097152/*2MB*/,0};    
    struct timeval start_time, finish_time;
    double elapsed_time;
    double throughput;
    uint32_t result;
    int i = 0;

//    LOG_I("############### RPMB Performance Test ###############");
    printf("############### RPMB Performance Test ###############\n");

    //open rpmb session for user X
    PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_0);
    ret = caOpen();
    PRINT_FUNC_CALL_RET(FUNC_OPEN_STR,ret);
    TEST_PASS_RET_ZERO(ret);

    //read rpmb size for user X
    //PRINT_RUN_RPMB_GET_SIZE(RPMB_USER_ID_0);
    //ret = caRpmbGetSize(RPMB_USER_ID_0, &userRpmbSize);
    //PRINT_FUNC_CALL_RET(FUNC_GET_SIZE_STR, ret);
    //TEST_PASS_RET_ZERO(ret);
    
    //alloc read/write buffer.
    write_buf = (uint8_t *)malloc(userRpmbSize);
    if (write_buf == NULL)
    {
        ret = -1;
        TEST_PASS_RET_ZERO(ret);        
    }

    read_buf = (uint8_t *)malloc(userRpmbSize);
    if (read_buf == NULL)
    {
        ret = -1;
        TEST_PASS_RET_ZERO(ret);        
    }

    //======= fill write buffer with test file data =======
    ret = read_data_from_file(write_buf, userRpmbSize);
    PRINT_FUNC_CALL_RET("read_data_from_file\n",ret);
    TEST_PASS_RET_ZERO(ret);

//    LOG_I("############### Write Performance Test ###############");
    printf("############### Write Performance Test ###############\n");
    for (i=0; data_len_tab[i] != 0; i++)
    {
        if (data_len_tab[i] <= userRpmbSize)
        {
            size = data_len_tab[i];
        }
        else
        {
            size = userRpmbSize;
            data_len_tab[i+1] = 0;
        }

        gettimeofday(&start_time, NULL);
        ret = (int)caRpmbWriteByOffset(write_buf, size, 0, RPMB_USER_ID_0, &result);
        gettimeofday(&finish_time, NULL);
        PRINT_FUNC_CALL_RET(FUNC_WRITE_STR, ret);        
        TEST_PASS_RET_ZERO(ret);
              
        elapsed_time = ((double)finish_time.tv_sec + (double)finish_time.tv_usec/1000000) -
                       ((double)start_time.tv_sec + (double)start_time.tv_usec/1000000);                    
        throughput = (double)size/1024/elapsed_time;
        printf("Write Data Lenth: %dB, elapsed time: %.4fs, throughput: %.4fKB/s\n\n", size, elapsed_time, throughput);
    }

//    LOG_I("############### Read Performance Test ###############");
    printf("############### Read Performance Test ###############\n");
    for (i=0; data_len_tab[i] != 0; i++)
    {
        if (data_len_tab[i] <= userRpmbSize)
        {
            size = data_len_tab[i];
        }
        else
        {
            size = userRpmbSize;
            data_len_tab[i+1] = 0;
        }
        
        gettimeofday(&start_time, NULL);
        ret = (int)caRpmbReadByOffset(read_buf, size, 0, RPMB_USER_ID_0, &result);
        gettimeofday(&finish_time, NULL);
        PRINT_FUNC_CALL_RET(FUNC_READ_STR, ret);
        TEST_PASS_RET_ZERO(ret);
        
        elapsed_time = ((double)finish_time.tv_sec + (double)finish_time.tv_usec/1000000) -
                       ((double)start_time.tv_sec + (double)start_time.tv_usec/1000000);
        throughput = (double)size/1024/elapsed_time;
        printf("Read Data Lenth: %dB, elapsed time: %.4fs, throughput: %.4fKB/s\n\n", size, elapsed_time, throughput);
    }
    
    LOG_I("Data Comare, Total Size: %u\n", size);
    printf("Data Comare, Total Size: %u\n\n", size);
    ret = memcmp(write_buf, read_buf, size);
    TEST_PASS_RET_ZERO(ret);

    caClose();
    return 0;  
}

void print_usage(void)
{
    printf("Usage: rpmb_test -t <TYPE>\n"  \
           "   <TYPE> = 1  --> rpmb read/write test\n" \
           "   <TYPE> = 2  --> rpmb performance test\n");
    return;
}

int main(int argc, char *argv[])
{
    int ret;
    const char *type = "";
    int c;

    if (argc == 2 && !strcmp(argv[1], "1"))
    {
        //This is the forked child process
        LOG_I("++++++++ One forked rpmb_test process is running+++++++\n");
        PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_0);
        ret = caOpen();
        PRINT_FUNC_CALL_RET(FUNC_OPEN_STR,ret);
        TEST_PASS_RET_NON_ZERO(ret);

        PRINT_RUN_RPMB_OPEN(RPMB_USER_ID_1);
        ret = caOpen();
        PRINT_FUNC_CALL_RET(FUNC_OPEN_STR,ret);
        TEST_PASS_RET_ZERO(ret);
        caClose();
        return 0;
    }
    
	do {
		c = getopt(argc, argv, "t:?h");
		if (c == EOF)
			break;
		switch (c) {
		case 't':
			type = optarg;
			break;
		case '?':
		case 'h':
            print_usage();
			exit(1);
		}
	} while (1);

    if (strcmp(type,"1") == 0)
    {
        printf("##################  RPMB Test Begin ###################\n");
        //LOG_I("##################  RPMB Test Begin ###################\n");
        //===========Test Open RPMB Session ===========
        //test_case_open();

        //===========Test RPMB Write/Read for USER X===========
        rpmb_write_read_test(RPMB_USER_ID_0);
        rpmb_write_read_test(RPMB_USER_ID_1);
        rpmb_write_read_test(RPMB_USER_ID_2);
        rpmb_write_read_test(RPMB_USER_ID_3);
    }
    else if (strcmp(type,"2") == 0)
    {    
        //===========Test RPMB Write/Read Performance ===========
        rpmb_performance_test();
    }
    else
    {
        print_usage();
        exit(1);
    }

    printf("############# RPMB Test finished, ALL test cases PASS. ##############\n");
    //LOG_I("############# RPMB Test finished, ALL test cases PASS. ##############\n");
    return 0;
}

