package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class LaunchCameraOperator extends Operator {
    public static final int INDEX_NORMAL = 0;
    public static final int INDEX_INTENT_PHOTO = 1;
    public static final int INDEX_INTENT_VIDEO = 2;
    public static final int INDEX_INTENT_PHOTO_NO_URI = 3;
    public static final int INDEX_INTENT_VIDEO_NO_URI = 4;
    public static final int INDEX_SECURE_CAMERA = 5;

    private static final LogUtil.Tag TAG = Utils.getTestTag(
            LaunchCameraOperator.class.getSimpleName());

    private static final String MTK_CAMERA_PACKAGE = "com.mediatek.camera";
    private static final long MAX_VIDEO_SIZE_BYTE = 500000;
    private static final int MAX_DURATION_SECONDS = 5;

    @Override
    protected void doOperate(int index) {
        Intent intent = null;
        switch (index) {
            case INDEX_NORMAL: {
                intent = Utils.getContext().getPackageManager()
                        .getLaunchIntentForPackage(MTK_CAMERA_PACKAGE);
            }
            break;
            case INDEX_INTENT_PHOTO: {
                String path = Environment.getExternalStorageDirectory().toString() + "image.jpg";
                Uri uri = Uri.parse("content://" + path);
                intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                intent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
            }
            break;
            case INDEX_INTENT_PHOTO_NO_URI: {
                intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
            }
            break;
            case INDEX_INTENT_VIDEO: {
                String path = Environment.getExternalStorageDirectory().toString() + "video.tmp";
                Uri uri = Uri.parse("content://" + path);
                intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
                intent.putExtra(MediaStore.EXTRA_SIZE_LIMIT, MAX_VIDEO_SIZE_BYTE);
                intent.putExtra(MediaStore.EXTRA_DURATION_LIMIT, MAX_DURATION_SECONDS);
                intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0); // use low quality to speed up
                intent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
            }
            break;
            case INDEX_INTENT_VIDEO_NO_URI: {
                intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
                intent.putExtra(MediaStore.EXTRA_SIZE_LIMIT, MAX_VIDEO_SIZE_BYTE);
                intent.putExtra(MediaStore.EXTRA_DURATION_LIMIT, MAX_DURATION_SECONDS);
                intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0); // use low quality to speed up
            }
            break;
            case INDEX_SECURE_CAMERA:
                intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE_SECURE);
                break;
            default:
                break;
        }
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Utils.getContext().startActivity(intent);
    }

    @Override
    public int getOperatorCount() {
        return 6;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return null;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_NORMAL:
                return "Launch camera activity in normal mode";
            case INDEX_INTENT_PHOTO:
                return "Launch camera activity in intent photo mode";
            case INDEX_INTENT_VIDEO:
                return "Launch camera activity in intent video mode";
            case INDEX_INTENT_PHOTO_NO_URI:
                return "Launch camera activity in intent photo mode, no uri in intent";
            case INDEX_INTENT_VIDEO_NO_URI:
                return "Launch camera activity in intent video mode, no uri in intent";
            case INDEX_SECURE_CAMERA:
                return "Launch secure camera in normal mode";
            default:
                return null;
        }
    }
}
