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
  // add 4 to the interval speed.  Rollover at >7
  intervalSpeed++;
  preferences.putInt("intervalSpeed", intervalSpeed);

  if (intervalSpeed >= 7) {
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
// end button attach