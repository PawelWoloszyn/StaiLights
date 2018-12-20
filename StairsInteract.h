#ifndef StairsInteract_h
#define StairsInteract_h

#include "Arduino.h"
#include "Bounce2.h"

class StairsInteract
{
public:

StairsInteract();
boolean turnLights(boolean, boolean);
void menuLayerOne();
void initialize();
int getFadeTime();
int getStepDelay();
int getMaxBrightness();
int getMinBrightness();
int getTimeOn();

private:
// functions
int potentiometerRead();
void beep(int amount);
void longBeep();
int menuLayerTwo(int arg);
void updateMyButtons();
int adjustValue(int arg, int multiplier);
void loadEepromValues();
// variables
int parameters[5];


Bounce button0;
Bounce button1;
Bounce button2;

};

#endif
