/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.provider;

import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.TreeMap;
import java.util.Collections;
import java.util.Map;

import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.FileTransferLog;

import android.net.Uri;

import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatListProvider;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatInfo;
import com.orangelabs.rcs.provider.messaging.ChatData;
import com.orangelabs.rcs.provider.messaging.FileTransferData;
import com.orangelabs.rcs.provider.messaging.MultiFileTransferData;
import com.orangelabs.rcs.provider.messaging.MessageData;
import com.orangelabs.rcs.provider.messaging.ChatProvider;
//import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.MediatekFactory;

/**
 * This class provided as interface for content provider
 */
public class RichMessagingDataProvider {
	
	   private static final String TAG = "RichMessagingDataProvider";
	   
		/**
		 * Current instance
		 */
		private static RichMessagingDataProvider instance = null;

		/**
		 * Content resolver
		 */
		private ContentResolver cr;
		
		/**
		 * Database URI
		 */
		private Uri msgdatabaseUri = ChatLog.Message.CONTENT_URI;

		/**
         * Unreg Group Database URI
         */
        private Uri msgUnregGroupUri = UnregGroupMessageProvider.CONTENT_URI;

		/**
		 * Database URI
		 */
		private Uri ftdatabaseUri = FileTransferLog.CONTENT_URI;

		
		private Uri chatDatabaseUri = ChatLog.GroupChat.CONTENT_URI;
		/**
		 * CHAT HISTORY LIMIT FOR CONTACT 
		 * */
		
		private String CONTACT_CHAT_HISTORY_LIMIT = "50";
		
		/**
		 * The logger
		 */
		//private Logger logger = Logger.getLogger(this.getClass().getName());
		
		/**
		 * Create instance
		 * 
		 * @param ctx Context
		 */
		public static synchronized void createInstance(Context ctx) {
			if (instance == null) {
				instance = new RichMessagingDataProvider(ctx);
			}
		}
		
		/**
		 * Returns instance
		 * 
		 * @return Instance
		 */
		public static RichMessagingDataProvider getInstance() {
			return instance;
		}
		
		/**
	     * Constructor
	     * 
	     * @param ctx Application context
	     */

		private RichMessagingDataProvider(Context ctx) {			
	        this.cr = ctx.getContentResolver();
		}

		 
		//get recent o2o chats
		private Cursor getRecentO2OChats() {
		   
	    	Uri databaseRecentMsgUri = Uri.parse("content://com.orangelabs.rcs.messaging/messaging/recent_messages");
	    	Cursor cursor = null;
	    	
	    	
	    	 Logger.d(TAG,"getRecentO2OChats() : fetching recentO2O Chats");
	    	 
	    
	    	
	    	try {
	    		//get the cursor for recent o2o chat history
	    		cursor = cr.query(databaseRecentMsgUri, null, null, null, null);
       
            } catch (Exception e) {
            	
            	Logger.d(TAG,"getRecentO2OChats() : EXCEPTION :"+ e.getMessage());
            	
            	
            	  if(cursor!=null){
                      cursor.close();
                    	cursor = null;	
                    }
                e.printStackTrace();
            }
	    	
	    	return cursor;
	    }
		
		//get recent group chats 
		 private Cursor getRecentGroupChats(){		  
		    	Uri databaseRecentGroupMsgUri = Uri.parse("content://com.orangelabs.rcs.messaging/messaging/recent_group_messages");
		    	Cursor cursor = null;	
		    	
		    	Logger.d(TAG,"getRecentGroupChats() : fetching recent group Chats");
		    	
		    	try {
		    		//get the cursor for recent o2o chat history
		    		cursor = cr.query(databaseRecentGroupMsgUri, null, null, null, null);
	       
	            } catch (Exception e) {
	            	
	            	
	            	Logger.d(TAG,"getRecentGroupChats() : EXCEPTION :"+ e.getMessage());
	            	
	            	
	            	  if(cursor!=null){
                      cursor.close();
                    	cursor = null;	
                    }
	                e.printStackTrace();
	            }	
		    	return cursor;
		 }
		
	/*
	 * get the recent contacts for a contact 
	 * */	
		public ArrayList<Integer> getRecentChatForContact(String Contact, Integer offset){
		    ArrayList<Integer> recentContactChats = new ArrayList<Integer>();
		    return recentContactChats;
		}
		
		
		public ArrayList<Integer> getRecentChatsForGroup(String groupChatID, Integer offset){
			ArrayList<Integer> recentChatsForGroup = new ArrayList<Integer>();    	
			    return recentChatsForGroup;

		}
		
		
		public ArrayList<ChatListProvider> getRecentChats(){
			
			Logger.d(TAG,"getRecentChats()");
			
			ArrayList<ChatListProvider> result = new ArrayList<ChatListProvider>();
			 return result;	
		}
		
