/* Copyright Statement:
*
* This oftware/firmware and related documentation ("MediaTek Software") are
* proteed under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MedTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESCT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALLALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THEEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE T ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#include "dynamic_object.h"
#include "do_service.h"
#include <FreeRTOS.h>
#include <task.h>
#include <dma.h>
#ifdef VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#include <stdio.h>
#include <string.h>
#include <mt_gpt.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DMA_RETRY 100
#define DMA_RETRY_DELAY_MS 10

typedef struct do_list_node{
    DOHeader_t *header;
    char *feat_list;
    struct do_list_node *next;
}DOListNode_t;

static DOHeader_t *current_do = NULL;
static DOListNode_t *headers = NULL;
static DOListNode_t *headers_tail = NULL;
//static uint32_t total_num_do = 0;
static uint32_t inited = 0;
static uint32_t do_start_addr = 0;

static uint32_t dma_ch = GENERAL_DMA_CH;

extern const uint32_t __do_ws_start;
extern const uint32_t __do_ws_end;

static uint32_t is_do_header_section_valid(doSectionsInfo_t *h, uint32_t do_ws_s, uint32_t do_ws_e);
static void init_do_bss(doSectionsInfo_t *h);
static int run_features_init(doSectionsInfo_t *h);
static int run_features_deinit(doSectionsInfo_t *h);

static const uint32_t do_pseudo_entry_len = 4;

/***************** DO helper functions *******************/
static uint32_t check_magic_hex(doSectionsInfo_t *h)
{
    if (h->magic[0] != DO_MAGIC_0 || h->magic[1] != DO_MAGIC_1 ||
            h->magic[2] != DO_MAGIC_2 || h->magic[3] != DO_MAGIC_3) {
        PRINTF_D("WARN: Magic hex don't match\n");
        return 0;
    }

    return 1;
}

static uint32_t check_boundary(doSectionsInfo_t *h, uint32_t s, uint32_t e)
{
    // there must be feat_funcs in .do_features, so do_features_end will
    // always be larger than do_features_start
    if (h->do_features_start >= h->do_features_end || h->do_features_end >= e) {
        PRINTF_E("do_features_end is invalid\n");
        return 0;
    }

    if (h->do_exidx_start != 0 &&
        (h->do_features_end > h->do_exidx_start || h->do_exidx_start >= e)) {
        PRINTF_E("do_exidx_start is invalid\n");
        return 0;
    }

    if (h->do_exidx_end != 0 &&
        (h->do_exidx_start > h->do_exidx_end || h->do_exidx_end > e)) {
        PRINTF_E("do_exidx_end is invalid\n");
        return 0;
    }

    // since the section order of DO is pre-defined, .do_header and .do_features
    // must be in front of .bss, do_bss_start should be always larger than
    // or equal to do_features_end
    if (h->do_bss_start != 0 &&
        (h->do_features_end > h->do_bss_start || h->do_bss_start >= e)) {
        PRINTF_E("do_bss_start is invalid\n");
        return 0;
    }

    // it's possible that len(bss) == 0, if so, do_bss_start == do_bss_end
    if (h->do_bss_end != 0 &&
        (h->do_bss_start > h->do_bss_end || h->do_bss_end > e)) {
        PRINTF_E("do_bss_end is invalid\n");
        return 0;
    }

    return 1;
}

static uint32_t is_do_header_section_valid(doSectionsInfo_t *h, uint32_t do_ws_s, uint32_t do_ws_e)
{
    PRINTF_D("&__do_ws_start = 0x%x\n", do_ws_s);
    PRINTF_D("&__do_ws_end = 0x%x\n", do_ws_e);
    PRINTF_D("magic[0] = 0x%x\n", h->magic[0]);
    PRINTF_D("magic[1] = 0x%x\n", h->magic[1]);
    PRINTF_D("magic[2] = 0x%x\n", h->magic[2]);
    PRINTF_D("magic[3] = 0x%x\n", h->magic[3]);
    PRINTF_D("do_features_start = 0x%x\n", h->do_features_start);
    PRINTF_D("do_features_end = 0x%x\n", h->do_features_end);
    PRINTF_D("do_exidx_start = 0x%x\n", h->do_exidx_start);
    PRINTF_D("do_exidx_end = 0x%x\n", h->do_exidx_end);
    PRINTF_D("do_bss_start = 0x%x\n", h->do_bss_start);
    PRINTF_D("do_bss_end = 0x%x\n", h->do_bss_end);

    if (check_magic_hex(h) == 0)
        return 0;

    if (check_boundary(h, do_ws_s, do_ws_e) == 0)
        return 0;

    // TODO: alignment checking ?

    return 1;
}

static void init_do_bss(doSectionsInfo_t *h)
{
    memset((void *)h->do_bss_start, 0, h->do_bss_end - h->do_bss_start);
}

