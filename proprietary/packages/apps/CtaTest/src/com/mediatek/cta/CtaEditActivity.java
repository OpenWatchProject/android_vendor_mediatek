/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.cta;

import android.Manifest;
import android.app.Activity;
import android.content.ContentValues;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.provider.CalendarContract.Events;
import android.provider.CallLog;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;

public class CtaEditActivity extends Activity implements View.OnClickListener {
    public static final String TAG = "CtaEditActivity";
    public static final String TYPE = "type";
    public static final String TEXT1 = "text1";
    public static final String ID = "id";

    public static final int TYPE_CONTACTS = 1000;
    public static final int TYPE_CALL_LOG = 1001;
    public static final int TYPE_SMS = 1002;
    public static final int TYPE_MMS = 1003;
    public static final int TYPE_CALENDAR = 1004;
    private static final int WRITE_OPERATION = 1000 ;
    private static final int DELETE_OPERATION = WRITE_OPERATION + 1000 ;

    public static final int TYPE_CONTACTS_WRITE = TYPE_CONTACTS + WRITE_OPERATION;
    public static final int TYPE_CALL_LOG_WRITE = TYPE_CALL_LOG + WRITE_OPERATION;
    public static final int TYPE_CALENDAR_WRITE = TYPE_CALENDAR + WRITE_OPERATION;
    public static final int TYPE_CONTACTS_DELETE = TYPE_CONTACTS + DELETE_OPERATION;
    public static final int TYPE_CALL_LOG_DELETE = TYPE_CALL_LOG + DELETE_OPERATION;
    public static final int TYPE_CALENDAR_DELETE = TYPE_CALENDAR + DELETE_OPERATION;

