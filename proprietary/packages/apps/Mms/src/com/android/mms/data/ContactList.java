package com.android.mms.data;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Parcelable;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.text.TextUtils;
import android.util.Log;

import com.android.mms.LogTag;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
/// @}

public class ContactList extends ArrayList<Contact>  {
    private static final long serialVersionUID = 1L;

    /// M:
    private static final String TAG = "Mms/ContactList";
    // query params for contact lookup by number
    private static final Uri PHONES_WITH_PRESENCE_URI = Data.CONTENT_URI;

    private static final String[] CALLER_ID_PROJECTION = new String[] {
        Phone.NUMBER, // 0
        Phone.LABEL, // 1
        Phone.DISPLAY_NAME, // 2
        Phone.CONTACT_ID, // 3
        Phone.CONTACT_PRESENCE, // 4
        Phone.CONTACT_STATUS, // 5
        Phone.NORMALIZED_NUMBER, //6
        Phone.PHOTO_FILE_ID          //7
    };

    private static final int PHONE_NUMBER_COLUMN = 0;
    private static final int PHONE_LABEL_COLUMN = 1;
    private static final int CONTACT_NAME_COLUMN = 2;
    private static final int CONTACT_ID_COLUMN = 3;
    private static final int CONTACT_PRESENCE_COLUMN = 4;
    private static final int CONTACT_STATUS_COLUMN = 5;
    private static final int PHONE_NORMALIZED_NUMBER = 6;
    private static final int PHONE_PHOTO_ID_COLUMN = 7;

    public static ContactList getByNumbers(Iterable<String> numbers, boolean canBlock) {
        ContactList list = new ContactList();
        for (String number : numbers) {
            if (!TextUtils.isEmpty(number)) {
                list.add(Contact.get(number, canBlock));
            }
        }
        return list;
    }

    public static ContactList getByNumbers(String semiSepNumbers,
                                           boolean canBlock,
                                           boolean replaceNumber) {
        ContactList list = new ContactList();
        /// M: @{
        if (semiSepNumbers != null) {
            for (String number : semiSepNumbers.split(";")) {
                if (!TextUtils.isEmpty(number)) {
                    Contact contact = Contact.get(number, canBlock);
                    list.add(contact);
                }
            }
        }
        /// @}
        return list;
    }

    /**
     * Returns a ContactList for the corresponding recipient URIs passed in. This method will
     * always block to query provider. The given URIs could be the phone data URIs or tel URI
     * for the numbers don't belong to any contact.
     *
     * @param uris phone URI to create the ContactList
     */
    public static ContactList blockingGetByUris(Parcelable[] uris) {
        ContactList list = new ContactList();
        if (uris != null && uris.length > 0) {
            for (Parcelable p : uris) {
                Uri uri = (Uri) p;
                if ("tel".equals(uri.getScheme())) {
                    Contact contact = Contact.get(uri.getSchemeSpecificPart(), true);
                    list.add(contact);
                }
            }
            final List<Contact> contacts = Contact.getByPhoneUris(uris);
            if (contacts != null) {
                list.addAll(contacts);
            }
        }
        return list;
    }

    /**
     * Returns a ContactList for the corresponding recipient ids passed in. This method will
     * create the contact if it doesn't exist, and would inject the recipient id into the contact.
     */
    public static ContactList getByIds(String spaceSepIds, boolean canBlock) {
        ContactList list = new ContactList();
        try {
            for (RecipientIdCache.Entry entry : RecipientIdCache.getAddresses(spaceSepIds)) {
                if (entry != null && !TextUtils.isEmpty(entry.number)) {
                    Contact contact = Contact.get(entry.number, canBlock);
                    contact.setRecipientId(entry.id);
                    list.add(contact);
                }
            }
        } catch (NullPointerException e) {
                e.printStackTrace();
        }
        return list;
    }

    public int getPresenceResId() {
        // We only show presence for single contacts.
        if (size() != 1)
            return 0;

        return get(0).getPresenceResId();
    }

