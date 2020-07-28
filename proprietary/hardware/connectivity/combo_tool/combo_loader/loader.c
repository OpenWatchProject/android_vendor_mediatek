
#include "loader.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "wmt_loader"

#define WCN_COMBO_LOADER_CHIP_ID_PROP    "persist.vendor.connsys.chipid"
#define WCN_DRIVER_READY_PROP            "vendor.connsys.driver.ready"
#define WCN_COMBO_LOADER_DEV             "/dev/wmtdetect"
#define WMT_MODULES_PRE                  "/system/lib/modules/"
#define WMT_MODULES_SUFF                 ".ko"
#define WMT_DETECT_IOC_MAGIC                    'w'

#define COMBO_IOCTL_GET_CHIP_ID          _IOR(WMT_DETECT_IOC_MAGIC, 0, int)
#define COMBO_IOCTL_SET_CHIP_ID          _IOW(WMT_DETECT_IOC_MAGIC, 1, int)
#define COMBO_IOCTL_EXT_CHIP_DETECT      _IOR(WMT_DETECT_IOC_MAGIC, 2, int)
#define COMBO_IOCTL_GET_SOC_CHIP_ID      _IOR(WMT_DETECT_IOC_MAGIC, 3, int)
#define COMBO_IOCTL_DO_MODULE_INIT       _IOR(WMT_DETECT_IOC_MAGIC, 4, int)
#define COMBO_IOCTL_MODULE_CLEANUP       _IOR(WMT_DETECT_IOC_MAGIC, 5, int)
#define COMBO_IOCTL_EXT_CHIP_PWR_ON      _IOR(WMT_DETECT_IOC_MAGIC, 6, int)
#define COMBO_IOCTL_EXT_CHIP_PWR_OFF     _IOR(WMT_DETECT_IOC_MAGIC, 7, int)
#define COMBO_IOCTL_DO_SDIO_AUDOK        _IOR(WMT_DETECT_IOC_MAGIC, 8, int)



#define STP_WMT_MODULE_PRE_FIX           "mtk_stp_wmt"
#define STP_BT_MODULE_PRE_FIX            "mtk_stp_bt"
#define STP_GPS_MODULE_PRE_FIX           "mtk_stp_gps"
#define HIF_SDIO_MODULE_PRE_FIX          "mtk_hif_sdio"
#define STP_SDIO_MODULE_PRE_FIX          "mtk_stp_sdio"
#define STP_UART_MODULE_PRE_FIX          "mtk_stp_uart"

#define WMT_PROC_DBG_PATH                "/proc/driver/wmt_dbg"
#define WMT_PROC_AEE_PATH                "/proc/driver/wmt_aee"

static int g_loader_fd = -1;

static int g_chipid_array[] = {
    0x6620, 0x6628, 0x6630, 0x6632, 0x6572, 0x6582, 0x6592, 0x8127,
    0x6571, 0x6752, 0x6735, 0x0321, 0x0335, 0x0337, 0x8163, 0x6580,
    0x6755, 0x0326, 0x6797, 0x0279, 0x6757, 0x0551, 0x8167, 0x6759,
    0x0507, 0x6763, 0x0690, 0x6570, 0x0713, 0x6775, 0x0788, 0x6771,
    0x6765, 0x3967, 0x6761, 0x6779, 0x6768, 0x6785,
};

static int loader_do_chipid_vaild_check(int chip_id)
{
    int ret = -1;
    unsigned char i;

    for (i = 0; i < sizeof(g_chipid_array)/sizeof(0x6630); i++) {
        if (chip_id == g_chipid_array[i]) {
            ALOGI("chipid vaild check: %d :0x%x!\n", i, chip_id);
            ret = 0;
            break;
        }
    }

    return ret;
}

static int loader_do_kernel_module_init(int loader_fd, int chip_id) {
    int ret = 0;

    if (loader_fd < 0) {
        ALOGE("invalid loaderfd: %d\n", loader_fd);
        return -1;
    }

    ret = ioctl(loader_fd, COMBO_IOCTL_MODULE_CLEANUP, chip_id);
    if (ret) {
        ALOGE("do WMT-DETECT module cleanup failed: %d\n", ret);
        return -2;
    }

    ret = ioctl(loader_fd, COMBO_IOCTL_DO_MODULE_INIT, chip_id);
    if (ret) {
        ALOGE("do kernel module init failed: %d\n", ret);
        return -3;
    }

    ALOGI("do kernel module init succeed: %d\n", ret);

    return 0;
}

