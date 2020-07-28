package com.mediatek.providers.contacts.unittest;

import android.accounts.Account;
import android.content.ContentUris;
import android.content.ContentValues;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;

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

public class ModifyPhoneContactTest extends BaseContactsProvider2Test {

    public static final String ACCOUNT_NAME_LOCAL_PHONE = "Phone";
    public static final String ACCOUNT_TYPE_LOCAL_PHONE = "Local Phone Account";

    private static final String TAG = ModifyPhoneContactTest.class.getSimpleName();
    Account mAccount = new Account(ACCOUNT_NAME_LOCAL_PHONE,
                                   ACCOUNT_TYPE_LOCAL_PHONE);

    /**
     *<Purpose>
     *Can insert phone contact successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one contact, contains name and number
     *
     *[Expected result]
     *Can save Contact successful
    */
    public void testcase01_insertContact() {
        // insert raw contact
        long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert name
        String name = "Android";
        Uri uri =  DataUtil.insertStructuredName(mResolver,rawContactId,name,null);
        long dataNameId = ContentUris.parseId(uri);
        // insert number
        String number = "123456789";
        uri = insertPhoneNumber(rawContactId, number);
        long dataNumberId = ContentUris.parseId(uri);

        // ----------- start to check the data -----------//
        // assert contact inserted and the name is right
        long contactId = queryContactId(rawContactId);
        assertEquals(name, queryDisplayName(contactId));

        // assert phone_lookup
        ContentValues values = new ContentValues();
        values.put(PhoneLookupColumns.DATA_ID, dataNumberId);
        assertStoredValuesDb("SELECT data_id FROM phone_lookup WHERE normalized_number LIKE '"
                + number + "'" + " AND raw_contact_id = '" + rawContactId + "'", null, values);
        values.clear();

        // assert name_lookup
        values.put(NameLookupColumns.DATA_ID, dataNameId);
        assertStoredValuesDb("SELECT data_id FROM name_lookup WHERE normalized_name GLOB '"
                + NameNormalizer.normalize(name) + "' AND raw_contact_id = '" + rawContactId + "'",
                null, new ContentValues[] { values, values });
        values.clear();

        // assert search_index
        values.put(SearchIndexColumns.CONTACT_ID, contactId);
        assertStoredValuesDb("SELECT contact_id FROM search_index WHERE name MATCH '"
                + NameNormalizer.normalize(name) + "' AND tokens = '" + number + "'", null, values);
        values.clear();
    }

    /**
     *<Purpose>
     *Can update phone contact successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one contact, contains name and number
     *2. update it's name and number
     *
     *[Expected result]
     *Can update Contact successful
    */
    public void testcase02_updateContact() {
        // insert raw contact
        long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert name
        String oldName = "Android";
        Uri uri =  DataUtil.insertStructuredName(mResolver,rawContactId,oldName,null);
        long dataNameId = ContentUris.parseId(uri);
        // insert number
        String oldNumber = "123456789";
        uri = insertPhoneNumber(rawContactId, oldNumber);
        long dataNumberId = ContentUris.parseId(uri);

        // update name
        // first delete the old data, then insert the new data, update at end.
        // 1. delete the old name
        DataUtil.delete(mResolver, dataNameId);
        // 2. insert the new name
        uri = DataUtil.insertStructuredName(mResolver, rawContactId, "AAA", null);
        long newDataNameId = ContentUris.parseId(uri);
        // 3. update
        String newName = "Test";
        ContentValues values = new ContentValues();
        values.put(StructuredName.DISPLAY_NAME, newName);
        DataUtil.update(mResolver, newDataNameId, values);

        // update number
        // first delete the old data, then insert the new data, update at end.
        // 1. delete the old number
        DataUtil.delete(mResolver, dataNumberId);
        // 2. insert the new number
        uri = insertPhoneNumber(rawContactId, "11111111");
        long newDataNumberId = ContentUris.parseId(uri);
        // 3. update
        String newNumber = "1122334455";
        values.clear();
        values.put(Phone.NUMBER, newNumber);
        DataUtil.update(mResolver, newDataNumberId, values);

        // ----------- start to check the data -----------//

        // assert phone_lookup
        values.clear();
        values.put(PhoneLookupColumns.DATA_ID, newDataNumberId);
        assertStoredValuesDb("SELECT data_id FROM phone_lookup WHERE (normalized_number LIKE '"
                + newNumber + "' OR normalized_number LIKE '" + oldNumber
                + "') AND raw_contact_id = '" + rawContactId + "'", null, values);
        values.clear();

        // assert name_lookup
        values.put(NameLookupColumns.DATA_ID, newDataNameId);
        assertStoredValuesDb("SELECT data_id FROM name_lookup WHERE (normalized_name GLOB '"
                + NameNormalizer.normalize(newName) + "' OR normalized_name GLOB '"
                + NameNormalizer.normalize(oldName) + "') AND raw_contact_id = " + rawContactId,
                null, new ContentValues[] { values, values });
        values.clear();

        // assert search_index
        long contactId = queryContactId(rawContactId);
        values.put(SearchIndexColumns.TOKENS, newNumber);
        assertStoredValuesDb("SELECT tokens FROM search_index WHERE contact_id = " + contactId
                + " AND (name MATCH '" + NameNormalizer.normalize(newName) + "')", null, values);
        values.clear();
    }


