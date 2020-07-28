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

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <lib/storage/storage.h>

#include <trusty_unittest.h>
#include <trusty_std.h>

#define LOG_TAG "ss_unittest"
#define TLOGE(fmt, ...) \
    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

typedef void (*test_body)(storage_session_t ss, storage_session_t ss_aux);

void test_wrapper(const char *port, test_body test_proc)
{
    storage_session_t ss;
    storage_session_t ss_aux;

    int rc = storage_open_session(&ss, port);
    if (rc < 0) {
        TLOGE("failed (%d) to open session\n", rc);
        return;
    }

    rc = storage_open_session(&ss_aux, port);
    if (rc < 0) {
        TLOGE("failed (%d) to open session\n", rc);
        storage_close_session(ss);
        return;
    }

    test_proc(ss, ss_aux);
    storage_close_session(ss);
    storage_close_session(ss_aux);
}


#define TEST_P(name) static void test_##name(storage_session_t ss, storage_session_t ss_aux)


#define RUN_TEST_P(port, tn) test_wrapper(port, test_## tn)


#define ASSERT_ALL_OK()  if (!_all_ok) { goto test_abort; }


#define ASSERT_EQ(e, a)    \
do {                       \
    EXPECT_EQ(e, a, "");   \
    ASSERT_ALL_OK();       \
} while(0)


#define ASSERT_NE(e, a)    \
do {                       \
    EXPECT_NE(e, a, "");   \
    ASSERT_ALL_OK();       \
} while(0)



static inline bool is_32bit_aligned(size_t sz)
{
    return ((sz & 0x3) == 0);
}

static inline bool is_valid_size(size_t sz) {
    return (sz > 0) && is_32bit_aligned(sz);
}

static bool is_valid_offset(storage_off_t off)
{
    return (off & 0x3) == 0ULL;
}

static void fill_pattern32(uint32_t *buf, size_t len, storage_off_t off)
{
    size_t cnt = len / sizeof(uint32_t);
    uint32_t pattern = (uint32_t)(off / sizeof(uint32_t));
    while (cnt--) {
        *buf++ = pattern++;
    }
}

static bool check_pattern32(const uint32_t *buf, size_t len, storage_off_t off)
{
    size_t cnt = len / sizeof(uint32_t);
    uint32_t pattern = (uint32_t)(off / sizeof(uint32_t));
    while (cnt--) {
        if (*buf != pattern)
            return false;
        buf++;
        pattern++;
    }
    return true;
}

static bool check_value32(const uint32_t *buf, size_t len, uint32_t val)
{
    size_t cnt = len / sizeof(uint32_t);
    while (cnt--) {
        if (*buf != val)
            return false;
        buf++;
    }
    return true;
}

static int WriteZeroChunk(file_handle_t handle, storage_off_t off,
                          size_t chunk_len, bool complete)
{
    uint32_t data_buf[chunk_len/sizeof(uint32_t)];

    assert(is_valid_size(chunk_len));
    assert(is_valid_offset(off));

    memset(data_buf, 0, chunk_len);

    return storage_write(handle, off, data_buf, sizeof(data_buf),
                         complete ? STORAGE_OP_COMPLETE : 0);
}

static int WritePatternChunk(file_handle_t handle, storage_off_t off,
                             size_t chunk_len, bool complete)
{
    uint32_t data_buf[chunk_len/sizeof(uint32_t)];

    assert(is_valid_size(chunk_len));
    assert(is_valid_offset(off));

    fill_pattern32(data_buf, chunk_len, off);

    return storage_write(handle, off, data_buf, sizeof(data_buf),
                         complete ? STORAGE_OP_COMPLETE : 0);
}

static int WritePattern(file_handle_t handle, storage_off_t off,
                        size_t data_len, size_t chunk_len, bool complete)
{
    size_t written = 0;

    assert(is_valid_size(data_len));
    assert(is_valid_size(chunk_len));

    while (data_len) {
        if (data_len < chunk_len)
            chunk_len = data_len;
        int rc = WritePatternChunk(handle, off, chunk_len, (chunk_len == data_len) && complete);
        if (rc < 0)
            return rc;
        if ((size_t)rc != chunk_len)
            return written + rc;
        off += chunk_len;
        data_len -= chunk_len;
        written += chunk_len;
    }
    return (int)written;
}


static int ReadChunk(file_handle_t handle,
                     storage_off_t off, size_t chunk_len,
                     size_t head_len, size_t pattern_len,
                     size_t tail_len)
{
    int rc;
    uint32_t data_buf[chunk_len/sizeof(uint32_t)];
    uint8_t *data_ptr = (uint8_t *)data_buf;

    assert(is_valid_size(chunk_len));
    assert(is_valid_offset(off));
    assert((head_len + pattern_len + tail_len) == chunk_len);

    rc = storage_read(handle, off, data_buf, chunk_len);
    if ((size_t)rc != chunk_len)
        return rc;

    if (head_len) {
        if (!check_value32((const uint32_t *)data_ptr, head_len, 0))
            return ERR_CHECKSUM_FAIL;
        data_ptr += head_len;
        off += head_len;
    }

    if (pattern_len) {
        if (!check_pattern32((const uint32_t *)data_ptr, pattern_len, off))
            return ERR_CHECKSUM_FAIL;
        data_ptr += pattern_len;
    }

    if (tail_len) {
        if (!check_value32((const uint32_t *)data_ptr, tail_len, 0))
            return ERR_CHECKSUM_FAIL;
    }

    return chunk_len;
}

static int ReadPattern(file_handle_t handle, storage_off_t off,
                       size_t data_len, size_t chunk_len)
{
    int rc;
    size_t bytes_read = 0;
    uint32_t data_buf[chunk_len/sizeof(uint32_t)];

    assert(is_valid_size(chunk_len));
    assert(is_valid_size(data_len));
    assert(is_valid_offset(off));

    while (data_len) {
        if (chunk_len > data_len)
            chunk_len = data_len;
        rc = storage_read(handle, off, data_buf, sizeof(data_buf));
        if (rc < 0)
            return rc;
        if ((size_t)rc != chunk_len)
            return bytes_read + rc;
        if (!check_pattern32(data_buf, chunk_len, off))
            return ERR_CHECKSUM_FAIL;
        off += chunk_len;
        data_len -= chunk_len;
        bytes_read += chunk_len;
    }
    return bytes_read;
}

static int ReadPatternEOF(file_handle_t handle,
                          storage_off_t off, size_t chunk_len)
{
    int rc;
    size_t bytes_read = 0;
    uint32_t data_buf[chunk_len/sizeof(uint32_t)];

    assert(is_valid_size(chunk_len));

    while (true) {
         rc = storage_read(handle, off, data_buf, sizeof(data_buf));
         if (rc < 0)
             return rc;
         if (rc == 0)
             break; // end of file reached
         if (!is_valid_size((size_t)rc))
             return ERR_BAD_LEN;
         if (!check_pattern32(data_buf, rc, off))
             return ERR_CHECKSUM_FAIL;
         off += rc;
         bytes_read += rc;
    }
    return bytes_read;
}



