/*
 * Arduino ESP8266 DCC++ over WiFi Throttle
 * Fussen RR
 * Version 1.0.0  April 11, 2020
 * By Scott Eaton
 */

#include <ESP8266WiFi.h> // includes WiFiClient.h
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>  //ESP Web Server Library to host a web page
#include <FS.h> // LittleFS / SPIFFS Library
#include <DNSServer.h>
#include <WebSocketsServer.h>

// #include <ArduinoOTA.h> // OTA update

#include "Config.h"

DNSServer dnsServer;
ESP8266WebServer server(80); //Server on port 80
WebSocketsServer webSocket(81);

typedef struct {
  const int id;  // system id number w/o system and type chars, used by MEGA only
  char pin[4];  // Arduino GPIO number
  char title[16];  // any string for name of output
  char type[2];  // T = turnout/semaphore, C = decoupler, L = Light
  char icon0[5];  // icons for state 0 (closed) and 1 (thrown)
  char icon1[5];
  int iFlag;  // See DCC++ Outlets for bits explanation, WiThrottle uses bits 3-6 to communicate output type to DCC++ in Load Outputs
              // T=8, C=16, F=32, L=64
  int zStatus;  // 0 = closed (straight, green, off, 0(DCC++)), 1 = thrown (round, diverge, red, on, 1(DCC++))
  int present;  // 0 = not present, 1 = present in DCC++
  char show[2]; // =1 if not to be displayed in throttle control screen
} tData;
/* Format {Switch number, pin number, name, type, X, X, X} */
tData ttt[]= {
  {0, "022", "To outer", "T", "\U00002B06", "\U00002196", 8, 0, 0, "0"},
  {1, "024", "To inner", "T", "\U00002B06", "\U00002197", 8, 0, 0, "0"},
  {2, "026", "Yard 1", "T", "\U00002B06", "\U00002196", 8, 0, 0, "0"},
  {3, "028", "Yard 1-1", "T", "\U00002B06", "\U00002197", 8, 0, 0, "0"},
  {4, "030", "Outer to Cross", "T", "\U00002B06", "\U00002196", 8, 0, 0, "0"},
  {5, "032", "Cross to Yard 2", "T", "\U0001F500", "\U0001F504", 8, 0, 0, "0"},
  {6, "034", "Yard 2-2", "T", "\U00002B06", "\U00002197", 8, 0, 0, "0"},
  {7, "036", "Sidetrack", "T", "\U00002B06", "\U00002197", 8, 0, 0, "0"},
  {8, "038", "Decoupler 1A", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0, "0"},
  {9, "040", "Decoupler 1B", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0, "0"},
  {10, "042", "Decoupler 2A", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0, "0"},
  {11, "044", "Decoupler 2B", "C", "\U0001F9F2", "\U0001F9F2", 16, 0, 0, "0"},
  {12, "046", "Semaphore", "T", "\U0001F6A6", "\U0001F6A5", 8, 0, 0, "0"},
  {13, "048", "Castle", "L", "\U0001F3F0", "\U0001F3F0", 64, 0, 0, "0"},
  {14, "048", "Signal Tressle", "L", "\U0001F3F0", "\U0001F3F0", 64, 0, 0, "0"}
};
const int totalOutputs = 15;  // Must include the zero row at the end

typedef struct {
  const int id;  // system id number w/o system and type chars, used by MEGA
  char pin[4];  // DCC encoded loco number 1-999
  char title[16];  // any string for throttle display name of locomotice
  char icon0[5];  // display icon choosen for locomotive
  char show[2]; // =1 if not to be display in throttle control
} cData;
/* Format {Switch number, pin number, name, type, X, X, X} */
cData ccc[]= {
  {0, "111", "SBB 111", "\U0001F682", "0"},
  {1, "003", "DB 112", "\U0001F683", "0"},
  {2, "046", "CFF FFS123", "\U0001F682", "0"},
  {3, "003", "DB 239", "\U0001F686", "0"},
  {4, "000", "Steam", "\U0001F682", "1"},
  {5, "000", "Bullet", "\U0001F685", "1"},
  {6, "000", "Monorail", "\U0001F69D", "1"},
  {7, "000", "Mountain", "\U0001F69E", "1"},
  {8, "000", "Trolley", "\U0001F68E", "1"},
  {9, "000", "Tuktuk", "\U0001F6FA", "1"}
};
const int totalLocos = 10;  // Must include the zero row at the end

