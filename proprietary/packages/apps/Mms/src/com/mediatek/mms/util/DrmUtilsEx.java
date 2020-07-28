package com.mediatek.mms.util;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.database.Cursor;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.provider.Telephony.Mms.Part;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;

import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.MediaModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.ui.MessageItem;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MessageResource;
import com.android.mms.util.MmsLog;

import com.mediatek.omadrm.OmaDrmUtils;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Iterator;

public class DrmUtilsEx {
    private final static String TAG = "DrmUtilsEx";
    public static boolean checkHasDrmContent(Context context, String src, Uri uri) {
        if (FeatureOption.MTK_DRM_APP) {
            if (src != null ? src.toLowerCase().endsWith(".dcf")
                                : checkUriContainsDrm(context, uri)) {
                return true;
            }
        }
        return false;
    }

    public static boolean checkHasDrmRight(Context context, boolean hasDrm, Uri uri) {
        if (FeatureOption.MTK_DRM_APP && hasDrm) {
            DrmManagerClient client = MmsApp.getApplication()
                    .getDrmManagerClient();
            FileInputStream is = null;
            try {
                is = (FileInputStream) context.getContentResolver().openInputStream(uri);
                if (is == null || is.getFD() == null) {
                    return false;
                }
                int rightsStatus = OmaDrmUtils.checkRightsStatusByFd(client, is.getFD());
                return rightsStatus == DrmStore.RightsStatus.RIGHTS_VALID;
            } catch (IOException e) {
                MmsLog.e(TAG, "checkHasDrmRight - " + e);
            } catch (IllegalStateException e) {
                MmsLog.e(TAG, "checkHasDrmRight - " + e);
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                        MmsLog.e(TAG, "checkHasDrmRight - " + e);
                    }
                }
            }
        }
        return false;
    }

    public static boolean hasDrmMediaInSlide(SlideModel slide) {
        if (!FeatureOption.MTK_DRM_APP) return false;
        for (Iterator<MediaModel> it = slide.iterator(); it.hasNext(); ) {
            MediaModel media = it.next();
            if (media.hasDrmContent()) {
                return true;
            }
        }
        return false;
    }

    /*
     * If no drm media, return true.
     */
    public static boolean hasDrmMediaRightInSlide(SlideModel slide) {
        if (!FeatureOption.MTK_DRM_APP) return false;
        for (Iterator<MediaModel> it = slide.iterator(); it.hasNext(); ) {
            MediaModel media = it.next();
            if (media.hasDrmContent() && !media.hasDrmRight()) {
                return false;
            }
        }
        return true;
    }

    public static boolean hasDrmMediaInSlideshow(SlideshowModel slideshow) {
        if (!FeatureOption.MTK_DRM_APP) return false;
        for (Iterator<SlideModel> it = slideshow.iterator(); it.hasNext(); ) {
            SlideModel slide = it.next();
            if (hasDrmMediaInSlide(slide)) {
                return true;
            }
        }
        return false;
    }

    public static boolean hasDrmMediaRightInSlideshow(SlideshowModel slideshow) {
        if (!FeatureOption.MTK_DRM_APP) return false;
        for (Iterator<SlideModel> it = slideshow.iterator(); it.hasNext(); ) {
            SlideModel slide = it.next();
            if (!hasDrmMediaRightInSlide(slide)) {
                return false;
            }
        }
        return true;
    }

    public static String getDrmContentType(String src, String extention,
            String path, String contentType) {
        if (!FeatureOption.MTK_DRM_APP) return contentType;
        if ((src != null && src.endsWith(".dcf"))
                || (extention != null && extention.equals("dcf"))) {
            return MmsApp.getApplication()
                    .getDrmManagerClient().getOriginalMimeType(path);
        }
        return contentType;
    }

    public static boolean isDrm(String name) {
        if (name != null && name.toLowerCase().endsWith(".dcf")) {
            return true;
        } else {
            return false;
        }
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            WorkingMessage msg, int drawableId) {
        if (msg == null || !msg.hasDrmMedia()) return null;
        return getDrmBitmapWithLockIcon(context, drawableId);
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            String src, int drawableId) {
        if (src == null || !src.endsWith(".dcf")) return null;
        return getDrmBitmapWithLockIcon(context, drawableId);
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            String src, Uri uri, boolean isVideo) {
        if (src == null || !src.endsWith(".dcf")) return null;
        return getDrmBitmapWithLockIcon(context, uri, isVideo);
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            MessageItem mi, int drawableId) {
        if (mi == null || !mi.hasDrmMedia()) return null;
        return getDrmBitmapWithLockIcon(context, drawableId);
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            int drawableId) {
        if (!FeatureOption.MTK_DRM_APP) return null;
        Bitmap drmBitmap = OmaDrmUtils.overlapLockIcon(
                MmsApp.getApplication().getDrmManagerClient(),
                context.getResources(), drawableId, null);
        return drmBitmap;
        }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            Bitmap bitmap) {
        if (!FeatureOption.MTK_DRM_APP) return null;
        Bitmap drmBitmap = OmaDrmUtils.overlapLockIcon(
                MmsApp.getApplication().getDrmManagerClient(),
                context.getResources(), bitmap, null);
        return drmBitmap;
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context,
            int drawableId, Uri uri) {
        MmsLog.dpi(TAG, "getDrmBitmapWithLockIcon uri = " + uri);
        Bitmap bgBitmap = BitmapFactory.decodeResource(context.getResources(),
                drawableId);
        if (!FeatureOption.MTK_DRM_APP) {
            return bgBitmap;
        }
        FileInputStream is = null;
        try {
            is = (FileInputStream) context.getContentResolver().openInputStream(uri);
            if (is == null || is.getFD() == null) {
                return bgBitmap;
            }
            Bitmap drmBitmap = OmaDrmUtils.overlapLockIcon(
                    MmsApp.getApplication().getDrmManagerClient(),
                    context, bgBitmap, is.getFD());
            return drmBitmap;
        } catch (IOException e) {
            MmsLog.e(TAG, "getDrmBitmapWithLockIcon - " + e);
        } catch (IllegalStateException e) {
            MmsLog.e(TAG, "getDrmBitmapWithLockIcon - " + e);
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    MmsLog.e(TAG, "getDrmBitmapWithLockIcon - " + e);
                }
            }
        }
        return bgBitmap;
    }

    public static Bitmap getDrmBitmapWithLockIcon(Context context, Uri uri, boolean isVideo) {
        MmsLog.dpi(TAG, "getDrmBitmapWithLockIcon uri = " + uri);
        Bitmap bitmap = null;
        if (isVideo) {
            bitmap = BitmapFactory.decodeResource(context.getResources(),
                    R.drawable.ic_missing_thumbnail_video);
        } else {
            bitmap = BitmapFactory.decodeResource(context.getResources(),
                    R.drawable.ic_missing_thumbnail_picture);
        }
        if (!FeatureOption.MTK_DRM_APP) {
            return bitmap;
        }
        FileInputStream is = null;
        try {
            is = (FileInputStream) context.getContentResolver().openInputStream(uri);
            if (is == null || is.getFD() == null) {
                return bitmap;
            }
            //overlay bitmap
            Bitmap drmBitmap = OmaDrmUtils.overlapLockIcon(
                    MmsApp.getApplication().getDrmManagerClient(),
                    context, bitmap, is.getFD());
            return drmBitmap;
        } catch (IOException e) {
            MmsLog.e(TAG, "getDrmBitmapWithLockIcon - " + e);
        } catch (IllegalStateException e) {
            MmsLog.e(TAG, "getDrmBitmapWithLockIcon - " + e);
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    MmsLog.e(TAG, "getDrmBitmapWithLockIcon - " + e);
                }
            }
        }
        return bitmap;
    }

    private static String getMmsAttachFilePath(Context context, Uri uri) {
        Cursor c = context.getContentResolver().query(uri,
                new String[] {Part._DATA}, null, null, null);
        if (c == null) {
            return null;
        }
        try {
            if (c.moveToFirst()) {
                String src = c.getString(c.getColumnIndexOrThrow(Part._DATA));
                return src;
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return null;
    }

    public static boolean checkUriContainsDrm(Context context, Uri uri) {
        if (uri != null && uri.getAuthority() != null && uri.getAuthority().equals("mms")) {
            String src = getMmsAttachFilePath(context, uri);
            if (!TextUtils.isEmpty(src) && src.endsWith(".dcf")) {
                MmsLog.d(TAG, "Media src ends with dcf, isDrmContent = true");
                return true;
            }
            MmsLog.d(TAG, "Media src does not end with dcf, isDrmContent = false");
            return false;
        }
        MmsLog.d(TAG, "checkUriContainsDrm begin!");
        DrmManagerClient client = MmsApp.getApplication().getDrmManagerClient();
        return OmaDrmUtils.isDrm(client, uri);
    }

    public static void showDrmAlertDialog(Context context) {
        Builder builder = new Builder(context);
        builder.setMessage(R.string.drm_not_support);
        builder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });
        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(false);
        dialog.show();
        if (context instanceof ComposeMessageActivity) {
            ((ComposeMessageActivity) context).setCanResponse(true);
        }
    }

    public static void showDrmLock(Context context, SlideshowModel slideshow,
            int position, ImageView drmImageVideoLock, ImageView drmAudioLock) {
        drmImageVideoLock.setVisibility(View.GONE);
        drmAudioLock.setVisibility(View.GONE);

        if (!FeatureOption.MTK_DRM_APP) return;
            Resources res = context.getResources();
            boolean hasContent = false;
            boolean hasRight = false;
        if (slideshow.get(position) != null) {
            if (slideshow.get(position).hasAudio()) {
                hasContent = slideshow.get(position).getAudio().hasDrmContent();
                hasRight = slideshow.get(position).getAudio().hasDrmRight();
                if (hasContent && hasRight) {
                    drmAudioLock.setImageDrawable(
                            res.getDrawable(MessageResource.drawable.drm_green_lock));
                    drmAudioLock.setVisibility(View.VISIBLE);
                } else if (hasContent) {
                    drmAudioLock.setImageDrawable(
                            res.getDrawable(MessageResource.drawable.drm_red_lock));
                    drmAudioLock.setVisibility(View.VISIBLE);
                }
            }
            if (slideshow.get(position).hasImage()) {
                hasContent = slideshow.get(position).getImage().hasDrmContent();
                hasRight = slideshow.get(position).getImage().hasDrmRight();
                if (hasContent && hasRight) {
                      drmImageVideoLock.setImageDrawable(
                              res.getDrawable(MessageResource.drawable.drm_green_lock));
                      drmImageVideoLock.setVisibility(View.VISIBLE);
                } else if (hasContent) {
                    drmImageVideoLock.setImageDrawable(
                            res.getDrawable(MessageResource.drawable.drm_red_lock));
                    drmImageVideoLock.setVisibility(View.VISIBLE);
                }
            } else if (slideshow.get(position).hasVideo()) {
                hasContent = slideshow.get(position).getVideo().hasDrmContent();
                hasRight = slideshow.get(position).getVideo().hasDrmRight();
                if (hasContent && hasRight) {
                    drmImageVideoLock.setImageDrawable(
                            res.getDrawable(MessageResource.drawable.drm_green_lock));
                    drmImageVideoLock.setVisibility(View.VISIBLE);
                } else if (hasContent) {
                    drmImageVideoLock.setImageDrawable(
                            res.getDrawable(MessageResource.drawable.drm_red_lock));
                    drmImageVideoLock.setVisibility(View.VISIBLE);
                }
            }
        }
    }
}
