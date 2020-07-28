/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "vpu_test"

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>

#include <ion/ion.h>         // Android standard ION api
#include <linux/ion_drv.h>   // define for ion_mm_data_t
#include <ion.h>             // interface for mtk ion

#include <vpu.h>                     // interface for vpu stream
#include "vpu_lib_sett_ksample.h"    // algo. header
#include <unistd.h>
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <map>

#define MAX_VPU_CORES 3

#define VT_LOGI(fmt, arg...) printf(LOG_TAG "(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define VT_LOGW(fmt, arg...) printf(LOG_TAG "(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)

#define VT_LOG(fmt, arg...)  do { \
    if (mVerbose) { \
        printf(LOG_TAG "(%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg); \
    } } while (0)

#define profile_inst()  \
    timeval __start, __end; \
    uint32_t __diff = 0;

#define profile_reset() do { \
    __diff = 0; } while (0);

#define profile_start() do { \
       if (mProfile) gettimeofday(&__start, NULL); \
    } while(0);

#define profile_end() do { \
    if (mProfile) { \
        gettimeofday(&__end, NULL); \
        __diff = __diff + (__end.tv_sec - __start.tv_sec)*1000000 + (__end.tv_usec - __start.tv_usec); \
    } } while(0);

#define profile_time() (__diff)

// #define TEST_IMG_WIDTH1         640     // image from header file

int ion_drv_handle = 0;
pthread_mutex_t mutex;

typedef struct d2d_k3_property_structure {
    int           param1;
    int           param2;
    int           param3;
    int           param4;
    int           param5;
} d2d_k3_property_structure_t;

typedef struct d2d_k7_sett_structure {
    int enable_compare;
    int result;
    int cnt_process;
    int cnt_dma;
} d2d_k7_sett_structure_t;

static void _vpu_setup_buf(VpuBuffer &buf, int buf_fd, VpuBufferFormat format,
                           unsigned int width,  unsigned int height, unsigned int buf_offset,
                           unsigned int stride, unsigned int length)
{

//    VT_LOGI("fd: %d, format: %d, width: %d, height: %d, offset: %d, stride: %d, length: %d",
//        buf_fd, format, width, height, buf_offset, stride, length);

    buf.format      = format;
    buf.width       = width;
    buf.height      = height;
    buf.planeCount  = 1;

    /* ion case */
    buf.planes[0].fd      = (unsigned int)buf_fd;
    buf.planes[0].offset  = buf_offset;
    buf.planes[0].stride  = stride;
    buf.planes[0].length  = length;
}

unsigned diff_img(uint8_t *a, uint8_t *b, unsigned size)
{
    unsigned i;

    for (i = 0; i < size; i++)
        if (a[i] != b[i])
            return (i+1);

    return 0;
}

