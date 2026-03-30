#include "Arduino.h"
#include "LcdService.h"

LcdService::LcdService(LiquidCrystal_I2C* lcd) {
  _lcd = lcd;
  _lcd->init();
  _lcd->backlight();
  print("Starting wind monitoring...");
}

void LcdService::clear() {
  _lcd->clear();
}

void LcdService::print(String message) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print(message);
}

void LcdService::printNoDelay(String message) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print(message);
  delay(3000);
}

void LcdService::printMonitoring(bool isSleeping, bool isGridProtectModeOn, bool isWindProtectModeOn,
                                  double anemVoltage, float voltToMeterPerSec,
                                  long windHighSpeedDuration, int minutesSunInterval,
                                  double solarAzimuth, double solarElevation,
                                  DateTime now) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  if (isGridProtectModeOn) {
    _lcd->print("Grid protection mode");
  } else {
    if (!isSleeping) {
      _lcd->print("Wind speed: ");
      _lcd->print(anemVoltage / voltToMeterPerSec);
      _lcd->print(" m/s ");
      _lcd->setCursor(0, 1);
      _lcd->print("Anem signal: ");
      _lcd->print(anemVoltage);
      _lcd->print(" v");
    } else {
      _lcd->print("Waiting sunrise for ");
      _lcd->setCursor(0, 1);
      _lcd->print(minutesSunInterval);
      _lcd->print(" min");
    }
    if (isWindProtectModeOn) {
      _lcd->setCursor(0, 1);
      _lcd->print("Wind protection mode");
      _lcd->setCursor(0, 2);
      _lcd->print("Time (sec):");
      _lcd->setCursor(0, 3);
      _lcd->print(windHighSpeedDuration);
    } else {
      _lcd->setCursor(0, 2);
      _lcd->print("az: ");
      _lcd->print(solarAzimuth);
      _lcd->print(" e: ");
      _lcd->print(solarElevation);

      _lcd->setCursor(0, 3);
      char bufTime[] = "hh:mm:ss";
      _lcd->print(now.toString(bufTime));
      _lcd->print("  ");
      char bufDate[] = "DD/MM/YY";
      _lcd->print(now.toString(bufDate));
    }
  }
}
