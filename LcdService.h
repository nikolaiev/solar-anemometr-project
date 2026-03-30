#ifndef LcdService_h
#define LcdService_h

#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include "ApplicationProperties.h"

class LcdService {
  public:
    LcdService(LiquidCrystal_I2C* lcd);
    void print(String message);
    void printWithDelay(String message, int delayMs);
    void printTrackerSystemGridProtectState();
    void printSleeping(int minutesSunInterval);
    void printTrackerSystemWindProtectState(double windSpeedMs, long windHighSpeedDuration);
    void printTrackerSystemNormalState(double windSpeedMs, double solarAzimuth, double solarElevation, DateTime now);
    void printInLine(String message, short line);
    void clear();
  private:
    LiquidCrystal_I2C* _lcd;
};

#endif
