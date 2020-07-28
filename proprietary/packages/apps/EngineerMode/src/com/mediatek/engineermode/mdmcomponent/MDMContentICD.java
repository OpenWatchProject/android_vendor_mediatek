package com.mediatek.engineermode.mdmcomponent;

public class MDMContentICD {
    public static final String MSG_TYPE_ICD_RECORD = "type_icd_record";
    public static final String MSG_TYPE_ICD_EVENT = "type_icd_event";

    public static final String MSG_NAME_ERRC_SERVING_CELL_INFO = "ERRC_Serving_Cell_Info";
    public static final String MSG_ID_ERRC_SERVING_CELL_INFO = "7100";
    public static final String MSG_NAME_ERRC_MEAS_REPORT_INFO = "ERRC_MEAS_Report_Info";
    public static final String MSG_ID_ERRC_MEAS_REPORT_INFO = "7101";

    public static final String MSG_VALUE_ICD_MCC = "MCC";
    public static final String MSG_VALUE_ICD_MNC = "MNC";
    public static final String MSG_VALUE_SERV_EARFCN = "Serv Earfcn";
    public static final String MSG_VALUE_PHYSICAL_CELL_ID = "Physical Cell ID";

    public static class MDMHeaderICD {
        int type;
        int version;
        int total_size;
        int timeestamp_type;
        int protocol_id;
        int msg_id;
        int check_sum;
        int timeStamp;
    }

}
