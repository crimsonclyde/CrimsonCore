/* *****************************************************************************
 * _________        .__
 * \_   ___ \_______|__| _____   __________   ____
 * /    \  \/\_  __ \  |/     \ /  ___/  _ \ /    \
 * \     \____|  | \/  |  | |  \\___ (  <_> )   |  \
 *  \______  /|__|  |__|__|_|  /____  >____/|___|  /
 *        \/                \/     \/  Core      \/
 *
 *
 * Author  : CrimsonClyde
 * Email   : clyde@darkpack.net
 *
 ***************************************************************************** */



// ------------------------------------------------------------------------------------------------------------------------------
//                                                Libraries
// ------------------------------------------------------------------------------------------------------------------------------
#include "application.h"
#include "pt-sem.h"
#include "lc-switch.h"
#include "lc.h"
#include "pt.h"
#include "OneWire.h"
#include "DS18B20.h"
#define _Digole_Serial_SPI_
#include "DigoleSerialDisp.h"

// ------------------------------------------------------------------------------------------------------------------------------
//                                                Variables
// ------------------------------------------------------------------------------------------------------------------------------

SYSTEM_MODE(SEMI_AUTOMATIC);                                // Execute Code _before cloud connect_

// Debug
int DEBUG=0;                                                // 1 = Serial debuggin possible;  0 = Debugging is off

// Intro
int INTRO=1;                                                // 1 = Intro executed at startup; 0 = no intro will be executed
int introDuration = 5000;                                   // Time intro runs after animation
int introDelay    = 20;                                     // Delay time globally used in the intro

// Clock
int timeZone = +1;                                          // Sommertime (DST) UTC+2, Wintertime (Normaltime) +1

// Digole TrueColor 1.8 OLED
DigoleSerialDisp digole(A2);

// Temperature Sensor
DS18B20 ds18b20 = DS18B20(D2);                              // DS18B20 to D2 with pullup 4,7 to 3,3V
char szInfo[64];                                            // DS18B20 char size
char tempCelsius[40];                                       // Init temp
float celsius;                                              // Init temp as float

// Photocell
int photocellReading;                                       // The analog reading from the analog resistor divider
int photocellResult;                                        // Devide the reading with 4 to get 1023
int photocellPin = A1;                                      // The cell and 4,7K pulldown are connected to A1
int dawnSignal   = D3;                                      // Signal send to D3 to activate relay
int dawnStart    = 17;                                      // Hour we want start measuring the brightness
int dawnEnd      = 20;                                      // Hour we want end measuring the brightness
int dawnLimit    = 930;                                     // When resistance falls under that value light will turned on
int dawnCounter  = 0;                                       // 0=off, 1=wait one hour, 2= sleep 3 hour
long dawnSleep   = 0;                                       // Emtpy variable for not active hours
long dawnTimer   = 0;                                       // Set global variable for further use

// Sleep (Deep-Sleep-Mode)
int sleepStart  = 1;                                        // Start time of the sleep mode (24h format)
int sleepTime   = 5 * 60 * 60;                              // Time (in seconds) how long the core should sleep

// Core Main LED
int mledDay         = 128;                                  // Brightness for Core Main LED, day (0-255)
int mledNight       = 64;                                   // Brightness for Core Main LED, night (0-255)
char trueMainLed    = '0,0,128';                            // RGB code for the Core´s Main LED
char falseMainLed   = '255,0,0';

// Thread Management (day example24 * 60 * 60 * 1000;)
static long thHeader     = 2 * 60 * 1000;                   // Update header
static long thDate       = 60 * 1000;                       // Update date + sync time
static long thTime       = 30 * 1000;                       // Update time
static long thPhotocell  = 60 * 1000;                       // OLED update photocell value
static long thTemp       = 40 * 1000;                       // OLED update temperature value
static long thSCL        = 6 * 60 * 1000;                   // StoneCircleLight (SCL)
static long thSleep      = 10 * 60 * 1000;                  // Sleep function
static long thWeb        = 5 * 1000;                        // Web publish
static long thMLED       = 2 * 60 * 1000;                   // Cores Main LED control
static int th = 0;                                          // Needed for thread management
static struct pt pt1, pt2, pt3, pt4, pt5, pt6, pt7, pt9, pt10;


