/*
 * Copyright (c) 2018-2019 Peter Bigot Consulting, LLC
 * Copyright (c) 2021 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BATTERY_H_
#define BATTERY_H_

struct battery_measurement_data
{
	struct sensor_value voltage;
};

/** A point in a battery discharge curve sequence.
 *
 * A discharge curve is defined as a sequence of these points, where
 * the first point has #lvl_pptt set to 10000 and the last point has
 * #lvl_pptt set to zero.  Both #lvl_pptt and #lvl_mV should be
 * monotonic decreasing within the sequence.
 */
struct battery_level_point
{
	/** Remaining life at #lvl_mV. */
	uint16_t lvl_pptt;

	/** Battery voltage at #lvl_pptt remaining life. */
	uint16_t lvl_mV;
};

/** Calculate the estimated battery level based on a measured voltage.
 *
 * @param batt_mV a measured battery voltage level.
 *
 * @param curve the discharge curve for the type of battery installed
 * on the system.
 *
 * @return the estimated remaining capacity in parts per ten
 * thousand.
 */
unsigned int battery_level_pptt(unsigned int batt_mV,
								const struct battery_level_point *curve);

#endif /* APPLICATION_BATTERY_H_ */