static bool buf_alloc(unsigned len, int *fd_out, unsigned char **va_out)
{
    ion_user_handle_t buf_hnd;
    bool ret = false;
    void *va;

    // allocate ion buffer handle
    pthread_mutex_lock(&mutex);
    if(ion_alloc_mm(ion_drv_handle, (size_t)len, 0, 0, &buf_hnd))  // no alignment, non-cache
    {
        VT_LOGW("fail to get ion buffer handle (drv_h=0x%x, len=%d)", ion_drv_handle, len);
        goto out;
    }

    // get ion buffer share handle
    if(ion_share(ion_drv_handle, buf_hnd, fd_out))
    {
        VT_LOGW("fail to get ion buffer share handle");
        goto free_out;
    }

    // get buffer virtual address
    va = ion_mmap(ion_drv_handle, NULL,
        (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, *fd_out, 0);

    // VT_LOGI("alloc ion: ion_buf_handle %d, share_fd %d", buf_hnd, *fd_out);

    if(!va) {
        VT_LOGW("fail to get buffer virtual address");
        goto free_out;
    }

    ret = true;
    *va_out = (uint8_t *)va;

    goto out;

free_out:
    if(ion_free(ion_drv_handle, buf_hnd))
        VT_LOGW("ion free fail");
out:
    pthread_mutex_unlock(&mutex);
    return ret;
}

static void buf_free(unsigned len, int fd, unsigned char *buf_va)
{
    ion_user_handle_t buf_hnd;

    if (!buf_va)
        goto out;

    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    pthread_mutex_lock(&mutex);

    if(ion_import(ion_drv_handle, fd, &buf_hnd))  {
        VT_LOGW("fail to get import share buffer fd");
        goto out;
    }
    // VT_LOGI("import ion: ion_buf_handle %d, share_fd %d", buf_hnd, fd);
    // 2. free for IMPORT ref cnt
    if(ion_free(ion_drv_handle, buf_hnd)) {
        VT_LOGW("fail to free ion buffer (free ion_import ref cnt)");
        goto out;
    }
    // 3. unmap virtual memory address
    if(ion_munmap(ion_drv_handle, (void *)buf_va, (size_t)len)) {
        VT_LOGW("fail to get unmap virtual memory");
        goto out;
    }
    // 4. close share buffer fd
    if(ion_share_close(ion_drv_handle, fd)) {
        VT_LOGW("fail to close share buffer fd");
        goto out;
    }
    // 5. pair of ion_alloc_mm
    if(ion_free(ion_drv_handle, buf_hnd)) {
        VT_LOGW("fail to free ion buffer (free ion_alloc_mm ref cnt)");
        goto out;
    }
out:
    pthread_mutex_unlock(&mutex);
}


void buf_ctrl(int fd, void *va, unsigned size, ION_CACHE_SYNC_TYPE ctrl, const char *prefix)
{
    int ret;
    struct ion_sys_data sys_data;
    ion_user_handle_t buf_handle;

    ret = ion_import(ion_drv_handle, fd, &buf_handle);

    if (ret) {
        VT_LOGW("buf %s: ion_import: fd: %d, va: %lx, size: %d, ret: %d",
            (prefix ? prefix : ""), fd, (unsigned long)va, size, ret);
        return;
    }

    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle=(ion_user_handle_t)buf_handle;
    sys_data.cache_sync_param.sync_type=ctrl;
    sys_data.cache_sync_param.va = va;
    sys_data.cache_sync_param.size = size;

    ret = ion_custom_ioctl(ion_drv_handle, ION_CMD_SYSTEM, &sys_data);

    // VT_LOGI("buf %s (%d): fd: %d, va: %lx, size: %d, ret: %d",
    //    (prefix ? prefix : ""), ctrl, fd, (unsigned long)va, size, ret);

    ret = ion_free(ion_drv_handle, buf_handle);

    if (ret) {
        VT_LOGW("buf %s: ion_free: fd: %d, user_hnd: %d, ret: %d",
            (prefix ? prefix : ""), fd, buf_handle, ret);
    }

}

void buf_sync(int fd, void *va, unsigned size)
{
    buf_ctrl(fd, va, size, ION_CACHE_FLUSH_BY_RANGE, "flush");
}

void buf_invalid(int fd, void *va, unsigned size)
{
    buf_ctrl(fd, va, size, ION_CACHE_INVALID_BY_RANGE, "invalid");
}

void dump_img(const char* name, uint8_t *v, unsigned size, unsigned offset)
{
    int i;
    VT_LOGW("%s %d (offset: %d):", name, size, offset);

    for (i = 0; i < 16; i++) {
        uint8_t *a = &v[offset + i*16];

        VT_LOGW("%d: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            (offset + i*16),
            a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7],
            a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15]);
    }
}

//------------------------------------------------------------------------------
class VpuTest{
    public:
        VpuTest() { };
        virtual ~VpuTest();

        // Parameters
        bool mVerbose = false;
        bool mProfile = false;
        unsigned mImgWidth = 0;
        unsigned mImgHeight = 0;
        unsigned mImgSize = 0;
        unsigned mParamLength = 0;

        unsigned char *mInput = nullptr;
        unsigned char *mParam = nullptr;
        unsigned char *mGolden = nullptr;

        uint32_t mLoopCount = 0;
        int mOpp = 0;       // 0~15: fix opp, <0: random(0~15)
        int mBoost = 100;   // 0~100: given boost, <=0: random(1~100)

        std::string mAlgoName = "unknown";

        // Load Functions
        bool Load(const char *path, unsigned char **buf, unsigned *size_out);
        bool LoadInput(const char *path)  { return Load(path, &mInput, &mImgSize); }
        bool LoadParam(const char *path)  { return Load(path, &mParam, &mParamLength); }
        bool LoadGolden(const char *path) { return Load(path, &mGolden, nullptr); }

        // Test Steps
        virtual bool PatternSetup() = 0;   // setup mInput, mParam, mGolden
        bool VpuInit();
        bool AlgoInit();
        virtual bool PrepareBuffers() = 0;
        virtual bool RequestInit() = 0;
        void SetPerformance();
        virtual bool Test() = 0;
        virtual bool Verify() = 0;
        virtual bool FreeBuffers() = 0;
        bool VpuExit();

        bool RunProcesses(int count);
        bool Run();

        // Runtime Settings
        std::string  mName = "unknown";
        VpuStream   *mStream = nullptr;
        VpuAlgo     *mAlgo = nullptr;
        VpuRequest  *mReq = nullptr;

        unsigned char *mImg1 = nullptr, *mImg2 = nullptr, *mData = nullptr;
        int            mImg1Fd = -1, mImg2Fd = -1, mDataFd = -1;

