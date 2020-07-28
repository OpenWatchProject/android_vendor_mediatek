package com.mediatek.browser.ext;

import android.content.Context;
import android.text.InputFilter;

public interface IBrowserUrlExt {

    /**
     * Get the input filter that used to check the url length
     * @param context the content
     * @return the input filter
     */
    InputFilter[] checkUrlLengthLimit(final Context context);

    /**
     * Check the url and get the trim url
     * @param url the original url
     * @return the trim url
     */
    String checkAndTrimUrl(String url);

    /**
     * Get the navigation bar title content
     * @param title the webpage title
     * @param url the webpage url
     * @return the navigation bar content
     */
    String getNavigationBarTitle(String title, String url);

    /**
     * Get the override focus content
     * @param hasFocus whether having focus or not
     * @param newContent the new content
     * @param oldContent the old content
     * @param url the webpage url
     * @return the navigation bar content
     */
    String getOverrideFocusContent(boolean hasFocus,
                                   String newContent,
                                   String oldContent,
                                   String url);

    /**
     * Get override focus title content
     * @param title the webpage title
     * @param content the current content
     * @return the navigation bar content
     */
    String getOverrideFocusTitle(String title, String content);

    /**
     * Redirect the url
     * @param url the webpage url
     * @return true if the url is handled and no need to be handled again in Browser
     */
    boolean redirectCustomerUrl(String url);

}
