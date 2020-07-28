package com.orangelabs.rcs.provider.eab;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Contacts.People;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.PhoneLookup;


import com.orangelabs.rcs.utils.logger.Logger;

/**
 * IP call history
 *
 * @author owom5460
 */
public class ContactsBackupHelper {

    private static final String MIMETYPE_RCS_CONTACT = "vnd.android.cursor.item/com.orangelabs.rcs.rcs-status";;
    private static final int RCS_CONTACT = 2;
    private static final int RCS_CAPABLE_CONTACT = 0;

    private static final String[] PROJECTION =
    {
            Phone.CONTACT_ID, Data.DATA1, Data.DATA2,
            Data.DISPLAY_NAME, Contacts.SORT_KEY_PRIMARY
    };

    private static final String SELECTION = Data.MIMETYPE + "=? AND " + "(" + Data.DATA2 + "=? OR " + Data.DATA2 + "=? )";
    private static final String[] SELECTION_ARGS = {
        MIMETYPE_RCS_CONTACT, Long.toString(RCS_CONTACT), Long.toString(RCS_CAPABLE_CONTACT)
    };
    public static final String SORT_ORDER = Contacts.SORT_KEY_PRIMARY;

    private static final String TAG = "ContactsBackupHelper";
    /**
     * Current instance
     */
    private static ContactsBackupHelper instance = null;

    /**
     * Content resolver
     */
    private ContentResolver cr;

    /**
     * Database URI
     */
    private static Uri databaseUri = ContactsBackupData.CONTENT_URI;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Create instance
     *
     * @param ctx Context
     */
    public static synchronized void createInstance(Context ctx) {
        if (instance == null) {
            instance = new ContactsBackupHelper(ctx);
        }
    }

    /**
     * Returns instance
     *
     * @return Instance
     */
    public static ContactsBackupHelper getInstance() {

        return instance;
    }

    /**
     * Constructor
     *
     * @param ctx Application context
     */
    private ContactsBackupHelper(Context ctx) {
        super();
        this.cr = ctx.getContentResolver();
    }

    /**
     * Add a new entry in the call history
     *
     * @param contact Remote contact
     * @param sessionId Session ID
     * @param direction Direction
     * @param audiocontent Audio content
     * @param videocontent Video content
     * @param status Call status
     */
    public void insertContacts(List<String> contacts) {
        if(logger.isActivated()){
            logger.debug("Inserting Contact");
        }
        deleteOldDb();
        ContentValues values = new ContentValues();
        values.put(ContactsBackupData.KEY_CONTACT_NUMBER, "farzi");
        values.put(ContactsBackupData.KEY_CONTACT_NAME, "23456");
        //values.put(ContactsBackupData.KEY_DIRECTION, direction);
        //values.put(ContactsBackupData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
        //values.put(ContactsBackupData.KEY_STATUS, status);


        cr.insert(databaseUri, values);
        if(logger.isActivated()){
            logger.debug(" Contact inserted");}
        for (int i=0; i < contacts.size(); i++) {
            if(logger.isActivated()){
                logger.debug(" Contact inserted0");
                }
            String contact = contacts.get(i);
        //    ContactsBackupHelper.InsertContact(contact);

        //contact = PhoneUtils.extractNumberFromUri(contact);
        //ContentValues values = new ContentValues();
        values.put(ContactsBackupData.KEY_CONTACT_NUMBER, contact);
        values.put(ContactsBackupData.KEY_CONTACT_NAME, contact);
        //values.put(ContactsBackupData.KEY_DIRECTION, direction);
        //values.put(ContactsBackupData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
        //values.put(ContactsBackupData.KEY_STATUS, status);


        cr.insert(databaseUri, values);
        if(logger.isActivated()){
            logger.debug(" Contact inserted");
            }
        }

    }


    /**
     * Add a new entry in the call history
     *
     * @param contact Remote contact
     * @param sessionId Session ID
     * @param direction Direction
     * @param audiocontent Audio content
     * @param videocontent Video content
     * @param status Call status
     */
    public void insertContactsFromNative(List<RcsBackupContact> contacts) {
        if(logger.isActivated()){
            logger.debug("Inserting Contact");
        }
        deleteOldDb();
        ContentValues values = new ContentValues();
    //    values.put(ContactsBackupData.KEY_CONTACT_NUMBER, "farzi");
    //    values.put(ContactsBackupData.KEY_CONTACT_NAME, "23456");
        //values.put(ContactsBackupData.KEY_DIRECTION, direction);
        //values.put(ContactsBackupData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
        //values.put(ContactsBackupData.KEY_STATUS, status);


    //    cr.insert(databaseUri, values);
        if(logger.isActivated()){
            logger.debug(" Contact inserted");}
        for (int i=0; i < contacts.size(); i++) {
            if(logger.isActivated()){
                logger.debug(" Contact inserted0");
                }
            RcsBackupContact contact = contacts.get(i);
        //    ContactsBackupHelper.InsertContact(contact);

        //contact = PhoneUtils.extractNumberFromUri(contact);
        //ContentValues values = new ContentValues();
        values.put(ContactsBackupData.KEY_CONTACT_NUMBER, contact.getNumber());
        values.put(ContactsBackupData.KEY_CONTACT_NAME, contact.getName());
        //values.put(ContactsBackupData.KEY_DIRECTION, direction);
        //values.put(ContactsBackupData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
        //values.put(ContactsBackupData.KEY_STATUS, status);


        cr.insert(databaseUri, values);
        if(logger.isActivated()){
            logger.debug(" Contact inserted");
            }
        }

    }

