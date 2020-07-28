package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.contact.ContactId;

/**
 * Chat event listener
 */
interface IExtendChatListener {
   void onMessageStatusChanged(in String chatId,in ContactId contact, in String mimeType, in String msgId,
			in int status, in int reasonCode);

	void onComposingEvent(in ContactId contact, in boolean status);

	void onMessagesDeleted(in ContactId contact, in List<String> msgIds);
}