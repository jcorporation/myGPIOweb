#!/bin/sh
#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

#save script path and change to it
STARTPATH=$(dirname "$(realpath "$0")")
cd "$STARTPATH" || exit 1

#exit on error
set -e

#exit on undefined variable
set -u

#set umask
umask 0022

#get mygpioweb version
VERSION=$(grep "  VERSION" CMakeLists.txt | sed 's/  VERSION //')

#check for command
check_cmd() {
    for DEPENDENCY in "$@"
    do
    if ! check_cmd_silent "$@"
    then
        echo "ERROR: ${DEPENDENCY} not found"
        return 1
    fi
    done
    return 0
}

check_cmd_silent() {
    for DEPENDENCY in "$@"
    do
    if ! command -v "${DEPENDENCY}" > /dev/null
    then
        return 1
    fi
    done
    return 0
}

builddebug() {
    CMAKE_SANITIZER_OPTIONS=""
    case "$ACTION" in
        asan)  CMAKE_SANITIZER_OPTIONS="-DMYGPIOD_ENABLE_ASAN=ON" ;;
        tsan)  CMAKE_SANITIZER_OPTIONS="-DMYGPIOD_ENABLE_TSAN=ON" ;;
        ubsan) CMAKE_SANITIZER_OPTIONS="-DMYGPIOD_ENABLE_UBSAN=ON" ;;
    esac
    echo "Compiling myGPIOweb v${VERSION}"
    cmake -B debug \
        -DCMAKE_INSTALL_PREFIX:PATH=/usr \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        $CMAKE_SANITIZER_OPTIONS \
        .
    make -j4 -C debug VERBOSE=1
    echo "Linking compilation database"
    sed -e 's/\\t/ /g' -e 's/-Wformat-overflow=2//g' -e 's/-fsanitize=bounds-strict//g' debug/compile_commands.json > src/compile_commands.json
}

buildrelease() {
  BUILD_TYPE=$1
  echo "Compiling myGPIOweb v${VERSION}"
  cmake -B release \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    .
  make -j4 -C release
}

installrelease() {
  echo "Installing myGPIOweb"
  addmygpiowebuser
  cd release || exit 1  
  [ -z "${DESTDIR+x}" ] && DESTDIR=""
  make install DESTDIR="$DESTDIR"
  echo "myGPIOweb installed"
}

addmygpiowebuser() {
  echo "Checking status of mygpioweb system user"
  if ! getent passwd mygpioweb > /dev/null
  then
    if check_cmd_silent useradd
    then
      groupadd -r mygpiod || true
      groupadd -r mygpioweb || true
      useradd -r -g mygpioweb -G mygpiod -s /bin/false -d /var/lib/mygpioweb mygpioweb
    elif check_cmd_silent adduser
    then
      #alpine
      addgroup -S mygpiod || true
      addgroup -S mygpioweb || true
      adduser -S -D -H -h /var/lib/mygpioweb -s /sbin/nologin -G mygpioweb -g myGPIOweb mygpioweb
      adduser mygpioweb mygpiod
    else
      echo "Can not add user mygpioweb"
      return 1
    fi
  fi
  return 0
}

check() {
  if [ ! -f src/compile_commands.json ]
  then
    echo "src/compile_commands.json not found"
    echo "run: ./build.sh debug"
    exit 1
  fi

  if check_cmd clang-tidy
  then
    echo "Running clang-tidy"
    cd src || exit 1
    find ./ -name '*.c' -exec clang-tidy \
        --config-file="$STARTPATH/.clang-tidy" {} \;
  else
    echo "clang-tidy not found"
  fi
}

#get action
if [ -z "${1+x}" ]
then
  ACTION=""
else
  ACTION="$1"
fi

case "$ACTION" in
  release|MinSizeRel)
    buildrelease "Release"
  ;;
  RelWithDebInfo)
    buildrelease "RelWithDebInfo"
  ;;
  install)
    installrelease
  ;;
  releaseinstall)
    buildrelease "Release"
    installrelease
  ;;
  debug|asan|tsan|ubsan)
    builddebug
  ;;
  check)
    check
  ;;
  *)
    echo "Usage: $0 <option>"
    echo "Version: ${VERSION}"
    echo ""
    echo "Build options:"
    echo "  release:          build release files in directory release (stripped)"
    echo "  RelWithDebInfo:   build release files in directory release (with debug info)"
    echo "  install:          installs release files from directory release"
    echo "                    following environment variables are respected"
    echo "                      - DESTDIR=\"\""
    echo "  releaseinstall:   calls release and install afterwards"
    echo "  debug:            builds debug files in directory debug,"
    echo "  asan|tsan|ubsan:  builds debug files in directory debug"
    echo "                    linked with the sanitizer"
    echo "  check:            runs clang-tidy on source files"
    echo "  addmygpiowebuser: adds mygpioweb group and user"
    echo ""
    exit 1
  ;;
esac

exit 0
