package com.mediatek.gallerybasic.base;

/**
 * If you want to extend the fields in MediaData, implement this interface to add fields by
 * {@link ExtFields#addImageFiled(String)}, {@link ExtFields#addVideoFiled(String)}.
 */
public interface IFieldDefinition {
    /**
     * This function will be called when initialize the ExtFields.
     */
    public void onFieldDefine();
}
