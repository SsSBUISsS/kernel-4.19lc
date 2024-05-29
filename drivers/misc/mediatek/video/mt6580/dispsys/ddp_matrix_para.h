// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 MediaTek Inc.
 *
 */

#ifndef _DDP_MATRIX_PARA_H_
#define _DDP_MATRIX_PARA_H_


#define TABLE_NO 10

#define YUV2RGB_601_16_16  0
#define YUV2RGB_601_16_0   1
#define YUV2RGB_601_0_0    2
#define YUV2RGB_709_16_16  3
#define YUV2RGB_709_16_0   4
#define YUV2RGB_709_0_0    5
#define RGB2YUV_601        6
#define RGB2YUV_601_XVYCC  7
#define RGB2YUV_709        8
#define RGB2YUV_709_XVYCC  9

static const short int coef_rdma_601_r2y[5][3] = {
	{263, 516, 100},
	{-152, -298, 450},
	{450, -377, -73},
	{0, 0, 0},
	{0, 128, 128}
};

static const short int coef_rdma_709_r2y[5][3] = {
	{187, 629, 63},
	{-103, -347, 450},
	{450, -409, -41},
	{0, 0, 0},
	{16, 128, 128}
};

static const short int coef_rdma_601_y2r[5][3] = {
	{1193, 0, 1633},
	{1193, -400, -832},
	{1193, 2065, 0},
	{-16, -128, -128},
	{0, 0, 0}
};

static const short int coef_rdma_709_y2r[5][3] = {
	{1193, 0, 1934},
	{1193, -217, -545},
	{1193, 2163, -1},
	{-16, -128, -128},
	{0, 0, 0}
};


