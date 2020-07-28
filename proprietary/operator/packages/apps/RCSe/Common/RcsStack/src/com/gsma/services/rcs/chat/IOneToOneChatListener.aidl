package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.chat.IChatMessage;
import com.gsma.services.rcs.contact.ContactId;

/**
 * Chat event listener
 */
interface IOneToOneChatListener {
    void onMessageStatusChanged(in ContactId contact, in String mimeType, in String msgId,
			in int status, in int reasonCode);

	void onComposingEvent(in ContactId contact, in boolean status);

	void onMessagesDeleted(in ContactId contact, in List<String> msgIds);
}