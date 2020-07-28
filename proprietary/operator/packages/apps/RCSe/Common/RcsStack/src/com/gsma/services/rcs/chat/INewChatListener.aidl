package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.chat.IChatMessage;

/**
 * New chat invitation event listener
 */
interface INewChatListener {
	void onNewSingleChat(in String contact, in IChatMessage message);
	
	void onNewGroupChat(in String chatId);
}
