/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef API_H
#define API_H

#include "dist/sds/sds.h"
#include "src/state.h"

#include <stdbool.h>

sds api_gpiolist(struct t_state *state, sds buffer, bool *rc);

#endif
