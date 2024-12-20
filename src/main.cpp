#include <WiFi.h>
#include <BluetoothSerial.h>

// Wi-Fi Credentials
const char* ssid = "ESP32-AP";
const char* password = "12345678";

// Wi-Fi Server
WiFiServer server(80);
String header;

// Bluetooth Serial
BluetoothSerial SerialBT;

// GPIO States
String output26State = "off";
String output27State = "off";
String output2State = "off";

// GPIO Pins
const int output26 = 26;
const int output27 = 27;
const int output2 = 2;

// Timeout Variables
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize Bluetooth
  SerialBT.begin("ESP32_BT_Server"); // Set Bluetooth device name
  Serial.println("Bluetooth started. Awaiting connection...");

  // Initialize GPIO Pins
  pinMode(output2, OUTPUT);
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);

  digitalWrite(output2, LOW);
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Start Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start Server
  server.begin();
}

void loop(){
  // Handle Wi-Fi Client
  WiFiClient client = server.available();
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // Handle Web Requests
            if (header.indexOf("GET /26/on") >= 0) {
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              output27State = "off";
              digitalWrite(output27, LOW);
            }
            // Send HTML Response
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>html { font-family: Helvetica; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
  }

  // Handle Bluetooth Commands
  if (SerialBT.available())
  {
    String btCommand = SerialBT.readStringUntil('\n');
    btCommand.trim(); // Remove trailing newline or spaces
    Serial.println("Bluetooth Command: " + btCommand);

    if (btCommand == "26_ON")
    {
      output26State = "on";
      digitalWrite(output26, HIGH);
      SerialBT.println("GPIO 26 turned ON");
    }
    else if (btCommand == "26_OFF")
    {
      output26State = "off";
      digitalWrite(output26, LOW);
      SerialBT.println("GPIO 26 turned OFF");
    }
    else if (btCommand == "27_ON")
    {
      output27State = "on";
      digitalWrite(output27, HIGH);
      SerialBT.println("GPIO 27 turned ON");
    }
    else if (btCommand == "27_OFF")
    {
      output27State = "off";
      digitalWrite(output27, LOW);
      SerialBT.println("GPIO 27 turned OFF");
    }
    else if (btCommand == "2_ON")
    { // Control for GPIO2
      digitalWrite(output2, HIGH);
      SerialBT.println("GPIO 2 turned ON");
    }
    else if (btCommand == "2_OFF")
    { // Control for GPIO2
      digitalWrite(output2, LOW);
      SerialBT.println("GPIO 2 turned OFF");
    }
    else
    {
      SerialBT.println("Unknown Command");
    }
  }
}
