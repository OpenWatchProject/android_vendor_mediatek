package com.mediatek.contacts.aassne;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.view.inputmethod.EditorInfo;

import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountType.EditField;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.model.account.BaseAccountType.SimpleInflater;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.dataitem.DataKind;
import com.android.contacts.model.RawContactDelta;
import com.android.contacts.model.RawContactDeltaList;
import com.android.contacts.model.RawContactModifier;
import com.android.contacts.R;
import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import com.google.android.collect.Lists;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.model.account.UsimAccountType;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;

import java.util.ArrayList;

public class SimSneEditor {
    private final static String TAG = "SimSneEditor";

    protected static final int FLAGS_PERSON_NAME = EditorInfo.TYPE_CLASS_TEXT
            | EditorInfo.TYPE_TEXT_FLAG_CAP_WORDS
            | EditorInfo.TYPE_TEXT_VARIATION_PERSON_NAME;

    private Context mContext = null;
    ArrayList<String> mNickNameArray = null;

    public SimSneEditor(Context context) {
        mContext = context;
    }

    private boolean buildNicknameValues(String accountType,
            ContentValues values, String nickName) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            values.put("sne", TextUtils.isEmpty(nickName) ? "" : nickName);
            return true;
        }
        return false;
    }

    private boolean updateDataToDb(int subId, String accountType, ContentResolver resolver,
            String updateNickname, String oldNickname, long rawId) {
        if (!PhbInfoUtils.usimHasSne(subId)) {
            Log.e(TAG, "[updateDataToDb]DB_UPDATE_NICKNAME-error subId");
            return false;
        }
        return updateNicknameToDB(accountType, resolver, updateNickname, oldNickname, rawId);
    }

    private boolean updateNicknameToDB(String accountType, ContentResolver resolver,
            String updateNickname, String oldNickname, long rawId) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            ContentValues nicknamevalues = new ContentValues();
            String whereNickname = Data.RAW_CONTACT_ID + " = \'" + rawId + "\'" + " AND "
                    + Data.MIMETYPE + "='" + Nickname.CONTENT_ITEM_TYPE + "'";

            updateNickname = TextUtils.isEmpty(updateNickname) ? "" : updateNickname;
            Log.sensitive(TAG, "[updateNicknameToDB]whereNickname is:" + whereNickname
                    + ",updateNickname:" + updateNickname);

            if (!TextUtils.isEmpty(updateNickname) && !TextUtils.isEmpty(oldNickname)) {
                nicknamevalues.put(Nickname.NAME, updateNickname);
                int upNickname = resolver.update(Data.CONTENT_URI,
                        nicknamevalues, whereNickname, null);
                Log.d(TAG, "[updateNickname] upNickname is " + Log.anonymize(upNickname));
            } else if (!TextUtils.isEmpty(updateNickname) && TextUtils.isEmpty(oldNickname)) {
                nicknamevalues.put(Nickname.RAW_CONTACT_ID, rawId);
                nicknamevalues.put(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                nicknamevalues.put(Nickname.NAME, updateNickname);
                Uri upNicknameUri = resolver.insert(Data.CONTENT_URI, nicknamevalues);
                Log.d(TAG, "[updateNickname] upNicknameUri is " + Log.anonymize(upNicknameUri));
            } else if (TextUtils.isEmpty(updateNickname)) {
                // update nickname is null,delete name row
                int deleteNickname = resolver.delete(Data.CONTENT_URI, whereNickname, null);
                Log.d(TAG, "[updateNickname] deleteNickname is "
                        + Log.anonymize(deleteNickname));
            }
            return true;
        }
        return false;
    }

    private boolean buildSneOperation(String accountType,
            ArrayList<ContentProviderOperation> operationList, String nickname, int backRef) {
        Log.d(TAG, "[buildSneOperation] entry");
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            // build SNE ContentProviderOperation
            Log.d(TAG, "[buildSneOperation] isUSIM true");
            if (!TextUtils.isEmpty(nickname)) {
                Log.d(TAG, "[buildSneOperation] nickname is not empty");
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(Data.CONTENT_URI);
                Log.d(TAG, "[buildSneOperation] nickname:" + Log.anonymize(nickname));
                builder.withValueBackReference(Nickname.RAW_CONTACT_ID, backRef);
                builder.withValue(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                builder.withValue(Nickname.DATA, nickname);
                operationList.add(builder.build());
                Log.sensitive(TAG, "[buildSneOperation] operationList:" + operationList);
                return true;
            }
        }
        return true;
    }

    public boolean buildOperationFromCursor(ArrayList<ContentProviderOperation> operationList,
            final Cursor cursor, int index) {
        // build SNE ContentProviderOperation from cursor
        int sneColumnIdx = cursor.getColumnIndex("sne");
        Log.d(TAG, "[buildOperationFromCursor] sneColumnIdx:" + sneColumnIdx);
        if (sneColumnIdx != -1) {
            String nickname = cursor.getString(sneColumnIdx);
            Log.d(TAG, "[buildOperationFromCurson] nickname:" + Log.anonymize(nickname));
            if (!TextUtils.isEmpty(nickname)) {
                Log.d(TAG, "[buildOperationFromCursor] nickname is not empty");
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(Data.CONTENT_URI);
                builder.withValueBackReference(Nickname.RAW_CONTACT_ID, index);
                builder.withValue(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);
                builder.withValue(Nickname.DATA, nickname);
                operationList.add(builder.build());
                return true;
            }
        }
        return false;
    }

    private boolean buildNicknameValueForInsert(int subId, ContentValues cv, String nickname) {
        if (PhbInfoUtils.usimHasSne(subId)) {
            Log.d(TAG, "[buildNicknameValueForInsert] plugin");
            cv.put("sne", TextUtils.isEmpty(nickname) ? "" : nickname);
            return true;
        }
        return false;
    }

    private boolean isNickname(String mimeType) {
        Log.d(TAG, "[isNickname] plugin");
        return Nickname.CONTENT_ITEM_TYPE.equals(mimeType);
    }

    /** start this API is used contactlist */
    public String buildSimNickname(String accountType, ArrayList<String> nicknameArray, int subId,
            String defValue) {
        Log.d(TAG, "[buildSimNickname] Entry");
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            Log.d(TAG, "[buildSimNickname] USIM is true");
            boolean hasSne = PhbInfoUtils.usimHasSne(subId);
            String simNickname = null;
            if (!nicknameArray.isEmpty() && hasSne) {
                Log.d(TAG, "[buildSimNickname] nickname array is not empty");
                Log.d(TAG, "[buildSimNickname] hasSne is:" + hasSne);
                simNickname = nicknameArray.remove(0);
                simNickname = TextUtils.isEmpty(simNickname) ? "" : simNickname;
                int len = PhbInfoUtils.getUsimSneMaxNameLength(subId);

                Log.d(TAG, "[buildSimNickname]before encode simNickname="
                        + Log.anonymize(simNickname));
                try { // the code copy from CustomAasActivity.
                    GsmAlphabet.stringToGsm7BitPacked(simNickname);
                    if (simNickname.length() > len) {
                        simNickname = ""; // simNickname.substring(0, len);
                    }
                } catch (EncodeException e) {
                    Log.e(TAG, "[buildSimNickname] Error at GsmAlphabet.stringToGsm7BitPacked()!");
                    if (simNickname.length() > ((len - 1) >> 1)) {
                        simNickname = ""; // simNickname.substring(0, len);
                    }
                }
                Log.d(TAG, "[buildSimNickname]after encode simNickname="
                        + Log.anonymize(simNickname));
            }
            return simNickname;
        }
        return defValue;
    }

    /**
     * New interface implementation here
     */

    public void onEditorBindEditors(RawContactDelta entity, AccountType type, int subId) {
        Log.d(TAG, "[onEditorBindEditors] Entry");
        boolean hasSne = PhbInfoUtils.usimHasSne(subId);
        if (AccountTypeUtils.isUsim(type.accountType)) {
            Log.d(TAG, "[onEditorBindEditors] isUSIM");
            SimAasSneUtils.setCurrentSubId(subId);
            if (hasSne) {
                addDataKindNickname(type);
                Log.d(TAG, "[onEditorBindEditors] ensurekindexists");
                RawContactModifier.ensureKindExists(entity, type, Nickname.CONTENT_ITEM_TYPE);
            } else {
                Log.d(TAG, "[onEditorBindEditors] removeDataKindNickname");
                removeDataKindNickname(type);
            }
            DataKind dataKind = type.getKindForMimetype(Nickname.CONTENT_ITEM_TYPE);
            if (dataKind != null) {
                Log.d(TAG, "[onEditorBindEditors] datakind not null");
                updateNickname(dataKind, hasSne);
            }
        }
    }

    /**
     * update the nick name DataKind for sim AccountType.
     */
    public void updateNickNameKind(AccountType type, int subId) {
        if (type != null && AccountTypeUtils.isUsim(type.accountType)) {
            boolean hasSne = PhbInfoUtils.usimHasSne(subId);
            if (hasSne) {
                addDataKindNickname(type);
            } else {
                removeDataKindNickname(type);
            }
            DataKind dataKind = type.getKindForMimetype(Nickname.CONTENT_ITEM_TYPE);
            if (dataKind != null) {
                updateNickname(dataKind, hasSne);
            }
        }
    }

    public void onEditorBindForCompactEditor(RawContactDeltaList state,
            int subId, Context context) {
        int numRawContacts = state.size();
        Log.d(TAG, "[onEditorBindForCompactEditor] Entry numRawContacts= " + numRawContacts);
        final AccountTypeManager accountTypes = AccountTypeManager.getInstance(mContext);
        for (int i = 0; i < numRawContacts; i++) {
            final RawContactDelta rawContactDelta = state.get(i);
            final AccountType type = rawContactDelta.getAccountType(accountTypes);
            Log.d(TAG, "[onEditorBindForCompactEditor] loop subid=" + subId);
            onEditorBindEditors(rawContactDelta, type, subId);
        }
    }

    private void updateNickname(DataKind kind, boolean hasSne) {
        Log.d(TAG, "[updateNickname]for USIM,hasSne:" + hasSne);
        if (hasSne) {
            if (kind.fieldList == null) {
                kind.fieldList = Lists.newArrayList();
            } else {
                kind.fieldList.clear();
            }
            kind.fieldList.add(new EditField(Nickname.NAME, R.string.nicknameLabelsGroup,
                    FLAGS_PERSON_NAME));
        } else {
            kind.fieldList = Lists.newArrayList();
        }
    }

    @SuppressWarnings("illegalcatch")
    private void addDataKindNickname(AccountType accountType) {
        // 1.check if instanceof UsimAccountType,then can cast it to
        // UsimAccountType for invoke it's method
        // 2.refer:addDataKindNickname()
        Log.d(TAG, "[addDataKindNickname]Entry");
        if (accountType instanceof UsimAccountType) {
            Log.d(TAG, "[addDataKindNickname]account type is instance of USIM");
            UsimAccountType uaccountType = (UsimAccountType) accountType;
            DataKind kind;
            try {
                kind = uaccountType.addKind(new DataKind(Nickname.CONTENT_ITEM_TYPE,
                        R.string.nicknameLabelsGroup, 115, true));
            } catch (Exception de) {
                Log.d(TAG, "[addDataKindNickname]addkind Exception & return");
                return;
            }
            kind.typeOverallMax = 1;
            kind.actionHeader = new SimpleInflater(R.string.nicknameLabelsGroup);
            kind.actionBody = new SimpleInflater(Nickname.NAME);
            kind.defaultValues = new ContentValues();
            kind.defaultValues.put(Nickname.TYPE, Nickname.TYPE_DEFAULT);
            Log.d(TAG, "[addDataKindNickname]adding kind");
        }
    }

    @SuppressWarnings("illegalcatch")
    private void removeDataKindNickname(AccountType accountType) {
        Log.d(TAG, "[removeDataKindNickname]Entry");
        if (accountType instanceof UsimAccountType) {
            Log.d(TAG, "[removeDataKindNickname]account type is instance of USIM");
            UsimAccountType uaccountType = (UsimAccountType) accountType;
            try {
                uaccountType.removeKind(Nickname.CONTENT_ITEM_TYPE);
            } catch (Exception de) {
                Log.d(TAG, "[removeDataKindNickname]removekind Exception & return");
                return;
            }
            Log.d(TAG, "[removeDataKindNickname] done");
        }
    }

    private void fillNickNameArray(Uri sourceUri) {
        Log.d(TAG, "[fillNickNameArray] Entry");
        mNickNameArray = new ArrayList<String>();

        final String[] projection = new String[] { Contacts.Data.MIMETYPE, Contacts.Data.DATA1, };

        ContentResolver resolver = mContext.getContentResolver();
        Cursor c = resolver.query(sourceUri, projection, null, null, null);
        if (c != null && c.moveToFirst()) {
            do {
                String mimeType = c.getString(0);
                if (isNickname(mimeType)) {
                    String nickName = c.getString(1);
                    Log.d(TAG, "[fillNickNameArray] nickname is:" + Log.anonymize(nickName));
                    mNickNameArray.add(nickName);
                }
            } while (c.moveToNext());
        }
        if (c != null) {
            c.close();
        }

    }

    // called when phone contacts are copied to USIM
    public void copySimSneToAccount(ArrayList<ContentProviderOperation> operationList,
            AccountWithDataSet targetAccount, Uri sourceUri, int backRef) {
        // 1.get simNickname by uri
        String simNickname = null;
        Log.d(TAG, "[copySimSneToAccount] Entry and sourceUri: " + sourceUri);
        fillNickNameArray(sourceUri);
        Log.d(TAG, "[copySimSneToAccount] after fillNickNameArray backRef " + backRef);
        // ContentValues updatevalues = new ContentValues();
        if (!(targetAccount instanceof AccountWithDataSetEx)) {
            throw new IllegalArgumentException("targetAccount is not AccountWithDataSetEx!");
        }
        AccountWithDataSetEx account = (AccountWithDataSetEx) targetAccount;
        int subId = account.getSubId();
        Log.d(TAG, "[copySimSneToAccount] subId " + subId);
        simNickname = buildSimNickname(targetAccount.type, mNickNameArray, subId, simNickname);
        Log.d(TAG, "[copySimSneToAccount] after buildSimNickname simNickname is:"
                + Log.anonymize(simNickname));
        buildSneOperation(targetAccount.type, operationList, simNickname, backRef);
        Log.d(TAG, "[copySimSneToAccount] after buildSneOperation");
    }

    public int importSimSne(ArrayList<ContentProviderOperation> operationList, Cursor cursor,
            int loopCheck) {
        // 1.get sne from cursor
        // 2.build it to operationList
        // default return 0
        Log.d(TAG, "[importSimSne] Entry");
        return buildOperationFromCursor(operationList, cursor, loopCheck) ? 1 : 0;
    }

    public void editSimSne(Intent intent, long indexInSim, int subId, long rawContactId) {
        // 1. get sNickname like getRawContactDataFromIntent()
        // 2. get mOldNickname like setOldRawContactData()
        // 3.check sNickname isTextValid() like editSIMContact()
        // 4.buildNicknameValueForInsert like setUpdateValues()
        // 5.save to sim first---if mOldNickname is empty,need insert,else update
        // 6.then save to contacts db:updateDataToDb() like editSIMContact()

        String sNickname = null;
        String sOldNickname = null;
        ArrayList<RawContactDelta> newSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_SIM_DATA);
        ArrayList<RawContactDelta> oldSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_OLD_SIM_DATA);
        Log.d(TAG, "[editSimSne] Entry");
        String accountType = newSimData.get(0).getValues().getAsString(RawContacts.ACCOUNT_TYPE);
        Log.sensitive(TAG, "[editSimSne] Accountype from newSimData:" + accountType);
        String data = null;
        String mimeType = null;
        if (!PhbInfoUtils.usimHasSne(subId) || !AccountTypeUtils.isUsimOrCsim(accountType)) {
            // sim is not USIM and slot id dont have sne field
            Log.d(TAG, "[editSimSne] do nothing & return ");
            return;
        }
        // calculate the newNickname
        int kindCount = newSimData.get(0).getContentValues().size();
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            for (int countIndex = 0; countIndex < kindCount; countIndex++) {
                mimeType = newSimData.get(0).getContentValues().get(countIndex)
                        .getAsString(Data.MIMETYPE);
                data = newSimData.get(0).getContentValues().get(countIndex).getAsString(Data.DATA1);
                Log.d(TAG, "[editSimSne]countIndex:" + countIndex + ",mimeType:" + mimeType
                        + "data:" + Log.anonymize(data));
                if (Nickname.CONTENT_ITEM_TYPE.equals(mimeType)) {
                    // this means this type is nickname type
                    sNickname = data;
                    sNickname = TextUtils.isEmpty(sNickname) ? "" : sNickname;
                    Log.d(TAG, "[editSimSne] updated nickname is" + Log.anonymize(sNickname));
                }
            }
        }

        // calculate the OldNickname
        if (oldSimData != null) {
            int oldCount = oldSimData.get(0).getContentValues().size();
            for (int oldIndex = 0; oldIndex < oldCount; oldIndex++) {
                mimeType = oldSimData.get(0).getContentValues().get(oldIndex)
                        .getAsString(Data.MIMETYPE);
                data = oldSimData.get(0).getContentValues().get(oldIndex).getAsString(Data.DATA1);
                Log.d(TAG, "[getOldRawContactData]Data.MIMETYPE: " + mimeType + ",data:"
                        + Log.anonymize(data));
                if (Nickname.CONTENT_ITEM_TYPE.equals(mimeType)) {
                    sOldNickname = data;
                    Log.d(TAG, "[editSimSne]sOldNickname=" + Log.anonymize(sOldNickname));
                }
            }
        }
        ContentResolver resolver = mContext.getContentResolver();
        // Write to PHB
        Log.d(TAG, "[editSimSne] RawcontactId" + rawContactId);
        updateDataToDb(subId, accountType, resolver, sNickname, sOldNickname, rawContactId);
    }

    public boolean isSneNicknameValid(String nickName, int subId) {
        if (!PhbInfoUtils.usimHasSne(subId)) {
            Log.i(TAG, "[isSneNicknameValid] return false, sim not support sne");
            return false;
        }
        if (TextUtils.isEmpty(nickName)) {
            Log.d(TAG, "[isSneNicknameValid] nickname valid true");
            return true;
        }
        final int maxLength = PhbInfoUtils.getUsimSneMaxNameLength(subId);
        Log.d(TAG, "[isSneNicknameValid] max sne length" + maxLength);
        try {
            GsmAlphabet.stringToGsm7BitPacked(nickName);
            Log.d(TAG, "[isSneNicknameValid] given sne length" + nickName.length());
            if (nickName.length() > maxLength) {
                Log.d(TAG, "[isSneNicknameValid] length exceeds & false");
                return false;
            }
        } catch (EncodeException e) {
            if (nickName.length() > ((maxLength - 1) >> 1)) {
                Log.d(TAG, "[isSneNicknameValid] exception & false");
                return false;
            }
        }
        return true;
    }

    public String getNickName(Intent intent, int subId) {
        Log.d(TAG, "[getNickName] getNickName subId value:" + subId);
        if (!PhbInfoUtils.usimHasSne(subId)) {
            Log.i(TAG, "[checkNickName] return null, sim not support sne");
            return null;
        }
        String nickname = null;
        ArrayList<RawContactDelta> newSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_SIM_DATA);
        String accountType = newSimData.get(0).getValues().getAsString(RawContacts.ACCOUNT_TYPE);
        String data = null;
        String mimeType = null;
        int kindCount = newSimData.get(0).getContentValues().size();

        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            for (int countIndex = 0; countIndex < kindCount; countIndex++) {
                mimeType = newSimData.get(0).getContentValues().get(countIndex)
                        .getAsString(Data.MIMETYPE);
                data = newSimData.get(0).getContentValues().get(countIndex).getAsString(Data.DATA1);
                Log.d(TAG, "[checkNickName]countIndex:" + countIndex + ",mimeType:" + mimeType
                        + "data:" + Log.anonymize(data));
                if (Nickname.CONTENT_ITEM_TYPE.equals(mimeType)) {
                    // this means this type is nickname type
                    nickname = data;
                    nickname = TextUtils.isEmpty(nickname) ? "" : nickname;
                    Log.d(TAG, "[checkNickName] updated nickname is"
                            + Log.anonymize(nickname));
                    break;
                }
            }
        }
        return nickname;
    }

    public void updateValues(Intent intent, int subId, ContentValues preContentValues) {
        String sNickname = null;

        if (!PhbInfoUtils.usimHasSne(subId)) {
            preContentValues.remove("sne");
            Log.i(TAG, "[updateValues] hasSne false & return");
            return;
        }
        ArrayList<RawContactDelta> newSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_SIM_DATA);
        String accountType = newSimData.get(0).getValues().getAsString(RawContacts.ACCOUNT_TYPE);
        SimAasSneUtils.setCurrentSubId(subId);
        String data = null;
        String mimeType = null;
        int kindCount = newSimData.get(0).getContentValues().size();
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            for (int countIndex = 0; countIndex < kindCount; countIndex++) {
                mimeType = newSimData.get(0).getContentValues().get(countIndex)
                        .getAsString(Data.MIMETYPE);
                data = newSimData.get(0).getContentValues().get(countIndex).getAsString(Data.DATA1);
                Log.d(TAG, "[updateValues]countIndex:" + countIndex + ",mimeType:" + mimeType
                        + "data:" + Log.anonymize(data));
                if (Nickname.CONTENT_ITEM_TYPE.equals(mimeType)) {
                    // this means this type is nickname type
                    sNickname = data;
                    sNickname = TextUtils.isEmpty(sNickname) ? "" : sNickname;
                    Log.d(TAG, "[updateValues] updated nickname is"
                            + Log.anonymize(sNickname));
                }
            }
        }

        Log.d(TAG, "[updateValues] hasSne and sne is:" + Log.anonymize(sNickname));
        preContentValues.put("sne", TextUtils.isEmpty(sNickname) ? "" : sNickname);

    }

    public void updateValuesforCopy(Uri sourceUri, int subId, String accountType,
            ContentValues simContentValues) {

        Log.d(TAG, "[updateValuesforCopy] Entry sourceUri is :"
                + Log.anonymize(sourceUri) + " Entry subId is : " + subId);
        ArrayList<String> nickNameArray = new ArrayList<String>();

        final String[] projection = new String[] { Contacts.Data.MIMETYPE, Contacts.Data.DATA1, };
        if (!PhbInfoUtils.usimHasSne(subId)) {
            // nothing to to
            Log.d(TAG, "[updateValuesforCopy] No sne field in SIM");
            return;
        }

        String simNickname = null;
        ContentResolver resolver = mContext.getContentResolver();
        Cursor c = resolver.query(sourceUri, projection, null, null, null);
        if (c != null && c.moveToFirst()) {
            do {
                String mimeType = c.getString(0);
                if (isNickname(mimeType)) {
                    String nickName = c.getString(1);
                    Log.d(TAG, "[updateValuesforCopy] nickname is:" + nickName);
                    nickNameArray.add(nickName);
                }
            } while (c.moveToNext());
        }
        if (c != null) {
            c.close();
        }

        simNickname = buildSimNickname(accountType, nickNameArray, subId, simNickname);
        Log.d(TAG, "[updateValuesforCopy] put values nickname is:" + simNickname);
        simContentValues.put("sne", simNickname);

    }

}
