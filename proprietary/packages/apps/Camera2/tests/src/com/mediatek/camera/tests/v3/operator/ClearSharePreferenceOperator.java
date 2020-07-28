package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.File;

public class ClearSharePreferenceOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ClearSharePreferenceOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        File file = new File("/data/data/com.mediatek.camera/shared_prefs");
        if (file.exists() && file.isDirectory()) {
            File[] files = file.listFiles();
            if (files == null) {
                return;
            }
            for (File f : files) {
                Utils.getTargetContext().deleteSharedPreferences(
                        f.getName().substring(0, f.getName().indexOf(".xml")));
                f.delete();
            }
        }
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Clear camera shared preferences";
    }
}
