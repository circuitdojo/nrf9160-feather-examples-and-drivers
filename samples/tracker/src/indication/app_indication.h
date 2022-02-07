
/*
 * Copyright Circuit Dojo (c) 2022
 *
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#ifndef APP_INDICATION_H
#define APP_INDICATION_H

enum app_indication_mode
{
    app_indication_none,
    app_indication_glow,
    app_indication_fast_blink,
    app_indication_solid,
    app_indication_error,
};

int app_indication_init(void);

int app_indication_set(enum app_indication_mode mode);

#endif