TEST_P(CreateDelete)
{
    int rc;
    file_handle_t handle;
    const char *fname = "test_create_delete_file";

    TEST_BEGIN(__func__);

    // make sure test file does not exist (expect success or ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    EXPECT_EQ(0, rc, "delete test file");
    ASSERT_ALL_OK();

    // one more time (expect ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    EXPECT_EQ(ERR_NOT_FOUND, rc, "delete again");
    ASSERT_ALL_OK();

    // create file (expect 0)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    EXPECT_EQ(0, rc, "create test file");
    ASSERT_ALL_OK();

    // try to create it again while it is still opened (expect ERR_ALREADY_EXISTS)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    EXPECT_EQ(ERR_ALREADY_EXISTS, rc, "create again");
    ASSERT_ALL_OK();

    // close it
    storage_close_file(handle);

    // try to create it again while it is closed (expect ERR_ALREADY_EXISTS)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    EXPECT_EQ(ERR_ALREADY_EXISTS, rc, "create again");
    ASSERT_ALL_OK();

    // delete file (expect 0)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    EXPECT_EQ(0, rc, "delete test file");
    ASSERT_ALL_OK();

    // one more time (expect ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    EXPECT_EQ(ERR_NOT_FOUND, rc, "delete again");
    ASSERT_ALL_OK();

test_abort:
    TEST_END;
}

TEST_P(DeleteOpened)
{
    int rc;
    file_handle_t handle;
    const char *fname = "delete_opened_test_file";

    TEST_BEGIN(__func__);

    // make sure test file does not exist (expect success or ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    ASSERT_EQ(0, rc);

    // one more time (expect ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // open/create file (expect 0)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // delete opened file (expect 0)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // one more time (expect ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // close file
    storage_close_file(handle);

    // one more time (expect ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

test_abort:
    TEST_END;
}


TEST_P(OpenNoCreate)
{
    int rc;
    file_handle_t handle;
    const char *fname = "test_open_no_create_file";

    TEST_BEGIN(__func__);

    // make sure test file does not exist (expect success or ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    ASSERT_EQ(0, rc);

    // open non-existing file (expect ERR_NOT_FOUND)
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // create file (expect 0)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);
    storage_close_file(handle);

    // open existing file (expect 0)
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(0, rc);

    // close it
    storage_close_file(handle);

    // delete file (expect 0)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

test_abort:
    TEST_END;
}

TEST_P(OpenOrCreate)
{
    int rc;
    file_handle_t handle;
    const char *fname = "test_open_create_file";

    TEST_BEGIN(__func__);

    // make sure test file does not exist (expect success or ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    ASSERT_EQ(0, rc);

    // open/create a non-existing file (expect 0)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);
    storage_close_file(handle);

    // open/create an existing file (expect 0)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);
    storage_close_file(handle);

    // delete file (expect 0)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

test_abort:
    TEST_END;
}

TEST_P(OpenCreateDeleteCharset)
{
    int rc;
    file_handle_t handle;
    const char *fname = "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz_01234.56789";

    TEST_BEGIN(__func__);

    // open/create file (expect 0)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);
    storage_close_file(handle);

    // open/create an existing file (expect 0)
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(0, rc);
    storage_close_file(handle);

    // delete file (expect 0)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open again
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

test_abort:
    TEST_END;
}

