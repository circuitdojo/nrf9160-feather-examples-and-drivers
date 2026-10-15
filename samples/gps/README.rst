.. _gps_sample:

GPS
###

.. contents::
   :local:
   :depth: 2

The GPS sample starts the nRF9151 modem's GNSS receiver and prints each position, velocity and time (PVT) fix along with the satellites being tracked.

The sample builds Nordic's ``nrf/samples/cellular/gnss`` with a Feather-specific startup hook.
The hook sends ``AT%XANTCFG=1`` when the modem library starts so that the GNSS antenna is configured.
Assistance data (A-GNSS) is disabled by default, so the first fix can take several minutes.
The sample's ``Kconfig`` lists the operating modes and assistance options that you can enable in ``prj.conf``.

Requirements
************

* nRF9151 Feather (``circuitdojo_feather_nrf9151/nrf9151/ns``)
* A GNSS antenna connected to the Feather, used outdoors or with a clear view of the sky

Building and running
********************

.. code-block:: console

   west build -b circuitdojo_feather_nrf9151/nrf9151/ns -s samples/gps --sysbuild
   west flash

Testing
=======

1. Open a terminal on the Feather's serial port at 115200 baud.
#. Reset the board.
#. Watch the number of tracked satellites rise.
   Once the receiver has a fix, the sample prints the position:

   .. code-block:: console

      Latitude:          63.421546
      Longitude:         10.437814
      Accuracy:          4.9 m
      Altitude:          163.7 m
