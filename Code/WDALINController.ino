/*********************
Ford / Bosch LIN Wiper Motor Controller.  Defines LIN packet to send over LIN using wiper stalk inputs (define if ground or 12v input to activate(!))
Designed & Built for Gavin Bey (@beytek_).  Requested for Caddy wipers by @xjamiexoe

** Uses LIN 2.x specification (and therefore checksum) Assumes Ford / Volvo Wiper Motor ** 
  if (!((this->version == LIN_V1) || (pid == 0x3C) || (pid == 0x7D)))    // if version 2  & no diagnostic frames (0x3C=60 (PID=0x3C) or 0x3D=61 (PID=0x7D))
    //chk = (uint16_t) pid;
    >>> NOTE slashes above, chk isn't calculated correctly - this is coded out in the Lib-"LIN_master_portable_Arduino-main"(!) 

** Ford / Volvo Wiper details are included - note the above change in the checksum routine(!) - LIN 2.0
** Bosch 'Motorsport' Wiper details are included ** - LIN 2.0
** VW Wiper details are included - from 2016 Caddy ** - LIN 1.0
** See '_LIN.ino' for more specifics

** Use 'linType' in _defs.h to change to the correct LIN parameters for your wiper motor **

** Assumes power (KL15) / Ignition Live means this module is powered and therefore the wipers are, so force active.  Possible revision to leave on KL30 and monitor ign. signal? **
  // LIN2.x uses extended checksum which includes protected ID, i.e. including parity bits
  // LIN1.x uses classical checksum only over data bytes
  // Diagnostic frames with ID 0x3C and 0x3D/0x7D always use classical checksum (see LIN spec "2.3.1.5 Checkum")

Buttons:
A LONG hold then release on the interval button will DECREASE the interval speed.  These buttons (on the default PCB) are pulled up by the board and pulled to ground by your stalk.  DO NOT supply ANY voltage to these pins(!)
Note; it's easier to use a defined button lib. than to deal with debouncing etc...

Forbes-Automotive.com; 2024
**********************/

#include "WDALINController_defs.h"
#include "LIN_master_HardwareSerial_ESP32.h"  // github.com/gicking/LIN_master_portable_Arduino
#include <ButtonLib.h>                        // github.com/JaredTCan/ButtonLib
#include <Preferences.h>                      // ESP32 lib; stores settings - mainly for last interval speed

// Setup 'button' library for inputs - 'true' is internal pullup
#if triggerType == 0
// if negative trigger (i.e. input pulled down, use the negative input pins)
buttonClass btnInt(pinIntNeg, true);
buttonClass btnPos1(pinPos1Neg, true);
buttonClass btnPos2(pinPos2Neg, true);
#endif

#if triggerType == 1
// if positive trigger (i.e. input pulled to 12v, use the positive input pins)
buttonClass btnInt(pinIntPos, false);
buttonClass btnPos1(pinPos1Pos, false);
buttonClass btnPos2(pinPos2Pos, false);
#endif

// Setup LIN library for Serial2, define pins
LIN_Master_HardwareSerial_ESP32 LIN(Serial2, pinRX, pinTX, "LIN_HW");  // parameters: interface, Rx, Tx, name

// EEPROM for remembering last interval speed
Preferences preferences;

void setup() {
  basicInit();  // basic init in _io.  Keeps this page clean because of all of the 'serial_debugs'
}

void loop() {
  btnInt.tick();   // using the 'button' library to detect held or single clicks
  btnPos1.tick();  // using the 'button' library to detect held or single clicks
  btnPos2.tick();  // using the 'button' library to detect held or single clicks

#if ledIndicator
  LED();  // light the PCB LEDs as per the button press - handy for knowing that it's actually seeing inputs.  Can disable if comfortable IO is okay.  In '_io.ino'
#endif

  LIN.handler();  // tick over the LIN handler

  if (intervalSpeed != lastIntervalSpeed) {  // check to see if the interval has been increased (double-click on 'int')
    lastIntervalSpeed = intervalSpeed;
    blinkLED(blinkInterval, intervalSpeed, false, true);  // this can cause a 'false' frame to go through because of the delay, consider not having it blink...

#if serialDebug
    Serial.println(intervalSpeed);
#endif
  }

  if ((millis() - lastMillis) > linPause) {  // check to see if x ms (linPause) has elapsed - slow down the frames!  If too slow, decrease linPause
    lastMillis = millis();
    defineWiperFrame();  // build the wiper frame before sending.  In '_LIN.ino'
    sendLINFrame();      // send the LIN frame.  In '_LIN.ino'
  }
}