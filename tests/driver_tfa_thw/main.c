/*
 * Copyright (C) 2019 HAW Hamburg
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
 * @brief       TFA Thermo-Hygro-Wind sensor driver test application
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <inttypes.h>
#include <stdio.h>

#include "tfa_thw.h"
#include "tfa_thw_params.h"

/*
 * @brief Number of sensor values to receive at once.
 *
 * The TFA THW sensor emits 2 messages: one for temperture and humidity,
 * and another for windspeeds.
 */
#define DATALEN     (2U)

static tfa_thw_t dev;
static tfa_thw_data_t data[DATALEN];

static void tfa_print(tfa_thw_data_t *data)
{
    printf(" - DEVID=%" PRIx32 ", VOLT=%u, CHAN=%u, TYPE=%u, ",
           (uint32_t)data->id, data->volt, data->chan, data->type);
    if (data->type == 1) {
        int tw = (int)(data->tempwind - 500);
        printf("TEMPERATURE=%d.%d, HUMIDITY=%u\n", (tw / 10), (tw % 10), data->humidity);
    }
    else if (data->type == 2) {
        printf("WIND=%u.%u\n", (unsigned)(data->tempwind / 10), (unsigned)(data->tempwind % 10));
    }
    else {
        printf("!! invalid data type (%u) !!\n", data->type);
    }
}

int main(void)
{
    printf("Init TFA_THW ... ");
    if (tfa_thw_init(&dev, &tfa_thw_params[0])) {
        puts("[FAIL]");
        return 1;
    }
    puts("[DONE]");
    while(1) {
        puts(".. reading data:");
        if (tfa_thw_read(&dev, data, DATALEN) == 0) {
            for (unsigned i = 0; i < DATALEN; ++i) {
                tfa_print(&data[i]);
            }
        }
        else{
            puts(" ! tfa_thw_read failed !");
        }
    }
    return 0;
}
