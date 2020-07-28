#define LOG_TAG "KMSetkey"

#include <log/log.h>
#include <kmsetkey.h>

#include <vendor/mediatek/hardware/keymaster_attestation/1.1/IKeymasterDevice.h>
using ::vendor::mediatek::hardware::keymaster_attestation::V1_1::IKeymasterDevice;
using ::vendor::mediatek::hardware::keymaster_attestation::V1_1::StatusCode;

using android::hardware::Return;

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t import_kb_offset = 0, import_kb_len = 0;
static uint8_t *import_kb_buf = NULL;

void import_clean(void)
{
	free(import_kb_buf);
	import_kb_len = 0;
	import_kb_offset = 0;
	import_kb_buf = NULL;
}

int32_t ree_import_attest_keybox(const uint8_t *peakb, const uint32_t peakb_len, const uint32_t finish)
{
	uint32_t payload_offset, payload_len;
	uint8_t *payload_buf;

	if ((finish == 0 && (peakb == NULL || peakb_len == 0)) || (finish != 0 && ((peakb == NULL && peakb_len != 0) || (peakb != NULL && peakb_len == 0)))) {
		ALOGE("attest keybox is NULL or keybox size is 0\n");
		import_clean();
		return -1;
	}

	payload_offset = import_kb_offset + peakb_len;
	if (payload_offset > import_kb_len) {
		payload_len = (finish == 1 ? payload_offset : 2 * payload_offset);
		payload_buf = (uint8_t *)realloc(import_kb_buf, payload_len);
		if (payload_buf == NULL) {
			ALOGE("keybox buffer is NULL\n");
			if (finish == 1)
				import_clean();
			return -2;
		}
		import_kb_buf = payload_buf;
		import_kb_len = payload_len;
	}

	if (peakb != NULL) {
		memcpy(import_kb_buf + import_kb_offset, peakb, peakb_len);
		import_kb_offset += peakb_len;
	}

	if (finish == 0)
		return 0;

	ALOGI("%s: get HIDL service!\n", __func__);
	android::sp<IKeymasterDevice> mDevice = IKeymasterDevice::getService();
	android::hardware::hidl_vec<uint8_t> _hidl_data;
	_hidl_data.setToExternal(const_cast<unsigned char*>(import_kb_buf), import_kb_offset);

	if (mDevice == nullptr) {
		ALOGE("%s: unable to getService.\n", __func__);
		import_clean();
		return -3;
	}

	ALOGI("%s: start to enter attestKeyInstall!\n", __func__);
	Return<vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode> mDevrep = mDevice->attestKeyInstall(_hidl_data);
	ALOGI("%s: exit attestKeyInstall!\n", __func__);

	if (mDevrep == vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode::UNIMPLEMENTED) {
		ALOGE("%s: attest key install failed, HAL not exists!\n", __func__);
		import_clean();
		return -4;
	}
	else if (mDevrep == vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode::KEYINSTALLFAILED) {
		ALOGE("%s: attest key install failed!\n", __func__);
		import_clean();
		return -5;
	}
	else if (mDevrep == vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode::OK) {
		ALOGI("%s: attest key install success!\n", __func__);
		import_clean();
		return 0;
	}

	ALOGE("%s: attest key install failed, unknown error!\n", __func__);
	import_clean();
	return -1000;
}


static uint32_t check_kb_offset = 0, check_kb_len = 0;
static uint8_t *check_kb_buf = NULL;

void check_clean(void)
{
	free(check_kb_buf);
	check_kb_len = 0;
	check_kb_offset = 0;
	check_kb_buf = NULL;
}

int32_t ree_check_attest_keybox(const uint8_t *peakb, const uint32_t peakb_len, const uint32_t finish)
{
	uint32_t payload_offset, payload_len;
	uint8_t *payload_buf;

	if (peakb == NULL || peakb_len == 0) {
		ALOGE("attest keybox is NULL\n");
		if (finish == 1)
			check_clean();
		return -1;
	}

	payload_offset = check_kb_offset + peakb_len;
	if (payload_offset > check_kb_len) {
		payload_len = (finish == 1 ? payload_offset : 2 * payload_offset);
		payload_buf = (uint8_t *)realloc(check_kb_buf, payload_len);
		if (payload_buf == NULL) {
			ALOGE("keybox buffer is NULL\n");
                        if (finish == 1)
                                check_clean();
                        return -2;
		}
		check_kb_buf = payload_buf;
		check_kb_len = payload_len;
	}
	memcpy(check_kb_buf + check_kb_offset, peakb, peakb_len);
	check_kb_offset += peakb_len;

	if (finish == 0)
		return 0;

	ALOGI("%s: get HIDL service!\n", __func__);
	android::sp<IKeymasterDevice> mDevice = IKeymasterDevice::getService();
	android::hardware::hidl_vec<uint8_t> _hidl_data;
	_hidl_data.setToExternal(const_cast<unsigned char*>(check_kb_buf), check_kb_offset);

	if (mDevice == nullptr) {
		ALOGE("%s: unable to getService.\n", __func__);
		check_clean();
		return -3;
	}

	ALOGI("%s: start to enter attestKeyCheck!\n", __func__);
	Return<StatusCode> mDevrep = mDevice->attestKeyCheck(_hidl_data);
	ALOGI("%s: exit attestKeyCheck!\n", __func__);

	if (mDevrep == StatusCode::UNIMPLEMENTED) {
		ALOGE("%s: attest key check failed, HAL not exists!\n", __func__);
		check_clean();
		return -4;
	}
	else if (mDevrep == StatusCode::KEYCHECKFAILED) {
		ALOGE("%s: attest key check failed!\n", __func__);
		check_clean();
		return -5;
	}
	else if (mDevrep == StatusCode::OK) {
		ALOGI("%s: attest key check success!\n", __func__);
		check_clean();
		return 0;
	}

	ALOGE("%s: attest key check failed, unknown error!\n", __func__);
	check_clean();
	return -1000;
}

#ifdef __cplusplus
}
#endif
