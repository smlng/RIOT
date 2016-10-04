/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 */

#include "net/gnrc/ipv6.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/sixlowpan/ctx.h"
#include "net/gnrc/sixlowpan/nd.h"
#include "net/icmpv6.h"
#include "net/ndp.h"
#include "net/sixlowpan/nd.h"

#include "net/gnrc/sixlowpan/nd/router.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#if ENABLE_DEBUG
static char addr_str[IPV6_ADDR_MAX_STR_LEN];
#endif

static gnrc_sixlowpan_nd_router_abr_t _abrs[GNRC_SIXLOWPAN_ND_ROUTER_ABR_NUMOF];
static gnrc_sixlowpan_nd_router_prf_t _prefixes[GNRC_SIXLOWPAN_ND_ROUTER_ABR_PRF_NUMOF];

static gnrc_sixlowpan_nd_router_abr_t *_get_abr(const ipv6_addr_t *addr)
{
    DEBUG("6lo nd router: get ABRO for  %s.\n",
          ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)));
    for (int i = 0; i < GNRC_SIXLOWPAN_ND_ROUTER_ABR_NUMOF; i++) {
        if (ipv6_addr_equal(&_abrs[i].addr, addr)) {
            return &_abrs[i];
        }
        else if (ipv6_addr_is_unspecified(&_abrs[i].addr)) {
            /* unused abr found, fresh init */
            DEBUG(" -- init new ABRO entry\n");
            abr->addr.u64[0] = addr->u64[0];
            abr->addr.u64[1] = addr->u64[1];
            abr->ltime = 0;
            abr->version = 0;
            abr->prfs = NULL;
            memset(abr->ctxs, 0, sizeof(abr->ctxs));
            return &_abrs[i];
        }
    }
    return NULL;
}

static gnrc_sixlowpan_nd_router_prf_t *_get_prefix(gnrc_ipv6_netif_t *ipv6_iface,
                                                   gnrc_ipv6_netif_addr_t *prefix)
{
    DEBUG("6lo nd router: get prefix for  %s.\n",
          ipv6_addr_to_str(addr_str, &(prefix->addr), sizeof(addr_str)));

    for (int i = 0; i < GNRC_SIXLOWPAN_ND_ROUTER_ABR_PRF_NUMOF; i++) {
        if ((ipv6_addr_match_prefix(&(_prefixes[i].prefix->addr),
                                    &(prefix->addr)) >= prefix->prefix_len) &&
            (_prefixes[i].prefix->prefix_len == prefix->prefix_len) &&
            (_prefixes[i].iface == ipv6_iface)) {
            return &_prefixes[i];
        }

        if (ipv6_addr_is_unspecified(&_prefixes[i].prefix->addr)) {
            /* init prefix */
            DEBUG(" -- init new ABRO prefix\n");
            _prefixes[i].iface = ipv6_iface;
            _prefixes[i].prefix = prefix;
            return &_prefixes[i];
        }
    }

    return NULL;
}

static void _abr_add_prefix(kernel_pid_t iface,
                            gnrc_sixlowpan_nd_router_abr_t *abr,
                            ndp_opt_pi_t *pi_opt)
{
    gnrc_ipv6_netif_t *ipv6_iface = gnrc_ipv6_netif_get(iface);
    ipv6_addr_t *prefix;
    DEBUG("6lo nd router: add prefix to ABRO\n");
    DEBUG(" -- ABRO %s\n", ipv6_addr_to_str(addr_str, &(abr->addr), sizeof(addr_str)));
    DEBUG(" -- prefix %s\n", ipv6_addr_to_str(addr_str, &(pi_opt->prefix.addr), sizeof(addr_str)));
    /* check PIO */
    if ((pi_opt->len != NDP_OPT_PI_LEN) ||
        ipv6_addr_is_link_local(&pi_opt->prefix) ||
        (pi_opt->flags & NDP_OPT_PI_FLAGS_A) ||
        (pi_opt->flags & NDP_OPT_PI_FLAGS_L) ||
        (pi_opt->valid_ltime.u32 == 0)) {
        return;
    }
    /* find prefix storage */
    if((prefix = gnrc_ipv6_netif_match_prefix(iface, &pi_opt->prefix)) != NULL) {
        gnrc_sixlowpan_nd_router_prf_t *prf_ent;
        prf_ent = _get_prefix(ipv6_iface, container_of(prefix, gnrc_ipv6_netif_addr_t, addr));
        if (prf_ent != NULL) {
            DEBUG(" -- SUCCESS\n");
            LL_PREPEND(abr->prfs, prf_ent);
        }
    }
}

