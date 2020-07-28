#define DEBUG_LOG_TAG "DBG"
#define LOG_NDEBUG 0

#include <stdio.h>

#include <sys/mman.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <ion.h>

#include <cutils/properties.h>
#include "graphics_mtk_defs.h"
#include "utils/debug.h"
#include "utils/tools.h"

int Debugger::m_skip_log = 1;

ANDROID_SINGLETON_STATIC_INSTANCE(Debugger);
ANDROID_SINGLETON_STATIC_INSTANCE(DbgLogBufManager);
ANDROID_SINGLETON_STATIC_INSTANCE(MDPFrameInfoDebugger);

//=================================================================================================
DbgLogBufManager::DbgLogBufManager()
{
    m_log_buf_uesd = 0;
    for (int i = 0; i < TMP_BUF_CNT; i++)
    {
        m_log_buf_slot[i] = i;
        m_log_pool[i] = NULL;
    }
}

DbgLogBufManager::~DbgLogBufManager()
{
    for (int i = 0; i < TMP_BUF_CNT; i++)
    {
        if (NULL != m_log_pool[i])
            free(m_log_pool[i]);
    }
}

void DbgLogBufManager::getLogBuf(DBG_BUF* dbg_buf)
{
    if (dbg_buf->addr != NULL)
        return;

    Mutex::Autolock _l(m_mutex);
    if (m_log_buf_uesd < TMP_BUF_CNT)
    {
        int id = m_log_buf_slot[m_log_buf_uesd++];

        if (NULL == m_log_pool[id])
        {
            m_log_pool[id] = (char*)malloc(DBG_LOGGER_BUF_LEN * sizeof(char));
            LOG_ALWAYS_FATAL_IF(m_log_pool[id] == nullptr, "DbgLog pool malloc(%zu) fail",
                DBG_LOGGER_BUF_LEN * sizeof(char));
        }

        dbg_buf->idx = id;
        dbg_buf->addr = m_log_pool[id];
    }
    else
    {
        dbg_buf->idx = SELF_ALLOCATED;
        dbg_buf->addr = (char*)malloc(DBG_LOGGER_BUF_LEN * sizeof(char));
        LOG_ALWAYS_FATAL_IF(dbg_buf->addr == nullptr, "Dbg buf malloc(%zu) fail",
            DBG_LOGGER_BUF_LEN * sizeof(char));
    }

    dbg_buf->len = DBG_LOGGER_BUF_LEN;
    return;
}

void DbgLogBufManager::releaseLogBuf(DBG_BUF* dbg_buf)
{
    if (dbg_buf->addr == NULL)
        return;

    if (SELF_ALLOCATED == dbg_buf->idx)
    {
        free(dbg_buf->addr);
    }
    else
    {
        Mutex::Autolock _l(m_mutex);
        int id = dbg_buf->idx;
        m_log_buf_slot[--m_log_buf_uesd] = id;
    }

    dbg_buf->addr = NULL;
    dbg_buf->len = 0;
}

void DbgLogBufManager::dump(String8* dump_str)
{
    Mutex::Autolock _l(m_mutex);
    dump_str->appendFormat("  DBGLBM: %d/%d used\n", m_log_buf_uesd, TMP_BUF_CNT);
}

//=================================================================================================
DbgLogger::DbgLogger(const uint32_t& type, const unsigned char& level, const char *fmt, ...)
    : m_len(0)
    , m_type(type)
    , m_last_flush_out(0)
    , m_level(level)
{
    // when level is under threshold, hwc should skip snprintf() because of performance
    if (Debugger::getInstance().getLogThreshold() != 'V' && getLogLevel() == 'V')
        return;

    getBuffer();

    if (NULL == m_buf.addr)
        return;

    if (m_len > m_buf.len - 1)
        return;

    if (fmt != nullptr)
    {
        va_list ap;
        va_start(ap, fmt);
        m_len += vsnprintf(m_buf.addr + m_len, m_buf.len - m_len, fmt, ap);
        va_end(ap);
    }
}

DbgLogger::~DbgLogger()
{
    if (NULL == m_buf.addr)
        return;

    flushOut();
    DbgLogBufManager::getInstance().releaseLogBuf(&m_buf);
}

void DbgLogger::getBuffer()
{
    if (TYPE_NONE == m_type)
        return;

    DbgLogBufManager::getInstance().getLogBuf(&m_buf);
    if (m_type & TYPE_PERIOD)
    {
        DbgLogBufManager::getInstance().getLogBuf(&m_bak_buf);
    }
    m_len = 0;
    m_buf.addr[0] = '\0';
}

