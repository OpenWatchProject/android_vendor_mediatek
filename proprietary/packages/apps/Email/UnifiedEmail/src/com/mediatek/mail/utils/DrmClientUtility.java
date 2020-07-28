package com.mediatek.mail.utils;

import android.content.Context;
import android.net.Uri;

import java.util.ArrayList;
import java.util.List;

import com.android.emailcommon.utility.FeatureOption;
import com.android.mail.providers.Attachment;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.content.ContentValues;

public class DrmClientUtility {
    /** M: drm single instance*/
    private static DrmManagerClient sDrmClientInstance = null;
    private static final String LOG_TAG = LogTag.getLogTag();
    /**
     * M: get Drm client single instance.
     * Even it is a single instance, need release it when not use it any more.
     */
    private static synchronized DrmManagerClient getDrmClientInstance(
            Context context) {
        if (sDrmClientInstance == null) {
            LogUtils.w(LogUtils.TAG, "getDrmClientInstance, create new instance");
            sDrmClientInstance = new DrmManagerClient(context);
        }
        return sDrmClientInstance;
    }
/**
     * M: Check the given uri is DRM file.
     * @param context
     * @param uri, file uri
     */
    private static boolean isDrmFile(Context context, Uri uri) {
        DrmManagerClient drmClient = getDrmClientInstance(context);
        ContentValues cv = null;
        try{
            cv = drmClient.getMetadata(uri);
        }catch(IllegalArgumentException e){
            LogUtils.e(LOG_TAG,"isDrmFile, getMetadata fail with uri : "+uri,e);
        }
        if(cv == null){
            return false;
        }
        final String key = "isdrm";
        Integer isDrm = (cv.containsKey(key) ? cv.getAsInteger(key): null);
        if (isDrm != null && isDrm > 0) {
            return true;
        }
        return false;
    }
    /**
     * M: Get the original MimeType of DRM file.
     * @param context the context to create DrmClient
     * @param uri the content URI
     * @return The original MimeType of DRM file if it was DRM file, otherwise return null.
     */
    public static String getDRMOriginalMimeType(Context context, Uri uri) {
        String type = null;
        if (FeatureOption.MTK_DRM_APP) {
            DrmManagerClient drmClient = getDrmClientInstance(context);
            if (isDrmFile(context, uri)) {
                type = drmClient.getOriginalMimeType(uri);
                LogUtils.d(LogUtils.TAG, "The original type of [%s] is %s.", uri, type);
            }
        }
        return type;
    }
    public static boolean fileCanBeForwarded(DrmManagerClient client, Uri uri) {
        int right = client.checkRightsStatus(uri, DrmStore.Action.TRANSFER);
        if (right == DrmStore.RightsStatus.RIGHTS_VALID) {
            return true;
        }
        return false;
    }

    /**
     * M: check if the attachment is drm protected.
     */
    public static boolean isDrmProtected(Context context, Uri uri) {
        boolean checkResult = false;
        if (FeatureOption.MTK_DRM_APP && uri != null) {
            DrmManagerClient drmClient = getDrmClientInstance(context);
            // Only normal file and SD type drm file can be forwarded
            if (isDrmFile(context, uri) && (!fileCanBeForwarded(drmClient,uri))) {
                LogUtils.w(LogUtils.TAG, "Not add attachment [%s], for Drm protected.", uri);
                checkResult = true;
            }
        }
        return checkResult;
    }

    /**
     * M: filter drm attachment for the given source attachments.
     */
    public static List<Attachment> filterDrmAttachments(Context context, List<Attachment> attachments) {
        if (!FeatureOption.MTK_DRM_APP || attachments == null || attachments.size() == 0) {
            return attachments;
        }
        List<Attachment> filterAttachments = new ArrayList<Attachment>();
        for (Attachment att : attachments) {
            if (isDrmProtected(context, att.contentUri)) {
                LogUtils.w(LogUtils.TAG,
                        "Not add attachment [%s], for Drm protected.",
                        att.contentUri);
            } else {
                filterAttachments.add(att);
            }
        }
        return filterAttachments;
    }
}
