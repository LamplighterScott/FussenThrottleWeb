/*
 * Arduino ESP8266 DCC++ over WiFi Throttle
 * Fussen RR
 * Version 1.0.0  April 11, 2020
 * By Scott Eaton
 */

#include <ESP8266WiFi.h> // includes WiFiClient.h
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>  //ESP Web Server Library to host a web page
#include <FS.h> // SPIFFS Library
#include <DNSServer.h>
#include <WebSocketsServer.h>

// #include <ArduinoOTA.h> // OTA update

#include "Config.h"

DNSServer dnsServer;
ESP8266WebServer server(80); //Server on port 80
WebSocketsServer webSocket(81);


typedef struct {
  int id;  // system id number w/o system and type chars
  int pin;  // Arduino GPIO number
  const char *title;  // any string for name of output
  const char *type;  // T = turnout/semaphore, C = decoupler, L = Light
  const char *icon0;  // icons for state 0 (closed) and 1 (thrown)
  const char *icon1;
  int iFlag;  // See DCC++ Outlets for bits explanation, WiThrottle uses bits 3-6 to communicate output type to DCC++ in Load Outputs
              // T=8, C=16, F=32, L=64
  int zStatus;  // 0 = closed (straight, green, off, 0(DCC++)), 1 = thrown (round, diverge, red, on, 1(DCC++))
  int present;  // 0 = not present, 1 = present in DCC++
} tData;
/* Format {Switch number, pin number, name, type, X, X, X} */
tData ttt[]= {
  {1, 22, "To outer", "T", "\U00002B06", "\U00002196", 8, 0, 0},
  {2, 24, "To inner", "T", "\U00002B06", "\U00002197", 8, 0, 0},
  {3, 26, "Yard 1", "T", "\U00002B06", "\U00002196", 8, 0, 0},
  {4, 28, "Yard 1-1", "T", "\U00002B06", "\U00002197", 8, 0, 0},
  {5, 30, "Outer to Cross", "T", "\U00002B06", "\U00002196", 8, 0, 0},
  {6, 32, "Cross to Yard 2", "T", "\U0001F500", "\U0001F504", 8, 0, 0},
  {7, 34, "Yard 2-2", "T", "\U00002B06", "\U00002197", 8, 0, 0},
  {8, 36, "Sidetrack", "T", "\U00002B06", "\U00002197", 8, 0, 0},
  {17, 38, "Decoupler 1A", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0},
  {18, 40, "Decoupler 1B", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0},
  {19, 42, "Decoupler 2A", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0},
  {20, 44, "Decoupler 2B", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0},
  {25, 46, "Semaphore", "T", "\U0001F6A6", "\U0001F6A5", 8, 0, 0},
  {30, 48, "Castle", "L", "\U0001F3F0", "\U0001F3F0", 64, 0, 0}
};
const int totalOutputs = 13;  // Must include the zero row at the end

int LocoState[4][7]={{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}}; // Set-up Function states [number of], (light, beam, cab 1, cab 2, shunt, speed, direction)
String locoAddresses[4]={"3  ","3  ","46 ","3  "};
int locoNumber = 0;
int maxNumberOfLocos = 4; // using four locos, arrays start at 0
String sendText = "";


//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  delay(1500); // From WiThrottle
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  // Serial.flush(); // From WiThrottle
  // Serial.setDebugOutput(true);

  startWiFi();
  startSPIFFS();
  startWebSocket();
  startMDNS();
  startServer();

  loadOutputs();  // register outputs with Arduino
  // delay(1000);
  // Serial.println("<J 3>"); // play first sound after loadling switches

}

//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  dnsServer.processNextRequest();
  webSocket.loop();
  server.handleClient();          //Handle client requests
}


void startWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ssid);
  // Serial.print("SSID: ");
  // Serial.println(ssid);
  // Serial.print("IP address: ");
  // Serial.println(WiFi.softAPIP());
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  // Serial.println("USP Server started");
}

void startSPIFFS() {
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  //Serial.println("SPIFFS started. Contents:"); {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      // String fileName = dir.fileName();
      // size_t fileSize = dir.fileSize();
      dir.fileName();
      dir.fileSize();
      // Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    // Serial.printf("\n");
  // }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  // Serial.println("WebSocket server started.");
}

