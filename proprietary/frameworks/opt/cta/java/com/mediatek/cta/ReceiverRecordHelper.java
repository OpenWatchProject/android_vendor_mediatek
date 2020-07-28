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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ParceledListSlice;
import android.content.pm.UserInfo;
import android.content.pm.ResolveInfo;
import android.os.Environment;
import android.os.IUserManager;
import android.os.FileObserver;
import android.os.Process;
import android.os.RemoteException;
import android.os.UserHandle;
import android.util.Log;
import android.util.Xml;
import com.android.internal.util.XmlUtils;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

class ReceiverRecordHelper {
    private static final String TAG = "Cta_ReceiverRecordHelper";
    private static final String FILE_NAME = "bootreceiver";
    private static final String FILE_EXTENSION = ".xml";
    private static final boolean DEFAULT_STATUS = true;
    private static boolean SUPPORT_SYSTEM_APP = false;
    private static final String CLIENT_PKGNAME = "com.mediatek.autobootcontroller";
    private static final String PKG_TAG = "pkg";

    private Context mContext = null;
    private File mFile = null;
    private Map<Integer, Map<String, ReceiverRecord>> mBootReceiverList =
            new HashMap<Integer, Map<String, ReceiverRecord>>();
    // Record the pending disbled settings
    private Map<Integer, List<String>> mPendingSettings =
            new HashMap<Integer, List<String>>();
    private IUserManager mUm = null;
    private IPackageManager mPm = null;
    private boolean mReady = false;
    private BootReceiverPolicy mBootReceiverPolicy = null ;
    private FileChangeListener mFileChangeListener = null ;
    private  boolean mShouldUpdate = true;