// Bitmaps for OLED Display
const uint8_t ccIntro[]    = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf3,0xc7,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,0x3f,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xff,0xff,0xff,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xff,0xff,0xff,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xf9,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2f,0xff,0xf7,0xff,0xff,0xff,0xff,0x81,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xf7,0xff,0xff,0xff,0xff,0xef,0xc0,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x13,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0x01,0xff,0xff,0xdf,0xff,0xff,0xfc,0x00,0x00,0xc0,0x09,0x0e,0x38,0x00,0x03,0xff,0x0d,0xf7,0xff,0xc3,0xff,0xff,0xff,0x00,0x02,0x60,0x3f,0x1c,0xf0,0x00,0x01,0xcd,0x9f,0xf7,0xff,0xff,0xff,0xff,0x7f,0x80,0x00,0x01,0xfe,0x50,0x80,0x00,0x01,0x88,0x38,0x7f,0x3f,0xff,0xbf,0xff,0xc0,0x00,0x00,0x07,0xff,0xf7,0x80,0x00,0x01,0xf0,0x00,0xcf,0xf3,0xc9,0xfb,0xdf,0xe0,0x00,0x00,0x07,0xff,0xfc,0xc0,0x00,0x03,0xc0,0x1f,0xff,0xe0,0xff,0xe0,0xff,0xf0,0x00,0x00,0x07,0xff,0x70,0x00,0x00,0x03,0x80,0x7f,0xff,0x27,0xbf,0xf1,0xff,0xf0,0x00,0x00,0x1f,0xff,0xf8,0x1f,0x00,0x0f,0x80,0xcf,0xff,0xff,0x8e,0x63,0xff,0xc0,0x00,0x00,0x3f,0xff,0xc0,0x7f,0xdc,0x0f,0x80,0x9f,0x7c,0x83,0xf4,0x9f,0xfc,0x00,0x00,0x00,0x01,0xff,0xf1,0xff,0x7f,0x8f,0x00,0x9c,0xf0,0x00,0x7e,0x7f,0xf8,0x00,0x00,0x00,0x00,0x3e,0xf0,0xfe,0x7f,0xff,0x81,0xf3,0xf0,0x00,0x3f,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x2c,0x3f,0xff,0xef,0x83,0xef,0xc0,0x00,0x41,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0xf9,0xc3,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xfe,0x05,0xc7,0xd8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xfc,0x87,0xcf,0x18,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xf8,0x3f,0xff,0x7f,0x8c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xfc,0x7f,0xff,0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x7f,0xff,0xff,0xe3,0x61,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xff,0xff,0x00,0xe1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xfe,0x00,0x30,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xcf,0xff,0xf4,0x00,0x3e,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xf9,0xfc,0x00,0x0f,0x73,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xf3,0xfe,0x00,0x07,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xff,0xe6,0x00,0x03,0xb3,0xf9,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xf2,0x00,0x03,0x39,0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xf2,0x00,0x03,0x0f,0x7f,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xff,0x02,0x00,0x07,0x87,0xff,0xf0,0x00,0xf0,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xfe,0x02,0x00,0x03,0x87,0xff,0xf0,0x00,0xfe,0x00,0x00,0x00,0x00,0x00,0x07,0xff,0xfe,0x06,0x00,0x01,0xcc,0x7f,0xf0,0x19,0xfe,0x00,0x00,0x00,0x00,0x04,0x0f,0xff,0xff,0xfc,0x00,0x00,0x7f,0x7f,0xf4,0x1d,0xfe,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xfe,0x00,0x00,0x00,0x0f,0xfc,0x0f,0xff,0xf0,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x14,0x00,0xc0,0x81,0x00,0x00,0x00,0x07,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x10,0x00,0x7f,0x86,0x01,0x00,0x03,0xff,0xff,0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x06,0xe3,0x07,0x00,0x0f,0xff,0xff,0xff,0xff,0xff,0xf8,0x00,0x00,0x08,0x00,0x00,0x00,0x80,0x3f,0x7c,0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x7c,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xfe,0x00,0x00,0x10,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x00,0x80,0x13,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x81,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff};
const uint8_t ccC[]        = {0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xe0,0x00,0x07,0xff,0xc0,0x00,0x0f,0xff,0x80,0x00,0x1f,0xe0,0x00,0x00,0x3f,0xc0,0x00,0x00,0x7f,0x80,0x00,0x00,0xff,0x00,0x00,0x01,0xfe,0x00,0x00,0x03,0xfc,0x00,0x00,0x07,0xf8,0x00,0x00,0x0f,0xf0,0x00,0x00,0x1f,0xe0,0x00,0x00,0x1f,0xc0,0x00,0x00,0x1f,0x80,0x00,0x00,0x1f,0x80,0x00,0x00,0x1f,0x80,0x00,0x00,0x1f,0x80,0x00,0x00,0x1f,0xc0,0x00,0x00,0x1f,0xe0,0x00,0x00,0x0f,0xf0,0x00,0x00,0x07,0xf8,0x00,0x00,0x03,0xfc,0x00,0x00,0x01,0xfe,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x7f,0x80,0x00,0x00,0x3f,0xc0,0x00,0x00,0x1f,0xe0,0x00,0x00,0x0f,0xff,0x80,0x00,0x07,0xff,0xc0,0x00,0x03,0xff,0xe0,0x00,0x00,0x00,0x00};
const uint8_t ccClock[]    = {0x83,0x81,0x68,0x16,0x41,0x02,0x01,0x00,0x41,0x02,0x01,0x00,0x81,0x01,0x81,0xfd,0x80,0x01,0x80,0x01,0x00,0x00,0x40,0x02,0x00,0x00,0x40,0x02,0x68,0x16,0x83,0x81};
const uint8_t ccTemp[]     = {0x88,0x82,0x41,0x21,0x2a,0x40,0x80,0x80,0x59,0x00,0x24,0x00,0x42,0x00,0x42,0xec,0x27,0xfe,0x1f,0xfc,0x3f,0xfe,0x7f,0xff,0x7f,0xfe,0x3f,0xfc,0x8f,0x99,0x40,0x02};
const uint8_t ccLightOff[] = {0xc3,0xc3,0x8c,0x31,0x18,0x18,0x30,0x0c,0x60,0x06,0x40,0x02,0x83,0xc1,0x82,0xc1,0x83,0x41,0x83,0xc1,0x40,0x02,0x60,0x06,0x30,0x0c,0x18,0x18,0x8c,0x31,0xc3,0xc3};
const uint8_t ccLightOn[]  = {0xc3,0xc3,0x8c,0x31,0x18,0x18,0x38,0x1c,0x7c,0x3e,0x4e,0x72,0x87,0xe1,0x83,0x41,0x82,0xc1,0x87,0xe1,0x4e,0x72,0x7c,0x3e,0x38,0x1c,0x18,0x18,0x8c,0x31,0xc3,0xc3};

