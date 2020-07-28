/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony.phb;

import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.MergeCursor;
import android.net.Uri;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.internal.telephony.IccInternalInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;

import java.io.UnsupportedEncodingException;
import java.util.List;


/**
 * {@hide}
 */
public class MtkIccProvider implements IccInternalInterface {
    private static final String TAG = "MtkIccProvider";
    private static final boolean DBG = !SystemProperties.get("ro.build.type").equals("user");

    private static final String[] ADDRESS_BOOK_COLUMN_NAMES = new String[] {
        "index",
        "name",
        "number",
        "emails",
        "additionalNumber",
        "groupIds",
        "_id",
        "aas",
        "sne",
    };
    private static final int ADDRESS_SUPPORT_AAS = 8;
    private static final int ADDRESS_SUPPORT_SNE = 9;

    protected static final int ADN = 1;
    protected static final int ADN_SUB = 2;
    protected static final int FDN = 3;
    protected static final int FDN_SUB = 4;
    protected static final int SDN = 5;
    protected static final int SDN_SUB = 6;
    protected static final int UPB = 7;
    protected static final int UPB_SUB = 8;
    protected static final int ADN_ALL = 9;

    protected static final String STR_TAG = "tag";
    protected static final String STR_NUMBER = "number";
    protected static final String STR_EMAILS = "emails";
    protected static final String STR_PIN2 = "pin2";
    protected static final String STR_ANR = "anr";
    protected static final String STR_INDEX = "index";

    private Context mContext;
    private static UriMatcher URL_MATCHER;

    public MtkIccProvider(UriMatcher URL_MATCHER, Context context) {
        logi("MtkIccProvider URL_MATCHER " + URL_MATCHER);
        URL_MATCHER.addURI("icc", "pbr", UPB);
        URL_MATCHER.addURI("icc", "pbr/subId/#", UPB_SUB);
        this.URL_MATCHER = URL_MATCHER;
        mContext = context;
    }

    @Override
    public Cursor query(Uri url, String[] projection, String selection,
            String[] selectionArgs, String sort) {
        logi("query " + url);
        switch (URL_MATCHER.match(url)) {
            case ADN:
                return loadFromEf(IccConstants.EF_ADN,
                        SubscriptionManager.getDefaultSubscriptionId());

            case ADN_SUB:
                return loadFromEf(IccConstants.EF_ADN, getRequestSubId(url));

            case FDN:
                return loadFromEf(IccConstants.EF_FDN,
                        SubscriptionManager.getDefaultSubscriptionId());

            case FDN_SUB:
                return loadFromEf(IccConstants.EF_FDN, getRequestSubId(url));

            case SDN:
                return loadFromEf(IccConstants.EF_SDN,
                        SubscriptionManager.getDefaultSubscriptionId());

            case SDN_SUB:
                return loadFromEf(IccConstants.EF_SDN, getRequestSubId(url));

            case UPB:
                return loadFromEf(IccConstants.EF_PBR,
                        SubscriptionManager.getDefaultSubscriptionId());

            case UPB_SUB:
                return loadFromEf(IccConstants.EF_PBR, getRequestSubId(url));

            case ADN_ALL:
                return loadAllSimContacts(IccConstants.EF_ADN);

            default:
                throw new IllegalArgumentException("Unknown URL " + url);
        }

    }

    private Cursor loadAllSimContacts(int efType) {
        SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        int[] subIdList = subscriptionManager.getActiveSubscriptionIdList();
        Cursor [] result = new Cursor[subIdList.length];

        int i = 0;
        for (int subId : subIdList) {
            result[i++] = loadFromEf(efType, subId);
            Rlog.i(TAG, "loadAllSimContacts: subId=" + subId);
        }

        return new MergeCursor(result);
    }

    private MatrixCursor loadFromEf(int efType, int subId) {
        if (DBG) log("loadFromEf: efType=0x" +
                Integer.toHexString(efType).toUpperCase() + ", subscription=" + subId);

        List<MtkAdnRecord> adnRecords = null;
        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();
            if (iccIpb != null) {
                adnRecords = iccIpb.getAdnRecordsInEfForSubscriber(subId, efType);
            }
        } catch (RemoteException ex) {
            if (DBG) log(ex.toString());
        } catch (SecurityException ex) {
            if (DBG) log(ex.toString());
        }

