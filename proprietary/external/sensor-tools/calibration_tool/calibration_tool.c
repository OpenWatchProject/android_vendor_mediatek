#include "libhwm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/Log.h>

/*--------------------------------------------------------------------------
 * Name: _print_help_message
 *
 * Description: This API print help message.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: -
 -----------------------------------------------------------------------------*/
static void _print_help_message(const char *execute) {
	printf("\n");
	printf("Usage: %s [options] <period> <count> <tolerance> <trace>\n",
			execute);
	printf("\n");
	printf("Description:\n");
	printf("\tThis tool used for calibrating gsensor or gyroscope.\n");
	printf("\n");
	printf("Options:\n");
	printf("\t-h\tShow the help information for this tool.\n");
	printf("\t-gs\tThis option to calibrate gsensor.\n");
	printf("\t-gy\tThis option to calibrate gyroscope.\n");
	printf("\t-c\tThis option used to clear driver and nvram data.\n");
	printf("\t-w\tThis option used to write calibration result to driver and nvram.\n");
	printf("\n");
	printf("Parameters:\n");
	printf("\t<period>: Delay time of read sensor data.\n");
	printf("\t<count>: Count of read sensor data.\n");
	printf("\t<tolerance>: Tolerance(20 or 40) of calibration fail.\n");
	printf("\t<trace>: Unused.\n");
	printf("\n");
}

static int _gsensor_cali(int to_clear, int to_cali, int to_write,
		const char *period, const char *count, const char *tolerance) {
	int fd = -1;
	int err = 0;
	int i_period = (int) atoi(period);
	int i_count = (int) atoi(count);
	int i_tolerance = (int) atoi(tolerance);
	int ret = -1;

	HwmData cali;

	err = gsensor_open(&fd);
	if (err < 0) {
		printf("gsensor_open() error. \n");
		return -1;
	}

	do {
		if (to_clear) {

			err = gsensor_rst_cali(fd);

			if (err != 0) {
				printf("gsensor_rst_cali() error. \n");
				ret = -1;
				break;
			}

			cali.x = 0;
			cali.y = 0;
			cali.z = 0;
			err = gsensor_write_nvram(&cali);
			if (err != 0) {
				printf("gsensor_write_nvram() reset error. \n");
				ret = -1;
				break;
			}
		}

		if (to_cali) {
			err = gsensor_calibration(fd, i_period, i_count, i_tolerance, &cali);
			if (err != 0) {
				printf("gsensor_calibration() error. \n");
				ret = -1;
				break;
			}
		}

		if (to_write) {
			err = gsensor_set_cali(fd, &cali);
			if (err != 0) {
				printf("gsensor_set_cali() error. \n");
				ret = -1;
				break;
			}

			err = gsensor_write_nvram(&cali);
			if (err != 0) {
				printf("gsensor_write_nvram() error. \n");
				ret = -1;
				break;
			}
		}

		ret = 0;
	} while (0);

	gsensor_close(fd);
	return ret;
}

static int _gyroscope_cali(int to_clear, int to_cali, int to_write,
		const char *period, const char *count, const char *tolerance,
		const char *trace) {
	int fd = -1;
	int err = 0;
	int i_period = (int) atoi(period);
	int i_count = (int) atoi(count);
	int i_tolerance = (int) atoi(tolerance);
	int ret = -1;

	HwmData cali;

	err = gyroscope_open(&fd);
	if (err < 0) {
		printf("gyroscope_open() error. \n");
		return -1;
	}

	do {
		if (to_clear) {
			err = gyroscope_rst_cali(fd);
			if (err != 0) {
				printf("gyroscope_calibration() error. \n");
				ret = -1;
				break;
			}

			cali.x = 0;
			cali.y = 0;
			cali.z = 0;
			err = gyroscope_write_nvram(&cali);
			if (err != 0) {
				printf("gyroscope_write_nvram() reset error. \n");
				ret = -1;
				break;
			}
		}

		if (to_cali) {
			err = gyroscope_calibration(fd, i_period, i_count, i_tolerance*10, &cali);
			if (err != 0) {
				printf("gyroscope_calibration() error. \n");
				ret = -1;
				break;
			}
		}

		if (to_write) {
			err = gyroscope_set_cali(fd, &cali);
			if (err != 0) {
				printf("gyroscope_set_cali() error. \n");
				ret = -1;
				break;
			}

			err = gyroscope_write_nvram(&cali);
			if (err != 0) {
				printf("gyroscope_write_nvram() error. \n");
				ret = -1;
				break;
			}
		}

		ret = 0;
	} while (0);

	gyroscope_close(fd);
	return ret;
}

