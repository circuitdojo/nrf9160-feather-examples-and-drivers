/*
 * Copyright Circuit Dojo (c) 2021
 *
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

/* external storage initialization */
#if defined(CONFIG_FILE_SYSTEM_LITTLEFS)
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_storage);

#if defined(CONFIG_FILE_SYSTEM_LITTLEFS)

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage,
    .storage_dev = (void *)FLASH_AREA_ID(external_flash),
    .mnt_point = "/lfs",
};
#endif

static int app_storage_init(const struct device *dev)
{
    struct fs_mount_t *mp = &lfs_storage_mnt;
    unsigned int id = (uintptr_t)mp->storage_dev;
    const struct flash_area *pfa;
    struct fs_statvfs sbuf;

    int rc;

    rc = flash_area_open(id, &pfa);
    if (rc < 0)
    {
        LOG_ERR("FAIL: unable to find flash area %u: %d",
                id, rc);
        return rc;
    }

    LOG_INF("Area %u at 0x%x on littlefs volume for %u bytes",
            id, (unsigned int)pfa->fa_off,
            (unsigned int)pfa->fa_size);

    /* Optional wipe flash contents */
    if (IS_ENABLED(CONFIG_APP_STORAGE_WIPE))
    {
        LOG_INF("Erasing flash area ... ");
        rc = flash_area_erase(pfa, 0, pfa->fa_size);
        LOG_INF("%d", rc);
    }

    flash_area_close(pfa);

    rc = fs_mount(mp);
    if (rc < 0)
    {
        LOG_ERR("FAIL: mount id %u at %s: %d",
                (unsigned int)mp->storage_dev, mp->mnt_point,
                rc);
        return rc;
    }
    LOG_INF("%s mount: %d", mp->mnt_point, rc);

    rc = fs_statvfs(mp->mnt_point, &sbuf);
    if (rc < 0)
    {
        LOG_ERR("FAIL: statvfs: %d", rc);
        return rc;
    }

    LOG_INF("%s: bsize = %lu ; frsize = %lu ;"
            " blocks = %lu ; bfree = %lu",
            mp->mnt_point,
            sbuf.f_bsize, sbuf.f_frsize,
            sbuf.f_blocks, sbuf.f_bfree);

    return 0;
}

SYS_INIT(app_storage_init, APPLICATION,
         CONFIG_APP_STORAGE_INIT_PRIORITY);

#endif