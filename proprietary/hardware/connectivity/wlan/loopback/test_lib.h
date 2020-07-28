#ifndef _TEST_LIB_H__
#define _TEST_LIB_H__

typedef enum _ENUM_WIFI_TEST_RESULT_T 
{
    WIFI_TEST_RESULT_PASS,
    WIFI_TEST_RESULT_LOOPBACK_RUNNING,
    WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT,
    WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP,
    WIFI_TEST_RESULT_FAIL_NOT_STARTED,
    WIFI_TEST_RESULT_FAIL_INVALID_PARAMS,
    WIFI_TEST_RESULT_NUM
} ENUM_WIFI_TEST_RESULT_T, *P_ENUM_WIFI_TEST_RESULT_T;

int
wifi_set_power(
    int on
    );

int
init_mtk_wifi_loopback(
    void
    );

int 
uninit_mtk_wifi_loopback(
    void
    );

int 
mtk_wifi_loopback(
    unsigned int packet_length,
    unsigned int packet_num
    );

ENUM_WIFI_TEST_RESULT_T
mtk_wifi_get_result(
    unsigned int *complete_packet_num
    );

#endif

