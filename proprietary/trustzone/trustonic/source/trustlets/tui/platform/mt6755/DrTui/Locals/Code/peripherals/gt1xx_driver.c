/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "mt_typedefs.h"
#include "i2c.h"
#include "DrApi/DrApiMm.h"

#include "log.h"

#include "mtk_common_tpd.h"
#include "gt1xx_touch_pram.h"


#define TAG "[touch_GT1XX]"
#define TOUCH_TUI_API 1

u8 gtp_rawdiff_mode = 0;
struct tpd_tui_obj gt1xx_tui_obj = {0};
extern void test_i2c(void);

static u32 last_x = 0;
static u32 last_y = 0;

static U8 *write_byte = NULL, *read_byte = NULL;
static U8 *write_byte_pa = 0xA3C00000, *read_byte_pa = 0xA3C10000;

#if !TOUCH_TUI_API
int i2c_read_bytes_non_dma(struct i2c_client *client, u16 addr, u8 *rxbuf, int len)
{
    u8 buffer[GTP_ADDR_LENGTH];
    u8 retry;
    u16 left = len;
    u16 offset = 0;

    struct i2c_msg msg[2] =
    {
        {
            .addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
            //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_PUSHPULL_FLAG)),
            .flags = 0,
            .buf = buffer,
            .len = GTP_ADDR_LENGTH,
            .timing = I2C_MASTER_CLOCK
        },
        {
            .addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
            //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_PUSHPULL_FLAG)),
            .flags = I2C_M_RD,
            .timing = I2C_MASTER_CLOCK
        },
    };

    if (rxbuf == NULL)
        return -1;

    drDbgPrintLnf("i2c_read_bytes to device %02X address %04X len %d", client->addr, addr, len);

    while (left > 0)
    {
        buffer[0] = ((addr + offset) >> 8) & 0xFF;
        buffer[1] = (addr + offset) & 0xFF;

        msg[1].buf = &rxbuf[offset];

        if (left > MAX_TRANSACTION_LENGTH)
        {
            msg[1].len = MAX_TRANSACTION_LENGTH;
            left -= MAX_TRANSACTION_LENGTH;
            offset += MAX_TRANSACTION_LENGTH;
        }
        else
        {
            msg[1].len = left;
            left = 0;
        }

        retry = 0;

        while (i2c_transfer(client->adapter, &msg[0], 2) != 2)
        {
            retry++;

            if (retry == 5)
            {
                GTP_ERROR("I2C read 0x%X length=%d failed\n", addr + offset, len);
                return -1;
            }
        }
    }

    return 0;
}


int i2c_read_bytes(struct i2c_client *client, u16 addr, u8 *rxbuf, int len)
{
#if GTP_SUPPORT_I2C_DMA
    return i2c_read_bytes_dma(client, addr, rxbuf, len);
#else
    return i2c_read_bytes_non_dma(client, addr, rxbuf, len);
#endif
}

s32 gtp_i2c_read(struct i2c_client *client, u8 *buf, s32 len)
{
    s32 ret = -1;
    u16 addr = (buf[0] << 8) + buf[1];

    ret = i2c_read_bytes(client, addr, &buf[2], len - 2);

    if (!ret)
    {
        return 2;
    }
    else
    {
		return ret;
    }
}

