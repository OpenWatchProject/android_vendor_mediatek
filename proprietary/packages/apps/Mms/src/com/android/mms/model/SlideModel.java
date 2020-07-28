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

package com.android.mms.model;

import android.text.TextUtils;
import android.util.Config;
import android.util.Log;

import com.android.mms.ContentRestrictionException;
import com.android.mms.dom.smil.SmilParElementImpl;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.callback.IMediaModelCallback;
import com.mediatek.mms.callback.ISlideModelCallback;
import com.mediatek.mms.util.DrmUtilsEx;

import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventListener;
import org.w3c.dom.smil.ElementTime;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.concurrent.CopyOnWriteArrayList;


public class SlideModel extends Model
        implements List<MediaModel>, EventListener, ISlideModelCallback {
    public static final String TAG = "Mms/slideshow";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = DEBUG ? Config.LOGD : Config.LOGV;
    private static final int DEFAULT_SLIDE_DURATION = 5000;

    /// M: change google default.
    private final CopyOnWriteArrayList<MediaModel> mMedia = new CopyOnWriteArrayList<MediaModel>();

    private MediaModel mText;
    private MediaModel mImage;
    private MediaModel mAudio;
    private MediaModel mVideo;

    private boolean mCanAddImage = true;
    private boolean mCanAddAudio = true;
    private boolean mCanAddVideo = true;

    private int mDuration;
    private boolean mVisible = true;
    private short mFill;
    private int mSlideSize;
    private SlideshowModel mParent;

    public SlideModel(SlideshowModel slideshow) {
        this(DEFAULT_SLIDE_DURATION, slideshow);
    }

    public SlideModel(int duration, SlideshowModel slideshow) {
        mDuration = duration;
        mParent = slideshow;
    }

    /**
     * Create a SlideModel with exist media collection.
     *
     * @param duration The duration of the slide.
     * @param mediaList The exist media collection.
     *
     * @throws IllegalStateException One or more media in the mediaList cannot
     *         be added into the slide due to a slide cannot contain image
     *         and video or audio and video at the same time.
     */
    public SlideModel(int duration, ArrayList<MediaModel> mediaList) {
        mDuration = duration;

        int maxDur = 0;
        for (MediaModel media : mediaList) {
            internalAdd(media);

            int mediaDur = media.getDuration();
            if (mediaDur > maxDur) {
                maxDur = mediaDur;
            }
        }

        updateDuration(maxDur);
    }

    private void internalAdd(MediaModel media) throws IllegalStateException {
        /// M: Code analyze 002, Fix bug ALPS00244940, seldom issue and unfixed @{
        /// @}
        if (media == null) {
            /// M: @{
            MmsLog.w(TAG, "internal add media. Media is null");
            Thread.dumpStack();
            /// @}
            // Don't add null value into the list.
            return;
        }

        if (media.isText()) {
            String contentType = media.getContentType();
            if (TextUtils.isEmpty(contentType) || MmsContentType.TEXT_PLAIN.equals(contentType)
                    || MmsContentType.TEXT_HTML.equals(contentType)) {
                internalAddOrReplace(mText, media);
                mText = media;
            } else {
                Log.w(TAG, "[SlideModel] content type " + media.getContentType() +
                        " isn't supported (as text)");
            }
        } else if (media.isImage()) {
            if (mCanAddImage) {
                internalAddOrReplace(mImage, media);
                mImage = media;
                mCanAddVideo = false;
            } else {
                Log.w(TAG, "[SlideModel] content type " + media.getContentType() +
                    " - can't add image in this state");
            }
        } else if (media.isAudio()) {
            if (mCanAddAudio) {
                internalAddOrReplace(mAudio, media);
                mAudio = media;
                mCanAddVideo = false;
            } else {
                Log.w(TAG, "[SlideModel] content type " + media.getContentType() +
                    " - can't add audio in this state");
            }
        } else if (media.isVideo()) {
            if (mCanAddVideo) {
                internalAddOrReplace(mVideo, media);
                mVideo = media;
                mCanAddImage = false;
                mCanAddAudio = false;
            } else {
                Log.w(TAG, "[SlideModel] content type " + media.getContentType() +
                    " - can't add video in this state");
            }
        }
    }

    private void internalAddOrReplace(MediaModel old, MediaModel media) {
        // If the media is resizable, at this point consider it to be zero length.
        // Just before we send the slideshow, we take the remaining space in the
        // slideshow and equally allocate it to all the resizeable media items and resize them.
        /// M: Code analyze 003, new feature, Slide MediaModel size must compute Smil_tag_size @{
        int addSize = /*media.getMediaResizable() ? 0 : */media.getMediaPackagedSize();
        int removeSize;
        if (old == null) {
            if (null != mParent) {
                mParent.checkMessageSize(addSize);
            }
            mMedia.add(media);
            increaseSlideSize(addSize);
            increaseMessageSize(addSize);
        } else {
            removeSize = old.getMediaPackagedSize();
            if (addSize > removeSize) {
                if (null != mParent) {
                    mParent.checkMessageSize(addSize - removeSize);
                }
                increaseSlideSize(addSize - removeSize);
                increaseMessageSize(addSize - removeSize);
            } else {
                decreaseSlideSize(removeSize - addSize);
                decreaseMessageSize(removeSize - addSize);
            }
            /// M: Code analyze 004, fix bug ALPS00234350, as below  @{
            /// [Root cause]: when replace an old media, the old media does not exist in
            /// history. So, there is an exception occurred.
            /// The up is not the real root cause which has not been found.
            /// [Solution]: there is no an effective way to solve this CR.
            /// This solution is a temporary solution.
            int mOldMediaIndex = mMedia.indexOf(old);
            if (mOldMediaIndex == -1) {
                MmsLog.i(TAG, "There may be an exception for the old media not exist in history.");
                mMedia.add(media);
                increaseSlideSize(addSize);
                increaseMessageSize(addSize);
            }
            /// @}
            else {
                mMedia.set(mOldMediaIndex, media);
                old.unregisterAllModelChangedObservers();
            }
        }

        for (IModelChangedObserver observer : mModelChangedObservers) {
            media.registerModelChangedObserver(observer);
        }
    }

    private boolean internalRemove(Object object) {
        if (mMedia.remove(object)) {
            if (object instanceof TextModel) {
                mText = null;
            } else if (object instanceof ImageModel) {
                mImage = null;
                mCanAddVideo = true;
            } else if (object instanceof AudioModel) {
                mAudio = null;
                mCanAddVideo = true;
            } else if (object instanceof VideoModel) {
                mVideo = null;
                mCanAddImage = true;
                mCanAddAudio = true;
            }
            // If the media is resizable, at this point consider it to be zero length.
            // Just before we send the slideshow, we take the remaining space in the
            // slideshow and equally allocate it to all the resizeable media items and resize them.
            /// M: Code analyze 003
            int decreaseSize = /*((MediaModel) object).getMediaResizable() ? 0
                                        : */((MediaModel) object).getMediaPackagedSize();
            decreaseSlideSize(decreaseSize);
            decreaseMessageSize(decreaseSize);

            ((Model) object).unregisterAllModelChangedObservers();

            mNeedUpdate = true;
            return true;
        }

        return false;
    }

    /**
     * @return the mDuration
     */
    public int getDuration() {
        return mDuration;
    }

    /**
     * @param duration the mDuration to set
     */
    public void setDuration(int duration) {
        mDuration = duration;
        /// M: add log for bug ALPS01276998
        MmsLog.d(TAG, "setDuration: " + mDuration);
        notifyModelChanged(true);
        mNeedUpdate = true;
    }

    public int getSlideSize() {
        return mSlideSize;
    }

    public void increaseSlideSize(int increaseSize) {
        if (increaseSize > 0) {
            mSlideSize += increaseSize;
        }
    }

    public void decreaseSlideSize(int decreaseSize) {
        if (decreaseSize > 0) {
            mSlideSize -= decreaseSize;
            /// M: google JB.MR1 patch, fix JE in com.android.mms at android.os.AsyncTask done @{
            if (mSlideSize < 0) {
                mSlideSize = 0;
            }
            /// @}
        }
    }

    public void setParent(SlideshowModel parent) {
        mParent = parent;
    }

    public void increaseMessageSize(int increaseSize) {
        if ((increaseSize > 0) && (null != mParent)) {
            /// M: change google default.
            int size = mParent.getCurrentSlideshowSize();
            size += increaseSize;
            /// M: change google default.
            mParent.setCurrentSlideshowSize(size);
        }
    }

    public void decreaseMessageSize(int decreaseSize) {
        if ((decreaseSize > 0) && (null != mParent)) {
            /// M: change google default.
            int size = mParent.getCurrentSlideshowSize();
            size -= decreaseSize;
            /// M: @{
            if (size < 0) {
                size = 0;
            }
            /// @}
            /// M: change google default.
            mParent.setCurrentSlideshowSize(size);
        }
    }

    //
    // Implement List<E> interface.
    //

    /**
     * Add a MediaModel to the slide. If the slide has already contained
     * a media object in the same type, the media object will be replaced by
     * the new one.
     *
     * @param object A media object to be added into the slide.
     * @return true
     * @throws IllegalStateException One or more media in the mediaList cannot
     *         be added into the slide due to a slide cannot contain image
     *         and video or audio and video at the same time.
     * @throws ContentRestrictionException when can not add this object.
     *
     */
    public boolean add(MediaModel object) {
        internalAdd(object);
        notifyModelChanged(true);
        object.setNeedUpdate(true);
        return true;
    }

    public boolean addAll(Collection<? extends MediaModel> collection) {
        throw new UnsupportedOperationException("Operation not supported.");
    }
    /* Mem opt*/
    public void clear() {
        MmsLog.d(TAG, "SlideMoel.reset");
        if (mMedia.size() > 0) {
            for (MediaModel media : mMedia) {
                media.unregisterAllModelChangedObservers();
                /// M: change google default.
                int decreaseSize = media.getMediaPackagedSize();
                decreaseSlideSize(decreaseSize);
                decreaseMessageSize(decreaseSize);
            }
            mMedia.clear();
            if (mImage != null) {
                MmsLog.d(TAG, "reset mImage");
                mImage.resetRes();
            }
            mText = null;
            mImage = null;
            mAudio = null;
            mVideo = null;

            mCanAddImage = true;
            mCanAddAudio = true;
            mCanAddVideo = true;

            notifyModelChanged(true);
        }
    }

    public boolean contains(Object object) {
        return mMedia.contains(object);
    }

    public boolean containsAll(Collection<?> collection) {
        return mMedia.containsAll(collection);
    }

    public boolean isEmpty() {
        return mMedia.isEmpty();
    }

    public Iterator<MediaModel> iterator() {
        return mMedia.iterator();
    }

    public boolean remove(Object object) {
        if ((object != null) && (object instanceof MediaModel)
                && internalRemove(object)) {
            notifyModelChanged(true);
            return true;
        }
        return false;
    }

    public boolean removeAll(Collection<?> collection) {
        throw new UnsupportedOperationException("Operation not supported.");
    }

    public boolean retainAll(Collection<?> collection) {
        throw new UnsupportedOperationException("Operation not supported.");
    }

    public int size() {
        return mMedia.size();
    }

    public Object[] toArray() {
        return mMedia.toArray();
    }

    public <T> T[] toArray(T[] array) {
        return mMedia.toArray(array);
    }

    public void add(int location, MediaModel object) {
        throw new UnsupportedOperationException("Operation not supported.");
    }

    public boolean addAll(int location,
            Collection<? extends MediaModel> collection) {
        throw new UnsupportedOperationException("Operation not supported.");
    }

    public MediaModel get(int location) {
        if (mMedia.size() == 0) {
            return null;
        }

        return mMedia.get(location);
    }

    public int indexOf(Object object) {
        return mMedia.indexOf(object);
    }

    public int lastIndexOf(Object object) {
        return mMedia.lastIndexOf(object);
    }

    public ListIterator<MediaModel> listIterator() {
        return mMedia.listIterator();
    }

    public ListIterator<MediaModel> listIterator(int location) {
        return mMedia.listIterator(location);
    }

    public MediaModel remove(int location) {
        MediaModel media = mMedia.get(location);
        if ((media != null) && internalRemove(media)) {
            notifyModelChanged(true);
        }
        return media;
    }

    public MediaModel set(int location, MediaModel object) {
        throw new UnsupportedOperationException("Operation not supported.");
    }

    public List<MediaModel> subList(int start, int end) {
        return mMedia.subList(start, end);
    }

    /**
     * @return the mVisible
     */
    public boolean isVisible() {
        return mVisible;
    }

    /**
     * @param visible the mVisible to set
     */
    public void setVisible(boolean visible) {
        mVisible = visible;
        notifyModelChanged(true);
    }

    /**
     * @return the mFill
     */
    public short getFill() {
        return mFill;
    }

    /**
     * @param fill the mFill to set
     */
    public void setFill(short fill) {
        mFill = fill;
        notifyModelChanged(true);
    }

    @Override
    protected void registerModelChangedObserverInDescendants(
            IModelChangedObserver observer) {
        for (MediaModel media : mMedia) {
            media.registerModelChangedObserver(observer);
        }
    }

    @Override
    protected void unregisterModelChangedObserverInDescendants(
            IModelChangedObserver observer) {
        for (MediaModel media : mMedia) {
            media.unregisterModelChangedObserver(observer);
        }
    }

    @Override
    protected void unregisterAllModelChangedObserversInDescendants() {
        for (MediaModel media : mMedia) {
            media.unregisterAllModelChangedObservers();
        }
    }

    // EventListener Interface
    public void handleEvent(Event evt) {
        if (evt.getType().equals(SmilParElementImpl.SMIL_SLIDE_START_EVENT)) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Start to play slide: " + this);
            }
            mVisible = true;
        } else if (mFill != ElementTime.FILL_FREEZE) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Stop playing slide: " + this);
            }
            mVisible = false;
        }

        notifyModelChanged(false);
    }

    public boolean hasText() {
        return mText != null;
    }

    public boolean hasImage() {
        return mImage != null;
    }

    public boolean hasAudio() {
        return mAudio != null;
    }

    public boolean hasVideo() {
        return mVideo != null;
    }

    public boolean removeText() {
        return remove(mText);
    }

    public boolean removeImage() {
        return remove(mImage);
    }

    public boolean removeAudio() {
        boolean result = remove(mAudio);
        resetDuration();
        return result;
    }

    public boolean removeVideo() {
        boolean result = remove(mVideo);
        resetDuration();
        return result;
    }

    public TextModel getText() {
        return (TextModel) mText;
    }

    public ImageModel getImage() {
        return (ImageModel) mImage;
    }

    public AudioModel getAudio() {
        return (AudioModel) mAudio;
    }

    public VideoModel getVideo() {
        return (VideoModel) mVideo;
    }

    public void resetDuration() {
        // If we remove all the objects that have duration, reset the slide back to its
        // default duration. If we don't do this, if the user replaces a 10 sec video with
        // a 3 sec audio, the duration will remain at 10 sec (see the way updateDuration() below
        // works).
        if (!hasAudio() && !hasVideo()) {
            mDuration = DEFAULT_SLIDE_DURATION;
        }
    }

    public void updateDuration(int duration) {
        if (duration <= 0) {
            return;
        }

        if ((duration > mDuration)
                || (mDuration == DEFAULT_SLIDE_DURATION)) {
            mDuration = duration;
        }
    }

    /// M: Code analyze 006, new feature and useless, get the real duration of slide @{
    public int getPlayDuration() {
        int maxDur = 0;
        for (MediaModel media : mMedia) {
            int mediaDur = media.getDuration();
            if (mediaDur > maxDur) {
                maxDur = mediaDur;
            }
        }
        return maxDur > mDuration ? maxDur : mDuration;
    }
    /// @}

    @Override
    public boolean needUpdate() {
        if (mNeedUpdate) {
            return true;
        }

        int size = mMedia.size();
        for (int i = 0; i < size; i++) {
            Model model = mMedia.get(i);
            if (model.needUpdate()) {
                return true;
            }
        }
        return false;
    }

    public void resetUpdateState() {
        mNeedUpdate = false;
        int size = mMedia.size();
        for (int i = 0; i < size; i++) {
            Model model = mMedia.get(i);
            model.setNeedUpdate(false);
        }
    }

    public boolean hasDrmMedia() {
        return DrmUtilsEx.hasDrmMediaInSlide(this);
    }

    public boolean hasDrmMediaRight() {
        return DrmUtilsEx.hasDrmMediaRightInSlide(this);
    }

    public int getMediaModelSize() {
        return size();
    }
    public IMediaModelCallback getMediaModel(int i) {
        return get(i);
    }
}
