/*
 * Copyright (c) 2018 PHYTEC Messtechnik GmbH
 * Copyright (c) 2026 Yeison Suarez
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ssd1316, CONFIG_DISPLAY_LOG_LEVEL);

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>

#include "ssd1316_regs.h"

#define SSD1316_CLOCK_DIV_RATIO         0x0
#define SSD1316_CLOCK_FREQUENCY         0x8
#define SSD1316_PANEL_VCOM_DESEL_LEVEL  0x40

#ifndef SSD1316_ADDRESSING_MODE
#define SSD1316_ADDRESSING_MODE         (SSD1316_SET_MEM_ADDRESSING_HORIZ)
#endif

union ssd1316_bus {
    struct i2c_dt_spec i2c;
    struct spi_dt_spec spi;
};

typedef bool (*ssd1316_bus_ready_fn)(const struct device *dev);
typedef int (*ssd1316_write_bus_fn)(const struct device *dev, uint8_t *buf, size_t len, bool command);

struct ssd1316_config {
    union ssd1316_bus bus;
    struct gpio_dt_spec data_cmd;
    struct gpio_dt_spec reset;
    struct gpio_dt_spec supply;
    ssd1316_bus_ready_fn bus_ready;
    ssd1316_write_bus_fn write_bus;
    uint16_t height;
    uint16_t width;
    uint8_t segment_offset;
    uint8_t page_offset;
    uint8_t display_offset;
    uint8_t multiplex_ratio;
    uint8_t prechargep;
    bool segment_remap;
    bool com_invdir;
    bool com_sequential;
    bool color_inversion;
    int ready_time_ms;
};

struct ssd1316_data {
    enum display_pixel_format pf;
};

#if DT_HAS_COMPAT_ON_BUS_STATUS_OKAY(solomon_ssd1316fb, i2c)
static bool ssd1316_bus_ready_i2c(const struct device *dev) {
    const struct ssd1316_config *config = dev->config;
    return i2c_is_ready_dt(&config->bus.i2c);
}

static int ssd1316_write_bus_i2c(const struct device *dev, uint8_t *buf, size_t len, bool command) {
    const struct ssd1316_config *config = dev->config;
    return i2c_burst_write_dt(&config->bus.i2c,
                  command ? SSD1316_CONTROL_ALL_BYTES_CMD : SSD1316_CONTROL_ALL_BYTES_DATA,
                  buf, len);
}
#endif

#if DT_HAS_COMPAT_ON_BUS_STATUS_OKAY(solomon_ssd1316fb, spi)
static bool ssd1316_bus_ready_spi(const struct device *dev) {
    const struct ssd1316_config *config = dev->config;
    if (config->data_cmd.port && gpio_pin_configure_dt(&config->data_cmd, GPIO_OUTPUT_INACTIVE) < 0) return false;
    return spi_is_ready_dt(&config->bus.spi);
}

static int ssd1316_write_bus_spi(const struct device *dev, uint8_t *buf, size_t len, bool command) {
    const struct ssd1316_config *config = dev->config;
    gpio_pin_set_dt(&config->data_cmd, command ? 0 : 1);
    struct spi_buf tx_buf = {.buf = buf, .len = len};
    struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
    return spi_write_dt(&config->bus.spi, &tx_bufs);
}
#endif

static inline int ssd1316_write_bus(const struct device *dev, uint8_t *buf, size_t len, bool command) {
    const struct ssd1316_config *config = dev->config;
    return config->write_bus(dev, buf, len, command);
}

static int ssd1316_resume(const struct device *dev) {
    const struct ssd1316_config *config = dev->config;
    uint8_t cmd = SSD1316_DISPLAY_ON;
    if (config->supply.port) {
        gpio_pin_set_dt(&config->supply, 1);
        k_sleep(K_MSEC(SSD1316_SUPPLY_DELAY));
    }
    return ssd1316_write_bus(dev, &cmd, 1, true);
}

static int ssd1316_suspend(const struct device *dev) {
    const struct ssd1316_config *config = dev->config;
    uint8_t cmd = SSD1316_DISPLAY_OFF;
    if (config->supply.port) {
        gpio_pin_set_dt(&config->supply, 0);
        k_sleep(K_MSEC(SSD1316_SUPPLY_DELAY));
    }
    return ssd1316_write_bus(dev, &cmd, 1, true);
}

static int ssd1316_write(const struct device *dev, const uint16_t x, const uint16_t y,
                         const struct display_buffer_descriptor *desc, const void *buf) {
    const struct ssd1316_config *config = dev->config;
    
    uint16_t x_start = x + config->segment_offset;
    uint16_t x_end = x_start + desc->width - 1;

    uint8_t page_start = y / 8;
    uint8_t page_end = (y + desc->height - 1) / 8;

    if (page_end >= (config->height / 8)) {
        page_end = (config->height / 8) - 1;
    }
    if (x_end >= config->width) {
        x_end = config->width - 1;
    }

    uint8_t cmd[] = { 
        SSD1316_SET_COLUMN_ADDRESS, (uint8_t)x_start, (uint8_t)x_end,
        SSD1316_SET_PAGE_ADDRESS, page_start, page_end 
    };

    if (ssd1316_write_bus(dev, cmd, sizeof(cmd), true)) return -EIO;
    
    return ssd1316_write_bus(dev, (uint8_t *)buf, desc->buf_size, false);
}

static int ssd1316_set_pixel_format(const struct device *dev, const enum display_pixel_format pf) {
    struct ssd1316_data *data = dev->data;
    uint8_t cmd;

    if (pf == data->pf) return 0;

    if (pf == PIXEL_FORMAT_MONO10) {
        cmd = SSD1316_SET_REVERSE_DISPLAY;
    } else if (pf == PIXEL_FORMAT_MONO01) {
        cmd = SSD1316_SET_NORMAL_DISPLAY;
    } else {
        return -ENOTSUP;
    }

    if (ssd1316_write_bus(dev, &cmd, 1, true)) return -EIO;
    data->pf = pf;
    return 0;
}

static void ssd1316_get_capabilities(const struct device *dev, struct display_capabilities *caps) {
    const struct ssd1316_config *config = dev->config;
    struct ssd1316_data *data = dev->data;

    caps->x_resolution = config->width;
    caps->y_resolution = config->height;
    caps->supported_pixel_formats = PIXEL_FORMAT_MONO10 | PIXEL_FORMAT_MONO01;
    caps->current_pixel_format = data->pf;
    caps->screen_info = SCREEN_INFO_MONO_VTILED;
    caps->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int ssd1316_init_device(const struct device *dev) {
    const struct ssd1316_config *config = dev->config;
    struct ssd1316_data *data = dev->data;

    if (config->reset.port) {
        gpio_pin_set_dt(&config->reset, 1); k_sleep(K_MSEC(SSD1316_RESET_DELAY));
        gpio_pin_set_dt(&config->reset, 0); k_sleep(K_MSEC(SSD1316_RESET_DELAY));
        gpio_pin_set_dt(&config->reset, 1);
    }

    data->pf = config->color_inversion ? PIXEL_FORMAT_MONO10 : PIXEL_FORMAT_MONO01;

    uint8_t init_cmds[] = {
        SSD1316_DISPLAY_OFF,
        SSD1316_SET_CLOCK_DIV_RATIO, 0x80,
        
        SSD1316_SET_MULTIPLEX_RATIO, config->multiplex_ratio,
        SSD1316_SET_DISPLAY_OFFSET, config->display_offset,
        
        SSD1316_SET_START_LINE | 0x00,
        SSD1316_SET_CHARGE_PUMP_ON, SSD1316_SET_CHARGE_PUMP_ENABLED,
        SSD1316_SET_MEM_ADDRESSING_MODE, SSD1316_SET_MEM_ADDRESSING_HORIZ,
        
        (config->segment_remap ? SSD1316_SET_SEGMENT_MAP_REMAPED : SSD1316_SET_SEGMENT_MAP_NORMAL),
        (config->com_invdir ? SSD1316_SET_COM_OUTPUT_SCAN_FLIPPED : SSD1316_SET_COM_OUTPUT_SCAN_NORMAL),
        
        SSD1316_SET_PADS_HW_CONFIG, 
        (config->com_sequential ? SSD1316_SET_PADS_HW_SEQUENTIAL : SSD1316_SET_PADS_HW_ALTERNATIVE),
        
        SSD1316_SET_CONTRAST_CTRL, 0x7F,
        SSD1316_SET_CHARGE_PERIOD, config->prechargep,
        SSD1316_SET_VCOM_DESELECT_LEVEL, 0x40,
        SSD1316_SET_ENTIRE_DISPLAY_OFF,
        (config->color_inversion ? SSD1316_SET_REVERSE_DISPLAY : SSD1316_SET_NORMAL_DISPLAY),
    };

    if (ssd1316_write_bus(dev, init_cmds, sizeof(init_cmds), true)) return -EIO;
    return ssd1316_resume(dev);
}

static int ssd1316_init(const struct device *dev) {
    const struct ssd1316_config *config = dev->config;
    if (config->supply.port) gpio_pin_configure_dt(&config->supply, GPIO_OUTPUT_INACTIVE);
    if (config->reset.port) gpio_pin_configure_dt(&config->reset, GPIO_OUTPUT_INACTIVE);
    return ssd1316_init_device(dev);
}

static DEVICE_API(display, ssd1316_driver_api) = {
    .blanking_on = ssd1316_suspend,
    .blanking_off = ssd1316_resume,
    .write = ssd1316_write,
    .get_capabilities = ssd1316_get_capabilities,
    .set_pixel_format = ssd1316_set_pixel_format,
};

#define SSD1316_DEFINE(node_id) \
    static struct ssd1316_data data##node_id; \
    static const struct ssd1316_config config##node_id = { \
        .reset = GPIO_DT_SPEC_GET_OR(node_id, reset_gpios, {0}), \
        .supply = GPIO_DT_SPEC_GET_OR(node_id, supply_gpios, {0}), \
        .height = DT_PROP(node_id, height), .width = DT_PROP(node_id, width), \
        .display_offset = DT_PROP(node_id, display_offset), \
        .multiplex_ratio = DT_PROP(node_id, multiplex_ratio), \
        .segment_remap = DT_PROP(node_id, segment_remap), \
        .com_invdir = DT_PROP(node_id, com_invdir), \
        .com_sequential = DT_PROP(node_id, com_sequential), \
        .prechargep = DT_PROP(node_id, prechargep), \
        .color_inversion = DT_PROP(node_id, inversion_on), \
        COND_CODE_1(DT_ON_BUS(node_id, spi), \
            (.bus = {.spi = SPI_DT_SPEC_GET(node_id, SPI_OP_MODE_MASTER | SPI_WORD_SET(8))}, .bus_ready = ssd1316_bus_ready_spi, .write_bus = ssd1316_write_bus_spi), \
            (.bus = {.i2c = I2C_DT_SPEC_GET(node_id)}, .bus_ready = ssd1316_bus_ready_i2c, .write_bus = ssd1316_write_bus_i2c)) \
    }; \
    DEVICE_DT_DEFINE(node_id, ssd1316_init, NULL, &data##node_id, &config##node_id, POST_KERNEL, CONFIG_DISPLAY_INIT_PRIORITY, &ssd1316_driver_api);

DT_FOREACH_STATUS_OKAY(solomon_ssd1316fb, SSD1316_DEFINE)