int i2c_write_bytes_non_dma(struct i2c_client *client, u16 addr, u8 *txbuf, int len)
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u16 left = len;
    u16 offset = 0;
    u8 retry = 0;

    struct i2c_msg msg =
    {
        .addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
        //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_PUSHPULL_FLAG)),
        .flags = 0,
        .buf = buffer,
        .timing = I2C_MASTER_CLOCK,
    };


    if (txbuf == NULL)
        return -1;

    drDbgPrintLnf("i2c_write_bytes to device %02X address %04X len %d", client->addr, addr, len);

    while (left > 0)
    {
        retry = 0;

        buffer[0] = ((addr + offset) >> 8) & 0xFF;
        buffer[1] = (addr + offset) & 0xFF;

        if (left > MAX_I2C_TRANSFER_SIZE)
        {
            memcpy(&buffer[GTP_ADDR_LENGTH], &txbuf[offset], MAX_I2C_TRANSFER_SIZE);
            msg.len = MAX_TRANSACTION_LENGTH;
            left -= MAX_I2C_TRANSFER_SIZE;
            offset += MAX_I2C_TRANSFER_SIZE;
        }
        else
        {
            memcpy(&buffer[GTP_ADDR_LENGTH], &txbuf[offset], left);
            msg.len = left + GTP_ADDR_LENGTH;
            left = 0;
        }

        //drDbgPrintLnf("byte left %d offset %d\n", left, offset);

        while (i2c_transfer(client->adapter, &msg, 1) != 1)
        {
            retry++;

            if (retry == 5)
            {
                GTP_ERROR("I2C write 0x%X%X length=%d failed\n", buffer[0], buffer[1], len);
                return -1;
            }
        }
    }

    return 0;
}

int i2c_write_bytes(struct i2c_client *client, u16 addr, u8 *txbuf, int len)
{
#if GTP_SUPPORT_I2C_DMA
    return i2c_write_bytes_dma(client, addr, txbuf, len);
#else
    return i2c_write_bytes_non_dma(client, addr, txbuf, len);
#endif
}

s32 gtp_i2c_write(struct i2c_client *client, u8 *buf, s32 len)
{
    s32 ret = -1;
    u16 addr = (buf[0] << 8) + buf[1];

    ret = i2c_write_bytes(client, addr, &buf[2], len - 2);

    if (!ret)
    {
        return 1;
    }
    else
    {
    #if GTP_SLIDE_WAKEUP
        if (DOZE_ENABLED == doze_status)
        {
            return ret;
        }
    #endif
    #if GTP_COMPATIBLE_MODE
        if (CHIP_TYPE_GT9F == gtp_chip_type)
        {
            gtp_recovery_reset(client);
        }
        else
    #endif
        {
            gtp_reset_guitar(client, 20);
        }
        return ret;
    }
}


#else
int i2c_read_bytes_dma_tui(struct mt_i2c_t *i2c_device, u16 addr, u8 *rxbuf, s32 len)
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u8 retry;
    u16 left = len;
    u16 offset = 0;
	u32 transfer_lenth = 0;
	int ret,times = 0;

    if (rxbuf == NULL)
        return -1;

	//drDbgPrintLnf(TAG "i2c_read_bytes_dma_tui1\n ");

    while (left > 0)
    {
		write_byte[0] = ((addr + offset) >> 8) & 0xFF;
        write_byte[1] = (addr + offset) & 0xFF;

		//drDbgPrintLnf(TAG "i2c_read_bytes_dma_tui2\n ");

        if (left > MAX_DMA_TRANSACTION_LENGTH)
        {
            transfer_lenth = MAX_DMA_TRANSACTION_LENGTH;

			for(retry = 0;retry < 5;retry++){
				ret = i2c_write(i2c_device, write_byte_pa, GTP_ADDR_LENGTH);
				if(ret != I2C_OK)
					continue;
				ret = i2c_read(i2c_device, read_byte_pa, transfer_lenth);
				if(ret == I2C_OK)
					break;
			}

			if(ret != I2C_OK){
				drDbgPrintLnf(TAG "I2C transfer error. errno:%d\n ", ret);
				return -1;
			}
			memcpy(&rxbuf[offset], read_byte, MAX_DMA_TRANSACTION_LENGTH);

            left -= MAX_DMA_TRANSACTION_LENGTH;
            offset += MAX_DMA_TRANSACTION_LENGTH;

        }
        else
        {
            transfer_lenth = left;

			for(retry = 0;retry < 5;retry++){
				ret = i2c_write(i2c_device, write_byte_pa, GTP_ADDR_LENGTH);
				if(ret != I2C_OK)
					continue;
				ret = i2c_read(i2c_device, read_byte_pa, transfer_lenth);
				if(ret == I2C_OK)
					break;
			}

			if(ret != I2C_OK){
				drDbgPrintLnf(TAG "I2C transfer error. errno:%d\n ", ret);
				return -1;
			}

			memcpy(&rxbuf[offset], read_byte, transfer_lenth);

            left = 0;
        }

		//drDbgPrintLnf(TAG "Read data: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n",read_byte[0], read_byte[1],read_byte[2],read_byte[3],read_byte[4],read_byte[5],read_byte[6],read_byte[7]);
    }

	return ret;
}

