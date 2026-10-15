.. _at_client_sample:

AT Client
#########

.. contents::
   :local:
   :depth: 2

The AT Client sample forwards AT commands from the Feather's USB serial port to the nRF9151 modem and prints the responses.
Use it to query the modem, check network registration or read the modem firmware version from a terminal.

The sample builds Nordic's ``nrf/samples/cellular/at_client`` with a Feather-specific startup hook.
The hook sends ``AT%XANTCFG=1`` when the modem library starts so that the GNSS antenna is configured.

Requirements
************

* nRF9151 Feather (``circuitdojo_feather_nrf9151/nrf9151/ns``)
* An activated SIM card for commands that need the network

Building and running
********************

.. code-block:: console

   west build -b circuitdojo_feather_nrf9151/nrf9151/ns -s samples/at_client --sysbuild
   west flash

Testing
=======

1. Open a terminal on the Feather's serial port at 115200 baud.
   Enable local echo and send each command with a carriage return and line feed.
#. Reset the board.
#. Send commands and check the responses, for example:

   * ``AT+CGMR`` returns the modem firmware version.
   * ``AT+CFUN=1`` turns on the modem.
   * ``AT+CEREG?`` shows the network registration status.
   * ``AT%XMONITOR`` shows the modem parameters.
   * ``AT%XTEMP?`` shows the modem temperature.

Sample output
=============

.. code-block:: console

   AT+CGMR
   mfw_nrf91x1_2.0.4
   OK

References
**********

* `nRF91x1 AT Commands Reference Guide <https://docs.nordicsemi.com/bundle/ref_at_commands_nrf91x1/page/REF/at_commands/intro.html>`_
