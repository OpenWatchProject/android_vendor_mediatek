/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.calendar;

import com.android.calendar.CalendarController.EventInfo;
import com.android.calendar.CalendarController.EventType;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.text.format.Time;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ProgressBar;
import android.widget.ViewSwitcher;
import android.widget.ViewSwitcher.ViewFactory;

import com.mediatek.calendar.LogUtil;
import com.mediatek.calendar.PDebug;

/**
 * This is the base class for Day and Week Activities.
 */
public class DayFragment extends Fragment implements CalendarController.EventHandler, ViewFactory {
    /**
     * The view id used for all the views we create. It's OK to have all child
     * views have the same ID. This ID is used to pick which view receives
     * focus when a view hierarchy is saved / restore
     */
    ///M:@{
    private static final String TAG = "DayFragment";
    ///@}
    private static final int VIEW_ID = 1;

    protected static final String BUNDLE_KEY_RESTORE_TIME = "key_restore_time";

    protected ProgressBar mProgressBar;
    protected ViewSwitcher mViewSwitcher;
    protected Animation mInAnimationForward;
    protected Animation mOutAnimationForward;
    protected Animation mInAnimationBackward;
    protected Animation mOutAnimationBackward;
    EventLoader mEventLoader;

    Time mSelectedDay = new Time();

    private final Runnable mTZUpdater = new Runnable() {
        @Override
        public void run() {
            if (!DayFragment.this.isAdded()) {
                return;
            }
            String tz = Utils.getTimeZone(getActivity(), mTZUpdater);
            mSelectedDay.timezone = tz;
            mSelectedDay.normalize(true);
        }
    };

    private int mNumDays;

    public DayFragment() {
        mSelectedDay.setToNow();
    }

    public DayFragment(long timeMillis, int numOfDays) {
        mNumDays = numOfDays;
        if (timeMillis == 0) {
            mSelectedDay.setToNow();
        } else {
            mSelectedDay.set(timeMillis);
        }
    }

    /**
     * M: pass the context in to get original displayed time in our calendar
     * @param context
     * @param timeMillis
     * @param numOfDays
     */
    public DayFragment(Context context, long timeMillis, int numOfDays) {
        mNumDays = numOfDays;
        mSelectedDay = Utils.getValidTimeInCalendar(context, timeMillis);
    }

    @Override
    public void onCreate(Bundle icicle) {
        PDebug.EndAndStart("AllInOneActivity.onCreate->DayFragment.onCreate", "DayFragment.onCreate");

        PDebug.Start("DayFragment.onCreate.superOnCreate");
        super.onCreate(icicle);
        PDebug.End("DayFragment.onCreate.superOnCreate");

        Context context = getActivity();

        PDebug.Start("DayFragment.onCreate.loadAnimations");
        mInAnimationForward = AnimationUtils.loadAnimation(context, R.anim.slide_left_in);
        mOutAnimationForward = AnimationUtils.loadAnimation(context, R.anim.slide_left_out);
        mInAnimationBackward = AnimationUtils.loadAnimation(context, R.anim.slide_right_in);
        mOutAnimationBackward = AnimationUtils.loadAnimation(context, R.anim.slide_right_out);
        PDebug.End("DayFragment.onCreate.loadAnimations");

        mEventLoader = new EventLoader(context);
        PDebug.EndAndStart("DayFragment.onCreate", "DayFragment.onCreate->DayFragment.onCreateView");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        PDebug.EndAndStart("DayFragment.onCreate->DayFragment.onCreateView", "DayFragment.onCreateView");

        PDebug.Start("DayFragment.onCreateView.inflateViewSwitcher");
        View v = inflater.inflate(R.layout.day_activity, null);
        mViewSwitcher = (ViewSwitcher) v.findViewById(R.id.switcher);
        PDebug.End("DayFragment.onCreateView.inflateViewSwitcher");

        mViewSwitcher.setFactory(this);

        PDebug.Start("DayFragment.onCreateView.updateViewSwitcher");
        mViewSwitcher.getCurrentView().requestFocus();
        ((DayView) mViewSwitcher.getCurrentView()).updateTitle();
        PDebug.End("DayFragment.onCreateView.updateViewSwitcher");

        PDebug.EndAndStart("DayFragment.onCreateView", "DayFragment.onCreateView->AllInOneActivity.onResume");

        return v;
    }