    public String formatNames(String separator) {
        String[] names = new String[size()];
        int i = 0;
        for (Contact c : this) {
            names[i++] = c.getName();
        }
        return TextUtils.join(separator, names);
    }

    public String formatNamesAndNumbers(String separator) {
        String[] nans = new String[size()];
        int i = 0;
        for (Contact c : this) {
            nans[i++] = c.getNameAndNumber();
        }
        return TextUtils.join(separator, nans);
    }

    public String serialize() {
        return TextUtils.join(";", getNumbers());
    }

    public boolean containsEmail() {
        for (Contact c : this) {
            if (c.isEmail()) {
                return true;
            }
        }
        return false;
    }

    public String[] getNumbers() {
        return getNumbers(false /* don't scrub for MMS address */);
    }

    public String[] getNumbers(boolean scrubForMmsAddress) {
        List<String> numbers = new ArrayList<String>();
        String number;
        for (Contact c : this) {
            number = c.getNumber();

            if (scrubForMmsAddress) {
                // parse/scrub the address for valid MMS address. The returned number
                // could be null if it's not a valid MMS address. We don't want to send
                // a message to an invalid number, as the network may do its own stripping,
                // and end up sending the message to a different number!
                number = MessageUtils.parseMmsAddress(number);
            }

            // Don't add duplicate numbers. This can happen if a contact name has a comma.
            // Since we use a comma as a delimiter between contacts, the code will consider
            // the same recipient has been added twice. The recipients UI still works correctly.
            // It's easiest to just make sure we only send to the same recipient once.
            if (!TextUtils.isEmpty(number) && !numbers.contains(number)) {
                numbers.add(number);
            }
        }
        return numbers.toArray(new String[numbers.size()]);
    }

    @Override
    public boolean equals(Object obj) {
        try {
            ContactList other = (ContactList) obj;
            // If they're different sizes, the contact
            // set is obviously different.
            if (size() != other.size()) {
                return false;
            }

            // Make sure all the individual contacts are the same.
            for (Contact c : this) {
                if (!other.contains(c)) {
                    return false;
                }
            }

            return true;
        } catch (ClassCastException e) {
            return false;
        }
    }

    private void log(String msg) {
        Log.d(LogTag.TAG, "[ContactList] " + msg);
    }

    /// M:
    public static ContactList getByNumbers(Context mContext, String semiSepNumbers,
            boolean canBlock, boolean replaceNumber) {
        ContactList list = new ContactList();
        if (semiSepNumbers.contains(";")) {
            String numbersForQuery = semiSepNumbers;
            if (numbersForQuery.contains("'")) {
                MmsLog.d(TAG, "replace single quotation to double quotation.");
                numbersForQuery = numbersForQuery.replaceAll("'", "''");
            }

            String selection = Phone.NUMBER
                    + " in ('" + numbersForQuery.replaceAll(";", "','") + "')";
            //selection = selection.replaceAll("-", "");
            LinkedList<String> numberList =
                new LinkedList<String>(Arrays.asList(semiSepNumbers.split(";")));
            MmsLog.dpi(TAG, "ContactList.getByNumbers()--selection = \"" + selection + "\"");
            //selection = DatabaseUtils.sqlEscapeString(selection);
            Cursor cursor = mContext.getContentResolver().query(PHONES_WITH_PRESENCE_URI,
                    CALLER_ID_PROJECTION, selection, null, null);
            if (cursor == null) {
                MmsLog.dpi(TAG, "ContactList.getByNumbers("
                        + semiSepNumbers + ") returned NULL cursor! contact uri used "
                        + PHONES_WITH_PRESENCE_URI);
                return list;
            }
            Collections.sort(numberList);
            try {
                while (cursor.moveToNext()) {
                    String number = cursor.getString(PHONE_NUMBER_COLUMN);
                    String label = cursor.getString(PHONE_LABEL_COLUMN);
                    String name = cursor.getString(CONTACT_NAME_COLUMN);
                    String mNumberE164 = cursor.getString(PHONE_NORMALIZED_NUMBER);
                    String nameAndNumber = Contact.formatNameAndNumber(name, number, mNumberE164);
                    long personId = cursor.getLong(CONTACT_ID_COLUMN);
                    long photoId = cursor.getLong(PHONE_PHOTO_ID_COLUMN);
                    int presence = cursor.getInt(CONTACT_PRESENCE_COLUMN);
                    String presenceText = cursor.getString(CONTACT_STATUS_COLUMN);
                    Contact entry = new Contact(number,
                            label, name, nameAndNumber, personId, presence, presenceText, photoId);

                    byte[] data = Contact.loadAvatarData(entry, mContext);
                    synchronized (entry) {
                        entry.mAvatarData = data;
                    }
                    list.add(entry);
                    removeNumberFromList(numberList, number);
                }
            } finally {
                cursor.close();
            }
            MmsLog.d(TAG, "getByNumbers(): numberList.size():" + numberList.size());
            if (numberList.size() > 0) {
                for (String number : numberList) {
                    Contact entry = Contact.get(number, false);
                    list.add(entry);
                }
            }
            return list;
        } else {
            // only one recipient, query with block
            canBlock = true;
            return getByNumbers(semiSepNumbers, canBlock, replaceNumber);
        }
    }

