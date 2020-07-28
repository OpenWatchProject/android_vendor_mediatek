/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.mediatek.presence.core.ims.userprofile;

import java.util.ListIterator;
import java.util.Vector;

import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.provider.settings.RcsSettings;

import javax2.sip.header.ExtensionHeader;
import javax2.sip.header.Header;

/**
 * User profile
 *
 * @author JM. Auffret
 */
public class UserProfile {

    /**
     * User name
     */
    private String username;

    /**
     * @tct-stack wuquan add
     * User name
     */
    private String username_full;

    /**
     * Private ID for HTTP digest
     */
    private String privateID;

    /**
     * Password for HTTP digest
     */
    private String password;

    /**
     * Realm for HTTP digest
     */
    private String realm;

    /**
     * Home domain
     */
    private String homeDomain;

    /**
     * XDM server address
     */
    private String xdmServerAddr;

    /**
     * XDM server login
     */
    private String xdmServerLogin;

    /**
     * XDM server password
     */
    private String xdmServerPassword;

    /**
     * IM conference URI
     */
    private String imConferenceUri;

    /**
     * IM conference URI
     */
    private String imMultiConferenceUri;

    /**
     * Associated URIs
     */
    private Vector<String> associatedUriList = new Vector<String>();

    /**
     * Preferred URI
     */
    private String preferredUri = null;

    /**
     * Preferred Username
     */
    private String preferredUsername = null;

    /**
     * M:Access Network Info.@{T-Mobile
     */
    public static String currentAccessNetworkInfo;
    /**
     * @}
     */

    /**
     * M:Last Access Network Info.@{T-Mobile
     */
    public static String lastAccessNetworkInfo;


    //USER-AGENT
    private static String userAgentSIP ;
    /**
     * @}
     */
    /**

    /**
     * Constructor
     *
     * @param username Username
     * @param homeDomain Home domain
     * @param privateID Private id
     * @param password Password
     * @param realm Realm
     * @param xdmServerAddr XDM server address
     * @param xdmServerLogin Outbound proxy address
     * @param xdmServerPassword Outbound proxy address
     * @param imConferenceUri IM conference factory URI
     */
    public UserProfile(String username,
            String homeDomain,
            String privateID,
            String password,
            String realm,
            String xdmServerAddr,
            String xdmServerLogin,
            String xdmServerPassword,
            String imConferenceUri,
            String username_full,
            String imMultiConferenceUri) {

        this.username = username;
        this.homeDomain = homeDomain;
        this.privateID = privateID;
        this.password = password;
        this.realm = realm;
        this.xdmServerAddr = xdmServerAddr;
        this.xdmServerLogin = xdmServerLogin;
        this.xdmServerPassword = xdmServerPassword;
        this.imConferenceUri = imConferenceUri;
        this.username_full = username_full;//@tct-stack wuquan add
        this.imMultiConferenceUri = imMultiConferenceUri;
        this.userAgentSIP = "";
    }

    /**
     * Get the user name
     *
     * @return Username
     */
    public String getUsername() {
        if (preferredUsername == null)
            return username;

        return preferredUsername;
    }

    /**
     * Set the user name
     *
     * @param username Username
     */
    public void setUsername(String username) {
        this.username = username;
    }

    /**
     * @tct-stack wuquan add
     * Get the user name
     *
     * @return Username
     */
    public String getUsername_full() {
        return username_full;
    }

    /**
     * Set the user name
     *
     * @param username Username
     */
    public void setUsername_full(String username_full) {
        this.username_full = username_full;
    }
    //@tct-stack wuquan end

    /**
     * Get the user preferred URI
     *
     * @return Preferred URI
     */
    public String getPreferredUri() {
        return preferredUri;
    }

    /**
     * Get the user public URI
     *
     * @return Public URI
     */
    public String getPublicUri() {
        if (preferredUri == null) {
            return "sip:" + username + "@" + homeDomain;
        } else {
            return preferredUri;
        }
    }

    /**
     * Set the user associated URIs
     *
     * @param uris List of URIs
     */
    public void setAssociatedUri(ListIterator<Header> uris) {
        if (uris == null) {
            return;
        }

        String sipUri = null;
        String telUri = null;

        // Take the first URI in associated URI list as default URI
        ExtensionHeader header = (ExtensionHeader)uris.next();
        String value = header.getValue();
        value = SipUtils.extractUriFromAddress(value);
        associatedUriList.addElement(value);

        if (value.startsWith("sip:")) {
            sipUri = value;
        } else {
            if (value.startsWith("tel:")) {
                telUri = value;
            }
        }

        if (telUri != null) {
            preferredUri = telUri;
            preferredUsername = telUri.substring(4).trim();
        } else if (sipUri != null) {
            preferredUri = sipUri;
            preferredUsername = sipUri.substring(4, sipUri.indexOf('@')).trim();
        }
    }

