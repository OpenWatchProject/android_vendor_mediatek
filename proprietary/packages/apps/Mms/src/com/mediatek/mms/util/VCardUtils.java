/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.mms.util;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Im;
import android.provider.ContactsContract.CommonDataKinds.Organization;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.SipAddress;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.support.v4.content.FileProvider;
import android.text.TextUtils;
import android.widget.Toast;

import com.android.mms.R;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;
import com.android.vcard.VCardConfig;
import com.android.vcard.VCardEntry;
import com.android.vcard.VCardEntry.AnniversaryData;
import com.android.vcard.VCardEntry.BirthdayData;
import com.android.vcard.VCardEntry.EmailData;
import com.android.vcard.VCardEntry.EntryElement;
import com.android.vcard.VCardEntry.EntryElementIterator;
import com.android.vcard.VCardEntry.EntryLabel;
import com.android.vcard.VCardEntry.ImData;
import com.android.vcard.VCardEntry.NameData;
import com.android.vcard.VCardEntry.NicknameData;
import com.android.vcard.VCardEntry.NoteData;
import com.android.vcard.VCardEntry.OrganizationData;
import com.android.vcard.VCardEntry.PhoneData;
import com.android.vcard.VCardEntry.PostalData;
import com.android.vcard.VCardEntry.SipData;
import com.android.vcard.VCardEntry.WebsiteData;
import com.android.vcard.VCardEntryConstructor;
import com.android.vcard.VCardEntryHandler;
import com.android.vcard.VCardParser;
import com.android.vcard.VCardParser_V21;
import com.android.vcard.exception.VCardException;