static void _abr_add_ctx(gnrc_sixlowpan_nd_router_abr_t *abr, sixlowpan_nd_opt_6ctx_t *ctx_opt)
{
    DEBUG("6lo nd router: add CTX to ABRO\n");
    DEBUG(" -- ABRO %s\n", ipv6_addr_to_str(addr_str, &(abr->addr), sizeof(addr_str)));
    if (((ctx_opt->ctx_len < 64) && (ctx_opt->len != 2)) ||
        ((ctx_opt->ctx_len >= 64) && (ctx_opt->len != 3))) {
        return;
    }
    bf_set(abr->ctxs, sixlowpan_nd_opt_6ctx_get_cid(ctx_opt));
    DEBUG(" -- SUCCESS\n");
}

#ifdef MODULE_GNRC_SIXLOWPAN_ND_BORDER_ROUTER
static inline bool _is_me(ipv6_addr_t *addr)
{
    ipv6_addr_t *res;

    return (gnrc_ipv6_netif_find_by_addr(&res, addr) != KERNEL_PID_UNDEF);
}
#else
#define _is_me(ignore)  (false)
#endif

void gnrc_sixlowpan_nd_router_set_rtr_adv(gnrc_ipv6_netif_t *netif, bool enable)
{
    if (enable && (gnrc_ipv6_netif_add_addr(netif->pid, &ipv6_addr_all_routers_link_local, 128,
                                            GNRC_IPV6_NETIF_ADDR_FLAGS_NON_UNICAST) != NULL)) {
        mutex_lock(&netif->mutex);
        netif->flags |= GNRC_IPV6_NETIF_FLAGS_RTR_ADV;
        netif->adv_ltime = GNRC_IPV6_NETIF_DEFAULT_ROUTER_LTIME;
#ifdef MODULE_GNRC_NDP_ROUTER
        /* for border router these values have to be initialized, too */
        netif->max_adv_int = GNRC_IPV6_NETIF_DEFAULT_MAX_ADV_INT;
        netif->min_adv_int = GNRC_IPV6_NETIF_DEFAULT_MIN_ADV_INT;
#endif
        mutex_unlock(&netif->mutex);
    }
    else {
        netif->flags &= ~GNRC_IPV6_NETIF_FLAGS_RTR_ADV;
        gnrc_ipv6_netif_remove_addr(netif->pid, (ipv6_addr_t *)&ipv6_addr_all_routers_link_local);
    }
}

gnrc_sixlowpan_nd_router_abr_t *gnrc_sixlowpan_nd_router_abr_get(void)
{
    if (ipv6_addr_is_unspecified(&_abrs[0].addr)) {
        return NULL;
    }
    return _abrs;
}

void gnrc_sixlowpan_nd_router_abr_remove(gnrc_sixlowpan_nd_router_abr_t *abr)
{
    for (int i = 0; i < GNRC_SIXLOWPAN_CTX_SIZE; i++) {
        if (bf_isset(abr->ctxs, i)) {
            gnrc_sixlowpan_ctx_remove(i);
            bf_unset(abr->ctxs, i);
        }
    }

    while (abr->prfs != NULL) {
        gnrc_sixlowpan_nd_router_prf_t *prefix = abr->prfs;
        LL_DELETE(abr->prfs, prefix);
        gnrc_ipv6_netif_remove_addr(prefix->iface->pid, &prefix->prefix->addr);
        prefix->next = NULL;
        prefix->iface = NULL;
        prefix->prefix = NULL;
    }
    ipv6_addr_set_unspecified(&abr->addr);
    abr->version = 0;
}

