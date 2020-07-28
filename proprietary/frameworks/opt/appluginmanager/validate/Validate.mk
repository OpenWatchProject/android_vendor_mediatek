LOCAL_XML_PATH := $(LOCAL_PATH)/res/raw/plugin.xml
LOCAL_JAR_PATH := $(call my-dir)/validate.jar
LOCAL_XSD_PATH := $(call my-dir)/../assets/plugin.xsd
LOCAL_SAVE_FILE_PATH := $(call my-dir)/.validate

$(info Begin schema validate for : $(LOCAL_XML_PATH))

ECHO_RESULT := $(shell java -jar $(LOCAL_JAR_PATH) \
                        $(LOCAL_XML_PATH) \
                        $(LOCAL_XSD_PATH) \
                        $(LOCAL_SAVE_FILE_PATH))
$(info $(ECHO_RESULT))

VALIDATE_RESULT := $(shell cat $(LOCAL_SAVE_FILE_PATH))
ifeq ($(VALIDATE_RESULT),true)
        $(info "Pass schema validate.")
else
        $(error Fail schema validate, Stop!)
endif
