package com.mediatek.presence.core.ims.service.presence;

import java.lang.StringBuilder;

import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.utils.DateUtils;
import com.mediatek.presence.utils.StringUtils;

import com.android.ims.internal.uce.presence.PresServiceInfo;

public class PresentityBuilder {
    private StringBuilder mRCSInfo = new StringBuilder();
    private StringBuilder mIMSInfo = new StringBuilder();
    private StringBuilder mGeolocInfo = new StringBuilder();
    private StringBuilder mPersionInfo = new StringBuilder();
    private StringBuilder mMediaInfo = new StringBuilder();

    //tuple ids
    public static final String RCS_TUPLE_ID_FILE_TRANSFER = "FileTransfer";
    public static final String RCS_TUPLE_ID_FILE_TRANSFR_THUMBNAIL = "FileTransferThumbnail";
    public static final String RCS_TUPLE_ID_SESS_MODE_MESSA = "SessModeMessa";
    public static final String RCS_TUPLE_ID_DISCOVERY_PRES = "DiscoveryPres";
    public static final String RCS_TUPLE_ID_FILE_TRANSFER_H = "FileTransferH";
    public static final String RCS_TUPLE_ID_VOLTE = "VOLTECALL";
    public static final String RCS_TUPLE_ID_STANDALONE = "StandAlone";

    public PresentityBuilder() {}

    public void addRCSCapability(String tupleId, String status, String serviceId,
                                                                       String serviceVerion, String serviceDesc,
                                                                       String contact) {
        mRCSInfo.append("<tuple id=\"" + tupleId + "\">" + SipUtils.CRLF);
        mRCSInfo.append("<status><basic>" + status + "</basic></status>" + SipUtils.CRLF);
        mRCSInfo.append("<op:service-description>" + SipUtils.CRLF);
        mRCSInfo.append("<op:service-id>" + serviceId + "</op:service-id>" + SipUtils.CRLF);
        mRCSInfo.append("<op:version>" + serviceVerion + "</op:version>" + SipUtils.CRLF);
        mRCSInfo.append("<op:description>" + serviceDesc + "</op:description>" + SipUtils.CRLF);
        mRCSInfo.append("</op:service-description>" + SipUtils.CRLF);
        mRCSInfo.append("<contact>" + contact + "</contact>" + SipUtils.CRLF);
        mRCSInfo.append("</tuple>" + SipUtils.CRLF);
    }

    public void addIMSCapability(String tupleId, String status, boolean volteSupport,
                                                                    boolean vilteSupport, boolean duplexSupport,
                                                                    String serviceId, String serviceVerion,
                                                                    String serviceDesc, String contact) {
        if (volteSupport) {
            mIMSInfo.append("<tuple id=\"" + tupleId + "\">" + SipUtils.CRLF);
            mIMSInfo.append("<status><basic>" + status + "</basic></status>" + SipUtils.CRLF);
            mIMSInfo.append("<caps:servcaps>" + SipUtils.CRLF);
            mIMSInfo.append("<caps:audio>" + volteSupport + "</caps:audio>" + SipUtils.CRLF);
            
            if (vilteSupport) {
                mIMSInfo.append("<caps:video>" + vilteSupport + "</caps:video>" + SipUtils.CRLF);
            }

            if (duplexSupport) {
               mIMSInfo.append("<caps:duplex>" + SipUtils.CRLF);
               mIMSInfo.append("<caps:supported>" + SipUtils.CRLF);
               mIMSInfo.append("<caps:full/>" + SipUtils.CRLF);
               mIMSInfo.append("</caps:supported>" + SipUtils.CRLF);
               mIMSInfo.append("</caps:duplex>" + SipUtils.CRLF);
            }
            mIMSInfo.append("</caps:servcaps>" + SipUtils.CRLF);

            mIMSInfo.append("<op:service-description>" + SipUtils.CRLF);
            mIMSInfo.append("<op:service-id>" + serviceId + "</op:service-id>" + SipUtils.CRLF);
            mIMSInfo.append("<op:version>" + serviceVerion + "</op:version>" + SipUtils.CRLF);
            mIMSInfo.append("<op:description>" + serviceDesc + "</op:description>" + SipUtils.CRLF);
            mIMSInfo.append("</op:service-description>" + SipUtils.CRLF);
            mIMSInfo.append("<contact>" + contact + "</contact>" + SipUtils.CRLF);
            mIMSInfo.append("</tuple>" + SipUtils.CRLF);
        }
    }

    public void addGeoloc(String tupleId, String status, Geoloc geoloc, 
                                                 String contact) {
        addGeoloc(tupleId, status, 3, geoloc, "GPS", contact);
    }

