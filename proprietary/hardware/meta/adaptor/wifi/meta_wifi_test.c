#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "meta_wifi.h"
#include <cutils/log.h>

#ifdef LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG         "WIFI_META "

static void wifi_info_callback(FT_WM_WIFI_CNF *cnf, void *buf, unsigned int size)
{
    unsigned int i;
    char *type[] = { "WIFI_CMD_SET_OID", "WIFI_CMD_QUERY_OID","WIFI_CMD_NVRAM_WRITE_ACCESS","WIFI_CMD_NVRAM_READ_ACCESS"
        ,"WIFI_CMD_INIT", "WIFI_CMD_DEINIT", "WIFI_CMD_SCRIPT"};
    OID_STRUC *poid;

    if (buf == NULL) {
        printf("[META_WIFI] %s is Null!\n",__func__);
        return;
    }
    printf("[META_WIFI] <CNF> %s, Drv Status: %d, Status: %d\n", type[cnf->type],
    cnf->drv_status, cnf->status);

    switch(cnf->type)
    {
        case WIFI_CMD_SET_OID:
        case WIFI_CMD_QUERY_OID:
        {
            poid = (OID_STRUC *)buf;
            printf("META_WIFI] <CNF> OID: %d, data len: %d\n",
                poid->QueryOidPara.oid, poid->QueryOidPara.dataLen);
            for (i = 0; i < poid->QueryOidPara.dataLen; i++) {
                printf("META_WIFI] <CNF> Data[%d] = 0x%x\n",
                    i, poid->QueryOidPara.data[i]);
            }
            break;
        }
        case WIFI_CMD_SCRIPT:
        {
            printf("[META_WIFI] <CNF> DataBuf(%d)= %s\n", size, buf);
            break;
        }
        default:
        {
            printf("[META_WIFI] %s is Null!",__func__);
            return;
        }
    }
}

int main()
{
    FT_WM_WIFI_REQ req;

    memset(&req, 0, sizeof(FT_WM_WIFI_REQ));

    META_WIFI_Register(wifi_info_callback);

    if (META_WIFI_init() == false) {
        printf("WLAN init failed\n");
        return -1;
    }

    #if 0 /* test program for load script function */

    req.type = WIFI_CMD_SCRIPT;
    char *cmd1 = "adb shell iwpriv wlan0 driver set_test_mode 2011 2011";

    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd1, strlen(cmd1));
    META_WIFI_OP(&req, cmd1, strlen(cmd1));

    char *cmd2 = "adb shell iwpriv driver set_mcr 2011 2011";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd2, strlen(cmd2));
    META_WIFI_OP(&req, cmd2, strlen(cmd2));

    char *cmd3 = "adb shell iwpriv driver get_mcr 0x820F4020";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd3, strlen(cmd3));
    META_WIFI_OP(&req, cmd3, strlen(cmd3));


    char *cmd4 = "adb shell iwpriv driver set_mcr 0x820F4020 0x14141414";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd4, strlen(cmd4));
    META_WIFI_OP(&req, cmd4, strlen(cmd4));


    char *cmd5 = "adb shell iwpriv driver get_mcr 0x820F4020";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd5, strlen(cmd5));
    META_WIFI_OP(&req, cmd5, strlen(cmd5));

    //GET_NOISE
    char *cmd6 = "adb shell iwpriv driver get_noise";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd6, strlen(cmd6));
    META_WIFI_OP(&req, cmd6, strlen(cmd6));

    //GET_NOISE
    char *cmd7 = "adb shell iwpriv get_noise";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd7, strlen(cmd7));
    META_WIFI_OP(&req, cmd7, strlen(cmd7));

    //GET_NOISE
    char *cmd8 = "adb shell get_noise";
    printf("[META_WIFI]  WIFI_CMD_SCRIPT execute:%s sizeof(%lu)\n", cmd8, strlen(cmd8));
    META_WIFI_OP(&req, cmd8, strlen(cmd8));

    #endif

    META_WIFI_deinit();
    META_WIFI_Register(NULL);

    return 0;
}