int i2c_read_bytes_tui(struct mt_i2c_t *i2c_device, u16 addr, u8 *rxbuf, s32 len)
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u8 retry;
    u16 left = len;
    u16 offset = 0;
	u32 transfer_lenth = 0;
	int ret,times = 0;

    if (rxbuf == NULL)
        return -1;

    while (left > 0)
    {
		buffer[0] = ((addr + offset) >> 8) & 0xFF;
        buffer[1] = (addr + offset) & 0xFF;


        if (left > MAX_TRANSACTION_LENGTH)
        {
            transfer_lenth = MAX_TRANSACTION_LENGTH;

			for(retry = 0;retry < 5;retry++){
				ret = i2c_write(i2c_device, buffer, GTP_ADDR_LENGTH);
				if(ret != I2C_OK)
					continue;
				ret = i2c_read(i2c_device, buffer, transfer_lenth);
				if(ret == I2C_OK)
					break;
			}

			if(ret != I2C_OK){
				drDbgPrintLnf(TAG "I2C transfer error. errno:%d\n ", ret);
				return -1;
			}
			memcpy(&rxbuf[offset], buffer, MAX_TRANSACTION_LENGTH);

            left -= MAX_TRANSACTION_LENGTH;
            offset += MAX_TRANSACTION_LENGTH;

        }
        else
        {
            transfer_lenth = left;

			for(retry = 0;retry < 5;retry++){
				ret = i2c_write(i2c_device, buffer, GTP_ADDR_LENGTH);
				if(ret != I2C_OK)
					continue;
				ret = i2c_read(i2c_device, buffer, transfer_lenth);
				if(ret == I2C_OK)
					break;
			}

			if(ret != I2C_OK){
				drDbgPrintLnf(TAG "I2C transfer error. errno:%d\n ", ret);
				return -1;
			}

			memcpy(&rxbuf[offset], buffer, transfer_lenth);

            left = 0;
        }

		//drDbgPrintLnf(TAG "Read data: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n",buffer[0], buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
    }

	return ret;
}

s32 tui_i2c_read(struct mt_i2c_t *i2c_device, u8 *buf, s32 len)
{
    s32 ret = -1;
    u16 addr = (buf[0] << 8) + buf[1];

    ret = i2c_read_bytes_dma_tui(i2c_device, addr, &buf[2], len - 2);

    if (!ret)
    {
        return 2;
    }
    else
    {
		return ret;
    }
}

int i2c_write_bytes_dma_tui(struct mt_i2c_t *i2c_device, u16 addr, u8 *txbuf, s32 len)
{
	u8 buffer[MAX_TRANSACTION_LENGTH];
	u16 left = len;
	u16 offset = 0;
	u8 retry = 0;
	u32 transfer_lenth = 0;
	int ret = 0;

	if (txbuf == NULL)
		return -1;

	while (left > 0)
	{
		retry = 0;

		write_byte[0] = ((addr + offset) >> 8) & 0xFF;
		write_byte[1] = (addr + offset) & 0xFF;

		if (left > MAX_I2C_DMA_TRANSFER_SIZE)
		{
			memcpy(&write_byte[GTP_ADDR_LENGTH], &txbuf[offset], MAX_I2C_DMA_TRANSFER_SIZE);
			transfer_lenth = MAX_DMA_TRANSACTION_LENGTH;
			left -= MAX_I2C_DMA_TRANSFER_SIZE;
			offset += MAX_I2C_DMA_TRANSFER_SIZE;
		}
		else
		{
			memcpy(&write_byte[GTP_ADDR_LENGTH], &txbuf[offset], left);
			transfer_lenth = left + 2;
			left = 0;
		}

 		for(retry = 0;retry < 5;retry++){
			ret = i2c_write(i2c_device, write_byte_pa, transfer_lenth);
			if(ret == I2C_OK)
				break;
		}

		//drDbgPrintLnf(TAG "write data: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n",write_byte[0], write_byte[1],write_byte[2],write_byte[3],write_byte[4],write_byte[5],write_byte[6],write_byte[7]);
    }

	return ret;
}

