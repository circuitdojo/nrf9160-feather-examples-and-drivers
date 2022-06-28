/*
 * Copyright (c) 2020 Circuit Dojo, LLC
 * Copyright (c) 2019 Peter Bigot Consulting, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Sample which uses the filesystem API with littlefs */

#include <stdio.h>

#include <zephyr.h>
#include <device.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <storage/flash_map.h>
#include <settings/settings.h>

/* Matches LFS_NAME_MAX */
#define MAX_PATH_LEN 255

/* Config */
struct system_config
{
	int32_t update_interval;
	int32_t status;
};

static struct system_config cfg = {
	.update_interval = 0,
	.status = 0,
};

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)FLASH_AREA_ID(littlefs_storage),
	.mnt_point = "/lfs",
};

static int test_settings_set(const char *name, size_t len,
							 settings_read_cb read_cb, void *cb_arg)
{
	const char *next;
	int rc;

	if (settings_name_steq(name, "entry", &next) && !next)
	{
		if (len != sizeof(cfg))
		{
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &cfg, sizeof(cfg));
		if (rc >= 0)
		{
			/* key-value pair was properly read.
			 * rc contains value length.
			 */
			return 0;
		}
		/* read-out error */
		return rc;
	}

	return -ENOENT;
}

struct settings_handler my_config = {
	.name = "config",
	.h_set = test_settings_set,
};

void main(void)
{
	struct fs_mount_t *mp = &lfs_storage_mnt;
	unsigned int id = (uintptr_t)mp->storage_dev;
	char fname[MAX_PATH_LEN];
	struct fs_statvfs sbuf;
	const struct flash_area *pfa;
	int rc;

	snprintf(fname, sizeof(fname), "%s/boot_count", mp->mnt_point);

	rc = flash_area_open(id, &pfa);
	if (rc < 0)
	{
		printk("FAIL: unable to find flash area %u: %d\n",
			   id, rc);
		return;
	}

	printk("Area %u at 0x%x on %s for %u bytes\n",
		   id, (unsigned int)pfa->fa_off, pfa->fa_dev_name,
		   (unsigned int)pfa->fa_size);

	/* Optional wipe flash contents */
	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE))
	{
		printk("Erasing flash area ... ");
		rc = flash_area_erase(pfa, 0, pfa->fa_size);
		printk("%d\n", rc);
	}

	flash_area_close(pfa);

	rc = fs_mount(mp);
	if (rc < 0)
	{
		printk("FAIL: mount id %u at %s: %d\n",
			   (unsigned int)mp->storage_dev, mp->mnt_point,
			   rc);
		return;
	}
	printk("%s mount: %d\n", mp->mnt_point, rc);

	rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc < 0)
	{
		printk("FAIL: statvfs: %d\n", rc);
		goto out;
	}

	printk("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   mp->mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

	struct fs_dirent dirent;

	rc = fs_stat(fname, &dirent);
	printk("%s stat: %d\n", fname, rc);
	if (rc >= 0)
	{
		printk("\tfn '%s' siz %u\n", dirent.name, dirent.size);
	}

	struct fs_file_t file;

	fs_file_t_init(&file);
	rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
	if (rc < 0)
	{
		printk("FAIL: open %s: %d\n", fname, rc);
		goto out;
	}

	uint32_t boot_count = 0;

	if (rc >= 0)
	{
		rc = fs_read(&file, &boot_count, sizeof(boot_count));
		printk("%s read count %u: %d\n", fname, boot_count, rc);
		rc = fs_seek(&file, 0, FS_SEEK_SET);
		printk("%s seek start: %d\n", fname, rc);
	}

	boot_count += 1;
	rc = fs_write(&file, &boot_count, sizeof(boot_count));
	printk("%s write new boot count %u: %d\n", fname,
		   boot_count, rc);

	rc = fs_close(&file);
	printk("%s close: %d\n", fname, rc);

	/* Using settings subsystem */
	settings_subsys_init();
	settings_register(&my_config);
	settings_load();

	printk("Settings: before: status %i, interval: %i\n", cfg.status, cfg.update_interval);

	cfg.status += 1;
	cfg.update_interval = 20;

	printk("Settings: after: status %i, interval: %i\n", cfg.status, cfg.update_interval);

	settings_save_one("config/entry", &cfg, sizeof(cfg));

	/* End of setting related items */

out:
	rc = fs_unmount(mp);
	printk("%s unmount: %d\n", mp->mnt_point, rc);
}