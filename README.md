# Simple webfrontend for myGPIOd

This project provides a simple webfrontend for myGPIOd.

## Websocket

Sends idle events from myGPIOd to connected clients.

- Path: `/ws`
- Format: `{"gpio": <gpio number>, "event": <event>, "ts_ms": <timestamp in ms>}`

## REST Endpoints

| Path | Method | Command |
| ---- | ------ | ----------- |
| `/api/gpio` | GET | gpiolist |
| `/api/gpio/{gpio number}` | GET | gpioget |
| `/api/gpio/{gpio number}` | OPTIONS | gpioinfo |
| `/api/gpio/{gpio number}` | POST | gpioblink, gpioset, gpiotoggle |

### POST /api/gpio/{gpio number}

- gpioblink: `{"action": "gpioblink", "timeout": <ms>, "interval": <ms>}`
- gpioset: `{"action": "gpioset", "value": "active|inactive"}`
- gpiotoggle: `{"action": "gpiotoggle"}`
