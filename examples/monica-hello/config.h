#ifndef CONFIG_H
#define CONFIG_H

#include "xtimer.h"

#define CONFIG_PROXY_ADDR_ULA       "fd16:abcd:ef21:3::1"
#define CONFIG_PROXY_ADDR_LL        "fe80::1ac0:ffee:c0ff:ee21"
#define CONFIG_PROXY_ADDR_MC        "ff02::1"
#define CONFIG_PROXY_ADDR           CONFIG_PROXY_ADDR_MC
#define CONFIG_PROXY_PORT           "5683"

#define CONFIG_PATH_TEMPERATURE     "/temperature"
#define CONFIG_PATH_HUMITIDY        "/humidity"
#define CONFIG_LOOP_WAIT            (30U)
#define CONFIG_STRBUF_LEN           (32U)

#define SENSOR_TIMEOUT              (3U)
#define SENSOR_NUM_SAMPLES          (10U)
#define SENSOR_THREAD_STACKSIZE     (2 * THREAD_STACKSIZE_DEFAULT)

void sensor_get_temperature(int16_t *t);
void sensor_get_humidity(int16_t *t);

size_t node_get_info(char *buf);

#endif /* CONFIG_H */
