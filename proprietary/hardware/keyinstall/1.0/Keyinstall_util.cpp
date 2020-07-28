#include "Keyinstall.h"

void convertCNF2HIDL(const FT_DRMKEY_INSTALL_CNF *s, HIDL_FT_DRMKEY_INSTALL_CNF *t)
{
	/* header */
	t->header.token = s->header.token;
	t->header.id = s->header.id;

	/* op */
	switch (s->op) {
		case FT_DRMKEY_INSTALL_SET:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_SET;
			break;
		case FT_DRMKEY_INSTALL_QUERY:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_QUERY;
			break;
		case FT_DRMKEY_INSTALL_END:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_END;
			break;
		default:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_END;
			break;
	}

	/* status */
	switch (s->status) {
		case DRMKEY_INSTALL_OK:
			t->status = HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_OK;
			break;
		case DRMKEY_INSTALL_FAIL:
			t->status = HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_FAIL;
			break;
		case DRMKEY_INSTALL_VERIFY_FAIL:
			t->status = HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_VERIFY_FAIL;
			break;
		default:
			t->status = HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_VERIFY_FAIL;
			break;
	}

	/* result */
	t->result.set_cnf.result = s->result.set_cnf.result;
	t->result.keyresult.keycount = s->result.keyresult.keycount;
	for (uint32_t i = 0; i < sizeof(t->result.keyresult.keytype)/sizeof(t->result.keyresult.keytype[0]); i++)
		t->result.keyresult.keytype[i] = s->result.keyresult.keytype[i];
}

void convertCNF2nonHIDL(const HIDL_FT_DRMKEY_INSTALL_CNF *s, FT_DRMKEY_INSTALL_CNF *t)
{
	/* header */
	t->header.token = s->header.token;
	t->header.id = s->header.id;

	/* op */
	switch (s->op) {
		case HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_SET:
			t->op = FT_DRMKEY_INSTALL_SET;
			break;
		case HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_QUERY:
			t->op = FT_DRMKEY_INSTALL_QUERY;
			break;
		case HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_END:
			t->op = FT_DRMKEY_INSTALL_END;
			break;
		default:
			t->op = FT_DRMKEY_INSTALL_END;
			break;
	}

	/* status */
	switch (s->status) {
		case HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_OK:
			t->status = DRMKEY_INSTALL_OK;
			break;
		case HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_FAIL:
			t->status = DRMKEY_INSTALL_FAIL;
			break;
		case HIDL_FT_DRMKEY_INSTALL_RESULT::HIDL_DRMKEY_INSTALL_VERIFY_FAIL:
			t->status = DRMKEY_INSTALL_VERIFY_FAIL;
			break;
		default:
			t->status = DRMKEY_INSTALL_VERIFY_FAIL;
			break;
	}

	/* result */
	t->result.set_cnf.result = s->result.set_cnf.result;
	t->result.keyresult.keycount = s->result.keyresult.keycount;
	for (uint32_t  i = 0; i < sizeof(t->result.keyresult.keytype)/sizeof(t->result.keyresult.keytype[0]); i++)
		t->result.keyresult.keytype[i] = s->result.keyresult.keytype[i];
}

void convertREQ2nonHIDL(const HIDL_FT_DRMKEY_INSTALL_REQ *s, FT_DRMKEY_INSTALL_REQ *t)
{
	/* header */
	t->header.token = s->header.token;
	t->header.id = s->header.id;

	/* op */
	switch (s->op) {
		case HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_SET:
			t->op = FT_DRMKEY_INSTALL_SET;
			break;
		case HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_QUERY:
			t->op = FT_DRMKEY_INSTALL_QUERY;
			break;
		case HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_END:
			t->op = FT_DRMKEY_INSTALL_END;
			break;
		default:
			t->op = FT_DRMKEY_INSTALL_END;
			break;

	}

	/* cmd */
	t->cmd.set_req.file_size = s->cmd.set_req.file_size;
	t->cmd.set_req.stage = s->cmd.set_req.stage;
	t->cmd.query_req.req = s->cmd.query_req.req;
}

void convertVector2Array(std::vector<uint8_t> in, char *out)
{
	int size = in.size();
	for (int i = 0; i < size; i++) {
		out[i] = in.at(i);
	}
}

void convertREQ2HIDL(const FT_DRMKEY_INSTALL_REQ *s, HIDL_FT_DRMKEY_INSTALL_REQ *t)
{
	/* header */
	t->header.token = s->header.token;
	t->header.id = s->header.id;

	/* op */
	switch (s->op) {
		case FT_DRMKEY_INSTALL_SET:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_SET;
			break;
		case FT_DRMKEY_INSTALL_QUERY:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_QUERY;
			break;
		case FT_DRMKEY_INSTALL_END:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_END;
			break;
		default:
			t->op = HIDL_FT_DRMKEY_INSTALL_OP::HIDL_FT_DRMKEY_INSTALL_END;
			break;
	}

	/* cmd */
	t->cmd.set_req.file_size = s->cmd.set_req.file_size;
	t->cmd.set_req.stage = s->cmd.set_req.stage;
	t->cmd.query_req.req = s->cmd.query_req.req;
}
