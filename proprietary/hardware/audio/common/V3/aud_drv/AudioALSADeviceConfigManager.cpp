
#include <tinyalsa/asoundlib.h>
#include "AudioALSADeviceConfigManager.h"
#include "AudioALSADriverUtility.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSADeviceConfigManager"

#define AUDIO_DEVICE_EXT_CONFIG_FILE         "/vendor/etc/audio_device.xml"

namespace android {

DeviceCtlDescriptor::DeviceCtlDescriptor() {
    DeviceStatusCounter = 0;
}


AudioALSADeviceConfigManager *AudioALSADeviceConfigManager::UniqueAlsaDeviceConfigParserInstance = NULL;

AudioALSADeviceConfigManager *AudioALSADeviceConfigManager::getInstance() {
    if (UniqueAlsaDeviceConfigParserInstance == 0) {
        UniqueAlsaDeviceConfigParserInstance = new AudioALSADeviceConfigManager();
    }
    return UniqueAlsaDeviceConfigParserInstance;
}

AudioALSADeviceConfigManager::AudioALSADeviceConfigManager():
    mConfigsupport(false),
    mInit(false),
    mMixer(NULL)
{
    ALOGV("%s()", __FUNCTION__);

#ifdef CONFIG_MT_ENG_BUILD
    mLogEnable = 1;
#else
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);
#endif

    int ret = LoadAudioConfig(AUDIO_DEVICE_EXT_CONFIG_FILE);
    if (ret != NO_ERROR) {
        mConfigsupport = false;
    } else {
        mConfigsupport = true;
    }

    if (mMixer == NULL) {
        mMixer = AudioALSADriverUtility::getInstance()->getMixer();
        ASSERT(mMixer != NULL);
    }

    mInit = true;
}

status_t AudioALSADeviceConfigManager::GetVersion(TiXmlElement *root) {
    const char *VersionNumber  = root->Attribute("value");  // get action
    VersionControl = String8(VersionNumber);
    ALOGV("GetVersion = %s", VersionNumber);
    return NO_ERROR;
}

bool AudioALSADeviceConfigManager::CheckDeviceExist(const char *devicename) {
    for (size_t i = 0; i < mDeviceVector.size(); i++) {
        DeviceCtlDescriptor *DeviceScriptor  = mDeviceVector.itemAt(i);
        if (strcmp(devicename, DeviceScriptor->mDevicename.string()) == 0) {
            ALOGV("%s() exist devicename = %s", __FUNCTION__, devicename);
            return true;
        }
    }
    ALOGV("%s() not exist devicename = %s", __FUNCTION__, devicename);
    return false;
}

String8 AudioALSADeviceConfigManager::CheckCustDevPathExist(const char *devicename) {
    String8 nameSeq, valSeq;

    for (size_t i = 0; i < mDevCustPathSeq.mDeviceCltNameVector.size() ; i++) {
        String8 nameSeq = mDevCustPathSeq.mDeviceCltNameVector.itemAt(i);
        ALOGV("%s(), nameSeq = %s", __FUNCTION__, nameSeq.string());
        if (strncmp(nameSeq, devicename, strlen(nameSeq.string())) == 0) {
            valSeq = mDevCustPathSeq.mDeviceCltValueVector.itemAt(i);
            return valSeq;
        }
    }

    return String8();
}

DeviceCtlDescriptor *AudioALSADeviceConfigManager::GetDeviceDescriptorbyname(const char *devicename) {
    ALOGV("%s", __FUNCTION__);
    for (size_t i = 0; i < mDeviceVector.size(); i++) {
        DeviceCtlDescriptor *DeviceScriptor  = mDeviceVector.itemAt(i);
        if (strcmp(devicename, DeviceScriptor->mDevicename.string()) == 0) {
            ALOGV("%s() exist devicename = %s", __FUNCTION__, devicename);
            return mDeviceVector.itemAt(i);
        }
    }
    return NULL;
}

