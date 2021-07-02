# nRF9160 Feather Examples and Drivers

Zephyr examples and drivers for the nRF9160 Feather.

## Setup

Before you get started, you'll need to install the nRF Connect SDK. Here are the full instructions:

* [Mac](https://docs.jaredwolff.com/nrf9160-sdk-setup-mac.html)
* [Windows](https://docs.jaredwolff.com/nrf9160-sdk-setup-windows.html)
* [Linux](https://docs.jaredwolff.com/nrf9160-sdk-setup-linux.html)

The below instructions are also included in the above. If you already have NCS setup you can follow these steps (**Mac and Windows only. These steps are not needed on Linux**):

1. To get the nRF9160 Feather examples we'll update `C:\Users\<your username>\ncs\v1.5.0\nrf\west.yml`. In the `projects` section add:

   ```yaml
    - name: nfed
      url: https://github.com/circuitdojo/nrf9160-feather-examples-and-drivers
      revision: v1.5.x
      path: nfed
   ```
    
   Here's the diff for the file afterwards:

   ```
   diff --git a/west.yml b/west.yml
   index db9211c27..345719599 100644
   --- a/west.yml
   +++ b/west.yml
   @@ -145,6 +145,14 @@ manifest:
          remote: nordicsemi
          revision: 24f1b2b0c64c694b7f9ac1b7eab60b39236ca0bf
          path: modules/lib/cddl-gen
   +    - name: nfed
   +      url: https://github.com/circuitdojo/nrf9160-feather-examples-and-drivers
   +      revision: v1.5.x
   +      path: nfed
   +    - name: pyrinas
   +      url: https://github.com/pyrinas-iot/pyrinas-zephyr
   +      revision: v1.5.x
   +      path: pyrinas
 
   # West-related configuration for the nrf repository.
   self:
   ```

2. Then run `west update` in your freshly created bash/command prompt session. This will fetch the nRF9160 Feather examples.