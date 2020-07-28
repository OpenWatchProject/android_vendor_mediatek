package com.mediatek.engineermode.rfdesense;

/**
 * Created by MTK09919 on 2017/11/9.
 */

public class RfDesenseServiceData {
    private String rat;
    private int channel;
    private int power;
    private int band;
    private int time;
    private int rb;
    private int bw;


    public RfDesenseServiceData() {
        this.rb = -1;
        this.bw = -1;
    }

    public RfDesenseServiceData(int channel, int power, int band, int time) {
        this.channel = channel;
        this.power = power;
        this.band = band;
        this.time = time;
        this.rb = -1;
        this.bw = -1;
    }

    public String getRat() {
        return rat;
    }

    public void setRat(String rat) {
        this.rat = rat;
    }

    public int getBand() {
        return band;
    }

    public void setBand(int band) {
        this.band = band;
    }

    public int getChannel() {
        return channel;
    }

    public void setChannel(int channel) {
        this.channel = channel;
    }

    public int getPower() {
        return power;
    }

    public void setPower(int power) {
        this.power = power;
    }

    public int getTime() {
        return time;
    }

    public void setTime(int time) {
        this.time = time;
    }

    public int getRb() {
        return rb;
    }

    public void setRb(int rb) {
        this.rb = rb;
    }

    public int getBw() {
        return bw;
    }

    public void setBw(int bw) {
        this.bw = bw;
    }

    @Override
    public String toString() {
        return "RfDesenseServiceData{" +
                "rat='" + rat + '\'' +
                ", channel=" + channel +
                ", power=" + power +
                ", band=" + band +
                ", time=" + time +
                '}';
    }
}
