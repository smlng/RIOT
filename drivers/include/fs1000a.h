/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_fs1000a FS1000A 433MHz Radio Transceiver
 * @ingroup     drivers_sensors
 * @brief       Driver for the FS1000A 433MHz Radio Transceiver
 *
 * @{
 * @file
 * @brief       Interface definition for the FS1000A driver
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef FS1000A_H
#define FS1000A_H

#include <stdint.h>

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FS1000A_MSG_RECV        (0x4601)
#define FS1000A_RECV_BUFLEN     (64U)
#define FS1000A_RECV_THRESHOLD  (5000U)

/**
 * @brief   Parameters needed for device initialization
 */
typedef struct {
    gpio_t send_pin;           /**< bus the device is connected to */
    gpio_t recv_pin;           /**< address on that bus */
} fs1000a_params_t;

/**
 * @brief   Device struct
 */
typedef struct {
    fs1000a_params_t p;
} fs1000a_t;

/**
 * @brief init function
 *
 * @param[out] dev          device descriptor
 * @param[in]  params       configuration parameters
 *
 * @return 0 on success, error otherwise
 */
int fs1000a_init(fs1000a_t *dev, const fs1000a_params_t *params);

/**
 * @brief enable background receiver decoding rc switch signals
 *
 * @return  0 on success, error otherwise
 */
int fs1000a_enable_switch_receive(const fs1000a_t *dev);

/**
 * @brief enable background sniffer for 433 MHz
 *
 * @return  0 on success, error otherwise
 */
int fs1000a_enable_sniffer(const fs1000a_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* FS1000A_H */
/** @} */
