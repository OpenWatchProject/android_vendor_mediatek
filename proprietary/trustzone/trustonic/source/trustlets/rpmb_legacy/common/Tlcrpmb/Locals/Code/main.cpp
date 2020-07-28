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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>



#include "tlcrpmb.h"

#define LOG_TAG "TLC RPMB"
#include "cutils/log.h"

#define LOG_I ALOGD
#define LOG_E ALOGE


struct rpmb_ioc_param {
    unsigned char *key;
    unsigned char *data;
    unsigned int  data_len;
    unsigned short addr;
    unsigned char *hmac;
    unsigned int hmac_len;
};

//accordingly. the key was followed by phone.
//please dump key under preloader and copy to here.
unsigned char key[32] = {0xE3, 0xF8, 0xEC, 0x9C,
			 0x30, 0xEC, 0xD0, 0xD9,
			 0x31, 0xE0, 0xC8, 0xED,
			 0xBD, 0x96, 0x1B, 0xD9,
			 0xA2, 0xE1, 0x75, 0x49,
			 0xF8, 0x7B, 0xB4, 0x3D,
			 0xF1, 0x21, 0x43, 0x18,
			 0xC4, 0x76, 0x52, 0x85};

unsigned char *buf;

int open_rpmb()
{
    int fd;

    fd = open("/dev/emmcrpmb0", O_RDONLY);
    if (fd >= 0)
        return fd;

    return open("/dev/rpmb0", O_RDONLY);
}

int main(int argc, char *argv[])
{
    int     c;
    unsigned int option  = 1;	/* 1: add, 2: minus */
    unsigned int num1	 = 6582;
    unsigned int num2	 = 6592;
    unsigned int result	 = 0;
    unsigned int val=8, size=512, cmd, i, argu3, argu4, argu5, argu6;
    char *ptr;
    mcResult_t ret = 0;
    int err;
    struct rpmb_ioc_param param;
    int fd = 0;

    LOG_I("");
    LOG_I("RPMB TLC called");

    ptr = (char *)buf;

    if (argc > 1 ){
        if (!strcmp(argv[1], "open")) {
            fprintf(stderr, "open\n");
            option = 5;
        } else if (!strcmp(argv[1], "read")){
	    fprintf(stderr, "read\n");
	    option = 3;
        } else if (!strcmp(argv[1], "write")) {
            fprintf(stderr, "write\n");
            option = 4;
        } else if (!strcmp(argv[1], "ioctl")) {
            fprintf(stderr, "ioctl\n");
            option = 6;
#ifdef CFG_RPMB_KEY_PROGRAMED_IN_KERNEL
        } else if (!strcmp(argv[1], "programkey")) {
            fprintf(stderr, "program_key\n");
            option = 8;
#endif
        } else if (!strcmp(argv[1], "exit")) {
            fprintf(stderr, "exit\n");
            option = 7;
        }
    }

    if ((argc == 1) || (option == 1)) {
        fprintf(stderr, "tlcrpmb command usage:\n");
        fprintf(stderr, "   tlcrpmb read [size]\n");
        fprintf(stderr, "   tlcrpmb write [size] [value]\n");
#ifdef CFG_RPMB_KEY_PROGRAMED_IN_KERNEL
        fprintf(stderr, "   tlcrpmb programkey\n");
#endif
        fprintf(stderr, "   tlcrpmb ioctl [write:read] [length] [value] [blkaddr]\n");
        fprintf(stderr, "   tlcrpmb ioctl setkey\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Write example: tlcrpmb ioctl write 32 2 1\n");
        fprintf(stderr, "(write data 32bytes, full filled with value 2, write to physical block addr 1.)\n");
    }

    if (argc > 2) {
        argu3 = atoi(argv[2]);
    }

    if (argc > 3) {
        argu4 = atoi(argv[3]);
    }

    if (argc > 4) {
        argu5 = atoi(argv[4]);
    }

    if (argc > 5) {
        argu6 = atoi(argv[5]);
    }

    LOG_E("option is %d!", option);

    ret = tlcOpen();

    if (MC_DRV_OK != ret) {
        LOG_E("open TL session failed!");
        return ret;
    }

    //
    // RPMB read operation
    //
    if (3 == option) {
        buf = (unsigned char *)malloc(argu3);

        ret = read((unsigned int *)buf, argu3, &result);
        for (i=0;i<argu3;i++) {
            if (i%16==0)
                fprintf(stderr, "\n");
            fprintf(stderr, "%02x, ", buf[i]);

        }
        free(buf);
    }
    fprintf(stderr, "\n");

    //
    // RPMB write operation
    //
    if (4 == option) {
        buf = (unsigned char *)malloc(argu3);

        memset(buf, argu4, argu3);

        ret = write((unsigned int *)buf, argu3, &result);
        LOG_I("Final Result showed in NWd (%d)", result);

        free(buf);
    }

    if (5 == option) {
        fd = open_rpmb();
        if (fd < 0)
            fprintf(stderr, "emmcrpmb0 and rpmb0 open failed!!");

        fprintf(stderr, "open 1!!\n");
    }

    if (6 == option) {
        fd = open_rpmb();
        if (fd < 0)
            fprintf(stderr, "emmcrpmb0 and rpmb0 open failed!!");

        buf = (unsigned char *)malloc(argu4);

        param.key = key;;
        param.data_len = argu4;
        param.data = (unsigned char *)buf;
        if (!strcmp(argv[2], "write")) {
            memset(param.data, argu5, argu4);
            param.addr = argu6;
            cmd = 3;
        } else if (!strcmp(argv[2], "read")) {
            param.addr = argu5;
            cmd = 4;
        } else if (!strcmp(argv[2], "setkey")) {
            memcpy(param.key, key, 32);
            cmd = 1;
        }

        err = ioctl(fd, cmd, &param);
        if (err < 0)
            fprintf(stderr, "emmcrpmb0 ioctl %d failed!!\n", cmd);

        for (i=0;i<argu4;i++) {
            if (i%16==0)
                fprintf(stderr, "\n");
            fprintf(stderr, "%02x, ",param.data[i]);
        }
        fprintf(stderr, "\n");

        close(fd);

        free(buf);
    }

#ifdef CFG_RPMB_KEY_PROGRAMED_IN_KERNEL
    if (8 == option) {
        ret = setkey( &result );
        LOG_I("Final Result showed in NWd (%d)", result);
        if (result == 2) {
            fprintf(stderr, "The RPMB key has been programmed already. Can't program again!\n");
        }
    }
#endif

    LOG_I("Final result is %d", result);

exit:
    tlcClose();
    return 0;
}
