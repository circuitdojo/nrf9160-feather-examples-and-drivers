# Accelerometer ZBus Sample 

**Important note:** JMP3 on the nRF9160 Feather must be soldered in order to use interrupts. If you are unable, you can comment out `CONFIG_LIS2DH_TRIGGER_GLOBAL_THREAD` in `prj.conf`:

```
#CONFIG_LIS2DH_TRIGGER_GLOBAL_THREAD=y
```

## Summary

Use ZBus to kick off a sensor value fetch and pass data from sensor module. When interrupts are enabled (`CONFIG_LIS2DH_TRIGGER_GLOBAL_THREAD`) the start measurement process occurs from the LIS2DH event handler instead of the main thread.