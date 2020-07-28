package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.checker.NoImagesVideosChecker;
import com.mediatek.camera.tests.v3.observer.CameraOpenFailedLogObserver;
import com.mediatek.camera.tests.v3.operator.ClearImagesVideosOperator;
import com.mediatek.camera.tests.v3.operator.ClearRecentAppListOperator;
import com.mediatek.camera.tests.v3.operator.ClearSharePreferenceOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.InitCameraParameterOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.Before;

public abstract class BaseTestCase extends TestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(BaseTestCase.class.getSimpleName());
    private long beginTimeStamp;
    private long endTimeStamp;
    protected boolean mNotClearImagesVideos = false;

    @Before
    public void setUp() {
        LogHelper.d(TAG, "[setUp] +");
        beginTimeStamp = System.currentTimeMillis();
        TestContext.reset();
        if (mNotClearImagesVideos) {
            new MetaCase()
                    .observeBegin(new CameraOpenFailedLogObserver())
                    .addOperator(new InitCameraParameterOperator())
                    .observeEnd()
                    .addOperator(new ClearSharePreferenceOperator())
                    .run();
        } else {
            new MetaCase()
                    .observeBegin(new CameraOpenFailedLogObserver())
                    .addOperator(new InitCameraParameterOperator())
                    .observeEnd()
                    .addOperator(new ClearImagesVideosOperator())
                    .addChecker(new NoImagesVideosChecker())
                    .addOperator(new ClearSharePreferenceOperator())
                    .addOperator(new ClearRecentAppListOperator())
                    .run();
        }

        LogHelper.d(TAG, "[setUp] -");
    }

    @After
    public void tearDown() {
        LogHelper.d(TAG, "[tearDown] +");
        if (mNotClearImagesVideos) {
            new MetaCase()
                    .addOperator(new ExitCameraOperator().ignoreBothPageCheck())
                    .addOperator(new ClearSharePreferenceOperator())
                    .run();
        } else {
            new MetaCase()
                    .addOperator(new ExitCameraOperator().ignoreBothPageCheck())
                    .addOperator(new ClearImagesVideosOperator())
                    .addChecker(new NoImagesVideosChecker())
                    .addOperator(new ClearSharePreferenceOperator())
                    .run();
        }

        endTimeStamp = System.currentTimeMillis();
        Utils.writeTestCaseCostTime(getClass().getName() + "#" + getName(),
                (int) (endTimeStamp - beginTimeStamp) / 1000);
        LogHelper.d(TAG, "[tearDown] -");
    }
}
