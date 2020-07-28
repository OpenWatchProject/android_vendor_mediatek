#ifndef __MNL_FLP_TEST_INTERFACE_H__
#define __MNL_FLP_TEST_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#ifdef __cplusplus
extern "C" {
#endif

#define IOCTL_EMI_MEMORY_INIT        1
#define IOCTL_MNL_NVRAM_FILE_TO_MEM  2
#define IOCTL_MNL_NVRAM_MEM_TO_FILE  3
#define IOCTL_ADC_CAPTURE_ADDR_GET   4
#define ADC_CAPTURE_BUFF_SIZE  0x50000

typedef struct {
    void (*flp_test_gps_start)();
    void (*flp_test_gps_stop)();
    int (*flp_test_lpbk_start)();
    int (*flp_test_lpbk_stop)();
} flp_test2mnl_interface;

//======================================================
// GPS FLP test -> MNLD
//======================================================
#define MTK_FLP_TEST2MNL "mtk_flp_test2mnl"

int flp_test2mnl_hdlr(int fd, flp_test2mnl_interface* hdlr);
int create_flp_test2mnl_fd(void);
void gps_emi_init(int type);
#ifdef __cplusplus
}
#endif

#endif

