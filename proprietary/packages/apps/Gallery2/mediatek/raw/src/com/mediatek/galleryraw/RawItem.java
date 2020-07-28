package com.mediatek.galleryraw;

import android.content.Context;

import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.MediaData;

import java.util.ArrayList;

/**
 * The item for all raw types.
 */
class RawItem extends ExtItem {
    public RawItem(Context context, MediaData md) {
        super(context, md);
    }

    public RawItem(MediaData md) {
        super(md);
    }

    @Override
    public ArrayList<SupportOperation> getNotSupportedOperations() {
        ArrayList<SupportOperation> res = new ArrayList<SupportOperation>();
        res.add(SupportOperation.EDIT);
        return res;
    }
}
