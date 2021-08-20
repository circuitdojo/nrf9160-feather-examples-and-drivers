#
# Copyright (c) 2021 Circuit Dojo LLC
#

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_BATTERY_MEASUREMENT battery.c)