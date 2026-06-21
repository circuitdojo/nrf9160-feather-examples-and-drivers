#
# Copyright (c) 2024-2026 Circuit Dojo LLC
#
# SPDX-License-Identifier: Apache-2.0
#

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_MCUBOOT_SERIAL startup.c)
