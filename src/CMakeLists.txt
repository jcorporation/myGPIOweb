# SPDX-License-Identifier: GPL-3.0-or-later
# myMPD (c) 2018-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_executable(mygpiod-web "")

if(CMAKE_BUILD_TYPE MATCHES "(Release|Debug)")
  target_compile_options(mygpiod-web
    PRIVATE
      "-Wconversion"
      "-Wformat=2"
  )
endif()

target_include_directories(mygpiod-web
  PRIVATE 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${LIBMYGPIO_INCLUDE_DIRS}
)

target_sources(mygpiod-web PRIVATE
  api.c
  json.c
  log.c
  main.c
  mygpiod.c
  state.c
  webserver.c
)

# link all together
target_link_libraries(mygpiod-web
  mongoose
  sds
  ${LIBMYGPIO_LIBRARIES}
)