int tOrder[15] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
int cOrder[10] = {0,1,2,3,4,5,6,7,8,9};
int LocoState[10][7]={{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}}; // Set-up Function states [number of], (light, beam, cab 1, cab 2, shunt, speed, direction)
int locoNumber = 0;  // global variable for current target loco number
char printTxt[40];  // buffer for creating communication cstrings


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
  server.on("/FussenSetUpHTML.html", handleSetup);
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

void handleSetup() {
  String path = "/FussenSetUpHTML.html";
  handleFileRead(path);
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
  // String toSend = captivePortalRespone;
  // const char *toSend = captivePortalRespone;
  // server.send(200, "text/html", toSend);
  server.send(200, "text/html", captivePortalRespone);
  delay(100);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received
  // num = client ID
  switch (type) {
    case WStype_DISCONNECTED: {
      // Serial.printf("[%u] Disconnected!\n", num);
      break;
    }
    case WStype_CONNECTED: {
      // IPAddress ip = webSocket.remoteIP(num);
      webSocket.remoteIP(num);
      // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      break;
    }
    case WStype_TEXT: {                     // if new text data is received - command from control device processor
      Serial.printf("[%u] get Text: %s\n", num, payload);
      char *pch = strtok((char *)payload,"<>");

      //const char payloadCommand = (const char) payload[1];
      char payloadCommand = payload[1];

      if (payloadCommand == '0') {                    // Power off
        powerOff(num);

      } else if (payloadCommand == '1') {             // Power on
        const char *on = "<1>";
        Serial.println(on);
        webSocket.sendTXT(num, on);

      }  else if (payloadCommand == 'F') {             // Loco functions <F ID Loco>
        int fKey;
        //pch = strtok((char *)payload,"F");
        pch++;
        if (sscanf(pch,"%d %d", &fKey)==1) {
          LocoState[locoNumber][fKey] = invert(LocoState[locoNumber][fKey]);
          int keyFunc = 128+LocoState[locoNumber][1]*1+LocoState[locoNumber][2]*2+LocoState[locoNumber][3]*4+LocoState[locoNumber][4]*8+LocoState[locoNumber][0]*16;
          if (locoNumber < totalLocos) {
            //Serial.println("<f " + String(ccc[locoNumber].pin) + " " + String(keyFunc) + ">");  // DCC++ returns?
            sprintf(printTxt, "<f %s %i>", ccc[locoNumber].pin, keyFunc);
            Serial.println(printTxt);
          }
        }

      } else if (payloadCommand == 'I') {  // settings: set-up, move and edit
        pch += 2;  // skip ove "I "
        char source[4];
        char target[4]; // also icon
        int x = sscanf(pch,"%s %s", source, target);
        if (x == 1) {
          sendSetUpData(num, source);
        } else if (x == 2) {
          moveItem(source, target);
        }

      } else if (payloadCommand == 'K') {
          saveItemData(num, pch);  // Updates ESP array

      } else if (payloadCommand == 'C') {   // used for getting loco status when changing locos
          // Send speed and direction for all throttles
          pch++;
          int lN;
          if (sscanf(pch,"%d", &lN)==1) {
            locoNumber = cOrder[lN];
            sprintf(printTxt, "<t %i %i %i>", locoNumber, LocoState[locoNumber][5], LocoState[locoNumber][6]);
            webSocket.sendTXT(num, printTxt);
          }

      } else if (payloadCommand == 't') {             // Direction: reverse(0), forward(1); Speed: eStop(<0), 0-126
        // Control device sends: <t REGISTER LOCO SPEED DIRECTION>; DCC++ returns <T REGISTER SPEED DIRECTION>, ignored!!
        int locoSpeed;
        int locoDirection;
        char *pch2;
        pch2 = pch + 6;
        if (sscanf(pch2,"%d %d", &locoSpeed, &locoDirection) == 2) {
            // Save new settings
          // Serial.println("locoNumber:" + String(locoNumber));
          LocoState[locoNumber][5]=locoSpeed;
          LocoState[locoNumber][6]=locoDirection;
          int locoID = locoNumber + 1;  // DCC++ loco register starts at 1
          sprintf(printTxt, "<%s>", pch);
          webSocket.sendTXT(num, printTxt);  // confirm new settings to throttle
          if (locoNumber < totalLocos) {
            sprintf(printTxt, "<t %i %s %i %i>", locoID, ccc[locoNumber].pin, locoSpeed, locoDirection);
            Serial.println(printTxt);
          }
        }

      } else if (payloadCommand == 'Z') { // Turnouts, decouplers and sounds <Z Type&ID>

        pch += 2;
        char type = pch[0];
        char *cID;
        cID = pch+1;
        int id;
        sscanf(cID,"%d", &id);

        if (type == 'A') {  // Action: Change status of Turnouts, Decouplers, Signals and Lights
          turnoutAction(pch, id, num);

        } else if (type == 'J') { // hange status of sounds
          // Sound instructions from control device: 0=stopLoop, 1=soundDN, 2=soundUP, sounds: 3 to x
          // Instructions in DFPlayer: -1=soundDN, 0=soundUP, sounds: 1 to x, Action 0=stop sound, execute by DF Player library function
          pch++;
          sprintf(printTxt, "<J %s>", pch);
          Serial.println(printTxt);

        } else if (type == 'T') { // request for turnout data. SendTurnoutDataToDevice(id)
          int i = tOrder[id];
          tData t = ttt[i];
          int state = t.zStatus;
          char *icon = (state>0) ? t.icon1 : t.icon0;
          sprintf(printTxt, "<T %s %i %s %s %s>", pch, state, t.show, icon, t.title);
          Serial.print(String(strlen(printTxt)));
          Serial.println(printTxt);
          webSocket.sendTXT(num, printTxt);

        } else if (type == 'C') {// request for cab data
          int i = cOrder[id];
          cData c = ccc[i];
          sprintf(printTxt, "<C %s %s %s %s>", pch, c.show, c.icon0, c.title);
          webSocket.sendTXT(num, printTxt);

        }
      }
    }
  }
}

  //////////////////////////
 //  Throttle functions  //
