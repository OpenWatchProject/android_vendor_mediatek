#define DEBUG_LOG_TAG "EVENT"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "hwc_priv.h"

#include <cutils/properties.h>

#include "utils/debug.h"
#include "utils/tools.h"

#include "event.h"
#include "display.h"
#include "hwc2.h"

#ifndef MTK_USER_BUILD
#define DEBUG_VSYNC_TIME
#endif

#define HWC_ATRACE_BUFFER(x, ...)                                               \
    if (ATRACE_ENABLED()) {                                                     \
        char ___traceBuf[256];                                                  \
        snprintf(___traceBuf, sizeof(___traceBuf), x, ##__VA_ARGS__);           \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);             \
    }

extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                                const struct timespec *request,
                                struct timespec *remain);

ANDROID_SINGLETON_STATIC_INSTANCE(HWVSyncEstimator);
// ---------------------------------------------------------------------------

VSyncThread::VSyncThread(int dpy)
    : m_disp_id(dpy)
    , m_enabled(false)
    , m_refresh(1e9/60)
    , m_loop(false)
    , m_fake_vsync(false)
    , m_prev_fake_vsync(0)
    , m_max_period_io(20)
    , m_max_period_req(500)
{
    snprintf(m_thread_name, sizeof(m_thread_name), "VSyncThread_%d", dpy);
}

VSyncThread::~VSyncThread()
{
}

void VSyncThread::initialize(bool force_sw_vsync, nsecs_t refresh)
{
    if (force_sw_vsync)
    {
        HWC_LOGI("Force to use sw vsync");
        m_fake_vsync = true;
    }

    if (refresh > 0)
    {
        const nsecs_t timeout = 4000000;
        m_refresh = refresh;
        m_max_period_io = m_refresh + timeout;
    }

    run(m_thread_name, PRIORITY_URGENT_DISPLAY);

    if (m_fake_vsync)
    {
        HWC_LOGD("(%d) HW VSync State(%d) sw vsync period:%d", m_disp_id, !m_fake_vsync, m_refresh);
    }
    else
    {
        HWC_LOGD("(%d) HW VSync State(%d)", m_disp_id, !m_fake_vsync);
    }
}

#ifdef DEBUG_VSYNC_TIME
nsecs_t g_time_prev = systemTime();
#endif

bool VSyncThread::threadLoop()
{
    bool is_enabled = false;

    {
        Mutex::Autolock _l(m_lock);
        while (!m_enabled && !m_loop)
        {
            m_state = HWC_THREAD_IDLE;
            m_condition.wait(m_lock);
            if (exitPending()) return false;
        }

        m_state = HWC_THREAD_TRIGGER;
        is_enabled = m_enabled;
        m_loop = false;
    }

    nsecs_t next_vsync = 0;

    bool use_fake_vsync = m_fake_vsync;
    if (!use_fake_vsync)
    {
#ifdef DEBUG_VSYNC_TIME
        const nsecs_t time_prev = systemTime();
#endif

        HWC_ATRACE_NAME("wait_vsync");

        int err = HWCMediator::getInstance().getOvlDevice(m_disp_id)->waitVSync(m_disp_id, &next_vsync);
        if (err == NO_ERROR)
        {
            static nsecs_t prev_next_vsync = 0;
            HWC_ATRACE_BUFFER("period: %" PRId64, next_vsync - prev_next_vsync)
            prev_next_vsync = next_vsync;
#ifdef DEBUG_VSYNC_TIME
            const nsecs_t time_curr = systemTime();
            const nsecs_t dur1 = time_curr - g_time_prev;
            const nsecs_t dur2 = time_curr - time_prev;
            g_time_prev = time_curr;
            long p1 = ns2ms(dur1);
            long p2 = ns2ms(dur2);
            if (p1 > m_max_period_req || p2 > m_max_period_io)
                HWC_LOGD("vsync/dpy=%d/req=%ld/io=%ld", m_disp_id, p1, p2);
#endif
        }
        else
        {
            // use fake vsync since fail to get hw vsync
            use_fake_vsync = true;
        }
    }

    if (use_fake_vsync)
    {
        const nsecs_t& period = m_refresh;
        const nsecs_t now = systemTime(CLOCK_MONOTONIC);
        next_vsync = m_prev_fake_vsync + period;
        nsecs_t sleep = next_vsync - now;

        if (sleep < 0)
        {
            sleep = (period - ((now - next_vsync) % period));
            next_vsync = now + sleep;
        }

        struct timespec spec;
        spec.tv_sec  = next_vsync / 1000000000;
        spec.tv_nsec = next_vsync % 1000000000;


        HWC_LOGD("(%d) use SW VSync sleep: %.2f ms", m_disp_id, sleep / 1000000.0);
        int err;
        do {

            err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
        } while (err<0 && errno == EINTR);

        m_prev_fake_vsync = next_vsync;
    }

    DisplayManager::getInstance().vsync(m_disp_id, next_vsync, is_enabled);

    return true;
}

