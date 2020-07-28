/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 */

#include "HalAdaptationLayer.h"
#include "module.h"
#include <memory>

#include "KeyboxInject.h"
#include "log.h"
#if !RPMB_SUPPORT
#include "mcSo.h"
#endif

extern struct kmsetkey_module HAL_MODULE_INFO_SYM;

extern "C" {

/******************************************************************************/
__attribute__((visibility("default")))
int nv_kmsetkey_open( const struct hw_module_t* module, const char* id,
					struct hw_device_t** device)
{
	LOG_I("opening nv kmsetkey device.\n");

	if ( id == NULL )
		return -EINVAL;

	// Make sure we initialize only if module provided is known
	if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
			(module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
			(module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
			(0!=memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
					   sizeof(KMSETKEY_HARDWARE_MODULE_NAME)-1)) )
	{
		return -EINVAL;
	}

	std::unique_ptr<HalAdaptationLayer> kmsetkey_device(
			new HalAdaptationLayer(const_cast<hw_module_t*>(module)));

	if (!kmsetkey_device)
	{
		LOG_E("Heap exhuasted. Exiting...");
		return -ENOMEM;
	}

	*device = reinterpret_cast<hw_device_t*>(kmsetkey_device.release());
	LOG_I("Kmsetkey device created");
	return 0;
}

/******************************************************************************/
__attribute__((visibility("default")))
int nv_kmsetkey_close(hw_device_t *hw)
{
	if (hw == NULL)
		return 0; // Nothing to close closed

	HalAdaptationLayer* gk = reinterpret_cast<HalAdaptationLayer*>(hw);
	if (NULL == gk)
	{
		LOG_E("Kmsetkey not initialized.");
		return -ENODEV;
	}

	delete gk;
	return 0;
}
} // extern "C"

#if !RPMB_SUPPORT
#define ATTEST_KEYBOX_FILE "/mnt/vendor/persist/attest_keybox.so"
static int saveFile(const char *pPath, uint8_t* ppContent, size_t size)
{
	size_t res = 0;
	FILE *f = fopen(pPath, "wb");
	if (f == NULL) {
		printf("Error opening file!\n");
		return -3;
	}

	res = fwrite(ppContent, sizeof(uint8_t), size, f);
	fclose(f);
	if (res != size) {
		fprintf(stderr, "Saving keybox failed: %zu != %zu\n", res, size);
		return -4;
	}

	return 0;
}

static long getFileContent(const char *pPath, uint8_t** ppContent)
{
	FILE *pStream;
	long filesize;
	uint8_t *content = NULL;

	/* Open the file */
	pStream = fopen(pPath, "rb");
	if (pStream == NULL)
	{
		LOG_E("Error: Cannot open file: %s.", pPath);
		goto error;
	}

	if (fseek(pStream, 0L, SEEK_END) != 0)
	{
		LOG_E("Error: Cannot read file: %s.", pPath);
		goto error;
	}

	filesize = ftell(pStream);
	if (filesize < 0)
	{
		LOG_E("Error: Cannot get the file size: %s.", pPath);
		goto error;
	}

	if (filesize == 0)
	{
		LOG_E("Error: Empty file: %s.", pPath);
		goto error;
	}

	/* Set the file pointer at the beginning of the file */
	if (fseek(pStream, 0L, SEEK_SET) != 0)
	{
		LOG_E("Error: Cannot read file: %s.", pPath);
		goto error;
	}

	/* Allocate a buffer for the content */
	content = (uint8_t*)malloc(filesize);
	if (content == NULL)
	{
		LOG_E("Error: Cannot read file: Out of memory.");
		goto error;
	}

	/* Read data from the file into the buffer */
	if (fread(content, (size_t)filesize, 1, pStream) != 1)
	{
		LOG_E("Error: Cannot read file: %s.", pPath);
		goto error;
	}

	/* Close the file */
	fclose(pStream);
	*ppContent = content;

	/* Return number of bytes read */
	return filesize;

error:
	if (content != NULL)
		free(content);

	if (pStream != NULL)
		fclose(pStream);

	return 0;
}
#endif

/* -------------------------------------------------------------------------
   Implementation of HalAdaptationLayer methods
   -------------------------------------------------------------------------*/
