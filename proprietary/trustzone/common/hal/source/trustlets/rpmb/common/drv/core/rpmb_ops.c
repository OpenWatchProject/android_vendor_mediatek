/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <drv_fwk.h>
#include <drv_fwk_macro.h>
#include <drv_error.h>
#include <drv_defs.h>

#include "taStd.h"

#include "tlRpmbDriverApi.h"
#include "drRpmbOps.h"
//
// NOTICE THIS!!!
// this is copy from bootable/bootloader/preloader/platform/[project]/security/trustzone
// If preloader has been modified, sync this file.
#include "tz_mem.h"

#include <tz_tags.h>

#define RPMB_GP_PART_SIZE_PERCENT	100
#define RPMB_PART_START_AUTO 0xFFFFFFFF

/* Debug message event */
#define DBG_EVT_NONE        (0)       /* No event */
#define DBG_EVT_CMD         (1 << 0)  /* SEC CMD related event */
#define DBG_EVT_FUNC        (1 << 1)  /* SEC function event */
#define DBG_EVT_INFO        (1 << 2)  /* SEC information event */
#define DBG_EVT_DAT         (1 << 3)
#define DBG_EVT_DBG         (1 << 29)
#define DBG_EVT_WRN         (1 << 30) /* Warning event */
#define DBG_EVT_ERR         (1 << 31) /* Error event */
#define DBG_EVT_ALL         (0xffffffff)
#define DBG_EVT_MASK        (DBG_EVT_ALL)

