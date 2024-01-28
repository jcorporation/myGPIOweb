/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 * https://github.com/jcorporation/mympd
 */

#include "compile_time.h"

#include "dist/mongoose/mongoose.h"
#include "src/log.h"
#include "src/mygpiod.h"
#include "src/options.h"
#include "src/state.h"
#include "src/webserver.h"

#include <libmygpio/libmygpio.h>
#include <poll.h>

int main(int argc, char **argv) {
    #ifdef MYGPIOWEB_DEBUG
        set_loglevel(LOG_DEBUG);
    #else
        set_loglevel(LOG_INFO);
    #endif
    log_on_tty = isatty(fileno(stdout));
    log_to_syslog = false;
    bool rc = EXIT_FAILURE;

    (void)setvbuf(stdout, NULL, _IOLBF, 0);
    (void)setvbuf(stderr, NULL, _IOLBF, 0);

    struct t_state state = {
        .conn = NULL,
        .pfds[0].events = POLLIN,
        .pfds[0].fd = -1,
        .npfds = 0,
        .reconnect_time = 0,
        .socket = strdup("/run/mygpiod/socket"),
        .listen_on = strdup("http://0.0.0.0:8000"),
        .webroot = strdup(MYGPIOWEB_HTDOCS)
    };

    // command line option
    int handle_options_rc = handle_options(&state, argc, argv);
    switch(handle_options_rc) {
        case OPTIONS_RC_INVALID:
            // invalid option or error
            loglevel = LOG_ERR;
            goto cleanup;
        case OPTIONS_RC_EXIT:
            // valid option and exit
            loglevel = LOG_ERR;
            rc = EXIT_SUCCESS;
            goto cleanup;
    }

    // init webserver
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mgr.userdata = &state;
    if (mg_http_listen(&mgr, state.listen_on, webserver_handler, NULL) == NULL) {
        PRINT_LOG_ERROR("Unable to listen on %s", state.listen_on);
        goto cleanup;
    }
    PRINT_LOG_INFO("Listening on %s", state.listen_on);

    // connect to myGPIOd
    mygpiod_connect(&state, 5000);

    // main event loop
    for (;;) {
        mg_mgr_poll(&mgr, 50);
        mygpiod_poll(&state, &mgr);
    }
    rc = EXIT_SUCCESS;

    cleanup:
    mg_mgr_free(&mgr);
    mygpiod_disconnect(&state);
    state_clear(&state);
    return rc;
}
