/*
 * Copyright (C) 2015-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_stm32f4
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PERIPH_CPU_H
#define PERIPH_CPU_H

#include "periph_cpu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Starting address of the CPU ID
 */
#define CPUID_ADDR          (0x1fff7a10)

/**
 * @brief   Available ports on the STM32F4 family
 */
enum {
    PORT_A = 0,             /**< port A */
    PORT_B = 1,             /**< port B */
    PORT_C = 2,             /**< port C */
    PORT_D = 3,             /**< port D */
    PORT_E = 4,             /**< port E */
    PORT_F = 5,             /**< port F */
    PORT_G = 6,             /**< port G */
    PORT_H = 7,             /**< port H */
    PORT_I = 8              /**< port I */
};

/**
 * @brief   Available number of ADC devices
 */
#if defined(CPU_MODEL_STM32F401RE) || defined(CPU_MODEL_STM32F410RB) \
    || defined(CPU_MODEL_STM32F411RE) || defined(CPU_MODEL_STM32F412ZG) \
    || defined(CPU_MODEL_STM32F413CG) || defined(CPU_MODEL_STM32F413RG) \
    || defined(CPU_MODEL_STM32F413MG) || defined(CPU_MODEL_STM32F413VG) \
    || defined(CPU_MODEL_STM32F413ZG) || defined(CPU_MODEL_STM32F413CH) \
    || defined(CPU_MODEL_STM32F413RH) || defined(CPU_MODEL_STM32F413MH) \
    || defined(CPU_MODEL_STM32F413VH) || defined(CPU_MODEL_STM32F413ZH) \
    || defined(CPU_MODEL_STM32F423CH) || defined(CPU_MODEL_STM32F423RH) \
    || defined(CPU_MODEL_STM32F423MH) || defined(CPU_MODEL_STM32F423VH) \
    || defined(CPU_MODEL_STM32F423ZH)
#define ADC_DEVS            (1U)
#elif defined(CPU_MODEL_STM32F407VG) || defined(CPU_MODEL_STM32F415RG) \
    || defined(CPU_MODEL_STM32F429ZI) || defined(CPU_MODEL_STM32F437VG) \
    || defined(CPU_MODEL_STM32F446RE) || defined(CPU_MODEL_STM32F446ZE)
#define ADC_DEVS            (3U)
#endif

#ifndef DOXYGEN
/**
 * @brief   Override the ADC resolution configuration
 * @{
 */
#define HAVE_ADC_RES_T
typedef enum {
    ADC_RES_6BIT  = 0x03000000,     /**< ADC resolution: 6 bit */
    ADC_RES_8BIT  = 0x02000000,     /**< ADC resolution: 8 bit */
    ADC_RES_10BIT = 0x01000000,     /**< ADC resolution: 10 bit */
    ADC_RES_12BIT = 0x00000000,     /**< ADC resolution: 12 bit */
    ADC_RES_14BIT = 1,              /**< ADC resolution: 14 bit (not supported) */
    ADC_RES_16BIT = 2               /**< ADC resolution: 16 bit (not supported)*/
} adc_res_t;
/** @} */
#endif /* ndef DOXYGEN */

/**
 * @brief   ADC channel configuration data
 */
typedef struct {
    gpio_t pin;             /**< pin connected to the channel */
    uint8_t dev;            /**< ADCx - 1 device used for the channel */
    uint8_t chan;           /**< CPU ADC channel connected to the pin */
} adc_conf_t;

/**
 * @brief   Ethernet Peripheral configuration
 */
typedef struct {
    enum {
        MII = 18,                       /**< Configuration for MII */
        RMII = 9,                       /**< Configuration for RMII */
        SMI = 2,                        /**< Configuration for SMI */
    } mode;                             /**< Select configuration mode */
    enum {
        ETH_SPEED_10T_HD = 0x0000,
        ETH_SPEED_10T_FD = 0x0100,
        ETH_SPEED_100TX_HD = 0x2000,
        ETH_SPEED_100TX_FD = 0x2100,
    } speed;                            /**< Speed selection */
    uint8_t dma_stream;                 /**< DMA stream used for TX */
    uint8_t dma_chan;                   /**< DMA channel used for TX */
    char mac[6];                        /**< Et hernet MAC address */
    char phy_addr;                      /**< PHY address */
    gpio_t pins[];                      /**< Pins to use. MII requires 18 pins,
                                             RMII 9 and SMI 9. Not all speeds are
                                             supported by all modes. */
} eth_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
/** @} */
