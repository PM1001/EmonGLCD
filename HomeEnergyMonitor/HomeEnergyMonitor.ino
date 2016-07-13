//------------------------------------------------------------------------------------------------------------------------------------------------
// emonGLCD Home Energy Monitor example
// to be used with nanode Home Energy Monitor example

// Uses power1 variable - change as required if your using different ports

// emonGLCD documentation http://openEnergyMonitor.org/emon/emonglcd

// RTC to reset Kwh counters at midnight is implemented is software. 
// Correct time is updated via NanodeRF which gets time from internet
// Temperature recorded on the emonglcd is also sent to the NanodeRF for online graphing

// GLCD library by Jean-Claude Wippler: JeeLabs.org
// 2010-05-28 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
//
// Authors: Glyn Hudson and Trystan Lea
// Part of the: openenergymonitor.org project
// Licenced under GNU GPL V3
// http://openenergymonitor.org/emon/license

// THIS SKETCH REQUIRES:

// Libraries in the standard arduino libraries folder:
//
//	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
//	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature
//                           or https://github.com/milesburton/Arduino-Temperature-Control-Library
//	- JeeLib		https://github.com/jcw/jeelib
//	- RTClib		https://github.com/jcw/rtclib
//	- GLCD_ST7565		https://github.com/jcw/glcdlib
//
// Other files in project directory (should appear in the arduino tabs above)
//	- icons.ino
//	- templates.ino
//
//-------------------------------------------------------------------------------------------------------------------------------------------------

#define RF69_COMPAT 1 // set to 1 to use RFM69CW 
#define RF_ENABLED
//#define LCD_ON_SPI 1

#ifdef RF_ENABLED
#include <JeeLib.h>   // make sure V12 (latest) is used if using RFM69CW
#endif
#include <U8glib.h>
#include <avr/pgmspace.h>


#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib
RTC_Millis RTC;

// LCD Pins
//#define U8G_HW_SPI_2X
U8GLIB_ST7920_128X64_1X glcd(5);    // SCK, SID, CS (hardware based SPI for LCD2)
//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 20            // Should be unique on network, node ID 30 reserved for base station
#define RF_freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210 

unsigned long fast_update, slow_update=0;

double temp,maxtemp,mintemp;

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------
typedef struct {                //  for data from emonPi for emonGLCD time & "Power Now"
  byte cmd;
  byte hour;
  byte minute;
  byte second;
} HeadPi;                    // create JeeLabs RF packet structure
HeadPi emonPiHead;
  
typedef struct {                //  for data from emonPi for emonGLCD time & "Power Now"
  byte cmd;
  byte hour;
  byte minute;
  byte second;
  int16_t kWhAccum;
  int16_t WattsNow;
  int16_t voltage;
  int16_t frequency;
  byte SendToNode;
} PayloadPi;                    // create JeeLabs RF packet structure
PayloadPi emonPi;

typedef struct {                //  for data from emonPi for emonGLCD time & "Power Now"
  byte cmd;
  int16_t outsideTemp;
  int16_t geyserTemp;
  int16_t geyserManifold;
  int16_t pumpReg;
  byte SendToNode;
} PayloadPi2;                    // create JeeLabs RF packet structure
PayloadPi2 emonPi2;

typedef struct { 
int temper1;
int temper2;
int temper3;                                               
int temper4;                                             
int temper5;                                           
int temper6;                                           
int temper7;                                         
int temper8; 
int geyserTemp;
int collecterTemp;
int digi1;
int digi2;
int digi3; 
int digi4; 
} PayloadGeyser;     
PayloadGeyser emonGeyser;
typedef struct { int temperature;int digi; } PayloadGLCD;
PayloadGLCD emonglcd;

int hour = 12, minute = 0;
double usekwh = 0;

const int greenLED=6;               // Green tri-color LED
const int redLED=7;                 // Red tri-color LED
const int LEDpin=9;                 // Red tri-color LED
const int LDRpin=4;    		    // analog pin of onboard lightsensor 
const int BacklightPin=3;
int cval_use;

//-------------------------------------------------------------------------------------------- 
// Flow control
//-------------------------------------------------------------------------------------------- 
unsigned long last_emonpi;                   // Used to count time from last emontx update
//--------------------------------------------------------------------------------------------
// Analog temperature
//--------------------------------------------------------------------------------------------
float  convertAnalogToTemperature(unsigned int analogReadValue)
{
  // If analogReadValue is 1023, we would otherwise cause a Divide-By-Zero,
  // Treat as crazy out-of-range temperature.
  if(analogReadValue == 1023) return 1000.0; 
                                               
  return (1/((log(((10000.0 * analogReadValue) / (1023.0 - analogReadValue))/22000.0)/4220.0) + (1 / (273.15 + 25.000)))) - 273.15;
}

unsigned getTempChannel(byte channel)
{
   return (unsigned)(convertAnalogToTemperature(analogRead(channel))*100.0);
}

