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

#define LOG_NDEBUG 1
#define LOG_TAG "DrmMtkUtil/DrmDef"
#include <utils/Log.h>

#include <drm/drm_framework_common.h>
#include <DrmDef.h>
#include <drm/DrmMtkDef.h>
#include <utils/String8.h>

using namespace android;

String8 DrmDef::getPermissionPath(int permissionIndex)
{
    if (permissionIndex == PERMISSION_PLAY_INDEX)
    {
        return PATH_PLAY;
    }
    else if (permissionIndex == PERMISSION_DISPLAY_INDEX)
    {
        return PATH_DISPLAY;
    }
    else if (permissionIndex == PERMISSION_EXECUTE_INDEX)
    {
        return PATH_EXECUTE;
    }
    else if (permissionIndex == PERMISSION_PRINT_INDEX)
    {
        return PATH_PRINT;
    }
    else
    {
        ALOGE("DrmDef::getPermissionPath : Invalid permissionIndex.");
        return String8();
    }
}

String8 DrmDef::getConstraintStr(int constraintType)
{
    if (constraintType == CONSTRAINT_COUNT)
    {
        return CONSTRAINT_COUNT_STR;
    }
    else if (constraintType == CONSTRAINT_DATETIME)
    {
        return CONSTRAINT_DATETIME_STR;
    }
    else if (constraintType == CONSTRAINT_INTERVAL)
    {
        return CONSTRAINT_INTERVAL_STR;
    }
    else if (constraintType == CONSTRAINT_NONE)
    {
        return CONSTRAINT_NONE_STR;
    }
    else
    {
        ALOGE("DrmDef::getConstraintStr : Invalid constraintType.");
        return String8();
    }
}

String8 DrmDef::getPermissionStr(int permissionIndex)
{
    if (permissionIndex == PERMISSION_PLAY_INDEX)
    {
        return PERMISSION_PLAY_STR;
    }
    else if (permissionIndex == PERMISSION_DISPLAY_INDEX)
    {
        return PERMISSION_DISPLAY_STR;
    }
    else if (permissionIndex == PERMISSION_EXECUTE_INDEX)
    {
        return PERMISSION_EXECUTE_STR;
    }
    else if (permissionIndex == PERMISSION_PRINT_INDEX)
    {
        return PERMISSION_PRINT_STR;
    }
    else
    {
        ALOGE("DrmDef::getPermissionStr : Invalid permissionIndex.");
        return String8();
    }
}

String8 DrmDef::getMetaDataStr(int metaIndex)
{
    if (metaIndex == META_CONTENTURI_INDEX)
    {
        return String8(DrmMetaKey::META_KEY_CONTENT_URI);
    }
    else if (metaIndex == META_RIGHTS_ISSUER_INDEX)
    {
        return String8(DrmMetaKey::META_KEY_RIGHTS_ISSUER);
    }
    else if (metaIndex == META_CONTENT_NAME_INDEX)
    {
        return String8(DrmMetaKey::META_KEY_CONTENT_NAME);
    }
    else if (metaIndex == META_CONTENT_DESCRIPTION_INDEX)
    {
        return String8(DrmMetaKey::META_KEY_CONTENT_DESCRIPTION);
    }
    else if (metaIndex == META_CONTENT_VENDOR_INDEX)
    {
        return String8(DrmMetaKey::META_KEY_CONTENT_VENDOR);
    }
    else if (metaIndex == META_ICON_URI_INDEX)
    {
        return String8(DrmMetaKey::META_KEY_ICON_URI);
    }
    else
    {
        ALOGE("DrmDef::getMetaDataStr : Invalid metaIndex.");
        return String8();
    }
}

String8 DrmDef::getMetaTokenStr(int metaIndex)
{
    if (metaIndex == META_CONTENTURI_INDEX)
    {
        return TOKEN_CONTENTURI;
    }
    else if (metaIndex == META_RIGHTS_ISSUER_INDEX)
    {
        return TOKEN_RIGHTS_ISSUER;
    }
    else if (metaIndex == META_CONTENT_NAME_INDEX)
    {
        return TOKEN_CONTENT_NAME;
    }
    else if (metaIndex == META_CONTENT_DESCRIPTION_INDEX)
    {
        return TOKEN_CONTENT_DESCRIPTION;
    }
    else if (metaIndex == META_CONTENT_VENDOR_INDEX)
    {
        return TOKEN_CONTENT_VENDOR;
    }
    else if (metaIndex == META_ICON_URI_INDEX)
    {
        return TOKEN_ICON_URI;
    }
    else
    {
        ALOGE("DrmDef::getMetaTokenStr : Invalid metaIndex.");
        return String8();
    }
}

