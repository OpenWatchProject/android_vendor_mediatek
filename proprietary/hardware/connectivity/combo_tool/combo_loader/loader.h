#ifndef __WMT_LOADER_H_
#define __WMT_LOADER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <cutils/properties.h>
#include <cutils/misc.h>
#include <sys/ioctl.h>
#include <cutils/android_filesystem_config.h>
#include <log/log.h>

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#define AEE_SYSTEM_EXCEPTION(String) \
    do { \
        aee_system_exception( \
            "wmt_loader", \
            NULL, \
            DB_OPT_DEFAULT, \
            String); \
    } while(0)
#endif
#endif
