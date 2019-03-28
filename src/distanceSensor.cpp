#include <Arduino.h>
#include "distanceSensor.h"


void DistanceSensor::beginSetup()
{
    distanceSensor.setSignalRateLimit(SignalRate);
    distanceSensor.setTimeout(500);
    distanceSensor.setMeasurementTimingBudget(200000);
    distanceSensor.init();
}

int DistanceSensor::readDistanceSensor()
{
    int CurrentDistance = distanceSensor.readRangeSingleMillimeters(); 
  
    if(CurrentDistance <= 1000)
    {
       return CurrentDistance;
    }
}

int DistanceSensor::detectObjects()
{
    int correctedDistance = readDistanceSensor();

    if(correctedDistance != 0)
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}