    public void addGeoloc(String tupleId, String status, int dimension,
                                                Geoloc geoloc, String method,
                                                String contact) {

        mGeolocInfo.append("<tuple id=\"" + tupleId + "\">" + SipUtils.CRLF);
        mGeolocInfo.append("<status><basic>" + status + "</basic></status>" + SipUtils.CRLF);
        mGeolocInfo.append("<gp:geopriv>" + SipUtils.CRLF);
        mGeolocInfo.append("<gp:location-info>" + SipUtils.CRLF);
        mGeolocInfo.append("<gml:location>" + SipUtils.CRLF);
        mGeolocInfo.append("<gml:Point srsDimension=\"" + dimension+ "\">" + SipUtils.CRLF);        
        mGeolocInfo.append("<gml:pos>");
        mGeolocInfo.append(geoloc.getLatitude() + " ");
        mGeolocInfo.append(geoloc.getLongitude() + " ");
        mGeolocInfo.append(geoloc.getAltitude());
        mGeolocInfo.append("</gml:pos>" + SipUtils.CRLF);        
        mGeolocInfo.append("</gml:Point>" + SipUtils.CRLF);        
        mGeolocInfo.append("</gml:location>" + SipUtils.CRLF);
        mGeolocInfo.append("</gp:location-info>" + SipUtils.CRLF);
        mGeolocInfo.append("<gp:method>" + method+ "</gp:method>" + SipUtils.CRLF);
        mGeolocInfo.append("</gp:geopriv>" + SipUtils.CRLF);
        mGeolocInfo.append("<contact>" + contact + "</contact>" + SipUtils.CRLF);
        mGeolocInfo.append("</tuple>" + SipUtils.CRLF);
    }

    public void addPersonInfo(PresenceInfo info) {
        mPersionInfo.append("<pdm:person id=\"p1\">" + SipUtils.CRLF);
        mPersionInfo.append("<op:overriding-willingness>" + SipUtils.CRLF);
        mPersionInfo.append("<op:basic>" + info.getPresenceStatus() + "</op:basic>" + SipUtils.CRLF);
        mPersionInfo.append("</op:overriding-willingness>" + SipUtils.CRLF);

        FavoriteLink favoriteLink = info.getFavoriteLink();
        if ((favoriteLink != null) && (favoriteLink.getLink() != null)) {
            mPersionInfo.append("<ci:homepage>" + StringUtils.encodeUTF8(
                StringUtils.encodeXML(favoriteLink.getLink())) + "</ci:homepage>" + SipUtils.CRLF);
        }

        PhotoIcon photoIcon = info.getPhotoIcon();
        if ((photoIcon != null) && (photoIcon.getEtag() != null)) {
            mPersionInfo.append(
                    "  <rpid:status-icon opd:etag=\"" + photoIcon.getEtag() +
                            "\" opd:fsize=\"" + photoIcon.getSize() +
                            "\" opd:contenttype=\"" + photoIcon.getType() +
                            "\" opd:resolution=\"" + photoIcon.getResolution() + "\">" +
                            "</rpid:status-icon>" + SipUtils.CRLF);
        }

        String freetext = info.getFreetext();
        if (freetext != null) {
            mPersionInfo.append("<pdm:note>" + StringUtils.encodeUTF8(StringUtils.encodeXML(freetext)) + "</pdm:note>" + SipUtils.CRLF);
        }
        mPersionInfo.append("</pdm:person>" + SipUtils.CRLF);
    }

    //AOSP procedure
    public void addMediaCapability(String tupleId, String status, boolean audioSupport,
             boolean videoSupport, String serviceId, String serviceVerion,
             String serviceDesc, String contact) {
        mMediaInfo.append("<tuple id=\"" + tupleId + "\">" + SipUtils.CRLF);
        mMediaInfo.append("<status><basic>" + status + "</basic></status>" + SipUtils.CRLF);
        mMediaInfo.append("<caps:servcaps>" + SipUtils.CRLF);
        mMediaInfo.append("<caps:audio>" + audioSupport + "</caps:audio>" + SipUtils.CRLF);
        mMediaInfo.append("<caps:video>" + videoSupport + "</caps:video>" + SipUtils.CRLF);
        mMediaInfo.append("</caps:servcaps>" + SipUtils.CRLF);

        mMediaInfo.append("<op:service-description>" + SipUtils.CRLF);
        mMediaInfo.append("<op:service-id>" + serviceId + "</op:service-id>" + SipUtils.CRLF);
        mMediaInfo.append("<op:version>" + serviceVerion + "</op:version>" + SipUtils.CRLF);
        mMediaInfo.append("<op:description>" + serviceDesc + "</op:description>" + SipUtils.CRLF);
        mMediaInfo.append("</op:service-description>" + SipUtils.CRLF);
        mMediaInfo.append("<contact>" + contact + "</contact>" + SipUtils.CRLF);
        mMediaInfo.append("</tuple>" + SipUtils.CRLF);
    }

    public String build() {
        StringBuilder builder = new StringBuilder();
        builder.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
                " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
                " xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
                " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
                " xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\""
                + " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\""
                + " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\""
                + " entity=\"" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF);
        builder.append(mRCSInfo.toString());
        builder.append(mIMSInfo.toString());
        builder.append(mGeolocInfo.toString());
        builder.append(mPersionInfo.toString());
        builder.append(mMediaInfo.toString());
        // Add last header
        builder.append("</presence>" + SipUtils.CRLF);

        mRCSInfo.setLength(0);
        mIMSInfo.setLength(0);
        mGeolocInfo.setLength(0);
        mPersionInfo.setLength(0);
        mMediaInfo.setLength(0);
        return builder.toString();
    }
}

