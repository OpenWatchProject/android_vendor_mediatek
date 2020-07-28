#include <platform.h>
#include <debug.h>
#include <devapc.h>
#include <devapc_hal.h>

/* TEE module's APC setting */
static struct devapc_set_callbacks devapc_tee_cb[] = {
	{ DEVAPC_MODULE_REQ_CAMERA_ISP, devapc_tee_camera_isp },
	{ DEVAPC_MODULE_REQ_IMGSENSOR, devapc_tee_imgsensor },
	{ DEVAPC_MODULE_REQ_VDEC, devapc_tee_vdec },
	{ DEVAPC_MODULE_REQ_VENC, NULL },
	{ DEVAPC_MODULE_REQ_M4U, devapc_tee_m4u },
	{ DEVAPC_MODULE_REQ_I2C, devapc_tee_i2c },
	{ DEVAPC_MODULE_REQ_SPI, devapc_tee_spi },
	{ DEVAPC_MODULE_REQ_DISP, devapc_tee_disp }

};

static struct devapc_set_callbacks devapc_tee_mast_cb[] = {
	{ DEVAPC_MASTER_REQ_SPI, devapc_tee_mast_spi },
};

uint32_t devapc_set_protect(enum DEVAPC_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	INFO("[DEVAPC] %s\n", __func__);
	for (int i = 0; i < DEVAPC_MODULE_REQ_NUM; i++) {
		if (devapc_tee_cb[i].req_type == module)
			return devapc_tee_cb[i].devapc_set_cb(onoff, param);
	}

	return DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED;
}

uint32_t devapc_set_master_trans(enum DEVAPC_MODULE_REQ_TYPE module,
		enum DEVAPC_PROTECT_ON_OFF onoff, uint32_t param)
{
	INFO("[DEVAPC] %s\n", __func__);
	for (int i = 0; i < DEVAPC_MASTER_REQ_NUM; i++) {
		if (devapc_tee_mast_cb[i].req_type == module)
			return devapc_tee_mast_cb[i].devapc_set_cb(onoff, param);
	}

	return DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED;
}
