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

#define ENABLE_DEBUG    (0)
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

static char devname (uint16_t num)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);
    switch (num) {
        case 1:
            return 'A';
        case 2:
            return 'B';
        case 4:
            return 'C';
        case 8:
            return 'D';
        case 16:
            return 'E';
        default:
            return -1;
    }
}

static uint16_t _decode(unsigned num)
{
    DEBUG("%s: enter\n", DEBUG_FUNC);
    if (num < 48) {
        puts("invalid data");
    }

    uint16_t data = 0;
    unsigned shift = 0;
    for (unsigned i = num - 48; i < num; i = i + 4) {
        uint8_t bit = 0;
        for (unsigned j=0; j < 4; ++j) {
            bit = bit | (((timings[i+j] < 500) ? 0 : 1) << j);
        }
        if (bit == 0x5) {
            data |= 1 << shift;
        }
        else if (bit == 0x3) {
            data |= 0 << shift;
        }
        else {
            return 1;
        }
        shift++;
    }
    return data;
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
        rcswitch_data_t rcdata;
        if (diff > FS1000A_RECV_THRESHOLD) {
            if (abs_diff(diff, timings[0]) < 200) {
                if (num_repeat++ > 1) {
                    rcdata.all = _decode(num_change);
                    char dname = devname(rcdata.bits.devnum);
                    if ((dname >= 'A') && (dname < 'F')) {
                        printf("%u:%c=%s\n", rcdata.bits.sysnum, dname,
                                             (rcdata.bits.onoff & 1) ? "ON" : "OFF");
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
