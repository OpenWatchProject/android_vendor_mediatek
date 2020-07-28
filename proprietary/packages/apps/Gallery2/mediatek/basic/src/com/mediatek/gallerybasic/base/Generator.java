package com.mediatek.gallerybasic.base;

import android.os.StatFs;

import com.mediatek.gallerybasic.util.Log;

import java.io.File;

public abstract class Generator {
    private static final String TAG = "MtkGallery2/AbstractVideoGenerator";

    public static final int VTYPE_THUMB = 0;
    public static final int VTYPE_SHARE = 1;
    public static final int VTYPE_SHARE_GIF = 2;

    public static final int STATE_NEED_GENERATE = 0;
    public static final int STATE_GENERATING = 1;
    public static final int STATE_GENERATED = 2;
    public static final int STATE_GENERATED_FAIL = 3;

    protected static final int DEFAULT_THUMBNAIL_SIZE = 224;

    protected static final int GENERATE_OK = 0;
    protected static final int GENERATE_CANCEL = 1;
    protected static final int GENERATE_ERROR = 2;

    public int[] videoState = {STATE_NEED_GENERATE, STATE_NEED_GENERATE, STATE_NEED_GENERATE};
    public String[] videoPath = {null, null, null};

    /**
     * (for sub classes' extension)
     * Generate a certain type of video for an media item. <br/>
     * This invocation may be blocked in its implementation and presumably
     * time-consuming. To make sure the generating process can be canceled as
     * soon as possible, shouldCancel() should be judged before every
     * significant and time-consuming generating step and if "shouldCancel",
     * give up the following generating routine and return GENERATE_CANCEL.
     *
     * @param item
     *            the media item for which to generate video
     * @param videoType
     *            the type of video to generate
     * @return GENERATE_OK, GENERATE_CANCEL, GENERATE_ERROR
     */
    protected abstract int generate(MediaData item, int videoType, final String targetFilePath);

    /**
     * (for sub classes' overriding)
     * generate video in an asynchronous style <br/>
     * note: now only accept VTYPE_THUMB for parameter videoType
     */
    protected int generateAsync(MediaData item, int videoType) {
        if (videoType != VTYPE_THUMB) {
            throw new UnsupportedOperationException(
                    "now only support syncGenerate for thumbnail play");
        }
        GeneratorCoordinator.requestThumbnail(this, item);
        return GENERATE_OK;
    }

    /**
     * (for sub classes' extension)
     * Called when canceling a certain type of video generating for an media
     * item. <br/>
     * This is typically useful if the implementation of generate() blocks the
     * generating thread (e.g. by Object.wait()), and in that case,
     * onCancelRequested() should notify (wake up) the thread as soon.
     *
     * @param item
     *            the media item for which to generate video
     * @param videoType
     *            the type of video to generate
     */
    public abstract void onCancelRequested(MediaData item, int videoType);

    /**
     * Generally called during generate(), to judge whether the caller intended
     * to cancel that generating pass. It is suggested to call shouldCancel()
     * before every significant and time-consuming generating step
     *
     */
    protected boolean shouldCancel() {
        return Thread.currentThread().isInterrupted();
    }

