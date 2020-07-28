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

package com.android.gallery3d.glrenderer;

import com.android.gallery3d.common.Utils;
import com.android.photos.data.GalleryBitmapPool;

import android.graphics.Bitmap;

//BitmapTexture is a texture whose content is specified by a fixed Bitmap.
//
//1. The default: The texture does not own the Bitmap. The user should make sure the Bitmap
//is valid during the texture's lifetime. When the texture is recycled, it
//does not free the Bitmap.
//2. If mNeedRecycleBitmap is true, Recycle the bitmap after
//uploaded texture for saving memory.
public class BitmapTexture extends UploadedTexture {
    protected Bitmap mContentBitmap;

    /// M: [MEMORY.ADD] Recycle the bitmap after uploaded texture for saving memory @{
    private boolean mNeedRecycleBitmap;
    /// @}

    public BitmapTexture(Bitmap bitmap) {
        this(bitmap, false);
    }

    /// M: [MEMORY.ADD] Recycle the bitmap after uploaded texture for saving memory @{
    /**
     * Constructor of BitmapTexture.
     *
     * @param freeBitmap
     *            If this is true, it means the bitmap will be recycled after
     *            uploaded to GPU
     * @param bitmap
     *            The bitmap of this texture
     */
    public BitmapTexture(boolean freeBitmap, Bitmap bitmap) {
        this(bitmap, false);
        mNeedRecycleBitmap = freeBitmap;
    }
    /// @}

    public BitmapTexture(Bitmap bitmap, boolean hasBorder) {
        super(hasBorder);
        Utils.assertTrue(bitmap != null && !bitmap.isRecycled());
        mContentBitmap = bitmap;
    }

    @Override
    protected void onFreeBitmap(Bitmap bitmap) {
        // Do nothing.
        /// M: [MEMORY.ADD] Recycle the bitmap after uploaded texture for saving memory @{
        if (mNeedRecycleBitmap && bitmap != null) {
            GalleryBitmapPool.getInstance().put(bitmap);
            bitmap = null;
        }
        /// @}
    }

    @Override
    protected Bitmap onGetBitmap() {
        return mContentBitmap;
    }

    public Bitmap getBitmap() {
        return mContentBitmap;
    }
}
