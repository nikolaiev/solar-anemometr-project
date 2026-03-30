#include "Arduino.h"
#include "LcdService.h"
#include "ApplicationProperties.h"

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

void LcdService::printWithDelay(String message, int delayMs) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print(message);
  delay(delayMs);
}

void LcdService::printGridProtect() {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print("Grid protection mode");
}

void LcdService::printSleeping(int minutesSunInterval) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print("Waiting sunrise for ");
  _lcd->setCursor(0, 1);
  _lcd->print(minutesSunInterval);
  _lcd->print(" min");
}

void LcdService::printWindProtection(double anemVoltage, long windHighSpeedDuration) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print("Wind speed: ");
  _lcd->print(anemVoltage / VOLT_TO_METER_PER_SEC);
  _lcd->print(" m/s ");
  _lcd->setCursor(0, 1);
  _lcd->print("Wind protection mode");
  _lcd->setCursor(0, 2);
  _lcd->print("Time (sec):");
  _lcd->setCursor(0, 3);
  _lcd->print(windHighSpeedDuration);
}

void LcdService::printNormal(double anemVoltage, double solarAzimuth, double solarElevation, DateTime now) {
  _lcd->clear();
  _lcd->setCursor(0, 0);
  _lcd->print("Wind speed: ");
  _lcd->print(anemVoltage / VOLT_TO_METER_PER_SEC);
  _lcd->print(" m/s ");
  _lcd->setCursor(0, 1);
  _lcd->print("Anem signal: ");
  _lcd->print(anemVoltage);
  _lcd->print(" v");
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
