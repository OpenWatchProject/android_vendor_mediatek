package com.mediatek.gallerybasic.base;

import android.content.Intent;
import android.view.ViewGroup;

/**
 * If you want to add more buttons at bottom controller in single image view, implement this
 * interface. It provides chances to add buttons, click buttons, and check whether to display
 * buttons.
 */
public interface IBottomControl {
    static final int DISPLAY_IGNORE = 0; // do not make decision whether to display
    static final int DISPLAY_TRUE = 1;   // display
    static final int DISPLAY_FALSE = 2;  // hide

    /**
     * When create bottom controller, trigger this callback.
     * You must do addButton at here.
     */
    public void onBottomControlCreated();

    /**
     * Do backup operation, when press back up menu.
     * @return Return false to continue to proceed, true to consume it here.
     */
    public boolean onUpPressed();

    /**
     * Do back operation, when press back key.
     * @return Return false to continue to proceed, true to consume it here.
     */
    public boolean onBackPressed();

    /**
     * When one of buttons on bottom controller is clicked, trigger this callback.
     *
     * @param id     The id of button which is clicked
     * @param data   The MediaData of current photo
     * @return Return false to allow other bottom control to proceed, true to consume it here
     */
    boolean onBottomControlButtonClicked(int id, MediaData data);

    /**
     * Whether display the whole bottom controller.
     *
     * @return When this interface has no idea whether to display, return DISPLAY_IGNORE.
     * When this interface wants to show the whole bottom controller, return DISPLAY_TRUE.
     * When this interface does not want to show the whole bottom controller, return DISPLAY_FALSE.
     * The result whether to display is decided by multi IBottomControl.
     */
    int canDisplayBottomControls();

    /**
     * Whether display one of button on bottom controller.
     *
     * @param id   The id of button
     * @param data The MediaData of current photo
     * @return When this interface has no idea whether to display, return DISPLAY_IGNORE.
     * When this interface wants to show the button, return DISPLAY_TRUE.
     * When this interface does not want to show the button, return DISPLAY_FALSE.
     */
    int canDisplayBottomControlButton(int id, MediaData data);

    /**
     * Set the backward operator of bottom controller.
     *
     * @param viewRoot The root layout of activity
     * @param controller BackwardBottomController
     */
    void init(ViewGroup viewRoot, BackwardBottomController controller);

    /**
     *  call back the caller(Gallery) to do sth. keep in mind: run on UI thread
     * @param requestCode request code
     * @param resultCode resultcode
     * @param data intent
     * @return whether consume the result or not.
     */
    boolean onActivityResult(int requestCode, int resultCode, Intent data);
}