    /**
     * (for sub classes' extension)
     * whether the videoType of video needs (re-)generating by considering the
     * mediaItem's latest status
     *
     * @param mediaItem
     *            the media item for which to generate video
     * @param videoType
     *            the type of video to generate
     * @return true if needs (re-)generating
     */
    protected boolean needGenerating(MediaData mediaItem, int videoType) {
        // better use cache file to indicate if the dyn thumb has been generated
        videoPath[videoType] = getVideoThumbnailPathFromOriginalFilePath(mediaItem, videoType);
        // re-generate video/Gif every time when sharing it
        if (videoType != VTYPE_THUMB) {
            return true;
        }
        File dynThumbFile = new File(videoPath[videoType]);
        if (dynThumbFile.exists()) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * (invoking point to caller)
     * it would generate video in a blocking way (generate and wait, for share),
     * or in an asynchronous style (for thumbnail play)
     */
    public String generateVideo(MediaData mediaData, int videoType) {
        String filePath = mediaData.filePath;
        if ((filePath == null) || (mediaData.width == 0) || (mediaData.height == 0)) {
            return null;
        }

        if (!needGenerating(mediaData, videoType)) {
            // already generated, now mTargetPath = generated path
            return videoPath[videoType];
        }

        if (videoType == VTYPE_THUMB) {
            generateAsync(mediaData, VTYPE_THUMB);
            return null;
        }

        int genRes = generateAndWait(mediaData, videoType);  // generate to videoPath
        return (genRes == GENERATE_OK ? videoPath[videoType] : null);
    }

    // TODO (obsolete) in case of needGenerating, return true in needGenerating()
    // and therefore this method is obsolete by design
    public void prepareToRegenerate(MediaData item) {
        videoState[VTYPE_THUMB] = STATE_NEED_GENERATE;
        videoState[VTYPE_SHARE] = STATE_NEED_GENERATE;
        videoState[VTYPE_SHARE_GIF] = STATE_NEED_GENERATE;
        //videoPath[VTYPE_THUMB] = videoPath[VTYPE_SHARE] = null;
//        VideoThumbnailHelper.deleteThumbnailFile(item, VTYPE_THUMB);
//        VideoThumbnailHelper.deleteThumbnailFile(item, VTYPE_SHARE);
//        VideoThumbnailHelper.deleteThumbnailFile(item, VTYPE_SHARE_GIF);
    }

    /**
     * generate video in a blocking way <br/>
     * designed to be called by Generator base class & GeneratorCoordinator <br/>
     * it's unsafe to be called in other places
     */
    public int generateAndWait(MediaData item, int videoType) {
        //TODO cancel or not
        String dirPath = item.filePath.substring(0, item.filePath.lastIndexOf('/'));
        File parentDir = new File(dirPath);
        if (!parentDir.exists()) {
            Log.e(TAG, "media file folder: " + dirPath
                    + " is not invalid! folder deleted or sdcard unmounted?");
            return GENERATE_ERROR;
        }
        if (!isStorageSafeForGenerating(dirPath)) { // storage not enough
            return GENERATE_ERROR;
        }

        String filePath = videoPath[videoType];
        dirPath = filePath.substring(0, filePath.lastIndexOf('/'));
        File dir = new File(dirPath);
        if (!dir.exists() && !dir.mkdir()) {
            Log.e(TAG, "exception when creating cache container!");
            return GENERATE_ERROR;
        }

        // TODO cancel or not
        File unCompleteDynThumb = new File(videoPath[videoType] + SUFFIX_TMP);
        int result = generate(item, videoType, videoPath[videoType] + SUFFIX_TMP);
        Log.v(TAG, "generate result: " + result);

        if (result == GENERATE_CANCEL) {
            return GENERATE_CANCEL;
        }

        boolean recrifiedResult = false;
        if (result == GENERATE_OK) {
            if (unCompleteDynThumb.exists()) {
                recrifiedResult = unCompleteDynThumb
                        .renameTo(new File(videoPath[videoType]));
            }
        }
        Log.v(TAG, "recrified generate result: " + result);

        if (recrifiedResult) {
            return GENERATE_OK;
        } else {
            if (unCompleteDynThumb.exists()) {
                unCompleteDynThumb.delete();
            }
            return GENERATE_ERROR;
        }
    }


    // TODO new class the below
    private static final int MIN_STORAGE_SPACE = 3 * 1024 * 1024;
    private static final String[] DYNAMIC_CACHE_FILE_POSTFIX = {".dthumb", ".mp4", ".gif"};
    private static final String SUFFIX_TMP = ".tmp";

    public static boolean isStorageSafeForGenerating(String dirPath) {
        try {
            StatFs stat = new StatFs(dirPath);
            long spaceLeft = (long) (stat.getAvailableBlocks())
                    * stat.getBlockSize();
            Log.v(TAG, "storage available in this volume is: " + spaceLeft);
            if (spaceLeft < MIN_STORAGE_SPACE) {
                return false;
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            Log.d(TAG, "may sdcard unmounted (or switched) for this moment");
            return false;
        }
        return true;
    }

    // out file name looks like videoxxx.dthumb
    private static String getVideoThumbnailPathFromOriginalFilePath(
            MediaData mediaData, int videoType) {
        String originalFilePath = mediaData.filePath;
        StringBuilder res = null;
        int i = originalFilePath.lastIndexOf("/");
        if (i == -1) {
            res = new StringBuilder(".dthumb/").append(
                    originalFilePath.substring(i + 1).hashCode()).append(
                    mediaData.dateModifiedInSec).append(
                    DYNAMIC_CACHE_FILE_POSTFIX[videoType]);
        } else {
            // i-1 can be -1. risk?
            res = new StringBuilder(originalFilePath.substring(0, i + 1))
                    .append(".dthumb/").append(
                            originalFilePath.substring(i + 1).hashCode())
                    .append(mediaData.dateModifiedInSec).append(
                            DYNAMIC_CACHE_FILE_POSTFIX[videoType]);
        }
        return res.toString();
    }
}
