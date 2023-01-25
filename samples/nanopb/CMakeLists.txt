# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nanopb_sample)

set(PROTOC_OPTIONS "-I${ZEPHYR_BASE}/samples/modules/nanopb/src/")
set(NANOPB_OPTIONS "-I${ZEPHYR_BASE}/samples/modules/nanopb/")

nanopb_generate_cpp(proto_sources proto_headers ${ZEPHYR_BASE}/samples/modules/nanopb/src/simple.proto)

# we need to be able to include generated header files
zephyr_library_include_directories(${CMAKE_CURRENT_BINARY_DIR})

FILE(GLOB app_sources ${ZEPHYR_BASE}/zephyr/samples/modules/nanopb/src*.c)
target_sources(app PRIVATE ${proto_sources} ${app_sources})
