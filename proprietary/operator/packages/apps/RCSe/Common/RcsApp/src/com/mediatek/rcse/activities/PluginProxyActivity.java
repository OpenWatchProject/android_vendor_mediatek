/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.activities;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Parcelable;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;


//import com.mediatek.omadrm.OmaDrmStore;
import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ContactIdUtils;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.Utils;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.AppSettings;
//import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.mediatek.rcse.settings.RcsSettings;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.JoynContactFormatException;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.chat.OneToOneChat;
import com.gsma.services.rcs.chat.ChatService;

/**
 * This class defined as a proxy activity for file transfer.
 */
public class PluginProxyActivity extends Activity {

    private static final String TAG = "PluginProxyActivity";
    private static final String ACTION_START_CONTACT =
            "mediatek.intent.action.contacts.list.PICKMULTIDATAS";

    private static final String PICK_RESULT =
            "com.mediatek.contacts.list.pickdataresult";

    private static final String RESTRICT_LIST = "restrictlist";
    private static final String INTENT_TYPE =
            "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    public static final String KEY_ADD_CONTACTS = "addContacts";
    private long mRequestTimeMillis;
    private static final long AUTOMATED_RESULT_THRESHOLD_MILLLIS = 250;
    private static final int REQUEST_CODE_CAMERA = 10;
    private static final int PERMISSION_REQUEST_CODE_CAMERA = 90;
    private static final int PERMISSION_REQUEST_FILE_PERMISSIONS = 91;
    private static final int PERMISSION_REQUEST_GALLERY_PERMISSIONS = 92;
    private static final int PERMISSION_REQUEST_EXTERNAL_STORAGE = 93;
    private static final int REQUEST_CODE_GALLERY = 11;
    private static final int REQUEST_CODE_FILE_MANAGER = 12;
    public static final int REQUEST_CODE_RCSE_CONTACT = 13;
    private static final int REQUEST_CODE_VCARD = 14;
    private static final int REQUEST_CODE_VCALENDER = 15;
    private static final int REQUEST_CODE_START_GROUP = 16;

    private static final String INVALID_FILE_TYPE = "invalid_file_type";
    private static final String INVALID_FILE_NAME = "invalid_file_name";

    private static final String GALLERY_TYPE = "image/*";
    private static final String CHOICE_FILEMANAGER_ACTION = "com.mediatek.filemanager.ADD_FILE";
    private static final String FILE_SCHEMA = "file://";
    private static final String CONTENT_SCHEMA = "content://";
    private static final String CONTACT_SCHEMA = "content://contacts/people/";
    private static final String CHAT_SCHEMA = "content://chats/";
    private static final String VCARD_SCHEMA = "content://com.android.contacts/contacts/as_vcard";
    private static final String ALL_VCARD_SCHEMA =
            "content://com.android.contacts/contacts/as_multi_vcard/";
    private static final String VCARD_DATA_TYPE = "text/x-vcard";
    private static final String VCARD_SUFFIX = ".vcf";

    private static final String VCALENDAR_SCHEMA = "content://com.mediatek.calendarimporter/";
    private static final String VCALENDAR_DATA_TYPE = "text/x-vcalendar";
    private static final String VCALENDAR_SUFFIX = ".vcs";

    private static final File SDCARD_DIR_FILE = Environment
            .getExternalStorageDirectory();
    private static final String SLASH = "/";
    private static final String RCSE_FILE_DIR = SDCARD_DIR_FILE + SLASH
            + "Joyn";
    private static final String RCSE_TEMP_FILE_DIR = RCSE_FILE_DIR + SLASH
            + "temp" + SLASH;
    private static final String RCSE_TEMP_FILE_NAME_HEADER = "tmp_joyn_";
    private static final String JPEG_SUFFIX = ".jpg";
    private static final String READABLE_RIGHT = "r";
    private ArrayList<Parcelable> mFileUriList = new ArrayList<Parcelable>();
    private static final String EXTRA_NAME_ADDRESS = "address";
    private static final String PAKAGE_NAME_MMS = "com.android.mms";
    private static final String CLASS_NAME_MMS = "com.android.mms.ui.ComposeMessageActivity";
    private static final String EXTENSION = "*";
    private ArrayList<String> mTypeList = new ArrayList<String>();
    private static final String IMAGE = "image/";
    private static final String VIDEO = "video/";
    private static final String AUDIO = "audio/";
    private static final String MIMETYPE_IMAGE = "image/*";
    private static final String MIMETYPE_VIDEO = "video/*";
    private static final String MIMETYPE_AUDIO = "audio/*";
    private static final String MIMETYPE_OTHER = "other_type";
    private static final String MIMETYPE_MULTIPLE = "*/*";
    private static final String SINGLE_SLASH = "/";
    public static final String MMSTO = "smsto: ";
    public boolean shareViaBrowser = false;
    public static final String WARNING_TYPE = "warning_type";

    private OptionAdapter mOptionAdapter = null;
    private OptionDialogFragment mOptionDialog = null;
    private Uri mCameraTempFileUri = null;
    private Intent mIntent = null;

    private boolean mIsDestroy = false;