void DbgLogger::printf(const char *fmt, ...)
{
    if (NULL == m_buf.addr)
        return;

    if (m_len > m_buf.len - 1)
        return;

    va_list ap;
    va_start(ap, fmt);
    m_len += vsnprintf(m_buf.addr + m_len, m_buf.len  - m_len, fmt, ap);
    va_end(ap);
}

void DbgLogger::flushOut(char mark)
{
    if (NULL == m_buf.addr)
        return;

    bool output_main_log = false;

    if (0 != (m_type & TYPE_HWC_LOG))
    {
        if (getLogLevel() == 'I' || getLogLevel() == 'W' || getLogLevel() == 'E' || getLogLevel() == 'F')
            ged_log_tpt_print(Debugger::getInstance().getGedHandleHWCErr(), "%c %s %c", m_level, m_buf.addr, mark);

        output_main_log = (m_type & TYPE_PERIOD) ?
#ifdef MTK_USER_BUILD
            Debugger::m_skip_log != 1 : Debugger::getInstance().checkLevel(getLogLevel());
#else
            mark != '@' || Debugger::m_skip_log != 1 : Debugger::getInstance().checkLevel(getLogLevel());
#endif

        if (output_main_log)
        {
            switch (getLogLevel())
            {
                case 'F':
                case 'E':
                    ALOGE("%s %c", m_buf.addr, mark);
                    break;

                case 'W':
                    ALOGW("%s %c", m_buf.addr, mark);
                    break;

                case 'I':
                    ALOGI("%s %c", m_buf.addr, mark);
                    break;

                case 'D':
                    ALOGD("%s %c", m_buf.addr, mark);
                    break;

                case 'V':
                    ALOGV("%s %c", m_buf.addr, mark);
                    break;

                default:
                    ALOGE("unknown log level(%c) %s", m_level, m_buf.addr);
            }
        }
        if (getLogLevel() != 'V' || Debugger::getInstance().getLogThreshold() == 'V')
            ged_log_tpt_print(Debugger::getInstance().getGedHandleHWC(), "%c %s %c", m_level, m_buf.addr, mark);
    }

    if (0 != (m_type & TYPE_DUMPSYS))
        Debugger::getInstance().m_logger->dumpsys->printf("\n  %s", m_buf.addr);

    if (0 != (m_type & TYPE_FENCE))
        ged_log_tpt_print(Debugger::getInstance().getGedHandleFENCE(),"%s", m_buf.addr);

    if ((m_type & TYPE_PERIOD))
    {
        if (mark == '!')
        {
            strncpy(m_bak_buf.addr, m_buf.addr, m_bak_buf.len - 1);
            m_bak_buf.addr[m_bak_buf.len - 1] = '\0';
        }

        if (output_main_log)
            m_last_flush_out = systemTime();
    }
    m_len = 0;
    m_buf.addr[0] = '\0';
}

void DbgLogger::tryFlush()
{
    const nsecs_t PERIOD_THRESHOLD = 5e+8;
    if (NULL == m_buf.addr)
        return;

    nsecs_t now = systemTime();
    char mark = ' ';
    if ((m_type & TYPE_PERIOD) && (1 == Debugger::m_skip_log))
    {
        if (NULL == m_bak_buf.addr)
            return;

        if (0 == strcmp(m_bak_buf.addr, m_buf.addr))
        {
            mark = (now - m_last_flush_out) < PERIOD_THRESHOLD ? '@' : '=';
        }
        else
        {
            mark = '!';
        }
    }
    flushOut(mark);
}

char* DbgLogger::getLogString()
{
    return m_buf.addr;
}

unsigned char DbgLogger::getLogLevel() const
{
    return m_level;
}
//=================================================================================================
Debugger::Debugger()
    : statistics_displayFrame_over_range(0)
    , m_log_threshold('I')
{
    m_log_level_precedence['V'] = 0;
    m_log_level_precedence['D'] = 1;
    m_log_level_precedence['I'] = 2;
    m_log_level_precedence['W'] = 3;
    m_log_level_precedence['E'] = 4;
    m_log_level_precedence['F'] = 5;

    m_ged_log_handle_hwc_err = ged_log_connect("HWC_err");
    m_ged_log_handle_hwc = ged_log_connect("HWC");
    m_ged_log_handle_fence = ged_log_connect("FENCE");
}

