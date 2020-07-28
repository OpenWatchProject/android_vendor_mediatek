/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mms.model;

import android.content.Context;
import android.util.Log;

import com.android.mms.LogTag;
import com.android.mms.MmsConfig;
import com.android.mms.UnsupportContentTypeException;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;

import org.w3c.dom.smil.SMILMediaElement;
import org.w3c.dom.smil.SMILRegionElement;
import org.w3c.dom.smil.SMILRegionMediaElement;
import org.w3c.dom.smil.Time;
import org.w3c.dom.smil.TimeList;

import java.io.IOException;

public class MediaModelFactory {
    private static final String TAG = "Mms:media";

    public static MediaModel getMediaModel(Context context,
            SMILMediaElement sme, LayoutModel layouts, PduBody pb)
            throws IOException, IllegalArgumentException, MmsException {
        String tag = sme.getTagName();
        String src = sme.getSrc();
        PduPart part = findPart(pb, src);

        if (sme instanceof SMILRegionMediaElement) {
            return getRegionMediaModel(
                    context, tag, src, (SMILRegionMediaElement) sme, layouts, part);
        } else {
            return getGenericMediaModel(
                    context, tag, src, sme, part, null);
        }
    }

    private static PduPart findPart(PduBody pb, String src) {
        PduPart part = null;

        MmsLog.d(TAG, "findPart() begin: src = " + src);
        if (src != null) {
            src = unescapeXML(src);
            if (src.startsWith("cid:")) {
                part = pb.getPartByContentId("<" + src.substring("cid:".length()) + ">");
                /// M: Code analyze 002, fix bug ALPS00305765,
                /// resolve the parse error problem when CID contians <> @{
                if (part == null) {
                    part = pb.getPartByContentId(src.substring("cid:".length()));
                }
                /// @}
            }
            /// M: Code analyze 003, fix bug ALPS00267318,
            /// mms slideshow layout invalid(unkown)(suspend) @{
            if (part == null) {
                MmsLog.i(TAG, "findPart(): src1 = " + src);
                part = pb.getPartByContentId("<" + src + ">");
            }
            if (part == null) {
                part = pb.getPartByName(src);
            }
            /// M: fix bug ALPS00419439, temporary solution, parser ContentLocation fristly @{
            if (part == null) {
                part = pb.getPartByContentLocation(src);
            }
            if (part == null) {
                part = pb.getPartByFileName(src);
            }
            /// @}
            if (part != null) {
                return part;
            }
            int lastDocCharAt = src.lastIndexOf(".");
            if (lastDocCharAt > 0) {
                if (part == null) {
                    MmsLog.i(TAG, "findPart(): src2 = " + src.substring(0, lastDocCharAt));
                    part = pb.getPartByContentLocation(src.substring(0, lastDocCharAt));
                }
                if (part == null) {
                    part = pb.getPartByFileName(src.substring(0, lastDocCharAt));
                }
                if (part == null) {
                    part = pb.getPartByName(src.substring(0, lastDocCharAt));
                }
                if (part == null) {
                    part = pb.getPartByContentId("<" + src.substring(0, lastDocCharAt) + ">");
                }
                if (part != null) {
                    return part;
                }
            }
        }
        throw new IllegalArgumentException("No part found for the model. src = " + src);
        /// @}
    }

    private static String unescapeXML(String str) {
        return str.replaceAll("&lt;", "<")
            .replaceAll("&gt;", ">")
            .replaceAll("&quot;", "\"")
            .replaceAll("&apos;", "'")
            .replaceAll("&amp;", "&");
    }

    private static MediaModel getRegionMediaModel(Context context,
            String tag, String src, SMILRegionMediaElement srme,
            LayoutModel layouts, PduPart part) throws IOException, MmsException {
        if (tag.equals(SmilHelper.ELEMENT_TAG_REF)) {
            throw new IllegalArgumentException("ref tag");
        }
        SMILRegionElement sre = srme.getRegion();
        if (sre != null) {
            RegionModel region = layouts.findRegionById(sre.getId());
            if (region != null) {
                return getGenericMediaModel(context, tag, src, srme, part, region);
            }
        } else {
            String rId = null;

            if (tag.equals(SmilHelper.ELEMENT_TAG_TEXT)) {
                rId = LayoutModel.TEXT_REGION_ID;
            } else {
                rId = LayoutModel.IMAGE_REGION_ID;
            }

            RegionModel region = layouts.findRegionById(rId);
            if (region != null) {
                return getGenericMediaModel(context, tag, src, srme, part, region);
            }
        }

        throw new IllegalArgumentException("Region not found or bad region ID.");
    }

    // When we encounter a content type we can't handle, such as "application/vnd.smaf", instead
    // of throwing an exception and crashing, insert an empty TextModel in its place.
    private static MediaModel createEmptyTextModel(Context context,  RegionModel regionModel)
            throws IOException {
        return new TextModel(context, MmsContentType.TEXT_PLAIN, null, regionModel);
    }

