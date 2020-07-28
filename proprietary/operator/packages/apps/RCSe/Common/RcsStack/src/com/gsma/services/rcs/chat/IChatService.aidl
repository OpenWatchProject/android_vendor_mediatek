package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.IRcsServiceRegistrationListener;
import com.gsma.services.rcs.chat.IOneToOneChatListener;
import com.gsma.services.rcs.chat.IExtendChatListener;
import com.gsma.services.rcs.chat.IOneToOneChat;
import com.gsma.services.rcs.chat.IExtendChat;
import com.gsma.services.rcs.chat.IGroupChatListener;
import com.gsma.services.rcs.chat.IGroupChat;
import com.gsma.services.rcs.chat.IChatServiceConfiguration;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ICommonServiceConfiguration;
import com.gsma.services.rcs.RcsServiceRegistration;
import com.gsma.services.rcs.chat.IChatMessage;

/**
 * Chat service API
 */
interface IChatService {
	boolean isServiceRegistered();
	
	int getServiceRegistrationReasonCode();
    
	void addEventListener(IRcsServiceRegistrationListener listener);

	void removeEventListener(IRcsServiceRegistrationListener listener);

	IChatServiceConfiguration getConfiguration();
    
	IGroupChat initiateGroupChat(in List<ContactId> contacts, in String subject);
	
	IGroupChat initiateClosedGroupChat(in List<ContactId> contacts, in String subject);

	IOneToOneChat getOneToOneChat(in ContactId contact);
	
	IExtendChat getExtendChat(in ContactId contact);
	
	IExtendChat getExtendMultiChat(in List<String> contacts);

	IGroupChat getGroupChat(in String chatId);
	
	void markMessageAsRead(in String msgId);

	void addEventListener3(in IGroupChatListener listener);

	void removeEventListener3(in IGroupChatListener listener);

	void addEventListener2(in IOneToOneChatListener listener);
	
	void addEventListenerExtend2(in IExtendChatListener listener);

	void removeEventListener2(in IOneToOneChatListener listener);
	
	void removeEventListenerExtend2(in IExtendChatListener listener);

	int getServiceVersion();

	IChatMessage getChatMessage(in String msgId);

	ICommonServiceConfiguration getCommonConfiguration();

	boolean isAllowedToInitiateGroupChat();

	boolean isAllowedToInitiateGroupChat2(in ContactId contact);

	void deleteOneToOneChats();

	void deleteGroupChats();

	void deleteOneToOneChat(in ContactId contact);

	void deleteGroupChat(in String chatId);
	
	void blockGroupMessages(in String chatId, in boolean flag);

	void deleteMessage(in String msgId);

	void clearMessageDeliveryExpiration(in List<String> msgIds);
}