#include "Arduino.h"
#include "ApplicationProperties.h"
#include <IRremote.hpp>

#include "LcdService.h"
#include "IrRemoteConfig.h"

extern const int READ_DELAY_MS = 350; //ms to wait between IR signal reads

IrRemoteConfig::IrRemoteConfig(LcdService* lcd, RTC_DS3231* rtc) {
  _lcd = lcd;
  _rtc = rtc;
  _menuPage = 0;

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void IrRemoteConfig::setRtcTime() {
  int dateTime[5];
  _readDateTime(dateTime);
  _rtc->adjust(DateTime(dateTime[0], //year
    dateTime[1], //month
    dateTime[2], //day
    dateTime[3], //hours
    dateTime[4], //minutes
    0)); //seconds
  _lcd->printWithDelay(F("Date time was set"), 2000);
}

void IrRemoteConfig::_readDateTime(int* dateTime) {
  int day   = (int)_readFloatNumberInputWithValidation(F("Enter day 1-31"),   1,    31);
  int month = (int)_readFloatNumberInputWithValidation(F("Enter month 1-12"), 1,    12);
  int year  = (int)_readFloatNumberInputWithValidation(F("Enter year"),       2023, 2100);
  int hours = (int)_readFloatNumberInputWithValidation(F("Enter hours 0-23"), 0,    23);
  int mins  = (int)_readFloatNumberInputWithValidation(F("Enter mins 0-59"),  0,    59);

  dateTime[0] = year;
  dateTime[1] = month;
  dateTime[2] = day;
  dateTime[3] = hours;
  dateTime[4] = mins;
}

float IrRemoteConfig::_readFloatNumberInputWithValidation(String title, float min, float max) {
  while (true) {
    _lcd->print(title);
    float value = _readInput().toFloat();
    if (value < min) {
      _lcd->printInLine(F("Value is too small"), 0);
      _lcd->printInLine(String(value), 1);
      delay(3000);
      continue;
    } else if (value > max) {
      _lcd->printInLine(F("Value is too big"), 0);
      _lcd->printInLine(String(value), 1);
      delay(3000);
    } else {
      return value;
    }
  }
}

String IrRemoteConfig::_readInput() {
  String buffer = "";
  while (true) {
    if (IrReceiver.decode()) {
      delay(READ_DELAY_MS);
      IrReceiver.resume();
      uint16_t command = IrReceiver.decodedIRData.command;
      if      (command == BTN_1) { _appendCharAndLcdPrint(&buffer, '1'); }
      else if (command == BTN_2) { _appendCharAndLcdPrint(&buffer, '2'); }
      else if (command == BTN_3) { _appendCharAndLcdPrint(&buffer, '3'); }
      else if (command == BTN_4) { _appendCharAndLcdPrint(&buffer, '4'); }
      else if (command == BTN_5) { _appendCharAndLcdPrint(&buffer, '5'); }
      else if (command == BTN_6) { _appendCharAndLcdPrint(&buffer, '6'); }
      else if (command == BTN_7) { _appendCharAndLcdPrint(&buffer, '7'); }
      else if (command == BTN_8) { _appendCharAndLcdPrint(&buffer, '8'); }
      else if (command == BTN_9) { _appendCharAndLcdPrint(&buffer, '9'); }
      else if (command == BTN_0) { _appendCharAndLcdPrint(&buffer, '0'); }
      else if (command == BTN_ASTERIX) { _appendCharAndLcdPrint(&buffer, '.'); }
      else if (command == BTN_HASH && buffer.length() > 0) {
        buffer = buffer.substring(0, buffer.length() - 1); // remove last char
        _lcd->printInLine(buffer, 1);
      } else if (command == BTN_OK) {
        break;
      }
    }
  }
  return buffer;
}

void IrRemoteConfig::_appendCharAndLcdPrint(String* pBuffer, char symbol) {
  pBuffer->concat(symbol);
  _lcd->printInLine(*pBuffer, 1);
}

void IrRemoteConfig::startIrRemoteConfigurationMenu() {
  _menuPage = 0;
  bool shouldEnterMenu = _askToEnterMenu(F("Start config?"));
  _lcd->clear();
  if (shouldEnterMenu) {
    while (true) {
      switch (_menuPage) {
        case 0: {
          bool shouldExit = _menuPage0();
          if (shouldExit) return;
          break;
        }
        default: {
          _lcd->printInLine(F("Error! No such menu"), 0);
        }
      }
      delay(1000);
    }
  }
}

bool IrRemoteConfig::_menuPage0() {
  _lcd->printInLine(F("1.Date time"), 0);
  _lcd->printInLine(F("2.Anem config"), 1);

  while (true) {
    if (IrReceiver.decode()) {
      delay(READ_DELAY_MS);
      IrReceiver.resume();
      uint16_t command = IrReceiver.decodedIRData.command;
      if (command == BTN_1) {
        if (_askToEnterMenu(F("Set Date time?"))) {
          setRtcTime();
        }
        return false;
      } else if (command == BTN_2) {
        if (_askToEnterMenu(F("Anem config?"))) {
          _configureAnemConfig();
        }
        return false;
      } else if (command == BTN_HASH) {
        return true; // exit menu
      }
    }
  }
}

void IrRemoteConfig::_configureAnemConfig() {
  if (_askToEnterMenu(F("Wind high m/s?"))) {
    float currValue;
    GET_ANEM_WIND_SPEED_HIGH(currValue);
    if (_askToEnterMenu(String(currValue))) {
      float newValue = _readFloatNumberInputWithValidation(F("Enter m/s"), 0.0, 30.0);
      SET_ANEM_WIND_SPEED_HIGH(newValue);
    }
  }
  if (_askToEnterMenu(F("Wind protect m/s?"))) {
    float currValue;
    GET_ANEM_WIND_SPEED_MAX(currValue);
    if (_askToEnterMenu(String(currValue))) {
      float newValue = _readFloatNumberInputWithValidation(F("Enter m/s"), 0.0, 30.0);
      SET_ANEM_WIND_SPEED_MAX(newValue);
    }
  }
  if (_askToEnterMenu(F("Wind wait secs?"))) {
    float currValue;
    GET_SECONDS_TO_WAIT_WIND_CALM(currValue);
    if (_askToEnterMenu(String(currValue))) {
      float newValue = _readFloatNumberInputWithValidation(F("Enter secs"), 0.0, 3600.0);
      SET_SECONDS_TO_WAIT_WIND_CALM(newValue);
    }
  }
}

bool IrRemoteConfig::_askToEnterMenu(String title) {
  _lcd->printInLine(title, 0);
  _lcd->printInLine(F("OK- Yes, # - No"), 1);
  uint32_t startSecs = _rtc->now().secondstime();
  while (_rtc->now().secondstime() - startSecs < MENU_ASK_TO_ENTER_TIMEOUT_SEC) {
    if (IrReceiver.decode()) {
      delay(READ_DELAY_MS);
      IrReceiver.resume();
      if (IrReceiver.decodedIRData.command == BTN_OK) {
        return true;
      } else if (IrReceiver.decodedIRData.command == BTN_HASH) {
        return false;
      }
    }
  }
  return false;
}
