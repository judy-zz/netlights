#include <SPI.h>
#include <WS2801.h>
#include <Ethernet.h>

int dataPin = 2;
int clockPin = 3;
int numberOfPixels = 50;
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,15,15); // Use a manual IP Address!
String currentLine;
String nextColor;
boolean grabbingColor;

int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;
int nextRed = 0;
int nextGreen = 0;
int nextBlue = 0;

EthernetServer server(23); 
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
            nextRed   = hexStringToInt(nextColor[0], nextColor[1]);
            nextGreen = hexStringToInt(nextColor[2], nextColor[3]);
            nextBlue  = hexStringToInt(nextColor[4], nextColor[5]);
            
            //colorWipe(Color(nextRed, nextGreen, nextBlue), 100);
            colorMiddleWipe(Color(nextRed, nextGreen, nextBlue), 100);
            // colorShow(Color(nextRed, nextGreen, nextBlue));
            // colorGradient(nextRed, nextGreen, nextBlue, 10);

            currentRed   = hexStringToInt(nextColor[0], nextColor[1]);
            currentGreen = hexStringToInt(nextColor[2], nextColor[3]);
            currentBlue  = hexStringToInt(nextColor[4], nextColor[5]);
            
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

}

void resetColorGrabbingProcess() {
  grabbingColor = false;
  nextColor = "";
  currentLine = "";
}

int hexStringToInt(char char1, char char2)
{
  // We don't have enough power! Dividing total power by 4...
  // return (hexCharToInt(char1) * 16 + hexCharToInt(char2));
  return ((hexCharToInt(char1) * 16 + hexCharToInt(char2)) / 6.1);
}

int hexCharToInt(char c)
{
  if (c >= 48 && c <= 57) {
    return (c - 48);
  } 
  else if (c >= 65 && c <= 70) {
    return (c - 55);
  }
}

// fill the dots one after the other with said color
void colorWipe(uint32_t c, uint8_t wait) {
  int i;

  for (i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// fill the dots one after the other with said color. start from the middle.
void colorMiddleWipe(uint32_t c, uint8_t wait) {
  int i;
  
  int halfNumOfPixels = strip.numPixels() / 2;

  for (i=0; i < halfNumOfPixels; i++) {
    strip.setPixelColor(i + halfNumOfPixels, c);
    strip.setPixelColor(halfNumOfPixels - i, c);
    strip.show();
    delay(wait);
  }
}

// fill the the entire strip said color
void colorShow(uint32_t c) {
  int i;

  for (i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

// slowly fade from current color into next color
void colorGradient(int red, int green, int blue, uint8_t wait) {
  int i, j;

  for (i=0; i < 80; i += 1) {
    float percent = i * 0.001;
    uint32_t color = gradientValue(currentRed, currentGreen, currentBlue, red, green, blue, percent);
    for (j=0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, color);
    }
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

// returns a color between beginning rgb and end rgb colors based upon percent of the range
uint32_t gradientValue(int br, int bg, int bb, int er, int eg, int eb, float percent) {
    int red   = br + (percent * (er - br));
    int green = bg + (percent * (eg - bg));
    int blue  = bb + (percent * (eb - bb));
//    if (red   < er && br > er)   { red   = er; };
//    if (green < eg && bg > eg)   { green = eg; };
//    if (blue  < eb && bb > eb)   { blue  = eb; };
//    if (red   > er && br < er)   { red   = er; };
//    if (green > eg && bg < eg)   { green = eg; };
//    if (blue  > eb && bb < eb)   { blue  = eb; };
    return Color(red, green, blue);
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

