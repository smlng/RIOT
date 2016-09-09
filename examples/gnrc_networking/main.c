/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "shell.h"
#include "msg.h"

#define MAIN_QUEUE_SIZE     (8)
#ifndef DEFAULT_TX_POWER
#define DEFAULT_TX_POWER    (0)
#endif

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int udp_cmd(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "udp", "send data over UDP and listen on UDP ports", udp_cmd },
    { NULL, NULL, NULL }
};

static void _set_tx_power(void)
{
    kernel_pid_t ifs[GNRC_NETIF_NUMOF];
    int16_t tx_power = DEFAULT_TX_POWER;
    /* get the PID of the first radio */
    if (gnrc_netif_get(ifs) <= 0) {
        puts("ERROR: comm init, not radio found!\n");
        return;
    }
    gnrc_netapi_set(ifs[0], NETOPT_TX_POWER, 0, (int16_t *)&tx_power, 2);
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("RIOT network stack example application");
    _set_tx_power();
    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
