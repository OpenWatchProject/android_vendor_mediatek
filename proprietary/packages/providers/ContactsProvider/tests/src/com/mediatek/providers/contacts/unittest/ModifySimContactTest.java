package com.mediatek.providers.contacts.unittest;

import java.util.List;

import android.accounts.Account;
import android.content.ContentUris;
import android.content.ContentValues;
import android.net.Uri;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.RawContacts;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.providers.contacts.BaseContactsProvider2Test;
import com.android.providers.contacts.ContactsDatabaseHelper.NameLookupColumns;
import com.android.providers.contacts.ContactsDatabaseHelper.PhoneLookupColumns;
import com.android.providers.contacts.ContactsDatabaseHelper.SearchIndexColumns;
import com.android.providers.contacts.ContactsProvider2;
import com.android.providers.contacts.HanziToPinyin;
import com.android.providers.contacts.NameNormalizer;
import com.android.providers.contacts.testutil.ContactUtil;
import com.android.providers.contacts.testutil.DataUtil;
import com.android.providers.contacts.testutil.RawContactUtil;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.provider.MtkContactsContract;

public class ModifySimContactTest extends BaseContactsProvider2Test {

    public static final String ACCOUNT_TYPE_SIM = "SIM Account";

    private static final String TAG = ModifySimContactTest.class.getSimpleName();

    String mName = "Test";
    String mNumber = "1111111111";
    Account mAccount;
    int mSubId = -1;
    long mContactId = -1;
    long mRawContactId = -1;
    long mNameId = -1;
    long mNumberId = -1;

    /**
     * <Purpose> Can insert SIM contact successful
     *
     * [Initial condition] no
     *
     * [Step] 1. insert one SIM contact, contains name and number
     *
     * [Expected result] Can save Contact successful
     */
    public void testcase01_insertContact() {
        insertSimContactToDb();

        // ----------- start to check the data -----------//

        // assert contact inserted and the name is right
        long contactId = queryContactId(mRawContactId);
        assertEquals(mName, queryDisplayName(contactId));

        // assert phone_lookup
        ContentValues values = new ContentValues();
        values.put(PhoneLookupColumns.DATA_ID, mNumberId);
        assertStoredValuesDb("SELECT data_id FROM phone_lookup WHERE normalized_number LIKE '"
                + mNumber + "'" + " AND raw_contact_id = '" + mRawContactId + "'", null, values);
        values.clear();

        // assert name_lookup
        values.put(NameLookupColumns.DATA_ID, mNameId);
        assertStoredValuesDb("SELECT data_id FROM name_lookup WHERE normalized_name GLOB '"
                + NameNormalizer.normalize(mName) + "' AND raw_contact_id = '" + mRawContactId
                + "'", null, new ContentValues[] { values, values });
        values.clear();

        // assert search_index
        values.put(SearchIndexColumns.CONTACT_ID, contactId);
        assertStoredValuesDb("SELECT contact_id FROM search_index WHERE name MATCH '"
                + NameNormalizer.normalize(mName) + "' AND tokens = '" + mNumber + "'", null,
                values);
        values.clear();

        // assert SIM info
        values.put(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM, mSubId);
        values.put(RawContacts.AGGREGATION_MODE, RawContacts.AGGREGATION_MODE_DISABLED);
        values.put(MtkContactsContract.RawContactsColumns.INDEX_IN_SIM, 1);
        assertStoredValuesDb("SELECT indicate_phone_or_sim_contact,aggregation_mode,"
                + "index_in_sim FROM raw_contacts WHERE _id = '" + mRawContactId + "'", null,
                new ContentValues[] { values });
        values.clear();
    }

    /**
     * <Purpose> Can update SIM contact successful
     *
     * [Initial condition] no
     *
     * [Step] 1. insert one SIM contact, contains name and number 2. update it's
     * name and number
     *
     * [Expected result] Can update Contact successful
     */
    public void testcase02_updateContact() {
        insertSimContactToDb();
        // update name
        String newName = "Test";
        ContentValues values = new ContentValues();
        values.put(StructuredName.DISPLAY_NAME, newName);
        DataUtil.update(mResolver, mNameId, values);

        // update number
        String newNumber = "1111111111";
        values.clear();
        values.put(Phone.NUMBER, newNumber);
        DataUtil.update(mResolver, mNumberId, values);

        // ----------- start to check the data -----------//

        // assert phone_lookup
        values.clear();
        values.put(PhoneLookupColumns.DATA_ID, mNumberId);
        assertStoredValuesDb("SELECT data_id FROM phone_lookup WHERE (normalized_number LIKE '"
                + newNumber + "' OR normalized_number LIKE '" + mNumber
                + "') AND raw_contact_id = '" + mRawContactId + "'", null, values);
        values.clear();

        // assert name_lookup
        values.put(NameLookupColumns.DATA_ID, mNameId);
        assertStoredValuesDb("SELECT data_id FROM name_lookup WHERE (normalized_name GLOB '"
                + NameNormalizer.normalize(newName) + "' OR normalized_name GLOB '"
                + NameNormalizer.normalize(mName) + "') AND raw_contact_id = " + mRawContactId,
                null, new ContentValues[] { values, values });
        values.clear();

        // assert search_index
        long contactId = queryContactId(mRawContactId);
        values.put(SearchIndexColumns.TOKENS, newNumber);
        assertStoredValuesDb("SELECT tokens FROM search_index WHERE contact_id = " + contactId
                + " AND (name MATCH '" + NameNormalizer.normalize(newName) + "')", null, values);
        values.clear();
    }