    public String getFirstName(String separator) {
        return this.get(0).getName() + " & " + (size() - 1);
    }

    public static void removeNumberFromList(LinkedList<String> list, String number) {
        boolean go = true;
        int listSize = list.size();
        int i = listSize / 2;
        int start = 0;
        int end = listSize;
        while (go) {
            if (i < 0 || i >= listSize) {
                break;
            }
            int j = number.compareToIgnoreCase(list.get(i));
            MmsLog.d(TAG, "removeNumberFromList(): i="
                    + i + ", j=" + j + ", start=" + start + ", end=" + end);
            if (j == 0) {
                MmsLog.dpi(TAG, "removeNumberFromList(): remove number: " + list.get(i));
                list.remove(i);
                break;
            } else if (j > 0) {
                start = i + 1;
            } else if (j < 0) {
                end = i;
            }
            if (i == 0 || i == (listSize - 1) || start == end) {
                break;
            }
            i = (start + end) / 2;
        }
        MmsLog.d(TAG, "removeNumberFromList(): after remove number: list.size()=" + list.size());
    }

    public static ContactList blockingGetByIds(long[] ids) {
        ContactList list = new ContactList();
        if (ids != null && ids.length > 0) {
            final List<Contact> contacts = Contact.getByPhoneIds(ids);
            if (contacts != null) {
                list.addAll(contacts);
            }
        }
        return list;
    }

    /// M: get protosomatic numbers.
    public String[] getProtosomaitcNumbers() {
        return getProtosomaticNumbers(true /* scrub for MMS address */);
    }

    /// M: get protosomatic numbers not duplicated. @{
    public String[] getProtosomaticNumbers(boolean scrubForMmsAddress) {
        List<String> numbers = new ArrayList<String>();
        List<String> formatedNumbers = new ArrayList<String>();
        String number;
        String formatedNumber = "";
        for (Contact c : this) {
            number = c.getNumberProtosomatic();

            if (scrubForMmsAddress) {
                // parse/scrub the address for valid MMS address. The returned number
                // could be null if it's not a valid MMS address. We don't want to send
                // a message to an invalid number, as the network may do its own stripping,
                // and end up sending the message to a different number!
                formatedNumber = MessageUtils.parseMmsAddress(number);
            }

            // Don't add duplicate numbers. This can happen if a contact name has a comma.
            // Since we use a comma as a delimiter between contacts, the code will consider
            // the same recipient has been added twice. The recipients UI still works correctly.
            // It's easiest to just make sure we only send to the same recipient once.
            if (!TextUtils.isEmpty(number) && !formatedNumbers.contains(formatedNumber)) {
                numbers.add(number);
                if (!TextUtils.isEmpty(formatedNumber)) {
                    formatedNumbers.add(formatedNumber);
                }
            }
        }
        return numbers.toArray(new String[numbers.size()]);
    }
    /// @}
}
