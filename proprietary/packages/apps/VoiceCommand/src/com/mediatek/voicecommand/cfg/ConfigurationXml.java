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
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.content.res.Resources.NotFoundException;
import android.os.SystemProperties;
import android.util.Xml;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;

public class ConfigurationXml {
    private static final String TAG = "ConfigurationXml";
    
    public static final String sPublicFeatureName = "android.mediatek.feature";
    
    private static final String VOICE_PROCESS_INFO_VOICE_PROCESS_INFO = "VoiceProcessInfo";
    private static final String VOICE_PROCESS_INFO_FEATURE_NAME = "FeatureName";
    private static final String VOICE_PROCESS_NAME = "ProcessName";
    private static final String VOICE_PROCESS_INFO_ID = "ID";
    private static final String VOICE_PROCESS_INFO_COMMAND_ID = "CommandID";
    private static final String VOICE_PROCESS_INFO_PERMISSION_ID = "PermissionID";
    private static final String VOICE_PROCESS_INFO_ENABLE = "Enable";
    
    private static final String VOICE_KEYWORD = "KeyWord";
    private static final String VOICE_KEYWORD_INFO = "KeyWordInfo";
    
    private final Context mContext;
    
    public ConfigurationXml(Context context) {
        Log.i(TAG, "[ConfigurationXml]new...");
        mContext = context;
    }

    /**
     * Read Keyword from res.
     * 
     * @param voiceKeyWordInfos
     *            voice KeyWord Infos instance
     * 
     * @param fileName
     *            file name in res
     */
    public void readKeyWordFromXml(HashMap<String, VoiceKeyWordInfo> voiceKeyWordInfos,
            String fileName) {
        Log.i(TAG, "[readKeyWordFromXml]from file:" + fileName);
        InputStream in = null;
        AssetManager assetManager = mContext.getAssets();
        try {
            int xmlEventType;
            String keyWord = null;
            String processName = null;
            String path = null;
            in = assetManager.open(fileName);
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(in, "UTF-8");
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG && VOICE_KEYWORD_INFO.equals(name)) {
                    processName = parser.getAttributeValue(null, VOICE_PROCESS_NAME);
                    keyWord = parser.getAttributeValue(null, VOICE_KEYWORD);
                    path = parser.getAttributeValue(null, "Path");
                } else if (xmlEventType == XmlPullParser.END_TAG && VOICE_KEYWORD_INFO.equals(name)) {
                    if (processName != null && keyWord != null) {
                        Log.d(TAG, "[readKeyWordFromXml] processName   = " + processName
                                + ",KeyWord =" + keyWord);
                        String[] keyWordArray = keyWord.split(",");
                        VoiceKeyWordInfo voiceKeyWordInfo = new VoiceKeyWordInfo(keyWordArray, path);
                        voiceKeyWordInfos.put(processName, voiceKeyWordInfo);
                    } else {
                        Log.e(TAG, "[readKeyWordFromXml]Error processName or keyWord :" + keyWord);
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.e(TAG, "[readKeyWordFromXml]XmlPullParserException:", e);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "[readKeyWordFromXml]FileNotFoundException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[readKeyWordFromXml]IOException:", e);
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

    /**
     * Read language info from res.
     * 
     * @param languageList
     *            language list instance
     */
    public int readVoiceLanguangeFromXml(ArrayList<VoiceLanguageInfo> languageList) {
        int curIndex = -1;
        Log.i(TAG, "[readVoiceLanguangeFromXml]...");
        if (languageList == null) {
            Log.w(TAG, "[readVoiceLanguangeFromXml]languageList is null,return.");
            return curIndex;
        }

        XmlPullParser parser = mContext.getResources().getXml(R.xml.voicelanguage);

        if (parser == null) {
            Log.w(TAG, "[readVoiceLanguangeFromXml]parser is null,return.");
            return curIndex;
        }
        String curlanguage = null;
        try {
            int xmlEventType;
            String languageName = null;
            String filepath = null;
            String id = null;
            String code = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG && "Language".equals(name)) {
                    languageName = parser.getAttributeValue(null, "TypeName");
                    filepath = parser.getAttributeValue(null, "FileName");
                    id = parser.getAttributeValue(null, "ID");
                    code = parser.getAttributeValue(null, "Code");

                } else if (xmlEventType == XmlPullParser.END_TAG && "Language".equals(name)) {
                    if (languageName != null && filepath != null && id != null) {
                        Log.d(TAG, "[readVoiceLanguangeFromXml] languageName   = " + languageName
                                + ",filepath =" + filepath + ",id = " + id + ",code = " + code);
                        VoiceLanguageInfo info = new VoiceLanguageInfo(
                        // languageName, mVoiceKeyWordPath + filepath,
                                languageName, filepath, Integer.valueOf(id), code);
                        languageList.add(info);
                    }
                } else if (xmlEventType == XmlPullParser.START_TAG
                        && "DefaultLanguage".equals(name)) {
                    curlanguage = parser.getAttributeValue(null, "ID");
                }
            }

        } catch (XmlPullParserException e) {
            Log.e(TAG, "[readVoiceLanguangeFromXml]XmlPullParserException:", e);
        } catch (NotFoundException e) {
            Log.e(TAG, "[readVoiceLanguangeFromXml]NotFoundException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[readVoiceLanguangeFromXml]IOException:", e);
        }

        Log.d(TAG, "[readVoiceLanguangeFromXml] curlanguage:" + curlanguage);
        if (curlanguage != null) {
            // String mCurrentLanguage = currentLanguage;
            int curlanguageID = Integer.valueOf(curlanguage).intValue();
            for (int i = 0; i < languageList.size(); i++) {
                if (languageList.get(i).mLanguageID == curlanguageID) {
                    curIndex = i;
                    break;
                }
            }
            if (curIndex < 0) {
                curIndex = 0;
            }
        }
        Log.d(TAG, "[readVoiceLanguangeFromXml] curIndex:" + curIndex);

        return curIndex;
    }

