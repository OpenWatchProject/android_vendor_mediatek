#include "white_list.h"

bool bypass_white_list()
{
    //white list: disable debug 15 process
    const char *progname = getprogname();
    if (!strcmp(progname, "/system/bin/patchoat") ||
        strstr(progname, "iptables") ||
        strstr(progname, "ip6tables") ||
        strstr(progname, "linker") ||
        strstr(progname, "profman") ||
        !strcmp(progname, "/system/bin/sh") ||
        !strcmp(progname, "/system/bin/logcat") ||
        !strcmp(progname, "/system/bin/installd") ||
        !strcmp(progname, "/system/bin/dex2oat") ||
        strstr(progname, "dumpsys") ||
        strstr(progname, "aee")) {
            return true;
        } else {
            return false;
        }
}