        VpuBuffer    mBufImg1 = {0}, mBufImg2 = {0}, mBufData = {0};
        VpuExtraParam mEP;
};

VpuTest::~VpuTest()
{
    free(mInput);
    free(mParam);
    free(mGolden);
}

bool VpuTest::Load(const char *path, unsigned char **buf, unsigned *size_out)
{
    int ret;
    unsigned char *b = nullptr;
    unsigned char *p = nullptr;
    unsigned long size;
    unsigned long remain;
    unsigned long len;
    unsigned long offset;
    struct stat st;
    int fd = -1;

    ret = stat(path, &st);

    if (ret) {
        VT_LOGW("Unable to stat %s: %s (%d)", path, strerror(errno), errno);
        goto error;
    }

    size = (unsigned long)st.st_size;

    b = (unsigned char*)malloc(size);

    if (!b) {
        VT_LOGW("Unable to allocate memory for \"%s\", size: %ld", path, size);
        goto error;
    }

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        VT_LOGW("Unable to open \"%s\" for read: %s (%d)", path, strerror(errno), errno);
        goto error;
    }

    p = b;
    remain = size;
    offset = 0;

    while (remain > 0) {
        len = read(fd, p, remain);

        if (len == (unsigned long)(-1)) {
            VT_LOGW("Read \"%s\" failed: %s (%d), offset: %ld",
                path, strerror(errno), errno, offset);
            goto error;
        } else if (len == 0) {
            break;
        }

        p += len;
        offset += len;
        remain -= len;
    }

    if (offset != size) {
        VT_LOGW("Read \"%s\" failed: unexpected end of file, read %ld bytes, expected: %ld bytes",
            path, offset, size);
        goto error;
    }

    VT_LOG("Read \"%s\", %ld bytes", path, size);

    if (size_out)
        *size_out = size;

    if (buf)
        *buf = b;
    else
        free(b);

    close(fd);

    return true;

error:
    if (fd >= 0)
        close(fd);

    if (b)
        free(b);

    return false;
}

void VpuTest::SetPerformance()
{
    VpuExtraParam extraP;
    uint8_t opp = 0;
    uint8_t boost = 100;

    if (mOpp < 0)
        opp = rand() % 16;
    else if (mOpp <= 15)
        opp = (mOpp & 0xf);

    extraP.opp_step = opp;

    if (mBoost <0)
        boost = (rand() % 100) + 1;
    else
        boost = mBoost;

    extraP.boost_value = boost;

    VT_LOG("%s: SetPerformance(opp:%d, boost:%d)", mName.c_str(), opp, boost);
    mReq->setExtraParam(extraP);
}

bool VpuTest::RunProcesses(int count)
{
    bool ret = true;
    std::map<pid_t, int> pids;

    for (int i = 0; i < count; i++) {
        pid_t pid;
        pid = fork();

        if (!pid) {
            return Run();
        } else {
            VT_LOGI("main: fork process #%d pid: %d", i+1, pid);
            pids[pid] = i+1;
        }
    }

    for (int i = 0; i < count; i++) {
        int index;
        pid_t pid;
        int status;

        pid = waitpid(-1, &status, 0);

        if (pid < 0) {
             VT_LOGW("waitpid failed");
             ret = false;
             break;
        }

        index = pids[pid];

        status = WEXITSTATUS(status);

        if (status) {
            VT_LOGI("main: test process #%d pid: %d, Failed: %d", index, pid, status);
            ret = false;
            pids[pid] = -1;
        } else {
            VT_LOGI("main: test process #%d pid: %d, Passed", index, pid);
            pids[pid] = 0;
        }
    }

    return ret;
}

bool VpuTest::Run()
{
    bool ret = false;
    unsigned i;

    VT_LOG("%s: PatternSetup", mName.c_str());
    if (!PatternSetup())
        goto out;

    VT_LOG("%s: VpuInit", mName.c_str());
    if (!VpuInit())
        goto out;

    VT_LOG("%s: AlgoInit", mName.c_str());
    if (!AlgoInit())
        goto out;

    VT_LOG("%s: PrepareBuffers", mName.c_str());
    if (!PrepareBuffers())
        goto vpu_exit;

    VT_LOG("%s: RequestInit", mName.c_str());
    if (!RequestInit())
        goto free_buf;

    for (i=0; i<mLoopCount; i++) {
        VT_LOG("%s: Loop #%d: Test", mName.c_str(), i+1);
        SetPerformance();
        if (!Test())
            goto free_buf;
        VT_LOG("%s: Loop #%d: Verify", mName.c_str(), i+1);
        if (!Verify())
            goto free_buf;
    }
    ret = true;
free_buf:
    VT_LOG("%s: FreeBuffers", mName.c_str());
    FreeBuffers();
vpu_exit:
    VT_LOG("%s: VpuExit", mName.c_str());
    VpuExit();
out:
    VT_LOGI("%s: Test Result: %s",
        mName.c_str(),
        (ret ? "PASS" : "FAIL"));

    return ret;
}

