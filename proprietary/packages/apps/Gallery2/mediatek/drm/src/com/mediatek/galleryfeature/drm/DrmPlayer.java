package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.gallerybasic.base.MediaCenter;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MTexture;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.galleryfeature.drm.DeviceMonitor.ConnectStatus;
import com.mediatek.galleryfeature.drm.DrmDataAdapter.DataListener;


/**
 * Generate drm texture and control the texture lift.
 */
public class DrmPlayer extends Player implements Player.OnFrameAvailableListener, DataListener {
    private static final String TAG = "MtkGallery2/DrmPlayer";
    public static final String PLACE_HOLDER_COLOR = "#333333";

    public static final int MSG_PREPARED = 0;
    public static final int MSG_CONSUMED = 1;
    public static final int MSG_SHOW_DIALOG = 2;
    private MediaCenter mMediaCenter;
    private DrmItem mDrmItem;
    private Player mRealPlayer;
    // when other device plug in, limit the display of drm
    private ConnectStatus mDrmDisplayLimit = ConnectStatus.DISCONNECTED;
    private DrmProtectTexture mDrmDisplayLimitTexture;
    private DrmDataAdapter mDataAdapter;
    private Resources mRes;
    private boolean mIsFLMethod;
    private boolean mShowRealPlayer = false;
    /**
     * Constructor.
     *
     * @param context    The context for drm texture.
     * @param md         The date.
     * @param outputType The type of output.
     * @param mc         The mediacenter.
     */
    public DrmPlayer(Context context, MediaData md, OutputType outputType, MediaCenter mc,
                     Resources res) {
        super(context, md, outputType);
        mMediaCenter = mc;
        mRes = res;
    }

    @Override
    public boolean onPrepare() {
        if (mMediaData == null || mMediaCenter == null) {
            Log.e(TAG, "<onPrepare> invalid mMediaData or mMediaCenter");
            return false;
        }
        Log.d(TAG, " <onPrepare> mMediaData.caption = " + mMediaData.caption);
        mDrmItem = (DrmItem) mMediaCenter.getItem(mMediaData);
        if (mDrmItem == null) {
            Log.d(TAG, "<onPrepare> mDrmItem == null, return false");
            return false;
        }
        boolean isCTA = DrmHelper.isDataProtectionFile(mMediaData.filePath);
        boolean isFL = false;
        if (mMediaData.extFileds != null) {
            Object method = mMediaData.extFileds.getImageField(DrmField.DRM_METHOD);
            if (method != null) {
                isFL = DrmHelper.isFLDrm((int) method);
            }
        }
        mIsFLMethod = isCTA | isFL;
        prepareDisplayLimit();
        mDataAdapter = new DrmDataAdapter(mContext, mMediaData, mDrmItem, this);
        // Only for DRM file. Do not decode thumbnail for CTA file.
        if (DrmHelper.isDrmFile(mMediaData.filePath)) {
            mDataAdapter.onPrepare();
        }
        mRealPlayer = mMediaCenter.getRealPlayer(mMediaData, ThumbType.MIDDLE, DrmMember.sType);
        if (mRealPlayer != null) {
            byte[] buffer = DrmHelper.forceDecryptFile(mMediaData.filePath, false);
            if (buffer != null) {
                mRealPlayer.setBuffer(buffer);
            }
            mRealPlayer.setOnFrameAvailableListener(this);
            return mRealPlayer.prepare();
        }
        return true;
    }

    @Override
    public boolean onStart() {
        Log.d(TAG, " <onStart> ");
        sendNotify(MSG_SHOW_DIALOG);
        return true;
    }

    @Override
    public boolean onPause() {
        Log.d(TAG, " <onPause> ");
        boolean success = true;
        if (mRealPlayer != null && mRealPlayer.getState() == Player.State.PLAYING) {
            success = mRealPlayer.pause();
        }
        if (mRealPlayer != null) {
            mShowRealPlayer = false;
        }
        if (mDataAdapter != null) {
            mDataAdapter.onPause();
        }
        return success;
    }

    @Override
    public boolean onStop() {
        Log.d(TAG, " <onStop> ");
        if (mRealPlayer != null && mRealPlayer.getState() == State.PLAYING) {
            return mRealPlayer.stop();
        }
        if (mRealPlayer != null) {
            mShowRealPlayer = false;
        }
        return true;
    }

