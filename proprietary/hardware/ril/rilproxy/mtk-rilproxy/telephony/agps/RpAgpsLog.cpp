/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdio.h>
#include "RpAgpsLog.h"

/*****************************************************************************
 * Functions
 *****************************************************************************/
void agpsLogPrint(int type, const char* tag, const char *fmt, ...) {
    char out_buf[1100] = {0};
    char buf[1024] = {0};
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    sprintf(out_buf, "%s %s", tag, buf);

    if(type == 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "rilp-agps", "%s", out_buf);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "rilp-agps", "%s", out_buf);
    }
}
