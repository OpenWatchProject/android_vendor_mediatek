package com.mediatek.galleryportable;

import android.app.Activity;
import android.content.Intent;
import android.widget.ActivityChooserModel;
import android.widget.ActivityChooserModel.OnChooseActivityListener;

/**
 * Wrap ActivityChooserModel.
 */
public class ActivityChooserModelWrapper {
    private static boolean sHasChecked = false;
    private static boolean sIsActivityChooserModelExist = false;
    private ActivityChooserModel mActivityChooserModel;
    private OnChooseActivityListenerWrapper mChooseActivityListenerWrapper;

    /**
     * Listener for choosing an activity.
     */
    public interface OnChooseActivityListenerWrapper {
        /**
         * Called when an activity has been chosen.
         * @param host
         *            The listener's host model.
         * @param intent
         *            The intent for launching the chosen activity.
         * @return Whether the intent is handled and should not be delivered to clients.
         */
        public boolean onChooseActivity(ActivityChooserModelWrapper host, Intent intent);
    }

    /**
     * Use for listen onChooseActivity method callback.
     */
    class MyOnChooseActivityListener implements OnChooseActivityListener {
        @Override
        public boolean onChooseActivity(ActivityChooserModel host, Intent intent) {
            return mChooseActivityListenerWrapper.onChooseActivity(
                    ActivityChooserModelWrapper.this, intent);
        }
    }

    /**
     * Create ActivityChooserModel object.
     * @param activity
     *            current activity
     * @param xml
     *            share history xml file
     */
    public ActivityChooserModelWrapper(final Activity activity, final String xml) {
        checkWetherSupport();
        if (sIsActivityChooserModelExist) {
            mActivityChooserModel = ActivityChooserModel.get(activity, xml);
        }
    }

    /**
     * Set OnChooseActivityListener.
     * @param listener
     *            OnChooseActivityListenerWrapper,wrap OnChooseActivityListener
     */
    public void setOnChooseActivityListener(OnChooseActivityListenerWrapper listener) {
        checkWetherSupport();
        if (sIsActivityChooserModelExist) {
            mChooseActivityListenerWrapper = listener;
            if (mChooseActivityListenerWrapper == null) {
                mActivityChooserModel.setOnChooseActivityListener(null);
            } else {
                mActivityChooserModel
                        .setOnChooseActivityListener(new MyOnChooseActivityListener());
            }
        }
    }

    private void checkWetherSupport() {
        if (!sHasChecked) {
            try {
                Class<?> clazz =
                        ActivityChooserModelWrapper.class.getClassLoader().loadClass(
                                "android.widget.ActivityChooserModel");
                sIsActivityChooserModelExist = (clazz != null);
            } catch (ClassNotFoundException e) {
                sIsActivityChooserModelExist = false;
            }
            sHasChecked = true;
        }
    }
}
