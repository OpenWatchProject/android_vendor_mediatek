/*
 * Copyright (c) 2018 MediaTek Inc.
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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <ion.h>
#include <ion/ion.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#ifdef ION_NUM_HEAP_IDS
#undef ION_NUM_HEAP_IDS
#endif
#include <linux/mtk_ion.h>

#include "pmem_common.h"
#include "pmem_dbg.h"

#define PERF_TAG "[PERF]"

#define PERF_MAX_ALLOC_COUNT (64)
#define PERF_ALLOC_CHUNK_SIZE (SIZE_1M)
#define PERF_ION_MASK ION_HEAP_MULTIMEDIA_TYPE_PROT_MASK
#define PERF_ION_FLAGS (ION_FLAG_MM_HEAP_INIT_ZERO)

#define TMEM_DEV "/proc/tmem0"

class PerfDeviceHelper
{
  public:
    PerfDeviceHelper()
    {
        int idx;

        for (idx = 0; idx < PERF_MAX_ALLOC_COUNT; idx++) {
            _protSharedIonHandle[idx] = 0;
            _protSharedPhyHandle[idx] = 0;
        }
        _ionFd = -1;
        _tmemFd = -1;
    }

    void SetUp()
    {
        ASSERT_TRUE((_ionFd = ion_open()) >= 0);
        _tmemFd = open(TMEM_DEV, O_RDWR, 0);
        ASSERT_TRUE((_tmemFd >= 0));
    }

    void TearDown()
    {
        if (_tmemFd >= 0)
            ASSERT_EQ(0, close(_tmemFd));
        ASSERT_EQ(0, ion_close(_ionFd));
    }

    int GetPhysicalHandle(ion_user_handle_t ion_handle, int* mem_handle)
    {
        struct ion_sys_data sys_data;
        sys_data.sys_cmd = ION_SYS_GET_PHYS;
        sys_data.get_phys_param.handle = ion_handle;
        if (ion_custom_ioctl(_ionFd, ION_CMD_SYSTEM, &sys_data))
            return -1;

        *mem_handle = sys_data.get_phys_param.phy_addr;
        return 0;
    }

    int TmemSendCommand(int cmd)
    {
        int ret;
        char buf[16];
        int buf_len;

        buf_len = snprintf(buf, 16, "%d", cmd);
        ret = write(_tmemFd, buf, buf_len);

        if (ret < 0) {
            CONSOLE_ERR(PERF_TAG, "write command failed=", ret);
            return ret;
        }
        return 0;
    }

    int IonAllocByIndex(int idx)
    {
        int ret;

        ret = ion_alloc(_ionFd, PERF_ALLOC_CHUNK_SIZE, PERF_ALLOC_CHUNK_SIZE,
                        PERF_ION_MASK, PERF_ION_FLAGS,
                        &_protSharedIonHandle[idx]);
        if (ret != 0) {
            CONSOLE_ERR("ion alloc idx=", idx, "failed, ret=", ret);
            return ret;
        }
        if (_protSharedIonHandle[idx] == 0) {
            CONSOLE_ERR("invalid ion handle=", _protSharedIonHandle[idx]);
            return -1;
        }

        return 0;
    }

    int IonQueryHandleByIndex(int idx)
    {
        int ret;

        ret = GetPhysicalHandle(_protSharedIonHandle[idx],
                                &_protSharedPhyHandle[idx]);
        if (ret != 0) {
            CONSOLE_ERR("ion query handle idx=", idx, "failed, ret=", ret);
            return ret;
        }
        if (_protSharedPhyHandle[idx] == 0) {
            CONSOLE_ERR("invalid query handle=", _protSharedPhyHandle[idx]);
            return -1;
        }

        return 0;
    }

    int IonFreeByIndex(int idx)
    {
        int ret;

        if (_protSharedIonHandle[idx] == 0)
            return 0;

        ret = ion_free(_ionFd, _protSharedIonHandle[idx]);
        if (ret != 0) {
            CONSOLE_ERR("ion free idx=", idx, "failed, ret=", ret);
            return ret;
        }

        _protSharedIonHandle[idx] = 0;
        _protSharedPhyHandle[idx] = 0;
        return 0;
    }

    int IonFreeAll()
    {
        int ret;
        int idx;

        for (idx = 0; idx < PERF_MAX_ALLOC_COUNT; idx++) {
            ret = IonFreeByIndex(idx);
            if (ret != 0)
                return ret;
        }

        return 0;
    }

  private:
    int _ionFd;
    int _tmemFd;
    ion_user_handle_t _protSharedIonHandle[PERF_MAX_ALLOC_COUNT];
    ion_user_handle_t _protSharedPhyHandle[PERF_MAX_ALLOC_COUNT];
};

class PerfTime
{
  public:
    void Reset()
    {
        _totalUsec = 0.0;
        _averageUsec = 0.0;
        _averageMsec = 0.0;
        _maxUsec = 0.0;
        _minUsec = (1000000.0 * 1000);
        _startTime.tv_sec = 0;
        _startTime.tv_usec = 0;
        _endTime.tv_sec = 0;
        _endTime.tv_usec = 0;
    }

    void StampStart()
    {
        gettimeofday(&_startTime, NULL);
    }

    void StampEnd()
    {
        gettimeofday(&_endTime, NULL);
    }

    void Add()
    {
        double sec = GetTimeDiffInSec(&_startTime, &_endTime);
        double usec = GetTimeDiffIUsec(&_startTime, &_endTime);
        double acc_usec = (sec * 1000000) + usec;

        _totalUsec += acc_usec;

        if (acc_usec > _maxUsec)
            _maxUsec = acc_usec;
        if (acc_usec < _minUsec)
            _minUsec = acc_usec;
    }

    void Dump(int count)
    {
        CONSOLE_INFO(PERF_TAG, "TOTAL SPEND USEC   (us)=", _totalUsec);

        _averageUsec = (_totalUsec / count);
        CONSOLE_INFO(PERF_TAG, "AVERAGE SPEND USEC (us)=", _averageUsec);
        _averageMsec = (_averageUsec / 1000);
        CONSOLE_INFO(PERF_TAG, "AVERAGE SPEND MSEC (ms)=", _averageMsec, "(v)");

        // CONSOLE_INFO(PERF_TAG, "MAXIMUM SPEND USEC (us)=", _maxUsec);
        CONSOLE_INFO(PERF_TAG, "MAXIMUM SPEND MSEC (ms)=", (_maxUsec / 1000));
        // CONSOLE_INFO(PERF_TAG, "MINIMUM SPEND USEC (us)=", _minUsec);
        CONSOLE_INFO(PERF_TAG, "MINIMUM SPEND MSEC (ms)=", (_minUsec / 1000));
    }

  private:
    double GetTimeDiffInSec(struct timeval* start, struct timeval* end)
    {
        return (double)((end->tv_usec > start->tv_usec)
                            ? (end->tv_sec - start->tv_sec)
                            : (end->tv_sec - start->tv_sec - 1));
    }

    double GetTimeDiffIUsec(struct timeval* start, struct timeval* end)
    {
        return (double)((end->tv_usec > start->tv_usec)
                            ? (end->tv_usec - start->tv_usec)
                            : (1000000 + end->tv_usec - start->tv_usec));
    }

  private:
    struct timeval _startTime;
    struct timeval _endTime;
    double _totalUsec;
    double _averageUsec;
    double _averageMsec;
    double _maxUsec;
    double _minUsec;
};

class PerfOperation
{
  public:
    PerfOperation(bool waitRegionOff, int perfCounts)
    {
        this->_waitRegionOff = waitRegionOff;
        this->_perfCounts = perfCounts;
        this->_perfTime.Reset();
    }
    virtual ~PerfOperation(){};

  public:
    int Run()
    {
        int ret;
        int idx;

        if (_perfCounts > PERF_MAX_ALLOC_COUNT) {
            CONSOLE_ERR("invalid perf counts");
            return -1;
        }

        SetUp();

        for (idx = 0; idx < _perfCounts; idx++) {
            SendStartCommandPreInit();
            ret = SendStartCommand(idx);
            SendStartCommandPostInit();
            if (ret != 0)
                return ret;

            ret = SendEndCommand(idx);
            SendEndCommandPostInit();
            if (ret != 0)
                return ret;
        }

        TearDown();
        return 0;
    }

    int GetPerfCounts()
    {
        return _perfCounts;
    }

  private:
    void SetUp()
    {
        WaitRegionOff();
        _perfTime.Reset();
    }

    void TearDown()
    {
        WaitRegionOff();
        _perfTime.Dump(_perfCounts);
    }

    void SendStartCommandPreInit()
    {
        _perfTime.StampStart();
    }

    void SendStartCommandPostInit()
    {
        _perfTime.StampEnd();
        _perfTime.Add();
    }

    void WaitRegionOff()
    {
        if (_waitRegionOff)
            sleep(2);
    }

    void SendEndCommandPostInit()
    {
        WaitRegionOff();
    }

  protected:
    virtual int SendStartCommand(int idx);
    virtual int SendEndCommand(int idx);

  protected:
    int _perfCounts;
    bool _waitRegionOff;
    PerfTime _perfTime;
};

class IonPerfOperation : public PerfOperation
{
  public:
    IonPerfOperation(PerfDeviceHelper* device, int wait_region_off,
                     int perf_counts)
        : PerfOperation(wait_region_off, perf_counts)
    {
        this->_device = device;
    }

    int SendStartCommand(int idx)
    {
        return _device->IonAllocByIndex(idx);
    }

    int SendEndCommand(int idx)
    {
        if (_device->IonQueryHandleByIndex(idx) != 0)
            return -1;
        if (_device->IonFreeByIndex(idx) != 0)
            return -1;
        return 0;
    }

  private:
    PerfDeviceHelper* _device;
};

class TmemPerfOperation : public PerfOperation
{
  public:
    TmemPerfOperation(PerfDeviceHelper* device, int cmd_on, int cmd_off,
                      bool wait_region_off, int perf_counts)
        : PerfOperation(wait_region_off, perf_counts)
    {
        this->_device = device;
        this->_start_cmd = cmd_on;
        this->_end_cmd = cmd_off;
    }

    int SendStartCommand(int idx)
    {
        (void)idx;
        return _device->TmemSendCommand(_start_cmd);
    }

    int SendEndCommand(int idx)
    {
        (void)idx;
        return _device->TmemSendCommand(_end_cmd);
    }

  private:
    PerfDeviceHelper* _device;
    int _start_cmd;
    int _end_cmd;
};

class PerfEnvironment : public testing::Environment
{
  public:
    PerfEnvironment()
    {
        perfDevice = new PerfDeviceHelper();
    }
    ~PerfEnvironment()
    {
        delete perfDevice;
    }

    virtual void SetUp()
    {
        CONSOLE_INFO(PERF_TAG, "PerfEnvironment SetUp!\n");
        perfDevice->SetUp();
    }

    virtual void TearDown()
    {
        CONSOLE_INFO(PERF_TAG, "PerfEnvironment TearDown!\n");
        perfDevice->TearDown();
    }

    int IonOperations(bool wait_region_off, int perf_counts)
    {
        int res;
        PerfOperation* perf_ops =
            new IonPerfOperation(perfDevice, wait_region_off, perf_counts);

        res = perf_ops->Run();
        delete perf_ops;
        return res;
    }

    int TmemOperations(int cmd_on, int cmd_off, bool wait_region_off,
                       int perf_counts)
    {
        int res;
        PerfOperation* perf_ops = new TmemPerfOperation(
            perfDevice, cmd_on, cmd_off, wait_region_off, perf_counts);

        res = perf_ops->Run();
        delete perf_ops;
        return res;
    }

  public:
    PerfDeviceHelper* perfDevice;
};

PerfEnvironment* g_perfEnv;

class PmemPerfSingleTest : public ::testing::Test
{
  protected:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

TEST_F(PmemPerfSingleTest, ion_first_alloc_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonAllocByIndex(0));
};

TEST_F(PmemPerfSingleTest, ion_first_query_handle_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonQueryHandleByIndex(0));
};

TEST_F(PmemPerfSingleTest, ion_first_free_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonFreeAll());
};

TEST_F(PmemPerfSingleTest, ion_first_alloc_query_handle_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonAllocByIndex(0));
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonQueryHandleByIndex(0));
};

TEST_F(PmemPerfSingleTest, ion_second_alloc_query_handle_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonAllocByIndex(1));
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonQueryHandleByIndex(1));
};

TEST_F(PmemPerfSingleTest, ion_second_free_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonFreeAll());
};

TEST_F(PmemPerfSingleTest, ion_continuous_alloc_query_handle_Success)
{
    CONSOLE_INFO(PERF_TAG, "ION CONTINUOUS ALLOC COUNTS=", PERF_MAX_ALLOC_COUNT);
    ASSERT_EQ(0, g_perfEnv->IonOperations(false, PERF_MAX_ALLOC_COUNT));
};

TEST_F(PmemPerfSingleTest, IonFreeAll_Success)
{
    ASSERT_EQ(0, g_perfEnv->perfDevice->IonFreeAll());
};

#define PERF_MAX_ION_REGION_ON_OFF_COUNT (16)
TEST_F(PmemPerfSingleTest, ion_discontinuous_region_on_Success)
{
    CONSOLE_INFO(PERF_TAG, "ION DISCONTINUOUS REGION ON/OFF COUNTS=",
                 PERF_MAX_ION_REGION_ON_OFF_COUNT);
    ASSERT_EQ(0,
              g_perfEnv->IonOperations(true, PERF_MAX_ION_REGION_ON_OFF_COUNT));
};

#define TMEM_PMEM_SSMR_ALLOC 10
#define TMEM_PMEM_SSMR_RELEASE 11
#define PERF_MAX_TMEM_SSMR_ON_OFF_COUNT (16)
TEST_F(PmemPerfSingleTest, tmem_continuous_ssmr_on_Success)
{
    CONSOLE_INFO(PERF_TAG, "TMEM SSMR ON/OFF COUNTS=",
                 PERF_MAX_TMEM_SSMR_ON_OFF_COUNT);
    ASSERT_EQ(0, g_perfEnv->TmemOperations(TMEM_PMEM_SSMR_ALLOC,
                                           TMEM_PMEM_SSMR_RELEASE, false,
                                           PERF_MAX_TMEM_SSMR_ON_OFF_COUNT));
};

#define TMEM_PMEM_SESSION_OPEN 12
#define TMEM_PMEM_SESSION_CLOSE 13
#define PERF_MAX_TMEM_SESSION_ON_OFF_COUNT (16)
TEST_F(PmemPerfSingleTest, tmem_continuous_session_on_Success)
{
    CONSOLE_INFO(PERF_TAG, "TMEM SESSION ON/OFF COUNTS=",
                 PERF_MAX_TMEM_SESSION_ON_OFF_COUNT);
    ASSERT_EQ(0, g_perfEnv->TmemOperations(TMEM_PMEM_SESSION_OPEN,
                                           TMEM_PMEM_SESSION_CLOSE, false,
                                           PERF_MAX_TMEM_SESSION_ON_OFF_COUNT));
};

#define TMEM_PMEM_REGION_ON 14
#define TMEM_PMEM_REGION_OFF 15
#define PERF_MAX_TMEM_REGION_ON_OFF_COUNT (16)
TEST_F(PmemPerfSingleTest, tmem_discontinuous_region_on_Success)
{
    CONSOLE_INFO(PERF_TAG, "TMEM REGION ON/OFF COUNTS=",
                 PERF_MAX_TMEM_REGION_ON_OFF_COUNT);
    ASSERT_EQ(
        0, g_perfEnv->TmemOperations(TMEM_PMEM_REGION_ON, TMEM_PMEM_REGION_OFF,
                                     true, PERF_MAX_TMEM_REGION_ON_OFF_COUNT));
};

int main(int argc, char* argv[])
{
    g_perfEnv = new PerfEnvironment;
    testing::AddGlobalTestEnvironment(g_perfEnv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
