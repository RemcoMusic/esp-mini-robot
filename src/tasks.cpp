#include <Arduino.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include "RemoteDebug.h" 
#include "OTA.h"
#include "motor.h"
#include "camera.h"
#include "distanceSensor.h"
#include "tasks.h"
#include "IR.h"

RemoteDebug Debug;
OTA OTAUpdate;
Motor motor;
DistanceSensor dSensor;
Camera openMV;
IR IRreciever;

TwoWire I2CdistanceSensor = TwoWire(0);

unsigned long oldIRinput = 0;

bool objectDetected = false;
bool motorDriverEnabled = false;

void processCmdRemoteDebug() 
{

	String lastCmd = Debug.getLastCommand();

	if (lastCmd == "ota") {
      debugI("Over the Air updating has been started");
      OTAUpdate.startOTA();
  }

  if (lastCmd == "start") {
      debugI("Started searching for target"); 
      motor.enable();
  }
  if (lastCmd == "stop") {
      debugI("Stop searching for target");
      motor.disable();    
      motor.Stop();
  }  
}

Tasks::Tasks()
{
  xTaskCreate(
    remoteDebugger,
    "Debugger",
    10000,
    NULL,
    1,
    NULL);
}

void Tasks::readDistanceSensor(void * parameter)
{
  I2CdistanceSensor.begin(27,32,100000);
  dSensor.beginSetup();
  for(;;)
  {
    if(dSensor.readDistanceSensor() < 140)
    {
      objectDetected = true;
      debugE("Object Detected");
    }
    else {
      objectDetected = false;
    }
    delay(50);
  }
}

void Tasks::cameraInput(void * parameter) 
{
  openMV.beginSetup();
  for(;;) {
    openMV.readCamera();
  }
}

void Tasks::motorDriver(void * parameter) 
{
  for(;;) 
    {
      motor.directMotors(openMV.getObjectLocation(), objectDetected);  
    }
}

void Tasks::IRdecoder(void * parameter)
{
  IRreciever.start();
  delay(500);
  for(;;)
  {
    unsigned long newIRinput = IRreciever.Recieve();

    if(newIRinput != oldIRinput)
    {
      oldIRinput = newIRinput;
      if(newIRinput != 1073482616)
      {
        Serial.println(newIRinput);

        //functions
      }
      IRreciever.Next();
    }
    else
    {
      delay(500);
    }
  }
}

void Tasks::remoteDebugger(void * parameter) 
{
    Debug.begin("AstontronDebug");  // Initiaze the telnet server
    Debug.setResetCmdEnabled(true); // Enable the reset command
	  Debug.showProfiler(false);  // Profiler measures times
	  Debug.showColors(true); // Colors inthe console
    Debug.setCallBackProjectCmds(&processCmdRemoteDebug);

    xTaskCreate(
      readDistanceSensor,
      "Read_Distance_Sensor",
      10000,
      NULL,
      1,
      NULL);

    xTaskCreate(
      cameraInput,
      "Camera_Input",
      10000,
      NULL,
      1,
      NULL);
    
    xTaskCreate(
      motorDriver,
      "Motor",
      10000,
      NULL,
      1,
      NULL);

    xTaskCreate(
      IRdecoder,
      "IRdecoder",
      10000,
      NULL,
      1,
      NULL);

    for(;;)
    {
      Debug.handle();   
    }
}