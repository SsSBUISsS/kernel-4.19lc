/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 2010 - 2014 Goodix Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Version: 1.0
 * Revision Record:
 *      V1.0:  first release. 2014/09/28.
 *
 */
#ifndef _GT9XX_CONFIG_H_
#define _GT9XX_CONFIG_H_

#define GTP_CFG_GROUP0 {\
0x41, 0x20, 0x03, 0x00, 0x05, 0x05, 0xF5, 0x40,\
0x01, 0x0A, 0x1E, 0x09, 0x50, 0x32, 0x0F, 0x05,\
0x00, 0x00, 0x00, 0x00, 0x53, 0x01, 0x00, 0x14,\
0x18, 0x22, 0x14, 0x8D, 0x2D, 0xAA, 0x26, 0x28,\
0xA4, 0x0B, 0x00, 0x00, 0x00, 0x02, 0x03, 0x1D,\
0x1E, 0x01, 0x00, 0x55, 0x00, 0x14, 0x00, 0x00,\
0x00, 0x01, 0x0A, 0x23, 0x55, 0x94, 0xC5, 0x02,\
0x08, 0x00, 0x00, 0x04, 0x97, 0x26, 0x00, 0x88,\
0x2D, 0x00, 0x7A, 0x36, 0x00, 0x6F, 0x41, 0x00,\
0x66, 0x4E, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,\
0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,\
0x10, 0x11, 0x12, 0x13, 0xFF, 0xFF, 0xFF, 0xFF,\
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1F,\
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,\
0x28, 0x29, 0x2A, 0x00, 0x01, 0x02, 0x03, 0x04,\
0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0xEC, 0x01\
}

#define GTP_CFG_GROUP0_CHARGER {}

#define GTP_CFG_GROUP1 {}

#define GTP_CFG_GROUP1_CHARGER {}

#define GTP_CFG_GROUP2 {}

#define GTP_CFG_GROUP2_CHARGER {}

#define GTP_CFG_GROUP3 {}

#define GTP_CFG_GROUP3_CHARGER {}

#define GTP_CFG_GROUP4 {}

#define GTP_CFG_GROUP5 {}

#define GTP_CFG_GROUP6 {}

#define TPD_CALIBRATION_MATRIX {-4096, 0, 3276800, 0, -4096, 5242880, 0, 0}
#endif
