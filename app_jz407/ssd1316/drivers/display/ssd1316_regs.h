/*
 * Copyright (c) 2018 Phytec Messtechnik GmbH
 * Copyright (c) 2026 Yeison Suarez
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SSD1316_REGS_H__
#define __SSD1316_REGS_H__

/* Control bytes */
#define SSD1316_CONTROL_ALL_BYTES_CMD       0x00
#define SSD1316_CONTROL_ALL_BYTES_DATA      0x40
#define SSD1316_CONTROL_BYTE_CMD            0x80
#define SSD1316_CONTROL_BYTE_DATA           0xc0

/* Status Mask */
#define SSD1316_READ_STATUS_MASK            0xc0
#define SSD1316_READ_STATUS_BUSY            0x80
#define SSD1316_READ_STATUS_ON              0x40

/* Fundamental Command Table */
#define SSD1316_SET_CONTRAST_CTRL           0x81 /* Double byte */
#define SSD1316_SET_ENTIRE_DISPLAY_OFF      0xa4
#define SSD1316_SET_ENTIRE_DISPLAY_ON       0xa5
#define SSD1316_SET_NORMAL_DISPLAY          0xa6
#define SSD1316_SET_REVERSE_DISPLAY         0xa7
#define SSD1316_DISPLAY_OFF                 0xae
#define SSD1316_DISPLAY_ON                  0xaf

/* Addressing Setting Command Table */
#define SSD1316_SET_LOWER_COL_ADDRESS       0x00
#define SSD1316_SET_LOWER_COL_ADDRESS_MASK  0x0f
#define SSD1316_SET_HIGHER_COL_ADDRESS      0x10
#define SSD1316_SET_HIGHER_COL_ADDRESS_MASK 0x0f

#define SSD1316_SET_MEM_ADDRESSING_MODE     0x20 /* Double byte */
#define SSD1316_SET_MEM_ADDRESSING_HORIZ    0x00
#define SSD1316_SET_MEM_ADDRESSING_VERT     0x01
#define SSD1316_SET_MEM_ADDRESSING_PAGE     0x02

#define SSD1316_SET_COLUMN_ADDRESS          0x21 /* Triple byte */
#define SSD1316_SET_PAGE_ADDRESS            0x22 /* Triple byte */

#define SSD1316_SET_PAGE_START_ADDRESS      0xb0
#define SSD1316_SET_PAGE_START_ADDRESS_MASK 0x07

/* Hardware Configuration Command Table */
#define SSD1316_SET_START_LINE              0x40
#define SSD1316_SET_START_LINE_MASK         0x3f
#define SSD1316_SET_SEGMENT_MAP_NORMAL      0xa0
#define SSD1316_SET_SEGMENT_MAP_REMAPED     0xa1
#define SSD1316_SET_MULTIPLEX_RATIO         0xa8 /* Double byte */
#define SSD1316_SET_COM_OUTPUT_SCAN_NORMAL  0xc0
#define SSD1316_SET_COM_OUTPUT_SCAN_FLIPPED 0xc8
#define SSD1316_SET_DISPLAY_OFFSET          0xd3 /* Double byte */
#define SSD1316_SET_PADS_HW_CONFIG          0xda /* Double byte */
#define SSD1316_SET_PADS_HW_SEQUENTIAL      0x02
#define SSD1316_SET_PADS_HW_ALTERNATIVE     0x12

/* Timing and Driving Scheme Setting Command Table */
#define SSD1316_SET_CLOCK_DIV_RATIO         0xd5 /* Double byte */
#define SSD1316_SET_CHARGE_PERIOD           0xd9 /* Double byte */
#define SSD1316_SET_VCOM_DESELECT_LEVEL     0xdb /* Double byte */
#define SSD1316_NOP                         0xe3

/* Charge Pump Command Table */
#define SSD1316_SET_CHARGE_PUMP_ON          0x8d /* Double byte */
#define SSD1316_SET_CHARGE_PUMP_DISABLED    0x10
#define SSD1316_SET_CHARGE_PUMP_ENABLED     0x14

/* Time constants in ms */
#define SSD1316_RESET_DELAY                 1
#define SSD1316_SUPPLY_DELAY                100 

#endif /* __SSD1316_REGS_H__ */