/* router-only functions from net/gnrc/sixlowpan/nd.h */
void gnrc_sixlowpan_nd_opt_abr_handle(kernel_pid_t iface, ndp_rtr_adv_t *rtr_adv, int sicmpv6_size,
                                      sixlowpan_nd_opt_abr_t *abr_opt)
{
    uint16_t opt_offset = 0;
    uint8_t *buf = (uint8_t *)(rtr_adv + 1);
    gnrc_sixlowpan_nd_router_abr_t *abr;
    uint32_t t = 0;
    uint32_t version;
    /* check wether this node is the 6Lo ABR */
    if (_is_me(&abr_opt->braddr)) {
        DEBUG("6lo nd router, opt_abr_handle: its me.\n");
        return;
    }
    /* get existing abro or a fresh one */
    if ((abr = _get_abr(&abr_opt->braddr)) == NULL) {
        DEBUG("6lo nd router, opt_abr_handle: cannot store ABRO.\n");
        return;
    }
    DEBUG("6lo nd router, opt_abr_handle: found ABRO for  %s.\n",
          ipv6_addr_to_str(addr_str, &abr->addr, sizeof(addr_str)));
    /* verify ABRO version is newer, than existing one */
    version = (uint32_t)byteorder_ntohs(abr_opt->vlow);
    version |= ((uint32_t)byteorder_ntohs(abr_opt->vhigh)) << 16;
    if (version < abr->version) {
        DEBUG("6lo nd router, opt_abr_handle: older version (%"PRIu32" < %"PRIu32").\n", version, abr->version);
        return;
    }
    abr->version = version;
    DEBUG("6lo nd router, opt_abr_handle: version %"PRIu32"\n", abr->version);
    /* set ABRO lifetime */
    abr->ltime = byteorder_ntohs(abr_opt->ltime);
    if (abr->ltime == 0) {
        abr->ltime = GNRC_SIXLOWPAN_ND_BORDER_ROUTER_DEFAULT_LTIME;
    }
    DEBUG("6lo nd router, opt_abr_handle: lifetime %"PRIu16"\n", abr->ltime);
    sicmpv6_size -= sizeof(ndp_rtr_adv_t);
    while (sicmpv6_size > 0) {
        ndp_opt_t *opt = (ndp_opt_t *)(buf + opt_offset);

        switch (opt->type) {
            case NDP_OPT_PI:
                _abr_add_prefix(iface, abr, (ndp_opt_pi_t *)opt);

            case NDP_OPT_6CTX:
                _abr_add_ctx(abr, (sixlowpan_nd_opt_6ctx_t *)opt);

            default:
                break;
        }

        opt_offset += (opt->len * 8);
        sicmpv6_size -= (opt->len * 8);
    }
    /* init abr removal timer */
    t = abr->ltime * 60 * SEC_IN_USEC;
    xtimer_remove(&abr->ltimer);
    abr->ltimer_msg.type = GNRC_SIXLOWPAN_ND_MSG_ABR_TIMEOUT;
    abr->ltimer_msg.content.ptr = abr;
    xtimer_set_msg(&abr->ltimer, t, &abr->ltimer_msg, gnrc_ipv6_pid);
}

gnrc_pktsnip_t *gnrc_sixlowpan_nd_opt_6ctx_build(uint8_t prefix_len, uint8_t flags, uint16_t ltime,
                                                 ipv6_addr_t *prefix, gnrc_pktsnip_t *next)
{
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_6CTX,
                                             sizeof(sixlowpan_nd_opt_6ctx_t) + (prefix_len / 8),
                                             next);

    if (pkt != NULL) {
        sixlowpan_nd_opt_6ctx_t *ctx_opt = pkt->data;
        ctx_opt->ctx_len = prefix_len;
        ctx_opt->resv_c_cid = flags;
        ctx_opt->resv.u16 = 0;
        ctx_opt->ltime = byteorder_htons(ltime);
        /* Bits beyond prefix_len MUST be 0 */
        memset(ctx_opt + 1, 0, pkt->size - sizeof(sixlowpan_nd_opt_6ctx_t));
        ipv6_addr_init_prefix((ipv6_addr_t *)(ctx_opt + 1), prefix, prefix_len);
    }

    return pkt;
}

gnrc_pktsnip_t *gnrc_sixlowpan_nd_opt_abr_build(uint32_t version, uint16_t ltime,
                                                ipv6_addr_t *braddr, gnrc_pktsnip_t *next)
{
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_ABR, sizeof(sixlowpan_nd_opt_abr_t), next);

    if (pkt != NULL) {
        sixlowpan_nd_opt_abr_t *abr_opt = pkt->data;
        abr_opt->vlow = byteorder_htons(version & 0xffff);
        abr_opt->vhigh = byteorder_htons(version >> 16);
        abr_opt->ltime = byteorder_htons(ltime);
        abr_opt->braddr.u64[0] = braddr->u64[0];
        abr_opt->braddr.u64[1] = braddr->u64[1];
#if ENABLE_DEBUG
        uint32_t tmpver;
        tmpver = (uint32_t)byteorder_ntohs(abr_opt->vlow);
        tmpver |= ((uint32_t)byteorder_ntohs(abr_opt->vhigh)) << 16;
#endif
        DEBUG("6lo nd router, opt_abr_build: for %s with version %"PRIu32" and ltime %"PRIu16".\n",
              ipv6_addr_to_str(addr_str, &abr_opt->braddr, sizeof(addr_str)), tmpver, byteorder_ntohs(abr_opt->ltime));
    }

    return pkt;
}

