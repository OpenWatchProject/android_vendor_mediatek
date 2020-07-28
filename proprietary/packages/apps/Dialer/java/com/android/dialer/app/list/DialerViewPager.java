/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.android.dialer.app.list;

import android.content.Context;
import android.graphics.Canvas;
import android.os.Handler;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.android.dialer.common.LogUtil;

/** Class that handles enabling/disabling swiping between @{ViewPagerTabs}. */
public class DialerViewPager extends ViewPager {

  private boolean enableSwipingPages;

  public DialerViewPager(Context context, AttributeSet attributeSet) {
    super(context, attributeSet);
    enableSwipingPages = true;
  }

  @Override
  public boolean onInterceptTouchEvent(MotionEvent event) {
    if (enableSwipingPages) {
      if (DialtactsPagerAdapter.SUPPORT_LAZY_ITEM) {
          addLazyItems();
      }
      return super.onInterceptTouchEvent(event);
    }

    return false;
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    if (enableSwipingPages) {
      return super.onTouchEvent(event);
    }

    return false;
  }

  public void setEnableSwipingPages(boolean enabled) {
    enableSwipingPages = enabled;
  }

  DialtactsPagerAdapter mAdapter;
  Handler mHandler = new Handler();
  static final int DELAY_TIME_TO_ADD_LAZIES = 500;

  @Override
  public void setCurrentItem(int item) {
    super.setCurrentItem(item);
    if (mAdapter != null) {
      mAdapter.setCurrentItem(item);
      addLazyItemIfNeed(item);
    }
  }

  @Override
  public void setCurrentItem(int item, boolean smoothScroll) {
      super.setCurrentItem(item, smoothScroll);
      if (mAdapter != null) {
          mAdapter.setCurrentItem(item);
          addLazyItemIfNeed(item);
      }
  }

  @Override
  public void setAdapter(PagerAdapter adapter) {
    super.setAdapter(adapter);
    if (adapter instanceof DialtactsPagerAdapter && DialtactsPagerAdapter.SUPPORT_LAZY_ITEM) {
      mAdapter = (DialtactsPagerAdapter) adapter;
    }
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    int currentItem = getCurrentItem();
    if (DialtactsPagerAdapter.SUPPORT_LAZY_ITEM && mAdapter != null
            && mAdapter.hasLazyItems(getCurrentItem())) {
      LogUtil.i("DialerViewPager.onDraw", "hasLazyItems and start timer to add them");
      mHandler.removeCallbacks(mAddLazyItemRunable);
      mHandler.postDelayed(mAddLazyItemRunable, DELAY_TIME_TO_ADD_LAZIES);
    }
  }

  Runnable mAddLazyItemRunable = new Runnable() {
    @Override
    public void run() {
//      LogUtil.i("ViewPagerAdapter.mAddLazyItemRunable", "running ...");
      addLazyItems();
    }
  };

  private void addLazyItemIfNeed(int position) {
    if (mAdapter.isLazyItem(position)) {
      LogUtil.i("DialerViewPager.addLazyItemIfNeed", ": " + position);
      mAdapter.startUpdate(this);
      mAdapter.addLazyItem(this, position);
      mAdapter.finishUpdate(this);
    }
  }

  private void addLazyItems() {
    if (mAdapter != null) {
      int currentPos = getCurrentItem();
      if (mAdapter.hasLazyItems(currentPos)) {
        LogUtil.i("DialerViewPager.addLazyItems", "currentPos is " + currentPos);
        mAdapter.startUpdate(this);
        mAdapter.addLazyItem(this, currentPos + 1);
        mAdapter.addLazyItem(this, currentPos - 1);
        mAdapter.finishUpdate(this);
      }
    }
  }

  @Override
  protected void onDetachedFromWindow() {
    super.onDetachedFromWindow();
    if (DialtactsPagerAdapter.SUPPORT_LAZY_ITEM && mHandler.hasCallbacks(mAddLazyItemRunable)) {
      LogUtil.i("DialerViewPager.onDetachedFromWindow", "remove Callback");
      mHandler.removeCallbacks(mAddLazyItemRunable);
    }
  }

}
