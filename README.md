# ESP8266 Websocket Control

This is a codebase for demonstration of connecting a NodeMCU ESP8266 to a node server and controlling through REST APIs.

## Prerequisite

- NodeMCU ESP8266 module
- Drivers for ESP module
- [Backend](https://github.com/Soumyajit2000-web/nodemcu_esp8266_server)

## Project Structure

```
node_mcu_esp8266_websockets/
├── include/
│   ├── config.h
│   └── ... (other headers if any)
├── lib/
│   └── ... (libraries if any)
├── src/
│   └── main.cpp (or main.ino)
├── platformio.ini
├── .gitignore
└── README.md
```


## Project Setup

### Clone the repository

```bash
$ git clone https://github.com/Soumyajit2000-web/Node_mcu_esp8266_websockets.git
```

### Configuring Environment Variables

Add a file named "config.h" and copy past the following code. (Make sure to replace the values with the necessary ones):

```cpp
#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID "Your_wifi_ssid"
#define WIFI_PASSWORD "Your_wifi_password"
#define WS_HOST "hosted_server_url" // Or IP of your local network in case you are running the server locally.
#define WS_PORT 443 // port should be a integer
#define WS_URL "/socket.io/?EIO=4" // Maybe no need to change this.

#endif
```

### Build and Upload

Click the upload button or run this command:

```bash
$ pio run
```
