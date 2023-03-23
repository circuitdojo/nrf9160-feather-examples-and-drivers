# Accelerometer Sample 

**Important note:** JMP3 on the nRF9160 Feather must be soldered in order to use interrupts. If you are unable, you can comment out `CONFIG_LIS2DH_TRIGGER_GLOBAL_THREAD` in `prj.conf`:

```
#CONFIG_LIS2DH_TRIGGER_GLOBAL_THREAD=y
```