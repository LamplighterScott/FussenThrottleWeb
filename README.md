# FussenThrottleSafari

ESP-01 based WiFi server for DCC++.

Provides throttle control via a web browser (Safari) as the client.

Serves up web page from SPIFFS on ESP-01.

SPIFFS files consist of a CCS, HTML and JavaScript.

Communication via WebSocket.

Sends commands via serial to an Arduino MEGA running "DCC++ Base Station sketch branched for Marklin Z".

Libraries:
ESP8266WiFi
ESP8266mDNS
ESP8266WEbServer
LittleFS
DNSServer
WebSocketsServer: https://github.com/Links2004/arduinoWebSockets
