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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

#ifndef _BT_MTK_H
#define _BT_MTK_H



/**MTK BT specific operations OPCODE  */
typedef enum {
/*  [operation]
 *      Return audio configuration for BT SCO on current chipset
 *  [input param]
 *      A pointer to union type with content of BT_INFO.
 *      Typecasting conversion: (BT_INFO *) param.
 *  [return]
 *      0 - default, don't care.
 *  [callback]
 *      None.
 */
    BT_MTK_OP_AUDIO_GET_CONFIG,
  /* Audio config related information */
} bt_mtk_opcode_t;

typedef enum {
  PCM = 0,          // PCM 4 pins interface
  I2S,              // I2S interface
  MERGE_INTERFACE,  // PCM & I2S merge interface
  CVSD_REMOVAL      // SOC consys
} AUDIO_IF;

typedef enum {
  SYNC_8K = 0,
  SYNC_16K
} SYNC_CLK;        // DAIBT sample rate

typedef enum {
  SHORT_FRAME = 0,
  LONG_FRAME
} SYNC_FORMAT;     // DAIBT sync

typedef struct {
  AUDIO_IF           hw_if;
  SYNC_CLK           sample_rate;
  SYNC_FORMAT        sync_format;
  unsigned int       bit_len;  // bit-length of sync frame in long frame sync
} AUDIO_CONFIG;

/* Information carring for all OPs (In/Out) */
typedef union {
  AUDIO_CONFIG       audio_conf;
} BT_INFO;


/* For BT_AUDIO_GET_CONFIG */
struct audio_t {
  unsigned int chip_id;
  AUDIO_CONFIG audio_conf;
};

/********************************************************************************
** Function Declaration
*/


int mtk_bt_op(bt_mtk_opcode_t opcode, void *param);




#endif
