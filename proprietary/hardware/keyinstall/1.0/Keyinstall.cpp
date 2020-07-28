#define LOG_TAG "DRMKEY_HIDL_IMPL"

#include "Keyinstall.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace keyinstall {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::keyinstall::V1_0::IKeyinstall follow.
Return<void> Keyinstall::meta_drmkey_install_op(const HIDL_FT_DRMKEY_INSTALL_REQ& fromClient_req, const HIDL_FT_DRMKEY_INSTALL_CNF& fromClient_cnf, const hidl_vec<uint8_t>& data, uint16_t len, meta_drmkey_install_op_cb _hidl_cb) {
	ALOGI("hello %s", __func__);
	FT_DRMKEY_INSTALL_REQ req;
	FT_DRMKEY_INSTALL_CNF drmkey_cnf;
	HIDL_FT_DRMKEY_INSTALL_CNF toClient_cnf;
	char *buf = NULL;

	convertREQ2nonHIDL(&fromClient_req, &req);

#ifdef DEBUG
	ALOGI("from client req:\n");
	ALOGI("req.header.token: %d\n", req.header.token);
	ALOGI("req.header.id: %d\n", req.header.id);
	ALOGI("req.op: %d\n", req.op);
	ALOGI("req.cmd.set_req.file_size: %d\n", req.cmd.set_req.file_size);
	ALOGI("req.cmd.set_req.stage: %d\n", req.cmd.set_req.stage);
	ALOGI("req.cmd.query_req.req: %d\n", req.cmd.query_req.req);
	ALOGI("==============================\n");
#endif

	convertCNF2nonHIDL(&fromClient_cnf, &drmkey_cnf);

#ifdef DEBUG
	ALOGI("from client cnf:\n");
	ALOGI("drmkey_cnf.header.token: %d\n", drmkey_cnf.header.token);
	ALOGI("drmkey_cnf.header.id: %d\n", drmkey_cnf.header.id);
	ALOGI("drmkey_cnf.op: %d\n", drmkey_cnf.op);
	ALOGI("drmkey_cnf.status: %d\n", drmkey_cnf.status);
	ALOGI("drmkey_cnf.result.set_cnf.result: %d\n", drmkey_cnf.result.set_cnf.result);
	ALOGI("drmkey_cnf.result.keyresult.keycount: %d\n", drmkey_cnf.result.keyresult.keycount);
#endif

	buf = (char *)malloc(len);
	convertVector2Array(data, buf);
	META_DRMKEY_INSTALL_OP(&req, &drmkey_cnf, buf, len);
	free(buf);

	convertCNF2HIDL(&drmkey_cnf, &toClient_cnf);

#ifdef DEBUG
	ALOGI("toClient_cnf.header.token: %d\n", toClient_cnf.header.token);
	ALOGI("toClient_cnf.header.id: %d\n", toClient_cnf.header.id);
	ALOGI("toClient_cnf.op: %d\n", toClient_cnf.op);
	ALOGI("toClient_cnf.status: %d\n", toClient_cnf.status);
	ALOGI("toClient_cnf.result.set_cnf.result: %d\n", toClient_cnf.result.set_cnf.result);
	ALOGI("toClient_cnf.result.keyresult.keycount: %d\n", toClient_cnf.result.keyresult.keycount);
#endif

	ALOGI("bye %s", __func__);

	_hidl_cb(toClient_cnf);
	return Void();
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IKeyinstall* HIDL_FETCH_IKeyinstall(const char* /* name */) {
    return new Keyinstall();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace keyinstall
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
