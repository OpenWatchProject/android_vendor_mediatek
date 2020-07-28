#define LOG_TAG "utilSystrace"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <android/log.h>
#include <sys/system_properties.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD_IF(cond, ...) do { if ( (cond) ) { __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__); } }while(0)
#define LOGE_IF(cond, ...) do { if ( (cond) ) { __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__); } }while(0)

#define ATRACE_MESSAGE_LEN (256)

int util_atrace_marker_fd = -1;
int util_atrace_log_enable = 0;

static int property_get(const char *key, char *value, const char *default_value)
{
    int len;
    len = __system_property_get(key, value);
    if (len > 0) {
        return len;
    }

    if (default_value) {
        len = strlen(default_value);
        memcpy(value, default_value, len + 1);
    }
    return len;
}

void utilTraceInitOnce()
{
    char value[255];
    util_atrace_log_enable = property_get("vendor.algodbg.atrace.enable", value, "0");
    util_atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);

    if (util_atrace_marker_fd == -1)
    {
        LOGD_IF(util_atrace_log_enable, "Error opening trace file: %s (%d)", strerror(errno), errno);
        LOGD_IF(util_atrace_log_enable, "utilTraceInit fail");
    }
    else
    {
        LOGD_IF(util_atrace_log_enable, "utilTraceInit ok");
    }
}

void utilTraceTerminate()
{
    LOGD_IF(util_atrace_log_enable, "[%s] fd is %d", __FUNCTION__, util_atrace_marker_fd);
    if (util_atrace_marker_fd != -1)
    {
        close(util_atrace_marker_fd);
    }
    LOGD_IF(util_atrace_log_enable, "utilTraceTerminate done");
}

void utilTraceBegin(const char *name)
{
    char buf[ATRACE_MESSAGE_LEN];
    int len = snprintf(buf, ATRACE_MESSAGE_LEN, "B|%d|%s", getpid(), name);
    if (util_atrace_marker_fd > 0)
    {
        write(util_atrace_marker_fd, buf, len);
    }
}

void utilTraceEnd()
{
    if (util_atrace_marker_fd > 0)
    {
        write(util_atrace_marker_fd, "E", 1);
    }
}

void utilTraceAsyncBegin(const char *name, const int32_t cookie)
{
    char buf[ATRACE_MESSAGE_LEN];
    int len = snprintf(buf, ATRACE_MESSAGE_LEN, "S|%d|%s|%i", getpid(), name, cookie);
    if (util_atrace_marker_fd > 0)
    {
        write(util_atrace_marker_fd, buf, len);
    }
}

void utilTraceAsyncEnd(const char *name, const int32_t cookie)
{
    char buf[ATRACE_MESSAGE_LEN];
    int len = snprintf(buf, ATRACE_MESSAGE_LEN, "F|%d|%s|%i", getpid(), name, cookie);
    if (util_atrace_marker_fd > 0)
    {
        write(util_atrace_marker_fd, buf, len);
    }
}

