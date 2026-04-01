#ifndef ApplicationProperties_h
#define ApplicationProperties_h

#include <EEPROM.h>

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

// Timing / behaviour
#define MILLIS_TRACKER_TO_PROTECT_MODE      300000L   //300 secs 5 mins
#define MINUTES_SUN_POSITION_INTERVAL_CHECK 3         //180 secs 3 mins
#define SUN_SLEEP_ELEVATION                 -1.0

// IR remote
#define IR_RECEIVE_PIN  2 // Digital pin. Not intercepted with POWER_CONTROLE_PIN_2 A2.

#define BTN_1           0x45
#define BTN_2           0x46
#define BTN_3           0x47
#define BTN_4           0x44
#define BTN_5           0x40
#define BTN_6           0x43
#define BTN_7           0x7
#define BTN_8           0x15
#define BTN_9           0x9
#define BTN_0           0x19
#define BTN_HASH        0xD
#define BTN_ASTERIX     0x16
#define BTN_OK          0x1C

#define MENU_ASK_TO_ENTER_TIMEOUT_SEC  10

// EEPROM configuration
#define INIT_ADDR  1023  // reserved cell — holds first-run key
#define INIT_KEY   49    // first-run key (0-254, unsigned char)

#define ANEM_WIND_SPEED_HIGH_ADDRESS        0  // 4 bytes (float, m/s)
#define ANEM_WIND_SPEED_MAX_ADDRESS         4  // 4 bytes (float, m/s)
#define SECONDS_TO_WAIT_WIND_CALM_ADDRESS   8  // 4 bytes (float, cast to long at runtime)

#define ANEM_WIND_SPEED_HIGH_DEF_VAL        4.5    // m/s — wind speed to start tracking last high time
#define ANEM_WIND_SPEED_MAX_DEF_VAL         5.0    // m/s — wind speed to trigger protection mode
#define SECONDS_TO_WAIT_WIND_CALM_DEF_VAL   300.0  // secs — calm duration before returning to normal

#define GET_ANEM_WIND_SPEED_HIGH(x)          EEPROM.get(ANEM_WIND_SPEED_HIGH_ADDRESS, x)
#define GET_ANEM_WIND_SPEED_MAX(x)           EEPROM.get(ANEM_WIND_SPEED_MAX_ADDRESS, x)
#define GET_SECONDS_TO_WAIT_WIND_CALM(x)     EEPROM.get(SECONDS_TO_WAIT_WIND_CALM_ADDRESS, x)

#define SET_ANEM_WIND_SPEED_HIGH(x)          EEPROM.put(ANEM_WIND_SPEED_HIGH_ADDRESS, x)
#define SET_ANEM_WIND_SPEED_MAX(x)           EEPROM.put(ANEM_WIND_SPEED_MAX_ADDRESS, x)
#define SET_SECONDS_TO_WAIT_WIND_CALM(x)     EEPROM.put(SECONDS_TO_WAIT_WIND_CALM_ADDRESS, x)

#endif
