/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.incallui.videosurface.impl;

import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import com.android.dialer.common.LogUtil;
import com.android.incallui.videosurface.protocol.VideoSurfaceDelegate;
import com.android.incallui.videosurface.protocol.VideoSurfaceTexture;
import java.util.Locale;
import java.util.Objects;

/**
 * Represents a {@link TextureView} and its associated {@link SurfaceTexture} and {@link Surface}.
 * Used to manage the lifecycle of these objects across device orientation changes.
 */
public class VideoSurfaceTextureImpl implements VideoSurfaceTexture {
  @SurfaceType private final int surfaceType;
  private final boolean isPixel2017;

  private VideoSurfaceDelegate delegate;
  private TextureView textureView;
  private Surface savedSurface;
  private SurfaceTexture savedSurfaceTexture;
  private Point surfaceDimensions;
  private Point sourceVideoDimensions;
  private boolean isDoneWithSurface;

  public VideoSurfaceTextureImpl(boolean isPixel2017, @SurfaceType int surfaceType) {
    this.isPixel2017 = isPixel2017;
    this.surfaceType = surfaceType;
  }

  @Override
  public void setDelegate(VideoSurfaceDelegate delegate) {
    LogUtil.d("VideoSurfaceTextureImpl.setDelegate", "delegate: " + delegate + " " + toString());
    this.delegate = delegate;
  }

  @Override
  public int getSurfaceType() {
    return surfaceType;
  }

  @Override
  public Surface getSavedSurface() {
    return savedSurface;
  }

  @Override
  public void setSurfaceDimensions(Point surfaceDimensions) {
    LogUtil.i(
        "VideoSurfaceTextureImpl.setSurfaceDimensions",
        "surfaceDimensions: " + surfaceDimensions + " " + toString());
    this.surfaceDimensions = surfaceDimensions;
    if (surfaceDimensions != null && savedSurfaceTexture != null) {
      // Only do this on O (not at least O) because we expect this issue to be fixed in OMR1
      if (VERSION.SDK_INT == VERSION_CODES.O && isPixel2017) {
        LogUtil.i(
            "VideoSurfaceTextureImpl.setSurfaceDimensions",
            "skip setting default buffer size on Pixel 2017 ODR");
        return;
      }
      savedSurfaceTexture.setDefaultBufferSize(surfaceDimensions.x, surfaceDimensions.y);
    }
  }

  @Override
  public Point getSurfaceDimensions() {
    return surfaceDimensions;
  }

  @Override
  public void setSourceVideoDimensions(Point sourceVideoDimensions) {
    this.sourceVideoDimensions = sourceVideoDimensions;
  }

  @Override
  public Point getSourceVideoDimensions() {
    return sourceVideoDimensions;
  }

  @Override
  public void attachToTextureView(TextureView textureView) {
    if (this.textureView == textureView) {
      return;
    }
    LogUtil.d("VideoSurfaceTextureImpl.attachToTextureView", "textureview " +
        textureView + " " + toString());
    /// M:ALPS03519010 fix surfaceTexture destory by clear listner when exist active call and answer
    ///MT call.@{
    /* if (this.textureView != null) {
      this.textureView.setOnClickListener(null);
      this.textureView.setSurfaceTextureListener(null);
    } */
    ///@}

    this.textureView = textureView;
    textureView.setSurfaceTextureListener(new SurfaceTextureListener());
    textureView.setOnClickListener(new OnClickListener());

    boolean areSameSurfaces = Objects.equals(savedSurfaceTexture, textureView.getSurfaceTexture());
    LogUtil.d("VideoSurfaceTextureImpl.attachToTextureView", "areSameSurfaces: " + areSameSurfaces
            + "savedSurfaceTexture released: "
            + (savedSurfaceTexture == null ? "null" : savedSurfaceTexture.isReleased())
            + "isDone: " + isDoneWithSurface);
    /// M: ALPS03541124 check savedSurfaceTexture release status before use. @{
    // if (savedSurfaceTexture != null && !areSameSurfaces ) {
    if (savedSurfaceTexture != null && !areSameSurfaces && !savedSurfaceTexture.isReleased()) {
    /// @}
      textureView.setSurfaceTexture(savedSurfaceTexture);
      if (surfaceDimensions != null && createSurface(surfaceDimensions.x, surfaceDimensions.y)) {
        onSurfaceCreated();
      }
    }
    isDoneWithSurface = false;
  }

  @Override
  public void setDoneWithSurface() {
    LogUtil.i("VideoSurfaceTextureImpl.setDoneWithSurface", toString());
    isDoneWithSurface = true;
    if (textureView != null && textureView.isAvailable()) {
      return;
    }
    if (savedSurface != null) {
      onSurfaceReleased();
      savedSurface.release();
      savedSurface = null;
    }
    if (savedSurfaceTexture != null) {
      savedSurfaceTexture.release();
      savedSurfaceTexture = null;
    }
  }

