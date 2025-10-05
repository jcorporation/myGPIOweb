# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOweb (c) 2024-2025 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOweb

add_executable(mygpioweb "")

if(CMAKE_BUILD_TYPE MATCHES "(Release|Debug)")
  target_compile_options(mygpioweb
    PRIVATE
      "-Wconversion"
      "-Wformat=2"
  )
endif()

target_include_directories(mygpioweb
  PRIVATE 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${LIBMYGPIO_INCLUDE_DIRS}
)

target_sources(mygpioweb PRIVATE
  api.c
  json.c
  log.c
  main.c
  mygpiod.c
  options.c
  state.c
  webserver.c
)

# link all together
target_link_libraries(mygpioweb
  mongoose
  sds
  ${LIBMYGPIO_LIBRARIES}
)

install(TARGETS mygpioweb DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
