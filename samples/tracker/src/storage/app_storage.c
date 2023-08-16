/*
 * @copyright Circuit Dojo (c) 2022
 */

/* external storage initialization */
#include <stdio.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nor_storage);

/* Used to determine if FS is in good state */
#define NOR_STORAGE_ERASED_ON_BOOT "/lfs/erased"

/* Matches LFS_NAME_MAX */
#define MAX_PATH_LEN 255

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage,
    .storage_dev = (void *)FLASH_AREA_ID(littlefs_storage),
    .mnt_point = "/lfs",
};

static int nor_storage_erase(void)
{
    int err;
    struct fs_mount_t *mp = &lfs_storage_mnt;
    unsigned int id = (uintptr_t)mp->storage_dev;
    const struct flash_area *pfa;

    /* Unmount if mounted */
    fs_unmount(&lfs_storage_mnt);

    err = flash_area_open(id, &pfa);
    if (err < 0)
    {
        LOG_ERR("FAIL: unable to find flash area %u: %d",
                id, err);
        return err;
    }

    LOG_INF("Area %u at 0x%x for %u bytes",
            id, (unsigned int)pfa->fa_off,
            (unsigned int)pfa->fa_size);

    LOG_INF("Erasing flash area ... ");
    err = flash_area_erase(pfa, 0, pfa->fa_size);
    LOG_INF("Done. Code: %i", err);

    if (err)
    {
        LOG_ERR("Failed to erase storage. (err: %d)", err);
        return err;
    }

    flash_area_close(pfa);

    return 0;
}

int nor_storage_init(void)
{
    int err;
    struct fs_dirent dirent;
    struct fs_statvfs sbuf;

    /* Mount filesystem */
    err = fs_mount(&lfs_storage_mnt);
    if (err)
        return err;

    err = fs_statvfs(lfs_storage_mnt.mnt_point, &sbuf);
    if (err < 0)
        LOG_ERR("statvfs: %d", err);

    LOG_INF("bsize = %lu ; frsize = %lu ; blocks = %lu ; bfree = %lu",
            sbuf.f_bsize, sbuf.f_frsize,
            sbuf.f_blocks, sbuf.f_bfree);

    /* Make sure the flash has been erased */
    err = fs_stat(NOR_STORAGE_ERASED_ON_BOOT, &dirent);
    if (err)
    {
        LOG_INF("Erasing storage!");

        err = nor_storage_erase();
        if (err < 0)
            return err;

        /* Re-mount */
        err = fs_mount(&lfs_storage_mnt);
        if (err)
            return err;

        /* Create folder */
        err = fs_mkdir(NOR_STORAGE_ERASED_ON_BOOT);
        if (err)
            return err;
    }

    return 0;
}

static int nor_storage_init_fn(void)
{
    return nor_storage_init();
}

SYS_INIT(nor_storage_init_fn, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);