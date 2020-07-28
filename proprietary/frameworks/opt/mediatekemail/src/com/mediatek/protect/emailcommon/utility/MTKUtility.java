package com.mediatek.protect.emailcommon.utility;

import java.util.regex.Pattern;

public class MTKUtility {
    /**
     * M: transform html to plain text employing regex, also remove all the cr/lf and space/tab
     * @param html
     * @return the plain text
     */
    public static String htmlToText(String html) {
        if (html == null) {
            return "";
        }

        html = Pattern.compile("<head>.*</head>", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("");
        html = Pattern.compile("< *script[^>]*>", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("<script>");
        html = Pattern.compile("<script>.*</script>", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("");
        html = Pattern.compile("< *style[^>]*>", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("<style>");
        html = Pattern.compile("<style>.*</style>", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("");
        html = Pattern.compile("<[^>]*>", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("");
        html = Pattern.compile("&.{2,6};", Pattern.CASE_INSENSITIVE).matcher(html).replaceAll("");
        html = Pattern.compile("[ \r\n\f\t]+").matcher(html).replaceAll("");
        return html;
    }
}
