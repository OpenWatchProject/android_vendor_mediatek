#ifndef PERFIOCTL_H
#define PERFIOCTL_H

#include <linux/types.h>
#include <linux/ioctl.h>

#include <vector>

using namespace std;

enum  {
    SWUI = 0,
    HWUI,
    GLSURFACE
};

/* enum: PERFMGR_CPU_PREFER */
enum {
    CPU_PREFER_NO     = 0,
    CPU_PREFER_BIG    = 1,
    CPU_PREFER_LITTLE = 2,
};

typedef struct _FPSGO_PACKAGE {
    __u32 tid;
    union {
        __u32 start;
        __u32 connectedAPI;
    };
    union {
        __u64 frame_time;
        __u64 bufID;
    };
    __u64 frame_id; /* for HWUI only*/
    __s32 queue_SF;
    __u64 identifier;
} FPSGO_PACKAGE;


#define MAX_DEVICE 2
typedef struct _EARA_NN_PACKAGE {
    __u32 pid;
    __u32 tid;
    __u64 mid;
    __s32 errorno;
    __s32 priority;
    __s32 num_step;

    __s32 dev_usage;
    __u32 bw_usage;

    union {
        __s32 *device;
        __u64 p_dummy_device;
    };
    union {
        __s32 *boost;
        __u64 p_dummy_boost;
    };
    union {
        __u64 *exec_time;
        __u64 p_dummy_exec_time;
    };
    union {
        __u64 *target_time;
        __u64 p_dummy_target_time;
    };

} EARA_NN_PACKAGE;

enum  {
    USAGE_DEVTYPE_CPU  = 0,
    USAGE_DEVTYPE_GPU  = 1,
    USAGE_DEVTYPE_APU  = 2,
    USAGE_DEVTYPE_MDLA = 3,
    USAGE_DEVTYPE_VPU  = 4,
    USAGE_DEVTYPE_MAX  = 5,
};

typedef struct _PERFMGR_PACKAGE {
	__u32 tid;
	union {
		__u32 prefer_type;
	};
} PERFMGR_PACKAGE;

#define FPSGO_QUEUE                  _IOW('g', 1, FPSGO_PACKAGE)
#define FPSGO_DEQUEUE                _IOW('g', 3, FPSGO_PACKAGE)
#define FPSGO_VSYNC                  _IOW('g', 5, FPSGO_PACKAGE)
#define FPSGO_TOUCH                  _IOW('g', 10, FPSGO_PACKAGE)
#define FPSGO_QUEUE_CONNECT          _IOW('g', 15, FPSGO_PACKAGE)
#define FPSGO_BQID                   _IOW('g', 16, FPSGO_PACKAGE)

#define EARA_NN_BEGIN               _IOW('g', 1, EARA_NN_PACKAGE)
#define EARA_NN_END                 _IOW('g', 2, EARA_NN_PACKAGE)
#define EARA_GETUSAGE               _IOW('g', 3, EARA_NN_PACKAGE)

#define PERFMGR_CPU_PREFER          _IOW('g', 1, PERFMGR_PACKAGE)

#endif
