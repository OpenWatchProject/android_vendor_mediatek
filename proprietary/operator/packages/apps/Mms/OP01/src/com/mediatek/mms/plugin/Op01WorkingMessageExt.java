package com.mediatek.mms.plugin;

import com.mediatek.mms.callback.ISlideshowEditorCallback;
import com.mediatek.mms.ext.DefaultOpWorkingMessageExt;

/**
 * Op01WorkingMessageExt.
 *
 */
public class Op01WorkingMessageExt extends DefaultOpWorkingMessageExt {

    @Override
    public int changeMedia(int increaseSize, int attachFilesSize) {
        increaseSize += attachFilesSize;
        return increaseSize;
    }

    @Override
    public boolean setAttachment(int type, int attachType,
            ISlideshowEditorCallback slideshowEditor) {
        if (type < attachType) {
            slideshowEditor.removeSlideCallback(slideshowEditor.getModelSize() - 1);
        }
        return true;
    }
}