    public View makeView() {
        PDebug.Start("DayFragment.makeView");

        mTZUpdater.run();
        DayView view = new DayView(getActivity(), CalendarController
                .getInstance(getActivity()), mViewSwitcher, mEventLoader, mNumDays);
        view.setId(VIEW_ID);
        view.setLayoutParams(new ViewSwitcher.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        view.setSelected(mSelectedDay, false, false);

        PDebug.End("DayFragment.makeView");
        return view;
    }

    @Override
    public void onResume() {
        PDebug.EndAndStart("AllInOneActivity.onResume->DayFragment.onResume", "DayFragment.onResume");

        super.onResume();
        mEventLoader.startBackgroundThread();
        mTZUpdater.run();
        eventsChanged();
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        view.handleOnResume();
        view.restartCurrentTimeUpdates();

        view = (DayView) mViewSwitcher.getNextView();
        view.handleOnResume();
        view.restartCurrentTimeUpdates();

        PDebug.End("DayFragment.onResume");
        PDebug.Start("DayFragment.onResume->DayView.onSizeChanged");
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        long time = getSelectedTimeInMillis();
        if (time != -1) {
            outState.putLong(BUNDLE_KEY_RESTORE_TIME, time);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        view.cleanup();
        view = (DayView) mViewSwitcher.getNextView();
        view.cleanup();
        mEventLoader.stopBackgroundThread();

        // Stop events cross-fade animation
        view.stopEventsAnimation();
        ((DayView) mViewSwitcher.getNextView()).stopEventsAnimation();
    }

    void startProgressSpinner() {
        // start the progress spinner
        mProgressBar.setVisibility(View.VISIBLE);
    }

    void stopProgressSpinner() {
        // stop the progress spinner
        mProgressBar.setVisibility(View.GONE);
    }

    private void goTo(Time goToTime, boolean ignoreTime, boolean animateToday) {
        if (mViewSwitcher == null) {
            // The view hasn't been set yet. Just save the time and use it later.
            mSelectedDay.set(goToTime);
            return;
        }

        DayView currentView = (DayView) mViewSwitcher.getCurrentView();
        ///M:@{
        if (currentView == null) {
            LogUtil.e(TAG, "getCurrentView() return null,return");
            return;
        }
        currentView.selectionFocusShow(false);
        ///@}
        // How does goTo time compared to what's already displaying?
        int diff = currentView.compareToVisibleTimeRange(goToTime);

        if (diff == 0) {
            // In visible range. No need to switch view
            currentView.setSelected(goToTime, ignoreTime, animateToday);
        } else {
            // Figure out which way to animate
            if (diff > 0) {
                mViewSwitcher.setInAnimation(mInAnimationForward);
                mViewSwitcher.setOutAnimation(mOutAnimationForward);
            } else {
                mViewSwitcher.setInAnimation(mInAnimationBackward);
                mViewSwitcher.setOutAnimation(mOutAnimationBackward);
            }

            DayView next = (DayView) mViewSwitcher.getNextView();
           ///M:@{
            next.selectionFocusShow(false);
            ///@}
            if (ignoreTime) {
                next.setFirstVisibleHour(currentView.getFirstVisibleHour());
            }

            next.setSelected(goToTime, ignoreTime, animateToday);
            next.reloadEvents();
            mViewSwitcher.showNext();
            next.requestFocus();
            next.updateTitle();
            next.restartCurrentTimeUpdates();
        }
    }

    /**
     * Returns the selected time in milliseconds. The milliseconds are measured
     * in UTC milliseconds from the epoch and uniquely specifies any selectable
     * time.
     *
     * @return the selected time in milliseconds
     */
    public long getSelectedTimeInMillis() {
        if (mViewSwitcher == null) {
            return -1;
        }
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        if (view == null) {
            return -1;
        }
        return view.getSelectedTimeInMillis();
    }

    public void eventsChanged() {
        PDebug.Start("DayFragment.eventsChanged");

        if (mViewSwitcher == null) {
            return;
        }
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        view.clearCachedEvents();
        view.reloadEvents();

        view = (DayView) mViewSwitcher.getNextView();
        view.clearCachedEvents();

        PDebug.End("DayFragment.eventsChanged");
    }

    Event getSelectedEvent() {
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        return view.getSelectedEvent();
    }

    boolean isEventSelected() {
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        return view.isEventSelected();
    }

    Event getNewEvent() {
        DayView view = (DayView) mViewSwitcher.getCurrentView();
        return view.getNewEvent();
    }

    public DayView getNextView() {
        return (DayView) mViewSwitcher.getNextView();
    }

    public long getSupportedEventTypes() {
        return EventType.GO_TO | EventType.EVENTS_CHANGED;
    }

    public void handleEvent(EventInfo msg) {
        if (msg.eventType == EventType.GO_TO) {
// TODO support a range of time
// TODO support event_id
// TODO support select message
            goTo(msg.selectedTime, (msg.extraLong & CalendarController.EXTRA_GOTO_DATE) != 0,
                    (msg.extraLong & CalendarController.EXTRA_GOTO_TODAY) != 0);
        } else if (msg.eventType == EventType.EVENTS_CHANGED) {
            eventsChanged();
        }
    }
}
