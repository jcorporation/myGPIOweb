/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "src/webserver.h"

#include "src/api.h"
#include "src/log.h"
#include "src/mygpiod.h"

/**
 * Broadcasts a gpio event to all connected websocket clients
 * @param mgr mongoose mgr
 * @param gpio gpio number
 * @param event event to emit
 * @param ts_ms timestamp of the event
 */
void websocket_broadcast(struct mg_mgr *mgr, unsigned gpio, enum mygpio_event event, uint64_t ts_ms) {
    struct mg_connection *nc = mgr->conns;
    while (nc != NULL) {
        if (nc->is_websocket == 1U) {
            PRINT_LOG_DEBUG("Send event to connection id: %lu", nc->id);
            mg_ws_printf(nc, WEBSOCKET_OP_TEXT, "{\"gpio\":%u,\"event\":\"%s\",\"ts_ms\":%llu}",
                gpio, mygpio_lookup_event(event), (unsigned long long)ts_ms);
        }
        nc = nc->next;
    }
}

/**
 * Handles the REST API requests
 * @param state pointer to state
 * @param nc client connection
 * @param hm mongoose http message struct
 */
void api_handler(struct t_state *state, struct mg_connection *nc, struct mg_http_message *hm) {
    bool rc = false;
    sds buffer = sdsempty();
    if (state->conn == NULL) {
        buffer = sdscat(buffer,"{\"error\":\"Not connected to myGPIOd\"}");
    }
    else {
        PRINT_LOG_DEBUG("Leaving idle mode");
        if (mygpio_send_noidle(state->conn) == false ||
            mygpiod_check_error(state) == false)
        {
            PRINT_LOG_ERROR("Error exiting idle mode");
            mygpiod_disconnect(state);
        }
        else {
            mygpiod_event_handler(state, nc->mgr);
            struct mg_str caps[2];
            if (mg_http_match_uri(hm, "/api/gpio") == true &&
                mg_vcmp(&hm->method, "GET") == 0)
            {
                buffer = api_gpio_get(state, buffer, &rc);
            }
            else if (mg_match(hm->uri, mg_str("/api/gpio/*"), caps) == true) {
                unsigned gpio = (unsigned)strtoul(caps[0].ptr, NULL, 10);
                if (gpio < 99) {
                    if (mg_vcmp(&hm->method, "GET") == 0) {
                        buffer = api_gpio_gpio_get(state, buffer, gpio, &rc);
                    }
                    else if (mg_vcmp(&hm->method, "OPTIONS") == 0) {
                        buffer = api_gpio_gpio_options(state, buffer, gpio, &rc);
                    }
                    else if (mg_vcmp(&hm->method, "POST") == 0) {
                        buffer = api_gpio_gpio_post(state, buffer, gpio, hm, &rc);
                    }
                }
                else {
                    PRINT_LOG_ERROR("Invalid gpio");
                }
            }
            PRINT_LOG_DEBUG("Entering idle mode");
            if (mygpio_send_idle(state->conn) == false) {
                PRINT_LOG_ERROR("Unable to send idle command");
            }
            if (mygpiod_check_error(state) == false) {
                mygpiod_disconnect(state);
                rc = false;
            }
        }
        if (sdslen(buffer) == 0) {
            buffer = sdscat(buffer,"{\"error\":\"Invalid API request\"}");
        }
    }
    mg_http_reply(nc, (rc == true ? 200 : 500), "Content-Type: application/json\r\n",
        "%s", buffer);
    sdsfree(buffer);
}

/**
 * Central mongoose event handler.
 * @param nc client connection
 * @param ev event type
 * @param ev_data event data
 * @param fn_data user data (unused)
 */
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
