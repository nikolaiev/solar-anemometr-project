#include <RTClib.h>
#include <avr/wdt.h>
#include "SolarPosition.h"
#include "LiquidCrystal_I2C.h"
#include "LowPower.h"
#include "ApplicationProperties.h"
#include "LcdService.h"
#include "IrRemoteConfig.h"
#include "TrackerSystemService.h"

RTC_DS3231 rtc;
SolarPosition sun (LATITUDE, LONGITUDE);
LiquidCrystal_I2C lcd (DISPLAY_I2C_ADDR, DISPLAY_COLS, DISPLAY_ROWS); //20*4
//LiquidCrystal_I2C lcd (0x27,DISPLAY_COLS,DISPLAY_ROWS); //16*2

LcdService* lcdService;
IrRemoteConfig* irRemoteConfig;
TrackerSystemService* trackerSystemService;

void setup() {
  float voltHigh, voltMax; long secsCalm;
  initEEPROM(voltHigh, voltMax, secsCalm); // MUST be first line

  //delay(5000); //to read while update
  // Serial.begin(9600);

  pinMode(ANEMOMETR_SIGNAL_PIN, INPUT);
  pinMode(GRID_STATE_SIGNAL_PIN, INPUT);

  pinMode(POWER_CONTROLE_PIN_1, OUTPUT);
  pinMode(POWER_CONTROLE_PIN_2, OUTPUT);

  lcdService = new LcdService(&lcd);
  irRemoteConfig = new IrRemoteConfig(lcdService, &rtc);
  trackerSystemService = new TrackerSystemService(lcdService, voltHigh, voltMax, secsCalm);

  initRtc(); //this one will put trackers into protection mode if anything happens with RTC module
  //should go after initRtc()
  initSolarPosition();

  //start configuration adjustment menu
  irRemoteConfig->startIrRemoteConfigurationMenu();
  // reload — picks up values changed in config menu, since we recalculate them and not directly use
  initEEPROM(voltHigh, voltMax, secsCalm);
  trackerSystemService->reload(voltHigh, voltMax, secsCalm);

  trackerSystemService->goToNormalMode(); // initial tracker power-on if was turned off
}

void initEEPROM(float& voltHigh, float& voltMax, long& secsCalm) {
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {
    SET_ANEM_WIND_SPEED_HIGH((float)ANEM_WIND_SPEED_HIGH_DEF_VAL);
    SET_ANEM_WIND_SPEED_MAX((float)ANEM_WIND_SPEED_MAX_DEF_VAL);
    SET_SECONDS_TO_WAIT_WIND_CALM((float)SECONDS_TO_WAIT_WIND_CALM_DEF_VAL);
    EEPROM.write(INIT_ADDR, INIT_KEY);
  }
  float speedHigh, speedMax, secsFloat;
  GET_ANEM_WIND_SPEED_HIGH(speedHigh);
  GET_ANEM_WIND_SPEED_MAX(speedMax);
  GET_SECONDS_TO_WAIT_WIND_CALM(secsFloat);
  voltHigh = speedHigh * VOLT_TO_METER_PER_SEC;
  voltMax  = speedMax  * VOLT_TO_METER_PER_SEC;
  secsCalm = (long)secsFloat;
}

void initRtc() {
  if (!rtc.begin()) {
    lcdService->printWithDelay(F("ERROR! Cannot find RTC"), 3000);
    trackerSystemService->goToProtectMode();
    lcdService->printWithDelay(F("ERROR! Cannot find RTC"), 3000);
    abort();
  }

  if (rtc.lostPower()) {
    lcdService->printWithDelay(F("ERROR! RTC lost power"), 3000);
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //rtc.adjust(DateTime(2026, 4, 1, 23, 0, 0));
    trackerSystemService->goToProtectMode();
    //lcdService->printWithDelay(F("ERROR! RTC lost power"), 3000);
    irRemoteConfig->setRtcTime();
  }
}

void initSolarPosition() {
  // set the Time service as the time provider
  SolarPosition::setTimeProvider(&getUnixtimeFromRtc);
}

// returns result in SECONDS; watchdog guards against RTC hang
unsigned long getUnixtimeFromRtc() {
  wdt_enable(WDTO_2S);
  unsigned long result = rtc.now().unixtime();
  wdt_disable();
  return result;
}

void loop() {
  sleepIfNight();

  trackerSystemService->checkGridOutageAndGoToProtectModeIfNeeded();
  if (!trackerSystemService->isInGridProtectMode()) { // do not care about wind if grid is out
    unsigned long nowUnix = getUnixtimeFromRtc();
    trackerSystemService->checkWindSpeedAndGoToProtectModeIfNeeded(nowUnix);
  }

  printSystemState();
  delay(250);
}

//start sleeping despite possible protection modes are active. Turns off protection modes after waking up
void sleepIfNight() {
  double sunElevation = sun.getSolarElevation();
  if (sunElevation <= SUN_SLEEP_ELEVATION) {
    do {
      lcdService->printSleeping(MINUTES_SUN_POSITION_INTERVAL_CHECK);
      sleepMinutesPowerSave(MINUTES_SUN_POSITION_INTERVAL_CHECK); //3 mins to sleep
      sunElevation = sun.getSolarElevation();
    } while (sunElevation <= SUN_SLEEP_ELEVATION);
    trackerSystemService->goToNormalMode();
    trackerSystemService->resetAfterSleep(); // grid/wind state unknown after sleep — force re-evaluation
  }
}

void sleepMinutesPowerSave(int minutesCount) {
  int cycle = (int)(minutesCount * 60 / 8.); //цикл в секундах по 8 секунд
  for (int i = 0; i < cycle; i++) {
    LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
         SPI_OFF, USART0_OFF, TWI_OFF);
  }
}

void printSystemState() {
  if (trackerSystemService->isInGridProtectMode()) {
    lcdService->printTrackerSystemGridProtectState();
  } else if (trackerSystemService->isInWindProtectMode()) {
    unsigned long nowUnix = getUnixtimeFromRtc();
    unsigned long lastTime = trackerSystemService->getLastTimeWindHighSpeed();
    lcdService->printTrackerSystemWindProtectState(trackerSystemService->getWindSpeedMs(), nowUnix - lastTime);
  } else {
    lcdService->printTrackerSystemNormalState(trackerSystemService->getWindSpeedMs(),
      sun.getSolarAzimuth(), sun.getSolarElevation(), rtc.now());
  }
}