int i2c_write_bytes_tui(struct mt_i2c_t *i2c_device, u16 addr, u8 *txbuf, s32 len)
{
	u8 buffer[MAX_TRANSACTION_LENGTH];
	u16 left = len;
	u16 offset = 0;
	u8 retry = 0;
	u32 transfer_lenth = 0;
	int ret = 0;

	if (txbuf == NULL)
		return -1;

	while (left > 0)
	{
		retry = 0;

		buffer[0] = ((addr + offset) >> 8) & 0xFF;
		buffer[1] = (addr + offset) & 0xFF;

		if (left > MAX_I2C_TRANSFER_SIZE)
		{
			memcpy(&buffer[GTP_ADDR_LENGTH], &txbuf[offset], MAX_I2C_TRANSFER_SIZE);
			transfer_lenth = MAX_TRANSACTION_LENGTH;
			left -= MAX_I2C_TRANSFER_SIZE;
			offset += MAX_I2C_TRANSFER_SIZE;
		}
		else
		{
			memcpy(&buffer[GTP_ADDR_LENGTH], &txbuf[offset], left);
			transfer_lenth = left;
			left = 0;
		}

 		for(retry = 0;retry < 5;retry++){
			ret = i2c_write(i2c_device, buffer, transfer_lenth);
			if(ret == I2C_OK)
				break;
		}

		//drDbgPrintLnf(TAG "write data: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x \n",buffer[0], buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
    }

	return ret;
}

s32 tui_i2c_write(struct mt_i2c_t *i2c_device, u8 *buf, s32 len)
{
    s32 ret = -1;
    u16 addr = (buf[0] << 8) + buf[1];

    ret = i2c_write_bytes_dma_tui(i2c_device, addr, &buf[2], len);
    if (!ret)
    {
        return 1;
    }
    else
    {
        return ret;
    }
}
#endif

/**
 * gt1x_touch_event_handler - handle touch event
 * (pen event, key event, finger touch envent)
 * @data:
 * Return    <0: failed, 0: succeed
 */
