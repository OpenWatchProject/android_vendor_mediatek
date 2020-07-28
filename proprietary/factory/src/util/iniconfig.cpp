/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */ 
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "utils.h"




#define TAG         "[INI] "
 
extern item_t ftm_auto_test_items[];
//extern item_t pc_control_items[]; 
//extern item_t ftm_debug_test_items[];
extern item_t ftm_test_items[];
extern item_t ftm_cust_items[ITEM_MAX_IDS];
extern item_t ftm_cust_auto_items[ITEM_MAX_IDS];
 

static char *trimspace(char *s)
{
	char *e;
    if(s == NULL)
    {
        return NULL;
    }
	while (isspace(*s)) 
    {   
        s++;
    }

	e = s + strlen(s) - 1;

	while (e > s && isspace(*e)) 
    {   
        e--;
    }

	*(e + 1) = '\0';

	return s;
}

int read_config(char *filename)
{
    char  buf[BUFSZ]={0};
    char *name, *val, *p, *test_type;
    int   num = 0, i, id, len, limit;
    int   auto_num = 0;
    item_t *items;
    item_t *auto_items;
    FILE *fp;

    if(filename == NULL)
    {
        return -1;
    }

    if (NULL == (fp = fopen(filename, "r")))
        return num;

    memset(ftm_cust_items, 0, sizeof(ftm_cust_items));
    memset(ftm_cust_auto_items, 0, sizeof(ftm_cust_auto_items));
    items = ftm_cust_items;
    auto_items = ftm_cust_auto_items;
    limit = ARRAY_SIZE(ftm_cust_items) - 1;

    while (fgets(buf, BUFSZ, fp)) 
    {
        if (NULL != (val = strstr(buf, "//")))
        {
            *val = '\0';
        }
        if (NULL != (val = strchr(buf, ';')))
        {
            *val = '\0';
        }
        if (NULL != (val = strchr(buf, '(')))
        {
            test_type = val+1;
            *val = '\0';
        }

        len = strlen(buf);
        if (!len)
        {
            continue;
        }

        name = p = buf;
        val  = NULL;

        for (i = 0; i < len; i++, p++) 
        {
            if ((*p == '=') && ((i + 1) < len)) 
            {
                *p = '\0';
                val = p + 1;
                break;
            }
        }
        if (i == len)
        {
            continue;
        }

        if (name)
        {
            name = trimspace(name);
        }
        if (val)
        {
            val = trimspace(val);
        }

        if (strcasestr(name, "menuitem") && num < limit) 
        {
            if (val && ((id = get_item_id(&ftm_test_items[0], val)) >= 0)) 
            {
                LOGD(TAG "set menuitem[%d]: %s (%d)\n", num, val, id);
                items[num].id   = id;
                items[num].name = strdup(val);
                if(*test_type == 'A')
                {
                    auto_items[auto_num].id = id;
                    auto_items[auto_num].name = strdup(val);
                    auto_items[auto_num].mode = FTM_AUTO_ITEM;
                    items[num].mode = FTM_AUTO_ITEM;
                    auto_num++;
                }
                else
                {
                    items[num].mode = FTM_MANUAL_ITEM;
                }
				LOGD(TAG "items[%d].mode=%d\n", num, items[num].mode);
                num++;
            }
        } 
        else 
        {
            LOGD(TAG "set prop: %s=%s\n", name, val ? val : "null");
            ftm_set_prop(name, val);
        }
    }
    items[num].id   = -1;
    items[num].name = NULL;

    fclose(fp);

    return num;

}


