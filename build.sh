#!/bin/sh

cmake -B debug \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    .
make -j4 -C debug VERBOSE=1
echo "Linking compilation database"
sed -e 's/\\t/ /g' -e 's/-Wformat-overflow=2//g' -e 's/-fsanitize=bounds-strict//g' debug/compile_commands.json > src/compile_commands.json
