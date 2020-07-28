package com.gsma.services.rcs.chat;

import java.util.List;

/**
 * Group chat event listener
 */
public abstract class GroupChatSyncingListener extends IGroupChatSyncingListener.Stub {
    /**
     */
    public abstract void onSyncStart(int goupCount);

    /**
     */
    public abstract void onSyncInfo(String chatId, ConferenceEventData info);

    /**
     */
    public abstract void onSyncDone(int result);

}
