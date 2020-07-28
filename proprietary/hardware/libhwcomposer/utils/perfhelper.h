#ifndef HWC_PERF_HELPER_H__
#define HWC_PERF_HELPER_H__

#include <stdint.h>
#include <mutex>

#include <mtkperf_resource.h>

typedef int (*PerfLockAcq)(int handle, int duration, int list[], int num_args);
typedef int (*PerfLockRel)(int handle);

class PerfHelper {
public:
    // singleton function
    static PerfHelper* getInstance()
    {
        static std::mutex s_mutex;
        static PerfHelper* s_instance = nullptr;
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_instance == nullptr)
        {
            s_instance = new PerfHelper();
        }
        return s_instance;
    }

    // used to acquire the specific performance source
    void perfLockAcq(int handle, int duration, int list[], int num_args);

    // used to release the performance source
    void perfLockRel(int handle);

protected:
    PerfHelper();
    ~PerfHelper();

private:
    int loadApi();
    void clearApi();

private:
    // handle of dlopen latency library
    void* m_so_handle;

    // the api of mtkperf
    PerfLockAcq m_fn_perf_lock_acq;
    PerfLockRel m_fn_perf_lock_rel;
};

#endif
