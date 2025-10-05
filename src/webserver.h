/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2024-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOweb
*/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "dist/mongoose/mongoose.h"
#include "src/state.h"

#include <libmygpio/libmygpio.h>

void websocket_broadcast(struct mg_mgr *mgr, unsigned gpio, enum mygpio_event event, uint64_t ts_ms);
void api_handler(struct t_state *state, struct mg_connection *nc, struct mg_http_message *hm);
void webserver_handler(struct mg_connection *nc, int ev, void *ev_data);

#endif
