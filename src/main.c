/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/mympd
 */

#include "dist/mongoose/mongoose.h"
#include "src/log.h"
#include "src/mygpiod.h"
#include "src/state.h"
#include "src/webserver.h"

#include <libmygpio/libmygpio.h>
#include <poll.h>

int main(void) {
    set_loglevel(LOG_DEBUG);
    log_on_tty = isatty(fileno(stdout));
    log_to_syslog = false;

    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    struct t_state state = {
        .conn = NULL,
        .pfds[0].events = POLLIN,
        .pfds[0].fd = -1,
        .npfds = 0,
        .reconnect_time = 0,
        .socket = strdup("/run/mygpiod/socket"),
        .listen_on = strdup("http://0.0.0.0:8000"),
        .webroot = strdup("htdocs")
    };

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mgr.userdata = &state;
    mygpiod_connect(&state, state.socket, 5000);
    if (mg_http_listen(&mgr, state.listen_on, webserver_handler, NULL) == NULL) {
        PRINT_LOG_ERROR("Unable to listen on %s", state.listen_on);
        return EXIT_FAILURE;
    }
    PRINT_LOG_INFO("Listening on %s", state.listen_on);
    for (;;) {
        mg_mgr_poll(&mgr, 50);
        if (state.npfds > 0) {
            errno = 0;
            int revents = poll(state.pfds, 1, 50);
            if (revents > 0) {
                if (revents & POLLIN) {
                    PRINT_LOG_DEBUG("Events waiting");
                    mygpiod_event_handler(&state, &mgr);
                    if (mygpio_send_idle(state.conn) == false) {
                        PRINT_LOG_ERROR("Unable to send idle command");
                    }
                    mygpiod_check_error(&state);
                }
                else if (revents & (POLLERR | POLLNVAL | POLLHUP)) {
                    mygpiod_disconnect(&state);
                }
            }
            else if (revents == -1) {
                PRINT_LOG_ERROR("Poll error: %s", strerror(errno));
                mygpiod_disconnect(&state);
            }
        }
        else {
            time_t now = time(NULL);
            if (now > state.reconnect_time) {
                mygpiod_connect(&state, state.socket, 5000);
            }
        }
    }
    mg_mgr_free(&mgr);
    mygpiod_disconnect(&state);
    return EXIT_SUCCESS;
}
