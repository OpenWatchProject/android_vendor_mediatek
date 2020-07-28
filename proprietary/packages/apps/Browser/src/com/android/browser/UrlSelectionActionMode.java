/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.android.browser;
import android.R;
import android.content.Context;
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;
import android.view.inputmethod.InputMethodManager;

public class UrlSelectionActionMode implements ActionMode.Callback {

    private UiController mUiController;

    public UrlSelectionActionMode(UiController controller) {
        mUiController = controller;
    }

    // ActionMode.Callback implementation

    @Override
    public boolean onCreateActionMode(ActionMode mode, Menu menu) {
        //mode.getMenuInflater().inflate(R.menu.url_selection, menu);
        /// M: Display the title always
        mode.setTitleOptionalHint(false);
        return true;
    }

    @Override
    public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
        switch (item.getItemId()) {
            case R.id.shareText:
                /// M: Hide the input method. @{
                if (mUiController.getCurrentTopWebView() != null) {
                    InputMethodManager inputMethod = (InputMethodManager)
                        mUiController.getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
                    inputMethod.hideSoftInputFromWindow(
                        mUiController.getCurrentTopWebView().getWindowToken(), 0);
                }
                /// @}
                mUiController.shareCurrentPage();
                mode.finish();
                break;
            default:
                return false;
        }
        return true;
    }

    @Override
    public void onDestroyActionMode(ActionMode mode) {
    }

    @Override
    public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
        return true;
    }

}
