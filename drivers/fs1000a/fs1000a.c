/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_fs1000a
 * @{
 *
 * @file
 * @brief       Driver for the FS1000a 433 MHz radio module
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <string.h>

#include "fs1000a.h"
#include "msg.h"
#include "thread.h"
#include "xtimer.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define RECV_QUEUE_LEN  (8U)

static char recv_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t recv_queue[RECV_QUEUE_LEN];
static uint32_t timings[FS1000A_RECV_BUFLEN];
static uint32_t last = 0;
static kernel_pid_t rt = KERNEL_PID_UNDEF;

static uint32_t abs_diff(uint32_t a, uint32_t b)
{
    if (a < b) {
        return (b - a);
    }
    return (a - b);
}

typedef union {
    uint16_t all;
    struct {
        uint16_t sysnum     : 5;
        uint16_t devnum     : 5;
        uint16_t onoff      : 2;
        uint16_t reserved   : 4;
    } bits;
} rcswitch_data_t;

static void _recv_cb(void *arg)
{
    (void)arg;
    uint32_t now = xtimer_now_usec();
    msg_t m;
    m.content.value = now - last;

    if (rt > KERNEL_PID_UNDEF) {
        msg_send(&m, rt);
    }

    last = now;
}

static uint32_t _decode_4bits(const uint8_t *inbuf, size_t inlen)
{
    DEBUG("%s: enter (inlen=%d)\n", DEBUG_FUNC, (int)inlen);
    uint32_t data = 0;
    for (size_t i = 0; i < (inlen * 2); ++i) {
        uint8_t val = 0;
        if (i % 2) {
            val = (inbuf[(i / 2)] >> 4) & 0x0F;
        }
        else {
            val = inbuf[(i / 2)] & 0x0F;
        }
        if (val == 0x5) {
            val = 0;
        }
        else if (val == 0x3) {
            val = 1;
        }
        else {
            return 0;
        }
        data |= (uint32_t)val << i;
    }
    DEBUG("data=0x%"PRIx32"\n", data);
    return data;
}

static int _decode_plain(uint32_t threshhold,
                         const uint32_t *inbuf, size_t inlen,
                         uint8_t *outbuf, size_t outlen)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);
    int pos = -1;
    /* align input on 4 bits, such that 1100 = 1 and 1010 = 0 */
    size_t start = inlen - ((inlen / 4) * 4);
    const uint32_t *tmp = &inbuf[start];
    for (size_t i = 0; i < (inlen - start); ++i) {
        bool onoff= ((tmp[i] < threshhold) ? 0 : 1);
        DEBUG("%d", onoff);
        if ((outbuf != NULL) && (outlen > 0)) {
            pos = (i / 8);
            outbuf[pos] |= (onoff << (i % 8));
        }
    }
    DEBUG("\n");
    return (pos + 1);
}

void *_receiver(void *arg)
{
    (void) arg;
    DEBUG("%s: enter\n", DEBUG_FUNC);

    msg_init_queue(recv_queue, RECV_QUEUE_LEN);

    unsigned num_change = 0;
    unsigned num_repeat = 0;

    while (1) {
        msg_t m;
        msg_receive(&m);
        uint32_t diff = m.content.value;
        if (diff > FS1000A_RECV_THRESHOLD) {
            if (abs_diff(diff, timings[0]) < 200) {
                if (num_repeat++ > 1) {
                    uint8_t buf[32];
                    memset(buf, 0, 32);
                    int ret = _decode_plain(666, &timings[0], num_change, buf, 32);
                    if (ret > 0) {
                        _decode_4bits(buf, ret);
                    }
                    num_repeat = 0;
                }
            }
            num_change = 0;
        }
        timings[num_change++] = diff;
        if (num_change > FS1000A_RECV_BUFLEN) {
            num_change = 0;
            num_repeat = 0;
        }
    }
}

void *_sniffer(void *arg)
{
    (void)arg;
    puts("FS1000A 433 MHz sniffer.");
    puts("");
    while (1) {
        msg_t m;
        msg_receive(&m);
        printf("%" PRIu32 "\n", m.content.value);
    }
}

int fs1000a_init(fs1000a_t *dev, const fs1000a_params_t *params)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);

    memcpy(&dev->p, params, sizeof(fs1000a_params_t));

    return 0;
}

static int _run_background_thread(const fs1000a_t *dev, void* func)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);
    if ((dev->p.recv_pin != GPIO_UNDEF) && (rt <= KERNEL_PID_UNDEF)) {
        rt = thread_create(recv_stack, sizeof(recv_stack),
                                       THREAD_PRIORITY_MAIN - 1,
                                       THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                                       func, NULL, "fs1000a");
        if (rt < 0) {
            DEBUG("%s: thread_create failed!\n", DEBUG_FUNC);
            return -1;
        }
        if (gpio_init_int(dev->p.recv_pin, GPIO_IN, GPIO_BOTH, _recv_cb, NULL) < 0) {
            DEBUG("%s: gpio_init_int failed!\n", DEBUG_FUNC);
            return -2;
        }
    }
    return 0;
}

int fs1000a_enable_switch_receive(const fs1000a_t *dev)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);
    return _run_background_thread(dev, _receiver);
}

int fs1000a_enable_sniffer(const fs1000a_t *dev)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);
    return _run_background_thread(dev, _sniffer);
}
