package com.gsma.services.rcs;
import android.util.SparseArray;

/**
 * Direction of the communication for Chat message, Geolocation, Filetransfer, Imageshare,
 * Videoshare etc.
 */
public enum Direction {

    /**
     * Incoming communication
     */
    INCOMING(0),

    /**
     * Outgoing communication
     */
    OUTGOING(1),

    /**
     * Irrelevant or not applicable (e.g. for group chat event message)
     */
    IRRELEVANT(2);

    private final int mValue;

    private static SparseArray<Direction> mValueToEnum = new SparseArray<>();
    static {
        for (Direction entry : Direction.values()) {
            mValueToEnum.put(entry.toInt(), entry);
        }
    }

    Direction(int value) {
        mValue = value;
    }

    /**
     * Gets integer value associated to Direction instance
     * 
     * @return value
     */
    public final int toInt() {
        return mValue;
    }

    /**
     * Returns a Direction instance for the specified integer value.
     * 
     * @param value the value associated to the Direction
     * @return instance
     */
    public static Direction valueOf(int value) {
        Direction entry = mValueToEnum.get(value);
        if (entry != null) {
            return entry;
        }
        throw new IllegalArgumentException("No enum const class " + Direction.class.getName()
                + "" + value + "!");
    }
}