    /**
     * Update the call status
     *
     * @param sessionId Session ID
     * @param status New status
     */
    public void deleteOldDb() {
        if (logger.isActivated()) {
            logger.debug("delete call for backup contacts ");
        }

    //    ContentValues values = new ContentValues();
    //    values.put(ContactsBackupHelper.KEY_STATUS, status);
        cr.delete(databaseUri,null, null);
    }
    public void onContactsBackup(Context mContext) {
        logger.debug("Backup Contact querying native contact");
        Cursor cursor = null;
        cursor = mContext.getContentResolver().query(Data.CONTENT_URI, PROJECTION, SELECTION, SELECTION_ARGS, SORT_ORDER);
        List<RcsBackupContact> contacts = new ArrayList<RcsBackupContact>();
        if (cursor != null && cursor.getCount() != 0) {
            logger.debug( "onQueryComplete() cursor.getCount() is " + cursor.getCount());
            cursor.moveToFirst();
            do {
                String displayName = cursor.getString(cursor.getColumnIndex(Data.DISPLAY_NAME));
                String number = cursor.getString(cursor.getColumnIndex(Data.DATA1));
                String sortKeyString = cursor.getString(cursor
                        .getColumnIndex(Contacts.SORT_KEY_PRIMARY));
                logger.debug( "onLoadComplete(), number  = " + number );
                logger.debug( "onLoadComplete(), sortKeyString = " + sortKeyString);
                RcsBackupContact contact =
                        new RcsBackupContact(displayName, number);
                contacts.add(contact);
                logger.debug( "onLoadComplete, contacts is " + contacts);
            } while (cursor.moveToNext());
        } else {
            logger.debug("getbackup list is null, cursor is null!");
        }
        if(cursor!=null)
        {
                cursor.close();
                cursor = null;
            logger.debug("cursor closed");
        }
        insertContactsFromNative(contacts);
    }

      /**
     *
     * @param rawContactId the id of the rawContact
     * @param rcsNumber The RCS number
     * @return the id of the associated RCS rawContact
     */
    public List<RcsBackupContact> getContactFromDb(Context ctx) {
        //long result = -1;
        Cursor cursor = ctx.getContentResolver().query(databaseUri,
                new String[]{ContactsBackupData.KEY_CONTACT_NAME, ContactsBackupData.KEY_CONTACT_NUMBER},
                null,
                null,
                null);
         List<RcsBackupContact> contacts = new ArrayList<RcsBackupContact>();
         if (cursor != null && cursor.getCount() != 0) {
             logger.debug( "onQueryComplete() cursor.getCount() is " + cursor.getCount());
             cursor.moveToFirst();
             do {
                 String displayName = cursor.getString(cursor.getColumnIndex(ContactsBackupData.KEY_CONTACT_NAME));
                 String number = cursor.getString(cursor.getColumnIndex(ContactsBackupData.KEY_CONTACT_NUMBER));
                 logger.debug( "onLoadComplete(), number  = " + number );
                // logger.debug( "onLoadComplete(), sortKeyString = " + sortKeyString);
                 RcsBackupContact contact =
                         new RcsBackupContact(displayName, number);
                 contacts.add(contact);
                 logger.debug( "onLoadComplete, contacts is " + contacts);
             } while (cursor.moveToNext());
         } else {
             logger.debug("getbackup list is null, cursor is null!");
         }
         if(cursor!=null)
         {
                 cursor.close();
                 cursor = null;
             logger.debug("cursor closed");
         }
         return contacts;
    }