    private static MediaModel getGenericMediaModel(Context context,
            String tag, String src, SMILMediaElement sme, PduPart part,
            RegionModel regionModel) throws IOException, MmsException {
        byte[] bytes = part.getContentType();
        if (bytes == null) {
            throw new IllegalArgumentException(
                    "Content-Type of the part may not be null.");
        }

        String contentType = new String(bytes);
        MediaModel media = null;
        if (tag.equals(SmilHelper.ELEMENT_TAG_TEXT)) {
            media = new TextModel(context, contentType, src,
                    part.getCharset(), part.getData(), regionModel);
        } else if (tag.equals(SmilHelper.ELEMENT_TAG_IMAGE)) {
            /// M: @{
            MmsLog.d(TAG, "mediamodelFactory. contenttype=" + contentType);
            if (contentType.equals("application/oct-stream")
                    || contentType.equals("application/octet-stream")) {
                if (src != null) {
                    MmsLog.d(TAG, "file name=" + src);
                    /// M: Code analyze 004, fix bug ALPS00114682,
                    /// Handle file with no postfix case @{
                    String suffix = src.contains(".") ?
                            src.substring(src.lastIndexOf("."), src.length()) : "";
                    if (suffix.equals("")) {
                        MmsLog.e(TAG,
                                "can not parse content-type from src, cause of no file type.");
                    } else if (suffix.equals(".bmp")) {
                        contentType = MmsContentType.IMAGE_BMP;
                    } else if (suffix.equals(".jpg")) {
                        contentType = MmsContentType.IMAGE_JPG;
                    } else if (suffix.equals(".wbmp")) {
                        contentType = MmsContentType.IMAGE_WBMP;
                    } else if (suffix.equals(".gif")) {
                        contentType = MmsContentType.IMAGE_GIF;
                    } else if (suffix.equals(".png")) {
                        contentType = MmsContentType.IMAGE_PNG;
                    } else if (suffix.equals(".jpeg")) {
                        contentType = MmsContentType.IMAGE_JPEG;
                    } else {
                        MmsLog.e(TAG, "can not parse content-type from src. src: " + src);
                    }
                } else {
                    throw new UnsupportContentTypeException("Unsupported Content-Type: "
                            + contentType + "; src is null");
                }
            }
            MmsLog.d(TAG, "Done! contentType=" + contentType);
            /// @}

            /// M: Code analyze 005, fix bug ALPS00248812,
            /// a img tag reference a text, new a TextModel @{
            if (contentType.equalsIgnoreCase("text/plain")) {
                media = new TextModel(context, contentType, src,
                        part.getCharset(), part.getData(), regionModel);
            }
            else {
                media = new ImageModel(context, contentType, src,
                        part.getDataUri(), regionModel);
            }
            /// @}
        } else if (tag.equals(SmilHelper.ELEMENT_TAG_VIDEO)) {
            media = new VideoModel(context, contentType, src,
                    part.getDataUri(), regionModel);
        } else if (tag.equals(SmilHelper.ELEMENT_TAG_AUDIO)) {
            media = new AudioModel(context, contentType, src,
                    part.getDataUri());
        } else if (tag.equals(SmilHelper.ELEMENT_TAG_REF)) {
            if (MmsContentType.isTextType(contentType)) {
                media = new TextModel(context, contentType, src,
                        part.getCharset(), part.getData(), regionModel);
            } else if (MmsContentType.isImageType(contentType)) {
                media = new ImageModel(context, contentType, src,
                        part.getDataUri(), regionModel);
            } else if (MmsContentType.isVideoType(contentType)) {
                media = new VideoModel(context, contentType, src,
                        part.getDataUri(), regionModel);
            } else if (MmsContentType.isAudioType(contentType)) {
                media = new AudioModel(context, contentType, src,
                        part.getDataUri());
            } else {
                Log.d(TAG, "[MediaModelFactory] getGenericMediaModel Unsupported Content-Type: "
                        + contentType);
                media = createEmptyTextModel(context, regionModel);
            }
        } else {
            throw new IllegalArgumentException("Unsupported TAG: " + tag);
        }

        // Set 'begin' property.
        int begin = 0;
        TimeList tl = sme.getBegin();
        if ((tl != null) && (tl.getLength() > 0)) {
            // We only support a single begin value.
            Time t = tl.item(0);
            begin = (int) (t.getResolvedOffset() * 1000);
        }
        media.setBegin(begin);

        // Set 'duration' property.
        int duration = (int) (sme.getDur() * 1000);
        if (duration <= 0) {
            tl = sme.getEnd();
            if ((tl != null) && (tl.getLength() > 0)) {
                // We only support a single end value.
                Time t = tl.item(0);
                if (t.getTimeType() != Time.SMIL_TIME_INDEFINITE) {
                    duration = (int) (t.getResolvedOffset() * 1000) - begin;

                    if (duration == 0 &&
                            (media instanceof AudioModel || media instanceof VideoModel)) {
                        duration = MmsConfig.getMinimumSlideElementDuration();
                        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                            Log.d(TAG, "[MediaModelFactory] compute new duration for " + tag +
                                    ", duration=" + duration);
                        }
                    }
                }
            }
        }

        media.setDuration(duration);

        if (!MmsConfig.getSlideDurationEnabled()) {
            /**
             * Because The slide duration is not supported by mmsc,
             * the device has to set fill type as FILL_FREEZE.
             * If not, the media will disappear while rotating the screen
             * in the slide show play view.
             */
            media.setFill(sme.FILL_FREEZE);
        } else {
            // Set 'fill' property.
            media.setFill(sme.getFill());
        }
        return media;
    }
}