/***************** DO load/callback handling functions *******************/
static int run_features_init(doSectionsInfo_t *h)
{
    doFeatureFunc_t *feat_funcs = NULL;
    for (feat_funcs = (doFeatureFunc_t *)(h->do_features_start);
            feat_funcs != (doFeatureFunc_t *)(h->do_features_end);
            feat_funcs++) {
        PRINTF_D("call init(): addr = 0x%x\n", (uint32_t) feat_funcs->init_func);
        // TODO: check if init_func is valid
        if (feat_funcs->init_func) {
            feat_funcs->init_func();
        } else {
            PRINTF_E("ERR: NULL init()\n");
            return 0;
        }
    }
    return 1;
}

static int run_features_deinit(doSectionsInfo_t *h)
{
    doFeatureFunc_t *feat_funcs = NULL;
    for (feat_funcs = (doFeatureFunc_t *)(h->do_features_start);
            feat_funcs != (doFeatureFunc_t *)(h->do_features_end);
            feat_funcs++) {
        PRINTF_D("call deinit(): addr = 0x%x\n", (uint32_t) feat_funcs->deinit_func);
        // TODO: check if deinit_func is valid
        if (feat_funcs->deinit_func) {
            feat_funcs->deinit_func();
        } else {
            PRINTF_E("ERR: NULL deinit()\n");
            return 0;
        }
    }
    return 1;
}

static inline uint32_t mt_do_init()
{
    doSectionsInfo_t *do_header_section = (doSectionsInfo_t *) &__do_ws_start;
    if (is_do_header_section_valid(do_header_section, (uint32_t)&__do_ws_start, (uint32_t)&__do_ws_end)) {
        init_do_bss(do_header_section);
        return run_features_init(do_header_section);
    } else {
        // TODO: error handling if do_header is invalid
        PRINTF_E("ERR: not a valid DO image\n\r");
        return 0;
    }
}

static inline uint32_t mt_do_deinit()
{
    int ret;
    doSectionsInfo_t *do_header_section = (doSectionsInfo_t *) &__do_ws_start;
    if (is_do_header_section_valid(do_header_section, (uint32_t)&__do_ws_start, (uint32_t)&__do_ws_end)) {
        ret = run_features_deinit(do_header_section);

        // clear task TCBs after calling DO deinit callbacks
        prvCheckTasksWaitingTermination();

        return ret;
    } else {
        // TODO: error handling if do_header is invalid
        PRINTF_E("ERR: not a valid DO image\n\r");
        return 0;
    }
}


/*
 * copy dynamic object from dram to sram through DMA
 * ret: 1 - success
 *      0 - failed
 */
static int load_do_from_dram(DOHeader_t *header)
{
    DMA_RESULT ret;
    uint32_t dram, sram, size;
    uint32_t res = 0;
    uint32_t count = 0;

    // Load DO through DMA
    if (header != NULL) {
        PRINTF_D("load do from dram: %s\n\r", header->id);

        dram = header->dram_addr;
        sram = header->sram_addr;
        //sram = (uint32_t)(&__do_ws_start);
        size = header->size;
        PRINTF_D("sram: 0x%x, dram: 0x%x, size: 0x%x\n\r", sram, dram, size);

#ifdef VCORE_DVFS_SUPPORT
        dvfs_enable_DRAM_resource(FRAM_MEM_ID);
#endif
        do {
            if (count)
                mdelay(DMA_RETRY_DELAY_MS); // delay 10 ms
            ret = dma_transaction_manual(sram, dram, size, NULL, (uint32_t*)&dma_ch);
            count++;
        } while(ret && count < MAX_DMA_RETRY); // retry max 100

        if (!ret) { // if ret == DMA_RESULT_DONE
            res = 1;
        } else {
            PRINTF_E("load do from dram: DMA retry fail. MAX_DMA_RETRY=%u\n", MAX_DMA_RETRY);
            PRINTF_E("load do from dram: last DMA ret=%u\n", ret);
            configASSERT(0);
        }

#ifdef VCORE_DVFS_SUPPORT
        dvfs_disable_DRAM_resource(FRAM_MEM_ID);
#endif
    }

    return res;
}

static DOHeader_t* get_target_do(char *do_name)
{
    DOListNode_t *ptr;
    char *name;
    DOHeader_t *header = NULL;

    if (!inited) {
        // not initialized yet, return -1
        PRINTF_D("DO not initialized yet\n\r");
        return NULL;
    }

    // Find target DO
    ptr = headers;
    while (ptr) {
        name = (char *)ptr->header->id;
        if (!strcmp(name, do_name)) {
            header = ptr->header;
            break;
        }
        ptr = ptr->next;
    }
    if (!header) {
        PRINTF_D("Cannot find target do %s in list\n\r", do_name);
    }
    return header;
}

