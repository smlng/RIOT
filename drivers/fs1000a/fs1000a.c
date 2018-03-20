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

#include "log.h"
#include "fs1000a.h"
#include "fmt.h"
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

static inline uint32_t abs_diff(uint32_t a, uint32_t b)
{
    return  (a < b) ? (b - a) : (a - b);
}

#define MAX(a,b)    ((a > b) ? a : b)
#define MIN(a,b)    ((a < b) ? a : b)

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
    m.content.value = (uint32_t)(now - last);

    if (rt > KERNEL_PID_UNDEF) {
        msg_send(&m, rt);
    }

    last = now;
}

static uint32_t _decode_4bits(const uint8_t *inbuf, size_t inlen)
{
    LOG_DEBUG("%s: enter (inlen=%d)\n", DEBUG_FUNC, (int)inlen);

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
#if 0
static int _decode_2bits(const uint8_t *inbuf, size_t inlen,
                         uint64_t *outbuf, size_t outlen)
{
    (void)outbuf;
    (void)outlen;

    DEBUG("%s: enter (inlen=%d)\n", DEBUG_FUNC, (int)inlen);
    /*
    unsigned outpos = 0;
    uint64_t tmp = 0;
    unsigned tmppos = 0;
    bool parsing = false;
    */
    for (size_t i = 0; i < inlen; ++i) {
        for (unsigned j = 0; j < 8; j += 2) {
            uint8_t val = (inbuf[i] >>j) & 0x3;
            if (val > 0) {
                val--;
                DEBUG("%u", val);
            }
            else {
                DEBUG("-");
            }
        }
    }
    DEBUG("\n");
    return 0;
}
#endif
static int _decode_plain(uint32_t threshhold,
                         const uint32_t *inbuf, size_t inlen,
                         uint8_t *outbuf, size_t outlen)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);

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

static int _decode_plain2(uint32_t t1, uint32_t t2, size_t inpos,
                         const uint32_t *inbuf, size_t inlen,
                         uint64_t *outbuf, size_t outlen)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);

    uint64_t u64 = 0;
    unsigned last = 2;
    unsigned shift = 0;
    unsigned outpos = 0;
    for (size_t i = 0; i < inlen; ++i) {
        size_t pos = (inpos + i) % inlen;
        unsigned val = 0;
        if (inbuf[pos] > t1) {
            val = 1;
        }
        if (inbuf[pos] > t2) {
            val = 2;
            u64 = 0;
            shift = 0;
        }
        if ((val < 2) && (last < 2)) {
            if ((last == 1) && (val == 0)) {
                u64 |= 1ULL << shift;
            }
            last = 2;
            ++shift;
        }
        else {
            last = val;
        }
        DEBUG("%d", val);

        if ((shift == 64) && (outbuf != NULL) &&
            (outlen > 0) && (outpos < outlen) && (u64 > 0)) {
            outbuf[outpos++] = u64;
            shift = 0;
            u64 = 0;
        }
    }
    DEBUG("\n");
    return outpos;
}

void *_receiver(void *arg)
{
    (void) arg;
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);

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
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);

    while (1) {
        msg_t m;
        msg_receive(&m);
        printf("%" PRIu32 "\n", m.content.value);
    }
}


#define BUFLEN              (2048U)
#define DCBLEN              (32U)
#define SENSOR_THRESHOLD    (100000UL)

static uint32_t buffer[BUFLEN];
static uint64_t outbuf[DCBLEN];

void *_recv_sensor_data(void *arg)
{
    (void)arg;
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);

    msg_init_queue(recv_queue, RECV_QUEUE_LEN);

    memset(buffer, 0, BUFLEN);
    unsigned counter = 0;
    while (1) {
        msg_t m;
        msg_receive(&m);
        uint32_t val = m.content.value;
        size_t pos = counter % BUFLEN;
        buffer[pos] = val;
        counter++;
        if (val > SENSOR_THRESHOLD) {
            memset(outbuf, 0, DCBLEN);
            unsigned ret = _decode_plain2(380, 580, pos, buffer, BUFLEN, outbuf, DCBLEN);
            if (ret > 0) {
                DEBUG("Decoded values U64 (%u):\n", ret);
                for (unsigned i = 0; i < ret; ++i) {
                    print_u64_hex(outbuf[i]);
                    puts("");
                }
                DEBUG("===================\n");
            }
        }
    }
}

int fs1000a_init(fs1000a_t *dev, const fs1000a_params_t *params)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);

    memcpy(&dev->p, params, sizeof(fs1000a_params_t));

    return 0;
}

static int _run_background_thread(const fs1000a_t *dev, void* func)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);
    if ((dev->p.recv_pin != GPIO_UNDEF) && (rt <= KERNEL_PID_UNDEF)) {
        rt = thread_create(recv_stack, sizeof(recv_stack),
                                       THREAD_PRIORITY_MAIN - 1,
                                       THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                                       func, NULL, "fs1000a");
        if (rt < 0) {
            LOG_ERROR("%s: thread_create failed!\n", DEBUG_FUNC);
            return -1;
        }
        if (gpio_init_int(dev->p.recv_pin, GPIO_IN, GPIO_BOTH, _recv_cb, NULL) < 0) {
            LOG_ERROR("%s: gpio_init_int failed!\n", DEBUG_FUNC);
            return -2;
        }
    }
    return 0;
}

int fs1000a_enable_switch_receive(const fs1000a_t *dev)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);
    return _run_background_thread(dev, _receiver);
}

int fs1000a_enable_sniffer(const fs1000a_t *dev)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);
    return _run_background_thread(dev, _sniffer);
}

int fs1000a_enable_sensor_receive(const fs1000a_t *dev)
{
    LOG_DEBUG("%s: enter\n", DEBUG_FUNC);
    return _run_background_thread(dev, _recv_sensor_data);
}

#define NUM_SAMPLES (256U)
int fs1000a_analyse_spectrum(const fs1000a_t *dev)
{
    msg_init_queue(recv_queue, RECV_QUEUE_LEN);
    if (rt > KERNEL_PID_UNDEF) {
        LOG_ERROR("ERROR: another background thread is running!\n");
        return (-1);
    }
    rt = thread_getpid();
    if (gpio_init_int(dev->p.recv_pin, GPIO_IN, GPIO_BOTH, _recv_cb, NULL) < 0) {
        LOG_ERROR("%s: gpio_init_int failed!\n", DEBUG_FUNC);
        return -2;
    }
    /* stats: max_now, max_all, min_now, min_all, avg_now, avg_all */
    //uint32_t samples[NUM_SAMPLES];
    uint32_t count, max, min, avg = 0;
    max = avg = count = 0;
    min = 0xffffffff;
    while(++count) {
        if ((count % NUM_SAMPLES) == 0) {
            //memset(samples, 0, NUM_SAMPLES);
            printf("%"PRIu32",%"PRIu32",%"PRIu32",%"PRIu32"\n", count, max, avg, min);
            max = 0;
            avg = 0;
            min = 0xffffffff;
        }
        msg_t m;
        msg_receive(&m);
        uint32_t val = m.content.value;
        //samples[count] = val;
        max = MAX(val, max);
        min = MIN(val, min);
        avg = (avg + val) / 2;
    }
    return 0;
}
