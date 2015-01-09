/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Satish Nair, Zachary Crockett and Mohit Bhoite
 * @version V1.0.0
 * @date    05-November-2013
 * @brief   Tinker application
 ******************************************************************************
  Copyright (c) 2013 Spark Labs, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/*
 * Brief of Drinkbot
 *
 * State Machine:
 *
 *                   BOOTING
 *                      |
 *                      v
 *   (collect data) LISTENING <-+
 *        API called? Y |       |
 *                      v       |
 *                    START     |
 *                      |       | finished? Y
 *                      v       |
 *              +--> PUMPING    |
 *  finished? N |       |       |
 *              +-------+-------+
 *                      |
 *                      v
 *                     end (never reach here)
 */

/* Includes ------------------------------------------------------------------*/
#include "application.h"
//#include "OneWire.h"
#include "OneWire/OneWire.h"
//#include "spark-dallas-temperature.h"
#include "spark-dallas-temperature/spark-dallas-temperature.h"
#include "neopixel/neopixel.h" //Neopixel RGBLED on pin D7
#include <math.h>

/* Definitions ---------------------------------------------------------------*/
SYSTEM_MODE(AUTOMATIC);

enum {
	STATE_BOOTING = 0,
	STATE_LISTENING = 1,
	STATE_START = 2,
	STATE_PUMPING = 3,
	STATE_MAX,
};

#define NPUMPS		8
#define vol2time(vol)	((unsigned long)vol*1000)	// vol -> millis, to be fixed according to actual case

#define IO_TEMP		D4
#define IO_ALCOH1	A3
#define IO_ALCOH2	A2
#define ALCOHOL_THRES	680	// reading value larger than this indicates alcohol sensor not ready
//Neopixel RGBLED
#define PIXEL_PIN D7
#define PIXEL_COUNT 24
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* Variables -----------------------------------------------------------------*/
static double temperature = 0.0;
static double bac1 = 0.0;
static double bac2 = 0.0;
static int state = STATE_BOOTING;
static int pumpsOnVol[NPUMPS] = {0, 0, 0, 0, 0, 0, 0, 0};	// unit: cl
static int pumpsStatus[NPUMPS] = {0, 0, 0, 0, 0, 0, 0, 0};	// 1: on, 0: off
static unsigned long timeStart = 0;

static const int pump2io[NPUMPS] = {D0, D1, A0, A1, A4, A5, A6, A7};

OneWire oneWire(IO_TEMP);
DallasTemperature tsensor(&oneWire);

/* Function prototypes -------------------------------------------------------*/
int drinkbotSetPumps(String command);
int drinkbotDebug(String command);

static void changestate(int s);
static int parseparams(String command, int id);
static double calculateBAC(int mq3_pin);
static int collectdata(void);
static void controlpump(int id, boolean on);
static void startpumpjobs(int p[NPUMPS]);
static int npumpson(int ps[NPUMPS]);
static void finishpumpjobs(int p[NPUMPS]);

/* This function is called once at start up ----------------------------------*/
void setup()
{
	int i;

	//Setup the logging functions
	Serial.begin(9600);
	Serial.println("setup...");

	//Setup communication bus with secondary processor
//	Serial1.begin(9600);

	//Neopixel RGBLED
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
	pixels.begin();
	pixels.show();

	//Register all the Drinkbot functions
	Spark.function("setpumps", drinkbotSetPumps);
	Spark.function("debug", drinkbotDebug);
	Spark.function("effects", drinkbotEffects);
	
	//Register all the Drinkbot variables
	Spark.variable("temperature", &temperature, DOUBLE);
	Serial.println("setup Dallas Temperature IC Control...");
	tsensor.begin();
	tsensor.setResolution(12);

	Spark.variable("bac1", &bac1, DOUBLE);
	pinMode(A3, INPUT);
	Spark.variable("bac2", &bac2, DOUBLE);
	pinMode(A2, INPUT);

	//Pumps
	for (i = 0; i < NPUMPS; i++) {
		pinMode(pump2io[i], OUTPUT);
		digitalWrite(pump2io[i], LOW);
	}

	//RGB LED
	pinMode(D7, OUTPUT);
}

/* This function loops forever --------------------------------------------*/
void loop()
{
	switch (state) {
	case STATE_BOOTING:
		Serial.println("Booting...");

		temperature = 0.0;
		bac1 = 0.0;
		bac2 = 0.0;
		state = STATE_BOOTING;
		memset(pumpsOnVol, 0, sizeof(pumpsOnVol));
		memset(pumpsStatus, 0, sizeof(pumpsStatus));
		timeStart = 0;

		changestate(STATE_LISTENING);
		break;
	case STATE_LISTENING:
		/*
		 * sleep here awaiting for remote api calls,
		 * or collect data from sensors
		 */
		collectdata();
		break;
	case STATE_START:
		startpumpjobs(pumpsOnVol);
		changestate(STATE_PUMPING);
		break;
	case STATE_PUMPING:
		finishpumpjobs(pumpsOnVol);
		break;
	default:
		Serial.print("no such state: ");
		Serial.println(state);
		break;
	}

	/*
	 * FIXME This delay ensures the whole system won't be blocked.
	 * Especially like virtual COM device, these guys will hang without
	 * delay. But we need to provide a proper value so that there won't
	 * be too much loss of the whole system efficiency.
	 */
	delay(100);
}

