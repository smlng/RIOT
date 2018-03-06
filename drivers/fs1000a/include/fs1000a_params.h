/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_fs1000a
 *
 * @{
 * @file
 * @brief       Default configuration for FS1000A devices
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef FS1000A_PARAMS_H
#define FS1000A_PARAMS_H

#include "board.h"
#include "fs1000a.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   Set default configuration parameters for the HTS221 driver
 * @{
 */
#ifndef FS1000A_PARAM_RECV_PIN
#define FS1000A_PARAM_RECV_PIN      GPIO_PIN(0, 1)
#endif
#ifndef FS1000A_PARAM_SEND_PIN
#define FS1000A_PARAM_SEND_PIN      GPIO_PIN(2, 15)
#endif
#ifndef FS1000A_PARAMS
#define FS1000A_PARAMS       { .recv_pin = FS1000A_PARAM_RECV_PIN, \
                               .send_pin = FS1000A_PARAM_SEND_PIN }
#endif /* FS1000A_PARAMS */

#ifndef FS1000A_SAULINFO
#define FS1000A_SAULINFO     { .name = "fs1000a" }
#endif
/**@}*/

/**
 * @brief   HTS221 configuration
 */
static const fs1000a_params_t fs1000a_params[] =
{
    FS1000A_PARAMS,
};

/**
 * @brief   Additional meta information to keep in the SAUL registry
 */
static const saul_reg_info_t fs1000a_saul_info[] =
{
    FS1000A_SAULINFO
};

#ifdef __cplusplus
}
#endif

#endif /* FS1000A_PARAMS_H */
/** @} */
