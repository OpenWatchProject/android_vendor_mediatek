/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.orangelabs.rcs.provider.sharing;

import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.service.richcall.RichcallService;
import com.orangelabs.rcs.core.ims.service.richcall.image.ImageTransferSession;
import com.orangelabs.rcs.provider.DeleteTask;
import com.orangelabs.rcs.provider.LocalContentResolver;
import com.orangelabs.rcs.service.api.ImageSharingServiceImpl;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;

import java.util.Set;

public class ImageSharingDeleteTask extends DeleteTask.GroupedByContactId {

    private static final Logger sLogger = Logger.getLogger(ImageSharingDeleteTask.class.getName());

    private final ImageSharingServiceImpl mImageSharingService;

    private final RichcallService mRichcallService;

    /**
     * Deletion of all image sharing.
     * 
     * @param imageSharingService the image service impl
     * @param richcallService the rich call service
     * @param contentResolver the local content resolver
     */
    public ImageSharingDeleteTask(ImageSharingServiceImpl imageSharingService,
            RichcallService richcallService, LocalContentResolver contentResolver) {
        super(contentResolver, ImageSharingData.CONTENT_URI, ImageSharingData.KEY_SHARING_ID,
                ImageSharingData.KEY_CONTACT, (String) null);
        mImageSharingService = imageSharingService;
        mRichcallService = richcallService;
    }

    /**
     * Deletion of a specific image sharing.
     * 
     * @param imageSharingService the image service impl
     * @param richcallService the rich call service
     * @param contentResolver the local content resolver
     * @param sharingId the sharing id
     */
    public ImageSharingDeleteTask(ImageSharingServiceImpl imageSharingService,
            RichcallService richcallService, LocalContentResolver contentResolver, String sharingId) {
        super(contentResolver, ImageSharingData.CONTENT_URI, ImageSharingData.KEY_SHARING_ID,
                ImageSharingData.KEY_CONTACT, null, sharingId);
        mImageSharingService = imageSharingService;
        mRichcallService = richcallService;
    }

    /**
     * Deletion of all image sharing with a specific contact.
     *
     * @param imageSharingService the image service impl
     * @param richcallService the rich call service
     * @param contentResolver the local content resolver
     * @param contact the contact id
     */
    public ImageSharingDeleteTask(ImageSharingServiceImpl imageSharingService,
            RichcallService richcallService, LocalContentResolver contentResolver, ContactId contact) {
        super(contentResolver, ImageSharingData.CONTENT_URI, ImageSharingData.KEY_SHARING_ID,
                ImageSharingData.KEY_CONTACT, contact);
        mImageSharingService = imageSharingService;
        mRichcallService = richcallService;
    }

    @Override
    protected void onRowDelete(ContactId contact, String sharingId) throws PayloadException {
        ImageTransferSession session = mRichcallService.getImageTransferSession(sharingId);
        if (session == null) {
            mImageSharingService.ensureFileCopyIsDeletedIfExisting(sharingId);
            mImageSharingService.removeImageSharing(sharingId);
            return;

        }
        try {
            session.deleteSession();
        } catch (NetworkException e) {
            /*
             * If network is lost during a delete operation the remaining part of the delete
             * operation (delete from persistent storage) can succeed to 100% anyway since delete
             * can be executed anyway while no network connectivity is present and still succeed.
             */
            if (sLogger.isActivated()) {
                sLogger.debug(e.getMessage());
            }
        }
        mImageSharingService.ensureFileCopyIsDeletedIfExisting(sharingId);
        mImageSharingService.removeImageSharing(sharingId);
    }

    @Override
    protected void onCompleted(ContactId contact, Set<String> deletedIds) {
        mImageSharingService.broadcastDeleted(contact, deletedIds);
    }
}
