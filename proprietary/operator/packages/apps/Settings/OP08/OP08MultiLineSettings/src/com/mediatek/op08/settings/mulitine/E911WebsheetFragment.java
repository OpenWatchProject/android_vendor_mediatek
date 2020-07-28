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


package com.mediatek.op08.settings.mulitine;

import android.app.Activity;
import android.app.Fragment;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
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

    private static final String TAG = "OP08E911WebsheetFragment";

    private View mRootView;
    private WebView mEmergencyWebView;
    private ProgressBar mProgressBar;
    private ProgressDialog mLoadingDialog;

    private MultiLineActivity mMultiLineActivity = null;

    /**
     * Websheet fragment constructor.
     * @param parent Parent Activity
     */
    public E911WebsheetFragment(MultiLineActivity parent) {
        mMultiLineActivity = parent;
    }

    /**
     * Default Websheet fragment constructor.
     */
    public E911WebsheetFragment() {
        Log.d(TAG, "Empty constructor");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = inflater.inflate(R.layout.emergency_activity, container, false);
        return mRootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        mProgressBar = (ProgressBar) mRootView.findViewById(R.id.progressBar);
        mEmergencyWebView = (WebView) mRootView.findViewById(R.id.webview);
        mEmergencyWebView.setWebViewClient(new EntitlementWebViewClient());

        WebSettings webSettings = mEmergencyWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setDomStorageEnabled(true);
        webSettings.setBuiltInZoomControls(true);
        showWebView();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);

        if (activity instanceof MultiLineActivity) {
            if (mMultiLineActivity == null) {
                mMultiLineActivity = (MultiLineActivity) activity;
            }
        }

    }

    /* NSDS 2.0 WebSheet Interface Description: Section 3.3:
        * In any case, navigation through the Web Sheet is intended to be entirely
        * programmatically controlled. Specifically, the NSDS 2.0 device should not make
        * available the back/forward navigation controls ordinarily found on Web browsers.
      */
    /*@Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }*/



    private void showWebView() {
        //URL should be of http:// format
        String url = mMultiLineActivity.getEntitlementUrl();
        String postData = mMultiLineActivity.getEntitlementPostData();
        dismissLoading();
        if (postData != null) {
            mEmergencyWebView.postUrl(url, postData.getBytes());
        } else {
            Toast.makeText(getActivity(), "Try again", Toast.LENGTH_SHORT).show();
            mEmergencyWebView.post(new Runnable() {
                @Override
                public void run() {
                    finishWebView(false);
                }
            });
        }
        //mEmergencyWebView.loadData(TestHtml, "text/html", null);
    }


    /**
     * Class for customized webClient.
     */
    private class EntitlementWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            Log.d(TAG, "shouldOverrideUrlLoading, url:" + url);
           // view.loadUrl(url);
            mProgressBar.setVisibility(View.VISIBLE);
            return false;
        }

        @Override
        public void onPageFinished(WebView view, String url) {
            // TODO Auto-generated method stub
            super.onPageFinished(view, url);
            Log.d(TAG, "onPageFinished, url:" + url);
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
                                showWebView();
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

    /**
     * Convert input stream of data to String.
     * @param inputStream Data stream received
     * @return Coverted stream to String form
     * @throws IOException incase of appending the String
     */
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

    /* NSDS 2.0 WebSheet Interface Description: Section 3.3:
        * In any case, navigation through the Web Sheet is intended to be entirely
        * programmatically controlled. Specifically, the NSDS 2.0 device should not make
        * available the back/forward navigation controls ordinarily found on Web browsers.
      */
    /*@Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && myWebView.canGoBack()) {
            myWebView.goBack();
            return true;
        }
        return super.onKeyDown(keyCode, event);
   }*/
    private void finishWebView(boolean isFinished) {
        mEmergencyWebView.destroy();
        mEmergencyWebView.setVisibility(View.GONE);
        mMultiLineActivity.webSheetStateChange(isFinished);
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