static void loader_do_combo_sdio_autok(int loader_fd, int chip_id) {
    int retry_counter = 10;
    int ret = -1;
    int no_ext_chip = -1;
    int chipid_detect = -1;
    int autok_ret = 0;

    ALOGV("chip id :%x\n", chip_id);

    /*trigger autok process, incase last autok process is interrupted by abnormal power off or battery down*/
    do {
        /*power on combo chip*/
        ret = ioctl(loader_fd, COMBO_IOCTL_EXT_CHIP_PWR_ON);
        if (0 != ret) {
            ALOGE("external combo chip power on failed\n");
            no_ext_chip = 1;
        } else {
            /*detect is there is an external combo chip, this step should not be must*/
            no_ext_chip = ioctl(loader_fd, COMBO_IOCTL_EXT_CHIP_DETECT, NULL);
        }

        ALOGI("external combo chip detected\n");
        chipid_detect = ioctl(loader_fd, COMBO_IOCTL_GET_CHIP_ID, NULL);
        ALOGI("chipid (0x%x) detected\n", chipid_detect);

        if (0 == no_ext_chip) {
            autok_ret = ioctl(loader_fd, COMBO_IOCTL_DO_SDIO_AUDOK, chipid_detect);
            ALOGE("do SDIO3.0 autok %s\n", autok_ret ? "fail" : "succeed");
        }

        ret = ioctl(loader_fd, COMBO_IOCTL_EXT_CHIP_PWR_OFF);
        ALOGI("external combo chip power off %s\n", ret ? "fail" : "succeed");

        if ((0 == no_ext_chip) && (-1 == chipid_detect)) {
            /*extenral chip detected, but no valid chipId detected, retry*/
            retry_counter--;
            ALOGE("chipId detect failed, retrying, left retryCounter:%d\n", retry_counter);
            usleep(500000);
        } else
            break;

    } while (0 < retry_counter);

}

static void loader_do_first_chip_detect(int loader_fd, int *chip) {
    int retry_counter = 20;
    int ret = -1;
    int no_ext_chip = -1;
    int chip_id = -1;
    int autok_ret = 0;
    char chipid_str[PROPERTY_VALUE_MAX] = {0};

    do {
        /*power on combo chip*/
        ret = ioctl(loader_fd, COMBO_IOCTL_EXT_CHIP_PWR_ON);
        if (-1 == ret) {
            ALOGI("SOC chip no need do combo chip power on.\n");
            no_ext_chip = 1;
            chip_id = ioctl(loader_fd, COMBO_IOCTL_GET_SOC_CHIP_ID, NULL);
        } else if (0 == ret) {
            /*detect is there is an external combo chip*/
            no_ext_chip = 0;
            ret = ioctl(loader_fd, COMBO_IOCTL_EXT_CHIP_DETECT, NULL);
            if (0 != ret)
                ALOGE("external combo chip detect failed (%d)\n", ret);
            else
                chip_id = ioctl(loader_fd, COMBO_IOCTL_GET_CHIP_ID, NULL);
        } else
            ALOGE("external combo power on failed (%d)\n", ret);

        ALOGI("chipid (0x%x) detected\n", chip_id);

        if (chip_id != -1) {
            sprintf(chipid_str, "0x%04x", chip_id);
            ret = property_set(WCN_COMBO_LOADER_CHIP_ID_PROP, chipid_str);
            if (0 != ret) {
                ALOGE("set property(%s) to %s failed,ret:%d, errno:%d\n",
                    WCN_COMBO_LOADER_CHIP_ID_PROP, chipid_str, ret, errno);
            } else
                ALOGI("set property(%s) to %s succeed.\n", WCN_COMBO_LOADER_CHIP_ID_PROP, chipid_str);
        }

        if (0 == no_ext_chip) {
            autok_ret = ioctl(loader_fd, COMBO_IOCTL_DO_SDIO_AUDOK, chip_id);
            ALOGI("do SDIO3.0 autok %s\n", autok_ret ? "fail" : "succeed");
            ret = ioctl(loader_fd, COMBO_IOCTL_EXT_CHIP_PWR_OFF);
            ALOGI("external combo chip power off %s\n", ret ? "fail" : "succeed");
        }

        if ((0 == no_ext_chip) && (-1 == chip_id)) {
            /*extenral chip detected, but no valid chipId detected, retry*/
            retry_counter--;
#ifdef HAVE_AEE_FEATURE
            if (0 == retry_counter) {
                AEE_SYSTEM_EXCEPTION("Connsys Combo chip detect failed");
            }
#endif
            usleep(500000);
            ALOGE("chipId detect failed, retrying, left retryCounter:%d\n", retry_counter);
        } else
            break;
    } while (0 < retry_counter);

    *chip = chip_id;
}

