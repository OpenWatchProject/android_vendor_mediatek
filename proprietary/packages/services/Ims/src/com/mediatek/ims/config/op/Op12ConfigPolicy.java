package com.mediatek.ims.config.op;

import com.android.ims.ImsConfig;
import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.config.ImsConfigPolicy;
import com.mediatek.ims.MtkImsConstants;

public class Op12ConfigPolicy extends ImsConfigPolicy {
    public Op12ConfigPolicy() {
        super("Op12ConfigPolicy");
    }

    public boolean onSetDefaultValue(int configId, ImsConfigPolicy.DefaultConfig config) {
        boolean set = true;
        switch (configId) {
            // ./ManagedObjects/DCMO/VOLTE/VocoderMode/AMRMODESET @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.VOCODER_AMRMODESET:
                config.defVal = "7";
                break;
            // ./ManagedObjects/DCMO/VOLTE/VocoderMode/AMRWBMODESET @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.VOCODER_AMRWBMODESET:
                config.defVal = "2";
                break;
            // ./ManagedObjects/DCMO/VOLTE/SIP_Session_Timer @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.SIP_SESSION_TIMER:
                config.defVal = "300";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Min_SE @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.MIN_SE:
                config.defVal = "300";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Timer_VZW @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.CANCELLATION_TIMER:
                config.defVal = "6";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/TDelay @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.TDELAY:
                config.defVal = "5";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/SILENT_REDIAL_ENABLE @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.SILENT_REDIAL_ENABLE:
                config.defVal = "1";
                break;
            // ./ManagedObjects/ConnMO/IMS/Setting/SIPT1 @LTE_OTADM
            case ImsConfig.ConfigConstants.SIP_T1_TIMER:
                config.defVal = "3";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/ConnMO/IMS/Setting/SIPT2 @LTE_OTADM
            case ImsConfig.ConfigConstants.SIP_T2_TIMER:
                config.defVal = "16";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/ConnMO/IMS/Setting/SIPTf @LTE_OTADM
            case ImsConfig.ConfigConstants.SIP_TF_TIMER:
                config.defVal = "30";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/FeatureStatus/VLT/Setting/Enabled @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.VLT_SETTING_ENABLED:
                config.defVal = "0";
                break;
            // ./ManagedObjects/DCMO/VOLTE/FeatureStatus/LVC/Setting/Enabled @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.LVC_SETTING_ENABLED:
                config.defVal = "1";
                break;
            // ./ManagedObjects/ConnMO/IMS/Setting/Domain @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.DOMAIN_NAME:
                config.defVal = "vzims.com";
                break;
            // ./ManagedObjects/ConnMO/IMS/Setting/smsformat @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.SMS_FORMAT:
                config.defVal = "1";
                break;
            // ./ManagedObjects/ConnMO/IMS/Setting/sms_over_IP_network_indication @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.SMS_OVER_IP:
                config.defVal = "1";
                break;
            // ./ManagedObjects/DCMO/VOLTE/Publish_Timer @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.PUBLISH_TIMER:
                config.defVal = "1200";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Publish_Timer_Extended @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.PUBLISH_TIMER_EXTENDED:
                config.defVal = "86400";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Capabilities_Cache_Expiration @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.CAPABILITIES_CACHE_EXPIRATION:
                config.defVal = "7776000";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Availability_Cache_Expiration @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.AVAILABILITY_CACHE_EXPIRATION:
                config.defVal = "60";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Capabilities_Poll_Interval @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.CAPABILITIES_POLL_INTERVAL:
                config.defVal = "625000";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Source_Throttle_Publish @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.SOURCE_THROTTLE_PUBLISH:
                config.defVal = "60";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/Max_NumEntries_in_RCL @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.MAX_NUMENTRIES_IN_RCL:
                config.defVal = "100";
                break;
            // ./ManagedObjects/DCMO/VOLTE/Capab_Poll_list_Sub_Exp @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP:
                config.defVal = "30";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            // ./ManagedObjects/DCMO/VOLTE/GZIP_Flag @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.GZIP_FLAG:
                config.defVal = "1";
                break;
            // ./ManagedObjects/DCMO/VOLTE/FeatureStatus/EAB/Setting/Enabled @MultimodeDevice_OTADM
            case ImsConfig.ConfigConstants.EAB_SETTING_ENABLED:
                config.defVal = "0";
                break;
            case ImsConfig.ConfigConstants.VOICE_OVER_WIFI_ROAMING:
                config.defVal = "0";
                break;
            case ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE:
                config.defVal = String.valueOf(
                        ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
                break;
            case ImsConfig.ConfigConstants.CAPABILITY_DISCOVERY_ENABLED:
                config.defVal = "1";
                break;
            case MtkImsConstants.ConfigConstants.PUBLISH_ERROR_RETRY_TIMER:
                config.defVal = "21600";
                config.unitId = ImsConfigContract.Unit.SECONDS;
                break;
            case MtkImsConstants.ConfigConstants.VOICE_OVER_WIFI_MDN:
                config.defVal = "";
                break;
            default:
                set = false;
                break;
        }
        return set;
    }
}
