#include "Arduino.h"
#include "StairsInteract.h"
#include <Tlc5940.h>
#include <tlc_fades.h>
#include <Bounce2.h>
#include <EEPROM.h>

#define BUTTON0 A0  // button responsible for setting change option/save
#define BUTTON1 A1 // button responsible for enter/back
#define BUTTON2 A3 // button responsible for exit
#define POTENTIOMETER_PIN A2
#define BRIGHTNESS_MULTIPLIER 4 // leave it at 4
#define ALL_ON_TIME_MULTIPLIER 13 // time regulation multiplier
#define FADE_TIME_MULTIPLIER 3
#define STEP_DELAY_MULTIPLIER 2
#define BUZZER 5
#define START_OUTPUT 2
#define STOP_OUTPUT 19
#define STEP_AMOUNT ((STOP_OUTPUT - START_OUTPUT) +1)
#define ON 0
#define OFF 1

int parameters[5];// = {1500, 5000, 2000, 1000, 0};

Bounce button0 = Bounce();
Bounce button1 = Bounce();
Bounce button2 = Bounce();

// CONSTRUCTOR
StairsInteract::StairsInteract(){
}
int StairsInteract::getTimeOn(){
	return parameters[1];
}
int StairsInteract::getFadeTime(){
	return parameters[2];
}

int StairsInteract::getStepDelay(){
	return parameters[3];
}

void StairsInteract::loadEepromValues(){
	for(int i = 0; i < 4; i++) {
		// value 255 is considered as not defined value
		if(EEPROM.read(i) != 255) {
			parameters[i] = EEPROM.read(i);
		}
	}
}

void StairsInteract::initialize(){
	Serial.begin(9600);
	Tlc.init();
	// these are default values:
	parameters[0]= 1500; // maxBrightness
	parameters[1]= 5000; // timeOn
	parameters[2]= 2000; // fadeTime
	parameters[3]= 1000; // stepDelay
	parameters[4]= 0;		 // minBrightness
	pinMode(BUTTON0, INPUT_PULLUP);
	pinMode(BUTTON1, INPUT_PULLUP);
	pinMode(BUTTON2, INPUT_PULLUP);
	pinMode(BUZZER, OUTPUT);
	button0.attach(BUTTON0);
	button0.interval(5);
	button1.attach(BUTTON1);
	button1.interval(5);
	button2.attach(BUTTON2);
	button2.interval(5);
	loadEepromValues();
}

void StairsInteract::updateMyButtons(){
	button0.update();
	button1.update();
	button2.update();
}

int StairsInteract::potentiometerRead(){
	return analogRead(POTENTIOMETER_PIN);
}

void StairsInteract::beep(int amount){
	int x = amount + 1;
	while(x >0) {
		digitalWrite(BUZZER, ON);
		delay(100);
		digitalWrite(BUZZER, OFF);
		delay(100);
		x--;
	}
}

void StairsInteract::longBeep(){
	digitalWrite(BUZZER, ON);
	delay(300);
	digitalWrite(BUZZER, OFF);
}

int StairsInteract::adjustValue(int arg, int multiplier){
	Tlc.clear();
	Tlc.update();
	while(true) {
		int value = potentiometerRead() * multiplier;
		// visual presentaton for max brihtness
		if(arg == 0) {
			Tlc.setAll(value);
			Tlc.update();
		}
		updateMyButtons();
		if(button0.fell()) {
			//settings were saved
			Tlc.clear();
			Tlc.update();
			int valueToSave = value/(multiplier * 4);
			//255 is considered as not defined value
			//it causes very insignificant precision loss
			if(valueToSave == 255)
				valueToSave = 254;
			EEPROM.write(arg, valueToSave);
			parameters[arg] = value;
			return 0;
		}
		if(button1.fell()) {
			// back to 1st level
			Tlc.clear();
			Tlc.update();
			return 1;
		}
		if(button2.fell()) {
			// exit settings menu completely
			Tlc.clear();
			Tlc.update();
			return 2;
		}
	}
}
//MENU LAYERS
int StairsInteract::menuLayerTwo(int arg){
	// BRIGHTNESS settings
	if(arg == 0) {
		return adjustValue(arg, BRIGHTNESS_MULTIPLIER);
	}
	// TIME FOR ALL TO BE ON SETTINGS
	else if(arg == 1) {
		return adjustValue(arg, ALL_ON_TIME_MULTIPLIER);
	}
	// FADE TIME SETTINGS
	else if(arg == 2) {
		return adjustValue(arg, FADE_TIME_MULTIPLIER);
	}
	// STEP DELAY SETTINGS
	else if(arg == 3) {
		return adjustValue(arg, STEP_DELAY_MULTIPLIER);
	}
	else return 2;     // Exit menus completely
}

void StairsInteract::menuLayerOne(){
	longBeep();
	int currentPosition = 0;
	while(true) {
		updateMyButtons();
		// reaction 1  CHANGE VALUE TO ADJUST
		if(button0.fell()) {
			if(currentPosition < 3)
				currentPosition++;
			else
				currentPosition = 0;
			beep(currentPosition);
		}
		// reaction 2  GO TO SECOND LAYER
		if(button1.fell()) {
			longBeep();
			beep(currentPosition);
			if(currentPosition >= 0 && currentPosition < 4) {
				int response = menuLayerTwo(currentPosition);
				if(response == 0) {
					// settings were saved
					longBeep();
					break;
				}else if(response == 1) {
					// you backed to first level menu
					beep(currentPosition);
				}else if(response == 2) {
					//you clicked exit from 2nd level settingsMenu
					longBeep();
					break;
				}
			}
		}
		//reaction 3  EXIT
		if(button2.fell()) {
			longBeep();
			break;
		}
	}
}

// main function for controlling
boolean StairsInteract::turnLights(boolean turnOn, boolean rising){
	int startOuput = (rising ? START_OUTPUT : STOP_OUTPUT);
	int startValue = (turnOn ? parameters[4] : parameters[0]);
	int stopValue = (turnOn ? parameters[0] : parameters[4]);
	int i, j;
	Serial.println("test1");
	for(i = startOuput, j = 0; j < STEP_AMOUNT; j++) {
		if(!tlc_isFading(i)) {
			unsigned long time = millis();
			tlc_addFade(i, startValue, stopValue,
			            time + (parameters[3] * j),
			            (time + (parameters[3] * j) + parameters[2] ));
		} else {
			return false;
		}

		if(rising) {
			i++;
		} else {
			i--;
		}
	}
	while(tlc_updateFades());
	// if statement below has to be added due to bug which doesn't shut down last segment
	if(!turnOn){
		if(rising){
			Tlc.set(STOP_OUTPUT,0);
			Tlc.update();
		}else{
			Tlc.set(START_OUTPUT, 0);
			Tlc.update();
		}
	}
	return true;
}
