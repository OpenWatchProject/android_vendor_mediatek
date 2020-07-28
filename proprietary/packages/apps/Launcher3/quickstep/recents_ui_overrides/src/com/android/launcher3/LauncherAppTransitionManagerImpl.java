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
 * limitations under the License.
 */

package com.android.launcher3;

import static com.android.launcher3.LauncherAnimUtils.VIEW_TRANSLATE_X;
import static com.android.launcher3.LauncherState.NORMAL;
import static com.android.launcher3.allapps.AllAppsTransitionController.ALL_APPS_PROGRESS;
import static com.android.launcher3.anim.Interpolators.AGGRESSIVE_EASE;
import static com.android.launcher3.anim.Interpolators.EXAGGERATED_EASE;
import static com.android.launcher3.anim.Interpolators.LINEAR;
import static com.android.launcher3.views.FloatingIconView.SHAPE_PROGRESS_DURATION;
import static com.android.quickstep.TaskViewUtils.findTaskViewToLaunch;
import static com.android.quickstep.TaskViewUtils.getRecentsWindowAnimator;
import static com.android.systemui.shared.system.QuickStepContract.getWindowCornerRadius;
import static com.android.systemui.shared.system.RemoteAnimationTargetCompat.MODE_CLOSING;
import static com.android.systemui.shared.system.RemoteAnimationTargetCompat.MODE_OPENING;

import static androidx.dynamicanimation.animation.DynamicAnimation.MIN_VISIBLE_CHANGE_PIXELS;
import static androidx.dynamicanimation.animation.SpringForce.DAMPING_RATIO_MEDIUM_BOUNCY;
import static androidx.dynamicanimation.animation.SpringForce.STIFFNESS_MEDIUM;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.app.ActivityOptions;
import android.content.Context;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.util.Pair;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.android.launcher3.allapps.AllAppsTransitionController;
import com.android.launcher3.anim.AnimatorPlaybackController;
import com.android.launcher3.anim.Interpolators;
import com.android.launcher3.shortcuts.DeepShortcutView;
import com.android.launcher3.views.FloatingIconView;
import com.android.launcher3.anim.SpringObjectAnimator;
import com.android.quickstep.util.ClipAnimationHelper;
import com.android.quickstep.util.MultiValueUpdateListener;
import com.android.quickstep.util.RemoteAnimationProvider;
import com.android.quickstep.util.RemoteAnimationTargetSet;
import com.android.quickstep.views.RecentsView;
import com.android.quickstep.views.TaskView;
import com.android.systemui.shared.system.ActivityOptionsCompat;
import com.android.systemui.shared.system.RemoteAnimationAdapterCompat;
import com.android.systemui.shared.system.RemoteAnimationRunnerCompat;
import com.android.systemui.shared.system.RemoteAnimationTargetCompat;
import com.android.systemui.shared.system.SyncRtSurfaceTransactionApplierCompat;
import com.android.systemui.shared.system.SyncRtSurfaceTransactionApplierCompat.SurfaceParams;

/**
 * A {@link QuickstepAppTransitionManagerImpl} that also implements recents transitions from
 * {@link RecentsView}.
 */
public final class LauncherAppTransitionManagerImpl extends QuickstepAppTransitionManagerImpl {

    public static final int INDEX_SHELF_ANIM = 0;
    public static final int INDEX_RECENTS_FADE_ANIM = 1;
    public static final int INDEX_RECENTS_TRANSLATE_X_ANIM = 2;

    public LauncherAppTransitionManagerImpl(Context context) {
        super(context);
    }

