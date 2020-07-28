package com.mediatek.gallery3d.adapter;

import android.app.Activity;
import android.content.Context;

import com.android.gallery3d.app.GalleryAppImpl;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.ExtFields;
import com.mediatek.gallerybasic.base.IFieldDefinition;
import com.mediatek.gallerybasic.base.IFilter;
import com.mediatek.gallerybasic.base.LayerManager;
import com.mediatek.gallerybasic.base.MediaCenter;
import com.mediatek.gallerybasic.base.MediaFilter;
import com.mediatek.gallerybasic.base.MediaMember;
import com.mediatek.gallerybasic.base.PlayEngine;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.dynamic.LayerManagerImpl;
import com.mediatek.gallerybasic.dynamic.PhotoPlayEngine;
import com.mediatek.gallerybasic.gl.GLIdleExecuter;
import com.mediatek.gallerybasic.platform.PlatformHelper;
import com.mediatek.gallerybasic.util.ExtFieldsUtils;
import com.mediatek.gallerybasic.util.Utils;

import java.util.ArrayList;

public class PhotoPlayFacade {
    private final static String TAG = "MtkGallery2/PhotoPlayFacade";

    private static MediaCenter sMediaCenter;
    private static boolean sHasIntialized = false;

    public static void initialize(GalleryAppImpl context, int microThumbnailSize,
                                  int thumbnailSize, int highQualitySize) {
        if (sHasIntialized) {
            return;
        }
        Utils.initialize(context);
        ThumbType.MICRO.setTargetSize(microThumbnailSize);
        ThumbType.MIDDLE.setTargetSize(thumbnailSize);
        ThumbType.HIGHQUALITY.setTargetSize(highQualitySize);
        PlatformHelper.setPlatform(new PlatformImpl(context));

        // register filter
        registerFilters();

        // register field definition
        registerFieldDefinitions(context);
        sHasIntialized = true;
    }

    public static MediaCenter getMediaCenter() {
        if (sMediaCenter == null) {
            sMediaCenter = new MediaCenter();
        }
        return sMediaCenter;
    }

    public static int getFullScreenPlayCount() {
        return Config.FULL_PLAY_COUNT;
    }

    public static int getThumbPlayCount() {
        return Config.THUMB_PLAY_COUNT;
    }

    public static int getFullScreenTotalCount() {
        return Config.FULL_TOTAL_COUNT;
    }

    public static int getThumbTotalCount() {
        return Config.THUMB_TOTAL_COUNT;
    }

    /**
     * Get the filters from feature manager, and register.
     */
    private static void registerFilters() {
        IFilter[] filters =
                (IFilter[]) FeatureManager.getInstance().getImplement(IFilter.class);
        for (IFilter filter : filters) {
            MediaFilter.registerFilter(filter);
        }
    }

    /**
     * Get the field definitions from feature manager, and register.
     * @param context The current application context
     */
    private static void registerFieldDefinitions(Context context) {
        IFieldDefinition[] fieldDefs =
                (IFieldDefinition[]) FeatureManager.getInstance().getImplement(
                        IFieldDefinition.class);
        ExtFields.initColumns(context);
        // add video rotation for fancy and video thumbnail placeholder. @{
        ExtFields.addVideoFiled(ExtFieldsUtils.VIDEO_ROTATION_FIELD);
        // @}
        ExtFields.registerFieldDefinition(fieldDefs);
    }

    public static void registerMedias(Context context, GLIdleExecuter exe) {
        Log.d(TAG, "<registerMedias> Context = " + context + ", GLIdleExecuter = " + exe
                + " Resources = " + context.getResources());
        MediaCenter mc = getMediaCenter();

        MediaMember[] members = null;
        if (exe != null) {
            members =
                    (MediaMember[]) FeatureManager.getInstance().getImplement(
                            MediaMember.class, context, exe, context.getResources());
        } else {
            members =
                    (MediaMember[]) FeatureManager.getInstance().getImplement(
                            MediaMember.class, context, context.getResources());
        }

        // put MediaMember to ArrayList
        ArrayList<MediaMember> memberList = new ArrayList<MediaMember>();
        for (MediaMember member : members) {
            memberList.add(member);
            member.setMediaCenter(getMediaCenter());
        }
        memberList.add(new MediaMember(context));

        // register
        mc.registerMedias(memberList);
    }

    public static void registerWidgetMedias(Context context) {
        Log.d(TAG, "<registerWidgetMedias> context = " + context);
        MediaCenter mc = getMediaCenter();
        if (mc.getMemberCount() <= 0) {
            registerMedias(context, null);
        }
    }

    public static PlayEngine createPlayEngineForFullScreen() {
        return new PhotoPlayEngine(getMediaCenter(), Config.FULL_TOTAL_COUNT,
                Config.FULL_PLAY_COUNT, Config.FULL_WORK_THREAD_NUM,
                ThumbType.MIDDLE);
    }

    public static LayerManager createLayerMananger(Activity activity) {
        return new LayerManagerImpl(activity, getMediaCenter());
    }

    private PhotoPlayFacade() {
    }
}
