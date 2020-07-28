package com.mediatek.calendar.patchs.alerts;

import com.android.calendar.alerts.NotificationMgr;

public class AlertServicePatch {

    /**
     * If the event has fired,then update it to alert again,it will
     * not pop-up.The function will fix this by determine whether cancel
     * the notify that will be update,so it will pop-up again.
     * @param id
     * @param quietUpdate
     * @param doPopup
     * @param notificationMgr
     */
    public void cancelNotify(int id, boolean quietUpdate, boolean doPopup, NotificationMgr notificationMgr) {
        if (!quietUpdate && doPopup) {
            notificationMgr.cancel(id);
        }
    }
}
