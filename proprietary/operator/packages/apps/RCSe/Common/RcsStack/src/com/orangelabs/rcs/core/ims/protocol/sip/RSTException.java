
package com.orangelabs.rcs.core.ims.protocol.sip;

/**
 * SIP exception
 * 
 * @author JM. Auffret
 */
public class RSTException extends java.lang.Exception {
    static final long serialVersionUID = 1L;
    
    /**
     * Constructor
     *
     * @param error Error message
     */
    public RSTException(String error) {
        super(error);
    }
}