String8 AudioALSADeviceConfigManager::ParseDeviceSequence(TiXmlElement *root) {
    DeviceCtlDescriptor *TempDeviceDescriptor  = NULL;
    String8 sequence = String8();
    if (root != NULL) {
        const char *devicename  = root->Attribute("name");  // get path name
        const char *action  = root->Attribute("value");  // get action
        ALOGV("%s() devicename = %s action = %s", __FUNCTION__, devicename, action);
        sequence = String8(devicename);
        if (CheckDeviceExist(devicename) == false) { // this device has not exist , new object
            TempDeviceDescriptor = new DeviceCtlDescriptor();
            TempDeviceDescriptor->mDevicename = String8(devicename);
            mDeviceVector.add(TempDeviceDescriptor);
        } else {
            TempDeviceDescriptor = GetDeviceDescriptorbyname(devicename);  // get instance in vector
        }

        if (TempDeviceDescriptor == NULL) {
            ALOGE("%s() is NULL pointer , return", __FUNCTION__);
            return sequence;
        }

        if (strcmp(action, AUDIO_DEVICE_TURNON) == 0) {
            ALOGV("add turn on sequnce");
            TiXmlElement *child = root->FirstChildElement("kctl");
            while (child != NULL) {
                const char *valname  = child->Attribute("name");
                const char *valvalue  = child->Attribute("value");
                ALOGV("valname = %s  valvalue = %s  ", valname, valvalue);
                TempDeviceDescriptor->mDeviceCltonVector.add(String8(valname));
                TempDeviceDescriptor->mDeviceCltonVector.add(String8(valvalue));
                child = child->NextSiblingElement("kctl");
            }
        } else if (strcmp(action, AUDIO_DEVICE_TURNOFF) == 0) {
            ALOGV("add turn off sequnce");
            TiXmlElement *child = root->FirstChildElement("kctl");
            while (child != NULL) {
                const char *valname  = child->Attribute("name");
                const char *valvalue  = child->Attribute("value");
                ALOGV("valname = %s  valvalue = %s  ", valname, valvalue);
                TempDeviceDescriptor->mDeviceCltoffVector.add(String8(valname));
                TempDeviceDescriptor->mDeviceCltoffVector.add(String8(valvalue));
                child = child->NextSiblingElement("kctl");
            }
        } else if (strcmp(action, AUDIO_DEVICE_SETTING) == 0) {
            ALOGV("add AUDIO_DEVICE_SETTING");
            TiXmlElement *child = root->FirstChildElement("kctl");
            while (child != NULL) {
                const char *valname  = child->Attribute("name");
                const char *valvalue  = child->Attribute("value");
                ALOGV("valname = %s  valvalue = %s  ", valname, valvalue);
                TempDeviceDescriptor->mDeviceCltsettingVector.add(String8(valname));
                TempDeviceDescriptor->mDeviceCltsettingVector.add(String8(valvalue));
                child = child->NextSiblingElement("kctl");
            }
        } else {
            ALOGD("device sequnce either not turn on and turn off");
        }
    }
    return sequence;
}

status_t AudioALSADeviceConfigManager::ParseInitSequence(TiXmlElement *root) {
    ALOGD("%s()", __FUNCTION__);
    TiXmlElement *child = root->FirstChildElement("kctl");
    while (child != NULL) {
        const char *valname  = child->Attribute("name");
        const char *valvalue  = child->Attribute("value");
        ALOGD("valname = %s  valvalue = %s  ", valname, valvalue);
        mDeviceCtlSeq.mDeviceCltNameVector.add(String8(valname));
        mDeviceCtlSeq.mDeviceCltValueVector.add(String8(valvalue));
        child = child->NextSiblingElement("kctl");
    }
    return NO_ERROR;
}

bool  AudioALSADeviceConfigManager::SupportConfigFile() {
    return mConfigsupport;
}

int AudioALSADeviceConfigManager::parseCustDevicePathSeq(TiXmlElement *root) {
    TiXmlElement *outputDev;
    const char *devStr = NULL;
    const char *pathStr = NULL;
    const char *childElementName = "outputdevice";
    const char *attrDevName = "name";
    const char *attrPathName = "path";

    if (root == NULL) {
        return INVALID_OPERATION;
    }

    outputDev = root->FirstChildElement(childElementName);
    if (outputDev == NULL) {
        ALOGV("%s(), outputDev is NULL", __FUNCTION__);
        return NAME_NOT_FOUND;
    }

    while(outputDev) {
        devStr = outputDev->Attribute(attrDevName);
        pathStr = outputDev->Attribute(attrPathName);

        mDevCustPathSeq.mDeviceCltNameVector.add(String8(devStr));
        mDevCustPathSeq.mDeviceCltValueVector.add(String8(pathStr));

        outputDev = outputDev->NextSiblingElement(childElementName);
    }

    return NO_ERROR;
}

