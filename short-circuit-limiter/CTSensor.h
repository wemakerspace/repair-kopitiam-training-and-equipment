#ifndef CTSensor_h
#define CTSensor_h

#define ADC_BITS    10
#define ADC_COUNTS  (1<<ADC_BITS)
#define ADC_REF_VOLTAGE 5000

class CTSensor
{
  public:
    CTSensor(int adcPinNum, double calibrationValue);

    void doIncrementalMeasurement();
    double getIrmsFromIncrementalMeasurement();
    
  private:
    int adcPinNum_;
    double calibrationValue_;
    
    double offsetI_ = ADC_COUNTS >> 1;

    //These variables are for incremental measurements
    double sqI_, sumI_, filteredI_;
    int incrementalMeasurements_ = 0;
   
};


#endif
