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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   GlobalDefs.h
 *
 * Description:
 * ------------
 *   This file is used to define macro variables
 *
 * Author:
 * -------
 *   Bo.Shang (MTK80204) 05/16/2019
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *******************************************************************************/

#pragma once

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "modemdbfilter_client"
#endif

#define UNUSED(x) (void)(x)

#define HIDL_SERVER_NAME "ModemDBFilterHidlServer"

#define DEFAULT_FILTER_FOLDER "/vendor/etc/firmware/"
#define CUSTOM_FILTER_FOLDER "/vendor/etc/firmware/customfilter/"
#define MODEM_DB_FOLDER "/vendor/etc/mddb/"

#define SYSTEM_DATA_DE_PATH "/data/system_de/"

#define SELF_DB_FILTER_FOLDER_PATH SYSTEM_DATA_DE_PATH "mdfilter/"
#define CUSTOM_FILTER_DATA_PATH SELF_DB_FILTER_FOLDER_PATH "firmware/customfilter/"
#define MODEM_DB_DATA_PATH SELF_DB_FILTER_FOLDER_PATH "mddb/"
#define DEFAULT_FILTER_DATA_PATH SELF_DB_FILTER_FOLDER_PATH "firmware/"


#define HIDL_SERVICE_RUN SELF_DB_FILTER_FOLDER_PATH "client_run"

#define FILE_OK "_ok"
#define FILTE_LOG "filter_log"
#define FILTE_LOG_OK FILTE_LOG FILE_OK
#define DEFAULT_FILTER_LOG_FILE DEFAULT_FILTER_DATA_PATH FILTE_LOG
#define DEFAULT_FILTER_LOG_FILE_OK DEFAULT_FILTER_DATA_PATH FILTE_LOG_OK

#define MDDB_LOG "MDDB_copy_log"
#define MDDB_LOG_OK MDDB_LOG FILE_OK
#define MDDB_LOG_FILE MODEM_DB_DATA_PATH MDDB_LOG
#define MDDB_LOG_FILE_OK MODEM_DB_DATA_PATH MDDB_LOG_OK

#define CUSTOM_FILTE_LOG "custom_filter_log"
#define CUSTOM_FILTE_LOG_OK CUSTOM_FILTE_LOG FILE_OK
#define CUSTOM_FILTER_LOG_FILE CUSTOM_FILTER_DATA_PATH CUSTOM_FILTE_LOG
#define CUSTOM_FILTER_LOG_FILE_OK CUSTOM_FILTER_DATA_PATH CUSTOM_FILTE_LOG_OK


#define PATH_MAX_LEN 256
#define FILE_PATH_FORMAT  "%s%s"
#define FILTER_KEY_WORD  "filter_"

#define PROP_SINGLE_BIN_FEATURE "ro.vendor.mtk_single_bin_modem_support"


