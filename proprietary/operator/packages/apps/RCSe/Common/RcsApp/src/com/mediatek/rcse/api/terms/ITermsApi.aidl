package com.mediatek.rcse.api.terms;

/**
 * Terms & conditions API
 */
interface ITermsApi {
    // Accept terms and conditions via SIP
    boolean acceptTerms(in String id, in String pin);

    // Reject terms and conditions via SIP
    boolean rejectTerms(in String id, in String pin);

}
