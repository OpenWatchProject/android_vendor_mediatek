package com.android.emailcommon.utility;

import java.util.regex.Pattern;

/**
 * M: A refactor class, abstract the same part of com.android.emailcommon.Address
 * and com.android.mail.providers.Address.
 */
public class AddressUtils {
    // Regex that matches address surrounded by '<>' optionally. '^<?([^>]+)>?$'
    public static final Pattern REMOVE_OPTIONAL_BRACKET = Pattern.compile("^<?([^>]+)>?$");
    // Regex that matches personal name surrounded by '""' optionally. '^"?([^"]+)"?$'
    public static final Pattern REMOVE_OPTIONAL_DQUOTE = Pattern.compile("^\"?([^\"]*)\"?$");
    // Regex that matches escaped character '\\([\\"])'
    public static final Pattern UNQUOTE = Pattern.compile("\\\\([\\\\\"])");


    // TODO: LOCAL_PART and DOMAIN_PART_PART are too permissive and can be improved.
    // TODO: Fix this to better constrain comments.
    /** Regex for the local part of an email address. */
    private static final String LOCAL_PART = "[^@]+";
    /** Regex for each part of the domain part, i.e. the thing between the dots. */
    private static final String DOMAIN_PART_PART = "[[\\w][\\d]\\-\\(\\)\\[\\]]+";
    /** Regex for the domain part, which is two or more {@link #DOMAIN_PART_PART} separated by . */
    private static final String DOMAIN_PART =
            "(" + DOMAIN_PART_PART + "\\.)+" + DOMAIN_PART_PART;

    /** Pattern to check if an email address is valid. */
    public static final Pattern EMAIL_ADDRESS =
            Pattern.compile("\\A" + LOCAL_PART + "@" + DOMAIN_PART + "\\z");


    // delimiters are chars that do not appear in an email address, used by pack/unpack
    public static final char LIST_DELIMITER_EMAIL = '\1';
    public static final char LIST_DELIMITER_PERSONAL = '\2';

    /**
     * Checks whether a string email address is valid.
     * E.g. name@domain.com is valid.
     */
    public static boolean isValidAddress(final String address) {
        return EMAIL_ADDRESS.matcher(address).find();
    }
}