//////////////////////////

void powerOff(uint8_t num) {
  const char *offText = "<0>";
  webSocket.sendTXT(num, offText);
  for (int x=0; x<totalLocos; x++) {
    LocoState[x][5]=0;
    LocoState[x][6]=0;
  }
  // sendText = "<t "+String(locoNumber)+" "+String(LocoState[locoNumber][5])+" "+String(LocoState[locoNumber][6])+">";  // Speed and direction to control device;
  // webSocket.sendTXT(num, sendText);
  Serial.println(offText);
  sprintf(printTxt, "<t %i %i %i>", locoNumber, LocoState[locoNumber][5], LocoState[locoNumber][6]);
  webSocket.sendTXT(num, printTxt);
}

void turnoutAction(char *pch, int id, uint8_t num) {
  // Return specified output. id already re-ordered
  int i = tOrder[id];
  tData t = ttt[i];
  int state = t.zStatus;
  char *icon;
  if (state > 0) {
    icon = t.icon0;
    state = 0;
  } else {
    icon = t.icon1;
    state = 1;
  }
  ttt[i].zStatus = state;
  sprintf(printTxt, "<Z %i %i>", i, state);
  Serial.println(printTxt);
  pch++;
  sprintf(printTxt, "<Y T%s %i %s>", pch, state, icon);
  webSocket.sendTXT(num, printTxt);

}  // turnoutAction

  /////////////////////////
 // Settings Functions  //
/////////////////////////

void sendSetUpData(uint8_t num, char *source) {  // for settings page
  char type = source[0];
  // char *pID = strtok(source, "TC");
  char *pID;
  pID = source + 1;
  int dID;
  sscanf(pID,"%d", &dID);
  if (type == 'T') {
    int id = tOrder[dID];
    tData t = ttt[id];
    sprintf(printTxt, "<I %s %s %s %s %s>", source, t.show, t.pin, t.icon0, t.title);
    webSocket.sendTXT(num, printTxt);

  } else if (type == 'C') {
    int id = cOrder[dID];
    cData c = ccc[id];
    sprintf(printTxt, "<I %s %s %s %s %s>", source, c.show, c.pin, c.icon0, c.title);
    webSocket.sendTXT(num, printTxt);

  }
}  // sendSetUpData

