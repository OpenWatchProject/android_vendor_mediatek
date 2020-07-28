/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

//package com.mediatek.omadownload;
package com.android.providers.downloads;

import android.util.Log;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;

import com.google.common.annotations.VisibleForTesting;

// Change to public class
class OmaDescription {
//public class OmaDescription {

    private int mSize = -1;
    private int mStatusCode = -1;
    private String mDDVersion = null;
    private String mDescription = null;
    private String mInstallParam = null;
    private String mName = null;
    private String mVendor = null;
    private URL mIconUrl = null;
    private URL mInfoUrl = null;
    private URL mInstallNotifyUrl = null;
    private URL mObjectUrl = null;
    private URL mNextUrl = null;
    private ArrayList<String> mType = null;
    private static final String TAG = "OmaDescription";

    protected static final  String DDVERSION = "DDVersion";
    protected static final  String DESCRIPTION = "Description";
    protected static final  String ICON_URL = "iconURI";
    protected static final  String INFO_URL = "infoURL";
    protected static final  String INSTALL_NOTIFY_URL = "installNotifyURI";
    protected static final  String INSTALL_PARAM = "installParam";
    protected static final  String OBJECT_URL = "objectURI";
    protected static final  String NAME = "name";
    protected static final  String NEXT_URL = "nextURL";
    protected static final  String ROOT = "media";
    protected static final  String SIZE = "size";
    protected static final  String STATUS_CODE = "statusCode";
    protected static final  String TYPE = "type";
    protected static final  String VENDOR = "vendor";

    /**
    * Default constructor used to construct an OmaDescription object.
    */
    public OmaDescription() {
        mType = new ArrayList<String>();
    }

    /**
    * This method gets the version of the download descriptor technology.
    @return     the version, or null if it is not defined.
    */
    protected String getDDVersion() {
        return mDDVersion;
    }

    /**
    * This method gets a short textual description of the media object.
    @return     the short description, or null if it is not defined.
    */
    protected String getDescription() {
        return mDescription;
    }

    /**
    * This method gets the URL of an icon
    @return     the URL of an icon, or null if it is not defined.
    */
    protected URL getIconUrl() {
        return mIconUrl;
    }

    /**
    * This method gets the URL for further describing the media object
    @return     the URL of the further description, or null if it is not defined.
    */
    protected URL getInfoUrl() {
        return mInfoUrl;
    }

    /**
    * This method gets the URL to which an installation status report is to be sent.
    @return     the URL to where the status report is to be sent, or null if it is not defined.
    */
    protected URL getInstallNotifyUrl() {
        return mInstallNotifyUrl;
    }

    /**
    * This method gets the installation parameter associated with the downloaded media object.
    @return     the parameters used for the installer, or null if it is not defined.
    */
    protected String getInstallParam() {
        return mInstallParam;
    }

    /**
    * This method gets the URL from which the media object can be loaded.
    @return     the URL of the media object, or null if it is not defined.
    */
    protected URL getObjectUrl() {
        return mObjectUrl;
    }

    /**
    * This method gets the name of the media object.
    @return     the name of the media object, or null if it is not defined.
    */
    protected String getName() {
        return mName;
    }

    /**
    * This method gets the URL to which the client should
     navigate after the download transaction has completed.
    @return     the URL to be navigated, or null if it is not defined.
    */
    protected URL getNextUrl() {
        return mNextUrl;
    }

    /**
    * This method gets the number of bytes of the media object.
    @return     the number of bytes, or -1 if it is not defined.
    */
    protected int getSize() {
        return mSize;
    }

    /**
    * This method gets the status code of the installation.
    @return     the installation status code, or -1 if it is not defined.
    */
    protected int getStatusCode() {
        return mStatusCode;
    }

    /**
    * This method gets the MIME media type of the media object.
    @return     the MIME media type, or null if it is not defined.
    */
    protected ArrayList<String> getType() {
        return mType;
    }

    /**
    * This method gets the name of organization that provides the media object.
    @return     the name of the organization, or null if it is not defined.
    */
    protected String getVendor() {
        return mVendor;
    }