int DrmDef::getPermissionIndex(int permission)
{
    if (permission == PERMISSION_PLAY)
    {
        return PERMISSION_PLAY_INDEX;
    }
    else if (permission == PERMISSION_DISPLAY)
    {
        return PERMISSION_DISPLAY_INDEX ;
    }
    else if (permission == PERMISSION_EXECUTE)
    {
        return PERMISSION_EXECUTE_INDEX ;
    }
    else if (permission == PERMISSION_PRINT)
    {
        return PERMISSION_PRINT_INDEX ;
    }
    else
    {
        ALOGE("DrmDef::getPermissionIndex : Invalid permission.");
        return PERMISSION_NONE_INDEX;
    }
}


int DrmDef::getPermission(int permissionIndex)
{
    if (permissionIndex == PERMISSION_PLAY_INDEX)
    {
        return PERMISSION_PLAY;
    }
    else if (permissionIndex == PERMISSION_DISPLAY_INDEX)
    {
        return PERMISSION_DISPLAY;
    }
    else if (permissionIndex == PERMISSION_EXECUTE_INDEX)
    {
        return PERMISSION_EXECUTE;
    }
    else if (permissionIndex == PERMISSION_PRINT_INDEX)
    {
        return PERMISSION_PRINT;
    }
    else
    {
        ALOGE("DrmDef::getPermission : Invalid permissionIndex.");
        return PERMISSION_NONE;
    }
}

int DrmDef::getPermissionFromAction(int action)
{
    int permission = PERMISSION_NONE;
    switch (action)
    {
        case Action::PLAY:
            permission = PERMISSION_PLAY;
            break;
        case Action::DISPLAY:
            permission = PERMISSION_DISPLAY;
            break;
        case Action::EXECUTE:
            permission = PERMISSION_EXECUTE;
            break;
        case OmaDrmAction::PRINT:
            permission = PERMISSION_PRINT;
            break;
        default:
            break;
    }
    return permission;
}

const String8 DrmDef::PATH_VERSION("/rights/context/version");
const String8 DrmDef::PATH_UID("/rights/agreement/asset/context/uid");
const String8 DrmDef::PATH_KEYVALUE("/rights/agreement/asset/KeyInfo/KeyValue");
const String8 DrmDef::PATH_PLAY("/rights/agreement/permission/play");
const String8 DrmDef::PATH_DISPLAY("/rights/agreement/permission/display");
const String8 DrmDef::PATH_EXECUTE("/rights/agreement/permission/execute");
const String8 DrmDef::PATH_PRINT("/rights/agreement/permission/print");
const String8 DrmDef::PATH_CONSTRAINT_COUNT("/constraint/count");
const String8 DrmDef::PATH_CONSTRAINT_DATETIME_START("/constraint/datetime/start");
const String8 DrmDef::PATH_CONSTRAINT_DATETIME_END("/constraint/datetime/end");
const String8 DrmDef::PATH_CONSTRAINT_INTERVAL("/constraint/interval");

const String8 DrmDef::PERMISSION_PLAY_STR("play");
const String8 DrmDef::PERMISSION_DISPLAY_STR("display");
const String8 DrmDef::PERMISSION_EXECUTE_STR("execute");
const String8 DrmDef::PERMISSION_PRINT_STR("print");

const String8 DrmDef::CONSTRAINT_COUNT_STR("count");
const String8 DrmDef::CONSTRAINT_DATETIME_STR("datetime");
const String8 DrmDef::CONSTRAINT_DATETIME_START_STR("start");
const String8 DrmDef::CONSTRAINT_DATETIME_END_STR("end");
const String8 DrmDef::CONSTRAINT_INTERVAL_STR("interval");
const String8 DrmDef::CONSTRAINT_NONE_STR("none");