bool VpuTest::VpuInit()
{
    mStream = VpuStream::createInstance(mName.c_str());
    if (!mStream) {
        VT_LOGW("%s: fail to get vpu handle", mName.c_str());
        return false;
    }
    return true;
}

bool VpuTest::AlgoInit()
{
    const char *name = mAlgoName.c_str();

    VT_LOG("%s: algo_name: %s", mName.c_str(), name);

    mAlgo = mStream->getAlgo(name);
    if (mAlgo == NULL) {
        VT_LOGW("%s: fail to get vpu algo: %s", mName.c_str(), name);
        return false;
    }

    return true;
}

bool VpuTest::VpuExit()
{
    if (mReq)
        mStream->release(mReq);

    if (mStream)
        delete mStream;
    return true;
}

//------------------------------------------------------------------------------
class DynAlgo {
    public:
        unsigned char *mBin = nullptr;
        unsigned mSize = 0;
        int mFd = 0;
};

class VpuTestDynamic: public VpuTest
{
    public:
        VpuTestDynamic(unsigned loop_count = 0) {
            mLoopCount = loop_count;
            mAlgoName = "custom_algo";
            mName = mAlgoName;
        };
        std::map<int, DynAlgo *> mDynAlgos;
        bool LoadAlgo(const char *path);
        virtual ~VpuTestDynamic();
        bool PatternSetup();
        bool AlgoInit();
        bool PrepareBuffers();
        bool RequestInit();
        bool Test();
        bool Verify();
        bool FreeBuffers();
};

VpuTestDynamic::~VpuTestDynamic()
{
}

bool VpuTestDynamic::LoadAlgo(const char *path)
{
    bool ret;
    int i;

    for (i = 0; i < MAX_VPU_CORES; i++) {
        DynAlgo *alg = new DynAlgo();
        std::string file = path;
        file = file + ".core" + std::to_string(i);

        ret = Load(file.c_str(), &alg->mBin, &alg->mSize);

        if (!ret) {
            delete alg;
            break;
        }

        mDynAlgos[i] = alg;
    }

    VT_LOG("%s: loaded %d algorithms", mName.c_str(), i);

    return (i > 0);
}

bool VpuTestDynamic::AlgoInit()
{
    int i;
    bool ret = false;
    const char *name = mAlgoName.c_str();

    if (mDynAlgos.empty()) {
        VT_LOGW("%s: algorithm was not loaded", mName.c_str());
        goto out;
    }

    VT_LOG("%s: algo_name: %s", mName.c_str(), name);

    for (i = 0; i < mDynAlgos.size(); i++) {
        DynAlgo *alg = mDynAlgos[i];

        alg->mFd = mStream->createAlgo(name,
            (char*)alg->mBin, alg->mSize, i /* core id */);

        if (!alg->mFd) {
            VT_LOGW("%s: createAlgo failed at core%d", mName.c_str(), i);
            goto out;
        }
    }

    mAlgo = mStream->getAlgo(name);

    if (mAlgo == NULL) {
        VT_LOGW("%s: fail to get vpu algo: %s", mName.c_str(), name);
        goto out;
    }

    ret = true;
out:
    return ret;
}


bool VpuTestDynamic::PatternSetup()
{
    return true;
}

bool VpuTestDynamic::PrepareBuffers()
{
    return true;
}

bool VpuTestDynamic::RequestInit()
{
    bool ret = false;

    VT_LOG("%s: acquire: %s", mName.c_str(), mAlgoName.c_str());
    mReq = mStream->acquire(mAlgo);

    if (mReq)
        ret = true;

    return ret;
}

bool VpuTestDynamic::Test()
{
    bool ret = false;

    // 4. enqueue & dequeue
    VT_LOG("%s: runReq: %lx", mName.c_str(), (unsigned long)mReq);
    if (!mStream->runReq(mReq)) {
        VT_LOGW("runReq failed");
        goto out;
    }

    VT_LOG("%s: waitReq: %lx", mName.c_str(), (unsigned long)mReq);
    if (!mStream->waitReq(mReq)) {
        VT_LOGW("waitReq failed");
        goto out;
    }

    ret = true;
out:
    return ret;
}

bool VpuTestDynamic::Verify()
{
    return true;
}

bool VpuTestDynamic::FreeBuffers()
{
    int i;

    for (i = 0; (unsigned int)i < mDynAlgos.size(); i++) {
        DynAlgo *alg = mDynAlgos[i];

        mDynAlgos[i] = nullptr;
        mStream->freeAlgo(alg->mFd);
        free(alg->mBin);
        delete alg;
    }

    return true;
}

