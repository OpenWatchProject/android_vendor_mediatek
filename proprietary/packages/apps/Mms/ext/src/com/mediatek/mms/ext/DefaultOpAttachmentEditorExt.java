package com.mediatek.mms.ext;

import android.content.Context;
import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import com.mediatek.mms.callback.IAttachmentEditorCallback;

public class DefaultOpAttachmentEditorExt implements IOpAttachmentEditorExt {

    public void init(IAttachmentEditorCallback attachmentEditor) {
    }

    public void createFileAttachmentView(View remove) {
    }

    public boolean update() {
        return false;
    }

    public boolean onTextChangeForOneSlide() {
        return false;
    }

    @Override
    public void updateSendButton(boolean canSend) {
    }

    @Override
    public void createSlideshowView(Context context,
            LinearLayout slideshowPanel, Button sendButton, Button removeButton) {
    }

    @Override
    public void createMediaView(Button removeButton) {

    }

    @Override
    public void setHandler(Handler handler) {
    }
}