const String8 DrmDef::TOKEN_CONTENTURI("cid:");
const String8 DrmDef::TOKEN_RIGHTS_ISSUER("Rights-Issuer:");
const String8 DrmDef::TOKEN_CONTENT_NAME("Content-Name:");
const String8 DrmDef::TOKEN_CONTENT_DESCRIPTION("Content-Description:");
const String8 DrmDef::TOKEN_CONTENT_VENDOR("Content-Vendor:");
const String8 DrmDef::TOKEN_ICON_URI("Icon-URI:");

const String8 DrmDef::TOKEN_CONTENT_TYPE("Content-Type:");
const String8 DrmDef::TOKEN_CONTENT_ID("Content-ID:");
const String8 DrmDef::TOKEN_ENCODING("Content-Transfer-Encoding:");

const String8 DrmDef::HEADER_TOKEN_CONTENT_TYPE("Content-Type");
const String8 DrmDef::HEADER_TOKEN_ID("Content-ID");
const String8 DrmDef::HEADER_TOKEN_ENCODING("Content-Transfer-Encoding");
const String8 DrmDef::HEADER_TOKEN_DESCRIPTION("Content-Description");

const String8 DrmDef::PLUGIN_DESCRIPTION("OMA V1 dcf");

const String8 DrmDef::MIME_RIGHTS_XML("application/vnd.oma.drm.rights+xml");
const String8 DrmDef::MIME_RIGHTS_WBXML("application/vnd.oma.drm.rights+wbxml");
const String8 DrmDef::MIME_DRM_CONTENT("application/vnd.oma.drm.content");
const String8 DrmDef::MIME_DRM_MESSAGE("application/vnd.oma.drm.message");

const String8 DrmDef::EXT_RIGHTS_XML(".dr");
const String8 DrmDef::EXT_RIGHTS_WBXML(".drc");
const String8 DrmDef::EXT_DRM_CONTENT(".dcf");
const String8 DrmDef::EXT_DRM_MESSAGE(".dm");

const String8 DrmDef::CONTENT_DIRECTORY("/sdcard/download/");
const String8 DrmDef::RO_DIRECTORY("/data/drm/ro/");
const String8 DrmDef::ST_DIR("/data/data/com.android.providers.drm/");
const String8 DrmDef::ST_FILE("/data/data/com.android.providers.drm/st.dat");
const String8 DrmDef::IMEI_DIR("/data/drm/imei/");
const String8 DrmDef::IMEI_FILE("/data/drm/imei/imei.dat");

const String8 DrmDef::MTK_FL_CID("MTK_FL_cid:");
const String8 DrmDef::MTK_CD_CID("MTK_CD_cid:");
const String8 DrmDef::MTK_FLDCF_CID("MTK_FLDCF_cid:");
const String8 DrmDef::MTK_CD_("MTK_CD_");
const String8 DrmDef::MTK_FLDCF_("MTK_FLDCF_");
const String8 DrmDef::MTK_FL_HEADER(
        "Encryption-Method:AES128CBC;padding=RFC2630;plaintextlen=%010d\r\nRight-Issuer:[FL]\r\n");
const String8 DrmDef::MTK_CD_HEADER(
        "Encryption-Method:AES128CBC;padding=RFC2630\r\nRight-Issuer:[CD]\r\n");

const String8 DrmDef::ENCODING_BASE64("base64");

//Add to support cta5
const String8 DrmDef::CTA5_PLUGIN_DESCRIPTION("CTA5 DRM plug-in");
const String8 DrmDef::MIME_CTA5_MESSAGE("application/vnd.mtk.cta5.message");
const String8 DrmDef::EXT_CTA5_FILE(".mudp");
const String8 DrmDef::CTA5_DONE("cta5_done");
const String8 DrmDef::CTA5_UPDATING("cta5_updating");
const String8 DrmDef::CTA5_CANCEL_DONE("cta5_cancel_done");
const String8 DrmDef::CTA5_CANCEL_ERROR("cta5_cancel_error");
const String8 DrmDef::CTA5_ERROR("cta5_error");
const String8 DrmDef::CTA5_MULTI_MEDIA_ENCRYPT_DONE("cta5_multimedia_encrypt_done");
const String8 DrmDef::CTA5_MULTI_MEDIA_DECRYPT_DONE("cta5_multimedia_decrypt_done");
const String8 DrmDef::CTA5_KEY_ERROR("cta5_error_key");
