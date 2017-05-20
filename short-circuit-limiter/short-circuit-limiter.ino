#include <U8g2lib.h>
#include "CTSensor.h"

#define CURRENT_ENABLE_THRESHOLD 0.2
#define CURRENT_MCB_CUT 16

#define TEMP_MAX 85
#define TEMP_WARN 70
#define TEMP_WARNING_RATE 120000 //2 x 60 x 1000ms

#define PIN_LED 26
#define PIN_CT A0
#define PIN_TEMP A11
#define PIN_BUZZER 4

#define PIN_BUTTON_UP 25
#define PIN_BUTTON_ENTER 24
#define PIN_BUTTON_DOWN 23

#define PIN_RELAY_MCB 5
#define PIN_RELAY_LIMITER 7

#define BUZZER_BEEP_ON_TIME 20
#define BUZZER_BEEP_OFF_TIME 200
#define BUZZER_LONG_BEEP_TIME 2000

#define CT_CALIBRATION_VALUE 60.6  // (100A / 0.05A) / 33 ohms
#define INITIAL_SETTLE_TIME 20000 //20 seconds

#define DELAY_BEFORE_LIMITER_RELAY_ENABLE 1000

//Number of temperature reads to average over
#define TEMP_READS_SET 50

//The MCU can take 5500 samples/s.
//0.05s seconds will require 275 samples/set
#define NUM_CT_SAMPLES 275

//Delay between LCD Print to avoid slowing down data collection
#define INTERVAL_PRINT 500

#define BACKLIGHT_BLINK_RATE 200

typedef enum {
  STATE_MCB_TRIPPED, STATE_WINDOW_BEFORE, STATE_WINDOW_WITHIN, STATE_WINDOW_EXITED, STATE_TEMP_MAX
} STATE;

//This is to prepare to transition to the STATE_WINDOW_BEFORE at the start
STATE currentState = STATE_MCB_TRIPPED;
STATE nextState = STATE_WINDOW_BEFORE;

U8G2_UC1701_MINI12864_F_4W_SW_SPI u8g2(U8G2_R2, 21, 20, 19, 22);
CTSensor clamp(PIN_CT, CT_CALIBRATION_VALUE);

double mcbTrippedCurrent;

int beepsLeft = 0;

void setup() {

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_RELAY_LIMITER, OUTPUT);
  pinMode(PIN_RELAY_MCB, OUTPUT);
  pinMode(PIN_TEMP, INPUT);

  pinMode(PIN_BUTTON_UP, INPUT);
  pinMode(PIN_BUTTON_ENTER, INPUT);
  pinMode(PIN_BUTTON_DOWN, INPUT);

  changeDisplayBacklight(true);
  passFullCurrentThrough(false);
  changeMCBRelayState(false);

  u8g2.begin();

  unsigned long initialTime = millis();
  unsigned long timeElapsedSinceStart;

  while((timeElapsedSinceStart = (millis() - initialTime)) < INITIAL_SETTLE_TIME){
    getCurrentMeasurement();
    
    unsigned long timeLeft = INITIAL_SETTLE_TIME - timeElapsedSinceStart;
    unsigned int timeLeftSeconds = timeLeft / 1000;
  
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0,10, "Short Circuit Limiter\n");
    u8g2.drawStr(0,21, "Starting in: ");

    u8g2.setFont(u8g2_font_9x18_tr);
    char secondsBuffer[10];
    sprintf(secondsBuffer, "%ds", timeLeftSeconds);

    u8g2.drawStr(80, 22, secondsBuffer);
    u8g2.setFont(u8g2_font_5x8_tr);

    char valueBuff[10];
    char valueBuff2[10];
    char fullBuff[30];

    float temperature = getTemperature();
    dtostrf(temperature, 3, 0, valueBuff);
    sprintf(fullBuff,"Curr Temp: %sC", valueBuff);
    u8g2.drawStr(0,31, fullBuff);

    dtostrf(TEMP_WARN, 2, 0, valueBuff);
    dtostrf(TEMP_MAX, 2, 0, valueBuff2);
    sprintf(fullBuff,"Temp Warn/Max : %s/%s C", valueBuff, valueBuff2);
    u8g2.drawStr(0,39, fullBuff);

    dtostrf(CURRENT_ENABLE_THRESHOLD, 4, 2, valueBuff);
    sprintf(fullBuff,"Enable thres: %sA", valueBuff);
    u8g2.drawStr(0,47, fullBuff);  

    dtostrf(CURRENT_MCB_CUT, 4, 1, valueBuff);
    sprintf(fullBuff,"Trip thres: %sA", valueBuff);
    u8g2.drawStr(0,55, fullBuff);

    u8g2.drawStr(0,63, "Designer: Yeo Kheng Meng");

    u8g2.sendBuffer();
   
  }

  Serial.begin(115200);
  Serial.println("Setup complete");

  enterWindowBeforeMode(0);
}

