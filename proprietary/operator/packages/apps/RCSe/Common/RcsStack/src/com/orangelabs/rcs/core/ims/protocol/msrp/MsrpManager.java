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

package com.orangelabs.rcs.core.ims.protocol.msrp;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;


import java.io.IOException;
import java.io.InputStream;

import com.orangelabs.rcs.utils.IpAddressUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import java.util.Vector;
/**
 * MSRP manager
 * 
 * @author jexa7410
 */
public class MsrpManager {
    /**
     * Local MSRP address
     */
    private String localMsrpAddress;

    /**
     * Local MSRP port
     */
    private int localMsrpPort;

    /**
     * MSRP session
     */
    private MsrpSession msrpSession = null;
    
    /**
     * Session Id
     */
    private long sessionId;
   
    /**
     * Secured connection
     */
    private boolean secured = false;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     * 
     * @param localIpAddress Local MSRP address
     * @param localMsrpPort Local MSRP port
     */    
    public MsrpManager(String localMsrpAddress, int localMsrpPort) {
        this.localMsrpAddress = localMsrpAddress;
        this.localMsrpPort = localMsrpPort;
        this.sessionId = System.currentTimeMillis();
    }

    /**
     * Returns the local MSRP port
     * 
     * @return Port number
     */
    public int getLocalMsrpPort() {
        return localMsrpPort;
    }
    
    /**
     * Get the local socket protocol path
     * 
     * @return Protocol
     */
    public String getLocalSocketProtocol() {
        if (secured) {
            return MsrpConstants.SOCKET_MSRP_SECURED_PROTOCOL;
        } else {
            return MsrpConstants.SOCKET_MSRP_PROTOCOL;
        }
    }
    
    public void setCpimType(boolean isCpimContent){
        msrpSession.setCpimContent(isCpimContent);
    }

    public void setCPIM(String cpim){
    	msrpSession.setCPIM(cpim);
    }
    