int32_t HalAdaptationLayer::attest_key_install(const uint8_t *peakb,
		const uint32_t peakb_len)
{
	int32_t ret = 0;
	uint8_t *data_p = NULL;
	uint32_t data_len = peakb_len;
#if !RPMB_SUPPORT
	uint8_t *blob_p = NULL;
	uint32_t blob_len = MC_SO_SIZE(0, peakb_len); /* first param = 0 : no clear text part in the secure object */
#endif

	data_p = (uint8_t *)malloc(data_len);
	if (data_p == NULL) {
		LOG_E("Allocate data buffer of size %u failed!\n", data_len);
		fprintf(stderr, "Allocate data buffer of size %u failed!\n", data_len);
		return -1;
	}

#if !RPMB_SUPPORT
	blob_p = (uint8_t *)malloc(blob_len);
	if (blob_p == NULL) {
		LOG_E("Allocate blob buffer of size %u failed!\n", blob_len);
		fprintf(stderr, "Allocate blob buffer of size %u failed!\n", blob_len);
		free(data_p);
		return -2;
	}
#endif

	memcpy(data_p, peakb, data_len);
#if !RPMB_SUPPORT
	memset(blob_p, 0, blob_len);
#endif

	ret = tlcOpen(MC_SPID_SYSTEM);
	if (ret != MC_DRV_OK) {
		LOG_E("open TL session failed (0x%X)!\n", ret);
		fprintf(stderr, "Could not open Trusted Application session (0x%X).\n", ret);
		goto exit;
	}

#if RPMB_SUPPORT
	ret = tlcSetAttestationData(data_p, data_len);
#else
	ret = tlcSetAttestationData(data_p, data_len, blob_p, &blob_len);
#endif
	tlcClose();
	if (ret != MC_DRV_OK) {
		LOG_E("Set Attestation Data failed: 0x%X (%d)!\n", ret, ret);
		fprintf(stderr, "Set Attestation Data failed: 0x%X (%d)!\n", ret, ret);
		goto exit;
	}

#if !RPMB_SUPPORT
	/* store the secure object */
	ret = saveFile(ATTEST_KEYBOX_FILE, blob_p, (size_t)blob_len);
	if (ret != 0) {
		LOG_E("Writing Secure object failed (%d)!\n", ret);
		fprintf(stderr, "Writing Secure object failed (%d)!\n", ret);
		goto exit;
	}
#endif

exit:
	free(data_p);
#if !RPMB_SUPPORT
	free(blob_p);
#endif

	return ret;
}

int32_t HalAdaptationLayer::attest_key_check(const uint8_t *peakb,
		const uint32_t peakb_len)
{
	int32_t ret = 0;
	uint8_t *data_p = NULL;
	uint32_t data_len = peakb_len;
#if !RPMB_SUPPORT
	uint8_t *blob_p = NULL;
	uint32_t blob_len = 0;
#endif

	data_p = (uint8_t *)malloc(data_len);
	if (data_p == NULL) {
		LOG_E("Allocate data buffer of size %u failed!\n", data_len);
		fprintf(stderr, "Allocate data buffer of size %u failed!\n", data_len);
		return -1;
	}

#if !RPMB_SUPPORT
	blob_len = getFileContent(ATTEST_KEYBOX_FILE, &blob_p);
	if (!blob_len) {
		LOG_E("Retrieve attestation key failed!\n");
		fprintf(stderr, "Retrieve attestation key failed!\n");
		free(data_p);
		return -2;
	}
#endif

	memcpy(data_p, peakb, data_len);
	ret = tlcOpen(MC_SPID_SYSTEM);
	if (ret != MC_DRV_OK) {
		LOG_E("open TL session failed (0x%uX)!\n", ret);
		fprintf(stderr, "Could not open Trusted Application session (0x%X).\n", ret);
		goto exit;
	}

#if RPMB_SUPPORT
	ret = tlcCheckAttestationData(data_p, data_len);
#else
	ret = tlcCheckAttestationData(data_p, data_len, blob_p, blob_len);
#endif
	tlcClose();
	if (ret != MC_DRV_OK) {
		LOG_E("Verify Attestation Data failed: 0x%X (%d)!\n", ret, ret);
		fprintf(stderr, "Verify Attestation Data failed: 0x%X (%d)!\n", ret, ret);
		goto exit;
	}

exit:
	free(data_p);
#if !RPMB_SUPPORT
	free(blob_p);
#endif

	return ret;
}

/******************************************************************************/
HalAdaptationLayer::HalAdaptationLayer(hw_module_t* module)
{
	LOG_I("kmsetkey: HalAdaptationLayer constructor initialized.");
	/* -------------------------------------------------------------------------
	   Device description
	   -------------------------------------------------------------------------*/
	_device.common.tag = HARDWARE_MODULE_TAG;
	_device.common.version = 1;
	_device.common.module = module;
	_device.common.close = nv_kmsetkey_close;

	/* -------------------------------------------------------------------------
	   All function pointers used by the HAL module
	   -------------------------------------------------------------------------*/
	_device.attest_key_install = HalAdaptationLayer::attest_key_install;
	_device.attest_key_check = HalAdaptationLayer::attest_key_check;

}

/******************************************************************************/