void VSyncThread::setEnabled(bool enabled)
{
    Mutex::Autolock _l(m_lock);

#ifdef DEBUG_VSYNC_TIME
    if (m_enabled != enabled)
        HWC_LOGD("vsync/dpy=%d/en=%d", m_disp_id, enabled);
#endif

    m_enabled = enabled;
    m_condition.signal();
}

void VSyncThread::setLoopAgain()
{
    Mutex::Autolock _l(m_lock);

#ifdef DEBUG_VSYNC_TIME
    HWC_LOGD("Set Loop Again");
#endif

    m_loop = true;
    m_condition.signal();
}

void VSyncThread::setProperty()
{
    char value[PROPERTY_VALUE_MAX];

    property_get("vendor.debug.sf.sw_vsync_fps", value, "0");
    int fps = atoi(value);
    if (fps > 0)
    {
        m_refresh = nsecs_t(1e9 / fps);
        HWC_LOGD("Set sw vsync fps(%d), period(%d)", __func__, fps, m_refresh);
    }

    property_get("vendor.debug.hwc.period_io", value, "0");
    if (atoi(value))
    {
        m_max_period_io = atoi(value);
        HWC_LOGD("Set max checking period_io(%d)", m_max_period_io);
    }

    property_get("vendor.debug.hwc.period_req", value, "0");
    if (atoi(value))
    {
        m_max_period_req = atoi(value);
        HWC_LOGD("Set max checking period_req(%d)", m_max_period_io);
    }
}

// ---------------------------------------------------------------------------
#define UEVENT_BUFFER_SIZE 2048

UEventThread::UEventThread()
    : m_socket(-1)
    , m_is_hotplug(false)
    , m_fake_hdmi(FAKE_HDMI_NONE)
    , m_fake_hotplug(false)
{
}

UEventThread::~UEventThread()
{
    if (m_socket > 0) ::protectedClose(m_socket);
}

void UEventThread::initialize()
{
    struct sockaddr_nl addr;
    int optval = 64 * 1024;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    m_socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (m_socket < 0)
    {
        HWC_LOGE("Failed to create socket");
        return;
    }

    if ((setsockopt(m_socket, SOL_SOCKET, SO_RCVBUFFORCE, &optval, sizeof(optval)) < 0) &&
        (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) < 0))
    {
        HWC_LOGE("Failed to set SO_RCVBUFFORCE/SO_RCVBUF option on socket");
        return;
    }

    if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        HWC_LOGE("Failed to bind socket");
        return;
    }

    run("UEventThreadHWC", PRIORITY_URGENT_DISPLAY);

    HWC_LOGW("Start to listen uevent, addr.nl_pid(%d)", addr.nl_pid);
}