    /**
     * On activity result.
     *
     * @param requestCode the request code
     * @param resultCode the result code
     * @param data the data
     */
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Logger.d(TAG, "onActivityResult " + requestCode + " resultCode is "
                + requestCode + " and data is " + data);
        super.onActivityResult(requestCode, resultCode, data);
        if (Activity.RESULT_CANCELED == resultCode) {
            Logger.w(TAG, "onActivityResult()-User cancel select a picture.");
            PluginProxyActivity.this.finish();
        } else {
            if (requestCode == REQUEST_CODE_RCSE_CONTACT) {
                Logger.d(TAG, "onActivityResult() the data is " + data);
                if (data != null) {
                    ArrayList<Participant> participantList = ContactsListManager
                            .getInstance().parseParticipantsFromIntent(data);
                    if (shareViaBrowser) {
                        prepareToUrlShare(participantList);
                        shareViaBrowser = false;
                    } else {
                        prepareToFileTransfer(participantList);
                    }
                } else {
                    Logger.d(TAG, "onActivityResult() the data is null ");
                }
            } else if (requestCode == REQUEST_CODE_START_GROUP) {
                Logger.d(TAG, "onActivityResult() the data is " + data);
                if (data != null) {
                    ArrayList<Participant> participants = ContactsListManager
                            .getInstance().parseParticipantsFromIntent(data);
                    Logger.d(TAG, "onActivityResult() participants is "
                            + participants);
                    if (participants != null && !participants.isEmpty()) {
                        Intent chat = new Intent();
                        chat.putParcelableArrayListExtra(
                                Participant.KEY_PARTICIPANT_LIST, participants);
                        chat.putExtra(KEY_ADD_CONTACTS,
                                ChatMainActivity.VALUE_ADD_CONTACTS);
                        if (Logger.getIsIntegrationMode()) {
                            if (participants.size() == BaseListFragment.ONE_PARTICIPANT_SIZE) {
                                String number = participants.get(
                                        BaseListFragment.NO_PARTICIPANT_SIZE)
                                        .getContact();
                                Logger.v(TAG,
                                        "onActivityResult() the number is:"
                                                + number);
                                Intent intent = new Intent();
                                intent.setAction(Intent.ACTION_SENDTO);
                                if (RcsSettings.getInstance().getMessagingUx() == 0) {
                                    intent.putExtra("chatmode",
                                            IpMessageConsts.ChatMode.JOYN);
                                    number = IpMessageConsts.JOYN_START
                                            + participants
                                                    .get(BaseListFragment.NO_PARTICIPANT_SIZE)
                                                    .getContact();
                                }
                                Uri uri = Uri.parse(MMSTO + number);
                                intent.setData(uri);
                                startActivity(intent);
                                PluginProxyActivity.this.finish();
                                return;
                            } else {
                                chat.setAction(PluginGroupChatActivity.ACTION);
                            }
                        } else {
                            chat.setClass(this, ChatScreenActivity.class);
                        }
                        chat.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        startActivity(chat);
                    } else {
                        Logger.d(TAG,
                                "onActivityResult() the participants size is 0 ");
                    }
                }
                PluginProxyActivity.this.finish();
            } else {
                new FileProcessingAsyncTask(requestCode, resultCode, data)
                        .execute();
            }
        }
    }

    /**
     * Prepare to url share.
     *
     * @param participantList the participant list
     */
    private void prepareToUrlShare(ArrayList<Participant> participantList) {
        Logger.d(TAG, "prepareToFileTransfer() entry");
        if (participantList != null) {
            if (participantList.size() > 1) {
                // start Group chat
                Logger.d(TAG, "prepareToUrlShare()-participants are "
                        + participantList);
                String url = mIntent.getStringExtra(Intent.EXTRA_TEXT);
                sendUrlToGroup(participantList, url);
            } else {
                for (Participant participant : participantList) {
                    if (participant != null) {
                        String name = participant.getDisplayName();
                        String number = participant.getContact();
                        sendUrlToContact(name, number,
                                mIntent.getStringExtra(Intent.EXTRA_TEXT));
                    } else {
                        Logger.w(TAG, "prepareToUrlShare()-participant is null");
                    }
                }
            }
        } else {
            Logger.w(TAG, "prepareToUrlShare()-participantList is null");
        }
    }

    /**
     * Send url to group.
     *
     * @param participants the participants
     * @param url the url
     */
    private void sendUrlToGroup(ArrayList<Participant> participants, String url) {
        Intent intent = new Intent();
        intent.setAction(PluginGroupChatActivity.ACTION);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST,
                participants);
        // intent.putExtra(KEY_ADD_CONTACTS,
        // ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putExtra(Utils.IS_GROUP_CHAT, true);
        intent.putExtra("urlfromshare", true);
        intent.putExtra("url", url);
        intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);

        try {
            PluginProxyActivity.this.startActivity(intent);
        } catch (android.content.ActivityNotFoundException e) {
            e.printStackTrace();
        }
        PluginProxyActivity.this.finish();
    }

    /**
     * Send url to contact.
     *
     * @param name the name
     * @param number the number
     * @param url the url
     */
    private void sendUrlToContact(String name, String number, String url) {
        Logger.d(TAG, "sendUrlToContact() entry with name is " + name
                + " and number is " + number + " and url is " + url);
        if (url.equals("")) {
            Logger.d(TAG, "sendUrlToContact()-The url is null");
        } else {
            if (PluginUtils.getMessagingMode() == 0) {
                number = IpMessageConsts.JOYN_START + number;

            }
            Intent mIntent = new Intent();
            mIntent.setAction(PluginUtils.ACTION_SEND_URL);
            mIntent.putExtra(PluginApiManager.RcseAction.SHARE_URL, url);
            mIntent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, name);
            mIntent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, number);
            MediatekFactory.getApplicationContext().sendBroadcast(mIntent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");

            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_SEND_MULTIPLE);
            intent.setAction(Intent.ACTION_SENDTO);
            intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
            intent.putExtra(EXTRA_NAME_ADDRESS, number);
            intent.setClassName(PAKAGE_NAME_MMS, CLASS_NAME_MMS);

            try {
                PluginProxyActivity.this.startActivity(intent);
            } catch (android.content.ActivityNotFoundException e) {
                e.printStackTrace();
            }
        }
        PluginProxyActivity.this.finish();
    }

    /**
     * Prepare to file transfer.
     *
     * @param participantList the participant list
     */
    private void prepareToFileTransfer(ArrayList<Participant> participantList) {
        Logger.d(TAG, "prepareToFileTransfer() entry");
        Logger.d(TAG, "prepareToFileTransfer()-participantList is "
                + participantList);
        if (participantList != null) {
            if (participantList.size() > 1) {
                // start group FT
                Logger.d(TAG, "prepareToFileTransfer()-participants are "
                        + participantList);
                ArrayList<String> files = null;
                if (mIntent
                        .hasExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI)) {
                    files = mIntent
                            .getStringArrayListExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI);
                } else if (mIntent
                        .hasExtra(PluginApiManager.RcseAction.SINGLE_FILE_URI)) {
                    files = new ArrayList<String>();
                    files.add(mIntent
                            .getStringExtra(PluginApiManager.RcseAction.SINGLE_FILE_URI));
                } else {
                    Logger.d(TAG, "prepareToFileTransfer()-No file selected");
                }
                new FtCapabilityAsyncTask(participantList, files, true)
                        .execute();

            } else {
                // one2one FT
                for (Participant participant : participantList) {
                    Logger.d(TAG, "prepareToFileTransfer()-participant is "
                            + participant);
                    if (participant != null) {
                        ArrayList<String> files = null;
                        if (mIntent
                                .hasExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI)) {
                            files = mIntent
                                    .getStringArrayListExtra(PluginApiManager.RcseAction.
                                            MULTIPLE_FILE_URI);
                        } else if (mIntent
                                .hasExtra(PluginApiManager.RcseAction.SINGLE_FILE_URI)) {
                            files = new ArrayList<String>();
                            files.add(mIntent
                                    .getStringExtra(PluginApiManager.RcseAction.SINGLE_FILE_URI));
                        } else {
                            Logger.d(TAG,
                                    "prepareToFileTransfer()-No file selected");
                        }
                        String name = participant.getDisplayName();
                        String number = participant.getContact();
                        new FtCapabilityAsyncTask(name, number, files)
                                .execute();
                    }
                }
            }
        }
    }

    /**
     * A subclass of {@link AsyncTask}. This class is used to process the file
     * which was selected via camera,gallery or file manager.
     */
    private class FileProcessingAsyncTask extends AsyncTask<Void, Void, String> {

        private int mRequestCode = -1;
        private int mResultCode = -1;
        private Intent mData = null;

        /**
         * Instantiates a new file processing async task.
         *
         * @param requestCode the request code
         * @param resultCode the result code
         * @param data the data
         */
        public FileProcessingAsyncTask(int requestCode, int resultCode,
                Intent data) {
            mRequestCode = requestCode;
            mResultCode = resultCode;
            mData = data;
        }

        @Override
        protected String doInBackground(Void... params) {
            String fileFullName = null;
            Uri uri = null;
            if (mRequestCode == REQUEST_CODE_CAMERA) {
                fileFullName = getFileFullPathFromUri(mCameraTempFileUri);
                mFileUriList.add(mCameraTempFileUri);
            } else if (mRequestCode == REQUEST_CODE_GALLERY
                    || mRequestCode == REQUEST_CODE_FILE_MANAGER) {
                Logger.d(TAG, "mData is " + mData);
                if (mData != null) {
                    uri = mData.getData();
                    mFileUriList.add(uri);
                    fileFullName = getFileFullPathFromUri(uri);
                }
            } else if (mRequestCode == REQUEST_CODE_VCARD
                    || mRequestCode == REQUEST_CODE_VCALENDER) {
                Logger.d(TAG, "mData is " + mData);
                if (mData != null) {
                    uri = (Uri) mData.getParcelableExtra(Intent.EXTRA_STREAM);
                    fileFullName = getFileFullPathFromUri(uri);
                }
            } else {
                Logger.w(TAG, "unkown result");
            }
            return fileFullName;
        }

        @Override
        protected void onPostExecute(String fileName) {
            Logger.v(TAG, "onPostExecute(),fileName = " + fileName
                    + ",mResultCode = " + mResultCode);
            if (fileName != null && mIntent != null) {
                if (fileName.equals(INVALID_FILE_TYPE)
                        || fileName.equals(INVALID_FILE_NAME)) {
                    showWarningDialog(
                            PluginProxyActivity.this,
                            WarningDialogFragment.WARNING_NOT_SUPPORTTED_FILE,
                            PluginProxyActivity.this.getFragmentManager());
                } else {
                    if (mRequestCode == REQUEST_CODE_CAMERA
                            || mRequestCode == REQUEST_CODE_GALLERY
                            || mRequestCode == REQUEST_CODE_FILE_MANAGER) {
                        Intent intent = new Intent();
                        intent.setAction(PluginApiManager.RcseAction.FT_ACTION);
                        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        String contactName = mIntent
                                .getStringExtra(PluginApiManager.RcseAction.CONTACT_NAME);
                        String contactNumber = mIntent
                                .getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
                        ArrayList<String> files = new ArrayList<String>();
                        files.add(fileName);
                        new FtCapabilityAsyncTask(contactName, contactNumber,
                                files).execute();
                    } else if (mRequestCode == REQUEST_CODE_VCARD
                            || mRequestCode == REQUEST_CODE_VCALENDER) {
                        ArrayList<String> formattedFiles = new ArrayList<String>();
                        formattedFiles.add(fileName);
                        mIntent.putExtra(
                                PluginApiManager.RcseAction.MULTIPLE_FILE_URI,
                                formattedFiles);
                        startConatcts();
                    } else {
                        Logger.i(TAG, "onPostExecute()-invalid request code");
                        PluginProxyActivity.this.finish();
                    }
                }
            } else {
                Logger.i(TAG, "onPostExecute()-no file selected");
                PluginProxyActivity.this.finish();
            }
        }
    }

    /**
     * Gets the data column.
     *
     * @param context the context
     * @param uri the uri
     * @param selection the selection
     * @param selectionArgs the selection args
     * @return the data column
     */
    public static String getDataColumn(Context context, Uri uri,
            String selection, String[] selectionArgs) {

        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = { column };

        try {
            cursor = context.getContentResolver().query(uri, projection,
                    selection, selectionArgs, null);
            if (cursor != null && cursor.moveToFirst()) {
                final int columnIndex = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(columnIndex);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return null;
    }

    /**
     * Checks if is downloads document.
     *
     * @param uri the uri
     * @return true, if is downloads document
     */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri
                .getAuthority());
    }

    /**
     * Checks if is raw data
     *
     * @param uri the uri
     * @return true, the Uri authority is raw data
     */
    public static boolean isRawUri(Uri uri) {
        String authority = uri.getAuthority();
        if (authority == null) {
            authority = uri.getEncodedAuthority();
        }
        Logger.i(TAG, "authority:" + authority);
        return "com.google.android.apps.photos.contentprovider".equals(uri.getAuthority()) ||
                "com.android.email.attachmentprovider".equals(uri.getAuthority()) ||
                "com.android.chrome.FileProvider".equals(uri.getAuthority()) ||
                "gmail-ls".equals(uri.getAuthority());
    }

    /**
     * @param context application context.
     * @param uri The Uri to check.
     * @return file name of uri.
     */
    static String getRawFilename(Context context, Uri uri) {
        return getDataColumn(context, uri, null, Images.Media.DISPLAY_NAME, null);
    }

    /**
     * @param context application context.
     * @param uri The Uri to check.
     * @return file size of uri.
     */
    static int getRawFilesize(Context context, Uri uri) {
        return getIntColumn(context, uri, null, Images.Media.SIZE, null);
    }

    static String getDataColumn(Context context, Uri uri, String selection,
        String column, String[] selectionArgs) {
        Cursor cursor = null;
        final String[] projection = {
             column
        };
        try {
            cursor = context.getContentResolver().query(uri, projection,
                selection, selectionArgs, null);
            if (cursor != null && cursor.moveToFirst()) {
                final int columnIndex = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(columnIndex);
            }
        } catch (SecurityException se) {
            se.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Logger.e(TAG, "Cursor is null or other");

        return buildSrcFromPath(uri);
    }

    static int getIntColumn(Context context, Uri uri, String selection,
        String column, String[] selectionArgs) {
        Cursor cursor = null;
        final String[] projection = {
             column
        };
        try {
            cursor = context.getContentResolver().query(uri, projection,
                selection, selectionArgs, null);
            if (cursor != null && cursor.moveToFirst()) {
                final int columnIndex = cursor.getColumnIndexOrThrow(column);
                return cursor.getInt(columnIndex);
            }
        } catch (SecurityException se) {
            se.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Logger.e(TAG, "Cursor is null or other");
        return -1;
    }

    private static String buildSrcFromPath(Uri uri) {
        String src = null;
        try {
            String path = uri.getPath();
            src = path.substring(path.lastIndexOf('/') + 1);
            if (src.startsWith(".") && src.length() > 1) {
               src = src.substring(1);
            }
        } catch (IndexOutOfBoundsException e) {
            Logger.e(TAG, "err:" + e);
        } catch (NullPointerException e) {
            Logger.e(TAG, "err:" + e);
        }
        return src;
    }


    /**
     * Gets the file full path from uri.
     *
     * @param uri the uri
     * @return the file full path from uri
     */
    private String getFileFullPathFromUri(Uri uri) {
        if (uri != null) {
            String uriString = Uri.decode(uri.toString());
            Logger.d(TAG, "getFileFullPathFromUri() - The uri is:[" + uriString
                    + "]");
            String fileFullName = INVALID_FILE_NAME;
            if (uriString != null && uriString.startsWith(FILE_SCHEMA)) {
                uriString = uriString.substring(FILE_SCHEMA.length(),
                        uriString.length());
                return uriString;
            } else if (uriString != null
                    && (uriString.startsWith(VCARD_SCHEMA) || uriString
                            .startsWith(ALL_VCARD_SCHEMA))) {
                fileFullName = RCSE_TEMP_FILE_DIR
                        + System.currentTimeMillis() + VCARD_SUFFIX;
                try {
                    AssetFileDescriptor fd = PluginProxyActivity.this
                            .getContentResolver().openAssetFileDescriptor(uri,
                                    READABLE_RIGHT);
                    if (!createTempFolder()) {
                        return INVALID_FILE_TYPE;
                    }
                    FileInputStream fis = fd.createInputStream();
                    byte[] data = new byte[fis.available()];
                    fis.read(data);
                    fis.close();
                    File file = new File(fileFullName);
                    file.setWritable(true);
                    file.setReadable(true);
                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(data);
                    fos.close();
                } catch (FileNotFoundException fileNotFoundException) {
                    fileFullName = INVALID_FILE_NAME;
                    fileNotFoundException.printStackTrace();
                } catch (IOException iOException) {
                    fileFullName = INVALID_FILE_NAME;
                    iOException.printStackTrace();
                } finally {
                    return fileFullName;
                }
            } else if (uriString != null
                    && uriString.startsWith(VCALENDAR_SCHEMA)) {
                fileFullName = RCSE_TEMP_FILE_DIR
                        + System.currentTimeMillis() + VCALENDAR_SUFFIX;
                FileInputStream fis = null;
                FileOutputStream fos = null;
                try {
                    AssetFileDescriptor fd = PluginProxyActivity.this
                            .getContentResolver().openAssetFileDescriptor(uri,
                                    READABLE_RIGHT);
                    if (!createTempFolder()) {
                        return INVALID_FILE_TYPE;
                    }
                    fis = fd.createInputStream();
                    byte[] data = new byte[fis.available()];
                    fis.read(data);

                    File file = new File(fileFullName);
                    file.setWritable(true);
                    file.setReadable(true);
                    fos = new FileOutputStream(file);
                    fos.write(data);
                } catch (FileNotFoundException fileNotFoundException) {
                    fileFullName = INVALID_FILE_NAME;
                    fileNotFoundException.printStackTrace();
                } catch (IOException iOException) {
                    fileFullName = INVALID_FILE_NAME;
                    iOException.printStackTrace();
                } finally {
                    try {
                        if (fis != null) {
                            fis.close();
                        }
                        if (fos != null) {
                            fos.close();
                        }
                    } catch (Exception e) {
                    }
                    return fileFullName;
                }
            } else if (uriString != null
                    && uriString.startsWith(CONTENT_SCHEMA)) {
                if (isDownloadsDocument(uri)) {
                    final String id = DocumentsContract.getDocumentId(uri);
                    final Uri contentUri = ContentUris.withAppendedId(
                            Uri.parse("content://downloads/all_downloads"),
                            Long.valueOf(id));
                    fileFullName = getDataColumn(this, contentUri, null, null);
                    return fileFullName;
                } else if (isRawUri(uri)) {
                    fileFullName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis()
                            + getRawFilename(PluginProxyActivity.this, uri);
                    int size = getRawFilesize(PluginProxyActivity.this, uri);
                    try {
                        if (!createTempFolder()) {
                            return INVALID_FILE_TYPE;
                        }
                        InputStream fis = PluginProxyActivity.this.
                                getContentResolver().openInputStream(uri);
                        byte[] data = new byte[size];
                        fis.read(data);
                        fis.close();
                        File file = new File(fileFullName);
                        file.setWritable(true);
                        file.setReadable(true);
                        FileOutputStream fos = new FileOutputStream(file);
                        fos.write(data);
                        fos.close();
                    } catch (Exception e) {
                        fileFullName = INVALID_FILE_NAME;
                        e.printStackTrace();
                    }
                } else {
                    Cursor cursor = PluginProxyActivity.this
                            .getContentResolver()
                            .query(uri,
                                    new String[] { MediaStore.Images.ImageColumns.DATA },
                                    null, null, null);
                    if (cursor != null && cursor.getCount() != 0) {
                        cursor.moveToFirst();
                        fileFullName = cursor
                                .getString(cursor
                                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
                    } else {
                        Logger.w(TAG, "getFileFullPathFromUri, cursor is null!");
                    }
                    if (cursor != null) {
                        cursor.close();
                        cursor = null;
                    }
                }
                return fileFullName;
            } else {
                Logger.e(TAG, "getFileFullPathFromUri() - uriString = "
                        + uriString
                        + ",is not start with file:// and content://");
                return INVALID_FILE_TYPE;
            }
        } else {
            Logger.e(TAG, "getFileFullPathFromUri() - uri is null");
            return INVALID_FILE_NAME;
        }
    }

    /**
     * Gets the register status.
     *
     * @return the register status
     */
    private boolean getRegisterStatus() {
        RegistrationApi registrationApi = ApiManager.getInstance()
                .getRegistrationApi();
        if (registrationApi == null) {
            return false;
        } else {
            return registrationApi.isRegistered();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Logger.d(TAG, "onCreate() entry");
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.proxy_layout);
        //ContactsManager.createInstance(getApplicationContext());
        if (!getRegisterStatus()) {
            showWarningDialog(
                    PluginProxyActivity.this,
                    WarningDialogFragment.WARNING_NO_SERVICE,
                    PluginProxyActivity.this.getFragmentManager());
        } else if (!hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            PluginProxyActivity.this.requestPermissions(
                    new String[]
                    { Manifest.permission.WRITE_EXTERNAL_STORAGE },
                    PERMISSION_REQUEST_EXTERNAL_STORAGE);
        } else {
            mOptionAdapter = new OptionAdapter(this);
            mIntent = this.getIntent();
            Logger.d(TAG, "onCreate()-The intent is " + mIntent);
            if (mIntent != null) {
                String action = mIntent.getAction();
                if (null != action) {
                    Logger.d(TAG, "onCreate() the action is " + action);
                    if (action.matches(Intents.Chat.ACTION_INITIATE_CHAT)) {
                        Uri uri = mIntent.getData();
                        if (uri != null) {
                            String contactID = uri.toString().substring(
                                    CONTACT_SCHEMA.length(),
                                    uri.toString().length());
                            List<RcsContact> list = ContactsListManager
                                    .getInstance().CONTACTS_LIST;
                            String contactNumber = "";
                            String displayName = "";
                            Logger.d(TAG,
                                    "onCreate() the ACTION_INITIATE_CHAT contact is "
                                            + contactID);
                            for (int i = 0; i < list.size(); i++) {
                                if (list.get(i).mContactId == Short
                                        .parseShort(contactID)) {
                                    contactNumber = list.get(i).mNumber;
                                    displayName = list.get(i).mDisplayName;
                                    Logger.d(TAG,
                                            "onCreate() ACTION_INITIATE_CHAT , number & name are "
                                                    + contactNumber
                                                    + displayName);
                                    break;
                                }
                            }
                            mIntent.putExtra(
                                    PluginApiManager.RcseAction.CONTACT_NUMBER,
                                    contactNumber);
                            mIntent.putExtra(
                                    PluginApiManager.RcseAction.CONTACT_NAME,
                                    displayName);
                            mIntent.putExtra("isjoyn", true);
                            handleImAction();
                        }
                    } else if (action.matches(Intents.Chat.ACTION_VIEW_CHAT)) {
                        Uri uri = mIntent.getData();
                        if (uri != null) {
                            String chatId = uri.toString().substring(
                                    CHAT_SCHEMA.length(),
                                    uri.toString().length());
                            ChatService chatService = ApiManager.getInstance()
                                    .getChatApi();
                            OneToOneChat chat = null;
                            String contact = "";
                            if (chatService != null) {
                                try {
                                    chat = chatService.getOneToOneChat(ContactIdUtils.createContactIdFromTrustedData(chatId));
                                    if (chat != null) {
                                        contact = chat.getRemoteContact().toString();
                                    } else {
                                        Logger.d(TAG,
                                                "onCreate() the ACTION_VIEW_CHAT " +
                                                "chat instance is null ");
                                    }
                                } catch (Exception e) {
                                    Logger.d(
                                            TAG,
                                            " ACTION_VIEW_CHAT Exception "
                                                    + e.getMessage());
                                    e.printStackTrace();
                                }
                            } else {
                                Logger.d(TAG,
                                        "onCreate() the ACTION_VIEW_CHAT chat service is null ");
                            }
                            mIntent.putExtra(
                                    PluginApiManager.RcseAction.CONTACT_NUMBER,
                                    contact);
                            mIntent.putExtra(
                                    PluginApiManager.RcseAction.CONTACT_NAME,
                                    contact);
                            mIntent.putExtra("isjoyn", true);
                            Logger.d(TAG,
                                    "onCreate() the ACTION_VIEW_CHAT contact is "
                                            + contact);
                            handleImAction();
                        }
                    } else if (action
                            .matches(PluginApiManager.RcseAction.PROXY_ACTION)) {
                        // launch IM chat
                        if (mIntent.getBooleanExtra(
                                PluginApiManager.RcseAction.IM_ACTION, false)) {
                            handleImAction();
                        } else if (mIntent.getBooleanExtra(
                                PluginApiManager.RcseAction.FT_ACTION, false)) {
                            handleFtAction();
                        } else {
                            Logger.d(TAG, "Invalid action type");
                            PluginProxyActivity.this.finish();
                        }
                    } else if (action
                            .matches(PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION)) {
                        handleSingleFileAction();
                    } else if (action
                            .matches(PluginApiManager.RcseAction.MULTIPLE_FILE_TRANSFER_ACTION)) {
                        handleMultiFileAction();
                    } else if (action
                            .matches(PluginApiManager.RcseAction.SELECT_PLUGIN_CONTACT_ACTION)) {
                        Logger.d(TAG, "onCreat() SELECT_PLUGIN_CONTACT_ACTION");
                        ArrayList<Participant> originalContacts = mIntent
                                .getParcelableArrayListExtra(ChatScreenActivity
                                        .KEY_EXSITING_PARTICIPANTS);
                        startSelectContactsActivity(originalContacts);
                    } else {
                        Logger.d(TAG, "Invalid action");
                        PluginProxyActivity.this.finish();
                    }
                } else {
                    Logger.d(TAG, "Invalid action is null");
                }
            }
        }
    }

    /**
     * Handle im action.
     */
    private void handleImAction() {
        Logger.v(TAG, "handleImAction() entry");
        String number = mIntent
                .getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
        String name = mIntent
                .getStringExtra(PluginApiManager.RcseAction.CONTACT_NAME);
        Boolean isJoynChat = mIntent.getBooleanExtra("isjoyn", false);
        boolean isRCSeDisabled = mIntent.getBooleanExtra("is_rcse_disabled",
                false);
        Logger.v(TAG, "handleImAction() contact number = " + number + "name = "
                + name + "isRcseDisabled" + isRCSeDisabled);
        String sNumber = Utils.formatNumberToInternational(number);
        new ImCapabilityAsyncTask(name, sNumber, isJoynChat, isRCSeDisabled)
                .execute();
    }

    /**
     * Handle ft action.
     */
    private void handleFtAction() {
        Logger.v(TAG, "handleFtAction() entry");
        String filePath = mIntent
                .getStringExtra(PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION);
        String number = mIntent
                .getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
        if (number != null && filePath == null) {
            new FtCapabilityAsyncTask(null, number, null).execute();
        } else if (number == null && filePath != null) {
            if (filePath == INVALID_FILE_TYPE || filePath == INVALID_FILE_NAME) {
                showWarningDialog(
                        PluginProxyActivity.this,
                        WarningDialogFragment.WARNING_NOT_SUPPORTTED_FILE,
                        PluginProxyActivity.this.getFragmentManager());
            } else {
                startConatcts();
            }
        } else {
            Logger.d(TAG, "onCreate()-Do nothing");
            PluginProxyActivity.this.finish();
        }
    }

    /**
     * Handle single file action.
     */
    private void handleSingleFileAction() {
        String type = mIntent.getType();
        Logger.d(TAG, "handleSingleFileAction() the type is " + type);
        if (type.equals(VCARD_DATA_TYPE)) {
            // VCard file
            new FileProcessingAsyncTask(REQUEST_CODE_VCARD, Activity.RESULT_OK,
                    mIntent).execute();
        } else if (type.equals(VCALENDAR_DATA_TYPE)) {
            // VCalendar file
            new FileProcessingAsyncTask(REQUEST_CODE_VCALENDER,
                    Activity.RESULT_OK, mIntent).execute();
        } else {
            // File that selected from Camera,File Manager,Gallery
            Uri uri = (Uri) mIntent.getParcelableExtra(Intent.EXTRA_STREAM);
            String url = mIntent.getStringExtra(Intent.EXTRA_TEXT);
            if (uri != null) {
                mFileUriList.add(uri);
                String filePath = getFileFullPathFromUri(uri);
                Logger.d(TAG, "File path from getFileFullPathFromUri:" + filePath);
                if (filePath == INVALID_FILE_TYPE
                        || filePath == INVALID_FILE_NAME) {
                    showWarningDialog(
                            PluginProxyActivity.this,
                            WarningDialogFragment.WARNING_NOT_SUPPORTTED_FILE,
                            PluginProxyActivity.this.getFragmentManager());
                } else {
                    ArrayList<String> formattedFiles = new ArrayList<String>();
                    formattedFiles.add(filePath);
                    mIntent.putExtra(
                            PluginApiManager.RcseAction.MULTIPLE_FILE_URI,
                            formattedFiles);
                    startConatcts();
                }
            } else if (!url.equals("")) {
                mIntent.putExtra("page_url", url);
                shareViaBrowser = true;
                startConatcts();
            } else {
                Logger.d(TAG, "handleSingleFileAction() uri is null");
                PluginProxyActivity.this.finish();
            }
        }
    }

    /**
     * Handle multi file action.
     */
    private void handleMultiFileAction() {
        Logger.v(TAG, "handleMultiFileAction() entry");
        if (mIntent.hasExtra(Intent.EXTRA_STREAM)) {
            ArrayList<Uri> originallist = mIntent
                    .getParcelableArrayListExtra(Intent.EXTRA_STREAM);
            ArrayList<String> formattedFiles = new ArrayList<String>();
            if (null != originallist) {
                int size = originallist.size();
                for (int i = 0; i < size; i++) {
                    String filePath = getFileFullPathFromUri(originallist
                            .get(i));
                    mFileUriList.add(originallist.get(i));
                    if (filePath == INVALID_FILE_TYPE
                            || filePath == INVALID_FILE_NAME) {
                        showWarningDialog(
                                PluginProxyActivity.this,
                                WarningDialogFragment.WARNING_NOT_SUPPORTTED_FILE,
                                PluginProxyActivity.this.getFragmentManager());
                    } else {
                        formattedFiles.add(filePath);
                    }
                }
                if (formattedFiles.size() > 0) {
                    mIntent.putExtra(
                            PluginApiManager.RcseAction.MULTIPLE_FILE_URI,
                            formattedFiles);
                    startConatcts();
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        Logger.d(TAG, "onDestroy called ");
        mIsDestroy = true;
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        Logger.d(TAG, "onStop called ");
        super.onStop();
    }

    private boolean createTempFolder() {
        if (!createDirectory(RCSE_FILE_DIR)) {
            Logger.w(TAG, "Create rcse dir failed");
            return false;
        }
        if (!createDirectory(RCSE_TEMP_FILE_DIR)) {
            Logger.w(TAG, "Create rcse tmp dir failed");
            return false;
        }
        return true;
    }

    /**
     * Creates the directory.
     *
     * @param path the path
     * @return true, if successful
     */
    private boolean createDirectory(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            return dir.mkdirs();
        }
        return true;
    }

    /**
     * Start file transfer.
     *
     * @param name the name
     * @param number the number
     * @param files the files
     */
    private void startFileTransfer(String name, String number,
            ArrayList<String> files) {
        Logger.d(TAG, "startFileTransfer() entry with name is " + name
                + " and number is " + number + " and files is " + files);
        Logger.d(TAG, "startFileTransfer()-The files is " + files);
        if (files != null) {
            Intent intent = getIntent(name, number, files);
            try {
                PluginProxyActivity.this.startActivity(intent);
            } catch (android.content.ActivityNotFoundException e) {
                e.printStackTrace();
            }
        }
        PluginProxyActivity.this.finish();
    }

    /**
     * Start group file transfer.
     *
     * @param participants the participants
     * @param files the files
     */
    private void startGroupFileTransfer(ArrayList<Participant> participants,
            ArrayList<String> files) {
        Logger.d(TAG, "startGroupFileTransfer() entry with participants is "
                + participants + "" + " and files is " + files);
        Logger.d(TAG, "startGroupFileTransfer()-The files is " + files);
        if (files != null) {
            Intent intent = getIntent(participants, files);
            try {
                PluginProxyActivity.this.startActivity(intent);
            } catch (android.content.ActivityNotFoundException e) {
                e.printStackTrace();
            }
        }
        PluginProxyActivity.this.finish();
    }

    /**
     * Gets the intent.
     *
     * @param participants the participants
     * @param files the files
     * @return the intent
     */
    private Intent getIntent(ArrayList<Participant> participants,
            ArrayList<String> files) {
        Intent intent = new Intent();
        Logger.d(
                TAG,
                "getIntent() IsIntegrationMode: "
                        + Logger.getIsIntegrationMode());
        if (Logger.getIsIntegrationMode()) {
            Logger.d(TAG,
                    "startFileTransfer() in integration mode with mFileUriList is "
                            + mFileUriList);

            // intent.setAction(PluginApiManager.RcseAction.FT_ACTION);
            intent.setAction(PluginGroupChatActivity.ACTION);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putParcelableArrayListExtra(
                    Participant.KEY_PARTICIPANT_LIST, participants);
            intent.putExtra(KEY_ADD_CONTACTS,
                    ChatMainActivity.VALUE_ADD_CONTACTS);
            intent.putExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI,
                    files);
            intent.putExtra(Utils.IS_GROUP_CHAT, true);
            intent.putExtra("ftfromshare", true);
            intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
            // intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST,
            // participants);
            return intent;
        } else {
            Logger.d(TAG, "file transter no integerate");
            intent.setAction(PluginApiManager.RcseAction.FT_ACTION);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI,
                    files);
            intent.putExtra(Utils.IS_GROUP_CHAT, true);
            intent.putParcelableArrayListExtra(
                    Participant.KEY_PARTICIPANT_LIST, participants);
            return intent;
        }
    }

    /**
     * Gets the intent.
     *
     * @param name the name
     * @param number the number
     * @param files the files
     * @return the intent
     */
    private Intent getIntent(String name, String number, ArrayList<String> files) {
        Intent intent = new Intent();
        Logger.d(
                TAG,
                "getIntent() IsIntegrationMode: "
                        + Logger.getIsIntegrationMode());
        if (Logger.getIsIntegrationMode()) {
            Logger.d(TAG,
                    "startFileTransfer() in integration mode with mFileUriList is "
                            + mFileUriList);
            if (PluginUtils.getMessagingMode() == 0) {
                number = IpMessageConsts.JOYN_START + number;

            }
            ArrayList<Parcelable> list = new ArrayList<Parcelable>();
            for (Parcelable fileUri : mFileUriList) {
                list.add(fileUri);
            }

            intent.setAction(Intent.ACTION_SEND_MULTIPLE);
            intent.setAction(Intent.ACTION_SENDTO);
            Intent it = new Intent();
            it.setAction(PluginUtils.ACTION_FILE_SEND);
            it.putExtra("sendFileFromContacts", true);
            it.putExtra("contact", number);
            it.putExtra("filePath", files.get(0));
            it.putStringArrayListExtra("filePaths", files);
            ArrayList<Integer> fileSize = new ArrayList<Integer>();
            for (int i = 0; i < files.size(); i++) {
                File mfile = new File(files.get(i));
                Long size = mfile.length();
                int sizeInt = size.intValue();
                fileSize.add(i, sizeInt);
            }

            it.putIntegerArrayListExtra("size", fileSize);
            it.putParcelableArrayListExtra(Intent.EXTRA_STREAM, list);
            MediatekFactory.getApplicationContext().sendBroadcast(it, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");

            // Sednd MMS intent to open joyn composer window

            intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, list);
            String mimeType = null;
            int size = files.size();
            Logger.d(TAG, "startFileTransfer() file size is " + size);
            if (size == 1) {
                mimeType = getSigleMimeType(files.get(0));
            } else if (size > 1) {
                for (String file : files) {
                    mimeType = getMultipleMimeType(file);
                }
            }

            intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
            Logger.d(TAG, "startFileTransfer() mimeType is " + mimeType);
            intent.setType(mimeType);
            intent.putExtra(EXTRA_NAME_ADDRESS, number);
            intent.setClassName(PAKAGE_NAME_MMS, CLASS_NAME_MMS);
            Logger.d(TAG, "startFileTransfer() new intent number is " + number
                    + "mimetype is" + mimeType + "list" + files.get(0));
            return intent;
        } else {
            Logger.d(TAG, "file transter no integerate");
            intent.setAction(PluginApiManager.RcseAction.FT_ACTION);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI,
                    files);
            intent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, name);
            intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, number);
            return intent;
        }
    }

    /**
     * Start conatcts.
     */
    private void startConatcts() {
        Logger.d(TAG, "startConatcts() entry ");
        if (ContactsListManager.IS_SUPPORT) {
            startMultiChoiceActivity();
        } else {
            Intent intent = new Intent(
                    PluginApiManager.RcseAction.SELECT_CONTACT_ACTION);
            startActivityForResult(intent, REQUEST_CODE_RCSE_CONTACT);
        }
    }

    /**
     * Start multi choice activity.
     */
    private void startMultiChoiceActivity() {
        Logger.d(TAG, "startMultiChoiceActivity() entry");
        Intent intent = new Intent(ChatMainActivity.ACTION_START_CONTACT);
        intent.setType(ChatMainActivity.INTENT_TYPE);
        long[] phoneIdList = ContactsListManager.getInstance()
                .getPhoneIdTobeShow(null);
        intent.putExtra(ChatMainActivity.RESTRICT_LIST, phoneIdList);
        startActivityForResult(intent, REQUEST_CODE_RCSE_CONTACT);
        Logger.d(TAG, "startMultiChoiceActivity() exit");
    }

    /**
     * Start camera.
     */
    private void startCamera() {
        if (this.createDirectory(RCSE_FILE_DIR)) {
            Logger.w(TAG, "Create rcse dir success");
        } else {
            Logger.w(TAG, "Create rcse dir failed");
        }
        if (this.createDirectory(RCSE_TEMP_FILE_DIR)) {
            Logger.w(TAG, "Create rcse tmp dir success");
        } else {
            Logger.w(TAG, "Create rcse tmp dir failed");
        }
        mCameraTempFileUri = Uri.fromFile(new File(RCSE_TEMP_FILE_DIR,
                RCSE_TEMP_FILE_NAME_HEADER
                        + String.valueOf(System.currentTimeMillis())
                        + JPEG_SUFFIX));

        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, mCameraTempFileUri);
        startActivityForResult(intent, REQUEST_CODE_CAMERA);


    }

    public boolean isNeverGrantedPermission(String permission) {
        return !(this.shouldShowRequestPermissionRationale(permission));
    }

    @Override
    public void onRequestPermissionsResult(final int requestCode,
            final String permissions[], final int[] grantResults) {

        if (grantResults.length <= 0
                || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            Logger.d(TAG, "onRequestPermissionsResult not granted");
            if(permissions.length != 0 && isNeverGrantedPermission(permissions[0])) {
                Toast.makeText(getApplicationContext(), "Permission denied.You can change them in Settings->Apps.", Toast.LENGTH_LONG).show();
            }
            finish();
            return;
        } else {
            Logger.d(TAG, "onRequestPermissionsResult granted");
        }
        if (requestCode == PERMISSION_REQUEST_CODE_CAMERA) {
            Logger.d(TAG, "onRequestPermissionsResult()  " + requestCode);
            startCamera();
        } else if (requestCode == PERMISSION_REQUEST_FILE_PERMISSIONS) {
            Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
            intent.addCategory(Intent.CATEGORY_DEFAULT);
            startActivityForResult(intent, REQUEST_CODE_FILE_MANAGER);
        } else if (requestCode == PERMISSION_REQUEST_GALLERY_PERMISSIONS) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType(GALLERY_TYPE);
            startActivityForResult(intent, REQUEST_CODE_GALLERY);
        } else if (requestCode == PERMISSION_REQUEST_EXTERNAL_STORAGE) {
            Toast.makeText(getApplicationContext(),
                    "Permission is granted\nPlease try again", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
    }

    public boolean hasPermission(final String permission) {
        final Context context = MediatekFactory.getApplicationContext();
        final int permissionState = this.checkSelfPermission(permission);
        Logger.d(TAG, "hasPermission() : permission = " + permission
                + " permissionState = " + permissionState);
        return permissionState == PackageManager.PERMISSION_GRANTED;

    }

    /**
     * Start gallery.
     */
    private void startGallery() {
        if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType(GALLERY_TYPE);
            startActivityForResult(intent, REQUEST_CODE_GALLERY);
        } else {
            mRequestTimeMillis = SystemClock.elapsedRealtime();
            requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                    PERMISSION_REQUEST_GALLERY_PERMISSIONS);
        }
    }

    /**
     * Start file manager.
     */
    private void startFileManager() {
        if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
        Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        startActivityForResult(intent, REQUEST_CODE_FILE_MANAGER);
        } else {
            mRequestTimeMillis = SystemClock.elapsedRealtime();
            requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                    PERMISSION_REQUEST_FILE_PERMISSIONS);
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mOptionDialog != null) {
                mOptionDialog.dismissAllowingStateLoss();
                PluginProxyActivity.this.finish();
                return true;
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    /**
     * Show warning dialog.
     *
     * @param type the type
     * @param fragmentManager the fragment manager
     */
    private void showWarningDialog(PluginProxyActivity activity, int type, FragmentManager fragmentManager) {
        try {
            WarningDialogFragment dialog = new WarningDialogFragment(activity);
            Bundle bundle = new Bundle();
            bundle.putInt(WARNING_TYPE, type);
            dialog.setArguments(bundle);
            dialog.show(fragmentManager, TAG);
        } catch (IllegalStateException e) {
            // this is called from onPostExecute, so it may cause Illegal state
            // exception because async task may take
            // long time in background
            // ideally in perfect design, UI work should not be done in
            // onPostExecute
            e.printStackTrace();
        }
    }

    /**
     * This class defined to display a warning dialog.
     */
    public static class WarningDialogFragment extends DialogFragment {

        private static final int WARNING_NO_SERVICE = 0;
        private static final int WARNING_NO_FT_CAPABILITY = 1;
        private static final int WARNING_NO_IM_CAPABILITY = 2;
        private static final int WARNING_NOT_SUPPORTTED_FILE = 3;
        private static final int WARNING_RCSe_DISABLED = 4;

        private PluginProxyActivity mActivity = null;

        /**
         * Instantiates a new warning dialog fragment.
         */
        public WarningDialogFragment(PluginProxyActivity activity) {
            mActivity = activity;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            int warningType = getArguments().getInt(WARNING_TYPE);
            Builder dialogBuilder = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT);
            dialogBuilder.setTitle(R.string.attention_title);
            if (warningType == WARNING_NO_SERVICE) {
                dialogBuilder.setMessage(this.getResources().getText(
                        R.string.warning_no_service));
            } else if (warningType == WARNING_NO_FT_CAPABILITY) {
                dialogBuilder.setMessage(this.getResources().getText(
                        R.string.warning_no_file_transfer_capability));
            } else if (warningType == WARNING_NO_IM_CAPABILITY) {
                dialogBuilder.setMessage(this.getResources().getText(
                        R.string.warning_no_im_capability));
            } else if (warningType == WARNING_NOT_SUPPORTTED_FILE) {
                dialogBuilder.setMessage(this.getResources().getText(
                        R.string.warning_not_support_file));
            } else if (warningType == WARNING_RCSe_DISABLED) {
                dialogBuilder.setMessage(this.getResources().getText(
                        R.string.warning_rcse_disabled));
            } else {
                Logger.d(TAG, "Invalid warning type");
                return null;
            }
            dialogBuilder.setPositiveButton(
                    R.string.rcs_dialog_positive_button, new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {
                            if (mActivity != null) {
                                mActivity.finish();
                                mActivity = null;
                            }
                        }
                    });
            AlertDialog dialog = dialogBuilder.create();
            dialog.setCancelable(false);
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            this.dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
                mActivity = null;
            }
        }
    }

    /**
     * This class defined to display a dialog.
     */
    public static class OptionDialogFragment extends DialogFragment {
        private static final int POSITION_GALLERY = 0;
        private static final int POSITION_CAMERA = 1;
        private static final int POSITION_FILE_MANAGER = 2;

        private PluginProxyActivity mActivity = null;

        /**
         * Instantiates a new option dialog fragment.
         */
        public OptionDialogFragment(PluginProxyActivity activity) {
            mActivity = activity;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Builder dialogBuilder = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT);
            dialogBuilder.setIcon(R.drawable.ic_dialog_attach);
            dialogBuilder.setTitle(R.string.file_transfer_title);
            dialogBuilder.setAdapter(mActivity.mOptionAdapter,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            switch (which) {
                            case POSITION_GALLERY:
                                mActivity.startGallery();
                                dismissAllowingStateLoss();
                                break;
                            case POSITION_CAMERA:
                                if(mActivity.hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                                    mActivity.startCamera();
                                } else {
                                    if (mActivity != null) {
                                        mActivity.requestPermissions(
                                                new String[]
                                                { Manifest.permission.WRITE_EXTERNAL_STORAGE },
                                                PERMISSION_REQUEST_CODE_CAMERA);
                                    }
                                }
                                dismissAllowingStateLoss();
                                break;
                            case POSITION_FILE_MANAGER:
                                mActivity.startFileManager();
                                dismissAllowingStateLoss();
                                break;
                            default:
                                if (mActivity != null) {
                                    mActivity.finish();
                                    mActivity = null;
                                }
                                break;
                            }
                        }
                    });
            AlertDialog dialog = dialogBuilder.create();
            dialog.setCancelable(true);
            dialog.setCanceledOnTouchOutside(true);
            return dialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            this.dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
                mActivity = null;
            }
        }
    }

    /**
     * This class defined a adaptor for OptionDialogFragment.
     */
    private static class OptionAdapter extends BaseAdapter {

        private final List<OptionItemData> mDataList = new ArrayList<OptionItemData>();
        private Context mContext = null;
        private LayoutInflater mInflater = null;

        /**
         * The Class OptionItemView.
         */
        public static class OptionItemView {
            ImageView mImage;
            TextView mText;
        }

        /**
         * The Class OptionItemData.
         */
        public static class OptionItemData {
            int mDrawableId;
            int mContentId;
        }

        /**
         * Constructor.
         *
         * @param context            The context of the parent
         */
        public OptionAdapter(Context context) {
            mInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            mContext = context;
            OptionItemData data = new OptionItemData();
            data.mDrawableId = R.drawable.ic_attach_picture_holo_light;
            data.mContentId = R.string.attach_picture;
            mDataList.add(data);
            data = new OptionItemData();
            data.mDrawableId = R.drawable.ic_attach_capture_picture_holo_light;
            data.mContentId = R.string.attach_capture_picture;
            mDataList.add(data);
            data = new OptionItemData();
            data.mDrawableId = R.drawable.ic_menu_move_to_holo_light;
            data.mContentId = R.string.file_type_file;
            mDataList.add(data);
        }

        /**
         * Gets the count.
         *
         * @return the count
         */
        public int getCount() {
            return mDataList.size();
        }

        /**
         * Gets the item.
         *
         * @param position the position
         * @return the item
         */
        public Object getItem(int position) {
            return mDataList.get(position);
        }

        /**
         * Gets the item id.
         *
         * @param position the position
         * @return the item id
         */
        public long getItemId(int position) {
            return position;
        }

        /**
         * Gets the view.
         *
         * @param position the position
         * @param convertView the convert view
         * @param parent the parent
         * @return the view
         */
        public View getView(int position, final View convertView,
                ViewGroup parent) {
            OptionItemView itemView = null;
            View view = convertView;
            if (view == null) {
                itemView = new OptionItemView();
                view = mInflater.inflate(R.layout.option_item, parent, false);
                itemView.mImage = (ImageView) view
                        .findViewById(R.id.attach_icon);
                itemView.mText = (TextView) view.findViewById(R.id.attach_text);
                view.setTag(itemView);
            } else {
                itemView = (OptionItemView) view.getTag();
            }
            OptionItemData data = mDataList.get(position);
            Resources resources = mContext.getResources();
            if (resources == null) {
                return null;
            } else {
                Drawable drawable = resources.getDrawable(data.mDrawableId);
                String content = resources.getString(data.mContentId);
                itemView.mImage.setBackgroundDrawable(drawable);
                itemView.mText.setText(content);
                return view;
            }
        }
    }

    private void startImChat(String name, String number, boolean isJoynChat) {
        Logger.d(TAG, "startImChat() entry wiht name is " + name
                + " and number is " + number);
        Intent intent = null;
        intent = getImChatIntent(name, number, isJoynChat);
        startActivity(intent);
        PluginProxyActivity.this.finish();
    }

    private Intent getImChatIntent(String name, String number,
            boolean isJoynChat) {
        Intent intent = null;
        if (Logger.getIsIntegrationMode()) {
            Logger.d(TAG, "startImChat() is in integration mode");
            intent = new Intent();
            Uri uri;
            intent.setAction(Intent.ACTION_SENDTO);
            intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
            if (isJoynChat) {
                intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
                intent.putExtra("isjoyn", true);
                uri = Uri.parse(MMSTO + IpMessageConsts.JOYN_START + number);
            } else {
                uri = Uri.parse(MMSTO + number);
            }
            intent.setData(uri);
            return intent;
        } else {
            intent = new Intent(PluginApiManager.RcseAction.IM_ACTION);
            intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, number);
            intent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, name);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            return intent;
        }
    }

    /*
     * AsyncTask for retrieving IM capability, if true returned, start IM chat,
     * otherwise popup a warning dialog.
     */
    /**
     * The Class ImCapabilityAsyncTask.
     */
    private class ImCapabilityAsyncTask extends AsyncTask<Void, Void, Boolean> {
        private static final String TAG = "ImCapabilityAsyncTask";
        private String mName = null;
        private String mNumber = null;
        private boolean mIsJoynChat = false;
        private boolean mIsRCSeDisabled = false;

        /**
         * Instantiates a new im capability async task.
         *
         * @param name the name
         * @param number the number
         * @param isJoynChat the is joyn chat
         * @param isRCSeDisabled the is rc se disabled
         */
        public ImCapabilityAsyncTask(String name, String number,
                boolean isJoynChat, boolean isRCSeDisabled) {
            mName = name;
            mNumber = number;
            mIsJoynChat = isJoynChat;
            mIsRCSeDisabled = isRCSeDisabled;
        }

        @Override
        protected Boolean doInBackground(Void... params) {
            Boolean result = false;
            Capabilities capabilities;
            try {
                if(AppSettings.getInstance()!= null && RcsSettings.getInstance().isSupportOP08()){
                    //in case of OP08 treat all numbers as capable of IM and FT
                    Logger.d(TAG, "OP08 support is true, so capability is true");
                    result = true;
                    return result;
                }
                capabilities = ApiManager.getInstance().getCapabilityApi()
                        .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(mNumber));

                if (capabilities != null && capabilities.isImSessionSupported()) {
                    result = true;
                    Logger.d(TAG, "doInbackground inside: " + result);
                    return result;
                }
                if (!mIsJoynChat) {
                    result = true;
                }
                Logger.d(TAG, "doInbackground: exit " + result);
                return result;
            } catch (Exception e) {
                e.printStackTrace();
            }
            return result;
        }

        /**
         * On post execute.
         *
         * @param isFtSupported the is ft supported
         */
        @Override
        protected void onPostExecute(Boolean isFtSupported) {
            if (!isFtSupported) {
                showWarningDialog(
                        PluginProxyActivity.this,
                        WarningDialogFragment.WARNING_NO_IM_CAPABILITY,
                        PluginProxyActivity.this.getFragmentManager());
            } else if (mIsRCSeDisabled) {
                showWarningDialog(
                        PluginProxyActivity.this,
                        WarningDialogFragment.WARNING_RCSe_DISABLED,
                        PluginProxyActivity.this.getFragmentManager());
            } else {
                startImChat(mName, mNumber, mIsJoynChat);
            }
        }
    }

    /*
     * AsyncTask for retrieving FT capability, if true returned, start file
     * transfer, otherwise popup a warning dialog.
     */
    /**
     * The Class FtCapabilityAsyncTask.
     */
    private class FtCapabilityAsyncTask extends AsyncTask<Void, Void, Boolean> {
        private static final String TAG = "FtCapabilityAsyncTask";
        private String mName = null;
        private String mNumber = null;
        private ArrayList<String> mFilePaths = null;
        private ArrayList<Participant> mParticipants = null;
        boolean mIsGroupFT = false;

        /**
         * Instantiates a new ft capability async task.
         *
         * @param name the name
         * @param number the number
         * @param filePaths the file paths
         */
        public FtCapabilityAsyncTask(String name, String number,
                ArrayList<String> filePaths) {
            mName = name;
            mNumber = number;
            mFilePaths = filePaths;
        }

        /**
         * Instantiates a new ft capability async task.
         *
         * @param participants the participants
         * @param filePaths the file paths
         * @param isGroupFT the is group ft
         */
        public FtCapabilityAsyncTask(ArrayList<Participant> participants,
                ArrayList<String> filePaths, boolean isGroupFT) {
            mParticipants = participants;
            mFilePaths = filePaths;
            mIsGroupFT = isGroupFT;
        }

        @Override
        protected Boolean doInBackground(Void... params) {
            Boolean result = false;
            if (ContactsListManager.getInstance() == null) {
                Logger.d(TAG,
                        "addChat() ContactsListManager is null ");
                ContactsListManager.initialize(MediatekFactory
                        .getApplicationContext());
            }
            if(AppSettings.getInstance()!= null && RcsSettings.getInstance().isSupportOP08()) {
                //in case of OP08 treat all numbers as capable of IM and FT
                Logger.d(TAG, "OP08 support is true, so capability is true");
                result = true;
                return result;
            }
            if (mIsGroupFT == false && ContactsListManager.getInstance() != null) {
                if (ContactsListManager.getInstance().isLocalContact(mNumber)) {
                    Capabilities capabilities;
                    try {
                        capabilities = ApiManager.getInstance()
                                .getCapabilityApi()
                                .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(mNumber));

                        if (capabilities != null
                                && capabilities.isFileTransferSupported()) {
                            result = true;
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    result = mIntent.getBooleanExtra("rcs_ft_supported", false);
                }
            } else {
                // group FT is http based as of now
                result = true;
            }
            return result;
        }

        @Override
        protected void onPostExecute(Boolean isFtSupported) {
            if (!isFtSupported) {
                if (!mIsDestroy) {
                    showWarningDialog(
                            PluginProxyActivity.this,
                            WarningDialogFragment.WARNING_NO_FT_CAPABILITY,
                            PluginProxyActivity.this.getFragmentManager());
                } else {
                    Logger.w(TAG,
                            "FtCapabilityAsyncTask->onPostExecute() Activity has been destroied");
                }
            } else {
                if (mFilePaths != null) {
                    if (mIsGroupFT == true) {
                        startGroupFileTransfer(mParticipants, mFilePaths);
                    } else {
                        startFileTransfer(mName, mNumber, mFilePaths);
                    }
                } else {
                    if (mOptionDialog == null) {
                        Logger.w(TAG, "The mOptionDialog is null");
                        mOptionDialog = new OptionDialogFragment(PluginProxyActivity.this);
                    } else {
                        Logger.w(TAG, "The mOptionDialog is not null");
                    }
                    mOptionDialog.show(
                            PluginProxyActivity.this.getFragmentManager(), TAG);
                }
            }
        }

    }

    /**
     * Start select contacts activity.
     *
     * @param originalContacts the original contacts
     */
    private void startSelectContactsActivity(
            ArrayList<Participant> originalContacts) {
        Logger.d(TAG, "startSelectContactsActivity() entry originalContacts: "
                + originalContacts);
        if (ContactsListManager.IS_SUPPORT) {
            startMultiChoiceActivity();
        } else {
            Intent intent = new Intent();
            intent.putExtra(ChatMainActivity.KEY_ADD_CONTACTS,
                    ChatMainActivity.VALUE_ADD_CONTACTS);
            intent.setClass(this, SelectContactsActivity.class);
            intent.putExtra(ChatScreenActivity.KEY_EXSITING_PARTICIPANTS,
                    originalContacts);
            if (null != originalContacts) {
                intent.putExtra(
                        SelectContactsActivity.KEY_IS_NEED_ORIGINAL_CONTACTS,
                        true);
            }
            startActivity(intent);
            PluginProxyActivity.this.finish();
        }
    }

    /**
     * Gets the sigle mime type.
     *
     * @param fileName the file name
     * @return the sigle mime type
     */
    private String getSigleMimeType(String fileName) {
        Logger.d(TAG, "getSigleMimeType() entry with fileName = " + fileName);
        StringBuilder sb = new StringBuilder();
        String type = AsyncGalleryView.getMimeType(fileName);
        if (type == null) {
            type = "";
        } else {
            type = type.substring(0, type.lastIndexOf(SINGLE_SLASH) + 1);
        }
        sb.append(type);
        sb.append(EXTENSION);
        Logger.d(TAG,
                "getSigleMimeType() eixt with sb.toString() = " + sb.toString());
        return sb.toString();
    }

    /**
     * Gets the multiple mime type.
     *
     * @param fileName the file name
     * @return the multiple mime type
     */
    private String getMultipleMimeType(String fileName) {
        Logger.d(TAG, "getMultipleMimeType() entry with fileName = " + fileName);
        String type = AsyncGalleryView.getMimeType(fileName);
        mTypeList.add(type);
        Logger.d(TAG, "getMultipleMimeType() mTypeList = " + mTypeList);
        String lastFileType = getType(mTypeList.get(0));
        int size = mTypeList.size();
        for (int i = 1; i < size; i++) {
            String mimeType = getType(mTypeList.get(i));
            if (mimeType == null) {
                return MIMETYPE_OTHER;
            }
            if (mimeType.equals(lastFileType)) {
                Logger.d(TAG, "getMultipleMimeType(),the same type file");
                lastFileType = mimeType;
            } else {
                Logger.d(TAG, "getMultipleMimeType(),different type file");
                lastFileType = MIMETYPE_MULTIPLE;
            }

        }
        Logger.d(TAG, "getMultipleMimeType() exit with lastFileType = "
                + lastFileType);
        return lastFileType;
    }

    /**
     * Gets the type.
     *
     * @param type the type
     * @return the type
     */
    private String getType(String type) {
        Logger.d(TAG, "getType() entry with type is " + type);
        String mimeType = "";
        if(type == null)
            type = "";
        if (type.startsWith(IMAGE)) {
            mimeType = MIMETYPE_IMAGE;
        } else if (type.startsWith(VIDEO)) {
            mimeType = MIMETYPE_VIDEO;
        } else if (type.startsWith(AUDIO)) {
            mimeType = MIMETYPE_AUDIO;
        }
        Logger.d(TAG, "getType() exit with mimeType is " + mimeType);
        return mimeType;
    }
}
