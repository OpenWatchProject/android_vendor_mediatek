package com.gsma.services.rcs.chat;

import java.util.List;

import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.chat.IGroupChatListener;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.ft.IFileTransfer;
import com.gsma.services.rcs.ft.IOneToOneFileTransferListener;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.chat.IChatMessage;


/**
 * Group chat interface
 */
interface IGroupChat {
	String getChatId();

	int getDirection();

	int getState();	

	int getReasonCode();

	ContactId getRemoteContact();

	String getSubject();	

	Map getParticipants();

	long getTimestamp();

	IChatMessage sendMessage(in String text);
	
	IChatMessage sendMessageEx(in String text, in int msgType);
	
	void resendMessage(in String msgId);

	void setComposingStatus(in boolean ongoing);

	void inviteParticipants(in List<ContactId> participants);
	
	void removeParticipants(in List<ContactId> participants);
	
	int getMaxParticipants();
	
	void leave();

	IChatMessage sendMessage2(in Geoloc geoloc);

	void openChat();

	boolean isAllowedToSendMessage();

	boolean isAllowedToInviteParticipants();

	boolean isAllowedToInviteParticipant(in ContactId participant);

	boolean isAllowedToLeave();
}