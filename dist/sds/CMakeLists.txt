# SPDX-License-Identifier: GPL-3.0-or-later
# myMPD (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_library(sds "")

target_compile_options(sds
  PRIVATE
    "-Wno-conversion"
)

target_sources(sds
  PRIVATE
    sds.c
    sdsalloc.h
  PUBLIC
    sds.h
)

target_include_directories(sds
  PRIVATE
    ${PROJECT_SOURCE_DIR}
)
