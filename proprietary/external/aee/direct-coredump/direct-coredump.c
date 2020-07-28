#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <cutils/properties.h>

#define SIGNUM 7

static int is_user_built(void)
{
    char propbuf[PROP_VALUE_MAX];

    property_get("ro.build.type", propbuf, "user");
    return strncmp(propbuf, "user", sizeof("user")) == 0 ? 1 : 0;
}

__attribute__((constructor)) static void __aeeDirectcoredump_init()
{
    int sigtype[SIGNUM] = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP, SIGSYS};
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    // eng&userdebug load direct-coredump default enable
    // user load direct-coredump default disable
    if (is_user_built()) {
        property_get("persist.vendor.aeev.core.direct", value, "disable");
    } else {
        property_get("persist.vendor.aeev.core.direct", value, "enable");
    }

    if (!strncmp(value, "enable", sizeof("enable"))) {
        int loop;
        for (loop = 0; loop < SIGNUM; loop++) {
            signal(sigtype[loop], SIG_DFL);
        }
    }
}
