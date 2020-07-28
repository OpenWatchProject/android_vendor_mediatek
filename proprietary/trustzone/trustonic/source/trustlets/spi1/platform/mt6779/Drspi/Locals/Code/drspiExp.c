

#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "spi.h"
#include "drtlspi_api.h"

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "spi_debug.h"


/* the fake function defaine to fix build error, if you meet the log below, please contact your fingerprint vendor for help */

int spi_read_image(uint8_t *origindata, uint32_t origindata_size, uint32_t package_size)
{
	int rx_buf = 0, tx_buf = 0;

	SPI_ERR("[%s] the fake function.\n", __func__);
	dr_spi_rw(&rx_buf, &tx_buf, 0);

    return 0;
}
