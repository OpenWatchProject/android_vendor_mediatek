#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "meta_gamma.h"


int main(int argc, const char** argv)
{
#if 0
    FT_WM_WIFI_REQ req;

    memset(&req, 0, sizeof(FT_WM_WIFI_REQ));

    META_WIFI_Register(wifi_info_callback);

    if (META_WIFI_init(&req) == false) {
        printf("WLAN init failed\n");
        return -1;
    }

    #if 0
    req.type = WIFI_CMD_SET_OID;
    META_WIFI_OP(&req, NULL, 0);

    req.type = WIFI_CMD_QUERY_OID;
    META_WIFI_OP(&req, NULL, 0);
    #endif

    META_WIFI_deinit();
    META_WIFI_Register(NULL);
#endif
    return 0;
}

