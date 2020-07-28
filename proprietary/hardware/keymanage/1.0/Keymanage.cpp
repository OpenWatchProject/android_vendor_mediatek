#define LOG_TAG "KM_DRMKEY_HIDL_IMPL"

#include "Keymanage.h"


namespace vendor {
namespace mediatek {
namespace hardware {
namespace keymanage {
namespace V1_0 {
namespace implementation {

// Methods from IKeymanage follow.
Return<void> Keymanage::meta_drmkey_install_op(const HIDL_FT_DRMKEY_INSTALL_REQ& fromClient_req, const HIDL_FT_DRMKEY_INSTALL_CNF& fromClient_cnf, const hidl_vec<uint8_t>& data, uint16_t len, meta_drmkey_install_op_cb _hidl_cb) {
    ALOGI("%s start", __func__);
    FT_DRMKEY_INSTALL_REQ req;
    FT_DRMKEY_INSTALL_CNF drmkey_cnf;
    HIDL_FT_DRMKEY_INSTALL_CNF toClient_cnf;
    char *buf = NULL;

    convertREQ2nonHIDL(&fromClient_req, &req);
    convertCNF2nonHIDL(&fromClient_cnf, &drmkey_cnf);

    buf = (char *)malloc(len);
    convertVector2Array(data, buf);
    META_DRMKEY_INSTALL_OP(&req, &drmkey_cnf, buf, len);
    free(buf);
    convertCNF2HIDL(&drmkey_cnf, &toClient_cnf);

    ALOGI("%s end", __func__);
    _hidl_cb(toClient_cnf);
    return Void();
}

Return<void> Keymanage::get_encrypt_drmkey_hidl(uint32_t keyID, get_encrypt_drmkey_hidl_cb _hidl_cb) {
    ALOGI("%s start", __func__);
    unsigned char *keyblock = NULL;
    std::vector<uint8_t> oneDrmkeyBlock;
    uint32_t blockLeng = 0;
    int32_t ret = -1;

    ret = get_encrypt_drmkey(keyID, &keyblock, &blockLeng);
    convertArray2Vector(keyblock, blockLeng, oneDrmkeyBlock);

    _hidl_cb(oneDrmkeyBlock, blockLeng, ret);
    free_encrypt_drmkey(keyblock);
    ALOGI("%s end", __func__);
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IKeymanage* HIDL_FETCH_IKeymanage(const char* /* name */) {
    return new Keymanage();
}

// API for HDMI ATA tool
int get_encrypt_drmkey_hidl_c(uint32_t keyID, unsigned int *inlength, unsigned char **enckbdrm) {
    int ret_temp = 0;

    android::sp<IKeymanage> hdmi_hdcp_client = IKeymanage::getService();

    auto hdmi_hdcp_callback = [&] (const android::hardware::hidl_vec<uint8_t>& oneDrmkeyBlock, uint32_t blockLeng, int32_t ret_val)
    {
        ret_temp = ret_val;

        ALOGI("[KM_HIDL]  blockLeng = %u\n", blockLeng);
        *enckbdrm = (unsigned char *)malloc(blockLeng);
        if (*enckbdrm == NULL)
        {
            ALOGE("[MtkHdmiService][KM_HIDL] malloc failed----\n");
        }

        convertVector2Array_U(oneDrmkeyBlock, *enckbdrm);
        *inlength = blockLeng;
    };

    hdmi_hdcp_client->get_encrypt_drmkey_hidl(keyID,  hdmi_hdcp_callback);

    return ret_temp;
}
}  // namespace implementation
}  // namespace V1_0
}  // namespace keymanage
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
