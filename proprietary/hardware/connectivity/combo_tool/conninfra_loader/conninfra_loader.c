
#include "conninfra_loader.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "conninfra_loader"

#define WCN_COMBO_LOADER_CHIP_ID_PROP    "persist.vendor.connsys.chipid"
#define WCN_DRIVER_READY_PROP            "vendor.connsys.driver.ready"
#define WCN_CONNINFRA_DEV             "/dev/conninfra_dev"

static int g_loader_fd = -1;

static int loader_do_driver_ready_set(void) {
    int ret = -1;
    char ready_str[PROPERTY_VALUE_MAX] = {0};

    ret = property_get(WCN_DRIVER_READY_PROP, ready_str, NULL);
    if ((0 >= ret) || (0 == strcmp(ready_str, "yes"))) {
        ALOGE("Property(%s) has been set as %s\n",
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
    int count = 0;

    ALOGV("argc:%d,argv:%s\n", argc, argv[0]);

    do {
        g_loader_fd = open(WCN_CONNINFRA_DEV, O_RDWR | O_NOCTTY);
        if (g_loader_fd < 0) {
            count++;
            ALOGI("Can't open device node(%s) count(%d)\n", WCN_CONNINFRA_DEV, count);
            usleep(300000);
        }
        else
            break;
    } while(1);


    loader_do_driver_ready_set();
    close(g_loader_fd);
    return 0;
}



