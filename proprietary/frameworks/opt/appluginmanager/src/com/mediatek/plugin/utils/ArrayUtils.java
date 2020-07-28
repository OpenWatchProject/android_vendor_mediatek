package com.mediatek.plugin.utils;

public class ArrayUtils {
    public static <T> boolean areExactMatch(T[] a, T[] b) {
        return (a.length == b.length) && ArrayUtils.containsAll(a, b)
                && ArrayUtils.containsAll(b, a);
    }

    public static <T> boolean isEmpty(T[] array) {
        return array == null || array.length == 0;
    }

    /**
     * Test if all {@code check} items are contained in {@code array}.
     */
    public static <T> boolean containsAll(T[] array, T[] check) {
        if (check == null) {
            return true;
        }
        for (T checkItem : check) {
            if (!contains(array, checkItem)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Checks that value is present as at least one of the elements of the array.
     *
     * @param array the array to check in
     * @param value the value to check for
     * @return true if the value is present in the array
     */
    public static <T> boolean contains(T[] array, T value) {
        return indexOf(array, value) != -1;
    }

    /**
     * Return first index of {@code value} in {@code array}, or {@code -1} if not found.
     */
    public static <T> int indexOf(T[] array, T value) {
        if (array == null) {
            return -1;
        }
        for (int i = 0; i < array.length; i++) {
            if (array[i] == null && value == null) {
                return i;
            }
            if (array[i] == null && value != null) {
                continue;
            }
            if (array[i].equals(value)) {
                return i;
            }
        }
        return -1;
    }
}
