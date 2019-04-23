/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_tfa_thw
 * @{
 *
 * @file
 * @brief       Driver for the TFA Thermo-Hygro-Wind sensor
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */
#include <string.h>

#include "msg.h"
#include "periph/gpio.h"
#include "thread.h"
#include "xtimer.h"

#include "tfa_thw.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"


#define TFA_DEVID_S         (40U)
#define TFA_DEVID_M         (0xFFFFF)
#define TFA_VOLT_S          (39U)
#define TFA_VOLT_M          (0x1)
#define TFA_CHAN_S          (36U)
#define TFA_CHAN_M          (0x7)
#define TFA_MSGID_S         (32U)
#define TFA_MSGID_M         (0x3)
#define TFA_WIND_S          (20U)
#define TFA_WIND_M          (0xFFF)
#define TFA_TEMP_S          (20U)
#define TFA_TEMP_M          (0xFFF)
#define TFA_HUM_S           (8U)
#define TFA_HUM_M           (0xFF)
#define TFA_CRC_S           (0U)
#define TFA_CRC_M           (0xFF)

#define TFA_STACK_SIZE  (THREAD_STACKSIZE_DEFAULT)
#define TFA_QUEUE_LEN   (32U)
#define TFA_RECV_BUFLEN (132U)
#define TFA_ZERO_US     (380U)      /* smaller is a 0 */
#define TFA_ONE_US      (580U)      /* smaller is a 1, above is sync */
#define TFA_PREAMBLE    (6U)
#define TFA_RECV_RAWLEN (128U)
#define TFA_TRIGGER_US  (100000UL)  /* interval time to trigger eval */

static char _stack[TFA_STACK_SIZE];
static msg_t _queue[TFA_QUEUE_LEN];
static volatile uint32_t last = 0;
static volatile kernel_pid_t epid = KERNEL_PID_UNDEF;

static void _isr_cb(void *arg)
{
    (void) arg;

    uint32_t now = xtimer_now_usec();
    msg_t m;
    m.content.value = (uint32_t)(now - last);

    if (epid > KERNEL_PID_UNDEF) {
        msg_send(&m, epid);
    }

    last = now;
}

static uint64_t _eval_buf(uint8_t *buf, unsigned len)
{
    uint64_t u64 = 0;
    unsigned shift = 64;
    for (unsigned i = 0; (i < (len/2)) && shift; ++i) {
        --shift;
        uint8_t v0 = buf[(2 * i)];
        uint8_t v1 = buf[(2 * i) + 1];
        DEBUG("%d%d", (int)v0, (int)v1);
        if (v0 == v1) {
            DEBUG("\n! %s: invalid encoding !\n", __func__);
            return 0;
        }
        if ((v0 == 1) && (v1 == 0)) {
            u64 |= 1ULL << shift;
        }
    }
    DEBUG("\n");
    return u64;
}

void *_eventloop(void *arg)
{
    DEBUG("%s: enter\n", __func__);

    msg_init_queue(_queue, TFA_QUEUE_LEN);

    tfa_thw_t *dev = (tfa_thw_t *)arg;
    tfa_thw_data_t data;
    data.u64 = 0;

    uint8_t recvbuf[TFA_RECV_BUFLEN] = { 0 };
    unsigned bufpos = 0;
    unsigned preamble = 0;

    while (1) {
        msg_t m;
        msg_receive(&m);
        uint32_t val = m.content.value;
        if (val > TFA_ONE_US) {
            if (bufpos >= TFA_RECV_RAWLEN) {
                gpio_irq_disable(dev->p.gpio);
                uint64_t v64 = _eval_buf(recvbuf, bufpos);
                if ((v64 != data.u64) && (dev->listener != KERNEL_PID_UNDEF)) {
                    data.u64 = v64;
                    DEBUG("(%"PRIu32", %u, %u, %u, %"PRIu16", %u, %u)\n",
                          data.id, data.volt, data.chan, data.type,
                          data.tempwind, data.humidity, data.csum);
                    if (data.type > 0) {
                        msg_t n;
                        n.content.ptr = &data;
                        msg_send(&n, dev->listener);
                    }
                }
                preamble = 0;
                bufpos = 0;
                memset(recvbuf, 0, TFA_RECV_BUFLEN);
                gpio_irq_enable(dev->p.gpio);
            }
            preamble++;
        }
        else if ((preamble > TFA_PREAMBLE) && (bufpos < TFA_RECV_BUFLEN)) {
            if (val > TFA_ZERO_US) {
                recvbuf[bufpos] = 1;
            }
            bufpos++;
        }
        else {
            preamble = 0;
            bufpos = 0;
            memset(recvbuf, 0, TFA_RECV_BUFLEN);
        }
    }
}

int tfa_thw_init(tfa_thw_t *dev, const tfa_thw_params_t *params)
{
    DEBUG("%s: enter\n", __func__);
    dev->p = *params;
    epid = thread_create(_stack, sizeof(_stack),
                         THREAD_PRIORITY_MAIN - 1,
                         THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                         _eventloop, dev, "tfa_thw");
    if (epid < 0) {
        DEBUG("%s: thread_create failed!\n", __func__);
        return -1;
    }
    xtimer_sleep(1);
    if (gpio_init_int(dev->p.gpio, GPIO_IN, GPIO_BOTH, _isr_cb, NULL) < 0) {
        DEBUG("%s: gpio_init_int failed!\n", __func__);
        return -2;
    }
    return 0;
}
