/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef STATE_H
#define STATE_H

#include <poll.h>
#include <time.h>

struct t_state {
    struct t_mygpio_connection *conn;
    struct pollfd pfds[1];
    nfds_t npfds;
    time_t reconnect_time;
    char *socket;
    char *listen_on;
    char *webroot;
};

void state_clear(struct t_state *state);

#endif
