/*
 * Copyright (C) 2018 HAW Hamburg
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
 * @brief       Default configuration for TFA_THW devices
 *
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef TFA_THW_PARAMS_H
#define TFA_THW_PARAMS_H

#include "board.h"
#include "tfa_thw.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name   Set default configuration parameters for the HTS221 driver
 * @{
 */
#ifndef TFA_THW_PARAM_GPIO
#define TFA_THW_PARAM_GPIO      GPIO_PIN(0, 1)
#endif
#ifndef TFA_THW_PARAMS
#define TFA_THW_PARAMS       { .gpio = TFA_THW_PARAM_GPIO }
#endif /* TFA_THW_PARAMS */

#ifndef TFA_THW_SAULINFO
#define TFA_THW_SAULINFO     { .name = "tfa_thw" }
#endif
/**@}*/

/**
 * @brief   HTS221 configuration
 */
static const tfa_thw_params_t tfa_thw_params[] =
{
    TFA_THW_PARAMS,
};

/**
 * @brief   Additional meta information to keep in the SAUL registry
 */
static const saul_reg_info_t tfa_thw_saul_info[] =
{
    TFA_THW_SAULINFO
};

#ifdef __cplusplus
}
#endif

#endif /* TFA_THW_PARAMS_H */
/** @} */