void UEventThread::handleUevents(const char *buff, int len)
{
    const char *s = buff;
    const int change_hdmi = !strcmp(s, "change@/devices/virtual/switch/hdmi");
    const int change_hdmi_res = !strcmp(s, "change@/devices/virtual/switch/res_hdmi");
    const int change_widevine = !strcmp(s, "change@/devices/virtual/switch/widevine");

    if (!change_hdmi && !change_hdmi_res && !change_widevine)
       return;

    HWC_LOGD("handle hdmi uevents: s=%p, len=%d", s, len);

    int state = 0;
    s += strlen(s) + 1;

    while (*s)
    {
        if (!strncmp(s, "SWITCH_STATE=", strlen("SWITCH_STATE=")))
        {
            state = atoi(s + strlen("SWITCH_STATE="));
            HWC_LOGD("uevents: SWITCH_STATE=%d", state);
        }

        HWC_LOGD("uevents: s=%p, %s", s, s);

        s += strlen(s) + 1;
        if (s - buff >= len)
            break;
    }

    if (change_hdmi)
    {
        if (state == 0x1)
        {
            HWC_LOGD("uevents: hdmi connecting...");
        }
        else
        {
            HWC_LOGD("uevents: hdmi disconnect");

            if (m_is_hotplug)
            {
                DisplayManager::getInstance().hotplugExt(HWC_DISPLAY_EXTERNAL, false);
            }

            m_is_hotplug  = false;
        }
    }
    else if (change_hdmi_res)
    {
        if (state != 0x0)
        {
            if (m_is_hotplug)
            {
                HWC_LOGD("uevents: disconnect before reconnect hdmi");
                DisplayManager::getInstance().hotplugExt(HWC_DISPLAY_EXTERNAL, false);
                usleep(32000);
            }

            HWC_LOGD("uevents: change hdmi resolution");
            DisplayManager::getInstance().hotplugExt(HWC_DISPLAY_EXTERNAL, true);
            m_is_hotplug = true;
        }
    }
    else if (change_widevine)
    {
        HWC_LOGI("uevents: video hdcp version(%d)", state);
        DisplayManager::getInstance().setVideoHdcp(state);
    }
    else
    {
        HWC_LOGE("unknown uevents");
    }
}

bool UEventThread::threadLoop()
{
    struct pollfd fds;
    static char uevent_desc[UEVENT_BUFFER_SIZE * 2];

    fds.fd = m_socket;
    fds.events = POLLIN;
    fds.revents = 0;
    int ret = poll(&fds, 1, -1);

    if (ret > 0 && (fds.revents & POLLIN))
    {
        /* keep last 2 zeroes to ensure double 0 termination */
        int count = recv(m_socket, uevent_desc, sizeof(uevent_desc) - 2, 0);
        if (count > 0) handleUevents(uevent_desc, count);
    }

    if (FAKE_HDMI_PLUG == m_fake_hdmi)
    {
        if (m_is_hotplug)
        {
            HWC_LOGD("Disconnect hdmi before reconnect");

            DisplayManager::getInstance().hotplugExt(HWC_DISPLAY_EXTERNAL, false);
            usleep(32000);
        }

        HWC_LOGD("FAKE_HDMI_PLUG !!");

        DisplayManager::getInstance().hotplugExt(HWC_DISPLAY_EXTERNAL, true, true);
        m_fake_hotplug = true;
    }
    else if ((FAKE_HDMI_UNPLUG == m_fake_hdmi) && m_fake_hotplug)
    {
        HWC_LOGD("FAKE_HDMI_UNPLUG !!");

        DisplayManager::getInstance().hotplugExt(HWC_DISPLAY_EXTERNAL, false, true);
        m_fake_hotplug = false;
    }

    m_fake_hdmi = FAKE_HDMI_NONE;

    return true;
}

void UEventThread::setProperty()
{
    char value[PROPERTY_VALUE_MAX];

    property_get("vendor.debug.hwc.test_hdmi_connect", value, "0");
    m_fake_hdmi = atoi(value);

    HWC_LOGD("Connect fake hdmi(%d)", m_fake_hdmi);
}

RefreshRequestThread::RefreshRequestThread()
    : m_enabled(false)
{
}

RefreshRequestThread::~RefreshRequestThread()
{
}

void RefreshRequestThread::initialize()
{
    run("RefreshRequestThread", PRIORITY_URGENT_DISPLAY);
}

void RefreshRequestThread::setEnabled(bool enable)
{
    Mutex::Autolock _l(m_lock);
    m_enabled = enable;
    m_condition.signal();
}