// Get external variable from digoleSerialDisplay.cpp
extern bool digoleDisplayStatus;

// ------------------------------------------------------------------------------------------------------------------------------
//                                                Functions
// ------------------------------------------------------------------------------------------------------------------------------

// Intro
void funcIntro(void) {
    /* Intro */
    digole.clearScreen();

    for (int intro1 = 160; intro1 > 0; intro1=intro1-1) {
        digole.drawBitmap(intro1,0,32,32, ccC);
        delay(introDelay);
    }
    digole.setTextPosAbs(33, 32); digole.print("rimson");

    for (int intro2 = 160; intro2 > 83; intro2=intro2-1) {
        digole.drawBitmap(intro2,0,32,32, ccC);
        delay(introDelay);
    }
    digole.setTextPosAbs(108, 32); digole.print("trl");
    digole.drawHLine(0,35,160);
    digole.drawBitmap(20, 50, 128, 64, ccIntro);
    delay(introDuration);
    digole.clearScreen();
}


// Date + Timesync+SetTimeZone
void funcDate(void) {
    if (DEBUG == 1) { Serial.println("DEBUG: funcDate -> LCD update date  (including sync and set timezone)"); }

    // LCD display current time (24h + including leading 0 correction)
    Time.zone( timeZone );      // Set timezone
    Spark.syncTime();           // Request time sync from Spark Cloud

    digole.setTextPosAbs(80, 77); digole.print("          ");

    // Day
    if (Time.day() <10) {
        digole.setTextPosAbs(80, 77); digole.print("0"); digole.print( Time.day() ); digole.print(".");
    } else {
        digole.setTextPosAbs(80, 77); digole.print( Time.day() ); digole.print(".");
    }

    // Month
    if (Time.month() <10) {
        digole.setTextPosAbs(102, 77); digole.print("0"); digole.print( Time.month() ); digole.print(".");
    } else {
        digole.setTextPosAbs(102, 77); digole.print( Time.month() ); digole.print(".");
    }

    // Year
    digole.setTextPosAbs(125, 77); digole.print( Time.year() );

}

