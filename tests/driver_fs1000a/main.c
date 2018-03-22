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

#define FS1000A_PARAM_RECV_PIN      GPIO_PIN(0, 28)

#include "fs1000a.h"
#include "fs1000a_params.h"
#include "thread.h"
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

    //fs1000a_enable_switch_receive(&dev);
    //fs1000a_enable_sniffer(&dev);
    //fs1000a_analyse_spectrum(&dev);
    fs1000a_enable_sensor_receive(&dev, thread_getpid());

    while(1) {
        msg_t m;
        msg_receive(&m);
        fs1000a_sensor_data_t *sdat = (fs1000a_sensor_data_t *)m.content.ptr;
        printf("V0: 0x%" PRIx64 ", V1: 0x%" PRIx64 "\n", sdat->values[0], sdat->values[1]);
    }
    return 0;
}
