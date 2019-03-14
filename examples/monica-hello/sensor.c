/**
 * @ingroup     climote
 * @{
 *
 * @file
 * @brief       Implements sensor control
 *
 * @author      smlng <s@mlng.net>
 *
 * @}
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "periph_conf.h"
#include "mutex.h"
#include "thread.h"
#include "xtimer.h"

#ifdef MODULE_HDC1000
#include "hdc1000.h"
#include "hdc1000_params.h"
static hdc1000_t dev_hdc1000;
#endif

#ifdef MODULE_TMP006
#include "tmp006.h"
#include "tmp006_params.h"
static tmp006_t dev_tmp006;
#endif

#if !defined(MODULE_HDC1000) || !defined(MODULE_TMP006)
#include "random.h"
#endif

#include "config.h"

#define ENABLE_DEBUG 1
#include "debug.h"

static int16_t s_humidity;
static int16_t s_temperature;
static mutex_t mutex;

static char sensor_thread_stack[SENSOR_THREAD_STACKSIZE];

/**
 * @brief get avg temperature over N samples in Celcius (C) with factor 100
 *
 * @return temperature
 */
void sensor_get_temperature(int16_t *t)
{
    DEBUG("[SENSOR] %s\n", __func__);
    mutex_lock(&mutex);
    *t = s_temperature;
    mutex_unlock(&mutex);
}

/**
 * @brief get avg humitity over N sampels in percent (%) with factor 100
 *
 * @return humidity
 */
void sensor_get_humidity(int16_t *h)
{
    DEBUG("[SENSOR] %s\n", __func__);
    mutex_lock(&mutex);
    *h = s_humidity;
    mutex_unlock(&mutex);
}

/**
 * @brief Measures the temperature and humitity with a HDC1000.
 *
 * @param[out] temp the measured temperature in degree celsius * 100
 * @param[out] hum the measured humitity in % * 100
 */
static int16_t _get_humidity(void) {
    DEBUG("[SENSOR] %s\n", __func__);
    int16_t h;
#ifdef MODULE_HDC1000
    int16_t t;
    hdc1000_read(&dev_hdc1000, &t, &h);
#else
    h = (int16_t) random_uint32_range(0, 10000);
#endif /* MODULE_HDC1000 */
    return h;
}

/**
 * @brief Measures the temperature with a TMP006.
 *
 * @param[out] temp the measured temperature in degree celsius * 100
 */
static int16_t _get_temperature(void)
{
    DEBUG("[SENSOR] %s\n", __func__);
    int16_t to;
#ifdef MODULE_TMP006
    int16_t ta;
    /* read sensor, quit on error */
    if (tmp006_read_temperature(&dev_tmp006, &ta, &to)) {
        DEBUG("[SENSOR] ERROR: tmp006_read_temperature failed!\n");
        return 0;
    }
#else
    to = (int16_t) random_uint32_range(0, 5000);
#endif /* MODULE_TMP006 */
    return to;
}

/**
 * @brief Intialise all sensores.
 *
 * @return 0 on success, anything else on error
 */
static int _init(void) {
    DEBUG("[SENSOR] %s\n", __func__);
#ifdef MODULE_HDC1000
    /* initialise humidity sensor hdc1000 */
    if ((hdc1000_init(&dev_hdc1000, &hdc1000_params[0]) != 0)) {
        DEBUG("[SENSOR] ERROR: hdc1000_init failed!\n");
        return 1;
    }
#endif /* MODULE_HDC1000 */
#ifdef MODULE_TMP006
    /* init temperature sensor tmp006 */
    if ((tmp006_init(&dev_tmp006, &tmp006_params[0]) != 0)) {
        DEBUG("[SENSOR] ERROR: tmp006_init failed!\n");
        return 1;
    }
    if (tmp006_set_active(&dev_tmp006)) {
        DEBUG("[SENSOR] ERROR: tmp006_set_active failed!\n");
        return 1;
    }
#endif /* MODULE_TMP006 */
    xtimer_sleep(SENSOR_TIMEOUT);
    mutex_lock(&mutex);
    s_humidity = _get_humidity();
    s_temperature = _get_temperature();
    mutex_unlock(&mutex);
    return 0;
}

/**
 * @brief udp receiver thread function
 *
 * @param[in] arg   unused
 */
static void *sensor_thread(void *arg)
{
    (void) arg;
    unsigned count = 0;

    while(1) {
        count = (count + 1) % SENSOR_NUM_SAMPLES;
        xtimer_sleep(SENSOR_TIMEOUT);
        /* get latest sensor data */
        mutex_lock(&mutex);
        s_humidity = (s_humidity + _get_humidity()) / 2;
        s_temperature = (s_temperature + _get_temperature()) / 2;
        mutex_unlock(&mutex);
        /* some Info message */
        if (!count) {
            printf("[SENSOR] INFO: H=%d, T=%d\n", s_humidity, s_temperature);
        }
    }
    /* should never be reached */
    return NULL;
}

/**
 * @brief start udp receiver thread
 *
 * @return PID of sensor control thread
 */
int sensor_init(void)
{
    /* init sensors */
    if (_init() != 0) {
        return -1;
    }
    /* start sensor thread for periodic measurements */
    return thread_create(sensor_thread_stack, sizeof(sensor_thread_stack),
                         THREAD_PRIORITY_MAIN-1, THREAD_CREATE_STACKTEST,
                         sensor_thread, NULL, "sensor");
}
