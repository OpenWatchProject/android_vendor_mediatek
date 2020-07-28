package com.mediatek.exchange;

/** M: Added for MTK EAS constants
 * Constants used throughout the EAS implementation are stored here.
 *
 */
public class MTKEas {

    // For logging.
    public static final String LOG_TAG = "Exchange";

    /// M: According the comments below, limit the truncation size is prevent from out of size for CursorWindow
    // These limits must never exceed about 500k which is half the max size of a Binder IPC buffer.

    // For EAS 12, we use HTML, so we want a larger size than in EAS 2.5
//    public static final String EAS12_TRUNCATION_SIZE = "200000";

    // For partial download, we just downloading 5kB for each message
    public static final String EAS12_TRUNCATION_SIZE_PARTIAL = "5120";
    // For EAS 2.5 fetch full message body once
    public static final String EAS2_5_FULL_SIZE = "8";

    // For optimize Exchange download window
    public static final String EMAIL_WINDOW_SIZE_EX = "50";
    public static final String EMAIL_FIRST_WINDOW_SIZE = "10";
}
