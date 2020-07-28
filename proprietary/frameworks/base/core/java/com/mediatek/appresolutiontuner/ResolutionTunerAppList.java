/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.mediatek.appresolutiontuner;

import android.os.IBinder;
import android.util.Slog;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Cache of white list and provide the operation of the cache
**/
public class ResolutionTunerAppList {

    private static final String TAG = "AppResolutionTuner";
    private static final String APP_LIST_PATH = "system/etc/resolution_tuner_app_list.xml";
    /** formart of app list XML file **/
    private static final String TAG_APP = "app";
    private static final String NODE_PACKAGE_NAME = "packagename";
    private static final String NODE_SCALE = "scale";
    private static final String NODE_FILTERED_WINDOW = "filteredwindow";
    private static final String NODE_SCALING_FLOW = "flow";
    private static final String VALUE_SCALING_FLOW_SURFACEVIEW = "surfaceview";
    private static final String VALUE_SCALING_FLOW_WMS = "wms";

    private static ResolutionTunerAppList sInstance;
    private ArrayList<Applic> mTunerAppCache;

    public ResolutionTunerAppList() {

    }

    public static ResolutionTunerAppList getInstance() {
        if (sInstance == null) {
            sInstance = new ResolutionTunerAppList();
        }
        return sInstance;
    }

    /**
    * load the xml in cache list
    */
    public void loadTunerAppList() {
        Slog.d(TAG, "loadTunerAppList + ");
        File target = null;
        InputStream inputStream = null;
        try {
            target = new File(APP_LIST_PATH);
            if (!target.exists()) {
               Slog.e(TAG, "Target file doesn't exist: " + APP_LIST_PATH);
               return;
            }
            inputStream = new FileInputStream(target);
            mTunerAppCache = parseAppListFile(inputStream);
        } catch (IOException e) {
            Slog.w(TAG, "IOException" , e);
        } finally {
            try {
                if (inputStream != null) inputStream.close();
            } catch (IOException e) {
                Slog.w(TAG, "close failed..", e);
            }
        }
        Slog.d(TAG, "loadTunerAppList - ");
    }

    /**
    * The window is define in white list and doesn't filter and reduce size by WMS
    */
    public boolean isScaledByWMS(String packageName, String windowName) {
        if (mTunerAppCache != null) {
            for (Applic app : mTunerAppCache) {
                if (app.getPackageName().equals(packageName)
                       && app.getScalingFlow().equals(VALUE_SCALING_FLOW_WMS)) {
                    return !app.isFiltered(windowName);
                }
            }
        }
        return false;
    }

    /**
    * The packageName is define in white list and reduce size by surfaceview.
    */
    public boolean isScaledBySurfaceView(String packageName) {
        if (mTunerAppCache != null) {
            for (Applic app : mTunerAppCache) {
                if (app.getPackageName().equals(packageName)
                       && !app.getScalingFlow().equals(VALUE_SCALING_FLOW_WMS)) {
                    return true;
                }
            }
        }
        return false;
    }

    public float getScaleValue(String packageName) {
        if (mTunerAppCache != null) {
            for (Applic app : mTunerAppCache) {
                if (app.getPackageName().equals(packageName)) {
                    return app.getScale();
                }
            }
        }
        return 1.f;
    }

    class Applic {
        private String packageName;
        private float scale;
        private ArrayList<String> filteredWindows = new ArrayList();
        private String scalingFlow = "";

        public String getPackageName() {
            return packageName;
        }

        public void setPackageName(String packageName) {
            this.packageName = packageName;
        }

        public float getScale() {
            return scale;
        }

        public void setScale(float scale) {
            this.scale = scale;
        }

        public void addFilteredWindow(String windowName) {
            filteredWindows.add(windowName);
        }

        public boolean isFiltered(String windowName) {
            return filteredWindows.contains(windowName);
        }

        public String getScalingFlow() {
            return scalingFlow;
        }

        public void setScalingFlow(String scalingFlow) {
            this.scalingFlow = scalingFlow;
        }

        @Override
        public String toString() {
            return "App{" +
                    "packageName='" + packageName + '\'' +
                    ", scale='" + scale + '\'' +
                    ", filteredWindows= " + filteredWindows + '\'' +
                    ", scalingFlow= " + scalingFlow + '\'' +
                    '}';
        }
    }

    private ArrayList<Applic> parseAppListFile(InputStream is) {
        ArrayList<Applic> list = new ArrayList();
        Document document = null;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(is);
        } catch (ParserConfigurationException e) {
            Slog.w(TAG, "dom2xml ParserConfigurationException" , e);
            return list;
        } catch (SAXException e) {
            Slog.w(TAG, "dom2xml SAXException" , e);
            return list;
        } catch (IOException e) {
            Slog.w(TAG, "IOException" , e);
            return list;
        }
        // get app list
        NodeList appList = document.getElementsByTagName(TAG_APP);
        //travesal app tag
        for (int i = 0; i < appList.getLength(); i++) {
            Node node_applic = appList.item(i);
            NodeList childNodes = node_applic.getChildNodes();
            Applic applic = new Applic();
            for (int j = 0; j < childNodes.getLength(); j++) {
                Node childNode = childNodes.item(j);
                if (childNode.getNodeName().equals(NODE_PACKAGE_NAME)) {
                    String packageName = childNode.getTextContent();
                    applic.setPackageName(packageName);
                } else if (childNode.getNodeName().equals(NODE_SCALE)) {
                    String scale = childNode.getTextContent();
                    applic.setScale(Float.parseFloat(scale));
                } else if (childNode.getNodeName().startsWith(NODE_FILTERED_WINDOW)) {
                    String filteredWindow = childNode.getTextContent();
                    applic.addFilteredWindow(filteredWindow);
                } else if (childNode.getNodeName().startsWith(NODE_SCALING_FLOW)) {
                    String scalingFlow = childNode.getTextContent();
                    applic.setScalingFlow(scalingFlow);
                }
            }
            list.add(applic);
            Slog.d(TAG,"dom2xml: " + applic);
        }
        return list;
    }
}
