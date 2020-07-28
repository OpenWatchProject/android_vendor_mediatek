/*
 * Copyright (c) 2019 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*Extern api to call Hardware Definition*/
int imgsensor_query_SEC_I2C_BUS(void);
int imgsensor_query_SEC_CSI(void);

/*Implement Hardware Definition*/
enum I2C_BUS_NUMBER {
	I2C_BUS_0 = 0x0,
	I2C_BUS_1 = 0x1,
	I2C_BUS_2 = 0x2,
	I2C_BUS_3 = 0x3,
	I2C_BUS_4 = 0x4,
	I2C_BUS_5 = 0x5,
	I2C_BUS_6 = 0x6,

};

/*follow top_mux ctrl list*/
enum CSI_NUM {
	CSI0  = 0x0,//seninf1
	CSI0A = 0x0,//seninf1
	CSI0B = 0x1,//seninf2
	CSI1  = 0x2,//seninf3
	CSI2  = 0x4,//seninf5
};



