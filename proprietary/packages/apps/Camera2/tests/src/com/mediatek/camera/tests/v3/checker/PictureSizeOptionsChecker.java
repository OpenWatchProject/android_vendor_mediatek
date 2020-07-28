package com.mediatek.camera.tests.v3.checker;

import android.content.Context;
import android.graphics.Point;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;
import android.view.WindowManager;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * Check the picture size options is meeting to spec.
 */

public class PictureSizeOptionsChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            PictureSizeOptionsChecker.class.getSimpleName());

    private static final String PICTURE_SIZE_SETTING_TITLE = "Picture size";
    private static final double ASPECT_TOLERANCE = 0.02;

    private List<SizeBucket> mSizeBuckets = new ArrayList<>();

    @Override
    protected void doCheck() {
        UiObject2 pictureSizeEntry = Utils
                .scrollOnScreenToFind(By.text(PICTURE_SIZE_SETTING_TITLE));
        String pictureSummary = pictureSizeEntry.getParent()
                .findObject(By.res("android:id/summary")).getText();
        pictureSizeEntry.click();

        UiObject2 listView = Utils.findObject(By.res("android:id/list"));
        List<UiObject2> titleViewList = listView.findObjects(By.res("android:id/title"));
        Utils.assertRightNow(titleViewList.size() > 0);
        for (int i = 0; i < titleViewList.size(); i++) {
            UiObject2 titleView = titleViewList.get(i);

            String title = titleView.getText();
            int index = title.indexOf("M");
            String size;
            String ratio;
            if (index == -1) {
                // picture size maybe QVGA(4:3), WQVGA(5:3), VGA(4:3), WVGA(5:3), SVGA(4:3).
                size = title.substring(0, title.length());
                if (title.startsWith("W")) {
                    ratio = "(5:3)";
                } else {
                    ratio = "(4:3)";
                }
            } else {
                size = title.substring(0, index);
                ratio = title.substring(index + 1, title.length());
            }
            UiObject2 summaryView = titleView.getParent()
                    .findObject(By.res("android:id/summary"));
            String resolution = summaryView.getText();
            LogHelper.d(TAG, "[doCheck], size:" + size + ", ratio:"
                    + ratio + ", resolution:" + resolution);
            classifyPictureSize(ratio, size, resolution);
        }

        // The count of picture size ratio can't more than 2.
        Utils.assertRightNow(mSizeBuckets.size() <= 2);
        for (int i = 0; i < mSizeBuckets.size(); i++) {
            SizeBucket bucket = mSizeBuckets.get(i);
            // The count of picture size with same ratio can't more then 3.
            Utils.assertRightNow(bucket.sizes.size() <= 3);

            boolean isDescending = true;
            // Picture sizes with same ratio must be different and descending.
            for (int j = 0; j < bucket.sizes.size() - 1; j++) {
                // picture size value may be like VGA, so used resolution to compare.
                int index;
                String sizeA = bucket.resolution.get(j);
                index = sizeA.indexOf("x");
                sizeA = sizeA.substring(0, index);
                String sizeB = bucket.resolution.get(j + 1);
                index = sizeB.indexOf("x");
                sizeB = sizeB.substring(0, index);
                if (Integer.parseInt(sizeA) <= Integer.parseInt(sizeB)) {
                    isDescending = false;
                    break;
                }
            }
            Utils.assertRightNow(isDescending);
        }

        // Check default value is full-screen ratio.
        WindowManager wm = (WindowManager) Utils.getContext()
                .getSystemService(Context.WINDOW_SERVICE);
        Point displaySize = new Point();
        wm.getDefaultDisplay().getRealSize(displaySize);
        double fullScreenRatio = (double) displaySize.y / (double) displaySize.x;
        LogHelper.d(TAG, "[doCheck], size.x:" + displaySize.x + ", size.y:" + displaySize.y
                + ", fullScreenRatio:" + fullScreenRatio);

        UiObject2 checkedRadio = listView.findObject(By.res("android:id/checkbox").checked(true));
        UiObject2 checkedTitleView = checkedRadio.getParent().getParent()
                .findObject(By.res("android:id/title"));
        String checkedTitle = checkedTitleView.getText();
        // The selected option's title must equal with the picture size setting summary.
        Utils.assertRightNow(pictureSummary.equals(checkedTitle));

        int index = checkedTitle.indexOf("M");
        String checkedSize = checkedTitle.substring(0, index);
//        String denominator = checkedTitle
//                .substring(checkedTitle.indexOf("(") + 1, checkedTitle.indexOf(":"));
//        String numerator = checkedTitle
//                .substring(checkedTitle.indexOf(":") + 1, checkedTitle.indexOf(")"));
//        LogHelper.d(TAG, "[doCheck], checkedSize:" + checkedSize + ", denominator:" + denominator
//                + ", numerator:" + numerator);
//        double checkedRatio = Double.parseDouble(denominator) / Double.parseDouble(numerator);
//        Utils.assertRightNow(Math.abs(fullScreenRatio - checkedRatio) <= ASPECT_TOLERANCE);

        // Check the default size is the max size in full-screen ratio.
        SizeBucket bucket = getSizeBucketByRatio(checkedTitle
                .substring(checkedTitle.indexOf("M") + 1, checkedTitle.length()));
        int maxSize = Integer.parseInt(bucket.sizes.get(0));
        Utils.assertRightNow(Integer.parseInt(checkedSize) == maxSize);

        Utils.getUiDevice().pressBack();
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        return "check the picture size setting default size is full-screen ratio and"
                + " it is max size in full-screen ratio, it must not have two same"
                + " picture size, count of same ratio sizes must no more than 3.";
    }

    private SizeBucket getSizeBucketByRatio(String ratio) {
        for (int i = 0; i < mSizeBuckets.size(); i++) {
            SizeBucket bucket = mSizeBuckets.get(i);
            if (bucket.ratio.equals(ratio)) {
                return bucket;
            }
        }
        return null;
    }

    private void classifyPictureSize(String ratio, String size, String resolution) {
        int index = -1;
        for (int i = 0; i < mSizeBuckets.size(); i++) {
            if (mSizeBuckets.get(i).ratio.equals(ratio)) {
                index = i;
                break;
            }
        }
        SizeBucket bucket;
        if (index == -1) {
            bucket = new SizeBucket();
            bucket.ratio = ratio;
            mSizeBuckets.add(bucket);
        } else {
            bucket = mSizeBuckets.get(index);
        }
        bucket.sizes.add(size);
        bucket.resolution.add(resolution);
    }

    /**
     * Picture size bucket by ratio.
     */
    private class SizeBucket {
        public String ratio;
        public List<String> sizes = new ArrayList<>();
        public List<String> resolution = new ArrayList<>();
    }
}
