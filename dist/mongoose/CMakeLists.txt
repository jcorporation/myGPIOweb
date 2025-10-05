# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd

add_library(mongoose "")

target_sources(mongoose
  PRIVATE
    mongoose.c
  PUBLIC
    mongoose.h
)

target_compile_options(mongoose
  PRIVATE
    "-DMG_ENABLE_DIRLIST=0"
    "-DMG_ENABLE_SSI=0"
)

if(CMAKE_BUILD_TYPE MATCHES "(Debug|Release|RelWithDebInfo|MinSizeRel)")
  target_compile_options(mongoose
    PRIVATE
      "-Wno-unused-parameter"
  )
endif()
