/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.gallery3d.video;

import android.content.Intent;
import android.net.Uri;
import android.view.Menu;
import android.view.MenuItem;

import com.android.gallery3d.R;
import com.android.gallery3d.app.PhotoPage;
import com.android.gallery3d.app.TrimVideo;
import com.mediatek.gallery3d.util.Log;

public class TrimVideoHooker extends MovieHooker {
    private static final String TAG = "VP_TrimVideoHooker";
    private static final int MENU_TRIM_VIDEO = 1;
    private MenuItem mMenutTrim;
    private IMovieItem mMovieItem;
    private static final String CONTENT_MEDIA = "content://media";

    @Override
    public void setParameter(final String key, final Object value) {
        super.setParameter(key, value);
        Log.d(TAG, "setParameter(" + key + ", " + value + ")");
        if (value instanceof IMovieItem) {
            mMovieItem = (IMovieItem) value;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(final Menu menu) {
        super.onCreateOptionsMenu(menu);
        mMenutTrim = menu.add(MENU_HOOKER_GROUP_ID, getMenuActivityId(MENU_TRIM_VIDEO), 0,
                R.string.trim_action);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(final Menu menu) {
        super.onPrepareOptionsMenu(menu);
        //when play stream, there is no need to have trim option in menu.
        //when uri can not query from media store, eg: email, mms, not show trim.
        if (MovieUtils.isLocalFile(mMovieItem.getUri(), mMovieItem.getMimeType()) &&
                isUriSupportTrim(mMovieItem.getUri()) &&
                mMovieItem.canBeRetrieved() &&
                !mMovieItem.isDrm()) {
            mMenutTrim.setVisible(true);
        } else {
            mMenutTrim.setVisible(false);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(final MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (getMenuOriginalId(item.getItemId())) {
        case MENU_TRIM_VIDEO:
            // start activity
            Uri original = mMovieItem.getUri();
            Log.d(TAG, "original=" + original);
            String videoPath = mMovieItem.getVideoPath();
            Log.d(TAG, "path=" + videoPath);
            Intent intent = new Intent(getContext(), TrimVideo.class);
            intent.setData(original);
            // We need the file path to wrap this into a RandomAccessFile.
            intent.putExtra(PhotoPage.KEY_MEDIA_ITEM_PATH, videoPath);
            getContext().startActivity(intent);
            //finish MovieActivity after start another one.
            getContext().finish();
            return true;
        default:
            return false;
        }
    }

    //trim support uri type:
    // 1. content://media/external/video/media
    // 2. filemanager uri
    private boolean isUriSupportTrim(Uri uri) {
        return String.valueOf(uri).toLowerCase().startsWith(CONTENT_MEDIA) ||
                String.valueOf(uri).toLowerCase().startsWith("file://");
    }
}
