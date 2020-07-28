#ifndef UTILS_DEBUG_H_
#define UTILS_DEBUG_H_

#include <map>

#include <cutils/log.h>

#include <utils/StrongPointer.h>
#include <utils/LightRefBase.h>
#include <utils/Singleton.h>
#include <utils/String8.h>
#include <ui/Rect.h>
#include <ged/ged_log.h>
#include <hardware/hwcomposer_defs.h>

#include <unordered_map>

#ifndef DEBUG_LOG_TAG
#error "DEBUG_LOG_TAG is not defined!!"
#endif

#define HWC_LOGV(x, ...)                                    \
        {                                                   \
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG,       \
                            'V',                            \
                            "[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__);        \
        }

#define HWC_LOGD(x, ...)                                    \
        {                                                   \
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG,       \
                            'D',                            \
                            "[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__);        \
        }

#define HWC_LOGI(x, ...)                                    \
        {                                                   \
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG,       \
                            'I',                            \
                            "[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__);        \
        }

#define HWC_LOGW(x, ...)                                    \
        {                                                   \
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG,       \
                            'W',                            \
                            "[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__);        \
        }

#define HWC_LOGE(x, ...)                                    \
        {                                                   \
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG,       \
                            'E',                            \
                            "[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__);        \
        }

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#define LIKELY( exp )       (__builtin_expect( (exp) != 0, true  ))
#define UNLIKELY( exp )     (__builtin_expect( (exp) != 0, false ))

enum HWC_DEBUG_COMPOSE_LEVEL
{
    COMPOSE_ENABLE_ALL  = 0,
    COMPOSE_DISABLE_MM  = 1,
    COMPOSE_DISABLE_UI  = 2,
    COMPOSE_DISABLE_ALL = 3
};

enum HWC_DEBUG_DUMP_LEVEL
{
    DUMP_NONE = 0,
    DUMP_MM   = 1,
    DUMP_UI   = 2,
    DUMP_SYNC = 4,
    DUMP_ALL  = 7
};

enum HWC_PROFILING_LEVEL
{
    PROFILE_NONE = 0,
    PROFILE_COMP = 1,
    PROFILE_BLT  = 2,
    PROFILE_TRIG = 4,
};

#ifdef USE_SYSTRACE
#define HWC_ATRACE_CALL() android::ScopedTrace ___tracer(ATRACE_TAG, __FUNCTION__)
#define HWC_ATRACE_NAME(name) android::ScopedTrace ___tracer(ATRACE_TAG, name)
#define HWC_ATRACE_INT(name, value) atrace_int(ATRACE_TAG, name, value)
#define HWC_ATRACE_ASYNC_BEGIN(name, cookie) atrace_async_begin(ATRACE_TAG, name, cookie)
#define HWC_ATRACE_ASYNC_END(name, cookie) atrace_async_end(ATRACE_TAG, name, cookie)

#ifndef MTK_BASIC_PACKAGE
#ifdef ATRACE_TAG_PERF
#define HWC_ATRACE_FORMAT_NAME(name, ...) FormatScopedTrace ___tracer(ATRACE_TAG|ATRACE_TAG_PERF, name, ##__VA_ARGS__);
#else
#define HWC_ATRACE_FORMAT_NAME(name, ...) FormatScopedTrace ___tracer(ATRACE_TAG, name, ##__VA_ARGS__);
#endif
#else // MTK_BASIC_PACKAGE
#define HWC_ATRACE_FORMAT_NAME(name, ...) FormatScopedTrace ___tracer(ATRACE_TAG, name, ##__VA_ARGS__);
#endif // MTK_BASIC_PACKAGE

#else // USE_SYSTRACE
#define HWC_ATRACE_CALL()
#define HWC_ATRACE_NAME(name)
#define HWC_ATRACE_INT(name, value)
#define HWC_ATRACE_ASYNC_BEGIN(name, cookie)
#define HWC_ATRACE_ASYNC_END(name, cookie)
#define HWC_ATRACE_FORMAT_NAME(name, ...)
#endif // USE_SYSTRACE

struct dump_buff
{
    char *msg;
    int msg_len;
    int len;
};

//=================================================================================================
//  DbgLogBufManager, DbgLogger and Debugger
//=================================================================================================

#define DBG_LOGGER_BUF_LEN 256

using namespace android;

//=================================================================================================
class DbgLogBufManager : public Singleton<DbgLogBufManager>
{
public:
    struct DBG_BUF
    {
        DBG_BUF()
            : addr(NULL)
            , len(0)
            , idx(UNDEFINED)
        {}
        char* addr;
        int len;
        int idx;
    };
    DbgLogBufManager();
    ~DbgLogBufManager();
    void getLogBuf(DBG_BUF* dbg_buf);
    void releaseLogBuf(DBG_BUF* dbg_buf);
    void dump(String8* dump_str);

private:
    enum PREALLOCAED_LOG_BUF
    {
        SELF_ALLOCATED = -1,
        UNDEFINED = -2,
        TMP_BUF_CNT = 99,
    };

    int m_log_buf_uesd;
    int m_log_buf_slot[TMP_BUF_CNT];
    char* m_log_pool[TMP_BUF_CNT];
    Mutex m_mutex;
};

//=================================================================================================
class FormatScopedTrace
{
public:
    inline FormatScopedTrace(uint64_t tag, const char* format, ...)
    {
        m_tag = tag;
        va_list args;
        va_start(args, format);
        vsnprintf(m_str, 128, format, args);
        atrace_begin(m_tag, m_str);
        va_end(args);
    }
    inline ~FormatScopedTrace()
    {
        atrace_end(m_tag);
    }
private:
    char m_str[128];
    uint64_t m_tag;
};

//=================================================================================================
class DbgLogger
{
public:
    enum DBG_LOGGER_TYPE
    {
        TYPE_NONE       = 0x00,
        TYPE_HWC_LOG    = 0x01 << 0,
        TYPE_HWC_LOGE   = 0x01 << 1,
        TYPE_GED_LOG    = 0x01 << 2,
        TYPE_DUMPSYS    = 0x01 << 3,
        TYPE_STATIC     = 0x01 << 4,
        TYPE_FENCE      = 0x01 << 5,
        TYPE_HIDE       = 0x01 << 6,
        TYPE_PERIOD     = 0x01 << 7,
    };

    DbgLogger(const uint32_t& type, const unsigned char& level = 'D', const char *fmt = nullptr, ...);
    ~DbgLogger();

    void printf(const char *fmt, ...);
    void getBuffer();
    void flushOut(char mark = ' ');
    void tryFlush();
    char* getLogString();
    int getLen() const { return m_len; };

private:
    unsigned char getLogLevel() const;

    DbgLogBufManager::DBG_BUF m_buf;
    DbgLogBufManager::DBG_BUF m_bak_buf;
    int m_len;
    uint32_t m_type;
    nsecs_t m_last_flush_out;
    unsigned char m_level;
};

//=================================================================================================
class Debugger : public Singleton<Debugger>
{
public:
    struct LOGGER : public android::LightRefBase<LOGGER>
    {
        LOGGER();

        ~LOGGER();

        int m_max_inputs;
        DbgLogger* dumpsys;
        DbgLogger* pre_info[HWC_NUM_DISPLAY_TYPES];
        DbgLogger* set_info[HWC_NUM_DISPLAY_TYPES];
        DbgLogger** ovlInput[HWC_NUM_DISPLAY_TYPES];
    };

    Debugger();
    ~Debugger();
    void dump(String8* dump_str);
    inline GED_LOG_HANDLE getGedHandleHWCErr() { return m_ged_log_handle_hwc_err; }
    inline GED_LOG_HANDLE getGedHandleHWC() { return m_ged_log_handle_hwc; }
    inline GED_LOG_HANDLE getGedHandleFENCE() { return m_ged_log_handle_fence; }
    static int m_skip_log;

    sp<LOGGER> m_logger;

    uint32_t statistics_displayFrame_over_range;
    bool checkLevel(const unsigned char& level);
    void setLogThreshold(const unsigned char& log_threshold);
    unsigned char getLogThreshold();
private:
    GED_LOG_HANDLE m_ged_log_handle_fence;
    GED_LOG_HANDLE m_ged_log_handle_hwc_err;
    GED_LOG_HANDLE m_ged_log_handle_hwc;

    unsigned char m_log_threshold;
    std::map<unsigned char, int32_t> m_log_level_precedence;
};

void dump_buf(
    const uint32_t& format,
    const int32_t& ion_fd,
    const int32_t& stride,
    const Rect& crop,
    const int32_t& downsample,
    const char* prefix,
    const bool& log_enable);

struct FrameFenceInfo
{
    FrameFenceInfo()
        : job_id(0)
        , acquire_fence_fd(-1)
        , present_fence_fd(-1)
        , HWC_config_MDP_time(0)
        , HWC_expect_MDP_finish_time(0)
    {
    }

    unsigned int job_id;
    int acquire_fence_fd;
    int present_fence_fd;
    nsecs_t HWC_config_MDP_time;
    nsecs_t HWC_expect_MDP_finish_time;
};

class MDPFrameInfoDebugger : public Singleton<MDPFrameInfoDebugger>
{
public:
    MDPFrameInfoDebugger();
    ~MDPFrameInfoDebugger();

    void insertJob(const int& job_id);
    void setJobAcquireFenceFd(const int& job_id, const int& fd);
    void setJobPresentFenceFd(const int& job_id, const int& fd);
    void setJobHWCExpectMDPFinsihTime(const int& job_id, const nsecs_t& time);
    void setJobHWCConfigMDPTime(const int& job_id, const nsecs_t& time);
    void checkMDPLayerExecTime();

private:
    mutable Mutex m_layer_frame_fence_info_lock;
    std::unordered_map<uint32_t, FrameFenceInfo> m_layer_frame_fence_info;

    nsecs_t m_avg_pre_minus_acq_time;
    uint32_t m_avg_count;
    uint32_t m_error_count;
    uint32_t m_sample_count;
};

#endif // UTILS_DEBUG_H_
