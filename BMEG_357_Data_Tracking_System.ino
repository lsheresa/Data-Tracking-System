#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>

// rocker switch right slot (gold) is VCC; middle slot is PIN 8; left slot is GND
// LED short leg to GND; long leg to PIN 13
int switch_pin = 8;
int led = 13;

char ssid[] = "TELUS965C";          //  your network SSID (name)
char pass[] = "6048732315";         //  your network password

char server[] = "script.google.com";
char fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbywGr6tAcBCNOxrfus4waZvajp43ZGA0ZWNkBm37GnSGdVVtt05aw5uoo9mrm742hSptg";

int status = WL_IDLE_STATUS;
//IPAddress server(74, 125, 115, 105); // Google


// Initialize the client library
WiFiClient client;

void setup() {
  Serial.begin(9600);
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    // don't do anything else:
    while (true);
  }
  else {
    Serial.println("Connected to wifi");
    Serial.println("\nStarting connection...");
    // if you get a connection, report back via serial:
    /*if (client.connect(server, 80)) {
      Serial.println("connected");
      // Make a HTTP request:
      client.println("GET /search?q=arduino HTTP/1.0");
      client.println();
      }*/
  }
  pinMode(switch_pin, INPUT_PULLUP);
  pinMode(led, OUTPUT);

  while (!Serial);  // wait for serial initialization
  Serial.print("LSM6DS3 IMU initialization ");
  if (IMU.begin()) {  // initialize IMU
    Serial.println("completed successfully.");
  } else {
    Serial.println("FAILED.");
    IMU.end();
    while (1);
  }
}

// Function for sending data to Google Spreadsheet
void sendData(float x, float y, float z) {
  Serial.print("Connecting to: ");
  Serial.println(server);

  String url = "/macros/s/" + GAS_ID + "/exec?x=" + x + "&y=" + y + "&z=" + z;
  Serial.print("requesting URL: ");
  Serial.println(url);

  if (client.connect(server, 80)) {
    Serial.println("Connected to host");
    client.println("GET " + url + " HTTP/1.1");
    Serial.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + (String)server + "\r\n"); 
    client.println("Connection: close\r\n\r\n");
    Serial.println("Connection: close\r\n\r\n");
    client.println();
  }

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

}

void loop() {
  float x, y, z;
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    Serial.print("Acceleration x: ");
    Serial.print(x);
    Serial.print('\t');
    Serial.print("Acceleration y: ");
    Serial.print(y);
    Serial.print('\t');
    Serial.print("Acceleration z: ");
    Serial.println(z);
  }

  sendData(x, y, z);
  delay(1000);
  /*
    if (digitalRead(switch_pin) == HIGH) {
      digitalWrite(led, HIGH);
      Serial.println("ON");
    }
    if (digitalRead(switch_pin) == LOW) {
      digitalWrite(led, LOW);
      Serial.println("OFF");
    }
  */
}
