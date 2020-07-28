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
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims.rcsua;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.*;

/**
 * Capability used for SIP Register.
 */
public class Capability implements Parcelable {

    public Capability() {
        initialize(null);
    }

    public Capability(String features) {
        initialize(features);
    }

    public Capability(long features) {
        initialize(features);
    }

    public Capability add(Capability features) {
        addFeature(features);
        return this;
    }

    public Capability remove(Capability features) {
        removeFeature(features);
        return this;
    }

    public Capability add(String features) {
        addFeature(new Capability(features));
        return this;
    }

    public Capability remove(String features) {
        removeFeature(new Capability(features));
        return this;
    }

    public Capability update(String features) {
        initialize(features);
        return this;
    }

    public TagValueList get(String tagName) {
        return features.get(tagName);
    }

    public boolean contains(String feature) {
        Capability capability = new Capability(feature);

        for (Map.Entry<String, TagValueList> entry : capability.features.entrySet()) {
            if (!features.containsKey(entry.getKey()))
                return false;
            TagValueList value = features.get(entry.getKey());
            if (!value.contains(entry.getValue()))
                return false;
        }

        return true;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.toString());
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<Capability> CREATOR = new Creator<Capability>() {
        @Override
        public Capability createFromParcel(Parcel in) {
            return new Capability(in.readString());
        }

        @Override
        public Capability[] newArray(int size) {
            return new Capability[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();

        if (features.size() > 0) {
            for (Map.Entry<String, TagValueList> entry : features.entrySet()) {
                if (builder.length() > 0) {
                    builder.append(';');
                }

                builder.append(entry.getKey());

                if (entry.getValue().size() > 0) {
                    String value = entry.getValue().toString();
                    builder.append('=');
                    builder.append(value);
                }
            }
        }

        return builder.toString();
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;

        if (obj instanceof Capability) {
            Capability other = (Capability)obj;
            return other.features.hashCode() == features.hashCode();
        }

        return false;
    }

    @Override
    public int hashCode() {
        return features.hashCode();
    }

    public long toNumeric() {
        long result = 0;

        if (features.size() > 0) {
            for (Map.Entry<String, TagValueList> entry : features.entrySet()) {
                String key = entry.getKey();
                TagValueList tags = entry.getValue();

                switch (key) {
                    case SVC_3GPP_ICSI:
                        for (String value : tags.values) {
                            if (ICSI_CPM_MSG.equalsIgnoreCase(value)) {
                                result |= 0x04;
                            } else
                            if (ICSI_CPM_SESSION.equalsIgnoreCase(value)) {
                                result |= 0x01;
                            } else
                            if (ICSI_CPM_LARGE_MSG.equalsIgnoreCase(value)) {
                                result |= 0x08;
                            } else
                            if (ICSI_CPM_FILETRANSFER.equalsIgnoreCase(value)) {
                                result |= 0x02;
                            }
                        }
                        break;

                    case SVC_3GPP_IARI:
                        for (String value : tags.values) {
                            if (IARI_GEO_PUSH.equalsIgnoreCase(value)) {
                                result |= 0x10;
                            } else
                            if (IARI_GEO_PULL.equalsIgnoreCase(value)) {
                                result |= 0x20;
                            } else
                            if (IARI_GEO_PULLFT.equalsIgnoreCase(value)) {
                                result |= 0x40;
                            } else
                            if (IARI_GEO_SMS.equalsIgnoreCase(value)) {
                                result |= 0x100;
                            } else
                            if (IARI_FT_HTTP.equalsIgnoreCase(value)) {
                                result |= 0x200;
                            } else
                            if (IARI_CHATBOT.equalsIgnoreCase(value)) {
                                result |= 0x800;
                            } else
                            if (IARI_CHATBOT_SA.equalsIgnoreCase(value)) {
                                result |= 0x1000;
                            }
                        }
                        break;

                    case SVC_IMDN_AGGREGATION:
                        result |= 0x80;
                        break;

                    case SVC_GSMA_CALLCOMPOSER:
                        result |= 0x400;
                        break;

                    case SVC_GSMA_BOTVERSION:
                        // TODO: Only support fixed version 1 for now,
                        // find a way to support multiple versions dynamically.
                        if (tags.size() == 1) {
                            for (String value : tags.values) {
                                if ("#=1".equals(value)) {
                                    result |= 0x2000;
                                    break;
                                }
                            }
                        }
                        break;
                }
            }
        }

        return result;
    }

    private void initialize(long features) {
        /**
         *  Bitmask used to transport RCS feature tags toward IMS stack avoid
         *  layer transport overhead. Detailed mapping as below:
         *         0x01  session(ICSI)
         *         0x02  filetransfer(ICSI)
         *         0x04  msg(ICSI)
         *         0x08  largemsg(ICSI)
         *         0x10  geopush(IARI)
         *         0x20  geopull(IARI)
         *         0x40  geopullft(IARI)
         *         0x80  imdn aggregation
         *        0x100  geosms(IARI)
         *        0x200  fthttp(IARI)
         *        0x400  callcomposer(GSMA)
         *        0x800  chatbot(IARI)
         *       0x1000  chatbot_sa(IARI)
         *       0x2000  botversion(GSMA)
         */
        StringBuilder ICSI = new StringBuilder(SVC_3GPP_ICSI + "=\"");
        StringBuilder IARI = new StringBuilder(SVC_3GPP_IARI + "=\"");

        if ((features & 0x01) > 0)
            ICSI.append(ICSI_CPM_SESSION + ",");
        if ((features & 0x02) > 0)
            ICSI.append(ICSI_CPM_FILETRANSFER + ",");
        if ((features & 0x04) > 0)
            ICSI.append(ICSI_CPM_MSG + ",");
        if ((features & 0x08) > 0)
            ICSI.append(ICSI_CPM_LARGE_MSG + ",");
        if ((features & 0x10) > 0)
            IARI.append(IARI_GEO_PUSH + ",");
        if ((features & 0x20) > 0)
            IARI.append(IARI_GEO_PULL + ",");
        if ((features & 0x40) > 0)
            IARI.append(IARI_GEO_PULLFT + ",");
        if ((features & 0x100) > 0)
            IARI.append(IARI_GEO_SMS + ",");
        if ((features & 0x200) > 0)
            IARI.append(IARI_FT_HTTP + ",");
        if ((features & 0x800) > 0)
            IARI.append(IARI_CHATBOT + ",");
        if ((features & 0x1000) > 0)
            IARI.append(IARI_CHATBOT_SA + ",");

        StringBuilder sb = new StringBuilder();
        if (ICSI.charAt(ICSI.length()-1) == ',') {
            ICSI.setCharAt(ICSI.length()-1, '\"');
            sb.append(ICSI);
            sb.append(';');
        }
        if (IARI.charAt(IARI.length()-1) == ',') {
            IARI.setCharAt(IARI.length()-1, '\"');
            sb.append(IARI);
            sb.append(';');
        }

        if ((features & 0x40) > 0) {
            sb.append(SVC_IMDN_AGGREGATION);
            sb.append(';');
        }

        if ((features & 0x400) > 0) {
            sb.append(SVC_GSMA_CALLCOMPOSER);
            sb.append(';');
        }

        if ((features & 0x2000) > 0) {
            sb.append(SVC_GSMA_BOTVERSION);
            sb.append("=");
            sb.append("\"#=1\"");
            sb.append(";");
        }
        if (sb.length() > 0
                && sb.charAt(sb.length()-1) == ';')
            sb.deleteCharAt(sb.length()-1);

        initialize(sb.toString());
    }

    private void initialize(String features) {
        if (features == null) {
            features = "";
        }

        this.features.clear();
        if (features.length() > 0) {
            for (String feature : features.split(";")) {
                String nameValue[] = feature.split("=", 2);
                if (nameValue.length < 2) {
                    this.features.put(feature, new TagValueList(""));
                } else {
                    String name = nameValue[0];
                    String value = nameValue[1];

                    // Tag value list need to be wrapped by DOUBLE QUOTE
                    if (value.indexOf('"') != 0
                            || value.lastIndexOf('"') != value.length() - 1) {
                        throw new RuntimeException("Invalid feature tag value list format");
                    }

                    if (this.features.containsKey(name)) {
                        // Merge feature values
                        this.features.get(name).add(value);
                    } else {
                        this.features.put(name, new TagValueList(value));
                    }
                }
            }
        }
    }

    private void addFeature(Capability capability) {

        if (capability.features.size() > 0) {
            for (Map.Entry<String, TagValueList> entry : capability.features.entrySet()) {
                String key = entry.getKey();
                TagValueList value = entry.getValue();

                if (this.features.containsKey(key)) {
                    this.features.get(key).add(value);
                } else {
                    this.features.put(key, value);
                }
            }
        }
    }

    private void removeFeature(Capability capability) {

        if (capability.features.size() > 0) {
            for (Map.Entry<String, TagValueList> entry : capability.features.entrySet()) {
                String key = entry.getKey();
                TagValueList tags = entry.getValue();

                if (this.features.containsKey(key)) {
                    this.features.get(key).remove(tags);
                }
            }
        }
    }

    /**
     * Class represent feature tag values.
     * Single feature tag name might related to 0~n feature tag values.
     */
    public static class TagValueList implements Parcelable {

        TagValueList(String values) {
            initialize(values);
        }

        public void add(TagValueList values) {
            addValues(values);
        }

        public void remove(TagValueList values) {
            removeValues(values);
        }

        public void add(String values) {
            add(new TagValueList(values));
        }

        public void remove(String values) {
            remove(new TagValueList(values));
        }

        public boolean contains(TagValueList otherValue) {
            for (String value : otherValue.values) {
                if (!values.contains(value))
                    return false;
            }
            return true;
        }

        int size() {
            return values.size();
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.toString());
        }

        @Override
        public int describeContents() {
            return 0;
        }

        public static final Creator<TagValueList> CREATOR = new Creator<TagValueList>() {
            @Override
            public TagValueList createFromParcel(Parcel in) {
                return new TagValueList(in.readString());
            }

            @Override
            public TagValueList[] newArray(int size) {
                return new TagValueList[size];
            }
        };

        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder();

            if (values.size() > 0) {

                builder.append('"');

                Iterator<String> it = values.iterator();
                builder.append(it.next());
                while (it.hasNext()) {
                    builder.append(',');
                    builder.append(it.next());
                }

                builder.append('"');
            }
            return builder.toString();
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj)
                return true;

            if (obj instanceof TagValueList) {
                TagValueList other = (TagValueList)obj;

                return other.values.hashCode() == values.hashCode();
            }

            return false;
        }

