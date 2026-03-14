/*
All connection related functions should be in here.  A good start is https://docs.arduino.cc/tutorials/mkr-wifi-1010/web-server-ap-mode/
Add more functions if needed
*/

/*
 complete the initial checks and create the network
*/

const char ssid[] = "EER_Profiler";
const char pass[] = "CrazyAssPassword69!";
const int NUMBER_OF_FILES = 4;
const String FILE_NAMES[NUMBER_OF_FILES] = {"index.htm", "output.css", "dygraph.js", "jquery.js"};
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

void readandSendFile(WiFiClient *client, String name) {
  File websiteFile = SD.open(name, FILE_READ);
  if (websiteFile) {
    Serial.println(name);
    while (websiteFile.available()) {
      client->write(websiteFile.read());
    }
    websiteFile.close();
  }
  
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
          }
          else { // useless info reset buffer
            Serial.println(currentLine);
            currentLine = "";
          }
        }
        else if (c != '\r') { //data being received.  append
          currentLine += c;
        }
        if (currentLine.startsWith("GET / ") || currentLine.startsWith("GET /HTTP")) {
          // nothing was asked, so first connection.  send web page
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          readandSendFile(&client, FILE_NAMES[0]);
          break;
        }
        if (currentLine.startsWith("GET /output.css")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/css");
          client.println("Connection: close");
          client.println();
          readandSendFile(&client, FILE_NAMES[1]);
          break;
        }
        if (currentLine.startsWith("GET /jquery.js")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/javascript");
          client.println("Connection: close");
          client.println();
          readandSendFile(&client, FILE_NAMES[3]);
          break;
        }
        if (currentLine.startsWith("GET /dygraph.js")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/javascript");
          client.println("Connection: close");
          client.println();
          readandSendFile(&client, FILE_NAMES[2]);
          break;
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
