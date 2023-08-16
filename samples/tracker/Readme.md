# nRF9160 Feather - Tracker Demo

In this sample, the nRF9160 Feather collects some GPS data and pushes to your
cloud interface of choice. Cloud backends incude:

- Golioth
- Pyrinas

This example is also configurable via the Golioth console or via `goliothctl`
(Golioths CLI). For more information check out the
[blog post here.](https://www.jaredwolff.com/nrf9160-feather-on-golioth/)

## Setup

1. Install the VSCode Extension and SDK.

   Here are the links for each operating system:

   - [Mac OS](https://docs.jaredwolff.com/nrf9160-sdk-setup-mac.html)
   - [Windows](https://docs.jaredwolff.com/nrf9160-sdk-setup-windows.html)
   - [Linux](https://docs.jaredwolff.com/nrf9160-sdk-setup-linux.html)

2. Initialize your repository using this Git url:
   `https://github.com/circuitdojo/nrf9160-feather-examples-and-drivers.git` and
   the **Zephyr Tools: Init Repo** command

   ![Init repo](../../img/init-repo.png)

   **Note:** It's best to select an empty folder to initialize the project to.

3. Sign up to download Nordic's SUPL library (optional and disabled by default)
   [here](https://response.nordicsemi.com/suplc-license-signup). Then, install
   using the instructions provided with the download package. (Files downloaded
   should be placed in the `nrf/ext` folder.)

## Building and flashing the code

Using the command window run **Zephyr Tools: Build**

![Build](../../img/build.png)

If prompted select the project `path/to/your/project/nfed/samples/tracker` and
type `circuitojo_feather_nrf9160_ns` as the target.

![Select project](../../img/select-project.png)

![Select target](../../img/select-target.png)

Then place the device into bootloader mode (only for v4 and older):

1. Hold the MODE button
2. Then tap the RST button while holding mode
3. **Hold the MODE button until the Blue LED illuminates**

Then, load using the **Zephyr Tools: Load via Bootloader** task.

![Option for loading](../../img/load-via-newtmgr.png)

Pay attention to the progress in the bottom console.

![Option for loading](../../img/load-via-newtmgr-progress.png)

Once complete, the extension will reset your device and should start executing!

## Update credentials in modem (one time only)

Using the built-in shell run the following

```
at AT+CFUN=4
at raw <CHAR COUNT>
AT%CMNG=0,515765868,4,"<DEVICE ID>@<LOCATION>"
at raw <CHAR COUNT>
AT%CMNG=0,515765868,3,"<FORMATTED PSK>"
at AT+CFUN=1
```

**Important note:** your PSK must be in hex string format. You can use a site
[like this one](https://www.rapidtables.com/convert/number/ascii-to-hex.html) to
convert it first. Make sure you select **Output delimeter** as **None**.

Or you can set them using the `at_client` sample like so:

![Setting credentials](../../img/set-credentials.png)

Make sure you enter the device IMEI and hex string PSK _exactly_ otherwise your
device will fail to authenticate with Golioth!

## Viewing console output

You can then view the output using the **Zephyr Tools: Serial Monitor** command.
You can also run **Zephyr Tools: Load via Booloader and Monitor** to both at the
same time.

![Serial monitor](../../img/serial-monitor.png)

Make sure you select the port that corresponds to your device. On Mac the serial
port will be `/dev/tty.SLAB_USBtoUART`

![Serial select](../../img/serial-select.png)
