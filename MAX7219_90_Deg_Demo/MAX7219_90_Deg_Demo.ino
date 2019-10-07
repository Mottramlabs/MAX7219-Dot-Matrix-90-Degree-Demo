/*

  September 2018
  Simple demo  of the 90 degree matrix module
  Yet to do
  Make this into a usable C++ library
  Tested with 4off 4 chip display modules (16 MAX7210 chips)

  
  Name    Arduino   ESP8266       MAX7219 7 Segment display
  -----   -------   -------       -------------------------
          +5V       5V            VCC
          GND       GND           GND
  DIN     D11/MOSI  GPIO13 (D7)   DIN
  CS      *D10/SS    *GPIO15 (D8)   CS
  CLK     D13/SCK   GPIO14 (D5)   CLK

*/

#include <SPI.h>
#include "MAX7219_Functions.h"

void setup() {

  Serial.begin(115200);
  Serial.println("Up and Clackin");

  // Start SPI, SPI slow on ESP8266 https://github.com/esp8266/Arduino/issues/2624
  SPI.begin();

  // Set load pin to output
  pinMode(MAX7219_CS, OUTPUT);

  // Reverse the SPI transfer to send the MSB first
  SPI.setBitOrder(MSBFIRST);

  delay(1000);

  MAX7219_Init();                               // Set All none data MAX registers
  Clear_All();                                  // Clear frame buffer and screen

  Scroll_Text_Start("Hello MAX7219!", 10); // Write new text to display and restart scroll

} // end of setup


void loop() {


  if (Scrolling == true) {
    Scroll_Update();                        // Update the scrolling display
  }
  else {

    // place text
    delay(1000);
    Display_Text("Display?", 2);            // (text to display, location(pixels))
    delay(1000);

    // display wipe
    Wipe_Text_Right("In from Left", 8);     // (text to display, speed)
    delay(1000);
    Wipe_Text_Left("Right", 4);     // (text to display, speed)
    delay(1000);

    // display scroll
    Scroll_Text_Start("Scrolling text right to left.....", 10); // Write new text to display and restart scroll

  } // end of else

} // end of loop