double getCurrentMeasurement(){
  
  for(int sampleIndex = 0; sampleIndex < NUM_CT_SAMPLES; sampleIndex++){
    clamp.doIncrementalMeasurement();
  }

  double currentValue = clamp.getIrmsFromIncrementalMeasurement();
  return currentValue;
}



void loop() {

  double currentValue = getCurrentMeasurement();
  
  if(currentValue >= CURRENT_MCB_CUT){
    nextState = enterMCBTrippedMode(currentValue);
  }

  switch(nextState){
    case STATE_MCB_TRIPPED:
      nextState = enterMCBTrippedMode(currentValue);
      break;
    case STATE_WINDOW_BEFORE:
      nextState = enterWindowBeforeMode(currentValue);
      break;
    case STATE_WINDOW_WITHIN:
      nextState = enterWindowWithinMode(currentValue);
      break;
    case STATE_WINDOW_EXITED:
      nextState = enterWindowExitedMode(currentValue);
      break;
    case STATE_TEMP_MAX:
      nextState = enterTempMaxMode();
      break;
    default:
      Serial.println("Something is really wrong here, this mode should not exist!!!");
      break;
   }

  shortBeepRunner();
  displayToScreen(currentValue);


}

void displayToScreen(double currentValue){
  long long currentTime = millis();

  static unsigned long lastDisplayTime;

  if(currentState == STATE_MCB_TRIPPED){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_9x18_tr);
    u8g2.drawStr(0,10, "Overcurrent!!!");

    u8g2.drawStr(0,31, "Reset MCB >>>>>>>>");

    char valueBuff[10];
    char fullBuff[30];
    
    dtostrf(mcbTrippedCurrent, 4, 1, valueBuff);
    sprintf(fullBuff,"Tripped at: %sA", valueBuff);
    u8g2.drawStr(0,50, fullBuff);

    u8g2.setFont(u8g2_font_5x8_tr);
    dtostrf(CURRENT_MCB_CUT, 4, 1, valueBuff);
    sprintf(fullBuff,"Trip threshold: %sA", valueBuff);
    u8g2.drawStr(0,63, fullBuff);
    
    u8g2.sendBuffer();

  } else if(currentState == STATE_TEMP_MAX){ 
    if((currentTime - lastDisplayTime) >= INTERVAL_PRINT){

      lastDisplayTime = currentTime;
      float temperature = getTemperature();

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_9x18_tr);
      u8g2.drawStr(0,10, "Overheated!!!");
  
      char valueBuff[10];
      char fullBuff[30];
      
      u8g2.setFont(u8g2_font_7x13_tr);
      dtostrf(temperature, 3, 0, valueBuff);
      sprintf(fullBuff,"Curr Temp: %sC", valueBuff);
      u8g2.drawStr(0,30, fullBuff);

      u8g2.setFont(u8g2_font_6x10_tr);
      dtostrf(TEMP_MAX, 3, 0, valueBuff);
      sprintf(fullBuff,"Temp Max : %sC", valueBuff);
      u8g2.drawStr(0,50, fullBuff);

      dtostrf(TEMP_WARN, 3, 0, valueBuff);
      sprintf(fullBuff,"Temp Warn: %sC", valueBuff);
      u8g2.drawStr(0,60, fullBuff);  
      
      u8g2.sendBuffer();


      
    }
  
  } else {

    static double runningTotal = 0;
    static int samplesTaken = 0;

    runningTotal += currentValue;
    samplesTaken++;
    
    //We don't want to keep printing to screen as it is slow so we just do a running average
    if((currentTime - lastDisplayTime) >= INTERVAL_PRINT){  
      lastDisplayTime = currentTime;

      float temperature = getTemperature();
      double average = runningTotal / samplesTaken;
  
      runningTotal = 0;
      samplesTaken = 0;

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_7x13_tr);

      switch(currentState){
        case STATE_WINDOW_BEFORE:
          u8g2.drawStr(0,10, "Limiter: In Effect");
          break;
        case STATE_WINDOW_WITHIN:
          u8g2.drawStr(0,10, "Limiter:");
          u8g2.drawStr(0,20, "Ready to Bypass...");
          break;
        case STATE_WINDOW_EXITED:
          u8g2.drawStr(0,10, "Limiter: Bypassed");
          break;
        case STATE_MCB_TRIPPED:
          //Fallthrough 
        default:
          Serial.println("LCD Printing shouldn't come to this mode!");
          break;
       }

       bool tempWarning = temperatureWarningCheck(temperature);

       char valueBuff[10];
       char fullBuff[30];
        
       dtostrf(currentValue, 4, 2, valueBuff);
       sprintf(fullBuff,"Current  : %sA", valueBuff);
       u8g2.drawStr(0,33, fullBuff);

       static bool blinkTempOn = false;
       blinkTempOn = !blinkTempOn;

       //Start blinking when temperature is warning range
       if(!tempWarning || blinkTempOn){
          dtostrf(temperature, 3, 0, valueBuff);
          sprintf(fullBuff,"Curr Temp: %sC", valueBuff);
          u8g2.drawStr(0,47, fullBuff);
       }

       u8g2.setFont(u8g2_font_5x8_tr);
       dtostrf(CURRENT_ENABLE_THRESHOLD, 4, 2, valueBuff);
       sprintf(fullBuff,"Enable Current: %sA", valueBuff);
       u8g2.drawStr(0,63, fullBuff);

       u8g2.sendBuffer();


    }
  }
  
}

