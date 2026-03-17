#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fs.h>
#include <zephyr/random/random.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include <stdio.h>

LOG_MODULE_REGISTER(main);

#define PATH_HELLO "/lfs:/hello.txt"
#define PATH_RAND "/lfs:/random.txt"

#define LFS_PARTITION_NODE DT_NODELABEL(lfs1)
#define LFS_PARTITION_ID DT_FIXED_PARTITION_ID(DT_PHANDLE(LFS_PARTITION_NODE, partition))

void wipe_if_requested(void) {
    if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
        const struct flash_area *fa;
        int rc;

        rc = flash_area_open(LFS_PARTITION_ID, &fa);
        if (rc == 0) {
            LOG_INF("Wiping flash area for LittleFS...");
            flash_area_erase(fa, 0, fa->fa_size);
            flash_area_close(fa);
            LOG_INF("Wipe complete. System will re-format on next mount.");
        }
    }
}

int main(void)
{
    struct fs_file_t file;
    char buf[16];
    int ret;

    if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
        wipe_if_requested();
        LOG_INF("Rebooting system...");
        k_sleep(K_MSEC(500));
        sys_reboot(SYS_REBOOT_COLD);
    }

    fs_file_t_init(&file);

    if (fs_open(&file, PATH_HELLO, FS_O_CREATE | FS_O_WRITE) == 0) {
        fs_write(&file, "hello world", 11);
        fs_close(&file);
    }

    uint32_t r = sys_rand32_get();
    snprintf(buf, sizeof(buf), "%u", r);

    if (fs_open(&file, PATH_RAND, FS_O_CREATE | FS_O_WRITE) == 0) {
        fs_write(&file, buf, strlen(buf));
        fs_close(&file);
    }

    if (fs_open(&file, PATH_RAND, FS_O_READ) == 0) {
        ret = fs_read(&file, buf, sizeof(buf) - 1);
        if (ret > 0) {
            buf[ret] = '\0';
            LOG_INF("Read from %s: %s", PATH_RAND, buf);
        }
        fs_close(&file);
    }
}