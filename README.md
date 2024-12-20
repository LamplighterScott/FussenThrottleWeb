# FussenThrottleSafari

ESP-01 based WiFi server for DCC++.

Provides throttle control via a web browser (Safari) as the client.

Serves up web page from files on EEPROM on ESP-01.

Files consist of two sets of CCS, HTML and JavaScript for 1) run control and 2) set-up/decoder programmming.

Decoder programming is limited to ID, accelleration, decelleration, reset and Velmo motor settings.  More thorough programming of decoders can be done with JMRI running on a PC connnected to the Arduindo Mega via USB.

In order to use JMRI, the communication setting in DCC++ needs to be changed accordingly.  In the Config.h file, change the COMM_INTERFACE value to "0" for JMRI or "4" for ESP-01 control.  In JMRI mode, communication to the ESP-01 and the DF Player are disabled.  JMRI can be used to completely control locomotives, tracks and signals.

Communication between the ESP-01 and web browser uses WebSockets.

Sends commands via serial to an Arduino MEGA running "DCC++ Base Station sketch branched for Marklin Z".
