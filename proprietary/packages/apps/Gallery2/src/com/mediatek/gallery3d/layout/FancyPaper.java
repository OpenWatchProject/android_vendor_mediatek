package com.mediatek.gallery3d.layout;

import android.graphics.Rect;
import android.opengl.Matrix;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;

import com.android.gallery3d.common.Utils;
import com.android.gallery3d.ui.AnimationTime;
import com.android.gallery3d.ui.Paper;

/**
 * This class does the over scroll effect.
 */
public class FancyPaper extends Paper {
    @SuppressWarnings("unused")
    private static final String TAG = "MtkGallery2/FancyPaper";
    private static final int ROTATE_FACTOR = 4;
    private static final int MATRIX_COUNT = 16;
    private EdgeAnimation mAnimationUp = new EdgeAnimation();
    private EdgeAnimation mAnimationDown = new EdgeAnimation();
    private int mHeight;
    private float[] mMatrix = new float[MATRIX_COUNT];

    @Override
    public void overScroll(float distance) {
        distance /= mHeight;  // make it relative to height
        if (distance < 0) {
            mAnimationUp.onPull(-distance);
        } else {
            mAnimationDown.onPull(distance);
        }
    }

    @Override
    public void edgeReached(float velocity) {
        velocity /= mHeight;  // make it relative to height
        if (velocity < 0) {
            mAnimationDown.onAbsorb(-velocity);
        } else {
            mAnimationUp.onAbsorb(velocity);
        }
    }

    @Override
    public void onRelease() {
        mAnimationUp.onRelease();
        mAnimationDown.onRelease();
    }

    @Override
    public boolean advanceAnimation() {
        // Note that we use "|" because we want both animations get updated.
        return mAnimationUp.update() | mAnimationDown.update();
    }

    @Override
    public void setSize(int width, int height) {
        mHeight = height;
    }

    @Override
    public float[] getTransform(Rect rect, float scrollY) {
        float up = mAnimationUp.getValue();
        float down = mAnimationDown.getValue();
        float screenY = rect.centerY() - scrollY;

        float y = screenY + mHeight / 4;
        int range = 3 * mHeight / 2;
        float t = ((range - y) * up - y * down) / range;
        // compress t to the range (-1, 1) by the function
        // f(t) = (1 / (1 + e^-t) - 0.5) * 2
        // then multiply by 90 to make the range (-45, 45)
        float degrees =
            (1 / (1 + (float) Math.exp(-t * ROTATE_FACTOR)) - 0.5f) * 2 * 45;
        Matrix.setIdentityM(mMatrix, 0);
        Matrix.translateM(mMatrix, 0, mMatrix, 0, rect.centerX(), rect.centerY(), 0);
        //Matrix.rotateM(mMatrix, 0, degrees, 0, 1, 0);
        Matrix.rotateM(mMatrix, 0, degrees, 1, 0, 0);
        Matrix.translateM(mMatrix, 0, mMatrix, 0, -rect.width() / 2, -rect.height() / 2, 0);
        return mMatrix;
    }
}

/**
 * This class follows the structure of frameworks's EdgeEffect class.
 */
class EdgeAnimation {
    @SuppressWarnings("unused")
    private static final String TAG = "MtkGallery2/EdgeAnimation";

    private static final int STATE_IDLE = 0;
    private static final int STATE_PULL = 1;
    private static final int STATE_ABSORB = 2;
    private static final int STATE_RELEASE = 3;

    // Time it will take the effect to fully done in ms
    private static final int ABSORB_TIME = 200;
    private static final int RELEASE_TIME = 500;

    private static final float VELOCITY_FACTOR = 0.1f;

    private final Interpolator mInterpolator;

    private int mState;
    private float mValue;

    private float mValueStart;
    private float mValueFinish;
    private long mStartTime;
    private long mDuration;

    public EdgeAnimation() {
        mInterpolator = new DecelerateInterpolator();
        mState = STATE_IDLE;
    }

    private void startAnimation(float start, float finish, long duration,
            int newState) {
        mValueStart = start;
        mValueFinish = finish;
        mDuration = duration;
        mStartTime = now();
        mState = newState;
    }

    // The deltaDistance's magnitude is in the range of -1 (no change) to 1.
    // The value 1 is the full length of the view. Negative values means the
    // movement is in the opposite direction.
    public void onPull(float deltaDistance) {
        if (mState == STATE_ABSORB) {
            return;
        }
        mValue = Utils.clamp(mValue + deltaDistance, -1.0f, 1.0f);
        mState = STATE_PULL;
    }

    public void onRelease() {
        if (mState == STATE_IDLE || mState == STATE_ABSORB) {
            return;
        }
        startAnimation(mValue, 0, RELEASE_TIME, STATE_RELEASE);
    }

    public void onAbsorb(float velocity) {
        float finish = Utils.clamp(mValue + velocity * VELOCITY_FACTOR,
                -1.0f, 1.0f);
        startAnimation(mValue, finish, ABSORB_TIME, STATE_ABSORB);
    }

    public boolean update() {
        if (mState == STATE_IDLE) {
            return false;
        }
        if (mState == STATE_PULL) {
            return true;
        }

        float t = Utils.clamp((float) (now() - mStartTime) / mDuration, 0.0f, 1.0f);
        /* Use linear interpolation for absorb, quadratic for others */
        float interp = (mState == STATE_ABSORB)
                ? t : mInterpolator.getInterpolation(t);

        mValue = mValueStart + (mValueFinish - mValueStart) * interp;

        if (t >= 1.0f) {
            switch (mState) {
            case STATE_ABSORB:
                startAnimation(mValue, 0, RELEASE_TIME, STATE_RELEASE);
                break;
            case STATE_RELEASE:
                mState = STATE_IDLE;
                break;
            default:
                break;
            }
        }

        return true;
    }

    public float getValue() {
        return mValue;
    }

    private long now() {
        return AnimationTime.get();
    }
}