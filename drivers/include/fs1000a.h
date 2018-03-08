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
#define FS1000A_RECV_BUFLEN     (128U)
#define FS1000A_RECV_THRESHOLD  (8000U)

/**
 * bit and byteorder = big endian, lowest bit first, highest last
 *
 * ## PROTO1 (vendors: heitech):
 * start/stop interval:  9000 - 10000 us
 * 0/1 threshold: 500us
 *
 * A ON  = 1010 1010 1010 1010 1010 1010 1100 1100 1100 1100 1010 1100 = 000000111101
 * A OFF = 1010 1010 1010 1010 1010 1010 1100 1100 1100 1100 1100 1010 = 000000111110
 * B ON  = 1010 1010 1010 1010 1010 1100 1010 1100 1100 1100 1010 1100 = 000001011101
 * B OFF = 1010 1010 1010 1010 1010 1100 1010 1100 1100 1100 1100 1010 = 000001011110
 * C ON  = 1010 1010 1010 1010 1010 1100 1100 1010 1100 1100 1010 1100 = 000001101101
 * C OFF = 1010 1010 1010 1010 1010 1100 1100 1010 1100 1100 1100 1010 = 000001101110
 * D ON  = 1010 1010 1010 1010 1010 1100 1100 1100 1010 1100 1010 1100 = 000001110101
 * D OFF = 1010 1010 1010 1010 1010 1100 1100 1100 1010 1100 1100 1010 = 000001110110
 *
 * 48 bits encoded -> 12 Bits data, with:
 *      [ 0- 4]=system id (0=00000, ... 31=11111)
 *      [ 5- 9]=device id (A=01111,B=10111,C=11011,D=11101,E=11110)
 *      [10-11]=onoff (01=ON, 10=OFF)
 *
 * ## PROTO2 (vendors: deylon):
 * start/stop interval: 10000 - 11000 us
 * 0/1 threshold: 500us
 *
 * 1 ON  = 1010 1010 1010 1100 1100 1010 1100 1010 1100 1100 1100 1100 = 000110101111
 * 1 OFF = 1010 1010 1010 1100 1100 1010 1100 1010 1100 1100 1100 1010 = 000110101110
 * 2 ON  = 1010 1010 1010 1100 1100 1100 1010 1010 1100 1100 1100 1100 = 000111001111
 * 2 OFF = 1010 1010 1010 1100 1100 1100 1010 1010 1100 1100 1100 1010 = 000111001110
 * 3 ON  = 1010 1010 1010 1100 1010 1100 1100 1010 1100 1100 1100 1100 = 000101101111
 * 3 OFF = 1010 1010 1010 1100 1010 1100 1100 1010 1100 1100 1100 1010 = 000101101110
 * 4 ON  = 1010 1010 1010 1010 1100 1100 1100 1010 1100 1100 1100 1100 = 000011101111
 * 4 OFF = 1010 1010 1010 1010 1100 1100 1100 1010 1100 1100 1100 1010 = 000011101110
 *
 * 48 bits encoded -> 12 Bits data, with:
 *      [ 0- 2]=not used (=000)
 *      [ 3- 6]=device id (1=1101,2=1110,3=1011,4=0111)
 *      [ 7-10]=not used (=0111)
 *      [   11]=onoff (1=ON, 0=OFF)
 */

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
 * @brief print stats
 */
int fs1000a_analyse_spectrum(const fs1000a_t *dev);

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
