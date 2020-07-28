package com.orangelabs.rcs.core.ims.service.presence.pidf;

import java.util.ArrayList;
import java.util.HashMap;

public class ServiceCapability {

    private ArrayList<CapabilityDetails> capabilityList = new ArrayList<CapabilityDetails>();
    
    void addCapability(CapabilityDetails capability)
    {
        capabilityList.add(capability);
    }
    
    public ArrayList<CapabilityDetails> getCapabilityList()
    {
        return capabilityList;
    }
}
