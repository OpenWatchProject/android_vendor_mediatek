package com.mediatek.presence.core.ims.service.presence.extension;

public interface PresenceExtensionListener {

      /**
     * Capability has chnaged for a presecne extension
     */
    public void handleCapabilityChanged(PresenceExtension extension);

}