static int loader_do_driver_ready_set(void) {
    int ret = -1;
    char ready_str[PROPERTY_VALUE_MAX] = {0};
    ret = property_get(WCN_DRIVER_READY_PROP, ready_str, NULL);
    if ((0 >= ret) || (0 == strcmp(ready_str, "yes"))) {
        ALOGE("get property(%s) failed iRet:%d or property is %s\n",
               WCN_DRIVER_READY_PROP, ret, ready_str);
    }
    /*set it to yes anyway*/
    sprintf(ready_str, "%s", "yes");
    ret = property_set(WCN_DRIVER_READY_PROP, ready_str);
    if (0 != ret) {
        ALOGE("set property(%s) to %s failed ret:%d\n",
              WCN_DRIVER_READY_PROP, ready_str, ret);
    } else
        ALOGI("set property(%s) to %s succeed\n", WCN_DRIVER_READY_PROP, ready_str);

    return ret;
}

int main(int argc, char *argv[]) {
    int ret = -1;
    int chip_id = -1;
    int count = 0;
    char chipid_str[PROPERTY_VALUE_MAX] = {0};
    char ready_str[PROPERTY_VALUE_MAX] = {0};

    ALOGV("argc:%d,argv:%s\n", argc, argv[0]);

    ret = property_get(WCN_DRIVER_READY_PROP, ready_str, NULL);
    if (0 == strcmp(ready_str, "yes")) {
        ALOGE("WMT driver has been ready.\n");
        return ret;
    }

    do {
        g_loader_fd = open(WCN_COMBO_LOADER_DEV, O_RDWR | O_NOCTTY);
        if (g_loader_fd < 0) {
            count++;
            ALOGI("Can't open device node(%s) count(%d)\n", WCN_COMBO_LOADER_DEV, count);
            usleep(300000);
        }
        else
            break;
    }while(1);

    /*read from system property*/
    ret = property_get(WCN_COMBO_LOADER_CHIP_ID_PROP, chipid_str, NULL);
    chip_id = strtoul(chipid_str, NULL, 16);
    ALOGI("chip id from property:%d\n", chip_id);
    if ((0 != ret) && (-1 != loader_do_chipid_vaild_check(chip_id))) {
        /*valid chip_id detected*/
        ALOGI("key:(%s)-value:(%s),chipId:0x%04x,ret(%d)\n",
              WCN_COMBO_LOADER_CHIP_ID_PROP, chipid_str, chip_id, ret);
        if (0x6630 == chip_id || 0x6632 == chip_id)
            loader_do_combo_sdio_autok(g_loader_fd, chip_id);
    } else {
        /*trigger external combo chip detect and chip identification process*/
        loader_do_first_chip_detect(g_loader_fd, &chip_id);
    }

    /*set chipid to kernel*/
    ioctl(g_loader_fd, COMBO_IOCTL_SET_CHIP_ID, chip_id);

    if ((0x0321 == chip_id) || (0x0335 == chip_id) || (0x0337 == chip_id))
        chip_id = 0x6735;

    if (0x0326 == chip_id)
        chip_id = 0x6755;

    if (0x0551 == chip_id)
        chip_id = 0x6757;

    if (0x0690 == chip_id)
        chip_id = 0x6763;

    if (0x0279 == chip_id)
        chip_id = 0x6797;

    if (0x0507 == chip_id)
        chip_id = 0x6759;

    if (0x0713 == chip_id)
        chip_id = 0x6775;

    if (0x0788 == chip_id)
        chip_id = 0x6771;

    /*how to handling if not all module init fail?*/
    if (chip_id == -1) {
        ALOGE("chip id error !!(0x%x)\n", chip_id);
        return -1;
    }
    loader_do_kernel_module_init(g_loader_fd, chip_id);
    if (g_loader_fd >= 0) {
        close(g_loader_fd);
        g_loader_fd = -1;
    }

    if ((chown(WMT_PROC_DBG_PATH, AID_SHELL, AID_SYSTEM) == -1) ||
       (chown(WMT_PROC_AEE_PATH, AID_SHELL, AID_SYSTEM) == -1))
        ALOGE("chown wmt_dbg or wmt_aee fail:%s\n", strerror(errno));

    ret = loader_do_driver_ready_set();

    return ret;
}



