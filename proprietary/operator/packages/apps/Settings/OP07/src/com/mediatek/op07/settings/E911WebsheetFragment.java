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


package com.mediatek.op07.settings;

import android.app.Activity;
import android.app.Fragment;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.webkit.JavascriptInterface;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ProgressBar;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;

/**
 * Activity for emergency webview.
 */
public class E911WebsheetFragment extends Fragment {

    private static final String TAG = "[OP07SettingsECCUI]E911WebsheetFragment";

    private View mRootView;
    private WebView mEmergencyWebView;
    private ProgressBar mProgressBar;
    private ProgressDialog mLoadingDialog;
    E911WebSheetListener mCallback;
    private boolean mErrorPage = false;
    private static final String ERROR_URL =
        "https://attdashboard.wireless.att.com/softphone/primary/attprepaid/#/error";

    //For Testing purpose only
    static String TestHtml = "<html><head><script>function count_rabbits(){WiFiCallingWebViewControllerCallback.phoneServicesAccountStatusChanged(true)}</script></head><body><h2>Press the button to start</h2><input type=\"button\" onclick=\"count_rabbits()\" value=\"Count rabbits!\"/></body></html>";

    private int mSlotId;
    // Container Activity must implement this interface
    public interface E911WebSheetListener {
        public boolean isServiceConnected(int slotId);
        public String getUrl(int slotId);
        public String getServerData(int slotId);
        public void startEntitlement(int slotId, boolean retry);
        public void webSheetStateChange(int slotId, boolean isComplete, boolean error);
        public void onBackPressed(int slotId);
    }

    public E911WebsheetFragment(int slotId) {
        //empty constructor
        mSlotId = slotId;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = inflater.inflate(R.layout.emergency_activity, container, false);
        getActivity().setTitle(R.string.emergency_address);
        setHasOptionsMenu(true);
        return mRootView;
    }

    @Override
    public void onCreateOptionsMenu(
            Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu, menu);
        super.onCreateOptionsMenu(menu, inflater);

    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.d(TAG, "onOptionsItemSelected");
        if (item.getItemId() == R.id.cancel_button) {
            if(mCallback != null) {
                mCallback.onBackPressed(mSlotId);
            } else {
                Log.e(TAG, "Null pointer exception on mCallback");
            }
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        mProgressBar = (ProgressBar) mRootView.findViewById(R.id.progressBar);
        mEmergencyWebView = (WebView) mRootView.findViewById(R.id.webview);
        mEmergencyWebView.addJavascriptInterface(new WiFiCallingWebViewControllerCallback(),
                "WiFiCallingWebViewController");
        mEmergencyWebView.setWebViewClient(new EntitlementWebViewClient());
        boolean isConnected = mCallback.isServiceConnected(mSlotId);
        Log.d(TAG, "onActivityCreated, isConnected = " + isConnected);
        if (isConnected) {
            showWebView(mSlotId);
        } else {
            showLoadingScreen();
        }
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);

