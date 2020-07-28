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

#ifndef DRMDEF_H_
#define DRMDEF_H_

#include <utils/String8.h>

namespace android
{

class DrmDef
{
public:
    static String8 getPermissionPath(int permissionIndex);
    static String8 getConstraintStr(int constraintType);
    static String8 getPermissionStr(int permissionIndex);
    static String8 getMetaDataStr(int metaIndex);
    static String8 getMetaTokenStr(int metaIndex);
    static int getPermissionIndex(int permission);
    static int getPermission(int permissionIndex);
    static int getPermissionFromAction(int action);

public:
    // ----------------------------------------------------------------------------
    // DRM method in OMA DRM v1
    const static int METHOD_NONE = 0;
    const static int METHOD_FL = 1;
    const static int METHOD_CD = 2;
    const static int METHOD_SD = 4;
    const static int METHOD_FLDCF = 8;

    // ----------------------------------------------------------------------------
    // DRM WBXML / XML parsing and tags
    const static String8 PATH_VERSION;
    const static String8 PATH_UID;
    const static String8 PATH_KEYVALUE;
    const static String8 PATH_PLAY;
    const static String8 PATH_DISPLAY;
    const static String8 PATH_EXECUTE;
    const static String8 PATH_PRINT;
    const static String8 PATH_CONSTRAINT_COUNT;
    const static String8 PATH_CONSTRAINT_DATETIME_START;
    const static String8 PATH_CONSTRAINT_DATETIME_END;
    const static String8 PATH_CONSTRAINT_INTERVAL;

    const static char DRM_WB_RIGHTS_TAG = 0x05;
    const static char DRM_WB_CONTEXT_TAG = 0x06;
    const static char DRM_WB_VERSION_TAG = 0x07;
    const static char DRM_WB_UID_TAG = 0x08;
    const static char DRM_WB_AGREEMENT_TAG = 0x09;
    const static char DRM_WB_ASSET_TAG = 0x0a;
    const static char DRM_WB_KEYINFO_TAG = 0x0b;
    const static char DRM_WB_KEYVALUE_TAG = 0x0c;
    const static char DRM_WB_PERMISSION_TAG = 0x0d;
    const static char DRM_WB_PLAY_TAG = 0x0e;
    const static char DRM_WB_DISPLAY_TAG = 0x0f;
    const static char DRM_WB_EXECUTE_TAG = 0x10;
    const static char DRM_WB_PRINT_TAG = 0x11;
    const static char DRM_WB_CONSTRAINT_TAG = 0x12;
    const static char DRM_WB_COUNT_TAG = 0x13;
    const static char DRM_WB_DATETIME_TAG = 0x14;
    const static char DRM_WB_START_TAG = 0x15;
    const static char DRM_WB_END_TAG = 0x16;
    const static char DRM_WB_INTERVAL_TAG = 0x17;

    const static int DRM_MAX_WBXML_SIZE = 512;
    const static int DRM_MAX_KEY_LENGTH = 16;
    const static int DRM_DCF_IV_LENGTH = 16;
    const static int DRM_DCF_BLK_LENGTH = 16;
    const static int DRM_MAX_MIME_LENGTH = 100;
    const static int DRM_MAX_META_LENGTH = 512;
    const static int DRM_MAX_ID_LENGTH = 32;

    // ----------------------------------------------------------------------------
    // DRM permission type on protected content
    const static String8 PERMISSION_PLAY_STR;
    const static String8 PERMISSION_DISPLAY_STR;
    const static String8 PERMISSION_EXECUTE_STR;
    const static String8 PERMISSION_PRINT_STR;

    const static int PERMISSION_NONE = 0x00;
    const static int PERMISSION_PLAY = 0x01;
    const static int PERMISSION_DISPLAY = 0x02;
    const static int PERMISSION_EXECUTE = 0x04;
    const static int PERMISSION_PRINT = 0x08;

    const static int PERMISSION_NONE_INDEX = -1;
    const static int PERMISSION_PLAY_INDEX = 0;
    const static int PERMISSION_DISPLAY_INDEX = 1;
    const static int PERMISSION_EXECUTE_INDEX = 2;
    const static int PERMISSION_PRINT_INDEX = 3;
    const static int PERMISSION_TOTAL_INDEX = 4;

    // ----------------------------------------------------------------------------
    // DRM constraint type on protected content
    const static String8 CONSTRAINT_COUNT_STR; // FOR LOG ONLY
    const static String8 CONSTRAINT_DATETIME_STR;
    const static String8 CONSTRAINT_DATETIME_START_STR;
    const static String8 CONSTRAINT_DATETIME_END_STR;
    const static String8 CONSTRAINT_INTERVAL_STR;
    const static String8 CONSTRAINT_NONE_STR;

    const static int CONSTRAINT_UNINIT = 0x00;
    const static int CONSTRAINT_COUNT = 0x01;
    const static int CONSTRAINT_DATETIME = 0x02;
    const static int CONSTRAINT_INTERVAL = 0x04;
    const static int CONSTRAINT_NONE = 0x08;