    public ReceiverRecordHelper(
            Context context, IUserManager um, IPackageManager pm) {
        File dataDir = Environment.getDataDirectory();
        mFile = new File(dataDir, "system/" + FILE_NAME + FILE_EXTENSION);

        mContext = context;
        mUm = um;
        mPm = pm;
        mBootReceiverPolicy = BootReceiverPolicy.getInstance(mContext);
        Log.d(TAG, "store path = " + mFile.getPath());
        mFileChangeListener = new FileChangeListener(mFile.getPath());

        IntentFilter filter = new IntentFilter(Intent.ACTION_SHUTDOWN);
        context.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "ShutDown") ;
                saveAutoBootPackagesInfo();
            }
        }, filter);
    }

    public void initReceiverList() {
        // Should NOT modify basic policy
        enforceBasicBootPolicy();

        List<UserInfo> userList = getUserList();
        synchronized (mBootReceiverList) {
            for (UserInfo userInfo: userList) {
                initReceiverCache(userInfo.id, false);
            }
        }
        // Read xml file to cache
        loadDataFromFileToCache();
        mReady = true;

        if (mFileChangeListener != null) {
            mFileChangeListener.startWatching();
        }
    }

    // Discard old data
    private void initReceiverCache(int userId, boolean readFile) {
        Log.d(TAG, "initReceiverCache() at User(" + userId + ")");
        List<String> list = getPackageListReceivingSpecifiedIntent(userId);
        Map<String, ReceiverRecord> receiverList = new HashMap<String, ReceiverRecord>();

        synchronized (mBootReceiverList) {
            mBootReceiverList.put(userId, receiverList);
        }

        for (int i = 0; i < list.size(); i++) {
            String packageName = list.get(i);
            receiverList.put(packageName, new ReceiverRecord(packageName, DEFAULT_STATUS));
        }
        list = null;
        receiverList = null;
        if (readFile) {
            loadDataFromFileToCache();
        }
    }

    //control 3rd party app
    List<String> getPackageListReceivingSpecifiedIntent(int userId) {
        List<String> bootControlList = new ArrayList<String>();
        List<ApplicationInfo> originalAppList =
                mContext.getPackageManager().getInstalledApplications(
                        PackageManager.GET_UNINSTALLED_PACKAGES | PackageManager
                                .GET_DISABLED_COMPONENTS);
        for (ApplicationInfo appInfo : originalAppList) {
            String packageName = appInfo.packageName;
            if ((SUPPORT_SYSTEM_APP == false)
                    && isSystemApp(userId, packageName)) {
                continue;
            }
            if (packageName != null && !bootControlList.contains(packageName)) {
                bootControlList.add(packageName);
            }
        }
        return bootControlList;
    }

    private void enforceBasicBootPolicy() {
        List<String> policy = mBootReceiverPolicy.getBootPolicy();
        boolean valid = true;

        do {
            if (!policy.contains(Intent.ACTION_BOOT_COMPLETED)) {
                valid = false;
                break;
            }
        } while (false);

        if (!valid) {
            throw new RuntimeException("Should NOT remove basic boot policy!");
        }
    }

    boolean getReceiverDataEnabled(int userId, String packageName) {
        if (mReady) {
            Map<String, ReceiverRecord> receiverList = getBootReceiverListByUser(userId);
            if (receiverList != null && receiverList.containsKey(packageName)) {
                ReceiverRecord data = receiverList.get(packageName);
                if (data != null) {
                    return data.mEnabled;
                }
            }
        } else {
            Log.e(TAG, "getReceiverDataEnabled() not ready!");
        }
        // Default status is enabled
        return true;
    }

    private boolean setReceiverRecord(int userId, String packageName, boolean enable) {
        boolean success = false;
        Map<String, ReceiverRecord> receiverList = null;
        receiverList = getBootReceiverListByUser(userId);
        if (receiverList != null && receiverList.containsKey(packageName)) {
            ReceiverRecord data = receiverList.get(packageName);
            data.mEnabled = enable;
            success = true;
        }
        return success;
    }

    public void addReceiverRecord(int userId, String packageName, boolean enabled) {
        Log.d(TAG, "addReceiverRecord() with " + packageName
            + " at User(" + userId + ") enabled: " + enabled);

        Map<String, ReceiverRecord> receiverList = null;
        receiverList = getBootReceiverListByUser(userId);
        if (receiverList == null) {
            receiverList = new HashMap<String, ReceiverRecord>();
            synchronized (mBootReceiverList) {
                mBootReceiverList.put(userId, receiverList);
            }
        }
        receiverList.put(packageName, new ReceiverRecord(packageName, enabled));
    }

    public void removeReceiverRecord(int userId, String packageName) {
        Log.d(TAG, "removeReceiverRecord() with " + packageName + " at User(" + userId + ")");

        Map<String, ReceiverRecord> receiverList = null;
        receiverList = getBootReceiverListByUser(userId);
        if (receiverList == null) {
            receiverList = new HashMap<String, ReceiverRecord>();
            synchronized (mBootReceiverList) {
                mBootReceiverList.put(userId, receiverList);
            }
        }
        receiverList.remove(packageName);
    }

    private boolean isPendingSetting(int userId, String packageName) {
        synchronized (mPendingSettings) {
            List<String> settings = mPendingSettings.get(userId);
            if (settings != null) {
                if (settings.contains(packageName)) {
                    Log.d(TAG, "Found a pending setting for pkg: " +
                            packageName + " at User(" + userId + ")");
                    return true;
                }
            }
        }
        return false;
    }

    private void removePendingSetting(int userId, String packageName) {
        synchronized (mPendingSettings) {
            List<String> settings = mPendingSettings.get(userId);
            if (settings != null) {
                if (settings.contains(packageName)) {
                    settings.remove(packageName);
                }
            }
        }
    }

    private void loadDataFromFileToCache() {
        Log.d(TAG, "loadDataFromFileToCache()");
        synchronized (mFile) {
            FileInputStream stream = null;
            try {
                stream = new FileInputStream(mFile);
            } catch (FileNotFoundException e) {
                Log.i(TAG, "No existing " + mFile.getPath()
                    + "; starting empty" + e.getMessage());
                //since the file is deleted, we need to reset all records.
                resetAllReceiverRecords() ;
                return;
            }
            boolean success = false;
            try {
                XmlPullParser parser = Xml.newPullParser();
                parser.setInput(stream, null);
                int type;
                while ((type = parser.next()) != XmlPullParser.START_TAG
                        && type != XmlPullParser.END_DOCUMENT) {
                    ;
                }

                if (type != XmlPullParser.START_TAG) {
                    success = true;
                    throw new IllegalStateException("no start tag found");
                }

                int outerDepth = parser.getDepth();
                while ((type = parser.next()) != XmlPullParser.END_DOCUMENT
                        && (type != XmlPullParser.END_TAG || parser.getDepth() > outerDepth)) {
                    if (type == XmlPullParser.END_TAG || type == XmlPullParser.TEXT) {
                        continue;
                    }
                    String tagName = parser.getName();
                    if (tagName.equals(PKG_TAG)) {
                        String pkgName = parser.getAttributeValue(null, "n");
                        int userId = Integer.parseInt(parser.getAttributeValue(null, "u"));
                        boolean enabled =
                            Boolean.parseBoolean(parser.getAttributeValue(null, "e"));
                        Log.d(TAG, "Read package name: " + pkgName
                                + " enabled: " + enabled + " at User(" + userId + ")");
                        if (!setReceiverRecord(userId, pkgName, enabled)) {
                            Log.w(TAG, "Found a pending settings for package: " + pkgName);
                            synchronized (mPendingSettings) {
                                List<String> pendingSettings = null;
                                if (!mPendingSettings.containsKey(userId)) {
                                    pendingSettings = new ArrayList<String>();
                                    mPendingSettings.put(userId, pendingSettings);
                                }
                                pendingSettings = mPendingSettings.get(userId);
                                if (!pendingSettings.contains(pkgName)) {
                                    pendingSettings.add(pkgName);
                                }
                            }
                        }

                    } else {
                        Log.w(TAG, "Unknown element under <boot-receiver>: "
                                + parser.getName());
                        XmlUtils.skipCurrentTag(parser);
                    }
                }
                success = true;
            } catch (IllegalStateException e) {
                Log.w(TAG, "Failed parsing " + e);
            } catch (NullPointerException e) {
                Log.w(TAG, "Failed parsing " + e);
            } catch (NumberFormatException e) {
                Log.w(TAG, "Failed parsing " + e);
            } catch (XmlPullParserException e) {
                Log.w(TAG, "Failed parsing " + e);
            } catch (IOException e) {
                Log.w(TAG, "Failed parsing " + e);
            } catch (IndexOutOfBoundsException e) {
                Log.w(TAG, "Failed parsing " + e);
            } finally {
                if (!success) {
                    synchronized (mBootReceiverList) {
                        mBootReceiverList.clear();
                    }
                }
                try {
                    stream.close();
                } catch (IOException e) {
                    throw new RuntimeException("Fail to read receiver list");
                }
            }
        }
    }

    private List<UserInfo> getUserList() {
        List<UserInfo> list = null;
        try {
            list = mUm.getUsers(false);
        } catch (RemoteException e) {
            Log.e(TAG, "getUserList() failed!", e);
        }
        return list;
    }

    private PackageInfo getPackageInfoByUser(int userId, String packageName) {
        PackageInfo packageInfo = null;
        try {
            packageInfo = mPm.getPackageInfo(packageName, PackageManager.GET_PERMISSIONS, userId);
        } catch (RemoteException e) {
            Log.e(TAG, "getPackageInfoByUser() failed! with userId: " + userId, e);
        }
        return packageInfo;
    }

    private boolean isSystemApp(int userId, String packageName) {
        PackageInfo pkgInfo = getPackageInfoByUser(userId, packageName);
        if (pkgInfo != null && pkgInfo.applicationInfo != null) {
            int appId = UserHandle.getAppId(pkgInfo.applicationInfo.uid);
            boolean result = ((pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) ||
                (appId == Process.SYSTEM_UID)
                || (pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0;
            return result ;
        } else {
            Log.d(TAG, "isSystemApp() return false with null packageName");
            return false;
        }
    }

    private Map<String, ReceiverRecord> getBootReceiverListByUser(int userId) {
        synchronized (mBootReceiverList) {
            if (mBootReceiverList.containsKey(userId)) {
                return mBootReceiverList.get(userId);
            }
        }
        return null;
    }

    private class FileChangeListener extends FileObserver {
        public FileChangeListener(String path) {
            super(path);
        }

        @Override
        public void onEvent(int event, String path) {
            if (path != null && path.equals(FILE_NAME + FILE_EXTENSION)) {
                switch(event) {
                    case FileObserver.CREATE:
                    case FileObserver.CLOSE_WRITE:
                    case FileObserver.DELETE:
                    case FileObserver.MODIFY:
                        Log.d(TAG, "FileChangeListener.onEvent()," +
                            " event = " + event +
                            ", reload the file.");
                        if (mShouldUpdate) {
                            loadDataFromFileToCache();
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    public void updateReceiverCache() {
        Log.d(TAG, "updateReceiverCache()") ;

        List<UserInfo> userList = getUserList() ;
        if (userList != null) {
            Iterator<UserInfo> itor = userList.iterator();
            while (itor.hasNext()) {
                UserInfo user = (UserInfo) itor.next();
                updateReceiverCache(user.id);
            }
        }
    }

    public void updateReceiverCache(int userId) {
        Log.d(TAG, "updateReceiverCache() at User(" + userId + ")");

        List<ReceiverRecord> oldList = getReceiverList(userId);
        List<String> updateList = getPackageListReceivingSpecifiedIntent(userId);

        for (String packageName : updateList) {
            boolean enabled = true;
            boolean found = false;
            for (ReceiverRecord record : oldList) {
                if (record != null && packageName.equals(record.mPackageName)) {
                    enabled = record.mEnabled;
                    found = true;
                    break;
                }
            }
            if (found == true) {
                setReceiverRecord(userId, packageName, enabled);
            } else {
                if (isPendingSetting(userId, packageName)) {
                    addReceiverRecord(userId, packageName, false);
                    removePendingSetting(userId, packageName);
                } else {
                    addReceiverRecord(userId, packageName, DEFAULT_STATUS);
                }
            }
        }
    }

    // Deep copy of receiver list
    public List<ReceiverRecord> getReceiverList(int userId) {
        List<ReceiverRecord> res = new ArrayList<ReceiverRecord>();
        Map<String, ReceiverRecord> data = getBootReceiverListByUser(userId);
        if (data != null) {
            Iterator<String> itor = data.keySet().iterator();
            while (itor.hasNext()) {
                String pkgName = itor.next();
                res.add(new ReceiverRecord(data.get(pkgName)));
            }
        }
        return res;
    }

    public void resetAllReceiverRecords() {
        Log.d(TAG, "resetAllReceiverRecords()") ;
        List<UserInfo> userList = getUserList() ;
        if (userList != null) {
            Iterator<UserInfo> itor = userList.iterator();
            while (itor.hasNext()) {
                UserInfo user = (UserInfo) itor.next();
                int userId = user.id ;
                List<ReceiverRecord> curList = getReceiverList(userId);

                for (int index = 0 ; index < curList.size() ; index++) {
                     ReceiverRecord record = curList.get(index) ;
                     if (record != null && record.mEnabled == false) {
                         //Log.d(TAG, "resetAllReceiverRecords() - found pkg = "
                         //    + record.mPackageName + " to be reset") ;
                         record.mEnabled = DEFAULT_STATUS ;
                     }
                }
            }
        }
    }

    public void saveAutoBootPackagesInfo() {
        mShouldUpdate = false;
        synchronized (mFile) {
            Map<String, Integer> denyList = new HashMap<String, Integer>();
            List<UserInfo> userList = getUserList();
            if (userList != null) {
                Iterator<UserInfo> itor = userList.iterator();
                while (itor.hasNext()) {
                    UserInfo user = (UserInfo) itor.next();
                    List<ReceiverRecord> list = getReceiverList(user.id);
                    for (ReceiverRecord record : list) {
                        if (!record.mEnabled) {
                            denyList.put(record.mPackageName, user.id);
                        }
                    }
                }
            }

            if (mFile.exists() && (!mFile.getAbsoluteFile().delete())) {
                Log.e(TAG, "saveToFile, deleteFile failed");
            }

            if (denyList.size() == 0) {
                Log.d(TAG, "saveToFile, size = 0");
                mShouldUpdate = true;
                return;
            }

            FileOutputStream os = null;
            try {
                mFile.getAbsoluteFile().createNewFile();
                os = new FileOutputStream(mFile);
                XmlSerializer xml = Xml.newSerializer();
                xml.setOutput(os, "UTF-8");
                xml.startDocument("UTF-8", true);
                xml.startTag(null, "bootlist");

                for (Map.Entry<String, Integer> entry : denyList.entrySet()) {
                    Log.d(TAG, "saveToFile, tag = " + entry.getKey() + " user id:" + entry.getValue());
                    xml.startTag(null, PKG_TAG);
                    xml.attribute(null, "n", entry.getKey());
                    xml.attribute(null, "u", String.valueOf(entry.getValue()));
                    xml.attribute(null, "e", "false");
                    xml.endTag(null, PKG_TAG);
                }

                xml.endTag(null, "bootlist");
                xml.endDocument();
            } catch (IOException | IllegalArgumentException | IllegalStateException e) {
                Log.e(TAG, "saveToFile, exception + " + e);
                e.printStackTrace();
            } finally {
                mShouldUpdate = true;
                if (os != null) {
                    try {
                        os.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }
}