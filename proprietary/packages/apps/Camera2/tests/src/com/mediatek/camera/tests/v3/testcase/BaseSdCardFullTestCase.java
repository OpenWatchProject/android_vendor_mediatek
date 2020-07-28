package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.operator.DefaultStorageOperator;
import com.mediatek.camera.tests.v3.operator.FillStorageOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.After;
import org.junit.Before;

public abstract class BaseSdCardFullTestCase extends BaseTestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            BaseSdCardFullTestCase.class.getSimpleName());

    @Before
    public void setUp() {
        LogHelper.d(TAG, "[setUp]");
        super.setUp();
        new MetaCase()
                .addOperator(new DefaultStorageOperator(), DefaultStorageOperator.INDEX_SD_CARD)
                .addOperator(new FillStorageOperator(), FillStorageOperator.INDEX_FULL)
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_NO_THUMB)
                .run();

    }

    @After
    public void tearDown() {
        LogHelper.d(TAG, "[tearDown]");
        new MetaCase()
                .addOperator(new DefaultStorageOperator(),
                        DefaultStorageOperator.INDEX_INTERNAL_STORAGE)
                .run();
        super.tearDown();
    }
}
