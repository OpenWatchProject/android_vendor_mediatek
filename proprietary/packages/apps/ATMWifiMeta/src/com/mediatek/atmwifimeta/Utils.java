package com.mediatek.atmwifimeta;

import java.lang.reflect.Method;

public class Utils {
	
	
    public static String pre_connect_type = "USB";
    public static String cur_connect_type = "USB";
	
	   public static final String ACTION_WM_CHNAGE =
	            "com.mediatek.atmwifimeta.SWITCH_CONNECT_MODEL";
	    
	    public static final String ACTION_WM_CHNAGE_RESULT =
	            "com.mediatek.atmwifimeta.SWITCH_CONNECT_MODEL_RESULT";

		   public static final String ACTION_WM_STARTSERVICE =
		            "com.mediatek.atmwifimeta.STARTSERVICE";
		   public static final String ACTION_WM_BOOTCOMPLETED =
		   "android.intent.action.BOOT_COMPLETED";
	    public static final String ATM_CMD_SWITCH_META_TO_USB = "ATM_SWITCH_META_TO_USB";
	    public static final String ATM_CMD_SWITCH_META_TO_WIFI = "ATM_SWITCH_META_TO_WIFI";
	    
	    public static final String ATM_CMD_EXTRA_TYPE = "TYPE";
	 
	    public static final String ATM_CMD_EXTRA_RESULT = "RESULT";

	    public static final String ATM_CMD_EXTRA_RESULT_SUCCEED = "TRUE";
	    public static final String ATM_CMD_EXTRA_RESULT_FAILED = "FAILED";
	    public static final String ATM_CMD_EXTRA_RESULT_EXIT = "EXIT";

	    

	    
	    //ATM properties
	    public static final String META_CONNECT_TYPE = "persist.vendor.meta.connecttype";
	    public static final String ATM_IP_ADDRESS = "persist.vendor.atm.ipaddress";
	    public static final String ATM_MD_MODE = "persist.vendor.atm.mdmode";
	    
	    //ATM socket and command
	    public static final String ATM_COMM_SOCKET = "com.mediatek.atm.communication";
	    public static final String ATM_CMD_DISCONNECT_WIFI = "ATM_DESTORY_WIFI_SOCKET";
	    public static final String ATM_CMD_CONNECT_WIFI = "ATM_NEW_WIFI_SOCKET";
	    public static final String ATM_CMD_SWITCH_MODEM_TO_META = "ATM_SWITCH_MODEM_TO_META";
	    public static final String ATM_CMD_SWITCH_MODEM_TO_NORMAL = "ATM_SWITCH_MODEM_TO_NORMAL";
	    public static final String ATM_CMD_EXIT_ACTIVITY = "ATM_EXIT_WIFI_APP";
	    public static String systemPropertyGet(String key, String def) {
	        try {
	            Class<?> sp = Class.forName("android.os.SystemProperties");
	            Method method = sp.getMethod("get", String.class, String.class);
	            return (String) method.invoke(null, key, def);
	        } catch (Exception e) {
	            //TODO more debug information
	            e.printStackTrace();
	            return def;
	        }
	    }
	 
	    public static void systemPropertySet(String key, String val){
	        try {
	            Class<?> sp = Class.forName("android.os.SystemProperties");
	            Method method = sp.getMethod("set", String.class, String.class);
	            method.invoke(null, key, val);
	        } catch (Exception e) {
	            //TODO more debug information
	            e.printStackTrace();
	        }
	 
	    }
		
}
