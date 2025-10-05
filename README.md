# Simple webfrontend for myGPIOd

This project provides a simple webfrontend and REST-API for [myGPIOd](https://github.com/jcorporation/myGPIOd). It uses the libmygpio library to connect to the myGPIOd socket.

## Build

1. Install dependencies: cmake, [libmygpio](https://github.com/jcorporation/myGPIOd)
2. Get myGPIOweb tarball from [GitHub](https://github.com/jcorporation/myGPIOweb/releases/latest)
3. Extract myGPIOweb tarball and change path to this directory
4. Run cmake: `cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release .`
5. Build: `make -C build`
6. Install (as root): `make -C build install`

## Run

```sh
mygpioweb [option]...
```

| Option | Description |
| ------ | ----------- |
| `-s`, `--socket` | myGPIOd socket (default: `/run/mygpiod/socket`) |
| `-l`, `--listen` | HTTP listening address (default: `http://0.0.0.0:8000`) |
| `-o`, `--syslog` | enable syslog logging (facility: daemon) |
| `-h`, `--help` | displays this help |
| `-v`, `--version` | displays this help |

## Provided endpoints

myGPIOweb provides a websocket to retrieve gpio events without polling and a REST-API to control GPIOs.

### Websocket

Sends idle events from myGPIOd to connected clients.

- Path: `/ws`
- Format: `{"gpio": <gpio number>, "event": <event>, "ts_ms": <timestamp in ms>}`

### REST Endpoints

- [OpenAPI-Documentation](openapi.yml)

| Path | Method | Command |
| ---- | ------ | ----------- |
| `/api/gpio` | GET | gpiolist |
| `/api/gpio/{gpio number}` | GET | gpioget |
| `/api/gpio/{gpio number}` | OPTIONS | gpioinfo |
| `/api/gpio/{gpio number}/{blink, set, toggle}` | POST | gpioblink, gpioset, gpiotoggle |
