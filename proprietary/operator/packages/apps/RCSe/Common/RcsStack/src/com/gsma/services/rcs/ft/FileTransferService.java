/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
package com.gsma.services.rcs.ft;

import com.gsma.services.rcs.ICoreServiceWrapper;
import com.gsma.services.rcs.Logger;
import com.gsma.services.rcs.RcsGenericException;
import com.gsma.services.rcs.RcsIllegalArgumentException;
import com.gsma.services.rcs.RcsPermissionDeniedException;
import com.gsma.services.rcs.RcsPersistentStorageException;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.RcsServiceControl;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;
import com.gsma.services.rcs.RcsServiceNotAvailableException;
import com.gsma.services.rcs.chat.IChatService;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer.Disposition;

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.IBinder;
import android.os.IInterface;
import android.os.RemoteException;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.WeakHashMap;

/**
 * This class offers the main entry point to transfer files and to
 * receive files. Several applications may connect/disconnect to the API.
 *
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 *
 * @author Jean-Marc AUFFRET
 */
public class FileTransferService extends RcsService {

    /**
     * API
     */
    private IFileTransferService mApi;

    private final Map<OneToOneFileTransferListener, WeakReference<IOneToOneFileTransferListener>> mOneToOneFileTransferListeners = new WeakHashMap<>();

    private final Map<GroupFileTransferListener, WeakReference<IGroupFileTransferListener>> mGroupFileTransferListeners = new WeakHashMap<>();

    private static boolean sApiCompatible = false;

    private Context mContext = null;

    public static final String TAG = "FileTransferService";

    public static final ComponentName DEFAULT_IMPL_COMPONENT =
            new ComponentName("com.orangelabs.rcs", "com.orangelabs.rcs.service.RcsCoreService");

    /**
     * Constructor
     *
     * @param ctx Application context
     * @param listener Service listener
     */
    public FileTransferService(Context ctx, RcsServiceListener listener) {
        super(ctx, listener);
        mContext = ctx;
    }

    /**
     * Connects to the API
     *
     * @throws RcsPermissionDeniedException
     */
    public final void connect() {
        Logger.i(TAG, "connect entry");
        /*if (!sApiCompatible) {
            try {
                sApiCompatible = mRcsServiceControl.isCompatible(this);
                if (!sApiCompatible) {
                    throw new RcsPermissionDeniedException(
                            "The TAPI client version of the file transfer service is not compatible with the TAPI service implementation version on this device!");
                }
            } catch (RcsServiceException e) {
                throw new RcsPermissionDeniedException(
                        "The compatibility of TAPI client version with the TAPI service implementation version of this device cannot be checked for the file transfer service!",
                        e);
            }
        }
        Intent serviceIntent = new Intent(IFileTransferService.class.getName());
        serviceIntent.setPackage(RcsServiceControl.RCS_STACK_PACKAGENAME);
        mCtx.bindService(serviceIntent, apiConnection, 0);*/

        Intent intent = new Intent();
        ComponentName cmp = DEFAULT_IMPL_COMPONENT;
        intent.setComponent(cmp);
        intent.setAction(IFileTransferService.class.getName());
        mContext.bindService(intent, apiConnection, 0);
    }

    /**
     * Disconnects from the API
     */
    public void disconnect() {
        try {
            Logger.i(TAG, "disconnect entry");
            mCtx.unbindService(apiConnection);
        } catch (IllegalArgumentException e) {
            // Nothing to do
            Logger.i(TAG, "disconnect exception");
        }
    }

    /**
     * Set API interface
     *
     * @param api API interface
     * @hide
     */
    protected void setApi(IInterface api) {
        super.setApi(api);
        mApi = (IFileTransferService) api;
    }

