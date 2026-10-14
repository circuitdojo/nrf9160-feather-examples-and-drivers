# Accelerometer ZBus Sample 

## Summary

Use ZBus to kick off a sensor value fetch and pass data from sensor module. When interrupts are enabled (`CONFIG_LIS2DH_TRIGGER_GLOBAL_THREAD`) the start measurement process occurs from the LIS2DH event handler instead of the main thread.
