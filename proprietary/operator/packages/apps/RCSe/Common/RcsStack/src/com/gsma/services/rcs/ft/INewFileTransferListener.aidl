package com.gsma.services.rcs.ft;

/**
 * Callback method for new file transfer invitations and delivery reports
 */
interface INewFileTransferListener {
	void onNewFileTransfer(in String transferId);
	
	void onReportFileDelivered(String transferId);
	
	void onReportFileDisplayed(String transferId);
	
	void onFileDeliveredReport(String transferId,String contact);
	
	void onFileDisplayedReport(String transferId,String contact);
	
	void onNewFileTransferReceived(String transferId,boolean isAutoAccept,boolean isGroup,String chatSessionId,String ChatId,int timeLen);
	
	void onNewPublicAccountChatFile(String transferId,boolean isAutoAccept,boolean isGroup,String chatSessionId,String ChatId);
	
    void onNewBurnFileTransfer(String transferId,boolean isGroup,String chatSessionId,String ChatId); 
	
	
}