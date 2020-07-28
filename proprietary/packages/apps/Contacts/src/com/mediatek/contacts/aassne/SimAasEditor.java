package com.mediatek.contacts.aassne;

import android.content.ContentProviderOperation;
import android.content.ContentProviderOperation.Builder;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.view.inputmethod.EditorInfo;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountType.EditField;
import com.android.contacts.model.account.AccountType.EditType;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.model.dataitem.DataKind;
import com.android.contacts.model.RawContactDelta;
import com.android.contacts.model.RawContactDeltaList;
import com.android.contacts.model.RawContactModifier;
import com.android.contacts.model.ValuesDelta;
import com.android.contacts.R;

import com.google.android.collect.Lists;

import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.MtkPhoneNumberUtils;
import com.mediatek.internal.telephony.phb.AlphaTag;
import com.mediatek.provider.MtkContactsContract;
import com.mediatek.provider.MtkContactsContract.Aas;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Pattern;

public class SimAasEditor {
    private final static String TAG = "SimAasEditor";

    public static final int TYPE_FOR_PHONE_NUMBER = 0;
    public static final int TYPE_FOR_ADDITIONAL_NUMBER = 1;

    public static final int VIEW_UPDATE_NONE = 0;
    public static final int VIEW_UPDATE_HINT = 1;
    public static final int VIEW_UPDATE_VISIBILITY = 2;
    public static final int VIEW_UPDATE_DELETE_EDITOR = 3;
    public static final int VIEW_UPDATE_LABEL = 4;
    public static final int TYPE_OPERATION_AAS = 0;
    public static final int VIEW_TYPE_SUB_KIND_TITLE_ENTRY = 6;

    public static final int OPERATION_CONTACT_INSERT = 1;
    public static final int OPERATION_CONTACT_EDIT = 2;
    public static final int OPERATION_CONTACT_COPY = 3;

    public static final char STRING_PRIMART = 0;
    public static final char STRING_ADDITINAL = 1;

    public static final int SIM_ID_DONT_KNOW_CURR = 10;
    //should be in sync with host app
    private static final int FLAGS_USIM_NUMBER = EditorInfo.TYPE_CLASS_PHONE;
    protected static final int FLAGS_EMAIL = EditorInfo.TYPE_CLASS_TEXT
            | EditorInfo.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
    protected static final int FLAGS_PHONE = EditorInfo.TYPE_CLASS_PHONE;
    private Context mContext = null;
    private ArrayList<Anr> mOldAnrsList = new ArrayList<Anr>();
    private ArrayList<Anr> mAnrsList = new ArrayList<Anr>();
    private ArrayList<Anr> mCopyAnrList = null;
    private ArrayList<AasIndicatorNamePair> mOldAasIndNameList = null;
    private Uri mCopyUri = null;
    private int mInsertFlag = 0;

    public SimAasEditor(Context context) {
        mContext = context;
    }

    // IP start
    public void setCurrentSubId(int subId) {
        Log.d(TAG, "[setCurrentSubId] subId: = " + subId);
        SimAasSneUtils.setCurrentSubId(subId);
    }

    public void updatePhoneType(int subId, DataKind kind) {
        if (kind.typeList == null) {
            kind.typeList = Lists.newArrayList();
        } else {
            kind.typeList.clear();
        }
        // Fix ALPS03889312: add EditType only if supports AAS.
        // May not support AAS, even if supports ANR.
        if (PhbInfoUtils.getUsimAasCount(subId) > 0) {
            List<AlphaTag> atList = SimAasSneUtils.getAAS(subId);
            final int specificMax = -1;

            kind.typeList.add((new EditType(Anr.TYPE_AAS, R.string.aas_phone_type_none))
                    .setSpecificMax(specificMax));
            for (AlphaTag tag : atList) {
                final int recordIndex = tag.getRecordIndex();
                Log.d(TAG, "[updatePhoneType] label=" + Log.anonymize(tag.getAlphaTag()));

                kind.typeList.add((new EditType(Anr.TYPE_AAS, Phone
                        .getTypeLabelResource(Anr.TYPE_AAS))).setSpecificMax(
                        specificMax).setCustomColumn(Aas.buildIndicator(subId, recordIndex)));

            }
            kind.typeList.add((new EditType(Phone.TYPE_CUSTOM, Phone
                    .getTypeLabelResource(Phone.TYPE_CUSTOM)))
                    .setSpecificMax(specificMax));
            Log.d(TAG, "[updatePhoneType] subId = " + subId + " specificMax=" + specificMax);
        }
        kind.fieldList = Lists.newArrayList();
        kind.fieldList.add(new EditField(Phone.NUMBER, mContext.getResources()
                .getIdentifier("local_phone_account", "string", "com.android.contacts"),
                FLAGS_USIM_NUMBER));
    }

