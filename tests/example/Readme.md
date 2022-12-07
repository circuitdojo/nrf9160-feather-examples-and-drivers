# Test example

To run this test you'll need to invoke `twister`. Here are the steps:

1. First, make sure your python environment is set

    ```
    source ~/.zephyrtools/env/bin/activate.fish
    ```

2. Make sure that  `ZEPHYR_TOOLCHAIN_VARIANT` and `ZEPHYR_SDK_INSTALL_DIR` are set. (if not already)

    ```
    export ZEPHYR_SDK_INSTALL_DIR=~/.zephyrtools/toolchain/zephyr-sdk-0.15.1/
    export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
    ```

3. Next, you can run `twister`. There are two ways to do it:
    a. Within the `nfed` directory run the following to do a compile time check:

    ```
    ../zephyr/scripts/twister -W -p circuitdojo_feather_nrf9160_ns -T tests/
    ```

    This will do a *compile time only* check. To fully test on hardware jump to option b

    b. To run directly on hardware, run like the following:

    ```
    ../zephyr/scripts/twister -W --device-testing --device-serial /dev/cu.SLAB_USBtoUART --west-runner=nrfjprog --west-flash="--erase,--softreset" -p circuitdojo_feather_nrf9160_ns -T tests/
    ```

    **Note:** minimally you shouldn't need the `--west-runner` and `--west-flash` options. These are required for any nRF9160 Feather project though!