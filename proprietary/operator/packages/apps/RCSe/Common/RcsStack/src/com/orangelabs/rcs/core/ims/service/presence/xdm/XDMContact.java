package com.orangelabs.rcs.core.ims.service.presence.xdm;

/**
 * 
 * XDMContact class
 * 
 */
public class XDMContact {
    String contact;
    String displayName;
   

    public XDMContact() {
        this.contact = "";
        this.displayName = "";
    }

    public XDMContact(String contact) {
        this.contact = contact;
        this.displayName = contact;
    } 
    public XDMContact(String contact, String displayName) {
        this.contact = contact;
        this.displayName = displayName;
    } 
    
    public String getContact() {
        return contact;
    }
    public String getDisplayname() {
        return displayName;
    }
    public void setDisplayName(String displayName){
        this.displayName = displayName;
    }

};