package com.mediatek.optin.parser;

import java.util.ArrayList;

/** Class containing all fields present in request/response in hierarchial manner.
 * If parsed data is null, do not pass null fileds in response.
 * This will reduce null checks effort in UI. Instead pass empty fields.
 */
public class RequestResponse {

    public static final int NO_REQUEST = -1;
    public static final int QUERY_REQUEST = NO_REQUEST + 1;
    public static final int ADD_REQUEST = NO_REQUEST + 2;
    public static final int PARTIAL_VALIDATION_REQUEST = NO_REQUEST + 3;
    public static final int DELETE_REQUEST = NO_REQUEST + 4;

    /** Uppermost class of a request.
    */
    public static class Request {
        public UserDetail details;

        /** Constructor.
         * @param reqType reqType
         * @param mdn mdn
         * @param isPartialValidation isPartialValidation
         * @param imei imei
         * @param uuid uuid
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         */
        public Request(String reqType, String isPartialValidation, String mdn, String imei,
                String uuid, String houseNumber, String road, String location, String city,
                String state, String zip, String country) {
            details = new UserDetail(reqType, isPartialValidation, mdn, imei, uuid, houseNumber,
                    road, location, city, state, zip, country);
        }
    }

    /** Uppermost class of add/update response.
    */
    public static class AddUpdateAddressResponse {
        public ErrorCode errStatus;
        public UserDetail userDetail;
        public ArrayList<Address> altAddress;

        /** Constructor.
         * @param code code
         * @param level level
         * @param message message
         * @param reqType reqType
         * @param isPartialValidation isPartialValidation
         * @param mdn mdn
         * @param imei imei
         * @param uuid uuid
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         */
        public AddUpdateAddressResponse(String code, String level, String message,
                String reqType, String isPartialValidation, String mdn, String imei, String uuid,
                String houseNumber, String road, String location, String city, String state,
                String zip, String country) {
            errStatus = new ErrorCode(code, level, message);
            userDetail =  new UserDetail(reqType, isPartialValidation, mdn, imei, uuid, houseNumber,
                    road, location, city, state, zip, country);
        }

        /** Constructor.
         * @param errDetails errDetails
         *@param userDetails userDetails
         *@param altAddress altAddress
         */
        public AddUpdateAddressResponse(ErrorCode errDetails, UserDetail userDetails,
                ArrayList<Address> altAddress) {
            errStatus = new ErrorCode(errDetails);
            userDetail =  new UserDetail(userDetails);
            this.altAddress = altAddress;
        }

        private void addAltAddress(ArrayList<Address> altAddress) {
            // TODO:
        }
    }

    /** Uppermost class of query response.
    */
    public static class QueryAddressResponse {
        public ErrorCode errStatus;
        public UserDetail userDetail;

        /** Constructor.
         * @param code code
         *@param level level
         *@param message message
         * @param reqType reqType
         * @param isPartialValidation isPartialValidation
         * @param mdn mdn
         * @param imei imei
         * @param uuid uuid
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         */
        public QueryAddressResponse(String code, String level, String message,
                String reqType, String isPartialValidation, String mdn, String imei, String uuid,
                String houseNumber, String road, String location, String city, String state,
                String zip, String country) {
            errStatus = new ErrorCode(code, level, message);
            userDetail =  new UserDetail(reqType, isPartialValidation, mdn, imei, uuid, houseNumber,
                    location, road, city, state, zip, country);
        }

        /** Constructor.
         * @param errDetails errDetails
         * @param userDetails userDetails
         */
        public QueryAddressResponse(ErrorCode errDetails, UserDetail userDetails) {
            errStatus = new ErrorCode(errDetails);
            userDetail =  new UserDetail(userDetails);
        }
    }

    /** Class containing error info.
     */
    public static class ErrorCode {
        public String errCode;
        public String errLevel;
        public String errMessage;

        /** Constructor.
         */
        public ErrorCode() {}

        /** Constructor.
         * @param code code
         * @param level level
         * @param message message
         */
        public ErrorCode(String code, String level, String message) {
            setParams(errCode, errLevel, errMessage);
        }

        /** Constructor.
         * @param details details
         */
        public ErrorCode(ErrorCode details) {
            if (details != null) {
                setParams(details.errCode, details.errLevel, details.errMessage);
            }
        }

        /** To intialize empty error info.
         * @return
         */
        public void setParams() {
        }
        /** To intialize error info.
         * @param code code
         * @param level level
         * @param message message
         * @return
         */
        public void setParams(String code, String level, String message) {
            errCode = code;
            errLevel = level;
            errMessage = message;
        }

