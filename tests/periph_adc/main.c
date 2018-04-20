/*
 * Copyright (C) 2014-2015 Freie Universität Berlin
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
 * @brief       Test application for peripheral ADC drivers
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "periph/adc.h"
#include "xtimer.h"

#define RES        ADC_RES_10BIT
#define SNUM	   (1200U)

static int16_t samples[SNUM];

int main(void)
{
    /* initialize all available ADC lines */
    if (adc_init(ADC_LINE(1)) < 0) {
        return 1;
    }

    unsigned count = 0;
    puts("");
    uint32_t last = xtimer_now_usec();
    while (1) {
        samples[count++] = (int16_t)adc_sample(ADC_LINE(1), RES);
        if (count == SNUM) {
            uint32_t now = xtimer_now_usec();
            printf("%u samples = %"PRIu32"usec\n", SNUM, (now - last));
            for (unsigned i = 0; i < SNUM; ++i) {
                printf("%u, %"PRIi16"\n", i, samples[i]);
            }
            count = 0;
            puts("");
            last = now;
        }
    }
    return 0;
}
