Stairs is electronics and programming project of controlling stair lights (led strips attached to bottom side of each step) using arduino nano and tlc5940.
Project contains both electronic schematics and code needed to create such controller, mind that I'm neither professional electronics
circuit designer nor software developer therefore both code and schematics might be adjusted to benefit your own implication.

Right now the device is under development and will remain in this state until implemented in real which is going to allow it's
long term testing and proof it's reliability. In this moment it only serves presentation purpose.

How does it work???
Arduino Nano controlls two TLC5940 controllers which are basicaly 16 channel PWM controller (each chip). PWM signal is inverted and used to drive MOSFET tranzistors which are responsible for regulating the current running through led strips. In order to automate this device DS1307 RTC is used. Thanks to this, device will only be switched on within specified period of time which is calculated by Dusk2Dawn library, lights are triggered by two PIR sensors one upstairs and one downstairs but there are plenty enough spare pins on Arduino nano to use other type of sesnor. Project also allows to externaly regulate device's parameters using microswitch buttons and potentiometer. 


Questions that might bother you:

* What do you need for logical inverters?
- TLC5940 cannot be connected directly to MOSFETS in this project due to it's 0 based logic, of course they could be driven with inverted logic but because of the way tlc functions, you wouldn't be able to completely shut down the lights.

* On schematic you started connecting TLC pins starting from output 2 not 0 why?
- It's because I bought one chip from second hand with one pin missing. Therefore I skipped pin0 and pin1(missing).

Future improvements:
* Definitely chaging DS1307 to DS3231 in order to gain benefits of having alarm clock function in real time clock, which could later on be used to     wake up arduino from sleep mode.
* Reworking board design to decrease it's dimensions, would probably make a 2 sided SMD PCB.



