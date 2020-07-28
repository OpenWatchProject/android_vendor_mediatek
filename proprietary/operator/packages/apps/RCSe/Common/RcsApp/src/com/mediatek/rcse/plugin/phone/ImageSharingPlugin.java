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
package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentTransaction;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.media.MediaFile;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.provider.MediaStore.Images;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.webkit.MimeTypeMap;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
//import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.File;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;

import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.sharing.image.ImageSharing;
import com.gsma.services.rcs.sharing.image.ImageSharing.State;
import com.gsma.services.rcs.sharing.image.ImageSharing.ReasonCode;
import com.gsma.services.rcs.sharing.image.ImageSharingIntent;
import com.gsma.services.rcs.sharing.image.ImageSharingListener;

/**
 * This class defined to implement the function interface of ICallScreenPlugIn,
 * and archive the main function here.
 */
public class ImageSharingPlugin extends SharingPlugin {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "ImageSharingPlugin";
    /* package *//**
                  * The Constant IMAGE_SHARING_INVITATION_ACTION.
                  */
    static final String IMAGE_SHARING_INVITATION_ACTION =
            "com.orangelabs.rcs.richcall.IMAGE_SHARING_INVITATION";
    /* package *//**
                  * The Constant IMAGE_SHARING_START_ACTION.
                  */
    static final String IMAGE_SHARING_START_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_START_ACTION";
    /* package *//**
                  * The Constant IMAGE_SHARING_REPIC_ACTION.
                  */
    static final String IMAGE_SHARING_REPIC_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_REPIC_ACTION";
    /* package *//**
                  * The Constant IMAGE_SHARING_WARN_ACTION.
                  */
    static final String IMAGE_SHARING_WARN_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_WARN_ACTION";
    /* package *//**
                  * The Constant IMAGE_NAME.
                  */
    static final String IMAGE_NAME = "imageName";
    /* package *//**
                  * The Constant SELECT_TYPE.
                  */
    static final String SELECT_TYPE = "select_type";
    /**
     * The m image sharing invitation receiver.
     */
    private ImageSharingInvitationReceiver mImageSharingInvitationReceiver = null;
    /**
     * The m image sharing status.
     */
    private int mImageSharingStatus = ImageSharingStatus.UNKNOWN;
    /**
     * The m outgoing image sharing session.
     */
    private ImageSharing mOutgoingImageSharingSession = null;
    /**
     * The m image sharing view.
     */
    private View mImageSharingView = null;
    /**
     * The m incoming image sharing session.
     */
    private ImageSharing mIncomingImageSharingSession = null;
    /**
     * The m incoming session event listener.
     */
    private IncomingSessionEventListener mIncomingSessionEventListener = null;
    /**
     * The m outgoing session event listener.
     */
    private OutgoingSessionEventListener mOutgoingSessionEventListener = null;
    /**
     * The Constant FILE_UNIT.
     */
    private static final String FILE_UNIT = "KB";
    /**
     * The Constant FILE_UNIT_SIZE.
     */
    private static final double FILE_UNIT_SIZE = 1024;
    // For incoming image share information
    /**
     * The m incoming contact.
     */
    private String mIncomingContact = null;
    /**
     * The m incoming contact displayname.
     */
    private String mIncomingContactDisplayname = null;
    /**
     * The m incoming session id.
     */
    private String mIncomingSessionId = null;
    /**
     * The m incoming image name.
     */
    private String mIncomingImageName = null;
    /**
     * The m out going image name.
     */
    private String mOutGoingImageName = null;
    /**
     * The m incoming thumbnail.
     */
    private byte[] mIncomingThumbnail = null;
    /**
     * The m incoming image size.
     */
    private long mIncomingImageSize = 0;
    /**
     * The m compress dialog.
     */
    private CompressDialog mCompressDialog = null;
    /**
     * The Constant URI_PREFIX.
     */
    private static final String URI_PREFIX = "file://";
    /**
     * The Constant IDENTIFY_TEL.
     */
    private static final String IDENTIFY_TEL = "tel:";
    /**
     * The Constant MAX_SIZE_PIXELS.
     */
    private static final int MAX_SIZE_PIXELS = 3 * 1024 * 1024;
    /**
     * The m call screen dialog manager.
     */
    private CallScreenDialogManager mCallScreenDialogManager = null;
    /**
     * The m pending actions.
     */
    private ArrayList<Runnable> mPendingActions = new ArrayList<Runnable>();
    /**
     * The m receive warning dialog.
     */
    private WarningDialog mReceiveWarningDialog = null;
    /**
     * The m invite warning dialog.
     */
    private WarningDialog mInviteWarningDialog = null;
    /**
     * The m repick dialog.
     */
    private RepickDialog mRepickDialog = null;
    /**
     * The Constant entry_initiate.
     */
    private static final int entry_initiate = 0;
    /**
     * The Constant entry_terminated.
     */
    private static final int entry_terminated = 1;
    /**
     * The m max image sharing size.
     */
    private long mMaxImageSharingSize = 0;
    
    private Uri mCameraTempFileUri = null;
    /**
     * The Constant SLASH.
     */
    private static final String SLASH = "/";
    /**
     * The Constant RCSE_FILE_DIR.
     */
    private static String RCSE_FILE_DIR = "";
    /**
     * The Constant RCSE_TEMP_FILE_DIR.
     */
    private static String RCSE_TEMP_FILE_DIR = "";
    /**
     * The Constant RCSE_TEMP_FILE_NAME_HEADER.
     */
    private static final String RCSE_TEMP_FILE_NAME_HEADER = "tmp_joyn_";
    /**
     * The Constant JPEG_SUFFIX.
     */
    private static final String JPEG_SUFFIX = ".jpg";
    /**
     * The Constant COLOR.
     */
    private static final int COLOR = 0xFFFFFFFF;

    private static final int PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE = 909;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE = 910;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_VIDEO = 911;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_CAMERA = 912;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_GALLERY = 914;

    /**
     * The Class ImageSharingStatus.
     */
    public static final class ImageSharingStatus {
        /**
         * The Constant UNKNOWN.
         */
        private static final int UNKNOWN = 0;
        /**
         * The Constant INCOMING.
         */
        private static final int INCOMING = 1;
        /**
         * The Constant INCOMING_BLOCKING.
         */
        private static final int INCOMING_BLOCKING = 2;
        /**
         * The Constant OUTGOING.
         */
        private static final int OUTGOING = 3;
        /**
         * The Constant OUTGOING_BLOCKING.
         */
        private static final int OUTGOING_BLOCKING = 4;
        /**
         * The Constant COMPLETE.
         */
        private static final int COMPLETE = 5;
        /**
         * The Constant DECLINE.
         */
        private static final int DECLINE = 6;
    }

