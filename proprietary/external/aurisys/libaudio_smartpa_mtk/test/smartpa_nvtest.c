#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "CFG_SMARTPA_CALIB_File.h"
#include "CFG_SMARTPA_CALIB_Default.h"
#include "libfile_op.h"

#define MAGIC_NUMBER	(5526789)

int main(int argc, char **argv)
{
	F_ID smartpa_calib_fd;
	int file_lid = AP_CFG_CUSTOM_FILE_SMARTPA_CALIB_LID;
	int size, num, result;
	int rd = 0, wr = 0, bk = 0, magic = 0, arg;
	ap_nvram_smartpa_calib_config_struct val;

#ifdef __DEBUG
	printf("smartpa_nvtest ++\n");
#endif /* __DEBUG */
	memset(&val, 0, sizeof(ap_nvram_smartpa_calib_config_struct));
	while ((arg = getopt(argc, argv, "m:rw:b")) != -1) {
		switch (arg) {
		case 'm':
			result = sscanf(optarg, "%d", &magic);
			if (result != 1)
				return -1;
			break;
		case 'r':
			rd = 1;
			break;
		case 'w':
			if (sscanf(optarg, "%d", &(val.calib_dcr_val)) == 1)
				wr = 1;
			break;
		case 'b':
			bk = 1;
		default:
			break;
		}
	}
	if (magic != MAGIC_NUMBER) {
#ifdef __DEBUG
		printf("magic number [%d] error\n", magic);
#endif /* __DEBUG */
		return -1;
	}
	if (wr) {
		smartpa_calib_fd = NVM_GetFileDesc(file_lid, &size, &num, ISWRITE);
#ifdef __DEBUG
		printf("size = %d, num = %d\n", size, num);
#endif /* __DEBUG */
#ifdef __DEBUG
		printf("write value dcr %d\n", val.calib_dcr_val);
#endif /* __DEBUG */
		result = write(smartpa_calib_fd.iFileDesc, &val, size * num);
		NVM_CloseFileDesc(smartpa_calib_fd);
	}
	if (rd) {
		smartpa_calib_fd = NVM_GetFileDesc(file_lid, &size, &num, ISREAD);
		result = read(smartpa_calib_fd.iFileDesc, &val, size * num);
		NVM_CloseFileDesc(smartpa_calib_fd);
		if (result != size * num) {
#ifdef __DEBUG
			printf("result size not match\n");
#endif /* __DEBUG */
			memcpy(&val, &smartpa_calib_ConfigDefault, sizeof(val));
			return -1;
#ifdef __DEBUG
		} else
			printf("result OK\n");
		printf("%d\n", val.calib_dcr_val);
#else
		}
		printf("%d\n", val.calib_dcr_val);
#endif /* __DEBUG */
	}
	if (bk) {
		result = FileOp_BackupToBinRegion_All() ? 1 : 0;
#ifdef __DEBUG
		printf("backup result %d\n", result);
#endif
	}
#ifdef __DEBUG
	printf("smartpa_nvtest --\n");
#endif /* __DEBUG */
	return 0;
}
