package com.mediatek.lbs.em2.utils;

public interface AgpsDebugListener {
    public void onToastMessageReceived(String message);
    public void onViewMessageReceived(int color, String message);
    public void onDialogMessageReceived(String title, String message);
    public void onVzwDebugMessageReceived(String message);
}