void startMDNS(){
  if (MDNS.begin(mdnsName)) {
    // Serial.print("mDNS responder started: http://");
    // Serial.print(mdnsName);
    // Serial.println(".local");
  } else {
    // Serial.println("Error setting up MDNS responder!");
  }
}

void startServer() {
  server.on("/", handleRoot);
  server.on("/hotspot-detect.html", handle_CaptivePortal);
  server.onNotFound([]() {                              // If the client requests any URI
      if (!handleFileRead(server.uri()))                  // send it if it exists
        server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

  //Start server
  server.begin();
  Serial.println("HTTP server started");

}

//===============================================================
//   Server Handlers
//===============================================================


void handleRoot() {
    String path = "/FussenHTML.html";
    handleFileRead(path);
    // Serial.println("Root page loaded");
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
    // Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
    String contentType = getContentType(path);            // Get the MIME type
    if (SPIFFS.exists(path)) {                            // If the file exists
      File file = SPIFFS.open(path, "r");                 // Open it
      server.streamFile(file, contentType); // And send it to the client
      file.close();                                       // Then close the file again
      return true;
    }
    // Serial.println("\tFile Not Found");
    return false;                                         // If the file doesn't exist, return false
}


void handle_CaptivePortal() {
  String toSend = captivePortalRespone;
  server.send(200, "text/html", toSend);
  delay(100);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED: {
      // Serial.printf("[%u] Disconnected!\n", num);
      break;
    }
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      break;
    }
    case WStype_TEXT: {                     // if new text data is received - command from control device processor
      // Serial.printf("[%u] get Text: %s\n", num, payload);
      char *pch;

      //const char payloadCommand = (const char) payload[1];
      const char *payloadChar = (const char *) payload;
      char payloadCommand = payload[1];

      if (payloadCommand == '0') {                    // Power off
        powerOff(num);
        webSocket.sendTXT(num, payloadChar);

      } else if (payloadCommand == '1') {             // Power on
        Serial.println("<1>");
        sendTurnoutStatusToDevice(0, num);
        webSocket.sendTXT(num, payloadChar);

      } else if (payloadCommand == 'Z') {             // Turnouts, decouplers and sounds <Z Type&ID>
        pch = strtok((char *)payload,"<>Z ");
        // pch = strtok(pch,"Z ");
        char type = pch[0];
        pch = strtok(pch, "TCLJ");
        int id;
        sscanf(pch,"%i", &id);

        if (type == 'T') {
          sendTurnoutStatusToDevice(id, num);

        } else if (type == 'C' || type == 'L') {
          sendText = String(id);
          Serial.println("<Z " + sendText + " 1>"); //  DCC++ returns nothing

        } else if (type == 'J') {
          int soundNo;
          sscanf(pch,"%i", &soundNo);
          // Sound instructions from control device: 0=stopLoop, 1=soundDN, 2=soundUP, sounds: 3 to x
          // Instructions in DFPlayer: -1=soundDN, 0=soundUP, sounds: 1 to x, Action 0=stop sound, execute by DF Player library function
          Serial.println("<J " +  String(soundNo) + ">"); //  DCC++ returns nothing
          // Serial.println("<H J " + String(soundNo) + ">");
        }

      }  else if (payloadCommand == 'F') {             // Loco functions <F ID Loco>
        int fKey;
        pch = strtok((char *)payload,"<>F");
        if (sscanf(pch,"%i %i", &locoNumber, &fKey)==2) {
          LocoState[locoNumber][fKey] = invert(LocoState[locoNumber][fKey]);
          int keyFunc = 128+LocoState[locoNumber][1]*1+LocoState[locoNumber][2]*2+LocoState[locoNumber][3]*4+LocoState[locoNumber][4]*8+LocoState[locoNumber][0]*16;
          if (locoNumber < maxNumberOfLocos) {
            String locoAddress = locoAddresses[locoNumber];
            Serial.println("<f " + String(locoAddress) + " " + String(keyFunc) + ">");  // DCC++ returns?
          }
        }

      } else if (payloadCommand == 't') {             // Direction: reverse(0), forward(1); Speed: eStop(<0), 0-126
        String payloadStr = payloadChar;
        // Serial.println(payloadStr);  // Control device sends: <t REGISTER LOCO SPEED DIRECTION>; DCC++ returns <T REGISTER SPEED DIRECTION>, ignored!!
        //int locoAddress;
        int locoSpeed;
        int locoDirection;
        pch = strtok((char *)payload,"<>t");
        if (sscanf(pch,"%i %i %i", &locoNumber, &locoSpeed, &locoDirection)==3) {
            // Save new settings
          LocoState[locoNumber][5]=locoSpeed;
          LocoState[locoNumber][6]=locoDirection;
          int locoID = locoNumber + 1;  // DCC++ loco register starts at 1
          webSocket.sendTXT(num, payloadStr);  // confirm new settings to throttle
          if (locoNumber < maxNumberOfLocos) {
            Serial.println("<t " + String(locoID)+ " " + locoAddresses[locoNumber] + " " + String(locoSpeed)+ " " + String(locoDirection) + ">");
          }
        }

      } else if (payloadCommand == 's') {   // used for getting loco status when changing locos
          // Send speed and direction for all throttles
          // int locoAddress;
          pch = strtok((char *)payload,"<>s");
          // int testNo = sscanf(pch,"%i", &locoAddress);
          // Serial.println("Scan No: " + String(testNo) + "  LocoAddress: " + String(locoAddress));
          if (sscanf(pch,"%i", &locoNumber)==1) {
            /*for (int x=0; x<maxNumberOfLocos; x++) {
               if (locoIDs[x] == locoAddress) {
                locoNumber=x;
                break;
               }
            }
            int testNoo = sscanf(pch,"%i", &locoAddress);*/
            Serial.println("LocoNumber: " + String(locoNumber));
            sendText = "<t "+String(locoNumber)+" "+String(LocoState[locoNumber][5])+" "+String(LocoState[locoNumber][6])+">";  // Speed and direction to control device;
            webSocket.sendTXT(num, sendText);
          }
      }
    }
  }
}

