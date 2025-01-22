/* various LIN types defined here - Bosch WDA (for the motorsport wiper), Ford (Focus), VW (Caddy), Unknown
Bosch WDA is +£1000 so will typically be unused but documented for reference
  -- baud 19200, wiperID 0x31, diag 0x32 as per datasheet
  -- 'maxIntervalCount' should be changed to suit the Ford system (15)

Ford (Focus) - uses a master/slave motor, the master frames are constructed using an 'on the bench' example so while it works, may not be 100% accurate - refinement/frame examples would be good
  -- baud 9600, wiperID 0x20 
  -- 'maxIntervalCount' should be changed to suit the Ford system (7)

VW (Caddy) - uses a 2016 UK Caddy, frames captured from a running vehicle and all modes tested
  -- baud 19200, wiperID 0xF0
  -- 'maxIntervalCount' should be changed to suit the VW Stalk (4)

Unknown - explicitly used to CAPTURE frames from a vehicle - will output results into the Serial monitor.  Consider changing LIN Protocol (V1 / V2) to confirm checksums etc.
*/

// define LIN types - currently available for WDA, Ford & VW.  If logging an unknown vehicle, choose 'Unknown' and capture the frames
typedef enum linTypes {
  LIN_WDA,
  LIN_Ford,
  LIN_VW,
  LIN_UNKNOWN
} linTypes;

#define linType LIN_VW          // change to suit final platform ** CAN CHANGE THIS **
#define triggerType 1           // change to suit trigger type - either ground to activate (0) or 12v to activate (1) ** CAN CHANGE THIS **

// general debug statements
#define serialDebug 0           // State Debug ** DISABLE ON RELEASE **
#define serialLIN 0             // print LIN frames ** DISABLE ON RELEASE **
#define testLED 0               // test LEDs on startup ** DISABLE ON RELEASE**

#define ledIndicator 1          // follow the button presses with the LEDs ** CAN CHANGE THIS **
#define ledOnLIN 1              // blink on-board LED on LIN frame sending - good to know if it's still awake... ** CAN CHANGE THIS **
#define blinkInterval 200       // blink LED interval for showing changes in interval speed.  May be too long and wiper motor gets confused; consider reducing

// define pins
#define pinTX 17                // transmit pin for LIN / TJA1020
#define pinRX 16                // receive pin for LIN / TJA1020

// define LIN parameters
#define linBaud 19200           // LIN 2.x > 19.2kBaud.  VW=19200; Ford=9600; WDA=9600 ** CAN CHANGE THIS **
#define linWiperID 0xF0         // Transmit under this ID as per datasheet (was 0x20).  VW=0x20; Ford=0x20; WDA=0x31 ** CAN CHANGE THIS **
#define linWiperDiag 0x32       // Diagnostic for wiper motor as per datasheet.  Assumed data return?  Could be 0xB1 on VW - not tested
#define linPause 120            // Send packets every x ms ** CAN CHANGE THIS **

// pin inputs for negative triggers
#define pinIntNeg 21            // stalk input > intermittent (Ground to activate)
#define pinPos1Neg 22           // stalk input > position 1 (Ground to activate)
#define pinPos2Neg 23           // stalk input > position 2 (Ground to activate)// define input pins for wiper stalk

// pin inputs for positive triggers
#define pinIntPos 25            // stalk input > intermittent (12V to activate)
#define pinPos1Pos 26           // stalk input > position 1 (12V to activate)
#define pinPos2Pos 27           // stalk input > position 2 (12V to activate)// define input pins for wiper stalk

// pin outputs for LED monitoring
#define pinIntLED 18            // stalk input > intermittent
#define pinPos1LED 19           // stalk input > position 1 
#define pinPos2LED 14           // stalk input > position 2

// define input pins for wiper stalk
bool wiperSingle = false;       // Bool for single wipe
bool wiperInt = false;          // Bool for intermittent
bool wiperPos1 = false;         // Bool for position 1
bool wiperPos2 = false;         // Bool for position 2
bool hasError = false;          // for displaying visual has error with built-in LED

uint8_t wiperFrame[8];          // For wiper frame output
uint8_t intervalSpeed = 1;      // Define interval speed ** CAN CHANGE THIS **
uint8_t lastIntervalSpeed = 1;  // Define interval speed ** CAN CHANGE THIS **
uint8_t maxIntervalCount = 7;   // maximum interval count - for how many 'intervals' you want to store ** CAN CHANGE THIS **

uint8_t frameCount = 10;         // Counter for wiper LIN frame count.  0 to 15 For Ford Motorport; 10 to 31 on VW
uint32_t lastMillis = 0;        // Counter for sending frames x ms
uint32_t blinkMillis = 0;       // Counter for sending frames x ms
