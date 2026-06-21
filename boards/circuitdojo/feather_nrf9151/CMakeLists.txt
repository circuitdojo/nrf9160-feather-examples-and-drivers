# Copyright (c) 2024-2026 Circuit Dojo
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_CIRCUITDOJO_FEATHER_NRF9151_PMIC_STARTUP startup.c)