STATE enterMCBTrippedMode(double currentValue){

  static bool initialWarningTriggered = false;
  static unsigned long initialWarningStart = 0;

  unsigned long currentTime = millis();

  if(currentState != STATE_MCB_TRIPPED){
    changeMCBRelayState(false);
    passFullCurrentThrough(false);

    currentState = STATE_MCB_TRIPPED;
    mcbTrippedCurrent = currentValue;
    Serial.print("MCB tripped at: ");
    Serial.println(mcbTrippedCurrent);

    initialWarningTriggered = true;
    initialWarningStart = currentTime;
    digitalWrite(PIN_BUZZER, HIGH);
  }

  //This is to sound the buzzer and blink the Backlight for BACKLIGHT_BLINK_RATE duration 
  if(initialWarningTriggered){

    static bool displayBacklightCurrentlyOn = false;
    static unsigned long displayBacklightLastChanged = 0;

    if((currentTime - displayBacklightLastChanged) > BACKLIGHT_BLINK_RATE){
      displayBacklightLastChanged = currentTime;
      displayBacklightCurrentlyOn = !displayBacklightCurrentlyOn;
      changeDisplayBacklight(displayBacklightCurrentlyOn);
    }


    if((currentTime - initialWarningStart) > BUZZER_LONG_BEEP_TIME){
      initialWarningTriggered = false;
      digitalWrite(PIN_BUZZER, LOW);
      changeDisplayBacklight(true);
       
    }
    
  }

  if(isAtLeastOneButtonPressed()){
    return STATE_WINDOW_BEFORE;
  }

  return STATE_MCB_TRIPPED; 
}

STATE enterWindowBeforeMode(double currentValue){
  if(currentState != STATE_WINDOW_BEFORE){

    passFullCurrentThrough(false);
    changeMCBRelayState(true);
    changeDisplayBacklight(false);

    //Don't bother beeping if we came from within window. Happens when the current measurement is noisy
    if(currentState != STATE_WINDOW_WITHIN){
      shortBeepXTimesNoDelay(2);
    }
    
    currentState = STATE_WINDOW_BEFORE;
    Serial.println("Window Before");
  }

  if(isAtLeastOneButtonPressed()){
    changeDisplayBacklight(true);  
  }


  if(currentValue >= CURRENT_ENABLE_THRESHOLD){
    return STATE_WINDOW_WITHIN;
  }

  return STATE_WINDOW_BEFORE;
   
}

STATE enterWindowWithinMode(double currentValue){
  unsigned long currentTime = millis();

  static unsigned long enableWindowTime = 0;
  
  if(currentState != STATE_WINDOW_WITHIN){
    currentState = STATE_WINDOW_WITHIN;
    enableWindowTime = currentTime;
    Serial.println("Enter window");
  }

  if(currentValue < CURRENT_ENABLE_THRESHOLD){
    return STATE_WINDOW_BEFORE;
  } else if((currentTime - enableWindowTime) >= DELAY_BEFORE_LIMITER_RELAY_ENABLE){
    //Exit window if no overcurrent is detected in this time
    return STATE_WINDOW_EXITED;
  } else {
    return STATE_WINDOW_WITHIN;
  } 
}