    private void ensureKindExists(RawContactDelta state, String mimeType,
            DataKind kind, int subId) {
        if (state == null) {
            Log.w(TAG, "[ensureKindExists] state is null,return!");
            return;
        }
        if (kind != null) {
            ArrayList<ValuesDelta> values = state.getMimeEntries(mimeType);
            final int slotAnrSize = PhbInfoUtils.getUsimAnrCount(subId);
            /// fix ALPS02795593.remove anr field when not support anr.@{
            removeAnrFieldIfNotSupportAnr(state, values, slotAnrSize);
            /// @}
            if (values != null && values.size() == slotAnrSize + 1) {
                // primary number + slotNumber size
                int anrSize = 0;
                for (ValuesDelta value : values) {
                    Integer isAnr = value.getAsInteger(
                            MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER);
                    if (isAnr != null && (isAnr.intValue() == 1)) {
                        anrSize++;
                    }
                }
                Log.d(TAG, "[ensureKindExists] size=" + values.size() + ",slotAnrSize="
                        + slotAnrSize + ",anrSize=" + anrSize);
                if (anrSize < slotAnrSize && values.size() > 1) {
                    for (int i = 1; i < values.size(); i++) {
                        values.get(i).put(
                            MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
                    }
                }

                return;
            }
            if (values == null || values.isEmpty()) {
                Log.d(TAG, "[ensureKindExists] Empty, insert primary: and anr:" + slotAnrSize);
                // Create child when none exists and valid kind
                final ValuesDelta child = RawContactModifier.insertChild(state, kind);
                if (kind.mimeType.equals(Phone.CONTENT_ITEM_TYPE)) {
                    child.setFromTemplate(true);
                }

                for (int i = 0; i < slotAnrSize; i++) {
                    final ValuesDelta slotChild = RawContactModifier.insertChild(state, kind);
                    slotChild.put(
                        MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
                }
            } else {
                int pnrSize = 0;
                int anrSize = 0;
                if (values != null) {
                    for (ValuesDelta value : values) {
                        Integer isAnr = value.getAsInteger(
                            MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER);
                        if (isAnr != null && (isAnr.intValue() == 1)) {
                            anrSize++;
                        } else {
                            pnrSize++;
                        }
                    }
                }
                Log.d(TAG, "[ensureKindExists] pnrSize=" + pnrSize + ", anrSize=" + anrSize +
                        ",slotAnrSize: " + slotAnrSize);
                if (pnrSize < 1) {
                    // insert a empty primary number if not exists.
                    final ValuesDelta slotChild = RawContactModifier.insertChild(state, kind);
                }
                for (; anrSize < slotAnrSize; anrSize++) {
                    // insert additional numbers if not full.
                    final ValuesDelta slotChild = RawContactModifier.insertChild(state, kind);
                    slotChild.put(MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
                }
            }
        }
    }

    public boolean ensurePhoneKindForEditor(AccountType type, int subId, RawContactDelta entity) {
        SimAasSneUtils.setCurrentSubId(subId);
        String accountType = entity.getAccountType();

        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            DataKind dataKind = type.getKindForMimetype(Phone.CONTENT_ITEM_TYPE);
            if (dataKind != null) {
                updatePhoneType(subId, dataKind);
            }
            ensureKindExists(entity, Phone.CONTENT_ITEM_TYPE, dataKind, subId);
        }
        return true; // need to check later
    }

    /**
     * update the phone DataKind for sim AccountType.
     */
    public void updatePhoneKind(AccountType type, int subId) {
        if (type != null && AccountTypeUtils.isUsimOrCsim(type.accountType)) {
            DataKind dataKind = type.getKindForMimetype(Phone.CONTENT_ITEM_TYPE);
            if (dataKind != null) {
                updatePhoneType(subId, dataKind);
            }
        }
    }

    public boolean handleLabel(DataKind kind, ValuesDelta entry, RawContactDelta state) {
        String accountType = state.getAccountType();
        if (AccountTypeUtils.isSimOrRuim(accountType) && AccountTypeUtils.isPhoneNumType(
                kind.mimeType)) {
            Log.d(TAG, "[handleLabel] hide label for sim card or Ruim");
            return true;
        }
        if (isPrimaryNumber(kind, entry, state)) {
            // primary number, hide phone label
            Log.d(TAG, "[handleLabel] hide label for primary number.");
            return true;
        }
        if (AccountTypeUtils.isUsimOrCsim(accountType)
                && Email.CONTENT_ITEM_TYPE.equals(kind.mimeType)) {
            Log.d(TAG, "[handleLabel] hide label for email");
            return true;
        }

        return false;
    }

    public boolean isPrimaryNumber(DataKind kind, ValuesDelta entry, RawContactDelta state) {
        String accountType = state.getAccountType();
        return (AccountTypeUtils.isUsimOrCsim(accountType)
                && AccountTypeUtils.isPhoneNumType(kind.mimeType)
                && !SimAasSneUtils.IS_ADDITIONAL_NUMBER.equals(entry.getAsString(
                        MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER)));
    }

    public ArrayList<ValuesDelta> rebuildFromState(RawContactDelta state, String mimeType) {

        String accountType = state.getAccountType();
        if (AccountTypeUtils.isPhoneNumType(mimeType)) {
            ArrayList<ValuesDelta> values = state.getMimeEntries(mimeType);
            if (values != null) {
                ArrayList<ValuesDelta> orderedDeltas = new ArrayList<ValuesDelta>();
                for (ValuesDelta entry : values) {
                    if (isAdditionalNumber(entry)) {
                        orderedDeltas.add(entry);
                    } else {
                        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
                            // add primary number to first.
                            orderedDeltas.add(0, entry);
                        } else {
                            orderedDeltas.add(entry);
                        }
                    }
                }
                return orderedDeltas;
            }
        }
        return state.getMimeEntries(mimeType);
    }

    public boolean updateView(RawContactDelta state, View view, ValuesDelta entry, int action) {
        int type = (entry == null) ? STRING_PRIMART
                : (isAdditionalNumber(entry) ? STRING_ADDITINAL : STRING_PRIMART);
        String accountType = state.getAccountType();
        Log.sensitive(TAG, "[updateView] type=" + type + ",action=" + action + ",accountType="
                + accountType);
        switch (action) {
        case VIEW_UPDATE_HINT:
            if (AccountTypeUtils.isUsimOrCsim(accountType)) {
                if (view instanceof TextView) {
                    if (type == STRING_PRIMART) {
                        ((TextView) view).setHint(R.string.aas_phone_primary);
                    } else if (type == STRING_ADDITINAL) {
                        ((TextView) view).setHint(R.string.aas_phone_additional);
                    }
                } else {
                    Log.e(TAG, "[updateView]  VIEW_UPDATE_HINT but view is not a TextView");
                }
            }
            break;
        case VIEW_UPDATE_VISIBILITY:
            if (AccountTypeUtils.isUsimOrCsim(accountType)) {
                view.setVisibility(View.GONE);
            } else {
                return false;
            }
            break;
        case VIEW_UPDATE_DELETE_EDITOR:
            if (!AccountTypeUtils.isUsimOrCsim(accountType)) {
                return false;
            }
            break;

            default:
                break;
        }
        return true;
    }

    public int getMaxEmptyEditors(RawContactDelta state, String mimeType) {
        String accountType = state.getAccountType();
        Log.sensitive(TAG, "[getMaxEmptyEditors] accountType=" + accountType + ",mimeType=" + mimeType);
        if (AccountTypeUtils.isUsimOrCsim(accountType)
                && AccountTypeUtils.isPhoneNumType(mimeType)) {
            //Host at the entry have already set for subId, we will get subId from simutils
            int subId = SimAasSneUtils.getCurSubId();
            int max = PhbInfoUtils.getUsimAnrCount(subId) + 1;
            Log.d(TAG, "[getMaxEmptyEditors] max=" + max);
            return max;
        }
        Log.d(TAG, "[getMaxEmptyEditors] max= 1");
        return 1;
    }

    public String getCustomTypeLabel(int type, String customColumn) {
        if (AccountTypeUtils.isUsimOrCsim(SimAasSneUtils.getCurAccount())
                && AccountTypeUtils.isAasPhoneType(type)) {
            if (!TextUtils.isEmpty(customColumn)) {
                CharSequence tag = MtkContactsContract.CommonDataKinds.
                            Phone.getTypeLabel(mContext, type, customColumn);
                Log.d(TAG, "[getCustomTypeLabel] index" + customColumn + " tag="
                        + Log.anonymize(tag));
                return tag.toString();
            }
        }
        return null;
    }

    public boolean rebuildLabelSelection(RawContactDelta state, Spinner label,
            ArrayAdapter<EditType> adapter, EditType item, DataKind kind) {
        if (item == null || kind == null) {
            label.setSelection(adapter.getPosition(item));
            return false;
        }
        if (AccountTypeUtils.isUsimOrCsim(state.getAccountType())
                && AccountTypeUtils.isPhoneNumType(kind.mimeType)
                && AccountTypeUtils.isAasPhoneType(item.rawValue)) {
            for (int i = 0; i < adapter.getCount(); i++) {
                EditType type = adapter.getItem(i);
                if (type.customColumn != null && type.customColumn.equals(item.customColumn)) {
                    label.setSelection(i);
                    Log.d(TAG, "[rebuildLabelSelection] position=" + i);
                    return true;
                }
            }
        }
        label.setSelection(adapter.getPosition(item));
        return false;
    }

    public boolean onTypeSelectionChange(RawContactDelta rawContact, ValuesDelta entry,
            DataKind kind, ArrayAdapter<EditType> editTypeAdapter,
            EditType select, EditType type, Context context) {
        String accountType = rawContact.getAccountType();
        Log.sensitive(TAG, "[onTypeSelectionChange] Entry: accountType= " + accountType);
        if (AccountTypeUtils.isUsimOrCsim(accountType)
                && AccountTypeUtils.isPhoneNumType(kind.mimeType)) {
            if (type == select) {
                Log.i(TAG, "[onTypeSelectionChange] same select");
                return true;
            }
            if (Phone.TYPE_CUSTOM == select.rawValue) {
                Log.i(TAG, "[onTypeSelectionChange] Custom Selected");
                onTypeSelectionChange(context, select.rawValue);
            } else {
                type = select; // modifying the type of host app so passed in para
                Log.i(TAG, "[onTypeSelectionChange] different Selected");
                // [ALPS03899165] data type should be String or comparison for changing check will
                // fail
                entry.put(kind.typeColumn, Integer.toString(type.rawValue));
                // insert aas index to entry.
                updatemEntryValue(entry, type);
            }
            return true;
        }
        return false;
    }

    private void onTypeSelectionChange(Context context, int position) {
        Log.d(TAG, "[onTypeSelectionChange] private");
        if (AccountTypeUtils.isUsimOrCsim(SimAasSneUtils.getCurAccount())) {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setAction("com.mediatek.contacts.action.EDIT_AAS");
            int subId = SimAasSneUtils.getCurSubId();
            Log.d(TAG, "[onTypeSelectionChange] internal: subId to fill in slot_key= " + subId);
            intent.putExtra(SimAasSneUtils.KEY_SUB_ID, subId);
            Log.d(TAG, "[onTypeSelectionChange] call for startActivity");
            /*
             * [ALPS03564655] When task is started from other App more than Contacts
             * the activity will start a new task or merge to existing contacts task
             * for mContext is not activity. This will make the activity return to
             * contacts unexpectedly while it is started by others.
             */
            if (context != null) {
                context.startActivity(intent);
            } else {
                mContext.startActivity(intent);
            }
        }
    }

    public EditType getCurrentType(ValuesDelta entry, DataKind kind, int rawValue) {
        if (AccountTypeUtils.isAasPhoneType(rawValue)) {
            return getAasEditType(entry, kind, rawValue);
        }
        return RawContactModifier.getType(kind, rawValue);
    }

    public static boolean updatemEntryValue(ValuesDelta entry, EditType type) {
        if (AccountTypeUtils.isAasPhoneType(type.rawValue)) {
            entry.put(Phone.LABEL, type.customColumn);
            return true;
        }
        return false;
    }

    // -----------for SIMImportProcessor.java start--------------//
    // interface will be called when USIM part are copied from USIM to create
    // master database
    public void updateOperation(String accountType, ContentProviderOperation.Builder builder,
            Cursor cursor, int type) {
        Log.d(TAG, "[updateOperation] Entry type: " + type + ",accountType: " + accountType);
        switch (type) {
        case TYPE_FOR_ADDITIONAL_NUMBER:
            checkAasOperationBuilder(accountType, builder, cursor);
        }
    }

    private boolean checkAasOperationBuilder(String accountType,
            ContentProviderOperation.Builder builder, Cursor cursor) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            int aasColumn = cursor.getColumnIndex("aas");
            Log.d(TAG, "[checkAasOperationBuilder] aasColumn " + aasColumn);
            if (aasColumn >= 0) {
                String aas = cursor.getString(aasColumn);
                Log.d(TAG, "[checkAasOperationBuilder] aas " + Log.anonymize(aas));
                builder.withValue(Data.DATA2, Anr.TYPE_AAS);
                builder.withValue(Data.DATA3, aas);
            }
            return true;
        }
        return false;
    }

    // -----------for SIMImportProcessor.java ends--------------//

    // -----------for CopyProcessor.java starts--------//

    private boolean buildAnrOperation(String accountType,
            ArrayList<ContentProviderOperation> operationList,
            ArrayList anrList, int backRef) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            // build Anr ContentProviderOperation
            for (Object obj : anrList) {
                Anr anr = (Anr) obj;
                if (!TextUtils.isEmpty(anr.additionalNumber)) {
                    Log.d(TAG, "[buildAnrOperation] additionalNumber="
                            + Log.anonymize(anr.additionalNumber) + " aas=" + anr.aasIndex);

                    ContentProviderOperation.Builder builder = ContentProviderOperation
                            .newInsert(Data.CONTENT_URI);
                    builder.withValueBackReference(Phone.RAW_CONTACT_ID, backRef);
                    builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                    builder.withValue(Data.DATA2, Anr.TYPE_AAS);
                    builder.withValue(Phone.NUMBER, anr.additionalNumber);

                    // indicator format: subId + "-" + index
                    String indicator = SimAasSneUtils.buildAASIndicator(anr.aasIndex,
                            SimAasSneUtils.getCurSubId());
                    builder.withValue(Data.DATA3, indicator);

                    builder.withValue(MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
                    operationList.add(builder.build());
                }
            }
            return true;
        }
        return false;
    }

    private int mCopyCount = 0;
    private ArrayList<Anr> additionalArray = new ArrayList<Anr>();

    public void updateValuesforCopy(Uri sourceUri, int subId, String accountType,
            ContentValues simContentValues) {

        Log.d(TAG, "[updateValuesforCopy] Entry");
        SimAasSneUtils.setCurrentSubId(subId);

        if (!AccountTypeUtils.isUsimOrCsim(accountType)) {
            Log.d(TAG, "[updateValuesforCopy] return account is not USIM");
            return;
        }

        mInsertFlag = OPERATION_CONTACT_COPY;
        // if (mCopyUri != sourceUri)
        if (mCopyCount == 0) {
            ArrayList<Anr> phoneArray = new ArrayList<Anr>();

            ContentResolver resolver = mContext.getContentResolver();
            final String[] newProjection = new String[] { Contacts._ID, Contacts.Data.MIMETYPE,
                    Contacts.Data.DATA1, MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER,
                    Contacts.Data.DATA2, Contacts.Data.DATA3 };

            Cursor c = resolver.query(sourceUri, newProjection, null, null, null);
            if (c != null && c.moveToFirst()) {
                do {
                    String mimeType = c.getString(1);
                    if (Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                        String number = c.getString(2);
                        Anr entry = new Anr();

                        entry.additionalNumber = c.getString(2);
                        Log.d(TAG, "[updateValuesforCopy] simAnrNum:"
                                + Log.anonymize(entry.additionalNumber));

                        entry.aasIndex = mContext.getString(Phone
                                .getTypeLabelResource(c.getInt(4)));
                        Log.d(TAG, "[updateValuesforCopy] aasIndex:"
                                + Log.anonymize(entry.aasIndex));

                        if (c.getInt(3) == 1) {
                            additionalArray.add(entry);
                        } else {
                            phoneArray.add(entry);
                        }

                    }
                } while (c.moveToNext());
            }
            if (c != null) {
                c.close();
            }

            if (phoneArray.size() > 0) {
                phoneArray.remove(0); // This entry is handled by host app for
                // primary
                // number
            }
            additionalArray.addAll(phoneArray);
            mCopyCount = additionalArray.size();

        } else {
            additionalArray.remove(0);
            mCopyCount--;
        }

        int uSimMaxAnrCount = PhbInfoUtils.getUsimAnrCount(subId);
        // use this count in case of multiple anr
        int count = additionalArray.size() > uSimMaxAnrCount ? uSimMaxAnrCount : additionalArray
                .size();

        mCopyAnrList = new ArrayList<Anr>();

        for (int i = 0; i < count; i++) {

            Anr entry = additionalArray.remove(0);
            int aasIndex = SimAasSneUtils.getAasIndexByName(entry.aasIndex, subId);
            Log.d(TAG, "[updateValuesforCopy] additionalNumber:"
                    + Log.anonymize(entry.additionalNumber));
            entry.additionalNumber = TextUtils.isEmpty(entry.additionalNumber) ? ""
                    : entry.additionalNumber.replace("-", "");
            Log.d(TAG, "[updateValuesforCopy] aasIndex:" + aasIndex);
            simContentValues.put("anr" + SimAasSneUtils.getSuffix(i),
                    PhoneNumberUtils.stripSeparators(entry.additionalNumber));
            simContentValues.put("aas" + SimAasSneUtils.getSuffix(i), aasIndex);
            mCopyAnrList.add(entry);
            mCopyCount--;
        }

    }

    public boolean cursorColumnToBuilder(Cursor srcCursor, Builder destBuilder,
            String srcAccountType, String srcMimeType, int destSubId, int indexOfColumn) {
        String[] columnNames = srcCursor.getColumnNames();
        return generateDataBuilder(null, srcCursor, destBuilder, columnNames, srcAccountType,
                srcMimeType, destSubId, indexOfColumn);
    }

    public boolean generateDataBuilder(Context context, Cursor dataCursor,
            Builder builder, String[] columnNames, String accountType,
            String mimeType, int destSubId, int index) {
        if (AccountTypeUtils.isAccountTypeIccCard(accountType) &&
            AccountTypeUtils.isPhoneNumType(mimeType)) {
            String isAnr = dataCursor.getString(dataCursor.getColumnIndex(
                    MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER));

            if (Data.DATA2.equals(columnNames[index])) {
                Log.d(TAG, "[generateDataBuilder] isAnr:" + isAnr);
                if ("1".equals(isAnr)) {
                    builder.withValue(Data.DATA2, Phone.TYPE_OTHER);
                    Log.d(TAG, "[generateDataBuilder] DATA2 to be TYPE_OTHER ");
                } else {
                    builder.withValue(Data.DATA2, Phone.TYPE_MOBILE);
                    Log.d(TAG, "[generateDataBuilder] DATA2 to be TYPE_MOBILE ");
                }
                return true;
            }
            if (Data.DATA3.equals(columnNames[index])) {
                Log.d(TAG, "[generateDataBuilder] DATA3 to be null");
                builder.withValue(Data.DATA3, null);
                return true;
            }
        }
        return false;
    }

    // -----------for CopyProcessor.java ends--------//
    // -------------------for SIMEditProcessor.java starts-------------//
    // this interface to check whether a entry is of additional number or not
    // for SIM & USIM
    public boolean checkAasEntry(ContentValues cv) {
        Log.sensitive(TAG, "[checkAasEntry] para = " + cv);
        if (isAdditionalNumber(cv)) {
            return true;
        }
        return false;
    }

    public String getSubheaderString(int subId, int type) {
        Log.d(TAG, "[getSubheaderString] subId = " + subId);
        if (subId == -1) {
            Log.d(TAG, "[getSubheaderString] Phone contact");
            return null;
        }
        String accountType = AccountTypeUtils.getAccountTypeBySub(subId);
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            if (AccountTypeUtils.isAasPhoneType(type)) {
                Log.d(TAG, "[getSubheaderString] USIM additional number");
                return mContext.getResources().getString(R.string.aas_phone_additional);

            } else {
                Log.d(TAG, "[getSubheaderString] USIM primary number ");
                return mContext.getResources().getString(R.string.aas_phone_primary);
            }
        }
        Log.d(TAG, "[getSubheaderString] Account is SIM ");
        return null;

    }

    // this interface to update additional number & aasindex while writing or
    // updating the USIMcard contact
    public boolean updateValues(Intent intent, int subId, ContentValues contentValues) {
        Log.d(TAG, "[updateValues] Entry.");
        ArrayList<RawContactDelta> newSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_SIM_DATA);
        ArrayList<RawContactDelta> oldSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_OLD_SIM_DATA);
        String accountType = newSimData.get(0).getValues().getAsString(RawContacts.ACCOUNT_TYPE);
        SimAasSneUtils.setCurrentSubId(subId);
        if (!AccountTypeUtils.isUsimOrCsim(accountType)) {
            Log.d(TAG, "[updateValues] Account type is not USIM.");
            return false;
        }
        // case of new contact
        if (oldSimData == null) {
            // put values for anr
            // aas as anr.aasIndex
            // set the mInsertFlag to insert, will be used later in
            // updateoperationList, prepare newanrlist
            Log.d(TAG, "[updateValues] for new contact.");
            mInsertFlag = OPERATION_CONTACT_INSERT;
            prepareNewAnrList(intent);
            Log.d(TAG, "[updateValues] for new contact Newanrlist filled");
            return buildAnrInsertValues(accountType, contentValues, mAnrsList);
        }
        // case of edit contact
        else {
            // put values for newAnr
            // aas as anr.aasIndex
            // set the mInsertFlag to edit, prepare old & new both anr list
            Log.d(TAG, "[updateValues] for Edit contact.");
            mInsertFlag = OPERATION_CONTACT_EDIT;
            prepareNewAnrList(intent);
            Log.d(TAG, "[updateValues] for New anrlist filled");
            prepareOldAnrList(intent);
            Log.d(TAG, "[updateValues] for Old anrlist filled");
            return buildAnrUpdateValues(accountType, contentValues, mAnrsList);
        }

    }

    private void prepareNewAnrList(Intent intent) {
        ArrayList<RawContactDelta> newSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_SIM_DATA);
        mAnrsList.clear();
        if (newSimData == null) {
            return;
        }
        // now fill tha data for newanrlist
        int kindCount = newSimData.get(0).getContentValues().size();
        String mimeType = null;
        for (int countIndex = 0; countIndex < kindCount; countIndex++) {
            mimeType = newSimData.get(0).getContentValues().get(countIndex)
                    .getAsString(Data.MIMETYPE);
            if (Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                final ContentValues cv = newSimData.get(0).getContentValues().get(countIndex);
                if (isAdditionalNumber(cv)) {
                    Anr addPhone = new Anr();
                    addPhone.additionalNumber = replaceCharOnNumber(cv.getAsString(Data.DATA1));
                    addPhone.aasIndex = cv.getAsString(Data.DATA3);
                    Log.d(TAG, "[prepareNewAnrList] additionalNumber:"
                            + Log.anonymize(addPhone.additionalNumber)
                            + ",aasIndex:" + addPhone.aasIndex);
                    mAnrsList.add(addPhone);
                }
            }
        }
    }

    private void prepareOldAnrList(Intent intent) {
        ArrayList<RawContactDelta> oldSimData = intent.getParcelableArrayListExtra(
                SimServiceUtils.KEY_OLD_SIM_DATA);
        mOldAnrsList.clear();
        if (oldSimData == null) {
            return;
        }
        // now fill the data for oldAnrlist
        int oldCount = oldSimData.get(0).getContentValues().size();
        String mimeType = null;
        for (int oldIndex = 0; oldIndex < oldCount; oldIndex++) {
            mimeType = oldSimData.get(0).getContentValues().get(oldIndex)
                    .getAsString(Data.MIMETYPE);
            if (Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                ContentValues cv = oldSimData.get(0).getContentValues().get(oldIndex);
                if (isAdditionalNumber(cv)) {
                    Anr addPhone = new Anr();
                    addPhone.additionalNumber = replaceCharOnNumber(cv.getAsString(Data.DATA1));
                    addPhone.aasIndex = cv.getAsString(Phone.DATA3);
                    Log.d(TAG, "[prepareOldAnrList] additionalNumber:"
                            + Log.anonymize(addPhone.additionalNumber)
                            + ",aasIndex: " + addPhone.aasIndex);
                    addPhone.id = cv.getAsInteger(Data._ID);
                    mOldAnrsList.add(addPhone);
                }
            }
        }
    }

    private boolean buildAnrInsertValues(String accountType,
            ContentValues values, ArrayList anrsList) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            int count = 0;
            for (Object obj : anrsList) {
                Anr anr = (Anr) obj;
                String additionalNumber = TextUtils.isEmpty(anr.additionalNumber) ? ""
                        : anr.additionalNumber;
                String additionalNumberToInsert = additionalNumber;
                if (!TextUtils.isEmpty(additionalNumber)) {
                    additionalNumberToInsert = PhoneNumberUtils.stripSeparators(additionalNumber);
                    Log.d(TAG, "[buildAnrInsertValues] additionalNumber updated : "
                            + Log.anonymize(additionalNumberToInsert));
                }
                values.put("anr" + SimAasSneUtils.getSuffix(count), additionalNumberToInsert);
                values.put("aas" + SimAasSneUtils.getSuffix(count),
                        SimAasSneUtils.getAasIndexFromIndicator(anr.aasIndex));
                count++;
                Log.d(TAG, "[buildAnrInsertValues] aasIndex=" + anr.aasIndex
                        + ", additionalNumber=" + Log.anonymize(additionalNumber));
            }
            return true;
        }
        return false;
    }

    private boolean buildAnrUpdateValues(String accountType,
            ContentValues updatevalues, ArrayList<Anr> anrsList) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            int count = 0;
            for (Anr anr : anrsList) {
                Log.d(TAG, "[buildAnrUpdateValues] additionalNumber : "
                        + Log.anonymize(anr.additionalNumber));
                if (!TextUtils.isEmpty(anr.additionalNumber)) {
                    String additionalNumber = anr.additionalNumber;
                    String additionalNumberToInsert = additionalNumber;
                    additionalNumberToInsert = PhoneNumberUtils
                            .stripSeparators(additionalNumber);
                    Log.d(TAG,
                            "[buildAnrUpdateValues] additionalNumber updated: "
                                    + Log.anonymize(additionalNumberToInsert));
                    updatevalues.put("newAnr" + SimAasSneUtils.getSuffix(count),
                            additionalNumberToInsert);
                    updatevalues.put("aas" + SimAasSneUtils.getSuffix(count),
                            SimAasSneUtils.getAasIndexFromIndicator(anr.aasIndex));
                }
                count++;
            }
            return true;
        }
        return false;
    }

    public boolean updateAdditionalNumberToDB(Intent intent, long rawContactId) {
        Log.d(TAG, "[updateAdditionalNumberToDB] Entry");
        ArrayList<RawContactDelta> newSimData = null;
        newSimData = intent.getParcelableArrayListExtra(SimServiceUtils.KEY_SIM_DATA);
        String accountType = newSimData.get(0).getValues()
                .getAsString(RawContacts.ACCOUNT_TYPE);

        if (!AccountTypeUtils.isUsimOrCsim(accountType)) {
            Log.d(TAG,
                    "[updateAdditionalNumberToDB] return false, account is not USIM");
            return false;
        }
        ContentResolver resolver = mContext.getContentResolver();
        // amit todo check whether passed anrlist & oldanr list are already
        // filled correctly by prevuious interfaces or
        // do we need to take it from intent
        Log.sensitive(TAG, "[updateAdditionalNumberToDB] mAnrlist:" + mAnrsList + ",mOldAnrsList: "
                + mOldAnrsList);
        return updateAnrToDb(accountType, resolver, mAnrsList, mOldAnrsList,
                rawContactId);

    }

    private boolean updateAnrToDb(String accountType, ContentResolver resolver,
            ArrayList anrsList, ArrayList oldAnrsList, long rawId) {
        if (AccountTypeUtils.isUsimOrCsim(accountType)) {
            String whereClause = Data.RAW_CONTACT_ID + "=\'" + rawId
                    + "\'" + " AND " + Data.MIMETYPE + "='"
                    + Phone.CONTENT_ITEM_TYPE + "'" + " AND "
                    + MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER
                    + "=1" + " AND " + Data._ID + "=?";
            Log.sensitive(TAG, "[updateAnrInfoToDb] whereClause:" + whereClause);

            // Here, mAnrsList.size() should be the same as mOldAnrsList.size()
            int newSize = anrsList.size();
            int oldSize = oldAnrsList.size();
            int count = Math.min(newSize, oldSize);
            String additionalNumber;
            String aas;
            String oldAdditionalNumber;
            String oldAas;
            ContentValues additionalValues = new ContentValues();

            int i = 0;
            for (; i < count; i++) {
                Anr newAnr = (Anr) anrsList.get(i);
                Anr oldAnr = (Anr) oldAnrsList.get(i);

                additionalValues.clear();
                if (!TextUtils.isEmpty(newAnr.additionalNumber)
                        && !TextUtils.isEmpty(oldAnr.additionalNumber)) { // update
                    additionalValues.put(Phone.NUMBER, newAnr.additionalNumber);
                    additionalValues.put(Data.DATA2, Anr.TYPE_AAS);
                    additionalValues.put(Data.DATA3, newAnr.aasIndex);

                    int updatedCount = resolver.update(Data.CONTENT_URI, additionalValues,
                            whereClause, new String[]{ Long.toString(oldAnr.id) });
                    Log.d(TAG, "[updateAnrInfoToDb] updatedCount: " + updatedCount);
                } else if (!TextUtils.isEmpty(newAnr.additionalNumber)
                        && TextUtils.isEmpty(oldAnr.additionalNumber)) { // insert
                    additionalValues.put(Phone.RAW_CONTACT_ID, rawId);
                    additionalValues
                            .put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                    additionalValues.put(Phone.NUMBER, newAnr.additionalNumber);
                    additionalValues.put(Data.DATA2, Anr.TYPE_AAS);
                    additionalValues.put(Data.DATA3, newAnr.aasIndex);
                    additionalValues.put(
                        MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);

                    Uri insertedUri = resolver.insert(Data.CONTENT_URI,
                            additionalValues);
                    Log.d(TAG, "[updateAnrInfoToDb] upAdditionalUri: " + insertedUri);
                } else if (TextUtils.isEmpty(newAnr.additionalNumber)) { // delete
                    int deletedCount = resolver.delete(Data.CONTENT_URI,
                            whereClause, new String[]{ Long.toString(oldAnr.id) });
                    Log.d(TAG, "[updateAnrInfoToDb] deletedCount:" + deletedCount);
                }
            }

            // in order to avoid error, do the following operations.
            while (i < oldSize) { // delete one
                Anr oldAnr = (Anr) oldAnrsList.get(i);
                int deleteAdditional = resolver.delete(Data.CONTENT_URI, whereClause,
                        new String[]{ Long.toString(oldAnr.id)});
                Log.d(TAG, "[updateAnrInfoToDb] deleteAdditional:" + deleteAdditional);
                i++;
            }

            while (i < newSize) { // insert one
                Anr newAnr = (Anr) anrsList.get(i);
                additionalValues.clear();
                if (!TextUtils.isEmpty(newAnr.additionalNumber)) {
                    additionalValues.put(Phone.RAW_CONTACT_ID, rawId);
                    additionalValues
                            .put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
                    additionalValues.put(Phone.NUMBER, newAnr.additionalNumber);
                    additionalValues.put(Data.DATA2, Anr.TYPE_AAS);
                    additionalValues.put(Data.DATA3, newAnr.aasIndex);
                    additionalValues.put(
                        MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);

                    Uri insertedUri = resolver.insert(Data.CONTENT_URI,
                            additionalValues);
                    Log.d(TAG, "[updateAnrInfoToDb] insertedUri: " + insertedUri);
                }
                i++;
            }
            return true;
        }
        return false;
    }

    // writing a common api interface for copy & insert as we have to update the
    // operation list, based on mInsertFlag will decide which list have to
    // process
    public boolean updateOperationList(AccountWithDataSet accounType,
            ArrayList<ContentProviderOperation> operationList, int backRef) {
        // bassed on op we have to decide which list we have to parse
        // for copy
        if (!AccountTypeUtils.isUsimOrCsim(accounType.type)) {
            Log.d(TAG,
                    "[updateOperationList] Account is not USIM so return false");
            return false;
        }

        if (mInsertFlag == OPERATION_CONTACT_COPY) {
            if (mCopyAnrList != null && mCopyAnrList.size() > 0) {
                Log.d(TAG, "[updateOperationList] for copy ");
                boolean result = buildAnrOperation(accounType.type,
                        operationList, mCopyAnrList, backRef);
                Log.d(TAG, "[updateOperationList] result : " + result);
                mCopyAnrList.clear();
                mCopyAnrList = null;
                return result;
            }
            Log.d(TAG, "[updateOperationList] result false");
            return false;

        }

        // for insert
        else {
            if (AccountTypeUtils.isUsimOrCsim(accounType.type)) {
                Log.d(TAG, "[updateOperationList] for Insert contact ");
                // build Anr ContentProviderOperation
                for (Object obj : mAnrsList) {
                    Anr anr = (Anr) obj;
                    if (!TextUtils.isEmpty(anr.additionalNumber)) {
                        Log.d(TAG, "[updateOperationList] additionalNumber="
                                + Log.anonymize(anr.additionalNumber) + ",aas=" + anr.aasIndex);

                        ContentProviderOperation.Builder builder = ContentProviderOperation
                                .newInsert(Data.CONTENT_URI);
                        builder.withValueBackReference(Phone.RAW_CONTACT_ID,
                                backRef);
                        builder.withValue(Data.MIMETYPE,
                                Phone.CONTENT_ITEM_TYPE);
                        builder.withValue(Data.DATA2, Anr.TYPE_AAS);
                        builder.withValue(Phone.NUMBER, anr.additionalNumber);
                        builder.withValue(Data.DATA3, anr.aasIndex);

                        builder.withValue(
                            MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
                        operationList.add(builder.build());
                    }
                }
                Log.d(TAG, "[updateOperationList] result true");
                return true;
            }

        }
        Log.d(TAG, "[updateOperationList] result false");
        return false;

    }

    public CharSequence getLabelForBindData(Resources res, int type,
            String customLabel, String mimeType, Cursor cursor,
            CharSequence defaultValue) {

        Log.d(TAG, "[getLabelForBindData] Entry mimetype:" + mimeType);
        CharSequence label = defaultValue;
        final int indicate = cursor.getColumnIndex(
                MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM);
        int subId = -1;
        if (indicate != -1) {
            subId = cursor.getInt(indicate);
        }
        String accountType = AccountTypeUtils.getAccountTypeBySub(subId);
        if (AccountTypeUtils.isUsimOrCsim(accountType)
                && mimeType.equals(Email.CONTENT_ITEM_TYPE)) {
            label = "";
        } else {

            label = getTypeLabel(type, (CharSequence) customLabel,
                    (String) defaultValue, subId);

        }
        return label;

    }

    private String replaceCharOnNumber(String number) {
        String trimNumber = number;
        if (!TextUtils.isEmpty(trimNumber)) {
            Log.d(TAG, "[replaceCharOnNumber]befor replaceall number : "
                    + Log.anonymize(trimNumber));
            trimNumber = trimNumber.replaceAll("-", "");
            trimNumber = trimNumber.replaceAll(" ", "");
            Log.d(TAG, "[replaceCharOnNumber]after replaceall number : "
                    + Log.anonymize(trimNumber));
        }
        return trimNumber;
    }

    public CharSequence getTypeLabel(int type, CharSequence label,
            String defvalue, int subId) {
        String accountType = AccountTypeUtils.getAccountTypeBySub(subId);
        Log.sensitive(TAG, "[getTypeLabel] subId=" + subId + " accountType="
                + accountType);
        if (AccountTypeUtils.isSim(accountType) || AccountTypeUtils.isRuim(accountType)) {
            Log.d(TAG, "[getTypeLabel] SIM Account no Label.");
            return "";
        }
        if (AccountTypeUtils.isUsimOrCsim(accountType) && AccountTypeUtils.isAasPhoneType(type)) {
            if (TextUtils.isEmpty(label)) {
                Log.d(TAG, "[getTypeLabel] Empty");
                return "";
            }
            try {
                CharSequence tag = MtkContactsContract.CommonDataKinds.
                            Phone.getTypeLabel(mContext, type, label);
                Log.d(TAG, "[getTypeLabel] label" + Log.anonymize(label)
                        + " tag=" + Log.anonymize(tag));
                return tag;
            } catch (NumberFormatException e) {
                Log.e(TAG, "[getTypeLabel] return label=" + label);
            }
        }
        if (AccountTypeUtils.isUsimOrCsim(accountType) && !AccountTypeUtils.isAasPhoneType(type)) {
            Log.d(TAG, "[getTypeLabel] account is USIM but type is not additional");
            return "";
        }
        return defvalue;
    }

    // Amit ends

    private boolean isAdditionalNumber(ValuesDelta entry) {
        final String key = MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER;
        Integer isAnr = entry.getAsInteger(key);
        return isAnr != null && 1 == isAnr.intValue();
    }

    private boolean isAdditionalNumber(final ContentValues cv) {
        final String key = MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER;
        Integer isAnr = null;
        if (cv != null && cv.containsKey(key)) {
            isAnr = cv.getAsInteger(key);
        }
        return isAnr != null && 1 == isAnr.intValue();
    }

    private EditType getAasEditType(ValuesDelta entry, DataKind kind,
            int phoneType) {
        if (phoneType == Anr.TYPE_AAS) {
            String customColumn = entry.getAsString(Data.DATA3);
            Log.d(TAG, "[getAasEditType] customColumn=" + customColumn);
            if (customColumn != null) {
                for (EditType type : kind.typeList) {
                    if (type.rawValue == Anr.TYPE_AAS
                            && customColumn.equals(type.customColumn)) {
                        return type;
                    }
                }
            }
            return null;
        }
        Log.e(TAG, "[getAasEditType] error Not Anr.TYPE_AAS, type=" + phoneType);
        return null;
    }

    public void ensurePhoneKindForCompactEditor(RawContactDeltaList state,
            int subId, Context context) {
        int numRawContacts = state.size();
        Log.d(TAG, "[ensurePhoneKindForCompactEditor] Entry numRawContacts= " + numRawContacts);
        final AccountTypeManager accountTypes = AccountTypeManager.getInstance(mContext);
        for (int i = 0; i < numRawContacts; i++) {
            final RawContactDelta rawContactDelta = state.get(i);
            final AccountType type = rawContactDelta.getAccountType(accountTypes);
            Log.d(TAG, "[ensurePhoneKindForCompactEditor] loop subid=" + subId);
            ensurePhoneKindForEditor(type, subId, rawContactDelta);
        }
    }

    /*
     * fix ALPS02795593.some usim cannot support aas and anr.when change account using
     * oldstate which have addition number column.so should remove it by get anr count.@{
     */
    private void removeAnrFieldIfNotSupportAnr(RawContactDelta state, ArrayList<ValuesDelta> values,
            int slotAnrSize) {
        Log.sensitive(TAG, "[removeAnrFieldIfNotSupportAnr] state:" + state + ",values:" + values +
                ",slotAnrSize:" + slotAnrSize);
        if ((state == null) || (values == null)) {//should judge vaules is or null
            Log.w(TAG, "[removeAnrFieldIfNotSupportAnr] state or value is null,return");
            return;
        }
        final boolean isSupportAnr = slotAnrSize > 0 ? true : false;
        if (!isSupportAnr) {//not support addition number,so remove related Entry if exist
            Iterator<ValuesDelta> iterator = values.iterator();
            while (iterator.hasNext()) {
                ValuesDelta value = iterator.next();
                Integer isAnr = value.getAsInteger(
                    MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER);
                if (isAnr != null && (isAnr.intValue() == 1)) {
                    Log.sensitive(TAG, "[removeAnrFieldIfNotSupportAnr] remove vaule: " + value);
                    iterator.remove();
                }
            }
            Log.sensitive(TAG, "[removeAnrFieldIfNotSupportAnr] after state:" + state);
        }
    }
    /* @} */

    /*
     * ALPS03057922. we should remove the vaule including addition_number cloumn for NonSimType
     * ex: local phone & exchange account.@{
     */
    public static void removeRedundantAnrFieldForNonSimAccount(AccountType oldAccountType,
        AccountType newAccountType, RawContactDelta newState, String mimeType) {
        Log.sensitive(TAG, "[removeRedundantAnrFieldForNonSimAccount] oldAccountType:" + oldAccountType +
                ",newAccountType: " + newAccountType + ",newState: " + newState + ",mimeType:"
                + mimeType);
        if ((newAccountType != null) && !AccountTypeUtils.isAccountTypeIccCard(
                newAccountType.accountType)) {
            ArrayList<ValuesDelta> values = newState.getMimeEntries(mimeType);
            removeAnrValueDirectly(values);
        }
        Log.sensitive(TAG, "[removeRedundantAnrFieldForNonSimAccount] result newState:" + newState);
    }

    private static void removeAnrValueDirectly(ArrayList<ValuesDelta> values) {
        Log.d(TAG, "[removeAnrValueDirectly]");
        if (values == null) {
            Log.w(TAG, "[removeAnrValueDirectly] null values,return!");
            return;
        }
        for (ValuesDelta value : values) {
            String data1 = value.getAsString(ContactsContract.Data.DATA1);//get data1 value
            boolean isDataInvalid = TextUtils.isEmpty(data1);
            Integer isAnr = value.getAsInteger(
                MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER);
            boolean canBeRemove = (isAnr != null) && (isAnr.intValue() == 1);
            if (isDataInvalid && canBeRemove) {
                Log.sensitive(TAG, "[removeAnrFieldDirectly] remove Anr value:" + value);
                values.remove(value);
            }
        }
    }
    /// @}

    private static class AasIndicatorNamePair {
        public String indicator; // AAS's indicator(e.g. subId-index="2-1") in DB
        public String name; // AAS name(e.g."AAS1") in SIM

        public AasIndicatorNamePair(String indicator, String name) {
            this.indicator = indicator;
            this.name = name;
        }

        @Override
        public String toString() {
            return "{indicator = " + this.indicator + ", name = " + this.name + "}";
        }
    }

    public void setOldAasIndicatorAndNames(RawContactDeltaList state) {
        if (mOldAasIndNameList != null) {
            mOldAasIndNameList.clear();
            mOldAasIndNameList = null;
        }
        mOldAasIndNameList = getAasIndicatorAndNames(state);
    }

    private ArrayList<AasIndicatorNamePair> getAasIndicatorAndNames(
            RawContactDeltaList state) {
        if (state == null) {
            return null;
        }
        ArrayList<ContentValues> dataList = state.get(0).getContentValues();
        int rowCount = dataList.size();
        ArrayList<AasIndicatorNamePair> pairList = null;
        String mimeType = null;
        for (int i = 0; i < rowCount; i++) {
            mimeType = dataList.get(i).getAsString(Data.MIMETYPE);
            if (!Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                continue;
            }
            ContentValues data = dataList.get(i);
            if (isAdditionalNumber(data)) {
                if (pairList == null) {
                    pairList = new ArrayList<AasIndicatorNamePair>();
                }
                String indicator = data.getAsString(Phone.DATA3);
                pairList.add(new AasIndicatorNamePair(indicator,
                        SimAasSneUtils.getAASByIndicator(indicator)));
            }
        }
        return pairList;
    }

    /**
     * Check if contact's AAS names changed only (AAS indicators are not changed).
     * @param curState Current state of contact.
     */
    public boolean isAasNameChangedOnly(RawContactDeltaList curState) {
        if (curState == null || curState.isEmpty()
                || !AccountTypeUtils.isUsimOrCsim(curState.get(0).getAccountType())) {
            return false;
        }
        ArrayList<AasIndicatorNamePair> curAasIndNameList = getAasIndicatorAndNames(curState);
        Log.sensitive(TAG, "[isAasNameChangedOnly] mOldAasIndNameList = "
                + mOldAasIndNameList + ", curAasIndNameList = " + curAasIndNameList);

        if (mOldAasIndNameList == null || curAasIndNameList == null
                || mOldAasIndNameList.size() != curAasIndNameList.size()) {
            return false;
        }
        int size = mOldAasIndNameList.size();
        boolean hasNameChanged = false;
        for (int i = 0; i < size; i++) {
            String oldInd = mOldAasIndNameList.get(i).indicator;
            String curInd = curAasIndNameList.get(i).indicator;
            if (oldInd == null && curInd == null) {
                // additional number originally and currently has no AAS, so no change.
                continue;
            }
            if (oldInd == null || curInd == null || !oldInd.equals(curInd)) {
                // AAS indicator in DB is changed, no need to compare AAS name
                return false;
            }
            if (!mOldAasIndNameList.get(i).name.equals(curAasIndNameList.get(i).name)) {
                Log.d(TAG, "[isAasNameChangedOnly] i = " + i);
                hasNameChanged = true;
            }
        }
        // now indicators are all not changed
        Log.d(TAG, "[isAasNameChangedOnly] return " + hasNameChanged);
        return hasNameChanged;
    }
}
