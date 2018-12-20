#include "Arduino.h"
#include "TomorrowDate.h"

byte today[3];
byte tomorrow[3];

long TomorrowDate::g(int y, int m, int d) {
	//http://alcor.concordia.ca/~gpkatch/gdate-algorithm.html
	m = (m + 9) % 12;
	y = y - m / 10;
	return 365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + (d - 1);
}

void TomorrowDate::CalcTomorrow(long g) {
	//http://alcor.concordia.ca/~gpkatch/gdate-algorithm.html

	g = g + 1; // add one day

	int y = (10000 * g + 14780) / 3652425;
	int ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
		if (ddd < 0) {
			y = y - 1;
			ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
		}
		int mi = (100 * ddd + 52) / 3060;
		int mm = (mi + 2) % 12 + 1;
		y = y + (mi + 2) / 12;
		int dd = ddd - (mi * 306 + 5) / 10 + 1;

		tomorrow[0] = y;
		tomorrow[1] = mm;
		tomorrow[2] = dd;
}

//constructor
TomorrowDate::TomorrowDate(int year, int month, int day){
  today[0]=year;
  today[1]=month;
  today[2]=day;
  CalcTomorrow(g(today[0],today[1],today[2]));
}

int TomorrowDate::getd(){
  return tomorrow[2];
}

int TomorrowDate::getm(){
  return tomorrow[1];
}

int TomorrowDate::gety(){
  return tomorrow[0];
}
