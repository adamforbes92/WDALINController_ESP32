/*********************
Ford / Bosch LIN Wiper Motor Controller.  Defines LIN packet to send over LIN using wiper stalk inputs (pull low to activate)
Designed & Built for Gavin Bey (@beytek_)

** Assumes Ford / Volvo Wiper Motor ** 

** Bosch 'Motorsport' Wiper details are included but commented out **
** Assumes power (KL15) / Ignition Live means this module is powered and therefore the wipers are, so force active.  Possible revision to leave on KL30 and monitor ign. signal? **

Forbes-Automotive.com; 2024
**********************/

#include "LIN_master_HardwareSerial_ESP32.h"
#include <ButtonLib.h>  //include the declaration for this class

// Define Pins
#define pinTX 16           // transmit pin for LIN
#define pinRX 17           // receive pin for LIN
#define pinWake 18         // wake for LIN
#define pinCS 19           // chip select for LIN

#define pinPos2 21         // stalk input > position 2
#define pinPos1 22         // stalk input > position 1
#define pinInt 23          // stalk input > intermittent

#define linBaud 9600       // LIN 2.x > 19.2kBaud
#define linWiperID 0x20    // Transmit under this ID as per datasheet
#define linWiperDiag 0x32  // Diagnostic for wiper motor as per datasheet.  Assumed data return?
#define linPause 20        // Send packets every x ms ** CAN CHANGE THIS **

#define stateDebug 0       // State Debug - set to 0 on release ** CAN CHANGE THIS **
#define testLED 1          // test LEDs on startup ** CAN CHANGE THIS**

// Define variables
bool wiperSingle = false;   // Bool for single wipe
bool wiperInt = false;      // Bool for intermittent
bool wiperPos1 = false;     // Bool for position 1
bool wiperPos2 = false;     // Bool for position 2
uint8_t wiperFrame[4];      // For wiper frame output
uint8_t intervalSpeed = 1;  // Define interval speed (1=1, 2=5, 3=9, 4=13) ** CAN CHANGE THIS **
uint8_t frameCount = 0;     // Counter for wiper LIN frame count.  0 to 15 ** For Ford Motorport, not req. on Volvo based systems **
uint16_t lastMillis = 0;    // Counter for sending frames x ms

// Setup 'button' library for inputs - 'true' is internal pullup
buttonClass btnInt(pinInt, true);
buttonClass btnPos1(pinPos1, true);
buttonClass btnPos2(pinPos2, true);

// Setup LIN library for Serial2, define pins
LIN_Master_HardwareSerial_ESP32 LIN(Serial2, pinRX, pinTX, "LIN_HW");  // parameter: interface, Rx, Tx, name

void setup() {
#if stateDebug
  Serial.begin(115200);
  Serial.println(F("Ford LIN Wiper Controller Initialising..."));
#endif
  setupPins();  // setup pins for IO
  LIN.begin(linBaud);

#if stateDebug
  Serial.println(F("Ford LIN Wiper Controller Initialised, LIN started!"));
#endif

#if stateDebug
  Serial.println(F("Setting up buttons as inputs..."));
#endif
  setupButtons();
#if stateDebug
  Serial.println(F("Buttons setup!"));
#endif
}

void loop() {
  btnInt.tick();   // using the 'button' library to detect held or single clicks
  btnPos1.tick();  // using the 'button' library to detect held or single clicks
  btnPos2.tick();  // using the 'button' library to detect held or single clicks

  // build the wiper frame before sending
  defineWiperFrame();

  // check to see if x ms (linPause) has elapsed - slow down the frames!
  if (millis() - lastMillis > linPause) {
    lastMillis = millis();
    sendLINFrame(); // send the LIN frame
  }
}

void setupPins() {
  // setup the pins for input/output
  pinMode(pinCS, OUTPUT);
  pinMode(pinWake, OUTPUT);

  // drive CS & Wake high to use the LIN chip
  digitalWrite(pinCS, HIGH);
  digitalWrite(pinWake, HIGH);

// setup the wiper position inputs
#if testLED
  pinMode(pinInt, OUTPUT);
  pinMode(pinPos1, OUTPUT);
  pinMode(pinPos2, OUTPUT);
  blinkLED(500, 5);
#endif

  // reset buttons if testLED is used (can be removed if 'testLED' is not used but keeping here for solidness)
  pinMode(pinInt, INPUT_PULLUP);
  pinMode(pinPos1, INPUT_PULLUP);
  pinMode(pinPos2, INPUT_PULLUP);
}

