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
import android.webkit.CookieManager;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.mediatek.digits.DigitsManager;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;



/**
 * Activity for emergency webview.
 */
public class SignUpWebsheetFragment extends Fragment {

    private static final String TAG = "OP08SignUpWebsheetFragment";

    private View mRootView;
    private WebView mDigitsWebView;
    private ProgressBar mProgressBar;
    private ProgressDialog mLoadingDialog;

    private MultiLineActivity mMultiLineActivity = null;

    /**
     * Websheet fragment constructor.
     * @param parent Parent Activity
     */
    public SignUpWebsheetFragment(MultiLineActivity parent) {
        mMultiLineActivity = parent;
    }

    /**
     * Default Websheet fragment constructor.
     */
    public SignUpWebsheetFragment() {
        Log.d(TAG, "Empty constructor");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = inflater.inflate(R.layout.activity_login, container, false);
        return mRootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        mProgressBar = (ProgressBar) mRootView.findViewById(R.id.progressBar);
        mDigitsWebView = (WebView) mRootView.findViewById(R.id.webview);
        mDigitsWebView.clearCache(true);
        CookieManager.getInstance().removeAllCookies(null);
        clearCacheForContext(mMultiLineActivity.getMultilineContext());
        mDigitsWebView.setWebViewClient(new MultiLineWebViewClient());

        WebSettings webSettings = mDigitsWebView.getSettings();
        webSettings.setCacheMode(WebSettings.LOAD_NO_CACHE);
        webSettings.setJavaScriptEnabled(true);
        webSettings.setDomStorageEnabled(true);
        webSettings.setBuiltInZoomControls(true);
        showWebView();
    }

    /**
     * Delete the filesfrom the application cache.
     * @param dir Directory for current activity's context
     */
    private int clearCacheFolder(final File dir) {

        int deletedFiles = 0;
        if (dir != null && dir.isDirectory()) {
            try {
                for (File child:dir.listFiles()) {

                    //first delete subdirectories recursively
                    if (child.isDirectory()) {
                        deletedFiles += clearCacheFolder(child);
                    }

                    //then delete the files and subdirectories in this dir
                    //only empty directories can be deleted, so subdirs have been done first
                    if (child.delete()) {
                        deletedFiles++;
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, String.format("Failed to clean the cache, error %s", e.getMessage()));
            }
        }
        return deletedFiles;
    }

    /**
     * Delete the files from the application cache.
     * @param context activity's context
     */
    public void clearCacheForContext(final Context context) {
        Log.i(TAG, String.format("Starting cache prune, deleting files"));
        int numDeletedFiles = clearCacheFolder(context.getCacheDir());
        Log.i(TAG, String.format("Cache pruning completed, %d files deleted", numDeletedFiles));
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

    /**
     * Open the login url to view.
     */
    private void showWebView() {
        //URL should be of http:// format
        String url = mMultiLineActivity.getSignUpUrl();
        mDigitsWebView.loadUrl(url);
        //mDigitsWebView.loadData(TestHtml, "text/html", null);
    }

    /**
     * Close the current Web sheet fragment.
     */
    public void closeFragmentHost() {
        //finishWebView(true);
        mMultiLineActivity.closeFragment(this);
    }

    /**
     * Open the Settings App and close the current Web sheet fragment.
     */
    public void closeFragmentHostAndOpenSettings() {
        //finishWebView(true);
        mMultiLineActivity.closeFragmentHostAndOpenSettings(this);
    }

    /**
     * Class for customized webClient.
     */
    private class MultiLineWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
            String url = request.getUrl().toString();
            Log.d(TAG, "shouldOverrideUrlLoading, url:" + url);
            if (mMultiLineActivity.getProfileEmail() != null) {
                closeFragmentHostAndOpenSettings();
                return true;
            } else {
                String authCode = fetchAuthCode(url);
                if (authCode == null) {
                    mProgressBar.setVisibility(View.VISIBLE);
                    view.loadUrl(url);
                    return false;
                } else {
                    DigitsManager manager = mMultiLineActivity.getDigitsManagerInstance();
                    manager.subscribe(authCode, manager.new ActionListener() {

                        @Override
                        public void onSuccess(Bundle extras) {
                            Log.i(TAG, "onSuccess()");
                            closeFragmentHostAndOpenSettings();
                        }

                        @Override
                        public void onFailure(final int reason, Bundle extras) {
                            mMultiLineActivity.runOnUiThread(new Runnable() {
                                public void run() {
                                    Toast.makeText(mMultiLineActivity, "Login failed",
                                    Toast.LENGTH_LONG).show();
                                }
                            });
                            Log.i(TAG, "onFailure(), reason:" + reason);
                            closeFragmentHost();
                        }
                    });
                    return true;
                }
            }
        }


        /**
         * Convert url stream of data to auth code.
         * @param url Url data
         * @return fetched authcode from url
         */
        private String fetchAuthCode(String url) {
            String token = "token?code=";
            int index = url.lastIndexOf(token);
            int sessionNumIndex = -1;
            String session = "&session_num";
            sessionNumIndex = url.indexOf(session, index);
            if (index != -1) {
                String authCode;
                if (sessionNumIndex != -1 && sessionNumIndex > index) {
                    authCode =  url.substring(index + token.length(), sessionNumIndex);
                } else {
                    authCode = url.substring(index + token.length());
                }
                Log.d(TAG, "Fetched authCode: " + authCode);
                return authCode;
            } else {
                return null;
            }
        }

        @Override
        public void onPageFinished(WebView view, String url) {
            // TODO Auto-generated method stub
            super.onPageFinished(view, url);
            Log.d(TAG, "onPageFinished, url:" + url);
            mProgressBar.setVisibility(View.GONE);
        }

        @Override
        public void onReceivedHttpError(android.webkit.WebView view,
                android.webkit.WebResourceRequest
                request, android.webkit.WebResourceResponse errorResponse) {
            int status = errorResponse.getStatusCode();
            Log.e(TAG, "HTTP error " + status);

            /*try {
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
            }*/
            Toast.makeText(mMultiLineActivity, "Login failed", Toast.LENGTH_LONG).show();
            closeFragmentHost();
        }

        public void onProgressChanged(WebView view, int progress) {
            if (progress < 100 && mProgressBar.getVisibility() == ProgressBar.GONE) {
                mProgressBar.setVisibility(ProgressBar.VISIBLE);
            }

            mProgressBar.setProgress(progress);
            if (progress == 100) {
                mProgressBar.setVisibility(ProgressBar.GONE);
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

    /**
     * Convert input stream of data to String.
     * @param isFinished Loading web sheet is finished
     */
    private void finishWebView(boolean isFinished) {
        mDigitsWebView.destroy();
        mDigitsWebView.setVisibility(View.GONE);
    }
}
