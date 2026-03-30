#ifndef IrRemoteConfig_h
#define IrRemoteConfig_h

#include "Arduino.h"
#include "RTClib.h"
#include "LcdService.h"

class IrRemoteConfig {
  public:
    IrRemoteConfig(LcdService* lcd, RTC_DS3231* rtc);
    void startIrRemoteConfigurationMenu();
    void setRtcTime();
  private:
    void _readDateTime(int* resultArray);
    String _readInput();
    void _appendCharAndLcdPrint(String* pBuffer, char symbol);
    float _readFloatNumberInputWithValidation(String title, float min, float max);
    bool _askToEnterMenu(String title);
    bool _menuPage0();
    void _configureAnemConfig();

    LcdService* _lcd;
    RTC_DS3231* _rtc;
    int _menuPage;
};

#endif
