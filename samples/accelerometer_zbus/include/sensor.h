#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <zephyr/drivers/sensor.h>

struct start_data
{
    bool started;
};

struct sensor_data
{
    int64_t ts;
    struct sensor_value value[3];
};

#endif