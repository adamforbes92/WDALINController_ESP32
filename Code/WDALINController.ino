/*********************
Ford / Bosch LIN Wiper Motor Controller.  Defines LIN packet to send over LIN using wiper stalk inputs (pull low to activate)
A LONG hold then release on the interval button will DECREASE the interval speed
Designed & Built for Gavin Bey (@beytek_)

** Assumes Ford / Volvo Wiper Motor ** 
** Uses LIN 2.x specification (and therefore checksum)
  if (!((this->version == LIN_V1) || (pid == 0x3C) || (pid == 0x7D)))    // if version 2  & no diagnostic frames (0x3C=60 (PID=0x3C) or 0x3D=61 (PID=0x7D))
    //chk = (uint16_t) pid;
    >>> NOTE slashes above, chk isn't calculated correctly - this is coded out in the Lib-"LIN_master_portable_Arduino-main"(!) 

** Bosch 'Motorsport' Wiper details are included but commented out **
** Assumes power (KL15) / Ignition Live means this module is powered and therefore the wipers are, so force active.  Possible revision to leave on KL30 and monitor ign. signal? **

  // LIN2.x uses extended checksum which includes protected ID, i.e. including parity bits
  // LIN1.x uses classical checksum only over data bytes
  // Diagnostic frames with ID 0x3C and 0x3D/0x7D always use classical checksum (see LIN spec "2.3.1.5 Checkum")

Forbes-Automotive.com; 2024
**********************/

#include "LIN_master_HardwareSerial_ESP32.h"
#include <ButtonLib.h>  //include the declaration for this class
#include "WDALINController_defs.h"
#include <Preferences.h>  // stores settings - mainly for last interval speed

// Setup 'button' library for inputs - 'true' is internal pullup
buttonClass btnInt(pinInt, true);
buttonClass btnPos1(pinPos1, true);
buttonClass btnPos2(pinPos2, true);

// Setup LIN library for Serial2, define pins
LIN_Master_HardwareSerial_ESP32 LIN(Serial2, pinRX, pinTX, "LIN_HW");  // parameters: interface, Rx, Tx, name

// EEPROM
Preferences preferences;

void setup() {
  basicInit();  // basic init in _io.  Keeps this page clean because of all of the 'serial_debugs'
}

void loop() {
  btnInt.tick();   // using the 'button' library to detect held or single clicks
  btnPos1.tick();  // using the 'button' library to detect held or single clicks
  btnPos2.tick();  // using the 'button' library to detect held or single clicks
  LIN.handler();   // tick over the LIN handler

  if (intervalSpeed != lastIntervalSpeed) {
    lastIntervalSpeed = intervalSpeed;
    blinkLED(200, intervalSpeed, false, true);

#if stateDebug
    Serial.println(intervalSpeed);
#endif
  }

  defineWiperFrame();  // build the wiper frame before sending

  if ((millis() - lastMillis) > linPause) {  // check to see if x ms (linPause) has elapsed - slow down the frames!
    lastMillis = millis();
    sendLINFrame();  // send the LIN frame
  }
}


void defineWiperFrame() {
  /*if (Ford) {
        // Frame 0:
        // add one to frame count, if 15, roll back to 0
        frameCount++;
    if (frameCount > 15) {
      frameCount = 0;
    }

    wiperFrame[0] = frameCount;  // Bit 0-3: Counter.  Bit 4: KL.15. Bit 5: KL.X. Bit 6: 0. Bit 7: 0.
    // set individual 'bits' on the frame for KL terms (fixed - if we have power, the wipers should be 'active')
    bitWrite(wiperFrame[0], 4, 1);  // KL.15 (Forced 1) - assumed power, so use wipers
    bitWrite(wiperFrame[0], 5, 1);  // KL.X (Forced 1) - assumed power, so use wipers
    bitWrite(wiperFrame[0], 6, 0);  // 0
    bitWrite(wiperFrame[0], 7, 0);  // 0

    // Frame 1
    // set interval speed from static number stored at top (1=1, 2=5, 3=9, 4=13)
    wiperFrame[1] = intervalSpeed;  // Bit 0-3: Counter.  Bit 4: KL.15. Bit 5: KL.X. Bit 6: 0. Bit 7: 0.
    // set individual 'bits' on the frame for wiper direction (based on inputs): single strike, intermittent, SPD1, SPD2
    bitWrite(wiperFrame[1], 4, wiperSingle);  // single strike
    bitWrite(wiperFrame[1], 5, wiperInt);     // intermittent
    bitWrite(wiperFrame[1], 6, wiperPos1);    // SPD1
    bitWrite(wiperFrame[1], 7, wiperPos2);    // SPD2

    // Frame 3 through 5 are 'empty' / not used, so send over 0x00
    // empty frames
    wiperFrame[2] = 0x00;  // empty, empty, empty, empty, empty
    wiperFrame[3] = 0x00;  // empty, empty, empty, empty, empty
    wiperFrame[4] = 0x00;  // empty, empty, empty, empty, empty
    wiperFrame[5] = 0x00;  // empty, empty, empty, empty, empty
    wiperFrame[6] = 0x00;  // empty, empty, empty, empty, empty
    wiperFrame[7] = 0x00;  // empty, empty, empty, empty, empty
  }*/

  // build the frame - get the easy ones out the way first
  switch (intervalSpeed) {
    case 1:
      wiperFrame[0] = 0xA0;
      break;
    case 2:
      wiperFrame[0] = 0xA1;
      break;
    case 3:
      wiperFrame[0] = 0xA2;
      break;
    case 4:
      wiperFrame[0] = 0xA3;
      break;
    case 5:
      wiperFrame[0] = 0xA4;
      break;
    case 6:
      wiperFrame[0] = 0xA5;
      break;
  }
  //wiperFrame[1] = 0x03;  // speed: 0x00=off; 0x01=int; 0x02=slow; 0x03=fast; 0x04=3x wipes, pause? 0x05=off?
  if (!wiperInt && !wiperPos1 && !wiperPos2) {
    wiperFrame[1] = 0x00;  // off
  }
  if (wiperInt) {
    wiperFrame[1] = 0x01;  // int
  }
  if (wiperPos1) {
    wiperFrame[1] = 0x02;  // slow
  }
  if (wiperPos2) {
    wiperFrame[1] = 0x03;  // fast
  }
  wiperFrame[2] = 0x03;  // set interval speed from static number stored at top (1=1, 2=5, 3=9, 4=13) (ORI 0x03)
  wiperFrame[3] = 0x00;  // ORI 0x00?
}

void sendLINFrame() {
  //LIN_Master_Base::error_t error;
  LIN_Master_Base::frame_t Type;
  LIN_Master_Base::error_t error;
  uint8_t Id;
  uint8_t NumData;
  uint8_t Data[8];
  if (LIN.getState() == LIN_Master_Base::STATE_DONE) {
    error = LIN.getError();

    // reset state machine & error
    LIN.resetStateMachine();
    LIN.resetError();

  }  // if LIN frame finished
  LIN.sendMasterRequest(LIN_Master_Base::LIN_V2, linWiperID, 4, wiperFrame);
}