// MLED (Control the Cores Main LED)
void funcMLED(void) {
    if (DEBUG == 1) { Serial.println("DEBUG: funcMLED -> Main LED manipulation  (Dimming and color replacement)"); }

    RGB.color(trueMainLed);
    if (Time.hour() <=4 || Time.hour() >=20) {
            RGB.brightness(mledNight);                    // Brightness control
    } else {
            RGB.brightness(mledDay);                      // Brightness control
    }
}

// Time
void funcTime(void) {
    if (DEBUG == 1) { Serial.println("DEBUG: funcTime "); }

        // Clock Symbol
        digole.drawBitmap(0, 65, 16, 16, ccClock);

        // Hour
        digole.setTextPosAbs(20, 77);
        if (Time.hour() <10 ) {
            digole.print("0"); digole.print( Time.hour() ); digole.print(":");
        } else {
            digole.print( Time.hour() ); digole.print(":");
        }

        // Minute
        digole.setTextPosAbs(43, 77);
        if (Time.minute() <10 ) {
            digole.print("0"); digole.print( Time.minute() );
        } else {
            digole.print( Time.minute() );
        }
}

// LCD Photocell value update
void funcPhotocell(void) {
  if ( DEBUG == 1 ) { Serial.println("DEBUG: funcPhotocell -> execute LCD update photocell value"); }
  // Update photocell info
  photocellReading = analogRead(photocellPin);
  photocellResult = photocellReading / 4;
  if ( DEBUG == 1 ) { Serial.print("DEBUG: funcPhotocell -> Photocell state: "); Serial.println(photocellReading); }


  // Arrange output
  // 1st step: clear
  digole.setTextPosAbs(20, 125);
  digole.print("                   ");
  digole.setTextPosAbs(20, 125);

  // 2nd step: print
  if (photocellResult == 1023)      { digole.print(" Daytime");         if ( DEBUG == 1 ) { digole.print(" ("); digole.print(photocellResult); digole.print(")"); } }
  else if (photocellResult < 1023)  { digole.print(" Dusk");  if ( DEBUG == 1 ) { digole.print(" ("); digole.print(photocellResult); digole.print(")"); } }
  else if (photocellResult <= 900)  { digole.print(" Nighttime");       if ( DEBUG == 1 ) { digole.print(" ("); digole.print(photocellResult); digole.print(")"); } }
  else { digole.print(photocellResult); }
}

// Temperature Sensor (DS18B20)
void funcTemp(void) {
    if (DEBUG == 1) { Serial.println("DEBUG: funcTemp -> executing the temperature function"); }
    digole.drawBitmap(0, 89, 16, 16, ccTemp);                       // Print Bitmap
    if(!ds18b20.search()){
         if (DEBUG == 1) { Serial.println("DEBUG: Temperature Sensor -> Could not reach the sensor"); }
        ds18b20.resetsearch();
        //delay(250);           // Hopefully we did not need delay in here

        return;
    }
    celsius = ds18b20.getTemperature();

    sprintf(tempCelsius, "%2.1f", celsius);                         // Round result to 1 digit after .

    if (DEBUG == 1) {
        Serial.print("DEBUG: Temperature Sensor -> ");
        Serial.println(tempCelsius);
    }

    digole.setTextPosAbs(25, 101); digole.print("       ");          // Clean Temperature
    digole.setTextPosAbs(25, 101); digole.print(tempCelsius);        // Print temperature
}

