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
 * @brief       Driver implementation of the TFA Thermo-Hygro-Wind sensor
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */
#include <stdint.h>
#include <string.h>

#include "msg.h"
#include "mutex.h"
#include "periph/gpio.h"
#include "thread.h"
#include "xtimer.h"

#include "tfa_thw.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

/* thread stack size for _eventloop */
#define TFA_THW_STACK_SIZE  (THREAD_STACKSIZE_DEFAULT)
/* msg queue size for _eventloop */
#define TFA_THW_QUEUE_LEN   (32U)
/* size of raw data, 2 * 64 = 128 */
#define TFA_THW_RECV_RAWLEN (128U)
/* buffer size for raw data */
#define TFA_THW_RECV_BUFLEN (132U)
/* smaller is a 0 */
#define TFA_THW_ZERO_US     (400U)
 /* smaller is a 1, above is sync */
#define TFA_THW_ONE_US      (600U)
/* number of sync symbols before eval */
#define TFA_THW_PREAMBLE    (6U)
/* IPC message type for internal demuxing */
#define TFA_THW_MSG_DATA    (0x6583)
#define TFA_THW_MSG_STOP    (0x6584)
/* Wait/sleep time between thread create and gpio init */
#define TFA_THW_INIT_WAIT   (2U)

static char _stack[TFA_THW_STACK_SIZE];
static msg_t _queue[TFA_THW_QUEUE_LEN];

static volatile uint32_t last = 0;
static mutex_t _lock = MUTEX_INIT;

static volatile kernel_pid_t epid = KERNEL_PID_UNDEF;
static volatile kernel_pid_t lpid = KERNEL_PID_UNDEF;

#define DBG_GPIO    GPIO_PIN(2, 4)
#define DBG_PORT    GPIOC
#define DBG_MASK    (1 << 4)

#define DBG_ON             (DBG_PORT->BSRR = DBG_MASK)
#define DBG_OFF            (DBG_PORT->BSRR = (DBG_MASK << 16))
#define DBG_TOGGLE         (DBG_PORT->ODR  ^= DBG_MASK)

/**
 * @brief   Interrupt callback
 */
static void _isr_cb(void *arg)
{
    (void) arg;
    DBG_TOGGLE;
    uint32_t now = xtimer_now_usec();
    msg_t m;
    m.type = TFA_THW_MSG_DATA;
    m.content.value = (uint32_t)(now - last);

    if (epid > KERNEL_PID_UNDEF) {
        msg_send(&m, epid);
    }

    last = now;
}

/**
 * @brief Parse receive buffer into uint64_t
 */
