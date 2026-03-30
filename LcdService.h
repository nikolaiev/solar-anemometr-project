#ifndef LcdService_h
#define LcdService_h

#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

class LcdService {
  public:
    LcdService(LiquidCrystal_I2C* lcd);
    void print(String message);
    void printNoDelay(String message);
    void printMonitoring(bool isSleeping, bool isGridProtectModeOn, bool isWindProtectModeOn,
                         double anemVoltage, float voltToMeterPerSec,
                         long windHighSpeedDuration, int minutesSunInterval,
                         double solarAzimuth, double solarElevation,
                         DateTime now);
    void clear();
  private:
    LiquidCrystal_I2C* _lcd;
};

#endif