    /**
    * This method reads the string and restores the OmaDescription object.
    @param  str     the string to be restored
    @return         an OmaDescription object
    */
    @VisibleForTesting
    protected static OmaDescription readObject(String str) {

        int key = 0;        //index for attribute name (e.g. name)
        int value = 1;      //index for value of the attribute (e.g. audio.mp3)
        int pair = 2;       //the length of the resulting array must be two (key:value)
        URL url = null;

        OmaDescription component = new OmaDescription();

        if (str != null) {
            String[] tokens = str.split(",");

            for (int x = 0; x < tokens.length; x++) {
                if ((tokens[x].length() >= 0) && (tokens[x].matches("[a-zA-Z]+:.+"))) {
                    /*
                     * use second argument in the String.split() method to control
                     * the maximum number of substrings generated by splitting a string.
                     */
                    String[] result = tokens[x].split(":", 2);

                    if ((result != null) && (result.length == pair)) {
                        if (result[key].equals(DDVERSION)) {
                            component.setDDVersion(result[value]);
                        } else if (result[key].equals(DESCRIPTION)) {
                            component.setDescription(result[value]);
                        } else if (result[key].equals(ICON_URL)) {
                            try {
                                url = new URL(result[value]);
                                component.setIconUrl(url);
                            } catch (MalformedURLException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(INFO_URL)) {
                            try {
                                url = new URL(result[value]);
                                component.setInfoUrl(url);
                            } catch (MalformedURLException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(INSTALL_NOTIFY_URL)) {
                            try {
                                url = new URL(result[value]);
                                component.setInstallNotifyUrl(url);
                            } catch (MalformedURLException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(INSTALL_PARAM)) {
                            component.setInstallParam(result[value]);
                        } else if (result[key].equals(OBJECT_URL)) {
                            try {
                                url = new URL(result[value]);
                                component.setObjectUrl(url);
                            } catch (MalformedURLException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(NAME)) {
                            component.setName(result[value]);
                        } else if (result[key].equals(NEXT_URL)) {
                            try {
                                url = new URL(result[value]);
                                component.setNextUrl(url);
                            } catch (MalformedURLException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(SIZE)) {
                            try {
                                int val = Integer.parseInt(result[value]);
                                component.setSize(val);
                            } catch (NumberFormatException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(STATUS_CODE)) {
                            try {
                                int val = Integer.parseInt(result[value]);
                                component.setStatusCode(val);
                            } catch (NumberFormatException e) {
                                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            }
                        } else if (result[key].equals(TYPE)) {
                            component.setType(result[value]);
                        } else if (result[key].equals(VENDOR)) {
                            component.setVendor(result[value]);
                        }
                    }
                }
            }
        }
        return component;
    }

    /**
    * This method sets the version of the download descriptor technology.
    @param  version        the version
    */
    protected void setDDVersion(String version) {
        mDDVersion = version;
    }

    /**
    * This method sets a short textual description of the media object.
    @param  description     the short description
    */
    protected void setDescription(String description) {
        mDescription = description;
    }

    /**
    * This method sets the URL of an icon
    @param  url     the URL of an icon
    */
    protected void setIconUrl(URL url) {
        mIconUrl = url;
    }

    /**
    * This method sets the URL for further describing the media object
    @param  url     the URL of the further description
    */
    protected void setInfoUrl(URL url) {
        mInfoUrl = url;
    }

    /**
    * This method sets the URL to which an installation status report is to be sent.
    @param  url     the URL to where the status report is to be sent
    */
    protected void setInstallNotifyUrl(URL url) {
        mInstallNotifyUrl = url;
    }

    /**
    * This method sets the installation parameter associated with the downloaded media object.
    @param  param   the parameters used for the installer
    */
    protected void setInstallParam(String param) {
        mInstallParam = param;
    }

    /**
    * This method sets the URL from which the media object can be loaded.
    @param  url     the URL of the media object
    */
    protected void setObjectUrl(URL url) {
        mObjectUrl = url;
    }

    /**
    * This method sets the name of the media object
    @param  name    the name of the media object
    */
    protected void setName(String name) {
        mName = name;
    }

    /**
    * This method sets the URL to which the client should
     navigate after the download transaction has completed.
    @param  url     the URL to be navigated
    */
    protected void setNextUrl(URL url) {
        mNextUrl = url;
    }

    /**
    * This method sets the number of bytes of the media object.
    @param  size    the number of bytes
    */
    protected void setSize(int size) {
        mSize = size;
    }

    /**
    * This method sets the status code of the installation.
    @param  status  the installation status code
    */
    protected void setStatusCode(int status) {
        mStatusCode = status;
    }

    /**
    * This method sets the MIME media type of the media object.
    @param  type    the type to be added to the array list
    */
    protected void setType(String type) {
        mType.add(type);
    }

    /**
    * This method sets the name of organization that provides the media object.
    @param  name    the name of the organization
    */
    protected void setVendor(String vendor) {
        mVendor = vendor;
    }

    /**
    * This method returns a string representation for an OmaDescription object.
    @param  component   the object to be transformed into a string
    @return             a string representation
    */
    @VisibleForTesting
    protected static String writeObject(OmaDescription component) {
        String str = null;

        if (component.mDDVersion != null) {
            str = DDVERSION + ":" + component.mDDVersion + ",";
        }

        if (component.mDescription != null) {
            str = str + DESCRIPTION + ":" + component.mDescription + ",";
        }

        if (component.mIconUrl != null) {
            str = str + ICON_URL + ":" + component.mIconUrl.toString() + ",";
        }

        if (component.mInfoUrl != null) {
            str = str + INFO_URL + ":" + component.mInfoUrl.toString() + ",";
        }

        if (component.mInstallNotifyUrl != null) {
            str = str + INSTALL_NOTIFY_URL + ":" + component.mInstallNotifyUrl.toString() + ",";
        }

        if (component.mInstallParam != null) {
            str = str + INSTALL_PARAM + ":" + component.mInstallParam + ",";
        }

        if (component.mObjectUrl != null) {
            str = str + OBJECT_URL + ":" + component.mObjectUrl.toString() + ",";
        }

        if (component.mName != null) {
            str = str + NAME + ":" + component.mName + ",";
        }

        if (component.mNextUrl != null) {
            str = str + NEXT_URL + ":" + component.mNextUrl.toString() + ",";
        }

        if (component.mSize != -1) {

            str = str + SIZE + ":" + Integer.toString(component.mSize) + ",";
        }

        if (component.mStatusCode != -1) {

            str = str + STATUS_CODE + ":" + Integer.toString(component.mStatusCode) + ",";
        }

        if (component.mType != null) {
            for (String s: component.mType) {
                str = str + TYPE + ":" + s + ",";
            }
        }

        if (component.mVendor != null) {
            str = str + VENDOR + ":" + component.mVendor + ",";
        }

        return str;
    }
}