//------------------------------------------------------------------------------
class VpuTestKP: public VpuTest
{
    public:
        unsigned long mScale = 0;
        unsigned i4PadWidth = 256;
        unsigned i4PadHeight = 256;

        VpuTestKP(unsigned loop_count = 0, unsigned long scale = 1024) {
            mLoopCount = loop_count;
            mAlgoName = "vpu_flo_d2d_kp";
            mName = mAlgoName;
            mScale = scale;
        };
        virtual ~VpuTestKP();
        bool PatternSetup();
        bool PrepareBuffers();
        bool RequestInit();
        bool Test();
        bool Verify();
        bool FreeBuffers();
};

VpuTestKP::~VpuTestKP()
{
}

bool VpuTestKP::PatternSetup()
{
    mImgSize = mScale;

    if (mScale > (i4PadWidth * i4PadHeight))
        i4PadHeight = (mScale/i4PadWidth)+1;
    else
        mImgSize = i4PadWidth * i4PadHeight;

    // prVaIn1:=mImg1, mImg1Fd:=i4In1FD,
    if (!buf_alloc(mImgSize, &mImg1Fd, &mImg1))
        return false;

    memset(mImg1, 0x01, mImgSize);

    return true;
}

bool VpuTestKP::PrepareBuffers()
{
    _vpu_setup_buf(mBufImg1, mImg1Fd, eFormatData,
        i4PadWidth, i4PadHeight, 0, i4PadWidth, i4PadWidth*i4PadHeight);
    return true;
}

bool VpuTestKP::RequestInit()
{
    bool ret = false;

    vpu_st_sett_ksample property;
    VpuExtraParam extraP;
    std::vector<VpuStatus> stat; // deprecated

    VT_LOG("%s: acquire: %s", mName.c_str(), mAlgoName.c_str());
    mReq = mStream->acquire(mAlgo);

    //fill id in buffer structure,
    VT_LOG("%s: addBuffer 1", mName.c_str());
    mBufImg1.port_id = 0;
    ret = mReq->addBuffer(mBufImg1);  // input

    property.S32_Wight = i4PadWidth;
    property.S32_Height = i4PadHeight;
    property.S32_Op = 0x77;
    property.U64_TotalLength = mScale; // totalLength

    VT_LOG("%s: setProperty", mName.c_str());
    ret = mReq->setProperty((void *)&property, sizeof(vpu_st_sett_ksample));

    return ret;
}

bool VpuTestKP::Test()
{
    profile_inst()
    uint32_t run_t, wait_t;
    bool ret = false;

    // flush input
    VT_LOG("%s: bufsync", mName.c_str());
    buf_sync(mImg1Fd, mImg1, mImgSize);

    // 4. enqueue & dequeue
    VT_LOG("%s: runReq: %lx", mName.c_str(), (unsigned long)mReq);
    profile_start()
    if (!mStream->runReq(mReq)) {
        VT_LOGW("runReq failed");
        goto out;
    }
    profile_end()
    run_t = profile_time();
    profile_reset()

    VT_LOG("%s: waitReq: %lx", mName.c_str(), (unsigned long)mReq);
    profile_start()
    if (!mStream->waitReq(mReq)) {
        VT_LOGW("waitReq failed");
        goto out;
    }
    profile_end()
    wait_t = profile_time();

    if (mProfile) {
         VT_LOGW("%s: runReq: %d us, waitReq: %d us",
            mName.c_str(), run_t, wait_t);
    }

    ret = true;
out:
    return ret;
}

bool VpuTestKP::Verify()
{
    return true;
}

bool VpuTestKP::FreeBuffers()
{
    buf_free(mImgSize, mImg1Fd, mImg1);
    return true;
}

//------------------------------------------------------------------------------
class VpuTestK7: public VpuTest
{
    public:
        VpuTestK7(unsigned loop_count = 0) {
            mLoopCount = loop_count;
            mAlgoName = "vpu_flo_d2d_k7";
            mName = mAlgoName;
        };
        virtual ~VpuTestK7();
        bool PatternSetup();
        bool PrepareBuffers();
        bool RequestInit();
        bool Test();
        bool Verify();
        bool FreeBuffers();
};

VpuTestK7::~VpuTestK7()
{
}

bool VpuTestK7::PatternSetup()
{
    return true;
}

bool VpuTestK7::PrepareBuffers()
{
    return true;
}

bool VpuTestK7::RequestInit()
{
    bool ret = false;

    d2d_k7_sett_structure_t property;
    VpuExtraParam extraP;

    VT_LOG("%s: acquire: %s", mName.c_str(), mAlgoName.c_str());
    mReq = mStream->acquire(mAlgo);

    // 3.3 set Property to request
    property.enable_compare = 1;
    property.result = 0;
    property.cnt_process = 0;
    property.cnt_dma = 0;

    VT_LOG("%s: setProperty", mName.c_str());
    ret = mReq->setProperty((void*)&property, sizeof(d2d_k7_sett_structure_t));

    return ret;
}

