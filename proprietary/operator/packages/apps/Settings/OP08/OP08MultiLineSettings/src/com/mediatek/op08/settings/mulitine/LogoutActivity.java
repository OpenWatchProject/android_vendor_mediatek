package com.mediatek.op08.settings.mulitine;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ProgressBar;

/**
 * Activity to Login into TMO Digits.
 */
public class LogoutActivity extends Activity {

    WebView mWebView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        Intent intent = getIntent();
        String message = intent.getStringExtra(MultiLineSettingsActivity.EXTRA_MESSAGE);
        mWebView = (WebView) findViewById(R.id.webview);
        mWebView.loadUrl(message);
        WebSettings webSettings = mWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        final ProgressBar pbar = (ProgressBar) findViewById(R.id.progressBar);
        mWebView.setWebViewClient(new WebViewClient() {
            public void onProgressChanged(WebView view, int progress) {
                if (progress < 100 && pbar.getVisibility() == ProgressBar.GONE) {
                    pbar.setVisibility(ProgressBar.VISIBLE);
                }

                pbar.setProgress(progress);
                if (progress == 100) {
                    pbar.setVisibility(ProgressBar.GONE);
                }
            }
        });
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // Check if the key event was the Back button and if there's history
        if ((keyCode == KeyEvent.KEYCODE_BACK) && mWebView.canGoBack()) {
            mWebView.goBack();
            return true;
        }
        // If it wasn't the Back key or there's no web page history, bubble up to the default
        // system behavior (probably exit the activity)
        return super.onKeyDown(keyCode, event);
    }
}
