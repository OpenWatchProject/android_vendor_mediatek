package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.chat.IExtendChatListener;
import com.gsma.services.rcs.chat.IChatMessage;

/**
 * Chat interface
 */
interface IExtendChat {
    List<String> getRemoteContacts();
    
    IChatMessage sendMessage(in String message, in int msgType);
    
    void sendDisplayedDeliveryReport(in String msgId);

    void sendBurnedDeliveryReport(in String msgId);

    String prosecuteMessage(in String msgId);

    void addEventListener(in IExtendChatListener listener);
    
    void removeEventListener(in IExtendChatListener listener);

    void resendMessage(in String msgId);
    
    String getExtendChatId();
}
