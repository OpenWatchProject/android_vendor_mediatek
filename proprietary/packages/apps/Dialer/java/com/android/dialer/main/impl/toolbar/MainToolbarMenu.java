/*
 * Copyright (C) 2018 The Android Open Source Project
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
 * limitations under the License
 */

package com.android.dialer.main.impl.toolbar;

import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.view.Gravity;
import android.view.MenuItem;
import android.view.View;
import android.widget.PopupMenu;
import com.android.dialer.simulator.Simulator;
import com.android.dialer.simulator.SimulatorComponent;

///M: Add for conference call @{
import android.view.Menu;
import com.android.dialer.util.PermissionsUtil;
import com.mediatek.dialer.util.DialerVolteUtils;
/// @}

import com.android.dialer.common.LogUtil;


/** Popup menu accessible from the search bar */
public final class MainToolbarMenu extends PopupMenu {

  ///M: @{
  private Context mContext;
  /// @}
  public MainToolbarMenu(Context context, View anchor) {
    super(context, anchor, Gravity.NO_GRAVITY, R.attr.actionOverflowMenuStyle, 0);
    mContext = context;
  }

  public void showClearFrequents(boolean show) {
    getMenu().findItem(R.id.clear_frequents).setVisible(show);
  }

  public void maybeShowSimulator(AppCompatActivity activity) {
    MenuItem simulatorMenuItem = getMenu().findItem(R.id.menu_simulator_submenu);
    Simulator simulator = SimulatorComponent.get(activity).getSimulator();
    if (simulator.shouldShow()) {
      simulatorMenuItem.setVisible(true);
      simulatorMenuItem.setActionProvider(simulator.getActionProvider(activity));
    } else {
      simulatorMenuItem.setVisible(false);
    }
  }

    @Override
    public void show() {
      LogUtil.i("MainToolbarMenu"," show popup menu");
      Menu menu = getMenu();
      /// M: [VoLTE ConfCall] Show conference call menu for VoLTE
      /// And for GCF test, add empty voice call and empty video call entry @{
      if (DialerVolteUtils.isTestSim()) {
        menu.findItem(R.id.menu_volte_conf_call).setVisible(false);
        menu.findItem(R.id.menu_empty_voice_conf_call).setVisible(true);
        menu.findItem(R.id.menu_empty_video_conf_call).setVisible(true);
      } else {
        menu.findItem(R.id.menu_empty_voice_conf_call).setVisible(false);
        menu.findItem(R.id.menu_empty_video_conf_call).setVisible(false);
         boolean visible = DialerVolteUtils.isVolteConfCallEnable(mContext)
                    && PermissionsUtil.hasContactsReadPermissions(mContext);
        menu.findItem(R.id.menu_volte_conf_call).setVisible(visible);
      }
      /// @}
      super.show();
    }
}
