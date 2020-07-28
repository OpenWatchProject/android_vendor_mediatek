/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.rsc;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;
import com.mediatek.engineermode.rsc.ConfigXMLData.ProjectData;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;

/**
 * Activity to show UI for users to switch telephony log.
 *
 */
public class RuntimeSwitchConfig extends Activity  {
    private static final String TAG = "rcs/RuntimeSwitchConfig";
    static final int DIALOG_WARNING_REBOOT = 1;
    static final int DIALOG_WRITING_SELECTION = 2;
    static final int DIALOG_LOADING_PROJECT = 3;
    static final String PROJECT_LIST_FILE_PATH = "/system/etc/rsc/rsc.xml";
    private static final String PROJ_PROP = "ro.boot.rsc";
    private Spinner mSpProject = null;
    private Button mBtnApply = null;
    private ConfigXMLData mConfigData = new ConfigXMLData();
    private String mProjCurName = SystemProperties.get(PROJ_PROP);
    private ArrayAdapter<String> mSpProjAdapter;

    /**
     * Function to get Rsc support situation.
     * @return true for supported
     */
    public static boolean supportRsc() {
        File xmlFile = new File(PROJECT_LIST_FILE_PATH);
        if ((xmlFile != null) && (xmlFile.exists())) {
            return true;
        }
        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.runtime_switch_config);
        mSpProjAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item);
        mSpProject = (Spinner) findViewById(R.id.rsc_project_spinner);
        mBtnApply = (Button) findViewById(R.id.rsc_btn_apply);
        mBtnApply.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                int index = mSpProject.getSelectedItemPosition();
                if ((mProjCurName != null) &&
                    (mProjCurName.equals(mSpProjAdapter.getItem(index)))) {
                    Toast.makeText(RuntimeSwitchConfig.this, R.string.rsc_proj_unchanged,
                            Toast.LENGTH_LONG).show();
                    return;
                }
                showDialog(DIALOG_WARNING_REBOOT);
            }
        });
        new ProjectLoadTask().execute();
    }

    /**
     * Class for loading project supports.
     *
     */
    private class ProjectLoadTask extends AsyncTask<Void, Void, Void> {


        private int mProjCurIndex = -1;
        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            showDialog(DIALOG_LOADING_PROJECT);
        }

        @Override
        protected Void doInBackground(Void... params) {
            loadProjects();
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            updateProjList();
            removeDialog(DIALOG_LOADING_PROJECT);

        }

        private void parseXMLWithSAX(InputStream inStream,
                ConfigXMLData data) throws SAXException,
                ParserConfigurationException, IOException {

            SAXParserFactory factory = SAXParserFactory.newInstance();

            XMLReader xmlReader = factory.newSAXParser().getXMLReader();

            ContentHandler handler = new ContentHandler(data);

            xmlReader.setContentHandler(handler);

            xmlReader.parse(new InputSource(inStream));
        }

        private void updateProjList() {

            mSpProject.setAdapter(mSpProjAdapter);
            if (mProjCurIndex != -1) {
                mSpProject.setSelection(mProjCurIndex);
            }

        }

        private void loadProjects() {
            try {
                File xmlFile = new File(PROJECT_LIST_FILE_PATH);
                InputStream inputStream = new FileInputStream(xmlFile);
                parseXMLWithSAX(inputStream, mConfigData);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (SAXException e) {
                e.printStackTrace();
            } catch (ParserConfigurationException e) {
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }


            mSpProjAdapter
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

            int size = mConfigData.getProjectList().size();
            for (int k = 0; k < size; k++) {
                ProjectData projData = mConfigData.getProjectList().get(k);
                mSpProjAdapter.add(projData.getName());
                if ((mProjCurIndex == -1) && (mProjCurName != null)) {
                    if (mProjCurName.equals(projData.getName())) {
                        mProjCurIndex = k;
                    }
                }
            }

        }
    }

    /**
     * Class for switching project.
     *
     */
    private class ProjectSwitchTask extends AsyncTask<Void, Void, Void> {

        private ProjectData mProj;
        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            showDialog(DIALOG_WRITING_SELECTION);
            int index = mSpProject.getSelectedItemPosition();
            mProj = mConfigData.getProjectList().get(index);
        }

        @Override
        protected Void doInBackground(Void... params) {
            storeProjSelected();
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            removeDialog(DIALOG_WRITING_SELECTION);
            doFactoryReset();
        }

        private void storeProjSelected() {
            Elog.i(TAG, "storeProjSelected");
            AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
            if (functionCall.startCallFunctionStringReturn(
                    AFMFunctionCallEx.FUNCTION_EM_RSC_WRITE)) {
                Elog.i(TAG, "start write");
                functionCall.writeParamInt(mConfigData.getVersion());
                functionCall.writeParamInt(mProj.getIndex());
                functionCall.writeParamString(mProj.getName());
                functionCall.writeParamString(mProj.getOptr());
                functionCall.writeParamString(mConfigData.getTarPartOffset());
                FunctionReturn funcRet = null;
                Elog.i(TAG, "end write");
                do {
                    funcRet = functionCall.getNextResult();
                    Elog.i(TAG, "funcRet:" + funcRet);
                } while (funcRet.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
            }
        }

        private void doFactoryReset() {
            Intent intent = new Intent(Intent.ACTION_FACTORY_RESET);
            intent.setPackage("android");
            intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
            intent.putExtra(Intent.EXTRA_REASON, "rsc");
            sendBroadcast(intent);
        }
    }









    @Override
    protected Dialog onCreateDialog(int id) {
        // TODO Auto-generated method stub
        switch (id) {
        case DIALOG_WARNING_REBOOT:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.rsc_dlg_reboot_title);
            builder.setMessage(getString(R.string.rsc_dlg_reboot_content));
            builder.setPositiveButton(R.string.dialog_ok, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    new ProjectSwitchTask().execute();
                }
            });
            builder.setNegativeButton(R.string.rsc_cancel, null);
            return builder.create();
        case DIALOG_WRITING_SELECTION:
            ProgressDialog dlgWriteSel = new ProgressDialog(this);
            dlgWriteSel.setMessage(getString(R.string.rsc_writing_selection));
            dlgWriteSel.setCancelable(false);
            dlgWriteSel.setIndeterminate(true);
            return dlgWriteSel;
        case DIALOG_LOADING_PROJECT:
            ProgressDialog dlgLoadProj = new ProgressDialog(this);
            dlgLoadProj.setMessage(getString(R.string.rsc_loading_project));
            dlgLoadProj.setCancelable(false);
            dlgLoadProj.setIndeterminate(true);
            return dlgLoadProj;
        default:
            return null;
        }
    }


}