bool VpuTestK7::Test()
{
    bool ret = false;

    // 4. enqueue & dequeue
    VT_LOG("%s: runReq: %lx", mName.c_str(), (unsigned long)mReq);
    if (!mStream->runReq(mReq)) {
        VT_LOGW("runReq failed");
        goto out;
    }

    VT_LOG("%s: waitReq: %lx", mName.c_str(), (unsigned long)mReq);
    if (!mStream->waitReq(mReq)) {
        VT_LOGW("waitReq failed");
        goto out;
    }

    ret = true;
out:
    return ret;
}

bool VpuTestK7::Verify()
{
    d2d_k7_sett_structure_t property;

    property.enable_compare = 1;
    property.result = 0;
    property.cnt_process = 0;
    property.cnt_dma = 0;
    mReq->getProperty((void*)&property, sizeof(d2d_k7_sett_structure_t));

    if ((unsigned)(property.result) != 0xc0000001)
        return false;

    return true;
}

bool VpuTestK7::FreeBuffers()
{
    return true;
}


//------------------------------------------------------------------------------
class VpuTestK3: public VpuTest
{
    public:
        VpuTestK3(unsigned loop_count = 0) {
            mLoopCount = loop_count;
            mAlgoName = "vpu_flo_d2d_k3";
            mName = mAlgoName;
        };
        virtual ~VpuTestK3();
        bool PatternSetup();
        bool PrepareBuffers();
        bool RequestInit();
        bool Test();
        bool Verify();
        bool FreeBuffers();
        bool VpuExit();
};

VpuTestK3::~VpuTestK3()
{
}

bool VpuTestK3::PatternSetup()
{
    if (!mInput || !mGolden || !mParam || !mImgWidth)
        return false;

    mImgHeight = mImgSize/mImgWidth;

    VT_LOG("%s: Size: %d, Width: %d, Height: %d",
        mName.c_str(), mImgSize, mImgWidth, mImgHeight);

    VT_LOG("%s: Src 0x%lx, Golden 0x%lx, Param 0x%lx", mName.c_str(),
        (unsigned long)mInput,
        (unsigned long)mGolden,
        (unsigned long)mParam);
    VT_LOG("%s: sizeSrc 0x%x, sizeGolden 0x%x, sizeParam 0x%x", mName.c_str(),
        mImgSize,
        mImgSize,
        mParamLength);

    // alloc ion buffers
    if (!buf_alloc(mImgSize, &mImg1Fd, &mImg1))
        return false;
    if (!buf_alloc(mImgSize, &mImg2Fd, &mImg2))
        return false;
    if (!buf_alloc(mParamLength, &mDataFd, &mData))
        return false;

    // init test image
    memcpy(mImg1, mInput, mImgSize);

    return true;
}

bool VpuTestK3::PrepareBuffers()
{
    _vpu_setup_buf(mBufImg1, mImg1Fd, eFormatImageY8, mImgWidth, mImgHeight, 0, mImgWidth, mImgSize);
    _vpu_setup_buf(mBufImg2, mImg2Fd, eFormatImageY8, mImgWidth, mImgHeight, 0, mImgWidth, mImgSize);
    _vpu_setup_buf(mBufData, mDataFd ,eFormatData, 0, 0, 0, 0, mParamLength);

    return true;
}

bool VpuTestK3::RequestInit()
{
    bool ret = false;

    d2d_k3_property_structure_t property;
    VpuExtraParam extraP;
    std::vector<VpuStatus> stat; // deprecated

    VT_LOG("%s: acquire: %s", mName.c_str(), mAlgoName.c_str());
    mReq = mStream->acquire(mAlgo);

    //fill id in buffer structure,
    VT_LOG("%s: addBuffer 1", mName.c_str());
    mBufImg1.port_id = 0;
    mReq->addBuffer(mBufImg1);  // input
    VT_LOG("%s: addBuffer 2", mName.c_str());
    mBufImg2.port_id = 1;
    mReq->addBuffer(mBufImg2);  // output
    VT_LOG("%s: addBuffer 3", mName.c_str());
    mBufData.port_id = 2;
    mReq->addBuffer(mBufData);  // input

    // 3.3 set Property to request
    property.param1 = 1;
    property.param2 = 2;
    property.param3 = 3;
    property.param4 = 4;
    property.param5 = 1;
    VT_LOG("%s: setProperty", mName.c_str());
    ret = mReq->setProperty((void*)&property, sizeof(d2d_k3_property_structure_t));

    return ret;
}

