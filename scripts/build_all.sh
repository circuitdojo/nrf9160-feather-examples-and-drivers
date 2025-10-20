#!/bin/bash

# Targets
declare -a targets=(
    "circuitdojo_feather_nrf9160/nrf9160/ns"
    "circuitdojo_feather_nrf9151/nrf9151/ns"
)

# Applications
declare -a apps=(
    "accelerometer"
    "accelerometer_zbus"
    "active_sleep"
    "at_client"
    "battery"
    "blinky"
    "bme280"
    "button"
    "deep_sleep"
    "direct_i2c_access"
    "external_flash"
    "external_rtc"
    "external_rtc_time_sync"
    "gps"
    "led_pwm"
    "mfw_update"
    "sms"
    "https"
    "serial_lte_modem"
    "usb_detect"
)

# Combinations to ignore
declare -a ignore_combinations=(
    "circuitdojo_feather_nrf9160/nrf9160/ns:deep_sleep"
    "circuitdojo_feather_nrf9160/nrf9160/ns:usb_detect"
    "circuitdojo_feather_nrf9160/nrf9160/ns:serial_lte_modem"
    "circuitdojo_feather_nrf9151/nrf9151/ns:external_rtc"
    "circuitdojo_feather_nrf9151/nrf9151/ns:external_rtc_time_sync"
    "circuitdojo_feather_nrf9151/nrf9151/ns:led_pwm"
    "circuitdojo_feather_nrf9151/nrf9151/ns:mfw_update"
)

# Get root directory of this project
prj_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
prj_dir=$(CDPATH= cd -- "$(dirname -- "$prj_dir")" && pwd -P)

cd $prj_dir

# Get Git tags
git fetch --prune --tags
version=$(git describe --tags --long)

# Make output dir
mkdir -p .out

# Stop on error
set -e

if ! west list -f "{url}" | grep https://github.com/circuitdojo/pcf85063a; then
    echo 'Missing pcf85063a driver required for external_rtc samples; https://github.com/circuitdojo/pcf85063a';
    exit 1
fi

# Function to check if combination should be ignored
should_ignore() {
    local combination="$1:$2"

    echo Combination: $combination

    for ignore in "${ignore_combinations[@]}"; do
        if [[ "$ignore" == "$combination" ]]; then
            return 0
        fi
    done
    return 1
}

# For each target
for app in "${apps[@]}"
do
    # Echo version
    echo "Building sample ${app} (ver: ${version})"

    # Change directory
    cd samples/${app}

    # For each target
    for target in "${targets[@]}";
    do
        # Check if this combination should be ignored
        if should_ignore "$target" "$app"; then
            echo "Ignoring ${app} for ${target}"
            continue
        fi

        # Check if sample is already built
        if [ -f build/$target/merged.hex ]; then
            echo "Skipping ${app} for ${target}, already built"
            continue
        fi

        # Grab part of target before first /
        target_basic=$(echo $target | cut -d'/' -f 1)

        # Build the target. Continue for loop if error
        west build -b $target -d build/$target/ --sysbuild -- -DEXTRA_ZEPHYR_MODULES=$prj_dir

        # Copy the target files over
        mkdir -p $prj_dir/.out/${version}/${app}

        # If app_update.bin exists, copy it
        if [ -f build/$target/zephyr/app_update.bin ]; then
            cp build/$target/app_update.bin $prj_dir/.out/${version}/${app}/${app}_${target_basic}_${version}_update.bin
        fi

        # Copy hex
        cp build/$target/merged.hex $prj_dir/.out/${version}/${app}/${app}_${target_basic}_${version}_merged.hex
    done

    # Go back
    cd $prj_dir
done
