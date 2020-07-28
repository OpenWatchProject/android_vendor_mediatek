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

package com.android.gallery3d.app;

import android.content.Context;
import android.content.res.Resources;

import com.android.gallery3d.R;
import com.android.gallery3d.ui.AlbumSetSlotRenderer;
import com.android.gallery3d.ui.SlotView;
import com.mediatek.gallery3d.layout.FancyHelper;

/// M: [FEATURE.MODIFY] @{
/*final class Config {*/
/**
 * set as public for Container.
 */
public final class Config {
/// @}
    public static class AlbumSetPage {
        private static AlbumSetPage sInstance;

        public SlotView.Spec slotViewSpec;
        public AlbumSetSlotRenderer.LabelSpec labelSpec;
        public int paddingTop;
        public int paddingBottom;
        public int placeholderColor;

        public static synchronized AlbumSetPage get(Context context) {
            /// M: [FEATURE.MODIFY] fancy layout @{
            // 1. connect phone to smb, launch Gallery on smb, then plug out phone
            // 2. launch Gallery on phone, album labels will became extremely small @{
            /*
            if (sInstance == null) {
                sInstance = new AlbumSetPage(context);
            }
            */
            sInstance = new AlbumSetPage(context);
            /// @}
            return sInstance;
        }

        /// M: [FEATURE.ADD] Multi-window. @{
        private static final int COLS_LAND_MULTI_WINDOW = 2;
        private static final int COLS_PORT_MULTI_WINDOW = 2;

        /**
         * Get AlbumSetPage config in Multi-window mode.
         * @param context context
         * @return AlbumSetPage config
         */
        public static synchronized AlbumSetPage getConfigInMultiWindow(Context context) {
            AlbumSetPage config = new AlbumSetPage(context);
            config.slotViewSpec.colsLand = COLS_LAND_MULTI_WINDOW;
            config.slotViewSpec.colsPort = COLS_PORT_MULTI_WINDOW;
            return config;
        }
        /// @}

        private AlbumSetPage(Context context) {
            Resources r = context.getResources();

            placeholderColor = r.getColor(R.color.albumset_placeholder);

            slotViewSpec = new SlotView.Spec();
            slotViewSpec.rowsLand = r.getInteger(R.integer.albumset_rows_land);
            slotViewSpec.rowsPort = r.getInteger(R.integer.albumset_rows_port);
            /// M: [FEATURE.ADD] fancy layout @{
            if (FancyHelper.isFancyLayoutSupported()) {
                slotViewSpec.colsLand = FancyHelper.ALBUMSETPAGE_COL_LAND;;
                slotViewSpec.colsPort = FancyHelper.ALBUMSETPAGE_COL_PORT;
            }
            /// @}
            slotViewSpec.slotGap = r.getDimensionPixelSize(R.dimen.albumset_slot_gap);
            slotViewSpec.slotHeightAdditional = 0;

            paddingTop = r.getDimensionPixelSize(R.dimen.albumset_padding_top);
            paddingBottom = r.getDimensionPixelSize(R.dimen.albumset_padding_bottom);

            labelSpec = new AlbumSetSlotRenderer.LabelSpec();
            labelSpec.labelBackgroundHeight = r.getDimensionPixelSize(
                    R.dimen.albumset_label_background_height);
            labelSpec.titleOffset = r.getDimensionPixelSize(
                    R.dimen.albumset_title_offset);
            labelSpec.countOffset = r.getDimensionPixelSize(
                    R.dimen.albumset_count_offset);
            labelSpec.titleFontSize = r.getDimensionPixelSize(
                    R.dimen.albumset_title_font_size);
            labelSpec.countFontSize = r.getDimensionPixelSize(
                    R.dimen.albumset_count_font_size);
            labelSpec.leftMargin = r.getDimensionPixelSize(
                    R.dimen.albumset_left_margin);
            /// M: [FEATURE.MODIFY] fancy layout @{
            /*
            labelSpec.titleRightMargin = r.getDimensionPixelSize(
                    R.dimen.albumset_title_right_margin);
            */
            if (FancyHelper.isFancyLayoutSupported()) {
                labelSpec.titleRightMargin = r.getDimensionPixelSize(
                        R.dimen.albumset_title_right_margin_fancy);
            } else {
                labelSpec.titleRightMargin = r.getDimensionPixelSize(
                        R.dimen.albumset_title_right_margin);
            }
            /// @}
            labelSpec.iconSize = r.getDimensionPixelSize(
                    R.dimen.albumset_icon_size);
            /// M: [FEATURE.MODIFY] fancy layout @{
            /*
            labelSpec.backgroundColor = r.getColor(
                    R.color.albumset_label_background);
            */
            if (FancyHelper.isFancyLayoutSupported()) {
                labelSpec.backgroundColor = r.getColor(
                        R.color.albumset_label_background_fancy);
            } else {
                labelSpec.backgroundColor = r.getColor(
                        R.color.albumset_label_background);
            }
            /// @}
            labelSpec.titleColor = r.getColor(R.color.albumset_label_title);
            labelSpec.countColor = r.getColor(R.color.albumset_label_count);
        }
    }

    public static class AlbumPage {
        private static AlbumPage sInstance;

        public SlotView.Spec slotViewSpec;
        public int placeholderColor;

        public static synchronized AlbumPage get(Context context) {
            if (sInstance == null) {
                sInstance = new AlbumPage(context);
            }
            return sInstance;
        }

        private AlbumPage(Context context) {
            Resources r = context.getResources();

            placeholderColor = r.getColor(R.color.album_placeholder);

            slotViewSpec = new SlotView.Spec();
            slotViewSpec.rowsLand = r.getInteger(R.integer.album_rows_land);
            slotViewSpec.rowsPort = r.getInteger(R.integer.album_rows_port);
            /// M: [FEATURE.ADD] fancy layout @{
            if (FancyHelper.isFancyLayoutSupported()) {
                slotViewSpec.colsLand = FancyHelper.ALBUMPAGE_COL_LAND;
                slotViewSpec.colsPort = FancyHelper.ALBUMPAGE_COL_PORT;
            }
            /// @}
            slotViewSpec.slotGap = r.getDimensionPixelSize(R.dimen.album_slot_gap);
        }
    }

    public static class ManageCachePage extends AlbumSetPage {
        private static ManageCachePage sInstance;

        public final int cachePinSize;
        public final int cachePinMargin;

        public static synchronized ManageCachePage get(Context context) {
            if (sInstance == null) {
                sInstance = new ManageCachePage(context);
            }
            return sInstance;
        }

        public ManageCachePage(Context context) {
            super(context);
            Resources r = context.getResources();
            cachePinSize = r.getDimensionPixelSize(R.dimen.cache_pin_size);
            cachePinMargin = r.getDimensionPixelSize(R.dimen.cache_pin_margin);
        }
    }
}