bool VpuTestK3::Test()
{
    bool ret = false;

    VT_LOG("%s: memcpy", mName.c_str());
    memcpy(mImg1, mInput, mImgSize);
    memset(mImg2, 0, mImgSize);  // clean output image buffer
    memcpy(mData, mParam, mParamLength); // MUST copy again because vpu will modify params after executing d2d

    // flush input
    VT_LOG("%s: bufsync", mName.c_str());
    buf_sync(mImg1Fd, mImg1, mImgSize);
    buf_sync(mImg2Fd, mImg2, mImgSize);
    buf_sync(mDataFd, mData, mParamLength);

    // 4. enqueue & dequeue
    VT_LOG("%s: runReq: %lx", mName.c_str(), (unsigned long)mReq);
    if (!mStream->runReq(mReq)) {
        VT_LOGW("runReq failed");
        goto out;
    }

    VT_LOG("%s: waitReq: %lx", mName.c_str(), (unsigned long)mReq);
    if (!mStream->waitReq(mReq)) {
        VT_LOGW("waitReq failed");
        goto out;
    }

    // discard output
    buf_invalid(mImg2Fd, mImg2, mImgSize);

    ret = true;
out:
    return ret;
}

bool VpuTestK3::Verify()
{
    if (memcmp(mImg2, mGolden, mImgSize)) {
        VT_LOGW("%s: compare failed", mName.c_str());
        unsigned diff;
        diff = diff_img(mImg2, mGolden, mImgSize);
        if (diff) {
            diff--;
            dump_img("input", mImg1, mImgSize, diff);
            dump_img("output", mImg2, mImgSize, diff);
            dump_img("golden", mGolden, mImgSize, diff);
        }
        return false;
    }
    return true;
}

bool VpuTestK3::FreeBuffers()
{
    buf_free(mImgSize, mImg1Fd, mImg1);
    buf_free(mImgSize, mImg2Fd, mImg2);
    buf_free(mParamLength, mDataFd, mData);
    return true;
}