void powerOff(uint8_t num) {
  for (int x=0; x<maxNumberOfLocos; x++) {
    LocoState[x][5]=0;
    LocoState[x][6]=0;
  }
  sendText = "<t "+String(locoNumber)+" "+String(LocoState[locoNumber][5])+" "+String(LocoState[locoNumber][6])+">";  // Speed and direction to control device;
  webSocket.sendTXT(num, sendText);
  Serial.println("<0>");
}

void sendTurnoutStatusToDevice(int id, uint8_t num) {
  String idTxt;
  int state;
  String icon;
  String stateTxt;
  String typeTxt;
  if (id>0) {  // Return specified output
    for (tData &t: ttt) {
      if (t.id == id) {
        state = t.zStatus;
        state = abs(state-1);  // change state and save
        t.zStatus = state;
        stateTxt = (state>0) ? "1" : "0";
        icon = (state>0) ? t.icon1 : t.icon0;
        idTxt = String(t.id);
        typeTxt = String(t.type);
        Serial.println("<Z " + idTxt + " " + stateTxt + ">");  //  To Arduino
        sendText = "<Y " + typeTxt + idTxt + " " + stateTxt + " " + icon + ">"; // To device
        webSocket.sendTXT(num, sendText);
        break;
      }
    }
  } else {  // Return all outputs
    // String pinTxt;
    // String iFlagTxt;
    for (tData &t: ttt) {
      // pinTxt = String(t.pin);
      state = t.zStatus;
      stateTxt = (state>0) ? "1" : "0";
      icon = (state>0) ? t.icon1 : t.icon0;
      idTxt = String(t.id);
      typeTxt = String(t.type);
      // iFlagTxt = String(t.iFlag);
      // Serial.println("<Z " + idTxt + " " + pinTxt + " " + iFlagTxt + ">");  //  To Arduino
      sendText = "<Y " + typeTxt + idTxt + " " + stateTxt + " " + icon + ">"; // To device
      // Serial.println(sendText);
      webSocket.sendTXT(num, sendText);
    }
  }
}  // sendTurnoutStatusToDevice

void loadOutputs() { // Accessories/Outputs
  // Update Outputs to DCC++
  for (tData t: ttt)
  {
      Serial.println("<Z "+String(t.id)+" "+String(t.pin)+" "+String(t.iFlag)+">");
      int zStatus = 0;
      if (t.zStatus > 2)
        zStatus = 1;
      Serial.print("<Z "+String(t.id)+" "+String(zStatus)+">");
  }

}  //  loadOutputs()

//===============================================================
//   Helper Functions
//===============================================================

int invert(int value){
  if(value == 0)
    return 1;
  else
    return 0;

}  // invert

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
