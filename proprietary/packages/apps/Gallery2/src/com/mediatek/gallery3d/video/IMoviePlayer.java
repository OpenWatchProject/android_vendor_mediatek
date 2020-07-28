package com.mediatek.gallery3d.video;

import android.view.SurfaceView;
import android.view.animation.Animation;

/**
 * MoviePlayer extension functions interface
 */
public interface IMoviePlayer {
    /**
     * start current item and stop playing video.
     * @param item
     */
    void startNextVideo(IMovieItem item);
    /**
     * Loop current video.
     * @param loop
     */
    void setLoop(boolean loop);
    /**
     * Loop current video or not
     * @return
     */
    boolean getLoop();
    /**
     * Can stop current video or not.
     * @return
     */
    boolean canStop();
    /**
     * Stop current video.
     */
    void stopVideo();
    /**
     * show current SubtitleView.
     */
    void showSubtitleViewSetDialog();
    /**
     * Get current video type.
     */
    int getVideoType();
   /**
     * Get current video Position.
     */
    int getVideoPosition();
   /**
     * Get current video LastDuration.
     */
    int getVideoLastDuration();
   /**
     * start video
     * @param enableFasten
     * @param position
     * @param duration
     */
    void startVideo(final boolean enableFasten, final int position, final int duration);
   /**
     * Get get video surface. only host
     */
    SurfaceView getVideoSurface();
   /**
     * @return current video is can seek or not.
     */
    boolean isVideoCanSeek();
   /**
     * @return current video is can seek forward or not.
     */
    boolean canSeekForward();
   /**
     * @return current video is can seek backward or not.
     */
    boolean canSeekBackward();
   /**
     * @return time bar is enabled or not.
     */
    boolean isTimeBarEnabled();
   /**
     * seek video to the position specified.
     * @param msec
     */
    void seekTo(int msec);
   /**
     * set video duration specified.
     * @param duration
     */
    void setDuration(int duration);
   /**
     * @return the position current video playing.
     */
    int getCurrentPosition();
   /**
     * @return the duration of current video.
     */
    int getDuration();
   /**
     * @return the animation hide.
     */
    Animation getHideAnimation();
   /**
     * update progress bar positon.
     */
    void updateProgressBar();
   /**
     * show Ended.
     */
    void showEnded();
   /**
     * notify current video play completed..
     */
    void notifyCompletion();

    /**
     * show movie controller.
     */
    void showMovieController();
}
