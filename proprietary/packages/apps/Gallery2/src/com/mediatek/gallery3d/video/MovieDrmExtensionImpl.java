package com.mediatek.gallery3d.video;

import android.content.Context;
import android.content.DialogInterface;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.net.Uri;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.omadrm.OmaDrmUtils;

public class MovieDrmExtensionImpl extends DefaultMovieDrmExtension {
    private static final String TAG = "VP_MovieDrmExt";
    private static final boolean LOG = true;

    @Override
    public boolean handleDrmFile(final Context context, final IMovieItem item,
                                 final IMovieDrmCallback callback) {
        boolean handle = false;
        DrmManagerClient client = ensureDrmClient(context);
        if (!item.isDrm()) {
            return false;
        }
        OmaDrmUtils.showConsumerDialog(context, client, item.getUri(),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        if (which == DialogInterface.BUTTON_POSITIVE) {
                            if (callback != null) {
                                callback.onContinue();
                            }
                        } else if (which == DialogInterface.BUTTON_NEGATIVE) {
                            if (callback != null) {
                                callback.onStop();
                            }
                        }
                    }
        });
        return true;
    }

    @Override
    public boolean canShare(final Context context, final IMovieItem item) {
        final Uri uri = item.getUri();
        Log.v(TAG, "canShare(" + uri + ")");
        final DrmManagerClient client = ensureDrmClient(context);
        boolean share = false;
        boolean isDrm = item.isDrm();

        if (isDrm) {
            int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
            try {
                rightsStatus = client.checkRightsStatus(uri, DrmStore.Action.TRANSFER);
            } catch (final IllegalArgumentException e) {
                Log.w(TAG, "canShare() : raise exception, we assume it has no rights to be shared");
            }
            share = (DrmStore.RightsStatus.RIGHTS_VALID == rightsStatus);
            if (LOG) {
                Log.v(TAG, "canShare(" + uri + "), rightsStatus=" + rightsStatus);
            }
        } else {
            share = true;
        }
        Log.v(TAG, "canShare(" + uri + "), share=" + share);
        return share;
    }

    private static DrmManagerClient sDrmClient;
    private static DrmManagerClient ensureDrmClient(final Context context) {
        if (sDrmClient == null) {
            synchronized (DrmManagerClient.class) {
                if (sDrmClient == null) {
                    sDrmClient = new DrmManagerClient(context.getApplicationContext());
                }
            }
        }
        return sDrmClient;
    }
}
