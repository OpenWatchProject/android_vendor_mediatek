package com.mediatek.ims.config.internal;

import android.content.BroadcastReceiver;
import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteFullException;
import android.os.Build;
import android.os.SystemProperties;
import android.os.UserManager;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.ims.ImsConfig;
import com.mediatek.ims.config.ImsConfigContract;
import static com.mediatek.ims.config.ImsConfigContract.Validator;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
/**
 * Provider class to handle DUI operation for IMS configuration databases, including
 * 1.) TABLE_CONFIG_SETTING: Basic settings for IMS configuration.
 * 2.) TABLE_DEFAULT: The default value of each configuration items, which will be loaded from
 *     xml resource file according to carrier's customization.
 * 3.) TABLE_PROVISION: The provisioned record from carrier.
 * 4.) TABLE_MASTER: The final result (cache) which combined with default & provisioned value.
 *     Items without setting any value will filled with value ImsConfigContract.VALUE_NO_DEFAULT,
 *     and application will get ImsException when calling getProvisioned(String)Value() API for
 *     such kind of configuration items.
 */
final public class ImsConfigProvider extends ContentProvider {
    private static final String TAG = "ImsConfigProvider";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    static final String AUTHORITY = ImsConfigContract.AUTHORITY;
    static HashMap<Integer, String> LatestSimState = new HashMap<Integer, String>();
    static HashMap<Integer, Boolean> ECCAllowSendCmd = new HashMap<Integer, Boolean>();
    static HashMap<Integer, Boolean> ECCAllowBroadcast = new HashMap<Integer, Boolean>();
    private Context mContext;

    SqlDatabaseHelper mDatabaseHelper = null;