s32 gt1x_touch_event_handler(struct mt_i2c_t *i2c, u8 *data, struct tpd_data *pointerdata)
{
	u8 touch_data[1 + 8 * GTP_MAX_TOUCH + 2] = { 0 };
	u8 touch_num = 0;
	u16 cur_event = 0;
	static u16 pre_event = 0;
	static u16 pre_index = 0;

	u8 key_value = 0;
	u8 *coor_data = NULL;
	s32 input_x = 0;
	s32 input_y = 0;
	s32 input_w = 0;
	s32 id = 0;
	s32 i = 0;
	s32 ret = -1;

	touch_num = data[0] & 0x0f;
	if (touch_num > GTP_MAX_TOUCH) {
		drDbgPrintLnf("Illegal finger number!");
		return ERROR_VALUE;
	}

	drDbgPrintLnf(TAG "finger number:%d!", touch_num);

	memcpy(touch_data, data, 11);
	drDbgPrintLnf(TAG "I2C  touch_data:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x, 0x%x, 0x%x, 0x%x!",touch_data[0], touch_data[1],touch_data[2],touch_data[3],touch_data[4],touch_data[5],touch_data[6],touch_data[7],touch_data[8], touch_data[9], touch_data[10]);

	/* read the remaining coor data */
	if (touch_num > 1) {
		ret = i2c_read_bytes_dma_tui(i2c, (GTP_READ_COOR_ADDR + 11), &touch_data[11], 1 + 8 * touch_num + 2 - 11);
		if (ret) {
			drDbgPrintLnf("Read coordinate i2c error.");
			return ret;
		}
	}

	/* checksum */

/*
 * cur_event , pre_event bit defination
 * bit4	bit3		    bit2	 bit1	   bit0
 * hover  stylus_key  stylus  key     touch
 *
 */
	key_value = touch_data[1 + 8 * touch_num];

	/* check current event */
	if ((touch_data[0] & 0x10) && key_value) {
		drDbgPrintLnf(TAG "TOUCH handler touch_data[0]:0x%x, key_value: 0x%x", touch_data[0], key_value);
	} else if (touch_num) {
		SET_BIT(cur_event, BIT_TOUCH);
	}

/* finger touch event*/
	if (CHK_BIT(cur_event, BIT_TOUCH)) {
		u8 report_num = 0;
		coor_data = &touch_data[1];
		id = coor_data[0] & 0x0F;
		for (i = 0; i < GTP_MAX_TOUCH; i++) {
			if (i == id) {
				input_x = coor_data[1] | (coor_data[2] << 8);
				input_y = coor_data[3] | (coor_data[4] << 8);
				input_w = coor_data[5] | (coor_data[6] << 8);

				pointerdata->position_x = input_x;
				pointerdata->position_y = input_y;
				pointerdata->finger_states = 1;
				last_x = input_x;
				last_y = input_y;
				drDbgPrintLnf("touch event down:ID:%d, (%d, %d)", i , input_x, input_y);
				if (report_num++ < touch_num) {
					coor_data += 8;
					id = coor_data[0] & 0x0F;
				}
				pre_index |= 0x01 << i;
			} else if (pre_index & (0x01 << i)) {
				pre_index &= ~(0x01 << i);
			}
		}
	} else if (CHK_BIT(pre_event, BIT_TOUCH)) {
		pointerdata->position_x = last_x;
		pointerdata->position_y = last_y;
		pointerdata->finger_states = 0;
		drDbgPrintLnf("touch event up:ID:%d, (%d, %d)", i , last_x, last_y);
		drDbgPrintLnf("Released Touch.");
		pre_index = 0;
	}

	if (!pre_event && !cur_event) {
		drDbgPrintLnf("Additional Int Pulse.");
	} else {
		pre_event = cur_event;
	}

	return 0;
}

int gt1xx_tpd_clear_intr(void)
{
	return 0;
}

int gt1xx_tpd_enable(int enable)
{
	return 0;
}

int gt1xx_tpd_read_data(struct tpd_data *pointerdata)
{
	u8 end_cmd = 0, i = 0;
	s32 ret = 0;
	u8 point_data[11] = { 0 };
	u8 data[299] = { 0 };
	u8 finger = 0;

#if TOUCH_TUI_API
		struct mt_i2c_t i2c = {0};

		i2c.id = 0;
		i2c.addr = 0x5d;
		i2c.mode = ST_MODE;
		i2c.speed = 100;
#endif


	ret = i2c_read_bytes_dma_tui(&i2c, GTP_READ_COOR_ADDR, point_data, 11);
	if (ret < 0) {
		drDbgPrintLnf("I2C transfer error!");
		goto exit_work_func;
	}

	//drDbgPrintLnf(TAG "gt1xx_tpd_read_data\n ");
	finger = point_data[0];

	/* response to a ic request */

	if ((finger & 0x80) == 0) {
		drDbgPrintLnf("buffer not ready:0x%02x", finger);
		return -1;
	}

	ret = gt1x_touch_event_handler(&i2c, point_data, pointerdata);
	if (ret < 0) {
		drDbgPrintLnf("I2C transfer error!");
		goto exit_work_func;
	}

	exit_work_func:
	if (!gtp_rawdiff_mode)
	{
		end_cmd = 0;
		ret = i2c_write_bytes_dma_tui(&i2c, GTP_READ_COOR_ADDR, &end_cmd, 1);

		if (ret < 0)
		{
			drDbgPrintLnf(TAG "I2C write end_cmd  error!");
		}

	}

	return 0;
}

