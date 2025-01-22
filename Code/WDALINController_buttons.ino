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
  switch (linType) {
    case LIN_WDA:
      intervalSpeed = intervalSpeed + 4;  // Bosch starts at 1 and adds 4 each time
      if (intervalSpeed >= maxIntervalCount) {
        intervalSpeed = 1;
      }
      break;
    case LIN_Ford:
      intervalSpeed++;  // Ford starts at 1 and adds 1 each time
      if (intervalSpeed >= maxIntervalCount) {
        intervalSpeed = 1;
      }
      break;
    case LIN_VW:
      intervalSpeed++;  // Ford starts at 1 and adds 1 each time
      if (intervalSpeed > maxIntervalCount) {
        intervalSpeed = 1;
      }
      break;
    default:
      // do nothing
      break;
  }

  preferences.putInt("intervalSpeed", intervalSpeed);
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
// end button attach