/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2024-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOweb
*/

#include "src/mygpiod.h"

#include "src/log.h"
#include "src/webserver.h"

#include <libmygpio/libmygpio.h>

/**
 * Polls myGPIOd socket
 * @param state pointer to state
 * @param mgr mongoose mgr
 */
void mygpiod_poll(struct t_state *state, struct mg_mgr *mgr) {
    if (state->npfds > 0) {
        errno = 0;
        int revents = poll(state->pfds, 1, 50);
        if (revents > 0) {
            if (revents & POLLIN) {
                PRINT_LOG_DEBUG("Events waiting");
                mygpiod_event_handler(state, mgr);
                if (mygpio_send_idle(state->conn) == false) {
                    PRINT_LOG_ERROR("Unable to send idle command");
                }
                if (mygpiod_check_error(state) == false) {
                    mygpiod_disconnect(state);
                }
            }
            else if (revents & (POLLERR | POLLNVAL | POLLHUP)) {
                PRINT_LOG_ERROR("Poll error: %s", strerror(errno));
                mygpiod_disconnect(state);
            }
        }
        else if (revents == -1) {
            PRINT_LOG_ERROR("Poll error: %s", strerror(errno));
            mygpiod_disconnect(state);
        }
    }
    else {
        time_t now = time(NULL);
        if (now > state->reconnect_time) {
            mygpiod_connect(state, 5000);
        }
    }
}

/**
 * Disconnects from myGPIOd socket, resets the poll fd's
 * and sets a reconnection timer
 * @param state pointer to state
 */
void mygpiod_disconnect(struct t_state *state) {
    if (state->conn != NULL) {
        PRINT_LOG_INFO("Disconnecting from myGPIOd");
        mygpio_connection_free(state->conn);
        state->conn = NULL;
    }
    state->pfds[0].fd = -1;
    state->npfds = 0;
    state->reconnect_time = time(NULL) + 5;
}

/**
 * Checks for a connection error state, tries to recover and
 * disconnects on fatal errors.
 * @param state pointer to state
 * @return true on MYGPIO_STATE_OK, else false
 */
bool mygpiod_check_error(struct t_state *state) {
    PRINT_LOG_DEBUG("Checking myGPIOd connection state");
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

/**
 * Connects to the myGPIOd socket and populates the poll fd's.
 * @param state pointer to state
 * @param timeout_ms connection timeout in ms
 * @return true on success, else false
 */
bool mygpiod_connect(struct t_state *state, int timeout_ms) {
    state->reconnect_time = time(NULL) + 5;
    if (state->conn != NULL && mygpio_connection_get_state(state->conn) == MYGPIO_STATE_FATAL) {
        PRINT_LOG_ERROR("myGPIOd connection in FATAL state");
        mygpiod_disconnect(state);
    }
    PRINT_LOG_INFO("Connecting to myGPIOd socket \"%s\"", state->socket);
    state->conn = mygpio_connection_new(state->socket, timeout_ms);
    if (state->conn == NULL) {
        PRINT_LOG_ERROR("Out of memory");
        return false;
    }
    if (mygpiod_check_error(state) == false) {
        return false;
    }
    PRINT_LOG_DEBUG("Entering idle mode");
    if (mygpio_send_idle(state->conn) == false) {
        PRINT_LOG_ERROR("Unable to send idle command");
    }
    if (mygpiod_check_error(state) == false) {
        mygpiod_disconnect(state);
        return false;
    }
    state->pfds[0].fd = mygpio_connection_get_fd(state->conn);
    state->npfds = 1;
    PRINT_LOG_INFO("Connected");
    return true;
}

/**
 * Reads idle events from the myGPIOd connection and broadcasts it
 * to all connected websocket clients.
 * @param state pointer to state
 * @param mgr pointer to mongoose mgr
 */
void mygpiod_event_handler(struct t_state *state, struct mg_mgr *mgr) {
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
}