    /**
     * Service connection
     */
    private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            ICoreServiceWrapper mCoreServiceWrapperBinder = ICoreServiceWrapper.Stub.asInterface(service);
            IBinder binder = null;
            try {
                binder = mCoreServiceWrapperBinder.getFileTransferServiceBinder();
            } catch (RemoteException e1) {
                e1.printStackTrace();
            }
            setApi(IFileTransferService.Stub.asInterface(binder));
            if (mListener != null) {
                mListener.onServiceConnected();
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            Logger.i(TAG, "onServiceDisconnected entry");
            setApi(null);
            if (mListener == null) {
                return;
            }
            ReasonCode reasonCode = ReasonCode.CONNECTION_LOST;
            try {
                if (!mRcsServiceControl.isActivated()) {
                    reasonCode = ReasonCode.SERVICE_DISABLED;
                }
            } catch (RcsServiceException e) {
                // Do nothing
            }
            mListener.onServiceDisconnected(reasonCode);
        }
    };

    /**
     * Granting temporary read Uri permission from client to stack service if it is a content URI
     *
     * @param file Uri of file to grant permission
     */
    private void tryToGrantUriPermissionToStackServices(Uri file) {
        if (!ContentResolver.SCHEME_CONTENT.equals(file.getScheme())) {
            return;
        }
        Intent fileTransferServiceIntent = new Intent(IFileTransferService.class.getName());
        List<ResolveInfo> stackServices = mCtx.getPackageManager().queryIntentServices(
                fileTransferServiceIntent, 0);
        for (ResolveInfo stackService : stackServices) {
            mCtx.grantUriPermission(stackService.serviceInfo.packageName, file,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION);
        }
    }

    /**
     * Returns the configuration of the file transfer service
     *
     * @return FileTransferServiceConfiguration
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransferServiceConfiguration getConfiguration()
            throws RcsServiceNotAvailableException, RcsGenericException {
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new FileTransferServiceConfiguration(mApi.getConfiguration());

        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to initiate file transfer to the contact specified by the
     * contact parameter, else returns false.
     *
     * @param contact the remote contact
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public boolean isAllowedToTransferFile(ContactId contact) throws RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "isAllowedToTransferFile entry: " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return mApi.isAllowedToTransferFile(contact);

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Transfers a file to a contact. The parameter file contains the URI of the file to be
     * transferred (for a local or a remote file). The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or Tel-URI. If the
     * format of the contact is not supported an exception is thrown.
     *
     * @deprecated Use
     *             {@link #transferFile(ContactId contact, Uri file, Disposition disposition, boolean attachFileIcon)}
     *             instead.
     * @param contact the remote contact Identifier
     * @param file Uri of file to transfer
     * @param attachFileIcon File icon option. If true, the stack tries to attach fileicon. Fileicon
     *            may not be attached if file is not an image or if local or remote contact does not
     *            support fileicon.
     * @return FileTransfer
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    @Deprecated
    public FileTransfer transferFile(ContactId contact, Uri file, boolean attachFileIcon)
            throws RcsPersistentStorageException, RcsServiceNotAvailableException,
            RcsGenericException {
        return transferFile(contact, file, Disposition.ATTACH, attachFileIcon);
    }

    /**
     * Transfers a file to a contact. The parameter file contains the URI of the file to be
     * transferred (for a local or a remote file). The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or Tel-URI. If the
     * format of the contact is not supported an exception is thrown.
     *
     * @param contact the remote contact Identifier
     * @param file Uri of file to transfer
     * @param disposition File disposition
     * @param attachFileIcon File icon option. If true, the stack tries to attach fileicon. Fileicon
     *            may not be attached if file is not an image or if local or remote contact does not
     *            support fileicon.
     * @return FileTransfer
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer transferFile(ContactId contact, Uri file, Disposition disposition,
            boolean attachFileIcon) throws RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "transferFile entry: contact " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            /* Only grant permission for content Uris */
            tryToGrantUriPermissionToStackServices(file);
            IFileTransfer ftIntf = mApi.transferFile2(contact, file, disposition.toInt(),
                    attachFileIcon);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);
            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Transfers a file to a contact. The parameter file contains the URI of the file to be
     * transferred (for a local or a remote file). The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or Tel-URI. If the
     * format of the contact is not supported an exception is thrown.
     *
     * @param contact the remote contact Identifier
     * @param file Uri of file to transfer
     * @param disposition File disposition
     * @param attachFileIcon File icon option. If true, the stack tries to attach fileicon. Fileicon
     *            may not be attached if file is not an image or if local or remote contact does not
     *            support fileicon.
     * @return FileTransfer
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer transferFileLargeMode(ContactId contact, Uri file, Disposition disposition,
            boolean attachFileIcon) throws RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "transferFileLargeMode entry: contact " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            /* Only grant permission for content Uris */
            tryToGrantUriPermissionToStackServices(file);
            IFileTransfer ftIntf = mApi.transferFileLargeMode(contact, file, disposition.toInt(),
                    attachFileIcon);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);
            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Transfers a file to multiple contacts. The parameter filename contains the complete
     * path of the file to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param contacts List of multiple contacts
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listener File transfer event listener
     * @return File transfer
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public FileTransfer transferFileToMultiple(
            List<String> contacts,
            String filename,
            boolean attachFileIcon,
            Disposition disposition)
            throws RcsPermissionDeniedException, RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        return transferFileToMultiple(
                contacts, filename, attachFileIcon, disposition);
    }

    /**
     * Transfers a file to a muliple chat with an optional file icon.
     *
     * @param chatId the chat ID
     * @param file Uri of file to transfer
     * @param disposition File disposition
     * @param attachFileIcon Attach file icon option. If true, the stack tries to attach fileIcon.
     *            FileIcon may not be attached if file is not an image or if local or remote contact
     *            does not support fileIcon.
     * @return FileTransfer
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer transferFileToMultiple(List<String> contacts, Uri file, Disposition disposition,
            boolean attachFileIcon) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "transferFileToMultiple entry: contact " + contacts.size());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            /* Only grant permission for content Uris */
            tryToGrantUriPermissionToStackServices(file);
            IFileTransfer ftIntf = mApi.transferFileToMultiple(contacts, file, disposition.toInt(),
                    attachFileIcon);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);

            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Resume a file transfer.
     *
     * @param fileTransferId the file
     * @return FileTransfer
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer resumeFileTransfer(String fileTransferId) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "resumeFileTransfer entry: fileTransferId " + fileTransferId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IFileTransfer ftIntf = mApi.resumeFileTransfer(fileTransferId);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);

            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Resend a file transfer.
     *
     * @param fileTransferId the file
     * @return FileTransfer
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer resendFileTransfer(String transferId,
            String chatId, List<String> contacts, Uri file,
            boolean attachFileicon) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "resendFileTransfer entry: fileTransferId " + transferId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IFileTransfer ftIntf = mApi.resendFileToGroup(transferId,chatId,contacts,file,
                                                          attachFileicon);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);

            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Resume a group file transfer.
     *
     * @param fileTransferId File Id of the file
     * @param chatId Chat Id of the group
     * @return FileTransfer
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer resumeGroupFileTransfer(String chatId, String fileTransferId) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "resumeGroupFileTransfer entry: fileTransferId " + fileTransferId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IFileTransfer ftIntf = mApi.resumeGroupFileTransfer(chatId, fileTransferId);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);

            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }



    /**
     * Returns true if it is possible to initiate file transfer to the group chat specified by the
     * chatId parameter, else returns false.
     *
     * @param chatId the chat ID
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public boolean isAllowedToTransferFileToGroupChat(String chatId)
            throws RcsPersistentStorageException, RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "isAllowedToTransferFileToGroupChat entry: chatId " + chatId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return mApi.isAllowedToTransferFileToGroupChat(chatId);

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Transfers a file to a group chat with an optional file icon.
     *
     * @deprecated Use
     *             {@link #transferFileToGroupChat(String chatId, Uri file, Disposition disposition, boolean attachFileIcon)}
     *             instead.
     * @param chatId the chat ID
     * @param file Uri of file to transfer
     * @param attachFileIcon Attach file icon option. If true, the stack tries to attach fileIcon.
     *            FileIcon may not be attached if file is not an image or if local or remote contact
     *            does not support fileIcon.
     * @return FileTransfer
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer transferFileToGroupChat(String chatId, Uri file, boolean attachFileIcon)
            throws RcsPermissionDeniedException, RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        return transferFileToGroupChat(chatId, file, Disposition.ATTACH, attachFileIcon);
    }

    /**
     * Transfers a file to a group chat with an optional file icon.
     *
     * @param chatId the chat ID
     * @param file Uri of file to transfer
     * @param disposition File disposition
     * @param attachFileIcon Attach file icon option. If true, the stack tries to attach fileIcon.
     *            FileIcon may not be attached if file is not an image or if local or remote contact
     *            does not support fileIcon.
     * @return FileTransfer
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer transferFileToGroupChat(String chatId, Uri file, Disposition disposition,
            boolean attachFileIcon) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "transferFileToGroupChat entry: chatId " + chatId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            /* Only grant permission for content Uris */
            tryToGrantUriPermissionToStackServices(file);
            IFileTransfer ftIntf = mApi.transferFileToGroupChat2(chatId, file, disposition.toInt(),
                    attachFileIcon);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);

            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Mark a received file transfer as read (i.e. the invitation or the file has been displayed in
     * the UI).
     *
     * @param transferId the file transfer ID
     * @throws RcsServiceNotAvailableException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void markFileTransferAsRead(String transferId) throws RcsServiceNotAvailableException,
            RcsPersistentStorageException, RcsGenericException {
        Logger.i(TAG, "markFileTransferAsRead entry: transferId " + transferId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.markFileTransferAsRead(transferId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns a current file transfer from its unique ID
     *
     * @param transferId the file transfer ID
     * @return FileTransfer File transfer or null if not found
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public FileTransfer getFileTransfer(String transferId) throws RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "getFileTransfer entry: transferId " + transferId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IFileTransfer ftIntf = mApi.getFileTransfer(transferId);
            if (ftIntf != null) {
                return new FileTransfer(ftIntf);

            }
            return null;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Adds a listener on file transfer events
     *
     * @param listener One-to-one file transfer listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void addEventListener(OneToOneFileTransferListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "addEventListener O2Oentry");
        if (listener == null) {
            throw new RcsIllegalArgumentException("listener must not be null!");
        }
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IOneToOneFileTransferListener rcsListener = new OneToOneFileTransferListenerImpl(
                    listener);
            mOneToOneFileTransferListeners.put(listener, new WeakReference<>(rcsListener));
            mApi.addEventListener2(rcsListener);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Removes a listener on file transfer events
     *
     * @param listener File transfer listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void removeEventListener(OneToOneFileTransferListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "removeEventListener O2Oentry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            WeakReference<IOneToOneFileTransferListener> weakRef = mOneToOneFileTransferListeners
                    .remove(listener);
            if (weakRef == null) {
                return;
            }
            IOneToOneFileTransferListener rcsListener = weakRef.get();
            if (rcsListener != null) {
                mApi.removeEventListener2(rcsListener);
            }
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Adds a listener on group file transfer events
     *
     * @param listener Group file transfer listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void addEventListener(GroupFileTransferListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "addEventListener groupentry");
        if (listener == null) {
            throw new RcsIllegalArgumentException("listener must not be null!");
        }
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IGroupFileTransferListener rcsListener = new GroupFileTransferListenerImpl(listener);
            mGroupFileTransferListeners.put(listener, new WeakReference<>(rcsListener));
            mApi.addEventListener3(rcsListener);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Removes a listener on group file transfer events
     *
     * @param listener Group file transfer listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void removeEventListener(GroupFileTransferListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "removeEventListener groupentry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            WeakReference<IGroupFileTransferListener> weakRef = mGroupFileTransferListeners
                    .remove(listener);
            if (weakRef == null) {
                return;
            }
            IGroupFileTransferListener rcsListener = weakRef.get();
            if (rcsListener != null) {
                mApi.removeEventListener3(rcsListener);
            }
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes all one to one file transfer from history and abort/reject any associated ongoing
     * session if such exists.
     *
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteOneToOneFileTransfers() throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "deleteOneToOneFileTransfers entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteOneToOneFileTransfers();
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes all group file transfer from history and abort/reject any associated ongoing session
     * if such exists.
     *
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteGroupFileTransfers() throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "deleteGroupFileTransfers entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteGroupFileTransfers();
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes file transfer corresponding to a given one to one chat specified by contact from
     * history and abort/reject any associated ongoing session if such exists.
     *
     * @param contact the remote contact
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteOneToOneFileTransfers(ContactId contact)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "deleteOneToOneFileTransfers entry contact: " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteOneToOneFileTransfers2(contact);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes file transfer corresponding to a given group chat specified by chat id from history
     * and abort/reject any associated ongoing session if such exists.
     *
     * @param chatId the chat ID
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteGroupFileTransfers(String chatId) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "deleteGroupFileTransfers entry chatId: " + chatId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteGroupFileTransfers2(chatId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes a file transfer by its unique id from history and abort/reject any associated ongoing
     * session if such exists.
     *
     * @param transferId the file transfer ID
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteFileTransfer(String transferId) throws RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "deleteFileTransfer entry transferId: " + transferId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteFileTransfer(transferId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Disables and clears any delivery expiration for a set of file transfers regardless if the
     * delivery of them has expired already or not.
     *
     * @param transferIds the file transfer IDs
     * @throws RcsServiceNotAvailableException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void clearFileTransferDeliveryExpiration(Set<String> transferIds)
            throws RcsServiceNotAvailableException, RcsPersistentStorageException,
            RcsGenericException {
        Logger.i(TAG, "clearFileTransferDeliveryExpiration entry size: " + transferIds.size());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.clearFileTransferDeliveryExpiration(new ArrayList<String>(transferIds));
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }
}
