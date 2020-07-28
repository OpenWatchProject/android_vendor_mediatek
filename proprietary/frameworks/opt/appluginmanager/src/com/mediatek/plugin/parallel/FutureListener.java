package com.mediatek.plugin.parallel;

public interface FutureListener<T> {
    public void onFutureDone(Future<T> future);
}
