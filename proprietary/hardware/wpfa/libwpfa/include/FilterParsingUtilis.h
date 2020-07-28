/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * APlicable license agreements with MediaTek Inc.
 */

#ifndef FILTERPARSINGUTILS_H
#define FILTERPARSINGUTILS_H

#define IPTALBE_NEW_RULE    0
#define IPTALBE_DELETE_RULE 1

#define FP_LOG_TAG "WpfaParsing"

#include "FilterStructure.h"
#include "RuleRegisterUtilis.h"

int checkFilterConfig(wifiProxy_filter_config_e filter_config, wifiProxy_filter_config_e config);
void dumpFilter(WPFA_filter_reg_t filter);

int executeFilterReg(wifiproxy_m2a_reg_dl_filter_t m2a_reg_dl_filter);
int executeFilterDeReg(uint32_t fid);

filter_md_ap_ver_enum getUsingFilterVersion();
void setUsingFilterVersion(filter_md_ap_ver_enum ver);

//For testing, will remove
void testingFilter();

void initTestingFilterStructue(wifiProxy_filter_reg_t *filter);
void initTestingFilterStructue1(wifiProxy_filter_reg_t *filter);
void initTestingFilterStructue2(wifiProxy_filter_reg_t *filter);
void initTestingFilterStructue3(wifiProxy_filter_reg_t *filter);
void initTestingFilterStructue7(wifiProxy_filter_reg_t *filter);
void initTestingFilterStructue8(wifiProxy_filter_reg_t *filter);

#endif // FILTERPARSINGUTILS_H

