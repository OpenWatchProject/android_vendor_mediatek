package com.android.packageinstaller.permission.model;

/**
 *  @hide
 */
public final class PermissionState {
    public static final int STATE_UNKNOWN = 0;
    public static final int STATE_ALLOWED = 1;
    public static final int STATE_DENIED = 2;

    private final AppPermissionGroup mGroup;
    private final Permission mPermission;
    private int mState = STATE_UNKNOWN;

    public PermissionState(AppPermissionGroup group, Permission permission) {
        this.mGroup = group;
        this.mPermission = permission;
    }

    public PermissionState(AppPermissionGroup group, Permission permission, int state) {
        this(group, permission);
        this.mState = state;
    }

    public int getState() {
        return this.mState;
    }

    public void setState(int state) {
        this.mState = state;
    }

    public Permission getPermission() {
        return this.mPermission;
    }

    public AppPermissionGroup getAppPermissionGroup() {
        return this.mGroup;
    }
}