TEST_P(WriteReadSequential)
{
    int rc;
    size_t blk = 2048;
    file_handle_t handle;
    const char *fname = "test_write_read_sequential";

    TEST_BEGIN(__func__);

    // make sure test file does not exist (expect success or ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    ASSERT_EQ(0, rc);

    // create file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write a bunch of blocks (sequentially)
    rc = WritePattern(handle, 0, 32 * blk, blk, true);
    ASSERT_EQ((int)(32 * blk), rc);

    rc = ReadPattern(handle, 0, 32 * blk, blk);
    ASSERT_EQ((int)(32 * blk), rc);

    // close file
    storage_close_file(handle);

    // open the same file again
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(0, rc);

    // read data back (sequentially) and check pattern again
    rc = ReadPattern(handle, 0, 32 * blk, blk);
    ASSERT_EQ((int)(32 * blk), rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(OpenTruncate)
{
    int rc;
    uint32_t val;
    size_t blk = 2048;
    file_handle_t handle;
    const char *fname = "test_open_truncate";

    TEST_BEGIN(__func__);

    // make sure test file does not exist (expect success or ERR_NOT_FOUND)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    ASSERT_EQ(0, rc);

    // create file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write some data and read it back
    rc = WritePatternChunk(handle, 0, blk, true);
    ASSERT_EQ((int)blk, rc);

    rc = ReadPattern(handle, 0, blk, blk);
    ASSERT_EQ((int)blk, rc);

     // close file
    storage_close_file(handle);

    // reopen with truncate
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_TRUNCATE, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    /* try to read data back (expect no data) */
    rc = storage_read(handle, 0LL, &val, sizeof(val));
    ASSERT_EQ(0, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(OpenSame)
{
    int rc;
    file_handle_t handle1;
    file_handle_t handle2;
    file_handle_t handle3;
    const char *fname = "test_open_same_file";

    TEST_BEGIN(__func__);

    // open/create file (expect 0)
    rc = storage_open_file(ss, &handle1, fname,
                           STORAGE_FILE_OPEN_CREATE, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);
    storage_close_file(handle1);

    // open an existing file first time (expect 0)
    rc = storage_open_file(ss, &handle1, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // open the same file second time (is not allowed)
    rc = storage_open_file(ss, &handle2, fname, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // delete file (expect 0)
    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open deleted file (expect ERR_NOT_FOUND)
    rc = storage_open_file(ss, &handle3, fname, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // close file
    storage_close_file(handle1);

test_abort:
    TEST_END;
}

TEST_P(OpenMany) {
    int rc;
    file_handle_t handles[10];
    char filename[10];
    const char *fname_fmt = "mf%d";

    TEST_BEGIN(__func__);

    // open or create a bunch of files (expect 0)
    for (uint i = 0; i < countof(handles); ++i) {
        snprintf(filename, sizeof(filename), fname_fmt, i);
        rc = storage_open_file(ss, &handles[i], filename,
                               STORAGE_FILE_OPEN_CREATE, STORAGE_OP_COMPLETE);
        ASSERT_EQ(0, rc);
    }

    // check that all handles are different
    for (uint i = 0; i < countof(handles)-1; i++) {
        for (uint j = i+1; j < countof(handles); j++) {
            ASSERT_NE(handles[i], handles[j]);
        }
    }

    // close them all
    for (uint i = 0; i < countof(handles); ++i) {
        storage_close_file(handles[i]);
    }

    // open all files without CREATE flags (expect 0)
    for (uint i = 0; i < countof(handles); ++i) {
        snprintf(filename, sizeof(filename), fname_fmt, i);
        rc = storage_open_file(ss, &handles[i], filename, 0, 0);
        ASSERT_EQ(0, rc);
    }

    // check that all handles are different
    for (uint i = 0; i < countof(handles)-1; i++) {
        for (uint j = i+1; j < countof(handles); j++) {
            ASSERT_NE(handles[i], handles[j]);
        }
    }

    // close and remove all test files
    for (uint i = 0; i < countof(handles); ++i) {
        storage_close_file(handles[i]);
        snprintf(filename, sizeof(filename), fname_fmt, i);
        rc = storage_delete_file(ss, filename, STORAGE_OP_COMPLETE);
        ASSERT_EQ(0, rc);
    }

test_abort:
    TEST_END;
}


TEST_P(ReadAtEOF)
{
    int rc;
    uint32_t val;
    size_t blk = 2048;
    file_handle_t handle;
    const char *fname = "test_read_eof";

    TEST_BEGIN(__func__);

    // open/create/truncate file
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write block at offset 0
    rc = WritePatternChunk(handle, 0, blk, true);
    ASSERT_EQ((int)blk, rc);

    // close file
    storage_close_file(handle);

    // open same file again
    rc = storage_open_file(ss, &handle, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // read the whole block back and check pattern again
    rc = ReadPattern(handle, 0, blk, blk);
    ASSERT_EQ((int)blk, rc);

    // read at end of file (expected 0 bytes)
    rc = storage_read(handle, blk, &val, sizeof(val));
    ASSERT_EQ(0, rc);

    // partial read at end of the file (expected partial data)
    rc = ReadPatternEOF(handle, blk/2, blk);
    ASSERT_EQ((int)blk/2, rc);

    // read past end of file
    rc = storage_read(handle, blk + 2, &val, sizeof(val));
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(GetFileSize)
{
    int rc;
    size_t blk = 2048;
    storage_off_t size;
    file_handle_t handle;
    const char *fname = "test_get_file_size";

    TEST_BEGIN(__func__);

    // open/create/truncate file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // check file size (expect success and size == 0)
    size = 1;
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, size);

    // write block
    rc = WritePatternChunk(handle, 0, blk, true);
    ASSERT_EQ((int)blk, rc);

    // check size
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(blk, size);

    // write another block
    rc = WritePatternChunk(handle, blk, blk, true);
    ASSERT_EQ((int)blk, rc);

    // check size again
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(blk*2, size);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(SetFileSize)
{
    int rc;
    size_t blk = 2048;
    storage_off_t size;
    file_handle_t handle;
    const char *fname = "test_set_file_size";

    TEST_BEGIN(__func__);

    // open/create/truncate file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // check file size (expect success and size == 0)
    size = 1;
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, size);

    // write block
    rc = WritePatternChunk(handle, 0, blk, true);
    ASSERT_EQ((int)blk, rc);

    // check size
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(blk, size);

    storage_close_file(handle);

    // reopen normally
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(0, rc);

    // check size again
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(blk, size);

    // set file size to half
    rc = storage_set_file_size(handle, blk/2, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // check size again (should be half of original size)
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(blk/2, size);

    // read data back
    rc = ReadPatternEOF(handle, 0, blk);
    ASSERT_EQ((int)blk/2, rc);

    // set file size to 0
    rc = storage_set_file_size(handle, 0, STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // check size again (should be 0)
    rc = storage_get_file_size(handle, &size);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0LL, size);

    // try to read again
    rc = ReadPatternEOF(handle, 0, blk);
    ASSERT_EQ(0, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(WriteReadAtOffset) {
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t blk_cnt = 32;
    const char *fname = "test_write_at_offset";

    TEST_BEGIN(__func__);

    // create/truncate file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write a bunch of blocks filled with zeroes
    for (uint i = 0; i < blk_cnt; i++) {
        rc = WriteZeroChunk(handle, i * blk, blk, true);
        ASSERT_EQ((int)blk, rc);
    }

    storage_off_t off1 = blk;
    storage_off_t off2 = blk * (blk_cnt-1);

    // write known pattern data at non-zero offset1
    rc = WritePatternChunk(handle, off1, blk, true);
    ASSERT_EQ((int)blk, rc);

    // write known pattern data at non-zero offset2
    rc = WritePatternChunk(handle, off2, blk, true);
    ASSERT_EQ((int)blk, rc);

    // read data back at offset1
    rc = ReadPattern(handle, off1, blk, blk);
    ASSERT_EQ((int)blk, rc);

    // read data back at offset2
    rc = ReadPattern(handle, off2, blk, blk);
    ASSERT_EQ((int)blk, rc);

    // read partially written data at end of file(expect to get data only, no padding)
    rc = ReadPatternEOF(handle, off2 + blk/2, blk);
    ASSERT_EQ((int)blk/2, rc);

    // read data at offset 0 (expect success and zero data)
    rc = ReadChunk(handle, 0, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    // read data from gap (expect success and zero data)
    rc = ReadChunk(handle, off1 + blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    // read partially written data (start pointing within written data)
    // (expect to get written data back and zeroes at the end)
    rc = ReadChunk(handle, off1 + blk/2, blk, 0, blk/2, blk/2);
    ASSERT_EQ((int)blk, rc);

    // read partially written data (start pointing withing unwritten data)
    // expect to get zeroes at the beginning and proper data at the end
    rc = ReadChunk(handle, off1 - blk/2, blk, blk/2, blk/2, 0);
    ASSERT_EQ((int)blk, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(WriteSparse)
{
    int rc;
    file_handle_t handle;
    const char *fname = "test_write_sparse";

    TEST_BEGIN(__func__);

    // open/create/truncate file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write value past en of file
    uint32_t val = 0xDEADBEEF;
    rc = storage_write(handle, 1, &val, sizeof(val), STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(CreatePersistent32K)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t file_size = 32768;
    const char *fname = "test_persistent_32K_file";

    TEST_BEGIN(__func__);

    // create/truncate file.
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write a bunch of blocks filled with pattern
    rc = WritePattern(handle, 0, file_size, blk, true);
    ASSERT_EQ((int)file_size, rc);

    // close but do not delete file
    storage_close_file(handle);

test_abort:
    TEST_END;
}

TEST_P(ReadPersistent32k)
{
    int rc;
    file_handle_t handle;
    size_t exp_len = 32 * 1024;
    const char *fname = "test_persistent_32K_file";

    TEST_BEGIN(__func__);

    // create/truncate file.
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(0, rc);

    rc = ReadPatternEOF(handle, 0, 2048);
    ASSERT_EQ((int)exp_len, rc);

    rc = ReadPatternEOF(handle, 0, 1024);
    ASSERT_EQ((int)exp_len, rc);

    rc = ReadPatternEOF(handle, 0,  332);
    ASSERT_EQ((int)exp_len, rc);

    // close but do not delete file
    storage_close_file(handle);

test_abort:
    TEST_END;
}


TEST_P(CleanUpPersistent32K)
{
    int rc;
    const char *fname = "test_persistent_32K_file";

    TEST_BEGIN(__func__);

    rc = storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);
    rc = (rc == ERR_NOT_FOUND) ? 0 : rc;
    ASSERT_EQ(0, rc);

test_abort:
    TEST_END;
}

TEST_P(WriteReadLong)
{
    int rc;
    file_handle_t handle;
    size_t wc = 10000;
    const char *fname = "test_write_read_long";

    TEST_BEGIN(__func__);

    uint32_t *test_buf_ = malloc(wc * sizeof(uint32_t));
    ASSERT_NE(NULL, test_buf_);

    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    fill_pattern32(test_buf_, wc * sizeof(uint32_t), 0);
    rc = storage_write(handle, 0, test_buf_, wc * sizeof(uint32_t), STORAGE_OP_COMPLETE);
    ASSERT_EQ((int)(wc * sizeof(uint32_t)), rc);

    rc = storage_read(handle, 0, test_buf_, wc * sizeof(uint32_t));
    ASSERT_EQ((int)(wc * sizeof(uint32_t)), rc);

    bool res = check_pattern32(test_buf_, wc * sizeof(uint32_t), 0);
    ASSERT_EQ(true, res);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    if (test_buf_)
        free(test_buf_);
    TEST_END;
}


// Negative tests

TEST_P(OpenInvalidFileName)
{
    int rc;
    file_handle_t handle;
    const char *fname1 = "";
    const char *fname2 = "ffff$ffff";
    const char *fname3 = "ffff\\ffff";
    char max_name[STORAGE_MAX_NAME_LENGTH_BYTES+1];

    TEST_BEGIN(__func__);

    rc = storage_open_file(ss, &handle, fname1,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    rc = storage_open_file(ss, &handle, fname2,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    rc = storage_open_file(ss, &handle, fname3,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    /* max name */
    memset(max_name, 'a', sizeof(max_name));
    max_name[sizeof(max_name)-1] = 0;

    rc = storage_open_file(ss, &handle, max_name,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    max_name[sizeof(max_name)-2] = 0;
    rc = storage_open_file(ss, &handle, max_name,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    storage_close_file(handle);
    storage_delete_file(ss, max_name, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(BadFileHandle)
{
    int rc;
    file_handle_t handle;
    const char *fname = "test_invalid_file_handle";

    TEST_BEGIN(__func__);

    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write to invalid file handle
    uint32_t val = 0xDEDBEEF;
    rc = storage_write(handle + 1,  0, &val, sizeof(val), STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // read from invalid handle
    rc = storage_read(handle + 1,  0, &val, sizeof(val));
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // set size
    rc = storage_set_file_size(handle + 1,  0, STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // get size
    storage_off_t fsize = (storage_off_t)(-1);
    rc = storage_get_file_size(handle + 1,  &fsize);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // close (there is no way to check errors here)
    storage_close_file(handle + 1);

    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(ClosedFileHandle)
{
    int rc;
    file_handle_t handle1;
    file_handle_t handle2;
    const char *fname1 = "test_invalid_file_handle1";
    const char *fname2 = "test_invalid_file_handle2";

    TEST_BEGIN(__func__);

    rc = storage_open_file(ss, &handle1, fname1,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    rc = storage_open_file(ss, &handle2, fname2,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // close first file handle
    storage_close_file(handle1);

    // write to invalid file handle
    uint32_t val = 0xDEDBEEF;
    rc = storage_write(handle1,  0, &val, sizeof(val), STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // read from invalid handle
    rc = storage_read(handle1,  0, &val, sizeof(val));
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // set size
    rc = storage_set_file_size(handle1,  0, STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // get size
    storage_off_t fsize = (storage_off_t)(-1);
    rc = storage_get_file_size(handle1,  &fsize);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // close (there is no way to check errors here)
    storage_close_file(handle1);

    // clean up
    storage_close_file(handle2);
    storage_delete_file(ss, fname1, STORAGE_OP_COMPLETE);
    storage_delete_file(ss, fname2, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactDiscardInactive)
{
    int rc;

    TEST_BEGIN(__func__);

    // discard current transaction (there should not be any)
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // try it again
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

test_abort:
    TEST_END;
}

TEST_P(TransactCommitInactive)
{
    int rc;

    TEST_BEGIN(__func__);

    // try to commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // try it again
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardWrite)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_discard_write";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // write (without commit)
    rc = WritePattern(handle, 0, exp_len, blk, false);
    ASSERT_EQ((int)exp_len, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // abort current transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardWriteAppend)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_write_append";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data with commit
    rc = WritePattern(handle, 0, exp_len/2, blk, true);
    ASSERT_EQ((int)exp_len/2, rc);

    // write data without commit
    rc = WritePattern(handle, exp_len/2, exp_len/2, blk, false);
    ASSERT_EQ((int)exp_len/2, rc);

    // check file size (should be exp_len)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // discard transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // check file size, it should be exp_len/2
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/2, fsize);

    // check file data
    rc = ReadPatternEOF(handle, 0, blk);
    ASSERT_EQ((int)exp_len/2, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardWriteRead)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_discard_write_read";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // Fill with zeroes (with commit)
    for (uint i = 0; i < 32; i++) {
        rc = WriteZeroChunk(handle, i * blk, blk, true);
        ASSERT_EQ((int)blk, rc);
    }

    // check that test chunk is filled with zeroes
    rc = ReadChunk(handle, blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    // write test pattern (without commit)
    rc = WritePattern(handle, blk, blk, blk, false);
    ASSERT_EQ((int)blk, rc);

    // read it back an check pattern
    rc = ReadChunk(handle, blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    // abort current transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // read same chunk back (should be filled with zeros)
    rc = ReadChunk(handle, blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactDiscardWriteMany)
{
    int rc;
    file_handle_t handle1;
    file_handle_t handle2;
    size_t blk = 2048;
    size_t exp_len1 = 32 * 1024;
    size_t exp_len2 = 31 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname1 = "test_transact_discard_write_file1";
    const char *fname2 = "test_transact_discard_write_file2";

    TEST_BEGIN(__func__);

    // open create truncate (with commit)
    rc = storage_open_file(ss, &handle1, fname1,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open create truncate (with commit)
    rc = storage_open_file(ss, &handle2, fname2,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // file1: fill file with pattern (without commit)
    rc = WritePattern(handle1, 0, exp_len1, blk, false);
    ASSERT_EQ((int)exp_len1, rc);

    // file2: fill file with pattern (without commit)
    rc = WritePattern(handle2, 0, exp_len2, blk, false);
    ASSERT_EQ((int)exp_len2, rc);

    // check file size, it should be exp_len1
    rc = storage_get_file_size(handle1, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len1, fsize);

    // check file size, it should be exp_len2
    rc = storage_get_file_size(handle2, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len2, fsize);

    // discard transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // check file size, it should be 0
    rc = storage_get_file_size(handle1, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // check file size, it should be 0
    rc = storage_get_file_size(handle2, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // check data
    rc = ReadPatternEOF(handle1, 0, blk);
    ASSERT_EQ(0, rc);

    rc = ReadPatternEOF(handle2, 0, blk);
    ASSERT_EQ(0, rc);

    // cleanup
    storage_close_file(handle1);
    storage_delete_file(ss, fname1, STORAGE_OP_COMPLETE);
    storage_close_file(handle2);
    storage_delete_file(ss, fname2, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardTruncate)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_discard_truncate";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // close file
    storage_close_file(handle);

    // open truncate file (without commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_TRUNCATE, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // abort current transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // check file size (should be an oruginal size)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardSetSize)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_discard_set_size";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // set file size to half of original (no commit)
    rc = storage_set_file_size(handle,  (storage_off_t)exp_len/2, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/2, fsize);

    // set file size to 1/3 of original (no commit)
    rc = storage_set_file_size(handle,  (storage_off_t)exp_len/3, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/3, fsize);

    // abort current transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // check file size (should be an original size)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardDelete)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_discard_delete";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // close it
    storage_close_file(handle);

    // delete file (without commit)
    rc = storage_delete_file(ss, fname, 0);
    ASSERT_EQ(0, rc);

    // try to open it (should fail)
    rc = storage_open_file(ss, &handle, fname, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // abort current transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // try to open it
    rc = storage_open_file(ss, &handle, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // check file size (should be an original size)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactDiscardCreate)
{
    int rc;
    file_handle_t handle;
    const char *fname = "test_transact_discard_create_excl";

    TEST_BEGIN(__func__);

    // delete test file just in case
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

    // create file (without commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           0);
    ASSERT_EQ(0, rc);

    // close it
    storage_close_file(handle);

    // open it again without create and without commit (expect success)
    rc = storage_open_file(ss, &handle, fname, 0,  0);
    ASSERT_EQ(0, rc);

    // close it
    storage_close_file(handle);

    // abort current transaction
    rc = storage_end_transaction(ss, false);
    ASSERT_EQ(0, rc);

    // open it again without create without commit (expect not found)
    rc = storage_open_file(ss, &handle, fname, 0,  0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // cleanup
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactCommitWrites)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_commit_writes";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open the same file in aux session
    rc = storage_open_file(ss_aux, &handle_aux, fname,  0, 0);
    ASSERT_EQ(0, rc);

    // check file size, it should be 0
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // write data in primary session (without commit)
    rc = WritePattern(handle, 0, exp_len/2, blk, false);
    ASSERT_EQ((int)exp_len/2, rc);

    // write more data in primary (without commit)
    rc = WritePattern(handle, exp_len/2, exp_len/2, blk, false);
    ASSERT_EQ((int)exp_len/2, rc);

    // check file size in aux session, it should still be 0
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check file size of aux session, should fail
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(ERR_BUSY, rc);

    // abort transaction in aux session to recover
    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    // check file size in aux session, it should be exp_len
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // check file size in primary session, it should be exp_len
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // check data in primary session
    rc = ReadPatternEOF(handle, 0, blk);
    ASSERT_EQ((int)exp_len, rc);

    // check data in aux session
    rc = ReadPatternEOF(handle_aux, 0, blk);
    ASSERT_EQ((int)exp_len, rc);

    // cleanup
    storage_close_file(handle);
    storage_close_file(handle_aux);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactCommitWrites2)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    size_t blk = 2048;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_commit_writes2";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open the same file in separate session
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // discard transaction in aux_session
    rc = storage_end_transaction(ss_aux,  false);
    ASSERT_EQ(0, rc);

    // Fill with zeroes (with commit)
    for (uint i = 0; i < 8; i++) {
        rc = WriteZeroChunk(handle, i * blk, blk, true);
        ASSERT_EQ((int)blk, rc);
    }

    // check that test chunks is filled with zeroes
    rc = ReadChunk(handle, blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    rc = ReadChunk(handle, 2 * blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    // write test pattern (without commit)
    rc = WritePattern(handle, blk, blk, blk, false);
    ASSERT_EQ((int)blk, rc);

    // write test pattern (without commit)
    rc = WritePattern(handle, 2 * blk, blk, blk, false);
    ASSERT_EQ((int)blk, rc);

    // read it back and check pattern
    rc = ReadChunk(handle, blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    rc = ReadChunk(handle, 2 * blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    // In aux session it still should be empty
    rc = ReadChunk(handle_aux, blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    rc = ReadChunk(handle_aux, 2 * blk, blk, blk, 0, 0);
    ASSERT_EQ((int)blk, rc);

    // commit current transaction
    rc = storage_end_transaction(ss,  true);
    ASSERT_EQ(0, rc);

    // read same chunk back in primary session
    rc = ReadChunk(handle, blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    rc = ReadChunk(handle, 2 * blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    // read same chunk back in aux session
    rc = ReadChunk(handle_aux, blk, blk, 0, blk, 0);
    ASSERT_EQ(ERR_BUSY, rc);

    rc = ReadChunk(handle_aux, 2 * blk, blk, 0, blk, 0);
    ASSERT_EQ(ERR_BUSY, rc);

    // abort transaction in aux session
    rc = storage_end_transaction(ss_aux,  false);
    ASSERT_EQ(0, rc);

    // read same chunk again in aux session
    rc = ReadChunk(handle_aux, blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    rc = ReadChunk(handle_aux, 2 * blk, blk, 0, blk, 0);
    ASSERT_EQ((int)blk, rc);

    // cleanup
    storage_close_file(handle);
    storage_close_file(handle_aux);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactCommitSetSize)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_commit_set_size";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open the same file in separate session
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // same in aux session
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // set file size to half of original (no commit)
    rc = storage_set_file_size(handle,  (storage_off_t)exp_len/2, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/2, fsize);

    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // set file size to 1/3 of original (no commit)
    rc = storage_set_file_size(handle,  (storage_off_t)exp_len/3, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/3, fsize);

    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check file size (should be 1/3 of an original size)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/3, fsize);

    // check file size from aux session
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(ERR_BUSY, rc);

    // abort transaction
    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    // check again
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/3, fsize);

    // write data, increasing file size to exp_len (no commit)
    rc = WritePattern(handle, 0, exp_len, blk, false);
    ASSERT_EQ((int)exp_len, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // abort aux transaction
    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    // check file size from aux session
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/3, fsize);

    // set file size without actually changing size, but ask to commit
    rc = storage_set_file_size(handle, (storage_off_t)exp_len,
                               STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // abort aux transaction
    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    // check file size from aux session
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // cleanup
    storage_close_file(handle);
    storage_close_file(handle_aux);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactCommitDelete)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    const char *fname = "test_transact_commit_delete";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // close it
    storage_close_file(handle);

    // open the same file in separate session
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);
    storage_close_file(handle_aux);

    // delete file (without commit)
    rc = storage_delete_file(ss, fname, 0);
    ASSERT_EQ(0, rc);

    // try to open it (should fail)
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // open the same file in separate session (should be fine)
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);
    storage_close_file(handle_aux);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // try to open it (still fails)
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // open the same file in separate session (should fail)
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

test_abort:
    TEST_END;
}


TEST_P(TransactCommitTruncate)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_commit_truncate";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // close file
    storage_close_file(handle);

    // check from different session
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);

    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);


    // open truncate file (without commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_TRUNCATE, 0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check file size (should be 0)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // check file size in aux session (should be ERR_BUSY)
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(ERR_BUSY, rc);

    // abort transaction in aux session
    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    // check again
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // cleanup
    storage_close_file(handle);
    storage_close_file(handle_aux);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactCommitCreate)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_commit_create";

    TEST_BEGIN(__func__);

    // delete test file just in case
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

    // check from different session
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // create file (without commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           0);
    ASSERT_EQ(0, rc);

    // check file size
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // close file
    storage_close_file(handle);

    // check from aux session (should fail)
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check open from normal session
    rc = storage_open_file(ss, &handle, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // check open from aux session (should succeed)
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // cleanup
    storage_close_file(handle);
    storage_close_file(handle_aux);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactCommitCreateMany)
{
    int rc;
    file_handle_t handle1;
    file_handle_t handle2;
    file_handle_t handle1_aux;
    file_handle_t handle2_aux;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname1 = "test_transact_commit_create1";
    const char *fname2 = "test_transact_commit_create2";

    TEST_BEGIN(__func__);

    // delete test file just in case
    storage_delete_file(ss, fname1, STORAGE_OP_COMPLETE);
    storage_delete_file(ss, fname2, STORAGE_OP_COMPLETE);

    // create file (without commit)
    rc = storage_open_file(ss, &handle1, fname1,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           0);
    ASSERT_EQ(0, rc);

    // create file (without commit)
    rc = storage_open_file(ss, &handle2, fname2,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           0);
    ASSERT_EQ(0, rc);

    // check file sizes
    rc = storage_get_file_size(handle1, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    rc = storage_get_file_size(handle1, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // close files
    storage_close_file(handle1);
    storage_close_file(handle2);

    rc = storage_open_file(ss_aux, &handle1_aux, fname1, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    rc = storage_open_file(ss_aux, &handle2_aux, fname2, 0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // open from primary session
    rc = storage_open_file(ss, &handle1, fname1, 0, 0);
    ASSERT_EQ(0, rc);

    rc = storage_open_file(ss, &handle2, fname2, 0, 0);
    ASSERT_EQ(0, rc);

    // open from aux session
    rc = storage_open_file(ss_aux, &handle1_aux, fname1, 0, 0);
    ASSERT_EQ(0, rc);

    rc = storage_open_file(ss_aux, &handle2_aux, fname2, 0, 0);
    ASSERT_EQ(0, rc);

    // cleanup
    storage_close_file(handle1);
    storage_close_file(handle1_aux);
    storage_delete_file(ss, fname1, STORAGE_OP_COMPLETE);
    storage_close_file(handle2);
    storage_close_file(handle2_aux);
    storage_delete_file(ss, fname2, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactCommitWriteMany)
{
    int rc;
    file_handle_t handle1;
    file_handle_t handle2;
    file_handle_t handle1_aux;
    file_handle_t handle2_aux;
    size_t blk = 2048;
    size_t exp_len1 = 32 * 1024;
    size_t exp_len2 = 31 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname1 = "test_transact_commit_write_file1";
    const char *fname2 = "test_transact_commit_write_file2";

    TEST_BEGIN(__func__);

    // open create truncate (with commit)
    rc = storage_open_file(ss, &handle1, fname1,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // open create truncate (with commit)
    rc = storage_open_file(ss, &handle2, fname2,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);


    // open same files from aux session
    rc = storage_open_file(ss_aux, &handle1_aux, fname1, 0, 0);
    ASSERT_EQ(0, rc);

    rc = storage_open_file(ss_aux, &handle2_aux, fname2, 0, 0);
    ASSERT_EQ(0, rc);

    // file1: fill file with pattern (without commit)
    rc = WritePattern(handle1, 0, exp_len1, blk, false);
    ASSERT_EQ((int)exp_len1, rc);

    // file2: fill file with pattern (without commit)
    rc = WritePattern(handle2, 0, exp_len2, blk, false);
    ASSERT_EQ((int)exp_len2, rc);

    // check file size, it should be exp_len1
    rc = storage_get_file_size(handle1, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len1, fsize);

    // check file size, it should be exp_len2
    rc = storage_get_file_size(handle2, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len2, fsize);

    // check file sizes from aux session (should be 0)
    rc = storage_get_file_size(handle1_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    rc = storage_get_file_size(handle2_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)0, fsize);

    // commit transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check file size, it should be exp_len1
    rc = storage_get_file_size(handle1, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len1, fsize);

    // check file size, it should be exp_len2
    rc = storage_get_file_size(handle2, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len2, fsize);

    // from aux session
    rc = storage_get_file_size(handle1_aux, &fsize);
    ASSERT_EQ(ERR_BUSY, rc);

    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    rc = storage_get_file_size(handle1_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len1, fsize);

    rc = storage_get_file_size(handle2_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len2, fsize);

    // check data
    rc = ReadPatternEOF(handle1, 0, blk);
    ASSERT_EQ((int)exp_len1, rc);

    rc = ReadPatternEOF(handle2, 0, blk);
    ASSERT_EQ((int)exp_len2, rc);

    rc = ReadPatternEOF(handle1_aux, 0, blk);
    ASSERT_EQ((int)exp_len1, rc);

    rc = ReadPatternEOF(handle2_aux, 0, blk);
    ASSERT_EQ((int)exp_len2, rc);

    // cleanup
    storage_close_file(handle1);
    storage_close_file(handle1_aux);
    storage_delete_file(ss, fname1, STORAGE_OP_COMPLETE);
    storage_close_file(handle2);
    storage_close_file(handle2_aux);
    storage_delete_file(ss, fname2, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactCommitDeleteCreate)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle_aux;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_delete_create";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write data (with commit)
    rc = WritePattern(handle, 0, exp_len, blk, true);
    ASSERT_EQ((int)exp_len, rc);

    // close it
    storage_close_file(handle);

    // delete file (without commit)
    rc = storage_delete_file(ss, fname, 0);
    ASSERT_EQ(0, rc);

    // try to open it (should fail)
    rc = storage_open_file(ss, &handle, fname,  0, 0);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // try to open it in aux session (should succeed)
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);

    // create file with the same name (no commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_CREATE_EXCLUSIVE,
                           0);
    ASSERT_EQ(0, rc);

    // write half of data (with commit)
    rc = WritePattern(handle, 0, exp_len/2, blk, true);
    ASSERT_EQ((int)exp_len/2, rc);

    // check file size (should be half)
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/2, fsize);

    // commit transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check data
    rc = ReadPatternEOF(handle, 0, blk);
    ASSERT_EQ((int)exp_len/2, rc);

    // check from aux session
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // abort trunsaction
    rc = storage_end_transaction(ss_aux, false);
    ASSERT_EQ(0, rc);

    // and try again
    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // close file and reopen it again
    storage_close_file(handle_aux);
    rc = storage_open_file(ss_aux, &handle_aux, fname, 0, 0);
    ASSERT_EQ(0, rc);

    rc = storage_get_file_size(handle_aux, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len/2, fsize);

    rc = ReadPatternEOF(handle_aux, 0, blk);
    ASSERT_EQ((int)exp_len/2, rc);

    // cleanup
    storage_close_file(handle);
    storage_close_file(handle_aux);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(TransactRewriteExistingTruncate)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    const char *fname = "test_transact_rewrite_existing_truncate";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // close it
    storage_close_file(handle);

    // up
    for (uint i = 1; i < 32; i++) {
        // open truncate (no commit)
        rc = storage_open_file(ss, &handle, fname, STORAGE_FILE_OPEN_TRUNCATE, 0);
        ASSERT_EQ(0, rc);

        // write data (with commit)
        rc = WritePattern(handle, 0, i * blk, blk, true);
        ASSERT_EQ((int)(i * blk), rc);

        // close
        storage_close_file(handle);
    }

    // down
    for (uint i = 1; i < 32; i++) {
        // open truncate (no commit)
        rc = storage_open_file(ss, &handle, fname, STORAGE_FILE_OPEN_TRUNCATE, 0);
        ASSERT_EQ(0, rc);

        // write data (with commit)
        rc = WritePattern(handle, 0, (32 - i) * blk, blk, true);
        ASSERT_EQ((int)((32 - i) * blk), rc);

        // close
        storage_close_file(handle);
    }

    // cleanup
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactRewriteExistingSetSize)
{
    int rc;
    file_handle_t handle;
    size_t blk = 2048;
    const char *fname = "test_transact_rewrite_existing_set_size";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // close it
    storage_close_file(handle);

    // up
    for (uint i = 1; i < 32; i++) {
        // open truncate (no commit)
        rc = storage_open_file(ss, &handle, fname, 0, 0);
        ASSERT_EQ(0, rc);

        // write data (with commit)
        rc = WritePattern(handle, 0, i * blk, blk, false);
        ASSERT_EQ((int)(i * blk), rc);

        // update size (with commit)
        rc = storage_set_file_size(handle, i * blk, STORAGE_OP_COMPLETE);
        ASSERT_EQ(0, rc);

        // close
        storage_close_file(handle);
    }

    // down
    for (uint i = 1; i < 32; i++) {
        // open trancate (no commit)
        rc = storage_open_file(ss, &handle, fname, 0, 0);
        ASSERT_EQ(0, rc);

        // write data (with commit)
        rc = WritePattern(handle, 0, (32 - i) * blk, blk, false);
        ASSERT_EQ((int)((32 - i) * blk), rc);

        // update size (with commit)
        rc = storage_set_file_size(handle, (32 - i) * blk, STORAGE_OP_COMPLETE);
        ASSERT_EQ(0, rc);

        // close
        storage_close_file(handle);
    }

    // cleanup
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}


TEST_P(TransactResumeAfterNonFatalError)
{
    int rc;
    file_handle_t handle;
    file_handle_t handle1;
    size_t blk = 2048;
    size_t exp_len = 32 * 1024;
    storage_off_t fsize = (storage_off_t)(-1);
    const char *fname = "test_transact_resume_writes";

    TEST_BEGIN(__func__);

    // open create truncate file (with commit)
    rc = storage_open_file(ss, &handle, fname,
                           STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
    ASSERT_EQ(0, rc);

    // write (without commit)
    rc = WritePattern(handle, 0, exp_len/2, blk, false);
    ASSERT_EQ((int)exp_len/2, rc);

    // issue some commands that should fail with non-fatal errors

    // write past end of file
    uint32_t val = 0xDEADBEEF;
    rc = storage_write(handle,  exp_len/2 + 1, &val, sizeof(val), 0);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // read past end of file
    rc = storage_read(handle, exp_len/2 + 1, &val, sizeof(val));
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // try to extend file past end of file
    rc = storage_set_file_size(handle, exp_len/2 + 1, 0);
    ASSERT_EQ(ERR_NOT_VALID, rc);

    // open non existing file
    rc = storage_open_file(ss, &handle1, "foo",
                           STORAGE_FILE_OPEN_TRUNCATE, STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // delete non-existing file
    rc = storage_delete_file(ss, "foo", STORAGE_OP_COMPLETE);
    ASSERT_EQ(ERR_NOT_FOUND, rc);

    // then resume writinga (without commit)
    rc = WritePattern(handle, exp_len/2, exp_len/2, blk, false);
    ASSERT_EQ((int)exp_len/2, rc);

    // commit current transaction
    rc = storage_end_transaction(ss, true);
    ASSERT_EQ(0, rc);

    // check file size, it should be exp_len
    rc = storage_get_file_size(handle, &fsize);
    ASSERT_EQ(0, rc);
    ASSERT_EQ((storage_off_t)exp_len, fsize);

    // check data
    rc = ReadPatternEOF(handle, 0, blk);
    ASSERT_EQ((int)exp_len, rc);

    // cleanup
    storage_close_file(handle);
    storage_delete_file(ss, fname, STORAGE_OP_COMPLETE);

test_abort:
    TEST_END;
}

TEST_P(ReadRpmbRawBlock)
{
    int rc;
    uint32_t blk_addr;
    uint32_t blk_num;

    const uint32_t rpmb_buf_sz = 256;

    TEST_BEGIN(__func__);

    uint8_t buf[4096];

    blk_addr = 0;
    blk_num = 0;
    rc = storage_rpmb_read(ss, blk_addr, blk_num, buf, sizeof(buf));
    ASSERT_EQ(0, rc);

    blk_addr = 0;
    blk_num = 16;
    rc = storage_rpmb_read(ss, blk_addr, blk_num, buf, sizeof(buf));
    ASSERT_EQ(blk_num * rpmb_buf_sz, rc);

    // negative test
    blk_addr = UINT_MAX;
    blk_num  = 1;
    rc = storage_rpmb_read(ss, blk_addr, blk_num, buf, sizeof(buf));
    ASSERT_EQ(ERR_NOT_VALID, rc);

    blk_addr = 0;
    blk_num = UINT_MAX;
    rc = storage_rpmb_read(ss, blk_addr, blk_num, buf, sizeof(buf));
    ASSERT_EQ(ERR_NOT_ENOUGH_BUFFER, rc);

    blk_addr = UINT_MAX;
    blk_num = UINT_MAX;
    rc = storage_rpmb_read(ss, blk_addr, blk_num, buf, sizeof(buf));
    ASSERT_EQ(ERR_NOT_ENOUGH_BUFFER, rc);

test_abort:
    TEST_END;
}

void run_all_tests(const char *port)
{
    int rc;
    storage_session_t session;

    TLOGI("SS-unittest: %s: waiting for server\n", port);
    do {
        rc = storage_open_session(&session, port);
        if (rc < 0) {
            TLOGE("failed (%d) to connect to storage server - retrying\n", rc);
        }
        nanosleep(0, 0, 1000000);
    } while (rc < 0);
    storage_close_session(session);

    TLOGI("SS-unittest: %s: begins\n", port);

    RUN_TEST_P(port, CreateDelete);
    RUN_TEST_P(port, DeleteOpened);
    RUN_TEST_P(port, OpenNoCreate);
    RUN_TEST_P(port, OpenOrCreate);
    RUN_TEST_P(port, OpenCreateDeleteCharset);
    RUN_TEST_P(port, WriteReadSequential);
    RUN_TEST_P(port, OpenTruncate);
    RUN_TEST_P(port, OpenSame);
    RUN_TEST_P(port, OpenMany);
    RUN_TEST_P(port, ReadAtEOF);
    RUN_TEST_P(port, GetFileSize);
    RUN_TEST_P(port, SetFileSize);
    RUN_TEST_P(port, WriteReadAtOffset);
    RUN_TEST_P(port, WriteSparse);
    RUN_TEST_P(port, CreatePersistent32K);
    RUN_TEST_P(port, ReadPersistent32k);
    RUN_TEST_P(port, CleanUpPersistent32K);
    RUN_TEST_P(port, WriteReadLong);
    RUN_TEST_P(port, OpenInvalidFileName);
    RUN_TEST_P(port, BadFileHandle);
    RUN_TEST_P(port, ClosedFileHandle);

    // Transaction tests
    RUN_TEST_P(port, TransactDiscardInactive);
    RUN_TEST_P(port, TransactCommitInactive);
    RUN_TEST_P(port, TransactDiscardWrite);
    RUN_TEST_P(port, TransactDiscardWriteAppend);
    RUN_TEST_P(port, TransactDiscardWriteRead);
    RUN_TEST_P(port, TransactDiscardWriteMany);
    RUN_TEST_P(port, TransactDiscardTruncate);
    RUN_TEST_P(port, TransactDiscardSetSize);
    RUN_TEST_P(port, TransactDiscardDelete);
    RUN_TEST_P(port, TransactDiscardCreate);

    RUN_TEST_P(port, TransactCommitWrites);
    RUN_TEST_P(port, TransactCommitWrites2);
    RUN_TEST_P(port, TransactCommitSetSize);
    RUN_TEST_P(port, TransactCommitDelete);
    RUN_TEST_P(port, TransactCommitTruncate);
    RUN_TEST_P(port, TransactCommitCreate);
    RUN_TEST_P(port, TransactCommitCreateMany);
    RUN_TEST_P(port, TransactCommitWriteMany);
    RUN_TEST_P(port, TransactCommitDeleteCreate);
    RUN_TEST_P(port, TransactRewriteExistingTruncate);
    RUN_TEST_P(port, TransactRewriteExistingSetSize);
    RUN_TEST_P(port, TransactResumeAfterNonFatalError);
    RUN_TEST_P(port, ReadRpmbRawBlock);

    TLOGI("SS-unittest: %s: ends\n", port);
}

#define MAX_PORT_BUF_SZ 512
#define MAX_MESG_BUF_SZ  64

#ifndef __NEBULA_HEE__
#define SS_UT_SRV_NAME "com.mediatek.geniezone.srv.ss-ut"
#else
#define SS_UT_SRV_NAME "nebula.com.mediatek.geniezone.srv.ss-ut"
#endif

static void handle_event_port(const uevent_t *ev)
{
    int rc;
    uuid_t peer_uuid;

    if ((ev->event & IPC_HANDLE_POLL_READY) == 0) {
        TLOGI("SS-unittest: unexpected event for handle %d\n", ev->handle);
        return;
    }

    rc = accept(ev->handle, &peer_uuid);
    if (rc < 0) {
        TLOGI("SS-unittest: failed (%d) to accept on handle %d\n", rc, ev->handle);
        return;
    }
}

static void handle_event_channel(const uevent_t *ev)
{
    int rc;

    ipc_msg_t       msg;
    ipc_msg_info_t  msg_info;
    uint8_t         msg_buf[MAX_MESG_BUF_SZ];

    iovec_t iov;

    handle_t chan;

    if (ev->event & IPC_HANDLE_POLL_HUP) {
        close(ev->handle);
        return;
    }

    if (ev->event & IPC_HANDLE_POLL_MSG) {

        iov.base = msg_buf;
        iov.len  = sizeof(msg_buf);

        msg.num_iov     = 1;
        msg.iov         = &iov;
        msg.num_handles = 0;
        msg.handles     = NULL;

        chan = ev->handle;

        rc = get_msg(chan, &msg_info);
        if (rc != NO_ERROR) {
            TLOGI("SS-unittest: failed (%d) to receive message info\n", rc);
            return;
        }

        rc = read_msg(chan, msg_info.id, 0, &msg);
        if (rc < 0) {
            TLOGI("SS-unittest: failed (%d) to read message\n", rc);
            return;
        }

        TLOGI("SS-unittest: message from normal world: %s\n", msg_buf);

        TLOGI("SS-unittest: running all\n");
        run_all_tests(STORAGE_CLIENT_TD_PORT);
        TLOGI("SS-unittest: complete!");
    }
}

int main(void)
{
    int rc;

    handle_t port;
    uevent_t ev;

    rc = port_create( SS_UT_SRV_NAME, 1, MAX_PORT_BUF_SZ, IPC_PORT_ALLOW_NS_CONNECT );
    if (rc < 0) {
        TLOGI("SS-unittest: failed to create TIPC port(%d): %s\n", rc, SS_UT_SRV_NAME);
        return -1;
    }

    port = (handle_t)rc;

    while (true) {
        ev.handle = INVALID_IPC_HANDLE;
        ev.event  = 0;
        ev.cookie = NULL;

        rc = wait_any(&ev, -1);

        if (rc != NO_ERROR) {
            TLOGI("SS-unittest: error(%d) in server loop\n", rc);
            break;
        }

        if (ev.handle == port)
            handle_event_port(&ev);
        else
            handle_event_channel(&ev);
    }
}

#if 0
int main(void)
{
    TLOGI("SS-unittest: running all\n");
    run_all_tests(STORAGE_CLIENT_TD_PORT);
//  run_all_tests(STORAGE_CLIENT_TDEA_PORT);
//  run_all_tests(STORAGE_CLIENT_TP_PORT);
    TLOGI("SS-unittest: complete!");
}
#endif
