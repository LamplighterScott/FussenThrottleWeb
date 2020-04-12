/*
 * Config.h
 * Copyright (c) 2019, FussenRR by Scott Eaton
 * For ESP8266 connected to Arduino
*/

/* Maximum WiFi clients that can be connected to Throttle */
#define maxClient 1

/* Access Point name and password. */
const char* ssid = "FussenRR";
// const char* password = "123456";

/* Network parameters */
const char* mdnsName = "Fussen";
IPAddress apIP(10, 10, 10, 10);
IPAddress netMsk(255, 255, 255, 0);
const byte DNS_PORT = 53;
int Server_Port = 80;
// char hostString[] = "FussenServer";

/* Power state on start 0=OFF, 1=ON. WARNING!!! If you use iOS WiThrottle.app this must set only ON*/
int power = 0;

const char* captivePortalRespone =  "<!DOCTYPE HTML PUBLIC ""-//W3C//DTD HTML 3.2//EN"">"
                                    "<HTML>"
                                    "<HEAD>"
                                        "<TITLE>Success</TITLE>"
                                    "</HEAD>"
                                    "<BODY>"
                                    "Success"
                                    "</BODY>"
                                    "</HTML>";
