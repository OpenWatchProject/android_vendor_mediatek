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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkUtil/DrmMessageParser"
#include <utils/Log.h>

#include <DrmMessageParser.h>
#include <DrmDef.h>
#include <StrUtil.h>

#include <utils/String8.h>

#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LOG 0

using namespace android;

String8 parse(const char* inputLine, const String8& token);

// note that the media type, type, and sub type are all CASE-SENSITIVE!
ContentType::ContentType(const char* inputLine) :
    type(""), subType(""), mediaType("") {

    if (NULL == inputLine || 0 >= strlen(inputLine)) {
        ALOGE("ContentType: invalid input for Content-Type header");
        return;
    }

    String8 input(inputLine);
    // find token "Content-Type:" (note: we ignore upper case & lower case of token.)
    input.toLower();
    String8 token = DrmDef::HEADER_TOKEN_CONTENT_TYPE;
    token.toLower();
    ALOGV("ContentType: input string [%s], token [%s]", input.string(), token.string());

    ssize_t index = input.find(token);
    ssize_t pos = index + token.length();
    if (index < 0) {
        ALOGD("invalid Content-Type header format with string [%s]", inputLine);
        return;
    }
    ALOGV("ContentType: token index [%d]", index);

    index = input.find(":"); // we assume that ':' can always be found.
    pos = index + 1;

    input.setTo(inputLine); // resume upper case & lower case.

    input.setTo(&input.string()[pos], input.length() - (size_t)pos);
    ALOGV("ContentType: token value position [%d], token value [%s]", pos, input.string());

    // find ';' which is the beginning of "parameter"
    // and we currently just ignore parameters.
    String8 result(input);
    ssize_t index_para = input.find(";");
    if (index_para > 0) { // have some parameters
        result.setTo(result.string(), (size_t)index_para);
    }
    result = StrUtil::trimLRSpace(result);
    ALOGD("ContentType: parsed media type: [%s]", result.string());

    // find '/' which is the separater of type and subtype
    ssize_t index_sprt = result.find("/");
    if (index_sprt < 0) {
        ALOGE("ContentType: invalid media type format with string [%s]", result.string());
        return;
    }
    mediaType = result;

    // the "type" and "sub type", e.g. type=="image" subType=="jpeg"
    type.setTo(result.string(), (size_t)index_sprt);
    subType.setTo(&result.string()[index_sprt + 1], result.length() - (size_t)index_sprt);
    ALOGD("ContentType: parsed type: [%s], sub type: [%s]", type.string(), subType.string());
}

bool ContentType::IsValid() {
    return !mediaType.isEmpty() && !type.isEmpty() && !subType.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////
// the Content-Transfer-Encoding is NOT CASE-SENSITIVE
// so we change it to lower-case finally
ContentTransferEncoding::ContentTransferEncoding(const char* inputLine) :
    mechanism("") {
    mechanism = parse(inputLine, DrmDef::HEADER_TOKEN_ENCODING);
    mechanism.toLower();
}

bool ContentTransferEncoding::IsValid() {
    return !mechanism.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////
ContentId::ContentId(const char* inputLine) :
    id("") {
    id = parse(inputLine, DrmDef::HEADER_TOKEN_ID);
}

bool ContentId::IsValid() {
    return !id.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////
ContentDescription::ContentDescription(const char* inputLine) :
    description("") {
    description = parse(inputLine, DrmDef::HEADER_TOKEN_DESCRIPTION);
}

bool ContentDescription::IsValid() {
    return !description.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////
String8 parse(const char* inputLine, const String8& token) {

    if (NULL == inputLine || 0 >= strlen(inputLine)) {
        ALOGE("parse: invalid input for MIME header [%s]", token.string());
        return String8("");
    }

    String8 input(inputLine);
    if (DEBUG_LOG) ALOGD("parse: >>> input string [%s]", input.string());
    input.toLower();
    String8 tk = token;
    tk.toLower();
    if (DEBUG_LOG) ALOGD("parse: >>> token [%s]", tk.string());

    // find token
    ssize_t index = input.find(tk);
    if (index < 0) {
        ALOGE("parse: failed to find token, invalid MIME header format with string [%s]",
                inputLine);
        return String8("");
    }
    if (DEBUG_LOG) ALOGD("parse: >>> token index [%d]", index);

    index = input.find(":"); // we assume that ':' can always be found.
    ssize_t pos = index + 1;

    input.setTo(inputLine); // resume upper case & lower case.

    input.setTo(&input.string()[pos], input.length() - (size_t)pos);
    if (DEBUG_LOG) ALOGD("parse: >>> token value position [%d], token value [%s]", pos, input.string());

    String8 result(input);
    result = StrUtil::trimLRSpace(result);
    ALOGD("parse: >>> parsed value: [%s]", result.string());

    return result;
}