    /**
     * Read customization info from res.
     * 
     * @param voiceCustomization
     *            VoiceCustomization instance
     */
    public void readVoiceCustomizationFromXml(VoiceCustomization voiceCustomization) {
        Log.i(TAG, "[readVoiceCustomizationFromXml]...");
        boolean isSystemLanguage = false;
        String systemLanguage = null;
        String defaultLanguage = null;

        XmlPullParser parser = mContext.getResources().getXml(R.xml.voicecustomization);
        if (parser == null) {
            Log.w(TAG, "[readVoiceCustomizationFromXml]parser is null,return.");
            return;
        }
        try {
            int xmlEventType;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG && "VoiceCustomization".equals(name)) {
                    systemLanguage = parser.getAttributeValue(null, "SystemLanguage");
                    defaultLanguage = parser.getAttributeValue(null, "DefaultLanguage");
                }
            }
            Log.d(TAG, "[readVoiceCustomizationFromXml]systemLanguage:" + systemLanguage
                    + ",defaultLanguage:" + defaultLanguage);
        } catch (XmlPullParserException e) {
            Log.e(TAG, "[readVoiceCustomizationFromXml]XmlPullParserException:", e);
        } catch (NotFoundException e) {
            Log.e(TAG, "[readVoiceCustomizationFromXml]NotFoundException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[readVoiceCustomizationFromXml]IOException:", e);
        }

        if ("TRUE".equals(systemLanguage)) {
            isSystemLanguage = true;
        }
        voiceCustomization.mIsSystemLanguage = isSystemLanguage;
        voiceCustomization.mDefaultLanguage = defaultLanguage;
    }

    /**
     * Read voice file path info from res.
     * 
     * @param pathMap
     *            pathmap instance
     */
    public void readVoiceCommandPathFromXml(HashMap<String, String> pathMap) {
        Log.i(TAG, "[readVoiceCommandPathFromXml]...");
        XmlPullParser parser = mContext.getResources().getXml(R.xml.voicecommandpath);
        if (parser == null) {
            Log.w(TAG, "[readVoiceCommandPathFromXml]parser is null,return.");
            return;
        }

        try {
            int xmlEventType;
            String processName = null;
            String path = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG && "Path".equals(name)) {
                    processName = parser.getAttributeValue(null, "Name");
                    path = parser.getAttributeValue(null, "Path");
                } else if (xmlEventType == XmlPullParser.END_TAG && "Path".equals(name)) {
                    if (processName != null & path != null) {
                        Log.d(TAG, "[readVoiceCommandPathFromXml] processName   = " + processName
                                + ",path =" + path);
                        pathMap.put(processName, path);
                    } else {
                        Log.d(TAG,
                                "[readVoiceCommandPathFromXml]the package has no voice command path ");
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.e(TAG, "[readVoiceCommandPathFromXml]XmlPullParserException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[readVoiceCommandPathFromXml]IOException:", e);
        } catch (NotFoundException e) {
            Log.e(TAG, "[readVoiceCommandPathFromXml]NotFoundException: ", e);
        }
    }

    /**
     * Read voice process info from res.
     * 
     * @param voiceProcessInfos
     *            voice Process infos instance
     * 
     * @param voiceUiList
     *            Voice Ui list instance
     */
    public void readVoiceProcessInfoFromXml(HashMap<String, VoiceProcessInfo> voiceProcessInfos,
            ArrayList<String> voiceUiList) {
        Log.i(TAG, "[readVoiceProcessInfoFromXml]...");
        XmlPullParser parser = mContext.getResources().getXml(R.xml.voiceprocessinfo);
        if (parser == null) {
            Log.w(TAG, "[readVoiceProcessInfoFromXml]parser is null,return.");
            return;
        }
        try {
            int xmlEventType;
            String featureName = null;
            VoiceProcessInfo voiceProcessInfo = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {

                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG && VOICE_PROCESS_INFO_VOICE_PROCESS_INFO.equals(name)) {

                    featureName = parser.getAttributeValue(null, VOICE_PROCESS_INFO_FEATURE_NAME);
                    if (featureName != null) {

                        voiceProcessInfo = new VoiceProcessInfo(featureName);

                        String processName = parser.getAttributeValue(null, VOICE_PROCESS_NAME);
                        if (processName == null) {
                            Log.v(TAG, " [readVoiceProcessInfoFromXml]voiceInfo XML processName = NULL");
                            continue;
                        }

                        voiceProcessInfo.mRelationProcessName = parser.getAttributeValue(null,
                                "RelationProcess");

                        String idStr = parser.getAttributeValue(null, VOICE_PROCESS_INFO_ID);
                        if (idStr == null) {
                            Log.d(TAG, " [readVoiceProcessInfoFromXml]voiceInfo XML ID = NULL");
                            continue;
                        }
                        voiceProcessInfo.mID = Integer.parseInt(idStr);
                        
                        String commandID = parser.getAttributeValue(null, VOICE_PROCESS_INFO_COMMAND_ID);
                        if (commandID == null) {
                            Log.d(TAG, " [readVoiceProcessInfoFromXml]voiceInfo XML commandID = NULL");
                            continue;
                        }

                        String permissionID = parser
                                .getAttributeValue(null, VOICE_PROCESS_INFO_PERMISSION_ID);
                        if (permissionID == null) {
                            Log.d(TAG, "[readVoiceProcessInfoFromXml]voiceInfo XML PermissionID = NULL");
                            continue;
                        }

                        String voiceEnable = parser.getAttributeValue(null, VOICE_PROCESS_INFO_ENABLE);
                        if (voiceEnable == null) {
                            Log.v(TAG, " [readVoiceProcessInfoFromXml]voiceInfo XML voiceEnable1 = NULL");
                            continue;
                        }
                        voiceProcessInfo.mIsVoiceEnable = voiceEnable.equals("TRUE") ? true : false;

                        Log.d(TAG, "[readVoiceProcessInfoFromXml] featureName = " + featureName
                                + ",processName = " + processName + ",commandID =" + commandID
                                + ",permissionID = " + permissionID + ",voiceEnable=" + voiceEnable);

                        if (commandID != null) {
                            String[] commandIDTemp = commandID.split(",");
                            for (int i = 0; i < commandIDTemp.length; i++) {
                                // Log.v(TAG,"commandIDTemp[i] = "+Integer.valueOf(commandIDTemp[i]));
                                voiceProcessInfo.mCommandIDList.add(Integer
                                        .valueOf(commandIDTemp[i]));
                            }
                        }

                        if (processName != null) {
                            String[] processNameTemp = processName.split(",");
                            for (int i = 0; i < processNameTemp.length; i++) {
                                Log.d(TAG, "[readVoiceProcessInfoFromXml],processNameTemp[i] = " + processNameTemp[i]);
                                voiceProcessInfo.mProcessNameList.add(processNameTemp[i]);
                            }
                        }

                        if (permissionID != null) {
                            String[] permissionIDTemp = permissionID.split(",");
                            for (int i = 0; i < permissionIDTemp.length; i++) {
                                // Log.v(TAG,"permissionIDTemp[i] = "+Integer.valueOf(permissionIDTemp[i]));
                                int permissionid = Integer.valueOf(permissionIDTemp[i]);
                                if (permissionid == VoiceCommandListener.ACTION_MAIN_VOICE_UI
                                        && (voiceProcessInfo.mRelationProcessName == null || !voiceProcessInfo.mRelationProcessName
                                                .endsWith(sPublicFeatureName))
                                        && checkPackageIllegal(voiceProcessInfo.mFeatureName)) {
                                    voiceUiList.add(voiceProcessInfo.mFeatureName);
                                }
                                voiceProcessInfo.mPermissionIDList.add(Integer
                                        .valueOf(permissionIDTemp[i]));
                            }
                        }

                    } else {
                        Log.d(TAG,
                                "[readVoiceProcessInfoFromXml]the package has no voice command info ");
                    }
                } else if (xmlEventType == XmlPullParser.END_TAG && VOICE_PROCESS_INFO_VOICE_PROCESS_INFO.equals(name)) {
                    if (featureName != null && voiceProcessInfo != null) {
                        voiceProcessInfos.put(featureName, voiceProcessInfo);
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.e(TAG, "[readVoiceProcessInfoFromXml]XmlPullParserException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[readVoiceProcessInfoFromXml]IOException:", e);
        } catch (NotFoundException e) {
            Log.e(TAG, "[readVoiceProcessInfoFromXml]NotFoundException:", e);
        }
    }

    /**
     * Read voice Wake up info from assert.
     * 
     * @param wakeupList
     *            wakeup Info list instance
     * 
     * @param fileName
     *            file name in assets
     */
    public void readVoiceWakeupFromXml(ArrayList<VoiceWakeupInfo> wakeupList, String fileName) {
        Log.i(TAG, "[readVoiceWakeupFromXml]fileName:" + fileName);
        InputStream in = null;
        AssetManager assetManager = mContext.getAssets();
        try {
            in = assetManager.open(fileName);
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(in, "UTF-8");
            int xmlEventType;
            int idIntValue = 0;
            String packageName = null;
            String className = null;
            String actionName = null;
            String id = null;
            String keyWord = null;
            String checkGMS = null;
            VoiceWakeupInfo info = null;

            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG && "VoiceWakeupInfo".equals(name)) {

                    packageName = parser.getAttributeValue(null, "PackageName");
                    className = parser.getAttributeValue(null, "ClassName");
                    actionName = parser.getAttributeValue(null, "ActionName");
                    id = parser.getAttributeValue(null, "ID");
                    keyWord = parser.getAttributeValue(null, "KeyWord");
                    checkGMS = parser.getAttributeValue(null, "CheckGMS");
                } else if (xmlEventType == XmlPullParser.END_TAG && "VoiceWakeupInfo".equals(name)) {
                    Log.d(TAG, "[readVoiceWakeupFromXml] packageName = " + packageName
                            + ", actionName = " + actionName
                            + ",className = " + className + ",id =" + id + ",keyWord = " + keyWord);
                    info = null;

                    if (packageName != null && (className != null || actionName != null)) {
                        if (checkGMS != null) {
                            if (!checkGmsInstalled() && (Integer.valueOf(checkGMS) == 1)) {
                                Log.d(TAG, "[readVoiceWakeupFromXml] skipping gms package");
                                continue;
                            } else if (checkGmsInstalled() && (Integer.valueOf(checkGMS) == 0)) {
                                Log.d(TAG, "[readVoiceWakeupFromXml] skipping non gms package");
                                continue;
                            }
                        }
                        if (keyWord != null) {
                            String[] keyWordArray = keyWord.split(",");
                            idIntValue = Integer.valueOf(id);
                            info = new VoiceWakeupInfo(packageName, className, actionName,
                                       idIntValue, keyWordArray);
                        } else {
                            info = new VoiceWakeupInfo(packageName, className, actionName,
                                       idIntValue);
                        }
                        if (ConfigurationManager.isValidCommandId(mContext, idIntValue)) {
                            //Command Id is valid if there is a registered VOW_COMMAND
                            Log.d(TAG, "[readVoiceWakeupFromXml] valid command id =" + id);
                            wakeupList.add(info);
                        } else {
                            Log.d(TAG, "[readVoiceWakeupFromXml] invalid command id =" + id);
                            continue;
                        }
                    }
                }
            }
            if ((wakeupList.size() == 0) && (info != null)) {
                Log.d(TAG, "[readVoiceWakeupFromXml] No valid app exist. " +
                           "So add one element with last info data.");
                wakeupList.add(info);
            }
        } catch (XmlPullParserException e) {
            Log.e(TAG, "[readVoiceWakeupFromXml]XmlPullParserException:", e);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "[readVoiceWakeupFromXml]FileNotFoundException:", e);
        } catch (IOException e) {
            Log.e(TAG, "[readVoiceWakeupFromXml]IOException:", e);
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "[readVoiceWakeupFromXml]close,IOException:", e);
            }
        }
    }

    /**
     * Check is the package illegal in PMS.
     * 
     * @param packageName
     *            package name
     * 
     * @return true if package is legal
     */
    private boolean checkPackageIllegal(String packageName) {
        if (packageName == null) {
            Log.w(TAG, "[checkPackageIllegal]packageName is null.");
            return false;
        }
        try {
            ApplicationInfo info = mContext.getPackageManager().getApplicationInfo(packageName,
                    PackageManager.GET_UNINSTALLED_PACKAGES);
            return true;
        } catch (NameNotFoundException e) {
            Log.e(TAG, "[checkPackageIllegal]NameNotFoundException:", e);
            return false;
        }
    }

    /**
     * Check if GMS package is installed.
     *
     * @param packageName
     *            package name
     *
     * @return true if gms is installed
     */
     private boolean checkGmsInstalled() {
        String gms = SystemProperties.get("ro.com.google.gmsversion", null);
        if (gms == null || gms.length() == 0) {
            return false;
        }
        return true;
    }
}
