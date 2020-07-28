package com.mediatek.camera.platform;

public interface ISelfTimeManager {

    /**
     * get status of self-timer
     * @return true means current self-timer is open,so can use this
     *         false means current self-time is closed;
     */
    public boolean isSelfTimerEnabled();

    /**
     * set you wanted duration of the self-timer
     * @param duration the time you want;such as "1000","2000"......
     */
    public void setSelfTimerDuration(String duration);

    /**
     * set whether need play the self-timer sound or not
     * @param isNeed true means need play the sound;otherwise not need play
     */
    public void needPlaySound(boolean isNeed);

    /**
     * start the self-timer
     * @return true means started success;otherwise failed
     */
    public boolean startSelfTimer();

    public boolean isSelfTimerCounting();

    public void setLowStorage();
}