        /** To convert error info in string.
         * @return String
         */
        public String toString() {
            return "code:" + errCode + ", level:" + errLevel + ", msg:" + errMessage;
        }
    }

    /** Class containing Address info.
    */
    public static class Address {
        public String houseNumber;
        public String road;
        public String location;
        public String city;
        public String state;
        public String zip;
        public String country;

        /** Constructor.
         */
        public Address() {}

        /** Constructor.
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         */
        public Address(String houseNumber, String road, String location, String city,
                String state, String zip, String country) {
            this.houseNumber = houseNumber;
            this.road = road;
            this.location = location;
            this.city = city;
            this.state = state;
            this.zip = zip;
            this.country = country;
        }

        /** Constructor.
         * @param details address details
         */
        public Address(Address details) {
            if (details == null) {
                setParams(null, null, null, null, null, null, null);
            } else {
                setParams(details);
            }
        }

        /** Method to set address info.
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         * @return
         */
        public void setParams(String houseNumber, String road, String location, String city,
                String state, String zip, String country) {
            this.houseNumber = houseNumber;
            this.road = road;
            this.location = location;
            this.city = city;
            this.state = state;
            this.zip = zip;
            this.country = country;
        }

        /** Method to set address info.
         * @param details address details
         * @return
         */
        public void setParams(Address details) {
            this.houseNumber = details.houseNumber;
            this.road = details.road;
            this.location = details.location;
            this.city = details.city;
            this.state = details.state;
            this.zip = details.zip;
            this.country = details.country;
        }

        /** To convert address info in string.
         * @return String
         */
        public String toString() {
            return "house number:" + houseNumber + ", road:" + road + ", location" + location +
                    ", city:" + city + ", state:" + state + ", zip:" + zip + "country:" + country;
        }
    }

    /** Class containing User details info.
    */
    public static class UserDetail {
        public String reqType;
        public String isPartialValidation;
        public String mdn;
        public String imei;
        public String uuid;
        public Address userAddress;

        /** Constructor.
         */
        public UserDetail() {
            userAddress = new Address();
        }

        /** Constructor.
         * @param reqType reqType
         * @param isPartialValidation isPartialValidation
         * @param mdn mdn
         * @param imei imei
         * @param uuid uuid
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         */
        public UserDetail(String reqType, String isPartialValidation, String mdn, String imei,
                String uuid, String houseNumber, String road, String location, String city,
                String state, String zip, String country) {
            this.reqType = reqType;
            this.isPartialValidation = isPartialValidation;
            this.mdn = mdn;
            this.imei = imei;
            this.uuid = uuid;
            userAddress = new Address(houseNumber, road, location, city, state, zip, country);
        }

        /** Constructor.
         * @param details details
         */
        public UserDetail(UserDetail details) {
            if (details != null) {
                userAddress = new Address();
                setParams(details.reqType, details.isPartialValidation, details.mdn, details.imei,
                        details.uuid, details.userAddress);
            }
        }

        /** method to set User details.
         * @param reqType reqType
         * @param isPartialValidation isPartialValidation
         * @param mdn mdn
         * @param imei imei
         * @param uuid uuid
         * @param houseNumber houseNumber
         * @param road road
         * @param location location
         * @param city city
         * @param state state
         * @param zip zip
         * @param country country
         * @return
         */
        public void setParams(String reqType, String isPartialValidation, String mdn,
                String imei, String uuid, String houseNumber, String road, String location,
                String city, String state, String zip, String country) {
            this.reqType = reqType;
            this.isPartialValidation = isPartialValidation;
            this.mdn = mdn;
            this.imei = imei;
            this.uuid = uuid;
            userAddress.setParams(houseNumber, road, location, city, state, zip, country);
        }

        /** Method to set user details.
         * @param reqType reqType
         * @param isPartialValidation isPartialValidation
         * @param mdn mdn
         * @param imei imei
         * @param uuid uuid
         * @param details address details
         * @return
         */
        public void setParams(String reqType, String isPartialValidation, String mdn,
                String imei, String uuid, Address details) {
            this.reqType = reqType;
            this.isPartialValidation = isPartialValidation;
            this.mdn = mdn;
            this.imei = imei;
            this.uuid = uuid;
            if (details == null) {
                this.userAddress = null;
            } else {
                this.userAddress.setParams(details);
            }
        }

        /** To convert user details in string.
                 * @return String
                 */
        public String toString() {
            return "reqType:" + reqType + ", isPartialValidation:" + isPartialValidation
                    + ", mdn:" + mdn + ", imei:" + imei
                    + ", uuid:" + uuid + (userAddress == null ? null : userAddress.toString());
        }
    }
}
