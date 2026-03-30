#include "Arduino.h"
#include "ApplicationProperties.h"
#include "TrackerSystemService.h"

TrackerSystemService::TrackerSystemService(LcdService* lcd,
                                           float anemVoltageHigh, float anemVoltageMax, long secondsToWaitWindCalm) {
  _lcd = lcd;
  _isGridProtectModeOn = false;
  _isWindProtectModeOn = false;
  _anemVoltage = 0.;
  _lastTimeWindHighSpeed = 0;
  reload(anemVoltageHigh, anemVoltageMax, secondsToWaitWindCalm);
}

void TrackerSystemService::reload(float anemVoltageHigh, float anemVoltageMax, long secondsToWaitWindCalm) {
  _anemVoltageHigh       = anemVoltageHigh;
  _anemVoltageMax        = anemVoltageMax;
  _secondsToWaitWindCalm = secondsToWaitWindCalm;
}

bool TrackerSystemService::isInGridProtectMode() { return _isGridProtectModeOn; }
bool TrackerSystemService::isInWindProtectMode()  { return _isWindProtectModeOn; }

unsigned long TrackerSystemService::getLastTimeWindHighSpeed() { return _lastTimeWindHighSpeed; }
double TrackerSystemService::getWindSpeedMs() { return _anemVoltage / VOLT_TO_METER_PER_SEC; }

void TrackerSystemService::checkGridOutageAndGoToProtectModeIfNeeded() {
  int gridStateCode = digitalRead(GRID_STATE_SIGNAL_PIN);
  if (gridStateCode == HIGH && !_isGridProtectModeOn) {
    goToProtectMode();
    _isGridProtectModeOn = true;
  } else if (gridStateCode == LOW && _isGridProtectModeOn) {
    goToNormalMode();
    _isGridProtectModeOn = false;
  }
}

void TrackerSystemService::checkWindSpeedAndGoToProtectModeIfNeeded(unsigned long nowUnix) {
  _anemVoltage = analogRead(ANEMOMETR_SIGNAL_PIN) * TO_ANEM_VOLTAGE;
  if (_anemVoltage >= _anemVoltageHigh) {
    _lastTimeWindHighSpeed = nowUnix;
  }
  if (_anemVoltage >= _anemVoltageMax && !_isWindProtectModeOn) {
    goToProtectMode();
    _isWindProtectModeOn = true;
    _lastTimeWindHighSpeed = nowUnix;
  } else if (_isWindProtectModeOn && _anemVoltage <= _anemVoltageHigh
             && (nowUnix - _lastTimeWindHighSpeed) > (unsigned long)_secondsToWaitWindCalm) {
    goToNormalMode();
    _isWindProtectModeOn = false;
  }
}

void TrackerSystemService::goToProtectMode() {
  _lcd->printWithDelay("Go to protect mode", 3000);
  turnTrackersOff();
  delay(10000); //10 sec wait till power blocks for trackers turn off completelly
  turnTrackersOn();
  _lcd->printWithDelay("Wait trackers to get a protect position", 3000);
  delay(MILLIS_TRACKER_TO_PROTECT_MODE); //300 sec — 5 mins to get the waiting position
  turnTrackersOff();
  _lcd->printWithDelay("Waiting for the better times", 3000);
}

void TrackerSystemService::goToNormalMode() {
  turnTrackersOn();
  _lcd->printWithDelay("Go to normal mode", 3000);
}

void TrackerSystemService::resetAfterSleep() {
  // Force both protection checks to re-evaluate on the next loop iteration.
  // Grid and wind conditions are unknown after a long sleep.
  _isGridProtectModeOn = true;
  _isWindProtectModeOn = true;
}

void TrackerSystemService::turnTrackersOn()  { digitalWritePowerControlPins(HIGH); }
void TrackerSystemService::turnTrackersOff() { digitalWritePowerControlPins(LOW);  }

void TrackerSystemService::digitalWritePowerControlPins(int value) {
  digitalWrite(POWER_CONTROLE_PIN_1, value);
  digitalWrite(POWER_CONTROLE_PIN_2, value);
}
