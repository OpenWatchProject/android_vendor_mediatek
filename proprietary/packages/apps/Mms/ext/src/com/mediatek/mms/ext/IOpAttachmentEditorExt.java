package com.mediatek.mms.ext;

import android.content.Context;
import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import com.mediatek.mms.callback.IAttachmentEditorCallback;

public interface IOpAttachmentEditorExt {

    /// M: add for attachment enhance
    static final int MSG_REMOVE_EXTERNAL_ATTACHMENT = 11;
    static final int MSG_REMOVE_SLIDES_ATTACHMENT = 12;

    /**
     * @internal
     */
    void createSlideshowView(Context context,
            LinearLayout slideshowPanel, Button sendButton, Button removeButton);

    /**
     * @internal
     */
    void init(IAttachmentEditorCallback attachmentEditor);

    /**
     * @internal
     */
    void createMediaView(Button removeButton);

    /**
     * @internal
     */
    void createFileAttachmentView(View remove);

    /**
     * @internal
     */
    boolean update();

    /**
     * @internal
     */
    boolean onTextChangeForOneSlide();

    /**
     * @internal
     */
    public void updateSendButton(boolean canSend);

    /**
     * @internal
     */
    public void setHandler(Handler handler);
}
