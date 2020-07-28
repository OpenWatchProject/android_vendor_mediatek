package com.mediatek.browser.ext;

public interface IBrowserSiteNavigationExt {

    /**
     * Get the predefined websites
     * @return the website array list
     */
    CharSequence[] getPredefinedWebsites();

    /**
     * Get the site navigation count
     * @return the number
     */
    int getSiteNavigationCount();
}
