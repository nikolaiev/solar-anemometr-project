#ifndef TrackerSystemService_h
#define TrackerSystemService_h

#include "Arduino.h"
#include "LcdService.h"

class TrackerSystemService {
  public:
    TrackerSystemService(LcdService* lcd,
                         float anemVoltageHigh, float anemVoltageMax, long secondsToWaitWindCalm);
    void reload(float anemVoltageHigh, float anemVoltageMax, long secondsToWaitWindCalm);
    void checkGridOutageAndGoToProtectModeIfNeeded();
    void checkWindSpeedAndGoToProtectModeIfNeeded(unsigned long nowUnix);
    void goToProtectMode();
    void goToNormalMode();
    void resetAfterSleep(); // forces re-evaluation of grid and wind state on next loop iteration
    bool isInGridProtectMode();
    bool isInWindProtectMode();
    unsigned long getLastTimeWindHighSpeed();
    double getWindSpeedMs();
  private:
    void turnTrackersOn();
    void turnTrackersOff();
    void digitalWritePowerControlPins(int value);

    LcdService* _lcd;
    bool _isGridProtectModeOn;
    bool _isWindProtectModeOn;
    double _anemVoltage;
    float _anemVoltageHigh;
    float _anemVoltageMax;
    long _secondsToWaitWindCalm;
    unsigned long _lastTimeWindHighSpeed; //unix timestamp of last high-speed wind event
};

#endif
