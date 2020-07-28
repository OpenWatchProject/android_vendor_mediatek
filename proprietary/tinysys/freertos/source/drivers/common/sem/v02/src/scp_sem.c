/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <FreeRTOS.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include "scp_sem.h"

int get_scp_semaphore(int flag)
{
    int read_back;
    int count = 0;
    int ret = -1;

    flag *= 2;
    read_back = (DRV_Reg32(SEMAPHORE) >> flag) & 0x1;
    if (read_back == 0) {
        /* try to get sema.*/
        DRV_WriteReg32(SEMAPHORE, (1 << flag));
        while (count != TIMEOUT) {
            /* repeat test if we get semaphore */
            read_back = (DRV_Reg32(SEMAPHORE) >> flag) & 0x1;
            if (read_back == 1) {
                /*get scp sema. here*/
                ret = 1;
                break;
            }
            DRV_WriteReg32(SEMAPHORE, (1 << flag));
            count++;
        }
        if (ret < 0)
            PRINTF_D("get sem timeout\n");
    } else {
        PRINTF_E("ERR sema was hold.\n");
    }

    return ret;
}

int release_scp_semaphore(int flag)
{
    int read_back;
    int ret = -1;

    flag *= 2;
    read_back = (DRV_Reg32(SEMAPHORE) >> flag) & 0x1;
    if (read_back == 0) {
        PRINTF_E("ERR not hold sema.\n");
    } else {
        /* Write 1 clear */
        DRV_WriteReg32(SEMAPHORE, (1 << flag));
        read_back = (DRV_Reg32(SEMAPHORE) >> flag) & 0x1;
        if (read_back == 0) {
            /*release sema. here*/
            ret = 1;
        } else {
            PRINTF_D("rel sem fail\n");
        }
    }

    return ret;
}

#ifdef SEMAPHORE_3WAY
int scp_get_semaphore_3way(int flag)
{
    int read_back;
    int count = 0;
    int ret = -1;

    if (flag >= SEMA_3WAY_TOTAL) {
        PRINTF_E("ERR get sem3way ID %d\n", flag);
        return ret;
    }

    if (SCP_CORE == SCP_A_ID) {
        /*SCP A bit*/
        flag *= 4;
    } else {
        /*SCP B bit*/
        flag *= 4;
        flag++;
    }

    read_back = (DRV_Reg32(SEMAPHORE_3WAY) >> flag) & 0x1;
    if (read_back == 0) {
        /* try to get sema.*/
        DRV_WriteReg32(SEMAPHORE_3WAY, (1 << flag));
        while (count != SEMA_3WAY_TIMEOUT) {
            /* repeat test if we get semaphore */
            read_back = (DRV_Reg32(SEMAPHORE_3WAY) >> flag) & 0x1;
            if (read_back == 1) {
                /*get scp sema. here*/
                ret = 1;
                break;
            }
            DRV_WriteReg32(SEMAPHORE_3WAY, (1 << flag));
            count++;
        }
        if (ret < 0)
            PRINTF_E("get sem3way timeout\n");
    } else {
        PRINTF_E("ERR sem3way was hold\n");
    }

    return ret;
}

int scp_release_semaphore_3way(int flag)
{
    int read_back;
    int ret = -1;

    if (flag >= SEMA_3WAY_TOTAL) {
        PRINTF_E("ERR get sem3way ID %d\n", flag);
        return ret;
    }

    if (SCP_CORE == SCP_A_ID) {
        /*SCP A bit*/
        flag *= 4;
    } else {
        /*SCP B bit*/
        flag *= 4;
        flag++;
    }

    read_back = (DRV_Reg32(SEMAPHORE_3WAY) >> flag) & 0x1;
    if (read_back == 0) {
        PRINTF_E("Err not hold sem3way\n");
    } else {
        /* Write 1 clear */
        DRV_WriteReg32(SEMAPHORE_3WAY, (1 << flag));
        read_back = (DRV_Reg32(SEMAPHORE_3WAY) >> flag) & 0x1;
        if (read_back == 0) {
            /*release sema. here*/
            ret = 1;
        } else {
            PRINTF_E("rel sem3way fail\n");
        }
    }

    return ret;
}
#endif

/*
 * scp_get_awake_semaphore
 * check the awake semphore status whether taken by AP
 * return 0 : scp may sleep
 *        1 : scp keep awake
 */
int scp_get_awake_semaphore(int flag)
{
    int read_back;

    flag = (flag * 2) + 1;
    read_back = (DRV_Reg32(SEMAPHORE) >> flag) & 0x1;

    return read_back;
}
