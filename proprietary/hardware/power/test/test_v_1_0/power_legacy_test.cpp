#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define LOG_TAG "PowerTest"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <sys/un.h>

#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>

#include "mtkperf_resource.h"

using namespace vendor::mediatek::hardware::power::V2_0;
using ::android::hardware::hidl_string;

//namespace android {

enum {
    CMD_AOSP_POWER_HINT = 1,
    CMD_QUERY_INFO,
    CMD_SET_INFO,
    CMD_SCN_REG,
    CMD_SCN_CONFIG,
    CMD_SCN_UNREG,
    CMD_SCN_ENABLE,
    CMD_SCN_DISABLE,
    CMD_SCN_ULTRA_CFG,
    CMD_UNIT_TEST,
};

android::sp<IPower> gPowerHal;

static void unit_test(int cmd)
{
    int handle_a = -1, handle_b = -1;

    switch(cmd) {
    case 1:
        handle_a = gPowerHal->scnReg();
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, 1000000, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, 1200000, 0, 0);
        gPowerHal->scnEnable(handle_a, 100000);
        sleep(5);

        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, 1200000, 0, 0);
        sleep(5);

        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, 1400000, 0, 0);
        sleep(5);

        handle_b = gPowerHal->scnReg();
        gPowerHal->scnConfig(handle_b, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 0, 1600000, 0, 0);
        gPowerHal->scnConfig(handle_b, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 1, 1800000, 0, 0);
        gPowerHal->scnEnable(handle_b, 100000);
        sleep(5);

        gPowerHal->scnConfig(handle_b, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 0, 1000000, 0, 0);
        sleep(5);

        gPowerHal->scnConfig(handle_b, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 0, 1500000, 0, 0);
        sleep(5);

        gPowerHal->scnDisable(handle_a);
        gPowerHal->scnDisable(handle_b);

        break;

    case 2:
        handle_a = gPowerHal->scnReg();
        handle_b = gPowerHal->scnReg();

        /* config */
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_GPU_FREQ_MIN, 0, 0, 0, 0); // A min: opp 0
        gPowerHal->scnConfig(handle_b, MtkPowerCmd::CMD_SET_GPU_FREQ_MAX, 1, 0, 0, 0); // B max: opp 1

        /* case 1 */
        gPowerHal->scnEnable(handle_a, 100000);
        sleep(5);
        gPowerHal->scnDisable(handle_a);
        sleep(5);

        /* case 2 */
        gPowerHal->scnEnable(handle_b, 100000);
        sleep(5);
        gPowerHal->scnDisable(handle_b);
        sleep(5);

        /* case 3 */
        gPowerHal->scnEnable(handle_a, 100000);
        sleep(5);
        gPowerHal->scnEnable(handle_b, 100000);
        sleep(5);

        gPowerHal->scnDisable(handle_a);
        sleep(5);
        gPowerHal->scnDisable(handle_b);
        sleep(5);

        /* case 4 */
        gPowerHal->scnEnable(handle_b, 100000);
        sleep(5);
        gPowerHal->scnEnable(handle_a, 100000);
        sleep(5);

        gPowerHal->scnDisable(handle_b);
        sleep(5);
        gPowerHal->scnDisable(handle_a);
        sleep(5);
        break;

    case 3:
        handle_a = gPowerHal->scnReg();

        /* config */
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_STUNE_THRESH, 12, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_SCHED_BOOST, 1, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_TA_BOOST_VALUE, 97, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_OPP_DDR, 0, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_DISP_IDLE_TIME, 123, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_IO_BOOST_VALUE, 85, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_STUNE_TA_PERFER_IDLE, 1, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_MTK_PREFER_IDLE, 1, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_WIPHY_CAM, 1, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_GED_BENCHMARK_ON, 1, 0, 0, 0);
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_GX_BOOST, 1, 0, 0, 0);

        gPowerHal->scnEnable(handle_a, 600000);
        sleep(60);
        gPowerHal->scnDisable(handle_a);
        sleep(5);
        break;

    case 4:
        handle_a = gPowerHal->scnReg();

        /* config */
        gPowerHal->scnConfig(handle_a, MtkPowerCmd::CMD_SET_CPU_PERF_MODE, 1, 0, 0, 0);
        gPowerHal->scnEnable(handle_a, 600000);
        sleep(60);
        gPowerHal->scnDisable(handle_a);
        sleep(5);
        break;

    default:
        break;
    }
}

static void usage(char *cmd);

