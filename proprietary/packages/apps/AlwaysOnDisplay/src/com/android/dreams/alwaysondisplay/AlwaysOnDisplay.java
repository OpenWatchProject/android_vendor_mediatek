/*
 * Copyright (C) 2016 The Android Open Source Project
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

package com.android.dreams.alwaysondisplay;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.ContentObserver;
import android.Manifest;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.UserHandle;
import android.provider.CallLog;
import android.provider.Settings;
import android.service.dreams.DreamService;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.TextClock;
import android.content.res.Configuration;


/**
 * Always on Display Feature.
 * <p>
 * adb shell setprop debug.doze.component com.android.dreams.alwaysondisplay/.AlwaysOnDisplay
 * This debug property will not be available in user load.
 * </p>
 */
public class AlwaysOnDisplay extends DreamService {

	private static final String TAG = AlwaysOnDisplay.class.getSimpleName();
	private static final boolean DEBUG = true;

	/**
	 * Identifier for call related operations.
	 */
	private static final int TYPE_MISSED_CALL = 1;

	/**
	 * Identifier for SMS related operations.
	 */
	private static final int TYPE_UNREAD_SMS = 2;

	/**
	 * Optimization of cursor query.
	 * When the content change notification occurs, instead of fire query,
	 * it would wait for given amount of time so that if multiple
	 * change notification occurs, then query requests will remain less.
	 */
	private static final long REFRESH_DELAY = 1000;

	/**
	 * Content URI to get the unread SMS
	 */
	private static final Uri SMS_URI = Uri.parse("content://sms/inbox");

	/**
	 * Content URI to which listen for new SMS.
	 * SMS_URI used to get SMS does not notify new SMS notification.
	 * Hence, used different URI.
	 */
	private static final Uri NOTFIY_SMS_URI = Uri.parse("content://sms/");
	//private static final Uri NOTFIY_SMS_URI = Uri.parse("content://mms-sms/");

	/**
	 * Instance of Power manager used for wake lock
	 */
	private PowerManager mPowerManager;

	/**
	 * Wake lock instance used to keep the processor alive.
	 */
	private PowerManager.WakeLock mWakeLock;

	/**
	 * Implementation of AsyncQueryHandler.
	 * It automatically create worker thread and runs query on it.
	 * so the UI thread does not gets blocked for database operation.
	 */
	private MyQueryHandler mHandler;

	/**
	 * Runnable used with MyQueryHandler.
	 */
	private AsyncQueryRunnable mAsyncQueryRunnable;

	/**
	 * Runnable used with MyQueryHandler.
	 */
	private SuspendStateRunnable mSuspendStateRunnable;

	/**
	 * Runnable used to move the content in the screen at regular intervals.
	 */
	//private AODMoveSaverRunnable mMoveSaverRunnable;

	/**
	 * Displays the time.
	 * In 12 hr format, shows only time excluding AM/PM
	 */
	private View  mDigitalClock;

	/**
	 * Displays AM/PM only when time format is 12 hr.
	 * It gets hidden in 24 hr format
	 */
	//private View  mAmPm;

	/**
	 * Root/Parent layout of window.
	 */
	private View mContentView;

	/**
	 * Container layout of content.
	 * used to float the content in window.
	 */
	private View mSaverView;

	/**
	 * Container layout of notifications - Call and SMS.
	 */
	private View mNotificationContainer;

	/**
	 * Displays Call icon.
	 * Hides it when either count is 0 or application does not have
	 * permission to read call log.
	 */
	private View mCallIcon;

	/**
	 * Displays unseen missed call count.
	 * Hides it when either count is 0 or application does not have
	 * permission to read call log.
	 */
	private TextView mCallCount;

	/**
	 * Displays SMS icon.
	 * Hides it when either count is 0 or application does not have
	 * permission to read SMS.
	 */
	private View mSmsIcon;

	/**
	 * Displays unseen-unread new SMS count.
	 * Hides it when either count is 0 or application does not have
	 * permission to read SMS.
	 */
	private TextView mSmsCount;