    /**
     * @return ActivityOptions with remote animations that controls how the window of the opening
     *         targets are displayed.
     */
    @Override
    public ActivityOptions getActivityLaunchOptions(Launcher launcher, View v) {
        if (hasControlRemoteAppTransitionPermission()) {
            boolean fromRecents = isLaunchingFromRecents(v, null /* targets */);
            RemoteAnimationRunnerCompat runner = new LauncherAnimationRunner(mHandler,
                true /* startAtFrontOfQueue */) {

                @Override
                public void onCreateAnimation(RemoteAnimationTargetCompat[] targetCompats,
                    AnimationResult result) {
                    AnimatorSet anim = new AnimatorSet();

                    boolean launcherClosing =
                        launcherIsATargetWithMode(targetCompats, MODE_CLOSING);

                    if (isLaunchingFromRecents(v, targetCompats)) {
                        composeRecentsLaunchAnimator(anim, v, targetCompats, launcherClosing);
                    } else {
                        composeIconLaunchAnimator(anim, v, targetCompats, launcherClosing);
                    }

                    if (launcherClosing) {
                        anim.addListener(mForceInvisibleListener);
                    }

                    result.setAnimation(anim);
                }
            };

            // Note that this duration is a guess as we do not know if the animation will be a
            // recents launch or not for sure until we know the opening app targets.
            long duration = fromRecents
                ? RECENTS_LAUNCH_DURATION
                : APP_LAUNCH_DURATION;

            long statusBarTransitionDelay = duration - STATUS_BAR_TRANSITION_DURATION
                - STATUS_BAR_TRANSITION_PRE_DELAY;
            return ActivityOptionsCompat.makeRemoteAnimation(new RemoteAnimationAdapterCompat(
                runner, duration, statusBarTransitionDelay));
        }
        return super.getActivityLaunchOptions(launcher, v);
    }

    @Override
    private boolean isLaunchingFromRecents(@NonNull View v,
            @Nullable RemoteAnimationTargetCompat[] targets) {
        return mLauncher.getStateManager().getState().overviewUi
                && findTaskViewToLaunch(mLauncher, v, targets) != null;
    }

