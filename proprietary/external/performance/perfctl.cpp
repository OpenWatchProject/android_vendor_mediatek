#include <sys/types.h>
#include <utils/Log.h>
#include <utils/Timers.h>
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include "perfctl.h"
#include <linux/types.h>
#include <errno.h>
#include <string.h>
#include <utils/Timers.h>
#include <pthread.h>
#include <vector>
#include <string>

#define PERF_LOG_TAG "libPerfCtl"
#ifdef ALOGD
#undef ALOGD
#define ALOGD(...) do{((void)ALOG(LOG_INFO, PERF_LOG_TAG, __VA_ARGS__));}while(0)
#endif

#define RENDER_THREAD_UPDATE_DURATION   250000000
#define PATH_PERF_IOCTL "/proc/perfmgr/perf_ioctl"
#define PATH_EARA_IOCTL "/proc/perfmgr/eara_ioctl"
#define PATH_PERFMGR_IOCTL "/proc/perfmgr/perfmgr_ioctl"
#define UNUSED(x) (void)(x)

static pthread_mutex_t sMutex = PTHREAD_MUTEX_INITIALIZER;

int devfd = -1;

static inline int check_perf_ioctl_valid(void)
{
    if (devfd >= 0) {
        return 0;
    } else if (devfd == -1) {
        devfd = open(PATH_PERF_IOCTL, O_RDONLY);
        // file not exits
        if (devfd < 0 && errno == ENOENT)
            devfd = -2;
        // file exist, but can't open
        if (devfd == -1) {
            ALOGD("Can't open %s: %s", PATH_PERF_IOCTL, strerror(errno));
            return -1;
        }
    // file not exist
    } else if (devfd == -2) {
        //ALOGD("Can't open %s: %s", PATH_PERF_IOCTL, strerror(errno));
        return -2;
    }
    return 0;
}

extern "C"
int fbcNotifyTouch(int enable)
{
    FPSGO_PACKAGE msg;
    msg.tid = gettid();
    msg.frame_time = enable;
    if (check_perf_ioctl_valid() == 0)
        ioctl(devfd, FPSGO_TOUCH, &msg);

    return 0;
}

extern "C"
int xgfNotifyQueue(__u32 value, __u64 bufID, __s32 queue_SF, __u64 identifier)
{
    FPSGO_PACKAGE msg;

    UNUSED(bufID);
    UNUSED(queue_SF);

    pthread_mutex_lock(&sMutex);

    msg.tid = gettid();
    msg.start = value;
    msg.identifier = identifier;
    if (check_perf_ioctl_valid() == 0)
        ioctl(devfd, FPSGO_QUEUE, &msg);

    pthread_mutex_unlock(&sMutex);
    return 0;
}

extern "C"
int xgfNotifyDequeue(__u32 value, __u64 bufID, __s32 queue_SF, __u64 identifier)
{
    FPSGO_PACKAGE msg;

    UNUSED(bufID);
    UNUSED(queue_SF);

    pthread_mutex_lock(&sMutex);

    msg.tid = gettid();
    msg.start = value;
    msg.identifier = identifier;
    if (check_perf_ioctl_valid() == 0)
        ioctl(devfd, FPSGO_DEQUEUE, &msg);

    pthread_mutex_unlock(&sMutex);
    return 0;
}

extern "C"
int xgfNotifyConnect(__u32 value, __u64 bufID, __u32 connectedAPI, __u64 identifier)
{
    FPSGO_PACKAGE msg;
    msg.tid = gettid();
    msg.identifier = identifier;
    if (value)
        msg.connectedAPI = connectedAPI;
    else
        msg.connectedAPI = 0;
    if (check_perf_ioctl_valid() == 0)
        ioctl(devfd, FPSGO_QUEUE_CONNECT, &msg);

    return 0;
}

extern "C"
int xgfNotifyBqid(__u32 value, __u64 bufID, __s32 queue_SF, __u64 identifier)
{
    FPSGO_PACKAGE msg;
    msg.tid = gettid();
    msg.bufID = bufID;
    msg.queue_SF = queue_SF;
    msg.identifier = identifier;
    msg.start = value;

    if (check_perf_ioctl_valid() == 0)
        ioctl(devfd, FPSGO_BQID, &msg);
    return 0;
}

extern "C"
int xgfNotifyVsync(__u32 value)
{
    FPSGO_PACKAGE msg;
    msg.tid = gettid();
    msg.frame_time = value;
    if (check_perf_ioctl_valid() == 0)
        ioctl(devfd, FPSGO_VSYNC, &msg);

    return 0;
}


int earadevfd = -1;
static inline int check_eara_ioctl_valid(void)
{
    if (earadevfd >= 0) {
        return 0;
    } else if (earadevfd == -1) {
        earadevfd = open(PATH_EARA_IOCTL, O_RDONLY);
        // file not exits
        if (earadevfd < 0 && errno == ENOENT)
            earadevfd = -2;
        // file exist, but can't open
        if (earadevfd == -1) {
            ALOGD("Can't open %s: %s", PATH_EARA_IOCTL, strerror(errno));
            return -1;
        }
        // file not exist
    } else if (earadevfd == -2) {
        //ALOGD("Can't open %s: %s", PATH_EARA_IOCTL, strerror(errno));
        return -2;
    }
    return 0;
}

extern "C"
int earaNotifyCVJobBegin(int tid, int mid, uint64_t *suggestExecTime)
{
    UNUSED(tid);
    UNUSED(mid);

    *suggestExecTime = 0;

    return 0;
}