    /**
     *<Purpose>
     *Can delete phone contact successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one contact, contains name and number
     *2. delete it, test all the delete ways, ex. by raw_contact_id, data_id, contact_id...
     *
     *[Expected result]
     *Can delete Contact successful
    */
    public void testcase03_deleteContact() {
        // insert raw contact
        long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        long contactId = queryContactId(rawContactId);
        // insert name
        String name = "Android";
        Uri uri = DataUtil.insertStructuredName(mResolver, rawContactId, name, null);
        long dataNameId = ContentUris.parseId(uri);

        // insert number
        String number = "123456789";
        uri = insertPhoneNumber(rawContactId, number);
        long dataNumberId = ContentUris.parseId(uri);

        ContactUtil.deleteInBatch(mResolver, contactId);

        // ----------- start to check the data -----------//

        // assert contact table
        assertStoredValuesDb("SELECT * FROM contacts WHERE _id = " + contactId, null);

        // assert data table
        assertStoredValuesDb("SELECT * FROM data WHERE _id IN (" + dataNameId + ", " + dataNumberId
                + ")", null);

        // assert phone_lookup, should be null
        assertStoredValuesDb("SELECT * FROM phone_lookup WHERE data_id = '" + dataNumberId + "'",
                null);

        // assert name_lookup, should be null
        assertStoredValuesDb("SELECT * FROM name_lookup WHERE data_id = '" + dataNameId + "'", null);

        // assert search_index
        assertStoredValuesDb("SELECT * FROM search_index WHERE contact_id = '" + contactId + "'",
                null);
    }



    /**
     *<Purpose>
     *Can insert phone group successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one group
     *
     *[Expected result]
     *Can save Contact successful
    */
    public void testcase04_insertGroup() {
        final long groupId = createGroup(mAccount, null, "group");

        // ----------- start to check the data -----------//

        // assert the group has been saved.
        ContentValues values = new ContentValues();
        values.put(Groups._ID, groupId);
        assertStoredValuesDb("SELECT _id FROM groups WHERE _id = " + groupId, null, values);
    }

    /**
     *<Purpose>
     *Can move contacts into group successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one group
     *2. update group name
     *
     *[Expected result]
     *Can update group name successful
    */
    public void testcase05_updateGroup() {
        final long groupId = createGroup(mAccount, null, "group");
        String newTitle = "newGroupTitle";
        ContentValues values = new ContentValues();
        values.put(Groups.TITLE, newTitle);
        mResolver.update(ContentUris.withAppendedId(
                Groups.CONTENT_URI, groupId), values, null, null);

        // ----------- start to check the data -----------//

        // assert the group name.
        assertStoredValuesDb("SELECT title FROM groups WHERE _id = " + groupId, null, values);

    }

