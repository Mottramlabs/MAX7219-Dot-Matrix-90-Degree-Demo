/*
    Title:        Simple Matrix Clock
    Date:         16th June 2020
    Version:      1
    Description:  Sample code
    Author:       David Mottram
    Device:       ESP8266

    NTP information based on the great work by G6EJD - David
    See his youtube channel https://www.youtube.com/user/G6EJD/videos
    NTP functions https://www.youtube.com/watch?v=g62Atuf1cm4 and https://github.com/G6EJD/ESP_Simple_Clock_Functions/blob/master/ESP_Basic_NTP_Time_Date.ino
*/

/* ********************************** Compiler settings, un-comment to use ************************** */
//#define Fixed_IP                      // un-comment to use a fixed IP address to speed up development
//#ifdef Watchdog_ON                    // watchdog items, comment out if not used
//#define Print_Report_Level_1          // un-comment for option
#define Print_Report_Level_2          // un-comment for option, report received MQTT message
#define Print_Report_Level_3          // un-comment for option
/* ************************************************************************************************** */

// https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#include <ESP8266WiFi.h>              // needed for EPS8266

// custom settings files
#include "Wifi_Settings.h"            // custom Wifi settings
#include "Project_Settings.h"         // board specific details.

// MAX7219
#include <SPI.h>
#include "MAX7219_Functions.h"

// incude WiFi and MQTT functions
WiFiClient espClient;                 // for ESP8266 boards
#include "WiFi_Functions.h"           // read wifi data

//https://www.arduino.cc/reference/en/language/functions/time/millis/
#include <time.h>


void setup() {

#ifdef Watchdog_ON
  // watchdog items, comment out if not used
  secondTick.attach(1, ISRwatchdog);
#endif

  // Start SPI, SPI slow on ESP8266 https://github.com/esp8266/Arduino/issues/2624
  SPI.begin();
  // Set load pin to output
  pinMode(MAX7219_CS, OUTPUT);
  // Reverse the SPI transfer to send the MSB first
  SPI.setBitOrder(MSBFIRST);
  delay(1000);
  Intensity = Default_Brightness;
  MAX7219_Init();                                   // Set All none data MAX registers
  Clear_All();                                      // Clear frame buffer and screen
  Wipe_Text_Left(" Hi", 4);                         // (text to display, speed)

  // connect to WiFi access point
  Get_Wifi();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region

  // Austria/Sydney   AEST-10AEDT,M10.1.0,M4.1.0/3
  // Los_Angeles      PST8PDT,M3.2.0,M11.1.0
  // Europe/London    GMT0BST,M3.5.0/1,M10.5.0
  setenv("TZ", "GMT0BST,M3.5.0/01,M10.5.0/02", 1);

} // end of setup



String get_time() {
  time_t now;
  time(&now);
  char time_output[30];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  //strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));

  // uncomment as required
  // 24 hour display
  strftime(time_output, 30, "%H", localtime(&now));

  // 12 hour display
  //    strftime(time_output, 30, "%I", localtime(&now));

  // get the hours and strip the leading zero if present, convert to int and back to string works
  String Hours = time_output;
  int zz = Hours.toInt();
  String Result = String(zz);

  // now add the minutes
  strftime(time_output, 30, "%M", localtime(&now));
  Result = Result + time_output;

  return String(Result);
}


void loop() {

  // **************************************************************************************************************************
  // clock update
  if (millis() - Flash_Timer > Flash_rate) {

    Flash_Timer = millis();
    Flash_Blink++;

    // blink the colon, default is every 0.5 second
    if ((bitRead(Flash_Blink, 0)) == 0) {
      Display_Clock(Clock, 1);
    }
    else {
      Display_Clock(Clock, 0);
    }

  } // end of clock


  // **************************************************************************************************************************
  // read NTP
  if (millis() - NTP_Update_Timer > NTP_Update_rate) {

    NTP_Update_Timer = millis();
    //get_time();
    Clock = get_time();
    Serial.println(Clock);

  } // end NTP update

} // end of loop
