/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "mt_check_partition.h"

#include "mt_partition.h"
#include "mt_pmt.h"
#include "recovery_ui/ui.h"

extern RecoveryUI* ui;
static int check_part_size_mntl(__unused ZipArchiveHandle zip)
{
    /* MNTL project */
    return 0;
}

/* If check part size pass return 0 */
static int check_part_size_emmc(ZipArchiveHandle zip)
{
    int ret = 1 , part_num_scatter = 0, part_num_device = 0, i = 0;
    part_info_t *part_device[MAX_PARTITION_NUM], *part_scatter[MAX_PARTITION_NUM];

    memset(part_device, 0, sizeof(part_info_t*)*MAX_PARTITION_NUM);
    memset(part_scatter, 0, sizeof(part_info_t*)*MAX_PARTITION_NUM);

    /* 1. Get partition info from scatter in update.zip */
    ret = get_partition_info_from_scatter(part_scatter, &part_num_scatter,zip);

    if(ret) {
        //ui->Print("Error: get_partition_info_from_scatter fail\n");
        goto check_fail;
    }

    /* 2. Get partition info from device */
    ret = get_partition_info(part_device, &part_num_device);

    if(ret) {
        //ui->Print("Error: get_partition_info from device fail\n");
        goto check_fail;
    }

    /* 3. Compare partition info between scatter and device */
    if(part_num_scatter != part_num_device) {
        //ui->Print("Error: Partition table not match\n");
        //ui->Print("Partition scatter %d\n", part_num_scatter);
        //ui->Print("Partition device %d\n", part_num_device);
        ret = 1;
        goto check_fail;
    }

    for(i=0; i< part_num_device; i++)
    {
        if((part_scatter[i]->offset != part_device[i]->offset) ||
           (strcasecmp(part_scatter[i]->name, part_device[i]->name) != 0)) {
            //ui->Print("Invalid partition setting \n");
            //ui->Print("index:%d partition:%s device:%jx scatter:%jx\n",
            //i, part_device[i]->name, part_device[i]->offset, part_scatter[i]->offset);
            ret = 1;
            goto check_fail;
        }
    }

check_fail:
    for (i = 0; i < part_num_scatter; i++) {
        free(part_scatter[i]);
    }
    for (i = 0; i < part_num_device; i++) {
        free(part_device[i]);
    }
    return ret;
}

/* If check part size pass return 0 */
int check_part_size(ZipArchiveHandle zip)
{
    int ret = 1;
    if(mt_get_phone_type()  == FS_TYPE_MNTL) { /* MNTL */

        ret = check_part_size_mntl(zip);
    }
    else {
        ret = check_part_size_emmc(zip);
    }

    return ret;
}

