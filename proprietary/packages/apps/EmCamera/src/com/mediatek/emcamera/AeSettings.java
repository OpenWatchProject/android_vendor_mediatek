package com.mediatek.emcamera;

public class AeSettings {
    private AeMode_e aeMode = AeMode_e.Auto;
    private int minus = 200;
    private int plus = 200;
    private int interval = 10;

    public void setAeMode(AeMode_e aeMode) {
        this.aeMode = aeMode;
    }

    public void setMinus(int minus) {
        this.minus = minus;
    }

    public void setPlus(int plus) {
        this.plus = plus;
    }

    public void setInterval(int interval) {
        this.interval = interval;
    }

    public AeMode_e getAeMode() {
        return aeMode;
    }

    public int getMinus() {
        return minus;
    }

    public int getPlus() {
        return plus;
    }

    public int getInterval() {
        return interval;
    }

    public void reset() {
        aeMode = AeMode_e.Auto;
        minus = 200;
        plus = 200;
        interval = 10;
    }

    @Override
    public String toString() {
        return "AeMode: " + aeMode.getDesc() + "; minus: " + minus + "; plus: " + plus
                + "; interval: " + interval;
    }
}
