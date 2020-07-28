package com.android.browser.sitenavigation;

import android.net.Uri;

/**
 * M: Site navigation
 */
public class SiteNavigation {

    public static final int WEBSITE_NUMBER = 9;

    public static final int WEBSITE_NUMBER_FOR_TABLET = 8;

    public static final String AUTHORITY = "com.android.browser.site_navigation";

    //site navigation url
    public static final String SITE_NAVIGATION
                               = "content://" + AUTHORITY + "/" + "websites"; // + "/";

    public static final Uri SITE_NAVIGATION_URI
                           = Uri.parse("content://com.android.browser.site_navigation/websites");

    public static final String ID = "_id";

    public static final String URL = "url";

    public static final String TITLE = "title";

    public static final String DATE_CREATED = "created";

    public static final String WEBSITE = "website";

    public static final String FAVICON = "favicon";

    public static final String THUMBNAIL = "thumbnail";

    public static final String DEFAULT_THUMB = "default_thumb";

}
