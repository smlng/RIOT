#include "msg.h"
#include "thread.h"
#include "net/gcoap.h"
// own
#include "config.h"

#define ENABLE_DEBUG 1
#include "debug.h"

static ssize_t _info_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _climate_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

/* CoAP resources */
static const coap_resource_t _resources[] = {
    { "/lgv/climate", COAP_GET, _climate_handler, NULL },
    { "/lgv/info", COAP_GET, _info_handler, NULL },
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL
};

static sock_udp_ep_t remote;
/*
 * Response callback.
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu,
                          sock_udp_ep_t *remote)
{
    DEBUG("[CoAP] %s\n", __func__);
    (void)remote;       /* not interested in the source currently */

    if (req_state == GCOAP_MEMO_TIMEOUT) {
        DEBUG("[CoAP] timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        DEBUG("[CoAP] error in response\n");
        return;
    }
}

static size_t _send(uint8_t *buf, size_t len)
{
    DEBUG("[CoAP] %s\n", __func__);

    return gcoap_req_send2(buf, len, &remote, _resp_handler);;
}

/*
 * Server callback for /cli/stats. Returns the count of packets sent by the
 * CLI.
 */
static ssize_t _info_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    DEBUG("[CoAP] %s\n", __func__);
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

    size_t payload_len = node_get_info((char *)pdu->payload);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_JSON);
}

/*
 * Server callback for /cli/stats. Returns the count of packets sent by the
 * CLI.
 */
static ssize_t _climate_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    DEBUG("[CoAP] %s\n", __func__);
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    int16_t h, t;
    sensor_get_humidity(&h);
    sensor_get_temperature(&t);
    size_t payload_len = sprintf((char *)pdu->payload, "{'temperature': %d, 'humidity': %d}", (int)t, (int)h);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_JSON);
}

void post_sensordata(char *data, char *path)
{
    DEBUG("[CoAP] %s\n", __func__);
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;

    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, path);
    memcpy(pdu.payload, data, strlen(data));
    len = gcoap_finish(&pdu, strlen(data), COAP_FORMAT_JSON);
    if (_send(&buf[0], len) <= 0) {
        DEBUG("[CoAP] ERROR: send failed");
    }
}

/**
 * @brief start CoAP thread
 */
int coap_init(void)
{
    ipv6_addr_t addr;

    remote.family = AF_INET6;
    remote.netif  = SOCK_ADDR_ANY_NETIF;

    /* parse destination address */
    if (ipv6_addr_from_str(&addr, CONFIG_PROXY_ADDR) == NULL) {
        DEBUG("[CoAP] ERROR: unable to parse destination address");
        return 0;
    }
    memcpy(&remote.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    /* parse port */
    remote.port = (uint16_t)atoi(CONFIG_PROXY_PORT);
    if (remote.port == 0) {
        DEBUG("[CoAP] ERROR: unable to parse destination port");
        return 0;
    }
    /* init listener for own resources */
    gcoap_register_listener(&_listener);
    return 0;
}