/********************* DO Public API (not opened yet) **********************/
uint32_t mt_do_load_do(char *do_name)
{
    uint32_t ret;
    DOHeader_t *header;

    PRINTF_D("mt_do_load_do: load %s\n\r", do_name);
    header = get_target_do(do_name);
    if (!header) {
        PRINTF_E("mt_do_load_do: cannot find/parse do %s\n\r", do_name);
        return 0;
    }

    if (current_do == header) {
        PRINTF_D("mt_do_load_do: DO %s already loaded\n\r", do_name);
        return 0;
    }

    if (current_do) {
        ret = mt_do_deinit();
        if (!ret) {
            PRINTF_E("mt_do_load_do: call %s deinit cbs failed.\n", current_do->id);
            return 0;
        }
    }
    ret = load_do_from_dram(header);
    if (!ret) {
        PRINTF_E("mt_do_load_do: load %s from DRAM failed\n\r", do_name);
        return 0;
    }
    ret = mt_do_init();
    if (!ret) {
        PRINTF_E("mt_do_load_do: call do %s init callbacks failed.\n", do_name);
        return 0;
    }
    current_do = header;

    return 1;
}

uint32_t mt_do_unload_do(char *do_name)
{
    uint32_t ret;
    DOHeader_t* header;

    PRINTF_D("mt_do_unload_do: unload %s\n\r", do_name);
    header = get_target_do(do_name);
    if (!header) {
        PRINTF_E("mt_do_unload_do: cannot find/parse do %s\n\r", do_name);
        return 0;
    }

    if (current_do != NULL && !strcmp((char *)current_do->id, do_name)) {
        ret = mt_do_deinit();
        if (!ret) {
            PRINTF_E("mt_do_load_do: call %s deinit cbs failed.\n", current_do->id);
            return 0;
        }
        current_do = NULL;
        return 1;
    } else {
        PRINTF_D("mt_do_unload_do: DO %s not loaded\n\r", do_name);
        return 0;
    }
}

char* mt_do_get_loaded_do(void)
{
    if (current_do) {
        return (char *)current_do->id;
    } else {
        return NULL;
    }
}

uint32_t mt_do_load_first_do(void)
{
    uint32_t ret;

    if (!headers || !headers->header) {
        PRINTF_E("mt_do_load_first_do: headers or headers->header is NULL\n\r");
        return 0;
    }

    ret = load_do_from_dram(headers->header);
    if (!ret) {
        PRINTF_E("mt_do_load_first_do: load first from DRAM failed\n\r");
        return 0;
    }
    ret = mt_do_init();
    if (!ret) {
        PRINTF_E("mt_do_load_first_do: call first init callbacks failed.\n");
        return 0;
    }
    current_do = headers->header;

    return 1;
}
/****************** DO manager initialization ******************/
static inline uint32_t check_part_magic(DO_part *header)
{
    char buf[50];
    int i;

    for (i = 0; i < DO_MAGIC_LENGTH; i++) {
        buf[i] = header->magic[i];
    }
    buf[DO_MAGIC_LENGTH] = '\0';
    PRINTF_D("check magic: %s\n", buf);
    if (!memcmp(header->magic, DO_MAGIC, DO_MAGIC_LENGTH))
        return 1;
    else
        return 0;
}

static void insert_do(DO_part *header, char *feat_list, uint32_t addr, int idx)
{
    /* insert to headers */
    if (headers == NULL) {
        headers = pvPortMalloc(sizeof(DOListNode_t));
        headers_tail = headers;
    } else {
        headers_tail->next = pvPortMalloc(sizeof(DOListNode_t));
        headers_tail = headers_tail->next;
    }

    headers_tail->header = pvPortMalloc(sizeof(DOHeader_t));
    strncpy((char *)headers_tail->header->id,
            header->id,
            sizeof(headers_tail->header->id));
    headers_tail->header->dram_addr = addr;
    headers_tail->header->sram_addr = (uint32_t)(&__do_ws_start);
    headers_tail->header->size = header->len;
    headers_tail->header->featlist_len = header->feat_list_len;
    headers_tail->next = NULL;

    headers_tail->feat_list = feat_list;
}

/*
 * parse DO partition in DRAM & initialize do tables
 * containing sram addr, dram addr, size & feature list
 * ret: total number of do parsed
 *      -1 if failed.
 */