extern "C"
int earaNotifyCVJobEnd(int tid, uint64_t mid, int32_t *jobPriority,
        int errorStatus, uint64_t execTimeNanoSecs, int32_t bandWidth,
        vector<vector<int32_t>> boost)
{
    UNUSED(tid);
    UNUSED(mid);
    UNUSED(errorStatus);
    UNUSED(execTimeNanoSecs);
    UNUSED(bandWidth);

    *jobPriority = 0;

    return 0;
}

extern "C"
int earaNotifyJobBegin(int tid, uint64_t mid,
        vector<vector<int32_t>> planStruct, vector<vector<uint64_t>> *suggestExecTime)
{
    EARA_NN_PACKAGE msg;
    int i, j;
    __u64 *pl_target_time;

    msg.pid = getpid();
    msg.tid = tid;
    msg.mid = mid;
    msg.num_step = planStruct.size();
    pl_target_time = (__u64 *)calloc(msg.num_step * MAX_DEVICE, sizeof(__u64));

    if (check_eara_ioctl_valid() == 0) {
        msg.target_time = pl_target_time;
        ioctl(earadevfd, EARA_NN_BEGIN, &msg);
    }

    for (i = 0; i < planStruct.size(); i++)
        for (j = 0; j < planStruct[i].size(); j++) {
            if (pl_target_time) {
                (*suggestExecTime)[i][j] = pl_target_time[i * MAX_DEVICE + j];
            } else {
                (*suggestExecTime)[i][j] = 0;
            }
        }

    if (pl_target_time)
        free(pl_target_time);

    return 0;
}

extern "C"
int earaNotifyJobEnd(int tid, uint64_t mid, int32_t *jobPriority,
        int errorStatus, vector<vector<int32_t>> planStruct,
        vector<vector<uint64_t>> nanoSecs, vector<vector<int32_t>> bandWidth,
        vector<vector<int32_t>> boost)
{
    EARA_NN_PACKAGE msg;
    int i, j;
    __s32 *pl_device;
    __u64 *pl_exec_time;
    __s32 *pl_boost;

    UNUSED(bandWidth);

    msg.pid = getpid();
    msg.tid = tid;
    msg.mid = mid;
    msg.num_step = planStruct.size();
    msg.errorno = errorStatus;
    msg.priority = 0;
    pl_device = (__s32 *)malloc(msg.num_step * MAX_DEVICE * sizeof(__s32));
    pl_exec_time = (__u64 *)malloc(msg.num_step * MAX_DEVICE * sizeof(__u64));
    pl_boost = (__s32 *)malloc(msg.num_step * MAX_DEVICE * sizeof(__s32));

    for (i = 0; i < planStruct.size(); i++)
        for (j = 0; j < MAX_DEVICE; j++) {
            if (j < planStruct[i].size()) {
                if (pl_device)
                    pl_device[i * MAX_DEVICE + j] = planStruct[i][j];
                if (pl_exec_time)
                    pl_exec_time[i * MAX_DEVICE + j] = nanoSecs[i][j];
                if (pl_boost)
                    pl_boost[i * MAX_DEVICE + j] = boost[i][j];
            } else {
                if (pl_device)
                    pl_device[i * MAX_DEVICE + j] = -1;
            }
        }


    if (check_eara_ioctl_valid() == 0) {
        msg.device = pl_device;
        msg.exec_time = pl_exec_time;
        msg.boost = pl_boost;
        ioctl(earadevfd, EARA_NN_END, &msg);
     }
    *jobPriority = msg.priority;

    if (pl_device)
        free(pl_device);
    if (pl_exec_time)
        free(pl_exec_time);
    if (pl_boost)
        free(pl_boost);

    return 0;
}

extern "C"
{

int earaGetUsage(int type, int *deviceLoading, int *bandwidth)
{
    EARA_NN_PACKAGE msg;

    if (!deviceLoading || !bandwidth)
        return -2;

    if (type < 0 || type >= USAGE_DEVTYPE_MAX)
        return -2;

    msg.dev_usage = type;
    msg.bw_usage = -1;
    msg.pid = getpid();

    if (check_eara_ioctl_valid() == 0)
        ioctl(earadevfd, EARA_GETUSAGE, &msg);

    *deviceLoading = msg.dev_usage;
    *bandwidth = msg.bw_usage;

    return 0;

}

}

/* PERFMGR IOCTL */
int perfmgrdevfd = -1;
static inline int check_perfmgr_ioctl_valid(void)
{
    if (perfmgrdevfd >= 0) {
        return 0;
    } else if (perfmgrdevfd == -1) {
        perfmgrdevfd = open(PATH_PERFMGR_IOCTL, O_RDONLY);
        // file not exits
        if (perfmgrdevfd < 0 && errno == ENOENT)
            perfmgrdevfd = -2;
        // file exist, but can't open
        if (perfmgrdevfd == -1) {
            ALOGD("Can't open %s: %s", PATH_PERFMGR_IOCTL, strerror(errno));
            return -1;
        }
        // file not exist
    } else if (perfmgrdevfd == -2) {
        //ALOGD("Can't open %s: %s", PATH_EARA_IOCTL, strerror(errno));
        return -2;
    }
    return 0;
}

extern "C"
int perfmgrSetCpuPrefer(int tid, __u32 prefer_type)
{
    PERFMGR_PACKAGE msg;

    ALOGD("perfmgrSetCpuPrefer, tid:%d, prefer:%d", tid, prefer_type);

    msg.tid = tid;
    msg.prefer_type = prefer_type;

    if (check_perfmgr_ioctl_valid() == 0) {
        //ALOGD("perfmgrSetCpuPrefer, ioctl");
        ioctl(perfmgrdevfd, PERFMGR_CPU_PREFER, &msg);
    }

    return 0;
}


