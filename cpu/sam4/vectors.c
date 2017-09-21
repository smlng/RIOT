/*
 * Copyright (C) 2014-2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_sam3x8e
 * @{
 *
 * @file
 * @brief       Startup code and interrupt vector definition
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>
#include "vectors_cortexm.h"

/* get the start of the ISR stack as defined in the linkerscript */
extern uint32_t _estack;

/* define a local dummy handler as it needs to be in the same compilation unit
 * as the alias definition */
void dummy_handler(void) {
    dummy_handler_default();
}

/* Cortex-M common interrupt vectors */
WEAK_DEFAULT void isr_svc(void);
WEAK_DEFAULT void isr_pendsv(void);
WEAK_DEFAULT void isr_systick(void);
/* SAM3X8E specific interrupt vector */
WEAK_DEFAULT void isr_supc(void);
WEAK_DEFAULT void isr_rstc(void);
WEAK_DEFAULT void isr_rtc(void);
WEAK_DEFAULT void isr_rtt(void);
WEAK_DEFAULT void isr_wdt(void);
WEAK_DEFAULT void isr_pmc(void);
WEAK_DEFAULT void isr_efc0(void);
WEAK_DEFAULT void isr_efc1(void);
WEAK_DEFAULT void isr_uart0(void);
WEAK_DEFAULT void isr_uart1(void);
WEAK_DEFAULT void isr_pioa(void);
WEAK_DEFAULT void isr_piob(void);
WEAK_DEFAULT void isr_pioc(void);
WEAK_DEFAULT void isr_usart0(void);
WEAK_DEFAULT void isr_usart1(void);
WEAK_DEFAULT void isr_hsmci(void);
WEAK_DEFAULT void isr_twi0(void);
WEAK_DEFAULT void isr_twi1(void);
WEAK_DEFAULT void isr_spi(void);
WEAK_DEFAULT void isr_ssc(void);
WEAK_DEFAULT void isr_tc0(void);
WEAK_DEFAULT void isr_tc1(void);
WEAK_DEFAULT void isr_tc2(void);
WEAK_DEFAULT void isr_tc3(void);
WEAK_DEFAULT void isr_tc4(void);
WEAK_DEFAULT void isr_tc5(void);
WEAK_DEFAULT void isr_adc(void);
WEAK_DEFAULT void isr_dacc(void);
WEAK_DEFAULT void isr_pwm(void);
WEAK_DEFAULT void isr_crcc(void);
WEAK_DEFAULT void isr_aac(void);
WEAK_DEFAULT void isr_udp(void);

/* interrupt vector table */
ISR_VECTORS const void *interrupt_vector[] = {
    /* Exception stack pointer */
    (void*) (&_estack),             /* pointer to the top of the stack */
    /* Cortex-M4 handlers */
    (void*) reset_handler_default,  /* entry point of the program */
    (void*) nmi_default,            /* non maskable interrupt handler */
    (void*) hard_fault_default,     /* hard fault exception */
    (void*) mem_manage_default,     /* memory manage exception */
    (void*) bus_fault_default,      /* bus fault exception */
    (void*) usage_fault_default,    /* usage fault exception */
    (void*) (0UL),                  /* Reserved */
    (void*) (0UL),                  /* Reserved */
    (void*) (0UL),                  /* Reserved */
    (void*) (0UL),                  /* Reserved */
    (void*) isr_svc,                /* system call interrupt, in RIOT used for
                                     * switching into thread context on boot */
    (void*) debug_mon_default,      /* debug monitor exception */
    (void*) (0UL),                  /* Reserved */
    (void*) isr_pendsv,             /* pendSV interrupt, in RIOT the actual
                                     * context switching is happening here */
    (void*) isr_systick,            /* SysTick interrupt, not used in RIOT */
    /* SAM4SXY specific peripheral handlers */
    (void*) isr_supc,               /*  0 supply controller */
    (void*) isr_rstc,               /*  1 reset controller */
    (void*) isr_rtc,                /*  2 real time clock */
    (void*) isr_rtt,                /*  3 real timer timer */
    (void*) isr_wdt,                /*  4 watchdog timer */
    (void*) isr_pmc,                /*  5 power management controller */
    (void*) isr_efc0,               /*  6 enhanced flash controller 0 */
    (void*) isr_efc1,               /*  7 enhanced flash controller 1 */
    (void*) isr_uart0,              /*  8 universal asynchronous receiver transceiver */
    (void*) isr_uart1,              /*  9 universal asynchronous receiver transceiver */
    (void*) (0UL),                  /* 10 NA */
    (void*) isr_pioa,               /* 11 GPIO port A */
    (void*) isr_piob,               /* 12 GPIO port B */
    (void*) isr_pioc,               /* 13 GPIO port C */
    (void*) isr_usart0,             /* 14 USART0 */
    (void*) isr_usart1,             /* 15 USART1 */
    (void*) (0UL),                  /* 16 NA */
    (void*) (0UL),                  /* 17 NA */
    (void*) isr_hsmci,              /* 18 multimedia card interface */
    (void*) isr_twi0,               /* 19 two-wire interface 0 */
    (void*) isr_twi1,               /* 20 two-wire interface 1 */
    (void*) isr_spi,                /* 21 serial peripheral interface */
    (void*) isr_ssc,                /* 22 synchronous serial controller */
    (void*) isr_tc0,                /* 23 timer counter 0 */
    (void*) isr_tc1,                /* 24 timer counter 1 */
    (void*) isr_tc2,                /* 25 timer counter 2 */
    (void*) isr_tc3,                /* 26 timer counter 3 */
    (void*) isr_tc4,                /* 27 timer counter 4 */
    (void*) isr_tc5,                /* 28 timer counter 5 */
    (void*) isr_adc,                /* 29 ADC controller */
    (void*) isr_dacc,               /* 30 DAC controller */
    (void*) isr_pwm,                /* 31 pulse width modulation controller */
    (void*) isr_crcc                /* 32 CRC calculation unit */
    (void*) isr_aac,                /* 33 analog comparator */
    (void*) isr_udp,                /* 34 USB device port */
};
