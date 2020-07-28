#ifndef _POWER_HAL_TYPE_H_
#define _POWER_HAL_TYPE_H_

typedef int (*scn_reg)(void);
typedef int (*scn_config)(int, int, int, int, int, int);
typedef int (*scn_enable)(int, int);
typedef int (*scn_unreg)(int);
typedef int (*scn_disable)(int);

//Ref: http://mbjswglx817:8010/alps-trunk-o0.tk/xref/vendor/mediatek/proprietary/hardware/interfaces/power/1.1/types.hal

enum {
    /* cpu */
    CMD_SET_CLUSTER_CPU_CORE_MIN        = 1,
    CMD_SET_CLUSTER_CPU_CORE_MAX        = 2,
    CMD_SET_CLUSTER_CPU_FREQ_MIN        = 3,
    CMD_SET_CLUSTER_CPU_FREQ_MAX        = 4,
    CMD_SET_CPU_PERF_MODE               = 5,
    /* gpu */
    CMD_SET_GPU_FREQ_MIN                = 6,
    CMD_SET_GPU_FREQ_MAX                = 7,

    /* vcore */
    CMD_SET_VCORE_BW_THRES              = 8,
    CMD_SET_VCORE_BW_ENABLED            = 9,
    CMD_SET_VCORE_MIN                   = 10,

    /* state */
    CMD_SET_SCREEN_OFF_STATE            = 11,
    /* DVFS */
    CMD_SET_CPUFREQ_HISPEED_FREQ        = 12,
    CMD_SET_CPUFREQ_MIN_SAMPLE_TIME     = 13,
    CMD_SET_CPUFREQ_ABOVE_HISPEED_DELAY = 14,
    CMD_SET_DVFS_POWER_MODE             = 15,
    /* HPS */
    CMD_SET_HPS_UP_THRESHOLD            = 16,
    CMD_SET_HPS_DOWN_THRESHOLD          = 17,
    CMD_SET_HPS_UP_TIMES                = 18,
    CMD_SET_HPS_DOWN_TIMES              = 19,
    CMD_SET_HPS_RUSH_BOOST              = 20,
    CMD_SET_HPS_HEAVY_TASK              = 21,
    CMD_SET_HPS_POWER_MODE              = 22,
    /* PPM */
    CMD_SET_PPM_ROOT_CLUSTER            = 23,
    CMD_SET_PPM_NORMALIZED_PERF_INDEX   = 24,
    CMD_SET_PPM_MODE                    = 25,
    CMD_SET_PPM_HICA_VAR                = 26,
    /* sched */
    CMD_SET_SCHED_HTASK_THRESH          = 27,
    CMD_SET_SCHED_AVG_HTASK_AC          = 28,
    CMD_SET_SCHED_AVG_HTASK_THRESH      = 29,
    CMD_SET_SCHED_MODE                  = 30,
    CMD_SET_IDLE_PREFER                 = 31,
    CMD_SET_SCHED_LB_ENABLE             = 32,
    CMD_SET_GLOBAL_CPUSET               = 33,
    CMD_SET_ROOT_BOOST_VALUE            = 34,
    CMD_SET_TA_BOOST_VALUE              = 35,
    CMD_SET_FG_BOOST_VALUE              = 36,
    CMD_SET_BG_BOOST_VALUE              = 37,

    /* customized */
    CMD_SET_PACK_BOOST_MODE             = 38,
    CMD_SET_PACK_BOOST_TIMEOUT          = 39,
    CMD_SET_DFPS                        = 40,
};

#endif