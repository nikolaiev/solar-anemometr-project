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
    void printGridProtect();
    void printSleeping(int minutesSunInterval);
    void printWindProtection(double anemVoltage, long windHighSpeedDuration);
    void printNormal(double anemVoltage, double solarAzimuth, double solarElevation, DateTime now);
    void clear();
  private:
    LiquidCrystal_I2C* _lcd;
};

#endif
