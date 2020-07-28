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


import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.provider.Settings;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SubMenu;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.mms.ContentRestrictionException;
import com.android.mms.ExceedMessageSizeException;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.ResolutionException;
import com.android.mms.RestrictedResolutionException;
import com.android.mms.TempFileProvider;
import com.android.mms.UnsupportContentTypeException;
import com.android.mms.data.WorkingMessage;
import com.android.mms.draft.DraftManager;
import com.android.mms.model.IModelChangedObserver;
import com.android.mms.model.LayoutModel;
import com.android.mms.model.Model;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.ui.BasicSlideEditorView.OnScrollTouchListener;
import com.android.mms.ui.BasicSlideEditorView.OnTextChangedListener;
import com.android.mms.ui.MessageUtils.ResizeImageResultCallback;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduPart;
import com.mediatek.android.mms.pdu.MtkPduPersister;

import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.ext.IOpSlideEditorActivityExt;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.omadrm.OmaDrmStore;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;

/**
 * This activity allows user to edit the contents of a slide.
 */
public class SlideEditorActivity extends Activity implements ITextSizeAdjustHost {
    private static final String TAG = "SlideEditorActivity";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;

    // Key for extra data.
    public static final String SLIDE_INDEX = "slide_index";

    // Menu ids.
    private final static int MENU_REMOVE_TEXT       = 0;
    private final static int MENU_ADD_PICTURE       = 1;
    private final static int MENU_TAKE_PICTURE      = 2;
    private final static int MENU_DEL_PICTURE       = 3;
    private final static int MENU_ADD_AUDIO         = 4;
    private final static int MENU_DEL_AUDIO         = 5;
    private final static int MENU_ADD_VIDEO         = 6;
    private final static int MENU_ADD_SLIDE         = 7;
    private final static int MENU_DEL_VIDEO         = 8;
    private final static int MENU_LAYOUT            = 9;
    private final static int MENU_DURATION          = 10;
    private final static int MENU_PREVIEW_SLIDESHOW = 11;
    private final static int MENU_RECORD_SOUND      = 12;
    private final static int MENU_SUB_AUDIO         = 13;
    private final static int MENU_TAKE_VIDEO        = 14;

    // Request code.
    private final static int REQUEST_CODE_EDIT_TEXT          = 0;
    private final static int REQUEST_CODE_CHANGE_PICTURE     = 1;
    private final static int REQUEST_CODE_TAKE_PICTURE       = 2;
    private final static int REQUEST_CODE_CHANGE_MUSIC       = 3;
    private final static int REQUEST_CODE_RECORD_SOUND       = 4;
    private final static int REQUEST_CODE_CHANGE_VIDEO       = 5;
    private final static int REQUEST_CODE_CHANGE_DURATION    = 6;
    private final static int REQUEST_CODE_TAKE_VIDEO         = 7;

    // number of items in the duration selector dialog that directly map from
    // item index to duration in seconds (duration = index + 1)
    private final static int NUM_DIRECT_DURATIONS = 10;

    private ImageButton mNextSlide;
    private ImageButton mPreSlide;
    private Button mPreview;
    private Button mReplaceImage;
    private Button mRemoveSlide;
    private EditText mTextEditor;
    private Button mDone;
    private BasicSlideEditorView mSlideView;

    private SlideshowModel mSlideshowModel;
    private SlideshowEditor mSlideshowEditor;
    private SlideshowPresenter mPresenter;
    private boolean mDirty;

    private int mPosition;
    private Uri mUri;

    private final static String MESSAGE_URI = "message_uri";
    private AsyncDialog mAsyncDialog;   // Used for background tasks.

    /// M: Code analyze 002, fix bug ALPS00331610 and new feature, support drm
    /// /sound file or ringrone/sizelimit show @{
    private final static int MENU_ADD_SD_SOUND      = 15;
    private TextView mTextView;
    private int mSizeLimit;
    private ImageView mDrmImageVideoLock;
    private ImageView mDrmAudioLock;
    public static final int REQUEST_CODE_ATTACH_SOUND     = 15;
    public static final int REQUEST_CODE_ATTACH_RINGTONE  = 20;
    private Uri mRestritedUri = null;
    private int mMediaType = REQUEST_CODE_EDIT_TEXT;
    /// @}

    private IOpSlideEditorActivityExt mOpSlideEditorActivityExt;

    /// M: fix bug ALPS00479024, used to count the preview button click times
    private boolean mIsCanPreview = true;

    private long mThreadId;

    /// M: disable when non-default sms
    private boolean mSmsEnable;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (PermissionCheckUtil.requestAllPermissions(this)) {
            return;
        }

        setContentView(R.layout.edit_slide_activity);
        /// M: Code analyze 003, fix bug ALPS00325381, check in plugin code(unknown) @{
        mOpSlideEditorActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpSlideEditorActivityExt();
        /// @}
        mSlideView = (BasicSlideEditorView) findViewById(R.id.slide_editor_view);
        mSlideView.setOnTextChangedListener(mOnTextChangedListener);
        mSlideView.setOnTouchListener(mOnScrollTouchListener);
        mPreSlide = (ImageButton) findViewById(R.id.pre_slide_button);
        mPreSlide.setOnClickListener(mOnNavigateBackward);

        mNextSlide = (ImageButton) findViewById(R.id.next_slide_button);
        mNextSlide.setOnClickListener(mOnNavigateForward);

        mPreview = (Button) findViewById(R.id.preview_button);
        mPreview.setOnClickListener(mOnPreview);

        mReplaceImage = (Button) findViewById(R.id.replace_image_button);
        mReplaceImage.setOnClickListener(mOnReplaceImage);

        mRemoveSlide = (Button) findViewById(R.id.remove_slide_button);
        mRemoveSlide.setOnClickListener(mOnRemoveSlide);

        mTextEditor = (EditText) findViewById(R.id.text_message);
        /// M: Code analyze 004, fix bug ALPS00305123, show toast when > specified-length @{
        mTextEditor.setFilters(new InputFilter[] {
                new TextLengthFilter(MmsConfig.getMaxTextLimit())});
        /// @}

        mDone = (Button) findViewById(R.id.done_button);
        mDone.setOnClickListener(mDoneClickListener);

        /// Code analyze 005, new feature, show size_info @{
        mTextView = (TextView) findViewById(R.id.media_size_info);
        mTextView.setVisibility(View.VISIBLE);

