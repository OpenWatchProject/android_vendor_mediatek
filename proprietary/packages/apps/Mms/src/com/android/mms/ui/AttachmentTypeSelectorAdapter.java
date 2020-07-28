/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.mms.ui;

import android.content.Context;

import com.android.mms.MmsConfig;
import com.android.mms.R;

import java.util.ArrayList;
import java.util.List;

/**
 * An adapter to store icons and strings for attachment type list.
 */
public class AttachmentTypeSelectorAdapter extends IconListAdapter {
    public final static int MODE_WITH_SLIDESHOW    = 1;
    public final static int MODE_WITHOUT_SLIDESHOW = 2;

    public final static int ADD_IMAGE               = 0;
    public final static int TAKE_PICTURE            = 1;
    public final static int ADD_VIDEO               = 2;
    public final static int RECORD_VIDEO            = 3;
    public final static int ADD_SOUND               = 4;
    public final static int RECORD_SOUND            = 5;
    public final static int ADD_SLIDESHOW           = 6;

    /// M: add for vCard
    public static final int MODE_WITH_FILE_ATTACHMENT        = 4;
    public static final int MODE_WITHOUT_FILE_ATTACHMENT     = 8;
    public static final int MODE_WITH_VCALENDAR              = 16;
    public static final int MODE_WITH_SOUNDRECORDER          = 32;
    public static final int ADD_VCARD               = 7;
    public static final int ADD_VCALENDAR           = 8;
    /// M: If recipient is not a user-IPMessage,Send IP multi-media message via MMS/SMS

    public AttachmentTypeSelectorAdapter(Context context, int mode) {
        super(context, getData(mode, context));
    }

    public int buttonToCommand(int whichButton) {
        AttachmentListItem item = (AttachmentListItem) getItem(whichButton);
        return item.getCommand();
    }

    protected static List<IconListItem> getData(int mode, Context context) {
        List<IconListItem> data = new ArrayList<IconListItem>(8);
        addItem(data, context.getString(R.string.attach_image),
                R.drawable.ic_attach_picture_holo_light, ADD_IMAGE);

        addItem(data, context.getString(R.string.attach_take_photo),
                R.drawable.ic_attach_capture_picture_holo_light, TAKE_PICTURE);

        addItem(data, context.getString(R.string.attach_video),
                R.drawable.ic_attach_video_holo_light, ADD_VIDEO);

        addItem(data, context.getString(R.string.attach_record_video),
                R.drawable.ic_attach_capture_video_holo_light, RECORD_VIDEO);

        if (MmsConfig.getAllowAttachAudio()) {
            addItem(data, context.getString(R.string.attach_sound),
                    R.drawable.ic_attach_audio_holo_light, ADD_SOUND);
        }

        if ((mode & MODE_WITH_SOUNDRECORDER) == MODE_WITH_SOUNDRECORDER) {
            addItem(data, context.getString(R.string.attach_record_sound),
                    R.drawable.ic_attach_capture_audio_holo_light, RECORD_SOUND);
        }

        if ((mode & MODE_WITH_SLIDESHOW) == MODE_WITH_SLIDESHOW) {
            addItem(data, context.getString(R.string.attach_slideshow),
                    R.drawable.ic_attach_slideshow_holo_light, ADD_SLIDESHOW);
        }

        /// M: add for vCard
        if ((mode & MODE_WITH_FILE_ATTACHMENT) == MODE_WITH_FILE_ATTACHMENT) {
            addItem(data, context.getString(R.string.attach_vcard),
                    R.drawable.ic_vcard_attach_menu, ADD_VCARD);
            if ((mode & MODE_WITH_VCALENDAR) == MODE_WITH_VCALENDAR) {
                addItem(data, context.getString(R.string.attach_vcalendar),
                        R.drawable.ic_vcalendar_attach_menu, ADD_VCALENDAR);
            }
        }

        return data;
    }

    protected static void addItem(List<IconListItem> data, String title,
            int resource, int command) {
        AttachmentListItem temp = new AttachmentListItem(title, resource, command);
        data.add(temp);
    }

    public static class AttachmentListItem extends IconListAdapter.IconListItem {
        private int mCommand;

        public AttachmentListItem(String title, int resource, int command) {
            super(title, resource);

            mCommand = command;
        }

        public int getCommand() {
            return mCommand;
        }
    }
}
