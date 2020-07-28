package com.mediatek.ims.rcsua;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Configuration data retrieved from ACS.
 */
public class AcsConfiguration implements Parcelable {
    protected AcsConfiguration(Parcel in) {
        xmlData = in.readString();
        version = in.readInt();
    }

    public AcsConfiguration(String xmlData, int version) {
        this.xmlData = xmlData;
        this.version = version;
    }

    /**
     * Get XML format of MO retrieved from ACS server
     *
     * @return String wrapped XML data, null for not available.
     */
    public String readXmlData() {
        return xmlData;
    }

    /**
     * Get configuration version.
     *
     * @return integer configuration version. Only positive value is meaningful.
     */
    public int getVersion() {
        return version;
    }

    void setXmlData(String xml) {
        this.xmlData = xml;
    }

    void setVersion(int version) {
        this.version = version;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(xmlData);
        dest.writeInt(version);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<AcsConfiguration> CREATOR = new Creator<AcsConfiguration>() {
        @Override
        public AcsConfiguration createFromParcel(Parcel in) {
            return new AcsConfiguration(in);
        }

        @Override
        public AcsConfiguration[] newArray(int size) {
            return new AcsConfiguration[size];
        }
    };

    private String xmlData;
    private int version;
}
