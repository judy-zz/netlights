#include <SPI.h>
#include <WS2801.h>
#include <Ethernet.h>

int dataPin = 2;
int clockPin = 3;
int numberOfPixels = 50;
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10,55,55,6); // Use a manual IP Address!
String currentLine;
String nextColor;
boolean grabbingColor;

EthernetServer server(80); // (port 80 is default for HTTP):
WS2801 strip = WS2801(numberOfPixels, dataPin, clockPin);

void setup() {
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);

  // Update LED contents, to start they are all 'off'
  strip.begin();
  strip.show();
}

void loop() {
  // Some example procedures showing how to display to the pixels
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        // read incoming bytes:
        char inChar = client.read();

        // add incoming byte to end of line:
        currentLine += inChar; 

        // if you get a newline, clear the line:
        if (inChar == '\n') {
          resetColorGrabbingProcess();
        } 
        // if you're currently reading the bytes of a color,
        // add them to the color String:
        if (grabbingColor) {
          if (nextColor.length() != 6) {
            nextColor += inChar;
          } 
          else {
            int red   = hexStringToByte(nextColor[0], nextColor[1]);
            int green = hexStringToByte(nextColor[2], nextColor[3]);
            int blue  = hexStringToByte(nextColor[4], nextColor[5]);
            
            colorWipe(Color(red, green, blue), 50);
            resetColorGrabbingProcess();
          }
        }
        // if the current line ends with #, it will be followed by a color. We run this AFTER processing the color command,
        // so the # sign isn't included in the string.
        if ( currentLine.endsWith("#")) {
          resetColorGrabbingProcess();
          grabbingColor = true;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }

  //  colorWipe(Color(255, 00, 0), 50);
}

void resetColorGrabbingProcess() {
  grabbingColor = false;
  nextColor = "";
  currentLine = "";
}

int hexStringToByte(char char1, char char2)
{
  return (hexCharToByte(char1) * 16 + hexCharToByte(char2));
}

int hexCharToByte(char c)
{
  if (c >= 48 && c <= 57) {
    return (c - 48);
  } 
  else if (c >= 65 && c <= 70) {
    return (c - 55);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;

  for (i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170; 
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

