package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.chat.IChatMessage;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.contact.ContactId;

/**
 * Chat interface
 */
interface IOneToOneChat {

    ContactId getRemoteContact();

	IChatMessage sendMessage(in String message);

	void setComposingStatus(in boolean ongoing);

	IChatMessage sendMessage2(in Geoloc geoloc);

	void openChat();

	void resendMessage(in String msgId);

	boolean isAllowedToSendMessage();
}
