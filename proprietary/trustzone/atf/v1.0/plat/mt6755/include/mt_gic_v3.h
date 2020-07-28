/****************************************************************************
 *
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 *
 ***************************************************************************/
#ifndef __MT_GIC_V3_H
#define __MT_GIC_V3_H

#define GICD_V3_IROUTER_SPI_MODE_ANY       (0)

static void gicd_v3_set_irouter(unsigned int gicd_base, unsigned int id, uint64_t aff)
{
	unsigned int val;

	val = gicd_read_itargetsr(gicd_base, id);

	gicd_write_itargetsr(gicd_base, id, val & ~(0xff << ((id%4)*8)));
	gicd_set_itargetsr(gicd_base, id, (unsigned int)aff);
}
#endif /* end of __MT_GIC_V3_H */
