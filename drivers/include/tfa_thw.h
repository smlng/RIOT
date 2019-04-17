/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_TFA_THW TFA_THW 433MHz Radio Transceiver
 * @ingroup     drivers_sensors
 * @brief       Driver for the TFA_THW 433MHz Radio Transceiver
 *
 * @{
 * @file
 * @brief       Interface definition for the TFA_THW driver
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef TFA_THW_H
#define TFA_THW_H

#include <stdint.h>

#include "periph/gpio.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Parameters needed for device initialization
 */
typedef struct {
    gpio_t gpio;           /**< bus the device is connected to */
} tfa_thw_params_t;

/**
 * @brief   Device struct
 */
typedef struct {
    tfa_thw_params_t p;
    kernel_pid_t listener;
} tfa_thw_t;

typedef struct {
    uint64_t values[2];
} tfa_thw_sensor_data_t;

typedef struct {
    int16_t t;
    int16_t h;
    int16_t w;
} tfa_thw_data_t;

/**
 * @brief init function
 *
 * @param[out] dev          device descriptor
 * @param[in]  params       configuration parameters
 *
 * @return 0 on success, error otherwise
 */
int tfa_thw_init(tfa_thw_t *dev, const tfa_thw_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* TFA_THW_H */
/** @} */
