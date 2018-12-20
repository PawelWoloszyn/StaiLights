#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Dusk2Dawn.h>
#include <TomorrowDate.h>
#include <StairsInteract.h>

//RTC connections SDA -> A4 SCL -> A5
#define ERROR_LED 8 // connect red diode to indicate errors
#define SENSOR_UPSTAIRS 7
#define SENSOR_DOWNSTAIRS 6
#define DOWNSTAIRS_DETECTION 1
#define UPSTAIRS_DETECTION 2
#define NO_DETECTION 0
#define ON 0

//change this for different location
#define XCORDS 50.039968
#define YCORDS 19.974380
#define UTC_OFFSET 1 // utc offset for your time zone

boolean daySaving = false; // set true if DST is in effect

boolean running = false;
Dusk2Dawn location(XCORDS, YCORDS, UTC_OFFSET);
int turnOnTime;
int turnOffTime;
int buffer[3];

StairsInteract stairs;

tmElements_t tm;

boolean setCurrentDate(){
	if(RTC.read(tm)) {
		buffer[0] = 1970 + tm.Year;
		buffer[1] = tm.Month;
		buffer[2] = tm.Day;
		return true;
	}
	return false;
}

void setOnTime(){
	if(setCurrentDate()) {
		int locationSunset = location.sunset(buffer[0], buffer[1], buffer[2], daySaving);
		if(locationSunset >= 0 && locationSunset <= 1440) {
			turnOnTime = locationSunset;
		}
	}
}

void setOffTime(){
	if(setCurrentDate()) {
		int locationSunrise = location.sunrise(buffer[0], buffer[1], buffer[2], daySaving);
		if(locationSunrise >= 0 && locationSunrise <= 1440) {
			turnOffTime = locationSunrise;
		}
	}
}

void setOffTimeTomorrow(){
	if(setCurrentDate()) {
		TomorrowDate date((buffer[0]-2000), buffer[1], buffer[2]);
		int locationSunrise = location.sunrise((2000 + date.gety()), date.getm(), date.getd(), daySaving);
		if(locationSunrise >= 0 && locationSunrise <= 1440) {
			turnOffTime = locationSunrise;
		}
	}
}

int getCurrentTime(){
	return ((tm.Hour*60) + tm.Minute);
}

//returns true when it should be dark outside
boolean check(){
	if(RTC.read(tm)) {
		int currentTime = getCurrentTime();
		if(currentTime > turnOnTime || currentTime < turnOffTime) {
			return true;
		}else{
			return false;
		}
	}else{
		Serial.println("Problem with RTC timer!");
		if (RTC.chipPresent()) {
			Serial.println("The DS1307 is stopped.  Please run the SetTime");
		} else {
			Serial.println("DS1307 read error!  Please check the circuitry.");
		}
		return false;
	}
	return false;
}

int checkSensors(){
	if(digitalRead(SENSOR_DOWNSTAIRS) == ON)
		return DOWNSTAIRS_DETECTION;
	else if(digitalRead(SENSOR_UPSTAIRS) == ON)
		return UPSTAIRS_DETECTION;
	else
		return NO_DETECTION;
}
/////////////////////////////// SETUP /////////////////////////////////////////
void setup() {
	//setup sensors
	pinMode(SENSOR_DOWNSTAIRS, INPUT_PULLUP);
	pinMode(SENSOR_UPSTAIRS, INPUT_PULLUP);
	pinMode(ERROR_LED, OUTPUT);
	stairs.initialize();
	//setup buttons
	Serial.begin(9600);
	delay(200);
	// wait for RTC data
	while(!RTC.read(tm)) {
		Serial.println("Problem with RTC data");
		delay(1000);
	}
	delay(200);
	setOnTime();
	setOffTime();
	running = check();
	if(running && (getCurrentTime() > turnOnTime)) {
		setOffTimeTomorrow();
	}
}
/////////////////////// MAIN loop()////////////////////////////////
void loop() {
	while(running) {
		if(!check()) {
			setOnTime();
			running = false;
			break;
		}
		if(checkSensors() == DOWNSTAIRS_DETECTION) {
			// Serial.println("Downstairs detection");
			if(stairs.turnLights(true, true)) {
				unsigned long timeOff = millis() + stairs.getTimeOn();
				// Serial.println("All stair ligts are up now");
				while(true) {
					if(checkSensors() != NO_DETECTION) {
						// if there was yet another detection reset the timer
						// Serial.println("Timer reset invoked");
						timeOff = millis() + stairs.getTimeOn();
					}
					if(millis() > timeOff) {
						// Serial.println("We are done here turn it off now");
						if(stairs.turnLights(false, true))
							break;
					}
				}

			}
		} else if(checkSensors() == UPSTAIRS_DETECTION) {
			// Serial.println("Upstairs detection");
			if(stairs.turnLights(true, false)) {
				unsigned long timeOff = millis() + stairs.getTimeOn();
				// Serial.println("All stair ligts are up now");
				while(true) {
					if(checkSensors() != NO_DETECTION) {
						// if there was yet another detection reset the timer
						// Serial.println("Timer reset invoked");
						timeOff = millis() + stairs.getTimeOn();
					}
					if(millis() > timeOff) {
						// Serial.println("We are done here turn it off now");
						if(stairs.turnLights(false, false))
							break;
					}
				}
			}
		}
		delay(1000); // a temporary console anti-spam
	}


	while(!running) {
		if(check()) {
			setOffTimeTomorrow();
			running = true;
			break;
		}
		delay(10000);
	}
}
