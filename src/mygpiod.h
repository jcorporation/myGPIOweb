/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef MYGPIOD_H
#define MYGPIOD_H

#include "dist/mongoose/mongoose.h"
#include "src/state.h"

#include <poll.h>
#include <stdbool.h>
#include <syslog.h>
#include <time.h>

void mygpiod_poll(struct t_state *state, struct mg_mgr *mgr);
void mygpiod_disconnect(struct t_state *state);
bool mygpiod_check_error(struct t_state *state);
bool mygpiod_connect(struct t_state *state, int timeout_ms);
void mygpiod_event_handler(struct t_state *state, struct mg_mgr *mgr);

#endif