void saveItemData(uint8_t num, char *pch) {
  pch += 2;
  char toScan[15];
  strncpy(toScan, pch, 14);
  toScan[14] = '\0';
  char source[4];
  char icon0[5];
  char pin[4];
  char show[2];
  Serial.println("toScan:" + String(toScan) + ":");
  sscanf(toScan, "%s %s %s %s", source, show, pin, icon0);
  Serial.println("source:" + String(source));
  Serial.println("show:" + String(show));
  Serial.println("pin:" + String(pin));
  Serial.println("icon0:" + String(icon0) + ":");

  const char type = source[0];
  char *cID;
  cID = source+1;
  int dID;
  sscanf(cID, "%d", &dID);
  char *title;
  title = pch+15;
  Serial.println("title:" + String(title));
  if (type == 'T') {
    int id = tOrder[dID];
    strcpy(ttt[id].icon0, icon0);
    strcpy(ttt[id].pin, pin);
    strcpy(ttt[id].show, show);
    strcpy(ttt[id].title, title);
  } else if (type == 'C') {
    int id = cOrder[dID];
    strcpy(ccc[id].icon0, icon0);
    strcpy(ccc[id].pin, pin);
    strcpy(ccc[id].show, show);
    strcpy(ccc[id].title, title);
  }
  sprintf(printTxt, "<I %s %s %s %s %s>", source, show, pin, icon0, title);
  webSocket.sendTXT(num, printTxt);

}  // saveItemData()

void moveItem(char *source, char *target) { //  Move cab or turnout location in table
  char sType = source[0];
  char tType = target[0];
  if (sType = tType) {
    source++;
    target++;
    int idSource;
    int idTarget;
    sscanf(source,"%d", &idSource);
    sscanf(target,"%d", &idTarget);
    if (sType == 'T') {
      moveTurnouts(idSource, idTarget);
    } else if (sType == 'C') {
      moveCabs(idSource, idTarget);
    }
  }
}  // moveItem()

void moveTurnouts (int idSource, int idTarget) {
  int transferID = tOrder[idSource];
  if (idSource > idTarget) {
    for (int x=idSource; x>idTarget; x--) {
      tOrder[x]=tOrder[x-1];
    }
  } else {
    for (int x=idSource; x<idTarget; x++) {
      tOrder[x]=tOrder[x+1];
    }
  }
  tOrder[idTarget] = transferID;
}  // moveTurnouts ()

/*
void moveTurnouts (int idSource, int idTarget) {
  tData temp = ttt[idSource];
  if (idSource > idTarget) {
    for (int x=idSource; x>idTarget; x--) {
      ttt[x]=ttt[x-1];
    }
  } else {
    for (int x=idSource; x<idTarget; x++) {
      ttt[x]=ttt[x+1];
    }
  }
  ttt[idTarget] = temp;
}*/

/*
void moveCabs (int idSource, int idTarget) {
  cData temp = ccc[idSource];
  if (idSource > idTarget) {
    for (int x=idSource; x>idTarget; x--) {
      ccc[x]=ccc[x-1];
    }
  } else {
    for (int x=idSource; x<idTarget; x++) {
      ccc[x]=ccc[x+1];
    }
  }
  ccc[idTarget] = temp;
}*/

void moveCabs (int idSource, int idTarget) {
  int transferID = cOrder[idSource];
  if (idSource > idTarget) {
    for (int x=idSource; x>idTarget; x--) {
      cOrder[x]=cOrder[x-1];
    }
  } else {
    for (int x=idSource; x<idTarget; x++) {
      cOrder[x]=cOrder[x+1];
    }
  }
  cOrder[idTarget] = transferID;
}  // moveCabs()


  /////////////////////////
 ///   Init Functions   //
/////////////////////////

void loadOutputs() { // Accessories/Outputs
  // Update Outputs to DCC++
  int zStatus;
  for (tData t: ttt)
  {
      // Serial.println("<Z "+String(t.id)+" "+t.pin+" "+String(t.iFlag)+">");

      sprintf(printTxt, "<Z %i %s %i>", t.id, t.pin, t.iFlag);
      Serial.println(printTxt);
      zStatus = 0;
      if (t.zStatus > 2)
        zStatus = 1;
      // Serial.print("<Z "+String(t.id)+" "+String(zStatus)+">");
      sprintf(printTxt, "<Z %i %i>", t.id, zStatus);
      Serial.print(printTxt);
  }

}  //  loadOutputs()

  //////////////////////////
 //   Helper Functions   //
//////////////////////////

int invert(int value){
  if(value == 0)
    return 1;
  else
    return 0;

}  // invert

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
