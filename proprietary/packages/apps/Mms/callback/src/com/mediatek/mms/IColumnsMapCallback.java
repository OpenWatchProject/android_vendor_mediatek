package com.mediatek.mms.callback;

public interface IColumnsMapCallback {
    public int getColumnMsgType();
    public int getColumnMmsSubId();
    public int getColumnSmsSubId();
    public int getColumnSmsAddress();
    public int getColumnSmsBody();
    public int getColumnSmsType();
    public int getColumnSmsIpMessageId();
    public int getColumnMsgId();

    /**
     * Get MMS CC column id.
     * @return int;
     */
    public int getColumnMmsCc();

    /**
     * Get MMS CCEncoding column Id.
     * @return int
     */
    public int getColumnMmsCcEncoding();
}
