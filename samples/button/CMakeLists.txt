# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)



find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(button)

target_sources(app PRIVATE ${ZEPHYR_BASE}/samples/basic/button/src/main.c)