    /**
     * <Purpose> Can delete SIM contact successful
     *
     * [Initial condition] no
     *
     * [Step] 1. insert one SIM contact, contains name and number 2. delete it,
     * test all the delete ways, ex. by raw_contact_id, data_id, contact_id...
     *
     * [Expected result] Can delete Contact successful
     */
    public void testcase03_deleteContact() {
        // insert one SIM contact
        insertSimContactToDb();

        // delete it

        ContactUtil.deleteInBatch(mResolver, mContactId);

        // ----------- start to check the data -----------//

        // assert contact table
        assertStoredValuesDb("SELECT * FROM contacts WHERE _id = " + mContactId, null);

        // assert raw_contact table
        // match ALPS02322033
        assertStoredValuesDb("SELECT * FROM raw_contacts WHERE _id = " + mRawContactId, null);

        // assert data table
        assertStoredValuesDb(
                "SELECT * FROM data WHERE _id IN (" + mNameId + ", " + mNumberId + ")", null);

        // assert phone_lookup, should be null
        assertStoredValuesDb("SELECT * FROM phone_lookup WHERE data_id = '" + mNumberId + "'", null);

        // assert name_lookup, should be null
        assertStoredValuesDb("SELECT * FROM name_lookup WHERE data_id = '" + mNameId + "'", null);

        // assert search_index
        assertStoredValuesDb("SELECT * FROM search_index WHERE contact_id = '" + mContactId + "'",
                null);
    }

    private boolean insertSimContactToDb() {
        return insertSimContactToDb(false, mName, mNumber);
    }

    private boolean insertSimContactToDb(boolean useVailSub, String name, String number) {
        int subId = -1;
        if (useVailSub) {
            // find valid sub
            List<SubscriptionInfo> subscriptionInfoList = SubscriptionManager.from(mContext)
                    .getActiveSubscriptionInfoList();
            if (subscriptionInfoList != null && subscriptionInfoList.size() > 0) {
                for (SubscriptionInfo subscriptionInfo : subscriptionInfoList) {
                    if (isPhoneBookReady(subscriptionInfo.getSubscriptionId())) {
                        subId = subscriptionInfo.getSubscriptionId();
                        break;
                    }
                }
            }
            if (subId == -1) {
                Log.d(TAG, "insertSimContactToDb has no valid sub");
                return false;
            }
        } else {
            subId = 1;
        }
        mSubId = subId;
        mAccount = new Account("SIM" + subId, ACCOUNT_TYPE_SIM);
        // imitate Contacts AP to insert a SIM contact
        ContentValues contactvalues = new ContentValues();
        contactvalues.put(RawContacts.ACCOUNT_NAME, mAccount.name);
        contactvalues.put(RawContacts.ACCOUNT_TYPE, mAccount.type);
        contactvalues.put(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM, subId);
        contactvalues.put(RawContacts.AGGREGATION_MODE, RawContacts.AGGREGATION_MODE_DISABLED);
        contactvalues.put(MtkContactsContract.RawContactsColumns.INDEX_IN_SIM, 1); // index in SIM
        mRawContactId = RawContactUtil.createRawContact(mResolver, mAccount, contactvalues);
        mContactId = queryContactId(mRawContactId);
        assertTrue(mRawContactId > 0);
        // insert name
        Uri uri = DataUtil.insertStructuredName(mResolver, mRawContactId, name, null);
        mNameId = ContentUris.parseId(uri);
        assertTrue(mNameId > 0);
        // insert number
        uri = insertPhoneNumber(mRawContactId, number);
        mNumberId = ContentUris.parseId(uri);
        assertTrue(mNumberId > 0);
        return true;
    }

    private boolean isPhoneBookReady(int subId) {
        final IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));

        if (null == telephonyEx) {
            Log.w(TAG, "[isPhoneBookReady]phoneEx == null");
            return false;
        }

        boolean isPbReady = false;
        try {
            isPbReady = telephonyEx.isPhbReady(subId);
        } catch (RemoteException e) {
            Log.e(TAG, "[isPhoneBookReady]catch exception:");
            e.printStackTrace();
        }

        Log.d(TAG, "[isPhoneBookReady]subId:" + subId + ", isPbReady:" + isPbReady);

        return isPbReady;
    }
}
