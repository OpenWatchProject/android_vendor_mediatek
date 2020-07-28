package com.mediatek.galleryfeature.drm;


import com.mediatek.gallerybasic.base.ExtFields;
import com.mediatek.gallerybasic.base.IFieldDefinition;

public class DrmField implements IFieldDefinition {
    public final static  String IS_DRM = "is_drm";
    public final static  String DRM_METHOD = "drm_method";
    @Override
    public void onFieldDefine() {
        if (DrmHelper.sSupportDrm) {
            ExtFields.addImageFiled(IS_DRM);
            ExtFields.addImageFiled(DRM_METHOD);
            ExtFields.addVideoFiled(IS_DRM);
            ExtFields.addVideoFiled(DRM_METHOD);
        }
    }
}
