package com.mediatek.gallerybasic.base;

public interface Work<T> {
    public boolean isCanceled();
    public T run();
}