#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>

LV_FONT_DECLARE(droid_sans_mono_8);

int main(void)
{
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) return 0;

    lv_obj_t *label = lv_label_create(lv_scr_act());
    // lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_font(label, &droid_sans_mono_8, 0);
    lv_label_set_text(label, "Yeisonint");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *debug = lv_label_create(lv_scr_act());
    // lv_obj_set_style_text_font(debug, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_font(debug, &droid_sans_mono_8, 0);
    lv_obj_align(debug, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    uint32_t uptime = 0;
    
    int64_t next_update = k_uptime_get() + 1000;
    int64_t now = k_uptime_get();

    while (1) {
        lv_timer_handler();

        now = k_uptime_get();
        if (now >= next_update) {
            uptime++;
            next_update = now + 1000;
            lv_label_set_text_fmt(debug, "Uptime: %u s", uptime);
			printk("Uptime: %u s\n", uptime);

        }

        k_msleep(50); 
    }
}