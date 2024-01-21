/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "src/webserver.h"

#include "src/api.h"
#include "src/log.h"
#include "src/mygpiod.h"

void websocket_broadcast(struct mg_mgr *mgr, unsigned gpio, enum mygpio_event event, uint64_t ts_ms) {
    struct mg_connection *nc = mgr->conns;
    while (nc != NULL) {
        if (nc->is_websocket == 1U) {
            PRINT_LOG_DEBUG("Send event to connection id: %lu", nc->id);
            mg_ws_printf(nc, WEBSOCKET_OP_TEXT, "{\"gpio\":%u,\"event\":%s,\"ts_ms\":%llu}",
                gpio, mygpio_lookup_event(event), (unsigned long long) ts_ms);
        }
        nc = nc->next;
    }
}

void api_handler(struct t_state *state, struct mg_connection *nc, struct mg_http_message *hm) {
    PRINT_LOG_DEBUG("Leaving idle mode");
    if (mygpio_send_noidle(state->conn) == true) {
        mygpiod_event_handler(state, nc->mgr);
    }
    sds buffer = sdsempty();
    bool rc = false;
    if (mg_http_match_uri(hm, "/api/gpio") == true &&
        mg_vcmp(&hm->method, "GET") == 0)
    {
        buffer = api_gpiolist(state, buffer, &rc);
    }
    PRINT_LOG_DEBUG("Entering idle mode");
    if (mygpio_send_idle(state->conn) == false) {
        PRINT_LOG_ERROR("Unable to send idle command");
    }
    if (mygpiod_check_error(state) == false) {
        rc = false;
    }
    if (rc == true) {
        mg_http_reply(nc, 200, "Content-Type: application/json\r\n",
            "%s", buffer);
    }
    else {
        if (sdslen(buffer) == 0) {
            buffer = sdscat(buffer,"{\"error\":\"Invalid API request\"}");
        }
        mg_http_reply(nc, 500, "Content-Type: application/json\r\n",
            "%s", buffer);
    }
    sdsfree(buffer);
}

void webserver_handler(struct mg_connection *nc, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        struct t_state *state = (struct t_state *)nc->mgr->userdata;

        if (mg_http_match_uri(hm, "/ws")) {
            PRINT_LOG_INFO("Websocket handshake");
            mg_ws_upgrade(nc, hm, NULL);
        }
        else if (mg_http_match_uri(hm, "/api/#")) {
            PRINT_LOG_INFO("REST call %.*s %.*s", (int)hm->method.len, hm->method.ptr, (int)hm->uri.len, hm->uri.ptr);
            api_handler(state, nc, hm);
        }
        else {
            struct mg_http_serve_opts opts = {
                .root_dir = state->webroot
            };
            PRINT_LOG_INFO("Serving %.*s", (int)hm->uri.len, hm->uri.ptr);
            mg_http_serve_dir(nc, ev_data, &opts);
        }
    }
    (void) fn_data;
}
