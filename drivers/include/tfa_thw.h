/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_tfa_thw TFA Thermo-Hygro-Wind Sensor
 * @ingroup     drivers_sensors
 * @brief       Driver for the TFA Thermo-Hygro-Wind Sensor with 433MHz radio
 *
 * @{
 * @file
 * @brief       Interface definition for the TFA THW driver
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef TFA_THW_H
#define TFA_THW_H

#include <stdint.h>

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Message types for sensor data
 */
enum {
    TFA_THW_TYPE_INVALID,       /**< invalid data */
    TFA_THW_TYPE_TEMPERATURE,   /**< data with temperature and humidity */
    TFA_THW_TYPE_WIND,          /**< data with wind speed */
    TFA_THW_TYPE_MAX            /**< invalid data */
};

/**
 * @brief   Device initalization parameters
 */
typedef struct {
    gpio_t gpio;            /**< gpio pin the device is connected to */
} tfa_thw_params_t;

/**
 * @brief   Device struct
 */
typedef struct {
    tfa_thw_params_t p;     /**< device parameters */
} tfa_thw_t;

/**
 * @brief TFA data struct for decoding 64 Bit data messages
 */
typedef union {
    uint64_t u64;                   /**< raw data as uint64_t */
    struct {
        uint8_t csum        : 8;    /**< check sum */
        uint8_t humidity    : 8;    /**< humidity, if type == 1, otherwise 0 */
        uint8_t res1        : 4;    /**< unused/unknown, mostly 0000 */
        /**
         * @brief Temperature (in C) or wind (in kph) scaled by 10
         *
         * Temperature, if type == 1; Wind, if type == 2
         * Either value is scaled by 10.0
         * Temperature value has an offset of +500
         */
        uint16_t tempwind   : 12;
        uint8_t type        : 2;    /**< data type, 1=b01=temp, 2=b10=wind */
        uint8_t res2        : 2;    /**< unused/unknown */
        uint8_t chan        : 2;    /**< channel, mostly 00 */
        uint8_t res3        : 1;    /**< unused/unknown */
        uint8_t volt        : 1;    /**< battery level, 0=okay, 1=low */
        uint32_t id         : 20;   /**< device id, randomly generated */
        uint8_t res4        : 4;    /**< unused/unknown, mostly 0000 */
    };
} tfa_thw_data_t;

/**
 * @brief Initialize sensor receiver
 *
 * @param[out] dev          device descriptor
 * @param[in]  params       configuration parameters
 *
 * @return 0 on success, error otherwise
 */
int tfa_thw_init(tfa_thw_t *dev, const tfa_thw_params_t *params);

/**
 * @brief Read data from sensor
 *
 * @param[in] dev           device descriptor
 * @param[out] data         buffer array to hold data
 * @param[in] dlen          buffer elements
 *
 * @return 0 on success, error otherwise
 */
int tfa_thw_read(tfa_thw_t *dev, tfa_thw_data_t data[], size_t dlen);

#ifdef __cplusplus
}
#endif

#endif /* TFA_THW_H */
/** @} */
