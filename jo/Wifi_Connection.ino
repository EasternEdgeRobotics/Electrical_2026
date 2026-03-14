/*
All connection related functions should be in here.  A good start is https://docs.arduino.cc/tutorials/mkr-wifi-1010/web-server-ap-mode/
Add more functions if needed
*/

/*
 complete the initial checks and create the network
*/
#include <WiFiNINA.h>

const char ssid[] = "EER_Profiler";
const char pass[] = "CrazyAssPassword69!";
const int NUMBER_OF_FILES = 4;
const String FILE_NAMES[NUMBER_OF_FILES] = {"index.txt", "output.txt", "dygraph.min.txt", "jquery.min.txt"};
int status = WL_IDLE_STATUS;

WiFiServer server(80);

void WifiSetup() {

  Serial.println("Access Point Web Server");

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(10000);

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();
}

/*
 main loop for the wifi connection.  calls other wifi functions that need to be executed at each iterations
 the function is not an infinite loop.  it gets called by loop() in Main
*/
void WifiLoop() {
  // listen for incoming clients
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");

    String currentLine = "";

    while (client.connected()) {
      delayMicroseconds(10);
      
      if (client.available()) {
        char c = client.read();

        if (c == '\n') {// end of line
          if (currentLine.length() == 0) { // end of HTTP request
            // nothing was asked, so first connection.  send web page
            for(int i = 0; i < NUMBER_OF_FILES; i++) {
              Serial.println("1");
              File websiteFile = SD.open(FILE_NAMES[i], FILE_READ);
              if (websiteFile) {
                Serial.println("2");
                while (websiteFile.available()) {
                  client.write(websiteFile.read());
                }
              }
              Serial.println("3");
              websiteFile.close();
            }
            
            Serial.println("4");
            break;
          }
          else { // useless info reset buffer
            currentLine = "";
          }
        }
        else if (c != '\r') { //data being received.  append
          currentLine += c;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