	/**
	 * Optimization of cursor query.
	 * If the query is processing and new content change occurs, then defer the
	 * new query request till the processing query completed.
	 * If multiple content change notification occurs while query is processing,
	 * then we will re-run query only once after the current query completed.
	 */
	private boolean mIsTaskRunning = false;

	/**
	 * Optimization of cursor query.
	 * It checks when the current query completed for any new request came
	 * while the query is processing.
	 * If requests came, then re run the query; else update result.
	 */
	private boolean mNeedToRunCursor = false;

	/**
	 * Stores the date format.
	 */
	private String mDateFormat;

	/**
	 * Stores the date format used for accessibility.
	 */
	private String mDateFormatForAccessibility;
	//private View mTime, mDate;

	/**
	 * Instance of Alarm Manager service.
	 * Must use alaram manager to listen for time interval.
	 * Because alarm fires even if the system is in Doze mode.
	 */
	private AlarmManager mAlarmManager;

	/**
	 * Pending intent fires everytime the alarm expires.
	 */
	private PendingIntent mAlarmIntent;

	/**
	 * Count of timer used to reposition the clock at every minute interval.
	 */
	private int mTimerCount = -1;

	/**
	 * Represents the number of different positions of clock.
	 */
	static final int REPEAT_TIMER_COUNT = 4;

	/**
	 * Array containing the co-ordinates of clock for different positions.
	 */
	private int[] mClockPosition;

	/**
	 * Time (in MS) after which clock changes its position.
	 */
	static final long MOVE_DELAY = 60000;
	//static final long SLIDE_TIME = 10000;
	//static final long FADE_TIME = 1000;

	/**
	 * Time (in MS) after which display state suspends.
	 * This value is based on assumption that within this much time after 
	 * we have udpated view, frame gets updated.
	 */
	static final long SUSPEND_STATE_TIMER = 2000;

	/**
	 * Time (in MS) for which wake lock would be held.
	 * This value should be grater than SUSPEND_STATE_TIMER, as wake lock should be 
	 * released only after evrything is done including STATE_DOZE_SUSPEND.
	 */
	static final long WAKELOCK_HELD_TIMEOUT = 3000;

	/**
	 * Time (in MS) for which wake lock would be held.
	 * This timeout is only for first time when the wake lock acquired in onCreate.
	 * This value is based on assumption that within this much time from creation
	 * its first frame gets updated.
	 * It might change in future as per the requirements.
	 */
	static final long WAKELOCK_HELD_TIMEOUT_FIRST = 5000;


    /**
     * Immersive mode confrimation
     */
    private static final String CONFIRMED = "confirmed";
    private boolean mBeforeImmersiveMode = false;

	// Not all hardware supports dozing.  We should use Display.STATE_DOZE but
	// for testing purposes it is convenient to use Display.STATE_ON so the
	// test still works on hardware that does not support dozing.

	/**
	 * Receiver to handle time reference changes.
	 */
	private final BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();
			if (DEBUG) Log.v(TAG, "Doze onReceive, action: " + action);
			if (action == null) {
				return;
			}

