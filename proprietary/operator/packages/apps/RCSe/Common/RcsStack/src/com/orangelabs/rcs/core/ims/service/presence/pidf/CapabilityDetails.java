package com.orangelabs.rcs.core.ims.service.presence.pidf;

public class CapabilityDetails {

    String name;
    String value;
    
    CapabilityDetails(String name, String value)
    {
        this.name = name;
        this.value = value;
    }
    
    void setValue(String value){
        this.value = value;
    }
    
    public String getValue(){
        return value;
    }
    public String getName(){
        return name;
    }
    
}
