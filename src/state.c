/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "src/state.h"

#include <stdlib.h>

/**
 * Clears the state struct
 * @param state pointer to state
 */
void state_clear(struct t_state *state) {
    free(state->socket);
    free(state->listen_on);
    free(state->webroot);
}
