package com.mediatek.mdmlsample;

public class OutDbgStr {
    private java.io.FileWriter fw;
    private java.io.BufferedWriter bw;

    public OutDbgStr(String fileName) {
        try {
            fw = new java.io.FileWriter(fileName);
            bw = new java.io.BufferedWriter(fw);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void write(String str) {
        try {
            bw.write(str);
            bw.flush();
        } catch (Exception e) {
            // Do nothing
        }
    }

    public void writeLine(String str) {
        try {
            bw.write(str);
            bw.newLine();
            bw.flush();
        } catch (Exception e) {
            // Do nothing
        }
    }

    public void newLine() {
        try {
            bw.newLine();
            bw.flush();
        } catch (Exception e) {
            // Do nothing
        }
    }

    public void close() {
        try {
            fw.close();
        } catch (Exception e) {
            // Do nothing
        }
    }
}