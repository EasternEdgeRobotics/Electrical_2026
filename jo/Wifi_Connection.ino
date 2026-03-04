/*
All connection related functions should be in here.  A good start is https://docs.arduino.cc/tutorials/mkr-wifi-1010/web-server-ap-mode/
Add more functions if needed
*/

/*
 complete the initial checks and create the network
*/
#include <WiFiNINA.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);  // the Wifi radio's status your network key Index number (needed only for WEP)

void WifiSetup() {
 //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("You're connected to the network");
}
/*
 begin the wifi connection
 */
void StartConnection() {
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }
  delay(10000);// wait 10 seconds for connection:
  server.begin();// start the web server on port 80
}
/*
 main loop for the wifi connection.  calls other wifi functions that need to be executed at each iterations
 the function is not an infinite loop.  it gets called by loop() in Main
*/
void WifiLoop() {
  // TODO
  if(status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if(status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }
 
}

/*
 called when a client connects.  needs to determine what action needs to be done
 possible actions: send website, receive profile data, transmit inital info, start profile, send collected data
*/
WiFiClient client = server.available();   // listen for incoming clients
void ClientConnect() {
  // TODO
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
          }
        }
      }
    }
  }
}
/*
end the wifi connection
*/
void EndConnection() {
  // TODO
  client.stop();
  Serial.println("client disconnected");
}