/*******************************************************************************
 * Function Name  : RGBLEd
 * Description    : Uses RGBLed port D7 to show on Led when drinks are done
 *                  
 * Input          : From Pump Process
 * Output         : none
 *******************************************************************************/
 
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t k=0; k<strip.numPixels(); k++) {
      strip.setPixelColor(k, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t k, j;

  for(j=0; j<256; j++) {
    for(k=0; k<strip.numPixels(); k++) {
      strip.setPixelColor(k, Wheel((k+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/*******************************************************************************
 * Function Name  : drinkbotEffects
 * Description    : This triggers various effects on the RGB Strip,
 * Input          : effect name
 * Output         : None.
 * Return         : 0 for success, others for error numbers
 *******************************************************************************/
int drinkbotEffects(String command)
{
//	int i;

	Serial.print(__func__);
	Serial.print(", args: ");
	Serial.println(command);

	if (command.equals("colorWipe")) {
		Serial.println("colorWipe Drinkbot...");
		colorWipe(strip.Color(0, 0, 200), 20); // Blue
		colorWipe(strip.Color(0, 0, 0), 20); // Black
	}
	
    if (command.equals("rainbow")) {
		Serial.println("Rainbow Drinkbot...");
		rainbow(20);
		delay(1000);
		colorWipe(strip.Color(0, 0, 0), 20); // Black
	}


	Serial.println("dump all variables:");
	return 0;
}

/*******************************************************************************
 * Function Name  : drinkbotSetPumps
 * Description    : Setup and start the pumping jobs,
 *                  e.g. P0:xx;P1:xx;P2:xx;P3:xx;P4:xx;P5:xx;P6:xx;P7:xx
 *                  where P0 ~ P7 are the 8 pumps and xx is the cl value.
 * Input          : pumps id and corresponding cl value string
 * Output         : None.
 * Return         : 0 for success, others for error numbers
 *******************************************************************************/
int drinkbotSetPumps(String command)
{
	int i;

	Serial.print(__func__);
	Serial.print(", args: ");
	Serial.println(command);

	if (command.length() != 47) {
		Serial.println("Please use correct format for args, e.g.");
		Serial.println("\tP0:10;P1:00;P2:20;P3:05;P4:07;P5:15;P6:00;P7:10");
		return -EINVAL;
	}

	if (state != STATE_LISTENING) {
		return -EBUSY;
	}

	/* parse command */
	Serial.print("pumpsOnVol: ");
	for (i = 0; i < NPUMPS; i++) {
		pumpsOnVol[i] = parseparams(command, i);
		Serial.print(pumpsOnVol[i]);
		Serial.print(' ');
	}
	Serial.println("");

	changestate(STATE_START);
	return 0;
}

/*******************************************************************************
 * Function Name  : drinkbotDebug
 * Description    : Debugging purpose
 * Input          : Random things
 * Output         : None
 * Return         : The character ascii code it received
 *******************************************************************************/
int drinkbotDebug(String command)
{
	int i;

	Serial.print(__func__);
	Serial.print(", args: ");
	Serial.println(command);

	if (command.equals("reset")) {
		Serial.println("Resetting Drinkbot...");
		RGB.control(true);
		RGB.brightness(64);
		RGB.color(255, 0, 0);
		colorWipe(strip.Color(5, 0, 0), 20); // Red
		colorWipe(strip.Color(0, 0, 0), 20); // Black
		RGB.control(false);
		for (i = 0; i < NPUMPS; i++)
			controlpump(i, false);
		changestate(STATE_BOOTING);
	}


	Serial.println("dump all variables:");

	Serial.print("temperature: ");
	Serial.println(temperature);

	Serial.print("bac1: ");
	Serial.println(bac1);

	Serial.print("bac2: ");
	Serial.println(bac2);

	Serial.print("state: ");
	Serial.println(state);

	Serial.print("timeStart: ");
	Serial.println(timeStart);

	Serial.print("pumpsOnVol: ");
	for (i = 0; i < NPUMPS; i++) {
		Serial.print(pumpsOnVol[i]);
		Serial.print(' ');
	}
	Serial.println("");

	Serial.print("pumpsStatus: ");
	for (i = 0; i < NPUMPS; i++) {
		Serial.print(pumpsStatus[i]);
		Serial.print(' ');
	}
	Serial.println("");

	return command.charAt(0);
}


static void changestate(int s)
{
	if (s < 0 || s >= STATE_MAX)
		Serial.print("change state failed: ");
	else
		Serial.print("change state: ");
	Serial.print(state);
	Serial.print(" -> ");
	Serial.println(s);
	state = s;
}

// the parameters should follow below format:
//     P0:XX;P1:XX;P2:XX;P3:XX;P4:XX;P5:XX;P6:XX;P7:XX
static int parseparams(String command, int id)
{
	return command.substring(6*id+3, 6*id+5).toInt();
}

static double calculateBAC(int mq3_pin)
{
	int mq3_raw = 0, mq3_raw_max = 0;
	int i;
	mq3_raw = analogRead(mq3_pin);
	if (mq3_raw < ALCOHOL_THRES) {
		for(i = 0; i < 100; i++) {
			mq3_raw = analogRead(mq3_pin);
			mq3_raw_max = (mq3_raw > mq3_raw_max) ? mq3_raw : mq3_raw_max;
		}
	}
	return pow(((-3.757)*pow(10, -7))*mq3_raw_max, 2) + 0.0008613*mq3_raw_max - 0.3919;
}

static int collectdata(void)
{

	bac1 = calculateBAC(IO_ALCOH1);
	bac2 = calculateBAC(IO_ALCOH2);

	tsensor.requestTemperatures();
	temperature = tsensor.getTempCByIndex(0);

	return 0;
}

static void controlpump(int id, boolean on)
{
	if (id < 0 || id > NPUMPS) {
		Serial.print("invalid pump id: ");
		Serial.println(id);
		id = constrain(id, 0, (NPUMPS-1));
	}

	pinMode(pump2io[id], OUTPUT);
	if (on) {
		digitalWrite(pump2io[id], HIGH);
		pumpsStatus[id] = 1;
	} else {
		digitalWrite(pump2io[id], LOW);
		pumpsStatus[id] = 0;
	}
}

static void startpumpjobs(int p[NPUMPS])
{
	int i;

	timeStart = millis();
	Serial.print("timeStart: ");
	Serial.println(timeStart);

	Serial.print("starting pumps: ");
	for (i = 0; i < NPUMPS; i++) {
		if (p[i] != 0) {
			controlpump(i, true);
			Serial.print(i);
			Serial.print(' ');
			pixels.show(); // Sends command to RGB Strip
			pixels.setPixelColor(i * 3, pixels.Color(0,0,5)); // Turns on the related led on the RGB Strip
			RGB.control(true);
			RGB.brightness(64);
			RGB.color(0, 0, 255);
			pixels.show(); // Sends command to RGB Strip
		}
	}
	Serial.println("");
}

static int npumpson(int ps[NPUMPS])
{
	int i, n = 0;
	for (i = 0; i < NPUMPS; i++)
		n += ps[i];
	return n;
}

static void finishpumpjobs(int p[NPUMPS])
{
	int i;
	unsigned long timeDelta;

	// FIXME
	// support up to about 49 days for an unsigned long millis
	// better to wrap around
	timeDelta = millis() - timeStart;

	for (i = 0; i < NPUMPS; i++) {
		if (timeDelta > vol2time(p[i]) &&
		    pumpsStatus[i] == 1) {
			Serial.print("stoping pump: ");
			Serial.print(i);
			pixels.show(); // Sends command to RGB Strip
			pixels.setPixelColor(i * 3, pixels.Color(0,0,0)); // Turns off the related led on the RGB Strip
			pixels.show(); // Sends command to RGB Strip
			RGB.control(true);
			RGB.brightness(64);
			RGB.color(255, 0, 0);
			Serial.print(" in ");
			Serial.print(timeDelta);
			Serial.println("ms");
			controlpump(i, false);
		}
	}

	if (npumpson(pumpsStatus) == 0) {
		Serial.print("All pumping jobs finished, time elapsed: ");
		Serial.println(timeDelta);
		RGB.control(true);
		RGB.color(0, 255, 0);
		RGB.brightness(64);
		colorWipe(strip.Color(0, 200, 0), 20); // Green
		colorWipe(strip.Color(0, 0, 0), 20); // Black
		colorWipe(strip.Color(0, 100, 0), 20); // Green
		colorWipe(strip.Color(0, 0, 0), 20); // Black
		colorWipe(strip.Color(0, 50, 0), 20); // Green
		colorWipe(strip.Color(0, 0, 0), 20); // Black
		colorWipe(strip.Color(0, 5, 0), 20); // Green
		colorWipe(strip.Color(0, 0, 0), 20); // Black
		RGB.control(false);
		changestate(STATE_LISTENING);
	}
}