// StoneCircleLight (SCL)
void funcSCL(void) {
 if ( DEBUG == 1 ) { Serial.println("DEBUG: funcSCL -> check time and brightness to send the relay signal"); }

 if (dawnCounter == 0 && Time.hour() >= dawnStart && Time.hour() <= dawnEnd && photocellResult < dawnLimit) {
   // Entering the twilight zone
   if ( DEBUG == 1 ) { Serial.println("Function (dawnCounter should be 0) -> Twilight: All criterias have been met, entering twilight"); }
   digitalWrite(dawnSignal, LOW);                       // Set Pin to low to activate the relay
   digole.drawBitmap(0, 112, 16, 16, ccLightOn);        // Change LCD light symbol to light "on"
   dawnCounter++;                                       // Increase dawn counter for next round
   dawnTimer = Time.now() + 3600L;                       // Add 1hour to current linux time (in sec.) 3600L
 }

 if ( dawnCounter == 1 ) {
   if( Time.now() >= dawnTimer ) {                    // Wait 1h
     if ( DEBUG == 1 ) { Serial.println("Function (dawnCounter should be 1) -> Twilight: 1hour wait"); }
     digole.drawBitmap(0, 112, 16, 16, ccLightOff);     // Change LCD light symbol to light "off"
     digitalWrite(dawnSignal, HIGH);                    // Disable relay
     dawnCounter++;                                     // Increase dawn counter for next round
     dawnSleep = Time.now() + 10800L;                   // Add 3hours to current linux time (in sec.) 10800L
     if ( DEBUG == 1 ) { Serial.print("dawnCounter= "); Serial.println(dawnCounter); }
    }
  }

  if (dawnCounter == 2 ) {
    if ( DEBUG == 1 ) { Serial.print(dawnSleep); long d_bug=Time.now(); Serial.println(d_bug); }
     if ( Time.now() >= dawnSleep ) {                   // Wait 3h
       if ( DEBUG == 1 ) { Serial.println("Function (dawnCounter should be 2) -> Twilight: Prevent doubling"); }
       dawnCounter = 0;                                 // Set dawn counter back to zero
     }
  }
}

// Web publish function
// Send some data to the cloud so we can get it via the website
void funcWeb(void) {
    if ( DEBUG == 1 ) { Serial.println("DEBUG: funcWeb -> publish information that we can see results on webpage"); }
    // Initiate variables
    char DateString[45];
    char TimeString[55];
    char PhotoString[20];
    char MiscString[55];

    // Using sprintf to store as string ressource
    sprintf(DateString, "{\"Day\": %u, \"Month\": %u, \"Year\": %u}",Time.day(),Time.month(),Time.year());
    sprintf(TimeString, "{\"Hours\": %u, \"Minutes\": %u, \"Seconds\": %u}",Time.hour(),Time.minute(),Time.second());
    sprintf(PhotoString,"{\"Photocell\": %u}",photocellResult);
    sprintf(MiscString, "{\"Temperature\": %2.1f, \"SCL\": %u, \"RSSI\": %d}",celsius,digitalRead(dawnSignal),WiFi.RSSI());

    // Send to cloud API
    Spark.publish("pDate",DateString);
    Spark.publish("pTime",TimeString);
    Spark.publish("pPhoto",PhotoString);
    Spark.publish("pMisc",MiscString);

}

// Sleep Function
//  Bring spark to deep sleep. First we do not want to waste power, second my WiFi is set off at night.
void funcSleep(void) {
    if ( DEBUG == 1 ) { Serial.println("DEBUG: funcSleep -> check if time is given and then send Core to sleep"); }

    if (Time.hour() == sleepStart ) {
        digole.setScreenOff();
        Spark.sleep(SLEEP_MODE_DEEP,sleepTime);
    }
}

