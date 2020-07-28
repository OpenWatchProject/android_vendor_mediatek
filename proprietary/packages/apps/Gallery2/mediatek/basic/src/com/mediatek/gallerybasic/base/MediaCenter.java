package com.mediatek.gallerybasic.base;

import android.content.Context;
import android.util.SparseIntArray;

import com.mediatek.gallerybasic.util.Log;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map.Entry;

public class MediaCenter {
    private static final String TAG = "MtkGallery2/MediaCenter";

    private LinkedHashMap<Integer, MediaMember> mCreatorMap =
            new LinkedHashMap<Integer, MediaMember>();
    private LinkedHashMap<Integer, Layer> mLayerMap = new LinkedHashMap<Integer, Layer>();
    private int mHighestPriority;

    public synchronized void registerMedias(ArrayList<MediaMember> memberList) {
        Log.d(TAG, "<registerMedias> clear all at first");
        mCreatorMap.clear();
        mHighestPriority = Integer.MIN_VALUE;
        int type = memberList.size();
        for (MediaMember member : memberList) {
            Log.d(TAG, "<registerMedias> put member = " + member + ", type = " + type
                    + ", priority = " + member.getPriority());
            member.setType(type--);
            mCreatorMap.put(member.getType(), member);
            if (member.getPriority() > mHighestPriority) {
                mHighestPriority = member.getPriority();
            }
        }
    }

    public synchronized int getMemberCount() {
        return mCreatorMap.size();
    }

    public ExtItem getItem(MediaData md) {
        MediaMember mb = getMainMember(md);
        if (mb == null) {
            return null;
        }
        return mb.getItem(md);
    }

    public ExtItem getRealItem(MediaData md, int currentType) {
        MediaMember mb = getNextMember(md, currentType);
        if (mb == null) {
            return null;
        }
        return mb.getItem(md);
    }

    public Player getPlayer(MediaData md, ThumbType type) {
        MediaMember mb = getMainMember(md);
        if (mb == null) {
            return null;
        }
        return mb.getPlayer(md, type);
    }

    public Player getRealPlayer(MediaData md, ThumbType type, int currentType) {
        MediaMember mb = getNextMember(md, currentType);
        if (mb == null) {
            return null;
        }
        return mb.getPlayer(md, type);
    }

    public Generator getGenerator(MediaData md) {
        MediaMember mb = getMainMember(md);
        if (mb == null) {
            return null;
        }
        return mb.getGenerator();
    }

    public Generator getRealGenerator(MediaData md, int currentType) {
        MediaMember mb = getNextMember(md, currentType);
        if (mb == null) {
            return null;
        }
        return mb.getGenerator();
    }

    public Layer getLayer(Context context, MediaData md) {
        MediaMember mb = getMainMember(md);
        if (mb == null) {
            return null;
        } else if (mb.isShelled()) {
            return mLayerMap.get(mb.getType());
        } else {
            return getLayerForUnshelledMembers(context, md);
        }
    }

    public Layer getRealLayer(Context context, MediaData md, int currentType) {
        MediaMember mb = getNextMember(md, currentType);
        if (mb == null) {
            return null;
        } else if (mb.isShelled()) {
            return mLayerMap.get(mb.getType());
        } else {
            return getLayerForUnshelledMembers(context, md);
        }
    }

    public synchronized final LinkedHashMap<Integer, Layer> getAllLayer() {
        mLayerMap = new LinkedHashMap<Integer, Layer>();
        Iterator<Entry<Integer, MediaMember>> itr = mCreatorMap.entrySet().iterator();
        while (itr.hasNext()) {
            MediaMember mm = itr.next().getValue();
            mLayerMap.put(mm.getType(), mm.getLayer());
        }
        return mLayerMap;
    }

    private synchronized MediaMember getMainMember(MediaData md) {
        if (!md.mediaType.isValid()) {
            initMediaType(md);
        }
        return mCreatorMap.get(md.mediaType.getMainType());
    }

    private synchronized MediaMember getNextMember(MediaData md, int currentType) {
        if (!md.mediaType.isValid()) {
            initMediaType(md);
        }
        int[] types = md.mediaType.getAllTypes();
        for (int i = 0; i < types.length; i++) {
            if (types[i] == currentType && i < (types.length - 1)) {
                return mCreatorMap.get(types[i + 1]);
            }
        }
        return null;
    }

    private void initMediaType(MediaData md) {
        Iterator<Entry<Integer, MediaMember>> itr = mCreatorMap.entrySet().iterator();

        SparseIntArray shelledType = new SparseIntArray();
        SparseIntArray normalType = new SparseIntArray();
        while (itr.hasNext()) {
            Entry<Integer, MediaMember> entry = itr.next();
            if (entry.getValue().isMatching(md)) {
                if (entry.getValue().isShelled()) {
                    shelledType.put(entry.getValue().getPriority(), entry.getKey());
                } else {
                    normalType.put(entry.getValue().getPriority(), entry.getKey());
                }
            }
        }
        // Make sure the priority of shelled type is higher than that of normal type
        for (int i = 0; i < shelledType.size(); i++) {
            md.mediaType.addType(shelledType.keyAt(i) + mHighestPriority, shelledType
                    .valueAt(i));
        }
        for (int i = 0; i < normalType.size(); i++) {
            md.mediaType.addType(normalType.keyAt(i), normalType.valueAt(i));
        }
    }

    private Layer getLayerForUnshelledMembers(Context context, MediaData data) {
        // get unshelled members
        ArrayList<MediaMember> members = new ArrayList<MediaMember>();
        synchronized (this) {
            if (!data.mediaType.isValid()) {
                initMediaType(data);
            }
            int[] types = data.mediaType.getAllTypes();
            for (int type : types) {
                if (!mCreatorMap.get(type).isShelled()) {
                    members.add(mCreatorMap.get(type));
                }
            }
        }
        if (members.size() == 0) {
            return null;
        }
        // get layer
        ArrayList<Layer> layers = new ArrayList<Layer>();
        for (MediaMember member : members) {
            if (mLayerMap.get(member.getType()) != null) {
                layers.add(mLayerMap.get(member.getType()));
            }
        }
        if (layers.size() == 1) {
            return layers.get(0);
        } else if (layers.size() > 1) {
            return new ComboLayer(context, layers);
        }
        return null;
    }
}
