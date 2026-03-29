 #include <RTClib.h>
#include "SolarPosition.h"
#include "LiquidCrystal_I2C.h"
#include "LowPower.h"
#include <avr/wdt.h>

RTC_DS3231 rtc;
SolarPosition sun (48.454642, 36.419804);  // Petropavlovka, Ukraine
const int DISPLAY_SYMBOLS = 20;
LiquidCrystal_I2C lcd (0x3f, DISPLAY_SYMBOLS, 4); //20 4 lcd display

const long SECONDS_TO_WAIT_WIND_CALM = 150L;// 10 mins; //300 secs 5 mins
const long MILLIS_TRACKER_TO_PROTECT_MODE = 300000L;//300000L; //300 secs 5 mins
const int MINUTES_SUN_POSITION_INTERVAL_CHECK = 3;//180 secs 3 mins
const double SUN_SLEEP_ELEVATION = -1.0;

const int ANEMOMETR_SIGNAL_PIN = A7; //сигнал от анемометра
const int GRID_STATE_SIGNAL_PIN = A3; //сигнал от датчика наличия сети
const int POWER_CONTROLE_PIN_1 = A1; //контрольный пин питания трекерной розетки
const int POWER_CONTROLE_PIN_2 = A2; //контрольный пин питания трекерной розетки

const double TO_ANEM_VOLTAGE = 5. / 1023.; //analogRead() returns value from 0 to 1023

const float VOLT_TO_METER_PER_SEC = 5./30.; //0.166666666666..
const float ANEM_VOLTAGE_HIGH = VOLT_TO_METER_PER_SEC * 5; //high wind speed 5 m/s
const float ANEM_VOLTAGE_MAX = VOLT_TO_METER_PER_SEC * 6; //high wind speed 6 m/s

//used to store current protection state
bool isGridProtectModeOn = false;
bool isWindProtectModeOn = false;

unsigned long lastTimeWindHighSpeed = 0; // last tine wind speed was higher than 6 m/s
double anemVoltage = 0.; //anemometr voltage

void setup() {
  //delay(5000); //to read while update
  Serial.begin(9600);

  pinMode(ANEMOMETR_SIGNAL_PIN, INPUT);
  pinMode(GRID_STATE_SIGNAL_PIN, INPUT);
  
  pinMode(POWER_CONTROLE_PIN_1, OUTPUT);
  pinMode(POWER_CONTROLE_PIN_2, OUTPUT);
  turnTrackersOn();
  initLcd();
  initRtc();
  initSolarPosition();
}

void initLcd() {
  lcd.init();                      // Инициализация дисплея
  lcd.backlight();
  lcdPrint("Starting wind monitoring...");
}

void initRtc() {
  if (!rtc.begin()) {
    lcdPrint("ERROR! Cannot find RTC");
    goToProtectMode();
    lcdPrint("ERROR! Cannot find RTC");
    abort();
  }

  if (rtc.lostPower()) {
    lcdPrint("ERROR! RTC lost power");
    goToProtectMode();
    lcdPrint("ERROR! RTC lost power");
    abort();
  }
}

void initSolarPosition(){
  // set the Time service as the time provider
  SolarPosition::setTimeProvider(&getUnixtimeFromRtc);
}

//returns result in SECONDS
unsigned long getUnixtimeFromRtc() {
  wdt_enable(WDTO_2S);
  unsigned long result = rtc.now().unixtime();
  wdt_disable();
  return result;
}

void loop() {
  //Serial.println("here again");
  sleepIfNight();
  gridOutageProtection();
  if(!isGridProtectModeOn){ // do not care about wind if grid is out
    windSpeedProtection();
  }
  lcdPrintMonitoring();
  delay(250);
}

//start sleeping despite possible protection modes are active. Turns off protection modes after waking up
void sleepIfNight(){
  double sunElevation = sun.getSolarElevation(); 
  //if(sunElevation <= SUN_SLEEP_ELEVATION){
  if(sunElevation <= SUN_SLEEP_ELEVATION){
    bool isSleeping = true;
    //sleeping loop
    do{
      lcdPrintMonitoring(isSleeping);
      sleepMinutesPowerSave(MINUTES_SUN_POSITION_INTERVAL_CHECK); //3 mins to sleep
      sunElevation = sun.getSolarElevation();
    } while(sunElevation <= SUN_SLEEP_ELEVATION);
    goToNormalMode();
    isGridProtectModeOn = true;
    isWindProtectModeOn = true;        
  }
}

void sleepMinutesPowerSave(int minutesCount){
  int cycle = (int)(minutesCount*60/8.); //цикл в секундах по 8 секунд
  for(int i=0; i<cycle; i ++){
    LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
         SPI_OFF, USART0_OFF, TWI_OFF);
  }
}