int gt1xx_tpd_rst(void)
{
	return 0;
}

int gt1xx_driver_init(void)
{
	int err = 0;
	drDbgPrintLnf(TAG "%s: enter init function", __func__);
	test_touch();

	if(&gt1xx_tui_obj == NULL){
		drDbgPrintLnf(TAG "%s: drApiMalloc tui memory fail", __func__);
		return -1;
	}

	gt1xx_tui_obj.vendor_tpd_clear_intr = gt1xx_tpd_clear_intr;
	gt1xx_tui_obj.vendor_tpd_enable 	= gt1xx_tpd_enable;
	gt1xx_tui_obj.vendor_tpd_read_data 	= gt1xx_tpd_read_data;
	gt1xx_tui_obj.vendor_tpd_rst		= gt1xx_tpd_rst;

	err = register_mtk_tpd_tui_api(&gt1xx_tui_obj);
	if(err){
		drDbgPrintLnf(TAG "%s: register common tui api fail", __func__);
		return err;
	}
    return err;
}

int map_i2c_hw_register(void)
{
    uint32_t mapFlag = MAP_READABLE | MAP_WRITABLE | MAP_IO;
    drApiMapPhysicalBuffer((uint64_t)I2C0_BASE_PA,0x1000,mapFlag,(void **)&i2c0_va);
    drApiMapPhysicalBuffer((uint64_t)I2C1_BASE_PA,0x1000,mapFlag,(void **)&i2c1_va);
    drApiMapPhysicalBuffer((uint64_t)I2C2_BASE_PA,0x1000,mapFlag,(void **)&i2c2_va);
	drApiMapPhysicalBuffer((uint64_t)AP_DMA_BASE,0x1000,mapFlag,(void **)&DMA_BASE_VA);
    printf("i2c0_map_regiser CFG, hasMapped [%x]\n", (i2c0_va));
    printf("i2c1_map_regiser CFG, hasMapped [%x]\n", (i2c1_va));
    printf("i2c2_map_regiser CFG, hasMapped [%x]\n", (i2c2_va));
    printf("fwq0   base=%x, base= %x\n",I2C0_BASE );
    printf("fwq1   base=%x, base= %x\n",(U32*)I2C1_BASE );
    printf("fwq2   base=%x, base= %x\n",I2C2_BASE );
}

int map_i2c_dma_buffer(void)
{
    uint32_t mapFlag = MAP_READABLE | MAP_WRITABLE | MAP_UNCACHED | MAP_IO;
	drApiMapPhysicalBuffer((uint64_t)read_byte_pa,0x100,mapFlag,(void **)&read_byte );
	drApiMapPhysicalBuffer((uint64_t)write_byte_pa,0x100,mapFlag,(void **)&write_byte );
    printf("i2c dma mmap read pa=0x%x va=0x%x\n", read_byte_pa, read_byte);
    printf("i2c dma mmap write pa=0x%x va=0x%x\n", write_byte_pa, write_byte);
}

int unmap_i2c_dma_buffer(void)
{
	if (read_byte)
		drApiUnmapBuffer(read_byte);
	if (write_byte)
		drApiUnmapBuffer(write_byte);
	read_byte = NULL;
	write_byte = NULL;
}

void set_and_map_i2c_dma_pa(uint32_t physAddr, uint32_t size)
{
	write_byte_pa = (U8*)physAddr;
	read_byte_pa = (U8*)(physAddr+0x10000);
	printf("i2c dma write pa:0x%x read pa:0x%x", write_byte_pa, read_byte_pa);
	map_i2c_dma_buffer();
}

void test_touch(void)
{
    drDbgPrintLnf("test_touch+4\n");

    int len, ret;
	struct tpd_data test_data = {0};
	int i = 100;

	map_i2c_hw_register();
	map_i2c_dma_buffer();

    drDbgPrintLnf("test_touch+4\n");
}
