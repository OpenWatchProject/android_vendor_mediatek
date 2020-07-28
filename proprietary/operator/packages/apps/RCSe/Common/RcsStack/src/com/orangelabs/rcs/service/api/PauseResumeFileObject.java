package com.orangelabs.rcs.service.api;

import java.io.BufferedOutputStream;

public class PauseResumeFileObject {
	
	public String mFileTransferId;
	public String mPath;
	public String hashSelector;
	public long mSize;
	public String mFileSelector;
	public long bytesTransferrred;
	public int mState = 1;
	public String mContact;
	public BufferedOutputStream pausedStream;
	public String mOldFileTransferId;
	public String mMimeType;
	public int mTransferType;
}
