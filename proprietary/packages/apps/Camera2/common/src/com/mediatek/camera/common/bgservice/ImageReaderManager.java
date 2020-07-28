package com.mediatek.camera.common.bgservice;

import android.media.ImageReader;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.ArrayList;


public class ImageReaderManager {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ImageReaderManager.class.getSimpleName());
    public int mCurrentImageReaderWidth;
    public int mCurrentImageReaderHeight;
    public ImageReader mCurrentImageReader;
    public ArrayList mImageReaderList = null;

    public ImageReaderManager() {
        mImageReaderList = new ArrayList();
    }

    public int getImageReaderIndex() {
        int size = mImageReaderList.size();
        int index = -1;
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            if (temp != null && temp.mImageReader == mCurrentImageReader) {
                index = i;
                break;
            }
        }
        LogHelper.d(TAG, "[getImageReaderIndex] index = " + index);
        return index;
    }

    public int getImageReaderId() {
        return mCurrentImageReader.hashCode();
    }

    public boolean hasImageReader(int width, int height, int format, int maxImages) {
        int size = mImageReaderList.size();
        ImageReaderSet imageReaderSet = null;
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            if (temp != null && temp.mWidth == width && temp
                    .mHeight == height && temp.mFormat == format && temp.mMaxImages == maxImages) {
                imageReaderSet = temp;
                break;
            }
        }
        return imageReaderSet != null;
    }

    public boolean hasTheImageReader(int imageReaderId) {
        int size = mImageReaderList.size();
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            if (temp.mImageReader.hashCode() == imageReaderId) {
                return true;
            }
        }
        return false;
    }

    public boolean hasNoImageReader() {
        return mImageReaderList.isEmpty();
    }

    public ImageReader getImageReader(int width, int height, int format, int maxImages) {
        int size = mImageReaderList.size();
        ImageReaderSet imageReaderSet = null;
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            if (temp != null && temp.mWidth == width && temp
                    .mHeight == height && temp.mFormat == format && temp.mMaxImages == maxImages) {
                imageReaderSet = temp;
                break;
            }
        }
        if (imageReaderSet != null) {
            LogHelper.d(TAG, "[getImageReader] get ImageReader = " + imageReaderSet.mImageReader
                    + " width = " + imageReaderSet.mWidth + " height = " + imageReaderSet.mHeight);
            mCurrentImageReader = imageReaderSet.mImageReader;
        } else {
            ImageReaderSet imageReaderSetTemp = new ImageReaderSet(width, height, format,
                    maxImages);
            mImageReaderList.add(imageReaderSetTemp);
            LogHelper.d(TAG, "[getImageReader] new ImageReader = " + imageReaderSetTemp.mImageReader
                    + " width = " + imageReaderSetTemp.mWidth
                    + " height = " + imageReaderSetTemp.mHeight);
            mCurrentImageReader = imageReaderSetTemp.mImageReader;
        }
        mCurrentImageReaderWidth = width;
        mCurrentImageReaderHeight = height;
        return mCurrentImageReader;
    }

    public void releaseImageReader() {
        int size = mImageReaderList.size();
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            LogHelper.d(TAG, "[releaseImageReader] release ImageReader = " + temp
                    .mImageReader + " width = " + temp.mWidth + " height = " + temp.mHeight);
            if(temp.mImageReader != null){
                temp.mImageReader.close();
            }
        }
        mImageReaderList.clear();
    }

    public void releaseImageReader(int width, int height, int format, int maxImages) {
        int size = mImageReaderList.size();
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            if (temp != null && temp.mWidth == width && temp.mHeight == height
                    && temp.mFormat == format && temp.mMaxImages == maxImages) {
                mImageReaderList.remove(i);
                LogHelper.d(TAG, "[releaseImageReader] release ImageReader = " + temp
                        .mImageReader + " width = " + temp.mWidth + " height = " + temp.mHeight);
                temp.mImageReader.close();
                break;
            }
        }
    }

    public void releaseImageReader(int width, int height) {
        int size = mImageReaderList.size();
        for (int i = 0; i < size; i++) {
            ImageReaderSet temp = (ImageReaderSet) mImageReaderList.get(i);
            if (temp != null && temp.mWidth == width && temp
                    .mHeight == height) {
                mImageReaderList.remove(i);
                temp.mImageReader.close();
                break;
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            releaseImageReader();
        } finally {
            super.finalize();
        }
    }

    private class ImageReaderSet {
        public int mWidth;
        public int mHeight;
        public int mFormat;
        public int mMaxImages;
        public ImageReader mImageReader;

        ImageReaderSet(int width, int height, int format, int maxImages) {
            mWidth = width;
            mHeight = height;
            mFormat = format;
            mMaxImages = maxImages;
            if(width > 0 && height > 0){
                mImageReader = ImageReader.newInstance(width, height, format,
                    maxImages);
            }
        }
    }
}