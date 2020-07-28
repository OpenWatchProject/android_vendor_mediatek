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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.voicecommand.cfg;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;
import android.content.res.AssetManager;
import android.os.Environment;
import android.os.FileUtils;
import android.text.TextUtils;
import android.util.Xml;

import com.mediatek.voicecommand.data.ModelDbHelper;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.TreeSet;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class VoiceModelCfg {
    private static final String TAG = "VoiceModelCfg";

    public static final String VOICE_MODEL_PACKAGE_NAME = "com.magiear.handsfree.assistant";
    private static final String[][] PACKAGENAME_SHA256SIG_PAIR = {
        { VOICE_MODEL_PACKAGE_NAME,
                "6E6E503630E3B4B645EA2EA6D9A3B0E2841902CAC840E508E44488C3AD807FB3" } };
    private static final String VOICE_MODEL_CFG_FILE_NAME = "model_cfg.xml";
    private static final String VOICE_MODEL_CFG_TAG = "model_cfg";
    private static final String VOICE_MODEL_TAG = "model";
    private static final String VOICE_MODEL_LOCALE = "locale";
    private static final String VOICE_MODEL_FILE = "file";
    private static final String VOICE_MODEL_SUBFOLDER = "subfolder";
    private static final String VOICE_MODEL_FOLDER = "folder";
    private static final String VOICE_MODEL_STAGE = "stage";
    private static final String VOICE_MODEL_VERSION = "version";
    private static final String VOICE_MODEL_DELETED = "deleted";
    private static final String VOICE_MODEL_WRITEOUT = "writeout";
    private static final String VOICE_MODEL_DEFAULT = "default";
    private static final String VOICE_MODEL_AUDIOHAL_DIR = "vendor/audiohal";
    private static final String VOICE_MODEL_DIR = "voice_model";
    private static final String VOICE_MODEL_LIST = "voice_model_upgrade_cfg.txt";
    private static final String VOICE_MODEL_MTK_SUPPLEMENT = "supplement";
    private final Context mContext;

    public VoiceModelCfg(Context context) {
        Log.i(TAG, "[VoiceModelCfgXml]new...");
        mContext = context;
    }

    private AssetManager getAssetManager(Context context, String packageName)
            throws NameNotFoundException {
        Context targetContext = context.createPackageContext(packageName, 0);
        return targetContext.getAssets();
    }

    public void parseModelInfosInPackage(String packageName, ArrayList<VoiceModelInfo> modelInfos) {
        InputStream in = null;
        AssetManager assetManager = null;
        try {
            assetManager = getAssetManager(mContext, packageName);
            int xmlEventType;
            in = assetManager.open(VOICE_MODEL_CFG_FILE_NAME);
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(in, "UTF-8");
            boolean cfgAvailable = false;
            VoiceModelInfo modelInfo = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (VOICE_MODEL_CFG_TAG.equals(name)) {
                    if (xmlEventType == XmlPullParser.START_TAG) {
                        cfgAvailable = true;
                    } else if (xmlEventType == XmlPullParser.END_TAG) {
                        cfgAvailable = false;
                    }
                } else if (cfgAvailable && VOICE_MODEL_TAG.equals(name)) {
                    if (xmlEventType == XmlPullParser.START_TAG) {
                        modelInfo = new VoiceModelInfo();
                        modelInfo.mPackageName = packageName;
                        modelInfo.mLocale = parser.getAttributeValue(null, VOICE_MODEL_LOCALE);
                        modelInfo.mFileName = parser.getAttributeValue(null, VOICE_MODEL_FILE);
                        modelInfo.mSubFolder = parser.
                                getAttributeValue(null, VOICE_MODEL_SUBFOLDER);
                        modelInfo.mFolder = parser.getAttributeValue(null, VOICE_MODEL_FOLDER);
                        String str = parser.getAttributeValue(null, VOICE_MODEL_STAGE);
                        if (str != null) {
                            modelInfo.mStageNo = Integer.parseInt(str);
                        }
                        str = parser.getAttributeValue(null, VOICE_MODEL_VERSION);
                        if (str != null) {
                            modelInfo.mVersion = Integer.parseInt(str);
                        }
                        str = parser.getAttributeValue(null, VOICE_MODEL_DELETED);
                        if (str != null) {
                            modelInfo.mDeleted = Boolean.parseBoolean(str);
                        }
                        str = parser.getAttributeValue(null, VOICE_MODEL_WRITEOUT);
                        if (str != null) {
                            modelInfo.mNeedWriteOut = Boolean.parseBoolean(str);
                        }
                        str = parser.getAttributeValue(null, VOICE_MODEL_DEFAULT);
                        if (str != null) {
                            modelInfo.mDefault = Boolean.parseBoolean(str);
                        }
                    } else if (xmlEventType == XmlPullParser.END_TAG) {
                        if (modelInfo != null && modelInfo.isAvalible()) {
                            modelInfos.add(modelInfo);
                            Log.d(TAG, "get model info:" + modelInfo);
                        } else {
                            Log.d(TAG, "fail model info: " + modelInfo);
                        }
                    }
                }
            }
            Log.d(TAG, "get model info size: " + modelInfos.size() + " from:" + packageName);
        } catch (XmlPullParserException e) {
            Log.e(TAG, "[parseModelInfosInPackage]XmlPullParserException:", e);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "[parseModelInfosInPackage]FileNotFoundException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[parseModelInfosInPackage]IOException:", e);
        } catch (NameNotFoundException e) {
            Log.e(TAG, "[parseModelInfosInPackage]NameNotFoundExp:", e);
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "[readKeyWordFromXml]close IOException:", e);
            }
        }
    }

    public boolean updateModels(ModelDbHelper dbHelper, ArrayList<VoiceModelInfo> modelInfos) {
        if (dbHelper == null || modelInfos == null || modelInfos.size() == 0) {
            Log.w(TAG, "updateModels dbHelper: " + dbHelper + ", modelInfos: "
                    + modelInfos + ", modelInfos.size(): " + modelInfos.size());
            return false;
        }
        boolean isNeedUpdate = updateModelStorage(dbHelper, modelInfos);
        if (isUpgradeVersion(modelInfos)) {
            generateModelUpgradeListForUpgrade(modelInfos);
            Log.d(TAG, "generateModelUpgradeListForUpgrade done");
        } else {
            generateModelUpgradeListForBase();
            Log.d(TAG, "generateModelUpgradeListForBase done");
        }
        return isNeedUpdate;
     }

    /**
     * Check if is the upgrade version, because if is not upgrade version,
     * there will not have mtk voice model in handsfree assistant.
     * And the voice_model_upgrade_cfg.txt should be write 0 0.
     * @param modelInfos
     * @return
     */
    private boolean isUpgradeVersion(ArrayList<VoiceModelInfo> modelInfos) {
        boolean isUpgrade = false;
        for (int i = 0; i < modelInfos.size(); i++) {
            VoiceModelInfo info = modelInfos.get(i);
            if (!info.mDeleted && info.mNeedWriteOut) {
                isUpgrade = true;
            }
        }
        Log.d(TAG, "isUpgradeVersion: " + isUpgrade);
        return isUpgrade;
    }

    private void generateModelUpgradeListForBase() {
        File file = getUbmUpgradeFileForBase();
        //Write to the file
        BufferedWriter os = null;
        try {
            //If just out of the factory:
            // 0
            // 0
            os = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
            os.write(0 + "\r\n");
            os.write(0 + "\r\n");
            os.flush();
        } catch (IOException e) {
            Log.d(TAG, "output stream error", e);
        } finally {
            if (os != null) {
                try {
                    os.close();
                } catch (IOException e) {
                    Log.d(TAG, "output stream error", e);
                }
            }
        }
        if (file.exists()) {
            setUbmFileUpgradePath(file.getAbsolutePath());
        }

        int successFile = FileUtils.setPermissions(file.getPath(), 0640, -1, -1);
    }

    private void generateModelUpgradeListForUpgrade(ArrayList<VoiceModelInfo> modelInfos) {
        if (modelInfos == null) {
            Log.w(TAG, "generateModelUpgradeListForUpgrade modelInfo is null");
            return;
        }
        File file = getUbmUpgradeFile();
        HashSet<String> modelUpgradeSet = new HashSet();
        TreeSet<String> modelUpgradeTreeSet = new TreeSet();
        HashSet<String> modelUpgradeSupplementSet = new HashSet();
        String supplementPath = null;
        for (int i = 0; i < modelInfos.size(); i++) {
            VoiceModelInfo info = modelInfos.get(i);
            if (!info.mDeleted && info.mNeedWriteOut) {
                File folder = getTargetFolder(info);
                Log.d(TAG, "folder: " +  folder.getAbsolutePath());
                File[] files = folder.listFiles();
                if (files == null) {
                    Log.w(TAG, "generateModelUpgradeListForUpgrade files is null");
                    continue;
                }
                for (File upgradeFile : files) {
                    String path = upgradeFile.getAbsolutePath();
                    String folderName = path.substring(path.lastIndexOf("/") + 1);
                    //Supplement update of directory, such as:
                    //etc/mediatek.com.testAPK/cfg/supplement
                    //2.Pcm
                    //4.Pcm
                    //6.pcm
                    if (VOICE_MODEL_MTK_SUPPLEMENT.equals(folderName)) {
                        //Get supplement path
                        supplementPath = path;
                        File[] supplementFile = upgradeFile.listFiles();
                        for (File supplement : supplementFile) {
                            String supplementAbsolutePath = supplement.getAbsolutePath();
                            modelUpgradeSupplementSet.add(supplementAbsolutePath.
                                    substring(supplementAbsolutePath.lastIndexOf("/") + 1));
                        }
                    } else {
                        //Necessary update of directory, such as:
                        //etc/mediatek.com.testAPK/cfg/p2
                        //etc/mediatek.com.testAPK/cfg/p2.3
                        //etc/mediatek.com.testAPK/cfg/p2.5
                        modelUpgradeSet.add(path);
                    }
                }

            }
            modelUpgradeTreeSet.addAll(modelUpgradeSet);
        }
        //Write to the file
        BufferedWriter os = null;
        try {
            os = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
            os.write(modelUpgradeTreeSet.size() + "\r\n");
            for (String str : modelUpgradeTreeSet) {
                os.write(str + "\r\n");
            }
            os.write(modelUpgradeSupplementSet.size() + "\r\n");
            if (modelUpgradeSupplementSet.size() > 0) {
                os.write(supplementPath + "\r\n");
                for (String str : modelUpgradeSupplementSet) {
                    os.write(str + "\r\n");
                }
            }
            os.flush();
        } catch (IOException e) {
            Log.d(TAG, "output stream error", e);
        } finally {
            if (os != null) {
                try {
                    os.close();
                } catch (IOException e) {
                    Log.d(TAG, "output stream error", e);
                }
            }
        }
        if (file.exists()) {
            setUbmFileUpgradePath(file.getAbsolutePath());
        }

        int successFile = FileUtils.setPermissions(file.getPath(), 0640, -1, -1);
    }

    private boolean updateModelStorage(ModelDbHelper dbHelper,
                                       ArrayList<VoiceModelInfo> modelInfos) {
        String packageName = null;
        AssetManager asset = null;
        Log.d(TAG, "begin get model info");
        boolean isNeedUpdate = false;
        for (int i = 0; i < modelInfos.size(); i++) {
            VoiceModelInfo info = modelInfos.get(i);
            if (info != null) {
                if (packageName != info.mPackageName) {
                    packageName = info.mPackageName;
                    try {
                        asset = getAssetManager(mContext, packageName);
                    } catch (NameNotFoundException e) {
                        Log.d(TAG, "getAssetManager for:" + packageName, e);
                        continue;
                    }
                }
                int version = dbHelper.getVoiceModelVersion(info);
                //Must handle apk upgrade or downgrade
                if (!info.mDeleted && (version == 0 || version != info.mVersion)) {
                    fillVoiceModelData(info, asset);
                }
                if (version == 0) {
                    if (info.mDeleted) {
                        // No need to add
                        continue;
                    } else {
                        // Add
                        if (addVoiceModel(dbHelper, info)) {
                            isNeedUpdate = true;
                        } else {
                            Log.d(TAG, "add voice model fail for:" + info);
                        }
                    }
                } else if (info.mDeleted) {
                    // Delete
                    if (deleteVoiceModel(dbHelper, info)) {
                        isNeedUpdate = true;
                    } else {
                        Log.d(TAG, "delete voice model fail for:" + info);
                    }
                    //Must handle apk upgrade or downgrade
                } else if (version != info.mVersion) {
                    // Update
                    if (updateVoiceModel(dbHelper, info, version)) {
                        isNeedUpdate = true;
                    } else {
                        Log.d(TAG, "update voice model fail for:" + info);
                    }
                }
            }
        }
        Log.d(TAG, "updateModelStorage isNeedUpdate: " + isNeedUpdate);
        return isNeedUpdate;
    }

    private void setUbmFileUpgradePath(String path) {
        ConfigurationManager.getInstance(mContext).setUbmFileUpgradePath(path);
    }

    private boolean addVoiceModel(ModelDbHelper dbHelper, VoiceModelInfo info) {
        boolean retDB = false;
        boolean retData = false;
        if (!info.mNeedWriteOut) {
            dbHelper.addVoiceModel(info, true);
            return false;
        } else {
            // mtk voice model no need to save database because of size too large.
            retDB = dbHelper.addVoiceModel(info, false);
            retData = unZipNewModel(info);
            // Only care mtk voice model is update, then retrain
            return retDB && retData;
        }
    }

    private boolean updateVoiceModel(ModelDbHelper dbHelper, VoiceModelInfo info, int version) {
        boolean retDB = false;
        boolean retData = false;
        boolean retDeleteOldData = false;

        if (!info.mNeedWriteOut) {
            dbHelper.updateVoiceModel(info, true);
            return false;
        } else {
            retDB = dbHelper.updateVoiceModel(info, false);
            //UnZip new voice model
            retData = unZipNewModel(info);
            //Delete old voice model
            retDeleteOldData = deleteOldModel(info, version);
            // Only care mtk voice model is update, then retrain
            return retDB && retData && retDeleteOldData;
        }
    }

    private boolean deleteVoiceModel(ModelDbHelper dbHelper, VoiceModelInfo info) {
        dbHelper.deleteVoiceModel(info);
        if (info.mNeedWriteOut) {
            // Only care mtk voice model is update, then retrain
            return deleteZipVoiceModel(info);
        }
        return false;
    }

    private File getTargetFile(VoiceModelInfo info) {
        //data/vendor/audiohal/voice_model
        File dataFile = new File(Environment.getDataDirectory() + "/" + VOICE_MODEL_AUDIOHAL_DIR);
        File targetFile = new File(dataFile, VOICE_MODEL_DIR + "/" + info.getResPath());
        File parentFile = targetFile.getParentFile();
        parentFile.mkdirs();
        int successParent = FileUtils.setPermissions(parentFile.getPath(), 0710, -1, -1);
        return targetFile;
    }

    private File getTargetFolder(VoiceModelInfo info) {
        //data/vendor/audiohal/voice_model/mtk_1
        File dataFile = new File(Environment.getDataDirectory() + "/" + VOICE_MODEL_AUDIOHAL_DIR);
        File targetFile = new File(dataFile, VOICE_MODEL_DIR + "/"
                + info.getResFolder(info.mVersion));
        File parentFile = targetFile.getParentFile();
        parentFile.mkdirs();
        int successParent = FileUtils.setPermissions(parentFile.getPath(), 0710, -1, -1);
        return targetFile;
    }

    private boolean unZipNewModel(VoiceModelInfo info) {
        // Create the directory folder
        File file = getTargetFolder(info);
        if (!file.exists()) {
            file.mkdirs();
        }
        try {
            //InputStream inputStream = context.getAssets().open(assetName);
            ZipInputStream zipInputStream =
                    new ZipInputStream(new ByteArrayInputStream(info.mData));
            ZipEntry zipEntry = null;
            zipEntry = zipInputStream.getNextEntry();
            while (zipEntry != null) {
                // If it is a directory
                if (zipEntry.isDirectory()) {
                    File dir = new File(file + File.separator +
                            zipEntry.getName().substring(0, zipEntry.getName().length() - 1));
                    if (!dir.exists()) {
                        dir.mkdirs();
                    }
                    int successDir = FileUtils.setPermissions(dir.getPath(), 0710, -1, -1);
                } else {
                    // If it is file
                    unZipNewModelFile(zipEntry, file, zipInputStream);
                }
                // Go to the next entry
                zipEntry = zipInputStream.getNextEntry();
            }
            zipInputStream.close();
            int successDir = FileUtils.setPermissions(file.getPath(), 0710, -1, -1);
            Log.d(TAG, "unZipNewModel successDir: " + successDir);
        } catch (IOException e) {
            Log.w(TAG, "unzipFile exception: " + e.getMessage());
            return false;
        }
        return true;
    }

    private void unZipNewModelFile(ZipEntry zipEntry, File file, ZipInputStream zipInputStream) {
        // If it is file
        try {
            byte[] buffer = new byte[1024];
            int count = 0;
            String fileName = zipEntry.getName();
            //fileName = fileName.substring(fileName.lastIndexOf("/") + 1);
            File newFile = new File(file + File.separator + fileName);
            Log.d(TAG, "unZipNewModelFile newFile: " + newFile.getAbsolutePath());
            FileOutputStream fileOutputStream = null;

            fileOutputStream = new FileOutputStream(newFile);

            while ((count = zipInputStream.read(buffer)) > 0) {
                fileOutputStream.write(buffer, 0, count);
            }
            fileOutputStream.close();
            // Must set permission for AudioHal to access.
            int successFile = FileUtils.setPermissions(newFile.getPath(), 0640, -1, -1);
        } catch (FileNotFoundException e) {
            Log.w(TAG, e.getMessage(), e);
        } catch (IOException e) {
            Log.w(TAG, e.getMessage(), e);
        }
    }

    private boolean deleteOldModel(VoiceModelInfo info, int version) {
        //data/vendor/audiohal/voice_model/mtk_1
        File dataFile = new File(Environment.getDataDirectory() + "/" + VOICE_MODEL_AUDIOHAL_DIR);
        File targetFile = new File(dataFile, VOICE_MODEL_DIR + "/"
                + info.getResFolder(version));
        return deleteFile(targetFile);
    }

    private boolean deleteZipVoiceModel(VoiceModelInfo info) {
        //data/vendor/audiohal/voice_model/mtk_1
        File dataFile = new File(Environment.getDataDirectory() + "/" + VOICE_MODEL_AUDIOHAL_DIR);
        File targetFile = new File(dataFile, VOICE_MODEL_DIR + "/"
                + info.getResFolder(info.mVersion));
        return deleteFile(targetFile);
    }

    private boolean deleteFile(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                file.delete();
            } else if (file.isDirectory()) {
                File files[] = file.listFiles();
                for (int i = 0; i < files.length; i++) {
                    this.deleteFile(files[i]);
                }
            }
            file.delete();
            Log.d(TAG, "deleteFile success");
            return true;
        } else {
            Log.d(TAG, "deleteFile file is not exists");
            return false;
        }
    }

    private File getUbmUpgradeFileForBase() {
        //data/vendor/audiohal/voice_model/
        File dataFile = new File(Environment.getDataDirectory() + "/" + VOICE_MODEL_AUDIOHAL_DIR);
        File ubmFile = new File(dataFile, VOICE_MODEL_DIR + "/" + VOICE_MODEL_LIST);
        File parentFile = ubmFile.getParentFile();
        parentFile.mkdirs();
        int successParent = FileUtils.setPermissions(parentFile.getPath(), 0710, -1, -1);
        return ubmFile;
    }

    private File getUbmUpgradeFile() {
        //data/vendor/audiohal/voice_model/
        File dataFile = new File(Environment.getDataDirectory() + "/" + VOICE_MODEL_AUDIOHAL_DIR);
        File ubmFile = new File(dataFile, VOICE_MODEL_DIR + "/" + VOICE_MODEL_LIST);
        File parentFile = ubmFile.getParentFile();
        parentFile.mkdirs();
        int successParent = FileUtils.setPermissions(parentFile.getPath(), 0710, -1, -1);
        return ubmFile;
    }

    private boolean fillVoiceModelData(VoiceModelInfo info, AssetManager asset) {
        InputStream in = null;
        ByteArrayOutputStream outStream = null;
        try {
            in = asset.open(info.getResPath());
            outStream = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int byteCount = 0;
            while ((byteCount = in.read(buffer)) != -1) {
                outStream.write(buffer, 0, byteCount);
            }
            info.mData = outStream.toByteArray();
            return true;
        } catch (IOException e) {
            Log.d(TAG, "read data error:", e);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    Log.d(TAG, "read data error:", e);
                }
            }
            if (outStream != null) {
                try {
                    outStream.close();
                } catch (IOException e) {
                    Log.d(TAG, "read data error:", e);
                }
            }
        }
        return false;
    }

    public static boolean isAvailablePkg(Context context, String pkgName) {
        if (VOICE_MODEL_PACKAGE_NAME.equals(pkgName)) {
            if (checkSign(context, pkgName)) {
                return true;
            }
        }
        return false;
    }

    private static boolean checkSign(Context context, String pkgName) {
        boolean result = false;
        PackageManager pm = context.getPackageManager();
        if (pkgName != null) {
            try {
                PackageInfo pi = pm.getPackageInfo(pkgName,
                        PackageManager.GET_SIGNING_CERTIFICATES);
                if (pi != null) {
                    Signature[] signatures = pi.signingInfo
                            .getApkContentsSigners();
                    if (signatures != null) {
                        for (Signature s : signatures) {
                            byte[] certHash256 = getCertHash(s, "SHA-256");
                            result = matches(certHash256, pkgName);
                            if (!result) {
                                Log.d(TAG, "checkSign failed: pkg=" + pkgName);
                            } else {
                                break;
                            }
                        }
                    }
                }
            } catch (PackageManager.NameNotFoundException e) {
                Log.w(TAG, e.getMessage(), e);
            }
        }
        return result;
    }

    private static byte[] getCertHash(Signature signature, String algo) {
        try {
            MessageDigest md = MessageDigest.getInstance(algo);
            return md.digest(signature.toByteArray());
        } catch (NoSuchAlgorithmException e) {
            Log.w(TAG, e.getMessage(), e);
        }
        return null;
    }

    private static boolean matches(byte[] certHash, String packageName) {
        for (String[] pkgSsigP : PACKAGENAME_SHA256SIG_PAIR) {
            if (!TextUtils.isEmpty(packageName)
                    && packageName.equals(pkgSsigP[0])) {
                if (certHash != null
                        && Arrays.equals(hexs2Bytes(pkgSsigP[1]), certHash))
                    return true;
            }
        }
        return false;
    }

    private static byte[] hexs2Bytes(String data) {
        if (data == null || data.length() % 2 != 0) {
            Log.w(TAG, "The length of hex string is not even numbers");
            return null;
        }
        return hexStringToBytes(data);
    }

    private static byte[] hexStringToBytes(String s) {
        byte[] ret;
        if (s == null)
            return null;
        int sz = s.length();
        ret = new byte[sz / 2];
        for (int i = 0; i < sz; i += 2) {
            ret[i / 2] = (byte) ((hexCharToInt(s.charAt(i)) << 4) | hexCharToInt(s
                    .charAt(i + 1)));
        }
        return ret;
    }

    private static int hexCharToInt(char c) {
        if (c >= '0' && c <= '9')
            return (c - '0');
        if (c >= 'A' && c <= 'F')
            return (c - 'A' + 10);
        if (c >= 'a' && c <= 'f')
            return (c - 'a' + 10);
        throw new RuntimeException("invalid hex char '" + c + "'");
    }
}