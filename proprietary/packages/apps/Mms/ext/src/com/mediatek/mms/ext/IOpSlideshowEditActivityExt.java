package com.mediatek.mms.ext;

import android.content.Context;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.mediatek.mms.callback.ISlideshowEditActivityCallback;
import com.mediatek.mms.callback.ISlideshowEditorCallback;
import com.mediatek.mms.callback.ISlideshowModelCallback;

public interface IOpSlideshowEditActivityExt {

    final static int MENU_ADD_SLIDE_TO_TOP = 7;
    final static int MENU_ADD_SLIDE_TO_BOTTOM = 8;

    /**
     * @internal
     */
    boolean onPrepareOptionsMenu(Menu menu, ISlideshowModelCallback slideshow, int menuAdd);

    /**
     * @internal
     */
    boolean adjustAddSlideVisibility(Menu optionsMenu, ISlideshowModelCallback slideshow,
            View addSlideItem);

    /**
     * @internal
     */
    void onOptionsItemSelected(Context context, MenuItem item,
            ISlideshowEditorCallback slideshowEditor,
            ISlideshowEditActivityCallback slideshowEditActivity);
}
