package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.chat.ConferenceEventData;

/**
 * Conference info listener
 */
interface IGroupChatSyncingListener {
    void onSyncStart(in int count);
    void onSyncInfo(in String chatId, in ConferenceEventData info); 
    void onSyncDone(in int result);
}
