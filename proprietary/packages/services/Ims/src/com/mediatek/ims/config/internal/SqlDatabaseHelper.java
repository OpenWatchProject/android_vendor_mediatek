package com.mediatek.ims.config.internal;

import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.content.Context;
import android.util.Log;

import com.mediatek.ims.config.ImsConfigContract;
import static com.mediatek.ims.config.ImsConfigContract.ConfigSetting;
import static com.mediatek.ims.config.ImsConfigContract.Feature;
import static com.mediatek.ims.config.ImsConfigContract.Default;
import static com.mediatek.ims.config.ImsConfigContract.Provision;
import static com.mediatek.ims.config.ImsConfigContract.Master;
import static com.mediatek.ims.config.ImsConfigContract.Resource;

/**
 * Helper class to initialize IMS configurations SQL databases.
 */
public class SqlDatabaseHelper extends SQLiteOpenHelper {
    static final String DATABASE_NAME = "imsconfig.db";
    private static final String TAG = "ImsConfigSqlDbHelper";
    private static final int DATABASE_VERSION = 2;
    private Context mContext;

    public SqlDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        mContext = context;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        createConfigSettingTable(db);
        createFeatureTable(db);
        createProvisionTable(db);
        createMasterTable(db);
        createDefaultTable(db);
        createResourceTable(db);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        Log.d(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion);
        dropAllTables(db);
        onCreate(db);
    }

    private void createConfigSettingTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + ImsConfigContract.TABLE_CONFIG_SETTING + " (" +
                ConfigSetting._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                ConfigSetting.PHONE_ID + " INTEGER NOT NULL," +
                ConfigSetting.SETTING_ID + " INTEGER NOT NULL," +
                ConfigSetting.VALUE + " TEXT NOT NULL," +
                "CONSTRAINT unique_id UNIQUE (" +
                ConfigSetting.PHONE_ID + ", " + ConfigSetting.SETTING_ID + ")" +
                ");");
    }

    private void createFeatureTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + ImsConfigContract.TABLE_FEATURE + " (" +
                Feature._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Feature.PHONE_ID + " INTEGER NOT NULL," +
                Feature.FEATURE_ID + " INTEGER NOT NULL," +
                Feature.NETWORK_ID + " INTEGER NOT NULL," +
                Feature.VALUE + " INTEGER NOT NULL," +
                "CONSTRAINT unique_id UNIQUE (" +
                Feature.PHONE_ID + ", " + Feature.FEATURE_ID + ", " + Feature.NETWORK_ID + ")" +
                ");");
    }

    private void createDefaultTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + ImsConfigContract.TABLE_DEFAULT + " (" +
                Default._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Default.PHONE_ID + " INTEGER NOT NULL," +
                Default.CONFIG_ID + " INTEGER NOT NULL," +
                Default.MIMETYPE_ID + " INTEGER NOT NULL," +
                Default.UNIT_ID + " INTEGER," +
                Default.DATA + " TEXT NOT NULL," +
                "CONSTRAINT unique_id UNIQUE (" +
                Default.PHONE_ID + ", " + Default.CONFIG_ID + ")" +
                ");");
    }

    private void createProvisionTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + ImsConfigContract.TABLE_PROVISION + " (" +
                Provision._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Provision.PHONE_ID + " INTEGER NOT NULL," +
                Provision.CONFIG_ID + " INTEGER NOT NULL," +
                Provision.MIMETYPE_ID + " INTEGER NOT NULL," +
                Provision.DATA + " TEXT NOT NULL," +
                Provision.DATETIME + " DATETIME DEFAULT CURRENT_TIMESTAMP," +
                "CONSTRAINT unique_id UNIQUE (" +
                Provision.PHONE_ID + ", " + Provision.CONFIG_ID + ")" +
                ");");
    }

    private void createMasterTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + ImsConfigContract.TABLE_MASTER + " (" +
                Master._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Master.PHONE_ID + " INTEGER NOT NULL," +
                Master.CONFIG_ID + " INTEGER NOT NULL," +
                Master.MIMETYPE_ID + " INTEGER NOT NULL," +
                Master.DATA + " TEXT," +
                "CONSTRAINT unique_id UNIQUE (" +
                Master.PHONE_ID + ", " + Master.CONFIG_ID + ")" +
                ");");
    }

    private void createResourceTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + ImsConfigContract.TABLE_RESOURCE + " (" +
                Resource._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Resource.PHONE_ID + " INTEGER NOT NULL," +
                Resource.FEATURE_ID + " INTEGER NOT NULL," +
                Resource.VALUE + " INTEGER NOT NULL," +
                "CONSTRAINT unique_id UNIQUE (" +
                Resource.PHONE_ID + ", " + Resource.FEATURE_ID + ")" +
                ");");
    }

    private void dropAllTables(SQLiteDatabase db) {
        db.execSQL("DROP TABLE IF EXISTS " + ImsConfigContract.TABLE_CONFIG_SETTING);
        db.execSQL("DROP TABLE IF EXISTS " + ImsConfigContract.TABLE_FEATURE);
        db.execSQL("DROP TABLE IF EXISTS " + ImsConfigContract.TABLE_DEFAULT);
        db.execSQL("DROP TABLE IF EXISTS " + ImsConfigContract.TABLE_PROVISION);
        db.execSQL("DROP TABLE IF EXISTS " + ImsConfigContract.TABLE_MASTER);
        db.execSQL("DROP TABLE IF EXISTS " + ImsConfigContract.TABLE_RESOURCE);
    }
}
