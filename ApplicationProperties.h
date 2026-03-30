#ifndef ApplicationProperties_h
#define ApplicationProperties_h

// Hardware pins
#define ANEMOMETR_SIGNAL_PIN    A7  //сигнал от анемометра
#define GRID_STATE_SIGNAL_PIN   A3  //сигнал от датчика наличия сети
#define POWER_CONTROLE_PIN_1    A1  //контрольный пин питания трекерной розетки
#define POWER_CONTROLE_PIN_2    A2  //контрольный пин питания трекерной розетки

// Display - 20x4 LCD, I2C
#define DISPLAY_COLS      20
#define DISPLAY_ROWS      4
#define DISPLAY_I2C_ADDR  0x3f

// Location - Petropavlivka, Ukraine
#define LATITUDE   48.454642
#define LONGITUDE  36.419804

// Anemometer calibration
#define TO_ANEM_VOLTAGE       (5.0 / 1023.0)           //analogRead() returns value from 0 to 1023
#define VOLT_TO_METER_PER_SEC (5.0 / 30.0)             //0.166666666666..
#define ANEM_VOLTAGE_HIGH     (VOLT_TO_METER_PER_SEC * 5)  //high wind speed 5 m/s
#define ANEM_VOLTAGE_MAX      (VOLT_TO_METER_PER_SEC * 6)  //high wind speed 6 m/s

// Timing / behaviour
#define SECONDS_TO_WAIT_WIND_CALM           150L      // 10 mins; //300 secs 5 mins
#define MILLIS_TRACKER_TO_PROTECT_MODE      300000L   //300 secs 5 mins
#define MINUTES_SUN_POSITION_INTERVAL_CHECK 3         //180 secs 3 mins
#define SUN_SLEEP_ELEVATION                 -1.0

#endif
