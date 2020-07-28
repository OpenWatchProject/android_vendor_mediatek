package com.mediatek.providers.contacts.unittest;

import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;

import com.android.providers.contacts.BaseContactsProvider2Test;
import com.android.providers.contacts.testutil.DataUtil;
import com.android.providers.contacts.testutil.RawContactUtil;

public class QueryContactTest extends BaseContactsProvider2Test {

    private static final String TAG = QueryContactTest.class.getSimpleName();

    /**
     *<Purpose>
     *Can query contact by name
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. Insert some contacts with valid name and number
     *2. Query contact from Contact APP by name
     *
     *[Expected result]
     *Can query Contact successful
    */
    public void testcase01_queryContactByName() {
        // insert raw contact
        long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert name
        DataUtil.insertStructuredName(mResolver, rawContactId, "Android", "Test", null , true);
        // insert number
        insertPhoneNumber(rawContactId, "123456789");
        /// Common use cases in Contacts APP
        long contactId = queryContactId(rawContactId);
        Uri filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("Android Test"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("A"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("T"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("a t"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("And"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("an Te"));
        assertStoredValue(filterUri, Contacts._ID, contactId);

        /// Common use cases in Mms
        filterUri = Uri.withAppendedPath(Phone.CONTENT_FILTER_URI, Uri.encode("A t"));
        assertStoredValue(filterUri, Data.CONTACT_ID, contactId);
        filterUri = Uri.withAppendedPath(
                Uri.withAppendedPath(Data.CONTENT_URI, "phone_email/filter"), Uri.encode("A t"));
        assertStoredValue(filterUri, Data.CONTACT_ID, contactId);

        //TODO Chinese situation can support more kinds of search way, imitate it and test it.
    }

    /**
     *<Purpose>
     *Can query contact by name
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. Insert some contacts with valid name and number
     *2. Query contact from Contact APP by number
     *
     *[Expected result]
     *Can query Contact successful
    */
    public void testcase02_queryContactByNumber() {
        // insert raw contact
        long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert number
        String number = "18980112211";
        insertPhoneNumber(rawContactId, number);
        long contactId = queryContactId(rawContactId);
        /// Common use cases in Contacts APP
        // 1. assert full match
        Uri filterUri = Uri
                .withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("18980112211"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        // 2. assert part match
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("1898"));
        assertStoredValue(filterUri, Contacts._ID, contactId);
        // 3. assert
        filterUri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode("1 8  98"));
        assertStoredValue(filterUri, Contacts._ID, contactId);

        /// Common use cases in Mms
        // 1. assert common case
        filterUri = Uri.withAppendedPath(Phone.CONTENT_FILTER_URI, Uri.encode("1 8 9"));
        assertStoredValue(filterUri, Data.CONTACT_ID, contactId);
        // 2. assert letters case
        filterUri = Uri.withAppendedPath(Phone.CONTENT_FILTER_URI, Uri.encode("1 t x"));
        assertStoredValue(filterUri, Data.CONTACT_ID, contactId);
        // 3. phone_email
        filterUri = Uri.withAppendedPath(
                Uri.withAppendedPath(Data.CONTENT_URI, "phone_email/filter"), Uri.encode("189 8"));
        assertStoredValue(filterUri, Data.CONTACT_ID, contactId);
    }
}
