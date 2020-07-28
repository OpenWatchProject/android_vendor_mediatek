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

import android.service.dreams.DreamService;
import android.view.Display;
import android.animation.Animator;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.animation.TimeInterpolator;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.RelativeSizeSpan;
import android.text.style.StyleSpan;
import android.text.style.SuperscriptSpan;
import android.text.style.SubscriptSpan;
import android.text.style.TypefaceSpan;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.DecelerateInterpolator;
import android.widget.TextClock;
import android.widget.TextView;
import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;
import android.util.Log;
import android.util.DisplayMetrics;
import java.lang.Math;

/**
 * Helper class for AlwaysOnDisplay.
 * Used to format date and time.
 * AODMoveSaverRunnable is used to move the clock every minute.
 */
public class Utils {

	private static final String TAG = "AlwaysOnDisplay";

	/**
	 * @return {@code true} if the device is {@link Build.VERSION_CODES#JELLY_BEAN_MR2} or later
	 */
	public static boolean isJBMR2OrLater() {
		return Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2;
	}

	/** 
	 *Clock views can call this to refresh their date. 
	 */
	public static void updateDate(String dateFormat, String dateFormatForAccessibility, View clock)	{
		Log.d(TAG, "start of Utils updateDate");
		Date now = new Date();
		TextView dateDisplay;
		dateDisplay = (TextView) clock.findViewById(R.id.date);
		if (dateDisplay != null) {
			Log.d(TAG, "MTK dateDisplay not null");
			final Locale l = Locale.getDefault();
			/*dateDisplay.setText(isJBMR2OrLater()
			                     ? new SimpleDateFormat(DateFormat.getBestDateTimePattern(l, dateFormat), l).format(now)
			                     : SimpleDateFormat.getDateInstance().format(now));*/

			String dateString = new SimpleDateFormat(dateFormat).format(now);
			Log.d(TAG, "\nDate String Original : "+dateString);
			int start = dateFormat.indexOf('E');
			int end = dateFormat.lastIndexOf('E') + 1;
			String spanDateString = dateString.substring(start,end).toUpperCase().concat(dateString.substring(end));
			Log.d(TAG, "\nDate String Modified : "+spanDateString);

			Spannable sp = new SpannableString(spanDateString.toString());
			//sp.setSpan(new AbsoluteSizeSpan(R.dimen.label_font_size,true), start, end,
                        //Spannable.SPAN_POINT_MARK);
			sp.setSpan(new TypefaceSpan("sans-serif-condensed"), start, end,
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
			sp.setSpan(new StyleSpan(android.graphics.Typeface.BOLD), start, end,
			            Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

			dateDisplay.setText(sp);
			dateDisplay.setVisibility(View.VISIBLE);
			dateDisplay.setContentDescription(isJBMR2OrLater()
			                                  ? new SimpleDateFormat(DateFormat.getBestDateTimePattern(l, dateFormatForAccessibility), l).format(now)
			                                  : SimpleDateFormat.getDateInstance(java.text.DateFormat.FULL).format(now));
		}
		Log.d(TAG, "end of Utils updateDate");
	}

	/**
	 * For AOD app to set whether the digital or analog clock should be displayed.
	 * Returns the view to be displayed.
	 */
	public static View setClockStyle(Context context, View digitalClock) {
		Log.d(TAG, "MTK start of Utils setClockStyle");
		//String defaultClockStyle = context.getResources().getString(R.string.default_clock_style);
		View returnView;
		digitalClock.setVisibility(View.VISIBLE);
		returnView = digitalClock;
		Log.d(TAG, "end of Utils setClockStyle");
		return returnView;
	}

	/**
	 * Formats the time in the TextClock according to the Locale with a special
	 * formatting treatment for the am/pm label.
	 * @param context - Context used to get user's locale and time preferences
	 * @param clock - TextClock to format
	 * @param amPmFontSize - size of the am/pm label since it is usually smaller
	 */
	public static void setTimeFormat(Context context, TextClock clock, int amPmFontSize) {
		Log.d(TAG, "start of Utils setTimeFormat");
		if (clock != null) {
			// Get the best format for 12 hours mode according to the locale
			clock.setFormat12Hour(get12ModeFormat(context, amPmFontSize));
			// Get the best format for 24 hours mode according to the locale
			clock.setFormat24Hour(get24ModeFormat());
			Log.d(TAG, "end of Utils setTimeFormat");
		}
	}

	/**
	 * @param context - context used to get time format string resource
	 * @param amPmFontSize - size of am/pm label (label removed is size is 0).
	 * @return format string for 12 hours mode time
	 */
	public static CharSequence get12ModeFormat(Context context, int amPmFontSize) {
		Log.d(TAG, "start of Utils get12ModeFormat");
		return context.getString(R.string.time_format_12_mode);
		//String pattern = isJBMR2OrLater()
		//? DateFormat.getBestDateTimePattern(Locale.getDefault(), "hma")
		//: context.getString(R.string.time_format_12_mode);
		//String pattern = context.getString(R.string.time_format_12_mode);

		// Remove the am/pm
		/*if (amPmFontSize <= 0) {
		    pattern.replaceAll("a", "").trim();
		}
		// Replace spaces with "Hair Space".   vaibhav
		pattern = pattern.replaceAll(" ", "\u200A");
		// Build a spannable so that the am/pm will be formatted
		int amPmPos = pattern.indexOf('a');
		if (amPmPos == -1) {
		    return pattern;
		}

		Spannable sp = new SpannableString(pattern);
		//sp.setSpan(new SubscriptSpan(), amPmPos, amPmPos + 1,
		           //Spannable.SPAN_POINT_MARK);
		sp.setSpan(new RelativeSizeSpan(0.27f), amPmPos, amPmPos + 1,
		           Spannable.SPAN_POINT_MARK);
		//sp.setSpan(new AbsoluteSizeSpan(amPmFontSize), amPmPos, amPmPos + 1,
		           //Spannable.SPAN_POINT_MARK);
		sp.setSpan(new TypefaceSpan("sans-serif-thin"), amPmPos, amPmPos + 1,
		           Spannable.SPAN_POINT_MARK);
		sp.setSpan(new StyleSpan(android.graphics.Typeface.BOLD), amPmPos, amPmPos + 1,
		           Spannable.SPAN_POINT_MARK);
		Log.d(TAG, "end of Utils get12ModeFormat");
		return sp;*/
	}

	public static CharSequence get24ModeFormat() {
		Log.d(TAG, "start of Utils get24ModeFormat");
		return isJBMR2OrLater()
		       ? DateFormat.getBestDateTimePattern(Locale.getDefault(), "Hm")
		       : (new SimpleDateFormat("k:mm", Locale.getDefault())).toLocalizedPattern();
	}

	public static boolean is24HrFormat(Context context)	{
		Log.d(TAG, "start of Utils is24HrFormat");
		if (DateFormat.is24HourFormat(context)) {
			return true;
		} else {
			return false;
		}
	}

	/** Runnable for use with AOD and dream, to move the clock every minute.
	 *  registerViews() must be called prior to posting.
	 */
	/*public static class AODMoveSaverRunnable implements Runnable {
		static final long MOVE_DELAY = 60000; // DeskClock.SCREEN_SAVER_MOVE_DELAY;
		static final long SLIDE_TIME = 10000;
		static final long FADE_TIME = 2000;
		static final int REPEAT_TIMER_COUNT = 4;
		private int[] mClockPosition;
		int anim_timer=-1;
		static final boolean SLIDE = false;
		private View mContentView;
		private View mSaverView;
		private final Handler mHandler;
		private final Context mContext;
		private static TimeInterpolator mSlowStartWithBrakes;
		//private static View mTime, mDate, mCall, mCallNum, mSms, mSmsNum;

		public AODMoveSaverRunnable(Context context, Handler handler) {
			mHandler = handler;
			mContext = context;
			mSlowStartWithBrakes = new TimeInterpolator() {
				@Override
				public float getInterpolation(float x) {
					return (float)(Math.cos((Math.pow(x,3) + 1) * Math.PI) / 2.0f) + 0.5f;
				}
			};
		}

		public void registerViews(View contentView, View saverView) {
			mContentView = contentView;
			mSaverView = saverView;
			mClockPosition = mContext.getResources().getIntArray(R.array.clock_position);
		}

	    public void test(View a, View b, View c, View d, View e, View f) {
		 mTime = a;
		 mDate = b;
		 mCall = c;
		 mCallNum = d;
		 mSms = e;
		 mSmsNum = f;
	    }

		@Override
		public void run() {
			long delay = MOVE_DELAY;
			if (mContentView == null || mSaverView == null) {
				mHandler.removeCallbacks(this);
				mHandler.postDelayed(this, delay);
				return;
			}

			final float xrange = mContentView.getWidth() - mSaverView.getWidth();
			final float yrange = mContentView.getHeight() - mSaverView.getHeight();

	        //Log.d(TAG,"mContentView : "+mContentView);
	        //Log.d(TAG,"mSaverView : "+mSaverView);
	        //Log.d(TAG,"mContentView width, height: "+mContentView.getWidth()+" "+mContentView.getHeight());
	        //Log.d(TAG,"mSaverView width, height: "+mSaverView.getWidth()+" "+mSaverView.getHeight());

	        Log.d(TAG,"mTime width, height: "+mTime.getWidth()+" "+mTime.getHeight());

			Log.d(TAG,"mDate width, height: "+mDate.getWidth()+" "+mDate.getHeight());

			Log.d(TAG,"mCall width, height: "+mCall.getWidth()+" "+mCall.getHeight());

			Log.d(TAG,"mCallNum width, height: "+mCallNum.getWidth()+" "+mCallNum.getHeight());

			Log.d(TAG,"mSms width, height: "+mSms.getWidth()+" "+mSms.getHeight());

	        Log.d(TAG,"mSmsNum width, height: "+mSmsNum.getWidth()+" "+mSmsNum.getHeight());

			anim_timer=anim_timer+1;
	        //Log.d(TAG,"anim_timer value is: "+anim_timer);
			if (anim_timer>=REPEAT_TIMER_COUNT) {
				anim_timer=0;
			}

			if (xrange == 0 && yrange == 0) {
				delay = 500; // back in a split second
			} else {
				float density = mContext.getResources().getDisplayMetrics().density;
	            //Log.d(TAG,"Density: "+density);
				int nextx = Math.round(mClockPosition[anim_timer*2]*density);
				int nexty = Math.round(mClockPosition[anim_timer*2+1]*density);
	            Log.d(TAG,"Clock-y without density: "+mClockPosition[anim_timer*2+1]);
				Log.d(TAG,"Clock-y before: "+nexty);
				if (DateFormat.is24HourFormat(mContext) == false) {
					//nexty -= mContext.getResources().getDimension(R.dimen.clock_offset_12hr_format);
					Log.d(TAG,"Clock-y after: "+nexty);
					//nexty -= R.dimen.clock_offset_12hr_format;
	            }

				((DreamService)mContext).setDozeScreenState(Display.STATE_DOZE);
				Log.d(TAG, "nextx and nexty is:"+" "+nextx+" "+nexty);
				if (mSaverView.getAlpha() == 0f) {
					// jump right there
					mSaverView.setX(nextx);
					mSaverView.setY(nexty);
					ObjectAnimator.ofFloat(mSaverView, "alpha", 0f, 1f)
					.setDuration(FADE_TIME).start();
					Log.d(TAG, "nextx and nexty is:"+" "+nextx+" "+nexty);
				} else {
					AnimatorSet s = new AnimatorSet();
					Animator xMove   = ObjectAnimator.ofFloat(mSaverView,
					                   "x", mSaverView.getX(), nextx);
					Animator yMove   = ObjectAnimator.ofFloat(mSaverView,
					                   "y", mSaverView.getY(), nexty);
	                //Log.d(TAG, "Inside else : nextx1[anim] and nexty1[anim] is:"+" "+nextx+" "+nexty);
					if (SLIDE) {
						s.play(xMove).with(yMove);
						s.setDuration(SLIDE_TIME);
					} else {
						s.play(xMove.setDuration(0)).after(FADE_TIME);
						s.play(yMove.setDuration(0)).after(FADE_TIME);
					}
					s.start();
				}
				long now = System.currentTimeMillis();
				long adjust = (now % MOVE_DELAY);
				delay = delay
				        + (MOVE_DELAY - adjust) // minute aligned
				        - (SLIDE ? 0 : FADE_TIME); // start moving before the fade
				((DreamService)mContext).setDozeScreenState(Display.STATE_DOZE_SUSPEND);
			}
			mHandler.removeCallbacks(this);
			mHandler.postDelayed(this, delay);
		}
	}*/
}
