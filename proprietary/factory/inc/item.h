/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */

#ifndef _FACTORYMODE_ITEMS_H_
#define _FACTORYMODE_ITEMS_H_

#ifdef __cplusplus
extern "C" {
#endif
 
  
int get_item_id(item_t *item, char *name);
const char * get_item_name(item_t *item, int id);
item_t *get_auto_item_list(void);
item_t *get_manual_item_list(void);
item_t *get_debug_item_list(void);
item_t *get_item_list(void);
int get_item_test_type(item_t *item, const char *name);
extern int mcard_init(void);

#ifdef __cplusplus
}
#endif


#endif