/*--------------------------------------------------------------------------
 * Name: main
 *
 * Description: This API begin run calibration tool.
 *
 * Inputs:  calibration tool option.
 *          gsensor module:
 *          argv[1]: -gs
 *          argv[2]: -w (Write calibration to nvram.)
 *          argv[3]: period, delay time of read sensor data(MTK Turnkey is 50ms).
 *          argv[4]: count, count of read sensor data(MTK Turnkey is 20 times).
 *          argv[5]: tolerance, tolerance of calibration fail(MTK Turnkey is 20% or 40%).
 *          argv[6]: trace, unused(MTK Turnkey is 0).
 *
 *          gyrosope module:
 *          argv[1]: -gy
 *          argv[2]: -w (Write calibration to nvram.)
 *          argv[3]: period, delay time of read sensor data(MTK Turnkey is 50ms).
 *          argv[4]: count, count of read sensor data(MTK Turnkey is 20 times).
 *          argv[5]: tolerance, tolerance of calibration fail(MTK Turnkey is 20% or 40%).
 *          argv[6]: trace, unused(MTK Turnkey is 0).
 *
 * Outputs: -
 *
 * Return:  0 Calibration pass.
 *         -1 Calibration fail.
 -----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

	int ret = -1;

	if (argc < 3 || argc > 8 || ((strcmp(argv[1], "-gs") != 0) && (strcmp(
			argv[1], "-gy") != 0))) {
		_print_help_message(argv[0]);
		return ret;
	}

	if ((argc == 3) && (0 == strcmp(argv[2], "-c"))) {
		if (0 == strcmp(argv[1], "-gs")) {
			ret = _gsensor_cali(1, 0, 0, "1", "1", "1");
		} else {
			ret = _gyroscope_cali(1, 0, 0, "1", "1", "1");
		}

		if (ret == 0) {
			printf("Clear Success.\n");
		} else {
			printf("Clear Fail.\n");
		}

		return 0;
	}

	if (argc == 6) {
		if (0 == strcmp(argv[1], "-gs")) {
			ret = _gsensor_cali(0, 1, 0, argv[2], argv[3], argv[4]);
		} else {
			ret = _gyroscope_cali(0, 1, 0, argv[2], argv[3], argv[4]);
		}
	}

	if ((argc == 7) && (0 == strcmp(argv[2], "-c"))) {
		if (0 == strcmp(argv[1], "-gs")) {
			ret = _gsensor_cali(1, 1, 0, argv[3], argv[4], argv[5]);
		} else {
			ret = _gyroscope_cali(1, 1, 0, argv[3], argv[4], argv[5]);
		}
	}

	if ((argc == 7) && (0 == strcmp(argv[2], "-w"))) {
		if (0 == strcmp(argv[1], "-gs")) {
			ret = _gsensor_cali(0, 1, 1, argv[3], argv[4], argv[5]);
		} else {
			ret = _gyroscope_cali(0, 1, 1, argv[3], argv[4], argv[5]);
		}
	}

	if ((argc == 8) && (0 == strcmp(argv[2], "-c")) && (0 == strcmp(argv[3],
			"-w"))) {
		if (0 == strcmp(argv[1], "-gs")) {
			ret = _gsensor_cali(1, 1, 1, argv[4], argv[5], argv[6]);
		} else {
			ret = _gyroscope_cali(1, 1, 1, argv[4], argv[5], argv[6]);
		}
	}

	if (ret == 0) {
		printf("Calibration Pass.\n");
	} else {
		printf("Calibration Fail.\n");
	}
	return ret;

}
