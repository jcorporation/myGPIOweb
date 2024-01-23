/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef STATE_H
#define STATE_H

#include <poll.h>
#include <time.h>

/**
 * Struct holding central data and states
 */
struct t_state {
    struct t_mygpio_connection *conn;  //!< myGPIOD connection
    struct pollfd pfds[1];             //!< fd's for polling
    nfds_t npfds;                      //!< number of fd's to poll
    time_t reconnect_time;             //!< timestamp for next reconnection attempt to myGPIOd
    char *socket;                      //!< myGPIOd socket path
    char *listen_on;                   //!< mongoose listening uri
    char *webroot;                     //!< mongoose document root
};

void state_clear(struct t_state *state);

#endif
