# nRF9160 Feather Examples and Drivers

Zephyr examples and drivers for the nRF9160 Feather.

## Setup

Before you get started, you'll need to install the nRF Connect SDK. Here are the full instructions:

* [Mac](https://docs.jaredwolff.com/nrf9160-sdk-setup-mac.html)
* [Windows](https://docs.jaredwolff.com/nrf9160-sdk-setup-windows.html)

The below instructions are also included in the above. If you already have NCS setup you can follow these steps:

1. To get the nRF9160 Feather examples we'll update `C:\Users\<your username>\ncs\v1.5.0\nrf\west.yml`. First in the `remotes` section add:

   ```yaml
    - name: circuitdojo
      url-base: https://github.com/circuitdojo
   ```

2. Then in the `projects` section add at the bottom:

    ```yaml
    - name: nfed
      repo-path: nrf9160-feather-examples-and-drivers
      revision: v1.5.x
      path: nfed
      remote: circuitdojo
    ```
    
    Here's the diff for the file afterwards:

    ```
    diff --git a/west.yml b/west.yml
    index 2065ad3f..de8ea812 100644
    --- a/west.yml
    +++ b/west.yml
    @@ -33,6 +33,8 @@ manifest:
           url-base: https://github.com/nanopb
         - name: alexa
           url-base: https://github.com/alexa
    +    - name: circuitdojo
    +      url-base: https://github.com/circuitdojo
     
       # If not otherwise specified, the projects below should be obtained
       # from the ncs remote.
    @@ -124,6 +126,13 @@ manifest:
           path: modules/alexa-embedded
           revision: face92d8c62184832793f518bb1f19379538c5c1
           remote: alexa
    +    - name: nfed
    +      repo-path: nrf9160-feather-examples-and-drivers
    +      revision: v1.5.x
    +      path: nfed
    +      remote: circuitdojo
    +    - name: pyrinas
    +      path: pyrinas
     
       # West-related configuration for the nrf repository.
       self:
    
3. Then run `west update` in your freshly created bash/command prompt session. This will fetch the nRF9160 Feather examples.