Debugger::~Debugger()
{
    ged_log_disconnect(m_ged_log_handle_hwc_err);
    ged_log_disconnect(m_ged_log_handle_hwc);
    ged_log_disconnect(m_ged_log_handle_fence);
}

void Debugger::dump(String8* dump_str)
{
    dump_str->appendFormat("[HWC DBG Dump] log threshold:%c", getLogThreshold());

    char* string_dumpsys = m_logger->dumpsys->getLogString();
    if (NULL == string_dumpsys)
        dump_str->appendFormat("%s\n", "NULL STRING");
    else
        dump_str->appendFormat("%s\n", string_dumpsys);

    DbgLogBufManager::getInstance().dump(dump_str);

    dump_str->appendFormat("\n[HWC Statistics]\n");
    dump_str->appendFormat("  %d - displayFrame over range\n", statistics_displayFrame_over_range);
}

bool Debugger::checkLevel(const unsigned char& level)
{
    return m_log_level_precedence[level] >= m_log_level_precedence[getLogThreshold()];
}

void Debugger::setLogThreshold(const unsigned char& log_threshold)
{
    m_log_threshold = log_threshold;
}

unsigned char Debugger::getLogThreshold()
{
    static bool is_first_called = true;
    if (UNLIKELY(is_first_called))
    {
        char value[PROPERTY_VALUE_MAX];
        property_get("persist.vendor.debug.hwc.log", value, "0");
        if (value[0] != '0')
        {
            setLogThreshold(value[0]);
        }
        else
        {
            // For a fps-related demo, someone asks us to give a non-log hwcomposer.
            // In that case, a better way is to modify the string by a binary file editor.
            snprintf(value, PROPERTY_VALUE_MAX - 1, "persist.vendor.debug.hwc.log=I");
            setLogThreshold(value[strlen(value) - 1]);
        }
        is_first_called = false;
    }
    return m_log_threshold;
}