import com.mediatek.mms.model.FileAttachmentModel;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class VCardUtils {
    private static final String TAG = "Mms/VCardUtils";
    private static final String TEMP_FILES_FOLDER_NAME =
        MessageUtils.MMS_SHARED_FILES_FOLDER_NAME;
    private static final String MMS_FILE_PROVIDER_AUTHORITIES =
        MessageUtils.MMS_SHARED_FILE_PROVIDER_AUTHORITIES;
    public static final int PARSE_ALL = 0;
    public static final int PARSE_ONE = 1;

    public static void importVCard(Context context, FileAttachmentModel attach) {
        MessageUtils.deleteAllSharedFiles(context, ".vcf");
        MmsLog.dpi(TAG, "importVCard, file uri: " + attach.getUri());
        final File tempVCard = MessageUtils.createTempFileExposed(
                context, attach.getUri(), attach.getSrc());
        if (tempVCard == null || !tempVCard.exists() || tempVCard.length() <= 0) {
            MmsLog.epi(TAG, "importVCard, file is not exists or empty " + tempVCard);
            return;
        }

        Uri vCardUri = FileProvider.getUriForFile(
                context, MMS_FILE_PROVIDER_AUTHORITIES, tempVCard);
        MmsLog.ipi(TAG, "importVCard, vCard uri: " + vCardUri);
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(vCardUri, attach.getContentType().toLowerCase());
        intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        context.startActivity(intent);
    }

    public static List<VCardEntry> parserVCardFile(Context context,
            Uri dataUri, VCardEntryConstructor interpreter, int parseFlag) {
        InputStream inputStream = null;
        VCardParser parser = new VCardParser_V21();
        MyVCardEntryHandler myVCardEntryHandler = new MyVCardEntryHandler();
        try {
            interpreter.addEntryHandler(myVCardEntryHandler);
            parser.addInterpreter(interpreter);
            inputStream = context.getContentResolver().openInputStream(dataUri);
            switch (parseFlag) {
            case PARSE_ALL:
                parser.parse(inputStream);
                break;
            case PARSE_ONE:
                parser.parseOne(inputStream);
                break;
            default:
                return null;
            }
        } catch (VCardException e) {
            MmsLog.e(TAG, "parserVCardFile(): VCardException.", e);
            toastErrorHappened(context, R.string.file_attachment_import_vcard);
            return null;
        } catch (IOException e) {
            MmsLog.e(TAG, "parserVCardFile(): IOException.", e);
            toastErrorHappened(context, R.string.file_attachment_import_vcard);
            return null;
        } finally {
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (IOException e) {
                MmsLog.e(TAG, "parserVCardFile(): IOException when close.", e);
            }
        }
        return myVCardEntryHandler.getVCardEntryList();
    }

    public static String getVCardFirstContactName(Context context, Uri dataUri) {
        List<VCardEntry> vCardEntryList = parserVCardFile(
                context, dataUri, new VCardEntryConstructor(), PARSE_ONE);
        if (vCardEntryList == null || vCardEntryList.size() == 0) {
            return "";
        } else {
            return vCardEntryList.get(0).getNameData().displayName;
        }
    }

    public static int getVCardContactsCount(Context context, Uri dataUri) {
        MmsLog.epi(TAG, "getVCardContactsCount(): dataUri = " + dataUri);
        if (context == null || dataUri == null) {
            return 0;
        }
        InputStream inputStream = null;
        BufferedReader bufferedReader = null;
        int count = 0;
        try {
            inputStream = context.getContentResolver().openInputStream(dataUri);
            InputStreamReader tmpReader = new InputStreamReader(
                    inputStream, VCardConfig.DEFAULT_INTERMEDIATE_CHARSET);
            bufferedReader = new BufferedReader(tmpReader);

            String line;
            while (true) {
                line = bufferedReader.readLine();
                if (line == null) {
                    return count;
                } else if (line.trim().length() > 0) {
                    String[] strArray = line.split(":", 2);
                    int length = strArray.length;
                    if (length == 2 && strArray[0].trim().equalsIgnoreCase("BEGIN")
                            && strArray[1].trim().equalsIgnoreCase("VCARD")) {
                        count++;
                    }
                }
            }
        } catch (IOException e) {
            MmsLog.e(TAG, "getVCardContactsCount(): IOException.", e);
            return 0;
        } finally {
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
                if (bufferedReader != null) {
                    bufferedReader.close();
                }
            } catch (IOException e) {
                MmsLog.e(TAG, "getVCardContactsCount(): IOException when close.", e);
            }
        }
    }

    private static class MyVCardEntryHandler implements VCardEntryHandler {
        private List<VCardEntry> mVCardEntryList = new ArrayList<VCardEntry>();

        @Override
        public void onStart() {
        }

        @Override
        public void onEntryCreated(final VCardEntry entry) {
            mVCardEntryList.add(entry);
        }

        @Override
        public void onEnd() {
        }

        public List<VCardEntry> getVCardEntryList() {
            return mVCardEntryList;
        }
    }

    public static class MyToStringIterator implements EntryElementIterator {
        private static final String STR_COLON = ": ";
        private static final String STR_NEWLINE = "\n";
        private static final String STR_SPLIT = ", ";

        private Context mContext;
        private StringBuilder mBuilder;

        public void setContext(Context context) {
            mContext = context;
        }

        @Override
        public void onIterationStarted() {
            mBuilder = new StringBuilder();
        }

        @Override
        public void onElementGroupStarted(EntryLabel label) {
            if (label == EntryLabel.NAME) {
                mBuilder.append(mContext.getResources().getString(R.string.label_full_name))
                    .append(STR_NEWLINE);
            } else if (label == EntryLabel.PHONE) {
                mBuilder.append(mContext.getResources().getString(R.string.label_phone))
                    .append(STR_NEWLINE);
            } else if (label == EntryLabel.EMAIL) {
                mBuilder.append(mContext.getResources().getString(R.string.label_email))
                    .append(STR_NEWLINE);
            } else if (label == EntryLabel.POSTAL_ADDRESS) {
                mBuilder.append(mContext.getResources().getString(R.string.label_postal))
                    .append(STR_NEWLINE);
            } else if (label == EntryLabel.ORGANIZATION) {
                mBuilder.append(mContext.getResources().getString(R.string.label_organization))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.IM) {
                mBuilder.append(mContext.getResources().getString(R.string.label_im))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.WEBSITE) {
                mBuilder.append(mContext.getResources().getString(R.string.label_website))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.SIP) {
                mBuilder.append(mContext.getResources().getString(R.string.label_sip_address))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.NICKNAME) {
                mBuilder.append(mContext.getResources().getString(R.string.label_nick_name))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.NOTE) {
                mBuilder.append(mContext.getResources().getString(R.string.label_notes))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.BIRTHDAY) {
                mBuilder.append(mContext.getResources().getString(R.string.label_event))
                .append(STR_NEWLINE);
            } else if (label == EntryLabel.ANNIVERSARY) {
                mBuilder.append(mContext.getResources().getString(R.string.label_event))
                .append(STR_NEWLINE);
            }
            /* M: For later use @{
              else if (label == EntryLabel.PHOTO) {
                /// M: do nothing for photo
            } else if (label == EntryLabel.ANDROID_CUSTOM) {
            } @} */
        }

        @Override
        public boolean onElement(EntryElement elem) {
            if (elem.isEmpty()) {
                MmsLog.d(TAG, "MyToStringIterator.onElement(): elem is empty.");
                return false;
            }

            if (elem instanceof NameData) {
                NameData nameData = (NameData) elem;
                mBuilder.append(nameData.displayName).append(STR_NEWLINE);
            } else if (elem instanceof PhoneData) {
                PhoneData phoneData = (PhoneData) elem;
                int resId = Phone.getTypeLabelResource(phoneData.getType());
                mBuilder.append(mContext.getResources().getString(resId)).append(STR_COLON)
                    .append(phoneData.getNumber()).append(STR_NEWLINE);
            } else if (elem instanceof EmailData) {
                EmailData emailData = (EmailData) elem;
                int resId = Email.getTypeLabelResource(emailData.getType());
                mBuilder.append(mContext.getResources().getString(resId)).append(STR_COLON)
                    .append(emailData.getAddress()).append(STR_NEWLINE);
            } else if (elem instanceof ImData) {
                ImData imData = (ImData) elem;
                int resId = Im.getTypeLabelResource(imData.getProtocol());
                mBuilder.append(mContext.getResources().getString(resId)).append(STR_COLON)
                    .append(imData.getAddress()).append(STR_NEWLINE);
            } else if (elem instanceof NicknameData) {
                NicknameData nicknameData = (NicknameData) elem;
                mBuilder.append(mContext.getResources().getString(R.string.label_nick_name))
                    .append(STR_COLON).append(nicknameData.getNickname()).append(STR_NEWLINE);
            } else if (elem instanceof WebsiteData) {
                WebsiteData websiteData = (WebsiteData) elem;
                mBuilder.append(websiteData.getWebsite()).append(STR_NEWLINE);
            } else if (elem instanceof BirthdayData) {
                BirthdayData birthdayData = (BirthdayData) elem;
                mBuilder.append(mContext.getResources()
                        .getString(com.android.internal.R.string.eventTypeBirthday))
                        .append(STR_COLON)
                    .append(birthdayData.getBirthday()).append(STR_NEWLINE);
            } else if (elem instanceof AnniversaryData) {
                AnniversaryData anniversaryData = (AnniversaryData) elem;
                mBuilder.append(mContext.getResources()
                        .getString(com.android.internal.R.string.eventTypeAnniversary))
                        .append(STR_COLON)
                    .append(anniversaryData.getAnniversary()).append(STR_NEWLINE);
            } else if (elem instanceof PostalData) {
                PostalData postalData = (PostalData) elem;
                int resId = StructuredPostal.getTypeLabelResource(postalData.getType());
                mBuilder.append(mContext.getResources().getString(resId)).append(STR_COLON)
                    .append(STR_NEWLINE);
                if (!TextUtils.isEmpty(postalData.getStreet())) {
                    mBuilder.append(postalData.getStreet()).append(STR_NEWLINE);
                }
                if (!TextUtils.isEmpty(postalData.getPobox())) {
                    mBuilder.append(postalData.getPobox()).append(STR_NEWLINE);
                }
                if (!TextUtils.isEmpty(postalData.getExtendedAddress())) {
                    mBuilder.append(postalData.getExtendedAddress()).append(STR_NEWLINE);
                }
                if (!TextUtils.isEmpty(postalData.getLocalty())) {
                    mBuilder.append(postalData.getLocalty()).append(STR_NEWLINE);
                }
                if (!TextUtils.isEmpty(postalData.getRegion())) {
                    mBuilder.append(postalData.getRegion()).append(STR_NEWLINE);
                }
                if (!TextUtils.isEmpty(postalData.getPostalCode())) {
                    mBuilder.append(postalData.getPostalCode()).append(STR_NEWLINE);
                }
            } else if (elem instanceof NoteData) {
                NoteData noteData = (NoteData) elem;
                mBuilder.append(noteData.getNote()).append(STR_NEWLINE);
            } else if (elem instanceof OrganizationData) {
                OrganizationData organizationData = (OrganizationData) elem;
                int resId = Organization.getTypeLabelResource(organizationData.getType());
                mBuilder.append(mContext.getResources().getString(resId)).append(STR_COLON);
                if (!TextUtils.isEmpty(organizationData.getTitle())) {
                    mBuilder.append(organizationData.getTitle()).append(STR_SPLIT);
                }
                if (!TextUtils.isEmpty(organizationData.getPhoneticName())) {
                    mBuilder.append(organizationData.getPhoneticName()).append(STR_SPLIT);
                }
                if (!TextUtils.isEmpty(organizationData.getDepartmentName())) {
                    mBuilder.append(organizationData.getDepartmentName()).append(STR_SPLIT);
                }
                if (!TextUtils.isEmpty(organizationData.getOrganizationName())) {
                    mBuilder.append(organizationData.getOrganizationName()).append(STR_SPLIT);
                }
                int lastIndex = mBuilder.lastIndexOf(STR_SPLIT);
                if (lastIndex > 0 && lastIndex == (mBuilder.length() - STR_SPLIT.length())) {
                    mBuilder = new StringBuilder(mBuilder.substring(0, lastIndex));
                }
                mBuilder.append(STR_NEWLINE);
            } else if (elem instanceof SipData) {
                SipData sipData = (SipData) elem;
                if (!sipData.isEmpty()) {
                    int resId = SipAddress.getTypeLabelResource(sipData.getType());
                    mBuilder.append(mContext.getResources().getString(resId)).append(STR_COLON)
                        .append(sipData.getAddress()).append(STR_NEWLINE);
                }
            }
            /* M: @{
            else if (elem instanceof PhotoData) {
                /// M: do nothing for photos
//              PhotoData photoData = (PhotoData) elem;
            } else if (elem instanceof AndroidCustomData) {
//                AndroidCustomData androidCustomData = (AndroidCustomData) elem;
            } @} */
            return true;
        }

        @Override
        public void onElementGroupEnded() {
            mBuilder.append(STR_NEWLINE);
        }

        @Override
        public void onIterationEnded() {
            int lastIndex = mBuilder.lastIndexOf(STR_NEWLINE);
            if (lastIndex > 0 && lastIndex == (mBuilder.length() - STR_NEWLINE.length())) {
                mBuilder = new StringBuilder(mBuilder.substring(0, lastIndex));
            }
        }

        @Override
        public String toString() {
            int lastIndex = mBuilder.lastIndexOf(STR_NEWLINE);
            if (lastIndex > 0 && lastIndex == (mBuilder.length() - 1)) {
                return mBuilder.substring(0, lastIndex);
            }
            return mBuilder.toString();
        }
    }

    public static void toastErrorHappened(Context context, int resId) {
        Toast.makeText(context, resId, Toast.LENGTH_SHORT).show();
    }

    /// M: fix bug ALPS01505548, delete VCard temp file
    public static void deleteVCardTempFile(Context context, String filename) {
        if (filename != null && filename.endsWith(".vcf")) {
            context.deleteFile(filename);
        }
    }

    public static void deleteVCardTempFiles(Context context, ArrayList<String> files) {
        for (int i = 0; i < files.size(); i++) {
            deleteVCardTempFile(context, files.get(i));
        }
        files.clear();
    }

    public static long[] getContactsIds(String vCardContactsIds) {
        long[] contactsIds = null;
        if (vCardContactsIds != null && !vCardContactsIds.equals("")) {
            String[] vCardConIds = vCardContactsIds.split(",");
            MmsLog.e(TAG, "getContactIds(): vCardConIds.length" + vCardConIds.length);
            contactsIds = new long[vCardConIds.length];
            try {
                for (int i = 0; i < vCardConIds.length; i++) {
                    contactsIds[i] = Long.parseLong(vCardConIds[i]);
                }
            } catch (NumberFormatException e) {
                contactsIds = null;
            }
        }

        return contactsIds;
    }
}
