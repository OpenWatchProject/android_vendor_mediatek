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
/**
 * @file JsonCmd.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c JsonCmd class including the key strings used in command Json object.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __JSON_CMD_H__
#define __JSON_CMD_H__
#include "libmdmonitor.h"
namespace libmdmonitor {
/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class defined several key strings used in command Json objects.
 *
 * @see JsonCmdDecoder
 * @see JsonCmdEncoder
 */
class JsonCmd {
public:
    static const char *STR_API_KEY;
    static const char *STR_METHOD_KEY;
    static const char *STR_VERSION_KEY;
    static const char *STR_SECRET_KEY;
    static const char *STR_DOER_KEY;
    static const char *STR_ARG_KEY;
    static const char *STR_SUCCESS_KEY;
    static const char *STR_DATA_KEY;
    static const char *STR_RET_KEY;
    static const char *STR_SESSIONID_KEY;
    static const char *STR_TYPE_KEY;
    static const char *STR_MSGID_KEY;
    static const char *STR_SERVERNAME_KEY;
    static const char *STR_RAW_SIZE_KEY;
    static const char *STR_CLIENTPACKAGE_KEY;
    static const char *STR_DEBUG_LEVEL_KEY;
    static const char *STR_ASYNC_KEY;
};
} //namespace libmdmonitor {

#endif
