/*
 * Copyright (C) 2017 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       HTS221 humidity and temperature sensor driver test application
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "fs1000a.h"
#include "fs1000a_params.h"
#include "xtimer.h"

#define SLEEP_S     (2U)

static fs1000a_t dev;

int main(void)
{
    puts("Init FS1000A");
    if (fs1000a_init(&dev, &fs1000a_params[0])) {
        puts("[FAILED]");
        return 1;
    }

    fs1000a_enable_switch_receive(&dev);
    //fs1000a_enable_sniffer(&dev);

    while(1) {
        xtimer_sleep(SLEEP_S);
    }
    return 0;
}