    public boolean contactExists(Context context, String number) {
        if (number != null) {
            Uri lookupUri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, Uri.encode(number));
            String[] mPhoneNumberProjection = { PhoneLookup._ID, PhoneLookup.NUMBER, PhoneLookup.DISPLAY_NAME };
            Cursor cur = context.getContentResolver().query(lookupUri, mPhoneNumberProjection, null, null, null);
            try {
                if (cur.moveToFirst()) {
                    if(logger.isActivated()){
                        logger.debug(" Contact inserted to native contact number "+number +"exists");
                        }
                    return true;
                }
                else {
                    if(logger.isActivated()){
                        logger.debug(" Contact inserted to native contact number "+number +"not exists");
                        }
                }
            } finally {
                if (cur != null)
                    cur.close();
            }
            return false;
        } else {
            return false;
        }
    }

    private void createContact(Context context, String name, String phone) {

         ArrayList < ContentProviderOperation > ops = new ArrayList < ContentProviderOperation > ();

         ops.add(ContentProviderOperation.newInsert(
         ContactsContract.RawContacts.CONTENT_URI)
             .withValue(ContactsContract.RawContacts.ACCOUNT_TYPE, null)
             .withValue(ContactsContract.RawContacts.ACCOUNT_NAME, null)
             .build());

         //------------------------------------------------------ Names
         if (name != null) {
             ops.add(ContentProviderOperation.newInsert(
             ContactsContract.Data.CONTENT_URI)
                 .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                 .withValue(ContactsContract.Data.MIMETYPE,
             ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)
                 .withValue(
             ContactsContract.CommonDataKinds.StructuredName.DISPLAY_NAME,
             name).build());
         }

         //------------------------------------------------------ Mobile Number
         if (phone != null) {
             ops.add(ContentProviderOperation.
             newInsert(ContactsContract.Data.CONTENT_URI)
                 .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                 .withValue(ContactsContract.Data.MIMETYPE,
             ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE)
                 .withValue(ContactsContract.CommonDataKinds.Phone.NUMBER, phone)
                 .withValue(ContactsContract.CommonDataKinds.Phone.TYPE,
             ContactsContract.CommonDataKinds.Phone.TYPE_MOBILE)
                 .build());
         }

         //------------------------------------------------------ Home Numbers
         /*if (HomeNumber != null) {
             ops.add(ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                 .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                 .withValue(ContactsContract.Data.MIMETYPE,
             ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE)
                 .withValue(ContactsContract.CommonDataKinds.Phone.NUMBER, HomeNumber)
                 .withValue(ContactsContract.CommonDataKinds.Phone.TYPE,
             ContactsContract.CommonDataKinds.Phone.TYPE_HOME)
                 .build());
         }

         //------------------------------------------------------ Work Numbers
         if (WorkNumber != null) {
             ops.add(ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                 .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                 .withValue(ContactsContract.Data.MIMETYPE,
             ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE)
                 .withValue(ContactsContract.CommonDataKinds.Phone.NUMBER, WorkNumber)
                 .withValue(ContactsContract.CommonDataKinds.Phone.TYPE,
             ContactsContract.CommonDataKinds.Phone.TYPE_WORK)
                 .build());
         }

         //------------------------------------------------------ Email
         if (emailID != null) {
             ops.add(ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                 .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                 .withValue(ContactsContract.Data.MIMETYPE,
             ContactsContract.CommonDataKinds.Email.CONTENT_ITEM_TYPE)
                 .withValue(ContactsContract.CommonDataKinds.Email.DATA, emailID)
                 .withValue(ContactsContract.CommonDataKinds.Email.TYPE, ContactsContract.CommonDataKinds.Email.TYPE_WORK)
                 .build());
         }

         //------------------------------------------------------ Organization
         if (!company.equals("") && !jobTitle.equals("")) {
             ops.add(ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                 .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                 .withValue(ContactsContract.Data.MIMETYPE,
             ContactsContract.CommonDataKinds.Organization.CONTENT_ITEM_TYPE)
                 .withValue(ContactsContract.CommonDataKinds.Organization.COMPANY, company)
                 .withValue(ContactsContract.CommonDataKinds.Organization.TYPE, ContactsContract.CommonDataKinds.Organization.TYPE_WORK)
                 .withValue(ContactsContract.CommonDataKinds.Organization.TITLE, jobTitle)
                 .withValue(ContactsContract.CommonDataKinds.Organization.TYPE, ContactsContract.CommonDataKinds.Organization.TYPE_WORK)
                 .build());
         }*/

         // Asking the Contact provider to create a new contact
         try {
             context.getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops);
         } catch (Exception e) {
             e.printStackTrace();
         //    Toast.makeText(myContext, "Exception: " + e.getMessage(), Toast.LENGTH_SHORT).show();
         }
      }

    public void insertContactToNative(Context context)
    {
        List<RcsBackupContact> contacts=getContactFromDb(context);

        for (int i=0; i < contacts.size(); i++) {
            if(logger.isActivated()){
                logger.debug(" Contact inserted to native contact");
                }
            RcsBackupContact contact = contacts.get(i);

            String number=contact.getNumber();
            String name=contact.getName();

            if(logger.isActivated()){
                logger.debug(" Contact inserted to native contact number "+number +" name "+name);
                }

            if(!contactExists(context, number)){
                createContact(context,name,number);
            }

        }

    }

}