  private boolean createSurface(int width, int height) {
    LogUtil.i(
        "VideoSurfaceTextureImpl.createSurface",
        "width: " + width + ", height: " + height + " " + toString());
    savedSurfaceTexture.setDefaultBufferSize(width, height);
    if (savedSurface != null) {
      savedSurface.release();
    }
    savedSurface = new Surface(savedSurfaceTexture);
    return true;
  }

  private void onSurfaceCreated() {
    if (delegate != null) {
      delegate.onSurfaceCreated(this);
    } else {
      LogUtil.e("VideoSurfaceTextureImpl.onSurfaceCreated", "delegate is null. " + toString());
    }
  }

  private void onSurfaceReleased() {
    if (delegate != null) {
      delegate.onSurfaceReleased(this);
    } else {
      LogUtil.e("VideoSurfaceTextureImpl.onSurfaceReleased", "delegate is null. " + toString());
    }
  }

  @Override
  public String toString() {
    return String.format(
        Locale.US,
        "VideoSurfaceTextureImpl<%s%s%s%s>",
        (surfaceType == SURFACE_TYPE_LOCAL ? "local, " : "remote, "),
        (savedSurface == null ? "no-surface, " : ""),
        (savedSurfaceTexture == null ? "no-texture, " : ""),
        (surfaceDimensions == null
            ? "(-1 x -1)"
            : (surfaceDimensions.x + " x " + surfaceDimensions.y)));
  }

  private class SurfaceTextureListener implements TextureView.SurfaceTextureListener {
    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture newSurfaceTexture, int width, int height) {
      LogUtil.d(
          "SurfaceTextureListener.onSurfaceTextureAvailable",
          "newSurfaceTexture: "
              + newSurfaceTexture
              + " "
              + VideoSurfaceTextureImpl.this.toString()
              + "savedSurfaceTexture released: "
              + (savedSurfaceTexture == null ? "null" : savedSurfaceTexture.isReleased()));

      // Where there is no saved {@link SurfaceTexture} available, use the newly created one.
      // If a saved {@link SurfaceTexture} is available, we are re-creating after an
      // orientation change.
      boolean surfaceCreated;
      if (savedSurfaceTexture == null
        /// M: ALPS03515693 Fix surfacetexture released by view @{
        || savedSurfaceTexture.isReleased()) {
        /// @}
        savedSurfaceTexture = newSurfaceTexture;
        surfaceCreated = createSurface(width, height);
      } else {
        // A saved SurfaceTexture was found.
        LogUtil.i(
            "SurfaceTextureListener.onSurfaceTextureAvailable", "replacing with cached surface...");
        textureView.setSurfaceTexture(savedSurfaceTexture);
        surfaceCreated = true;
      }

      // Inform the delegate that the surface is available.
      if (surfaceCreated) {
        onSurfaceCreated();
      }
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture destroyedSurfaceTexture) {
      LogUtil.i(
          "SurfaceTextureListener.onSurfaceTextureDestroyed",
          "destroyedSurfaceTexture: %s, %s, isDoneWithSurface: %b",
          destroyedSurfaceTexture,
          VideoSurfaceTextureImpl.this.toString(),
          isDoneWithSurface);
      if (delegate != null) {
        delegate.onSurfaceDestroyed(VideoSurfaceTextureImpl.this);
      } else {
        LogUtil.e("SurfaceTextureListener.onSurfaceTextureDestroyed", "delegate is null");
      }

      if (isDoneWithSurface) {
        onSurfaceReleased();
        if (savedSurface != null) {
          savedSurface.release();
          savedSurface = null;
        }
      }
      return isDoneWithSurface;
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
      /// M: ALPS03796906, the first frame is blurred after upgrade. @{
      LogUtil.i("SurfaceTextureListener.onSurfaceTextureSizeChanged",
          "surfaceId=" + surfaceType + ", w:" + width + "h:" + height);
      if (surfaceType == SURFACE_TYPE_LOCAL
          && surfaceDimensions != null
          && (width != surfaceDimensions.x || height != surfaceDimensions.y)) {
        setSurfaceDimensions(surfaceDimensions);
      }
      /// @}
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {}
  }

  private class OnClickListener implements View.OnClickListener {
    @Override
    public void onClick(View view) {
      if (delegate != null) {
        delegate.onSurfaceClick(VideoSurfaceTextureImpl.this);
      } else {
        LogUtil.e("OnClickListener.onClick", "delegate is null");
      }
    }
  }

  /// M: ALPS04808364, After back to background, can not get correct bitmap from textureView,
  /// and the bitmap have been cleared before.
  /// InCall will use cached bitmap to show blur image. @{
  @Override
  public TextureView getTextureView() {
    return textureView;
  }
  /// @}
}