static const short int coef[10][5][3] =
    /* ----------------------------------------- */
    /* YUV to RGB SD */
    /* ----------------------------------------- */
    /* yuv2rgb_601_16_16, y=16~235, uv=16~240, rgb=16~235 */
{{	{0x0400, 0x0000, 0x057c},	/* 1,  0    ,  1.371    13bit: sign+2.10 */
	{0x0400, 0x1ea8, 0x1d35},	/* 1, -0.336, -0.698    13bit: sign+2.10 */
	{0x0400, 0x06ee, 0x0000},	/* 1,  1.732,  0        13bit: sign+2.10 */
	{0, 0x180, 0x180},		/* 0, -128  , -128       9bit: sign+8.0 */
	{0, 0, 0} },
  /* yuv2rgb_601_16_0, y=16~235, uv=16~240, rgb=0~255 */
{	{0x04a7, 0x0000, 0x0662},	/* 1.164,  0     ,  1.596   13bit: sign+2.10 */
	{0x04a7, 0x1e70, 0x1cc0},	/* 1.164, -0.391 , -0.813   13bit: sign+2.10 */
	{0x04a7, 0x0812, 0x0000},	/* 1.164,  2.018 ,  0       13bit: sign+2.10 */
	{0x1f0, 0x180, 0x180},		/* -16, -128   , -128      9bit: sign+8.0 */
	{0, 0, 0} },
  /* yuv2rgb_601_0_0, yuv=0~255, rgb=0~255 */
{	{0x0400, 0x0000, 0x059b},	/* 1,  0     ,  1.402   13bit: sign+2.10 */
	{0x0400, 0x1ea0, 0x1d25},	/* 1, -0.3341, -0.7141  13bit: sign+2.10 */
	{0x0400, 0x0716, 0x0000},	/* 1,  1.772 ,  0       13bit: sign+2.10 */
	{0, 0x180, 0x180},		/* 0, -128  , -128       9bit: sign+8.0 */
	{0, 0, 0} },
  /* ----------------------------------------- */
  /* YUV to RGB HD */
  /* ----------------------------------------- */
  /* yuv2rgb_709_16_16, y=16~235, uv=16~240, rgb=16~235 */
{	{0x0400, 0x0000, 0x0628},	/* 1,  0    ,  1.54      13bit: sign+2.10 */
	{0x0400, 0x1f45, 0x1e2a},	/* 1, -0.183, -0.459     13bit: sign+2.10 */
	{0x0400, 0x0743, 0x0000},	/* 1,  1.816,  0         13bit: sign+2.10 */
	{0, 0x180, 0x180},		/* 0, -128  , -128        9bit: sign+8.0 */
	{0, 0, 0} },
  /* yuv2rgb_709_16_0, y=16~235, uv=16~240, rgb=0~255 */
{	{0x04a7, 0x0000, 0x072c},	/* 1.164,  0     ,  1.793    13bit: sign+2.10 */
	{0x04a7, 0x1f26, 0x1dde},	/* 1.164, -0.213 , -0.534    13bit: sign+2.10 */
	{0x04a7, 0x0875, 0x0000},	/* 1.164,  2.115 ,  0        13bit: sign+2.10 */
	{0x1f0, 0x180, 0x180},		/* -16, -128   , -128      9bit: sign+8.0 */
	{0, 0, 0} },
  /* yuv2rgb_709_0_0, yuv=0~255, rgb=0~255 */
{	{0x0400, 0x0000, 0x064d},	/* 1,  0     ,  1.5748   13bit: sign+2.10 */
	{0x0400, 0x1f40, 0x1e21},	/* 1, -0.1873, -0.4681   13bit: sign+2.10 */
	{0x0400, 0x076c, 0x0000},	/* 1,  1.8556,  0        13bit: sign+2.10 */
	{0, 0x180, 0x180},		/* 0, -128   , -128       9bit: sign+8.0 */
	{0, 0, 0} },
  /* ----------------------------------------- */
  /* RGB to YUV SD */
  /* ----------------------------------------- */
  /* rgb2yuv_601, rgb=0~255, y=16~235, uv=16~240 */
{	{0x0107, 0x0204, 0x0064},	/* 0.257 ,  0.504 ,  0.098  13bit: sign+2.10 */
	{0x1f68, 0x1ed6, 0x01c2},	/* -0.148 , -0.291 ,  0.439  13bit: sign+2.10 */
	{0x01c2, 0x1e87, 0x1fb7},	/* 0.439 , -0.368 , -0.071  13bit: sign+2.10 */
	{0, 0, 0},
	{0x010, 0x080, 0x080} },	/* 16 ,    128 ,    128   9bit: sign+8.0 */
  /* rgb2yuv_601_xvycc, rgb=0~255, yuv=0~255 */
{	{0x0132, 0x0259, 0x0075},	/* 0.299 ,  0.587 ,  0.114   13bit: sign+2.10 */
	{0x1f53, 0x1ead, 0x0200},	/* -0.1687, -0.3313,  0.5     13bit: sign+2.10 */
	{0x0200, 0x1e53, 0x1fad},	/* 0.5   , -0.4187, -0.0813  13bit: sign+2.10 */
	{0, 0, 0},
	{0x010, 0x080, 0x080} },	/* 16 ,    128 ,    128   9bit: sign+8.0 */
  /* ----------------------------------------- */
  /* RGB to YUV SD */
  /* ----------------------------------------- */
  /* rgb2yuv_709, rgb=0~255, y=16~235, uv=16~240 */
{	{0x00bb, 0x0275, 0x003f},	/* 0.183 ,  0.614 ,  0.062  13bit: sign+2.10 */
	{0x1f98, 0x1ea6, 0x01c2},	/* -0.101 , -0.338 ,  0.439  13bit: sign+2.10 */
	{0x01c2, 0x1e67, 0x1fd7},	/* 0.439 , -0.399 , -0.04   13bit: sign+2.10 */
	{0, 0, 0},
	{0x010, 0x080, 0x080} },	/* 16 ,    128 ,    128   9bit: sign+8.0 */
  /* rgb2yuv_709_xvycc, rgb=0~255, yuv=0~255 */
{	{0x00da, 0x02dc, 0x004a},	/* 0.2126,  0.7152,  0.0722  13bit: sign+2.10 */
	{0x1f8b, 0x1e75, 0x0200},	/* -0.1146, -0.3854,  0.5     13bit: sign+2.10 */
	{0x0200, 0x1e2f, 0x1fd1},	/* 0.5   , -0.4542, -0.0458  13bit: sign+2.10 */
	{0, 0, 0},
	{0x010, 0x080, 0x080} }		/* 16 ,    128 ,    128   9bit: sign+8.0 */
};

#endif