    @Override
    protected void composeRecentsLaunchAnimator(@NonNull AnimatorSet anim, @NonNull View v,
            @NonNull RemoteAnimationTargetCompat[] targets, boolean launcherClosing) {
        RecentsView recentsView = mLauncher.getOverviewPanel();
        boolean skipLauncherChanges = !launcherClosing;

        TaskView taskView = findTaskViewToLaunch(mLauncher, v, targets);

        ClipAnimationHelper helper = new ClipAnimationHelper(mLauncher);
        anim.play(getRecentsWindowAnimator(taskView, skipLauncherChanges, targets, helper)
                .setDuration(RECENTS_LAUNCH_DURATION));

        Animator childStateAnimation = null;
        // Found a visible recents task that matches the opening app, lets launch the app from there
        Animator launcherAnim;
        final AnimatorListenerAdapter windowAnimEndListener;
        if (launcherClosing) {
            launcherAnim = recentsView.createAdjacentPageAnimForTaskLaunch(taskView, helper);
            launcherAnim.setInterpolator(Interpolators.TOUCH_RESPONSE_INTERPOLATOR);
            launcherAnim.setDuration(RECENTS_LAUNCH_DURATION);

            // Make sure recents gets fixed up by resetting task alphas and scales, etc.
            windowAnimEndListener = new AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(Animator animation) {
                    mLauncher.getStateManager().moveToRestState();
                    mLauncher.getStateManager().reapplyState();
                }
            };
        } else {
            AnimatorPlaybackController controller =
                    mLauncher.getStateManager().createAnimationToNewWorkspace(NORMAL,
                            RECENTS_LAUNCH_DURATION);
            controller.dispatchOnStart();
            childStateAnimation = controller.getTarget();
            launcherAnim = controller.getAnimationPlayer().setDuration(RECENTS_LAUNCH_DURATION);
            windowAnimEndListener = new AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(Animator animation) {
                    mLauncher.getStateManager().goToState(NORMAL, false);
                }
            };
        }
        anim.play(launcherAnim);

        // Set the current animation first, before adding windowAnimEndListener. Setting current
        // animation adds some listeners which need to be called before windowAnimEndListener
        // (the ordering of listeners matter in this case).
        mLauncher.getStateManager().setCurrentAnimation(anim, childStateAnimation);
        anim.addListener(windowAnimEndListener);
    }

    /**
     * Compose the animations for a launch from the app icon.
     *
     * @param anim the animation to add to
     * @param v the launching view with the icon
     * @param targets the list of opening/closing apps
     * @param launcherClosing true if launcher is closing
     */
    private void composeIconLaunchAnimator(@NonNull AnimatorSet anim, @NonNull View v,
        @NonNull RemoteAnimationTargetCompat[] targets, boolean launcherClosing) {
        // Set the state animation first so that any state listeners are called
        // before our internal listeners.
        mLauncher.getStateManager().setCurrentAnimation(anim);

        Rect windowTargetBounds = getWindowTargetBounds(targets);
        boolean isAllOpeningTargetTrs = true;
        for (int i = 0; i < targets.length; i++) {
            RemoteAnimationTargetCompat target = targets[i];
            if (target.mode == MODE_OPENING) {
                isAllOpeningTargetTrs &= target.isTranslucent;
            }
            if (!isAllOpeningTargetTrs) break;
        }
        anim.play(getOpeningWindowAnimators(v, targets, windowTargetBounds,
            !isAllOpeningTargetTrs));
        if (launcherClosing) {
            Pair<AnimatorSet, Runnable> launcherContentAnimator =
                getLauncherContentAnimator(true /* isAppOpening */,
                    new float[] {0, -mContentTransY});
            anim.play(launcherContentAnimator.first);
            anim.addListener(new AnimatorListenerAdapter() {
                @Override
                public void onAnimationEnd(Animator animation) {
                    launcherContentAnimator.second.run();
                }
            });
        }
    }

    /**
     * Return the window bounds of the opening target.
     * In multiwindow mode, we need to get the final size of the opening app window target to help
     * figure out where the floating view should animate to.
     */
    private Rect getWindowTargetBounds(RemoteAnimationTargetCompat[] targets) {
        Rect bounds = new Rect(0, 0, mDeviceProfile.widthPx, mDeviceProfile.heightPx);
        if (mLauncher.isInMultiWindowMode()) {
            for (RemoteAnimationTargetCompat target : targets) {
                if (target.mode == MODE_OPENING) {
                    bounds.set(target.sourceContainerBounds);
                    bounds.offsetTo(target.position.x, target.position.y);
                    return bounds;
                }
            }
        }
        return bounds;
    }

    /**
     * @return Animator that controls the window of the opening targets.
     */
    private ValueAnimator getOpeningWindowAnimators(View v, RemoteAnimationTargetCompat[] targets,
        Rect windowTargetBounds, boolean toggleVisibility) {
        RectF bounds = new RectF();
        FloatingIconView floatingView = FloatingIconView.getFloatingIconView(mLauncher, v,
            toggleVisibility, bounds, true /* isOpening */);
        Rect crop = new Rect();
        Matrix matrix = new Matrix();

        RemoteAnimationTargetSet openingTargets = new RemoteAnimationTargetSet(targets,
            MODE_OPENING);
        SyncRtSurfaceTransactionApplierCompat surfaceApplier =
            new SyncRtSurfaceTransactionApplierCompat(floatingView);
        openingTargets.addDependentTransactionApplier(surfaceApplier);

        // Scale the app icon to take up the entire screen. This simplifies the math when
        // animating the app window position / scale.
        float smallestSize = Math.min(windowTargetBounds.height(), windowTargetBounds.width());
        float maxScaleX = smallestSize / bounds.width();
        float maxScaleY = smallestSize / bounds.height();
        float scale = Math.max(maxScaleX, maxScaleY);
        float startScale = 1f;
        if (v instanceof BubbleTextView && !(v.getParent() instanceof DeepShortcutView)) {
            Drawable dr = ((BubbleTextView) v).getIcon();
            if (dr instanceof FastBitmapDrawable) {
                startScale = ((FastBitmapDrawable) dr).getAnimatedScale();
            }
        }
        final float initialStartScale = startScale;

        int[] dragLayerBounds = new int[2];
        mDragLayer.getLocationOnScreen(dragLayerBounds);

        // Animate the app icon to the center of the window bounds in screen coordinates.
        float centerX = windowTargetBounds.centerX() - dragLayerBounds[0];
        float centerY = windowTargetBounds.centerY() - dragLayerBounds[1];

        float dX = centerX - bounds.centerX();
        float dY = centerY - bounds.centerY();

        boolean useUpwardAnimation = bounds.top > centerY
            || Math.abs(dY) < mLauncher.getDeviceProfile().cellHeightPx;
        final long xDuration = useUpwardAnimation ? APP_LAUNCH_CURVED_DURATION
            : APP_LAUNCH_DOWN_DURATION;
        final long yDuration = useUpwardAnimation ? APP_LAUNCH_DURATION
            : APP_LAUNCH_DOWN_CURVED_DURATION;
        final long alphaDuration = useUpwardAnimation ? APP_LAUNCH_ALPHA_DURATION
            : APP_LAUNCH_ALPHA_DOWN_DURATION;

        RectF targetBounds = new RectF(windowTargetBounds);
        RectF currentBounds = new RectF();
        RectF temp = new RectF();

        ValueAnimator appAnimator = ValueAnimator.ofFloat(0, 1);
        appAnimator.setDuration(APP_LAUNCH_DURATION);
        appAnimator.setInterpolator(LINEAR);
        appAnimator.addListener(floatingView);
        appAnimator.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                if (v instanceof BubbleTextView) {
                    ((BubbleTextView) v).setStayPressed(false);
                }
                openingTargets.release();
            }
        });

        float shapeRevealDuration = APP_LAUNCH_DURATION * SHAPE_PROGRESS_DURATION;

        final float startCrop;
        final float endCrop;
        if (mDeviceProfile.isVerticalBarLayout()) {
            startCrop = windowTargetBounds.height();
            endCrop = windowTargetBounds.width();
        } else {
            startCrop = windowTargetBounds.width();
            endCrop = windowTargetBounds.height();
        }

        final float windowRadius = mDeviceProfile.isMultiWindowMode
            ? 0 : getWindowCornerRadius(mLauncher.getResources());
        appAnimator.addUpdateListener(new MultiValueUpdateListener() {
            FloatProp mDx = new FloatProp(0, dX, 0, xDuration, AGGRESSIVE_EASE);
            FloatProp mDy = new FloatProp(0, dY, 0, yDuration, AGGRESSIVE_EASE);
            FloatProp mIconScale = new FloatProp(initialStartScale, scale, 0, APP_LAUNCH_DURATION,
                EXAGGERATED_EASE);
            FloatProp mIconAlpha = new FloatProp(1f, 0f, APP_LAUNCH_ALPHA_START_DELAY,
                alphaDuration, LINEAR);
            FloatProp mCroppedSize = new FloatProp(startCrop, endCrop, 0, CROP_DURATION,
                EXAGGERATED_EASE);
            FloatProp mWindowRadius = new FloatProp(startCrop / 2f, windowRadius, 0,
                RADIUS_DURATION, EXAGGERATED_EASE);

            @Override
            public void onUpdate(float percent) {
                // Calculate app icon size.
                float iconWidth = bounds.width() * mIconScale.value;
                float iconHeight = bounds.height() * mIconScale.value;

                // Animate the window crop so that it starts off as a square.
                final int windowWidth;
                final int windowHeight;
                if (mDeviceProfile.isVerticalBarLayout()) {
                    windowWidth = (int) mCroppedSize.value;
                    windowHeight = windowTargetBounds.height();
                } else {
                    windowWidth = windowTargetBounds.width();
                    windowHeight = (int) mCroppedSize.value;
                }
                crop.set(0, 0, windowWidth, windowHeight);

                // Scale the app window to match the icon size.
                float scaleX = iconWidth / windowWidth;
                float scaleY = iconHeight / windowHeight;
                float scale = Math.min(1f, Math.max(scaleX, scaleY));

                float scaledWindowWidth = windowWidth * scale;
                float scaledWindowHeight = windowHeight * scale;

                float offsetX = (scaledWindowWidth - iconWidth) / 2;
                float offsetY = (scaledWindowHeight - iconHeight) / 2;

                // Calculate the window position
                temp.set(bounds);
                temp.offset(dragLayerBounds[0], dragLayerBounds[1]);
                temp.offset(mDx.value, mDy.value);
                Utilities.scaleRectFAboutCenter(temp, mIconScale.value);
                float transX0 = temp.left - offsetX;
                float transY0 = temp.top - offsetY;

                float croppedHeight = (windowTargetBounds.height() - crop.height()) * scale;
                float croppedWidth = (windowTargetBounds.width() - crop.width()) * scale;
                SurfaceParams[] params = new SurfaceParams[targets.length];
                for (int i = targets.length - 1; i >= 0; i--) {
                    RemoteAnimationTargetCompat target = targets[i];
                    Rect targetCrop;
                    final float alpha;
                    final float cornerRadius;
                    if (target.mode == MODE_OPENING) {
                        matrix.setScale(scale, scale);
                        matrix.postTranslate(transX0, transY0);
                        targetCrop = crop;
                        alpha = 1f - mIconAlpha.value;
                        cornerRadius = mWindowRadius.value;
                        matrix.mapRect(currentBounds, targetBounds);
                        if (mDeviceProfile.isVerticalBarLayout()) {
                            currentBounds.right -= croppedWidth;
                        } else {
                            currentBounds.bottom -= croppedHeight;
                        }
                        floatingView.update(currentBounds, mIconAlpha.value, percent, 0f,
                            cornerRadius * scale, true /* isOpening */);
                    } else {
                        matrix.setTranslate(target.position.x, target.position.y);
                        targetCrop = target.sourceContainerBounds;
                        alpha = 1f;
                        cornerRadius = 0;
                    }

                    params[i] = new SurfaceParams(target.leash, alpha, matrix, targetCrop,
                        RemoteAnimationProvider.getLayer(target, MODE_OPENING),
                        cornerRadius);
                }
                surfaceApplier.scheduleApply(params);
            }
        });
        return appAnimator;
    }

    @Override
    protected Runnable composeViewContentAnimator(@NonNull AnimatorSet anim, float[] alphas,
            float[] trans) {
        RecentsView overview = mLauncher.getOverviewPanel();
        ObjectAnimator alpha = ObjectAnimator.ofFloat(overview,
                RecentsView.CONTENT_ALPHA, alphas);
        alpha.setDuration(CONTENT_ALPHA_DURATION);
        alpha.setInterpolator(LINEAR);
        anim.play(alpha);
        overview.setFreezeViewVisibility(true);

        ObjectAnimator transY = ObjectAnimator.ofFloat(overview, View.TRANSLATION_Y, trans);
        transY.setInterpolator(AGGRESSIVE_EASE);
        transY.setDuration(CONTENT_TRANSLATION_DURATION);
        anim.play(transY);

        return () -> {
            overview.setFreezeViewVisibility(false);
            mLauncher.getStateManager().reapplyState();
        };
    }

    @Override
    public int getStateElementAnimationsCount() {
        return 3;
    }

    @Override
    public Animator createStateElementAnimation(int index, float... values) {
        switch (index) {
            case INDEX_SHELF_ANIM:
                return mLauncher.getAllAppsController().createSpringAnimation(values);
            case INDEX_RECENTS_FADE_ANIM:
                return ObjectAnimator.ofFloat(mLauncher.getOverviewPanel(),
                        RecentsView.CONTENT_ALPHA, values);
            case INDEX_RECENTS_TRANSLATE_X_ANIM:
                return new SpringObjectAnimator<>(mLauncher.getOverviewPanel(),
                        VIEW_TRANSLATE_X, MIN_VISIBLE_CHANGE_PIXELS, 0.8f, 250, values);
            default:
                return super.createStateElementAnimation(index, values);
        }
    }
}
