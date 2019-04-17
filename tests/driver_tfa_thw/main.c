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

#define TFA_THW_PARAM_GPIO      GPIO_PIN(0, 0)

#include "tfa_thw.h"
#include "tfa_thw_params.h"
#include "fmt.h"
#include "thread.h"
#include "xtimer.h"

#define SLEEP_S     (10U)

static tfa_thw_t dev;

int main(void)
{
    puts("Init TFA_THW");
    dev.listener = thread_getpid();
    if (tfa_thw_init(&dev, &tfa_thw_params[0])) {
        puts("[FAILED]");
        return 1;
    }

    char hexbuf[32];
    puts("..INIT DONE..");
    while(1) {
        msg_t m;
        msg_receive(&m);
        tfa_thw_sensor_data_t data = *((tfa_thw_sensor_data_t *)m.content.ptr);
        size_t len = fmt_u64_hex(hexbuf, data.values[0]);
        hexbuf[len] = '\0';
        printf("(%s, ", hexbuf);
        len = fmt_u64_hex(hexbuf, data.values[1]);
        hexbuf[len] = '\0';
        printf("%s)\n", hexbuf);
    }
    return 0;
}
