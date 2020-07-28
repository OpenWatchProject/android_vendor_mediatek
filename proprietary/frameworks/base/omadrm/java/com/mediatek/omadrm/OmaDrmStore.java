/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.omadrm;

/**
 * M: Defines constants that are used by the OMA DRM v1.0
 *
 */
public class OmaDrmStore {
    /**
     * Interface definition for the columns that represent DRM constraints.
     * {@link android.drm.DrmManagerClient#getMetadata DrmManagerClient.getMetadata()}
     * can be called by an application to find out the metadatas on the
     * {@link android.drm.DrmStore.Action actions} that can be performed
     * on right-protected content. The constants defined in this interface
     * represent three most common types of metadatas: is-drm, drm-method,
     * drm-mimetype, drm-rights-issuer and so on. Mediatek define this columns
     * to match return key-value in metadata.
     */
    public  interface MetadatasColumns {
        /**
         * This represents whether the check file is a drm file.
         * <p>
         * Type: BOOLEAN
         */
        public static final String IS_DRM = "isdrm";

        /**
         * This represents the check file drm method, in OMA DRM define, contain FL, CD, SD and
         * FLSD.
         * <p>
         * Type: INTEGER
         */
        public static final String DRM_METHOD = "drm_method";

        /**
         * This represents the original mimetype of this drm file
         * <p>
         * Type: TEXT
         */
        public static final String DRM_MIME_TYPE = "drm_mime_type";

        /**
         * This represents the rights issuer of this SD method drm file, use it user can download
         * rights from it point address.
         * <p>
         * Type: TEXT
         */
        public static final String DRM_RIGHTS_ISSUER = "drm_rights_issuer";

        public static final String DRM_ICON_URI = "drm_icon_uri";
        public static final String DRM_CONTENT_URI = "drm_content_uri";
        public static final String DRM_OFFSET = "drm_offset";
        public static final String DRM_DATALEN = "drm_dataLen";
        public static final String DRM_CONTENT_NAME = "drm_content_name";
        public static final String DRM_CONTENT_VENDOR = "drm_content_vendor";
        public static final String DRM_CONTENT_DESCRIPTION = "drm_content_description";
    }

    /**
     * Defines method type for OMA DRM v1.0
     */
    public static class Method {
        /**
         * Forward Lock
         */
        public static final int FL   = 1 << 0;
        public static final int CD   = 1 << 1;
        public static final int SD   = 1 << 2;
        public static final int FLSD = 1 << 3;
    }

    /**
     * Defines all object mime type for OMA DRM v1.0.
     */
    public static class DrmObjectMimeType {
        public static final String MIME_TYPE_RIGHTS_XML = "application/vnd.oma.drm.rights+xml";
        public static final String MIME_TYPE_RIGHTS_WBXML = "application/vnd.oma.drm.rights+wbxml";
        public static final String MIME_TYPE_DRM_CONTENT = "application/vnd.oma.drm.content";
        public static final String MIME_TYPE_DRM_MESSAGE = "application/vnd.oma.drm.message";
        //For cta
        public static final String MIME_TYPE_CTA5_MESSAGE = "application/vnd.mtk.cta5.message";
    }

    /**
     * Defines the extensions of OMA DRM v1.0 files
     */
    public static class DrmFileExtension {
        public static final String EXTENSION_RIGHTS_XML = ".dr";
        public static final String EXTENSION_RIGHTS_WBXML = ".drc";
        public static final String EXTENSION_DRM_CONTENT = ".dcf";
        public static final String EXTENSION_DRM_MESSAGE = ".dm";
    }

    /**
     * Defines the drm intent extra key & value for OMA DRM v1.0, app can use this mark started
     * module show request level oma drm files. This most use when choosing drm file as wallpaper,
     * ringtone or attachment. In OMA DRM v1.0 spec, only FL can use as wallpaper and ringtone, and
     * only SD can be forwarded.
     */
    public static class DrmIntentExtra {
        public static final String EXTRA_DRM_LEVEL = "android.intent.extra.drm_level";
        public static final int LEVEL_NONE = 0;
        public static final int LEVEL_FL   = 1;
        public static final int LEVEL_SD   = 2;
        public static final int LEVEL_ALL  = 4;
    }

  /**
     * defines media mime type prefix for OMA DRM v1.0
     */
    public static class MimePrefix {
     /**
         * Constant field signifies that image prefix
         */
        public static final String IMAGE = "image/";
        /**
         * Constant field signifies that audio prefix
         */
        public static final String AUDIO = "audio/";
        /**
         * Constant field signifies that video prefix
         */
        public static final String VIDEO = "video/";
    }
 }
