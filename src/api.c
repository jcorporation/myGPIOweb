/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "src/api.h"

#include "src/json.h"
#include "src/log.h"
#include "src/state.h"

#include <libmygpio/libmygpio.h>
#include <stdio.h>
#include <string.h>

sds api_gpiolist(struct t_state *state, sds buffer, bool *rc) {
    if (mygpio_gpiolist(state->conn) == true) {
        struct t_mygpio_gpio *gpio;
        unsigned i = 0;
        buffer = sdscat(buffer, "{\"data\":[");
        while ((gpio = mygpio_recv_gpio_list(state->conn)) != NULL) {
            if (i++) {
                buffer = sdscatlen(buffer, ",", 1);
            }
            const char *direction = mygpio_gpio_lookup_direction(mygpio_gpio_get_direction(gpio));
            const char *value = mygpio_gpio_lookup_value(mygpio_gpio_get_value(gpio));
            buffer = sdscatlen(buffer, "{", 1);
            buffer = sdscatfmt(buffer, "\"gpio\":%u", mygpio_gpio_get_gpio(gpio));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscat(buffer, "\"direction\":");
            buffer = sds_catjson(buffer, direction, strlen(direction));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscat(buffer, "\"value\":");
            buffer = sds_catjson(buffer, value, strlen(value));
            buffer = sdscatlen(buffer, "}", 1);
            mygpio_free_gpio(gpio);
        }
        buffer = sdscatfmt(buffer, "],\"entries\":%u}", i);
        mygpio_response_end(state->conn);
        *rc = true;
        return buffer;
    }

    const char *error = mygpio_connection_get_error(state->conn);
    PRINT_LOG_ERROR("Error: %s", error);

    buffer = sdscat(buffer, "{\"error\":");
    buffer = sds_catjson(buffer, error, strlen(error));
    buffer = sdscat(buffer, "}");
    mygpio_response_end(state->conn);
    *rc = false;
    return buffer;
}