        MessageUtils.setMmsLimitSize(this);
        mSizeLimit = MmsConfig.getUserSetMmsSizeLimit(false);
        /// @}

        /// Code analyze 006, new feature, show Drm lock icon @{
        mDrmImageVideoLock = (ImageView) findViewById(R.id.drm_imagevideo_lock);
        mDrmAudioLock = (ImageView) findViewById(R.id.drm_audio_lock);
        /// @}

        mThreadId = getIntent().getLongExtra("thread_id", -1L);

        initActivityState(savedInstanceState, getIntent());

        /// M: Code analyze 007, fix bug ALPS00116011, update creation mode from preference @{
        WorkingMessage.updateCreationMode(this);
        /// @}
        try {
            mSlideshowModel = SlideshowModel.createFromMessageUri(this, mUri);
            // Confirm that we have at least 1 slide to display
            if (mSlideshowModel.size() == 0) {
                Log.e(TAG, "Loaded slideshow is empty; can't edit nothingness, exiting.");
                finish();
                return;
            }
            // Register an observer to watch whether the data model is changed.
            mSlideshowModel.registerModelChangedObserver(mModelChangedObserver);
            mSlideshowEditor = new SlideshowEditor(this, mSlideshowModel);
            mPresenter = (SlideshowPresenter) PresenterFactory.getPresenter(
                    "SlideshowPresenter", this, mSlideView, mSlideshowModel);

            // Sanitize mPosition
            if (mPosition >= mSlideshowModel.size()) {
                mPosition = Math.max(0, mSlideshowModel.size() - 1);
            } else if (mPosition < 0) {
                mPosition = 0;
            }

            showCurrentSlide();
            /// Code analyze 005, new feature, show size_info @{
            showSizeDisplay();
            /// @}
        } catch (MmsException e) {
            Log.e(TAG, "Create SlideshowModel failed!", e);
            finish();
            return;
        }
    }
    private void initActivityState(Bundle savedInstanceState, Intent intent) {
        if (savedInstanceState != null) {
            mUri = (Uri) savedInstanceState.getParcelable(MESSAGE_URI);
            mPosition = savedInstanceState.getInt(SLIDE_INDEX, 0);
            /// Code analyze 005, new feature, show size_info @{
            MessageUtils.setMmsLimitSize(this);
            intent.putExtra(SLIDE_INDEX, mPosition);
        } else {
            mUri = intent.getData();
            mPosition = intent.getIntExtra(SLIDE_INDEX, 0);
            MessageUtils.setMmsLimitSize(this);
            /// @}
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putInt(SLIDE_INDEX, mPosition);
        outState.putParcelable(MESSAGE_URI, mUri);
    }

    @Override
    protected void onStart() {
        super.onStart();

        if (!PermissionCheckUtil.checkAllPermissions(this)) {
            return;
        }

        /// M: Code analyze 008, new feature, set TextSize throungh mms_preference @{
        float textSize = MessageUtils.getPreferenceValueFloat(this,
                    SettingListActivity.TEXT_SIZE, 18);
        setTextSize(textSize);
        /// @}

        /// M: Code analyze 003, fix bug ALPS00325381, check in plugin code(unknown) @{
        mOpSlideEditorActivityExt.onStart(this, this);
        /// @}
    }

    @Override
    protected void onResume()  {
        super.onResume();
        showSizeDisplay();
        /// M: disable when non-default sms
        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        if (mSmsEnable != isSmsEnabled) {
            mSmsEnable = isSmsEnabled;
            invalidateOptionsMenu();
        }
        mPreview.setEnabled(isSmsEnabled);
        mRemoveSlide.setEnabled(isSmsEnabled);
        mReplaceImage.setEnabled(isSmsEnabled);
        mTextEditor.setEnabled(isSmsEnabled);
        mTextEditor.setFocusableInTouchMode(isSmsEnabled);
        mDone.setEnabled(isSmsEnabled);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO Auto-generated method stub
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mIsCanPreview = true;
        // remove any callback to display a progress spinner
        if (mAsyncDialog != null) {
            mAsyncDialog.clearPendingProgressDialog();
        }

        synchronized (this) {
            if (mDirty) {
                if (mSlideshowModel != null) {
                    mSlideshowModel.setNeedUpdate(true);
                    DraftManager.getInstance().update(DraftManager.SYNC_UPDATE_ACTION,
                            mThreadId, this, mUri, mSlideshowModel, null);

                    MmsLog.i(TAG, "onPause() Slideshow num = " + mSlideshowModel.size());
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mSlideshowModel != null) {
            mSlideshowModel.unregisterModelChangedObserver(
                    mModelChangedObserver);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (hasFocus) {
            mIsCanPreview = true;
        }
    }

    private final IModelChangedObserver mModelChangedObserver =
        new IModelChangedObserver() {
            public void onModelChanged(Model model, boolean dataChanged) {
                synchronized (SlideEditorActivity.this) {
                    mDirty = true;
                }
                setResult(RESULT_OK);
            }
        };

    private final OnClickListener mOnRemoveSlide = new OnClickListener() {
        public void onClick(View v) {
            // Validate mPosition
            if (mPosition >= 0 && mPosition < mSlideshowModel.size()) {
                mSlideshowEditor.removeSlide(mPosition);
                int size = mSlideshowModel.size();
                if (size > 0) {
                    if (mPosition >= size) {
                        mPosition--;
                    }
                    showCurrentSlide();
                    showSizeDisplay();
                    needRefreshMenu = true;
                } else {
                    /// Code analyze 009, new feature, TextLaout set to default
                    /// when remove all slide @{
                    mSlideshowEditor.changeLayout(LayoutModel.LAYOUT_BOTTOM_TEXT);
                    /// @}
                    finish();
                    return;
                }
            }
        }
    };

    private Handler mUiHandler = new Handler();
    private Runnable dialogRunnable = new Runnable() {
        public void run() {
            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                    R.string.exceed_message_size_limitation,
                    R.string.exceed_message_size_limitation, 0, 0);
            /// M: Delay to set text to avoid JE if replace text
            String text = mSlideshowEditor.getSlideTextOf(mPosition);
            mSlideView.setText("", text);
        }
    };

    private final OnTextChangedListener mOnTextChangedListener = new OnTextChangedListener() {
        public void onTextChanged(String s) {
            if (!isFinishing()) {
                /// Code analyze 010, new feature, catch Exception when change Text @{
                try {
                    mSlideshowEditor.changeText(mPosition, s);
                } catch (ExceedMessageSizeException e) {
                    mUiHandler.removeCallbacks(dialogRunnable);
                    mUiHandler.postDelayed(dialogRunnable, 300);
                }
                /// @}
                showSizeDisplay();
            }
        }
    };

    private final OnClickListener mOnPreview = new OnClickListener() {
        public void onClick(View v) {
            /// M: fix bug ALPS00479024, used to count the preview button click times
            if (mIsCanPreview) {
                previewSlideshow();
                mIsCanPreview = false;
            }
        }
    };

    private final OnClickListener mOnReplaceImage = new OnClickListener() {
        public void onClick(View v) {
            SlideModel slide = mSlideshowModel.get(mPosition);
            if (slide != null && slide.hasVideo()) {
                Toast.makeText(SlideEditorActivity.this, R.string.cannot_add_picture_and_video,
                        Toast.LENGTH_SHORT).show();
                return;
            }
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT, null);
            intent.setType(MmsContentType.IMAGE_UNSPECIFIED);
            /// M: Code analyze 011, fix bug ALPS00065732, let SD Drm file insert Mms @{
            if (FeatureOption.MTK_DRM_APP) {
                intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                        OmaDrmStore.DrmIntentExtra.LEVEL_SD);
            }
            /// @}
            startActivityForResult(intent, REQUEST_CODE_CHANGE_PICTURE);
            showSizeDisplay();
        }
    };

    private final OnClickListener mOnNavigateBackward = new OnClickListener() {
        public void onClick(View v) {
            if (mPosition > 0) {
                mPosition --;
                showCurrentSlide();
                showSizeDisplay();
                needRefreshMenu = true;
            }
        }
    };

    private final OnClickListener mOnNavigateForward = new OnClickListener() {
        public void onClick(View v) {
            if (mPosition < mSlideshowModel.size() - 1) {
                mPosition ++;
                showCurrentSlide();
                showSizeDisplay();
                needRefreshMenu = true;
            }
        }
    };

    private final OnClickListener mDoneClickListener = new OnClickListener() {
        public void onClick(View v) {
            /// M: Code analyze 012, fix bug ALPS00237809, hide SoftKeyBoard when click 'done' @{
            hideInputMethod();
            /// @}
            Intent data = new Intent();
            data.putExtra("done", true);
            setResult(RESULT_OK, data);
            finish();
        }
    };

    private AsyncDialog getAsyncDialog() {
        if (mAsyncDialog == null) {
            mAsyncDialog = new AsyncDialog(this);
        }
        return mAsyncDialog;
    }

    private void previewSlideshow() {
        MessageUtils.viewMmsMessageAttachment(SlideEditorActivity.this, mUri, mSlideshowModel,
                getAsyncDialog());
    }

    private void updateTitle() {
        setTitle(getString(R.string.slide_show_part, (mPosition + 1), mSlideshowModel.size()));
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (isFinishing()) {
            return false;
        }
        menu.clear();

        SlideModel slide = mSlideshowModel.get(mPosition);

        if (slide == null) {
            return false;
        }

        /// M: disable when non-default sms
        if (!MmsConfig.isSmsEnabled(this)) {
            return true;
        }

        // Preview slideshow.
        menu.add(0, MENU_PREVIEW_SLIDESHOW, 0, R.string.preview_slideshow).setIcon(
                com.android.internal.R.drawable.ic_menu_play_clip);

        // Text
        if (slide.hasText() && !TextUtils.isEmpty(slide.getText().getText())) {
            //"Change text" if text is set.
            menu.add(0, MENU_REMOVE_TEXT, 0, R.string.remove_text).setIcon(
                    R.drawable.ic_menu_remove_text);
        }

        // Picture
        if (slide.hasImage()) {
            menu.add(0, MENU_DEL_PICTURE, 0, R.string.remove_picture).setIcon(
                    R.drawable.ic_menu_remove_picture);
        } else if (!slide.hasVideo()) {
            menu.add(0, MENU_ADD_PICTURE, 0, R.string.add_picture).setIcon(
                    R.drawable.ic_menu_picture);
            menu.add(0, MENU_TAKE_PICTURE, 0, R.string.attach_take_photo).setIcon(
                    R.drawable.ic_menu_picture);
        }

        // Music
        if (slide.hasAudio()) {
            menu.add(0, MENU_DEL_AUDIO, 0, R.string.remove_music).setIcon(
                    R.drawable.ic_menu_remove_sound);
        } else if (!slide.hasVideo()) {
            if (MmsConfig.getAllowAttachAudio()) {
                SubMenu subMenu = menu.addSubMenu(0, MENU_SUB_AUDIO, 0, R.string.add_music)
                    .setIcon(R.drawable.ic_menu_add_sound);
                /// M: Code analyze 013, new feature, add sound file and ringtone support @{
                subMenu.add(0, MENU_ADD_AUDIO, 0, R.string.attach_ringtone);
                subMenu.add(0, MENU_ADD_SD_SOUND, 0, R.string.attach_sound);
                /// @}
                if (MessageUtils.isSoundRecorderAvailable(this)) {
                    subMenu.add(0, MENU_RECORD_SOUND, 0, R.string.attach_record_sound);
                }
            } else {
                if (MessageUtils.isSoundRecorderAvailable(this)) {
                    menu.add(0, MENU_RECORD_SOUND, 0, R.string.attach_record_sound)
                        .setIcon(R.drawable.ic_menu_add_sound);
                }
            }
        }

        // Video
        if (slide.hasVideo()) {
            menu.add(0, MENU_DEL_VIDEO, 0, R.string.remove_video).setIcon(
                    R.drawable.ic_menu_remove_video);
        } else if (!slide.hasAudio() && !slide.hasImage()) {
            menu.add(0, MENU_ADD_VIDEO, 0, R.string.add_video).setIcon(R.drawable.ic_menu_movie);
            menu.add(0, MENU_TAKE_VIDEO, 0, R.string.attach_record_video)
                .setIcon(R.drawable.ic_menu_movie);
        }

        /// M: Code analyze 014, new feature, check MAX_SLIDE_NUM @{
        if (mSlideshowModel.size() < SlideshowEditor.MAX_SLIDE_NUM) {
        menu.add(0, MENU_ADD_SLIDE, 0, R.string.add_slide).setIcon(
                R.drawable.ic_menu_add_slide);
        }
        /// @}

        // Slide duration
        String duration = getResources().getString(R.string.duration_sec);
        /// M: Code analyze 015, new feature, Duration menu show format @{
        int pos = duration.indexOf("%s");
        duration = duration.substring(0, pos + 2) + ")";
        Context context = SlideEditorActivity.this;
        /// M: Code analyze 016, fix bug ALPS00116755, set smil par Duration as SlideDuration @{
        // if need according media file duration , use like this :
        // int dur = slide.getPlayDuration() / 1000;
        // if need according slide setting duration , use like this:
        int dur = (int) Math.ceil(slide.getDuration() / 1000.0);
        /// @}
        String format = context.getResources().getQuantityString(
                R.plurals.slide_duration, dur, dur);
        menu.add(0, MENU_DURATION, 0,
                duration.replace("%s", format)).setIcon(
                        R.drawable.ic_menu_duration);
        /// @}

        // Slide layout
        int resId;
        if (mSlideshowModel.getLayout().getLayoutType() == LayoutModel.LAYOUT_TOP_TEXT) {
            resId = R.string.layout_top;
        } else {
            resId = R.string.layout_bottom;
        }
        // FIXME: set correct icon when layout icon is available.
        menu.add(0, MENU_LAYOUT, 0, resId).setIcon(R.drawable.ic_menu_picture);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_PREVIEW_SLIDESHOW:
                previewSlideshow();
                break;

            case MENU_REMOVE_TEXT:
                SlideModel slide = mSlideshowModel.get(mPosition);
                if (slide != null) {
                    slide.removeText();
                }
                showSizeDisplay();
                break;

            case MENU_ADD_PICTURE:
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT, null);
                intent.setType(MmsContentType.IMAGE_UNSPECIFIED);
                /// M: Code analyze 011, fix bug ALPS00065732, let SD Drm file insert Mms @{
                if (FeatureOption.MTK_DRM_APP) {
                    intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                            OmaDrmStore.DrmIntentExtra.LEVEL_SD);
                }
                /// @}

                startActivityForResult(intent, REQUEST_CODE_CHANGE_PICTURE);
                break;

            case MENU_TAKE_PICTURE:
                MessageUtils.capturePicture(this, REQUEST_CODE_TAKE_PICTURE);
                break;

            case MENU_DEL_PICTURE:
                mSlideshowEditor.removeImage(mPosition);
                setReplaceButtonText(R.string.add_picture);
                showSizeDisplay();
                break;
            case MENU_RECORD_SOUND:
                slide = mSlideshowModel.get(mPosition);
                /// M: Code analyze 017, fix bug ALPS00261194,
                /// let recorder auto stop when the messaging reach limit @{
                long sizeLimit
                        = ComposeMessageActivity.computeAttachmentSizeLimit(mSlideshowModel, 0);
                if (sizeLimit > ComposeMessageActivity.MIN_SIZE_FOR_RECORD_AUDIO) {
                    MessageUtils.recordSound(this, REQUEST_CODE_RECORD_SOUND, sizeLimit);
                } else {
                    Toast.makeText(this,
                            getString(R.string.space_not_enough_for_audio),
                            Toast.LENGTH_SHORT).show();
                }
                /// @}
                break;
            /// M: Code analyze 013, new feature, add sound file and ringtone support @{
            case MENU_ADD_AUDIO:
                MessageUtils.selectRingtone(SlideEditorActivity.this,
                        REQUEST_CODE_ATTACH_RINGTONE);
                break;
            case MENU_ADD_SD_SOUND:
                if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                   Toast.makeText(SlideEditorActivity.this, getString(R.string.Insert_sdcard),
                           Toast.LENGTH_LONG).show();
                   break;
                } else {
                    MessageUtils.selectAudio(SlideEditorActivity.this, REQUEST_CODE_ATTACH_SOUND);
                }
                break;
            /// @}
            case MENU_DEL_AUDIO:
                mSlideshowEditor.removeAudio(mPosition);
                showSizeDisplay();
                break;

            case MENU_ADD_VIDEO:
                intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.setType(MmsContentType.VIDEO_UNSPECIFIED);
                intent.putExtra(Intent.EXTRA_LOCAL_ONLY, true);
                /// M: Code analyze 011, fix bug ALPS00065732, let SD Drm file insert Mms @{
                if (FeatureOption.MTK_DRM_APP) {
                    intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                            OmaDrmStore.DrmIntentExtra.LEVEL_SD);
                }
                /// @}

                startActivityForResult(intent, REQUEST_CODE_CHANGE_VIDEO);
                break;

            case MENU_TAKE_VIDEO:
                slide = mSlideshowModel.get(mPosition);
                sizeLimit = ComposeMessageActivity.computeAttachmentSizeLimit(mSlideshowModel, 0);
                /// M: Code analyze 018, fix bug ALPS00241584, VideoRereveSize must > 10K. @{
                if (sizeLimit > ComposeMessageActivity.MIN_SIZE_FOR_CAPTURE_VIDEO) {
                /// @}
                    MessageUtils.recordVideo(this, REQUEST_CODE_TAKE_VIDEO, sizeLimit);
                } else {
                    Toast.makeText(this,
                            getString(R.string.message_too_big_for_video),
                            Toast.LENGTH_SHORT).show();
                }
                break;

            case MENU_DEL_VIDEO:
                mSlideshowEditor.removeVideo(mPosition);
                showSizeDisplay();
                break;

            case MENU_ADD_SLIDE:
                mPosition++;
                if (mSlideshowEditor.addNewSlide(mPosition)) {
                    // add successfully
                    showCurrentSlide();
                    showSizeDisplay();
                } else {
                    // move position back
                    mPosition--;
                    Toast.makeText(this, R.string.cannot_add_slide_anymore,
                            Toast.LENGTH_SHORT).show();
                }
                break;

            case MENU_LAYOUT:
                showLayoutSelectorDialog();
                break;

            case MENU_DURATION:
                showDurationDialog();
                break;
        }
        showSizeDisplay();
        return true;
    }

    private void setReplaceButtonText(int text) {
        mReplaceImage.setText(text);
    }

    private void showDurationDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setIcon(R.drawable.ic_mms_duration);
        String title = getResources().getString(R.string.duration_selector_title);
        builder.setTitle(title + (mPosition + 1) + "/" + mSlideshowModel.size());

        /// M: add radio button in slide duration selection dialog. @{
        String[] items = getResources().getStringArray(R.array.select_dialog_items);
        ArrayAdapter arrayAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_single_choice, items);
        int checkedItem;
        final int dur = (int) Math.ceil(mSlideshowModel.get(mPosition).getDuration() / 1000.0);
        if ((dur > 0) && (dur <= NUM_DIRECT_DURATIONS)) {
            checkedItem = dur - 1;
        } else {
            checkedItem = NUM_DIRECT_DURATIONS;
        }

        builder.setSingleChoiceItems(arrayAdapter, checkedItem,
                new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                if ((which >= 0) && (which < NUM_DIRECT_DURATIONS)) {
                    mSlideshowEditor.changeDuration(
                            mPosition, (which + 1) * 1000);
                } else {
                    Intent intent = new Intent(SlideEditorActivity.this,
                            EditSlideDurationActivity.class);
                    intent.putExtra(EditSlideDurationActivity.SLIDE_INDEX, mPosition);
                    intent.putExtra(EditSlideDurationActivity.SLIDE_TOTAL,
                            mSlideshowModel.size());
                    intent.putExtra(EditSlideDurationActivity.SLIDE_DUR, dur); // in seconds
                    startActivityForResult(intent, REQUEST_CODE_CHANGE_DURATION);
                }
                dialog.dismiss();
            }
        });
        /// @}
        builder.show();
    }

    private void showLayoutSelectorDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setIcon(R.drawable.ic_mms_layout);

        String title = getResources().getString(R.string.layout_selector_title);
        builder.setTitle(title + (mPosition + 1) + "/" + mSlideshowModel.size());

        LayoutSelectorAdapter adapter = new LayoutSelectorAdapter(this);
        builder.setAdapter(adapter, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                /// M: disable when non-default sms
                if (!mSmsEnable) {
                    Toast.makeText(SlideEditorActivity.this,
                            R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
                    return;
                }
                switch (which) {
                    case 0: // Top text.
                        mSlideshowEditor.changeLayout(LayoutModel.LAYOUT_TOP_TEXT);
                        break;
                    case 1: // Bottom text.
                        mSlideshowEditor.changeLayout(LayoutModel.LAYOUT_BOTTOM_TEXT);
                        break;
                }
                dialog.dismiss();
            }
        });

        builder.show();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        /// M: fix JE when 3rd-app return null-intent
        if (resultCode != RESULT_OK) {
            return;
        }

        /// M: disable when non-default sms
        if (!MmsConfig.isSmsEnabled(this)) {
            Toast.makeText(SlideEditorActivity.this,
                    R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
            return;
        }

        switch(requestCode) {
            case REQUEST_CODE_EDIT_TEXT:
                // XXX where does this come from?  Action is probably not the
                // right place to have the text...
                /// Code analyze 010, new feature, catch Exception when change Text @{
                try {
                    mSlideshowEditor.changeText(mPosition, data.getAction());
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.exceed_message_size_limitation,
                            R.string.failed_to_add_media, 0, getAudioString());
                }
                /// @}
                showSizeDisplay();
                break;

            case REQUEST_CODE_TAKE_PICTURE:
                Uri pictureUri = null;
                boolean showError = false;
                try {
                    /// M: Code analyze 019, fix bug ALPS00117331, avoid JE happens
                    /// after take a photo in slide (unknown) @{

                    pictureUri = TempFileProvider.renameScrapFile(".jpg",
                                /** M: use time stamp as the unique name,mPosition can be the same
                                 *  it has bug, see CR515255
                                 */
                                System.currentTimeMillis() + "", this);
                                //Integer.toString(mPosition), this);

                    if (pictureUri == null) {
                        showError = true;
                    } else {
                        // Remove the old captured picture's thumbnail from the cache
                        MmsApp.getApplication().getThumbnailManager().removeThumbnail(pictureUri);

                        mSlideshowEditor.changeImage(mPosition, pictureUri);
                        setReplaceButtonText(R.string.replace_image);
                    }
                } catch (MmsException e) {
                    Log.e(TAG, "add image failed", e);
                    notifyUser("add picture failed");
                    Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getPictureString()),
                            Toast.LENGTH_SHORT).show();
                } catch (RestrictedResolutionException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.select_different_media_type,
                            R.string.image_resolution_too_large, 0, 0);
                } catch (UnsupportContentTypeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.unsupported_media_format,
                            R.string.select_different_media,
                            getPictureString(), getPictureString());
                } catch (ResolutionException e) {
                    MessageUtils.resizeImageAsync(this, pictureUri, new Handler(),
                            mResizeImageCallback, false);
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.resizeImageAsync(this, pictureUri, new Handler(),
                            mResizeImageCallback, false);
                /// M: Code analyze 020, new feature, addRestrictedMedia in Waring_Mode @{
                } catch (ContentRestrictionException e) {
                    addRestrictedMedia(pictureUri, requestCode, R.string.confirm_restricted_image);
                }
                /// @}
                showSizeDisplay();
                break;

            case REQUEST_CODE_CHANGE_PICTURE:
                try {
                    if (data == null) {
                        Log.e(TAG, "REQUEST_CODE_CHANGE_PICTURE, add image failed");
                        return;
                    }
                    mSlideshowEditor.changeImage(mPosition, data.getData());
                    setReplaceButtonText(R.string.replace_image);
                } catch (MmsException e) {
                    Log.e(TAG, "add image failed", e);
                    notifyUser("add picture failed");
                    Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getPictureString()),
                            Toast.LENGTH_SHORT).show();
                } catch (RestrictedResolutionException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.select_different_media_type,
                            R.string.image_resolution_too_large, 0, 0);
                } catch (UnsupportContentTypeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.unsupported_media_format,
                            R.string.select_different_media,
                            getPictureString(), getPictureString());
                } catch (ResolutionException e) {
                    MessageUtils.resizeImageAsync(this, data.getData(), new Handler(),
                            mResizeImageCallback, false);
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.resizeImageAsync(this, data.getData(), new Handler(),
                            mResizeImageCallback, false);
                /// M: Code analyze 020, new feature, addRestrictedMedia in Waring_Mode @{
                } catch (ContentRestrictionException e) {
                    addRestrictedMedia(data.getData(), requestCode,
                            R.string.confirm_restricted_image);
                }
                /// @}
                showSizeDisplay();
                break;

            /// M: Code analyze 013, new feature, add sound file and ringtone support @{
            case REQUEST_CODE_ATTACH_RINGTONE:
                Uri uri = (Uri) data.getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
                if (Settings.System.DEFAULT_RINGTONE_URI.equals(uri) || uri == null) {
                    break;
                }
            /// @}
                try {
                    mSlideshowEditor.changeAudio(mPosition, uri);
                    /// M: Code analyze 021, fix bug ALPS00267249,
                    /// Set audio name to BasicSlideEditorView @{
                    SlideModel mSlideModelTemp = mSlideshowModel.get(mPosition);
                    mSlideView.setAudio(null, mSlideModelTemp.getAudio().getSrc(), null);
                    /// @}
                } catch (MmsException e) {
                    Log.e(TAG, "add audio failed", e);
                    notifyUser("add music failed");
                    Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getAudioString()),
                            Toast.LENGTH_SHORT).show();
                } catch (UnsupportContentTypeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.unsupported_media_format,
                            R.string.select_different_media, getAudioString(), getAudioString());
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.exceed_message_size_limitation,
                            R.string.failed_to_add_media, 0, getAudioString());
                /// M: Code analyze 020, new feature, addRestrictedMedia in Waring_Mode @{
                } catch (ContentRestrictionException e) {
                    addRestrictedMedia(uri, requestCode, R.string.confirm_restricted_audio);
                }
                /// @}
                showSizeDisplay();
                break;
            case REQUEST_CODE_RECORD_SOUND:
            case REQUEST_CODE_ATTACH_SOUND:
                try {
                    mSlideshowEditor.changeAudio(mPosition, data.getData());
                    /// M: Code analyze 021, fix bug ALPS00267249,
                    /// Set audio name to BasicSlideEditorView @{
                    SlideModel mSlideModelTemp = mSlideshowModel.get(mPosition);
                    mSlideView.setAudio(null, mSlideModelTemp.getAudio().getSrc(), null);
                    /// @}
                } catch (MmsException e) {
                    Log.e(TAG, "add audio failed", e);
                    notifyUser("add music failed");
                    Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getAudioString()),
                            Toast.LENGTH_SHORT).show();
                } catch (UnsupportContentTypeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.unsupported_media_format,
                            R.string.select_different_media, getAudioString(), getAudioString());
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.exceed_message_size_limitation,
                            R.string.failed_to_add_media, 0, getAudioString());
                /// M: Code analyze 020, new feature, addRestrictedMedia in Waring_Mode @{
                } catch (ContentRestrictionException e) {
                    addRestrictedMedia(data.getData(),
                            requestCode, R.string.confirm_restricted_audio);
                } catch (Exception e) {
                   Log.e(TAG, "add audio failed because of runtime exception", e);
                   Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getAudioString()),
                            Toast.LENGTH_SHORT).show();
                }
                /// @}
                showSizeDisplay();
                break;

            case REQUEST_CODE_TAKE_VIDEO:
                try {
                    /// M: Code analyze 022, fix bug ALPS00241707, rename ScrapVideoFile
                    /// for pass a new uri to other app(camera) @{
                    Uri videoUri = TempFileProvider.renameScrapVideoFile(".3gp",
                            /** M: use time stamp as the unique name,mPosition can be the same
                             *  it has bug, see CR515255
                             */
                            System.currentTimeMillis() + "", this);
                            //Integer.toString(mPosition), this);
                    /// @}

                    mSlideshowEditor.changeVideo(mPosition, videoUri);
                } catch (MmsException e) {
                    notifyUser("add video failed");
                    Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getVideoString()),
                            Toast.LENGTH_SHORT).show();
                } catch (UnsupportContentTypeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.unsupported_media_format,
                            R.string.select_different_media, getVideoString(), getVideoString());
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.exceed_message_size_limitation,
                            R.string.failed_to_add_media, 0, getVideoString());
                } catch (Exception e) {
                    /// M: Code analyze 023, fix bug ALPS00297024, catch the exception
                    /// and back to compose @{
                    finishAndBack();
                    /// @}
                }
                break;

            case REQUEST_CODE_CHANGE_VIDEO:
                try {
                    if (data == null) {
                        Log.e(TAG, "REQUEST_CODE_CHANGE_VIDEO, add video failed");
                        return;
                    }
                    mSlideshowEditor.changeVideo(mPosition, data.getData());
                } catch (MmsException e) {
                    Log.e(TAG, "add video failed", e);
                    notifyUser("add video failed");
                    Toast.makeText(SlideEditorActivity.this,
                            getResourcesString(R.string.failed_to_add_media, getVideoString()),
                            Toast.LENGTH_SHORT).show();
                } catch (UnsupportContentTypeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.unsupported_media_format,
                            R.string.select_different_media, getVideoString(), getVideoString());
                } catch (ExceedMessageSizeException e) {
                    MessageUtils.showErrorDialog(SlideEditorActivity.this,
                            R.string.exceed_message_size_limitation,
                            R.string.failed_to_add_media, 0, getVideoString());
                /// M: Code analyze 020, new feature, addRestrictedMedia in Waring_Mode @{
                } catch (ContentRestrictionException e) {
                    addRestrictedMedia(data.getData(),
                            requestCode, R.string.confirm_restricted_video);
                } catch (Exception e) {
                    /// M: Code analyze 023, fix bug ALPS00297024, catch the exception
                    /// and back to compose @{
                    finishAndBack();
                    /// @}
                }
                /// @}
                showSizeDisplay();
                break;

            case REQUEST_CODE_CHANGE_DURATION:
                mSlideshowEditor.changeDuration(mPosition,
                    Integer.valueOf(data.getAction()) * 1000);
                break;
        }
        showSizeDisplay();
    }

    private final ResizeImageResultCallback mResizeImageCallback =
        new ResizeImageResultCallback() {
        public void onResizeResult(PduPart part, boolean append) {
            Context context = SlideEditorActivity.this;
            if (part == null) {
                Toast.makeText(SlideEditorActivity.this,
                        getResourcesString(R.string.failed_to_add_media, getPictureString()),
                        Toast.LENGTH_SHORT).show();
                return;
            }
            /// M: Code analyze 007, fix bug ALPS00116011, update creation mode from preference @{
            int createMode = WorkingMessage.sCreationMode;
            WorkingMessage.sCreationMode = 0;
            /// @}
            try {
                long messageId = ContentUris.parseId(mUri);
                MtkPduPersister persister = MtkPduPersister.getPduPersister(context);
                Uri newUri = persister.persistPart(part, messageId, null);
                /*Because pdu has been update, we should set mDirty = true
                for updating pdupart while onPause*/
                mDirty = true;
                mSlideshowEditor.changeImage(mPosition, newUri);
                setReplaceButtonText(R.string.replace_image);
            } catch (MmsException e) {
                notifyUser("add picture failed");
                Toast.makeText(SlideEditorActivity.this,
                        getResourcesString(R.string.failed_to_add_media, getPictureString()),
                        Toast.LENGTH_SHORT).show();
            } catch (UnsupportContentTypeException e) {
                MessageUtils.showErrorDialog(SlideEditorActivity.this,
                        R.string.unsupported_media_format,
                        R.string.select_different_media, getPictureString(), getPictureString());
            } catch (ResolutionException e) {
                MessageUtils.showErrorDialog(SlideEditorActivity.this,
                        R.string.failed_to_resize_image,
                        R.string.resize_image_error_information, 0, 0);
            } catch (ExceedMessageSizeException e) {
                MessageUtils.showErrorDialog(SlideEditorActivity.this,
                        R.string.exceed_message_size_limitation,
                        R.string.failed_to_add_media, 0, getPictureString());
            /// M: Code analyze 007, fix bug ALPS00116011, update creation mode from preference @{
            } finally {
                WorkingMessage.sCreationMode = createMode;
            }
            /// @}
            showSizeDisplay();
        }
    };

    private String getResourcesString(int id, int mediaNameId) {
        Resources r = getResources();
        String mediaName = r.getString(mediaNameId);
        return r.getString(id, mediaName);
    }

    private String getResourcesString(int id) {
        Resources r = getResources();
        return r.getString(id);
    }

    private int getAudioString() {
        return R.string.type_audio;
    }

    private int getPictureString() {
        return R.string.type_picture;
    }

    private int getVideoString() {
        return R.string.type_video;
    }

    private void notifyUser(String message) {
        if (LOCAL_LOGV) {
            Log.v(TAG, "notifyUser: message=" + message);
        }
    }

    private void showCurrentSlide() {
        mPresenter.setLocation(mPosition);
        mPresenter.present(null);
        updateTitle();
        /// Code analyze 006, new feature, show Drm lock icon @{
        DrmUtilsEx.showDrmLock(this, mSlideshowModel,
                mPosition, mDrmImageVideoLock, mDrmAudioLock);
        /// @}
        if (mSlideshowModel.get(mPosition).hasImage()) {
            setReplaceButtonText(R.string.replace_image);
        } else {
            setReplaceButtonText(R.string.add_picture);
        }
    }

    /// M: Code analyze 024, fix bug ALPS00259088, catch the ActivityNOtFoundException @{
    @Override
    public void startActivityForResult(Intent intent, int requestCode) {
        try {
            super.startActivityForResult(intent, requestCode);
        } catch (ActivityNotFoundException e) {
            Intent mNewIntent = Intent.createChooser(intent, null);
            super.startActivityForResult(mNewIntent, requestCode);
        }
    }
    /// @}

    /// M: Code analyze 004, fix bug ALPS00305123, show toast when > specified-length @{
    /**
    * This filter will constrain edits not to make the length of the text
    * greater than the specified- length.
    */
    class TextLengthFilter implements InputFilter {
        public TextLengthFilter(int max) {
            /// M: fix bug ALPS00689403, shouldn't - 1 like Composer
            mMaxLength = max;
        }

        public CharSequence filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {

            int keep = mMaxLength - (dest.length() - (dend - dstart));

            if (keep < (end - start)) {
                Toast.makeText(SlideEditorActivity.this,
                        R.string.exceed_editor_size_limitation,
                        Toast.LENGTH_SHORT).show();
            }

            if (keep <= 0) {
                return "";
            } else if (keep >= end - start) {
                return null; // keep original
            } else {
                return source.subSequence(start, start + keep);
            }
        }

        private int mMaxLength;
    }
    /// @}

    /// Code analyze 005, new feature, show size_info @{
    private void showSizeDisplay() {
        int showSize = (mSlideshowModel.getCurrentSlideshowSize() - 1) / 1024 + 1;
        mTextView.setText(showSize + "K/" + mSizeLimit + "K");
        DrmUtilsEx.showDrmLock(this, mSlideshowModel,
                mPosition, mDrmImageVideoLock, mDrmAudioLock);
    }
    /// @}

    /// M: Code analyze 023, fix bug ALPS00297024, catch the exception and back to compose @{
    private void finishAndBack() {
        hideInputMethod();
        Intent data = new Intent();
        data.putExtra("done", true);
        setResult(RESULT_OK, data);
        finish();
    }
    /// @}

    /// M: Code analyze 020, new feature, addRestrictedMedia in Waring_Mode @{
    private void addRestrictedMedia(Uri mediaUri, int type, int messageId) {
        mRestritedUri = mediaUri;
        mMediaType = type;
        if (WorkingMessage.sCreationMode == WorkingMessage.WARNING_TYPE) {
            new AlertDialog.Builder(this)
            .setTitle(R.string.unsupport_media_type)
            /// M: Code analyze 025, fix bug ALPS00241042, replace image
            /// with more clear image @{
            .setIconAttribute(android.R.attr.alertDialogIcon)
            /// @}
            .setMessage(messageId)
            .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                public final void onClick(DialogInterface dialog, int which) {
                    /// M: disable when non-default sms
                    if (!mSmsEnable) {
                        Toast.makeText(SlideEditorActivity.this,
                                R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    if (mRestritedUri == null || mMediaType == REQUEST_CODE_EDIT_TEXT) {
                        return;
                    }
                    int createMode = WorkingMessage.sCreationMode;
                    WorkingMessage.sCreationMode = 0;
                    switch (mMediaType) {
                    case REQUEST_CODE_TAKE_PICTURE:
                    case REQUEST_CODE_CHANGE_PICTURE: {
                         try {
                             mSlideshowEditor.changeImage(mPosition, mRestritedUri);
                             setReplaceButtonText(R.string.replace_image);
                         } catch (MmsException e) {
                             MmsLog.e(TAG, "add image failed", e);
                             notifyUser("add picture failed");
                             Toast.makeText(SlideEditorActivity.this,
                                     getResourcesString(R.string.failed_to_add_media,
                                             getPictureString()),
                                     Toast.LENGTH_SHORT).show();
                         } catch (UnsupportContentTypeException e) {
                             MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                     R.string.unsupported_media_format,
                                     R.string.select_different_media,
                                     getPictureString(), getPictureString());
                         } catch (ResolutionException e) {
                             MessageUtils.resizeImageAsync(SlideEditorActivity.this,
                                     mRestritedUri, new Handler(),
                                     mResizeImageCallback, false);
                         } catch (ExceedMessageSizeException e) {
                             MessageUtils.resizeImageAsync(SlideEditorActivity.this,
                                     mRestritedUri, new Handler(),
                                     mResizeImageCallback, false);
                         }
                         break;
                    }
                    /// M: Code analyze 013, new feature, add sound file and ringtone support @{
                    case REQUEST_CODE_ATTACH_RINGTONE:
                        try {
                            mSlideshowEditor.changeAudio(mPosition, mRestritedUri);
                        } catch (MmsException e) {
                            MmsLog.e(TAG, "add audio failed", e);
                            notifyUser("add music failed");
                            Toast.makeText(SlideEditorActivity.this,
                                    getResourcesString(R.string.failed_to_add_media,
                                            getAudioString()),
                                    Toast.LENGTH_SHORT).show();
                        } catch (UnsupportContentTypeException e) {
                            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                    R.string.unsupported_media_format,
                                    R.string.select_different_media, getAudioString(),
                                    getAudioString());
                        } catch (ExceedMessageSizeException e) {
                            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                    R.string.exceed_message_size_limitation,
                                    R.string.failed_to_add_media, 0, getAudioString());
                        }
                        break;
                    case REQUEST_CODE_RECORD_SOUND:
                    case REQUEST_CODE_ATTACH_SOUND: {
                        try {
                            mSlideshowEditor.changeAudio(mPosition, mRestritedUri);
                        } catch (MmsException e) {
                            MmsLog.e(TAG, "add audio failed", e);
                            notifyUser("add music failed");
                            Toast.makeText(SlideEditorActivity.this,
                                    getResourcesString(R.string.failed_to_add_media,
                                            getAudioString()),
                                    Toast.LENGTH_SHORT).show();
                        } catch (UnsupportContentTypeException e) {
                            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                    R.string.unsupported_media_format,
                                    R.string.select_different_media, getAudioString(),
                                    getAudioString());
                        } catch (ExceedMessageSizeException e) {
                            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                    R.string.exceed_message_size_limitation,
                                    R.string.failed_to_add_media, 0, getAudioString());
                        }
                        break;
                    }
                    /// @}
                    case REQUEST_CODE_CHANGE_VIDEO: {
                        try {
                            mSlideshowEditor.changeVideo(mPosition, mRestritedUri);
                        } catch (MmsException e) {
                            MmsLog.e(TAG, "add video failed", e);
                            notifyUser("add video failed");
                            Toast.makeText(SlideEditorActivity.this,
                                    getResourcesString(R.string.failed_to_add_media,
                                            getVideoString()),
                                    Toast.LENGTH_SHORT).show();
                        } catch (UnsupportContentTypeException e) {
                            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                    R.string.unsupported_media_format,
                                    R.string.select_different_media, getVideoString(),
                                    getVideoString());
                        } catch (ExceedMessageSizeException e) {
                            MessageUtils.showErrorDialog(SlideEditorActivity.this,
                                    R.string.exceed_message_size_limitation,
                                    R.string.failed_to_add_media, 0, getVideoString());
                        } catch (Exception e) {
                            /// M: Code analyze 023, fix bug ALPS00297024, catch the exception
                            /// and back to compose @{
                            finishAndBack();
                            /// @}
                        }
                        break;
                    }
                    default:
                        MmsLog.epi(TAG, "error Restricted Midea: dataUri=" + mRestritedUri);
                    }

                    WorkingMessage.sCreationMode = createMode;
                    /// Fix ALPS00450932, size should be updated for warning mode
                    showSizeDisplay();
                }
            })
            .setNegativeButton(android.R.string.cancel, null)
            .show();
        } else {
            switch (mMediaType) {
            case REQUEST_CODE_TAKE_PICTURE:
            case REQUEST_CODE_CHANGE_PICTURE: {
                 MessageUtils.showErrorDialog(SlideEditorActivity.this,
                         R.string.unsupported_media_format,
                         R.string.select_different_media, getPictureString(), getPictureString());
                 break;
            }
            /// M: Code analyze 013, new feature, add sound file and ringtone support @{
            case REQUEST_CODE_ATTACH_RINGTONE:
            case REQUEST_CODE_RECORD_SOUND:
            case REQUEST_CODE_ATTACH_SOUND: {
                MessageUtils.showErrorDialog(SlideEditorActivity.this,
                        R.string.unsupported_media_format,
                        R.string.select_different_media, getAudioString(), getAudioString());
                break;
            }
            /// @}
            case REQUEST_CODE_CHANGE_VIDEO: {
                MessageUtils.showErrorDialog(SlideEditorActivity.this,
                        R.string.unsupported_media_format,
                        R.string.select_different_media, getVideoString(), getVideoString());
            }
          }
        }
    }
    /// @}

    /// M: Code analyze 008, new feature, set TextSize throungh mms_preference @{
    public void setTextSize(float size) {
        if (mTextEditor != null) {
            mTextEditor.setTextSize(size);
        }
    }
    /// @}

    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean ret = false;
        if (mOpSlideEditorActivityExt != null) {
            ret = mOpSlideEditorActivityExt.dispatchTouchEvent(ev);
        }
        if (ret == false) {
            ret = super.dispatchTouchEvent(ev);
        }
        return ret;
    }
    /// @}

    /// M: Code analyze 012, fix bug ALPS00237809, hide SoftKeyBoard when click 'done' @{
    private void hideInputMethod() {
        InputMethodManager inputMethodManager =
            (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        if (this.getWindow() != null && this.getWindow().getCurrentFocus() != null) {
            inputMethodManager.hideSoftInputFromWindow(
                    this.getWindow().getCurrentFocus().getWindowToken(), 0);
        }
    }
    /// @}

    private final OnScrollTouchListener mOnScrollTouchListener = new OnScrollTouchListener() {
        public void onTouch(View v) {
            hideInputMethod();
        }
    };

    /// M: If options menu create first, it should be refresh. @{
    boolean needRefreshMenu = true;
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (needRefreshMenu && keyCode == KeyEvent.KEYCODE_MENU) {
            invalidateOptionsMenu();
            needRefreshMenu = false;
        }
        return super.onKeyDown(keyCode, event);
    }
    /// @}

    @Override
    protected void onRestart() {
        super.onRestart();
        if (PermissionCheckUtil.requestAllPermissions(this)) {
            MmsLog.d(TAG, "onRestart() requestAllPermissions return !!");
            return;
        }
    }

}
