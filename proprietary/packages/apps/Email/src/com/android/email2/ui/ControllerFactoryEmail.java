package com.android.email2.ui;

import com.android.mail.ui.ActivityController;
import com.android.mail.ui.ControllerFactory;
import com.android.mail.ui.MailActivity;
import com.android.mail.ui.ViewMode;

public final class ControllerFactoryEmail extends ControllerFactory {
    /**
     * Create the appropriate type of ActivityController.
     *
     * @return the appropriate {@link ActivityControllerEmail} to control {@link MailActivity}.
     */
    public static ActivityController forActivity(MailActivity activity, ViewMode viewMode,
            boolean isTabletDevice) {
        return isTabletDevice ? new TwoPaneControllerEmail(activity, viewMode)
        : new OnePaneControllerEmail(activity, viewMode);
    }
}