// Webpage Button Function
// This control post command from the webpage.
// Test to activate: curl https://api.spark.io/v1/devices/<<CoreID>>/SCL -d access_token=<<accessToken>> -d params=d3,LOW
int funcSCLswitch(String command)
{
   if ( DEBUG == 1 ) { Serial.println("DEBUG: ledSCLswitch -> executing stone circle light"); }

   int state = 0;

     //find out the pin number and convert the ascii to integer
     int pinNumber = (command.charAt(1) - '0') - 1;
     //Sanity check to see if the pin numbers are within limits
     if (pinNumber < 2 || pinNumber > 3) return -1;
     // find out the state of the led

     if(command.substring(3,7) == "HIGH") state = 1;
     else if(command.substring(3,6) == "LOW") state = 0;
     else return -1;

     // write to the appropriate pin
     digitalWrite(dawnSignal, state);

     if (digitalRead(dawnSignal == LOW)) {
         digole.drawBitmap(0, 112, 16, 16, ccLightOn);        // Change LCD light symbol to light "on"
     } else {
         digole.drawBitmap(0, 112, 16, 16, ccLightOff);       // Change LCD light symbol to light "off"
     }
     return 1;
}


void funcUpdateLCD(void) {
   if (DEBUG == 1) { Serial.println("DEBUG: Function Update LCD -> setting up the main screen"); }

    // Activate RGB LED control
    RGB.control(true);

    // Setup the header
    digole.drawBitmap(0,0,32,32, ccC);
    digole.setTextPosAbs(33, 32); digole.print("rimson");
    digole.drawBitmap(83,0,32,32, ccC);
    digole.setTextPosAbs(108, 32); digole.print("trl");
    digole.drawHLine(0,35,160);

    // Setup clock icon
    digole.drawBitmap(0, 65, 16, 16, ccClock);

    // Setup temperature icon
    digole.drawBitmap(0, 89, 16, 16, ccTemp);


    // Setup SCL icon (inclusive check off/on)
    if ( digitalRead(dawnSignal) == LOW ) {
        digole.drawBitmap(0, 112, 16, 16, ccLightOn);
    } else {
        digole.drawBitmap(0, 112, 16, 16, ccLightOff);
    }
    digole.disableCursor();

}

// ------------------------------------------------------------------------------------------------------------------------------
//                                                Threads
// ------------------------------------------------------------------------------------------------------------------------------
// Update the header
static int thread1( struct pt *pt, int th, long timeout ) {
  static long t1 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t1) > timeout );
    if (DEBUG == 1) { Serial.println("DEBUG thread: thHeader -> executing thread"); }
    //funcHeader();     // Simple call the function
    funcUpdateLCD();
    t1 = millis();
  }
  PT_END( pt );
}

// Set timezone, sync time update OLED date value
static int thread2( struct pt *pt, int th, long timeout ) {
  static long t2 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t2) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thDate -> executing thread"); }
    funcDate();                         // Simple call the function
    t2 = millis();
  }
  PT_END( pt );
}

// Update OLED time value
static int thread3( struct pt *pt, int th, long timeout ) {
  static long t3 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t3) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thTime -> executing thread"); }
    funcTime();   // Simple call the function
    t3 = millis();
  }
  PT_END( pt );
}

// Update OLED photocell value
static int thread4( struct pt *pt, int th, long timeout ) {
  static long t4 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t4) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thSCL -> executing thread"); }
    funcPhotocell();     // Simple call the function
    t4 = millis();
  }
  PT_END( pt );
}

// Update OLED temperature value
static int thread5( struct pt *pt, int th, long timeout ) {
  static long t5 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t5) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thPhotocell -> executing thread"); }
    funcTemp();    // Simple call the function
    t5 = millis();
  }
  PT_END( pt );
}

// StoneCircleLight
static int thread6( struct pt *pt, int th, long timeout ) {
  static long t6 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t6) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thPhotocell -> executing thread"); }
    funcSCL();    // Simple call the function
    t6 = millis();
  }
  PT_END( pt );
}

