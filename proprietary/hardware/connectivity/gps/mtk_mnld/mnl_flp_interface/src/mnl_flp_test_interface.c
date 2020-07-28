#include <errno.h>
#include <unistd.h>  // for close
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

#include "mnl_flp_test_interface.h"
#include "mtk_lbs_utility.h"
#include "mtk_gps_type.h"

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[mnl_flp_test]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnl_flp_test] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnl_flp_test] ERR: ", __VA_ARGS__);
#else
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif

extern UINT32 adc_emi_address;
extern int gps_emi_fd;

/*****************************************
FLP test -> MNL Testing
*****************************************/
int flp_test2mnl_hdlr(int fd, flp_test2mnl_interface* hdlr) {
    int ret;
    char buff[1024] = {0};
    ret = safe_recvfrom(fd, buff, sizeof(buff));

    LOGD("[OFL]data from ofl tst: %d\n", (int)buff[0]);
    switch (buff[0]) {
    case '1':
        if (hdlr->flp_test_gps_start) {
            hdlr->flp_test_gps_start();
        } else {
            LOGE("flp2mnl_hdlr() flp_test_gps_start is NULL");
            ret = -1;
        }
        break;
    case '2':
        if (hdlr->flp_test_gps_stop) {
            hdlr->flp_test_gps_stop();
        } else {
            LOGE("flp2mnl_hdlr() flp_test_gps_stop is NULL");
            ret = -1;
        }
        break;

    case '3':
    {
        // Loopback test start
        if (hdlr->flp_test_lpbk_start) {
            hdlr->flp_test_lpbk_start();
        } else {
            LOGE("flp2mnl_hdlr() flp_lpbk is NULL");
            ret = -1;
        }
        break;
    }
    case '4':
        // Loopback test stop
        if (hdlr->flp_test_lpbk_stop) {
            hdlr->flp_test_lpbk_stop();
        } else {
            LOGE("flp2mnl_hdlr() flp_lpbk is NULL");
            ret = -1;
        }
        break;

    default:
        LOGD("[OFL] invalide opid :%d!", buff[0]);
        break;
    }

    return MTK_GPS_SUCCESS;
}

void *thread_emi_download(void* arg) {
    int emi_fd;
    LOGD("create: %.8X, arg = %p\r\n", (unsigned int)pthread_self(), arg);
    pthread_detach(pthread_self());

    LOGD("EMI download thread start arg=%p\n", arg);
    emi_fd = open("/dev/gps_emi", O_RDWR);
    if (emi_fd == -1) {
        LOGD("open_port: Unable to open /dev/gps_emi\n");
        return 0;
    } else {
        LOGD("open gps_emi successfully\n");
    }

    if (ioctl(emi_fd, IOCTL_EMI_MEMORY_INIT, NULL) <= 0) {
        LOGD("set IOCTL_EMI_MEMORY_INIT failed\n");
        close(emi_fd);
        return 0;
    }
    usleep(2000000);
    LOGD("EMI download successful and thread exit done");
    pthread_exit(NULL);
    return 0;
}

void *thread_adc_capture_init(void* arg) {
    int count;
    UNUSED(arg);
    LOGD("EMI ADC thread start\n");
    for (count = 1; count <= 10; count++) {
        gps_emi_fd = open("/dev/gps_emi", O_RDWR);
        if (gps_emi_fd == -1) {
            LOGW("open_port: Unable to open /dev/gps_emi, retry:%d\n", count);
            usleep(500*1000);
            continue;
        } else {
            LOGD("open gps_emi successfully\n");
            break;
        }
    }
    if (count >= 10) {
        LOGE("open gps_emi failed\n");
        return 0;
    }

    errno = 0;
    if (ioctl(gps_emi_fd, IOCTL_EMI_MEMORY_INIT, NULL) <= 0) {
        LOGE("set IOCTL_EMI_MEMORY_INIT failed,(%s)\n", strerror(errno));
    }
    errno = 0;
    if (ioctl(gps_emi_fd, IOCTL_ADC_CAPTURE_ADDR_GET, &adc_emi_address) < 0) {
        LOGE("set IOCTL_ADC_CAPTURE_ADDR_GET failed,(%s)\n", strerror(errno));
    }

    LOGD("ADC init successful and thread exit done, ADC emi address:(0x%08x)\n", adc_emi_address);
    pthread_exit(NULL);
    return 0;
}

void gps_emi_init(int type) {
    int ret = -1;
    pthread_t emi_download_thread;

    if (type == 0) {
        ret = pthread_create(&emi_download_thread, NULL, thread_emi_download, NULL);
    } else {
        ret = pthread_create(&emi_download_thread, NULL, thread_adc_capture_init, NULL);
    }
    if (0 != ret) {
        LOGE("emi download thread create fail: %s\n", strerror(errno));
        exit(1);
    }
}

// -1 means failure
int create_flp_test2mnl_fd() {
    int fd = socket_bind_udp(MTK_FLP_TEST2MNL);
    socket_set_blocking(fd, 0);
    return fd;
}

