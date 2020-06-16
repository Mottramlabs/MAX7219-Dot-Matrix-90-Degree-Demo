/*
    variables and functions used by this project
*/

// Watchdog items
volatile int Watchdog_Timout = 1200;              // time in seconds before watchdog times out, 1200 = 1200 seconds or 20 minutes

// Custom default values
String WiFi_SSID = "None";                        // SSID string
String My_MAC = "";                               // MAC address, tobe read from ESP8266
char MAC_array[13] = "000000000000";              // MAC definition
String My_IP = "";                                // IP address

// ********************************************************************************************************
// ************************ project specific variables ****************************************************
// ********************************************************************************************************

String Clock = "0000";
int8_t Flash_Blink = 0;
int Flash_Timer = 0;
#define Flash_rate 500                            // flash the colon every 0.5 seconds
#define Default_Brightness 8

int NTP_Update_Timer = 0;
#define NTP_Update_rate 30000                     // NTP update in mS (3000 = 30 seconds)

// ********************************************************************************************************
// ************************ watchdog items ****************************************************************
// ********************************************************************************************************

#include <Ticker.h>
Ticker secondTick;
volatile int watchdogCount = 0;

// watchdog timer function, watchdog is clear when the ststus report is requested by a GET request
void ISRwatchdog() {
  watchdogCount++;
  if (watchdogCount == Watchdog_Timout) {
    Serial.println();
    Serial.println("The watchdog bites !!!!!");
    ESP.reset();
  } // end of timeout test
} // end of watchdog count