#ifdef MODULE_GNRC_SIXLOWPAN_ND_BORDER_ROUTER
gnrc_sixlowpan_nd_router_abr_t *gnrc_sixlowpan_nd_router_abr_create(ipv6_addr_t *addr,
                                                                    unsigned int ltime)
{
    assert(addr != NULL);
    gnrc_sixlowpan_nd_router_abr_t *abr = _get_abr(addr);
    if (abr == NULL) {
        return NULL;
    }
    /* TODO: store and get this somewhere stable */
    abr->version = 0;
    abr->ltime = (uint16_t)ltime;
    return abr;
}

int gnrc_sixlowpan_nd_router_abr_add_prf(gnrc_sixlowpan_nd_router_abr_t* abr,
                                         gnrc_ipv6_netif_t *iface, gnrc_ipv6_netif_addr_t *prefix)
{
    assert((iface != NULL) && (prefix != NULL));
    gnrc_sixlowpan_nd_router_prf_t *prf_ent;
    if ((abr < _abrs) || (abr > (_abrs + GNRC_SIXLOWPAN_ND_ROUTER_ABR_NUMOF))) {
        return -ENOENT;
    }
    prf_ent = _get_prefix(iface, prefix);
    if (prf_ent == NULL) {
        return -ENOMEM;
    }
    /* add prefix to abr */
    LL_PREPEND(abr->prfs, prf_ent);
    abr->version++; /* TODO: store somewhere stable */

    return 0;
}


void gnrc_sixlowpan_nd_router_abr_rem_prf(gnrc_sixlowpan_nd_router_abr_t *abr,
                                          gnrc_ipv6_netif_t *iface, gnrc_ipv6_netif_addr_t *prefix)
{
    assert((iface != NULL) && (prefix != NULL));
    gnrc_sixlowpan_nd_router_prf_t *prf_ent = abr->prfs, *prev = NULL;
    if ((abr < _abrs) || (abr > (_abrs + GNRC_SIXLOWPAN_ND_ROUTER_ABR_NUMOF))) {
        return;
    }
    while (prf_ent) {
        if (prf_ent->prefix == prefix) {
            if (prev == NULL) {
                abr->prfs = prf_ent->next;
            }
            else {
                prev->next = prf_ent->next;
            }
            prf_ent->next = NULL;
            prf_ent->iface = NULL;
            prf_ent->prefix = NULL;
            abr->version++; /* TODO: store somewhere stable */
            break;
        }
        prev = prf_ent;
        prf_ent = prf_ent->next;
    }
}

int gnrc_sixlowpan_nd_router_abr_add_ctx(gnrc_sixlowpan_nd_router_abr_t *abr, uint8_t cid)
{
    if ((abr < _abrs) || (abr > (_abrs + GNRC_SIXLOWPAN_ND_ROUTER_ABR_NUMOF))) {
        return -ENOENT;
    }
    if (cid >= GNRC_SIXLOWPAN_CTX_SIZE) {
        return -EINVAL;
    }
    if (bf_isset(abr->ctxs, cid)) {
        return -EADDRINUSE;
    }
    bf_set(abr->ctxs, cid);
    abr->version++; /* TODO: store somewhere stable */
    return 0;
}

void gnrc_sixlowpan_nd_router_abr_rem_ctx(gnrc_sixlowpan_nd_router_abr_t *abr, uint8_t cid)
{
    if ((abr < _abrs) || (abr > (_abrs + GNRC_SIXLOWPAN_ND_ROUTER_ABR_NUMOF))) {
        return;
    }
    if (cid >= GNRC_SIXLOWPAN_CTX_SIZE) {
        return;
    }
    bf_unset(abr->ctxs, cid);
    abr->version++; /* TODO: store somewhere stable */
    return;
}
#endif
/** @} */
