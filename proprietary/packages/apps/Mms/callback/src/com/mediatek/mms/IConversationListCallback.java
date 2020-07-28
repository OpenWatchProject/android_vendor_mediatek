package com.mediatek.mms.callback;

import android.widget.TextView;

public interface IConversationListCallback {

    /**
     * callback startQuery
     * @param selection: selection
     */
    public void startIpQuery(String selection);

    /**
     * callback setEmptyViewVisible
     * @param visible: visible
     */
    public void setEmptyViewVisible(int visible);

    /**
     * callback updateUnreadView
     * @param ipUnreadView: ipUnreadView
     */
    public void updateUnreadView(TextView ipUnreadView);

    /**
     * callback notifyDataSetChanged
     */
    public void notifyDataSetChanged();

    /**
     * callback loadNormalLayout
     */
    public void loadNormalLayout();

    /**
     * callback startQuery
     */
    public void startQuery();

    /**
     * callback updateGroupInfo
     * @param number: number
     * @return IpContact
     */
//    public IIpContactExt updateGroupInfo(String number);

    /**
     * callback getNumbersByThreadId
     * @param threadId: threadId
     * @return number
     */
    public String getNumbersByThreadId(long threadId);

    /**
     * callback invalidateGroupCache
     */
    public void invalidateGroupCache();
}
