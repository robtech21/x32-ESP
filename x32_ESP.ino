// TODO
// Figure out how to make the program read from the stupid soundboard

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <WifiUdp.h>
#include <OSCMessage.h>
#include <OSCData.h>
#include <SPI.h>
#include "ESPRotary.h"

// Define LED and rotary encoder's button
#define LED D1 // D1(gpio5)
#define BUTTON D2 // D2(gpio4)

#define ROTARY_PIN1 D5
#define ROTARY_PIN2 D6

#define CLICKS_PER_STEP 2   // this number depends on your rotary encoder
#define MIN_POS         0
#define MAX_POS         100
#define START_POS       0
#define INCREMENT       1   // this number is the counter increment on each step

ESPRotary r;

int switchState     = 0;
int oldSwitchState  = 0;
int lightsOn        = 0;
int channelOn;

WiFiUDP Udp;

const char* ssid      = "ssid";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password  = "password";     // The password of the Wi-Fi network
const char* X32IP     = "x.x.x.x";      // The IP of the X32 sound board

char packetBuffer[255]; 
IPAddress             destinationIP   (192, 168, 1, 141);

const unsigned int         ourPort = 10023;
const unsigned int destinationPort = 10023;

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(1000);

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  
  pinMode(BUTTON,INPUT);
  pinMode(LED, OUTPUT);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  Udp.begin(10023);
}

void channelState(int state) {
  // Will take either 0 or 1; 0 is off and 1 is on
  Udp.beginPacket(destinationIP, destinationPort);
  OSCMessage msg("/mtx/06/mix/on");
  msg.add(state);
  msg.send(Udp);
  msg.empty();
  Udp.endPacket();
}

void getInfo() {
  Udp.beginPacket(destinationIP, destinationPort);
  OSCMessage msg("/info");
  msg.send(Udp);
  msg.empty();
  Udp.endPacket();
}

void valueSet(float value) {
  Udp.beginPacket(destinationIP, destinationPort);
  OSCMessage msg("/mtx/06/mix/fader");
  msg.add(value);
  msg.send(Udp);
  msg.empty();
  Udp.endPacket();
}

void onOffToggle() {
  switchState = digitalRead(BUTTON);
  if (switchState == oldSwitchState) {
    // Do nothing if we didn't change the switch state
    return;
  }
  
  // Cache state
  oldSwitchState = switchState;

  // Switch light state when button pressed.
  if (switchState == HIGH) { 
    lightsOn = !lightsOn; // Global variable
  }

  if (lightsOn) {
    // Turn on (This works better as you can't have glitches because of the toggling)
    digitalWrite(LED,LOW);
    Serial.println("Toggle channel on");
    channelState(1);
    return;
  }
  
  // Turn off
  digitalWrite(LED,HIGH);
  Serial.println("Toggle channel off");
  channelState(0);
}

void testPacket() {
  signed int value = 1;
  Serial.println("Running test packet");
  delay(500);
  channelState(1);
  delay(500);
  valueSet(1);
  delay(1000);
  channelState(0);
  delay(500);
  valueSet(0.4);
}

void readPacket() {
  int packetSize = Udp.parsePacket();

  if (packetSize) {

    Serial.print("Received packet of size ");

    Serial.println(packetSize);

    Serial.print("From ");

    IPAddress remoteIp = Udp.remoteIP();

    Serial.print(remoteIp);

    Serial.print(", port ");

    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer

    int len = Udp.read(packetBuffer, 255);

    if (len > 0) {

      packetBuffer[len] = 0;

    }

    Serial.println("Contents:");

    Serial.println(packetBuffer);  
}
}

void xremote() {
  Udp.beginPacket(destinationIP, destinationPort);
  OSCMessage msg("/xremote");
  msg.add(1);
  Serial.print("Message type:");
  char result = msg.getType(0);
  Serial.println(result);
  msg.send(Udp);
}

void getFader() {
  Udp.beginPacket(destinationIP, destinationPort);
  xremote();
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.println("Got packet");
    int len = Udp.read(packetBuffer,255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents of packet:");
    Serial.println(packetBuffer);
  }
}

/* Main loop. We run the setup() function defined further up this file, then we run this 
 * bit until the arduino gets turned off.
 * We check 6 channels, and only check each channel if it is enabled in channel_enabled[] at the start of this file.
 */

int loopnum = 0;

void loop() {
    r.loop();
    //testPacket();
    onOffToggle();

    
    ++loopnum;
    if (loopnum == 10000) {
      //xremote();
      loopnum = 0;
    }
}


// on change
void rotate(ESPRotary& r) {
   float rotaryPos = r.getPosition();
   float sendPos = (rotaryPos / 100);
   Serial.print("Actual Value: ");
   Serial.println(rotaryPos);
   Serial.print("Value to send to board: ");
   valueSet(sendPos);
   Serial.println(sendPos);
}

// on left or right rotation
void showDirection(ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
}
