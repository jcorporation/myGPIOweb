/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef API_H
#define API_H

#include "dist/mongoose/mongoose.h"
#include "dist/sds/sds.h"
#include "src/state.h"

#include <stdbool.h>

sds api_gpio_get(struct t_state *state, sds buffer, bool *rc);
sds api_gpio_gpio_get(struct t_state *state, sds buffer, unsigned gpio_nr, bool *rc);
sds api_gpio_gpio_options(struct t_state *state, sds buffer, unsigned gpio_nr, bool *rc);
sds api_gpio_gpio_post(struct t_state *state, sds buffer, unsigned gpio_nr, struct mg_str *action, struct mg_http_message *hm, bool *rc);

#endif