			// To handle the seldom issue, when the date update intent received before the AOD setup.
			if (mContentView != null) {
				mWakeLock.acquire(WAKELOCK_HELD_TIMEOUT);
				mHandler.removeCallbacks(mSuspendStateRunnable);
				setDozeScreenState(Display.STATE_DOZE);
				if (action.equals(Intent.ACTION_TIME_CHANGED) ||
				        action.equals(Intent.ACTION_TIMEZONE_CHANGED) ||
				        action.equals(Intent.ACTION_DATE_CHANGED)) {
					Utils.updateDate(mDateFormat, mDateFormatForAccessibility, mContentView);
				}
				//setDozeScreenState(Display.STATE_DOZE_SUSPEND);
				mHandler.postDelayed(mSuspendStateRunnable,SUSPEND_STATE_TIMER);
			}
		}
	};

	public AlwaysOnDisplay() {
		if (DEBUG) Log.d(TAG, "Always on Display dream started");
		//mMoveSaverRunnable = new AODMoveSaverRunnable(mHandler);
		//mTestSMSObs = new TestSMSObs(mHandler);
		mAsyncQueryRunnable = new AsyncQueryRunnable();
		mSuspendStateRunnable = new SuspendStateRunnable();
	}

	@Override
	public void onCreate() {
		Log.d(TAG, "Always on Display's onCreate");
		super.onCreate();
                int orientation = getResources().getConfiguration().orientation;
		mHandler = new MyQueryHandler(getContentResolver());
		//mMoveSaverRunnable = new AODMoveSaverRunnable(this,mHandler);

		mPowerManager = (PowerManager)getSystemService(Context.POWER_SERVICE);
		mWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

		mDateFormat = getString(R.string.abbrev_wday_month_day_no_year);
		mDateFormatForAccessibility = getString(R.string.full_wday_month_day_no_year);

		if(orientation == Configuration.ORIENTATION_LANDSCAPE)
		{
		    mClockPosition = getResources().getIntArray(R.array.clock_position_land);
		}
		else
		{
		    mClockPosition = getResources().getIntArray(R.array.clock_position);
		}
		// Register receiver to handle time reference changes.
		final IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_DATE_CHANGED);
		filter.addAction(Intent.ACTION_TIME_CHANGED);
		filter.addAction(Intent.ACTION_TIMEZONE_CHANGED);
		registerReceiver(mIntentReceiver, filter);

		// Set pending intent and register receiver to catch alaram expiry.
		// Used Alarm manager in place of handler, as handler does not gets called in Doze mode.
		mAlarmManager = (AlarmManager)getSystemService(Context.ALARM_SERVICE);
		Intent intent = new Intent("com.android.dreams.alwaysondisplay.ACTION_AOD");
		intent.setPackage(getPackageName());
		IntentFilter filter_alarm = new IntentFilter();
		filter_alarm.addAction(intent.getAction());
		registerReceiver(mAlarmReceiver, filter_alarm);
		mAlarmIntent = PendingIntent.getBroadcast(this, 0, intent,
		               PendingIntent.FLAG_CANCEL_CURRENT);

		// Set Displat state to STATE_DOZE.
		setDozeScreenState(Display.STATE_DOZE);
		mWakeLock.acquire(WAKELOCK_HELD_TIMEOUT_FIRST);
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mIntentReceiver);
		//mHandler.removeCallbacks(mMoveSaverRunnable);
		mHandler.removeCallbacks(mAsyncQueryRunnable);
		mHandler.removeCallbacks(mSuspendStateRunnable);
		cancelTimeUpdate();
		unregisterReceiver(mAlarmReceiver);
	}

	@Override
	public void onAttachedToWindow() {
		super.onAttachedToWindow();
                // force set "Viewing full screen" as "confirmed",
                // must called here before "setSystemUiVisibility"
                mBeforeImmersiveMode = getImmerModeSetting();
                setImmerModeSetting(true);
                setInteractive(true);
		setLowProfile(true);
		setFullscreen(true);
		setContentView(R.layout.alwaysondisplay);
		setScreenBright(false);
		View decorView = getWindow().getDecorView();

		// Set flags to disable status bar and navigation bar permanently.
		int flags =  View.SYSTEM_UI_FLAG_LAYOUT_STABLE
		             | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
		             | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
		             | View.SYSTEM_UI_FLAG_FULLSCREEN
		             | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
		             | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
		decorView.setSystemUiVisibility(flags);
		layoutClockSaver();
		//mHandler.post(mMoveSaverRunnable);
	}

	@Override
	public void onDreamingStarted() {
		super.onDreamingStarted();
		Log.d(TAG, "Always on Display's Dream started: canDoze=" + canDoze());

		//Register receiver only if has REAM_SMS permission.
		if (hasRunTimePermission(TYPE_UNREAD_SMS)) {
			this.getContentResolver().registerContentObserver(
			    NOTFIY_SMS_URI, true, mSmsChangeObserver);
		}
		performTimeUpdate();
		// Start Dozing after the UI operation completed
		startDozing();
	}

	@Override
	public void onDreamingStopped() {
		super.onDreamingStopped();
                //restore the before status
		setImmerModeSetting(mBeforeImmersiveMode);
		//Unegister receiver only if has REAM_SMS permission.
		if (hasRunTimePermission(TYPE_UNREAD_SMS)) {
			this.getContentResolver().unregisterContentObserver(mSmsChangeObserver);
		}
		Log.d(TAG, "Always on Display's Dream ended: isDozing=" + isDozing());

		// Stop dozing when the dream stops.
		stopDozing();
	}

      private void setImmerModeSetting(boolean confirmed) {
        try {
           final String value = confirmed ? CONFIRMED : null;
           Settings.Secure.putStringForUser(getContentResolver(),
                   Settings.Secure.IMMERSIVE_MODE_CONFIRMATIONS,
                   value,
                   UserHandle.USER_CURRENT);
           Log.d(TAG, "Saved value=" + value);
        } catch (Throwable t) {
           Log.w(TAG, "Error saving confirmations, confirmed=" + confirmed, t);
       }
    }

    private boolean getImmerModeSetting() {
        String value = null;
        value = Settings.Secure.getStringForUser(getContentResolver(),
                Settings.Secure.IMMERSIVE_MODE_CONFIRMATIONS,
                UserHandle.USER_CURRENT);
        Log.d(TAG, "Loaded value=" + value);
        return CONFIRMED.equals(value);

    }

	private void setClockStyle() {
		Log.d(TAG, "Always on Display's setClockStyle");
		Utils.setClockStyle(getApplicationContext(), mDigitalClock);
		mSaverView = findViewById(R.id.main_clock);
	}

	private void layoutClockSaver() {
		Log.d(TAG, "Always on Display's LayoutClockSaver");

		// Set STATE_DOZE before drawing so that UI update does not get lost.
		// In STATE_DOZE_SUSPEND, UI updates may be ignored.
		setDozeScreenState(Display.STATE_DOZE);

		mDigitalClock = findViewById(R.id.dig_cloc);
		setClockStyle();

		mContentView = (View) mSaverView.getParent();
		mSaverView.setAlpha(0);
		//mMoveSaverRunnable.registerViews(mContentView, mSaverView);

		// Hide AM/PM if 24 hr format.
		/*mAmPm = findViewById(R.id.am_pm);
		if (Utils.is24HrFormat(this)) {
		    mAmPm.setVisibility(View.GONE);
		} else {
		    mAmPm.setVisibility(View.VISIBLE);
		}*/

		mCallIcon = (View)findViewById(R.id.call_icon);
		mCallCount = (TextView)findViewById(R.id.call_count);
		mSmsIcon = (View)findViewById(R.id.sms_icon);
		mSmsCount = (TextView)findViewById(R.id.sms_count);
		mNotificationContainer = (View)findViewById(R.id.notif_layout);

		//Test code, Should be removed.
		//mTime = (View)findViewById(R.id.dig_cloc);
		//mDate = (View)findViewById(R.id.date);
		//mMoveSaverRunnable.test(mTime , mDate, mCallIcon, mCallCount, mSmsIcon, mSmsCount);

		Utils.updateDate(mDateFormat, mDateFormatForAccessibility, mContentView);
		Log.d(TAG, "end of  Always on Display's layoutClockSaver");

		// layout Call and SMS notifications
		layoutNotificationData();

		// Reset to STATE_DOZE_SUSPEND after drawing, so that power consumption is less.
		//setDozeScreenState(Display.STATE_DOZE_SUSPEND);
		mHandler.postDelayed(mSuspendStateRunnable,SUSPEND_STATE_TIMER);
	}

	/**
	 * Layout content - time/date/call/sms
	 */
	private void layoutNotificationData() {
		// Hide missed calls notification if don't have READ_CALl_LOG permission.
		if (hasRunTimePermission(TYPE_MISSED_CALL)) {
			//Log.d(TAG, "Call Permission Enabled ");
			queryCallData();
		} else {
			//Log.d(TAG, "Call Permission Disabled ");
			updateNotificationVisibility(TYPE_MISSED_CALL, false);
		}

		// Hide unseen new SMS notification if don't have READ_SMS permission.
		if (hasRunTimePermission(TYPE_UNREAD_SMS)) {
			//Log.d(TAG, "SMS Permission Enabled ");
			querySmsData();
		} else {
			//Log.d(TAG, "SMS Permission Disabled ");
			updateNotificationVisibility(TYPE_UNREAD_SMS, false);
		}
	}

	/**
	 * Query CallLog content provider for the unseen missed calls count.
	 * Also update the calls notification information and the visibility.
	 */
	private void queryCallData() {
		String where = CallLog.Calls.TYPE+"="+CallLog.Calls.MISSED_TYPE+" AND "+CallLog.Calls.IS_READ+"="+0;

		// For first display, we will fire the query from the UI thread, so keep notifiation display in sync
		// with date and time.
		// In future, we may change it to worker thread through AsyncQueryHandler.
		Cursor call_cursor = this.getContentResolver().query(CallLog.Calls.CONTENT_URI, null,where, null, null);
		call_cursor.moveToFirst();

		int count = call_cursor.getCount();
		Log.d(TAG, "Missed call Count : "+count);
		mCallCount.setText(""+count);
		if (count > 0) {
			updateNotificationVisibility(TYPE_MISSED_CALL, true);
		} else {
			updateNotificationVisibility(TYPE_MISSED_CALL, false);
		}
	}

	/**
	 * Query SMS content provider for the unseen new SMS count.
	 * Also update the SMS notification information and the visibility.
	 */
	private void querySmsData() {
		// For first display, we will fire the query from the UI thread, so keep notifiation display in sync
		// with date and time.
		// In future, we may change it to worker thread through AsyncQueryHandler.
		Cursor sms_cursor = this.getContentResolver().query(SMS_URI, null,"seen=0", null, null);
		sms_cursor.moveToFirst();

		int count = sms_cursor.getCount();
		Log.d(TAG, "Sms Count : "+count);
		mSmsCount.setText(" "+count);
		if (count > 0) {
			updateNotificationVisibility(TYPE_UNREAD_SMS, true);
		} else {
			updateNotificationVisibility(TYPE_UNREAD_SMS, false);
		}
	}

	private boolean hasRunTimePermission(int type) {
		boolean ret = false;

		switch (type) {
			case TYPE_MISSED_CALL:
				if (getApplicationContext().checkSelfPermission(Manifest.permission.READ_CALL_LOG)
				        == PackageManager.PERMISSION_GRANTED) {
					ret = true;
				}
				break;
			case TYPE_UNREAD_SMS:
				if (getApplicationContext().checkSelfPermission(Manifest.permission.READ_SMS)
				        == PackageManager.PERMISSION_GRANTED) {
					ret = true;
				}
				break;
			default:
				break;
		}

		return ret;
	}

	/**
	 * Update the visibility of Call and SMS notifications.
	 * Hide them if no permission or count is 0
	 */
	private void updateNotificationVisibility(int type, boolean isShow) {
		switch (type) {
			case TYPE_MISSED_CALL:
				if (isShow == true) {
					mCallCount.setVisibility(View.VISIBLE);
					mCallIcon.setVisibility(View.VISIBLE);
				} else {
					mCallCount.setVisibility(View.GONE);
					mCallIcon.setVisibility(View.GONE);
				}
				break;
			case TYPE_UNREAD_SMS:
				if (isShow == true) {
					mSmsCount.setVisibility(View.VISIBLE);
					mSmsIcon.setVisibility(View.VISIBLE);
				} else {
					mSmsCount.setVisibility(View.GONE);
					mSmsIcon.setVisibility(View.GONE);
				}
				break;
			default:
				break;
		}

		if (mCallCount.getVisibility() == View.VISIBLE ||
		        mSmsCount.getVisibility() == View.VISIBLE) {
			mNotificationContainer.setVisibility(View.VISIBLE);
		} else {
			mNotificationContainer.setVisibility(View.GONE);
		}
	}

	/**
	 * Update SMS count and the visibility.
	 * It gets called when the SMS content changed.
	 */
	private void updateSmsCount(int count) {
		mWakeLock.acquire(WAKELOCK_HELD_TIMEOUT);
		mHandler.removeCallbacks(mSuspendStateRunnable);
		setDozeScreenState(Display.STATE_DOZE);
		mSmsCount.setText(""+count);
		if (count > 0) {
			updateNotificationVisibility(TYPE_UNREAD_SMS, true);
		} else {
			updateNotificationVisibility(TYPE_UNREAD_SMS, false);
		}
		//setDozeScreenState(Display.STATE_DOZE_SUSPEND);
		mHandler.postDelayed(mSuspendStateRunnable,SUSPEND_STATE_TIMER);
	}

	/**
	 * Fire query to fetch latest SMS data.
	 */
	private void startAsyncQueryHandler() {
		mHandler.startQuery(TYPE_UNREAD_SMS,null,SMS_URI, null,"seen=0", null, null);
	}

	/**
	 * Content observer used to listen to new SMS.
	 * Gets notified whenever new SMS comes.
	 * Post query request.
	 */
	private ContentObserver mSmsChangeObserver = new ContentObserver(mHandler) {

		@Override
		public void onChange(boolean selfChange) {
			//Log.d(TAG, "Content Changed ");
			this.onChange(selfChange, null);
		}

		@Override
		public void onChange(boolean selfChange, Uri uri) {
			Log.d(TAG, "Content Changed ");

			// If Query is processing, then defer the new content change notification.
			// else post the query to fetch latest data.
			if (mIsTaskRunning == false) {
				Log.d(TAG, "Start Async Handler ");
				mIsTaskRunning = true;
				mHandler.postDelayed(mAsyncQueryRunnable,REFRESH_DELAY);
			} else {
				//Log.d(TAG, "Handler running, defer");
				mNeedToRunCursor = true;
			}
		}
	};

	/**
	 * Runnable used with MyQueryHandler.
	 * Its task is to fire query to get the latest SMS data.
	 */
	private class AsyncQueryRunnable implements Runnable {
		public void run() {
			startAsyncQueryHandler();
		}
	}

	/**
	 * Runnable used to set state STATE_DOZE_SUSPEND.
	 * We don't have any idea of frame update timings so better to suspend
	 * after some delay.
	 */
	private class SuspendStateRunnable implements Runnable {
		public void run() {
			Log.d(TAG, "AlwaysOnDisplay's diplay State Suspended");
			setDozeScreenState(Display.STATE_DOZE_SUSPEND);
		}
	}

	/**
	 * Implementation of AsyncQueryHandler.
	 * It automatically create worker thread and runs query on it.
	 * so the UI thread does not gets blocked for database operation.
	 */
	private class MyQueryHandler extends AsyncQueryHandler {

		public MyQueryHandler(ContentResolver cr) {
			super(cr);
		}

		@Override
		protected void onQueryComplete(int token, Object cookie, Cursor result) {
			// If content change notification came when this query is processing,
			// then fire new query to fetch latest data.
			// else update the result.
			if (mNeedToRunCursor == true) {
				Log.d(TAG, "Restart deferred Async Queue Handler ");
				mNeedToRunCursor = false;
				mHandler.post(mAsyncQueryRunnable);
			} else {
				Log.d(TAG, "Post the result");
				mIsTaskRunning = false;
				if (result != null) {
					result.moveToFirst();
					updateSmsCount(result.getCount());
				}
			}
		}
	}

	private void cancelTimeUpdate() {
		mAlarmManager.cancel(mAlarmIntent);
	}

	private final BroadcastReceiver mAlarmReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			performTimeUpdate();
		}
	};

	/**
	 * Moves the clock to different positions after every 1 minute interval
	 * Uses Alarm manager to check for time interval.
	 * Must use alaram manager to listen for time interval.
	 * Because alarm fires even if the system is in Doze mode.
	 */
	public void performTimeUpdate() {
		long delay = MOVE_DELAY;
		long now = System.currentTimeMillis();
		now -= now % 60000;
		if (mContentView == null || mSaverView == null) {
			//mHandler.removeCallbacks(this);
			//mHandler.postDelayed(this, delay);
			mAlarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, now + delay, mAlarmIntent);
			return;
		}

		final float xrange = mContentView.getWidth() - mSaverView.getWidth();
		final float yrange = mContentView.getHeight() - mSaverView.getHeight();

		//Log.d(TAG,"mContentView : "+mContentView);
		//Log.d(TAG,"mSaverView : "+mSaverView);
		//Log.d(TAG,"mContentView width, height: "+mContentView.getWidth()+" "+mContentView.getHeight());
		//Log.d(TAG,"mSaverView width, height: "+mSaverView.getWidth()+" "+mSaverView.getHeight());

		/*Log.d(TAG,"mTime width, height: "+mTime.getWidth()+" "+mTime.getHeight());

		Log.d(TAG,"mDate width, height: "+mDate.getWidth()+" "+mDate.getHeight());

		Log.d(TAG,"mCall width, height: "+mCall.getWidth()+" "+mCall.getHeight());

		Log.d(TAG,"mCallNum width, height: "+mCallNum.getWidth()+" "+mCallNum.getHeight());

		Log.d(TAG,"mSms width, height: "+mSms.getWidth()+" "+mSms.getHeight());

		Log.d(TAG,"mSmsNum width, height: "+mSmsNum.getWidth()+" "+mSmsNum.getHeight());*/

		mTimerCount = mTimerCount + 1;
		Log.d(TAG,"mTimerCount value is: "+mTimerCount);
		if (mTimerCount >= REPEAT_TIMER_COUNT) {
			mTimerCount = 0;
		}

		if (xrange == 0 && yrange == 0) {
			delay = 500; // back in a split second
		} else {
			float density = getResources().getDisplayMetrics().density;
			//Log.d(TAG,"Density: "+density);
			int nextx = Math.round(mClockPosition[mTimerCount*2]*density);
			int nexty = Math.round(mClockPosition[mTimerCount*2+1]*density);

			mWakeLock.acquire(WAKELOCK_HELD_TIMEOUT);
			mHandler.removeCallbacks(mSuspendStateRunnable);
			setDozeScreenState(Display.STATE_DOZE);
			Log.d(TAG, "nextx and nexty is:"+" "+nextx+" "+nexty);
			if (mSaverView.getAlpha() == 0f) {
				// jump right there
				mSaverView.setX(nextx);
				mSaverView.setY(nexty);
				mSaverView.setAlpha(1f);
				/*ObjectAnimator.ofFloat(mSaverView, "alpha", 0f, 1f)
				.setDuration(FADE_TIME).start();*/
				//Log.d(TAG, "nextx and nexty is:"+" "+nextx+" "+nexty);
			} else {
				/*AnimatorSet s = new AnimatorSet();
				Animator xMove   = ObjectAnimator.ofFloat(mSaverView,
				                   "x", mSaverView.getX(), nextx);
				Animator yMove   = ObjectAnimator.ofFloat(mSaverView,
				                   "y", mSaverView.getY(), nexty);
				//Log.d(TAG, "Inside else : nextx1[anim] and nexty1[anim] is:"+" "+nextx+" "+nexty);
				//if (SLIDE) {
				    //s.play(xMove).with(yMove);
				    //s.setDuration(SLIDE_TIME);
				//} else {
				    s.play(xMove.setDuration(0)).after(FADE_TIME);
				    s.play(yMove.setDuration(0)).after(FADE_TIME);
				//}
				s.start();*/
				mSaverView.setX(nextx);
				mSaverView.setY(nexty);
			}
			//long adjust = (now % MOVE_DELAY);
			//delay = delay
			//+ (MOVE_DELAY - adjust); // minute aligned
			//- (FADE_TIME);
			//- (SLIDE ? 0 : FADE_TIME); // start moving before the fade
			//((DreamService)mContext).setDozeScreenState(Display.STATE_DOZE_SUSPEND);
			//now -= now % MOVE_DELAY; // back up to last minute boundary
			//setDozeScreenState(Display.STATE_DOZE_SUSPEND);
			mHandler.postDelayed(mSuspendStateRunnable,SUSPEND_STATE_TIMER);
		}

		// It is OKAY if timer expires not exactly at 1 minute interval, but with some delay.
		// So EXACT timer mode is not used, as it will consume more power.
		mAlarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, now + delay, mAlarmIntent);
	}
}
