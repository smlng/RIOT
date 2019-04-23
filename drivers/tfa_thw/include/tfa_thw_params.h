/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_tfa_thw
 *
 * @{
 * @file
 * @brief       Default configuration for a TFA THW device
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef TFA_THW_PARAMS_H
#define TFA_THW_PARAMS_H

#include "periph/gpio.h"

#include "tfa_thw.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   Set default configuration parameters for the TFA THW driver
 * @{
 */
#ifndef TFA_THW_PARAM_GPIO
#define TFA_THW_PARAM_GPIO  GPIO_PIN(0, 0)
#endif

#ifndef TFA_THW_PARAMS
#define TFA_THW_PARAMS      { .gpio = TFA_THW_PARAM_GPIO }
#endif /* TFA_THW_PARAMS */
/**@}*/

/**
 * @brief   TFA THW configuration
 */
static const tfa_thw_params_t tfa_thw_params[] =
{
    TFA_THW_PARAMS,
};

#ifdef __cplusplus
}
#endif

#endif /* TFA_THW_PARAMS_H */
/** @} */
