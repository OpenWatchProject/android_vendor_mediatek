package com.orangelabs.rcs.core.ims.security;

/** Support_IMS SIM
 * @author sub
 *
 */
public class Http_authorization {
        private String method;
        private String realm;
        private String nonce;
        private String opaque;
        private String algorithm;
        private String userid;
        private String password;
        private String digest_uri;

        String response_digest;
        char[] response_aka;

        public Http_authorization(String method, String realm, String nonce,
                        String opaque, String algorithm, String userid, String password,
                        String digest_uri) {
                super();
                this.method = method;
                this.realm = realm;
                this.nonce = nonce;
                this.opaque = opaque;
                this.algorithm = algorithm;
                this.userid = userid;
                this.password = password;
                this.digest_uri = digest_uri;
                response_aka = new char[16];
                System.out.println(this.method + " " + this.realm + " " + this.nonce
                                + " " + this.opaque + " " + this.algorithm + " "
                                + this.userid + " " + this.password + " " + this.digest_uri);
        }

        public String getMethod() {
                return method;
        }

        public void setMethod(String method) {
                this.method = method;
        }

        public String getRealm() {
                return realm;
        }

        public void setRealm(String realm) {
                this.realm = realm;
        }

        public String getNonce() {
                return nonce;
        }

        public void setNonce(String nonce) {
                this.nonce = nonce;
        }

        public String getOpaque() {
                return opaque;
        }

        public void setOpaque(String opaque) {
                this.opaque = opaque;
        }

        public String getAlgorithm() {
                return algorithm;
        }

        public void setAlgorithm(String algorithm) {
                this.algorithm = algorithm;
        }

        public String getUserid() {
                return userid;
        }

        public void setUserid(String userid) {
                this.userid = userid;
        }

        public String getPassword() {
                return password;
        }

        public void setPassword(String password) {
                this.password = password;
        }

        public String getDigest_uri() {
                return digest_uri;
        }

        public void setDigest_uri(String digest_uri) {
                this.digest_uri = digest_uri;
        }

        public String getResponse_digest() {
                return response_digest;
        }

        public void setResponse_digest(String response_digest) {
                this.response_digest = response_digest;
        }

        public char[] getResponse_aka() {
                return response_aka;
        }

        public void setResponse_aka(char[] response_aka) {
                this.response_aka = response_aka;
        }

        
}


