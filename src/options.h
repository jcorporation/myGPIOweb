/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myMPD (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#include "src/state.h"

#include <stdbool.h>

enum handle_options_rc {
    OPTIONS_RC_INVALID = -1,
    OPTIONS_RC_OK = 0,
    OPTIONS_RC_EXIT = 1
};

enum handle_options_rc handle_options(struct t_state *state, int argc, char **argv);

#endif