    /**
     * The m image sharing call back.
     */
    private ISharingPlugin mImageSharingCallBack = new ISharingPlugin() {
        @Override
        public void onApiConnected() {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    // there may be an case that content sharing plugin starts
                    // earlier than core service, so we need to refresh data
                    // that may be modified by auto-configuration
                    //PhoneUtils.initialize(mContext);
                    mMaxImageSharingSize = RcsSettings.getInstance()
                            .getMaxImageSharingSize() * 1024;
                }
            });
            int size = mPendingActions.size();
            for (int i = 0; i < size; i++) {
                mPendingActions.get(i).run();
            }
            mPendingActions.clear();
        }
        @Override
        public void onFinishSharing() {
            finishImageSharing();
        }
    };

    /**
     * Constructor.
     *
     * @param context the context
     */
    public ImageSharingPlugin(final Context context) {
        super(context);
        Logger.d(TAG,
                "ImageSharingPlugin contructor entry, context: "
                        + context);
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.v(TAG,
                        "ImageSharingPlugin contructor doInBackground entry");
                RcsSettings rcsSetting = RcsSettings.getInstance();
                if (rcsSetting != null) {
                    mMaxImageSharingSize = rcsSetting
                            .getMaxImageSharingSize() * 1024;
                    Logger.v(
                            TAG,
                            "ImageSharingPlugin contructor doInBackground  mMaxImageSharingSize = "
                                    + mMaxImageSharingSize);
                } else {
                    Logger.e(
                            TAG,
                            "ImageSharingPlugin contructor doInBackground" +
                            " RcsSettings.getInstance() return null");
                }
                Logger.v(TAG,
                        "ImageSharingPlugin contructor doInBackground exit");
                return null;
            }
            @Override
            protected void onPostExecute(Void result) {
                Logger.v(TAG,
                        "ImageSharingPlugin contructor onPostExecute entry");
                mImageSharingInvitationReceiver = new ImageSharingInvitationReceiver();
                // Register rich call invitation listener
                IntentFilter intentFilter = new IntentFilter();
                intentFilter
                        .addAction(ImageSharingIntent.ACTION_NEW_INVITATION);
                intentFilter.addAction(IMAGE_SHARING_START_ACTION);
                intentFilter.addAction(IMAGE_SHARING_REPIC_ACTION);
                intentFilter.addAction(IMAGE_SHARING_WARN_ACTION);
                mContext.registerReceiver(
                        mImageSharingInvitationReceiver, intentFilter);
                mCountDownLatch.countDown();
                Logger.v(TAG,
                        "ImageSharingPlugin contructor onPostExecute exit");
            }
        } .execute();
        mCallScreenDialogManager = new CallScreenDialogManager();
        mInterface = mImageSharingCallBack;
    }
    /**
     * Start image sharing.
     *
     * @param imageName the image name
     */
    private void startImageSharing(final String imageName) {
        Logger.d(TAG, "startImageSharing entry, with image name: "
                + imageName + " mImageSharingStatus: "
                + mImageSharingStatus);
        showThumbnails(imageName);
        if (mRichCallStatus == RichCallStatus.CONNECTED) {
            mImageSharingStatus = ImageSharingStatus.OUTGOING;
            addOutGoingListener(imageName);
        } else {
            mImageSharingStatus = ImageSharingStatus.OUTGOING_BLOCKING;
            mPendingActions.add(new Runnable() {
                @Override
                public void run() {
                    addOutGoingListener(imageName);
                }
            });
        }
        Logger.d(TAG, "startImageSharing exit");
    }
    /**
     * Adds the out going listener.
     *
     * @param imageName the image name
     */
    private void addOutGoingListener(final String imageName) {
        Logger.d(TAG, "addOutGoingListener entry");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                boolean success = false;
                mOutgoingSessionEventListener = new OutgoingSessionEventListener(
                        imageName);
                try {
                    mOutgoingImageSharingSession = mImageSharingApi
                            .shareImage(null,Uri.fromFile(new File(imageName))); // ToDo
                    Logger.d(TAG,
                            "addOutGoingListener() mOutgoingImageSharingSession: "
                                    + mOutgoingImageSharingSession);
                    if (mOutgoingImageSharingSession != null) {
                        mImageSharingApi
                                .addEventListener(mOutgoingSessionEventListener);
                        success = true;
                    }
                } catch (RcsServiceException e) {
                    e.printStackTrace();
                }
                if (!success) {
                    reset();
                }
                Logger.d(TAG, "addOutGoingListener() success: "
                        + success);
            }
        });
        Logger.d(TAG, "addOutGoingListener exit");
    }

    /**
     * The Class ThumbnailAsyncTask.
     */
    private class ThumbnailAsyncTask extends
            AsyncTask<Void, Void, Bitmap> {
        /**
         * The m image name.
         */
        private String mImageName = null;
        /**
         * The m file size.
         */
        private long mFileSize = 0;

        /**
         * Instantiates a new thumbnail async task.
         *
         * @param imageName the image name
         */
        public ThumbnailAsyncTask(String imageName) {
            mImageName = imageName;
        }
        /* (non-Javadoc)
         * @see android.os.AsyncTask#doInBackground(Params[])
         */
        @Override
        protected Bitmap doInBackground(Void... params) {
            Logger.d(TAG, "ThumbnailAsyncTask doInBackground entry");
            Bitmap image = null;
            if (com.mediatek.rcse.service.Utils
                    .isFileExist(mImageName)) {
                image = ThumbnailUtils.createImageThumbnail(
                        mImageName, Images.Thumbnails.MINI_KIND);
                int degrees = com.mediatek.rcse.service.Utils
                        .getDegreesRotated(mImageName);
                if (0 != degrees) {
                    image = com.mediatek.rcse.service.Utils.rotate(
                            image, degrees);
                } else {
                    Logger.d(TAG,
                            "ThumbnailAsyncTask file degress is zero, so no need to rotate");
                }
            } else {
                Logger.e(TAG, "ThumbnailAsyncTask the file "
                        + mImageName + " doesn't exist!");
            }
            java.io.File file = new File(mImageName);
            mFileSize = file.length();
            Logger.d(TAG, "ThumbnailAsyncTask doInBackground exit");
            return image;
        }
        /* (non-Javadoc)
         * @see android.os.AsyncTask#onPostExecute(java.lang.Object)
         */
        @Override
        protected void onPostExecute(Bitmap bitmap) {
            Logger.d(TAG, "ThumbnailAsyncTask onPostExecute entry");
            ImageView view = (ImageView) mImageSharingView
                    .findViewById(R.id.shared_image);
            if (view != null) {
                view.setImageBitmap(bitmap);
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask view is null");
            }
            final ImageView cancelView = (ImageView) mImageSharingView
                    .findViewById(R.id.cancel_image);
            cancelView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mCallScreenDialogManager
                            .showTerminateConfirmDialog();
                }
            });
            TextView fileNameView = (TextView) mImageSharingView
                    .findViewById(R.id.file_name);
            if (mImageName != null) {
                fileNameView.setText(mImageName.substring(mImageName
                        .lastIndexOf("/") + 1));
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask imageName is null");
            }
            TextView fileSizeView = (TextView) mImageSharingView
                    .findViewById(R.id.file_size);
            if (fileSizeView != null) {
                String value = roundDouble((double) mFileSize
                        / FILE_UNIT_SIZE, 1)
                        + FILE_UNIT;
                fileSizeView.setText(value);
            } else {
                Logger.d(TAG,
                        "ThumbnailAsyncTask fileSizeView is null");
            }
            Logger.d(TAG, "ThumbnailAsyncTask onPostExecute exit");
        }
    }

    /**
     * Show thumbnails.
     *
     * @param imageName the image name
     */
    private void showThumbnails(final String imageName) {
        Logger.v(TAG, "showThumbnails entry");
        initTransferringLayout();
        if (mCallScreenHost != null) {
            if (mDisplayArea == null) {
                // Here requestAreaForDisplay() may return null.
                mDisplayArea = mCallScreenHost
                        .requestAreaForDisplay();
            }
            if (mDisplayArea != null) {
                mDisplayArea.removeAllViews();
                mDisplayArea.addView(mImageSharingView);
                mCallScreenHost.onStateChange(getCurrentState());
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask mDisplayArea is:"
                        + mDisplayArea);
            }
        } else {
            Logger.d(TAG,
                    "ThumbnailAsyncTask mCallScreenHost is null");
        }
        new ThumbnailAsyncTask(imageName).execute();
        Logger.v(TAG, "showThumbnails exit");
    }

    /**
     * Outgoing image sharing session event listener.
     *
     * @see OutgoingSessionEventEvent
     */
    private class OutgoingSessionEventListener extends
            ImageSharingListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "OutgoingSessionEventListener";
        /**
         * The m file.
         */
        private String mFile = null;

        /**
         * Instantiates a new outgoing session event listener.
         *
         * @param file the file
         */
        public OutgoingSessionEventListener(String file) {
            mFile = file;
        }
        
        /**
         * Callback called when the image sharing state/reasonCode has been changed.
         * 
         * @param contact Contact ID
         * @param sharingId ID of image sharing
         * @param state State of image sharing
         * @param reasonCode Reason code of the image sharing state
         */
        public void onStateChanged(ContactId contact, String sharingId, State state,
                ReasonCode reasonCode){
            if(state == State.STARTED) {
                Logger.v(TAG, "onSharingStarted()");
                showAcceptedToast();            
            } else if(state == State.ABORTED) {
                Logger.v(TAG, "onSharingAborted()");
                finishImageSharing();         
                
            } else if(state == State.REJECTED) {
                mCallScreenDialogManager.showRejectedDialog(); 
                finishImageSharing();
            } else if(state == State.FAILED) {
                mCallScreenDialogManager.showInitFailDialog();    
                finishImageSharing();                
            } else if(state == State.TRANSFERRED) {
                try {
                    Logger.d(TAG, "onImageShared entry, file:");
                    //showFullImage(arg0); ToDo
                    mImageSharingApi
                            .removeEventListener(mOutgoingSessionEventListener);
                    if (mWakeLockCount.decrementAndGet() >= 0) {
                        mWakeLock.release();
                    }
                    Logger.d(TAG, "handleImageTransfered exit");
                } catch (RcsServiceException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                           
            }
        }

        /**
         * Callback called during the sharing progress.
         * 
         * @param contact Contact ID
         * @param sharingId ID of image sharing
         * @param currentSize Current transferred size in bytes
         * @param totalSize Total size to transfer in bytes
         */
        public void onProgressUpdate(ContactId contact, String sharingId, final long currentSize,
                final long totalSize){

            Logger.d(TAG, "onSharingProgress entry");
            /*
             * work around for ALPS00288583, because SUCCESS-REPORT cannot be
             * received on the sender side.
             */
            try {
                if (currentSize < totalSize) {
                    mMainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            updateProgressBar(currentSize, totalSize);
                        }
                    });
                } else {
                    Logger.d(TAG,
                            "handleSharingProgress handleImageTransfered");
                    onImageShared(mFile);
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            Logger.d(TAG, "handleSharingProgress exit");
        
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several image
         * sharings was deleted specified by the sharingIds parameter corresponding to a specific
         * contact.
         *
         * @param contact Contact ID
         * @param sharingIds ids of those deleted image sharing
         */
        public void onDeleted(ContactId contact, Set<String> sharingIds){
            
        }
        // Session is started
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingStarted()
         */
        public void onSharingStarted() {
            Logger.v(TAG, "onSharingStarted()");
            showAcceptedToast();
        }
        // Session has been aborted
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingAborted()
         */
        public void onSharingAborted() {
            Logger.v(TAG, "onSharingAborted()");
            finishImageSharing();
        }
        /* // Session has been terminated by remote
         public void handleSessionTerminatedByRemote() {
             Logger.v(TAG, "handleSessionTerminatedByRemote()");
             mCallScreenDialogManager.showTerminatedDialog();
             finishImageSharing();
         }
        */
        // Content sharing error
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingError(int)
         */
        public void onSharingError(final int error) {/*
            Logger.v(TAG, "onSharingError(), error = " + error);
            switch (error) {
            case ImageSharing.Error.INVITATION_DECLINED:
                mCallScreenDialogManager.showRejectedDialog();
                break;
            case ImageSharing.Error.SHARING_FAILED:
                mCallScreenDialogManager.showInitFailDialog();
                break;
            case ContentSharingError.SESSION_INITIATION_CANCELLED:
                mCallScreenDialogManager.showTerminatedDialog();
                break;
             case ContentSharingError.SESSION_INITIATION_FAILED:
                 mCallScreenDialogManager.showInitFailDialog();
                 break;
            case ContentSharingError.SESSION_INITIATION_TIMEOUT:
                mCallScreenDialogManager.showTimeOutDialog();
                break;
            case ImageSharing.Error.SAVING_FAILED:
                mCallScreenDialogManager.showNoStorageDialog();
                break;
            default:
                break;
            }
            finishImageSharing();
        */}
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onImageShared(java.lang.String)
         */
        //@Override
        public void onImageShared(String arg0) {
            try {
                Logger.d(TAG, "onImageShared entry, file: " + arg0);
                showFullImage(arg0);
                mImageSharingApi
                        .removeEventListener(mOutgoingSessionEventListener);
                if (mWakeLockCount.decrementAndGet() >= 0) {
                    mWakeLock.release();
                }
                Logger.d(TAG, "handleImageTransfered exit");
            } catch (RcsServiceException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingProgress(long, long)
         */
       // @Override
        public void onSharingProgress(final long arg0, final long arg1) {
            Logger.d(TAG, "onSharingProgress entry");
            /*
             * work around for ALPS00288583, because SUCCESS-REPORT cannot be
             * received on the sender side.
             */
            try {
                if (arg0 < arg1) {
                    mMainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            updateProgressBar(arg0, arg1);
                        }
                    });
                } else {
                    Logger.d(TAG,
                            "handleSharingProgress handleImageTransfered");
                    onImageShared(mFile);
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            Logger.d(TAG, "handleSharingProgress exit");
        }
    }

    public void clearImageSharingViews()
    {
    	 Logger.w(TAG,
                 "clearImageSharingViews entery");
    	reset();
        if (mDisplayArea != null) {
            mDisplayArea.removeAllViews();
        } else {
            Logger.d(TAG,
                    "clearImageSharingViews mDisplayArea is null");
        }
        if (mCallScreenHost != null) {
            mCallScreenHost.onStateChange(getCurrentState());
        } else {
            Logger.w(TAG,
                    "clearImageSharingViews mCallScreenHost is null");
        }
    }

    /*
     * finish image sharing when there is abort, error, terminated or cancel
     * happened.
     */
    /**
     * Finish image sharing.
     */
    private void finishImageSharing() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "finishImageSharing entry");
                if (mWakeLockCount.decrementAndGet() >= 0) {
                    mWakeLock.release();
                }
                reset();
                if (mDisplayArea != null) {
                    mDisplayArea.removeAllViews();
                } else {
                    Logger.d(TAG,
                            "finishImageSharing mDisplayArea is null");
                }
                if (mCallScreenHost != null) {
                    mCallScreenHost.onStateChange(getCurrentState());
                } else {
                    Logger.w(TAG,
                            "finishImageSharing mCallScreenHost is null");
                }
                Logger.d(TAG, "finishImageSharing exit");
            }
        });
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.d(TAG,
                        "finishImageSharing doInBackground entry");
                clearSavedDialogs();
                if (mOutgoingImageSharingSession != null) {
                    try {
                        mImageSharingApi
                                .removeEventListener(mOutgoingSessionEventListener);
                        mOutgoingImageSharingSession.abortSharing();
                    } catch (RcsServiceException e) {
                        e.printStackTrace();
                    }
                } else {
                    Logger.d(TAG,
                            "finishImageSharing mOutgoingImageSharingSession is null");
                }
                if (mIncomingImageSharingSession != null) {
                    try {
                        mImageSharingApi
                                .removeEventListener(mIncomingSessionEventListener);
                        mIncomingImageSharingSession.abortSharing();
                    } catch (RcsServiceException e) {
                        e.printStackTrace();
                    }
                } else {
                    Logger.d(TAG,
                            "finishImageSharing mIncomingImageSharingSession is null");
                }
                Logger.d(TAG,
                        "finishImageSharing doInBackground exit");
                return null;
            }
        } .execute();
    }
    /**
     * Reset.
     */
    private void reset() {
        Logger.v(TAG, "reset() entry");
        mImageSharingStatus = ImageSharingStatus.UNKNOWN;
        Utils.setInImageSharing(false);
        Logger.v(TAG, "reset() exit");
    }
    /**
     * Update progress bar.
     *
     * @param currentSize the current size
     * @param totalSize the total size
     */
    private void updateProgressBar(long currentSize, long totalSize) {
        Logger.d(TAG, "updateProgressBar entry, with currentSize: "
                + currentSize);
        if (mImageSharingView == null) {
            initTransferringLayout();
        }
        if (mImageSharingView != null) {
            ProgressBar progressBar = (ProgressBar) mImageSharingView
                    .findViewById(R.id.progress_bar);
            if (progressBar != null) {
                if (currentSize != 0) {
                    double position = ((double) currentSize / (double) totalSize) * 100.0;
                    progressBar.setProgress((int) position);
                } else {
                    progressBar.setProgress(0);
                }
            } else {
                Logger.e(TAG,
                        "updateProgressBar(), progressBar is null!");
            }
        } else {
            Logger.e(TAG,
                    "updateProgressBar(), mImageSharingView is null!");
        }
        Logger.d(TAG, "updateProgressBar exit");
    }
    /**
     * Inits the transferring layout.
     */
    private void initTransferringLayout() {
        Logger.v(TAG, "initTransferringLayout entry");
        mImageSharingView = LayoutInflater.from(mContext).inflate(
                R.layout.richcall_image_sharing, null);
        Logger.d(TAG,
                "initTransferringLayout exit, mImageSharingView: "
                        + mImageSharingView);
    }

    /**
     * Incoming video sharing session event listener.
     *
     * @see IncomingSessionEventEvent
     */
    private class IncomingSessionEventListener extends
            ImageSharingListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "IncomingSessionEventListener";
        
        /**
         * Callback called when the image sharing state/reasonCode has been changed.
         * 
         * @param contact Contact ID
         * @param sharingId ID of image sharing
         * @param state State of image sharing
         * @param reasonCode Reason code of the image sharing state
         */
        public void onStateChanged(ContactId contact, String sharingId, State state,
                ReasonCode reasonCode){
            if(state == State.STARTED) {
                Logger.v(TAG, "onSharingStarted() entry");            
            } else if(state == State.ABORTED) {
                Logger.v(TAG, "onSharingAborted()");
                mCallScreenDialogManager.showTerminatedDialog();
                finishImageSharing();      
                
            } else if(state == State.REJECTED) {
                mCallScreenDialogManager.showRejectedDialog(); 
                finishImageSharing();
            } else if(state == State.FAILED) {
                mCallScreenDialogManager.showInitFailDialog();    
                finishImageSharing();                
            } else if(state == State.TRANSFERRED) {
                try {
                    Logger.d(TAG, "onImageShared entry, file: ");
                    //showFullImage(arg0);// ToDo
                    mImageSharingStatus = ImageSharingStatus.COMPLETE;
                    mMainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mCallScreenHost != null) {
                                mCallScreenHost
                                        .onStateChange(getCurrentState());
                            } else {
                                Logger.w(TAG,
                                        "handleImageTransfered mCallScreenHost is null");
                            }
                        }
                    });
                    mImageSharingApi
                            .removeEventListener(mIncomingSessionEventListener);
                    if (mWakeLockCount.decrementAndGet() >= 0) {
                        mWakeLock.release();
                    }
                    Logger.d(TAG, "handleImageTransfered exit");
                } catch (RcsServiceException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }

        /**
         * Callback called during the sharing progress.
         * 
         * @param contact Contact ID
         * @param sharingId ID of image sharing
         * @param currentSize Current transferred size in bytes
         * @param totalSize Total size to transfer in bytes
         */
        public void onProgressUpdate(ContactId contact, String sharingId, final long currentSize,
                final long totalSize){
            try {
                Logger.d(TAG, "onSharingProgress entry");
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        updateProgressBar(currentSize, totalSize);
                    }
                });
                Logger.d(TAG, "handleSharingProgress exit");
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several image
         * sharings was deleted specified by the sharingIds parameter corresponding to a specific
         * contact.
         *
         * @param contact Contact ID
         * @param sharingIds ids of those deleted image sharing
         */
        public void onDeleted(ContactId contact, Set<String> sharingIds){
            
        }

        // Session is started
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingStarted()
         */
        public void onSharingStarted() {
            Logger.v(TAG, "onSharingStarted() entry");
        }
        // Session has been aborted
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingAborted()
         */
        public void onSharingAborted() {
            Logger.v(TAG, "onSharingAborted()");
            mCallScreenDialogManager.showTerminatedDialog();
            finishImageSharing();
        }
        // Session has been terminated by remote
        /**
         * Handle session terminated by remote.
         */
        public void handleSessionTerminatedByRemote() {
            Logger.v(TAG, "handleSessionTerminatedByRemote()");
            mCallScreenDialogManager.showTerminatedDialog();
            finishImageSharing();
        }
        // Sharing error
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingError(int)
         */
        public void onSharingError(final int error) {/*
            Logger.v(TAG, "onSharingError(), error = " + error);
            switch (error) {
            case ImageSharing.Error.INVITATION_DECLINED:
                mCallScreenDialogManager.showTerminatedDialog();
                break;
            case ImageSharing.Error.SHARING_FAILED:
                mCallScreenDialogManager.showInitFailDialog();
                break;
            case ImageSharing.Error.SAVING_FAILED:
                mCallScreenDialogManager.showNoStorageDialog();
                break;
            default:
                break;
            }
            finishImageSharing();
        */}
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onImageShared(java.lang.String)
         */
        
        public void onImageShared(String arg0) {
            try {
                Logger.d(TAG, "onImageShared entry, file: " + arg0);
                showFullImage(arg0);
                mImageSharingStatus = ImageSharingStatus.COMPLETE;
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mCallScreenHost != null) {
                            mCallScreenHost
                                    .onStateChange(getCurrentState());
                        } else {
                            Logger.w(TAG,
                                    "handleImageTransfered mCallScreenHost is null");
                        }
                    }
                });
                mImageSharingApi
                        .removeEventListener(mIncomingSessionEventListener);
                if (mWakeLockCount.decrementAndGet() >= 0) {
                    mWakeLock.release();
                }
                Logger.d(TAG, "handleImageTransfered exit");
            } catch (RcsServiceException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.image.ImageSharingListener#onSharingProgress(long, long)
         */
        
        public void onSharingProgress(final long arg0, final long arg1) {
            try {
                Logger.d(TAG, "onSharingProgress entry");
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        updateProgressBar(arg0, arg1);
                    }
                });
                Logger.d(TAG, "handleSharingProgress exit");
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    /**
     * Show full image.
     *
     * @param imageName the image name
     */
    private void showFullImage(final String imageName) {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                Logger.d(TAG, "showFullImage doInBackground entry");
                RcseImage image = new RcseImage(mContext,
                        Uri.parse(URI_PREFIX + imageName));
                Bitmap bitmap = image.fullSizeBitmap(MAX_SIZE_PIXELS,
                        true);
                Logger.d(TAG, "showFullImage doInBackground exit");
                return bitmap;
            }
            @Override
            protected void onPostExecute(Bitmap bitmap) {
                Logger.d(TAG,
                        "showFullImage()123, onPostExecute entry");
                if (mDisplayArea != null) {
                    if (mCallScreenHost != null) {
                        //mDisplayArea.removeView(mImageSharingView);
                        mDisplayArea.removeAllViews();
                        mImageSharingView = LayoutInflater.from(
                                mContext).inflate(
                                R.layout.richcall_image_full_display,
                                mDisplayArea);
                        ImageView imageview = (ImageView) mImageSharingView
                                .findViewById(R.id.shared_image);
                        imageview.setImageBitmap(bitmap);
                    } else {
                        mDisplayArea.removeAllViews();
                        //mDisplayArea.removeView(mImageSharingView);
                        mImageSharingView = LayoutInflater.from(
                                mContext).inflate(
                                R.layout.richcall_image_full_display,
                                mDisplayArea);
                        ImageView imageview = (ImageView) mImageSharingView
                                .findViewById(R.id.shared_image);
                        imageview.setImageBitmap(bitmap);
                        Logger.d(TAG,
                                "showFullImage()3, mCallScreenHost is null");
                    }
                    Logger.d(TAG,
                            "showFullImage(), mDisplayArea display full image: "
                                    + bitmap);
                } else {
                    Logger.d(TAG,
                            "showFullImage(), mDisplayArea is null");
                }
                mImageSharingStatus = ImageSharingStatus.COMPLETE;
                Utils.setInImageSharing(false);
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mCallScreenHost != null) {
                            mCallScreenHost
                                    .onStateChange(getCurrentState());
                        } else {
                            Logger.w(TAG,
                                    "showFullImage()2, mCallScreenHost is null");
                        }
                    }
                });
                Logger.d(TAG, "showFullImage(), onPostExecute exit");
            }
        } .execute();
    }
    /**
     * On user accept image sharing.
     *
     * @param context the context
     * @param fileName the file name
     * @param fileSize the file size
     */
    private void onUserAcceptImageSharing(Context context,
            String fileName, long fileSize) {
        Logger.i(TAG, "onUserAcceptImageSharing(), entry!");
        try {
            long maxFileSize;
            long warningFileSize;
            maxFileSize = mImageSharingApi.getConfiguration()
                    .getMaxSize() * 1024; // ToDo
            warningFileSize = mImageSharingApi.getConfiguration()
                    .getMaxSize() * 1024;
            Logger.w(TAG,
                    "onUserAcceptImageSharing() maxFileSize is "
                            + maxFileSize);
            Logger.w(TAG,
                    "onUserAcceptImageSharing() warningFileSize is "
                            + warningFileSize);
            if (fileSize >= warningFileSize && warningFileSize != 0) {
                boolean isRemind = AppSettings.getInstance()
                        .restoreRemindWarningLargeImageFlag();
                Logger.w(TAG,
                        "WarningDialog onCreateDialog the remind status is "
                                + isRemind);
                if (isRemind) {
                    Activity activity = null;
                    if (mCallScreenHost != null) {
                        activity = mCallScreenHost
                                .getCallScreenActivity();
                        if (activity != null) {
                            Logger.d(TAG, "show compress dialog");
                            mReceiveWarningDialog = new WarningDialog();
                            mReceiveWarningDialog.saveParameters(
                                    fileName, entry_terminated);
                            mReceiveWarningDialog.show(
                                    activity.getFragmentManager(),
                                    WarningDialog.TAG);
                        }
                        return;
                    }
                } else {
                    acceptImageSharing();
                }
            } else {
                acceptImageSharing();
            }
            Logger.i(TAG, "onUserAccept(), exit!");
        } catch (RcsServiceException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Accept image sharing.
     */
    private void acceptImageSharing() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected void onPreExecute() {
                Logger.d(TAG, "acceptImageSharing onPreExecute entry");
                initTransferringLayout();
                TextView fileNameView = (TextView) mImageSharingView
                        .findViewById(R.id.file_name);
                if (mIncomingImageName != null) {
                    fileNameView.setText(mIncomingImageName);
                } else {
                    Logger.d(TAG,
                            "acceptImageSharing mIncomingImageName is null");
                }
                ImageView thumbView = (ImageView) mImageSharingView
                        .findViewById(R.id.shared_image);
                if (thumbView != null) {
                    //view.setImageBitmap(bitmap);
                    if (mIncomingThumbnail != null) {
                        Logger.d(TAG,
                                "ThumbnailAsyncTask view is not null");
                        thumbView
                                .setImageBitmap(BitmapFactory
                                        .decodeByteArray(
                                                mIncomingThumbnail,
                                                0,
                                                mIncomingThumbnail.length));
                    }
                } else {
                    Logger.d(TAG, "ThumbnailAsyncTask view is null");
                }
                final ImageView cancelView = (ImageView) mImageSharingView
                        .findViewById(R.id.cancel_image);
                cancelView.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        mCallScreenDialogManager
                                .showTerminateConfirmDialog();
                    }
                });
                TextView fileSizeView = (TextView) mImageSharingView
                        .findViewById(R.id.file_size);
                if (mIncomingImageName != null) {
                    String value = roundDouble(
                            (double) mIncomingImageSize
                                    / FILE_UNIT_SIZE, 1)
                            + FILE_UNIT;
                    fileSizeView.setText(value);
                } else {
                    Logger.d(TAG,
                            "acceptImageSharing fileSizeView is null");
                }
                if (mCallScreenHost != null) {
                    mDisplayArea = mCallScreenHost
                            .requestAreaForDisplay();
                    if (mDisplayArea != null) {
                        mDisplayArea.removeAllViews();
                        mDisplayArea.addView(mImageSharingView);
                    } else {
                        Logger.d(TAG,
                                "acceptImageSharing mDisplayArea is null");
                    }
                    mCallScreenHost.onStateChange(getCurrentState());
                } else {
                    Logger.d(TAG,
                            "acceptImageSharing mCallScreenHost is null");
                }
                Logger.d(TAG, "acceptImageSharing onPreExecute exit");
            }
            @Override
            protected Void doInBackground(Void... params) {
                Logger.d(TAG,
                        "acceptImageSharing doInBackground entry, mRichCallStatus: "
                                + mRichCallStatus);
                if (mRichCallStatus == RichCallStatus.CONNECTED) {
                    mImageSharingStatus = ImageSharingStatus.INCOMING;
                    acceptImageSharingSession();
                } else {
                    mImageSharingStatus = ImageSharingStatus.INCOMING_BLOCKING;
                    mPendingActions.add(new Runnable() {
                        @Override
                        public void run() {
                            acceptImageSharingSession();
                        }
                    });
                }
                Logger.d(TAG,
                        "acceptImageSharing doInBackground exit");
                return null;
            }
        } .execute();
    }
    /**
     * Round double.
     *
     * @param val the val
     * @param precision the precision
     * @return the double
     */
    private double roundDouble(double val, int precision) {
        double result = 0;
        double factor = Math.pow(10, precision);
        if (factor != 0) {
            result = Math.floor(val * factor + 0.5) / factor;
        } else {
            Logger.d(TAG, "roundDouble factor is 0");
        }
        return result;
    }
    /**
     * Decline image sharing.
     */
    private void declineImageSharing() {
        Logger.v(TAG, "declineImageSharing() entry");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                boolean success = false;
                try {
                    Logger.d(TAG,
                            "addIncomingImageSharingListener mRichCallStatus is: "
                                    + mRichCallStatus);
                    if (mRichCallStatus == RichCallStatus.CONNECTED) {
                        mImageSharingStatus = ImageSharingStatus.DECLINE;
                        mIncomingImageSharingSession = mImageSharingApi
                                .getImageSharing(mIncomingSessionId);
                        Logger.w(
                                TAG,
                                "declineImageSharing mIncomingImageSharingSession: "
                                        + mIncomingImageSharingSession);
                        if (mIncomingImageSharingSession != null) {
                            mImageSharingApi
                                    .removeEventListener(mIncomingSessionEventListener);
                            mIncomingImageSharingSession
                                    .rejectInvitation();
                            success = true;
                        }
                    } else {
                        mPendingActions.add(new Runnable() {
                            @Override
                            public void run() {
                                declineImageSharing();
                            }
                        });
                    }
                } catch (RcsServiceException e) {
                    e.printStackTrace();
                }
                if (!success) {
                    reset();
                }
                Logger.d(TAG, "declineImageSharing() success: "
                        + success);
            }
        });
        Logger.v(TAG, "declineImageSharing() exit");
    }

    /**
     * Image sharing invitation receiver.
     */
    private class ImageSharingInvitationReceiver extends
            BroadcastReceiver {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "ImageSharingInvitationReceiver";

        /* (non-Javadoc)
         * @see android.content.BroadcastReceiver#onReceive
         * (android.content.Context, android.content.Intent)
         */
        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.v(TAG, "onReceive entry, with intent: " + intent);
            if (intent != null) {
                String action = intent.getAction();
                if (ImageSharingIntent.ACTION_NEW_INVITATION
                        .equals(action)) {
                    handleImageSharingInvitation(context, intent);
                } else if (IMAGE_SHARING_START_ACTION.equals(action)) {
                    mOutGoingImageName = intent
                            .getStringExtra(IMAGE_NAME);
                    Logger.v(TAG, "onReceive mOutGoingImageName is: "
                            + mOutGoingImageName);
                    if (mCallScreenHost != null) {
                        Utils.setInImageSharing(true);
                        prepareStartImageSharing(mOutGoingImageName);
                    } else {
                        Logger.d(TAG,
                                "onReceive mCallScreenHost is null");
                    }
                } else if (IMAGE_SHARING_REPIC_ACTION.equals(action)) {
                    int selectType = intent
                            .getIntExtra(
                                    SELECT_TYPE,
                                    RichcallProxyActivity.REQUEST_CODE_GALLERY);
                    Activity activity = null;
                    if (mCallScreenHost != null) {
                        activity = mCallScreenHost
                                .getCallScreenActivity();
                        if (activity != null) {
                            Logger.d(TAG, "show Repic dialog");
                            mRepickDialog = new RepickDialog();
                            mRepickDialog.saveSelection(selectType);
                            mRepickDialog.show(
                                    activity.getFragmentManager(),
                                    RepickDialog.TAG);
                        }
                        return;
                    }
                } else if (IMAGE_SHARING_WARN_ACTION.equals(action)) {
                    String fileName = intent
                            .getStringExtra(IMAGE_NAME);
                    if (fileName == null) {
                        return;
                    }
                    Activity activity = null;
                    if (mCallScreenHost != null) {
                        activity = mCallScreenHost
                                .getCallScreenActivity();
                        if (activity != null) {
                            Logger.d(TAG, "show Warnings dialog");
                            mInviteWarningDialog = new WarningDialog();
                            mInviteWarningDialog.saveParameters(
                                    fileName, entry_initiate);
                            mInviteWarningDialog.show(
                                    activity.getFragmentManager(),
                                    WarningDialog.TAG);
                        }
                        return;
                    }
                } else {
                    Logger.w(TAG, "onReceive unknown action");
                }
            } else {
                Logger.w(TAG, "onReceive intent is null");
            }
            Logger.v(TAG, "onReceive exit");
        }
        /**
         * Handle image sharing invitation.
         *
         * @param context the context
         * @param intent the intent
         */
        private void handleImageSharingInvitation(Context context,
                Intent intent) {
            Logger.v(TAG, "handleImageSharingInvitation entry");
            MediatekFactory.setApplicationContext(context);
            mIncomingContact = intent
                    .getStringExtra(ImageSharingIntent.EXTRA_CONTACT);
            mIncomingContactDisplayname = intent
                    .getStringExtra(ImageSharingIntent.EXTRA_DISPLAY_NAME);
            mIncomingSessionId = intent
                    .getStringExtra(ImageSharingIntent.EXTRA_SHARING_ID);
            mIncomingImageName = intent
                    .getStringExtra(ImageSharingIntent.EXTRA_FILENAME);
            mIncomingImageSize = intent.getLongExtra(
                    ImageSharingIntent.EXTRA_FILESIZE, 0);
            mIncomingThumbnail = intent
                    .getByteArrayExtra(RichcallProxyActivity.THUMBNAIL_TYPE);
            long availabeSize = com.mediatek.rcse.service.Utils
                    .getFreeStorageSize();
            Logger.v(TAG,
                    "handleImageSharingInvitation mIncomingContact: "
                            + mIncomingContact
                            + " mIncomingImageName: "
                            + mIncomingImageName
                            + " mIncomingImageSize: "
                            + mIncomingImageSize
                            + " mIncomingContactDisplayname: "
                            + mIncomingContactDisplayname
                            + " availabeSize: " + availabeSize);
            boolean noStorage = false;
            if (mIncomingImageSize > availabeSize) {
                noStorage = true;
                final String toastText;
                if (availabeSize == -1) {
                    toastText = context
                            .getString(R.string.rcse_no_external_storage_for_image_share);
                } else {
                    toastText = context
                            .getString(R.string.rcse_no_enough_storage_for_image_share);
                }
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(mContext, toastText,
                                Toast.LENGTH_LONG).show();
                    }
                });
            }
            boolean supportedCapability = false;
            supportedCapability = getCapability(mIncomingContact);
            if (noStorage || !supportedCapability) {
                try {
                    ImageSharing sharingSession = mImageSharingApi
                            .getImageSharing(mIncomingSessionId);
                    if (sharingSession != null) {
                        try {
                            sharingSession.rejectInvitation();
                            Logger.w(TAG,
                                    "handleImageSharingInvitation reject session");
                        } catch (RcsServiceException e) {
                            e.printStackTrace();
                        }
                    }
                } catch (RcsServiceException e) {
                    e.printStackTrace();
                }
                return;
            }
            mIncomingSessionEventListener = new IncomingSessionEventListener();
            mCallScreenDialogManager.showInvitationConfirmDialog();
            Logger.v(TAG, "handleImageSharingInvitation exit");
        }
    }

    /**
     * Adds the incoming image sharing listener.
     */
    private void addIncomingImageSharingListener() {
        Logger.d(TAG,
                "addIncomingImageSharingListener() entry, mRichCallStatus: "
                        + mRichCallStatus);
        boolean success = false;
        try {
            if (mRichCallStatus == RichCallStatus.CONNECTED) {
                mImageSharingStatus = ImageSharingStatus.INCOMING;
                mIncomingImageSharingSession = mImageSharingApi
                        .getImageSharing(mIncomingSessionId);
                Logger.w(TAG,
                        "addIncomingImageSharingListener() mIncomingImageSharingSession: "
                                + mIncomingImageSharingSession);
                if (mIncomingImageSharingSession != null) {
                    mImageSharingApi
                            .addEventListener(mIncomingSessionEventListener);
                    success = true;
                }
            } else {
                mImageSharingStatus = ImageSharingStatus.INCOMING_BLOCKING;
                mPendingActions.add(new Runnable() {
                    @Override
                    public void run() {
                        addIncomingImageSharingListener();
                    }
                });
            }
        } catch (RcsServiceException e) {
            e.printStackTrace();
        }
        if (!success) {
            reset();
        }
        Logger.d(TAG,
                "addIncomingImageSharingListener() exit, success: "
                        + success);
    }
    /**
     * Accept image sharing session.
     */
    private void acceptImageSharingSession() {
        Logger.d(
                TAG,
                "acceptImageSharingSession() entry, mIncomingImageSharingSession is "
                        + (mIncomingImageSharingSession != null ? mIncomingImageSharingSession
                                .toString() : "null"));
        boolean success = false;
        try {
            if (mIncomingImageSharingSession != null) {
                long receivedFileSize = mIncomingImageSharingSession
                        .getFileSize();
                long currentStorageSize = Utils.getFreeStorageSize();
                Logger.d(TAG, "receivedFileSize = "
                        + receivedFileSize + "/currentStorageSize = "
                        + currentStorageSize);
                if (currentStorageSize > 0) {
                    if (receivedFileSize <= currentStorageSize) {
                        mIncomingImageSharingSession
                                .acceptInvitation();
                        Utils.setInImageSharing(true);
                        success = true;
                    } else {
                        mIncomingImageSharingSession
                                .rejectInvitation();
                        Utils.setInImageSharing(false);
                        success = false;
                        new Handler(Looper.getMainLooper())
                                .post(new Runnable() {
                                    @Override
                                    public void run() {
                                        Context context = ApiManager
                                                .getInstance()
                                                .getContext();
                                        String strToast =
               context.getString(R.string.rcse_no_enough_storage_for_image_share);
                                        Toast.makeText(context,
                                                strToast,
                                                Toast.LENGTH_LONG)
                                                .show();
                                    }
                                });
                    }
                } else {
                    mIncomingImageSharingSession.rejectInvitation();
                    Utils.setInImageSharing(false);
                    success = false;
                    new Handler(Looper.getMainLooper())
                            .post(new Runnable() {
                                @Override
                                public void run() {
                                    Context context = ApiManager
                                            .getInstance()
                                            .getContext();
                                    String strToast = context
                .getString(R.string.rcse_no_external_storage_for_image_share);
                                    Toast.makeText(context, strToast,
                                            Toast.LENGTH_LONG).show();
                                }
                            });
                }
            }
        } catch (RcsServiceException e) {
            e.printStackTrace();
        }
        if (!success) {
            reset();
        }
        Logger.d(TAG, "acceptImageSharingSession() exit, success: "
                + success);
    }
    /**
     * Show accepted toast.
     */
    private void showAcceptedToast() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.v(TAG, "showAcceptedToast entry");
                String message = mContext.getResources().getString(
                        R.string.image_sharing_accepted);
                showToast(message);
                Logger.v(TAG, "showAcceptedToast exit");
            }
        });
    }
    /**
     * Already on going.
     *
     * @return true, if successful
     */
    private boolean alreadyOnGoing() {
        Logger.v(TAG, "alreadyOnGoing entry");
        if (Utils.isInVideoSharing()
                && RCSeInCallUIExtension.getInstance()
                        .getVideoSharePlugIn().getStatus() == Constants.LIVE_OUT) {
            Logger.v(TAG, "alreadyOnGoing isInImageSharing true");
            mCallScreenDialogManager.alreadyOnGoingVideoShare();
            return false;
        } else if (Utils.isInVideoSharing()
                && RCSeInCallUIExtension.getInstance()
                        .getVideoSharePlugIn().getStatus() == Constants.LIVE_IN) {
            Logger.v(TAG,
                    "alreadyOnGoing isInImageSharing Incmoing file true");
            return false;
            //mVideoFinished = false;
            //startVideoShare(true);
        } else {
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    String message = mContext
                            .getResources()
                            .getString(
                                    R.string.image_sharing_already_ongoing);
                    showToast(message);
                }
            });
        }
        Logger.v(TAG, "alreadyOnGoing exit");
        return true;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#getCurrentState()
     */
    @Override
    public int getCurrentState() {
        Logger.d(TAG, "getCurrentState entry");
        int state = Constants.SHARE_FILE_STATE_IDLE;
        if (mImageSharingStatus == ImageSharingStatus.UNKNOWN
                || mImageSharingStatus == ImageSharingStatus.DECLINE) {
            state = Constants.SHARE_FILE_STATE_IDLE;
        } else if (mImageSharingStatus == ImageSharingStatus.COMPLETE) {
            state = Constants.SHARE_FILE_STATE_DISPLAYING;
        } else if (mImageSharingStatus == ImageSharingStatus.INCOMING) {
            state = Constants.SHARE_FILE_INCOMING;
        } else if (mImageSharingStatus == ImageSharingStatus.OUTGOING) {
            state = Constants.SHARE_FILE_OUTGOING;
        }
        Logger.d(TAG, "getCurrentState exit with state: " + state);
        return state;
    }

    /**
     * The Class CallScreenDialogManager.
     */
    private final class CallScreenDialogManager {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "CallScreenDialogManager";
        /**
         * The Constant CAMERA_POSITION.
         */
        private static final int CAMERA_POSITION = 0;
        /**
         * The Constant GALLERY_POSITION.
         */
        private static final int GALLERY_POSITION = 1;
        /**
         * The Constant CHAT_POSITION.
         */
        private static final int CHAT_POSITION = 2;
        /**
         * The m dialogs.
         */
        private CopyOnWriteArraySet<CallScreenDialog> mDialogs =
                new CopyOnWriteArraySet<CallScreenDialog>();
        /**
         * The m saved dialogs.
         */
        private List<CallScreenDialog> mSavedDialogs =
                new ArrayList<CallScreenDialog>();
        /**
         * The view.
         */
        private View mView = null;

        /**
         * Save alert dialogs.
         */
        public void saveAlertDialogs() {
            Logger.d(TAG, "saveAlertDialogs entry");
            mSavedDialogs.clear();
            mSavedDialogs.addAll(mDialogs);
            Logger.d(TAG, "saveAlertDialogs exit" + mSavedDialogs);
        }
        /**
         * Show alert dialogs.
         */
        public void showAlertDialogs() {
            Logger.d(TAG, "showAlertDialogs entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mView != null) {
                        ViewGroup parentViewGroup = (ViewGroup) mView
                                .getParent();
                        if (parentViewGroup != null) {
                            Logger.d(TAG,
                                    "Parent View Group not null");
                            parentViewGroup.removeAllViews();
                        }
                    }
                    if (mSavedDialogs.size() > 1) {
                        mDialogs.add(mSavedDialogs.get(mSavedDialogs
                                .size() - 1));
                        mSavedDialogs.get(mSavedDialogs.size() - 1)
                                .show();
                    } else if (mSavedDialogs.size() == 1) {
                        mDialogs.add(mSavedDialogs.get(0));
                        mSavedDialogs.get(0).show();
                    }
                    mSavedDialogs.clear();
                }
            });
            Logger.d(TAG, "showAlertDialogs exit");
        }
        /**
         * Clear saved dialogs.
         */
        public void clearSavedDialogs() {
            Logger.v(TAG,
                    "clearSavedDialogs() entry mSavedDialogs size is "
                            + mSavedDialogs.size());
            mSavedDialogs.clear();
            Logger.v(TAG,
                    "clearSavedDialogs() entry mSavedDialogs size is "
                            + mSavedDialogs.size());
        }
        /**
         * Dismiss other dialog.
         */
        public void dismissOtherDialog() {
            Logger.i(TAG, "dismissOtherDialog entry()");
            if (mCompressDialog != null) {
                mCompressDialog.dismiss();
            }
            if (mReceiveWarningDialog != null) {
                mReceiveWarningDialog.dismiss();
            }
            if (mInviteWarningDialog != null) {
                mInviteWarningDialog.dismiss();
            }
            if (mRepickDialog != null) {
                mRepickDialog.dismiss();
            }
            mCompressDialog = null;
            mReceiveWarningDialog = null;
            mInviteWarningDialog = null;
            mRepickDialog = null;
            Logger.i(TAG, "dismissOtherDialog exit()");
        }
        /**
         * Already on going video share.
         */
        public void alreadyOnGoingVideoShare() {
            Logger.v(TAG, "alreadyOnGoingVideoShare entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    String message = mContext
                            .getResources()
                            .getString(
                                    R.string.video_share_ongoing_image_start);
                    Logger.v(TAG, "alreadyOnGoingVideoShare msg = "
                            + message);
                    //createAndShowAlertDialog(message,mAlreadyOnGoingShareListener );
                    showTerminateConfirmDialog(message);
                }
            });
            Logger.v(TAG, "alreadyOnGoing exit");
        }

        /**
         * The m already on going share listener.
         */
        DialogInterface.OnClickListener mAlreadyOnGoingShareListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Logger.v(TAG, "onClick(), which = " + which);
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    Logger.v(TAG, "stop image share first");
                    RCSeInCallUIExtension.getInstance()
                            .getmShareFilePlugIn().stop();
                    // mVideoFinished = false;
                    // startVideoShare(true);
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    Logger.v(TAG, "Do nothing");
                    break;
                default:
                    Logger.v(TAG, "Unknown option");
                    break;
                }
                dismissDialog();
                clearSavedDialogs();
            }
        };

        /**
         * Show invitation confirm dialog.
         */
        public void showInvitationConfirmDialog() {
            Logger.d(TAG, "showInvitationConfirmDialog entry");
            addIncomingImageSharingListener();
            if (mCallScreenHost == null) {
                Logger.d(TAG,
                        "showInvitationConfirmDialog mCallScreenHost is null");
                return;
            }
            Activity activity = mCallScreenHost
                    .getCallScreenActivity();
            if (activity == null) {
                Logger.d(TAG,
                        "showInvitationConfirmDialog getCallScreenActivity is null");
                return;
            }
            final CallScreenDialog callScreenDialog = new CallScreenDialog(
                    activity);
            String filesize = com.mediatek.rcse.service.Utils
                    .formatFileSizeToString(
                            mContext,
                            mIncomingImageSize,
                            com.mediatek.rcse.service.Utils.SIZE_TYPE_TOTAL_SIZE);
            mView = LayoutInflater.from(mContext).inflate(
                    R.layout.image_invitation_content, null);
            TextView imageName = (TextView) mView
                    .findViewById(R.id.image_name);
            TextView imageSize = (TextView) mView
                    .findViewById(R.id.image_size);
            ImageView imageType = (ImageView) mView
                    .findViewById(R.id.image_type);
            if (mIncomingImageName == null) {
                Logger.d(TAG,
                        "showInvitationConfirmDialog mIncomingImageName is null");
                return;
            }
            String mimeType = MediaFile
                    .getMimeTypeForFile(mIncomingImageName);
            if (mimeType == null) {
                mimeType = MimeTypeMap
                        .getSingleton()
                        .getMimeTypeFromExtension(
                                com.mediatek.rcse.service.Utils
                                        .getFileExtension(mIncomingImageName));
            }
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(com.mediatek.rcse.service.Utils
                    .getFileNameUri(mIncomingImageName), mimeType);
            PackageManager packageManager = mContext
                    .getPackageManager();
            List<ResolveInfo> list = packageManager
                    .queryIntentActivities(intent,
                            PackageManager.MATCH_DEFAULT_ONLY);
            if (mIncomingThumbnail == null) {
                int size = list.size();
                if (size > 0) {
                    imageType
                            .setImageDrawable(list.get(0).activityInfo
                                    .loadIcon(packageManager));
                }
            } else {
                imageType.setImageBitmap(BitmapFactory
                        .decodeByteArray(mIncomingThumbnail, 0,
                                mIncomingThumbnail.length));
            }
            imageName.setTextColor(Color.BLACK);
            imageName.setText(mIncomingImageName);
            imageSize.setText(filesize);
            imageSize.setTextColor(Color.BLACK);
            callScreenDialog.setContent(mView);
            String contactName = (mIncomingContactDisplayname != null) ? mIncomingContactDisplayname
                    : mIncomingContact;
            callScreenDialog.setTitle(mContext
                    .getString(R.string.file_type_image)
                    + mContext.getString(R.string.file_transfer_from)
                    + contactName);
            if (mIncomingImageSize < mMaxImageSharingSize
                    || mMaxImageSharingSize == 0) {
                callScreenDialog
                        .setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                        if(!hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {           
                                            if(!mCallScreenHost.getCallScreenActivity().shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                                                Logger.d(TAG,
                                                        "WRITE_EXTERNAL_STORAGE never ask again");
                                                Toast.makeText(mContext, "Permission denied.You can change them in Settings->Apps.", Toast.LENGTH_LONG).show();
                                            } else {
                                                Logger.d(TAG,
                                                        "WRITE_EXTERNAL_STORAGE denied");
                                                Toast.makeText(mContext, "Can't accept file due to no storage permissions",
                                                    Toast.LENGTH_LONG).show();
                                            }
                                         } else {  
                                        onUserAcceptImageSharing(
                                                mContext,
                                                mIncomingImageName,
                                                mIncomingImageSize);
                                    }
                                        
                                    }
                                });
                callScreenDialog
                        .setNegativeButton(
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                        declineImageSharing();
                                    }
                                });
            } else {
                declineImageSharing();
                String notifymessage = mContext.getString(
                        R.string.file_size_notification, contactName);
                callScreenDialog.setMessage(notifymessage);
                callScreenDialog
                        .setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
            }
            //Vibrate
            Utils.vibrate(mContext, Utils.MIN_VIBRATING_TIME);
            callScreenDialog.setCancelable(false);
            clearSavedDialogs();
            callScreenDialog.show();
            mDialogs.add(callScreenDialog);
            saveAlertDialogs();
            Logger.d(TAG, "showInvitationConfirmDialog exit");
        }
        /**
         * Show time out dialog.
         */
        public void showTimeOutDialog() {
            Logger.d(TAG, "showTimeOutDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    Activity activity = null;
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.image_sharing_invitation_time_out));
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showTimeOutDialog mCallScreenHost is null or" +
                                " getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTimeOutDialog exit");
        }
        /**
         * Show init fail dialog.
         */
        public void showInitFailDialog() {
            Logger.d(TAG, "showInitFailDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.image_sharing_terminated_due_to_network));
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showInitFailDialog mCallScreenHost is null or" +
                                " getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showInitFailDialog exit");
        }
        /**
         * Show rejected dialog.
         */
        public void showRejectedDialog() {
            Logger.d(TAG, "showRejectedDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.rejected_share_image));
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showRejectedDialog mCallScreenHost is null" +
                                " or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showRejectedDialog exit");
        }
        /**
         * Show terminated dialog.
         */
        public void showTerminatedDialog() {
            Logger.d(TAG, "showTerminatedDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.terminated_share_image));
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showTerminatedDialog mCallScreenHost is null" +
                                " or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTerminatedDialog exit");
        }
        
        public void removeViewsWhenFinished()
        {
        	if(mDisplayArea != null )
        	{
        		mDisplayArea.setVisibility(View.INVISIBLE);
        	}
        }
        /**
         * Show terminate confirm dialog.
         */
        public void showTerminateConfirmDialog() {
            Logger.d(TAG, "showTerminateConfirmDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        Logger.d(TAG,
                                                "showTerminateConfirmDialog onClick");
                                        finishImageSharing();
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setNegativeButton(
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.terminate_share_image));
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showTerminateConfirmDialog mCallScreenHost is null or" +
                                " getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTerminateConfirmDialog exit");
        }
        /**
         * Show terminate confirm dialog.
         *
         * @param msg the msg
         */
        public void showTerminateConfirmDialog(final String msg) {
            Logger.d(TAG, "showTerminateConfirmDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        Logger.v(TAG,
                                                "stop video share first");
                                        RCSeInCallUIExtension
                                                .getInstance()
                                                .getVideoSharePlugIn()
                                                .stop();
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setNegativeButton(
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        //callScreenDialog.setMessage(mContext
                        //        .getString(R.string.terminate_share_image));
                        callScreenDialog.setMessage(msg);
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showTerminateConfirmDialog mCallScreenHost is" +
                                " null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTerminateConfirmDialog exit");
        }
        /**
         * Show no storage dialog.
         */
        public void showNoStorageDialog() {
            Logger.d(TAG, "showNoStorageDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null
                            && mCallScreenHost
                                    .getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost
                                .getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(
                                activity);
                        callScreenDialog
                                .setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(
                                            DialogInterface dialog,
                                            int which) {
                                        callScreenDialog
                                                .dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog
                                .setCancelListener(new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(
                                            DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext
                                .getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.no_storage));
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(
                                TAG,
                                "showNoStorageDialog mCallScreenHost" +
                                " is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showNoStorageDialog exit");
        }
        /**
         * Dismiss dialogs.
         */
        public void dismissDialogs() {
            Logger.d(TAG, "dismissDialogs entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    for (CallScreenDialog dialog : mDialogs) {
                        dialog.dismissDialog();
                    }
                    mDialogs.clear();
                }
            });
            Logger.d(TAG, "dismissDialogs exit");
        }

        /**
         * The Class SelectionItem.
         */
        private class SelectionItem {
            /**
             * The text.
             */
            public String text;
        }

        /**
         * Show select image dialog.
         */
        public void showSelectImageDialog() {
            final List<SelectionItem> list = new ArrayList<SelectionItem>();
            SelectionItem cameraItem = new SelectionItem();
            cameraItem.text = mContext
                    .getString(R.string.camera_item);
            list.add(cameraItem);
            SelectionItem galleryItem = new SelectionItem();
            galleryItem.text = mContext
                    .getString(R.string.gallery_item);
            list.add(galleryItem);
            SelectionItem joynChatItem = new SelectionItem();
            joynChatItem.text = mContext
                    .getString(R.string.startchat);
            list.add(joynChatItem);
            if (mCallScreenHost != null) {
                final Context context = mCallScreenHost
                        .getCallScreenActivity();
                if (context == null) {
                    Logger.d(TAG,
                            "showSelectImageDialog getCallScreenActivity is null");
                    return;
                }
                final LayoutInflater dialogInflater = (LayoutInflater) context
                        .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                final ArrayAdapter<SelectionItem> adapter = new ArrayAdapter<SelectionItem>(
                        context, R.layout.image_selection_layout,
                        list) {
                    @Override
                    public View getView(int position,
                            final View convertView, ViewGroup parent) {
                        View itemView = convertView;
                        if (itemView == null) {
                            itemView = dialogInflater
                                    .inflate(
                                            mContext.getResources()
                                                    .getLayout(
                                                            R.layout.image_selection_layout),
                                            parent, false);
                        }
                        final TextView text = (TextView) itemView
                                .findViewById(R.id.item_text);
                        SelectionItem item = getItem(position);
                        text.setText(item.text);
                        text.setTextColor(Color.BLACK);
                        return itemView;
                    }
                };
                final CallScreenDialog selectionDialog = new CallScreenDialog(
                        context);
                selectionDialog.setTitle(mContext
                        .getString(R.string.share_image));
                selectionDialog.setSingleChoiceItems(adapter, 0,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(
                                    DialogInterface dialog, int which) {
                                dialog.dismiss();
                                mDialogs.remove(selectionDialog);
                                clearSavedDialogs();
                                if (CAMERA_POSITION == which) {
                                    startCamera();
                                } else if (GALLERY_POSITION == which) {
                                    startGallery();
                                } else if (CHAT_POSITION == which) {
                                    startJoynChat();
                                }
                            }
                        });
                selectionDialog.setCancelable(true);
                selectionDialog
                        .setCancelListener(new DialogInterface.OnCancelListener() {
                            @Override
                            public void onCancel(
                                    DialogInterface dialog) {
                                mDialogs.remove(selectionDialog);
                                clearSavedDialogs();
                            }
                        });
                for (CallScreenDialog callDialog : mDialogs) {
                    callDialog.dismissDialog();
                    mDialogs.remove(callDialog);
                }
                clearSavedDialogs();
                selectionDialog.show();
                mDialogs.add(selectionDialog);
                saveAlertDialogs();
            } else {
                Logger.d(TAG,
                        "showSelectImageDialog mCallScreenHost is null");
            }
        }
    }

    public boolean hasPermission(final String permission) {        
        Activity activity = mCallScreenHost
                .getCallScreenActivity();
        final int permissionState = activity.checkSelfPermission(permission);
        Logger.v("ImagesharingPlugin", "hasPermission() : permission = " + permission + " permissionState = " + permissionState);
        return permissionState == PackageManager.PERMISSION_GRANTED;

    }

    public static boolean createDirectory(String path) {
        File dir = new File(path); 
        if (!dir.exists()) {
            if (!dir.mkdirs()) {
                return false; 
            }
        }
        return true;
    }   

    /**
     * Start camera.
     */
    private void startCamera() {
        RCSE_FILE_DIR = getSDCardPath(mCallScreenHost
                .getCallScreenActivity()) + SLASH  + "Joyn";
        RCSE_TEMP_FILE_DIR = RCSE_FILE_DIR + SLASH + "temp";        
        Logger.d(TAG, "startCamera entry RCSE_FILE_DIR " + RCSE_FILE_DIR + "RCSE_TEMP_FILE_DIR" + RCSE_TEMP_FILE_DIR);
        if (createDirectory(RCSE_FILE_DIR)) {
            Logger.w(TAG, "Create rcse dir success");
        } else {
            Logger.w(TAG, "Create rcse dir failed");           
        }
        if (createDirectory(RCSE_TEMP_FILE_DIR)) {
            Logger.w(TAG, "Create rcse tmp dir success");
        } else {
            Logger.w(TAG, "Create rcse tmp dir failed");
        }
        mCameraTempFileUri = Uri.fromFile(new File(
                RCSE_TEMP_FILE_DIR, RCSE_TEMP_FILE_NAME_HEADER
                        + String.valueOf(System.currentTimeMillis())
                        + JPEG_SUFFIX));
        
        Intent intent = new Intent(
                RichcallProxyActivity.IMAGE_SHARING_SELECTION);
        intent.putExtra(RichcallProxyActivity.SELECT_TYPE,
                RichcallProxyActivity.SELECT_TYPE_CAMERA);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        intent.putExtra("imageUri", mCameraTempFileUri);
        mContext.startActivity(intent);
    }
    
    public static String getSDCardPath(Context c) {
        Logger.v(TAG, "getSDCardPath");
        File sdDir = null;
        String sdStatus = Environment.getExternalStorageState();
        if (TextUtils.isEmpty(sdStatus)) {
            return c.getFilesDir().getAbsolutePath();
        }
        boolean sdCardExist = sdStatus.equals(android.os.Environment.MEDIA_MOUNTED);
        if (sdCardExist) {
            sdDir = Environment.getExternalStorageDirectory();
            Logger.v(TAG, "getSDCardPath sdCardExist path" + sdDir.toString());
            return sdDir.toString();
        }
        String path = c.getFilesDir().getAbsolutePath();
        Logger.v(TAG, "getSDCardPath sdCardExist no exist path" + path);
        return path;
    }
    /**
     * Start gallery.
     */
    private void startGallery() {
        Intent intent = new Intent(
                RichcallProxyActivity.IMAGE_SHARING_SELECTION);
        intent.putExtra(RichcallProxyActivity.SELECT_TYPE,
                RichcallProxyActivity.SELECT_TYPE_GALLERY);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        mContext.startActivity(intent);
    }
    /**
     * Start joyn chat.
     */
    private void startJoynChat() {
        Intent imIntent = null;
        if (MediatekFactory.getApplicationContext() == null) {
            MediatekFactory.setApplicationContext(mContext);
        }
        if (RcsSettings.getInstance() == null) {
            RcsSettings.createInstance(mContext);
        }
        int integratedMode = RcsSettings.getInstance()
                .getMessagingUx();
        Logger.d(TAG, "startJoynChat entry, mode is "
                + integratedMode);
        imIntent = new Intent(
                PluginApiManager.RcseAction.PROXY_ACTION);
        imIntent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);
        imIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        imIntent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER,
                getOperatorAccount(sNumber));
        if (integratedMode == 0) {
            imIntent.putExtra("isjoyn", true);
        } else {
            imIntent.putExtra("isjoyn", false);
        }
        imIntent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME,
                PluginUtils.getNameByNumber(sNumber));
        mContext.startActivity(imIntent);
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#getState()
     */
    @Override
    public int getState() {
        Logger.d(TAG, "getState entry");
        int state = Constants.SHARE_FILE_STATE_IDLE;
        if (mImageSharingStatus == ImageSharingStatus.UNKNOWN
                || mImageSharingStatus == ImageSharingStatus.DECLINE) {
            state = Constants.SHARE_FILE_STATE_IDLE;
        } else if (mImageSharingStatus == ImageSharingStatus.COMPLETE) {
            state = Constants.SHARE_FILE_STATE_DISPLAYING;
        } else {
            state = Constants.SHARE_FILE_STATE_TRANSFERING;
        }
        Logger.d(TAG, "getState exit with state: " + state);
        return state;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#getStatus()
     */
    @Override
    public int getStatus() {
        return 0; //not to be used
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#stop()
     */
    @Override
    public void stop() {
        Logger.v(TAG, "stop() entry");
        finishImageSharing();
        if (mCallScreenDialogManager != null) {
            mCallScreenDialogManager.dismissDialogs();
            mCallScreenDialogManager.dismissOtherDialog();
        } else {
            Logger.d(TAG,
                    "dismissDialog mCallScreenDialogManager is null");
        }
        Logger.v(TAG, "stop() exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#start(java.lang.String)
     */
    @Override
    public void start(String number) {
        super.start(number);
        Logger.d(TAG, "start() entry, number: " + number
                + " mImageSharingStatus: " + mImageSharingStatus);
        boolean inImageShare = mImageSharingStatus != ImageSharingStatus.UNKNOWN
                && mImageSharingStatus != ImageSharingStatus.COMPLETE
                && mImageSharingStatus != ImageSharingStatus.DECLINE;
        if (Utils.isInVideoSharing() || inImageShare) {
            if (alreadyOnGoing()) {
                return;
            }
        }
        if (!isImageShareSupported(number)) {
            imageShareNotSupported();
            return;
        }
        if (mCallScreenHost != null) {
            mDisplayArea = mCallScreenHost.requestAreaForDisplay();
        } else {
            Logger.d(TAG, "start mCallScreenHost is null");
        }
        mCallScreenDialogManager.showSelectImageDialog();
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#saveAlertDialogs()
     */
    @Override
    public void saveAlertDialogs() {
        Logger.v(TAG, "saveAlertDialogs() entry");
        mCallScreenDialogManager.saveAlertDialogs();
        Logger.v(TAG, "saveAlertDialogs() exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#showAlertDialogs()
     */
    @Override
    public void showAlertDialogs() {
        Logger.v(TAG, "showAlertDialogs() entry");
        mCallScreenDialogManager.showAlertDialogs();
        Logger.v(TAG, "showAlertDialogs() exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#dismissDialog()
     */
    @Override
    public boolean dismissDialog() {
        Logger.v(TAG, "dismissDialog()");
        if (mCallScreenDialogManager != null) {
            mCallScreenDialogManager.dismissDialogs();
        }
        return false;
    }
    /**
     * Prepare start image sharing.
     *
     * @param imageName the image name
     */
    private void prepareStartImageSharing(final String imageName) {
        Logger.d(TAG, "prepareStartImageSharing()");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                String compressedFileName = null;
                Logger.d(TAG, "Need to compress");
                if (AppSettings.getInstance()
                        .isEnabledCompressingImageFromDB()) {
                    Logger.d(TAG,
                            "Compress the image, do not hit the user");
                    compressedFileName = com.mediatek.rcse.service.Utils
                            .compressImage(imageName);
                    Logger.d(TAG, "The compressed image file name = "
                            + compressedFileName);
                    final String nameString = compressedFileName;
                    mMainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            startImageSharing(nameString);
                        }
                    });
                } else {
                    boolean remind = AppSettings.getInstance()
                            .restoreRemindCompressFlag();
                    Logger.d(TAG,
                            "Do hit the user to select whether to compress. remind = "
                                    + remind);
                    if (remind) {
                        mCompressDialog = new CompressDialog();
                        mCompressDialog.setOrigFileName(imageName);
                        Activity activity = null;
                        if (mCallScreenHost != null) {
                            activity = mCallScreenHost
                                    .getCallScreenActivity();
                            if (activity != null) {
                                Logger.d(TAG, "show compress dialog");
                                FragmentTransaction ft = activity.getFragmentManager().beginTransaction();
                                ft.add(mCompressDialog, CompressDialog.TAG);
                                ft.commitAllowingStateLoss();
                                /*mCompressDialog.show(
                                        activity.getFragmentManager(),
                                        CompressDialog.TAG);*/
                            }
                            return;
                        }
                    } else {
                        Logger.d(TAG, "Do not compress image");
                        compressedFileName = imageName;
                        Logger.d(TAG,
                                "The compressed image file name = "
                                        + compressedFileName);
                        final String nameString = compressedFileName;
                        mMainHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                startImageSharing(nameString);
                            }
                        });
                    }
                }
            }
        });
    }

    /**
     * A dialog to hint the user that a picture will be compressed before
     * sending.
     */
    public class CompressDialog extends DialogFragment implements
            DialogInterface.OnClickListener,
            DialogInterface.OnCancelListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "CompressDialog";
        /**
         * The m check not remind.
         */
        private CheckBox mCheckNotRemind = null;
        /**
         * The m activity.
         */
        private Activity mActivity = null;
        /**
         * The m origin file name.
         */
        private String mOriginFileName;

        /**
         * Constructor.
         */
        public CompressDialog() {
            Logger.d(TAG, "CompressDialog()");
        }
        /**
         * Sets the orig file name.
         *
         * @param originFileName the new orig file name
         */
        public void setOrigFileName(String originFileName) {
            Logger.d(TAG, "setOrigFileName():" + originFileName);
            mOriginFileName = originFileName;
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onSaveInstanceState(android.os.Bundle)
         */
        @Override
        public void onSaveInstanceState(Bundle saveState) {
            // Override this method to workaround a google issue happen when API
            // level > 11
            Logger.d(TAG, "onSaveInstanceState()");
            saveState.putString(TAG, TAG);
           // super.onSaveInstanceState(saveState);
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCreateDialog(android.os.Bundle)
         */
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Logger.d(TAG, "onCreateDialog");
            final AlertDialog alertDialog = new AlertDialog.Builder(
                    getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .create();
            mActivity = getActivity();
            if (mContext != null && mActivity != null) {
                alertDialog.setTitle(mContext
                        .getString(R.string.compress_image_title));
                LayoutInflater inflater = LayoutInflater
                        .from(mActivity.getApplicationContext());
                View customView = inflater.inflate(
                        mContext.getResources().getLayout(
                                R.layout.warning_dialog), null);
                mCheckNotRemind = (CheckBox) customView
                        .findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
                TextView contentView = (TextView) customView
                        .findViewById(R.id.warning_content);
                contentView.setText(mContext
                        .getString(R.string.compress_image_content));
                TextView remindAgainView = (TextView) customView
                        .findViewById(R.id.remind_content);
                remindAgainView
                        .setText(mContext
                                .getString(R.string.file_size_remind_contents));
                alertDialog
                        .setButton(
                                DialogInterface.BUTTON_POSITIVE,
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                this);
                alertDialog
                        .setButton(
                                DialogInterface.BUTTON_NEGATIVE,
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                this);
                alertDialog
                        .setIconAttribute(android.R.attr.alertDialogIcon);
            } else {
                Logger.e(TAG, "activity is null in CompressDialog");
            }
            return alertDialog;
        }
        /* (non-Javadoc)
         * @see android.content.DialogInterface.
         * OnClickListener#onClick(android.content.DialogInterface, int)
         */
        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            AppSettings.getInstance().saveRemindCompressFlag(
                    mCheckNotRemind.isChecked());
            if (which == DialogInterface.BUTTON_POSITIVE) {
                handleOk();
            } else {
                Logger.d(TAG, "the user cancle compressing image");
                handleCancel();
            }
            this.dismissAllowingStateLoss();
            mCompressDialog = null;
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCancel(android.content.DialogInterface)
         */
        @Override
        public void onCancel(DialogInterface dialog) {
            Logger.i(TAG, "onCancel{} in CompressDialog entry");
            reset();
            Logger.i(TAG, "onCancel{} in CompressDialog exit");
        }
        /**
         * Handle ok.
         */
        private void handleOk() {
            Logger.i(TAG, "handleOk()");
            new AsyncTask<Void, Void, String>() {
                @Override
                protected String doInBackground(Void... params) {
                    if (mCheckNotRemind.isChecked()) {
                        Logger.d(TAG,
                                "the user enable compressing image and not remind again");
                        AppSettings.getInstance()
                                .setCompressingImage(true);
                    }
                    return com.mediatek.rcse.service.Utils
                            .compressImage(mOriginFileName);
                }
                @Override
                protected void onPostExecute(String result) {
                    Logger.v(TAG, "onPostExecute(),result = "
                            + result);
                    if (result != null) {
                        mOutGoingImageName = result;
                        startImageSharing(result);
                    } else {
                        reset();
                    }
                }
            } .execute();
        }
        /**
         * Handle cancel.
         */
        private void handleCancel() {
            Logger.i(TAG, "handleCancel()");
            mOutGoingImageName = mOriginFileName;
            startImageSharing(mOriginFileName);
        }
    }

    /**
     * The Class WarningDialog.
     */
    public class WarningDialog extends DialogFragment implements
            DialogInterface.OnClickListener,
            DialogInterface.OnCancelListener {
        /**
         * The Constant TAG.
         */
        static final String TAG = "WarningDialog";
        /**
         * The m check remind.
         */
        private CheckBox mCheckRemind = null;
        /**
         * The m activity.
         */
        Activity mActivity = null;
        /**
         * The m file name.
         */
        String mFileName = null;
        /**
         * The entry_type.
         */
        int mEntryType = entry_initiate;

        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCreateDialog(android.os.Bundle)
         */
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(),
                    AlertDialog.THEME_HOLO_LIGHT).create();
            if (mContext != null && mActivity != null) {
                alertDialog.setTitle(mContext
                        .getString(R.string.file_size_warning));
                LayoutInflater inflater = LayoutInflater
                        .from(mActivity.getApplicationContext());
                View customView = inflater.inflate(
                        mContext.getResources().getLayout(
                                R.layout.warning_dialog), null);
                mCheckRemind = (CheckBox) customView
                        .findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
                TextView contentView = (TextView) customView
                        .findViewById(R.id.warning_content);
                contentView
                        .setText(mContext
                                .getString(R.string.file_size_warning_contents));
                TextView remindAgainView = (TextView) customView
                        .findViewById(R.id.remind_content);
                remindAgainView
                        .setText(mContext
                                .getString(R.string.file_size_remind_contents));
                alertDialog
                        .setButton(
                                DialogInterface.BUTTON_POSITIVE,
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                this);
                alertDialog
                        .setButton(
                                DialogInterface.BUTTON_NEGATIVE,
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                this);
                alertDialog
                        .setIconAttribute(android.R.attr.alertDialogIcon);
            } else {
                Logger.e(TAG, "activity is null in WarningDialog");
            }
            return alertDialog;
        }
        /* (non-Javadoc)
         * @see android.content.DialogInterface.
         * OnClickListener#onClick(android.content.DialogInterface, int)
         */
        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mCheckRemind != null) {
                    boolean isCheck = mCheckRemind.isChecked();
                    Logger.w(TAG, "WarningDialog onClick ischeck"
                            + isCheck);
                    AppSettings.getInstance()
                            .saveRemindWarningLargeImageFlag(isCheck);
                }
                if (mFileName != null) {
                    if (mEntryType == entry_terminated) {
                        acceptImageSharing();
                    } else if (mEntryType == entry_initiate) {
                        mOutGoingImageName = mFileName;
                        Logger.v(TAG,
                                "onwarning Dialog mOutGoingImageName is: "
                                        + mOutGoingImageName);
                        if (mCallScreenHost != null) {
                            Utils.setInImageSharing(true);
                            prepareStartImageSharing(mOutGoingImageName);
                        } else {
                            Logger.d(TAG,
                                    "onReceive mCallScreenHost is null");
                        }
                    }
                }
            } else {
                declineImageSharing();
            }
            this.dismissAllowingStateLoss();
            if (mReceiveWarningDialog != null) {
                mReceiveWarningDialog = null;
            } else if (mInviteWarningDialog != null) {
                mInviteWarningDialog = null;
            }
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCancel(android.content.DialogInterface)
         */
        @Override
        public void onCancel(DialogInterface dialog) {
            Logger.i(TAG, "onCancel{} in WarningDialog entry");
            reset();
            Logger.i(TAG, "onCancel{} in WarningDialog exit");
        }
        /**
         * Save parameters.
         *
         * @param fileName the file name
         * @param entryType the entry_type
         */
        public void saveParameters(String fileName, int entryType) {
            this.mFileName = fileName;
            this.mEntryType = entryType;
        }
    }

    /**
     * The Class RepickDialog.
     */
    public class RepickDialog extends DialogFragment implements
            DialogInterface.OnClickListener,
            DialogInterface.OnCancelListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "RepickDialog";
        /**
         * The m request code.
         */
        private int mRequestCode = 0;
        /**
         * The m activity.
         */
        Activity mActivity = null;

        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCreateDialog(android.os.Bundle)
         */
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(),
                    AlertDialog.THEME_HOLO_LIGHT).create();
            if (mContext != null && mActivity != null) {
                alertDialog
                        .setIconAttribute(android.R.attr.alertDialogIcon);
                alertDialog.setTitle(mContext
                        .getString(R.string.large_file_repick_title));
                alertDialog
                        .setMessage(mContext
                                .getString(R.string.large_file_repick_message));
                alertDialog
                        .setButton(
                                DialogInterface.BUTTON_POSITIVE,
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                this);
                alertDialog
                        .setButton(
                                DialogInterface.BUTTON_NEGATIVE,
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                this);
            } else {
                Logger.e(TAG, "activity is null in RepickDialog");
            }
            return alertDialog;
        }
        /* (non-Javadoc)
         * @see android.content.DialogInterface.
         * OnClickListener#onClick(android.content.DialogInterface, int)
         */
        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mRequestCode == RichcallProxyActivity.REQUEST_CODE_CAMERA) {
                    startCamera();
                } else if (mRequestCode == RichcallProxyActivity.REQUEST_CODE_GALLERY) {
                    startGallery();
                }
            }
            this.dismissAllowingStateLoss();
            mRepickDialog = null;
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCancel(android.content.DialogInterface)
         */
        @Override
        public void onCancel(DialogInterface dialog) {
            Logger.i(TAG, "onCancel{} in RepickDialog entry");
            reset();
            Logger.i(TAG, "onCancel{} in RepickDialog exit");
        }
        /**
         * Save selection.
         *
         * @param which the which
         */
        public void saveSelection(int which) {
            this.mRequestCode = which;
        }
    }
}