int32_t mt_do_parse_headers(uint32_t *addr)
{
    DO_part header;
    char *feat_list;
    uint32_t do_addr, offset;
    uint32_t feat_list_len = 0;
    int32_t ret, do_num = 0, failed = 0;

    /* save DO dram start addr */
    do_start_addr = ap_to_scp(*addr);
    offset = do_start_addr;
    PRINTF_D("parse_headers, ap addr: 0x%x, scp addr: 0x%x\n", *addr, offset);

#ifdef VCORE_DVFS_SUPPORT
    /* enable DRAM */
    dvfs_enable_DRAM_resource(FRAM_MEM_ID);
#endif

    /* copy DO part header from DRAM */
    ret = dma_transaction_manual(
            (uint32_t) &header,
            offset,
            sizeof(DO_part),
            NULL, (uint32_t*)&dma_ch);
    if (ret) { // if ret == DMA_RESULT_DONE
        PRINTF_E("copy DO header failed in DMA: %u\n", ret);
        failed = 1;
    }

    while (!failed && check_part_magic(&header)) {

        if (!header.len) {
            PRINTF_D("WARN: do empty\n");
        }

        /* get feature list (copy from DRAM) */
        feat_list_len = header.feat_list_len;
        feat_list = (char *)pvPortMalloc(sizeof(char) * (feat_list_len + 1));

        ret = dma_transaction_manual(
                (uint32_t) feat_list,
                offset + sizeof(DO_part), /* may need to check addr boundary here */
                feat_list_len,
                NULL, (uint32_t*)&dma_ch);
        if (ret) { // if ret != DMA_RESULT_DONE
            PRINTF_E("copy DO header failed in DMA: %u\n", ret);
            failed = 1;
            break;
        }

        do_addr = offset + sizeof(DO_part) + feat_list_len;

        insert_do(&header, feat_list, do_addr, do_num);
        PRINTF_D("insert do %s : addr = 0x%x, len = 0x%x\n",
             header.id, do_addr, header.len);
        PRINTF_D("feature list: %s\n", feat_list);

        /* point to the next do header */
        offset = do_addr + header.len;/* may need to check addr boundary here */

        PRINTF_D("parse next header: 0x%x\n", offset);
        ret = dma_transaction_manual(
                (uint32_t) &header,
                offset,
                sizeof(DO_part),
                NULL, (uint32_t*)&dma_ch);
        if (ret) { // if ret != DMA_RESULT_DONE
            PRINTF_E("copy DO header failed in DMA: %u\n", ret);
            failed = 1;
            break;
        }
        do_num++;
    }
#ifdef VCORE_DVFS_SUPPORT
    /* disable DRAM */
    dvfs_disable_DRAM_resource(FRAM_MEM_ID);
#endif

    if (failed){
        PRINTF_E("mt_do_parse_headers failed.\n");
        return -1;
    }

    inited = 1;
    return do_num;
}

uint32_t mt_do_send_obj_info(void)
{
    uint8_t buf[IPI_BUFF_SIZE];
    uint8_t *next_header;
    char *p_feats;
    DOHeader_t *ptr;
    int ret;
    uint32_t do_sent = 0;
    uint32_t payload_size = 4;  /* 4 bytes for DOInfo.num_of_do */
    uint32_t str_len;

    /* Make DOInfo header */
    DOInfo_t *info = (DOInfo_t *)buf;

    /* Append DOHeaders */
    DOListNode_t *node = headers;

    ptr = &(info->headers);
    while (node) {
        str_len = node->header->featlist_len;
        next_header = ((uint8_t*)ptr) + sizeof(DOHeader_t) + str_len;

        /* check if the buffer is going to be full */
        if (next_header >= &(buf[IPI_BUFF_SIZE])) {
            PRINTF_W("size of DOs info exceeds buffer size, send ipi first\n");

            info->num_of_do = do_sent;
            ret = do_ipi_send(DO_MSG_INFO, buf, payload_size);

            /* clear buffer */
            ptr = &(info->headers);
            payload_size = 0;
            do_sent = 0;
        }

        strncpy((char *)ptr->id, (char *)node->header->id, sizeof(ptr->id));
        ptr->dram_addr = node->header->dram_addr; /* NOTE: assume only needs 32 bits */
        ptr->sram_addr = node->header->sram_addr;
        ptr->size = node->header->size;
        ptr->featlist_len = node->header->featlist_len;

        PRINTF_D("---- DO INFO ------\n");
        PRINTF_D("name: %s\n", ptr->id);
        PRINTF_D("dram: 0x%x\n", ptr->dram_addr);
        PRINTF_D("size: %u\n", ptr->size);
        PRINTF_D("featlist len: %u\n", ptr->featlist_len);

        /* Append feature list */
        p_feats = ((char *)ptr) + sizeof(DOHeader_t);
        memcpy(p_feats, node->feat_list, str_len);

        PRINTF_D("feat list: %s\n", p_feats);

        /* Point to the next header */
        ptr = (DOHeader_t *) (p_feats + str_len);
        do_sent++;
        payload_size += (sizeof(DOHeader_t) + str_len);
        node = node->next;

    }
    info->num_of_do = do_sent;
    ret = do_ipi_send(DO_MSG_INFO, buf, payload_size);

    return (ret == 1) ? ret : 0;
}

#ifdef __cplusplus
}
#endif
