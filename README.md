# nRF9160 and nRF9151 Feather Examples and Drivers

Zephyr examples and drivers for the [nRF9160 Feather](https://github.com/circuitdojo/nrf9160-feather) and [nRF9151 Feather](https://github.com/circuitdojo/nrf9151-feather).

## Setup

For the most up to date instructions, please see:

- [nRF9160 Feather](https://docs.jaredwolff.com/nrf9160-getting-started.html)
- [nRF9151 Feather](https://docs.circuitdojo.com/nrf9151-feather/using-vscode.html)

## Purchase

You can purchase the nRF9160 Feather and nRF9151 Feather direct from [Circuit Dojo](https://circuitdojo.com/) and from Digi-Key ([nRF9160](https://www.digikey.com/en/products/detail/circuit-dojo/PASSY-NRF9160-FEATHER/13668137)) ([nRF9151](https://www.digikey.com/en/products/detail/circuit-dojo/PASSY-NRF9151-FEATHER/26580618)).

## Samples

All samples can be found in this respository except for the following external samples:

- [Serial Modem](https://github.com/circuitdojo/ncs-serial-modem) - used for controlling the nRF9151 via UART from another application processor.
- [Asset Tracker Template](https://github.com/circuitdojo/asset-tracker-template) - template for common use case of asset tracking provided by Nordic.

## Contributing

All contributions must include a "Signed-off-by" line in the commit message to comply with the [Developer Certificate of Origin (DCO)](DCO.txt). This certifies that you have the right to submit the contribution under the project's open source license.

To automatically add the sign-off:

```bash
git commit -s
```

Ensure your Git configuration has your real name and email:

```bash
git config user.name "Your Full Name"
git config user.email "your.email@example.com"
```

The email must match the one used in your commits. Pull requests with unsigned commits will be rejected.

## License

Apache 2.0 applies to all samples. Some samples are based off of the nRF Connect
SDK or Zephyr SDK. Full credit goes to those authors. See license information at
the top of those files fore details.