bool RefreshRequestThread::threadLoop()
{
    {
        Mutex::Autolock _l(m_lock);
        if (exitPending()) {
            return false;
        }
        while (!m_enabled)
        {
            m_state = HWC_THREAD_IDLE;
            m_condition.wait(m_lock);
            if (exitPending()) {
                return false;
            }
        }
        m_state = HWC_THREAD_TRIGGER;
    }

    unsigned int type;
    int err = getHwDevice()->waitRefreshRequest(&type);
    if (err == NO_ERROR) {
        AbortMessager::getInstance().printf("RefreshRequestThread m_refresh_vali_lock lock +");
        HWCMediator::getInstance().lockRefreshVali();
        AbortMessager::getInstance().printf("RefreshRequestThread m_refresh_vali_lock lock -");
        DisplayManager::getInstance().refresh(type);
        HWCMediator::getInstance().unlockRefreshVali();
        AbortMessager::getInstance().printf("RefreshRequestThread m_refresh_vali_lock unlock");
    }

    return true;
}

HWVSyncEstimator::HWVSyncEstimator()
    : m_sample_count(0)
    , m_last_signaled_prenset_fence_time(0)
{
    resetAvgVSyncPeriod(DisplayManager::getInstance().getDisplayData(HWC_DISPLAY_PRIMARY)->refresh);
}

HWVSyncEstimator::~HWVSyncEstimator()
{
}

nsecs_t HWVSyncEstimator::getLastSignaledPresentFenceTime()
{
    return m_last_signaled_prenset_fence_time;
}

void HWVSyncEstimator::resetAvgVSyncPeriod(nsecs_t period)
{
    AutoMutex l(m_present_fence_queue_lock);
    for (int fence_fd : m_present_fence_queue)
    {
        ::protectedClose(fence_fd);
    }
    m_present_fence_queue.clear();
    m_avg_period = period;
}

nsecs_t HWVSyncEstimator::getAvgHWVSyncPeriod()
{
    return m_avg_period;
}

void HWVSyncEstimator::pushPresentFence(const int& fd)
{
    AutoMutex l(m_present_fence_queue_lock);
    m_present_fence_queue.push_back(fd);
}

nsecs_t HWVSyncEstimator::getNextHWVsync(nsecs_t cur)
{
    return m_last_signaled_prenset_fence_time +
            (((cur - m_last_signaled_prenset_fence_time) / m_avg_period) + 1) * m_avg_period;
}

void HWVSyncEstimator::update(nsecs_t refresh_alignment)
{
    AutoMutex l(m_present_fence_queue_lock);
    nsecs_t cur_fence_signal_time = 0, prev_fence_signal_time = 0;
    std::list<int>::iterator it;
    std::list<int>::iterator last_signaled = m_present_fence_queue.begin();
    refresh_alignment = refresh_alignment != 0 ? refresh_alignment :
        DisplayManager::getInstance().getDisplayData(HWC_DISPLAY_PRIMARY)->refresh;

    for (it = m_present_fence_queue.begin(); it != m_present_fence_queue.end(); ++it)
    {
        nsecs_t temp = getFenceSignalTime(*it);
        if (temp != SIGNAL_TIME_INVALID && temp != SIGNAL_TIME_PENDING) {
            prev_fence_signal_time = cur_fence_signal_time;
            cur_fence_signal_time = temp;
            last_signaled = it;

            if (it != m_present_fence_queue.begin())
            {
                uint32_t num_of_vsync =
                    static_cast<uint32_t>((cur_fence_signal_time - prev_fence_signal_time) /
                                          static_cast<float>(refresh_alignment) + 0.5f);

                if (m_sample_count < 100)
                    m_sample_count++;

                if (num_of_vsync > 0 && num_of_vsync < 5)
                    m_avg_period = (((m_sample_count - 1) * m_avg_period) +
                                    ((cur_fence_signal_time - prev_fence_signal_time) / num_of_vsync))
                                    / m_sample_count;
            }
        }
    }

    if (last_signaled != m_present_fence_queue.begin())
    {
        for (it = m_present_fence_queue.begin(); it != last_signaled; ++it) {
            ::protectedClose(*it);
        }
        m_present_fence_queue.erase(m_present_fence_queue.begin(), last_signaled);
    }
    m_last_signaled_prenset_fence_time = cur_fence_signal_time;
}