        if (adnRecords != null) {
            // Load the results
            final int N = adnRecords.size();
            final MatrixCursor cursor = new MatrixCursor(ADDRESS_BOOK_COLUMN_NAMES, N);
            if (DBG) log("adnRecords.size=" + N);
            for (int i = 0; i < N ; i++) {
                loadRecord(adnRecords.get(i), cursor, i);
            }
            logi("query success, size = " + N);
            return cursor;
        } else {
            // No results to load
            Rlog.w(TAG, "Cannot load ADN records");
            return new MatrixCursor(ADDRESS_BOOK_COLUMN_NAMES);
        }
    }

    @Override
    public Uri insert(Uri url, ContentValues initialValues) {
        int efType;
        String pin2 = null;
        int subId;

        logi("insert " + url);
        int match = URL_MATCHER.match(url);
        switch (match) {
            case ADN:
                efType = IccConstants.EF_ADN;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case ADN_SUB:
                efType = IccConstants.EF_ADN;
                subId = getRequestSubId(url);
                break;

            case FDN:
                efType = IccConstants.EF_FDN;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                pin2 = initialValues.getAsString("pin2");
                break;

            case FDN_SUB:
                efType = IccConstants.EF_FDN;
                subId = getRequestSubId(url);
                pin2 = initialValues.getAsString("pin2");
                break;

            case UPB:
                efType = IccConstants.EF_PBR;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case UPB_SUB:
                efType = IccConstants.EF_PBR;
                subId = getRequestSubId(url);
                break;

            default:
                throw new UnsupportedOperationException(
                        "Cannot insert into URL: " + url);
        }

        String tag = initialValues.getAsString("tag");
        String number = initialValues.getAsString("number");
        int result = 0;
        if (UPB == match || UPB_SUB == match) {
            String strGas = initialValues.getAsString("gas");
            String strAnr = initialValues.getAsString("anr");
            String strEmail = initialValues.getAsString("emails");
            if (ADDRESS_BOOK_COLUMN_NAMES.length >= ADDRESS_SUPPORT_AAS) {
                Integer aasIndex = initialValues.getAsInteger("aas");
                if (number == null) {
                    number = "";
                }
                if (tag == null) {
                    tag = "";
                }
                MtkAdnRecord record = new MtkAdnRecord(efType, 0, tag, number);
                record.setAnr(strAnr);
                if (initialValues.containsKey("anr2")) {
                    String strAnr2 = initialValues.getAsString("anr2");
                    if (DBG) {
                        log("insert anr2: " + getMaskString(strAnr2));
                    }
                    record.setAnr(strAnr2, 1);
                }
                if (initialValues.containsKey("anr3")) {
                    String strAnr3 = initialValues.getAsString("anr3");
                    if (DBG) {
                        log("insert anr3: " + getMaskString(strAnr3));
                    }
                    record.setAnr(strAnr3, 2);
                }
                record.setGrpIds(strGas);
                String[] emails = null;
                if (strEmail != null && !strEmail.equals("")) {
                    emails = new String[1];
                    emails[0] = strEmail;
                }
                record.setEmails(emails);
                if (aasIndex != null) {
                    record.setAasIndex(aasIndex);
                }
                if (ADDRESS_BOOK_COLUMN_NAMES.length >= ADDRESS_SUPPORT_SNE) {
                    String sne = initialValues.getAsString("sne");
                    record.setSne(sne);
                }

                logi("updateUsimPBRecordsBySearchWithError ");
                result = updateUsimPBRecordsBySearchWithError(efType, new MtkAdnRecord("", "", ""),
                        record, subId);
            } else {
                logi("addUsimRecordToEf ");
                result = addUsimRecordToEf(efType, tag, number, strAnr, strEmail, strGas, subId);
            }
            /// M: CSIM PHB handling @{
            // Update csim phb storage for modem3
            if (result > 0) {
                updatePhbStorageInfo(1, subId);
            }
            /// @}
        } else {
            if (number == null) {
                number = "";
            }
            if (tag == null) {
                tag = "";
            }
            logi("addIccRecordToEf:" + getMaskString(number) + ",tag:" + getMaskString(tag));
            result = addIccRecordToEf(efType, tag, number, null, pin2, subId);
        }

        StringBuilder buf = new StringBuilder("content://icc/");

        if (result <= ERROR_ICC_PROVIDER_UNKNOWN) {
            buf.append("error/");
            buf.append(result);
        } else {
            switch (match) {
                case ADN:
                    buf.append("adn/");
                    break;

                case ADN_SUB:
                    buf.append("adn/subId/");
                    break;

                case FDN:
                    buf.append("fdn/");
                    break;

                case FDN_SUB:
                    buf.append("fdn/subId/");
                    break;
                case UPB:
                    buf.append("pbr/");
                    break;

                case UPB_SUB:
                    buf.append("pbr/subId/");
                    break;
                default:
                throw new UnsupportedOperationException(
                        "Cannot insert into URL: " + url);
            }

            // TODO: we need to find out the rowId for the newly added record
            buf.append(result);
        }

        Uri resultUri = Uri.parse(buf.toString());
        logi(resultUri.toString());

        return resultUri;
    }

    private String normalizeValue(String inVal) {
        int len = inVal.length();
        // If name is empty in contact return null to avoid crash.
        if (len == 0) {
            if (DBG) log("len of input String is 0");
            return inVal;
        }
        String retVal = inVal;

        if (inVal.charAt(0) == '\'' && inVal.charAt(len - 1) == '\'') {
            retVal = inVal.substring(1, len - 1);
        }

        return retVal;
    }

    @Override
    public int delete(Uri url, String where, String[] whereArgs) {
        int efType;
        int subId;

        logi("delete " + url);
        int match = URL_MATCHER.match(url);
        switch (match) {
            case ADN:
                efType = IccConstants.EF_ADN;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case ADN_SUB:
                efType = IccConstants.EF_ADN;
                subId = getRequestSubId(url);
                break;

            case FDN:
                efType = IccConstants.EF_FDN;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case FDN_SUB:
                efType = IccConstants.EF_FDN;
                subId = getRequestSubId(url);
                break;

            case UPB:
                efType = IccConstants.EF_PBR;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case UPB_SUB:
                efType = IccConstants.EF_PBR;
                subId = getRequestSubId(url);
                break;
            default:
                throw new UnsupportedOperationException(
                        "Cannot insert into URL: " + url);
        }

        // parse where clause
        String tag = "";
        String number = "";
        String[] emails = null;
        String pin2 = null;
        int nIndex = -1;

        String[] tokens = where.split(" AND ");
        int n = tokens.length;

        while (--n >= 0) {
            String param = tokens[n];
            int index = param.indexOf('=');
            if (index == -1) {
                Rlog.e(TAG, "resolve: bad whereClause parameter: " + param);
                continue;
            }


            String key = param.substring(0, index).trim();
            String val = param.substring(index + 1).trim();

            /*
             * String[] pair = param.split("="); if (pair.length != 2) {
             * Rlog.e(TAG, "resolve: bad whereClause parameter: " + param);
             * continue; } String key = pair[0].trim(); String val =
             * pair[1].trim();
             */

            if (STR_INDEX.equals(key)) {
                nIndex = Integer.parseInt(val);
            } else if (STR_TAG.equals(key)) {
                tag = normalizeValue(val);
            } else if (STR_NUMBER.equals(key)) {
                number = normalizeValue(val);
            } else if (STR_EMAILS.equals(key)) {
                //TODO(): Email is null.
                emails = null;
            } else if (STR_PIN2.equals(key)) {
                pin2 = normalizeValue(val);
            }
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;
        if (nIndex > 0) {
            logi("delete index is " + nIndex);
            if (UPB == match || UPB_SUB == match) {
                logi("deleteUsimRecordFromEfByIndex ");
                result = deleteUsimRecordFromEfByIndex(efType, nIndex, subId);
                /// M: CSIM PHB handling @{
                // Update csim phb storage for modem3
                if (result > 0) {
                    updatePhbStorageInfo(-1, subId);
                }
                /// @}
            } else {
                logi("deleteIccRecordFromEfByIndex ");
                result = deleteIccRecordFromEfByIndex(efType, nIndex, pin2, subId);
            }
            logi("delete result = " + result);
            return result;
        }

        if (efType == IccConstants.EF_FDN && TextUtils.isEmpty(pin2)) {
            return ERROR_ICC_PROVIDER_PASSWORD_ERROR;
        }


        if (tag.length() == 0 && number.length() == 0) {
            return ERROR_ICC_PROVIDER_UNKNOWN;
        }

        if (UPB == match || UPB_SUB == match) {
            if (ADDRESS_BOOK_COLUMN_NAMES.length >= ADDRESS_SUPPORT_AAS) {
                logi("updateUsimPBRecordsBySearchWithError ");
                result = updateUsimPBRecordsBySearchWithError(efType,
                        new MtkAdnRecord(tag, number, ""), new MtkAdnRecord("", "", ""), subId);
            } else {
                logi("deleteUsimRecordFromEf ");
                result = deleteUsimRecordFromEf(efType, tag, number, emails, subId);
            }
            /// M: CSIM PHB handling @{
            // Update csim phb storage for modem3
            if (result > 0) {
                updatePhbStorageInfo(-1, subId);
            }
            /// @}
        } else {
            logi("deleteIccRecordFromEf ");
            result = deleteIccRecordFromEf(efType, tag, number, emails, pin2, subId);
        }

        logi("delete result = " + result);
        return result;
    }

    @Override
    public int update(Uri url, ContentValues values,
            String where, String[] whereArgs) {
        String pin2 = null;
        int efType;
        int subId;

        logi("update " + url);
        int match = URL_MATCHER.match(url);
        switch (match) {
            case ADN:
                efType = IccConstants.EF_ADN;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case ADN_SUB:
                efType = IccConstants.EF_ADN;
                subId = getRequestSubId(url);
                break;

            case FDN:
                efType = IccConstants.EF_FDN;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                pin2 = values.getAsString("pin2");
                break;

            case FDN_SUB:
                efType = IccConstants.EF_FDN;
                subId = getRequestSubId(url);
                pin2 = values.getAsString("pin2");
                break;

            case UPB:
                efType = IccConstants.EF_PBR;
                subId = SubscriptionManager.getDefaultSubscriptionId();
                break;

            case UPB_SUB:
                efType = IccConstants.EF_PBR;
                subId = getRequestSubId(url);
                break;

            default:
                throw new IllegalArgumentException("Unknown URL " + match);
        }

        String tag = values.getAsString("tag");
        String number = values.getAsString("number");
        String[] emails = null;
        String newTag = values.getAsString("newTag");
        String newNumber = values.getAsString("newNumber");
        Integer idInt = values.getAsInteger("index");
        int index = 0;
        if (idInt != null) {
            index = idInt.intValue();
        }
        logi("update: index=" + index);
        int result = 0;
        if (UPB == match || UPB_SUB == match) {
            String strAnr = values.getAsString("newAnr");
            String strEmail = values.getAsString("newEmails");

            Integer aasIndex = values.getAsInteger("aas");
            String sne = values.getAsString("sne");
            if (newNumber == null) {
                newNumber = "";
            }
            if (newTag == null) {
                newTag = "";
            }
            MtkAdnRecord record = new MtkAdnRecord(efType, 0, newTag, newNumber);
            record.setAnr(strAnr);
            if (values.containsKey("newAnr2")) {
                String strAnr2 = values.getAsString("newAnr2");
                if (DBG) log("update newAnr2: " + strAnr2);
                record.setAnr(strAnr2, 1);
            }
            if (values.containsKey("newAnr3")) {
                String strAnr3 = values.getAsString("newAnr3");
                if (DBG) log("update newAnr3: " + strAnr3);
                record.setAnr(strAnr3, 2);
            }
            if (strEmail != null && !strEmail.equals("")) {
                emails = new String[1];
                emails[0] = strEmail;
            }
            record.setEmails(emails);
            if (aasIndex != null) {
                record.setAasIndex(aasIndex);
            }
            if (sne != null) {
                record.setSne(sne);
            }
            if (index > 0) {
                if (ADDRESS_BOOK_COLUMN_NAMES.length >= ADDRESS_SUPPORT_AAS) {
                    logi("updateUsimPBRecordsByIndexWithError");
                    result = updateUsimPBRecordsByIndexWithError(efType, record, index, subId);
                } else {
                    logi("updateUsimRecordInEfByIndex");
                    result = updateUsimRecordInEfByIndex(efType, index, newTag, newNumber, strAnr,
                            strEmail, subId);
                }
            } else {
                if (ADDRESS_BOOK_COLUMN_NAMES.length >= ADDRESS_SUPPORT_AAS) {
                    logi("updateUsimPBRecordsBySearchWithError");
                    result = updateUsimPBRecordsBySearchWithError(efType, new MtkAdnRecord(tag,
                            number, ""), record, subId);
                } else {
                    logi("updateUsimRecordInEf");
                    result = updateUsimRecordInEf(efType, tag, number, newTag, newNumber, strAnr,
                            strEmail, subId);
                }
            }
        } else {
            if (index > 0) {
                logi("updateIccRecordInEfByIndex");
                result = updateIccRecordInEfByIndex(efType, index, newTag, newNumber, pin2, subId);
            } else {
                logi("updateIccRecordInEf");
                result = updateIccRecordInEf(efType, tag, number, newTag, newNumber, pin2, subId);
            }
        }

        logi("update result = " + result);
        return result;
    }

    private int
    addIccRecordToEf(int efType, String name, String number, String[] emails,
            String pin2, int subId) {
        if (DBG) log("addIccRecordToEf: efType=0x" + Integer.toHexString(efType).toUpperCase() +
                ", name=" + getMaskString(name) + ", number=" + getMaskString(number) +
                ", emails=" + ((emails == null) ? "null" : getMaskString(emails[0])) +
                ", subscription=" + subId);

        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();
            if (iccIpb != null) {
                result = iccIpb.updateAdnRecordsInEfBySearchWithError(subId, efType,
                        "", "", name, number, pin2);
            }
        } catch (RemoteException ex) {
            if (DBG) log(ex.toString());
        } catch (SecurityException ex) {
            if (DBG) log(ex.toString());
        }
        if (DBG) log("addIccRecordToEf: " + result);
        return result;
    }

    private int addUsimRecordToEf(int efType, String name, String number, String strAnr,
            String strEmail, String strGas, int subId) {

        if (DBG) {
            log("addUSIMRecordToEf: efType=" + efType + ", name=" + getMaskString(name) +
                    ", number=" + getMaskString(number) + ", anr =" + getMaskString(strAnr) +
                    ", emails=" + getMaskString(strEmail) + ", subId=" + subId);
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        String[] emails = null;
        if (strEmail != null && !strEmail.equals("")) {
            emails = new String[1];
            emails[0] = strEmail;
        }

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsInEfBySearchWithError(subId, efType,
                        "", "", "", null, null, name, number, strAnr, null, emails);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("addUsimRecordToEf: " + result);
        return result;
    }
    private int
    updateIccRecordInEf(int efType, String oldName, String oldNumber,
            String newName, String newNumber, String pin2, int subId) {
        if (DBG) log("updateIccRecordInEf: efType=0x" + Integer.toHexString(efType).toUpperCase() +
                ", oldname=" + getMaskString(oldName) + ", oldnumber=" + getMaskString(oldNumber) +
                ", newname=" + getMaskString(newName) + ", newnumber=" + getMaskString(newNumber) +
                ", subscription=" + subId);

        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();
            if (iccIpb != null) {
                result = iccIpb.updateAdnRecordsInEfBySearchWithError(subId, efType, oldName,
                        oldNumber, newName, newNumber, pin2);
            }
        } catch (RemoteException ex) {
            if (DBG) log(ex.toString());
        } catch (SecurityException ex) {
            if (DBG) log(ex.toString());
        }
        if (DBG) log("updateIccRecordInEf: " + result);
        return result;
    }

    private int updateIccRecordInEfByIndex(int efType, int nIndex, String newName,
            String newNumber, String pin2, int subId) {
        if (DBG) {
            log("updateIccRecordInEfByIndex: efType=" + efType + ", index=" + nIndex +
                    ", newname=" + getMaskString(newName) +
                    ", newnumber=" + getMaskString(newNumber));
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateAdnRecordsInEfByIndexWithError(subId, efType,
                        newName, newNumber, nIndex, pin2);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("updateIccRecordInEfByIndex: " + result);
        return result;
    }

    private int updateUsimRecordInEf(int efType, String oldName, String oldNumber,
            String newName, String newNumber, String strAnr, String strEmail, int subId) {

        if (DBG) {
            log("updateUsimRecordInEf: efType=" + efType +
                    ", oldname=" + getMaskString(oldName) +
                    ", oldnumber=" + getMaskString(oldNumber) +
                    ", newname=" + getMaskString(newName) +
                    ", newnumber=" + getMaskString(newNumber) +
                    ", anr =" + getMaskString(strAnr) +
                    ", emails=" + getMaskString(strEmail));
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        String[] emails = null;
        if (strEmail != null) {
            emails = new String[1];
            emails[0] = strEmail;
        }

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsInEfBySearchWithError(subId, efType,
                        oldName, oldNumber, "", null, null, newName, newNumber, strAnr, null,
                        emails);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("updateUsimRecordInEf: " + result);
        return result;
    }

    private int updateUsimRecordInEfByIndex(int efType, int nIndex, String newName,
            String newNumber,
            String strAnr, String strEmail, int subId) {

        if (DBG) {
            log("updateUsimRecordInEfByIndex: efType=" + efType + ", Index=" + nIndex +
                    ", newname=" + getMaskString(newName) +
                    ", newnumber=" + getMaskString(newNumber) +
                    ", anr =" + getMaskString(strAnr) + ", emails=" + getMaskString(strEmail));
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        String[] emails = null;
        if (strEmail != null) {
            emails = new String[1];
            emails[0] = strEmail;
        }

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsInEfByIndexWithError(subId, efType,
                        newName, newNumber, strAnr, null, emails, nIndex);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("updateUsimRecordInEfByIndex: " + result);
        return result;
    }

    private int deleteIccRecordFromEf(int efType, String name, String number, String[] emails,
            String pin2, int subId) {
        if (DBG) log("deleteIccRecordFromEf: efType=0x" +
                Integer.toHexString(efType).toUpperCase() +
                ", name=" + getMaskString(name) + ", number=" + getMaskString(number) +
                ", pin2=" + getMaskString(pin2) + ", subscription=" + subId);

        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();
            if (iccIpb != null) {
                result = iccIpb.updateAdnRecordsInEfBySearchWithError(subId, efType,
                          name, number, "", "", pin2);
            }
        } catch (RemoteException ex) {
            if (DBG) log(ex.toString());
        } catch (SecurityException ex) {
            if (DBG) log(ex.toString());
        }
        if (DBG) log("deleteIccRecordFromEf: " + result);
        return result;
    }

    private int deleteIccRecordFromEfByIndex(int efType, int nIndex, String pin2, int subId) {
        if (DBG) {
            log("deleteIccRecordFromEfByIndex: efType=" + efType +
                    ", index=" + nIndex + ", pin2=" + getMaskString(pin2));
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.
                      updateAdnRecordsInEfByIndexWithError(subId, efType, "", "", nIndex, pin2);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("deleteIccRecordFromEfByIndex: " + result);
        return result;
    }

    private int deleteUsimRecordFromEf(int efType, String name,
            String number, String[] emails, int subId) {
        if (DBG) {
            log("deleteUsimRecordFromEf: efType=" + efType +
                    ", name=" + getMaskString(name) + ", number=" + getMaskString(number));
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsInEfBySearchWithError(subId, efType,
                        name, number, "", null, null, "", "", "", null, null);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("deleteUsimRecordFromEf: " + result);
        return result;
    }

    private int deleteUsimRecordFromEfByIndex(int efType, int nIndex, int subId) {
        if (DBG) {
            log("deleteUsimRecordFromEfByIndex: efType=" + efType + ", index=" + nIndex);
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsInEfByIndexWithError(subId, efType,
                        "", "", "", null, null, nIndex);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("deleteUsimRecordFromEfByIndex: " + result);
        return result;
    }

    /**
     * Loads an AdnRecord into a MatrixCursor. Must be called with mLock held.
     *
     * @param record the ADN record to load from
     * @param cursor the cursor to receive the results
     */
    private void loadRecord(MtkAdnRecord record, MatrixCursor cursor, int id) {
        int len = ADDRESS_BOOK_COLUMN_NAMES.length;
        if (!record.isEmpty()) {
            Object[] contact = new Object[len];
            String alphaTag = record.getAlphaTag();
            String number = record.getNumber();
            String[] emails = record.getEmails();
            String anr = null;
            String grpIds = record.getGrpIds();
            String index = Integer.toString(record.getRecId());

            if (len >= ADDRESS_SUPPORT_AAS) {
                int aasIndex = record.getAasIndex();
                contact[7] = aasIndex;
            }
            if (len >= ADDRESS_SUPPORT_SNE) {
                String sne = record.getSne();
                contact[8] = sne;
            }
            if (DBG) {
                log("loadRecord: record:" + record);
            }
            contact[0] = index;
            contact[1] = alphaTag;
            contact[2] = number;

            // ALPS01977595 - KOR MVNO Operator Support (For AOSP)
            if (SystemProperties.get("ro.vendor.mtk_kor_customization").equals("1")) {
                if (alphaTag.length() >= 2 && alphaTag.charAt(0) == '\uFEFE') {
                    int     ucslen = 0;
                    String  strKSC = "";

                    try {
                        byte[] inData = alphaTag.substring(1).getBytes("utf-16be");

                        strKSC = new String(inData , "KSC5601");
                    } catch (UnsupportedEncodingException ex) {
                        if (DBG) log("Implausible UnsupportedEncodingException : " + ex);
                    }

                    if (strKSC != null) {
                        ucslen = strKSC.length();
                        while (ucslen > 0 && strKSC.charAt(ucslen - 1) == '\uF8F7') {
                            ucslen--;
                        }

                        contact[1] = strKSC.substring(0, ucslen);
                        if (DBG) log("Decode ADN using KSC5601 : " + contact[1]);
                    }
                }
            }

            if (emails != null) {
                StringBuilder emailString = new StringBuilder();
                for (String email: emails) {
                    emailString.append(email);
                    emailString.append(",");
                }
                contact[3] = emailString.toString();
            }
            contact[4] = record.getAdditionalNumber();
            contact[5] = grpIds;
            contact[6] = id;
            cursor.addRow(contact);
        }
    }

    private void log(String msg) {
        Rlog.d(TAG, msg);
    }

    private void logi(String msg) {
        Rlog.i(TAG, msg);
    }

    private IMtkIccPhoneBook getIccPhbService() {
        IMtkIccPhoneBook iccIpb = IMtkIccPhoneBook.Stub.asInterface(
                ServiceManager.getService("mtksimphonebook"));

        return iccIpb;
    }
    private int getRequestSubId(Uri url) {
        if (DBG) log("getRequestSubId url: " + url);

        try {
            return Integer.parseInt(url.getLastPathSegment());
        } catch (NumberFormatException ex) {
            throw new IllegalArgumentException("Unknown URL " + url);
        }
    }

    private int updateUsimPBRecordsBySearchWithError(int efType, MtkAdnRecord oldAdn,
            MtkAdnRecord newAdn, int subId) {
        if (DBG) {
            log("updateUsimPBRecordsBySearchWithError subId:" + subId + ",oldAdn:" + oldAdn
                 + ",newAdn:" + newAdn);
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsBySearchWithError(subId, efType, oldAdn, newAdn);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("updateUsimPBRecordsBySearchWithError: " + result);
        return result;
    }

    private int updateUsimPBRecordsByIndexWithError(int efType, MtkAdnRecord newAdn, int index,
            int subId) {
        if (DBG) {
            log("updateUsimPBRecordsByIndexWithError subId:" + subId + ",index:" + index
                    + ",newAdn:" + newAdn);
        }
        int result = ERROR_ICC_PROVIDER_UNKNOWN;

        try {
            IMtkIccPhoneBook iccIpb = getIccPhbService();

            if (iccIpb != null) {
                result = iccIpb.updateUsimPBRecordsByIndexWithError(subId, efType, newAdn, index);
            }
        } catch (RemoteException ex) {
            log(ex.toString());
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("updateUsimPBRecordsByIndexWithError: " + result);
        return result;
    }

    /// M: CSIM PHB handling @{
    // Update csim phb storage for modem3
    private void updatePhbStorageInfo(int update, int subId) {
        boolean res = false;
        try {
            int phoneId = SubscriptionManager.getPhoneId(subId);
            Phone phone = PhoneFactory.getPhone(phoneId);
            if (phone != null) {
                IccFileHandler mFh = phone.getIccFileHandler();
                if (!CsimPhbUtil.hasModemPhbEnhanceCapability(mFh)) {
                    res = CsimPhbUtil.updatePhbStorageInfo(update);
                } else {
                    log("[updatePhbStorageInfo] is not a csim card");
                    res = false;
                }
            }
        } catch (SecurityException ex) {
            log(ex.toString());
        }
        log("[updatePhbStorageInfo] res = " + res);
    }
    /// @}

    private String getMaskString(String str) {
        if (str == null) {
            return "null";
        } else if (str.length() > 2) {
            return str.substring(0, str.length() >> 1) + "xxxxx";
        } else {
            return "xx";
        }
    }
}