    /**
     *<Purpose>
     *Can insert contacts which contains group info
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one group
     *2. insert one contact, contains name and number and group info
     *
     *[Expected result]
     *Contacts group info saved successful
    */
    public void testcase06_insertContactWithGroup() {
        final long groupId = createGroup(mAccount, null, "group");
        final long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert name
        DataUtil.insertStructuredName(mResolver, rawContactId, "test06", null);
        // insert number
        insertPhoneNumber(rawContactId, "1008611");
        // insert group info
        insertGroupMembership(rawContactId, groupId);

        // ----------- start to check the data -----------//

        // assert the contact has been looped into the group.
        ContentValues values = new ContentValues();
        values.put(Data.RAW_CONTACT_ID, rawContactId);
        assertStoredValuesDb("SELECT raw_contact_id FROM data WHERE "
                + GroupMembership.GROUP_ROW_ID + " = " + groupId, null, values);
    }

    /**
     *<Purpose>
     *Can remove contacts from group successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one contact, contains name and number
     *2. insert one group
     *3. move the contact into that group
     *4. remove the contact from that group
     *
     *[Expected result]
     *Can remove contacts from group successful
    */
    public void testcase07_removeContactFromGroup() {
        final long groupId1 = createGroup(mAccount, null, "group1");
        final long groupId2 = createGroup(mAccount, null, "group2");
        final long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert contact into 2 groups
        insertGroupMembership(rawContactId, groupId1);
        insertGroupMembership(rawContactId, groupId2);
        // just remove contact from 1 group
        removeGroupMemberships(groupId1, rawContactId);

        // ----------- start to check the data -----------//

        // assert the contact group info.
        assertStoredValuesDb("SELECT raw_contact_id FROM data WHERE "
                + GroupMembership.GROUP_ROW_ID + " = " + groupId1, null);
        ContentValues values = new ContentValues();
        values.put(Data.RAW_CONTACT_ID, rawContactId);
        assertStoredValuesDb("SELECT raw_contact_id FROM data WHERE "
                + GroupMembership.GROUP_ROW_ID + " = " + groupId2, null, values);
    }


    /**
     *<Purpose>
     *Can delete group successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one contact, contains name and number
     *2. insert one group
     *3. move the contact into that group
     *4. delete the group
     *
     *[Expected result]
     *The group deleted and the contact group info reset to null
    */
    public void testcase08_deleteGroup() {
        final long groupId = createGroup(mAccount, null, "group");
        final long rawContactId = RawContactUtil.createRawContact(mResolver, mAccount);
        // insert contact into group
        insertGroupMembership(rawContactId, groupId);
        // delete the group
        deleteGroup(groupId);

        // ----------- start to check the data -----------//

        // assert the group deleted.
        assertStoredValuesDb("SELECT _id FROM groups WHERE _id = " + groupId + " AND "
                + Groups.DELETED + " = 0", null);
        // assert the contact group info deleted
        assertStoredValuesDb("SELECT raw_contact_id FROM data WHERE  "
                + GroupMembership.GROUP_ROW_ID + " = " + groupId, null);
    }

    /**
     *<Purpose>
     *Can insert other data successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert one contact, contains name and Mobile Number.
     *2. insert Phonetic name into that contact,
     *3. insert Nickname into that contact,
     *4. insert home Phone Number into that contact,
     *5. insert Email into that contact,
     *6. insert Street into that contact,
     *7. insert PO box into that contact,
     *8. insert City and state into that contact,
     *9. insert Company into that contact,
     *10. insert Internet call into that contact,
     *11. insert Website into that contact,
     *12. insert IMS call into that contact.
     *
     *[Expected result]
     *Can save all data successful
    */
    public void testcase09_insertContactWithOtherData() {

    }

    private void removeGroupMemberships(long groupId, long rawContactId) {
        mResolver.delete(Data.CONTENT_URI, Data.MIMETYPE + "=? AND "
                + GroupMembership.RAW_CONTACT_ID + "=? AND " + GroupMembership.DATA1 + "=?",
                new String[] { GroupMembership.CONTENT_ITEM_TYPE, String.valueOf(rawContactId),
                        String.valueOf(groupId) });
    }

    private void deleteGroup(long groupId){
        mResolver.delete(Groups.CONTENT_URI, Groups._ID + "=?",
                new String[] { String.valueOf(groupId) });
    }
}