int main(int argc, char* argv[])
{
    int test_cmd=0;
    int hint=0, timeout=0, data=0;
    int flexcmd=0, p1=0, p2=0, p3=0, p4=0;
    int cmd=0;
    int handle = -1;

    if(argc < 2) {
        usage(argv[0]);
        return 0;
    }

    //fprintf(stderr, "IPower\n");
    //sleep(1);
    gPowerHal = IPower::tryGetService();
    if (gPowerHal == nullptr) {
        fprintf(stderr, "no IPower\n");
        return -1;
    }
    //fprintf(stderr, "IMtkPower\n");
    //sleep(1);

    test_cmd = atoi(argv[1]);
    //printf("argc:%d, command:%d\n", argc, command);
    switch(test_cmd) {
        case CMD_SCN_REG:
            if(argc!=2) {
                usage(argv[0]);
                return -1;
            }
            break;

        case CMD_SCN_UNREG:
        case CMD_SCN_DISABLE:
        case CMD_UNIT_TEST:
            if(argc!=3) {
                usage(argv[0]);
                return -1;
            }
            break;

        case CMD_AOSP_POWER_HINT:
        case CMD_SET_INFO:
        case CMD_QUERY_INFO:
        case CMD_SCN_ENABLE:
            if(argc!=4) {
                usage(argv[0]);
                return -1;
            }
            break;

        case CMD_SCN_CONFIG:
        case CMD_SCN_ULTRA_CFG:
            if(argc!=8) {
                usage(argv[0]);
                return -1;
            }
            break;

        default:
            usage(argv[0]);
            return -1;
    }

    if(test_cmd == CMD_AOSP_POWER_HINT) {
        hint = atoi(argv[2]);
        data = atoi(argv[3]);
    }
    else if(test_cmd == CMD_QUERY_INFO || test_cmd == CMD_SET_INFO) {
        cmd = atoi(argv[2]);
        p1 = atoi(argv[3]);
    }
    else if(test_cmd == CMD_SCN_UNREG || test_cmd == CMD_SCN_DISABLE) {
        handle = atoi(argv[2]);
    }
    else if(test_cmd == CMD_SCN_ENABLE) {
        handle = atoi(argv[2]);
        timeout = atoi(argv[3]);
    }
    else if(test_cmd == CMD_SCN_CONFIG || test_cmd == CMD_SCN_ULTRA_CFG) {
        handle = atoi(argv[2]);
        flexcmd = atoi(argv[3]);
        p1 = atoi(argv[4]);
        p2 = atoi(argv[5]);
        p3 = atoi(argv[6]);
        p4 = atoi(argv[7]);
    }
    else if(test_cmd == CMD_UNIT_TEST) {
        cmd = atoi(argv[2]);
    }

    /* command */
    if(test_cmd == CMD_AOSP_POWER_HINT) {
        if(hint <= (int)::android::hardware::power::V1_0::PowerHint::LAUNCH)
            gPowerHal->powerHintAsync((::android::hardware::power::V1_0::PowerHint)hint, data);
    }
    else if(test_cmd == CMD_QUERY_INFO) {
        data = gPowerHal->querySysInfo((enum MtkQueryCmd)cmd, p1);
        printf("data:%d\n", data);
    }
    else if(test_cmd == CMD_SCN_REG) {
            handle = gPowerHal->scnReg();
            printf("handle:%d\n", handle);
    }
    else if(test_cmd == CMD_SCN_CONFIG)
        gPowerHal->scnConfig(handle, (enum MtkPowerCmd)flexcmd, p1, p2, p3, p4);
    else if(test_cmd == CMD_SCN_UNREG)
        gPowerHal->scnUnreg(handle);
    else if(test_cmd == CMD_SCN_ENABLE)
        gPowerHal->scnEnable(handle, timeout);
    else if(test_cmd == CMD_SCN_DISABLE)
        gPowerHal->scnDisable(handle);
    else if(test_cmd == CMD_SCN_ULTRA_CFG)
        gPowerHal->scnUltraCfg(handle, flexcmd, p1, p2, p3, p4);
    else if(test_cmd == CMD_UNIT_TEST) {
        unit_test(cmd);
    }

    return 0;
}

static void usage(char *cmd) {
    fprintf(stderr, "\nUsage: %s command scenario\n"
                    "    command\n"
                    "        1: AOSP power hint\n"
                    "        2: query info\n"
                    "        3: set info\n"
                    "        4: scn register\n"
                    "        5: scn config\n"
                    "        6: scn unregister\n"
                    "        7: scn enable\n"
                    "        8: scn disable\n"
                    "        9: scn ultracfg\n"
                    "       10: unit test\n", cmd);
}



//} // namespace

