#!/bin/bash

# Targets
declare -a targets=("circuitdojo_feather_nrf9160_ns")

# Applications
declare -a apps=("accelerometer" "active_sleep" "at_client" "battery" "blinky" "bme280" "button" "deep_sleep" "external_flash" "external_rtc" "external_rtc_time_sync" "gps" "led_pwm" "mfw_update" "nanopb" "sms")

# Get Git tags
git fetch --prune --tags
version=$(git describe --tags --long)

# Make output dir
mkdir -p .out

# For each target
for app in "${apps[@]}"
do

# For each target
for target in "${targets[@]}"
do

# Echo version
echo "Building ${app} (ver: ${version}) for ${target}".

# Build the target
west build -b $target -s samples/${app} -p

# Copy the target files over
mkdir -p .out/${version}/${app}
cp build/zephyr/app_update.bin .out/${version}/${app}/${app}_${target}_${version}_update.bin
cp build/zephyr/merged.hex .out/${version}/${app}/${app}_${target}_${version}_merged.hex

done
done