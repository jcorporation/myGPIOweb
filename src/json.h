/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/myGPIOd
*/

#ifndef JSON_H
#define JSON_H

#include "dist/sds/sds.h"

sds sds_catjson_plain(sds s, const char *p, size_t len);
sds sds_catjson(sds s, const char *p, size_t len);

#endif