status_t AudioALSADeviceConfigManager::LoadAudioConfig(const char *path) {
    if (mInit == true) {
        return ALREADY_EXISTS;
    }

    ALOGV("%s()", __FUNCTION__);
    String8 sequence;
    TiXmlDocument doc(path);
    bool loadOkay = doc.LoadFile();
    if (loadOkay) {
        // document is loaded
        ALOGV("LoadAudioConfig success ");

        // here handle for xml version and other inforamtion
        TiXmlDeclaration *declaration = doc.FirstChild()->ToDeclaration();
        ALOGD("TiXmlDeclaration version = %s, Encoding = %s, Standalone = %s",
              declaration->Version(), declaration->Encoding(), declaration->Standalone());

        TiXmlElement *root = doc.FirstChildElement("root");

        if (root == NULL) { // no root element in this xml
            root = doc.FirstChildElement("mixercontrol");
        } else {
            parseCustDevicePathSeq(root);
            root = root->FirstChildElement("mixercontrol");
        }

        if (root != NULL) {
            TiXmlElement *Version = root->FirstChildElement("versioncontrol"); // find with version contol
            if (Version != NULL) {
                GetVersion(Version);
            }
        }

        if (root) {
            ALOGV("FirstChildElement can find mixer");
            ParseInitSequence(root);
            TiXmlElement *child = root->FirstChildElement("path");
            while (child) {
                sequence = ParseDeviceSequence(child);

                child = child->NextSiblingElement("path");
            }
        }
    } else {
        // load failed
        ALOGE("LoadAudioConfig fail ");
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

int AudioALSADeviceConfigManager::setMixerByteCtl(struct mixer_ctl *ctl, char **values, unsigned int numValues) {
    int ret = 0;
    char *buf;
    char *end;
    unsigned int i;
    long n;

    buf = (char *)malloc(numValues);

    if (buf == NULL) {
        ALOGD("%s(), Failed to alloc mem for bytes %d\n", __FUNCTION__, numValues);
        return -ENOMEM;
    }

    for (i = 0; i < numValues; i++) {
        errno = 0;
        n = strtol(values[i], &end, 0);
        if (*end) {
            ALOGW("%s not an integer\n", values[i]);
            ret = -EINVAL;
            break;
        }
        if (errno) {
            ALOGW("strtol: %s: %s\n", values[i], strerror(errno));
            ret = -EINVAL;
            break;
        }
        if (n < 0 || n > 0xff) {
            ALOGW("%s should be between [0, 0xff]\n", values[i]);
            ret = -EINVAL;
            break;
        }
        buf[i] = n;
    }

    if (ret == 0) {
        ret = mixer_ctl_set_array(ctl, buf, numValues);
        if (ret < 0) {
            ALOGW("Failed to set binary control\n");
        }
    }

    free(buf);
    return ret;
}

int AudioALSADeviceConfigManager::setMixerCtlValue(String8 cltName, String8 cltValue) {
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    struct mixer *mixer = mMixer;

    unsigned int num_ctl_values;
    unsigned int num_values = 0;
    unsigned int str_len = cltValue.size();

    unsigned int i, j;

    int ret = 0;

    const char *charCtlValue = cltValue.string();
    const char *charCltName = cltName.string();

    ALOGV("%s(), charCltName = %s, charCtlValue = %s\n", __FUNCTION__, charCltName, charCtlValue);

    if (isdigit(charCltName[0])) {
        ctl = mixer_get_ctl(mixer, atoi(charCltName));
    } else {
        ctl = mixer_get_ctl_by_name(mixer, charCltName);
    }

    if (!ctl) {
        ALOGD("Invalid mixer control\n");
        return -EINVAL;
    }

    type = mixer_ctl_get_type(ctl);
    num_ctl_values = mixer_ctl_get_num_values(ctl);

    char values[num_ctl_values][str_len + 1];
    for (i = 0; i < num_ctl_values; i++) {
        memset(values[i], 0, str_len + 1);
    }

    /* To get num of values*/
    if (num_ctl_values > 1) {
        for (i = 0; i <= str_len && num_values < num_ctl_values; i++) {
            bool find_value = false;
            if (*(charCtlValue) == 0) {
                break;
            }

            for (j = 0; j < str_len; j++) {
                if ((*(charCtlValue) >= '0') && (*(charCtlValue) <= '9')) {
                    values[num_values][j] = *charCtlValue++;
                    if (find_value == false) {
                        find_value = true;
                    }
                } else {
                    if (*(charCtlValue) != 0) {
                      charCtlValue++;
                    }
                    break;
                }
            }

            if (find_value) {
                num_values++;
            }
        }

        ALOGD("num_values is %u, And the value is as bellow\n", num_values);
        for (i = 0; i < num_values; i++) {
            ALOGD("String values[%d]:%s\n", i, values[i]);
        }
    } else {
        num_values = 1;
        memcpy(values[0], cltValue.string(), cltValue.size());
    }

    if (type == MIXER_CTL_TYPE_BYTE) {
        return setMixerByteCtl(ctl, (char **)values, num_values);
    }

    if (isdigit(values[0][0])) {
        if (num_values == 1) {
            /* Set all values the same */
            int value = atoi(values[0]);

            for (i = 0; i < num_ctl_values; i++) {
                ret = mixer_ctl_set_value(ctl, i, value);
                if (ret) {
                    ALOGD("Error: invalid value\n");
                    return ret;
                }
            }
        } else {
            /* Set multiple values */
            if (num_values > num_ctl_values) {
                ALOGD("Error: %u values given, but control only takes %d\n",
                        num_values, num_ctl_values);
                return -EINVAL;
            }
            for (i = 0; i < num_values; i++) {
                ret = mixer_ctl_set_value(ctl, i, atoi(values[i]));
                if (ret) {
                    ALOGD("Error: invalid value for index %d\n", i);
                    return ret;
                }
            }
        }
    } else {
        ALOGD("%s(), Enum_values Num %u\n", __FUNCTION__, num_values);
        if (type == MIXER_CTL_TYPE_ENUM) {
            if (num_values != 1) {
                ALOGD("Enclose strings in quotes and try again\n");
                return -EINVAL;
            }
            ret = mixer_ctl_set_enum_by_string(ctl, cltValue.string());
            if (ret)
                ALOGD("Error: invalid enum value\n");
        } else {
            ALOGD("Error: only enum types can be set with strings\n");
        }
    }

    return ret;
}

int AudioALSADeviceConfigManager::setMixerCtl(String8 cltName, String8 cltValue) {
    int ret = 0;
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, cltName.string());
    enum mixer_ctl_type type = mixer_ctl_get_type(ctl);
    ALOGV("%s(), cltName = %s, cltValue = %s, cltType = %d", __FUNCTION__, cltName.string(), cltValue.string(), type);

    switch (type) {
    case MIXER_CTL_TYPE_ENUM:
        ret = mixer_ctl_set_enum_by_string(ctl, cltValue.string());
        break;
    case MIXER_CTL_TYPE_BOOL:
    case MIXER_CTL_TYPE_INT:
    case MIXER_CTL_TYPE_BYTE:
        ret = setMixerCtlValue(cltName, cltValue);
        break;
    default:
        ALOGE("%s(), Error: Unsupport mixer ctl type %d, cltName = %s, cltValue = %s",
              __FUNCTION__, type, cltName.string(), cltValue.string());
        ASSERT(0);
        ret = -EINVAL;
        break;
    }

    return ret;
}

status_t AudioALSADeviceConfigManager::ApplyDeviceTurnonSequenceByName(const char *DeviceName) {
    DeviceCtlDescriptor *descriptor = GetDeviceDescriptorbyname(DeviceName);
    if (descriptor == NULL) {
        ALOGE("%s  DeviceName = %s descriptor == NULL", __FUNCTION__, DeviceName);
        return INVALID_OPERATION;
    }
    ALOGD("%s() DeviceName = %s descriptor->DeviceStatusCounte = %d", __FUNCTION__, DeviceName, descriptor->DeviceStatusCounter);
    if (descriptor->DeviceStatusCounter == 0) {
        for (size_t count = 0; count < descriptor->mDeviceCltonVector.size(); count += 2) {
            String8 cltname = descriptor->mDeviceCltonVector.itemAt(count);
            String8 cltvalue = descriptor->mDeviceCltonVector.itemAt(count + 1);
            ALOGV("cltname = %s cltvalue = %s", cltname.string(), cltvalue.string());
            if (setMixerCtl(cltname, cltvalue)) {
                ALOGE("Error: %s  cltname.string () = %s cltvalue.string () = %s", __FUNCTION__, cltname.string(), cltvalue.string());
                ASSERT(false);
            }
        }
    }
    descriptor->DeviceStatusCounter++;
    return NO_ERROR;
}

status_t AudioALSADeviceConfigManager::ApplyDeviceTurnoffSequenceByName(const char *DeviceName) {
    DeviceCtlDescriptor *descriptor = GetDeviceDescriptorbyname(DeviceName);
    if (descriptor == NULL) {
        ALOGE("%s(), DeviceName = %s descriptor == NULL", __FUNCTION__, DeviceName);
        return INVALID_OPERATION;
    }
    ALOGD("%s(), DeviceName = %s descriptor->DeviceStatusCounter = %d", __FUNCTION__, DeviceName, descriptor->DeviceStatusCounter);

    descriptor->DeviceStatusCounter--;
    if (descriptor->DeviceStatusCounter == 0) {
        for (size_t count = 0; count < descriptor->mDeviceCltoffVector.size(); count += 2) {
            String8 cltname = descriptor->mDeviceCltoffVector.itemAt(count);
            String8 cltvalue = descriptor->mDeviceCltoffVector.itemAt(count + 1);
            ALOGV("%s(), cltname = %s cltvalue = %s", __FUNCTION__, cltname.string(), cltvalue.string());
            if (setMixerCtl(cltname, cltvalue)) {
                ALOGE("%s(), Error: ApplyDeviceTurnoffSequenceByName devicename = %s", __FUNCTION__, descriptor->mDevicename.string());
                ASSERT(false);
            }
        }
    } else if (descriptor->DeviceStatusCounter < 0) {
        ALOGW("%s(), DeviceName = %s DeviceStatusCounter < 0", __FUNCTION__, DeviceName);
        descriptor->DeviceStatusCounter = 0;
    }
    return NO_ERROR;
}


status_t AudioALSADeviceConfigManager::ApplyDeviceSettingByName(const char *DeviceName) {
    DeviceCtlDescriptor *descriptor = GetDeviceDescriptorbyname(DeviceName);
    if (descriptor == NULL) {
        ALOGE("%s  DeviceName = %s descriptor == NULL", __FUNCTION__, DeviceName);
        return INVALID_OPERATION;
    }
    ALOGD_IF(mLogEnable, "%s() DeviceName = %s descriptor->DeviceStatusCounter = %d", __FUNCTION__, DeviceName, descriptor->DeviceStatusCounter);

    for (size_t count = 0; count < descriptor->mDeviceCltsettingVector.size(); count += 2) {
        String8 cltname = descriptor->mDeviceCltsettingVector.itemAt(count);
        String8 cltvalue = descriptor->mDeviceCltsettingVector.itemAt(count + 1);
        ALOGD_IF(mLogEnable, "cltname = %s cltvalue = %s", cltname.string(), cltvalue.string());
        if (setMixerCtl(cltname, cltvalue)) {
            ALOGE("Error: %s() devicename = %s", __FUNCTION__, descriptor->mDevicename.string());
            ASSERT(false);
        }
    }
    return NO_ERROR;
}

void AudioALSADeviceConfigManager::dump() {
    DeviceCtlDescriptor *descriptor = NULL;

    ALOGD("AudioALSADeviceConfigManager dump init sequence");
    for (size_t count = 0; count < mDeviceCtlSeq.mDeviceCltNameVector.size() ; count++) {
        String8 temp = mDeviceCtlSeq.mDeviceCltNameVector.itemAt(count);
        String8 temp1 = mDeviceCtlSeq.mDeviceCltValueVector.itemAt(count);
        ALOGD("init sequnce kclt = %s value = %s", temp.string(), temp1.string());
    }

    for (size_t count = 0; count < mDeviceVector.size() ; count++) {
        descriptor = mDeviceVector.itemAt(count);
        ALOGD("mDescritor->name = %s", descriptor->mDevicename.string());
        for (size_t sequence = 0; sequence < descriptor->mDeviceCltonVector.size() ; sequence += 2) {
            String8 temp = descriptor->mDeviceCltonVector.itemAt(sequence);
            String8 temp1 = descriptor->mDeviceCltonVector.itemAt(sequence + 1);
            ALOGD("turn on name = %s value = %s ", temp.string(), temp1.string());
        }
        for (size_t sequence = 0; sequence < descriptor->mDeviceCltoffVector.size() ; sequence += 2) {
            String8 temp = descriptor->mDeviceCltoffVector.itemAt(sequence);
            String8 temp1 = descriptor->mDeviceCltoffVector.itemAt(sequence + 1);
            ALOGD("turn off name = %s value = %s ", temp.string(), temp1.string());
        }
        for (size_t sequence  = 0; sequence < descriptor->mDeviceCltsettingVector.size() ; sequence += 2) {
            String8 temp = descriptor->mDeviceCltsettingVector.itemAt(sequence);
            String8 temp1 = descriptor->mDeviceCltsettingVector.itemAt(sequence + 1);
            ALOGD("mDeviceCltsettingVector  name = %s value = %s ", temp.string(), temp1.string());
        }
    }
}

}
