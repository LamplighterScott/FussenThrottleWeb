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
#include <Arduino.h>

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
  char icon0[8];  // icons for state 0 (closed) and 1 (thrown)
  char icon1[8];
  byte iFlag;  // See DCC++ Outlets for bits explanation, Fuseen Throttle uses bits 3-6 to communicate output type to DCC++ in Load Outputs
              // bit0 not used in Fussen Throttle, bit1: 0=hide, 1=show; bit2: 0=disabled or rund, 1= enable or gerade
              // bit no. (int value) T=3(8), D=4(16), S=5(32), L=6(64)
} tData;
/* Format {Switch number, pin number, name, type, X, X, X} */
tData ttt[]= {
  {0, "022", "1 To outer", "T", "\U00002196\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {1, "024", "2 To inner", "T", "\U00002197\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {2, "026", "3 Yard 1", "T", "\U00002196\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {3, "028", "4 Yard 1-1", "T", "\U00002197\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {4, "030", "5 Outer to Yard", "T", "\U00002196\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {5, "032", "6 Cross to Yard", "T", "\U0001F504", "\U0001F500", 10},
  {6, "034", "7 Yard 2-2", "T", "\U00002197\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {7, "036", "8 Sidetrack", "T", "\U00002197\U0000FE0F", "\U00002B06\U0000FE0F", 10},
  {8, "038", "A Decoupler 1A", "D", "\U0001F9F2", "\U0001F9F2", 16},
  {9, "040", "B Decoupler 1B", "D", "\U0001F9F2", "\U0001F9F2", 16},
  {10, "042", "C Decoupler 2A", "D", "\U0001F9F2", "\U0001F9F2", 16},
  {11, "044", "D Decoupler 2B", "D", "\U0001F9F2", "\U0001F9F2", 16},
  {12, "046", "Semaphore", "S", "\U0001F6A5", "\U0001F6A6", 32},
  {13, "048", "Castle", "L", "\U0001F3F0", "\U0001F3F0", 64},
  {14, "048", "Signal Tressle", "L", "\U0001F3F0", "\U0001F3F0", 64}
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
  {0, "111", "SBB 111", "\U0001F682", "1"},
  {1, "003", "DB 112", "\U0001F683", "1"},
  {2, "046", "CFF FFS123", "\U0001F682", "1"},
  {3, "003", "DB 239", "\U0001F686", "1"},
  {4, "000", "Steam", "\U0001F682", "0"},
  {5, "000", "Bullet", "\U0001F685", "0"},
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

const int maxCommandLength = 30;
char commandString[30+1];
char c;
int outputToLoad=0;

void startWiFi();
void startFS();
void startWebSocket();
void startMDNS();
void startServer();
void handleSerialCommand();
void handleRoot();
void handleSetup();
bool handleFileRead(String path);
void handle_CaptivePortal();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void powerOff(uint8_t num);
void outputAction(char *pch, int id, uint8_t num);
void sendSetUpData(uint8_t num, char *pch, char *source);
void saveItemData(char *pch);
void saveTitle(char *pch);
void moveItem(char *source, char *target);
void moveTurnouts (int idSource, int idTarget);
void moveCabs (int idSource, int idTarget);
void loadOutputs(bool startLoading);
void retrieveSettings();
int invert(int value);
String getContentType(String filename);

//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  delay(1500); // From WiThrottle
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  Serial.flush();
  // Serial.setDebugOutput(true);
  
  startWiFi();
  startFS();

  retrieveSettings();
  delay(10);

  startWebSocket();
  startMDNS();
  startServer();

  // loadOutputs();  // register outputs with Arduino
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
  handleSerialCommand();
  
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

void startFS() {
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
  // Serial.println("HTTP server started");

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

//===============================================================
//   Serial Handlers
//===============================================================

void handleSerialCommand() {

  while(Serial.available()>0)
    {
      c=Serial.read();
      if(c=='<') {
          // sprintf(commandString,"");
          memset(commandString, 0, sizeof commandString);
      } else if(c=='>') {
      // if(c=='>') {

        if (commandString[0] == 'C') {
          loadOutputs(false);

        } else if (commandString[0] == 'S') {
          loadOutputs(true);
        }
      // } else if (c!='<') {
      } else {
        int commandStringLength = (unsigned)strlen(commandString);
        if(commandStringLength < maxCommandLength) {
          sprintf(commandString,"%s%c",commandString,c);
        }
      }
    }
}


//===============================================================
//   Websocket Handler
//===============================================================

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
      // Serial.printf("[%u] get Text: %s\n", num, payload);
      char *pch = strtok((char *)payload,"<>");

      //const char payloadCommand = (const char) payload[1];
      char payloadCommand = payload[1];

      if (payloadCommand == '0') {                    // Power off
        powerOff(num);

      } else if (payloadCommand == '1') {             // Power on
        const char *on = "<1>";
        Serial.println(on);
        webSocket.sendTXT(num, on);

      } else if (payloadCommand == 'C') {   // used for getting loco status when changing locos
        // Send speed and direction for all throttles
        pch++;
        int lN;
        if (sscanf(pch,"%d", &lN)==1) {
          locoNumber = cOrder[lN];
          sprintf(printTxt, "<t %i %i %i>", locoNumber, LocoState[locoNumber][5], LocoState[locoNumber][6]);
          webSocket.sendTXT(num, printTxt);
        }

      }  else if (payloadCommand == 'F') {             // Loco functions <F ID Loco>
        int fKey;
        pch++;
        if (sscanf(pch,"%d", &fKey)==1) {
          LocoState[locoNumber][fKey] = invert(LocoState[locoNumber][fKey]);
          int keyFunc = 128+LocoState[locoNumber][1]*1+LocoState[locoNumber][2]*2+LocoState[locoNumber][3]*4+LocoState[locoNumber][4]*8+LocoState[locoNumber][0]*16;
          if (locoNumber < totalLocos) {
            //Serial.println("<f " + String(ccc[locoNumber].pin) + " " + String(keyFunc) + ">");  // DCC++ returns?
            sprintf(printTxt, "<f %s %i>", ccc[locoNumber].pin, keyFunc);
            Serial.println(printTxt);
          }
        }

      } else if (payloadCommand == 'I' || payloadCommand == 'M') {  // settings: set-up, move and edit
        char source[4];
        char target[4]; // also icon
        int x = sscanf(pch+2,"%s %s", source, target);
        if (x == 1) {
          sendSetUpData(num, pch, source);
        } else if (x == 2) {
          moveItem(source, target);
        }

      } else if (payloadCommand == 'K') {
        saveItemData(pch);  // Updates ESP array less title

      } else if (payloadCommand == 'L') {
        saveTitle(pch);  // Updates ESP array for title

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
          outputAction(pch, id, num);

        } else if (type == 'J') { // change status of sounds
          // Sound instructions from control device: 0=stopLoop, 1=soundDN, 2=soundUP, sounds: 3 to x
          // Instructions in DFPlayer: -1=soundDN, 0=soundUP, sounds: 1 to x, Action 0=stop sound, execute by DF Player library function
          pch++;
          sprintf(printTxt, "<J %s>", pch);
          Serial.println(printTxt);

        } else if (type == 'T') { // request for turnout data. SendTurnoutDataToDevice(id)
          int i = tOrder[id];
          tData t = ttt[i];
          // int state = t.zStatus;
          int state = bitRead(t.iFlag, 2);
          char *icon = (state>0) ? t.icon1 : t.icon0;
          int show = bitRead(t.iFlag, 1);
          sprintf(printTxt, "<T %s %i %i %s %s>", pch, state, show, icon, t.title);
          // Serial.print(String(strlen(printTxt)));
          // Serial.println(printTxt);
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

void outputAction(char *pch, int id, uint8_t num) {
  // Return specified output. id already re-ordered
  int i = tOrder[id];
  tData t = ttt[i];
  // int state = t.zStatus;
  int state = bitRead(t.iFlag, 2);
  char *icon = t.icon0;
  if (t.type[0] == 'D') {
    state = 0;
  } else {
    state = (state>0) ? 0 : 1; // flip state for turnouts and signals
    if (t.type[0] != 'L') {
      if (state>0) {
       icon = t.icon1;
      }
    }
  }

  // update ttt
  bitWrite(ttt[i].iFlag, 2, state);
  bitWrite(t.iFlag, 2, state);

  // save to EEPROM
  char path[4];
  sprintf(path, "/T%d", t.id);
  // if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "w");
    file.write((byte *)&t, sizeof(t));
    file.close();
 //}

  sprintf(printTxt, "<Z %i %i>", i, state);
  Serial.println(printTxt);
  pch++;
  sprintf(printTxt, "<Y T%s %i %s>", pch, state, icon);
  webSocket.sendTXT(num, printTxt);

}  // outputAction

  /////////////////////////
 // Settings Functions  //
/////////////////////////

void retrieveSettings() {
  char path[4];
  for (int id = 0; id<totalOutputs; ++id) {
    sprintf(path, "/T%d", id);
    if (SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "r");
      tData t = ttt[id];
      file.read((byte *)&t, sizeof(t));
      strcpy(ttt[id].icon0, t.icon0);
      strcpy(ttt[id].icon1, t.icon1);
      strcpy(ttt[id].type, t.type);
      strcpy(ttt[id].pin, t.pin);
      // strcpy(ttt[id].show, t.show);
      // ttt[id].zStatus, t.zStatus;
      ttt[id].iFlag = t.iFlag;
      file.close();
    }
  }

  for (int id=0; id<totalLocos; ++id) {
    sprintf(path, "/C%d", id);
    if (SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "r");
      cData c = ccc[id];
      file.read((byte *)&c, sizeof(c));
      strcpy(ccc[id].icon0, c.icon0);
      strcpy(ccc[id].pin, c.pin);
      strcpy(ccc[id].show, c.show);
      file.close();
    }
  }

  char tPath[] = "/tOrder";
  if (SPIFFS.exists(tPath)) {
    File file = SPIFFS.open(tPath, "r");
    file.read((byte *)&tOrder, sizeof(tOrder));
    file.close();
  }

  char cPath[] = "/cOrder";
  if (SPIFFS.exists(cPath)) {
    File file = SPIFFS.open(cPath, "r");
    file.read((byte *)&cOrder, sizeof(cOrder));
    file.close();
  }
}

void sendSetUpData(uint8_t num, char *pch, char *source) {  // for settings page
  char callCode = pch[0];
  char type = source[0];
  char *pID;
  pID = source + 1;
  int dID;
  sscanf(pID,"%d", &dID);
  if (type == 'T') {  // Outputs
    int id = tOrder[dID];
    tData t = ttt[id];
    int show = bitRead(t.iFlag, 1);
    if (t.type[0] == 'T' || t.type[0] == 'S') {
      if (callCode == 'M') {    // Called by FussenSetUpJS
        sprintf(printTxt, "<K %i %s %s%s %s>", show, t.pin, t.icon0, t.icon1, t.type);
        webSocket.sendTXT(num, printTxt);
        sprintf(printTxt, "<L %s>", t.title);
      } else {  // Called by FussenJS
        sprintf(printTxt, "<I %s %i %s %s%s %s>", source, show, t.pin, t.icon0, t.icon1, t.title);
      }
    } else {
        if (callCode == 'M') {  // Called by FussenSetUpJS
          sprintf(printTxt, "<K %i %s %s %s>", show, t.pin, t.icon0, t.type);
          webSocket.sendTXT(num, printTxt);
          sprintf(printTxt, "<L %s>", t.title);
        } else {  // Called by FussenJS
          sprintf(printTxt, "<I %s %i %s %s %s>", source, show, t.pin, t.icon0, t.title);
        }
    }
    webSocket.sendTXT(num, printTxt);

  } else if (type == 'C') { // Cabs
    int id = cOrder[dID];
    cData c = ccc[id];
      if (callCode == 'M') {
        sprintf(printTxt, "<K %s %s %s>", c.show, c.pin, c.icon0);
        webSocket.sendTXT(num, printTxt);
        sprintf(printTxt, "<L %s>", c.title);  // Called by FussenSetUpJS
      } else {
        sprintf(printTxt, "<I %s %s %s %s %s>", source, c.show, c.pin, c.icon0, c.title);  // Called by FussenJS
      }
      webSocket.sendTXT(num, printTxt);

  }
}  // sendSetUpData

void saveItemData(char *pch) {  // from settings page, modal edit
  char source[4];
  char icon0[8];
  char icon1[8];
  char tType[2];
  char pin[4];
  char show[2];
  if (sscanf(pch+2, "%s %s %s %s %s %s", source, show, tType, pin, icon0, icon1) > 4) {  // Outputs
    const char type = source[0];
    char *cID;
    cID = source+1;
    int dID;
    sscanf(cID, "%d", &dID);
    
    if (type == 'C') { // Cabs, z=zero/zeht/no type
      int id = cOrder[dID];
      strcpy(ccc[id].icon0, icon0);
      strcpy(ccc[id].pin, pin);
      strcpy(ccc[id].show, show);

    } else {  // Outputs T, D, S, L
      int iShow = atoi(show);
      int id = tOrder[dID];
      tData t=ttt[id];
      strcpy(ttt[id].icon0, icon0);
      strcpy(ttt[id].icon1, icon1);
      int tShow = bitRead(t.iFlag, 1);
      bool changeArduino = !(strcmp(t.type,tType) && strcmp(t.pin,pin) && tShow==iShow);
      if (changeArduino) {
        sprintf(printTxt, "<Z %i>", t.id);  // Remove output
        Serial.println(printTxt);
        delay(50);
        strcpy(ttt[id].type, tType);
        strcpy(ttt[id].pin, pin);
        bitWrite(t.iFlag, 1, iShow);
        // strcpy(ttt[id].show, show);
        int bitToChange = 0;
        switch (tType[0]) {
          case 'T': {  // Turnout
            bitToChange=3;
            break;
          }
          case 'D': {  // Decoupler
            bitToChange=4;
            break;
          }
          case 'S': {  // Semaphore
            bitToChange=5;
            break;
          }
          case 'L': {  // Light
            bitToChange=6;
            break;
          }
        }
        if (bitToChange>2 && bitToChange<7) {
          byte mask = 131;  // bits 0(1),1(2), 7(128)
          byte iFlag = t.iFlag & mask; // zero out type bits and status bit
          bitWrite(iFlag, bitToChange, 1);  // change to new type
          ttt[id].iFlag = iFlag;
          if (iShow > 0) {  // Add output. if show
            sprintf(printTxt, "<Z %i %s %i>", id, pin, iFlag);
            Serial.println(printTxt);
          }
        }
      }
    } 
  }
} // saveItemData()

void saveTitle(char *pch) {  // from settings page, modal edit
  
  const char type = pch[2];
  char cID[3];
  strncpy(cID, pch+3, 2);
  cID[2] = '\0';
  int dID;
  sscanf(cID, "%d", &dID);

  char title[16];
  strncpy(title, pch+6, 15);
  title[15] = '\0';

  if (type == 'T') {
    int id = tOrder[dID];
    strcpy(ttt[id].title, title);
    tData t = ttt[id];

    char path[4];
    sprintf(path, "/T%d", t.id);
    //if (SPIFFS.exists(path)) {
      File file = SPIFFS.open(path, "w");
      file.write((byte *)&t, sizeof(t));
      file.close();
    // }

  } else if (type == 'C') {
    int id = cOrder[dID];
    strcpy(ccc[id].title, title);
    cData c = ccc[id];

    char path[4];
    sprintf(path, "/C%d", c.id);
      File file = SPIFFS.open(path, "w");
      file.write((byte *)&c, sizeof(c));
      file.close();
  }
}  // saveTitle()

void moveItem(char *source, char *target) { //  Move cab or turnout location in table
  char sType = source[0];
  char tType = target[0];
  if (sType == tType) {
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
  char path[] = "/tOrder";
  File file = SPIFFS.open(path, "w");
  file.write((byte *)&tOrder, sizeof(tOrder));
  file.close();
}  // moveTurnouts ()

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
  char path[] = "/cOrder";
  File file = SPIFFS.open(path, "w");
  file.write((byte *)&cOrder, sizeof(cOrder));
  file.close();
}  // moveCabs()


  //////////////////////////////////
 ///   Load Outputs to Arduino   //
//////////////////////////////////

void loadOutputs(bool startLoading) { // Accessories/Outputs

  if (startLoading) {
    Serial.println("<e>");  // clear Arduino eeprom
    outputToLoad = 0;
  } else {
    outputToLoad++;
  }

  while (outputToLoad<totalOutputs) {
    tData t = ttt[outputToLoad];
    if (bitRead(t.iFlag,1) > 0) {
      sprintf(printTxt, "<Z %i %s %i>", t.id, t.pin, t.iFlag);  // create output
      Serial.println(printTxt);
      break;
    } else {
      outputToLoad++;
    }
  }
  
  // Update Outputs to DCC++
  
  // delay(1000);
  /* int waitCycles=0;
   for (tData t: ttt)
  {
    if (atoi(t.show) > 0) {
      sprintf(printTxt, "<Z %i %s %i>", t.id, t.pin, t.iFlag);
      Serial.println(printTxt);
      waitCycles = 0;
      while(handleSerialCommand()<1) {
        delay(20);
        waitCycles++;
        if (waitCycles>10) {
          Serial.println("<H Loading Timeout>");
          break;
        }
      }

    }
  }*/
}  //  loadOutputs()


  //////////////////////////
 //   Helper Functions   //
//////////////////////////

int invert(int value) {
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
