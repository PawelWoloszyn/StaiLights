#ifndef TomorrowDate_h
#define TomorrowDate_h

#include "Arduino.h"

class TomorrowDate
{
public:
TomorrowDate(int y, int m, int d);
int getm();
int gety();
int getd();

private:
  void CalcTomorrow(long g);
  long g(int year, int month, int day);
  int y;
  int ddd;
  int mi;
  int mm;
  int dd;
};
#endif
