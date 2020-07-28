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

import static com.android.mms.dom.smil.SmilMediaElementImpl.SMIL_MEDIA_END_EVENT;
import static com.android.mms.dom.smil.SmilMediaElementImpl.SMIL_MEDIA_PAUSE_EVENT;
import static com.android.mms.dom.smil.SmilMediaElementImpl.SMIL_MEDIA_SEEK_EVENT;
import static com.android.mms.dom.smil.SmilMediaElementImpl.SMIL_MEDIA_START_EVENT;
import static com.android.mms.dom.smil.SmilParElementImpl.SMIL_SLIDE_END_EVENT;
import static com.android.mms.dom.smil.SmilParElementImpl.SMIL_SLIDE_START_EVENT;

import android.content.Context;
import android.drm.DrmManagerClient;
import android.text.TextUtils;
import android.util.Config;
import android.util.Log;

import com.android.mms.MmsApp;
import com.android.mms.dom.smil.SmilDocumentImpl;
import com.android.mms.dom.smil.parser.SmilXmlParser;
import com.android.mms.dom.smil.parser.SmilXmlSerializer;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.ext.IOpSmilHelperExt;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.opmsg.util.OpMessageUtils;

import org.w3c.dom.events.EventTarget;
import org.w3c.dom.smil.SMILDocument;
import org.w3c.dom.smil.SMILElement;
import org.w3c.dom.smil.SMILLayoutElement;
import org.w3c.dom.smil.SMILMediaElement;
import org.w3c.dom.smil.SMILParElement;
import org.w3c.dom.smil.SMILRegionElement;
import org.w3c.dom.smil.SMILRegionMediaElement;
import org.w3c.dom.smil.SMILRootLayoutElement;
import org.xml.sax.SAXException;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;

public class SmilHelper {
    private static final String TAG = "Mms/smil";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = DEBUG ? Config.LOGD : Config.LOGV;

    public static final String ELEMENT_TAG_TEXT = "text";
    public static final String ELEMENT_TAG_IMAGE = "img";
    public static final String ELEMENT_TAG_AUDIO = "audio";
    public static final String ELEMENT_TAG_VIDEO = "video";
    public static final String ELEMENT_TAG_REF = "ref";

    private static IOpSmilHelperExt sOpSmilHelperExt =
        OpMessageUtils.getOpMessagePlugin().getOpSmilHelperExt();

    private SmilHelper() {
        // Never instantiate this class.
    }

    /// M: Code analyze 001, new feature, add duration to par
    /// throungh MediaModel.initMediaDuration(context, uri) @{
    /// M: modified for IOT issue, which MMS with mixed type,
    /// and no SMIL part, and has audio or video
    public static SMILDocument getDocument(Context context, PduBody pb) {
    /// @}
        // Find SMIL part in the message.
        PduPart smilPart = findSmilPart(pb);
        SMILDocument document = null;

        // Try to load SMIL document from existing part.
        if (smilPart != null) {
            document = getSmilDocument(smilPart);
        }

        if (document == null) {
            // Create a new SMIL document.
            document = createSmilDocument(context, pb);
        }

        return document;
    }

    public static SMILDocument getDocument(SlideshowModel model) {
        return createSmilDocument(model);
    }

