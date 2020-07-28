package com.mediatek.mail.vip.utils;

import android.net.Uri;

/**
 * Utility methods for dealing with URIs.
 */
public class UriUtils {
    /** Static helper, not instantiable. */
    private UriUtils() {}

    /** Checks whether two URI are equal, taking care of the case where either is null. */
    public static boolean areEqual(Uri uri1, Uri uri2) {
        if (uri1 == null && uri2 == null) {
            return true;
        }
        if (uri1 == null || uri2 == null) {
            return false;
        }
        return uri1.equals(uri2);
    }

    /** Parses a string into a URI and returns null if the given string is null. */
    public static Uri parseUriOrNull(String uriString) {
        if (uriString == null) {
            return null;
        }
        return Uri.parse(uriString);
    }

    /** Converts a URI into a string, returns null if the given URI is null. */
    public static String uriToString(Uri uri) {
        return uri == null ? null : uri.toString();
    }
}

