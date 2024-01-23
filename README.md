# Simple webfrontend for myGPIOd

This project provides a simple webfrontend for myGPIOd. It uses the libmygpio library to connect to the myGPIOd socket.

## Build

Building myGPIOd is straight forward.

### Dependencies

- C build environment
- cmake >= 3.13
- libmygpio

### Build myGPIOweb

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOweb/releases/latest)
2. Extract myGPIOweb tarball and change path to this directory
3. Run cmake: `cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release .`
4. Build: `make -C build`
5. Install (as root): `make -C build install`

## Provided endpoints

myGPIOweb provides a websocket to retrieve gpio events without polling and a REST API to control GPIOs.

## Run



### Websocket

Sends idle events from myGPIOd to connected clients.

- Path: `/ws`
- Format: `{"gpio": <gpio number>, "event": <event>, "ts_ms": <timestamp in ms>}`

## #REST Endpoints

| Path | Method | Command |
| ---- | ------ | ----------- |
| `/api/gpio` | GET | gpiolist |
| `/api/gpio/{gpio number}` | GET | gpioget |
| `/api/gpio/{gpio number}` | OPTIONS | gpioinfo |
| `/api/gpio/{gpio number}` | POST | gpioblink, gpioset, gpiotoggle |

#### POST /api/gpio/{gpio number}

- gpioblink: `{"action": "gpioblink", "timeout": <ms>, "interval": <ms>}`
- gpioset: `{"action": "gpioset", "value": "active|inactive"}`
- gpiotoggle: `{"action": "gpiotoggle"}`