static uint64_t _eval_buf(uint8_t *buf, unsigned len)
{
    DEBUG("%s: enter\n", __func__);

    uint64_t u64 = 0;
    unsigned shift = 64;

    for (unsigned i = 0; (i < (len / 2)) && shift; ++i) {
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

/**
 * @brief Background driver thread to handle sensor data
 */
void *_eventloop(void *arg)
{
    DEBUG("%s: enter\n", __func__);

    msg_init_queue(_queue, TFA_THW_QUEUE_LEN);

    tfa_thw_t *dev = (tfa_thw_t *)arg;
    tfa_thw_data_t last;
    last.u64 = 0;

    uint8_t recvbuf[TFA_THW_RECV_BUFLEN] = { 0 };
    unsigned bufpos = 0;
    unsigned preamble = 0;

    if (gpio_init_int(dev->p.gpio, GPIO_IN, GPIO_BOTH, _isr_cb, NULL) < 0) {
        DEBUG("%s: gpio_init_int failed!\n", __func__);
        return NULL;
    }

    while (1) {
        msg_t m;
        msg_receive(&m);
        if (m.type == TFA_THW_MSG_STOP) {
            DEBUG("tfa_thw: stop event loop!\n");
            break;
        }
        else if (m.type = TFA_THW_MSG_DATA) {
            uint32_t val = m.content.value;
            /* large time interval means sync or keep alive */
            if (val > TFA_THW_ONE_US) {
                /* eval buffer, if enough data was received */
                if (bufpos >= TFA_THW_RECV_RAWLEN) {
                    DEBUG("tfa_thw: gpio_irq_disable ...");
                    gpio_irq_disable(dev->p.gpio);
                    DEBUG("[DONE]\n");
                    tfa_thw_msg_t msg;
                    msg.u64 = _eval_buf(recvbuf, bufpos);
                    DEBUG("tfa_thw: msg(%"PRIx32", %u, %u, %u, %"PRIu16", %u, %u)\n",
                          (uint32_t)msg.id, msg.volt, msg.chan, msg.type,
                          msg.tempwind, msg.humidity, msg.csum);
                    if ((msg.type > 0) && (msg.u64 != last.u64) && (dev->cb != NULL)) {
                        last.u64 = msg.u64;
                        DEBUG("tfa_thw: sending data to listener ...");
                        dev->cb(&last_data);
                        DEBUG("[DONE]\n");
                    }
                    /* reset intermediate vars and buffer */
                    preamble = 0;
                    bufpos = 0;
                    memset(recvbuf, 0, TFA_THW_RECV_BUFLEN);
                    DEBUG("tfa_thw: gpio_irq_enable ...");
                    gpio_irq_enable(dev->p.gpio);
                    DEBUG("[DONE]\n");
                }
                preamble++;
            }
            else if ((preamble > TFA_THW_PREAMBLE) &&
                     (bufpos < TFA_THW_RECV_BUFLEN)) {
                /* decode time interval into 0 and 1 */
                if (val > TFA_THW_ZERO_US) {
                    recvbuf[bufpos] = 1;
                }
                bufpos++;
            }
            else {
                /* bogos data, reset vars and buffer */
                preamble = 0;
                bufpos = 0;
                memset(recvbuf, 0, TFA_THW_RECV_BUFLEN);
            }
        }
    }
    DEBUG("tfa_thw: de-initialize GPIO, set to output.\n");
    gpio_init(dev->p.gpio, GPIO_OUT);
}

int tfa_thw_init(tfa_thw_t *dev, const tfa_thw_params_t *params)
{
    DEBUG("%s: enter\n", __func__);

    dev->p = *params;
    dev->epid = KERNEL_PID_UNDEF;
    dev->cb = NULL;

    return 0;
}

void tfa_thw_enable(tfa_thw_t *dev, tfa_thw_cb_t cb)
{
    /* create _eventloop thread */
    dev->epid = thread_create(_stack, sizeof(_stack),
                              THREAD_PRIORITY_MAIN - 1,
                              THREAD_CREATE_STACKTEST,
                              _eventloop, dev, "tfa_thw");
}

void tfa_thw_disable(tfa_thw_t *dev)
{
    if (dev->epid != KERNEL_PID_UNDEF) {
        msg_t m;
        m.type = TFA_THW_MSG_STOP;
        msg_send(&m, dev->epid);
        dev->epid = KERNEL_PID_UNDEF;
    }
}

int tfa_thw_read(tfa_thw_t *dev, tfa_thw_data_t *data, size_t dlen)
{
    DEBUG("%s: enter\n", __func__);

    if (!dev || !data || !dlen) {
        DEBUG("%s: invalid params!\n", __func__);
        return 1;
    }
    DEBUG("%s: locking ...", __func__);
    /* acquire driver lock to ensure single listener */
    mutex_lock(&_lock);
    DEBUG("[DONE]\n");
    lpid = thread_getpid();
    size_t i = 0;
    while (i < dlen) {
        DEBUG("%s: receive data (%u)\n", __func__, (unsigned)i);
        msg_t m;
        msg_receive(&m);
        if (m.type == TFA_THW_MSG_TYPE) {
            memcpy(&data[i], m.content.ptr, sizeof(tfa_thw_data_t));
            ++i;
        }
    }
    /* unset listener pid */
    lpid = KERNEL_PID_UNDEF;
    DEBUG("%s: unlocking ...", __func__);
    mutex_unlock(&_lock);
    DEBUG("[DONE]\n");
    return 0;
}