    @Override
    public void onRelease() {
        Log.d(TAG, " <onRelease> ");
        if (mRealPlayer != null) {
            mRealPlayer.release();
            mRealPlayer = null;
            mShowRealPlayer = false;
        }
        if (mDataAdapter != null) {
            mDataAdapter.onRelease();
        }
        mDrmItem = null;
    }

    @Override
    public int getOutputWidth() {
        if (mDrmDisplayLimit != ConnectStatus.DISCONNECTED && mDrmDisplayLimitTexture != null) {
            return mDrmDisplayLimitTexture.getWidth();
        } else if (mShowRealPlayer && mRealPlayer != null && mRealPlayer.getOutputWidth() > 0) {
            return mRealPlayer.getOutputWidth();
        } else if (mDataAdapter != null && mDataAdapter.getTexture() != null) {
            return mDataAdapter.getTexture().getWidth();
        } else {
            return 0;
        }
    }

    @Override
    public int getOutputHeight() {
        if (mDrmDisplayLimit != ConnectStatus.DISCONNECTED && mDrmDisplayLimitTexture != null) {
            return mDrmDisplayLimitTexture.getHeight();
        } else if (mShowRealPlayer && mRealPlayer != null && mRealPlayer.getOutputHeight() > 0) {
            return mRealPlayer.getOutputHeight();
        } else if (mDataAdapter != null && mDataAdapter.getTexture() != null) {
            return mDataAdapter.getTexture().getHeight();
        } else {
            return 0;
        }
    }

    @Override
    public MTexture getTexture(MGLCanvas canvas) {
        if (mDrmDisplayLimit != ConnectStatus.DISCONNECTED) {
            if (mDrmDisplayLimitTexture != null) {
                mDrmDisplayLimitTexture.setProtectStatus(mDrmDisplayLimit);
            }
            return mDrmDisplayLimitTexture;
        } else if (mShowRealPlayer
                && mRealPlayer != null && mRealPlayer.getTexture(canvas) != null) {
            return mRealPlayer.getTexture(canvas);
        } else if (mDataAdapter != null) {
            return mDataAdapter.getTexture();
        } else {
            return null;
        }
    }

    /**
     * Set Drm display limit.
     *
     * @param status The current connection status.
     */
    public void setDrmDisplayLimit(ConnectStatus status) {
        mDrmDisplayLimit = status;
        if (getState() == Player.State.PLAYING) {
            sendFrameAvailable();
        }
    }

    @Override
    public void onFrameAvailable(Player player) {
        if (mFrameAvailableListener != null) {
            mFrameAvailableListener.onFrameAvailable(this);
        }
    }

    public Player getRealPlayer() {
        return mRealPlayer;
    }

    /**
     * Decode High quality bitmap for full display.
     */
    public void doFullScreenPreview() {
        if (mRealPlayer != null) {
            mShowRealPlayer =
                    DrmHelper.hasRightsToShow(mContext, mMediaData.filePath, mMediaData.isVideo);
            if (mShowRealPlayer) {
                mRealPlayer.start();
            }
        }
        // 1. Consume right.
        // 2. If there has not right for display.
        //    Decode bitmap operation trigger Dialog.
        if (mDataAdapter != null) {
            mDataAdapter.onStart();
        }
    }

    /**
     * Check current state: on drm thumbnail or full image display.
     *
     * @return Whether on full display state or not.
     */
    public boolean isFullScreenPreview() {
        return (mDataAdapter != null && mDataAdapter.isScreenNailReady())
                || (mShowRealPlayer && mRealPlayer != null && (mRealPlayer.getOutputHeight() > 0 &&
                mRealPlayer.getOutputHeight() > 0)) || (mIsFLMethod && !mMediaData.isVideo);
    }

    private void prepareDisplayLimit() {
        synchronized (this) {
            if (mDrmDisplayLimitTexture == null) {
                mDrmDisplayLimitTexture = new DrmProtectTexture(mContext, mRes);
            }
        }
    }

    @Override
    public void onDataUpdated() {
        sendFrameAvailable();
    }

}