        @Override
        public int hashCode() {
            return values.hashCode();
        }

        private void initialize(String values) {
            if (values.length() == 0)
                return;

            /* Guarantee format before creation, no need to check here */
            String data = values.substring(1, values.lastIndexOf('"'));

            this.values.addAll(Arrays.asList(data.split(",")));
        }

        private void addValues(TagValueList valuesToAdd) {
            if (valuesToAdd.values.size() > 0) {
                this.values.addAll(valuesToAdd.values);
            }
        }

        private void removeValues(TagValueList valuesToRemove) {
            if (valuesToRemove.values.size() > 0) {
                this.values.removeAll(valuesToRemove.values);
            }
        }

        private LinkedHashSet<String> values = new LinkedHashSet<>();
        private int hashCode = 0;
    }

    private LinkedHashMap<String, TagValueList> features = new LinkedHashMap<>();
    private final String SVC_3GPP_ICSI = "+g.3gpp.icsi-ref";
    private final String SVC_3GPP_IARI = "+g.3gpp.iari-ref";
    private final String SVC_IMDN_AGGREGATION = "imdn-aggregation";
    private final String SVC_GSMA_CALLCOMPOSER = "+g.gsma.callcomposer";
    private final String SVC_GSMA_BOTVERSION = "+g.gsma.rcs.botversion";

    private final String ICSI_CPM_MSG = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg";
    private final String ICSI_CPM_LARGE_MSG = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg";
    private final String ICSI_CPM_SESSION = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session";
    private final String ICSI_CPM_FILETRANSFER = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer";

    private final String IARI_GEO_PUSH = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush";
    private final String IARI_GEO_PULL = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopull";
    private final String IARI_GEO_PULLFT = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopullft";
    private final String IARI_GEO_SMS = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms";
    private final String IARI_FT_HTTP = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp";
    private final String IARI_CHATBOT = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot";
    private final String IARI_CHATBOT_SA = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa";

}
