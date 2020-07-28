package com.mediatek.mail.ui.utils;

import android.content.Context;
import android.util.AttributeSet;
import android.webkit.WebView;

/**
 * M: Email webview base class, work around for destory itself when detached
 * from window. To workaround for Memory leak.
 *
 * Root cause: AwContent in
 * WebView some case not release ComponentCallbacks2 (Email component), which
 * will block compose activity, and Attachment bitmap never released, and memory
 * leak happened.
 *
 * ComponentCallbacks2 register in onAttachedToWindow, and unregister in
 * onDetachedFromWindow. Note that: if the destroy method has called, it never
 * unregister.
 *
 * The resource dependency as following:
 * AwContent->ComponentCallbacks2->ComposeActivity->AttachmentTile->Bitmap
 *
 * Solution: Since L MR1 design ComponentCallbacks2 not released in
 * onDetachedFromWindow method after webview under destroyed status. Release
 * webview resource when detached from windows.
 */
public class EmailWebView extends WebView {

    public EmailWebView(Context c) {
        this(c, null);
    }

    public EmailWebView(Context c, AttributeSet attrs) {
        super(c, attrs);
    }

    @Override
    protected void onDetachedFromWindowInternal() {
        super.onDetachedFromWindowInternal();
        destroy();
    }
}
