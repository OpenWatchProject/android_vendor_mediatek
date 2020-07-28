 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "tlcfoo.h"

#define LOG_TAG "tuitest"
#include <android/log.h>

mcResult_t send_cmd(int test_id);

int main(int argc, char *argv[])
{
    int     c;
    unsigned int option  = 0;
    unsigned int num1	 = 6582;
    unsigned int num2	 = 6592;
    unsigned int result	 = 0;
    mcResult_t ret=MC_DRV_OK;

	LOG_I("tuitest TLC called");

    if(argc == 2) {
        char *p = argv[1];
        option = atoi(p);
    }
    if(option <= 0) {
        printf("Usage: tuiteset [OPTION]\n");
        printf(" 1      test i2c\n");
        printf(" 2      test touch\n");
        printf(" 3      test gpio\n");
        printf(" 4      test display\n");
        printf(" 5      test led\n");
        return 0;
    }

	LOG_I("OPTION: %d", option);

    ret = tlcOpen();
    if (MC_DRV_OK != ret)
    {
        LOG_E("open TL session failed!");
        return ret;
    }

    send_cmd(option);

    LOG_I("Final result is %d", result);

exit:
    tlcClose();
    return ret;
}
