/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "src/mygpiod.h"

#include "src/log.h"
#include "src/webserver.h"

#include <libmygpio/libmygpio.h>

void mygpiod_disconnect(struct t_state *state) {
    PRINT_LOG_INFO("Disconnecting from myGPIOd");
    if (state->conn != NULL) {
        mygpio_connection_free(state->conn);
        state->conn = NULL;
    }
    state->pfds[0].fd = -1;
    state->npfds = 0;
    state->reconnect_time = time(NULL) + 5;
}

bool mygpiod_check_error(struct t_state *state) {
    enum mygpio_conn_state conn_state = mygpio_connection_get_state(state->conn);
    if (conn_state == MYGPIO_STATE_OK) {
        return true;
    }
    const char *err_str = conn_state == MYGPIO_STATE_FATAL ? "fatal" : "error";
    PRINT_LOG_ERROR("myGPIOd %s: %s", err_str, mygpio_connection_get_error(state->conn));
    if (conn_state == MYGPIO_STATE_ERROR) {
        mygpio_connection_clear_error(state->conn);
        mygpio_response_end(state->conn);
        return false;
    }
    mygpiod_disconnect(state);
    return false;
}

bool mygpiod_connect(struct t_state *state, const char *socket, int timeout_ms) {
    state->reconnect_time = time(NULL) + 5;
    if (state->conn != NULL && mygpio_connection_get_state(state->conn) == MYGPIO_STATE_FATAL) {
        PRINT_LOG_ERROR("myGPIOd connection in FATAL state");
        mygpiod_disconnect(state);
    }
    PRINT_LOG_INFO("Connecting to myGPIOd socket \"%s\"", socket);
    state->conn = mygpio_connection_new(socket, timeout_ms);
    if (state->conn == NULL) {
        PRINT_LOG_ERROR("Out of memory");
        return false;
    }
    if (mygpiod_check_error(state) == false) {
        return false;
    }
    if (mygpio_send_idle(state->conn) == false) {
        PRINT_LOG_ERROR("Unable to send idle command");
    }
    if (mygpiod_check_error(state) == false) {
        mygpiod_disconnect(state);
        return false;
    }
    state->pfds[0].fd = mygpio_connection_get_fd(state->conn);
    state->npfds = 1;
    mygpio_response_end(state->conn);
    return true;
}

bool mygpiod_event_handler(struct t_state *state, struct mg_mgr *mgr) {
    PRINT_LOG_INFO("Receiving events");
    struct t_mygpio_idle_event *event;
    while ((event = mygpio_recv_idle_event(state->conn)) != NULL) {
        PRINT_LOG_DEBUG("Received event for gpio %u", mygpio_idle_event_get_gpio(event));
        websocket_broadcast(mgr,
            mygpio_idle_event_get_gpio(event),
            mygpio_idle_event_get_event(event),
            mygpio_idle_event_get_timestamp_ms(event)
        );
        mygpio_free_idle_event(event);
    }
    mygpio_response_end(state->conn);
    return true;
}