void gridOutageProtection(){
  //Serial.println("gridOutageProtection");

  int gridStateCode = digitalRead(GRID_STATE_SIGNAL_PIN);
  if(gridStateCode == HIGH && !isGridProtectModeOn){ //grid is out
    goToProtectMode();
    isGridProtectModeOn = true;
  } 
  else if(gridStateCode == LOW && isGridProtectModeOn){
    goToNormalMode();
    isGridProtectModeOn = false;
  }
  //Serial.println("gridStateCode");
  //Serial.println(gridStateCode);
  //Serial.println("isGridProtectModeOn");
  //Serial.println(isGridProtectModeOn);  
    
}

void windSpeedProtection(){
  anemVoltage = analogRead(ANEMOMETR_SIGNAL_PIN) * TO_ANEM_VOLTAGE;  // read the input pin
  Serial.print("Anemometr anemVoltage: ");
  Serial.println(anemVoltage);

  if(anemVoltage >= ANEM_VOLTAGE_HIGH) {
    lastTimeWindHighSpeed = getUnixtimeFromRtc();
  }
  
  if(anemVoltage >= ANEM_VOLTAGE_MAX && !isWindProtectModeOn ) {
    goToProtectMode();
    isWindProtectModeOn = true;
    lastTimeWindHighSpeed = getUnixtimeFromRtc(); // to start after protection mode is applied
  } else if (isWindProtectModeOn && anemVoltage <= ANEM_VOLTAGE_HIGH && checkLastTimeHighSpeed()) {
    goToNormalMode();
    isWindProtectModeOn = false;
  }
}

void goToProtectMode() {
  Serial.println("Go to protect mode");
  lcdPrintNoDelay("Go to protect mode");
  turnTrackersOff();
  delay(10000); //10 sec wait till power blocks for trackers turn off completelly

  turnTrackersOn();
  Serial.println("Wait trackers to get a protect position");
  lcdPrintNoDelay("Wait trackers to get a protect position");
  delay(MILLIS_TRACKER_TO_PROTECT_MODE); //300 sec //5 mins to get the waiting position

  turnTrackersOff();
  Serial.print("Waiting for the better weather");
  lcdPrint("Waiting for the better weather");
}

void goToNormalMode(){
  turnTrackersOn();
  Serial.println("Go to normal mode");
  lcdPrint ("Go to normal mode");  
}

void turnTrackersOn(){
  digitalWritePowerControlPins(HIGH);
}

void turnTrackersOff(){
  digitalWritePowerControlPins(LOW);
}

void digitalWritePowerControlPins(int value) {
  digitalWrite(POWER_CONTROLE_PIN_1, value); //tracker are on
  digitalWrite(POWER_CONTROLE_PIN_2, value); //tracker are on
}

bool checkLastTimeHighSpeed() {
  if (getWindHighSpeedDuration() > SECONDS_TO_WAIT_WIND_CALM) { //make sure wind has been normal at least for SECONDS_TO_WAIT_WIND_CALM secs
    return true;
  }
  return false;
}

long getWindHighSpeedDuration(){
  return getUnixtimeFromRtc() - lastTimeWindHighSpeed;
}

void lcdPrintNoDelay(String message) {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(3000);
}

void lcdPrint(String message) {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void lcdPrintMonitoring(){
  lcdPrintMonitoring(false);
}

void lcdPrintMonitoring(bool isSleeping){
  lcd.clear();
  lcd.setCursor(0, 0);
  if(isGridProtectModeOn){
    lcd.print("Grid protection mode");
  } else{
    if(!isSleeping){
      lcd.print("Wind speed: ");
      lcd.print(anemVoltage / VOLT_TO_METER_PER_SEC);
      lcd.print(" m/s ");
      lcd.setCursor(0, 1);
      lcd.print("Anem signal: ");
      lcd.print(anemVoltage);
      lcd.print(" v");
    } else{ //we do not read anemometr value while sleep
      lcd.print("Waiting sunrise for ");
      lcd.setCursor(0, 1);
      lcd.print(MINUTES_SUN_POSITION_INTERVAL_CHECK);
      lcd.print(" min");
    }
    if(isWindProtectModeOn){
      lcd.setCursor(0, 1);
      lcd.print("Wind protection mode");
      lcd.setCursor(0, 2);
      lcd.print("Time (sec):");
      lcd.setCursor(0, 3);
      lcd.print(getWindHighSpeedDuration());
    }else{
      lcd.setCursor(0, 2);
      lcd.print("az: ");
      lcd.print(sun.getSolarAzimuth());
      lcd.print(" e: ");
      lcd.print(sun.getSolarElevation());
      
      DateTime now = rtc.now();
      lcd.setCursor(0, 3);
      //lcd.print("time: ");
      char bufTime[] = "hh:mm:ss";
      lcd.print(now.toString(bufTime));
      
      //lcd.setCursor(0, 2);
      //lcd.print("date: ");
      lcd.print("  ");
      char bufDate[] = "DD/MM/YY";
      lcd.print(now.toString(bufDate));        
    }
  }
}
