package com.mediatek.engineermode.otaairplanemode;

import com.mediatek.engineermode.Elog;
import com.mediatek.mdml.Msg;

class MdmBaseComponent {
    private static final String TAG = "MdmBaseComponent";
    private String[] emComponentName = null;

    public String[] getEmComponentName() {
        return emComponentName;
    }

    public void setEmComponentName(String[] emComponentName) {
        this.emComponentName = emComponentName;
    }

    public void update(String name, Msg data) {
        Elog.d(TAG, "update = " + name);
    }

    public boolean hasEmType(String type) {
        String[] types = getEmComponentName();
        if (types != null) {
            for (int i = 0; i < types.length; i++) {
                if (types[i].equals(type)) {
                    return true;
                }
            }
        }
        return false;
    }


}