// Sleep
static int thread7( struct pt *pt, int th, long timeout ) {
  static long t7 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t7) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thSleep -> executing thread"); }
    funcSleep();    // Simple call the function
    t7 = millis();
  }
  PT_END( pt );
}

// Web publish
static int thread9( struct pt *pt, int th, long timeout ) {
  static long t9 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t9) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thWeb -> executing thread"); }
    funcWeb();                          // Simple call the function
    t9 = millis();
  }
  PT_END( pt );
}

// Web publish
static int thread10( struct pt *pt, int th, long timeout ) {
  static long t10 = 0;
  PT_BEGIN( pt );
  while(1) {
    PT_WAIT_UNTIL( pt, (millis() - t10) > timeout );
    if ( DEBUG == 1 ) { Serial.println("DEBUG thread: thMLED -> executing thread"); }
    funcMLED();     // Simple call the function
    t10 = millis();
  }
  PT_END( pt );
}


// ------------------------------------------------------------------------------------------------------------------------------
//                                                Setup
// ------------------------------------------------------------------------------------------------------------------------------

void setup() {
if (DEBUG == 1) {
    Serial.begin(57600);
    Serial.println("-----------------------------------------------------------------------------------------------");
    Serial.println("                                DEBUG MONITOR");
    Serial.println("-----------------------------------------------------------------------------------------------");
    }

// Initialise OLED
  digole.begin();
  digole.setScreenOn();
  digole.clearScreen();
  digole.disableCursor();


// Dawn setup part
  Spark.function("SCL", funcSCLswitch);      // Register function
  pinMode(dawnSignal, OUTPUT);               // Configure as output
  digitalWrite(dawnSignal, HIGH);            // Set PIN to high (relay off)

// Thread Setup
  PT_INIT( &pt1 );      // Header
  PT_INIT( &pt2 );      // Date (including sync + set tz)
  PT_INIT( &pt3 );      // Time
  PT_INIT( &pt4 );      // Photocell
  PT_INIT( &pt5 );      // Temperature
  PT_INIT( &pt6 );      // StoneCircleLight
  PT_INIT( &pt7 );      // Sleep
  PT_INIT( &pt9 );      // Publish to Webpage

// Run the Intro
    if ( INTRO == 1) {
        if (DEBUG == 1) { Serial.println("DEBUG: Setup -> initiate the intro"); }
        funcIntro();
    }

// First run and initalise basics
   funcUpdateLCD();
}

// ------------------------------------------------------------------------------------------------------------------------------
//                                                Main Loop
// ------------------------------------------------------------------------------------------------------------------------------
void loop() {
    // Semi Automated Mode
    // Problem workaround, if automatic mode is running and the internet connection is down or the cloud is not available
    // user code will not be executed anymore! PINmodes are resettet to default and so the relay control PIN is set to LOW
    // This results in an closed circuit and the stone-circle-light is on as long as there is no internet/cloud available.
    if (!Spark.connected()) {
        Spark.connect();                            // Try (again) establishing a connection to the cloud
        delay(10000);                               // Give it a moment to settle the connection
            while (!Spark.connected()) {
                RGB.color(255,0,0);                 // Set Core LED to false (red) to get visual feedback that we currently have no connectivity
                digole.setScreenOff();              // No waste of engery - set the screen off
                Spark.connect();                    // Maybe it´s possible to connect again, so we try
                delay(60000);                       // This code run only in a worst-case scenario. So we do not need to pool for new con every sec.
            }
    } else {
        if (!digoleDisplayStatus) { digole.setScreenOn(); };  // We are back online, turn the screen on
        thread1( &pt1, th, thHeader );
        thread2( &pt2, th, thDate );
        thread3( &pt3, th, thTime );
        thread4( &pt4, th, thPhotocell );
        thread5( &pt5, th, thTemp );
        thread6( &pt6, th, thSCL );
        thread7( &pt7, th, thSleep );
        thread9( &pt9, th, thWeb );
        thread10( &pt10,th, thMLED );
    }

}
