#include "CTSensor.h"
#include <Arduino.h>

CTSensor::CTSensor(int adcPinNum, double calibrationValue) {
    adcPinNum_ = adcPinNum;
    calibrationValue_ = calibrationValue;
    offsetI_ = ADC_COUNTS >> 1;

    pinMode(adcPinNum, INPUT);
}


void CTSensor::doIncrementalMeasurement(){
  int sampleI = analogRead(adcPinNum_);
    
  // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
  //  then subtract this - signal is now centered on 0 counts.
  offsetI_ = (offsetI_ + (sampleI - offsetI_) / 1024);
  filteredI_ = sampleI - offsetI_;

  // Root-mean-square method current
  // 1) square current values
  sqI_ = filteredI_ * filteredI_;
  // 2) sum
  sumI_ += sqI_;

  incrementalMeasurements_ += 1;
}


double CTSensor::getIrmsFromIncrementalMeasurement(){
  double I_RATIO = calibrationValue_ * ((ADC_REF_VOLTAGE / 1000.0) / (ADC_COUNTS));
  double currentIrms = I_RATIO * sqrt(sumI_ / incrementalMeasurements_);

  sumI_ = 0;
  incrementalMeasurements_ = 0;
 
  return currentIrms;
}
