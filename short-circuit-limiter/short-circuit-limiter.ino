#include <U8g2lib.h>
#include "CTSensor.h"

#define CURRENT_ENABLE_THRESHOLD 0.2
#define CURRENT_MCB_CUT 20

#define PIN_LED 26
#define PIN_ANALOG A0
#define PIN_BUTTON_ENTER 24

#define PIN_RELAY_MCB 5
#define PIN_RELAY_LIMITER 7

#define CT_CALIBRATION_VALUE 60.6  // (100A / 0.05A) / 33 ohms
#define INITIAL_SETTLE_TIME 15000 //15 seconds

#define DELAY_BEFORE_LIMITER_RELAY_ENABLE 1000

//The MCU can take 5500 samples/s.
//0.05s seconds will require 275 samples/set
#define NUM_CT_SAMPLES 275
#define INTERVAL_PRINT 500

typedef enum {
  STATE_MCB_TRIPPED, STATE_WINDOW_BEFORE, STATE_WINDOW_WITHIN, STATE_WINDOW_EXITED
} STATE;

//This is to prepare to transition to the STATE_WINDOW_BEFORE at the start
STATE currentState = STATE_MCB_TRIPPED;
STATE nextState = STATE_WINDOW_BEFORE;

U8G2_UC1701_MINI12864_F_4W_SW_SPI u8g2(U8G2_R2, 21, 20, 19, 22);
CTSensor clamp(PIN_ANALOG, CT_CALIBRATION_VALUE);

unsigned long lastDisplayTime = 0;
double runningTotal = 0;
int samplesTaken = 0;

unsigned long enableWindowTime = 0;


double mcbTrippedCurrent;

void setup() {

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAY_LIMITER, OUTPUT);
  pinMode(PIN_RELAY_MCB, OUTPUT);
  pinMode(PIN_BUTTON_ENTER, INPUT);

  changeDisplayBacklight(true);
  passFullCurrentThrough(false);
  changeMCBRelayState(false);

  u8g2.begin();

  unsigned long initialTime = millis();
  unsigned long timeElapsedSinceStart;

  while((timeElapsedSinceStart = (millis() - initialTime)) < INITIAL_SETTLE_TIME){
    clamp.doIncrementalMeasurement();
    unsigned long timeLeft = INITIAL_SETTLE_TIME - timeElapsedSinceStart;
    unsigned int timeLeftSeconds = timeLeft / 1000;
  
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0,10, "Short Circuit Limiter\n");
    u8g2.drawStr(0,35, "Starting in: ");

    u8g2.setFont(u8g2_font_9x18_tr);
    char secondsBuffer[10];
    sprintf(secondsBuffer, "%ds", timeLeftSeconds);
    
    u8g2.drawStr(80, 35, secondsBuffer);
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(0,60, "Designer: Yeo Kheng Meng");

    u8g2.sendBuffer();
   
  }

  //To clear the accmulated reading
  clamp.getIrmsFromIncrementalMeasurement();
  
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
    default:
      Serial.println("Something is really wrong here, this mode should not exist!!!");
      break;
   }


//  long long currentTime = millis();
//
//  runningTotal += currentValue;
//  samplesTaken++;
//
//  //We don't want to keep printing to screen as it is slow so we just do a running average
//  if((currentTime - lastDisplayTime) >= INTERVAL_PRINT){
//
//    lastDisplayTime = currentTime;
//    double average = runningTotal / samplesTaken;
//
//    runningTotal = 0;
//    samplesTaken = 0;
//
//    lcd.setCursor(0, 0);
//    lcd.print(currentValue);
//  }


}

STATE enterMCBTrippedMode(double currentValue){

  if(currentState != STATE_MCB_TRIPPED){
    changeMCBRelayState(false);
    passFullCurrentThrough(false);

    currentState = STATE_MCB_TRIPPED;
    mcbTrippedCurrent = currentValue;
    Serial.println("MCB tripped");
  }

  int buttonPressed = digitalRead(PIN_BUTTON_ENTER);

  if(buttonPressed == LOW){
    return STATE_WINDOW_BEFORE;
  }

  return STATE_MCB_TRIPPED; 
}

STATE enterWindowBeforeMode(double currentValue){
  if(currentState != STATE_WINDOW_BEFORE){

    passFullCurrentThrough(false);
    changeMCBRelayState(false);

    //In order to turn off the Bypass SSR, we have cut power to the entire setup first to prevent residual current from holding the Bypass SSR open
    delay(50);
    changeMCBRelayState(true);

    currentState = STATE_WINDOW_BEFORE;
    Serial.println("Window Before");
  }


  if(currentValue >= CURRENT_ENABLE_THRESHOLD){
    return STATE_WINDOW_WITHIN;
  }

  return STATE_WINDOW_BEFORE;
   
}

STATE enterWindowWithinMode(double currentValue){
  unsigned long currentTime = millis();
  
  if(currentState != STATE_WINDOW_WITHIN){
    currentState = STATE_WINDOW_WITHIN;
    enableWindowTime = currentTime;
    Serial.println("Enter window");
  }

  if(currentValue < CURRENT_ENABLE_THRESHOLD){
    return STATE_WINDOW_BEFORE;
  } else if((currentTime - enableWindowTime) >= DELAY_BEFORE_LIMITER_RELAY_ENABLE){
    return STATE_WINDOW_EXITED;
  } else {
    return STATE_WINDOW_WITHIN;
  } 
}

STATE enterWindowExitedMode(double currentValue){

  if(currentState != STATE_WINDOW_EXITED){
    currentState = STATE_WINDOW_EXITED;
    Serial.println("Exited window, bypass resistor");
  }

  passFullCurrentThrough(true);

  if(currentValue < CURRENT_ENABLE_THRESHOLD){
    return STATE_WINDOW_BEFORE;
  } else {
    return STATE_WINDOW_EXITED;
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
