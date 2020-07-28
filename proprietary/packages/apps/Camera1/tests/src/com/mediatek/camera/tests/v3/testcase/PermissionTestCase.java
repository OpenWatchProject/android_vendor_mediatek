package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.RuntimePermissionTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PermissionOperator;

import org.junit.Test;

/**
 * To check permission case.
 */

@CameraBasicTest
@RuntimePermissionTest
public class PermissionTestCase extends BaseCameraTestCase {
    @Test
    @FunctionTest
    public void testCameraLocationPermission() {
        new MetaCase("TC_Camera_permission_0005")
                .addChecker(new PreviewChecker())
                .addOperator(new PermissionOperator(), PermissionOperator.INDEX_ENABLE_LOCATION)
                .addOperator(new LaunchCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }
}
