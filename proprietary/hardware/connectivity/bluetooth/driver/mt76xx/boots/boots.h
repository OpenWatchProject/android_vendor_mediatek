//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_H__
#define __BOOTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#include "boots_cfg.h"

//---------------------------------------------------------------------------
#define VERSION                 "1.8.4_no_hci"

#define NONE "\033[m"
#define BLUE "\033[0;32;34m"
#define RED "\033[0;32;31m"

#define HCI_BUF_SIZE            0x2000

#define BOOTS_MSG_LVL_DBG       4
#define BOOTS_MSG_LVL_INFO      3
#define BOOTS_MSG_LVL_WARN      2
#define BOOTS_MSG_LVL_ERR       1
#define BOOTS_MSG_LVL_NONE      0

#define BPRINT_D_RAW(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_DBG) \
        printf(fmt, ##__VA_ARGS__);                         } while (0);
#define BPRINT_D(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_DBG) \
        printf("[%s:D] "fmt"\n", LOG_TAG, ##__VA_ARGS__);   } while (0);
#define BPRINT_I_RAW(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_INFO) \
        printf(fmt, ##__VA_ARGS__);                         } while (0);
#define BPRINT_I(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_INFO) \
        printf("[%s] "fmt"\n", LOG_TAG, ##__VA_ARGS__);     } while (0);
#define BPRINT_W(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_WARN) \
        printf(BLUE "[%s:W] "fmt"\n" NONE, LOG_TAG, ##__VA_ARGS__);   } while (0);
#define BPRINT_E(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_ERR) \
        printf(RED "[%s:E] "fmt" !!!\n" NONE, LOG_TAG, ##__VA_ARGS__);} while (0);

#define SHOW_RAW(len, buf) \
    do { \
        int i = 0; \
        while (i < len) { \
            printf("%02X ", buf[i]); \
            i++; \
        } \
        printf("\n"); \
    } while (0);

#define MAX(a, b) \
    (((a) > (b)) ? (a) : (b))

#define ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0]))

#define CHECK_USERID() \
    if (getuid() != 0) { \
        BPRINT_E("Please run boots as ROOT!!!"); \
        exit(1); \
    }

#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

/** Boots BT Interface */
typedef enum {
    BOOTS_IF_NONE = 0,
    BOOTS_BTIF_STPBT,
    BOOTS_BTIF_HCI,
    BOOTS_BTIF_ALL,
    BOOTS_CSIF_SKT,
    BOOTS_CSIF_UART,
    BOOTS_CSIF_ETH,
    BOOTS_CLIF_USER,
    BOOTS_CLIF_UART,
} boots_if_e;

typedef struct {
    boots_if_e inf; // interface
    char *n;        // name
    char *p;        // path
} boots_btif_s;

typedef struct {
    boots_if_e      btif;
    boots_if_e      csif;
    boots_if_e      clif;
    char            bt[16];
    char            cs[16];
    char            cli[16];
    int             cs_speed;
    int             cli_speed;
} boots_if_s;

//---------------------------------------------------------------------------
static boots_btif_s boots_btif[] = {
    {BOOTS_BTIF_STPBT,  "stpbt",    "/dev/stpbt"},
    {BOOTS_BTIF_HCI,    "hci",      NULL},
    {BOOTS_IF_NONE,     "",         NULL},      // Should leave bottom
};

//---------------------------------------------------------------------------
#endif // __BOOTS_H__
