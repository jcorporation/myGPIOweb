/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOweb (c) 2024-2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOweb
*/

#include "compile_time.h"
#include "src/options.h"

#include "src/log.h"
#include "src/state.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct option long_options[] = {
    {"socket",  required_argument, 0, 's'},
    {"listen",  required_argument, 0, 'l'},
    {"help",    no_argument,       0, 'h'},
    {"syslog",  no_argument,       0, 'o'},
    {"version", no_argument,       0, 'v'},
};

/**
 * Prints the command line usage information
 * @param config pointer to config struct
 * @param cmd argv[0] from main function
 */
static void print_usage(const char *cmd) {
    fprintf(stderr, "\nUsage: %s [OPTION]...\n\n"
                    "myGPIOweb %s\n"
                    "(c) 2020-2024 Juergen Mang <mail@jcgames.de>\n"
                    "https://github.com/jcorporation/myGPIOweb\n\n"
                    "Options:\n"
                    "  -s, --socket    myGPIOd socket (default: /run/mygpiod/socket)\n"
                    "  -l, --listen    HTTP listening address (default: http://0.0.0.0:8000)\n"
                    "  -o, --syslog    enable syslog logging (facility: daemon)\n"
                    "  -h, --help      displays this help\n"
                    "  -v, --version   displays this help\n\n",
        cmd, MYGPIOWEB_VERSION);
}

/**
 * Handles the command line arguments
 * @param state pointer to state
 * @param argc from main function
 * @param argv from main function
 * @return OPTIONS_RC_INVALID on error
 *         OPTIONS_RC_EXIT if myGPIOweb should exit
 *         OPTIONS_RC_OK if arguments are parsed successfully
 */
enum handle_options_rc handle_options(struct t_state *state, int argc, char **argv) {
    int n = 0;
    int option_index = 0;
    while ((n = getopt_long(argc, argv, "l:os:vh", long_options, &option_index)) != -1) { /* Flawfinder: ignore */
        switch(n) {
            case 'l':
                free(state->listen_on);
                state->listen_on = strdup(optarg);
                break;
            case 'o':
                log_to_syslog = true;
                break;
            case 's':
                free(state->socket);
                state->socket = strdup(optarg);
                break;
            case 'v':
            case 'h':
                print_usage(argv[0]);
                return OPTIONS_RC_EXIT;
            default:
                print_usage(argv[0]);
                return OPTIONS_RC_INVALID;
        }
    }
    return OPTIONS_RC_OK;
}
