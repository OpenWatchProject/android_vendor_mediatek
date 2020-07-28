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

#ifndef _DVFS_DEFINE_
#define _DVFS_DEFINE_

enum dvfsrc_req_value {
    DVFSRC_REQ_VALUE1 = 0x01,
    DVFSRC_REQ_VALUE2 = 0x02,
    DVFSRC_REQ_VALUE3 = 0x04,
    DVFSRC_REQ_VALUE4 = 0x08,
    DVFSRC_REQ_VALUE5 = 0x10,
    DVFSRC_REQ_VALUE6 = 0x20,
    DVFSRC_REQ_VALUE7 = 0x40,
    DVFSRC_REQ_VALUE8 = 0x80,
};

enum dvfsrc_req_idx {
    DVFSRC_REQ_IDX1 = 0,
    DVFSRC_REQ_IDX2,
    DVFSRC_REQ_IDX3,
    DVFSRC_REQ_IDX4,
    DVFSRC_REQ_IDX5,
    DVFSRC_REQ_IDX6,
    DVFSRC_REQ_IDX7,
    DVFSRC_REQ_IDX8,
    DVFSRC_REQ_NUM,
    DVFSRC_REQ_UNINIT = 0xFFFF,
};

enum vcore_dvfs_level {
    DVFS_LEVEL_0 = 0,
    DVFS_LEVEL_1,    /* reserved */
    DVFS_LEVEL_2,    /* reserved */
    DVFS_LEVEL_3,    /* reserved */
    DVFS_LEVEL_4,
    DVFS_LEVEL_5,
    DVFS_LEVEL_6,    /* reserved */
    DVFS_LEVEL_7,
    DVFS_LEVEL_8,
    DVFS_LEVEL_9,
    DVFS_LEVEL_10,
    DVFS_LEVEL_11,
    DVFS_LEVEL_12,
    DVFS_LEVEL_NUM,
    DVFS_LEVEL_UNINIT = 0xFFFF,
};

enum vcore_dvfs_opp {
    DVFS_OPP_0 = 0,
    DVFS_OPP_1,    /* reserved */
    DVFS_OPP_2,    /* reserved */
    DVFS_OPP_3,    /* reserved */
    DVFS_OPP_4,
    DVFS_OPP_5,
    DVFS_OPP_6,    /* reserved */
    DVFS_OPP_7,
    DVFS_OPP_8,
    DVFS_OPP_9,
    DVFS_OPP_10,
    DVFS_OPP_11,
    DVFS_OPP_12,
    DVFS_OPP_NUM,
    DVFS_OPP_UNINIT = 0xFFFF,
};

enum ddr_rate {
    DDR_RATE_800  = 800,
    DDR_RATE_1200 = 1200,
    DDR_RATE_1534 = 1534,   //1600
    DDR_RATE_2400 = 2400,
    DDR_RATE_3094 = 3094,   //3200
    DDR_RATE_3733 = 3733,
};

enum vcore_vol {
    VCORE_0625 = 0,
    VCORE_0700,
    VCORE_0800,
    VCORE_NUM,
};

struct vcore_dvfs_opp_tb {
    uint32_t level;            // level = opp
    uint32_t ddr_rate;
    uint32_t vcore;
    uint32_t cpu_freq;
};

struct vcore_threshold_tb {
    uint32_t max_freq;
    uint32_t upper_bound;
    uint32_t lower_bound;
};

#define ADSP_DVFSRC_UNIT(ulevel, uddr_rate, uvcore, freq) \
    {.level = ulevel, .ddr_rate = uddr_rate, .vcore = uvcore, .cpu_freq = freq}
#define VCORE_THR(freq, upper, lower) \
    {.max_freq = freq, .upper_bound = upper, .lower_bound = lower}

#endif /* _DVFS_DEFINE_ */