/*******************************************************************************
*  Main Function
********************************************************************************/
static struct option long_options[] =
{
    {"algo", required_argument, 0, 'a'},
    {"loop", required_argument, 0, 'l'},
    {"threads", required_argument, 0, 't'},
    {"input", required_argument, 0, 'i'},
    {"width", required_argument, 0, 'w'},
    {"param", required_argument, 0, 'p'},
    {"golden", required_argument, 0, 'g'},
    {"opp", required_argument, 0, 'o'},
    {"boost", required_argument, 0, 'b'},
    {"kp_scale", required_argument, 0, 's'},
    {"profile", no_argument, 0, 'f'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void showhelp(void)
{
    printf("VPU unit test program. Usage:\n\n");
    printf("  vpu_test64 [-a algorithm] [-l loop_count] [-t threads] [-i input] [-w input image width]\n");
    printf("             [-p param] [-g golden] [-o opp] [-b boost] [-f] [-h]\n\n");

    printf("  --help, -h       show this help\n");
    printf("  --algo, -a       algorithm name, or path to the algorithm file (dynamic load algo)\n");
    printf("                   available built-in algorithms:\n");
    printf("                     \"vpu_flo_d2d_k3\", \"k3\"\n");
    printf("                     \"vpu_flo_d2d_k7\", \"k7\"\n");
    printf("                     \"vpu_flo_d2d_kp\", \"kp\"\n\n");

    printf("  --loop, -l       loop count\n\n");

    printf("  --threads, -t    (optional) number of con-current test processes, default=1 \n");
    printf("  --opp, -o        (optional) opp value (0~15), or string \"random\"\n");
    printf("  --boost, -b      (optional) boost value (1~100), or string \"random\"\n");
    printf("  --verbose, -v    (optional) show verbose logs\n\n");

    printf("  --input, -i      [k3] path to the input image\n");
    printf("  --width, -w      [k3] width of input image\n");
    printf("  --param, -p      [k3] path to the parameter file3\n");
    printf("  --golden, -g     [k3] path to the golden image\n\n");

    printf("  --kp_scale, -s   [k7] idma scale, default=1024\n");
    printf("  --profile, -f    [k7] enable profiling logs\n\n");

    printf("Examples:\n");
    printf("  vpu_test64 -a k3 -l 10 -w 640 -i /data/vpu_test_src.raw -p /data/vpu_test_src_param.raw -g /data/vpu_test_golden.raw\n");
    printf("  vpu_test64 -a k7 -l 20 -t 3\n\n");
}

int main(int argc, char ** argv)
{
    int ret = 0;
    unsigned cfg_threads = 0;
    unsigned cfg_loop = 0;
    unsigned long cfg_kp_scale = 1024;
    unsigned cfg_input_width = 0;
    std::string cfg_algo = "";
    std::string cfg_input;
    std::string cfg_golden;
    std::string cfg_param;
    bool cfg_profile = false;
    bool cfg_verbose = false;
    int cfg_opp = 0;
    int cfg_boost = 0;

    VpuTest *test;

    // reading paramters
    // /data/local/tmp/vpu_test64 -a vpu_flo_d2d_k3 -l 1000 -t 3 -i /data/vpu_test_src.raw -p /data/vpu_test_src_param.raw -g /data/vpu_test_golden.raw
    // HQA script: adb shell "vpu_test64 2 2 1 0 0 0 | grep -i 'vpu test'"
    while (1) {
        int c;
        int option_index;
        c = getopt_long(argc, argv, "a:l:t:i:p:g:o:s:b:w:fhv",
            long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'a':
                VT_LOGI("algo: %s", optarg);
                cfg_algo = optarg;
                break;
            case 'b':
                if (!strcmp(optarg, "random")) {
                    cfg_boost = -1; // random opp
                    VT_LOGI("boost: random");
                } else {
                    cfg_boost = atoi(optarg);
                    VT_LOGI("boost: %d", cfg_boost);
                }
                break;
            case 'f':
                VT_LOGI("profile enabled");
                cfg_profile = true;
                break;
            case 'l':
                VT_LOGI("loops: %d", atoi(optarg));
                cfg_loop = (unsigned)atoi(optarg);
                break;
            case 't':
                VT_LOGI("threads: %d", atoi(optarg));
                cfg_threads = (unsigned)atoi(optarg);
                break;
            case 'i':
                VT_LOGI("input image: %s", optarg);
                cfg_input = optarg;
                break;
            case 'w':
                VT_LOGI("input image width: %d", atoi(optarg));
                cfg_input_width = (unsigned)atoi(optarg);
                break;
            case 'p':
                VT_LOGI("parameter image: %s", optarg);
                cfg_param = optarg;
                break;
            case 'g':
                VT_LOGI("golden output image: %s", optarg);
                cfg_golden = optarg;
                break;
            case 'o':
                if (!strcmp(optarg, "random")) {
                    cfg_opp = -1; // random opp
                    VT_LOGI("opp: random");
                } else {
                    cfg_opp = atoi(optarg);
                    VT_LOGI("opp: %d", cfg_opp);
                }
                break;
            case 's':
                cfg_kp_scale = (unsigned long)atol(optarg);
                VT_LOGI("kp_scale: %ld", cfg_kp_scale);
                break;
            case 'v':
                VT_LOGI("verbose");
                cfg_verbose = true;
                break;
            case 'h':
            case '?':
                showhelp();
                return 0;
            default:
                break;
        }

    }

    ion_drv_handle = mt_ion_open(__FILE__);
    pthread_mutex_init(&mutex,NULL);
    if (!ion_drv_handle)
    {
      printf("fail to get ion driver handle\n");
      goto out;
    }

    if (cfg_algo.empty()) {
        printf("Please assign algorithm (-a)\n\n");
        showhelp();
        goto out;
    }

    if (cfg_algo == "vpu_flo_d2d_k3" || cfg_algo == "k3") {
        test = new VpuTestK3(cfg_loop);

        if (cfg_input_width <= 0) {
            printf("k3: cfg_input_width was not specified or invalid\n");
            goto out;
        }

        if (cfg_input.empty() || cfg_param.empty() || cfg_golden.empty()) {
            printf("k3: input/param/golden file was not specified\n");
            goto out;
        }

        if (!test->LoadInput(cfg_input.c_str()) ||
            !test->LoadParam(cfg_param.c_str()) ||
            !test->LoadGolden(cfg_golden.c_str())) {
            goto out;
        }

        test->mImgWidth = cfg_input_width;
    }
    else if (cfg_algo == "vpu_flo_d2d_k7" || cfg_algo == "k7") {
        test = new VpuTestK7(cfg_loop);
    }
    else if (cfg_algo == "vpu_flo_d2d_kp" || cfg_algo == "kp") {
        test = new VpuTestKP(cfg_loop, cfg_kp_scale);
    }
    else {
        test = new VpuTestDynamic(cfg_loop);

        auto td = static_cast<VpuTestDynamic*>(test);

        if (!td->LoadAlgo(cfg_algo.c_str())) {
            delete test;
            goto out;
        }
    }

    test->mVerbose = cfg_verbose;
    test->mProfile = cfg_profile;
    test->mBoost = cfg_boost;
    test->mOpp = cfg_opp;

    if (cfg_threads > 1) {
        ret = test->RunProcesses(cfg_threads);
    } else {
        ret = test->Run();
    }

    delete test;

    return (ret ? 0 : -1);

out:
    if(ion_drv_handle)
        ion_close(ion_drv_handle);

    return 0;
}
