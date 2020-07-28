package com.mediatek.gallerybasic.base;

import java.util.Comparator;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.TreeMap;

public class MediaType {
    public static final int TYPE_INVALID = -1;
    private TreeMap<Integer, Integer> mTypes;

    public MediaType() {
        mTypes = new TreeMap<Integer, Integer>(new Comparator<Integer>() {
            @Override
            public int compare(Integer lhs, Integer rhs) {
                return rhs.compareTo(lhs);
            }
        });
    }

    public boolean isValid() {
        return !mTypes.isEmpty();
    }

    public void addType(int priority, int type) {
        mTypes.put(Integer.valueOf(priority), Integer.valueOf(type));
    }

    public int getMainType() {
        if (mTypes.isEmpty()) {
            return TYPE_INVALID;
        }
        return mTypes.firstEntry().getValue();
    }

    public int[] getAllTypes() {
        if (mTypes.isEmpty()) {
            return new int[]{
                    TYPE_INVALID
            };
        }

        int[] types = new int[mTypes.size()];
        Iterator<Entry<Integer, Integer>> itr = mTypes.entrySet().iterator();
        int i = 0;
        while (itr.hasNext()) {
            types[i++] = itr.next().getValue();
        }
        return types;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("main type = " + getMainType() + ", ");
        sb.append("all type = [");
        int[] allType = getAllTypes();
        boolean first = true;
        for (int type : allType) {
            if (!first) {
                sb.append(", ");
            } else {
                first = false;
            }
            sb.append(type);
        }
        sb.append("]");
        return sb.toString();
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null || !(obj instanceof MediaType)) {
            return false;
        }

        return this.toString().equals(obj.toString());
    }
}