#define MSG(evt, fmt, args...) \
do {    \
    if ((DBG_EVT_##evt) & DBG_EVT_MASK) { \
        msee_logd("[Driver rpmb:%s]"fmt"\n", __func__, ##args); \
    }   \
} while(0)


#define RPMB_KEY_DUMP 0
#define RPMB_TEST_KEY 0

#if RPMB_TEST_KEY
//
// According to tester.
//
const uint8_t mac_key[32]= {
        0x76, 0x75, 0x74, 0x73,
        0x72, 0x71, 0x70, 0x6F,
        0x6E, 0x6D, 0x6C, 0x6B,
        0x6A, 0x69, 0x68, 0x67,
        0x66, 0x65, 0x64, 0x63,
        0x62, 0x61, 0x39, 0x38,
        0x37, 0x36, 0x35, 0x34,
        0x33, 0x32, 0x31, 0x30};
#endif

uint8_t rpmb_session[8];

/* Static variables */
dciMessage_t *message;
static unsigned int g_msg_auth_key[8] = {0};
static unsigned int g_rpmb_size= 0;
static unsigned int g_emmc_rel_wr_sec_c = 0;
static unsigned int g_rpmb_setkey_flag = 0;

const char * const rpmb_err_msg[] = {
    "",
    "General failure",
    "Authentication failure",
    "Counter failure",
    "Address failure",
    "Write failure",
    "Read failure",
    "Authentication key not yet programmed",

};

typedef struct _rpmb_part {
    uint32_t uid;
    uint32_t start_block;
    uint32_t length;

} DRRPMB_PART, *PDRRPMB_PART;

static DRRPMB_PART rpmb_part_tbl[RPMB_MAX_USER_NUM] = {
    {RDA_ID,	RPMB_PART_START_AUTO, 0x4000}, //16KB.
    {WIDEVINE_ID, 	RPMB_PART_START_AUTO, 0x4000},
    {SOTER_ATTK, 	RPMB_PART_START_AUTO, 0x4000},
    {SOTER_COUNTER, 	RPMB_PART_START_AUTO, 0x4000},
    {GATEKEEPER_ID, 	RPMB_PART_START_AUTO, 0x4000},
    {KEYMASTER_ID, 	RPMB_PART_START_AUTO, 0x4000},
    {UNDEFINED6_ID, 	RPMB_PART_START_AUTO, 0x4000},
    {UNDEFINED7_ID, 	RPMB_PART_START_AUTO, 0x4000}
};

static unsigned int rpmb_user_cnt = 0;

int drRpmbInit()
{
    uint32_t ret;
    uint8_t *tee_parm_vaddr = 0;
    uint8_t sec_mem_tag_buf[TAG_STRUCT_MAX_SIZE];
    uint8_t *sec_mem_tag_ptr = sec_mem_tag_buf;

    if((ret = msee_mmap_region((uint64_t)TEE_PARAMETER_BASE, (void **)&tee_parm_vaddr, 0x1000, MSEE_MAP_READABLE)) != DRV_FWK_API_OK) {
        MSG(ERR, "MAP SRAM to virtual address fail! ERROR: %d\n", ret);
        return -1;
    }

    // copy memory pool information to sec_mem_arg
    msee_memcpy((void *)sec_mem_tag_buf, (uint8_t *)(tee_parm_vaddr + (TEE_PARAMETER_ADDR - TEE_PARAMETER_BASE)), TAG_STRUCT_MAX_SIZE);
    // unmap tee_parm_vaddr since it's useless
    msee_unmmap_region(tee_parm_vaddr, 0x1000);

    if (IS_TAG_FMT((struct tz_tag*)(sec_mem_tag_ptr))) {
        struct tz_tag* flash_dev_tag = find_tag((unsigned long)sec_mem_tag_ptr, TZ_TAG_FLASH_DEV_INFO);
        if (!IS_NULL_TAG(flash_dev_tag)) {
            memcpy(g_msg_auth_key, (uint8_t *)&flash_dev_tag->u.flash_dev_cfg.rpmb_msg_auth_key, RPMB_SZ_MAC);
            g_rpmb_size = flash_dev_tag->u.flash_dev_cfg.rpmb_size;
            g_emmc_rel_wr_sec_c = flash_dev_tag->u.flash_dev_cfg.emmc_rel_wr_sec_c;
            g_rpmb_setkey_flag = flash_dev_tag->u.flash_dev_cfg.emmc_setkey_flag;
        }
    } else {
        sec_mem_arg_t *sec_mem_arg = (sec_mem_arg_t *)sec_mem_tag_ptr;
        memcpy(g_msg_auth_key, sec_mem_arg->msg_auth_key, RPMB_SZ_MAC);
        g_rpmb_size = sec_mem_arg->rpmb_size;
        g_emmc_rel_wr_sec_c = sec_mem_arg->emmc_rel_wr_sec_c;
#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
        g_rpmb_setkey_flag = sec_mem_arg->rpmb_setkey_flag;
#endif
    }

    MSG(INFO, "rpmb size is %x\n", g_rpmb_size);

    drRpmbPartInit(g_rpmb_size);

    return 0;
}

void drRpmbPartInit(uint32_t rpmb_size)
{
    int i;
    uint32_t start_blk;

    start_blk = (rpmb_size / 256) * (100 - RPMB_GP_PART_SIZE_PERCENT) / 100;

    rpmb_part_tbl[0].start_block = start_blk;

    for (i = 1; i < RPMB_MAX_USER_NUM; i++)
        rpmb_part_tbl[i].start_block = rpmb_part_tbl[i - 1].start_block + rpmb_part_tbl[i - 1].length / 256;

}
int drRpmbOpenSession(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_OPEN_SESSION\n", __func__);

    if (rpmb_session[pRpmbData->uid] == 0) {
        rpmb_session[pRpmbData->uid] = 1;
        MSG(INFO, "rpmb open session %d\n", pRpmbData->uid);
        pRpmbData->ret = 0;
    } else {
        MSG(ERR, "rpmb session %d is already opened\n", pRpmbData->uid);
        pRpmbData->ret = -1;
    }
    return 0;
}
int drRpmbCloseSession(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_CLOSE_SESSION\n", __func__);

    if (rpmb_session[pRpmbData->uid] == 1) {
        rpmb_session[pRpmbData->uid] = 0;
        MSG(INFO, "rpmb close session %d\n", pRpmbData->uid);
        pRpmbData->ret = 0;
    } else {
        MSG(ERR, "rpmb session %d is not opened\n ", pRpmbData->uid);
        pRpmbData->ret = -1;;
    }
    return 0;
}

int drRpmbGetKey(tlApiRpmb_ptr pRpmbData)
{
#if RPMB_KEY_DUMP
    int i;
    MSG(DBG, "[RPMB][KEY]");
    for (i = 0; i < 8; i++)
        MSG(DBG, "%x, ", g_msg_auth_key[i]);
#endif

    MSG(INFO, "%s: cmd_id: FID_DRV_GET_KEY\n", __func__);

    memcpy(pRpmbData->mac_key, g_msg_auth_key, RPMB_SZ_MAC);
#if RPMB_TEST_KEY
    memcpy(pRpmbData->mac_key, mac_key, RPMB_SZ_MAC);
#endif
    return 0;
}

int drRpmbGetRelWrSecC(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s, cmd_id: FID_DRV_GET_REL_WR_SEC_C\n", __func__);

    pRpmbData->emmc_rel_wr_sec_c = (uint16_t)g_emmc_rel_wr_sec_c;
    MSG(DBG, "get rel write cnt: %d\n", pRpmbData->emmc_rel_wr_sec_c);
    return 0;
}

int drRpmbGetPartSize(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_GET_PART_SIZE\n", __func__);
    pRpmbData->part_size = rpmb_part_tbl[pRpmbData->uid].length;
    MSG(DBG, "User%d get part size: %d\n", pRpmbData->uid, pRpmbData->part_size);
    return 0;
}

int drRpmbGetPartAddr(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_GET_PART_ADDR\n", __func__);
    pRpmbData->part_addr = rpmb_part_tbl[pRpmbData->uid].start_block;
    MSG(DBG, "User%d get part addr: %d\n", pRpmbData->uid, pRpmbData->part_addr);
    return 0;
}

int drRpmbReadData(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_READ_DATA\n", __func__);
    pRpmbData->result = 0;
    message->command.header.commandId = DCI_RPMB_CMD_READ_DATA;
    message->request.type = RPMB_READ_DATA;
    message->request.addr = pRpmbData->start_blk;
    message->request.blks = pRpmbData->blks;
    memcpy(message->request.frame, pRpmbData->buf, 512 * pRpmbData->blks);
    MSG(INFO, "atomicSendReceive read data start\n");
    msee_notify_ree();
    MSG(INFO, "atomicSendReceive read data end\n");
    pRpmbData->result = message->request.result;
    memcpy(pRpmbData->buf, message->request.frame, 512 * pRpmbData->blks);
    MSG(DAT, "frame[196]=%x", message->request.frame[196]);
    MSG(DAT, "frame[197]=%x", message->request.frame[197]);
    MSG(DAT, "frame[198]=%x", message->request.frame[198]);
    MSG(DAT, "frame[199]=%x", message->request.frame[199]);

    return 0;
}

int drRpmbGetWcounter(tlApiRpmb_ptr pRpmbData)
{
    uint16_t wc;

    MSG(INFO, "%s: cmd_id: FID_DRV_GET_WCOUNTER\n", __func__);

    message->command.header.commandId = DCI_RPMB_CMD_GET_WCNT;
    message->request.type = RPMB_GET_WRITE_COUNTER;
    message->request.addr = 0;
    message->request.blks = 1;

    memcpy(message->request.frame, pRpmbData->buf, 512);

    MSG(INFO, "atomicSendReceive get wc start\n");
    msee_notify_ree();
    MSG(INFO, "atomicSendReceive get wc end\n");

    memcpy(pRpmbData->buf, message->request.frame, 512);

    wc = cpu_to_be32p(message->request.frame + RPMB_WCOUNTER_BEG);
    MSG(DBG, "wc=%x", wc);

    return 0;
}

int drRpmbWriteData(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_WRITE_DATA\n", __func__);

    message->command.header.commandId = DCI_RPMB_CMD_WRITE_DATA;
    message->request.type = RPMB_WRITE_DATA;
    message->request.addr = pRpmbData->start_blk;
    message->request.blks = pRpmbData->blks;

    memcpy(message->request.frame, pRpmbData->buf, 512 * pRpmbData->blks);
    MSG(DAT, "RpmbData->buf[196]=%x", pRpmbData->buf[196]);
    MSG(DAT, "RpmbData->buf[228]=%x", pRpmbData->buf[228]);
    MSG(DAT, "RpmbData->buf[229]=%x", pRpmbData->buf[229]);
    MSG(DAT, "RpmbData->buf[230]=%x", pRpmbData->buf[230]);
    MSG(DAT, "RpmbData->buf[231]=%x", pRpmbData->buf[231]);
    MSG(DAT, "RpmbData->buf[506]=%x", pRpmbData->buf[506]);
    MSG(DAT, "RpmbData->buf[507]=%x", pRpmbData->buf[507]);

    MSG(INFO, "atomicSendReceive write data start\n");
    msee_notify_ree();
    MSG(INFO, "atomicSendReceive write data end\n");
    memcpy(pRpmbData->buf, message->request.frame, 512);

    return 0;
}

#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
int drRpmbProgramKey(tlApiRpmb_ptr pRpmbData)
{
    MSG(INFO, "%s: cmd_id: FID_DRV_RROGRAM_KEY\n", __func__);

    message->command.header.commandId = DCI_RPMB_CMD_PROGRAM_KEY;
    message->request.type = RPMB_PROGRAM_KEY;
    message->request.addr = 0;
    message->request.blks = 1;

    memcpy(message->request.frame, pRpmbData->buf, 512);
    /* memcpy(message->request.frame + RPMB_MAC_BEG, pRpmbData->mac_key, RPMB_SZ_MAC); *
    /* message->request.frame[RPMB_TYPE_BEG + 1] = RPMB_PROGRAM_KEY; */

    MSG(INFO, "atomicSendReceive program key start\n");
    msee_notify_ree();
    MSG(INFO, "atomicSendReceive program key end\n");
    memcpy(pRpmbData->buf, message->request.frame, 512);

    return 0;
}

int drRpmbProgramKeyCheckFlag(tlApiRpmb_ptr pRpmbData)
{
     MSG(INFO, "%s: cmd_id: FID_DRV_CHECK_FLAG\n", __func__);
     pRpmbData->result = g_rpmb_setkey_flag;
     MSG(INFO, "rpmb program key flag is %d\n", pRpmbData->result);

     return 0;
}

int drRpmbProgramKeySetFlag(tlApiRpmb_ptr pRpmbData)
{
     MSG(INFO, "%s: cmd_id: FID_DRV_SET_KEY_FLAG\n", __func__);
     g_rpmb_setkey_flag = pRpmbData->result;
     MSG(INFO, "rpmb program flag is set to %d\n", g_rpmb_setkey_flag);

     return 0;
}
#endif

int drRpmbGetSize()
{
    return 128*1024*2; //temp 256KB.
}

uint16_t cpu_to_be16p(uint16_t *p)
{
    return (((*p << 8)&0xFF00) | (*p >> 8));
}

uint32_t cpu_to_be32p(uint32_t *p)
{
    return (((*p & 0xFF) << 24) | ((*p & 0xFF00) << 8) | ((*p & 0xFF0000) >> 8) | (*p & 0xFF000000) >> 24 );
}