		public void updateFileTransferStatus(String fileTransferId, int status) {
			Logger.i(TAG,"updateFileTransferStatus() : for fileTransferId = "+ fileTransferId + "status: " + status);
			Uri ftDatabaseUri = Uri.parse("content://com.orangelabs.rcs.ft/ft");
			if(!isMultiFT(fileTransferId)) {
		        ContentValues values = new ContentValues();
		        values.put(FileTransferData.KEY_STATE, status);
		        if (status == FileTransfer.State.DELIVERED.toInt()) {
		            // Delivered
		            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, Calendar.getInstance().getTimeInMillis());
		        } else
		        if (status == FileTransfer.State.DISPLAYED.toInt()) {
		            // Displayed
		            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, Calendar.getInstance().getTimeInMillis());
		        }
		        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId, null);
		        } else {
		            updateMultiFileTransferStatus(fileTransferId, status);
		        }
			
		}
		
		/**
	     * Update Multi file transfer status
	     * 
	     * @param sessionId Session ID
	     * @param status New status
	     */

	    public void updateMultiFileTransferStatus(String fileTransferId, int status) {
	        
	        Uri multiFtDatabaseUri = Uri.parse("content://com.orangelabs.rcs.multift/multift");
	        ContentValues values = new ContentValues();
	        values.put(MultiFileTransferData.KEY_STATUS, status);
	        if (status == FileTransfer.State.DELIVERED.toInt()) {
	            // Delivered
	            values.put(MultiFileTransferData.KEY_TIMESTAMP_DELIVERED, Calendar.getInstance().getTimeInMillis());
	        } else
	        if (status == FileTransfer.State.DISPLAYED.toInt()) {
	            // Displayed
	            values.put(MultiFileTransferData.KEY_TIMESTAMP_DISPLAYED, Calendar.getInstance().getTimeInMillis());
	        }
	        cr.update(multiFtDatabaseUri, values, MultiFileTransferData.KEY_FT_ID + " = " + fileTransferId, null);
	    }
		
		public boolean isMultiFT(String fileTransferId){
	        boolean status = false;
	        
	        Uri multiFtDatabaseUri = Uri.parse("content://com.orangelabs.rcs.multift/multift");
	        Logger.d(TAG, "isMultiFile Transfer : " + fileTransferId);
	        
	        List<String> result = new ArrayList<String>();
	        try {
	            Cursor cursor = cr.query(multiFtDatabaseUri, 
	                    new String[] {
	                    MultiFileTransferData.KEY_PARTICIPANTS_LIST
	                    },
	                     "(" + MultiFileTransferData.KEY_FT_ID + "=" + fileTransferId + ")",
	                    null, 
	                    MultiFileTransferData.KEY_TIMESTAMP + " DESC");
	            
	            while(cursor!=null && cursor.moveToNext()) {
	                status = true;
	            }
	            if(cursor!=null){
	                cursor.close();
			}
	        } catch(Exception e) {
	            e.printStackTrace();
	        }
	        
	        
	        Logger.d(TAG, "isMultiFile Transfer : " + fileTransferId + " status: " + status);
	        
	        return status;
		}

		public void updateFileTransferUrl(String fileTransferId, String url) {
			Logger.i(TAG,"updateFileTransferUrl() : for fileTransferId = "+ fileTransferId + "url: " + url);
			Uri ftDatabaseUri = Uri.parse("content://com.orangelabs.rcs.ft/ft");
			ContentValues values = new ContentValues();
	        values.put(FileTransferData.KEY_FILENAME, url);
	        values.put(FileTransferData.KEY_STATE, FileTransfer.State.TRANSFERRED.toInt());
	        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId, null);
			
		}
		
		
 /* delete the group chat + messages */
        public void deleteGroupChat(String chatID){
        		Logger.d(TAG,"deleteGroupChat() : for chat_id = "+ chatID);
        	//delete the message for messages table
 
        
        		// Delete entries
        		int deletedRows = cr.delete(msgdatabaseUri, 
        				ChatLog.Message.CHAT_ID + " = '" + chatID +"'", 
        				null);
        		Logger.d(TAG,"Messages delete for chat : "+chatID +" , msg count : " + deletedRows);	
        		
        		
        		//deleet the chat info
        		int deletedChat = cr.delete(chatDatabaseUri, 
        				ChatLog.GroupChat.CHAT_ID + " = '" + chatID +"'", 
        				null);
        		
        		Logger.d(TAG,"chat : "+chatID +" information deleted");	
        }

		 public void deleteGroupFt(String chatID){
        		Logger.d(TAG,"deleteGroupFt() : for chat_id = "+ chatID);       
        		// Delete entries
        		int deletedRows = cr.delete(ftdatabaseUri, 
        				FileTransferLog.CHAT_ID + " = '" + chatID +"'", 
        				null);
        		Logger.d(TAG,"FT delete for chat : "+chatID +" , msg count : " + deletedRows);	
        		       		
        		Logger.d(TAG,"chat : "+chatID +" information deleted");	
        }


        public void deleteMessagingLogForContact(String contact)
        {
        	Logger.d(TAG,"deleteMessagingLogForContact() : for contact = "+ contact);
        	
        	// Delete entries
    		int deletedRows = cr.delete(msgdatabaseUri, 
    				ChatLog.Message.CHAT_ID + " = '" + contact +"' AND " +
    				ChatLog.Message.MESSAGE_TYPE + " = " + ChatLog.Message.Type.CONTENT + " )",
    				null);
        	
    		Logger.d(TAG,"Messages delete for contact : "+contact +" , msg count : " + deletedRows);	
        }
        
		 public void deleteFtLogForContact(String contact)
        {
        	Logger.d(TAG,"deleteFtLogForContact() : for contact = "+ contact);
        	
        	// Delete entries
    		int deletedRows = cr.delete(ftdatabaseUri, 
    				FileTransferLog.CONTACT + " = '" + contact +"'", 
    				null);
        	
    		Logger.d(TAG,"FTS delete for contact : "+contact +" , msg count : " + deletedRows);	
        }
        
        
        public void markChatMessageAsRead(String msgId, boolean isRead){
             Logger.d(TAG,"markChatMessageAsRead() : for msgId = "+ msgId + " ;isread : "+isRead); 
             
             if(isRead){ 
                     ContentValues values = new ContentValues(); 
                     values.put(ChatLog.Message.STATUS, ChatLog.Message.Content.Status.DISPLAYED.toInt()); 
                     cr.update(msgdatabaseUri, values, ChatLog.Message.MESSAGE_ID + " = '" + msgId+"'", null); 
             } 
             
     }
        
        public void markFTMessageAsRead(String msgId, boolean isRead){ 
            Logger.d(TAG,"markFTMessageAsRead() : for FT msgId = "+ msgId + " ;isread : "+isRead); 
            
            if(isRead){ 
                    ContentValues values = new ContentValues(); 
                    values.put(FileTransferLog.STATE, FileTransfer.State.DISPLAYED.toInt()); 
                    cr.update(ftdatabaseUri, values, FileTransferLog.FT_ID + " = " + msgId, null); 
                    
            } 
        	
        }
        
        public void deleteImSessionEntry(String sessionId){
        	
        }

        //delete a message from the datbase by msgID
        public void deleteMessage(String msgID){
		    
           Logger.d(TAG,"deleteMessage with msgID = "+ msgID);
        	
        	// Delete entries
    		int deletedRows = cr.delete(msgdatabaseUri, 
    				ChatLog.Message.MESSAGE_ID + " = '" + msgID +"'", 
    				null);
        }
        
        //delete a message from the database by messageTag
        public void deleteUnregGroupMessage(int  messageTag){
            
           Logger.d(TAG,"deleteUnregGroupMessage with messageTag = "+ messageTag);
            
            // Delete entries
            int deletedRows = cr.delete(msgUnregGroupUri, 
                    UnregGroupMessageProvider.KEY_MESSAGE_TAG + " = '" + messageTag +"'", 
                    null);
        }
        
		//delete a FT from the datbase by msgID
        public void deleteFileTranfser(String ftId){
		    
           Logger.i(TAG,"deleteFileTranfser with ftId = "+ ftId);

			if(ftId != null){
	        	// Delete entries
	    		int deletedRows = cr.delete(ftdatabaseUri, 
	    				FileTransferLog.FT_ID + " = '" + ftId +"'", 
	    				null);
			}
			else{
				int deletedRows = cr.delete(ftdatabaseUri, null,null);
			}
        }
        
        public Cursor getAllMessageforGroupChat(String groupchatID){
        	Cursor cur = null;
        	Logger.d(TAG,"Get group chat Messages for " + groupchatID);

        	cur = cr.query(msgdatabaseUri, 
        			null,
        			"(" + ChatLog.Message.CHAT_ID + "='" + groupchatID + "' AND  " + 
        			ChatLog.Message.MESSAGE_TYPE + " = " + ChatLog.Message.Type.CONTENT + " )"
        			, 
    				null, 
    				ChatLog.Message.ID + " ASC ");
        	return cur;
        }
        
        //get the all messages id for o2o chats for all contacts 
        public ArrayList<Integer> getAllO2OMessageID(){
        	Cursor cursor = null;
        	ArrayList<Integer> msgIDList = new ArrayList<Integer>();
        	Logger.d(TAG,"Get all o2o messages ");

        	cursor = cr.query(msgdatabaseUri, 
        			new String[] {
        			ChatLog.Message.ID, ChatLog.Message.TIMESTAMP
                    },
        			"(" + ChatLog.Message.CHAT_ID + " = " + ChatLog.Message.CONTACT + " AND  " + 
        			ChatLog.Message.MESSAGE_TYPE + " = " + ChatLog.Message.Type.CONTENT + " )"
        			, 
    				null, 
    				ChatLog.Message.ID + " ASC ");
        	
        	if(cursor!=null){
	        	while(cursor.moveToNext()){
	        		 Integer msgID = -1;
	        		 msgID = cursor.getInt(0);
	        		 msgIDList.add(msgID);
	             }
	             cursor.close();
	        } 
        	
        	return msgIDList;
        }
        
        //get the all messages id for o2o chats for all contacts 
        public Cursor getAllO2OMessageIDCursor(){
        	Cursor cursor = null;
        	ArrayList<Integer> msgIDList = new ArrayList<Integer>();
        	Logger.d(TAG,"Get all o2o messages ");

        	cursor = cr.query(msgdatabaseUri, 
        			new String[] {
        			ChatLog.Message.ID, ChatLog.Message.TIMESTAMP
                    },
        			"(" + ChatLog.Message.CHAT_ID + " = " + ChatLog.Message.CONTACT + " AND  " + 
        			ChatLog.Message.MESSAGE_TYPE + " = " + ChatLog.Message.Type.CONTENT + " )"
        			, 
    				null, 
    				ChatLog.Message.ID + " ASC ");
        	
        	if(cursor!=null){
        		return cursor;
	        	/*while(cursor.moveToNext()){
	        		 Integer msgID = -1;
	        		 msgID = cursor.getInt(0);
	        		 msgIDList.add(msgID);*/
	             
	            // cursor.close();
	        } 
        	
        	return cursor;
        }
        


		/**
	 * Get the group chat participants who have been connected to the chat
	 * 
	 * @param chatId Chat ID
	 * @result List of contacts
	 */
	public List<String> getGroupChatConnectedParticipants1(String chatId) {
		

		Logger.d(TAG,"Get connected participants for " + chatId);
		List<String> result = new ArrayList<String>();
     	Cursor cursor = cr.query(msgdatabaseUri, 
    			new String[] {
					ChatLog.Message.CONTACT
    			},
    			"(" + ChatLog.Message.CHAT_ID + "='" + chatId + "') AND (" + 
    				ChatLog.Message.MESSAGE_TYPE + "=" + ChatLog.Message.Type.SYSTEM + ")",
    			null, 
    			ChatLog.Message.TIMESTAMP + " DESC");
    	while(cursor.moveToNext()) {
    		String participant = cursor.getString(0);
    		if ((participant != null) && (!result.contains(participant))) {
    			result.add(participant);
    		}
    	}
    	cursor.close();
    	return result;
	}

			/**
	 * Get the group chat participants who have been connected to the chat
	 * 
	 * @param chatId Chat ID
	 * @result List of contacts
	 */
	public List<String> getGroupChatConnectedParticipants(String chatId) {
		

		Logger.d(TAG,"Get connected participants for " + chatId);
		List<String> result = new ArrayList<String>();	

     	Cursor cursor = cr.query(chatDatabaseUri, 
    			new String[] {
					ChatLog.GroupChat.PARTICIPANTS
    			},
    			"(" + ChatLog.GroupChat.CHAT_ID + "='" + chatId + "')",
    			null, 
    			ChatLog.GroupChat.TIMESTAMP + " DESC");	
		if(cursor != null && cursor.moveToNext()){	        			
	    	String participant = cursor.getString(0);
			Logger.d(TAG,"getGroupChatConnectedParticipants: " + participant);
			
			String[] words = participant.split(",");
			for (int i = 0 ; i < words.length; i++){
			    String[] results = words[i].split("=");
				result.add(results[0]);
				Logger.d(TAG,"getGroupChatConnectedParticipants: " + results[0]);
			}
		}
		if(cursor != null){
    		cursor.close();
		}
    	return result;
	}
			
      //get the all messages id for o2o chats for all contacts 
        public Cursor getAllGroupChatMessageID(){
        	Cursor cursor = null;
        	Logger.d(TAG,"getAllGroupChatMessageID ");
        	ArrayList<String> msgIDList = new ArrayList<String>();

        	//get all the groupchat id
      
        	cursor = cr.query(chatDatabaseUri, 
        			new String[] {
        			ChatLog.GroupChat.CHAT_ID
                    },
        			null,null,null);
        	String chatIDList = "";
        	String chatID = "";
        	boolean startFlag = false;
        	if(cursor!=null){
	        	while(cursor.moveToNext()){
	        		
	        		if(!startFlag){
	        			chatIDList +="(";
	        			startFlag = true;	
	        		}else{
	        			chatIDList+=",";
	        		}
	        		
	        		Logger.d(TAG,"ChatID " + cursor.getString(0));
	        		chatID = cursor.getString(0);
	        		chatIDList += "'"+chatID+"'";
	             }
	        	if(startFlag){
	        		chatIDList +=")";
	        	}
	        	
	             cursor.close();
	        }else{
	        	return cursor;
	        }
        	
        	
        	//query the messages table
        	if(chatIDList!=""){
        	cursor = cr.query(msgdatabaseUri, 
        			new String[] {
        			ChatLog.Message.ID,ChatLog.Message.CHAT_ID,ChatLog.Message.TIMESTAMP
                    },
        			"(" + ChatLog.Message.CHAT_ID + " IN  " + chatIDList + " AND  " + 
        			ChatLog.Message.MESSAGE_TYPE + " = " + ChatLog.Message.Type.CONTENT + " )"
        			, 
    				null, 
    				ChatLog.Message.ID + " ASC ");
        	
        	if(cursor!=null){
	        	return cursor;
	        }else{
	        	return null;
	        }
        	}
        	else{
        	return null;
        	}
        }
        
        public Cursor getAllFtMessageID(){
        	Cursor cursor = null;
        //	ArrayList<Integer> ftIDList = new ArrayList<Integer>();
        	Logger.d(TAG,"Get all o2o messages ");

        	cursor = cr.query(ftdatabaseUri, 
        			new String[] {
        			FileTransferLog.FT_ID, FileTransferLog.CHAT_ID,FileTransferLog.TIMESTAMP
                    },
        			null
        			, 
    				null, 
    				FileTransferLog.FT_ID + " ASC ");
        	
        	
        	
        	return cursor;
        }
        
        public Cursor getAllO2OFtMessageID(){
        	Cursor cursor = null;
        //	ArrayList<Integer> ftIDList = new ArrayList<Integer>();
        	Logger.d(TAG,"Get all o2o messages ");

        	cursor = cr.query(ftdatabaseUri, 
        			new String[] {
        			FileTransferLog.FT_ID, FileTransferLog.CHAT_ID, FileTransferLog.TIMESTAMP
                    },
                    "(" + FileTransferLog.CHAT_ID + " =  " + FileTransferLog.CONTACT +  " )"
        			, 
    				null, 
    				FileTransferLog.FT_ID + " ASC ");
        	
        	
        	
        	return cursor;
        }
        
        public Cursor getAllGroupFtMessageID(){
        	Cursor cursor = null;
        //	ArrayList<Integer> ftIDList = new ArrayList<Integer>();
        	Logger.d(TAG,"Get all o2o messages ");

        	cursor = cr.query(ftdatabaseUri, 
        			new String[] {
        			FileTransferLog.FT_ID, FileTransferLog.CHAT_ID, FileTransferLog.TIMESTAMP
                    },
                    "(" + FileTransferLog.CHAT_ID + " <>  " + FileTransferLog.CONTACT +  " )"
        			, 
    				null, 
    				FileTransferLog.FT_ID + " ASC ");
        	
        	
        	
        	return cursor;
        }
}