void dump_buf(
    const uint32_t& format,
    const int32_t& ion_fd,
    const int32_t& stride,
    const Rect& crop,
    const int32_t& downsample,
    const char* prefix,
    const bool& log_enable)
{
    static const int32_t dev_fd = ::open("/dev/ion", O_RDONLY);
    const int32_t height = HEIGHT(crop);
    const int32_t width = WIDTH(crop);
    const int32_t Bpp = getBitsPerPixel(format) / 8;
    const int32_t mmap_size = stride * crop.bottom * Bpp;
    const String8 path = String8::format("%s_w%dh%d_B%d_[x%d,y%d,w%d,h%d].%s",
        prefix,
        (width % downsample == 0) ? width / downsample : width / downsample + 1,
        (height % downsample == 0) ? height / downsample : height / downsample + 1,
        Bpp,
        crop.left, crop.top, WIDTH(crop), HEIGHT(crop),
        getFormatString(format).string());

    if (Bpp <= 0)
    {
        HWC_LOGE("%s wrong Bpp(%d) path:%s", __func__, Bpp, path.string());
        return;
    }

    ion_user_handle_t ion_hnd = -1;
    if (ion_import(dev_fd, ion_fd, &ion_hnd))
    {
        HWC_LOGE("ion_import is failed: %s, ion_fd(%d)", strerror(errno), ion_fd);
        ion_hnd = -1;
    }

    int32_t shared_fd = -1;
    if (ion_hnd != -1 && ion_share(dev_fd, ion_hnd, &shared_fd))
    {
        HWC_LOGE("ion_share is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        shared_fd = -1;
    }

    void *ptr = nullptr;
    if (ion_fd != -1 && shared_fd != -1)
    {
        ptr = ion_mmap(ion_fd, nullptr, mmap_size, PROT_READ, MAP_SHARED, shared_fd, 0);
    }

    if (ptr == nullptr || ptr == (void*)(-1))
        HWC_LOGE("ion mmap fail");

    FILE* fp = fopen(path.string(), "wb");

    if (fp && ptr != nullptr)
    {
        unsigned char* pptr = (unsigned char*)(ptr);
        unsigned char* pptr_offset = pptr + (crop.top * stride + crop.left) * Bpp;
        uint32_t curr_dump_height = 0;

        int32_t print_cnt = 0;
        while(pptr_offset < pptr + mmap_size)
        {
            if (log_enable && print_cnt < 3)
            {
                HWC_LOGI("pptr_offset-ptr:%" PRIu64 " crop[%d,%d,%d,%d] w:%d h:%d s:%d downsample:%d mmap_size:%d Bpp:%d",
                        static_cast<uint64_t>(pptr_offset - pptr),
                        crop.left, crop.top, crop.right, crop.bottom, width, height, stride,
                        downsample, mmap_size, Bpp);
                ++print_cnt;
            }
            const uint32_t a = *(uint32_t*)(pptr_offset);
            fwrite(&a, Bpp, 1, fp);
            pptr_offset += downsample * Bpp;
            if ((pptr_offset - pptr) >= int32_t(((crop.top + curr_dump_height) * stride + crop.left + width) * Bpp))
            {
                HWC_LOGI("last pptr_offset:%" PRIu64, static_cast<uint64_t>(pptr_offset  - pptr) -  downsample * Bpp);
                curr_dump_height += downsample;
                pptr_offset = pptr + ((crop.top + curr_dump_height) * stride + crop.left) * Bpp;
                HWC_LOGI("crop.top + curr_dump_height:%d curr_dump_height:%d", crop.top + curr_dump_height, curr_dump_height);
                print_cnt = 0;
            }
        }
        fclose(fp);
        fp = nullptr;
    }
    else
    {
        if (fp)
        {
            fclose(fp);
            fp = nullptr;
        }
        HWC_LOGE("open file failed");
    }

    if (ptr)
    {
        ion_munmap(ion_hnd, ptr, mmap_size);
        ptr = nullptr;
    }

    if (ion_hnd != -1 && ion_free(dev_fd, ion_hnd))
    {
        HWC_LOGE("ion_free is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        ion_hnd = -1;
    }
    if (shared_fd != -1 && ion_share_close(dev_fd, shared_fd))
    {
        HWC_LOGW("ion_share_close is failed: %s , share_fd(%d)", strerror(errno), shared_fd);
        shared_fd = -1;
    }

    HWC_LOGI("dump buf: %s format:%d Bpp:%d stride:%d width:%d height:%d downsample:%d",
        path.string(), format, Bpp, stride, width, height, downsample);
}

Debugger::LOGGER::LOGGER()
{
    m_max_inputs = getHwDevice()->getMaxOverlayInputNum();

    dumpsys = new DbgLogger(DbgLogger::TYPE_STATIC);
    for (int j = 0; j < DisplayManager::MAX_DISPLAYS; j++)
    {
        pre_info[j] = new DbgLogger(DbgLogger::TYPE_HWC_LOG | DbgLogger::TYPE_PERIOD);
        set_info[j] = new DbgLogger(DbgLogger::TYPE_HWC_LOG);

        ovlInput[j] = (DbgLogger**)malloc(m_max_inputs * sizeof(DbgLogger*));
        LOG_ALWAYS_FATAL_IF(ovlInput[j] == nullptr, "DbgLogger* malloc(%zu) fail",
                m_max_inputs * sizeof(DbgLogger*));
        for (int i = 0; i < m_max_inputs; i++)
            ovlInput[j][i] = new DbgLogger(DbgLogger::TYPE_HWC_LOG | DbgLogger :: TYPE_PERIOD, 'D');
    }
}

Debugger::LOGGER::~LOGGER()
{
    delete dumpsys;
    for (int j = 0; j < DisplayManager::MAX_DISPLAYS; j++)
    {
        delete pre_info[j];
        delete set_info[j];

        for (int i = 0; i < m_max_inputs; i++)
            delete ovlInput[j][i];

        delete ovlInput[j];
    }
}

MDPFrameInfoDebugger::MDPFrameInfoDebugger()
{
    m_layer_frame_fence_info.clear();
    m_avg_pre_minus_acq_time = 0;
    m_avg_count = 0;
    m_error_count = 0;
    m_sample_count = 0;
}

MDPFrameInfoDebugger::~MDPFrameInfoDebugger()
{
}

void MDPFrameInfoDebugger::insertJob(const int& job_id)
{
    AutoMutex l(m_layer_frame_fence_info_lock);
    FrameFenceInfo temp;
    temp.job_id = job_id;
    m_layer_frame_fence_info.insert({job_id, temp});
}

void MDPFrameInfoDebugger::setJobAcquireFenceFd(const int& job_id, const int& fd)
{
    AutoMutex l(m_layer_frame_fence_info_lock);
    auto temp = m_layer_frame_fence_info.find(job_id);
    if (temp != m_layer_frame_fence_info.end())
    {
        temp->second.acquire_fence_fd = fd;
    }
}

void MDPFrameInfoDebugger::setJobPresentFenceFd(const int& job_id, const int& fd)
{
    AutoMutex l(m_layer_frame_fence_info_lock);
    auto temp = m_layer_frame_fence_info.find(job_id);
    if (temp != m_layer_frame_fence_info.end())
    {
        temp->second.present_fence_fd = fd;
    }

}

void MDPFrameInfoDebugger::setJobHWCConfigMDPTime(const int& job_id, const nsecs_t& time)
{
    AutoMutex l(m_layer_frame_fence_info_lock);
    auto temp = m_layer_frame_fence_info.find(job_id);
    if (temp != m_layer_frame_fence_info.end())
    {
        temp->second.HWC_config_MDP_time = time;
    }
}

void MDPFrameInfoDebugger::setJobHWCExpectMDPFinsihTime(const int& job_id, const nsecs_t& time)
{
    AutoMutex l(m_layer_frame_fence_info_lock);
    auto temp = m_layer_frame_fence_info.find(job_id);
    if (temp != m_layer_frame_fence_info.end())
    {
        temp->second.HWC_expect_MDP_finish_time = time;
    }
}

void MDPFrameInfoDebugger::checkMDPLayerExecTime()
{
    AutoMutex l(m_layer_frame_fence_info_lock);

    nsecs_t curTime = systemTime();
    uint32_t removeStartJobID = 0;
    std::unordered_map<uint32_t, FrameFenceInfo>::iterator it;

    for (auto& info: m_layer_frame_fence_info)
    {
        nsecs_t acqTime = getFenceSignalTime(info.second.acquire_fence_fd);
        nsecs_t preTime = getFenceSignalTime(info.second.present_fence_fd);
        if (acqTime != SIGNAL_TIME_INVALID && acqTime != SIGNAL_TIME_PENDING &&
            preTime != SIGNAL_TIME_INVALID && preTime != SIGNAL_TIME_PENDING)
        {
            nsecs_t diff_pres_HWCExpectTime = (preTime > info.second.HWC_expect_MDP_finish_time)?
                    preTime - info.second.HWC_expect_MDP_finish_time:info.second.HWC_expect_MDP_finish_time-preTime;
            HWC_LOGI("[%d] p:%" PRIu64 ",m_HWCExpectDisplayTime:%" PRIu64 ",p-a diff:%" PRIu64 ",a-s diff:%" PRIu64 ",p-s diff:%" PRIu64 ",p-hwc diff:%" PRIu64 ,
                info.second.job_id,
                preTime,
                info.second.HWC_expect_MDP_finish_time,
                preTime - acqTime,
                acqTime - info.second.HWC_config_MDP_time,
                preTime - info.second.HWC_config_MDP_time,
                diff_pres_HWCExpectTime);

            if (m_avg_count < 100)
                m_avg_count++;

            m_avg_pre_minus_acq_time = (m_avg_pre_minus_acq_time*(m_avg_count -1 ) + (preTime - acqTime))/(m_avg_count);

            if (diff_pres_HWCExpectTime > 16 * 1000 * 1000)
                m_error_count++;

            m_sample_count++;
        }

        if (curTime > preTime && (preTime != SIGNAL_TIME_INVALID && preTime != SIGNAL_TIME_PENDING))
        {
            if (removeStartJobID == 0) {
                removeStartJobID = info.first;
            }
        }
    }

    HWC_LOGI("m_avg_pre_minus_acq_Time:%" PRIu64 ",m_count:%d,(%d/%d)", m_avg_pre_minus_acq_time, m_avg_count, m_error_count, m_sample_count);

    for (it = m_layer_frame_fence_info.find(removeStartJobID); it != m_layer_frame_fence_info.end(); ++it)
    {
        if ((*it).second.acquire_fence_fd != -1) {
            ::protectedClose((*it).second.acquire_fence_fd);
            (*it).second.acquire_fence_fd = -1;
        }
        if ((*it).second.present_fence_fd != -1) {
            ::protectedClose((*it).second.present_fence_fd);
            (*it).second.present_fence_fd = -1;
        }
    }

    if (m_layer_frame_fence_info.find(removeStartJobID) != m_layer_frame_fence_info.end()) {
        m_layer_frame_fence_info.erase(m_layer_frame_fence_info.find(removeStartJobID),
                                       m_layer_frame_fence_info.end());
    }
}