    // ----------------------------------------------------------------------------
    // DRM DCF content headers
    const static int META_CONTENTURI_INDEX = 0;          /* 1.0: "ContentURI" field */
    const static int META_RIGHTS_ISSUER_INDEX = 1;       /* 1.0: "Rights-Issuer" header */
    const static int META_CONTENT_NAME_INDEX = 2;        /* 1.0: "Content-Name" header */
    const static int META_CONTENT_DESCRIPTION_INDEX = 3; /* 1.0: "Content-Description" header */
    const static int META_CONTENT_VENDOR_INDEX = 4;      /* 1.0: "Content-Vendor" header */
    const static int META_ICON_URI_INDEX = 5;            /* 1.0: "Icon URI" header */

    const static String8 TOKEN_CONTENTURI;
    const static String8 TOKEN_RIGHTS_ISSUER;
    const static String8 TOKEN_CONTENT_NAME;
    const static String8 TOKEN_CONTENT_DESCRIPTION;
    const static String8 TOKEN_CONTENT_VENDOR;
    const static String8 TOKEN_ICON_URI;

    const static String8 TOKEN_CONTENT_TYPE;
    const static String8 TOKEN_CONTENT_ID;
    const static String8 TOKEN_ENCODING;

    const static String8 HEADER_TOKEN_CONTENT_TYPE;
    const static String8 HEADER_TOKEN_ID;
    const static String8 HEADER_TOKEN_ENCODING;
    const static String8 HEADER_TOKEN_DESCRIPTION;

    // ----------------------------------------------------------------------------
    // DRM engine plug-in specific settings
    const static String8 PLUGIN_DOTEXTENSION_DCF;
    const static String8 PLUGIN_MIMETYPE_DCF;
    const static String8 PLUGIN_DESCRIPTION;

    const static String8 MIME_RIGHTS_XML;
    const static String8 MIME_RIGHTS_WBXML;
    const static String8 MIME_DRM_CONTENT;
    const static String8 MIME_DRM_MESSAGE;

    const static String8 EXT_RIGHTS_XML;
    const static String8 EXT_RIGHTS_WBXML;
    const static String8 EXT_DRM_CONTENT;
    const static String8 EXT_DRM_MESSAGE;

    const static String8 CONTENT_DIRECTORY; // the directory to store content
    const static String8 RO_DIRECTORY;      // the directory to store RO files
    const static String8 ST_DIR;
    const static String8 ST_FILE;           // the secure timer file
    const static String8 IMEI_DIR;
    const static String8 IMEI_FILE;         // the file stores IMEI number

    // ----------------------------------------------------------------------------
    // installing .dm message to .dcf format
    const static String8 MTK_FL_CID;
    const static String8 MTK_CD_CID;
    const static String8 MTK_FLDCF_CID;
    const static String8 MTK_CD_;
    const static String8 MTK_FLDCF_;
    const static String8 MTK_FL_HEADER;
    const static String8 MTK_CD_HEADER;

    const static int INSTALL_OK = 1;
    const static int INSTALL_PARSE_FAILED = -1;

    // ----------------------------------------------------------------------------
    // representing the maximum end time and minimun start time (in seconds) in Rights Ojbect
    const static int MAX_END_TIME = 0x7fffffff;
    const static int MIN_START_TIME = 0x80000000;

    // ----------------------------------------------------------------------------
    // others
    const static String8 ENCODING_BASE64;

    const static int DT_SEC_PER_MIN = 60;
    const static int DT_SEC_PER_HOUR = 60 * DT_SEC_PER_MIN;
    const static int DT_SEC_PER_DAY = 24 * DT_SEC_PER_HOUR;

    //Add to support CTA5
    const static String8 CTA5_PLUGIN_DESCRIPTION;
    const static String8 MIME_CTA5_MESSAGE;
    const static String8 EXT_CTA5_FILE;
    const static String8 CTA5_DONE;
    const static String8 CTA5_UPDATING;
    const static String8 CTA5_CANCEL_DONE;
    const static String8 CTA5_CANCEL_ERROR;
    const static String8 CTA5_ERROR;
    const static String8 CTA5_MULTI_MEDIA_ENCRYPT_DONE;
    const static String8 CTA5_MULTI_MEDIA_DECRYPT_DONE;
    const static String8 CTA5_KEY_ERROR;

    const static int CTA5_CALLBACK = 10001;
};

class OmaDrmAction
{
private:
    OmaDrmAction();
public:
    static const int PRINT = 0x08;
    static const int WALLPAPER = 0x09; // for FL only

};

class OmaDrmRightsStatus
{
private:
    OmaDrmRightsStatus();
public:
    static const int SECURE_TIMER_INVALID = 0x04;

};

}

#endif /* DRMDEF_H_ */