    private int mType;
    private int mId;
    private SimpleCursorAdapter mAdapter;
    private TextView mLabel;
    private EditText mEditor;
    private Button mButtonWrite;
    private Button mButtonDelete;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cta_edit);
        mLabel = (TextView) findViewById(R.id.cta_label_1);
        mEditor = (EditText) findViewById(R.id.cta_editor_1);
        mButtonWrite = (Button) findViewById(R.id.cta_button_write);
        mButtonDelete = (Button) findViewById(R.id.cta_button_delete);
        mButtonWrite.setOnClickListener(this);
        mButtonDelete.setOnClickListener(this);

        mType = getIntent().getIntExtra(TYPE, TYPE_CONTACTS);
        mEditor.setText(getIntent().getStringExtra(TEXT1));
        mId = getIntent().getIntExtra(ID, 0);

        switch (mType) {
        case TYPE_CONTACTS:
            mLabel.setText("Name:");
            break;
        case TYPE_CALL_LOG:
            mLabel.setText("Phone Number:");
            break;
        case TYPE_SMS:
            mLabel.setText("Content:");
            break;
        case TYPE_MMS:
            mLabel.setText("Subject:");
            break;
        case TYPE_CALENDAR:
            mLabel.setText("Description:");
            break;

        default:
            return;
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonWrite) {
            switch (mType) {
            case TYPE_CONTACTS:
                if (alreadyGetPermission(Manifest.permission.WRITE_CONTACTS, TYPE_CONTACTS_WRITE)) {
                    testWriteContactIfGranted() ;
                }
                break;
            case TYPE_CALL_LOG:
                if (alreadyGetPermission(Manifest.permission.WRITE_CALL_LOG, TYPE_CALL_LOG_WRITE)) {
                    testWriteCallLogIfGranted() ;
                }
                break;
            case TYPE_SMS:
                testWriteSmsIfGranted() ;
                break;
            case TYPE_MMS:
                testWriteMmsIfGranted() ;
                break;
            case TYPE_CALENDAR:
                if (alreadyGetPermission(Manifest.permission.WRITE_CALENDAR, TYPE_CALENDAR_WRITE)) {
                    testWriteCalendarIfGranted() ;
                }
                break;
            default:
                return;
            }
        }
        if (v == mButtonDelete) {
            switch (mType) {
            case TYPE_CONTACTS:
                if (alreadyGetPermission(Manifest.permission.WRITE_CONTACTS,
                    TYPE_CONTACTS_DELETE)) {
                    testDeleteContactIfGranted() ;
                }
                break;
            case TYPE_CALL_LOG:
                if (alreadyGetPermission(Manifest.permission.WRITE_CALL_LOG,
                    TYPE_CALL_LOG_DELETE)) {
                    testDeleteCallLogIfGranted() ;
                }
                break;
            case TYPE_SMS:
                testDeleteSmsIfGranted();
                break;
            case TYPE_MMS:
                testDeleteMmsIfGranted();
                break;
            case TYPE_CALENDAR:
                if (alreadyGetPermission(Manifest.permission.WRITE_CALENDAR,
                    TYPE_CALENDAR_DELETE)) {
                    testDeleteCalendarIfGranted() ;
                }
                break;
            default:
                return;
            }
        }
    }

    void showOnceDenyToast(String permission) {
        showDenyToast(permission, false) ;
    }

    void showAlwaysDenyToast(String permission) {
        showDenyToast(permission, true) ;
    }

    void showDenyToast(String permission, boolean isNeverAskAgain) {
        String msg = permission ;
        if (isNeverAskAgain) {
            msg = msg + " is alwyas denied. Goto Settings to re-grant." ;
        } else {
            msg = msg + " is denied." ;
        }
        Log.d(TAG, "showDenyToast() - msg = " + msg) ;
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    boolean alreadyGetPermission(String permission, int requestCode) {
        Log.d(TAG, "alreadyGetPermission(perm = " + permission +
                ", requestCode = " + requestCode) ;
        if (checkSelfPermission(permission) !=
                PackageManager.PERMISSION_GRANTED) {
            Log.d(TAG, "alreadyGetPermission()" + permission +
                    " is not granted, request it") ;
            if (!shouldShowRequestPermissionRationale(permission)) {
                showAlwaysDenyToast(permission);
                return false;
            }

            requestPermissions(new String[] {permission}, requestCode) ;
            return false;
        }
        return true;
    }

     @Override
     public void onRequestPermissionsResult(int requestCode,
         String[] permissions, int[] grantResults) {
         Log.d(TAG, "onRequestPermissionsResult() - requestCode = " + requestCode) ;

         boolean allPermissionsGranted = true ;
         String deniedPermission = null ;
         for (String permission : permissions) {
             if (checkSelfPermission(permission) !=
                     PackageManager.PERMISSION_GRANTED) {
                 Log.d(TAG, "onRequestPermissionsResult() - requestCode = " +
                         requestCode +
                         " perm = " + permission + " is revoked.") ;
                 allPermissionsGranted = false ;
                 deniedPermission = permission ;
                 break ;
             }
         }

         if (allPermissionsGranted) {
             switch (requestCode) {
                case TYPE_CONTACTS_WRITE:
                     testWriteContactIfGranted() ;
                     break;
                 case TYPE_CALL_LOG_WRITE:
                    testWriteCallLogIfGranted();
                    break;
                 case TYPE_CALENDAR_WRITE:
                    testWriteCalendarIfGranted() ;
                    break;
                case TYPE_CONTACTS_DELETE:
                     testDeleteContactIfGranted() ;
                     break;
                 case TYPE_CALL_LOG_DELETE:
                    testDeleteCallLogIfGranted();
                    break;
                 case TYPE_CALENDAR_DELETE:
                    testDeleteCalendarIfGranted() ;
                    break;
                default:
                    break;
             }
         } else {
             showOnceDenyToast(deniedPermission) ;
         }
     }

     void testWriteContactIfGranted() {
        ContentValues values = new ContentValues();
        String text = mEditor.getText().toString();

        values.put(StructuredName.GIVEN_NAME, "");
        values.put(StructuredName.FAMILY_NAME, "");
        values.put(StructuredName.PREFIX, "");
        values.put(StructuredName.MIDDLE_NAME, "");
        values.put(StructuredName.SUFFIX, "");
        values.put(StructuredName.DISPLAY_NAME, text);
        getContentResolver().update(Data.CONTENT_URI, values,
                Data.CONTACT_ID + " = ? AND " + Data.MIMETYPE + " = ?",
                new String[] {String.valueOf(mId), StructuredName.CONTENT_ITEM_TYPE});
        finish();
    }

    void testWriteCallLogIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();

         values.put(CallLog.Calls.NUMBER, text);
         getContentResolver().update(CallLog.Calls.CONTENT_URI, values,
                 CallLog.Calls._ID + " = ?", new String[] {String.valueOf(mId)});
         finish() ;
     }

     void testWriteSmsIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         values.put(Sms.BODY, text);
         getContentResolver().update(Sms.CONTENT_URI, values,
                 Sms._ID + " = ?", new String[] {String.valueOf(mId)});
         finish() ;
     }

     void testWriteMmsIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         values.put(Mms.SUBJECT, text);
         getContentResolver().update(Mms.CONTENT_URI, values,
                 Sms._ID + " = ?", new String[] {String.valueOf(mId)});
         finish() ;
     }

     void testWriteCalendarIfGranted() {
        ContentValues values = new ContentValues();
        String text = mEditor.getText().toString();

         values.put(Events.DESCRIPTION, text);
         getContentResolver().update(Events.CONTENT_URI, values,
                 Sms._ID + " = ?", new String[] {String.valueOf(mId)});
         finish();
     }

     void testDeleteContactIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         getContentResolver().delete(Data.CONTENT_URI,
                 Data.CONTACT_ID + " = ?", new String[] {String.valueOf(mId)});
         getContentResolver().delete(RawContacts.CONTENT_URI,
                 RawContacts.CONTACT_ID + " = ?", new String[] {String.valueOf(mId)});
         getContentResolver().delete(Contacts.CONTENT_URI,
                 Contacts._ID + " = ?", new String[] {String.valueOf(mId)});
         finish();
     }

     void testDeleteCallLogIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         getContentResolver().delete(CallLog.Calls.CONTENT_URI,
                 CallLog.Calls._ID + " = ?", new String[] {String.valueOf(mId)});
         finish() ;
     }

     void testDeleteSmsIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         getContentResolver().delete(Sms.CONTENT_URI,
                 Sms._ID + " = ?", new String[] {String.valueOf(mId)});
         finish() ;
     }

     void testDeleteMmsIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         getContentResolver().delete(Mms.CONTENT_URI,
                 Mms._ID + " = ?", new String[] {String.valueOf(mId)});
         finish() ;
     }

     void testDeleteCalendarIfGranted() {
         ContentValues values = new ContentValues();
         String text = mEditor.getText().toString();
         getContentResolver().delete(Events.CONTENT_URI,
                 Events._ID + " = ?", new String[] {String.valueOf(mId)});
         finish();
     }
}