void setupButtons() {
  //setup buttons / inputs
  btnInt.attachSingleClick(intSingle);           // call intSingle on a single click (single wipe)
  btnInt.attachSingleNotHeld(intSingleRelease);  // call intSingleRelease on a single click release (clears flags)
  btnInt.attachDoubleClick(intAdjustSpeed);      // call adjustSpeed on double-click on intermediate - will +4 on the speed until overflow
  btnInt.attachContinuousHold(intHold);          // call intHold if the button is 'always down'
  btnInt.attachContinuousNotHeld(intRelease);    // call intRelease if the button is now not held (clears flags)
  btnPos1.attachContinuousHold(pos1Hold);        // call pos1Hold if the button is 'always down'
  btnPos1.attachContinuousNotHeld(pos1Release);  // call pos1Release if the button is now not held (clears flags)
  btnPos2.attachContinuousHold(pos2Hold);        // call pos1Hold if the button is 'always down'
  btnPos2.attachContinuousNotHeld(pos2Release);  // call pos2Release if the button is now not held (clears flags)
}

void defineWiperFrame() {
  /*
  if (Ford) {
    for
      Ford Motorsport Wiper Motor
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
  }
*/

  // build the frame - get the easy ones out the way first
  wiperFrame[0] = 0xA2;  // always 0xA2?
  wiperFrame[2] = 0x03;  // always 0x03?
  wiperFrame[3] = 0x00;  // always 0x00?

  if (!wiperInt && !wiperPos1 && !wiperPos2) {
    wiperFrame[1] = 0x00;  // speed?
  }

  if (wiperInt) {
    wiperFrame[1] = 0x01;  // speed?
  }
  if (wiperPos1) {
    wiperFrame[1] = 0x02;  // speed?
  }
  if (wiperPos2) {
    wiperFrame[1] = 0x03;  // speed?
  }

  //checksum shoudl be 0x59
  // 0x00 is stop
  // 0x01 is intermittent
  // 0x02 is slow
  // 0x03 is fast
  // 0x04 is slow
  // 0x05 is fast then stop?
  // 0x06 is fast then stop?
}

void sendLINFrame() {
  LIN_Master::error_t error;

  // send master request frame and get result immediately
  error = LIN.sendMasterRequestBlocking(LIN_Master::LIN_V2, linWiperID, 4, wiperFrame);
  LIN.resetStateMachine();
  LIN.resetError();
}

// begin diag
void printBits(byte myByte) {
  // for Serial debug - used to check the binary leaving...
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}
void blinkLED(int duration, int flashes) {
  for (int i = 0; i < flashes; i++) {
    delay(duration);
    digitalWrite(pinInt, HIGH);
    digitalWrite(pinPos1, HIGH);
    digitalWrite(pinPos2, HIGH);
    delay(duration);
    digitalWrite(pinInt, LOW);
    digitalWrite(pinPos1, LOW);
    digitalWrite(pinPos2, LOW);
  }
}
// end diag

// begin attachments for buttons
void intSingle() {
  wiperSingle = true;  // set the 'single wipe' flag
  wiperInt = false;    // clear the 'intermittent' flag - robustness
}
void intSingleRelease() {
  wiperSingle = false;  // clear the flags - robustness
  wiperInt = false;     // clear the flags - robustness
}
void intAdjustSpeed() {
  // add 4 to the interval speed.  Rollover at >13
  intervalSpeed = intervalSpeed + 4;
  if (intervalSpeed >= 13) {
    intervalSpeed = 1;
  }
}
void intHold() {
  wiperInt = true;
}
void intRelease() {
  wiperInt = false;
}
void pos1Hold() {
  wiperPos1 = true;
}
void pos1Release() {
  wiperPos1 = false;
}
void pos2Hold() {
  wiperPos2 = true;
}
void pos2Release() {
  wiperPos2 = false;
}
// end button attachments