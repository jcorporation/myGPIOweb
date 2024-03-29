/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#include "src/api.h"

#include "dist/mongoose/mongoose.h"
#include "dist/sds/sds.h"
#include "src/json.h"
#include "src/log.h"
#include "src/mygpiod.h"
#include "src/state.h"

#include <libmygpio/libmygpio.h>
#include <stdio.h>
#include <string.h>

/**
 * Handles the REST API request for GET /api/gpio
 * @param state pointer to state
 * @param buffer already allocated buffer to populate with the response
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds api_gpio_get(struct t_state *state, sds buffer, bool *rc) {
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
            buffer = sds_catjson(buffer, direction);
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscat(buffer, "\"value\":");
            buffer = sds_catjson(buffer, value);
            buffer = sdscatlen(buffer, "}", 1);
            mygpio_free_gpio(gpio);
        }
        buffer = sdscatfmt(buffer, "],\"entries\":%u}", i);
        mygpio_response_end(state->conn);
        *rc = true;
        return buffer;
    }
    mygpiod_check_error(state);
    mygpio_response_end(state->conn);
    buffer = sdscat(buffer, "{\"error\":\"myGPIOd error\"}");
    *rc = false;
    return buffer;
}

/**
 * Handles the REST API request for GET /api/gpio/{gpio_nr}
 * @param state pointer to state
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds api_gpio_gpio_get(struct t_state *state, sds buffer, unsigned gpio_nr, bool *rc) {
    enum mygpio_gpio_value value = mygpio_gpioget(state->conn, gpio_nr);
    if (mygpiod_check_error(state) == true) { 
        buffer = sdscatlen(buffer, "{", 1);
        buffer = sdscatfmt(buffer, "\"gpio\": %u,", gpio_nr);
        buffer = sdscatfmt(buffer, "\"value\":\"%s\"", mygpio_gpio_lookup_value(value));
        buffer = sdscatlen(buffer, "}", 1);
        mygpio_response_end(state->conn);
        *rc = true;
        return buffer;
    }
    mygpiod_check_error(state);
    mygpio_response_end(state->conn);
    buffer = sdscat(buffer, "{\"error\":\"myGPIOd error\"}");
    *rc = false;
    return buffer;
}

/**
 * Handles the REST API request for OPTIONS /api/gpio/{gpio_nr}
 * @param state pointer to state
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds api_gpio_gpio_options(struct t_state *state, sds buffer, unsigned gpio_nr, bool *rc) {
    if (mygpio_gpioinfo(state->conn, gpio_nr) == true) {
        struct t_mygpio_gpio *gpio = mygpio_recv_gpio_info(state->conn);
        buffer = sdscat(buffer, "{\"data\":");
        enum mygpio_gpio_direction direction = mygpio_gpio_get_direction(gpio);
        buffer = sdscatlen(buffer, "{", 1);
        buffer = sdscatfmt(buffer, "\"gpio\":%u", gpio_nr);
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscat(buffer, "\"direction\":");
        buffer = sds_catjson(buffer, mygpio_gpio_lookup_direction(mygpio_gpio_get_direction(gpio)));
        buffer = sdscatlen(buffer, ",", 1);
        buffer = sdscat(buffer, "\"value\":");
        buffer = sds_catjson(buffer, mygpio_gpio_lookup_value(mygpio_gpio_get_value(gpio)));
        buffer = sdscatlen(buffer, ",", 1);
        if (direction == MYGPIO_GPIO_DIRECTION_IN) {
            buffer = sdscatfmt(buffer, "\"active_low\":%s", bool_to_str(mygpio_gpio_in_get_active_low(gpio)));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscat(buffer, "\"bias\":");
            buffer = sds_catjson(buffer, mygpio_gpio_lookup_bias(mygpio_gpio_in_get_bias(gpio)));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscat(buffer, "\"event_request\":");
            buffer = sds_catjson(buffer, mygpio_gpio_lookup_event_request(mygpio_gpio_in_get_event_request(gpio)));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscatfmt(buffer, "\"is_debounced\":%s", bool_to_str(mygpio_gpio_in_get_is_debounced(gpio)));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscatfmt(buffer, "\"debounce_period_us\":%I", (int64_t)mygpio_gpio_in_get_debounce_period_us(gpio));
            buffer = sdscatlen(buffer, ",", 1);
            buffer = sdscat(buffer, "\"event_clock\":");
            buffer = sds_catjson(buffer, mygpio_gpio_lookup_event_clock(mygpio_gpio_in_get_event_clock(gpio)));
        }
        else if (direction == MYGPIO_GPIO_DIRECTION_OUT) {
            buffer = sdscat(buffer, "\"drive\":");
            buffer = sds_catjson(buffer, mygpio_gpio_lookup_drive(mygpio_gpio_out_get_drive(gpio)));
        }
        buffer = sdscatlen(buffer, "}}", 2);
        mygpio_free_gpio(gpio);
        mygpio_response_end(state->conn);
        *rc = true;
        return buffer;
    }
    mygpiod_check_error(state);
    mygpio_response_end(state->conn);
    buffer = sdscat(buffer, "{\"error\":\"myGPIOd error\"}");
    *rc = false;
    return buffer;
}

/**
 * Handles the REST API request for POST /api/gpio/{gpio_nr}
 * @param state pointer to state
 * @param buffer already allocated buffer to populate with the response
 * @param gpio_nr gpio number
 * @param action action for the gpio, one of blink, set, toggle
 * @param hm mongoose http message struct
 * @param rc pointer to bool to set the result code
 * @return sds pointer to buffer
 */
sds api_gpio_gpio_post(struct t_state *state, sds buffer, unsigned gpio_nr, struct mg_str *action, struct mg_http_message *hm, bool *rc) {
    char *value_str = NULL;
    long timeout;
    long interval;
    enum mygpio_gpio_value value;
    if (mg_vcmp(action, "blink") == 0 &&
        (timeout = mg_json_get_long(hm->body, "$.timeout", -1)) > -1 &&
        (interval = mg_json_get_long(hm->body, "$.interval", -1)) > -1 &&
        timeout < INT_MAX &&
        interval < INT_MAX)
    {
        *rc = mygpio_gpioblink(state->conn, gpio_nr, (int)timeout, (int)interval) ||
            mygpiod_check_error(state);
    }
    else if (mg_vcmp(action, "set") == 0 &&
             (value_str = mg_json_get_str(hm->body, "$.value")) != NULL &&
             (value = mygpio_gpio_parse_value(value_str)) != MYGPIO_GPIO_VALUE_UNKNOWN)
    {
        *rc = mygpio_gpioset(state->conn, gpio_nr, value) ||
            mygpiod_check_error(state);
    }
    else if (mg_vcmp(action, "toggle") == 0) {
        *rc = mygpio_gpiotoggle(state->conn, gpio_nr) ||
            mygpiod_check_error(state);
    }
    else {
        PRINT_LOG_ERROR("Invalid action \"%.*s\" or values for uri %.*s", (int)action->len, action->ptr, (int)hm->uri.len, hm->uri.ptr);
    }
    free(action);
    if (value_str != NULL) {
        free(value_str);
    }
    if (*rc == true) {
        buffer = sdscat(buffer, "{\"message\":\"OK\"}");
    }
    return buffer;
}