    /**
     * Find a SMIL part in the MM.
     *
     * @return The existing SMIL part or null if no SMIL part was found.
     */
    private static PduPart findSmilPart(PduBody body) {
        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            if (Arrays.equals(part.getContentType(),
                    MmsContentType.APP_SMIL.getBytes())) {
                // Sure only one SMIL part.
                return part;
            }
        }
        return null;
    }

    private static SMILDocument validate(SMILDocument in) {
        // TODO: add more validating facilities.
        return in;
    }

    /**
     * Parse SMIL message and retrieve SMILDocument.
     *
     * @return A SMILDocument or null if parsing failed.
     */
    private static SMILDocument getSmilDocument(PduPart smilPart) {
        try {
            byte[] data = smilPart.getData();
            if (data != null) {
                if (LOCAL_LOGV) {
                    Log.v(TAG, "Parsing SMIL document.");
                    Log.v(TAG, new String(data));
                }

                ByteArrayInputStream bais = new ByteArrayInputStream(data);
                SMILDocument document = new SmilXmlParser().parse(bais);
                return validate(document);
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to parse SMIL document.", e);
        } catch (SAXException e) {
            Log.e(TAG, "Failed to parse SMIL document.", e);
        } catch (MmsException e) {
            Log.e(TAG, "Failed to parse SMIL document.", e);
        }
        return null;
    }

    public static SMILParElement addPar(SMILDocument document) {
        SMILParElement par = (SMILParElement) document.createElement("par");
        // Set duration to eight seconds by default.
        // M: ALPS03633089, change default duration to 0 second
        par.setDur(0.0f);
        document.getBody().appendChild(par);
        return par;
    }

    public static SMILMediaElement createMediaElement(
            String tag, SMILDocument document, String src) {
        SMILMediaElement mediaElement =
                (SMILMediaElement) document.createElement(tag);
        mediaElement.setSrc(escapeXML(src));
        return mediaElement;
    }

    static public String escapeXML(String str) {
        /// M: fix bug ALPS00423475, fix NPE for Gmail Image Attachment
        /// without Image.Media.Data column @{
        if (str == null) {
            return new String("");
        } else {
            return str.replaceAll("&", "&amp;")
            .replaceAll("<", "&lt;")
            .replaceAll(">", "&gt;")
            .replaceAll("\"", "&quot;")
            .replaceAll("'", "&apos;");
        }
        /// @}
    }

    /// M: modified for IOT issue, which MMS with mixed type,
    /// and no SMIL part, and has audio or video
    private static SMILDocument createSmilDocument(Context context, PduBody pb) {
        if (Config.LOGV) {
            Log.v(TAG, "Creating default SMIL document.");
        }

        SMILDocument document = new SmilDocumentImpl();

        // Create root element.
        // FIXME: Should we create root element in the constructor of document?
        SMILElement smil = (SMILElement) document.createElement("smil");
        smil.setAttribute("xmlns", "http://www.w3.org/2001/SMIL20/Language");
        document.appendChild(smil);

        // Create <head> and <layout> element.
        SMILElement head = (SMILElement) document.createElement("head");
        smil.appendChild(head);

        SMILLayoutElement layout = (SMILLayoutElement) document.createElement("layout");
        head.appendChild(layout);

        // Create <body> element and add a empty <par>.
        SMILElement body = (SMILElement) document.createElement("body");
        smil.appendChild(body);
        SMILParElement par = addPar(document);

        // Create media objects for the parts in PDU.
        int partsNum = pb.getPartsNum();
        if (partsNum == 0) {
            return document;
        }

        DrmManagerClient drmManagerClient = MmsApp.getApplication().getDrmManagerClient();

        boolean hasText = false;
        boolean hasMedia = false;
        for (int i = 0; i < partsNum; i++) {
            /// M: Code analyze 002, fix bug ALPS00262862, change && to ||,
            /// to support mix type mms(no smil part and have audio or video) shown ok @{
            if ((par == null) || (hasMedia || hasText)) {
                par = addPar(document);
                hasText = false;
                hasMedia = false;
            }
            /// @}

            PduPart part = pb.getPart(i);
            String contentType = new String(part.getContentType());
            if (MmsContentType.isDrmType(contentType)) {
                contentType = drmManagerClient.getOriginalMimeType(part.getDataUri());
            }

            if (contentType.equals(MmsContentType.TEXT_PLAIN)
                    || contentType.equalsIgnoreCase(MmsContentType.APP_WAP_XHTML)
                    || contentType.equals(MmsContentType.TEXT_HTML)) {
                SMILMediaElement textElement = createMediaElement(
                        ELEMENT_TAG_TEXT, document, part.generateLocation());
                par.appendChild(textElement);
                hasText = true;
            } else if (MmsContentType.isImageType(contentType)) {
                SMILMediaElement imageElement = createMediaElement(
                        ELEMENT_TAG_IMAGE, document, part.generateLocation());
                par.appendChild(imageElement);
                hasMedia = true;
            } else if (MmsContentType.isVideoType(contentType)) {
                SMILMediaElement videoElement = createMediaElement(
                        ELEMENT_TAG_VIDEO, document, part.generateLocation());
                par.appendChild(videoElement);
                /// M: Code analyze 001, new feature, add duration to par
                /// throungh MediaModel.initMediaDuration(context, uri) @{
                MmsLog.dpi(TAG, "createSmilDocument(): video, uri = " + part.getDataUri());
                // get media play time. unit: millisecond
                int duration = MediaModel.initMediaDuration(context, part.getDataUri());
                MmsLog.d(TAG, "createSmilDocument(): video, duration = " + duration);
                if (duration != 0) {
                    // Change unit from millisecond to second.
                    par.setDur(duration / 1000);
                }
                /// @}
                hasMedia = true;
            } else if (MmsContentType.isAudioType(contentType)) {
                SMILMediaElement audioElement = createMediaElement(
                        ELEMENT_TAG_AUDIO, document, part.generateLocation());
                par.appendChild(audioElement);
                /// M: Code analyze 001, new feature, add duration to par
                /// throungh MediaModel.initMediaDuration(context, uri) @{
                MmsLog.dpi(TAG, "createSmilDocument(): audio, uri = " + part.getDataUri());
                // get media play time. unit: millisecond
                int duration = MediaModel.initMediaDuration(context, part.getDataUri());
                MmsLog.d(TAG, "createSmilDocument(): audio, duration = " + duration);
                if (duration != 0) {
                    // Change unit from millisecond to second.
                    par.setDur(duration / 1000);
                }
                /// @}
                hasMedia = true;
            } else {
                // TODO: handle other media types.
                Log.w(TAG, "unsupport media type");
            }
        }

        return document;
    }

    private static SMILDocument createSmilDocument(SlideshowModel slideshow) {
        if (Config.LOGV) {
            Log.v(TAG, "Creating SMIL document from SlideshowModel.");
        }

        /// M: Code analyze 003, new feature, check NPE @{
        if (slideshow == null) {
            MmsLog.e(TAG, "Creating SMIL document from SlideshowModel failed.");
            return null;
        }
        /// @}
        SMILDocument document = new SmilDocumentImpl();

        // Create SMIL and append it to document
        SMILElement smilElement = (SMILElement) document.createElement("smil");
        document.appendChild(smilElement);

        // Create HEAD and append it to SMIL
        SMILElement headElement = (SMILElement) document.createElement("head");
        smilElement.appendChild(headElement);

        // Create LAYOUT and append it to HEAD
        SMILLayoutElement layoutElement = (SMILLayoutElement)
                document.createElement("layout");
        headElement.appendChild(layoutElement);

        // Create ROOT-LAYOUT and append it to LAYOUT
        SMILRootLayoutElement rootLayoutElement =
                (SMILRootLayoutElement) document.createElement("root-layout");
        LayoutModel layouts = slideshow.getLayout();
        rootLayoutElement.setWidth(layouts.getLayoutWidth());
        rootLayoutElement.setHeight(layouts.getLayoutHeight());
        String bgColor = layouts.getBackgroundColor();
        if (!TextUtils.isEmpty(bgColor)) {
            rootLayoutElement.setBackgroundColor(bgColor);
        }
        layoutElement.appendChild(rootLayoutElement);

        // Create REGIONs and append them to LAYOUT
        ArrayList<RegionModel> regions = layouts.getRegions();
        ArrayList<SMILRegionElement> smilRegions = new ArrayList<SMILRegionElement>();
        for (RegionModel r : regions) {
            SMILRegionElement smilRegion = (SMILRegionElement) document.createElement("region");
            smilRegion.setId(r.getRegionId());
            smilRegion.setLeft(r.getLeft());
            smilRegion.setTop(r.getTop());
            smilRegion.setWidth(r.getWidth());
            smilRegion.setHeight(r.getHeight());
            smilRegion.setFit(r.getFit());
            smilRegions.add(smilRegion);
        }

        // Create BODY and append it to the document.
        SMILElement bodyElement = (SMILElement) document.createElement("body");
        smilElement.appendChild(bodyElement);

        boolean txtRegionPresentInLayout = false;
        boolean imgRegionPresentInLayout = false;
        /// M: change google default. @{
        for (int i = 0; i < slideshow.size(); i ++) {
            /// M: Code analyze 004, fix bug ALPS00275894, update
            /// txt and img Region status in a new slide @{
            txtRegionPresentInLayout = false;
            imgRegionPresentInLayout = false;
            /// @}
            // Create PAR element.
            SMILParElement par = addPar(document);
            SlideModel slideModel = slideshow.get(i);
            if (slideModel != null) {
                par.setDur(slideModel.getDuration() / 1000f);
                /// M: add log for bug ALPS01276998
                MmsLog.d(TAG, "createSmilDocument SlideModel duration: "
                        + slideModel.getDuration() + ", slideModel.getDuration() / 1000f: "
                        + (slideModel.getDuration() / 1000f));
            }

            addParElementEventListeners((EventTarget) par, slideshow.get(i));
            /// @}

            // Add all media elements.
            /// M: change google default arguments.
            for (MediaModel media : slideshow.get(i)) {
                SMILMediaElement sme = null;
                String src = media.getSrc();

                /// M: fix bug ALPS00967096, must have file name for every media
                if (src != null && src.lastIndexOf(".") == 0) {
                    src = System.currentTimeMillis() + src;
                    media.setSrc(src);
                }

                if (media instanceof TextModel) {
                    TextModel text = (TextModel) media;
                    if (TextUtils.isEmpty(text.getText())) {
                        if (LOCAL_LOGV) {
                            MmsLog.vpi(TAG, "Empty text part ignored: " + text.getSrc());
                        }
                        continue;
                    }
                    sme = SmilHelper.createMediaElement(SmilHelper.ELEMENT_TAG_TEXT, document, src);
                    txtRegionPresentInLayout = setRegion((SMILRegionMediaElement) sme,
                                                         smilRegions,
                                                         layoutElement,
                                                         LayoutModel.TEXT_REGION_ID,
                                                         txtRegionPresentInLayout);
                } else if (media instanceof ImageModel) {
                    sme =
                        SmilHelper.createMediaElement(SmilHelper.ELEMENT_TAG_IMAGE, document, src);
                    imgRegionPresentInLayout = setRegion((SMILRegionMediaElement) sme,
                                                         smilRegions,
                                                         layoutElement,
                                                         LayoutModel.IMAGE_REGION_ID,
                                                         imgRegionPresentInLayout);
                } else if (media instanceof VideoModel) {
                    sme =
                        SmilHelper.createMediaElement(SmilHelper.ELEMENT_TAG_VIDEO, document, src);
                    imgRegionPresentInLayout = setRegion((SMILRegionMediaElement) sme,
                                                         smilRegions,
                                                         layoutElement,
                                                         LayoutModel.IMAGE_REGION_ID,
                                                         imgRegionPresentInLayout);
                } else if (media instanceof AudioModel) {
                    sme =
                        SmilHelper.createMediaElement(SmilHelper.ELEMENT_TAG_AUDIO, document, src);
                } else {
                    MmsLog.wpi(TAG, "Unsupport media: " + media);
                    continue;
                }

                // Set timing information.
                int begin = media.getBegin();
                if (begin != 0) {
                    sme.setAttribute("begin", String.valueOf(begin / 1000));
                }
                int duration = media.getDuration();
                if (duration != 0) {
                    sme.setDur((float) duration / 1000);
                  /// M: add log for bug ALPS01276998
                    MmsLog.d(TAG, "createSmilDocument media duration: " + duration
                            + ", (float) duration / 1000: "
                            + ((float) duration / 1000));
                }
                par.appendChild(sme);

                addMediaElementEventListeners((EventTarget) sme, media);
            }
        }
        /// M: Add files in part
        if (!sOpSmilHelperExt.createSmilDocument()) {
            ArrayList<FileAttachmentModel> files = slideshow.getAttachFiles();
            for (int i = 0; i < files.size(); i ++) {
                SMILParElement par = addPar(document);
                SMILMediaElement sme = SmilHelper.createMediaElement(
                        SmilHelper.ELEMENT_TAG_REF, document, files.get(i).getSrc());
                txtRegionPresentInLayout = setRegion((SMILRegionMediaElement) sme,
                                                     smilRegions,
                                                     layoutElement,
                                                     LayoutModel.TEXT_REGION_ID,
                                                     txtRegionPresentInLayout);
                par.appendChild(sme);
            }
        }
        /// @}
        if (LOCAL_LOGV) {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            SmilXmlSerializer.serialize(document, out);
            Log.v(TAG, out.toString());
        }

        return document;
    }

    private static SMILRegionElement findRegionElementById(
            ArrayList<SMILRegionElement> smilRegions, String rId) {
        for (SMILRegionElement smilRegion : smilRegions) {
            if (smilRegion.getId().equals(rId)) {
                return smilRegion;
            }
        }
        return null;
    }

    private static boolean setRegion(SMILRegionMediaElement srme,
                                     ArrayList<SMILRegionElement> smilRegions,
                                     SMILLayoutElement smilLayout,
                                     String regionId,
                                     boolean regionPresentInLayout) {
        SMILRegionElement smilRegion = findRegionElementById(smilRegions, regionId);
        if (!regionPresentInLayout && smilRegion != null) {
            srme.setRegion(smilRegion);
            smilLayout.appendChild(smilRegion);
            return true;
        }
        return false;
    }

    static void addMediaElementEventListeners(
            EventTarget target, MediaModel media) {
        // To play the media with SmilPlayer, we should add them
        // as EventListener into an EventTarget.
        target.addEventListener(SMIL_MEDIA_START_EVENT, media, false);
        target.addEventListener(SMIL_MEDIA_END_EVENT, media, false);
        target.addEventListener(SMIL_MEDIA_PAUSE_EVENT, media, false);
        target.addEventListener(SMIL_MEDIA_SEEK_EVENT, media, false);
    }

    static void addParElementEventListeners(
            EventTarget target, SlideModel slide) {
        // To play the slide with SmilPlayer, we should add it
        // as EventListener into an EventTarget.
        target.addEventListener(SMIL_SLIDE_START_EVENT, slide, false);
        target.addEventListener(SMIL_SLIDE_END_EVENT, slide, false);
    }
}
