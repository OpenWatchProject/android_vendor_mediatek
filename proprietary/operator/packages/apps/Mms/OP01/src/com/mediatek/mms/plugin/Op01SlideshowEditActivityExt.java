package com.mediatek.mms.plugin;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import com.mediatek.mms.callback.IMessageUtilsCallback;
import com.mediatek.mms.callback.ISlideshowEditActivityCallback;
import com.mediatek.mms.callback.ISlideshowEditorCallback;
import com.mediatek.mms.callback.ISlideshowModelCallback;
import com.mediatek.mms.ext.DefaultOpSlideshowEditActivityExt;
import com.mediatek.mms.plugin.R;

/**
 * Op01SlideshowEditActivityExt.
 *
 */
public class Op01SlideshowEditActivityExt extends
        DefaultOpSlideshowEditActivityExt {
    private static final String TAG = "Op01SlideshowEditActivityExt";

    /**
     * Construction.
     * @param context Context.
     */
    public Op01SlideshowEditActivityExt(Context context) {
        super(context);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu, ISlideshowModelCallback slideshow,
            int menuAdd) {
        Drawable dbAdd = Op01MessagePluginExt.sMessageUtilsCallback.getDrawable(
                IMessageUtilsCallback.ic_menu_add_slide);
        //add slide to top
        if (slideshow.sizeCallback() >= 1
                && slideshow.sizeCallback() < Op01MessagePluginExt
                        .sMessageUtilsCallback.getMaxSlideNum()) {
            menu.add(0, MENU_ADD_SLIDE_TO_TOP, 0, getString(R.string.add_top_slide)).setIcon(dbAdd);
            menu.add(0, MENU_ADD_SLIDE_TO_BOTTOM, 0, getString(R.string.add_bottom_slide))
                                .setIcon(dbAdd);
        }

        // add slide when there is no slide
        if (slideshow.sizeCallback() == 0) {
            String addSlide = Op01MessagePluginExt.sMessageUtilsCallback.getString(
                    IMessageUtilsCallback.add_slide);
            menu.add(0, menuAdd, 0, addSlide).setIcon(dbAdd);
        }

        return true;
    }

    @Override
    public boolean adjustAddSlideVisibility(Menu optionsMenu, ISlideshowModelCallback slideshow,
            View addSlideItem) {
        MenuItem itemAddTop = null;
        MenuItem itemAddBottom = null;

        Drawable dbAdd = Op01MessagePluginExt.sMessageUtilsCallback.getDrawable(
                IMessageUtilsCallback.ic_menu_add_slide);

        if (optionsMenu != null) {
            itemAddTop = optionsMenu.findItem(MENU_ADD_SLIDE_TO_TOP);
            itemAddBottom = optionsMenu.findItem(MENU_ADD_SLIDE_TO_BOTTOM);
        }

        if (slideshow.sizeCallback() >= Op01MessagePluginExt
                .sMessageUtilsCallback.getMaxSlideNum()) {
            addSlideItem.setVisibility(View.GONE);
            if (itemAddTop != null && itemAddBottom != null) {
                itemAddTop.setVisible(false);
                itemAddBottom.setVisible(false);
            }
        } else {
            addSlideItem.setVisibility(View.VISIBLE);
            if (itemAddTop != null && itemAddBottom != null) {
                itemAddTop.setVisible(true);
                itemAddBottom.setVisible(true);
            } else if (optionsMenu != null) {
                itemAddTop = optionsMenu.add(0, MENU_ADD_SLIDE_TO_TOP, 0,
                        getString(R.string.add_top_slide)).setIcon(dbAdd);
                itemAddBottom = optionsMenu
                        .add(0, MENU_ADD_SLIDE_TO_BOTTOM, 0, getString(R.string.add_bottom_slide))
                        .setIcon(dbAdd);
            }
        }

        if (itemAddTop != null && itemAddBottom != null) {
            Log.i(TAG, "itemAddTop isVisible = " + itemAddTop.isVisible()
                    + "itemAddBottom isVisible = " + itemAddBottom.isVisible());
        }

        return true;
    }

    @Override
    public void onOptionsItemSelected(Context context, MenuItem item,
            ISlideshowEditorCallback slideshowEditor,
            ISlideshowEditActivityCallback slideshowEditActivity) {
        switch (item.getItemId()) {
        case MENU_ADD_SLIDE_TO_TOP:
            addNewSlideToTop(context, slideshowEditor, slideshowEditActivity);
            break;
        case MENU_ADD_SLIDE_TO_BOTTOM:
            slideshowEditActivity.addNewSlideCallback();
            break;
        default:
            break;
        }
    }

    private void addNewSlideToTop(Context context, ISlideshowEditorCallback slideshowEditor,
            ISlideshowEditActivityCallback slideshowEditActivity) {
        if (slideshowEditor.addNewSlideCallback(0)) {
            // add successfully
            slideshowEditActivity.notifyAdapterDataSetChanged();

            // Select the new slide.
            slideshowEditActivity.requestListFocus();
            slideshowEditActivity.setListSelection(
                    slideshowEditActivity.getSlideshowModelSize() - 1);
        } else {
            String cannotAdd = Op01MessagePluginExt.sMessageUtilsCallback.getString(
                    IMessageUtilsCallback.cannot_add_slide_anymore);
            Toast.makeText(context, cannotAdd, Toast.LENGTH_SHORT).show();
        }
    }
}