    public void setAssociatedUri(String[] uris) {
        if (uris == null)
            return;
        String sipUri = null;
        String telUri = null;

        // Take the first URI in associated URI list as default URI
        if (uris[0].startsWith("sip:")) {
            sipUri = uris[0];
        } else if (uris[0].startsWith("tel:")) {
            telUri = uris[0];
        }

        if (telUri != null) {
            preferredUri = telUri;
            preferredUsername = telUri.substring(4).trim();
        } else if (sipUri != null) {
            preferredUri = sipUri;
            preferredUsername = sipUri.substring(4, sipUri.indexOf('@')).trim();
        }
    }

    /**
     * Get the private ID used for HTTP Digest authentication
     *
     * @return Private ID
     */
    public String getPrivateID() {
        return privateID;
    }

    /**
     * Returns the password used for HTTP Digest authentication
     *
     * @return Password
     */
    public String getPassword() {
        return password;
    }

    /**
     * Returns the realm used for HTTP Digest authentication
     *
     * @return Realm
     */
    public String getRealm() {
        return realm;
    }

    /**
     * Returns the home domain
     *
     * @return Home domain
     */
    public String getHomeDomain() {
        return homeDomain;
    }

    /**
     * Set the home domain
     *
     * @param domain Home domain
     */
    public void setHomeDomain(String domain) {
        this.homeDomain = domain;
    }

    /**
     * Set the XDM server address
     *
     * @param addr Server address
     */
    public void setXdmServerAddr(String addr) {
        this.xdmServerAddr = addr;
    }

    /**
     * Returns the XDM server address
     *
     * @return Server address
     */
    public String getXdmServerAddr() {
        return xdmServerAddr;
    }

    /**
     * Set the XDM server login
     *
     * @param login Login
     */
    public void setXdmServerLogin(String login) {
        this.xdmServerLogin = login;
    }

    /**
     * Returns the XDM server login
     *
     * @return Login
     */
    public String getXdmServerLogin() {
        return xdmServerLogin;
    }

    /**
     * Set the XDM server password
     *
     * @param pwd Password
     */
    public void setXdmServerPassword(String pwd) {
        this.xdmServerPassword = pwd;
    }

    /**
     * Returns the XDM server password
     *
     * @return Password
     */
    public String getXdmServerPassword() {
        return xdmServerPassword;
    }

    /**
     * Set the IM conference URI
     *
     * @param uri URI
     */
    public void setImConferenceUri(String uri) {
        this.imConferenceUri = uri;
    }

    /**
     * Returns the IM conference URI
     *
     * @return URI
     */
    public String getImConferenceUri() {
        return imConferenceUri;
    }

    /**
     * Set the IM Multi conference URI
     *
     * @param uri URI
     */
    public void setMultiImConferenceUri(String uri) {
        this.imMultiConferenceUri = uri;
    }

    /**
     * Returns the IM Multi conference URI
     *
     * @return URI
     */
    public String getMultiImConferenceUri() {
        return imMultiConferenceUri;
    }

    /**
     * M:Returns the information of last access. @{T-Mobile
     */
    /**
     * Returns the last access position information
     *
     * @return The last access position information
     */
    public String getLastAccessNetworkInfo() {
        return this.lastAccessNetworkInfo;
    }

    /**
     * M:Returns the information of current access. @{T-Mobile
     */
    /**
     * Returns the current access position information
     *
     * @return The current access position information
     */
    public String getAccessNetworkInfo() {
        return this.currentAccessNetworkInfo;
    }

    /**
     * @}
     */

    /**
     * Returns the profile value as string
     *
     * @return String
     */
    @Override
    public String toString() {
        String result = "IMS username=" + username + ", "
                + "IMS private ID=" + privateID + ", "
                + "IMS password=" + password + ", "
                + "IMS home domain=" + homeDomain + ", "
                + "XDM server=" + xdmServerAddr + ", "
                + "XDM login=" + xdmServerLogin + ", "
                + "XDM password=" + xdmServerPassword + ", "
                + "IM Conference URI=" + imConferenceUri;
        return result;
    }

    public void setUserAgent(String userAgent){
    	this.userAgentSIP = userAgent;
    }

    public String getUserAgent(){
    	return this.userAgentSIP;
    }
}