    /// M: Sync volte setting value. @{
    private static final String PROPERTY_IMSCONFIG_FORCE_NOTIFY =
                                  "vendor.ril.imsconfig.force.notify";
    /// @}

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "[onReceive] action=" + intent.getAction());

            final Context deviceContext = context.createDeviceProtectedStorageContext();

            if (!deviceContext.moveDatabaseFrom(context, SqlDatabaseHelper.DATABASE_NAME)) {
                Log.wtf(TAG, "[onReceive] Failed to move database");
            }
            Log.d(TAG, "[onReceive] Move database successfully");

            mDatabaseHelper = new SqlDatabaseHelper(deviceContext);
            Log.d(TAG, "[onReceive] Create mDatabaseHelper again");

            context.unregisterReceiver(mBroadcastReceiver);
        }
    };

    @Override
    public boolean onCreate() {
        final Context context = getContext();
        final Context deviceContext = context.createDeviceProtectedStorageContext();

        if (UserManager.get(context).isUserUnlocked()) {
            if (!deviceContext.moveDatabaseFrom(context, SqlDatabaseHelper.DATABASE_NAME)) {
                Log.wtf(TAG, "[onCreate] Failed to move database");
            }
            Log.d(TAG, "[onCreate] Move database successfully");
        } else {
            Log.d(TAG, "[onCreate] User locked, register receiver for BOOT_COMPLETED");

            final IntentFilter filter = new IntentFilter(Intent.ACTION_BOOT_COMPLETED);
            context.registerReceiver(mBroadcastReceiver, filter);
        }

        mDatabaseHelper = new SqlDatabaseHelper(deviceContext);
        mContext = getContext();
        return true;
    }

    @Override
    public String getType(Uri uri) {
        return "vnd.android.cursor.item/imsconfig";
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        int result = 0;
        Arguments args = new Arguments(OperationMode.MODE_DELETE, uri, selection, selectionArgs);

        try {
            SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();
            result = db.delete(args.table, args.selection, args.selectionArgs);
        } catch (SQLiteFullException e) {
            onDiskFull(e);
        }
        return result;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        Uri result = null;
        Arguments args = new Arguments(OperationMode.MODE_INSERT, values, uri);
        long newId = 0L;

        try {
            SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();
            newId = db.insertWithOnConflict(
                    args.table, null, values, SQLiteDatabase.CONFLICT_REPLACE);
        } catch (SQLiteFullException e) {
            onDiskFull(e);
        }
        return prepareResultUri(args, newId);
    }

    private Uri prepareResultUri(Arguments args, long newId) {
        Uri result = Uri.parse("content://" + AUTHORITY + "/" + args.table + "/" + args.phoneId);
        switch (args.table) {
            case ImsConfigContract.TABLE_DEFAULT:
            case ImsConfigContract.TABLE_PROVISION:
            case ImsConfigContract.TABLE_MASTER:
                result = Uri.withAppendedPath(result, args.itemId);
                break;
            default:
                result = ContentUris.withAppendedId(result, newId);
                break;
        }
        if (!TextUtils.isEmpty(args.param)) {
            result = ContentUris.withAppendedId(result, Integer.parseInt(args.param));
        }
        return result;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
                      String[] selectionArgs) {
        int count = 0;
        Arguments args = new Arguments(
                OperationMode.MODE_UPDATE, uri, values, selection, selectionArgs);

        try {
            SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();
            count = db.update(args.table, values, args.selection, args.selectionArgs);
            if (count > 0) {
                notifyChange(uri, args, values);
            }
        } catch (SQLiteFullException e) {
            onDiskFull(e);
        }
        return count;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
                        String[] selectionArgs, String sortOrder) {
        Cursor result = null;
        Arguments args = new Arguments(OperationMode.MODE_QUERY, uri);
        SQLiteDatabase db = mDatabaseHelper.getReadableDatabase();

        final int MAX_COLUMN_NAME_LENGTH = 64;
        Pattern pattern = Pattern.compile("[\\W]");// Find any non-word characters.

        for (String projectionArg : projection) {
            if (projectionArg.length() > MAX_COLUMN_NAME_LENGTH) {
                Log.e(TAG, "Found column name that was too long (" +
                            projectionArg.length() + " characters)");
                return null;
            }

            Matcher matcher = pattern.matcher(projectionArg);
            if (matcher.find()) {
                Log.e(TAG, "Found invalid character in column name: " + projectionArg +
                           ", uri: " + uri);
                return null;
            }
        }

        result = db.query(args.table,
                projection,
                args.selection,
                args.selectionArgs,
                null,
                null,
                null);
        return result;
    }

    @Override
    public void shutdown() {
        super.shutdown();
        if (mDatabaseHelper != null) {
            mDatabaseHelper.close();
        }
    }

    private void notifyChange(Uri uri, Arguments args, ContentValues cv) {
        //final long oldId = Binder.clearCallingIdentity();
        try {
            int itemId, mimeType;
            String simState;
            int value = -1;
            boolean isECCBroadcastFlag = false;
            boolean isFeatureEnabled = false;
            boolean isSimStateAllow = false;
            String valueStr = "";
            Intent intent;

            switch (args.table) {
                case ImsConfigContract.TABLE_DEFAULT:
                case ImsConfigContract.TABLE_MASTER:
                case ImsConfigContract.TABLE_PROVISION:
                    itemId = ImsConfigContract.configNameToId(args.itemId);

                    // For getting the updated value, should check whether is int / string!
                    mimeType = cv.getAsInteger(ImsConfigContract.BasicConfigTable.MIMETYPE_ID);
                    if (mimeType == ImsConfigContract.MimeType.INTEGER) {
                        value = cv.getAsInteger(ImsConfigContract.BasicConfigTable.DATA);
                    } else if (mimeType == ImsConfigContract.MimeType.STRING) {
                        valueStr = cv.getAsString(ImsConfigContract.BasicConfigTable.DATA);
                    } else {
                        Log.e(TAG, "Config " + itemId + " not String or Integer, mimeType " + mimeType);
                        break;
                    }
                    // For observers who don't have dedicated process, use broadcast mechanism.
                    intent = new Intent(ImsConfigContract.ACTION_IMS_CONFIG_CHANGED, uri);
                    intent.putExtra(ImsConfigContract.EXTRA_PHONE_ID, Integer.parseInt(args.phoneId));
                    intent.putExtra(ImsConfigContract.EXTRA_CHANGED_ITEM, itemId);
                    intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

                    if (mimeType == ImsConfigContract.MimeType.INTEGER) {
                        intent.putExtra(ImsConfigContract.EXTRA_NEW_VALUE, value);
                    } else if (mimeType == ImsConfigContract.MimeType.STRING) {
                        intent.putExtra(ImsConfigContract.EXTRA_NEW_VALUE, valueStr);
                    }
                    intent.putExtra(ImsConfigContract.EXTRA_MIMETYPE, mimeType);
                    getContext().sendBroadcast(intent);
                    // Notify content observers
                    getContext().getContentResolver().notifyChange(uri, null);

                    if (mimeType == ImsConfigContract.MimeType.INTEGER && DEBUG) {
                        Log.d(TAG, "Update uri " + uri + " on phone " + args.phoneId +
                                " value: " + value);
                    } else if (mimeType == ImsConfigContract.MimeType.STRING && DEBUG) {
                        Log.d(TAG, "Update uri " + uri + " on phone " + args.phoneId +
                                " value: " + valueStr);
                    }
                    break;
                case ImsConfigContract.TABLE_CONFIG_SETTING:
                    itemId = Integer.parseInt(args.itemId);
                    Log.d(TAG, "Don't Update uri " + uri + " on phone " + args.phoneId +
                                " for CONFIG_SETTING");
                    break;
                case ImsConfigContract.TABLE_FEATURE:
                    simState = LatestSimState.get(Integer.parseInt(args.phoneId));
                    Log.d(TAG, "getSimState() for checking whether broadcast phoneId: " +
                            Integer.parseInt(args.phoneId) + ", Sim state: " + simState);

                    itemId = Integer.parseInt(args.itemId);
                    value = cv.getAsInteger(ImsConfigContract.Feature.VALUE);

                    if (simState == null) {
                        simState = "";
                    }

                    // ECCAllowBroadcast: The flag is used to allow broadcast for PS ECC
                    // when sim is absent and the calculated platform support of VoLTE is true
                    if (ECCAllowBroadcast.get(Integer.parseInt(args.phoneId)) == null) {
                        isECCBroadcastFlag = false;
                    } else {
                        isECCBroadcastFlag = (simState.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT) &&
                                ECCAllowBroadcast.get(Integer.parseInt(args.phoneId)) &&
                                itemId == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE);
                        isFeatureEnabled = (value == ImsConfig.FeatureValueConstants.ON);
                        if (isECCBroadcastFlag && !isFeatureEnabled) {
                            ECCAllowBroadcast.put(Integer.parseInt(args.phoneId), false);
                            Log.d(TAG, "Sim absent but the calculated VoLTE is false," +
                                    " so no need broadcast");
                        }
                    }

                    /// M: Sync volte setting value. @{
                    boolean isForceNotify =
                            (itemId == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE) &&
                            (SystemProperties.getInt(PROPERTY_IMSCONFIG_FORCE_NOTIFY, 0) == 1);
                    if (DEBUG) Log.d(TAG, "isForceNotify " + isForceNotify);
                    /// @}

                    // Check SIM state
                    if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_READY) ||
                          simState.equals(IccCardConstants.INTENT_VALUE_ICC_IMSI) ||
                          simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED) ||
                          simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOCKED)) {
                        isSimStateAllow = true;
                    } else {
                        isSimStateAllow = false;
                    }

                    if (isSimStateAllow || (isECCBroadcastFlag && isFeatureEnabled) ||
                            isForceNotify) {
                        // For observers who don't have dedicated process, use broadcast mechanism.
                        intent = new Intent(ImsConfigContract.ACTION_IMS_FEATURE_CHANGED);
                        intent.putExtra(ImsConfigContract.EXTRA_PHONE_ID, Integer.parseInt(args.phoneId));
                        intent.putExtra(ImsConfigContract.EXTRA_CHANGED_ITEM, itemId);
                        intent.putExtra(ImsConfigContract.EXTRA_NEW_VALUE, value); // need to modify
                        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                        getContext().sendBroadcast(intent);
                        // Notify content observers
                        getContext().getContentResolver().notifyChange(uri, null);

                        ECCAllowBroadcast.put(Integer.parseInt(args.phoneId), false);
                        if (DEBUG) {
                            Log.d(TAG, "Update uri " + uri + " on phone " + args.phoneId +
                                    " value: " + value);
                        }
                    }
                    break;
                case ImsConfigContract.TABLE_RESOURCE:
                    break;
                default:
                    Log.e(TAG, "Invalid table " + args.table + " with uri " + uri);
                    return;
            }

        } finally {
            //Binder.restoreCallingIdentity(oldId);
        }
    }

    // Process content://com.mediatek.ims.config.provider/$table/$phoneId/$itemId
    private static class Arguments {
        public String table = null;
        public String phoneId = null;
        public String itemId = null;
        public String param = null;
        public String selection = null;
        public String[] selectionArgs = null;

        private static final int INDEX_TABLE = 0;
        private static final int INDEX_PHONE_ID = 1;
        private static final int INDEX_ITEM_ID = 2;
        private static final int INDEX_PARAM_ID = 3;

        Arguments(int opMode, Uri uri, ContentValues cv, String selection, String[] selectionArgs) {
            String[] args = null;

            enforceValidUri(uri);
            this.table = getValidTable(uri);
            parseContentValue(uri, this.table, opMode, cv);
            enforceOpMode(opMode, uri, cv, selection, selectionArgs);

            int urlArgSize = uri.getPathSegments().size();
            switch (urlArgSize) {
                case 1: // Table-level
                    this.selection = selection;
                    this.selectionArgs = selectionArgs;
                    if (opMode == OperationMode.MODE_UPDATE ||
                            opMode == OperationMode.MODE_INSERT) {
                        this.phoneId = cv.getAsString(ImsConfigContract.ConfigSetting.PHONE_ID);
                    }
                    break;
                case 2: // Phone-level
                    this.phoneId = uri.getPathSegments().get(INDEX_PHONE_ID);
                    args = new String[1];
                    args[0] = this.phoneId;
                    this.selection = ImsConfigContract.ConfigSetting.PHONE_ID + " = ?";
                    if (!TextUtils.isEmpty(selection)) {
                        this.selection += " AND " + selection;
                        this.selectionArgs = join(args, selectionArgs);
                    } else {
                        this.selectionArgs = args;
                    }
                    break;
                case 3: // Item-level
                    this.phoneId = uri.getPathSegments().get(INDEX_PHONE_ID);
                    this.itemId = uri.getPathSegments().get(INDEX_ITEM_ID);
                    args = new String[2];
                    args[0] = this.phoneId;
                    switch (this.table) {
                        case ImsConfigContract.TABLE_CONFIG_SETTING:
                            args[1] = this.itemId;
                            this.selection = ImsConfigContract.ConfigSetting.PHONE_ID + " = ?" +
                                    " AND " + ImsConfigContract.ConfigSetting.SETTING_ID + " = ?";
                            if (!TextUtils.isEmpty(selection)) {
                                this.selection += " AND " + selection;
                                this.selectionArgs = join(args, selectionArgs);
                            } else {
                                this.selectionArgs = args;
                            }
                            break;
                        case ImsConfigContract.TABLE_DEFAULT:
                        case ImsConfigContract.TABLE_PROVISION:
                        case ImsConfigContract.TABLE_MASTER:
                            // Convert config name to id
                            args[1] = String.valueOf(ImsConfigContract.configNameToId(this.itemId));
                            this.selection = ImsConfigContract.BasicConfigTable.PHONE_ID + " = ?" +
                                    " AND " + ImsConfigContract.BasicConfigTable.CONFIG_ID + " = ?";
                            if (!TextUtils.isEmpty(selection)) {
                                this.selection += " AND " + selection;
                                this.selectionArgs = join(args, selectionArgs);
                            } else {
                                this.selectionArgs = args;
                            }
                            break;
                        case ImsConfigContract.TABLE_RESOURCE:
                            this.selection = ImsConfigContract.Resource.PHONE_ID + "=? AND "
                                             + ImsConfigContract.Resource.FEATURE_ID + "=?";
                            args[1] = this.itemId;
                            if (!TextUtils.isEmpty(selection)) {
                                this.selection += " AND " + selection;
                                this.selectionArgs = join(args, selectionArgs);
                            } else {
                                this.selectionArgs = args;
                            }
                            break;
                    }
                    break;
                case 4: // feature update / delete
                    this.phoneId = uri.getPathSegments().get(INDEX_PHONE_ID);
                    this.itemId = uri.getPathSegments().get(INDEX_ITEM_ID);
                    this.param = uri.getPathSegments().get(INDEX_PARAM_ID);
                    args = new String[3];
                    args[0] = this.phoneId;
                    args[1] = this.itemId;
                    args[2] = this.param;
                    switch (this.table) {
                        case ImsConfigContract.TABLE_FEATURE:

                            this.selection = ImsConfigContract.Feature.PHONE_ID + " = ?" +
                                    " AND " + ImsConfigContract.Feature.FEATURE_ID + " = ?" +
                                    " AND " + ImsConfigContract.Feature.NETWORK_ID + " = ?";
                            if (!TextUtils.isEmpty(selection)) {
                                this.selection += " AND " + selection;
                                this.selectionArgs = join(args, selectionArgs);
                            } else {
                                this.selectionArgs = args;
                            }
                            break;
                        default:
                            throw new IllegalArgumentException("Invalid URI: " + uri);
                    }
                    break;
                default:
                    throw new IllegalArgumentException("Invalid URI: " + uri);
            }
        }
        Arguments(int opMode, Uri uri, String selection, String[] selectionArgs) {
            this(opMode, uri, null, selection, selectionArgs);
        }
        Arguments(int opMode, ContentValues cv, Uri uri) {
            this(opMode, uri, cv, null, null);
        }
        Arguments(int opMode, Uri uri) {
            this(opMode, uri, null, null, null);
        }

        private static String[] join(String[]...arrays) {

            final List<String> output = new ArrayList<String>();
            for (String[] array : arrays) {
                output.addAll(Arrays.asList(array));
            }
            return output.toArray(new String[output.size()]);
        }

        private String getValidTable(Uri uri) {
            String table = uri.getPathSegments().get(INDEX_TABLE);
            enforceValidTable(table);
            return table;
        }

        private static void enforceOpMode(int opMode, Uri uri,
                ContentValues cv, String selection, String[] selectionArgs) {

        }

        private static void enforceValidTable(String table) {
            if (!Validator.isValidTable(table)) {
                throw new IllegalArgumentException("Bad table: " + table);
            }
        }

        private static void enforceValidUri(Uri uri) {
            if (uri == null) {
                throw new IllegalArgumentException("Bad request: null url");
            }
            if (uri.getPathSegments().size() == 0) {
                throw new IllegalArgumentException("Operate on entire database is not supported");
            }
        }

        private void parseContentValue(
                Uri uri, String table, int opMode, ContentValues cv) {
            if (opMode == OperationMode.MODE_QUERY || opMode == OperationMode.MODE_DELETE) {
                return;
            }
            enforceValidTable(table);

            this.phoneId = String.valueOf(
                    cv.getAsInteger(ImsConfigContract.ConfigSetting.PHONE_ID));
            if (TextUtils.isEmpty(this.phoneId)) {
                throw new IllegalArgumentException("Expect phone id in cv with " + uri);
            }

            int configId = 0;
            switch (table) {
                case ImsConfigContract.TABLE_CONFIG_SETTING:

                    int settingId = cv.getAsInteger(ImsConfigContract.ConfigSetting.SETTING_ID);
                    if (!Validator.isValidSettingId(settingId)) {
                        throw new IllegalArgumentException(
                                "Invalid setting id in cv: " + settingId + " with " + uri);
                    }
                    this.itemId = String.valueOf(settingId);
                    break;
                case ImsConfigContract.TABLE_FEATURE:
                    int featureId = cv.getAsInteger(ImsConfigContract.Feature.FEATURE_ID);
                    if (!Validator.isValidFeatureId(featureId)) {
                        throw new IllegalArgumentException(
                                "Invalid feature id in cv: " + featureId + " with " + uri);
                    }
                    this.itemId = String.valueOf(featureId);
                    int network = cv.getAsInteger(ImsConfigContract.Feature.NETWORK_ID);
                    if (!Validator.isValidNetwork(network)) {
                        throw new IllegalArgumentException(
                                "Invalid network in cv: " + network + " with " + uri);
                    }
                    this.param = String.valueOf(network);

                    int value = cv.getAsInteger(ImsConfigContract.Feature.VALUE);
                    if(!Validator.isValidFeatureValue(value)) {
                        throw new IllegalArgumentException(
                                "Invalid value in cv: " + value + " with " + uri);
                    }
                    break;
                case ImsConfigContract.TABLE_DEFAULT:
                    if (cv.containsKey(ImsConfigContract.Default.UNIT_ID)) {
                        int timeUnitId = cv.getAsInteger(ImsConfigContract.Default.UNIT_ID);
                        if (!Validator.isValidUnitId(timeUnitId)) {
                            throw new IllegalArgumentException(
                                    "Invalid time unit in cv: " + timeUnitId + " with " + uri);
                        }
                    }
                    configId = cv.getAsInteger(ImsConfigContract.BasicConfigTable.CONFIG_ID);
                    if (!Validator.isValidConfigId(configId)) {
                        throw new IllegalArgumentException(
                                "Invalid config id in cv: " + configId + " with " + uri);
                    }
                    this.itemId = ImsConfigContract.configIdToName(configId);
                case ImsConfigContract.TABLE_PROVISION:
                case ImsConfigContract.TABLE_MASTER:
                    int mimeTypeId = cv.getAsInteger(
                            ImsConfigContract.BasicConfigTable.MIMETYPE_ID);
                    if (!Validator.isValidMimeTypeId(mimeTypeId)) {
                        throw new IllegalArgumentException(
                                "Invalid mime type in cv: " + mimeTypeId + " with " + uri);
                    }
                    configId = cv.getAsInteger(ImsConfigContract.BasicConfigTable.CONFIG_ID);
                    if (!Validator.isValidConfigId(configId)) {
                        throw new IllegalArgumentException(
                                "Invalid config id in cv: " + configId + " with " + uri);
                    }
                    this.itemId = ImsConfigContract.configIdToName(configId);
                    break;
            }
        }
    }

    private static class OperationMode {
        static final int MODE_QUERY = 0;
        static final int MODE_INSERT = 1;
        static final int MODE_UPDATE = 2;
        static final int MODE_DELETE = 3;
    }

    private void onDiskFull(SQLiteFullException e) {
        Log.e(TAG, "Disk full, all write operations will be ignored", e);
    }
}