    /**
     * Get the local MSRP path
     * 
     * @return MSRP path
     */
    public String getLocalMsrpPath() {
        if (IpAddressUtils.isIPv6(localMsrpAddress)) {
            if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()){
               return getMsrpProtocol() + "://[" + localMsrpAddress + "]:" + localMsrpPort + "/" + sessionId + ";tcp";
             }
            else{
            return getMsrpProtocol() + "://[" + localMsrpAddress + "]:" + localMsrpPort + "/" + sessionId + ";tcp";
            }
        } else {
            return getMsrpProtocol() + "://" + localMsrpAddress + ":" + localMsrpPort + "/" + sessionId + ";tcp";
        }
    }
    
        /** M: add msrps path for MSRPoTLS @{ */
    /**
     * Get the local MSRPs path
     * 
     * @return MSRPs path
     */
    public String getLocalMsrpsPath() {
        if (IpAddressUtils.isIPv6(localMsrpAddress)) {
            return "msrps://[" + localMsrpAddress + "]:" + localMsrpPort + "/" + sessionId + ";tcp";
        } else {
            return "msrps://" + localMsrpAddress + ":" + localMsrpPort + "/" + sessionId + ";tcp";
        }
    }

    /** @} */

    /**
     * Get the MSRP protocol
     * 
     * @return MSRP protocol
     */
    public String getMsrpProtocol() {
        if (secured) {
            return MsrpConstants.MSRP_SECURED_PROTOCOL;
        } else {
            return MsrpConstants.MSRP_PROTOCOL;
        }
    }
    
    /**
     * Return the MSRP session
     * 
     * @return MSRP session
     */
    public MsrpSession getMsrpSession() {
        return msrpSession;
    }
    /**
     * Is secured
     * 
     * @return Boolean
     */
    public boolean isSecured() {
        return secured;
    }
    
    /**
     * Set secured
     * 
     * @param flag Boolean flag
     */
    public void setSecured(boolean flag) {
        this.secured = flag;
    }
    
    /**
     * Is established
     * 
     * @return true If the empty packet was sent successfully
     */
    public boolean isEstablished() {
        return msrpSession != null /*&& msrpSession.isEstablished()*/;
    }
    
    /**
     * Open the MSRP session
     * 
     * @throws IOException
     */
    public void openMsrpSession() throws IOException {
        if (logger.isActivated()) {
                   logger.error("ABC openMsrpSession1: ");
           }
        if ((msrpSession == null) || (msrpSession.getConnection() == null)) {
            throw new IOException("Session not yet created");
        }
        
        msrpSession.getConnection().open();
    }
    
    /**
     * Open the connection with SO_TIMEOUT on the socket
     * 
     * @param timeout Timeout value (in seconds)
     * @throws IOException
     */
    public void openMsrpSession(int timeout) throws IOException {
        if (logger.isActivated()) {
                   logger.error("ABC openMsrpSession2: ");
           }
        if ((msrpSession == null) || (msrpSession.getConnection() == null)) {
            throw new IOException("Session not yet created");
        }

        msrpSession.getConnection().open(timeout);
    }
    
    /**
     * Create a MSRP client session
     * 
     * @param remoteHost Remote host
     * @param remotePort Remote port
     * @param remoteMsrpPath Remote MSRP path
     * @param listener Event listener
     * @return Created session
     * @throws MsrpException
     */
    public MsrpSession createMsrpClientSession(String remoteHost, int remotePort, String remoteMsrpPath, MsrpEventListener listener) throws MsrpException {
        try {
            /** 
             * M: Create MSRP session according the configuration @{ 
             */
            if (logger.isActivated()) {
                logger.info("ABC Create MSRP client end point at " + remoteHost + ":" + remotePort + ", remoteMsrpPath = " + remoteMsrpPath);
            }
    
            // Create a new MSRP session
            msrpSession = new MsrpSession();
            msrpSession.setFrom(getLocalMsrpPath());
            msrpSession.setTo(remoteMsrpPath);

            if (isSecured()) {
                if (logger.isActivated()) {
                    logger.info("Create a MSRPs client connection.");
                }
                msrpSession.setFrom(getLocalMsrpsPath());
                // Create a MSRPs client connection
                final MsrpConnection connection = new MsrpsClientConnection(msrpSession,
                        remoteHost, remotePort);

            // Associate the connection to the session
            msrpSession.setConnection(connection);
            } else {
                if (logger.isActivated()) {
                    logger.info("Create a MSRP client connection");
                }
                msrpSession.setFrom(getLocalMsrpPath());
                // Create a MSRP client connection
                final MsrpConnection connection = new MsrpClientConnection(msrpSession, remoteHost,remotePort,localMsrpAddress,localMsrpPort);
                // Associate the connection to the session
                msrpSession.setConnection(connection);
            }
            /** 
             * @} 
             */
            
            // Add event listener
            msrpSession.addMsrpEventListener(listener);
            
            // Return the created session
            return msrpSession;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("ABC Can't create the MSRP client session", e);
            }
            throw new MsrpException("Create MSRP client session has failed");
        }
    }
    
    /**
     * Create a MSRP server session
     *
     * @param remoteMsrpPath Remote MSRP path
     * @param listener Event listener
     * @return Created session
     * @throws MsrpException
     */
    public MsrpSession createMsrpServerSession(String remoteMsrpPath, MsrpEventListener listener) throws MsrpException {
        if (logger.isActivated()) {
            logger.info("ABC Create MSRP server end point at " + localMsrpPort + ", remoteMsrpPath = " + remoteMsrpPath);
        }

        // Create a MSRP session
        msrpSession = new MsrpSession();
        msrpSession.setFrom(getLocalMsrpPath());
        msrpSession.setTo(remoteMsrpPath);


        /** 
         * M: Create MSRP session according the configuration @{ 
         */
        if (isSecured()) {
            if (logger.isActivated()) {
                logger.debug("ABC Create a MSRPs server connection");
            }
            msrpSession.setFrom(getLocalMsrpsPath());
            // Create a MSRP server connection
            final MsrpConnection connection = new MsrpsServerConnection(msrpSession, localMsrpPort);
            // Associate the connection to the session
            msrpSession.setConnection(connection);
        } else {
            if (logger.isActivated()) {
                logger.debug("ABCCreate a MSRP server connection");
            }
            msrpSession.setFrom(getLocalMsrpPath());
            // Create a MSRP server connection
            final MsrpConnection connection = new MsrpServerConnection(msrpSession, localMsrpPort);
            // Associate the connection to the session
            msrpSession.setConnection(connection);
        }
        /** 
         * @} 
         */
        
        // Add event listener
        msrpSession.addMsrpEventListener(listener);

        // Return the created session
        return msrpSession;
    }

    
    public void sendChunks(InputStream inputStream, String msgId, String contentType, long contentSize) throws MsrpException {
    	sendChunks( inputStream,  msgId,  contentType,  contentSize,false);
    }

    /**
     * Send data chunks
     * 
     * @param inputStream Input stream
     * @param msgId Message ID
     * @param contentType Content type
     * @param contentSize Content size
     * @throws MsrpException
     */
    public void sendChunks(InputStream inputStream, String msgId, String contentType, long contentSize, boolean noFailureFlag) throws MsrpException {
        if (msrpSession == null) {
            throw new MsrpException("ABC sendChunks MSRP session is null");
        }

        if(noFailureFlag==true)
        	msrpSession.sendChunks(inputStream, msgId, contentType, contentSize,noFailureFlag);
        else
        msrpSession.sendChunks(inputStream, msgId, contentType, contentSize);

    }
    
    /**
     * Send an empty chunk
     * 
     * @throws MsrpException
     */
    public void sendEmptyChunk() throws MsrpException {
        if (msrpSession == null) {
            throw new MsrpException("MSRP session is null");
        }

        msrpSession.sendEmptyChunk();
    }
    
    /**
     * Close the MSRP session
     */
    public synchronized void closeSession() {
        if (msrpSession != null) {
            if (logger.isActivated()) {
                logger.info("Close the MSRP session");
            }
            try {
                msrpSession.close();
            } catch(Exception e) {
                // Intentionally blank
            }
            msrpSession = null;
        }
    }
    
    /**
     * Close the MSRP session
     */
    public synchronized void abortSession() {
        if (msrpSession != null) {
            if (logger.isActivated()) {
                logger.info("Close the MSRP session");
            }
            try {
                msrpSession.abort();
            } catch(Exception e) {
                // Intentionally blank
            }
            msrpSession = null;
        }
    }
    
    /**
     * Close the MSRP session
     */
   /* public synchronized void closeChatSession() {
        if (msrpSession != null) {
            if (logger.isActivated()) {
                logger.info("Close chat  MSRP session");
            }
            try {
                msrpSession.closeChat();
            } catch(Exception e) {
                // Intentionally blank
            }
            msrpSession = null;
        }
    }
*/

    public MsrpSession createMsrpSession(byte[] sdp, MsrpEventListener listener) {
        SdpParser parser = new SdpParser(sdp);

        Vector<MediaDescription> media = parser.getMediaDescriptions();
        MediaDescription mediaDesc = media.elementAt(0);
        MediaAttribute pathAttribute = mediaDesc.getMediaAttribute("path");
        String remoteMsrpPath = pathAttribute.getValue();

        // Create the MSRP session
        MsrpSession session = null;
        MediaAttribute setupAttribute = mediaDesc.getMediaAttribute("setup");
        String setup = null;
        if (setupAttribute != null) {
            setup = setupAttribute.getValue();
        } else {
            logger.error("Media attribute \"setup\" is missing!");
            logger.warn("media=" + mediaDesc.toString());
            if (mediaDesc.mediaAttributes != null)
                for (MediaAttribute attribute : mediaDesc.mediaAttributes) {
                    logger.warn("attribute key=" + attribute.getName() + " value="
                            + attribute.getValue());
                }

        }
        // if remote peer is active this client needs to be passive (i.e. act as server)
        try{
        if ("active".equalsIgnoreCase(setup)) {
            session = createMsrpServerSession(remoteMsrpPath, listener);
        } else {
            String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaDesc);
            int remotePort = mediaDesc.port;
            String fingerprint = SdpUtils.extractFingerprint(parser, mediaDesc);
            session = createMsrpClientSession(remoteHost, remotePort, remoteMsrpPath, listener);
        }
        }catch(Exception e){
        	
        }

        return session;
    }

}