void double_LED_flash(byte led)
{
  digitalWrite(led, HIGH);  delay(25); digitalWrite(led, LOW);
  digitalWrite(led, HIGH);  delay(25); digitalWrite(led, LOW);
}

void single_LED_flash(byte led)
{
  digitalWrite(led, HIGH);  delay(100); digitalWrite(led, LOW);
}

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup()
{
    // Set up high frequency PWM for LCD backlight
  TCCR2A = 0x23; //FAst PWM
  TCCR2B = 0x09;//62500Hz
  OCR2A = 255;
  OCR2B = 30;  // set the PWM 
  pinMode(BacklightPin, OUTPUT);     // sets as output
  //digitalWrite(BacklightPin,HIGH);
  delay(500); 				   //wait for power to settle before firing up the RF

  glcd.setColorIndex(1);         // pixel on
  //glcd.setRot180();
   
  temp = (getTempChannel(0))/100.0;     // get inital temperture reading
  mintemp = temp; maxtemp = temp;          // reset min and max
  
#ifdef RF_ENABLED
  rf12_initialize(MYNODE, RF_freq,group);
  delay(100);	//wait for RF to settle befor turning on display
  int temper = getTempChannel(0);
  rf12_sendNow(0, &temper, sizeof(int));                     //send temperature data via RFM12B using new rf12_sendNow wrapper -glynhudson
  rf12_sendWait(2);  
#endif			

  pinMode(greenLED, OUTPUT); 
  pinMode(redLED, OUTPUT); 
  pinMode(LEDpin, OUTPUT); 
  digitalWrite(greenLED, HIGH);
  //single_LED_flash(LEDpin);
  //single_LED_flash(greenLED);
}

//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------

void displayUpdate(){
    cli();
    glcd.firstPage();           // Update LCD 2
    //sei();
    //delay(50);
    //cli();
    //delay(50);
    do {
      draw_power_page(emonPi.WattsNow, usekwh,temp, mintemp, maxtemp, hour,minute,emonPi2.geyserTemp,emonPi2.outsideTemp,emonPi.voltage,emonPi.frequency,emonPi2.geyserManifold,emonPi2.pumpReg);
    } while( glcd.nextPage() );
    sei();
}
void loop()
{
#ifdef RF_ENABLED
  if (rf12_recvDone())
  {
    //double_LED_flash(redLED);
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {

      int node_id = (rf12_hdr & 0x1F);
      if (node_id == 5)			//Assuming 15 is the emonBase node ID
      {
         
        emonPiHead = *(HeadPi*) rf12_data;
        if(emonPiHead.cmd==0){
          RTC.adjust(DateTime(2012, 1, 1, emonPi.hour+2, emonPi.minute, emonPi.second));
          emonPi = *(PayloadPi*) rf12_data;
        }
        if(emonPiHead.cmd==1){
          emonPi2 = *(PayloadPi2*) rf12_data;
        }
          
        last_emonpi = millis();
      } 
      if(node_id ==22)
      {
        emonGeyser = *(PayloadGeyser*) rf12_data;
      }
    }
    displayUpdate();
  }
  #endif
  if ((millis()-slow_update)>30000)
  {
    slow_update = millis();
    fast_update = millis();
   
  rf12_initialize(MYNODE, RF_freq,group);
  delay(150);	//wait for RF to settle befor turning on display
  
#ifdef RF_ENABLED
    emonglcd.digi = 50;
    emonglcd.temperature = getTempChannel(0);
    rf12_sendNow(0, &emonglcd, sizeof(emonglcd));                     //send temperature data via RFM12B using new rf12_sendNow wrapper -glynhudson
    rf12_sendWait(1);  
    single_LED_flash(LEDpin);
    
#endif
  }
  
  //--------------------------------------------------------------------------------------------
  // Display update every 1000ms
  //--------------------------------------------------------------------------------------------
  if ((millis()-fast_update)>500)
  {
    fast_update = millis();
    
    DateTime now = RTC.now();
    int last_hour = hour;
    hour = now.hour();
    minute = now.minute();

    usekwh = emonPi.kWhAccum;
    if (last_hour == 23 && hour == 00) usekwh = 0;                //reset Kwh/d counter at midnight
    cval_use = emonPi.WattsNow/10.0;//cval_use + (emonPi.WattsNow/10 - cval_use)*0.50;        //smooth transitions
    
    temp = getTempChannel(0)/100.0;
    if (temp > maxtemp) maxtemp = temp;
    if (temp < mintemp) mintemp = temp;
    //rf12_sleep(-1);

    displayUpdate();
    int LDR = analogRead(LDRpin);                     // Read the LDR Value so we can work out the light level in the room.
    int LDRbacklight = map(LDR, 0, 1023, 50, 250);    // Map the data from the LDR from 0-1023 (Max seen 1000) to var GLCDbrightness min/max
    LDRbacklight = constrain(LDRbacklight, 0, 255);   // Constrain the value to make sure its a PWM value 0-255
    if ((hour > 22) ||  (hour < 5)) OCR2B=50; else OCR2B=LDRbacklight;  
  } 
  

}



