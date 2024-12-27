// define pins
#define pinTX 16            // transmit pin for LIN
#define pinRX 17            // receive pin for LIN
#define pinWake 18          // wake for LIN
#define pinCS 19            // chip select for LIN

#define pinPos2 21          // stalk input > position 2
#define pinPos1 22          // stalk input > position 1
#define pinInt 23           // stalk input > intermittent

#define linBaud 9600        // LIN 2.x > 19.2kBaud
#define linWiperID 0x20     // Transmit under this ID as per datasheet (was 0x20)
#define linWiperDiag 0x32   // Diagnostic for wiper motor as per datasheet.  Assumed data return?
#define linPause 80         // Send packets every x ms ** CAN CHANGE THIS **

#define stateDebug 1        // State Debug - set to 0 on release ** CAN CHANGE THIS **
#define testLED 0           // test LEDs on startup ** DISABLE ON RELEASE**

#define blinkInterval       // blink LED interval

bool wiperSingle = false;   // Bool for single wipe
bool wiperInt = false;      // Bool for intermittent
bool wiperPos1 = false;     // Bool for position 1
bool wiperPos2 = false;     // Bool for position 2
uint8_t wiperFrame[4];      // For wiper frame output
uint8_t intervalSpeed = 1;  // Define interval speed ** CAN CHANGE THIS **
uint8_t lastIntervalSpeed = 1;  // Define interval speed ** CAN CHANGE THIS **
uint8_t frameCount = 0;     // Counter for wiper LIN frame count.  0 to 15 ** For Ford Motorport, not req. on Volvo based systems **
uint32_t lastMillis = 0;    // Counter for sending frames x ms
uint32_t blinkMillis = 0;    // Counter for sending frames x ms