        // This makes sure that the container activity has implemented
        // the callback interface. If not, it throws an exception
        try {
            mCallback = (E911WebSheetListener) activity;
            Log.d(TAG, "mCallback = " + mCallback);
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString()
                    + " must implement E911WebSheetListener");
        }
    }

    private void showWebView(final int slotId) {
        //URL should be of http:// format
        String url = mCallback.getUrl(slotId);
        String postData = mCallback.getServerData(slotId);
        Log.d(TAG, "showWebView() setwebseting, url = " + url
               + "slotId =" + slotId
               +"postData = " + postData);
        dismissLoading();
        WebSettings webSettings = mEmergencyWebView.getSettings();
        /**
         * for The attackers can use
         * "mWebView.loadUrl("file:///data/data/[Your_Package_Name]/[File]");"
         *  to access app's local file.
         *  so set webSettings.setAllowFileAccess(false);
         *  because for this case webSettings just though showWebView() connect net
         */
        webSettings.setAllowFileAccess(false);
        webSettings.setAllowFileAccessFromFileURLs(false);
        webSettings.setAllowUniversalAccessFromFileURLs(false);
        /**
         * for The attackers can use
         * "mWebView.loadUrl("file:///data/data/[Your_Package_Name]/[File]");"
         *  to access app's local file.
         *  so set webSettings.setJavaScriptEnabled(false);
         *  because for this case webSettings just though showWebView() connect net
         */
        if (url.startsWith("file://")) {
            webSettings.setJavaScriptEnabled(false);
        } else {
            webSettings.setJavaScriptEnabled(true);
        }
        webSettings.setDomStorageEnabled(true);
        webSettings.setBuiltInZoomControls(true);
        if (postData != null) {
            mEmergencyWebView.postUrl(url, postData.getBytes());
        } else {
            Toast.makeText(getActivity(), R.string.try_again, Toast.LENGTH_SHORT).show();
            mEmergencyWebView.post(new Runnable() {
                @Override
                public void run() {
                    finishWebView(slotId, false);
                }
            });
        }
    }

    /**
     * Class for customized webClient.
     */
    private class EntitlementWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            Log.d(TAG, "shouldOverrideUrlLoading, url:" + url);
            mProgressBar.setVisibility(View.VISIBLE);
            return false;
        }

        @Override
        public void onPageFinished(WebView view, String url) {
            // TODO Auto-generated method stub
            super.onPageFinished(view, url);
            Log.d(TAG, "onPageFinished, url:" + url);
            if (url.equals(ERROR_URL)) {
                Log.d(TAG,"AT&T error page, set error flag");
                mErrorPage = true;
            }
            mProgressBar.setVisibility(View.GONE);
            dismissLoading();
        }

        @Override
        public void onReceivedHttpError(android.webkit.WebView view,
                android.webkit.WebResourceRequest
                request, android.webkit.WebResourceResponse errorResponse) {
            int status = errorResponse.getStatusCode();
            Log.e(TAG, "HTTP error " + status);

            try {
                switch (status) {
                    case 400: // Bad Request
                        break;
                    case 401:
                        JSONObject body = new JSONObject(fromStream(errorResponse.getData()));
                        String error = body.getString("error");
                        Log.e(TAG, "error: " + error);
                        switch (error) {
                            case "invalid_authtoken":
                                // Nothing we can do...
                                break;
                            case "expired_authtoken":
                                // obtain a new server-data string from SES and try again
                                showWebView(mSlotId);
                                break;
                        }
                        break;
                    case 500:   // Internal Server Error
                        // TODO: should retry
                        break;
                }
            } catch (IOException | JSONException e) {
                Log.e(TAG, "failed to read body", e);
            }
        }
    }

    public static String fromStream(InputStream inputStream) throws IOException {
        int bufSize = 1028;
        byte[] buffer = new byte[bufSize];
        int inSize;
        StringBuilder stringBuilder = new StringBuilder();
        if (inputStream != null) {
            while ((inSize = inputStream.read(buffer)) > 0) {
                stringBuilder.append(new String(buffer, 0, inSize));
            }
        }
        return stringBuilder.toString();
    }
   private class WiFiCallingWebViewControllerCallback {
        @JavascriptInterface
        public void phoneServicesAccountStatusChanged(final boolean isFinished) {
            Log.d(TAG, "phoneServicesAccountStatusChanged: " + isFinished);

            mEmergencyWebView.post(new Runnable() {
                @Override
                public void run() {
                    finishWebView(mSlotId,isFinished);
                }
            });

            if (isFinished) {
                mCallback.startEntitlement(mSlotId, false);
            } else {
                mCallback.startEntitlement(mSlotId, true);
            }
        }
    }

    private void finishWebView(int slotId, boolean isFinished) {
        mEmergencyWebView.destroy();
        mEmergencyWebView.setVisibility(View.GONE);
        mCallback.webSheetStateChange(slotId, isFinished, mErrorPage);
        mErrorPage = false;
    }
    private void showLoadingScreen() {
        Context context = getActivity();
        mLoadingDialog = new ProgressDialog(context);
        mLoadingDialog.setMessage(context.getString(R.string.loading_message));
        mLoadingDialog.setCancelable(true);
        mLoadingDialog.setCanceledOnTouchOutside(false);
        mLoadingDialog.show();
    }

    private void dismissLoading() {
        if (mLoadingDialog != null && mLoadingDialog.isShowing()) {
            mLoadingDialog.dismiss();
        }
    }
}