STATE enterWindowExitedMode(double currentValue){

  if(currentState != STATE_WINDOW_EXITED){
    currentState = STATE_WINDOW_EXITED;
    changeDisplayBacklight(true);
    shortBeepXTimesNoDelay(1);
    Serial.println("Exited window, bypass resistor");
  }

  passFullCurrentThrough(true);

  if(currentValue < CURRENT_ENABLE_THRESHOLD){
    return STATE_WINDOW_BEFORE;
  } else {
    return STATE_WINDOW_EXITED;
  } 

}

STATE enterTempMaxMode(){
  if(currentState != STATE_TEMP_MAX){
    currentState = STATE_TEMP_MAX;
    
    changeMCBRelayState(false);
    passFullCurrentThrough(false);
    
    changeDisplayBacklight(true);
    shortBeepXTimesNoDelay(10);
    Serial.println("Enter Max Temp mode");
  }

  float temperature = getTemperature();

  //We remain in this mode until the temperature drops below TEMP_WARN to prevent oscillating into this mode and out
  if(temperature >= TEMP_WARN){
    return STATE_TEMP_MAX; 
  } else {
    return STATE_WINDOW_BEFORE;  
  }
}

void changeMCBRelayState(bool state){
  if(state){
    digitalWrite(PIN_RELAY_MCB, HIGH);
  } else {
    digitalWrite(PIN_RELAY_MCB, LOW);
  }
}

void passFullCurrentThrough(bool state){
  if(state){
    digitalWrite(PIN_RELAY_LIMITER, HIGH);
  } else {
    digitalWrite(PIN_RELAY_LIMITER, LOW);
  }
}

void changeDisplayBacklight(bool state){

  //State is flipped for some reason
  if(state){
    digitalWrite(PIN_LED, LOW);
  } else {
    digitalWrite(PIN_LED, HIGH);
  }
}

//Obtained from https://learn.adafruit.com/tmp36-temperature-sensor/using-a-temp-sensor
float getTemperature(){

  long total = 0; 
  
  //First value is usually off
  analogRead(PIN_TEMP);
  
  for(int i = 0; i < TEMP_READS_SET; i++){
    total += analogRead(PIN_TEMP);
  }
  
  int reading = total / TEMP_READS_SET; 
  
  // converting that reading to voltage, for 3.3v arduino use 3.3
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  
  // now print out the temperature
  float temperatureC = (voltage - 0.5) * 100 ;
  
  return temperatureC;
}

bool isAtLeastOneButtonPressed(){

  int upPressed = digitalRead(PIN_BUTTON_UP);
  int enterPressed = digitalRead(PIN_BUTTON_ENTER);
  int downPressed = digitalRead(PIN_BUTTON_DOWN);

  if(upPressed == LOW || enterPressed == LOW || downPressed == LOW){
    return true;
  } else {
    return false;
  }
}

void shortBeepXTimesNoDelay(int times){
  beepsLeft = times;  
}

//We don't use delay from beeps to avoid holding up the controller
void shortBeepRunner(){

  static bool beepCurrentlyOn = false;
  static unsigned long beepChangedTime = 0;

  unsigned long currentTime = millis();

  if(beepsLeft > 0){

    if(beepCurrentlyOn){

      if((currentTime - beepChangedTime) > BUZZER_BEEP_ON_TIME){
          beepChangedTime = currentTime;
          digitalWrite(PIN_BUZZER, LOW);
          beepCurrentlyOn = false;
          beepsLeft--;
      }
      
    } else {

      if((currentTime - beepChangedTime) > BUZZER_BEEP_OFF_TIME){
          beepChangedTime = currentTime;
          digitalWrite(PIN_BUZZER, HIGH);
          beepCurrentlyOn = true;
      }
    }
  }
  
}

//Returns true if temperature threshold has been reached
bool temperatureWarningCheck(float temperature){

  if(temperature >= TEMP_MAX){
    nextState = STATE_TEMP_MAX;
    return true;
  
  } else if(temperature >= TEMP_WARN){
    static unsigned long lastTempAlert = 0;

    unsigned long currentTime = millis();

    if((currentTime - lastTempAlert) > TEMP_WARNING_RATE){
      lastTempAlert = currentTime;
      shortBeepXTimesNoDelay(5); 
    }

    return true;
    
  }

  return false;
}

